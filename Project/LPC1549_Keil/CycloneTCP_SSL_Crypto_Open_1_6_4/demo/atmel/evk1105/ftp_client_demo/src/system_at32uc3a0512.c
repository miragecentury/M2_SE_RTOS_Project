/**
 * @file system_at32uc3a0512.c
 * @brief AT32UC3A0512 system initialization
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
#include <stdlib.h>
#include <avr32/io.h>
#include "evk1105.h"
#include "pm.h"
#include "flashc.h"
#include "sdramc.h"

//Low-level initialization function
extern void _init_startup(void);


/**
 * @brief System initialization
 **/

void systemInit(void)
{
   //Low-level initialization
   _init_startup();

   //Switch main clock to OSC0 (12MHz)
   pm_switch_to_osc0(&AVR32_PM, OSC0_FREQ, AVR32_PM_OSCCTRL1_STARTUP_2048_RCOSC);

   //Start OSC1 (11.2896MHz)
   pm_enable_osc1_crystal(&AVR32_PM, OSC1_FREQ);
   pm_enable_clk1(&AVR32_PM, AVR32_PM_OSCCTRL1_STARTUP_2048_RCOSC);

   //Configure PLL0 (132MHz)
   pm_pll_setup(&AVR32_PM, 0, 10, 1, 0, 16);
   //Set PLL operating range (80 to 180MHz) and output divider (2)
   pm_pll_set_option(&AVR32_PM, 0, 1, 1, 0);

   //Start PLL0
   pm_pll_enable(&AVR32_PM, 0);
   //Wait for the PLL to lock
   pm_wait_for_pll0_locked(&AVR32_PM);

   //Set FLASH wait-state
   flashc_set_wait_state(1);

   //Switch main clock to PLL0
   pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0);

   //Configure PLL1 (96MHz)
   pm_pll_setup(&AVR32_PM, 1, 7, 1, 0, 16);
   //Set PLL operating range (80 to 180MHz) and output divider (2)
   pm_pll_set_option(&AVR32_PM, 1, 1, 1, 0);

   //Start PLL1
   pm_pll_enable(&AVR32_PM, 1);
   //Wait for the PLL to lock
   pm_wait_for_pll1_locked(&AVR32_PM);

   //Set clock dividers for PBA, PBB and HSB clocks
   pm_cksel(&AVR32_PM, 0, 0, 0, 0, 0, 0);

   //Initialize SDRAM memory
   sdramc_init(HSB_FREQ);
}
