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
#include "sam4s.h"
#include "core/net.h"
#include "spi_driver.h"
#include "debug.h"

//SPI bitrate
#define SPI_BITRATE 10000000


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
   uint32_t div;

   //Enable PIO peripheral clock
   PMC->PMC_PCER0 = (1 << ID_PIOA);
   //Enable SPI peripheral clock
   PMC->PMC_PCER0 = (1 << ID_SPI);

   //Configure EXT3_CS as an output
   PIOA->PIO_PER = PIO_PA10;
   PIOA->PIO_OER = PIO_PA10;
   PIOA->PIO_SODR = PIO_PA10;

   //Disable interrupts on SPI pins
   PIOA->PIO_IDR = PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO;
   //Internal pull-up resistors
   PIOA->PIO_PUER = PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO;
   //Assign corresponding pins to Peripheral A function
   PIOA->PIO_ABCDSR[0] &= ~(PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO);
   PIOA->PIO_ABCDSR[1] &= ~(PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO);
   //Disable the PIO from controlling the corresponding pins
   PIOA->PIO_PDR = PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO;

   //Disable SPI module
   SPI->SPI_CR = SPI_CR_SPIDIS;
   //Reset SPI module
   SPI->SPI_CR = SPI_CR_SWRST;
   SPI->SPI_CR = SPI_CR_SWRST;

   //Master mode operation
   SPI->SPI_MR = SPI_MR_MODFDIS | SPI_MR_MSTR;

   //Calculate clock divider
   div = SystemCoreClock / SPI_BITRATE;

   //SPI configuration (8-bit words, clock phase = 1, clock polarity = 0)
   SPI->SPI_CSR[0] = SPI_CSR_SCBR(div) | SPI_CSR_BITS_8_BIT | SPI_CSR_NCPHA;

   //Enable SPI module
   SPI->SPI_CR = SPI_CR_SPIEN;

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
   //Assert EXT3_CS
   PIOA->PIO_CODR = PIO_PA10;
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
   //Deassert EXT3_CS
   PIOA->PIO_SODR = PIO_PA10;
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
   while(!(SPI->SPI_SR & SPI_SR_TDRE));
   //Start to transfer data
   SPI->SPI_TDR = data;
   //Wait for the operation to complete
   while(!(SPI->SPI_SR & SPI_SR_RDRF));

   //Return the received character
   return SPI->SPI_RDR;
}
