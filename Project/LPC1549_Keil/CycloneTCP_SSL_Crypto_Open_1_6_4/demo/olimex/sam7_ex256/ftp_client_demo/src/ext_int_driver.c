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
#include "sam7_ex256.h"
#include "core/net.h"
#include "ext_int_driver.h"
#include "debug.h"

//Reset controller control register (RSTC_CR)
#define AT91C_RSTC_KEY_A5 ((uint32_t) 0xA5 << 24)
//Reset controller mode register (RSTC_MR)
#define AT91C_RSTC_ERSTL_1MS (4 << 8)


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
   AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOB);

   //Enable pull-up resistor on INT pin
   AT91C_BASE_PIOB->PIO_PPUER = AT91C_PIO_PB26;
   //Configure the corresponding pin as an input
   AT91C_BASE_PIOB->PIO_ODR = AT91C_PIO_PB26;
   AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB26;
   //Enable interrupts-on-change
   AT91C_BASE_PIOB->PIO_IDR = 0xFFFFFFFF;
   AT91C_BASE_PIOB->PIO_IER = AT91C_PIO_PB26;

   //Reset PHY transceiver by asserting NRST pin
   AT91C_BASE_RSTC->RSTC_RMR = AT91C_RSTC_KEY_A5 | AT91C_RSTC_ERSTL_1MS;
   AT91C_BASE_RSTC->RSTC_RCR = AT91C_RSTC_KEY_A5 | AT91C_RSTC_EXTRST;
   //Wait for the reset to complete
   while(!(AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_NRSTL));

   //Delay before accessing PHY transceiver
   sleep(10);

   //Read PIO ISR register to clear any pending interrupt
   status = AT91C_BASE_PIOB->PIO_ISR;
   //Configure interrupt controller
   AT91C_BASE_AIC->AIC_SMR[AT91C_ID_PIOB] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
   AT91C_BASE_AIC->AIC_SVR[AT91C_ID_PIOB] = (uint32_t) piobIrqWrapper;

   //Clear PHY IRQ interrupt flag
   AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_PIOB);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable interrupt
   AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_PIOB);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable interrupt
   AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_PIOB);
}


/**
 * @brief External interrupt handler
 **/

void extIntHandler(void)
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
   status = AT91C_BASE_PIOB->PIO_ISR;

   //Ensure the INT pin is asserted
   if(!(AT91C_BASE_PIOB->PIO_PDSR & AT91C_PIO_PB26))
   {
      //A PHY event is pending...
      interface->phyEvent = TRUE;
      //Notify the user that the link state has changed
      flag = osSetEventFromIsr(&interface->nicRxEvent);
   }

   //Write AIC_EOICR register before exiting
   AT91C_BASE_AIC->AIC_EOICR = 0;

   //Leave interrupt service routine
   osExitIsr(flag);
}
