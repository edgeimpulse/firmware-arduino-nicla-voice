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
/* Include ----------------------------------------------------------------- */
#include "ei_syntiant_ndp120.h"
#include "Nicla_System.h"
#include "NDP.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "ingestion-sdk-platform/nicla_syntiant/ei_at_handlers.h"
#include "ingestion-sdk-platform/nicla_syntiant/ei_device_syntiant_nicla.h"
#include "ingestion-sdk-platform/sensors/ei_inertial.h"
#include "inference/ei_run_impulse.h"
#ifdef WITH_IMU
#include "ingestion-sdk-platform/sensors/ei_inertial.h"
#include "model-parameters/model_metadata.h"
#else
#include "ingestion-sdk-platform/sensors/ei_microphone.h"
#endif

#include "rtos.h"
#include "Thread.h"
#include "EventQueue.h"

#define TEST_READ_TANK      0

/* device class */
extern NDPClass NDP;
static ATServer *at;
static bool ndp_is_init;

#if TEST_READ_TANK == 1
static void test_ndp_extract(void);
#endif


static void error_event(void);
static void match_event(char* label);
static void irq_event(void);

static bool _on_match_enabled = false;
static volatile bool got_match = false;
static volatile bool got_event = false;

/* Public functions -------------------------------------------------------- */
/**
 * @brief 
 * 
 */
void ei_setup(char* fw1, char* fw2, char* fw3)
{
    uint8_t valid_synpkg = 0;    
    bool board_flashed = false;
    uint8_t flashed_count = 0;
    EiDeviceSyntiantNicla *dev = static_cast<EiDeviceSyntiantNicla*>(EiDeviceInfo::get_device());
    char* ptr_fw[] = {fw1, fw2, fw3};

    ndp_is_init = false;
    Serial.begin(115200);

    nicla::begin();
    nicla::disableLDO();    // needed 
    nicla::leds.begin();

    while (!Serial) {   /* if Serial not avialable */
        nicla::leds.setColor(red);
    }
    
    ei_printf("Hello from Edge Impulse on Arduino Nicla Voice\r\n"
            "Compiled on %s %s\r\n",
            __DATE__,
            __TIME__);

    nicla::leds.setColor(green);
    //NDP.onError(error_event);
    NDP.onEvent(irq_event);
    NDP.onMatch(match_event);

    dev->get_ext_flash()->init_fs();
    for (int8_t i = 0; i < 3 ; i++) {
        if (ptr_fw[i] != nullptr){                  // nullptr check
            if (dev->get_file_exist(ptr_fw[i])){
                ei_printf("%s exist\n", ptr_fw[i]);
                valid_synpkg++;
            }
            else{
                ei_printf("%s not found!\n", ptr_fw[i]);
            }
        }
    }
    //dev->get_ext_flash()->deinit_fs();  // de init as NDP re init it

    if (valid_synpkg == 3){
        NDP.begin(fw1);
        NDP.load(fw2);
        NDP.load(fw3);
        NDP.getInfo();
        ndp_is_init = true;

#ifdef WITH_IMU
        NDP.configureInferenceThreshold(EI_CLASSIFIER_NN_INPUT_FRAME_SIZE);
#else   
        NDP.turnOnMicrophone();     
        NDP.getAudioChunkSize();    /* otherwise it is not initialized ! */
#endif
        NDP.interrupts();

        ei_syntiant_set_match();
        nicla::leds.setColor(off);
    }
    else{
        ei_printf("NDP not properly initialized\n");
        nicla::leds.setColor(red);
    }
    
    dev->get_ext_flash()->init_fs();    // NDP probably will de init and unmount

    /* init ar server */
    at = ei_at_init(dev);

    /* start inference */
    if (ndp_is_init == true) {
        /* sensor init */
        ei_inertial_init();
        ei_run_nn_normal();
    }    

    ei_printf("Type AT+HELP to see a list of commands.\r\n");
    at->print_prompt();
}

/**
 * @brief 
 * 
 */
void ei_main(void)
{
    int match = -1;
    /* handle command comming from uart */
    char data = Serial.read();

    while (data != 0xFF) {
        at->handle(data);

        if (ei_run_impulse_is_active() && data == 'b') {
            ei_start_stop_run_impulse(false);
        } 
        
        data = Serial.read();
    }

    if (ei_run_impulse_is_active() ==true) {
        if (got_match == true){
            got_match = false;
            nicla::leds.setColor(blue);
            ThisThread::sleep_for(100);
            nicla::leds.setColor(off);
        }

        if (got_event == true){
            got_event = false;
            nicla::leds.setColor(green);
            ThisThread::sleep_for(100);
            nicla::leds.setColor(off);
            match = NDP.poll();
        }

        if (match > 0) {
            ei_printf("match: %d\n", match);
            match = -1;
        }

#ifdef WITH_IMU
        // for now by default we stay in inference
        if (ei_run_impulse_is_active()) {
            ei_run_impulse();
        }
#endif
    }

}


/**
 * @brief disable interrupt from NDP class
 * 
 */
void ei_syntiant_clear_match(void)
{
    _on_match_enabled = false;
    //NDP.turnOffMicrophone();
    //NDP.noInterrupts();
}

/**
 * @brief enable interrupt from NDP clas
 * 
 */
void ei_syntiant_set_match(void)
{
    _on_match_enabled = true;
    //NDP.turnOnMicrophone();
    //NDP.interrupts();
}

/**
 * @brief Callback when an Error is triggered
 * @note it never exit!
 */
static void error_event(void)
{
    nicla::leds.begin();
    while (1) {
        nicla::leds.setColor(red);
        ThisThread::sleep_for(250);
        nicla::leds.setColor(off);
        ThisThread::sleep_for(250);
    }
    nicla::leds.end();
}

/**
 * @brief Callback when a Match is triggered
 * 
 * @param label The Match label
 */
static void match_event(char* label)
{
    if (_on_match_enabled == true) {
        if (strlen(label) > 0) {
            got_match = true;            
            ei_printf("Match: %s\n", label);            
        }
    }
}

/**
 * @brief 
 * 
 */
static void irq_event(void)
{    
    if (_on_match_enabled == true) {
        got_event = true;
    }
}
