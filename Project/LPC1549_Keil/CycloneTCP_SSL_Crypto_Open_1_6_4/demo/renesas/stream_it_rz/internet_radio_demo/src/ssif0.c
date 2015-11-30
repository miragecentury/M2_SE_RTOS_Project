/**
 * @file ssif0.c
 * @brief SSIF (Serial Sound Interface)
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
#include "cpg_iobitmask.h"
#include "ssif0.h"
#include "debug.h"


/**
 * @brief Initialize SSIF0 interface
 **/

void ssif0Init(void)
{
   uint32_t temp;

   //Debug message
   TRACE_INFO("Initializing SSIF0 interface...\r\n");

   //Enable SSIF0 peripheral clock
   CPG.STBCR11 &= ~CPG_STBCR11_MSTP115;

   //SSIF0 software reset
   CPG.SWRSTCR1 |= CPG_SWRSTCR1_SRST16;
   CPG.SWRSTCR1 &= ~CPG_SWRSTCR1_SRST16;

   //Configure SSISCK0 (P6_8)
   PORT6.PMCn.BIT.PMCn8 = 1;
   PORT6.PFCn.BIT.PFCn8 = 0;
   PORT6.PFCEn.BIT.PFCEn8 = 1;
   PORT6.PFCAEn.BIT.PFCAEn8 = 0;
   PORT6.PIPCn.BIT.PIPCn8 = 1;

   //Configure SSIWS0 (P6_9)
   PORT6.PMCn.BIT.PMCn9 = 1;
   PORT6.PFCn.BIT.PFCn9 = 0;
   PORT6.PFCEn.BIT.PFCEn9 = 1;
   PORT6.PFCAEn.BIT.PFCAEn9 = 0;
   PORT6.PIPCn.BIT.PIPCn9 = 1;

   //Configure SSIRXD0 (P6_11)
   PORT6.PMCn.BIT.PMCn11 = 1;
   PORT6.PFCn.BIT.PFCn11 = 0;
   PORT6.PFCEn.BIT.PFCEn11 = 1;
   PORT6.PFCAEn.BIT.PFCAEn11 = 0;
   PORT6.PIPCn.BIT.PIPCn11 = 1;

   //Configure SSITXD0 (P6_10)
   PORT6.PMCn.BIT.PMCn10 = 1;
   PORT6.PFCn.BIT.PFCn10 = 0;
   PORT6.PFCEn.BIT.PFCEn10 = 1;
   PORT6.PFCAEn.BIT.PFCAEn10 = 0;
   PORT6.PIPCn.BIT.PIPCn10 = 1;

   //Clock source for oversampling (AUDIO_X1)
   SSIF0.SSICR &= ~SSIF0_SSICR_CKS;

   //Number of channels per system word (1 channel)
   temp = SSIF0.SSICR & ~SSIF0_SSICR_CHNL;
   SSIF0.SSICR = temp | (0 << SSIF0_SSICR_CHNL_SHIFT);
   //Number of bits in a data word (16 bits)
   temp = SSIF0.SSICR & ~SSIF0_SSICR_DWL;
   SSIF0.SSICR = temp | (1 << SSIF0_SSICR_DWL_SHIFT);
   //Number of bits in a system word (16 bits)
   temp = SSIF0.SSICR & ~SSIF0_SSICR_SWL;
   SSIF0.SSICR = temp | (1 << SSIF0_SSICR_SWL_SHIFT);

   //Serial bit clock direction (master mode)
   SSIF0.SSICR |= SSIF0_SSICR_SCKD;
   //Serial WS direction (master mode)
   SSIF0.SSICR |= SSIF0_SSICR_SWSD;
   //Serial bit clock polarity
   SSIF0.SSICR &= ~SSIF0_SSICR_SCKP;
   //Serial WS polarity
   SSIF0.SSICR &= ~SSIF0_SSICR_SWSP;
   //Serial padding polarity
   SSIF0.SSICR &= ~SSIF0_SSICR_SPDP;
   //Serial data alignment
   SSIF0.SSICR &= ~SSIF0_SSICR_SDTA;
   //Parallel data alignment
   SSIF0.SSICR &= ~SSIF0_SSICR_PDTA;
   //Serial data delay
   SSIF0.SSICR |= SSIF0_SSICR_DEL;

   //Serial oversampling clock division ratio (16)
   temp = SSIF0.SSICR & ~SSIF0_SSICR_CKDV;
   SSIF0.SSICR = temp | (4 << SSIF0_SSICR_CKDV_SHIFT);

   //Disable mute
   SSIF0.SSICR &= ~SSIF0_SSICR_MUEN;

   //Enable WS continue mode
   SSIF0.SSITDMR |= SSIF0_SSITDMR_CONT;
   //Disable TDM mode
   SSIF0.SSITDMR &= ~SSIF0_SSITDMR_TDM;

   //Reset transmit FIFO
   SSIF0.SSIFCR |= SSIF0_SSIFCR_TFRST;

   //Transmit data trigger number (7)
   temp = SSIF0.SSIFCR & ~SSIF0_SSIFCR_TTRG;
   SSIF0.SSIFCR = temp | (0 << SSIF0_SSIFCR_TTRG_SHIFT);
}


/**
 * @brief Start SSIF0 interface
 **/

void ssif0Start(void)
{
   //Take TX FIFO out of reset
   SSIF0.SSIFCR &= ~SSIF0_SSIFCR_TFRST;

   //Clear error flags
   SSIF0.SSISR &= ~(SSIF0_SSISR_TUIRQ | SSIF0_SSISR_TOIRQ);
   //Enable error interrupts
   SSIF0.SSICR |= SSIF0_SSICR_TUIEN | SSIF0_SSICR_TOIEN;

   //Enable transmit interrupt
   SSIF0.SSIFCR |= SSIF0_SSIFCR_TIE;
   //Enable transmit operation
   SSIF0.SSICR |= SSIF0_SSICR_TEN;
}


/**
 * @brief Stop SSIF0 interface
 **/

void ssif0Stop(void)
{
   //Disable transmit operation
   SSIF0.SSICR &= ~SSIF0_SSICR_TEN;
   //Reset TX FIFO
   SSIF0.SSIFCR |= SSIF0_SSIFCR_TFRST;
}
