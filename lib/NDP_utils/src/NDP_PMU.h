/*
 * Copyright (c) 2021 Syntiant Corp.  All rights reserved.
 * Contact at http://www.syntiant.com
 * 
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License (GPL) Version 2, available from the file LICENSE in the main
 * directory of this source tree, or the OpenIB.org BSD license below.  Any
 * code involving Linux software will require selection of the GNU General
 * Public License (GPL) Version 2.
 * 
 * OPENIB.ORG BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NDP_PMU_H
#define NDP_PMU_H

#include "SGM41512.h"

#include "NDP_GPIO.h"
#include "NDP_Serial.h"

// I2C address for SGM41512
#define SGM41512 0x6B

#define PMU_OTG A6
#define ENABLE_5V 2 // enable 5v out to companion Arduino. PMU must also be placed in BOOST mode

#define POWERON_CONFIG_REG 0x01
#define CHARGE_CURRENT_CTL_REG 0x02
#define CHARGE_TERMINATION_TIMER_CTL_REG 0x05
#define RESET_AND_VERSION_REG 0x0B

void writeTo(int device, byte address, byte val);
void readFrom(int device, byte address, int num, byte buff[]);
void printPmu();
void printBattery();
void pmuBoost();
void pmuCharge();

#endif
