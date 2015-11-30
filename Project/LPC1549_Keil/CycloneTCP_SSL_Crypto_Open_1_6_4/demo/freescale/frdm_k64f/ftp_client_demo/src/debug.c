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
#include "mk64f12.h"
#include "debug.h"


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   uint32_t div;
   uint32_t busClockFreq;

   //Clock 1 output divider value
   div = (SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT;
   //Bus clock frequency
   busClockFreq = SystemCoreClock / (div + 1);

   //Compute the UART baudrate prescaler
   div = ((2 * busClockFreq) + (baudrate / 2)) / baudrate;

   //Enable PORTB peripheral clock
   SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
   //Enable UART0 peripheral clock
   SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;

   //Configure PTB16 (UART0_RX) and PTB17 (UART0_TX)
   PORTB->PCR[16] = PORT_PCR_MUX(3);
   PORTB->PCR[17] = PORT_PCR_MUX(3);

   //LSB is transmitted first
   UART0->S2 = 0;

   //8-bit mode, no parity
   UART0->C1 = 0;
   //Transmit data is not inverted
   UART0->C3 = 0;
   //Baud rate fine ajust
   UART0->C4 = UART_C4_BRFA(div & 0x1F);

   //Select baudrate
   UART0->BDH = UART_BDH_SBR((div >> 13) & 0x1F);
   UART0->BDL = UART_BDL_SBR((div >> 5) & 0xFF);

   //Transmit FIFO watermark
   UART0->TWFIFO = UART_TWFIFO_TXWATER(0);
   //Receive FIFO watermark
   UART0->RWFIFO = UART_RWFIFO_RXWATER(1);

   //Enable transmitter and receiver
   UART0->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK;
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
      while(!(UART0->S1 & UART_S1_TDRE_MASK));
      //Send character
      UART0->D = c;
      //Wait for the transfer to complete
      while(!(UART0->S1 & UART_S1_TDRE_MASK));

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
