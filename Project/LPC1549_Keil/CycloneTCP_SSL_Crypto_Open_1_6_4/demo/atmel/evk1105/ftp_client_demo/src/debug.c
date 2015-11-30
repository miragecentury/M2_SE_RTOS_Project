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
#include <avr32/io.h>
#include "evk1105.h"
#include "debug.h"

//Function declaration
void lcdPutChar(char_t c);


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   uint32_t cd;
   uint32_t fp;

   //Configure RXD0 (PA00)
   AVR32_GPIO.port[0].pmr0c = 1 << AVR32_USART0_RXD_0_0_PIN;
   AVR32_GPIO.port[0].pmr1c = 1 << AVR32_USART0_RXD_0_0_PIN;
   AVR32_GPIO.port[0].gperc = 1 << AVR32_USART0_RXD_0_0_PIN;

   //Configure TXD0 (PA01)
   AVR32_GPIO.port[0].pmr0c = 1 << AVR32_USART0_TXD_0_0_PIN;
   AVR32_GPIO.port[0].pmr1c = 1 << AVR32_USART0_TXD_0_0_PIN;
   AVR32_GPIO.port[0].gperc = 1 << AVR32_USART0_TXD_0_0_PIN;

   //Compute clock divider
   cd = (PBA_FREQ / (2 * baudrate)) / 8;
   //Compute fractional part
   fp = (PBA_FREQ / (2 * baudrate)) % 8;

   //Configure baud rate
   AVR32_USART0.brgr = (fp << AVR32_USART_BRGR_FP_OFFSET) |
      (cd << AVR32_USART_BRGR_CD_OFFSET);

   //Configure mode register
   AVR32_USART0.mr = (AVR32_USART_NBSTOP_1 << AVR32_USART_NBSTOP_OFFSET) |
      (AVR32_USART_MR_PAR_NONE << AVR32_USART_MR_PAR_OFFSET) |
      (AVR32_USART_MR_CHRL_8 << AVR32_USART_MR_CHRL_OFFSET) |
      (AVR32_USART_MR_MODE_NORMAL << AVR32_USART_MR_MODE_OFFSET);

   //Enable transmitter and receiver
   AVR32_USART0.cr = AVR32_USART_CR_TXEN_MASK | AVR32_USART_CR_RXEN_MASK;
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
      //Wait for the transmitter to be ready
      while(!(AVR32_USART0.csr & AVR32_USART_CSR_TXEMPTY_MASK));
      //Send character
      AVR32_USART0.thr = c;
      //Wait for the transfer to complete
      while(!(AVR32_USART0.csr & AVR32_USART_CSR_TXEMPTY_MASK));

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
