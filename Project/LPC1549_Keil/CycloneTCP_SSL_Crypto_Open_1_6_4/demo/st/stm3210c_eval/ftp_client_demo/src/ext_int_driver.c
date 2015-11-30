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
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32_eval.h"
#include "stm3210c_eval_ioe.h"
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
   //Configure IO expander
   IOE_Config();
   //Configure PHY transceiver IRQ
   IOE_ITConfig(IOE_ITSRC_MII_IRQ);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable EXTI15_10 interrupt
   NVIC_EnableIRQ(EXTI15_10_IRQn);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable EXTI15_10 interrupt
   NVIC_DisableIRQ(EXTI15_10_IRQn);
}


/**
 * @brief External interrupt handler
 **/

void EXTI15_10_IRQHandler(void)
{
   bool_t flag;
   NetInterface *interface;

   //Enter interrupt service routine
   osEnterIsr();

   //Point to the structure describing the network interface
   interface = &netInterface[0];
   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Check whether the specified EXTI line is asserted
   if(EXTI_GetITStatus(EXTI_LINE_IOE_ITLINE) != RESET)
   {
      //Clear interrupt flag
      EXTI_ClearITPendingBit(EXTI_LINE_IOE_ITLINE);

      //Check IO expander interrupt status
      if(IOE_GetGITStatus(IOE_2_ADDR, IOE_GIT_GPIO) != RESET)
      {
         //Clear interrupt flag
         IOE_ClearGITPending(IOE_2_ADDR, IOE_GIT_GPIO);

         //Checks the status of the MII_IRQ pin
         if(IOE_GetIOITStatus(IOE_2_ADDR, IOE_MII_IRQ_IT) != RESET)
         {
            //Clear interrupt flag
            IOE_ClearIOITPending(IOE_2_ADDR, IOE_MII_IRQ_IT);
            //A PHY event is pending...
            interface->phyEvent = TRUE;
            //Notify the user that the link state has changed
            flag = osSetEventFromIsr(&interface->nicRxEvent);
         }
      }
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}
