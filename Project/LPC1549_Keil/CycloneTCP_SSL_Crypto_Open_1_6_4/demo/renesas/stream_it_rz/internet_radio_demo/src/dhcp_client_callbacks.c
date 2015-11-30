/**
 * @file dhcp_client_callbacks.c
 * @brief DHCP client custom callbacks
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
#include <strings.h>
#include "core/net.h"
#include "dhcp/dhcp_client.h"
#include "settings.h"
#include "debug.h"


/**
 * @brief DHCP configuration timeout callback
 * @param[in] context Pointer to the DHCP client context
 * @param[in] interface interface Underlying network interface
 **/

void dhcpClientTimeoutCallback(DhcpClientCtx *context,
   NetInterface *interface)
{
   uint_t i;
   Ipv4Addr ipv4Addr;

   //Stop DHCP client
   dhcpClientStop(context);

   //Set IPv4 host address
   ipv4SetHostAddr(interface, appSettings.lan.hostAddr);

   //Set subnet mask
   ipv4SetSubnetMask(interface, appSettings.lan.subnetMask);

   //Set default gateway
   ipv4SetDefaultGateway(interface, appSettings.lan.defaultGateway);

   //Set primary and secondary DNS servers
   ipv4SetDnsServer(interface, 0, appSettings.lan.primaryDns);
   ipv4SetDnsServer(interface, 1, appSettings.lan.secondaryDns);

   //Debug message
   TRACE_INFO("\r\nStatic configuration:\r\n");

   //Host address
   ipv4GetHostAddr(interface, &ipv4Addr);
   TRACE_INFO("  IPv4 Address = %s\r\n", ipv4AddrToString(ipv4Addr, NULL));

   //Subnet mask
   ipv4GetSubnetMask(interface, &ipv4Addr);
   TRACE_INFO("  Subnet Mask = %s\r\n", ipv4AddrToString(ipv4Addr, NULL));

   //Default gateway
   ipv4GetDefaultGateway(interface, &ipv4Addr);
   TRACE_INFO("  Default Gateway = %s\r\n", ipv4AddrToString(ipv4Addr, NULL));

   //DNS servers
   for(i = 0; i < IPV4_MAX_DNS_SERVERS; i++)
   {
      ipv4GetDnsServer(interface, i, &ipv4Addr);
      TRACE_INFO("  DNS Server %u = %s\r\n", i + 1, ipv4AddrToString(ipv4Addr, NULL));
   }
}


/**
 * @brief Link state change callback
 * @param[in] context Pointer to the DHCP client context
 * @param[in] interface interface Underlying network interface
 * @param[in] linkState Current link state
 **/

void dhcpClientLinkChangeCallback(DhcpClientCtx *context,
   NetInterface *interface, bool_t linkState)
{
   //Check whether the link is up
   if(linkState)
   {
      //Start DHCP client
      dhcpClientStart(context);
   }
}
