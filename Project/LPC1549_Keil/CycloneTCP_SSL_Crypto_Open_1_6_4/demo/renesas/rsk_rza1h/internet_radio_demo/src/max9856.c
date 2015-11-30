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
#include "dev_drv.h"
#include "riic.h"
#include "max9856.h"
#include "debug.h"


/**
 * @brief MAX9856 audio CODEC initialization
 * @return Error code
 **/

error_t max9856Init(void)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing MAX9856...\r\n");

   //Dump MAX9856 registers for debugging purpose
   //max9856DumpReg();

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
   //max9856DumpReg();

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
   uint8_t buffer[3];

   //Set up a write operation
   buffer[0] = (MAX9856_ADDR << 1);
   buffer[1] = address;
   buffer[2] = data;

   //Send a start condition
   R_RIIC_WriteCond(DEVDRV_CH_3, RIIC_TX_MODE_START);

   //Write the specified register
   error = R_RIIC_Write(DEVDRV_CH_3, buffer, 3);

   //Send a stop condition
   R_RIIC_WriteCond(DEVDRV_CH_3, RIIC_TX_MODE_STOP);
   R_RIIC_DetectStop(DEVDRV_CH_3);

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
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
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
