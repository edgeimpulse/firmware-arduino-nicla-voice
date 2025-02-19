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
/* Includes --------------------------------------------------------------------- */
#include "ei_run_impulse.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/model_variables.h"
#include "ingestion-sdk-platform/sensors/ei_inertial.h"

bool _run_impulse = false;

/**
 * @brief      Start impulse, print settings
 */
void ei_run_nn_normal(void)
{
    _run_impulse = true;
    ei_start_stop_run_impulse(true);
    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf(" ms.\n");
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
            sizeof(ei_classifier_inferencing_categories[0]));

    ei_printf("Classes:\n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("\t%s\n", ei_classifier_inferencing_categories[ix]);
    }

    ei_printf("Starting inferencing, press 'b' to break\n");
#ifdef WITH_IMU
    ei_inertial_prepare_impulse();
#endif
}

/**
 * @brief      Called from the ndp120 read out. Print classification output
 *             and send matched output string to user callback
 *
 * @param[in]  matched_feature
 */
void ei_classification_output(int matched_feature)
{
    if (_run_impulse == true) {

        ei_printf("\nMatch: %d\n", matched_feature);
        ei_printf("Predictions:\r\n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: \t%d\r\n", ei_classifier_inferencing_categories[ix],
                (matched_feature == ix) ? 1 : 0);
        }

    }
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool ei_run_impulse_is_active(void)
{
    return _run_impulse;
}

/**
 * @brief 
 * 
 */
void ei_run_impulse(void)
{
#ifdef WITH_IMU    
    ei_inertial_send_to_ndp(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
    ei_sleep(EI_CLASSIFIER_INTERVAL_MS);
    
#endif
}

/**
 * @brief 
 * 
 * @param start 
 */
void ei_start_stop_run_impulse(bool start)
{    
    _run_impulse = start;

    if(!start) {
        ei_printf("Inferencing stopped by user\r\n");
    }
}
