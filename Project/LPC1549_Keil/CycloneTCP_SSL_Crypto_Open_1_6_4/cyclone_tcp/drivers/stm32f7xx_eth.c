/**
 * @file stm32f7xx_eth.c
 * @brief STM32F746/756 Ethernet MAC controller
 *
 * @section License
 *
 * Copyright (C) 2010-2015 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
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

//Switch to the appropriate trace level
#define TRACE_LEVEL NIC_TRACE_LEVEL

//Dependencies
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#include "core/net.h"
#include "drivers/stm32f7xx_eth.h"
#include "debug.h"

//Underlying network interface
static NetInterface *nicDriverInterface;

//IAR EWARM compiler?
#if defined(__ICCARM__)

//Transmit buffer
#pragma data_alignment = 4
static uint8_t txBuffer[STM32F7XX_ETH_TX_BUFFER_COUNT][STM32F7XX_ETH_TX_BUFFER_SIZE];
//Receive buffer
#pragma data_alignment = 4
static uint8_t rxBuffer[STM32F7XX_ETH_RX_BUFFER_COUNT][STM32F7XX_ETH_RX_BUFFER_SIZE];
//Transmit DMA descriptors
#pragma data_alignment = 4
static Stm32f7xxTxDmaDesc txDmaDesc[STM32F7XX_ETH_TX_BUFFER_COUNT];
//Receive DMA descriptors
#pragma data_alignment = 4
static Stm32f7xxRxDmaDesc rxDmaDesc[STM32F7XX_ETH_RX_BUFFER_COUNT];

//Keil MDK-ARM or GCC compiler?
#else

//Transmit buffer
static uint8_t txBuffer[STM32F7XX_ETH_TX_BUFFER_COUNT][STM32F7XX_ETH_TX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Receive buffer
static uint8_t rxBuffer[STM32F7XX_ETH_RX_BUFFER_COUNT][STM32F7XX_ETH_RX_BUFFER_SIZE]
   __attribute__((aligned(4)));
//Transmit DMA descriptors
static Stm32f7xxTxDmaDesc txDmaDesc[STM32F7XX_ETH_TX_BUFFER_COUNT]
   __attribute__((aligned(4)));
//Receive DMA descriptors
static Stm32f7xxRxDmaDesc rxDmaDesc[STM32F7XX_ETH_RX_BUFFER_COUNT]
   __attribute__((aligned(4)));

#endif

//Pointer to the current TX DMA descriptor
static Stm32f7xxTxDmaDesc *txCurDmaDesc;
//Pointer to the current RX DMA descriptor
static Stm32f7xxRxDmaDesc *rxCurDmaDesc;


/**
 * @brief STM32F746/756 Ethernet MAC driver
 **/

const NicDriver stm32f7xxEthDriver =
{
   NIC_TYPE_ETHERNET,
   ETH_MTU,
   stm32f7xxEthInit,
   stm32f7xxEthTick,
   stm32f7xxEthEnableIrq,
   stm32f7xxEthDisableIrq,
   stm32f7xxEthEventHandler,
   stm32f7xxEthSetMacFilter,
   stm32f7xxEthSendPacket,
   stm32f7xxEthWritePhyReg,
   stm32f7xxEthReadPhyReg,
   TRUE,
   TRUE,
   TRUE
};


/**
 * @brief STM32F746/756 Ethernet MAC initialization
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32f7xxEthInit(NetInterface *interface)
{
   error_t error;

   //Debug message
   TRACE_INFO("Initializing STM32F7xx Ethernet MAC...\r\n");

   //Save underlying network interface
   nicDriverInterface = interface;

   //GPIO configuration
   stm32f7xxEthInitGpio(interface);

   //Enable Ethernet MAC clock
   __HAL_RCC_ETHMAC_CLK_ENABLE();
   __HAL_RCC_ETHMACTX_CLK_ENABLE();
   __HAL_RCC_ETHMACRX_CLK_ENABLE();

   //Reset Ethernet MAC peripheral
   __HAL_RCC_ETHMAC_FORCE_RESET();
   __HAL_RCC_ETHMAC_RELEASE_RESET();

   //Perform a software reset
   ETH->DMABMR |= ETH_DMABMR_SR;
   //Wait for the reset to complete
   while(ETH->DMABMR & ETH_DMABMR_SR);

   //Adjust MDC clock range depending on HCLK frequency
   ETH->MACMIIAR = ETH_MACMIIAR_CR_Div102;

   //PHY transceiver initialization
   error = interface->phyDriver->init(interface);
   //Failed to initialize PHY transceiver?
   if(error) return error;

   //Use default MAC configuration
   ETH->MACCR = ETH_MACCR_ROD;

   //Set the MAC address
   ETH->MACA0LR = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ETH->MACA0HR = interface->macAddr.w[2];

   //Initialize hash table
   ETH->MACHTLR = 0;
   ETH->MACHTHR = 0;

   //Configure the receive filter
   ETH->MACFFR = ETH_MACFFR_HPF | ETH_MACFFR_HM;
   //Disable flow control
   ETH->MACFCR = 0;
   //Enable store and forward mode
   ETH->DMAOMR = ETH_DMAOMR_RSF | ETH_DMAOMR_TSF;

   //Configure DMA bus mode
   ETH->DMABMR = ETH_DMABMR_AAB | ETH_DMABMR_USP | ETH_DMABMR_RDP_1Beat |
      ETH_DMABMR_RTPR_1_1 | ETH_DMABMR_PBL_1Beat | ETH_DMABMR_EDE;

   //Initialize DMA descriptor lists
   stm32f7xxEthInitDmaDesc(interface);

   //Disable MAC interrupts
   ETH->MACIMR = 0;
   //Configure DMA interrupts as desired
   ETH->DMAIER |= ETH_DMAIER_NISE | ETH_DMAIER_RIE | ETH_DMAIER_TIE;

   //Set priority grouping (4 bits for pre-emption priority, no bits for subpriority)
   NVIC_SetPriorityGrouping(STM32F7XX_ETH_IRQ_PRIORITY_GROUPING);

   //Configure Ethernet interrupt priority
   NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(STM32F7XX_ETH_IRQ_PRIORITY_GROUPING,
      STM32F7XX_ETH_IRQ_GROUP_PRIORITY, STM32F7XX_ETH_IRQ_SUB_PRIORITY));

   //Enable MAC transmission and reception
   ETH->MACCR |= ETH_MACCR_TE | ETH_MACCR_RE;
   //Enable DMA transmission and reception
   ETH->DMAOMR |= ETH_DMAOMR_ST | ETH_DMAOMR_SR;

   //Force the TCP/IP stack to check the link state
   osSetEvent(&interface->nicRxEvent);
   //STM32F746/756 Ethernet MAC is now ready to send
   osSetEvent(&interface->nicTxEvent);

   //Successful initialization
   return NO_ERROR;
}


//STM32F746G-DISCOVERY evaluation board?
#if defined(USE_STM32746G_DISCO)

/**
 * @brief GPIO configuration
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthInitGpio(NetInterface *interface)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   //Enable SYSCFG clock
   __HAL_RCC_SYSCFG_CLK_ENABLE();

   //Enable GPIO clocks
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();

   //Select RMII interface mode
   SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL;

   //Configure MII pins
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
   GPIO_InitStructure.Alternate = GPIO_AF11_ETH;

   //Configure ETH_RMII_REF_CLK (PA1), ETH_MDIO (PA2) and ETH_RMII_CRS_DV (PA7)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   //Configure ETH_MDC (PC1), ETH_RMII_RXD0 (PC4) and ETH_RMII_RXD1 (PC5)
   GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Configure ETH_RMII_RXER (PG2), RMII_TX_EN (PG11), ETH_RMII_TXD0 (PG13)
   //and ETH_RMII_TXD1 (PG14)
   GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

#endif


/**
 * @brief Initialize DMA descriptor lists
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;

   //Initialize TX DMA descriptor list
   for(i = 0; i < STM32F7XX_ETH_TX_BUFFER_COUNT; i++)
   {
      //Use chain structure rather than ring structure
      txDmaDesc[i].tdes0 = ETH_TDES0_IC | ETH_TDES0_TCH;
      //Initialize transmit buffer size
      txDmaDesc[i].tdes1 = 0;
      //Transmit buffer address
      txDmaDesc[i].tdes2 = (uint32_t) txBuffer[i];
      //Next descriptor address
      txDmaDesc[i].tdes3 = (uint32_t) &txDmaDesc[i + 1];
      //Reserved fields
      txDmaDesc[i].tdes4 = 0;
      txDmaDesc[i].tdes5 = 0;
      //Transmit frame time stamp
      txDmaDesc[i].tdes6 = 0;
      txDmaDesc[i].tdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   txDmaDesc[i - 1].tdes3 = (uint32_t) &txDmaDesc[0];
   //Point to the very first descriptor
   txCurDmaDesc = &txDmaDesc[0];

   //Initialize RX DMA descriptor list
   for(i = 0; i < STM32F7XX_ETH_RX_BUFFER_COUNT; i++)
   {
      //The descriptor is initially owned by the DMA
      rxDmaDesc[i].rdes0 = ETH_RDES0_OWN;
      //Use chain structure rather than ring structure
      rxDmaDesc[i].rdes1 = ETH_RDES1_RCH | (STM32F7XX_ETH_RX_BUFFER_SIZE & ETH_RDES1_RBS1);
      //Receive buffer address
      rxDmaDesc[i].rdes2 = (uint32_t) rxBuffer[i];
      //Next descriptor address
      rxDmaDesc[i].rdes3 = (uint32_t) &rxDmaDesc[i + 1];
      //Extended status
      rxDmaDesc[i].rdes4 = 0;
      //Reserved field
      rxDmaDesc[i].rdes5 = 0;
      //Receive frame time stamp
      rxDmaDesc[i].rdes6 = 0;
      rxDmaDesc[i].rdes7 = 0;
   }

   //The last descriptor is chained to the first entry
   rxDmaDesc[i - 1].rdes3 = (uint32_t) &rxDmaDesc[0];
   //Point to the very first descriptor
   rxCurDmaDesc = &rxDmaDesc[0];

   //Start location of the TX descriptor list
   ETH->DMATDLAR = (uint32_t) txDmaDesc;
   //Start location of the RX descriptor list
   ETH->DMARDLAR = (uint32_t) rxDmaDesc;
}


/**
 * @brief STM32F746/756 Ethernet MAC timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthTick(NetInterface *interface)
{
   //Handle periodic operations
   interface->phyDriver->tick(interface);
}


/**
 * @brief Enable interrupts
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthEnableIrq(NetInterface *interface)
{
   //Enable Ethernet MAC interrupts
   NVIC_EnableIRQ(ETH_IRQn);
   //Enable Ethernet PHY interrupts
   interface->phyDriver->enableIrq(interface);
}


/**
 * @brief Disable interrupts
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthDisableIrq(NetInterface *interface)
{
   //Disable Ethernet MAC interrupts
   NVIC_DisableIRQ(ETH_IRQn);
   //Disable Ethernet PHY interrupts
   interface->phyDriver->disableIrq(interface);
}


/**
 * @brief STM32F746/756 Ethernet MAC interrupt service routine
 **/

void ETH_IRQHandler(void)
{
   bool_t flag;
   uint32_t status;

   //Enter interrupt service routine
   osEnterIsr();

   //This flag will be set if a higher priority task must be woken
   flag = FALSE;

   //Read DMA status register
   status = ETH->DMASR;

   //A packet has been transmitted?
   if(status & ETH_DMASR_TS)
   {
      //Clear TS interrupt flag
      ETH->DMASR = ETH_DMASR_TS;

      //Check whether the TX buffer is available for writing
      if(!(txCurDmaDesc->tdes0 & ETH_TDES0_OWN))
      {
         //Notify the user that the transmitter is ready to send
         flag |= osSetEventFromIsr(&nicDriverInterface->nicTxEvent);
      }
   }
   //A packet has been received?
   if(status & ETH_DMASR_RS)
   {
      //Disable RIE interrupt
      ETH->DMAIER &= ~ETH_DMAIER_RIE;

      //Notify the user that a packet has been received
      flag |= osSetEventFromIsr(&nicDriverInterface->nicRxEvent);
   }

   //Clear NIS interrupt flag
   ETH->DMASR = ETH_DMASR_NIS;

   //Leave interrupt service routine
   osExitIsr(flag);
}


/**
 * @brief STM32F746/756 Ethernet MAC event handler
 * @param[in] interface Underlying network interface
 **/

void stm32f7xxEthEventHandler(NetInterface *interface)
{
   error_t error;
   size_t length;
   bool_t linkStateChange;

   //PHY event is pending?
   if(interface->phyEvent)
   {
      //Acknowledge the event by clearing the flag
      interface->phyEvent = FALSE;
      //Handle PHY specific events
      linkStateChange = interface->phyDriver->eventHandler(interface);

      //Check whether the link state has changed?
      if(linkStateChange)
      {
         //Set speed and duplex mode for proper operation
         if(interface->linkState)
         {
            //Read current MAC configuration
            uint32_t config = ETH->MACCR;

            //10BASE-T or 100BASE-TX operation mode?
            if(interface->speed100)
               config |= ETH_MACCR_FES;
            else
               config &= ~ETH_MACCR_FES;

            //Half-duplex or full-duplex mode?
            if(interface->fullDuplex)
               config |= ETH_MACCR_DM;
            else
               config &= ~ETH_MACCR_DM;

            //Update MAC configuration register
            ETH->MACCR = config;
         }

         //Process link state change event
         nicNotifyLinkChange(interface);
      }
   }

   //Packet received?
   if(ETH->DMASR & ETH_DMASR_RS)
   {
      //Clear interrupt flag
      ETH->DMASR = ETH_DMASR_RS;

      //Process all pending packets
      do
      {
         //Read incoming packet
         error = stm32f7xxEthReceivePacket(interface,
            interface->ethFrame, ETH_MAX_FRAME_SIZE, &length);

         //Check whether a valid packet has been received
         if(!error)
         {
            //Pass the packet to the upper layer
            nicProcessPacket(interface, interface->ethFrame, length);
         }

         //No more data in the receive buffer?
      } while(error != ERROR_BUFFER_EMPTY);
   }

   //Re-enable DMA interrupts
   ETH->DMAIER |= ETH_DMAIER_NISE | ETH_DMAIER_RIE | ETH_DMAIER_TIE;
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t stm32f7xxEthSetMacFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint32_t hashTable[2];

   //Debug message
   TRACE_DEBUG("Updating STM32F7xx hash table...\r\n");

   //Clear hash table
   hashTable[0] = 0;
   hashTable[1] = 0;

   //The MAC filter table contains the multicast MAC addresses
   //to accept when receiving an Ethernet frame
   for(i = 0; i < interface->macFilterSize; i++)
   {
      //Compute CRC over the current MAC address
      crc = stm32f7xxEthCalcCrc(&interface->macFilter[i].addr, sizeof(MacAddr));
      //The upper 6 bits in the CRC register are used to index the contents of the hash table
      k = (crc >> 26) & 0x3F;
      //Update hash table contents
      hashTable[k / 32] |= (1 << (k % 32));
   }

   //Write the hash table
   ETH->MACHTLR = hashTable[0];
   ETH->MACHTHR = hashTable[1];

   //Debug message
   TRACE_DEBUG("  MACHTLR = %08" PRIX32 "\r\n", ETH->MACHTLR);
   TRACE_DEBUG("  MACHTHR = %08" PRIX32 "\r\n", ETH->MACHTHR);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Send a packet
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t stm32f7xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset)
{
   //Retrieve the length of the packet
   size_t length = netBufferGetLength(buffer) - offset;

   //Check the frame length
   if(length > STM32F7XX_ETH_TX_BUFFER_SIZE)
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
      //Report an error
      return ERROR_INVALID_LENGTH;
   }

   //Make sure the current buffer is available for writing
   if(txCurDmaDesc->tdes0 & ETH_TDES0_OWN)
      return ERROR_FAILURE;

   //Copy user data to the transmit buffer
   netBufferRead((uint8_t *) txCurDmaDesc->tdes2, buffer, offset, length);

   //Write the number of bytes to send
   txCurDmaDesc->tdes1 = length & ETH_TDES1_TBS1;
   //Set LS and FS flags as the data fits in a single buffer
   txCurDmaDesc->tdes0 |= ETH_TDES0_LS | ETH_TDES0_FS;
   //Give the ownership of the descriptor to the DMA
   txCurDmaDesc->tdes0 |= ETH_TDES0_OWN;

   //Data synchronization barrier
   __DSB();

   //Transmission is currently suspended?
   if(ETH->DMASR & ETH_DMASR_TBUS)
   {
      //Clear TBUS flag to resume processing
      ETH->DMASR = ETH_DMASR_TBUS;
      //Instruct the DMA to poll the transmit descriptor list
      ETH->DMATPDR = 0;
   }

   //Point to the next descriptor in the list
   txCurDmaDesc = (Stm32f7xxTxDmaDesc *) txCurDmaDesc->tdes3;

   //Check whether the next buffer is available for writing
   if(!(txCurDmaDesc->tdes0 & ETH_TDES0_OWN))
   {
      //The transmitter can accept another packet
      osSetEvent(&interface->nicTxEvent);
   }

   //Data successfully written
   return NO_ERROR;
}


/**
 * @brief Receive a packet
 * @param[in] interface Underlying network interface
 * @param[out] buffer Buffer where to store the incoming data
 * @param[in] size Maximum number of bytes that can be received
 * @param[out] length Number of bytes that have been received
 * @return Error code
 **/

error_t stm32f7xxEthReceivePacket(NetInterface *interface,
   uint8_t *buffer, size_t size, size_t *length)
{
   error_t error;
   size_t n;

   //The current buffer is available for reading?
   if(!(rxCurDmaDesc->rdes0 & ETH_RDES0_OWN))
   {
      //FS and LS flags should be set
      if((rxCurDmaDesc->rdes0 & ETH_RDES0_FS) && (rxCurDmaDesc->rdes0 & ETH_RDES0_LS))
      {
         //Make sure no error occurred
         if(!(rxCurDmaDesc->rdes0 & ETH_RDES0_ES))
         {
            //Retrieve the length of the frame
            n = (rxCurDmaDesc->rdes0 & ETH_RDES0_FL) >> 16;
            //Limit the number of data to read
            n = MIN(n, size);

            //Copy data from the receive buffer
            memcpy(buffer, (uint8_t *) rxCurDmaDesc->rdes2, n);

            //Total number of bytes that have been received
            *length = n;
            //Packet successfully received
            error = NO_ERROR;
         }
         else
         {
            //The received packet contains an error
            error = ERROR_INVALID_PACKET;
         }
      }
      else
      {
         //The packet is not valid
         error = ERROR_INVALID_PACKET;
      }

      //Give the ownership of the descriptor back to the DMA
      rxCurDmaDesc->rdes0 = ETH_RDES0_OWN;
      //Point to the next descriptor in the list
      rxCurDmaDesc = (Stm32f7xxRxDmaDesc *) rxCurDmaDesc->rdes3;
   }
   else
   {
      //No more data in the receive buffer
      error = ERROR_BUFFER_EMPTY;
   }

   //Reception process is suspended?
   if(ETH->DMASR & ETH_DMASR_RBUS)
   {
      //Clear RBUS flag to resume processing
      ETH->DMASR = ETH_DMASR_RBUS;
      //Instruct the DMA to poll the receive descriptor list
      ETH->DMARPDR = 0;
   }

   //Return status code
   return error;
}


/**
 * @brief Write PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 **/

void stm32f7xxEthWritePhyReg(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
   //Take care not to alter MDC clock configuration
   uint32_t value = ETH->MACMIIAR & ETH_MACMIIAR_CR;
   //Set up a write operation
   value |= ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;
   //PHY address
   value |= (phyAddr << 11) & ETH_MACMIIAR_PA;
   //Register address
   value |= (regAddr << 6) & ETH_MACMIIAR_MR;

   //Data to be written in the PHY register
   ETH->MACMIIDR = data & ETH_MACMIIDR_MD;

   //Start a write operation
   ETH->MACMIIAR = value;
   //Wait for the write to complete
   while(ETH->MACMIIAR & ETH_MACMIIAR_MB);
}


/**
 * @brief Read PHY register
 * @param[in] phyAddr PHY address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t stm32f7xxEthReadPhyReg(uint8_t phyAddr, uint8_t regAddr)
{
   //Take care not to alter MDC clock configuration
   uint32_t value = ETH->MACMIIAR & ETH_MACMIIAR_CR;
   //Set up a read operation
   value |= ETH_MACMIIAR_MB;
   //PHY address
   value |= (phyAddr << 11) & ETH_MACMIIAR_PA;
   //Register address
   value |= (regAddr << 6) & ETH_MACMIIAR_MR;

   //Start a read operation
   ETH->MACMIIAR = value;
   //Wait for the read to complete
   while(ETH->MACMIIAR & ETH_MACMIIAR_MB);

   //Return PHY register contents
   return ETH->MACMIIDR & ETH_MACMIIDR_MD;
}


/**
 * @brief CRC calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t stm32f7xxEthCalcCrc(const void *data, size_t length)
{
   uint_t i;
   uint_t j;

   //Point to the data over which to calculate the CRC
   const uint8_t *p = (uint8_t *) data;
   //CRC preset value
   uint32_t crc = 0xFFFFFFFF;

   //Loop through data
   for(i = 0; i < length; i++)
   {
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         //Update CRC value
         if(((crc >> 31) ^ (p[i] >> j)) & 0x01)
            crc = (crc << 1) ^ 0x04C11DB7;
         else
            crc = crc << 1;
      }
   }

   //Return CRC value
   return ~crc;
}
