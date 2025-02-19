/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef EI_DEVICE_SYNTIANT_NICLA_
#define EI_DEVICE_SYNTIANT_NICLA_

#include "ei_device_info_lib.h"
#include "ei_ext_flash_nicla_syntiant.h"
#include "ei_flash_nicla_syntiant.h"
#include "mbed.h"
#include "Stream.h"

/** Sensors */
typedef enum
{
#ifndef WITH_IMU
    MICROPHONE      = 0,
#endif
    MAX_SENSORS_NUMBER
}used_sensors_t;

/** Number of sensors used */
#define EI_DEVICE_N_SENSORS		MAX_SENSORS_NUMBER

class EiDeviceSyntiantNicla: public EiDeviceInfo {
    private:
#ifndef WITH_IMU
        ei_device_sensor_t sensors[EI_DEVICE_N_SENSORS];
#endif
        void init_device_id(void);        
        EiExtFlashMemory *_external_flash;
        bool is_flashing;

        //
    public:
        ~EiDeviceSyntiantNicla();
        EiDeviceSyntiantNicla(EiDeviceMemory* internal_flash, EiExtFlashMemory* ext_mem);

        bool get_sensor_list(const ei_device_sensor_t **sensor_list, size_t *sensor_list_size) override ;
        void clear_config(void);
        uint32_t get_max_data_output_baudrate(void)
        {
            return 230400;
        }
        void set_max_data_output_baudrate(void);
        void set_default_data_output_baudrate(void);

        void read_raw(uint8_t *buffer, uint32_t pos, uint32_t bytes);
        uint8_t wait_flash_model(void);
        bool get_file_exist(char* filename);
        bool get_is_flashing (void)
        {
            return this->is_flashing;
        }

        void test_flash(void)
        {
            Serial.println("Test flash\n");
            _external_flash->test_flash();
        }

        EiExtFlashMemory* get_ext_flash(void)
        {
            return _external_flash;
        }

#ifdef WITH_IMU
        bool start_sample_thread(void (*sample_read_cb)(void), float sample_interval_ms) override;
        bool stop_sample_thread(void) override ;
#endif

};

extern mbed::Stream* ei_get_serial(void);
extern void print_memory_info(void);

#endif  /* */