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
#include "sam4e.h"
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
   PMC->PMC_PCER0 = (1 << ID_PIOD);

   //Enable pull-up resistor on PHY IRQ pin
   PIOD->PIO_PUER = PIO_PD28;
   //Configure the corresponding pin as an input
   PIOD->PIO_ODR = PIO_PD28;
   PIOD->PIO_PER = PIO_PD28;
   //Enable interrupts-on-change
   PIOD->PIO_IDR = 0xFFFFFFFF;
   PIOD->PIO_IER = PIO_PD28;

   //Reset PHY transceiver by asserting NRST pin
   //RSTC->RSTC_MR = RSTC_MR_KEY(0xA5) | RSTC_MR_ERSTL(4);
   //RSTC->RSTC_CR = RSTC_CR_KEY(0xA5) | RSTC_CR_EXTRST;
   //Wait for the reset to complete
   //while(!(RSTC->RSTC_SR & RSTC_SR_NRSTL));

   //Delay before accessing PHY transceiver
   sleep(10);

   //Read PIO ISR register to clear any pending interrupt
   status = PIOD->PIO_ISR;

   //Configure PIOD interrupt priority
   NVIC_SetPriority(PIOD_IRQn, NVIC_EncodePriority(3, 15, 0));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable PIOD interrupt
   NVIC_EnableIRQ(PIOD_IRQn);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable PIOD interrupt
   NVIC_DisableIRQ(PIOD_IRQn);
}


/**
 * @brief External interrupt handler
 **/

void PIOD_Handler(void)
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
   status = PIOD->PIO_ISR;

   //Ensure the PHY IRQ pin is asserted
   if(!(PIOD->PIO_PDSR & PIO_PD28))
   {
      //A PHY event is pending...
      interface->phyEvent = TRUE;
      //Notify the user that the link state has changed
      flag = osSetEventFromIsr(&interface->nicRxEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}
