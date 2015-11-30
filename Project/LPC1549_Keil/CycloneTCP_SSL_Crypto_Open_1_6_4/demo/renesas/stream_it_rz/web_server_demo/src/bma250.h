/**
 * @file bma250.h
 * @brief BMA250 triaxial acceleration sensor
 *
 * @section License
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

#ifndef _BMA250_H
#define _BMA250_H

//Dependencies
#include "os_port.h"
#include "error.h"

//BMA250 registers
#define BMA250_REG_CHIP_ID    0x00
#define BMA250_REG_ACC_X_LSB  0x02
#define BMA250_REG_ACC_X_MSB  0x03
#define BMA250_REG_ACC_Y_LSB  0x04
#define BMA250_REG_ACC_Y_MSB  0x05
#define BMA250_REG_ACC_Z_LSB  0x06
#define BMA250_REG_ACC_Z_MSB  0x07
#define BMA250_REG_G_RANGE    0x0F
#define BMA250_REG_BANDWITH   0x10

//CHIP_ID register
#define CHIP_ID_BMA250        0x03

//RANGE register
#define G_RANGE_MASK          0x0F
#define G_RANGE_2G            0x03
#define G_RANGE_4G            0x05
#define G_RANGE_8G            0x08
#define G_RANGE_16G           0x0C

//BANDWITH register
#define BANDWITH_MASK         0x1F
#define BANDWITH_8HZ          0x08
#define BANDWITH_16HZ         0x09
#define BANDWITH_31HZ         0x0A
#define BANDWITH_62HZ         0x0B
#define BANDWITH_125HZ        0x0C
#define BANDWITH_250HZ        0x0D
#define BANDWITH_500HZ        0x0E
#define BANDWITH_1000HZ       0x0F

//BMA250 related functions
error_t bma250Init(void);
error_t bma250GetAcc(int8_t *ax, int8_t *ay, int8_t *az);
error_t bma250WriteReg(uint8_t address, uint8_t data);
error_t bma250ReadReg(uint8_t address, uint8_t *data);

//I2C related functions
void i2cInit(void);
void i2cDelay(void);
void i2cStart(void);
void i2cStop(void);
void i2cRepeatedStart(void);
error_t i2cWrite(uint8_t data);
uint8_t i2cRead(bool_t ack);

#endif
