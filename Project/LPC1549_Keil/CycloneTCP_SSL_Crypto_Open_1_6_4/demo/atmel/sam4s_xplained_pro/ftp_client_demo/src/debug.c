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
#include "sam4s.h"
#include "debug.h"


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   //Enable PIO peripheral clock
   PMC->PMC_PCER0 = (1 << ID_PIOB);
   //Enable UART1 peripheral clock
   PMC->PMC_PCER0 = (1 << ID_UART1);

   //Disable interrupts on TXD and RXD pins
   PIOB->PIO_IDR = PIO_PB3A_UTXD1 | PIO_PB2A_URXD1;
   //No pull-up resistors
   PIOB->PIO_PUDR = PIO_PB3A_UTXD1 | PIO_PB2A_URXD1;
   //Assign corresponding pins to Peripheral A function
   PIOB->PIO_ABCDSR[0] &= ~(PIO_PB3A_UTXD1 | PIO_PB2A_URXD1);
   PIOB->PIO_ABCDSR[1] &= ~(PIO_PB3A_UTXD1 | PIO_PB2A_URXD1);
   //Disable the PIO from controlling the corresponding pins
   PIOB->PIO_PDR = PIO_PB3A_UTXD1 | PIO_PB2A_URXD1;

   //Reset transmitter and receiver
   UART1->UART_CR = UART_CR_RSTTX | UART_CR_RSTRX;
   //Disable interrupts
   UART1->UART_IDR = 0xFFFFFFFF;

   //Configure baud rate
   UART1->UART_BRGR = SystemCoreClock / (16 * baudrate);

   //Configure mode register
   UART1->UART_MR = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO;

   //Disable DMA channel
   UART1->UART_PTCR = UART_PTCR_TXTDIS | UART_PTCR_RXTDIS;

   //Enable transmitter and receiver
   UART1->UART_CR = UART_CR_TXEN | UART_CR_RXEN;
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
      while(!(UART1->UART_SR & UART_SR_TXEMPTY));
      //Send character
      UART1->UART_THR = c;
      //Wait for the transfer to complete
      while(!(UART1->UART_SR & UART_SR_TXEMPTY));

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
