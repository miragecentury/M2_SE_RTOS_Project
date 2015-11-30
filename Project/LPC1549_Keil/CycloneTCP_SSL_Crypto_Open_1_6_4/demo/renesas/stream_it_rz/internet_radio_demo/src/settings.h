/**
 * @file settings.h
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

#ifndef _SETTINGS_H
#define _SETTINGS_H

//Dependencies
#include "core/net.h"

//Settings base address
#define SETTINGS_BASE_ADDR 0x0000


/**
 * @brief Icecast settings
 **/

typedef __start_packed struct
{
   char_t url[60];
   uint16_t port;
} __end_packed IcecastSettings;


/**
 * @brief LAN settings
 **/

typedef __start_packed struct
{
   MacAddr macAddr;
   char_t hostname[16];
   uint8_t enableDhcp;
   Ipv4Addr hostAddr;
   Ipv4Addr subnetMask;
   Ipv4Addr defaultGateway;
   Ipv4Addr primaryDns;
   Ipv4Addr secondaryDns;
} __end_packed LanSettings;


/**
 * @brief Proxy settings
 **/

typedef __start_packed struct
{
   uint8_t enable;
   char_t name[16];
   uint16_t port;
} __end_packed ProxySettings;


/**
 * @brief Settings
 **/

typedef __start_packed struct
{
   IcecastSettings icecast;
   LanSettings lan;
   ProxySettings proxy;
   uint32_t crc;
} __end_packed Settings;


//Global variables
extern Settings appSettings;

//Settings management
error_t getDefaultSettings(Settings *settings);
error_t loadSettings(Settings *settings);
error_t saveSettings(Settings *settings);

uint32_t calcCrc32(const void *data, size_t length);

#endif
