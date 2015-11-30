/**
 * @file bma250.c
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

//Dependencies
#include "iodefine.h"
#include "gpio_iobitmask.h"
#include "bma250.h"
#include "error.h"
#include "debug.h"

//I2C slave address
#define BMA250_SLAVE_ADDR_W 0x30
#define BMA250_SLAVE_ADDR_R 0x31

//I2C pins
#define SCL_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR118 | GPIO_PMSR1_PMSR12) : (GPIO_PMSR1_PMSR118))
#define SDA_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR119 | GPIO_PMSR1_PMSR13) : (GPIO_PMSR1_PMSR119))
#define SDA_IN() (PORT1.PPRn.WORD & GPIO_PPR1_PPR13)


/**
 * @brief BMA250 initialization
 * @return Error code (see #ErrorCode enumeration)
 **/

error_t bma250Init(void)
{
   error_t error;
   uint8_t value;

   //Debug message
   TRACE_INFO("Initializing BMA250...\r\n");

   //I2C initialization
   i2cInit();

   //Read chip ID
   error = bma250ReadReg(BMA250_REG_CHIP_ID, &value);
   //Any error to report?
   if(error) return error;

   //Verify chip identifier
   if(value != CHIP_ID_BMA250)
      return ERROR_INVALID_VERSION;

   //Select +/-2g range
   error = bma250WriteReg(BMA250_REG_G_RANGE, G_RANGE_2G);
   //Any error to report?
   if(error) return error;

   //Select 31Hz bandwidth
   error = bma250WriteReg(BMA250_REG_BANDWITH, BANDWITH_8HZ);
   //Any error to report?
   if(error) return error;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Get acceleration data
 * @param[out] ax Acceleration value (X axis)
 * @param[out] ay Acceleration value (Y axis)
 * @param[out] az Acceleration value (Z axis)
 * @param[in] data Register value
 * @return Error code (see #ErrorCode enumeration)
 **/

error_t bma250GetAcc(int8_t *ax, int8_t *ay, int8_t *az)
{
   error_t error;

   //Issue a start condition
   i2cStart();

   //Send I2C slave address
   error = i2cWrite(BMA250_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Write register address
      error = i2cWrite(BMA250_REG_ACC_X_LSB);
   }

   //Issue a stop condition
   i2cStop();

   //Check status code
   if(!error)
   {
      //Issue a start condition
      i2cStart();

      //Send I2C slave address
      error = i2cWrite(BMA250_SLAVE_ADDR_R);

      if(!error)
      {
         //Read ax acceleration value (LSB)
         i2cRead(TRUE);
         //Read ax acceleration value (MSB)
         *ax = i2cRead(TRUE);

         //Read ay acceleration value (LSB)
         i2cRead(TRUE);
         //Read ay acceleration value (MSB)
         *ay = i2cRead(TRUE);

         //Read az acceleration value (LSB)
         i2cRead(TRUE);
         //Read az acceleration value (MSB)
         *az = i2cRead(FALSE);
      }

      //Issue a stop condition
      i2cStop();
   }

   //Return status code
   return error;
}


/**
 * @brief Write BMA250 register
 * @param[in] address Register address
 * @param[in] data Register value
 * @return Error code (see #ErrorCode enumeration)
 **/

error_t bma250WriteReg(uint8_t address, uint8_t data)
{
   int_t error;

   //Issue a start condition
   i2cStart();

   //Send I2C slave address
   error = i2cWrite(BMA250_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Write register address
      error = i2cWrite(address);
   }

   //Check status code
   if(!error)
   {
      //Write register value
      error = i2cWrite(data);
   }

   //Issue a stop condition
   i2cStop();

   //Return status code
   return error;
}


/**
 * @brief Read BMA250 register
 * @param[in] address Register address
 * @param[out] data Register value
 * @return Error code (see #ErrorCode enumeration)
 **/

error_t bma250ReadReg(uint8_t address, uint8_t *data)
{
   error_t error;

   //Issue a start condition
   i2cStart();

   //Send I2C slave address
   error = i2cWrite(BMA250_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Write register address
      error = i2cWrite(address);
   }

   //Issue a stop condition
   i2cStop();

   //Check status code
   if(!error)
   {
      //Issue a start condition
      i2cStart();

      //Send I2C slave address
      error = i2cWrite(BMA250_SLAVE_ADDR_R);

      //Check status code
      if(!error)
      {
         //Read register value
         *data = i2cRead(FALSE);
      }

      //Issue a stop condition
      i2cStop();
   }

   //Return status code
   return error;
}


/**
 * @brief I2C initialization
 **/

void i2cInit(void)
{
   //Configure SCL1 pin
   PORT1.PMCn.BIT.PMCn2 = 0;
   PORT1.PIBCn.BIT.PIBCn2 = 1;
   PORT1.PMn.BIT.PMn2 = 1;
   PORT1.Pn.BIT.Pn2 = 0;

   //Configure SDA1 pin
   PORT1.PMCn.BIT.PMCn3 = 0;
   PORT1.PIBCn.BIT.PIBCn3 = 1;
   PORT1.PMn.BIT.PMn3 = 1;
   PORT1.Pn.BIT.Pn3 = 0;
}


/**
 * @brief I2C delay
 **/

void i2cDelay(void)
{
   volatile uint_t delay;

   //Delay loop
   for(delay = 0; delay < 500; delay++);
}


/**
 * @brief I2C start condition
 **/

void i2cStart(void)
{
   SDA_OUT(1);
   SCL_OUT(1);
   i2cDelay();

   //Pull SDA to low level
   SDA_OUT(0);
   i2cDelay();

   //Pull SCL to low level
   SCL_OUT(0);
   i2cDelay();
}


/**
 * @brief I2C stop condition
 **/

void i2cStop(void)
{
   SDA_OUT(0);
   i2cDelay();

   //Release SCL
   SCL_OUT(1);
   i2cDelay();

   //Release SDA
   SDA_OUT(1);
   i2cDelay();
}


/**
 * @brief I2C repeated start condition
 **/

void i2cRepeatedStart(void)
{
   //Release SDA
   SDA_OUT(1);
   i2cDelay();

   //Release SCL
   SCL_OUT(1);
   i2cDelay();

   //Pull SDA to low level
   SDA_OUT(0);
   i2cDelay();

   //Pull SCL to low level
   SCL_OUT(0);
   i2cDelay();
}


/**
 * @brief I2C write operation
 * @param[in] data Data byte to be written
 * @return Error code
 **/

error_t i2cWrite(uint8_t data)
{
   error_t error;
   uint_t i;

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Set SDA state
      if(data & 0x80)
         SDA_OUT(1);
      else
         SDA_OUT(0);

      //Pulse SCL
      i2cDelay();
      SCL_OUT(1);
      i2cDelay();
      SCL_OUT(0);
      i2cDelay();

      //Shift data byte
      data <<= 1;
   }

   //Release SDA in order to read ACK bit
   SDA_OUT(1);
   i2cDelay();

   //Set SCL to high level
   SCL_OUT(1);
   i2cDelay();

   //Retrieve ACK value
   if(SDA_IN())
      error = ERROR_FAILURE;
   else
      error = NO_ERROR;

   //Pull SCL to low level
   SCL_OUT(0);
   i2cDelay();

   //Return status code
   return error;
}


/**
 * @brief I2C read operation
 * @param[out] ack ACK value
 * @return Data byte resulting from the read operation
 **/

uint8_t i2cRead(bool_t ack)
{
   uint_t i;
   uint8_t data;

   //Clear data
   data = 0;

   //Release SDA
   SDA_OUT(1);
   i2cDelay();

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Shift data byte
      data <<= 1;

      //Set SCL to high level
      SCL_OUT(1);
      i2cDelay();

      //Retrieve bit value
      if(SDA_IN())
         data |= 1;

      //Pull SCL to low level
      SCL_OUT(0);
      i2cDelay();
   }

   //Write ACK bit
   if(ack)
      SDA_OUT(0);
   else
      SDA_OUT(1);

   //Pulse SCL
   i2cDelay();
   SCL_OUT(1);
   i2cDelay();
   SCL_OUT(0);
   i2cDelay();

   //Return data byte
   return data;
}
