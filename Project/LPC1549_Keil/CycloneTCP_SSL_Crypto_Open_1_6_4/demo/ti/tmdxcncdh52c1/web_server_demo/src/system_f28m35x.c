/**
 * @file system_f28m35x
 * @brief System configuration for F28M35x device
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

#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "system_f28m35x.h"

//System clock frequency (75MHz)
uint32_t SystemCoreClock = 75000000;


/**
 * @brief Setup the system clock
 **/

void SystemInit(void)
{
   //Disable protection
   HWREG(SYSCTL_MWRALLOW) = 0xA5A5A5A5;

   // Sets up PLL (M3 running at 75MHz and C28 running at 150MHz)
   SysCtlClockConfigSet(SYSCTL_USE_PLL | (SYSCTL_SPLLIMULT_M & 0xF) |
      SYSCTL_SYSDIV_1 | SYSCTL_M3SSDIV_2 | SYSCTL_XCLKDIV_4);

   //Initialize Flash wait-states
   //FlashInit();

   //Disable clock for watchdog modules
   SysCtlPeripheralDisable(SYSCTL_PERIPH_WDOG1);
   SysCtlPeripheralDisable(SYSCTL_PERIPH_WDOG0);
}


/**
 * @brief Update the variable SystemCoreClock
 **/

void SystemCoreClockUpdate(void)
{
   //Retrieve system clock
   SystemCoreClock = SysCtlClockGet(20000000);
}
