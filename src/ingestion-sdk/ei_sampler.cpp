/* Edge Impulse ingestion SDK
 * Copyright (c) 2020 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Include ----------------------------------------------------------------- */
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include <stdint.h>
#include <stdlib.h>

#include "ei_sampler.h"
#include "ei_config_types.h"
#include "ingestion-sdk-platform/nicla_syntiant/ei_device_syntiant_nicla.h"
#include "Nicla_System.h"

#include "mbed.h"
#include "ingestion-sdk-platform/nicla_syntiant/ei_flash_nicla_syntiant.h"
#include "sensor_aq_mbedtls_hs256.h"

using namespace rtos;
using namespace events;

/* Private variables ------------------------------------------------------- */
static uint32_t samples_required;
static uint32_t current_sample;
static uint32_t sample_buffer_size;
static uint32_t headerOffset = 0;

static char write_word_buf[32];
static int write_addr = 0;

EI_SENSOR_AQ_STREAM stream;

/* Private function prototypes --------------------------------------------- */
static bool sample_data_callback(const void *sample_buf, uint32_t byteLenght);
static bool create_header(sensor_aq_payload_info *payload);
static void ei_write_last_data(void);
static int ei_seek(EI_SENSOR_AQ_STREAM*, long int offset, int origin);
static size_t ei_write(const void *buffer, size_t size, size_t count, EI_SENSOR_AQ_STREAM*);

static unsigned char ei_mic_ctx_buffer[1024];
static sensor_aq_signing_ctx_t ei_mic_signing_ctx;
static sensor_aq_mbedtls_hs256_ctx_t ei_mic_hs_ctx;
static sensor_aq_ctx ei_mic_ctx = {
    { ei_mic_ctx_buffer, 1024 },
    &ei_mic_signing_ctx,
    &ei_write,
    &ei_seek,
    NULL,
};

/* Public function --------------------------------------------------------- */
/**
 * @brief      Setup and start sampling, write CBOR header to flash
 *
 * @param      v_ptr_payload  sensor_aq_payload_info pointer hidden as void
 * @param[in]  sample_size    Number of bytes for 1 sample (include all axis)
 *
 * @return     true if successful
 */
bool ei_sampler_start_sampling(void *v_ptr_payload, starter_callback ei_sample_start, uint32_t sample_size)
{
#ifndef WITH_IMU
    return false;
#else
    EiDeviceSyntiantNicla *dev = static_cast<EiDeviceSyntiantNicla*>(EiDeviceInfo::get_device());
    EiExtFlashMemory* ext_mem = dev->get_ext_flash();
    sensor_aq_payload_info *payload = (sensor_aq_payload_info *)v_ptr_payload;

    ei_printf("Sampling settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float(dev->get_sample_interval_ms());
    ei_printf(" ms.\n");
    ei_printf("\tLength: %lu ms.\n", dev->get_sample_length_ms());
    ei_printf("\tName: %s\n", dev->get_sample_label().c_str());
    ei_printf("\tHMAC Key: %s\n", dev->get_sample_hmac_key().c_str());
    ei_printf("\tFile name: /fs/%s\n", dev->get_sample_label().c_str());

    samples_required = (uint32_t)((dev->get_sample_length_ms()) / dev->get_sample_interval_ms());
    sample_buffer_size = samples_required * sample_size * 2;
    current_sample = 0;

    // Minimum delay of 2000 ms for daemon
    if(((sample_buffer_size / ext_mem->get_erase_block_size())+1) * ext_mem->block_erase_time < 2000) {
        ei_printf("Starting in %lu ms... (or until all flash was erased)\n", 2000);
        ThisThread::sleep_for(2000 - ((sample_buffer_size / ext_mem->get_erase_block_size())+1) * ext_mem->block_erase_time);
    }
    else {
        ei_printf("Starting in %lu ms... (or until all flash was erased)\n",
        ((sample_buffer_size / ext_mem->get_erase_block_size())+1) * ext_mem->block_erase_time);
    }

	if(ext_mem->erase_sample_data(0, sample_buffer_size) != SPIF_BD_ERROR_OK) {
        ei_printf("ERR: Failed to erase sample memory\r\n");
		return false;
    }

    if(create_header(payload) == false)
        return false;

    if(ei_sample_start(&sample_data_callback, dev->get_sample_interval_ms()) == false)
        return false;

	ei_printf("Sampling...\n");
    nicla::leds.setColor(green);

    while(current_sample < samples_required) {
        __NOP();
        //ThisThread::sleep_for(10);
    };

    //dev->stop_sample_thread();

    ei_write_last_data();
    write_addr++;

    uint8_t final_byte[] = { 0xff };
    int ctx_err = ei_mic_ctx.signature_ctx->update(ei_mic_ctx.signature_ctx, final_byte, 1);
    if (ctx_err != 0) {
        return ctx_err;
    }

    // finish the signing
    ctx_err = ei_mic_ctx.signature_ctx->finish(ei_mic_ctx.signature_ctx, ei_mic_ctx.hash_buffer.buffer);

#if 0
    // load the first page in flash...
    uint8_t *page_buffer = (uint8_t*)ei_malloc(ext_mem->block_size);
    if (!page_buffer) {
        ei_printf("Failed to allocate a page buffer to write the hash\n");
        return false;
    }

    int j = ext_mem->read_sample_data(page_buffer, 0, ext_mem->block_size);
    if (j != ext_mem->block_size) {
        ei_printf("Failed to read first page\n");
        ei_free(page_buffer);
        return false;
    }

    // update the hash
    uint8_t *hash = ei_mic_ctx.hash_buffer.buffer;
    // we have allocated twice as much for this data (because we also want to be able to represent in hex)
    // thus only loop over the first half of the bytes as the signature_ctx has written to those
    for (size_t hash_ix = 0; hash_ix < (ei_mic_ctx.hash_buffer.size / 2); hash_ix++) {
        // this might seem convoluted, but snprintf() with %02x is not always supported e.g. by newlib-nano
        // we encode as hex... first ASCII char encodes top 4 bytes
        uint8_t first = (hash[hash_ix] >> 4) & 0xf;
        // second encodes lower 4 bytes
        uint8_t second = hash[hash_ix] & 0xf;

        // if 0..9 -> '0' (48) + value, if >10, then use 'a' (97) - 10 + value
        char first_c = first >= 10 ? 87 + first : 48 + first;
        char second_c = second >= 10 ? 87 + second : 48 + second;

        page_buffer[ei_mic_ctx.signature_index + (hash_ix * 2) + 0] = first_c;
        page_buffer[ei_mic_ctx.signature_index + (hash_ix * 2) + 1] = second_c;
    }

    if(ext_mem->erase_sample_data(0, ext_mem->block_size) != SPIF_BD_ERROR_OK) {
        ei_printf("ERR: Failed to erase sample memory\r\n");
        ei_free(page_buffer);
		return false;
    }

    j = ext_mem->write_sample_data(page_buffer, 0, ext_mem->block_size);
    //ext_mem->flush_data();
    ei_free(page_buffer);

    if (j != ext_mem->block_size) {
        ei_printf("Failed to write first page with updated hash\n");
        return false;
    }
#endif

    ei_printf("Done sampling, total samples collected: %u\n", current_sample);
    ei_printf("[1/1] Uploading file to Edge Impulse...\n");
    ei_printf("Not uploading file, not connected to WiFi. Used buffer, from=0, to=%lu.\n", write_addr + headerOffset);//sample_buffer_size + headerOffset);
    ei_printf("[1/1] Uploading file to Edge Impulse OK (took 0 ms.)\n");
    ei_printf("OK\n");

    nicla::leds.setColor(off);

    return true;
#endif
}


static bool create_header(sensor_aq_payload_info *payload)
{
    EiDeviceSyntiantNicla *dev = static_cast<EiDeviceSyntiantNicla*>(EiDeviceInfo::get_device());
    EiExtFlashMemory* ext_mem = dev->get_ext_flash();

    sensor_aq_init_mbedtls_hs256_context(&ei_mic_signing_ctx, &ei_mic_hs_ctx, dev->get_sample_hmac_key().c_str());

    int tr = sensor_aq_init(&ei_mic_ctx, payload, NULL, true);

    if (tr != AQ_OK) {
        ei_printf("sensor_aq_init failed (%d)\n", tr);
        return false;
    }
    // then we're gonna find the last byte that is not 0x00 in the CBOR buffer.
    // That should give us the whole header
    size_t end_of_header_ix = 0;
    for (size_t ix = ei_mic_ctx.cbor_buffer.len - 1; ix >= 0; ix--) {
        if (((uint8_t*)ei_mic_ctx.cbor_buffer.ptr)[ix] != 0x0) {
            end_of_header_ix = ix;
            break;
        }
    }

    if (end_of_header_ix == 0) {
        ei_printf("Failed to find end of header\n");
        return false;
    }

    // Write to blockdevice
    tr = ext_mem->write_sample_data((const uint8_t*)ei_mic_ctx.cbor_buffer.ptr, 0, end_of_header_ix);
    ei_printf("Try to write %d bytes\r\n", end_of_header_ix);
    if (tr != end_of_header_ix) {
        ei_printf("Failed to write to header blockdevice\n");
        return false;
    }
    
    ei_mic_ctx.stream = &stream;

    headerOffset = end_of_header_ix;
    write_addr = 0;

    return true;
}

/**
 * @brief      Write samples to FLASH in CBOR format
 *
 * @param[in]  sample_buf  The sample buffer
 * @param[in]  byteLenght  The byte lenght
 *
 * @return     true if all required samples are received. Caller should stop sampling,
 */
static bool sample_data_callback(const void *sample_buf, uint32_t byteLenght)
{
    sensor_aq_add_data(&ei_mic_ctx, (float *)sample_buf, byteLenght / sizeof(float));

    if (++current_sample >= samples_required) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * @brief      Write out remaining data in word buffer to FLASH.
 *             And append CBOR end character.
 */
static void ei_write_last_data(void)
{
    EiDeviceSyntiantNicla *dev = static_cast<EiDeviceSyntiantNicla*>(EiDeviceInfo::get_device());
    EiExtFlashMemory* ext_mem = dev->get_ext_flash();

    uint8_t fill = ((uint8_t)write_addr & 0x1F);
    uint8_t insert_end_address = 0;

    if (fill != 0x00) {
        for (uint8_t i = fill; i < 32; i++) {
            write_word_buf[i] = 0xFF;
        }
        ext_mem->write_sample_data((const uint8_t*)write_word_buf, (write_addr & ~0x1F) + headerOffset, 32);
        insert_end_address = 32;
    }

    /* Write appending word for end character */
    for (uint8_t i = 0; i < 32; i++) {
        write_word_buf[i] = 0xFF;
    }
    ext_mem->write_sample_data((const uint8_t*)write_word_buf, (write_addr & ~0x1F) + headerOffset + insert_end_address, 32);
}

static int ei_seek(EI_SENSOR_AQ_STREAM*, long int offset, int origin)
{
    return 0;
}

static size_t ei_write(const void *buffer, size_t size, size_t count, EI_SENSOR_AQ_STREAM*)
{
    EiDeviceSyntiantNicla *dev = static_cast<EiDeviceSyntiantNicla*>(EiDeviceInfo::get_device());
    EiExtFlashMemory* ext_mem = dev->get_ext_flash();
    //uint32_t start_time = 0;
    //uint32_t stop_time = start_time;

    //start_time = rtos::Kernel::get_ms_count();

    for(int i=0; i<count; i++) {

        write_word_buf[write_addr& 0x1F] = *((char *)buffer++);

        if((++write_addr & 0x1F) == 0x00) {
            ext_mem->write_sample_data((const uint8_t*)write_word_buf, (write_addr - 32) + headerOffset, 32);
        }
    }

    //stop_time = rtos::Kernel::get_ms_count();
    //ei_printf("ei_write %lu \n", (stop_time - start_time));
    
    return count;
}
