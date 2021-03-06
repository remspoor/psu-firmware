/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
/** @file conf.h
@brief Compile time configuration.
*/

/** @page conf_h Configuration
@brief Compile time configuration.
This file is used to define compile time configuration options.
Use `conf_user.h` file to override anything from here.
*/

#pragma once

#include "conf_channel.h"

/* 
 * PSU settings
 */

/// Maximum number of channels existing.
#define CH_MAX 2

/// Number of channels visible (less then or equal to CH_MAX)
#define CH_NUM 2

/// Channel capability
#define CHANNELS \
    CHANNEL(1, CH_PINS_1, CH_PARAMS_50V_3A), \
    CHANNEL(2, CH_PINS_2, CH_PARAMS_50V_3A) \

/* 
 * Ardiuno shield SPI peripherals
 */

/// Is Ethernet present?
#define OPTION_ETHERNET   1

/// Is RTC present?
#define OPTION_EXT_RTC    1

/// Is SD card present?
#define OPTION_SD_CARD    0

/// Is external EEPROM present?
#define OPTION_EXT_EEPROM 1

/// Is binding post present?
#define OPTION_BP         1

/// Min. delay between power down and power up.
#define MIN_POWER_UP_DELAY 5

/* 
 * Over-temperature protection (OTP) parameters
 */

/// Is OTP enabled by default?
#define OTP_MAIN_DEFAULT_STATE 1

/// Default OTP delay in seconds
#define OTP_MAIN_DEFAULT_DELAY 10.0f

/// Default OTP threshold in oC 
#define OTP_MAIN_DEFAULT_LEVEL 70.0f

/* 
 * Serial communication settings
 */

/// Data rate in bits per second (baud) for serial data transmission.
#define SERIAL_SPEED 9600

#include "conf_advanced.h"
#include "conf_user.h"
