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
#ifndef EI_DEVICE_SYNTIANT_NICLA_
#define EI_DEVICE_SYNTIANT_NICLA_

#include "ei_device_info_lib.h"
#include "ei_ext_flash_nicla_syntiant.h"
#include "ei_flash_nicla_syntiant.h"
#include "mbed.h"
#include "Stream.h"

#define WITH_IMU    0

/** Sensors */
typedef enum
{
    MICROPHONE      = 0,
#if WITH_IMU == 1
    ACCELEROMETER   = 1,
    MAGNETOMERTER   = 2
#endif
    MAX_SENSORS_NUMBER
}used_sensors_t;

/** Number of sensors used */
#define EI_DEVICE_N_SENSORS		MAX_SENSORS_NUMBER

class EiDeviceSyntiantNicla: public EiDeviceInfo {
    private:
        ei_device_sensor_t sensors[EI_DEVICE_N_SENSORS];
        void init_device_id(void);        
        EiExtFlashMemory *_external_flash;
        bool is_flashing;

        //
    public:
        ~EiDeviceSyntiantNicla();
        EiDeviceSyntiantNicla(EiDeviceMemory* internal_flash, EiExtFlashMemory* ext_mem);

        bool get_sensor_list(const ei_device_sensor_t **sensor_list, size_t *sensor_list_size);
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

};

extern mbed::Stream* ei_get_serial(void);

/* Reference to object for external usage ---------------------------------- */
//extern EiDeviceSyntiantNicla EiDevice;

#endif  /* */