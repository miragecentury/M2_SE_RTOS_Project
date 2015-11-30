/**
 * @file mp3_decoder.c
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


//Dependencies
#include "iodefine.h"
#include "dmac_iobitmask.h"
#include "dev_drv.h"
#include "intc.h"
#include "os_port.h"
#include "mp3dec.h"
#include "mp3_decoder.h"
#include "ssif0.h"
#include "dmac0.h"
#include "debug.h"

//Input buffer size (encoded bitstream)
#define INPUT_SIZE 4096
//Output buffer size (PCM samples)
#define OUTPUT_SIZE 4


/**
 * @brief PCM sample buffer
 **/

typedef struct
{
   uint_t length;
   uint32_t data[1152];
} PcmSampleBuffer;


//Global variables
static bool_t running;
static uint_t next;
static uint8_t input[INPUT_SIZE];
static uint_t inputLength;
static uint8_t *inputWritePos;
static uint8_t *inputReadPos;
static uint_t outputLength;
static uint_t outputWriteIndex;
static uint_t outputReadIndex;
static OsEvent outputWriteEvent;

//The PCM sample buffer is shared with the DMA controller and
//shall reside in a non-cacheable memory region
static PcmSampleBuffer output[OUTPUT_SIZE]
   __attribute__((section(".BSS_DMAC_SAMPLE_INTERNAL_RAM")));


/**
 * @brief Start MP3 decoder
 * @param[in] context Pointer to the Icecast client context
 * @return Error code
 **/

error_t mp3DecoderStart(IcecastClientContext *icecastClientContext)
{
   OsTask *task;

   //Debug message
   TRACE_INFO("Starting MP3 decoder...\r\n");

   //Create a task to decode the MP3 stream
   task = osCreateTask("MP3 Decoder", mp3DecoderTask,
      icecastClientContext, MP3_DECODER_STACK_SIZE, MP3_DECODER_PRIORITY);

   //Failed to create the task?
   if(task == OS_INVALID_HANDLE)
      return ERROR_OUT_OF_RESOURCES;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief MP3 decoder task
 * @param[in] param Pointer to the Icecast client context
 **/

void mp3DecoderTask(void *param)
{
   error_t error;
   int_t ret;
   uint_t i;
   uint_t n;
   uint8_t *p;
   IcecastClientContext *icecastClientContext;
   HMP3Decoder mp3DecoderContext;
   MP3FrameInfo mp3FrameInfo;

   //Retrieve the Icecast client context
   icecastClientContext = (IcecastClientContext *) param;

   //Create an event to notify the MP3 decoder when
   //the output buffer is available for writing
   if(!osCreateEvent(&outputWriteEvent))
   {
      //Report an error
      TRACE_ERROR("Failed to create event!\r\n");
   }

   //Variable initialization
   running = FALSE;
   inputLength = 0;
   inputWritePos = input;
   inputReadPos = input;
   outputLength = 0;
   outputWriteIndex = 0;
   outputReadIndex = 0;

   //MP3 decoder initialization
   mp3DecoderContext = MP3InitDecoder();

   //Failed to initialize MP3 decoder?
   if(!mp3DecoderContext)
   {
      //Report an error
      TRACE_ERROR("Failed to initialize MP3 decoder!\r\n");
   }

   //Main loop
   while(1)
   {
      //Read more data
      error = icecastClientReadStream(icecastClientContext, inputWritePos,
         input + INPUT_SIZE - inputWritePos, &n, INFINITE_DELAY);

      //Check status code
      if(error)
      {
         //This should never happen since the call to the function is blocking
         continue;
      }

      //Increment write pointer
      inputWritePos += n;
      inputLength += n;

      //Disable DMAC0 interrupt
      dmac0DisableIrq();

      //Check whether the buffer is available for writing
      if(outputLength < OUTPUT_SIZE)
         osSetEvent(&outputWriteEvent);

      //Re-enable DMAC0 interrupt
      dmac0EnableIrq();

      //Process input stream
      while(1)
      {
         //Wait for the output buffer is available for writing
         osWaitForEvent(&outputWriteEvent, 1000);

         //Check whether the buffer is available for writing
         if(outputLength < OUTPUT_SIZE)
         {
            //Find synchronization word
            n = MP3FindSyncWord(inputReadPos, inputLength);

            //Failed to find synchronization word?
            if((int_t) n < 0)
            {
               //Keep the last byte of the stream
               input[0] = inputReadPos[inputLength - 1];
               //Update pointers
               inputWritePos = input + 1;
               inputReadPos = input;
               //Discard the rest of the stream
               inputLength = 1;
               //Read more data...
               break;
            }

            //Skip the data bytes that precede the synchronization word
            inputReadPos += n;
            inputLength -= n;

            //Point to the MP3 frame to be decoded
            p = inputReadPos;
            n = inputLength;

            //Decode current MP3 frame
            ret = MP3Decode(mp3DecoderContext, &p, (int_t *) &n,
               (int16_t *) (output[outputWriteIndex].data), 0);

            //Out of data?
            if(ret == ERR_MP3_INDATA_UNDERFLOW)
            {
               if(inputLength >= INPUT_SIZE)
               {
                  //Flush buffer
                  inputWritePos = input;
                  inputReadPos = input;
                  inputLength = 0;
                  //Read more data...
                  break;
               }
               else if(inputWritePos >= (input + INPUT_SIZE))
               {
                  //Move data to the beginning of the buffer
                  memmove(input, inputReadPos, inputLength);
                  //Update pointers
                  inputWritePos = input + inputLength;
                  inputReadPos = input;
                  //Read more data...
                  break;
               }
               else
               {
                  //Read more data...
                  break;
               }
            }
            //Not enough data in bit reservoir from previous frames?
            else if(ret == ERR_MP3_MAINDATA_UNDERFLOW)
            {
               //Skip current frame
               inputReadPos = p;
               inputLength = n;
            }
            //Failed to decode MP3 frame?
            else if(ret)
            {
               if(inputLength > 0)
               {
                  //Skip first byte
                  inputReadPos++;
                  inputLength--;
               }
               else
               {
                  //Flush buffer
                  inputWritePos = input;
                  inputReadPos = input;
                  inputLength = 0;
                  //Read more data...
                  break;
               }
            }
            //MP3 frame successfully decoded?
            else
            {
               //Point to the next MP3 frame to be decoded
               inputReadPos = p;
               inputLength = n;

               //Retrieve information about the last MP3 frame decoded
               MP3GetLastFrameInfo(mp3DecoderContext, &mp3FrameInfo);

               //Mono audio?
               if(mp3FrameInfo.nChans < 2)
               {
                  //Point to the PCM samples
                  uint16_t *data = (uint16_t *) output[outputWriteIndex].data;

                  //Convert from mono to stereo
                  for(i = mp3FrameInfo.outputSamps; i > 0; i--)
                  {
                     //Left channel
                     data[2 * i - 2] = data [i - 1];
                     //Right channel
                     data[2 * i - 1] = data [i - 1];
                  }

                  //Size of the buffer in bytes
                  output[outputWriteIndex].length = mp3FrameInfo.outputSamps * 4;
               }
               //Stereo audio?
               else
               {
                  //Size of the buffer in bytes
                  output[outputWriteIndex].length = mp3FrameInfo.outputSamps * 2;
               }

               //Disable DMAC0 interrupt
               dmac0DisableIrq();

               //Increment index and wrap around if necessary
               if(++outputWriteIndex >= OUTPUT_SIZE)
                  outputWriteIndex = 0;

               //Update buffer length
               outputLength++;

               //Re-enable DMAC0 interrupt
               dmac0EnableIrq();
            }
         }
         else
         {
            //Just for sanity...
            running = FALSE;
         }

         //Wait for the output buffer to be full before starting DMA transfer
         if(!running && outputLength >= OUTPUT_SIZE)
         {
            //Debug message
            TRACE_DEBUG("Starting DMA transfer...\r\n");

            //Disable any pending DMA transfer...
            dmac0Stop();
            //Stop SSIF interface
            ssif0Stop();
            //Re-initialize DMA controller
            dmac0Init();

            //Configure Next0 register set
            dmac0SetNext0(output[outputReadIndex].data,
               (void *) &SSIF0.SSIFTDR, output[outputReadIndex].length);

            //Increment index and wrap around if necessary
            if(++outputReadIndex >= OUTPUT_SIZE)
               outputReadIndex = 0;

            //Configure Next1 register set
            dmac0SetNext1(output[outputReadIndex].data,
               (void *) &SSIF0.SSIFTDR, output[outputReadIndex].length);

            //Increment index and wrap around if necessary
            if(++outputReadIndex >= OUTPUT_SIZE)
               outputReadIndex = 0;

            //Select the next register set to write
            next = 0;
            //SSIF is now running...
            running = TRUE;

            //Start SSIF interface
            ssif0Start();
            //Start DMA transfer
            dmac0Start();

            //Debug message
            TRACE_DEBUG("Playing...\r\n");
         }

         //Disable DMAC0 interrupt
         dmac0DisableIrq();

         //Check whether the buffer is available for writing
         if(outputLength < OUTPUT_SIZE)
            osSetEvent(&outputWriteEvent);

         //Re-enable DMAC0 interrupt
         dmac0EnableIrq();
      }
   }
}


/**
 * @brief DMAC0 interrupt service routine
 * @param intSense Unused parameter
 **/

void dmac0IrqHandler(uint32_t int_sense)
{
   //Clear TC and END flags
   DMAC0.CHCTRL_n = DMAC0_CHCTRL_n_CLRTC | DMAC0_CHCTRL_n_CLREND;

   //DMA operation disabled?
   if(!(DMAC0.CHSTAT_n & DMAC0_CHSTAT_n_EN))
   {
      //Flush output buffer
      outputWriteIndex = 0;
      outputReadIndex = 0;
      outputLength = 0;

      //Mask the DMA transfer end interrupt
      DMAC0.CHCFG_n |= DMAC0_CHCFG_n_DEM;
      //SSIF is no more running...
      running = FALSE;
   }
   //Any data pending in the buffer?
   else if(outputLength > 0)
   {
      //Check the register set currently selected
      //if(!next)
      if(DMAC0.CHSTAT_n & DMAC0_CHSTAT_n_SR)
      {
         //Configure Next0 register set
         dmac0SetNext0(output[outputReadIndex].data,
            (void *) &SSIF0.SSIFTDR, output[outputReadIndex].length);

         //Select the next register set to write
         next = 1;
      }
      else
      {
         //Configure Next1 register set
         dmac0SetNext1(output[outputReadIndex].data,
            (void *) &SSIF0.SSIFTDR, output[outputReadIndex].length);

         //Select the next register set to write
         next = 0;
      }

      //Increment index and wrap around if necessary
      if(++outputReadIndex >= OUTPUT_SIZE)
         outputReadIndex = 0;

      //Update buffer length
      outputLength--;

       //DMA transfers are continued using the next register set
       DMAC0.CHCFG_n |= DMAC0_CHCFG_n_REN;
       //Unmask the DMA transfer end interrupt
       DMAC0.CHCFG_n &= ~DMAC0_CHCFG_n_DEM;
   }
   //Empty buffer?
   else
   {
      //Mask the DMA transfer end interrupt
      DMAC0.CHCFG_n |= DMAC0_CHCFG_n_DEM;
      //SSIF is no more running...
      running = FALSE;
   }

   //Check whether the buffer is available for writing
   if(outputLength < OUTPUT_SIZE)
     osSetEventFromIsr(&outputWriteEvent);
}
