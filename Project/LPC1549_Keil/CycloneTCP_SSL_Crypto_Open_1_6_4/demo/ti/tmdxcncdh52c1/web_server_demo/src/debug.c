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
#include <stdio.h>
#include <file.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "system_f28m35x.h"
#include "debug.h"

//Forward declaration of functions
int_t debugStreamOpen(const char_t *path, uint_t flags, int_t fd);
int_t debugStreamClose(int_t fd);
int_t debugStreamRead(int fd, char_t *buffer, uint_t count);
int_t debugStreamWrite(int_t fd, const char_t *buffer, uint_t count);
off_t debugStreamSeek(int_t fd, off_t offset, int_t origin);
int_t debugStreamUnlink(const char_t * path);
int_t debugStreamRename(const char_t *old, const char_t *new);


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   //Enable GPIO clock
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
   //Enable UART0 clock
   SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

   //Select the relevant alternate function for PE4 and PE5
   GPIOPinConfigure(GPIO_PE4_U0RX);
   GPIOPinConfigure(GPIO_PE5_U0TX);

   //Configure UART0 pins for proper operation
   GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

   //UART0 configuration
   UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, baudrate,
      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);

#ifdef __TI_ARM__
   //Add a new device
   add_device("debug", _MSA, debugStreamOpen, debugStreamClose, debugStreamRead,
      debugStreamWrite, debugStreamSeek, debugStreamUnlink, debugStreamRename);

   //Redirect stderr to the debug UART
   freopen("debug:", "w", stderr);

   //Disable buffering
   setvbuf(stderr, NULL, _IONBF, 0);
#endif
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
      //Send character
      UARTCharPut(UART0_BASE, c);

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


/**
 * @brief Open file for I/O
 * @param[in] path Filename of the file to be opened
 * @param[in] flags Attributes that specify how the file is manipulated
 * @param[in] fd File descriptor
 * @return Non-negative file descriptor
 */

int_t debugStreamOpen(const char_t *path, uint_t flags, int_t fd)
{
   //Not implemented
   return 0;
}


/**
 * @brief Close file for I/O
 * @param[in] fd File descriptor
 * @return This function should return -1 to indicate an error
 */

int_t debugStreamClose(int_t fd)
{
   //Not implemented
   return 0;
}


/**
 * @brief Read characters from a file
 * @param[in] fd File descriptor
 * @param[out] buffer Buffer where the read characters are placed
 * @param[in] count Number of characters to read from the file
 * @return The number of bytes read
 **/

int_t debugStreamRead(int fd, char_t *buffer, uint_t count)
{
   //Not implemented
   return 0;
}


/**
 * @brief Write characters from a file
 * @param[in] fd File descriptor
 * @param[in] buffer Buffer where the write characters are placed
 * @param[in] count Number of characters to write to the file
 * @return The number of bytes written
 **/

int_t debugStreamWrite(int_t fd, const char_t *buffer, uint_t count)
{
   uint_t i;

   //Send all the characters
   for(i = 0; i < count; i++)
      fputc(buffer[i], stdout);

   //Return the number of characters written
   return count;
}


/**
 * @brief Set the file position indicator
 * @param[in] fd File descriptor
 * @param[in] offset Offset from the origin
 * @param[in] origin Position used as reference for the offset
 * @return The new value of the file position indicator
 **/

off_t debugStreamSeek(int_t fd, off_t offset, int_t origin)
{
   //Not implemented
   return 0;
}


/**
 * @brief Delete file
 * @param[in] path Filename of the file to be deleted
 * @return If successful, this function returns 0
 **/

int_t debugStreamUnlink(const char_t * path)
{
   //Not implemented
   return 0;
}


/**
 * @brief Rename file
 * @param[in] old Current name of the file
 * @param[in] new New name for the file
 * @return If successful, this function returns 0
 **/

int_t debugStreamRename(const char_t *old, const char_t *new)
{
   //Not implemented
   return 0;
}
