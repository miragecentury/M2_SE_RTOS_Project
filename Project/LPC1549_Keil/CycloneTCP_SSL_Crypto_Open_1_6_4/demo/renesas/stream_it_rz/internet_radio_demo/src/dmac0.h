/**
 * @file dmac0.h
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

#ifndef _DMAC0_H
#define _DMAC0_H

//Dependencies
#include "os_port.h"

//DMA channel 0 interrupt priority
#ifndef DMAC0_IRQ_PRIORITY
   #define DMAC0_IRQ_PRIORITY 25
#elif (DMAC0_IRQ_PRIORITY < 0)
   #error DMAC0_IRQ_PRIORITY parameter is not valid
#endif

//DMAC0 related functions
void dmac0Init(void);

void dmac0SetNext0(const void *srcAddr, const void *destAddr, size_t byteCount);
void dmac0SetNext1(const void *srcAddr, const void *destAddr, size_t byteCount);

void dmac0Start(void);
void dmac0Stop(void);

void dmac0EnableIrq(void);
void dmac0DisableIrq(void);

void dmac0IrqHandler(uint32_t int_sense);

#endif
