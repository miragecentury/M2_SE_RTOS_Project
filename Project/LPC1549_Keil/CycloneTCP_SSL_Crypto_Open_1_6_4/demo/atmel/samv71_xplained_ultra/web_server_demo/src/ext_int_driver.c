/**
 * @file ext_int_driver.c
 * @brief External interrupt line driver
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
#include "samv71.h"
#include "core/net.h"
#include "ext_int_driver.h"
#include "debug.h"


/**
 * @brief External interrupt line driver
 **/

const ExtIntDriver extIntDriver =
{
   extIntInit,
   extIntEnableIrq,
   extIntDisableIrq
};


/**
 * @brief EXTI configuration
 * @return Error code
 **/

error_t extIntInit(void)
{
   volatile uint32_t status;

   //Enable PIO peripheral clock
   PMC->PMC_PCER0 = (1 << ID_PIOA);

   //Enable pull-up resistor on PHY_INT pin
   PIOA->PIO_PUER = PIO_PA19;
   //Configure the corresponding pin as an input
   PIOA->PIO_ODR = PIO_PA19;
   PIOA->PIO_PER = PIO_PA19;
   //Enable interrupts-on-change
   PIOA->PIO_IDR = 0xFFFFFFFF;
   PIOA->PIO_IER = PIO_PA19;

   //Read PIO ISR register to clear any pending interrupt
   status = PIOA->PIO_ISR;

   //Configure PIOA interrupt priority
   NVIC_SetPriority(PIOA_IRQn, NVIC_EncodePriority(3, 15, 0));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable PIOA interrupts
   NVIC_EnableIRQ(PIOA_IRQn);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable PIOA interrupts
   NVIC_DisableIRQ(PIOA_IRQn);
}


/**
 * @brief External interrupt handler
 **/

void PIOA_Handler(void)
{
   bool_t flag;
   volatile uint_t status;
   NetInterface *interface;

   //Enter interrupt service routine
   osEnterIsr();

   //Point to the structure describing the network interface
   interface = &netInterface[0];
   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read status register to clear interrupt
   status = PIOA->PIO_ISR;

   //Ensure the EXT3_IRQ pin is asserted
   if(!(PIOA->PIO_PDSR & PIO_PA19))
   {
      //A PHY event is pending...
      interface->phyEvent = TRUE;
      //Notify the user that the link state has changed
      flag = osSetEventFromIsr(&interface->nicRxEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}
