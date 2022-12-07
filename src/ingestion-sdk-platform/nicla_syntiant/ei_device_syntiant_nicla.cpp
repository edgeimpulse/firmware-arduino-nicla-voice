/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
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
#include "ei_device_syntiant_nicla.h"
#include "ei_flash_nicla_syntiant.h"
#include "ei_ext_flash_nicla_syntiant.h"
#include "mbed.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "ingestion-sdk-platform/sensors/ei_microphone.h"
#include "file-transfer/ymodem.h"
#include "Nicla_System.h"
#include "ei_syntiant_ndp120.h"


/**
 * @brief Construct a new Ei Device Syntiant Nicla:: Ei Device Syntiant Nicla object
 * 
 */
EiDeviceSyntiantNicla::EiDeviceSyntiantNicla(EiDeviceMemory* int_mem, EiExtFlashMemory* ext_mem)
{
    device_type = "ARDUINO_NICLA_VOICE";
    device_id = "02:04:06:08:10";   // will be overwritten by init_device_id

    memory = int_mem;
    _external_flash = ext_mem;
    
    sensors[MICROPHONE].name = "Microphone";
    sensors[MICROPHONE].frequencies[0] = 16000.0f;
    sensors[MICROPHONE].start_sampling_cb = &ei_microphone_start_sampling;
    sensors[MICROPHONE].max_sample_length_s = 5;   // TODO

    init_device_id();   // set ID
    load_config();
    is_flashing = false;
}

/**
 * @brief Destroy the Ei Device Syntiant Nicla:: Ei Device Syntiant Nicla object
 * 
 */
EiDeviceSyntiantNicla::~EiDeviceSyntiantNicla(void)
{

}

/**
 * @brief 
 * 
 * @param buffer 
 * @param pos 
 * @param bytes 
 */
void EiDeviceSyntiantNicla::read_raw(uint8_t *buffer, uint32_t pos, uint32_t bytes)
{
    _external_flash->read_sample_data(buffer, pos, bytes);
}

/**
 *
 * @param sensor_list
 * @param sensor_list_size
 * @return
 */
bool EiDeviceSyntiantNicla::get_sensor_list(const ei_device_sensor_t **sensor_list, size_t *sensor_list_size)
{
    *sensor_list      = sensors;
    *sensor_list_size = EI_DEVICE_N_SENSORS;

    return true;
}

/**
 * @brief 
 * 
 */
void EiDeviceSyntiantNicla::init_device_id(void)
{
    char temp[20];
    uint32_t read_device_id[2];        
    
    read_device_id[0] = NRF_FICR->DEVICEADDR[0];
    read_device_id[1] = NRF_FICR->DEVICEADDR[1];
    
    snprintf(temp, sizeof(temp), "%02x:%02x:%02x:%02x:%02x:%02x",
        (read_device_id[0] & 0xFF),
        ((read_device_id[0] & 0xFF00) >> 8),
        ((read_device_id[0] & 0xFF0000) >> 16),
        ((read_device_id[0] & 0xFF000000) >> 24),
        (read_device_id[1] & 0xFF),
        ((read_device_id[1] & 0xFF00) >> 8));

    device_id = std::string(temp);
}

/**
 * @brief 
 * 
 */
void EiDeviceSyntiantNicla::clear_config(void)
{
    EiDeviceInfo::clear_config();

    init_device_id();
    save_config();
}

/**
 * @brief 
 * 
 */
void EiDeviceSyntiantNicla::set_max_data_output_baudrate(void)
{
    Serial.end();
    Serial.begin(230400);
}

/**
 * @brief 
 * 
 */
void EiDeviceSyntiantNicla::set_default_data_output_baudrate(void)
{
    //
    Serial.end();
    Serial.begin(115200);
}

/**
 * @brief 
 * 
 */
uint8_t EiDeviceSyntiantNicla::wait_flash_model(void)
{
    uint8_t flashed = 0;
    uint8_t command = 0xFF;
    char filename[256] = {0};

    nicla::leds.setColor(blue);

    this->is_flashing = true;
    ei_syntiant_clear_match();
    //set_max_data_output_baudrate();

    while(flashed == 0){
        if (Serial.available()) {
            nicla::leds.setColor(green);

            command = Serial.read();
        }

        if (command == 'Y') {                    
            FILE* f = fopen("/fs/temp.bin", "wb");
            while (Serial.available()) {
                Serial.read();
            }
            Serial.write("Y\n");
            
            if(f == NULL){
                while(1)
                {
                    nicla::leds.setColor(off);
                    ei_sleep(250);
                    nicla::leds.setColor(red);
                    ei_sleep(250);
                }
            }
            
            int ret = Ymodem_Receive(f, 1024 * 1024, filename);
            String name = String(filename);
            if (ret > 0 && name != "") {
                name = "/fs/" + name;
                fclose(f);
                rename("/fs/temp.bin", name.c_str());                
            }
            else{
                fclose(f);
                remove("/fs/temp.bin");
                ei_printf("Error in file update\n");
            }

            
            flashed = 1;
            while (Serial.available()) {
                Serial.read();
            }
            delay(10);        
        }
    }

    //set_default_data_output_baudrate();
    this->is_flashing = false;
    ei_syntiant_set_match();
    
    nicla::leds.setColor(off);

    return flashed;
}

/**
 * @brief 
 * 
 * @param filename 
 * @return true 
 * @return false 
 */
bool EiDeviceSyntiantNicla::get_file_exist(char* filename)
{
    std::string file = filename;
    return _external_flash->get_file_exist(file);
}

/**
 * @brief get_device is a static method of EiDeviceInfo class
 * It is used to implement singleton paradigm, so we are returning
 * here pointer always to the same object (dev)
 * 
 * @return EiDeviceInfo* 
 */
EiDeviceInfo* EiDeviceInfo::get_device(void)
{
    /** Device object, for this class only 1 object should exist */
    static EiFlashMemory internal_flash(sizeof(EiConfig));
    static EiExtFlashMemory external_flash;
    static EiDeviceSyntiantNicla dev(&internal_flash, &external_flash);

    return &dev;
}
