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
#include "s6e2cc.h"
#include "debug.h"


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   //Deactivate AN31
   FM4_GPIO->ADE_f.AN31 = 0;

   //Configure SIN0_0 (P21) and SOT0_0 (P22)
   FM4_GPIO->PFR2_f.P1 = 1;
   FM4_GPIO->PFR2_f.P2 = 1;

   //Peripheral assignment
   FM4_GPIO->EPFR07_f.SIN0S = 1;
   FM4_GPIO->EPFR07_f.SOT0B = 1;

   //Default UART configuration
   FM4_MFS0->SCR = 0;
   FM4_MFS0->SMR = 0;
   FM4_MFS0->ESCR = 0;

   //Software reset
   FM4_MFS0->SCR_f.UPCL = 1;
   //Enable serial data output
   FM4_MFS0->SMR_f.SOE = 1;

   //Configure baud rate
   FM4_MFS0->BGR  = SystemCoreClock / (2 * baudrate) - 1;

   //Enable transmitter and receiver
   FM4_MFS0->SCR_f.TXE = 1;
   FM4_MFS0->SCR_f.RXE = 1;
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
   //Standard output or error output?
   if(stream == stdout || stream == stderr)
   {
      //Wait for the transmitter to be ready
      while(!FM4_MFS0->SSR_f.TDRE);
      //Send character
      FM4_MFS0->RDR = c;
      //Wait for the transfer to complete
      while(!FM4_MFS0->SSR_f.TDRE);

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
