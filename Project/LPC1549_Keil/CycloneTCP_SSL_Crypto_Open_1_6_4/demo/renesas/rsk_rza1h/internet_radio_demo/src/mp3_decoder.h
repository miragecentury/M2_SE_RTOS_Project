/**
 * @file mp3_decoder.h
 * @brief MP3 decoder task
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

#ifndef _MP3_DECODER_H
#define _MP3_DECODER_H

//Dependencies
#include "os_port.h"
#include "core/net.h"
#include "icecast/icecast_client.h"

//Stack size required to run the MP3 decoder
#ifndef MP3_DECODER_STACK_SIZE
   #define MP3_DECODER_STACK_SIZE 800
#elif (MP3_DECODER_STACK_SIZE < 1)
   #error MP3_DECODER_STACK_SIZE parameter is not valid
#endif

//Priority at which the MP3 decoder should run
#ifndef MP3_DECODER_PRIORITY
   #define MP3_DECODER_PRIORITY 2
#elif (MP3_DECODER_PRIORITY < 0)
   #error MP3_DECODER_PRIORITY parameter is not valid
#endif

//MP3 decoder related functions
error_t mp3DecoderStart(IcecastClientContext *icecastClientContext);
void mp3DecoderTask(void *param);

#endif
