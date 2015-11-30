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
#include "s6e2cc.h"
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
   //Configure INT05_0 (P7D)
   bFM4_GPIO_PFR7_PD = 1;

   //Use INT05_0 at the input pin of EINT channel 5
   bFM4_GPIO_EPFR06_EINT05S0 = 0;
   bFM4_GPIO_EPFR06_EINT05S1 = 0;

   //Disable external interrupts
   bFM4_EXTI_ENIR_EN5 = 0;
   //Set detection condition (falling edge)
   bFM4_EXTI_ELVR_LA5 = 1;
   bFM4_EXTI_ELVR_LB5 = 1;
   //Clear interrupt flag
   bFM_EXTI_EICL_ECL5 = 0;
   //Enable external interrupts
   bFM4_EXTI_ENIR_EN5 = 1;

   //Configure EXINT5 interrupt priority
   NVIC_SetPriority(EXINT5_IRQn, NVIC_EncodePriority(3, 15, 0));

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable EXINT5 interrupts
   NVIC_EnableIRQ(EXINT5_IRQn);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable EXINT5 interrupts
   NVIC_DisableIRQ(EXINT5_IRQn);
}


/**
 * @brief External interrupt handler
 **/

void EXINT5_IRQHandler(void)
{
   bool_t flag;
   NetInterface *interface;

   //Enter interrupt service routine
   osEnterIsr();

   //Point to the structure describing the network interface
   interface = &netInterface[0];
   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Check interrupt status
   if(bFM4_EXTI_EIRR_ER5)
   {
      //Clear interrupt flag
      bFM4_EXTI_EICL_ECL5 = 0;

      //A PHY event is pending...
      interface->phyEvent = TRUE;
      //Notify the user that the link state has changed
      flag = osSetEventFromIsr(&interface->nicRxEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}
