/**
 * @file debug.c
 * @brief Debugging facilities
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
#include "cpg_iobitmask.h"
#include "scif_iobitmask.h"
#include "debug.h"

//Peripheral clock 1 (66.67 MHz)
#define PCLK1_HZ (400000000 / 6)

//Forward declaration of functions
void lcdPutChar(char_t c);


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   uint32_t status;

   //Enable SCIF3 peripheral clock
   CPG.STBCR4 &= ~CPG_STBCR4_MSTP44;
   //Disable SCIF3 module
   SCIF3.SCSCR = 0;

   //Reset transmit and receive FIFO
   SCIF3.SCFCR |= SCIF3_SCFCR_TFRST | SCIF3_SCFCR_RFRST;

   //Read serial status register
   status = SCIF3.SCFSR;
   //Clear ER, BRK and DR flags
   SCIF3.SCFSR = status & ~(SCIF3_SCFSR_ER | SCIF3_SCFSR_BRK | SCIF3_SCFSR_DR);

   //Read line status register
   status = SCIF3.SCLSR;
   //Clear ORER flag
   SCIF3.SCLSR = status & ~SCIF3_SCLSR_ORER;

   //Select internal clock
   SCIF3.SCSCR &= ~SCIF3_SCSCR_CKE;

   //Configure UART (8 bits, no parity, 1 stop bit)
   SCIF3.SCSMR = 0;

   //Enable double-speed mode
   SCIF3.SCEMR = SCIF3_SCEMR_BGDM;

   //Set baudrate
   SCIF3.SCBRR = PCLK1_HZ / (16 * baudrate) - 1;

   //Configure transmit and receive FIFO triggers
   SCIF3.SCFCR = 0;

   //Clear TFRST and RFRST bits
   SCIF3.SCFCR &= ~(SCIF3_SCFCR_TFRST | SCIF3_SCFCR_RFRST);

   //Configure TXD3 (P7_11)
   PORT7.PMCn.BIT.PMCn11 = 1;
   PORT7.PFCn.BIT.PFCn11 = 0;
   PORT7.PFCEn.BIT.PFCEn11 = 0;
   PORT7.PFCAEn.BIT.PFCAEn11 = 1;
   PORT7.PIPCn.BIT.PIPCn11 = 1;

   //Configure RXD3 (P7_10)
   PORT7.PMCn.BIT.PMCn10 = 1;
   PORT7.PFCn.BIT.PFCn10 = 0;
   PORT7.PFCEn.BIT.PFCEn10 = 0;
   PORT7.PFCAEn.BIT.PFCAEn10 = 1;
   PORT7.PIPCn.BIT.PIPCn10 = 1;

   //Enable transmission and reception
   SCIF3.SCSCR = SCIF3_SCSCR_TE | SCIF3_SCSCR_RE;
}


/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(FILE *stream,
   const char_t *prepend, const void *data, size_t length)
{
   uint_t i;

   for(i = 0; i < length; i++)
   {
      //Beginning of a new line?
      if((i % 16) == 0)
         fprintf(stream, "%s", prepend);
      //Display current data byte
      fprintf(stream, "%02" PRIX8 " ", *((uint8_t *) data + i));
      //End of current line?
      if((i % 16) == 15 || i == (length - 1))
         fprintf(stream, "\r\n");
   }
}


/**
 * @brief Write character to stream
 * @param[in] c The character to be written
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @return On success, the character written is returned. If a writing
 *   error occurs, EOF is returned
 **/

int_t fputc(int_t c, FILE *stream)
{
   //Standard output?
   if(stream == stdout)
   {
      //Display current character
      lcdPutChar(c);

      //On success, the character written is returned
      return c;
   }
   //Standard error output?
   else if(stream == stderr)
   {
      //Wait for the transmit FIFO to be available for writing
      while(!(SCIF3.SCFSR & SCIF3_SCFSR_TDFE));
      //Send character
      SCIF3.SCFTDR = c;
      //Clear TDFE flag
      SCIF3.SCFSR &= ~SCIF3_SCFSR_TDFE;

      //On success, the character written is returned
      return c;
   }
   //Unknown output?
   else
   {
      //If a writing error occurs, EOF is returned
      return EOF;
   }
}
