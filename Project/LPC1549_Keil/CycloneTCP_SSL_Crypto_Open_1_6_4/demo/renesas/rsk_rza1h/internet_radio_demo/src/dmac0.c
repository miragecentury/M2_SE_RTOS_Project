/**
 * @file dmac0.c
 * @brief DMAC (Direct Access Memory Controller)
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
#include <string.h>
#include "iodefine.h"
#include "dmac_iobitmask.h"
#include "dev_drv.h"
#include "intc.h"
#include "dmac0.h"
#include "debug.h"


/**
 * @brief Initialize DMAC0 controller
 **/

void dmac0Init(void)
{
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing DMAC0 controller...\r\n");

   //Select DMA mode (register mode)
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_DMS;
   //DMA transfers are continued using the next register set
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_REN;
   //Invert RSEL automatically after a DMA transaction
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_RSW;
   //Execute the next 0 register set
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_RSEL;
   //Stop the DMA transfer without sweeping the buffer
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_SBE;
   //Mask the DMA transfer end interrupt
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_DEM;
   //Set DMA transfer mode (single transfer mode)
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_TM;
   //Destination address counting direction (fixed)
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_DAD;
   //Source address counting direction (increment)
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_SAD;

   //Destination data size (32 bits)
   temp = DMAC0.CHCFG_n & ~DMAC0_CHCFG_n_DDS;
   DMAC0.CHCFG_n = temp | (2 << DMAC0_CHCFG_n_DDS_SHIFT);
   //Source data size (32 bits)
   temp = DMAC0.CHCFG_n & ~DMAC0_CHCFG_n_SDS;
   DMAC0.CHCFG_n = temp | (2 << DMAC0_CHCFG_n_SDS_SHIFT);
   //Set DMAACK output mode (level mode)
   temp = DMAC0.CHCFG_n & ~DMAC0_CHCFG_n_AM;
   DMAC0.CHCFG_n = temp | (1 << DMAC0_CHCFG_n_AM_SHIFT);

   //Detect a DMA request based on the level of the signal
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_LVL;
   //Detect a request when the signal is at the high level
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_HIEN;
   //Do not detect a request even when the signal is at the low level
   DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_LOEN;
   //Request direction (DMAACK is to become active when written)
   DMAC0.CHCFG_n |= DMAC0_CHCFG_n_REQD;

   //Set DMAC channel (CH0/CH8)
   temp = DMAC0.CHCFG_n & ~DMAC0_CHCFG_n_SEL;
   DMAC0.CHCFG_n = temp | (0 << DMAC0_CHCFG_n_SEL_SHIFT);

   //Transfer interval for the DMA channel
   DMAC0.CHITVL_n = 0;
   //Clear channel extension register
   DMAC0.CHEXT_n = 0;
   //Clear channel control register
   DMAC0.CHCTRL_n = 0;

   //DMA resource selectors (SSIF0 peripheral)
   DMAC01.DMARS = (0x38 << DMAC01_DMARS_CH0_MID_SHIFT) |
      (0x01 << DMAC01_DMARS_CH0_RID_SHIFT);

   //Use level output for the DMA transfer end interrupt
   DMAC07.DCTRL_0_7 |= DMAC07_DCTRL_0_7_LVINT;
   //Set transfer priority control mode (fixed priority)
   DMAC07.DCTRL_0_7 &= ~DMAC07_DCTRL_0_7_PR;

   //Register DMAINT0 interrupt handler
   R_INTC_Regist_Int_Func(INTC_ID_DMAINT0, dmac0IrqHandler);
   //Configure DMAINT0 interrupt priority
   R_INTC_Set_Priority(INTC_ID_DMAINT0, DMAC0_IRQ_PRIORITY);
}


/**
 * @brief Configure Next0 register set
 * @param[in] srcAddr Source start address
 * @param[in] destAddr Destination start address
 * @param[in] byteCount Transfer size
 **/

void dmac0SetNext0(const void *srcAddr, const void *destAddr, size_t byteCount)
{
   //Set source start address
   DMAC0.N0SA_n = (uint32_t) srcAddr;
   //Set destination start address
   DMAC0.N0DA_n = (uint32_t) destAddr;
   //Set transfer size
   DMAC0.N0TB_n = byteCount;
}


/**
 * @brief Configure Next1 register set
 * @param[in] srcAddr Source start address
 * @param[in] destAddr Destination start address
 * @param[in] byteCount Transfer size
 **/

void dmac0SetNext1(const void *srcAddr, const void *destAddr, size_t byteCount)
{
   //Set source start address
   DMAC0.N1SA_n = (uint32_t) srcAddr;
   //Set destination start address
   DMAC0.N1DA_n = (uint32_t) destAddr;
   //Set transfer size
   DMAC0.N1TB_n = byteCount;
}


/**
 * @brief Start DMAC0 controller
 **/

void dmac0Start(void)
{
   uint32_t dummy;

   //Make sure that both the EN bit and TACT bit are set to 0
   if(!(DMAC0.CHSTAT_n & (DMAC0_CHSTAT_n_EN | DMAC0_CHSTAT_n_TACT)))
   {
      //Reset channel status register
      DMAC0.CHCTRL_n = DMAC0_CHCTRL_n_SWRST;
      //Dummy read
      dummy = DMAC0.CHCTRL_n;

      //DMA transfers are continued using the next register set
      DMAC0.CHCFG_n |= DMAC0_CHCFG_n_REN;
      //Execute the next 0 register set
      DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_RSEL;
      //An interrupt is issued when the DMA transaction is completed
      DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_DEM;

      //Enable DMA transfer
      DMAC0.CHCTRL_n = DMAC0_CHCTRL_n_SETEN;
   }

   //Enable DMAINT0 interrupt
   R_INTC_Enable(INTC_ID_DMAINT0);
}


/**
 * @brief Stop DMAC0 controller
 **/

void dmac0Stop(void)
{
   //Disable DMAINT0 interrupt
   R_INTC_Disable(INTC_ID_DMAINT0);

   //Check whether the DMA channel is enabled
   if(DMAC0.CHSTAT_n & DMAC0_CHSTAT_n_EN)
   {
      //Suspend the current DMA transfer
      DMAC0.CHCTRL_n = DMAC0_CHCTRL_n_SETSUS;
      //Wait for the DMA transfer to be suspended
      while(!(DMAC0.CHSTAT_n & DMAC0_CHSTAT_n_SUS));

      //Stop the DMA transfer
      DMAC0.CHCTRL_n = DMAC0_CHCTRL_n_CLREN;
      //Wait until the DMA channel is completely inactive
      while(DMAC0.CHSTAT_n & DMAC0_CHSTAT_n_TACT);
   }
}


/**
 * @brief Enable DMAC0 interrupt
 **/

void dmac0EnableIrq(void)
{
   //Enable DMAINT0 interrupt
   R_INTC_Enable(INTC_ID_DMAINT0);
}


/**
 * @brief Enable DMAC0 interrupt
 **/

void dmac0DisableIrq(void)
{
   //Disable DMAINT0 interrupt
   R_INTC_Disable(INTC_ID_DMAINT0);
}
