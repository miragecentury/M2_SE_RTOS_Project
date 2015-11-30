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
#include "91x_lib.h"
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
   GPIO_InitTypeDef GPIO_InitStructure;
   WIU_InitTypeDef WIU_InitStructure;

   //Enable GPIO5 clock
   SCU_APBPeriphClockConfig(__GPIO5, ENABLE);
   //Enable WIU clock
   SCU_APBPeriphClockConfig(__WIU, ENABLE);

   //Configure EXTINT13 (P5.5) as an input
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
   GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
   GPIO_InitStructure.GPIO_IPInputConnected = GPIO_IPInputConnected_Disable;
   GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1;
   GPIO_Init(GPIO5, &GPIO_InitStructure);

   //Reset WIU peripheral
   WIU_DeInit();

   //Configure external interrupt line
   WIU_InitStructure.WIU_Line = WIU_Line13;
   WIU_InitStructure.WIU_TriggerEdge = WIU_FallingEdge;
   WIU_Init(&WIU_InitStructure);

   //Configure WIU interrupt priority
   VIC_Config(WIU_ITLine, VIC_IRQ, 15);

   //Enable WIU
   WIU_Cmd(ENABLE);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable external interrupts
 **/

void extIntEnableIrq(void)
{
   //Enable WIU interrupt
   VIC_ITCmd(WIU_ITLine, ENABLE);
}


/**
 * @brief Disable external interrupts
 **/

void extIntDisableIrq(void)
{
   //Disable WIU interrupt
   VIC_ITCmd(WIU_ITLine, DISABLE);
}


/**
 * @brief External interrupt handler
 **/

void WIU_IRQHandler(void)
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
   if(WIU_GetITStatus(WIU_Line13) != RESET)
   {
      //Clear interrupt flag
      WIU_ClearITPendingBit(WIU_Line13);
      //A PHY event is pending...
      interface->phyEvent = TRUE;
      //Notify the user that the link state has changed
      flag = osSetEventFromIsr(&interface->nicRxEvent);
   }

   //Leave interrupt service routine
   osExitIsr(flag);
}
