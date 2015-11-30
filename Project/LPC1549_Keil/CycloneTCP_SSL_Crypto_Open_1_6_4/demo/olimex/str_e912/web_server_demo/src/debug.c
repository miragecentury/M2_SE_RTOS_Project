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
#include "91x_lib.h"
#include "debug.h"

//Function declaration
void lcdPutChar(char_t c);


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   UART_InitTypeDef UART_InitStructure;

   //Enable GPIO5 clock
   SCU_APBPeriphClockConfig(__GPIO5, ENABLE);
   //Enable UART0 clock
   SCU_APBPeriphClockConfig(__UART0, ENABLE);

   //Configure UART0_TX (P5.0)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
   GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
   GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt3;
   GPIO_Init(GPIO5, &GPIO_InitStructure);

   //Configure UART0_RX (P5.1)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
   GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
   GPIO_InitStructure.GPIO_IPInputConnected = GPIO_IPInputConnected_Enable;
   GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1;
   GPIO_Init(GPIO5, &GPIO_InitStructure);

   //Reset UART0 peripheral
   UART_DeInit(UART0);

   //Configure UART0
   UART_InitStructure.UART_BaudRate = baudrate;
   UART_InitStructure.UART_WordLength = UART_WordLength_8D;
   UART_InitStructure.UART_StopBits = UART_StopBits_1;
   UART_InitStructure.UART_Parity = UART_Parity_No;
   UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
   UART_InitStructure.UART_Mode = UART_Mode_Tx_Rx;
   UART_InitStructure.UART_FIFO = UART_FIFO_Disable;
   UART_Init(UART0, &UART_InitStructure);

   //Enable UART0
   UART_Cmd(UART0, ENABLE);
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
      while(UART_GetFlagStatus(UART0, UART_FLAG_TxFIFOFull) != RESET);
      //Send character
      UART_SendData(UART0, c);
      //Wait for the transfer to complete
      while(UART_GetFlagStatus(UART0, UART_FLAG_TxFIFOFull) != RESET);

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
