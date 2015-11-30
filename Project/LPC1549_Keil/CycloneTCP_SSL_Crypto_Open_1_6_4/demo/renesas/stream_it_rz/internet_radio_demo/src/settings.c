/**
 * @file settings.c
 * @brief Settings management
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "settings.h"
#include "eeprom.h"

//Global variables
Settings appSettings;


/**
 * @brief Retrieve default settings
 * @param[out] settings Pointer to the settings structure
 * @return Error code
 **/

error_t getDefaultSettings(Settings *settings)
{
   //Check parameters
   if(settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Clear settings
   memset(settings, 0, sizeof(Settings));

   //URL
   strcpy(settings->icecast.url, "ice-sov.musicradio.com/CapitalMP3");
   //Icecast server port
   settings->icecast.port = 80;

   //MAC address
   macStringToAddr("00-AB-CD-EF-00-A1", &settings->lan.macAddr);
   //Host name
   strcpy(settings->lan.hostname, "WebRadio");

   //Enable DHCP client
   settings->lan.enableDhcp = TRUE;
   //IPv4 address
   ipv4StringToAddr("192.168.0.10", &settings->lan.hostAddr);
   //Subnet mask
   ipv4StringToAddr("255.255.255.0", &settings->lan.subnetMask);
   //Default gateway
   ipv4StringToAddr("192.168.0.254", &settings->lan.defaultGateway);
   //Primary DNS
   ipv4StringToAddr("8.8.8.8", &settings->lan.primaryDns);
   //Secondary DNS
   ipv4StringToAddr("8.8.4.4", &settings->lan.secondaryDns);

   //Enable Proxy server
   settings->proxy.enable = FALSE;
   //Proxy server name
   strcpy(settings->proxy.name, "192.168.0.1");
   //Proxy server port
   settings->proxy.port = 8080;

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Load settings from external EEPROM
 * @param[out] settings Pointer to the settings structure
 * @return Error code
 **/

error_t loadSettings(Settings *settings)
{
   error_t error;
   uint32_t crc;
   uint8_t version;

   //Check parameters
   if(settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Read user settings from EEPROM memory
   error = eepromRead(SETTINGS_BASE_ADDR, settings, sizeof(Settings));

   //Check status code
   if(!error)
   {
      //Compute CRC over the complete structure
      crc = calcCrc32(settings, sizeof(Settings) - sizeof(uint32_t));

      //Wrong CRC?
      if(crc != settings->crc)
         error = ERROR_BAD_CRC;
   }

   //Failed to retrieve user settings?
   if(error)
   {
      //Restore factory settings...
      getDefaultSettings(settings);
   }

   //Return status code
   return error;
}


/**
 * @brief Save settings to external EEPROM
 * @param[out] settings Pointer to the settings structure
 * @return Error code
 **/

error_t saveSettings(Settings *settings)
{
   error_t error;
   uint32_t crc;

   //Check parameters
   if(settings == NULL)
      return ERROR_INVALID_PARAMETER;

   //Compute CRC over the complete structure
   crc = calcCrc32(settings, sizeof(Settings) - sizeof(uint32_t));
   //Save CRC
   settings->crc = crc;

   //Write settings to EEPROM memory
   error = eepromWrite(SETTINGS_BASE_ADDR, settings, sizeof(Settings));
   //Return status code
   return error;
}


/**
 * @brief CRC-32 calculation
 * @param[in] data Pointer to the data over which to calculate the CRC
 * @param[in] length Number of bytes to process
 * @return Resulting CRC value
 **/

uint32_t calcCrc32(const void *data, size_t length)
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
      //Update CRC value
      crc ^= p[i];
      //The message is processed bit by bit
      for(j = 0; j < 8; j++)
      {
         if(crc & 0x00000001)
            crc = (crc >> 1) ^ 0xEDB88320;
         else
            crc = crc >> 1;
      }
   }

   //Return 1's complement value
   return ~crc;
}
