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
#include "sam7se_ek.h"
#include "core/net.h"
#include "drivers/dm9000.h"
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

   //Configure interrupt controller
   AT91C_BASE_AIC->AIC_SMR[AT91C_ID_IRQ1] = AT91C_AIC_SRCTYPE_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
   AT91C_BASE_AIC->AIC_SVR[AT91C_ID_IRQ1] = (uint32_t) irq1Wrapper;

   //Clear interrupt flag
   AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_IRQ1);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable IRQ1 interrupt
   AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_IRQ1);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable IRQ1 interrupt
   AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_IRQ1);
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

   //Call interrupt handler
   flag = dm9000IrqHandler(interface);

   //Clear IRQ1 interrupt flag
   AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_IRQ1);
   //Write AIC_EOICR register before exiting
   AT91C_BASE_AIC->AIC_EOICR = 0;

   //Leave interrupt service routine
   osExitIsr(flag);
}
