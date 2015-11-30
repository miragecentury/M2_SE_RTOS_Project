/**
 * @file max9856.c
 * @brief MAX9856 audio CODEC
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
#include <stdio.h>
#include <string.h>
#include "iodefine.h"
#include "gpio_iobitmask.h"
#include "max9856.h"
#include "debug.h"

//I2C slave address
#define MAX9856_SLAVE_ADDR_W 0x20
#define MAX9856_SLAVE_ADDR_R 0x21

//I2C pins
#define SCL1_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR118 | GPIO_PMSR1_PMSR12) : (GPIO_PMSR1_PMSR118))
#define SDA1_OUT(a) PORT1.PMSRn.LONG = ((a) ? (GPIO_PMSR1_PMSR119 | GPIO_PMSR1_PMSR13) : (GPIO_PMSR1_PMSR119))
#define SDA1_IN() (PORT1.PPRn.WORD & GPIO_PPR1_PPR13)


/**
 * @brief MAX9856 audio CODEC initialization
 * @return Error code
 **/

error_t max9856Init(void)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing MAX9856...\r\n");

   //I2C initialization
   i2c1Init();

   //Dump MAX9856 registers for debugging purpose
   max9856DumpReg();

   //Power on audio CODEC
   error = max9856WriteReg(MAX9856_REG_PM_SYS, PM_SYS_SHDN | PM_SYS_DIGEN |
      PM_SYS_LOUTEN | PM_SYS_DALEN | PM_SYS_DAREN | PM_SYS_ADLEN | PM_SYS_ADREN);
   //Any error to report?
   if(error) return error;

   //Enable slave mode and select clock range
   error = max9856WriteReg(MAX9856_REG_CLK_CTRL, LK_CTRL_PSCLK_20_32MHZ);
   //Any error to report?
   if(error) return error;

   //Configure I2S interface
   error = max9856WriteReg(MAX9856_REG_DAC_SYS, DAC_SYS_DRATE_HIGH);
   //Any error to report?
   if(error) return error;

   //Enable DAC PLL
   error = max9856WriteReg(MAX9856_REG_DAC_IF1, DAC_IF1_DPLLEN);
   //Any error to report?
   if(error) return error;

   //Set DAC LRCLK divider to default value
   error = max9856WriteReg(MAX9856_REG_DAC_IF2, 0);
   //Any error to report?
   if(error) return error;

   //Configure output mixer
   error = max9856WriteReg(MAX9856_REG_OUT_MIXER,
      OUT_MIXER_MXOUTL_DAC_OUT | OUT_MIXER_MXOUTR_DAC_OUT);
   //Any error to report?
   if(error) return error;

   //Set headphone volume (left)
   error = max9856WriteReg(MAX9856_REG_HPL_VOL, HPL_VOL_HPVOLL_MUTE);
   //Any error to report?
   if(error) return error;

  //Set headphone volume (right)
   error = max9856WriteReg(MAX9856_REG_HPR_VOL, HPR_VOL_HPVOLR_MUTE);
   //Any error to report?
   if(error) return error;

   //Set output mode
   error = max9856WriteReg(MAX9856_REG_OUT_MODE,
      OUT_MODE_VSEN | OUT_MODE_HPMODE_STEREO);
   //Any error to report?
   if(error) return error;

   //Dump MAX9856 registers for debugging purpose
   max9856DumpReg();

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Adjust headphone volume
 * @param[in] left Left volume (should be in the range 0 to 100)
 * @param[in] right Right volume (should be in the range 0 to 100)
 * @return Error code
 **/

error_t max9856SetVolume(uint_t left, uint_t right)
{
   error_t error;

   //Make sure the volume is in the range 0 to 100
   left = MIN(left, 100);
   right = MIN(right, 100);

   //Left volume
   if(left > 0)
      left = 23 + (left * 40 / 100);

   //Right volume
   if(right > 0)
      right = 23 + (right * 40 / 100);

   //Set headphone volume (left)
   error = max9856WriteReg(MAX9856_REG_HPL_VOL, 63 - left);
   //Any error to report?
   if(error) return error;

   //Set headphone volume (right)
   error = max9856WriteReg(MAX9856_REG_HPR_VOL, 63 - right);
   //Return status code
   return error;
}


/**
 * @brief Write MAX9856 register
 * @param[in] address Register address
 * @param[in] data Register value
 * @return Error code
 **/

error_t max9856WriteReg(uint8_t address, uint8_t data)
{
   int_t error;

   //Issue a start condition
   i2c1Start();

   //Send I2C slave address
   error = i2c1Write(MAX9856_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Write register address
      error = i2c1Write(address);
   }

   //Check status code
   if(!error)
   {
      //Write register value
      error = i2c1Write(data);
   }

   //Issue a stop condition
   i2c1Stop();

   //Return status code
   return error;
}


/**
 * @brief Read MAX9856 register
 * @param[in] address Register address
 * @param[out] data Register value
 * @return Error code
 **/

error_t max9856ReadReg(uint8_t address, uint8_t *data)
{
   error_t error;

   //Issue a start condition
   i2c1Start();

   //Send I2C slave address
   error = i2c1Write(MAX9856_SLAVE_ADDR_W);

   //Check status code
   if(!error)
   {
      //Write register address
      error = i2c1Write(address);
   }

   //Check status code
   if(!error)
   {
      //Issue a repeated start condition
      i2c1RepeatedStart();

      //Send I2C slave address
      error = i2c1Write(MAX9856_SLAVE_ADDR_R);
   }

   //Check status code
   if(!error)
   {
      //Read register value
      *data = i2c1Read(FALSE);
   }

   //Issue a stop condition
   i2c1Stop();

   //Return status code
   return error;
}


/**
 * @brief Dump MAX9856 registers for debugging purpose
 * @return Error code
 **/

error_t max9856DumpReg(void)
{
   error_t error;
   uint8_t i;
   uint8_t data;

   //Loop through MAX9856 registers
   for(i = 0; i <= 28; i++)
   {
      //Read register contents
      error = max9856ReadReg(i, &data);
      //Any error to report?
      if(error) break;

      //Display current register
      TRACE_DEBUG("0x%02" PRIX8 ": 0x%02" PRIX8 "\r\n", i, data);
   }

   //Terminate with a line feed
   TRACE_DEBUG("\r\n");

   //Return status code
   return error;
}


/**
 * @brief I2C initialization
 **/

void i2c1Init(void)
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

void i2c1Delay(void)
{
   volatile uint_t delay;

   //Delay loop
   for(delay = 0; delay < 500; delay++);
}


/**
 * @brief I2C start condition
 **/

void i2c1Start(void)
{
   SDA1_OUT(1);
   SCL1_OUT(1);
   i2c1Delay();

   //Pull SDA to low level
   SDA1_OUT(0);
   i2c1Delay();

   //Pull SCL to low level
   SCL1_OUT(0);
   i2c1Delay();
}


/**
 * @brief I2C stop condition
 **/

void i2c1Stop(void)
{
   SDA1_OUT(0);
   i2c1Delay();

   //Release SCL
   SCL1_OUT(1);
   i2c1Delay();

   //Release SDA
   SDA1_OUT(1);
   i2c1Delay();
}


/**
 * @brief I2C repeated start condition
 **/

void i2c1RepeatedStart(void)
{
   //Release SDA
   SDA1_OUT(1);
   i2c1Delay();

   //Release SCL
   SCL1_OUT(1);
   i2c1Delay();

   //Pull SDA to low level
   SDA1_OUT(0);
   i2c1Delay();

   //Pull SCL to low level
   SCL1_OUT(0);
   i2c1Delay();
}


/**
 * @brief I2C write operation
 * @param[in] data Data byte to be written
 * @return Error code
 **/

error_t i2c1Write(uint8_t data)
{
   error_t error;
   uint_t i;

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Set SDA state
      if(data & 0x80)
         SDA1_OUT(1);
      else
         SDA1_OUT(0);

      //Pulse SCL
      i2c1Delay();
      SCL1_OUT(1);
      i2c1Delay();
      SCL1_OUT(0);
      i2c1Delay();

      //Shift data byte
      data <<= 1;
   }

   //Release SDA in order to read ACK bit
   SDA1_OUT(1);
   i2c1Delay();

   //Set SCL to high level
   SCL1_OUT(1);
   i2c1Delay();

   //Retrieve ACK value
   if(SDA1_IN())
      error = ERROR_FAILURE;
   else
      error = NO_ERROR;

   //Pull SCL to low level
   SCL1_OUT(0);
   i2c1Delay();

   //Return status code
   return error;
}


/**
 * @brief I2C read operation
 * @param[out] ack ACK value
 * @return Data byte resulting from the read operation
 **/

uint8_t i2c1Read(bool_t ack)
{
   uint_t i;
   uint8_t data;

   //Clear data
   data = 0;

   //Release SDA
   SDA1_OUT(1);
   i2c1Delay();

   //Iterate 8 times
   for(i = 0; i < 8; i++)
   {
      //Shift data byte
      data <<= 1;

      //Set SCL to high level
      SCL1_OUT(1);
      i2c1Delay();

      //Retrieve bit value
      if(SDA1_IN())
         data |= 1;

      //Pull SCL to low level
      SCL1_OUT(0);
      i2c1Delay();
   }

   //Write ACK bit
   if(ack)
      SDA1_OUT(0);
   else
      SDA1_OUT(1);

   //Pulse SCL
   i2c1Delay();
   SCL1_OUT(1);
   i2c1Delay();
   SCL1_OUT(0);
   i2c1Delay();

   //Return data byte
   return data;
}
