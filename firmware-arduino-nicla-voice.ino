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
#include "ei_syntiant_ndp120.h"
#include "Nicla_System.h"

#include "mbed.h"
#include "rtos.h"

/* Private variables ------------------------------------------------------- */

/* Public functions -------------------------------------------------------- */
void NDP_INT_service(int match){ // This is developer's sandbox
    switch (match){
    case 0:
        break;
    case 11:
        nicla::leds.setColor(green);                       // Setting up RGB = Green
        ei_printf("Classifier 10 detected");            // printing on the native console
        delay(1000);                                        // Delay to keep the LED on for 1 second. Reduce it to reduce total power consumption
        nicla::leds.setColor(off);                        // Turning off the Green LED
        break;
    default:
        break;
    }
}

void setup() {
    char mcu_fw[] = {"mcu_fw_120_v90.synpkg"};
    char dsp_fw[] = {"dsp_firmware_v90.synpkg"};
    char model[] = {"ei_model.synpkg"};

    ei_setup(mcu_fw, dsp_fw, model);
    /* Done with EI init */
}

void loop() {
    ei_main();
    delay(10);
}
