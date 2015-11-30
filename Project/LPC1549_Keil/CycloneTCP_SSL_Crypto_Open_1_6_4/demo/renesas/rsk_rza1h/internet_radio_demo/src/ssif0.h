/**
 * @file ssif0.h
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

#ifndef _SSIF0_H
#define _SSIF0_H

//Dependencies
#include "os_port.h"

//SSICR register
#define SSIF0_SSICR_CKS          0x40000000
#define SSIF0_SSICR_TUIEN        0x20000000
#define SSIF0_SSICR_TOIEN        0x10000000
#define SSIF0_SSICR_RUIEN        0x08000000
#define SSIF0_SSICR_ROIEN        0x04000000
#define SSIF0_SSICR_IIEN         0x02000000
#define SSIF0_SSICR_CHNL         0x00C00000
#define SSIF0_SSICR_DWL          0x00380000
#define SSIF0_SSICR_SWL          0x00030000
#define SSIF0_SSICR_SCKD         0x00008000
#define SSIF0_SSICR_SWSD         0x00004000
#define SSIF0_SSICR_SCKP         0x00002000
#define SSIF0_SSICR_SWSP         0x00001000
#define SSIF0_SSICR_SPDP         0x00000800
#define SSIF0_SSICR_SDTA         0x00000400
#define SSIF0_SSICR_PDTA         0x00000200
#define SSIF0_SSICR_DEL          0x00000100
#define SSIF0_SSICR_CKDV         0x000000F0
#define SSIF0_SSICR_MUEN         0x00000008
#define SSIF0_SSICR_TEN          0x00000002
#define SSIF0_SSICR_REN          0x00000001

#define SSIF0_SSICR_CHNL_SHIFT   22
#define SSIF0_SSICR_DWL_SHIFT    19
#define SSIF0_SSICR_SWL_SHIFT    16
#define SSIF0_SSICR_CKDV_SHIFT   4

//SSISR register
#define SSIF0_SSISR_TUIRQ        0x20000000
#define SSIF0_SSISR_TOIRQ        0x10000000
#define SSIF0_SSISR_RUIRQ        0x08000000
#define SSIF0_SSISR_ROIRQ        0x04000000
#define SSIF0_SSISR_IIRQ         0x02000000
#define SSIF0_SSISR_TCHNO        0x00000060
#define SSIF0_SSISR_TSWNO        0x00000010
#define SSIF0_SSISR_RCHNO        0x0000000C
#define SSIF0_SSISR_RSWNO        0x00000002
#define SSIF0_SSISR_IDST         0x00000001

#define SSIF0_SSISR_TCHNO_SHIFT  5
#define SSIF0_SSISR_RCHNO_SHIFT  2

//SSIFCR register
#define SSIF0_SSIFCR_TTRG        0x000000C0
#define SSIF0_SSIFCR_RTRG        0x00000030
#define SSIF0_SSIFCR_TIE         0x00000008
#define SSIF0_SSIFCR_RIE         0x00000004
#define SSIF0_SSIFCR_TFRST       0x00000002
#define SSIF0_SSIFCR_RFRST       0x00000001

#define SSIF0_SSIFCR_TTRG_SHIFT  6
#define SSIF0_SSIFCR_RTRG_SHIFT  4

//SSITDMR register
#define SSIF0_SSITDMR_RXDMUTE    0x00020000
#define SSIF0_SSITDMR_CONT       0x00000100
#define SSIF0_SSITDMR_TDM        0x00000001

//SSIF0 related functions
void ssif0Init(void);
void ssif0Start(void);
void ssif0Stop(void);

#endif
