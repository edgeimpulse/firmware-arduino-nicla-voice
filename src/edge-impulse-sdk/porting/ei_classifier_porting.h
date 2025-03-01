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

#ifndef _EI_CLASSIFIER_PORTING_H_
#define _EI_CLASSIFIER_PORTING_H_

#include <stdint.h>
#include <stdlib.h>

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C" {
#endif // defined(__cplusplus)

typedef enum {
    EI_IMPULSE_OK = 0,
    EI_IMPULSE_ERROR_SHAPES_DONT_MATCH = -1,
    EI_IMPULSE_CANCELED = -2,
    EI_IMPULSE_TFLITE_ERROR = -3,
    EI_IMPULSE_DSP_ERROR = -5,
    EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED = -6,
    EI_IMPULSE_CUBEAI_ERROR = -7,
    EI_IMPULSE_ALLOC_FAILED = -8,
    EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES = -9,
    EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE = -10,
    EI_IMPULSE_OUT_OF_MEMORY = -11,
    EI_IMPULSE_NOT_SUPPORTED_WITH_I16 = -12,
    EI_IMPULSE_INPUT_TENSOR_WAS_NULL = -13,
    EI_IMPULSE_OUTPUT_TENSOR_WAS_NULL = -14,
    EI_IMPULSE_SCORE_TENSOR_WAS_NULL = -15,
    EI_IMPULSE_LABEL_TENSOR_WAS_NULL = -16,
    EI_IMPULSE_TENSORRT_INIT_FAILED = -17
} EI_IMPULSE_ERROR;

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms);

/**
 * Check if the sampler thread was canceled, use this in conjunction with
 * the same signaling mechanism as ei_sleep
 */
EI_IMPULSE_ERROR ei_run_impulse_check_canceled();

/**
 * Read the millisecond timer
 */
uint64_t ei_read_timer_ms();

/**
 * Read the microsecond timer
 */
uint64_t ei_read_timer_us();

/**
 * Set Serial baudrate
 */
void ei_serial_set_baudrate(int baudrate);

/**
 * @brief      Connect to putchar of target
 *
 * @param[in]  c The chararater
 */
void ei_putchar(char c);

/**
 * Print wrapper around printf()
 * This is used internally to print debug information.
 */
__attribute__ ((format (printf, 1, 2)))
void ei_printf(const char *format, ...);

/**
 * Override this function if your target cannot properly print floating points
 * If not overriden, this will be sent through `ei_printf()`.
 */
void ei_printf_float(float f);

/**
 * Wrapper around malloc
 */
void *ei_malloc(size_t size);

/**
 * Wrapper around calloc
 */
void *ei_calloc(size_t nitems, size_t size);

/**
 * Wrapper around free
 */
void ei_free(void *ptr);

#if defined(__cplusplus) && EI_C_LINKAGE == 1
}
#endif // defined(__cplusplus) && EI_C_LINKAGE == 1

// Load porting layer depending on target
#ifndef EI_PORTING_ARDUINO
#ifdef ARDUINO
#define EI_PORTING_ARDUINO      1
#else
#define EI_PORTING_ARDUINO      0
#endif
#endif

#ifndef EI_PORTING_ECM3532
#ifdef ECM3532
#define EI_PORTING_ECM3532      1
#else
#define EI_PORTING_ECM3532      0
#endif
#endif

#ifndef EI_PORTING_ESPRESSIF
#ifdef CONFIG_IDF_TARGET_ESP32
#define EI_PORTING_ESPRESSIF      1
#else
#define EI_PORTING_ESPRESSIF     0
#endif
#endif

#ifndef EI_PORTING_MBED
#ifdef __MBED__
#define EI_PORTING_MBED      1
#else
#define EI_PORTING_MBED      0
#endif
#endif

#ifndef EI_PORTING_POSIX
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define EI_PORTING_POSIX      1
#else
#define EI_PORTING_POSIX      0
#endif
#endif

#ifndef EI_PORTING_SILABS
#if defined(EFR32MG12P332F1024GL125)
#define EI_PORTING_SILABS      1
#else
#define EI_PORTING_SILABS      0
#endif
#endif

#ifndef EI_PORTING_RASPBERRY
#ifdef PICO_BOARD 
#define EI_PORTING_RASPBERRY      1
#else
#define EI_PORTING_RASPBERRY      0
#endif
#endif

#ifndef EI_PORTING_ZEPHYR
#if defined(__ZEPHYR__)
#define EI_PORTING_ZEPHYR      1
#else
#define EI_PORTING_ZEPHYR      0
#endif
#endif

#ifndef EI_PORTING_STM32_CUBEAI
#if defined(USE_HAL_DRIVER) && !defined(__MBED__) && EI_PORTING_ZEPHYR == 0
#define EI_PORTING_STM32_CUBEAI      1
#else
#define EI_PORTING_STM32_CUBEAI      0
#endif
#endif

#ifndef EI_PORTING_HIMAX
#ifdef CPU_ARC
#define EI_PORTING_HIMAX        1
#else
#define EI_PORTING_HIMAX        0
#endif
#endif

#ifndef EI_PORTING_MINGW32
#ifdef __MINGW32__
#define EI_PORTING_MINGW32      1
#else
#define EI_PORTING_MINGW32      0
#endif
#endif
// End load porting layer depending on target

#endif // _EI_CLASSIFIER_PORTING_H_
