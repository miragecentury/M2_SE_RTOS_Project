/**
 * @file eeprom.c
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

//Dependencies
#include <string.h>
#include "iodefine.h"
#include "gpio_iobitmask.h"
#include "eeprom.h"
#include "debug.h"

//I2C slave address
#define EEPROM_SLAVE_ADDR_W 0xA2
#define EEPROM_SLAVE_ADDR_R 0xA3

//I2C pins
#define SCL0_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR116 | GPIO_PMSR1_PMSR10) : (GPIO_PMSR1_PMSR116))
#define SDA0_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR117 | GPIO_PMSR1_PMSR11) : (GPIO_PMSR1_PMSR117))
#define SDA0_IN() (PORT1.PPRn.WORD & GPIO_PPR1_PPR11)

//Mutex preventing simultaneous access to the EEPROM
OsMutex eepromMutex;


/**
 * @brief EEPROM initialization
 * @return Error code
 **/

error_t eepromInit(void)
{
   //Debug message
   TRACE_INFO("Initializing EEPROM...\r\n");

   //Create a mutex to prevent simultaneous access to the EEPROM
   if(!osCreateMutex(&eepromMutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //I2C initialization
   i2c0Init();

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Write data to EEPROM memory
 * @param[in] address Memory location where to write data
 * @param[in] data Pointer to the data to be written
 * @param[in] length Number of data bytes
 * @return Error code
 **/

error_t eepromWrite(uint8_t address, const void *data, size_t length)
{
   error_t error;
   size_t i;
   size_t n;
   const uint8_t *p;

   //Acquire exclusive access to the EEPROM
   osAcquireMutex(&eepromMutex);

   //Cast data pointer
   p = (const uint8_t *) data;

   //Initialize error code
   error = NO_ERROR;

   //Write data
   while(length > 0)
   {
      //Prevent page write operations that would attempt to cross a page boundary
      n = 8 - (address % 8);
      //Limit the number of bytes to write
      n = MIN(n, length);

      //Issue a start condition
      i2c0Start();

      //Send I2C slave address
      error = i2c0Write(EEPROM_SLAVE_ADDR_W);

      //Check status code
      if(!error)
      {
         //Send the address
         error = i2c0Write(address);
      }

      //Check status code
      if(!error)
      {
         //Send up to 64 bytes
         for(i = 0; i < n; i++)
         {
            //Send current data byte
            error = i2c0Write(p[i]);
            //Any error to report?
            if(error) break;
         }
      }

      //Issue a stop condition
      i2c0Stop();

      //Check status code
      if(!error)
      {
         //Acknowledge polling
         do
         {
            //Issue a start condition
            i2c0Start();
            //Send control byte
            error = i2c0Write(EEPROM_SLAVE_ADDR_W);
            //Issue a stop condition
            i2c0Stop();

            //Check whether the master can then proceed with the next write command
         } while(error);
      }

      //Advance data pointer
      p += n;
      //Increment word address
      address += n;
      //Remaining bytes to be written
      length -= n;
   }

   //Release exclusive access to the EEPROM
   osReleaseMutex(&eepromMutex);

   //Return status code
   return error;
}


/**
 * @brief Read data from EEPROM memory
 * @param[in] address Memory location where to read data
 * @param[in] data Pointer to the buffer where to copy the data
 * @param[in] length Number of data bytes to read
 * @return Error code
 **/

error_t eepromRead(uint8_t address, void *data, size_t length)
{
   error_t error;
   uint8_t *p;

   //Acquire exclusive access to the EEPROM
   osAcquireMutex(&eepromMutex);

   //Cast data pointer
   p = (uint8_t *) data;

   //Issue a start condition
   i2c0Start();

   //Send I2C slave address
   error = i2c0Write(EEPROM_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Send the address
      error = i2c0Write(address);
   }

   //Issue a stop condition
   i2c0Stop();

   //Check status code
   if(!error)
   {
      //Issue a start condition
      i2c0Start();

      //Send I2C slave address
      error = i2c0Write(EEPROM_SLAVE_ADDR_R);

      //Check status code
      if(!error)
      {
         //Sequential read operation
         while(length > 1)
         {
            //Read current data byte
            *p = i2c0Read(TRUE);

            //Next byte
            p++;
            length--;
         }

         //Do not acknowledge the last byte
         if(length > 0)
            *p = i2c0Read(FALSE);
      }

      //Issue a stop condition
      i2c0Stop();
   }

   //Release exclusive access to the EEPROM
   osReleaseMutex(&eepromMutex);

   //Return status code
   return error;
}


/**
 * @brief I2C initialization
 **/

void i2c0Init(void)
{
   //Configure SCL0 pin
   PORT1.PMCn.BIT.PMCn0 = 0;
   PORT1.PIBCn.BIT.PIBCn0 = 1;
   PORT1.PMn.BIT.PMn0 = 1;
   PORT1.Pn.BIT.Pn0 = 0;

   //Configure SDA0 pin
   PORT1.PMCn.BIT.PMCn1 = 0;
   PORT1.PIBCn.BIT.PIBCn1 = 1;
   PORT1.PMn.BIT.PMn1 = 1;
   PORT1.Pn.BIT.Pn1 = 0;
}


/**
 * @brief I2C delay
 **/

void i2c0Delay(void)
{
   volatile uint_t delay;

   //Delay loop
   for(delay = 0; delay < 500; delay++);
}


/**
 * @brief I2C start condition
 **/

void i2c0Start(void)
{
   SDA0_OUT(1);
   SCL0_OUT(1);
   i2c0Delay();

   //Pull SDA to low level
   SDA0_OUT(0);
   i2c0Delay();

   //Pull SCL to low level
   SCL0_OUT(0);
   i2c0Delay();
}


/**
 * @brief I2C stop condition
 **/

void i2c0Stop(void)
{
   SDA0_OUT(0);
   i2c0Delay();

   //Release SCL
   SCL0_OUT(1);
   i2c0Delay();

   //Release SDA
   SDA0_OUT(1);
   i2c0Delay();
}


/**
 * @brief I2C write operation
 * @param[in] data Data byte to be written
 * @return Error code
 **/

error_t i2c0Write(uint8_t data)
{
   error_t error;
   uint_t i;

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Set SDA state
      if(data & 0x80)
         SDA0_OUT(1);
      else
         SDA0_OUT(0);

      //Pulse SCL
      i2c0Delay();
      SCL0_OUT(1);
      i2c0Delay();
      SCL0_OUT(0);
      i2c0Delay();

      //Shift data byte
      data <<= 1;
   }

   //Release SDA in order to read ACK bit
   SDA0_OUT(1);
   i2c0Delay();

   //Set SCL to high level
   SCL0_OUT(1);
   i2c0Delay();

   //Retrieve ACK value
   if(SDA0_IN())
      error = ERROR_FAILURE;
   else
      error = NO_ERROR;

   //Pull SCL to low level
   SCL0_OUT(0);
   i2c0Delay();

   //Return status code
   return error;
}


/**
 * @brief I2C read operation
 * @param[out] ack ACK value
 * @return Data byte resulting from the read operation
 **/

uint8_t i2c0Read(bool_t ack)
{
   uint_t i;
   uint8_t data;

   //Clear data
   data = 0;

   //Release SDA
   SDA0_OUT(1);
   i2c0Delay();

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Shift data byte
      data <<= 1;

      //Set SCL to high level
      SCL0_OUT(1);
      i2c0Delay();

      //Retrieve bit value
      if(SDA0_IN())
         data |= 1;

      //Pull SCL to low level
      SCL0_OUT(0);
      i2c0Delay();
   }

   //Write ACK bit
   if(ack)
      SDA0_OUT(0);
   else
      SDA0_OUT(1);

   //Pulse SCL
   i2c0Delay();
   SCL0_OUT(1);
   i2c0Delay();
   SCL0_OUT(0);
   i2c0Delay();

   //Return data byte
   return data;
}
