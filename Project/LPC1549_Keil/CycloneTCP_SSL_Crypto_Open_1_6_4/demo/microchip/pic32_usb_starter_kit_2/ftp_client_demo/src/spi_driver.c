/**
 * @file spi_driver.c
 * @brief SPI driver
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
#include <p32xxxx.h>
#include <plib.h>
#include "core/net.h"
#include "spi_driver.h"
#include "debug.h"


/**
 * @brief SPI driver
 **/

const SpiDriver spiDriver =
{
   spiInit,
   spiSetMode,
   spiSetBitrate,
   spiAssertCs,
   spiDeassertCs,
   spiTransfer
};


/**
 * @brief SPI initialization
 * @return Error code
 **/

error_t spiInit(void)
{
   //Configure ENC624J600 MDIX (RB3)
   LATBCLR = _LATB_LATB3_MASK;
   TRISBCLR = _TRISB_TRISB3_MASK;

   //Configure ENC624J600 CS (RD14)
   LATDSET = _LATD_LATD14_MASK;
   TRISDCLR = _TRISD_TRISD14_MASK;

   //Configure ENC624J600 SHDN (RD15)
   LATDSET = _LATD_LATD15_MASK;
   TRISDCLR = _TRISD_TRISD15_MASK;

   //Configure SPI1
   SpiChnOpen(1, SPI_CON_MSTEN | SPI_CON_MODE8 | SPI_CON_ON, 16);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Set SPI mode
 * @param mode SPI mode (0, 1, 2 or 3)
 **/

error_t spiSetMode(uint_t mode)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Set SPI bitrate
 * @param bitrate Bitrate value
 **/

error_t spiSetBitrate(uint_t bitrate)
{
   //Not implemented
   return ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Assert CS
 **/

void spiAssertCs(void)
{
   //Assert ENC624J600 CS (RD14)
   LATDCLR = _LATD_LATD14_MASK;
   //CS setup time
   usleep(1);
}


/**
 * @brief Deassert CS
 **/

void spiDeassertCs(void)
{
   //CS hold time
   usleep(1);
   //Deassert ENC624J600 CS (RD14)
   LATDSET = _LATD_LATD14_MASK;
   //CS disable time
   usleep(1);
}


/**
 * @brief Transfer a single byte
 * @param[in] data The data to be written
 * @return The data received from the slave device
 **/

uint8_t spiTransfer(uint8_t data)
{
   //Ensure the TX buffer is empty
   while(!(SPI1STAT & _SPI1STAT_SPITBE_MASK));
   //Start to transfer data
   SPI1BUF = data;
   //Wait for the operation to complete
   while(!(SPI1STAT & _SPI1STAT_SPIRBF_MASK));

   //Return the received character
   return SPI1BUF;
}
