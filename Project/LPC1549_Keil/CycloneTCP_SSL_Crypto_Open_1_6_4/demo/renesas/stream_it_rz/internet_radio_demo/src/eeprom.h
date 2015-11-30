/**
 * @file eeprom.h
 * @brief 24LC01 EEPROM memory
 *
 * Copyright (C) 2010-2015 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.6.4
 **/

#ifndef _EEPROM_H
#define _EEPROM_H

//Dependencies
#include "compiler_port.h"
#include "error.h"

//EEPROM memory management
error_t eepromInit(void);
error_t eepromWrite(uint8_t address, const void *data, size_t length);
error_t eepromRead(uint8_t address, void *data, size_t length);

void i2c0Init(void);
void i2c0Delay(void);
void i2c0Start(void);
void i2c0Stop(void);
error_t i2c0Write(uint8_t data);
uint8_t i2c0Read(bool_t ack);

#endif
