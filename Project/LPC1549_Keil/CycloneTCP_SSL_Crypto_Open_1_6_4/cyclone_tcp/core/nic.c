/**
 * @file nic.c
 * @brief Network interface controller abstraction layer
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
#include "core/net.h"
#include "core/nic.h"
#include "ipv4/arp.h"
#include "ipv4/igmp.h"
#include "ipv6/mld.h"
#include "core/socket.h"
#include "core/raw_socket.h"
#include "core/tcp_misc.h"
#include "core/udp.h"
#include "dhcp/dhcp_client.h"
#include "dns/dns_cache.h"
#include "dns/dns_client.h"
#include "mdns/mdns_client.h"
#include "mdns/mdns_responder.h"
#include "snmp/mib2_module.h"
#include "snmp/mib2_impl.h"
#include "debug.h"

//Tick counter to handle periodic operations
systime_t nicTickCounter;


/**
 * @brief Network controller timer handler
 *
 * This routine is periodically called by the TCP/IP stack to
 * handle periodic operations such as polling the link state
 *
 * @param[in] interface Underlying network interface
 **/

void nicTick(NetInterface *interface)
{
   //Get exclusive access to the device
   osAcquireMutex(&interface->nicDriverMutex);
   //Disable interrupts
   interface->nicDriver->disableIrq(interface);

   //Handle periodic operations
   interface->nicDriver->tick(interface);

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Release exclusive access to the device
   osReleaseMutex(&interface->nicDriverMutex);
}


/**
 * @brief Configure multicast MAC address filtering
 * @param[in] interface Underlying network interface
 * @return Error code
 **/

error_t nicSetMacFilter(NetInterface *interface)
{
   error_t error;

   //Get exclusive access to the device
   osAcquireMutex(&interface->nicDriverMutex);
   //Disable interrupts
   interface->nicDriver->disableIrq(interface);

   //Update MAC filter table
   error = interface->nicDriver->setMacFilter(interface);

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Release exclusive access to the device
   osReleaseMutex(&interface->nicDriverMutex);

   //Return status code
   return error;
}


/**
 * @brief Send a packet to the network controller
 * @param[in] interface Underlying network interface
 * @param[in] buffer Multi-part buffer containing the data to send
 * @param[in] offset Offset to the first data byte
 * @return Error code
 **/

error_t nicSendPacket(NetInterface *interface, const NetBuffer *buffer, size_t offset)
{
   error_t error;
   bool_t status;

#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   //Retrieve the length of the packet
   size_t length = netBufferGetLength(buffer) - offset;

   //Debug message
   TRACE_DEBUG("Sending packet (%" PRIuSIZE " bytes)...\r\n", length);
   TRACE_DEBUG_NET_BUFFER("  ", buffer, offset, length);
#endif

   //Wait for the transmitter to be ready to send
   status = osWaitForEvent(&interface->nicTxEvent, INFINITE_DELAY);

   //Check whether the specified event is in signaled state
   if(status)
   {
      //Get exclusive access to the device
      osAcquireMutex(&interface->nicDriverMutex);
      //Disable interrupts
      interface->nicDriver->disableIrq(interface);

      //Send Ethernet frame
      error = interface->nicDriver->sendPacket(interface, buffer, offset);

      //Re-enable interrupts if necessary
      if(interface->configured)
         interface->nicDriver->enableIrq(interface);

      //Release exclusive access to the device
      osReleaseMutex(&interface->nicDriverMutex);
   }
   else
   {
      //The transmitter is busy...
      return ERROR_FAILURE;
   }

   //Return status code
   return error;
}


/**
 * @brief Handle a packet received by the network controller
 * @param[in] interface Underlying network interface
 * @param[in] packet Incoming packet to process
 * @param[in] length Total packet length
 **/

void nicProcessPacket(NetInterface *interface, void *packet, size_t length)
{
   NicType type;

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Release exclusive access to the device
   osReleaseMutex(&interface->nicDriverMutex);

   //Debug message
   TRACE_DEBUG("Packet received (%" PRIuSIZE " bytes)...\r\n", length);
   TRACE_DEBUG_ARRAY("  ", packet, length);

   //Retrieve network interface type
   type = interface->nicDriver->type;

   //Ethernet interface?
   if(type == NIC_TYPE_ETHERNET)
   {
#if (ETH_SUPPORT == ENABLED)
      //Process incoming Ethernet frame
      ethProcessFrame(interface, packet, length);
#endif
   }
   //PPP interface?
   else if(type == NIC_TYPE_PPP)
   {
#if (PPP_SUPPORT == ENABLED)
      //Process incoming PPP frame
      pppProcessFrame(interface, packet, length);
#endif
   }
   //6LoWPAN interface?
   else if(type == NIC_TYPE_6LOWPAN)
   {
#if (IPV6_SUPPORT == ENABLED)
      NetBuffer1 buffer;

      //The incoming packet fits in a single chunk
      buffer.chunkCount = 1;
      buffer.maxChunkCount = 1;
      buffer.chunk[0].address = packet;
      buffer.chunk[0].length = length;
      buffer.chunk[0].size = 0;

      //Process incoming IPv6 packet
      ipv6ProcessPacket(interface, (NetBuffer *) &buffer);
#endif
   }

   //Get exclusive access to the device
   osAcquireMutex(&interface->nicDriverMutex);
   //Disable interrupts
   interface->nicDriver->disableIrq(interface);
}


/**
 * @brief Process link state change event
 * @param[in] interface Underlying network interface
 **/

void nicNotifyLinkChange(NetInterface *interface)
{
   uint_t i;
   Socket *socket;
#if (MIB2_SUPPORT == ENABLED)
   systime_t time;
#endif

   //Re-enable interrupts if necessary
   if(interface->configured)
      interface->nicDriver->enableIrq(interface);

   //Release exclusive access to the device
   osReleaseMutex(&interface->nicDriverMutex);

#if (IPV4_SUPPORT == ENABLED)
   //Restore default MTU
   interface->ipv4Config.mtu = interface->nicDriver->mtu;
#endif

#if (IPV4_SUPPORT == ENABLED && ETH_SUPPORT == ENABLED)
   //Flush ARP cache contents
   arpFlushCache(interface);
#endif

#if (IPV4_SUPPORT == ENABLED && IPV4_FRAG_SUPPORT == ENABLED)
   //Flush the reassembly queue
   ipv4FlushFragQueue(interface);
#endif

#if (IPV4_SUPPORT == ENABLED && IGMP_SUPPORT == ENABLED)
   //Notify IGMP of link state changes
   igmpLinkChangeEvent(interface);
#endif

#if (IPV4_SUPPORT == ENABLED && AUTO_IP_SUPPORT == ENABLED)
   //Auto-IP is currently used?
   if(interface->autoIpContext != NULL)
   {
      //Notify Auto-IP of link state changes
      autoIpLinkChangeEvent(interface->autoIpContext);
   }
#endif

#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)
   //DHCP client is currently used?
   if(interface->dhcpClientContext != NULL)
   {
      //Notify the DHCP client of link state changes
      dhcpClientLinkChangeEvent(interface->dhcpClientContext);
   }
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Restore default IPv6 parameters
   interface->ipv6Config.mtu = interface->nicDriver->mtu;
   interface->ipv6Config.curHopLimit = IPV6_DEFAULT_HOP_LIMIT;
   interface->ipv6Config.reachableTime = NDP_REACHABLE_TIME;
   interface->ipv6Config.retransTimer = NDP_RETRANS_TIMER;
#endif

#if (IPV6_SUPPORT == ENABLED && NDP_SUPPORT == ENABLED)
   //Flush Neighbor cache contents
   ndpFlushCache(interface);
#endif

#if (IPV6_SUPPORT == ENABLED && IPV6_FRAG_SUPPORT == ENABLED)
   //Flush the reassembly queue
   ipv6FlushFragQueue(interface);
#endif

#if (IPV6_SUPPORT == ENABLED && MLD_SUPPORT == ENABLED)
   //Notify MLD of link state changes
   mldLinkChangeEvent(interface);
#endif

#if (IPV6_SUPPORT == ENABLED && SLAAC_SUPPORT == ENABLED)
   //Stateless Address Autoconfiguration is currently used?
   if(interface->slaacContext != NULL)
   {
      //Notify SLAAC of link state changes
      slaacLinkChangeEvent(interface->slaacContext);
   }
#endif

#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)
   //DHCPv6 client is currently used?
   if(interface->dhcpv6ClientContext != NULL)
   {
      //Notify the DHCPv6 client of link state changes
      dhcpv6ClientLinkChangeEvent(interface->dhcpv6ClientContext);
   }
#endif

#if (IPV6_SUPPORT == ENABLED && IPV6_ROUTER_SUPPORT == ENABLED)
   //IPv6 router is currently used?
   if(interface->ipv6RouterContext != NULL)
   {
      //Notify the IPv6 router of link state changes
      ipv6RouterLinkChangeEvent(interface->ipv6RouterContext);
   }
#endif

#if (DNS_CLIENT_SUPPORT == ENABLED || MDNS_CLIENT_SUPPORT == ENABLED || \
   NBNS_CLIENT_SUPPORT == ENABLED)
   //Flush DNS cache
   dnsFlushCache(interface);
#endif

#if (MDNS_RESPONDER_SUPPORT == ENABLED)
   //Whenever a mDNS responder receives an indication of a link
   //change event, it must perform probing and announcing
   mdnsLinkChangeEvent(interface);
#endif

#if (MIB2_SUPPORT == ENABLED)
   //Get current time
   time = osGetSystemTime();

   //Enter critical section
   MIB2_LOCK();

   //Interface's current bandwidth
   if(interface->speed100)
      MIB2_SET_GAUGE32(interface->mibIfEntry->ifSpeed, 100000000);
   else
      MIB2_SET_GAUGE32(interface->mibIfEntry->ifSpeed, 10000000);

   //The current operational state of the interface
   if(interface->linkState)
      MIB2_SET_INTEGER(interface->mibIfEntry->ifOperStatus, MIB2_IF_OPER_STATUS_UP);
   else
      MIB2_SET_INTEGER(interface->mibIfEntry->ifOperStatus, MIB2_IF_OPER_STATUS_DOWN);

   //The time at which the interface entered its current operational state
   MIB2_SET_TIME_TICKS(interface->mibIfEntry->ifLastChange, time / 10);

   //Leave critical section
   MIB2_UNLOCK();
#endif

   //Notify registered users of link state changes
   netInvokeLinkChangeCallback(interface, interface->linkState);

   //Acquire exclusive access to sockets
   osAcquireMutex(&socketMutex);

   //Loop through opened sockets
   for(i = 0; i < SOCKET_MAX_COUNT; i++)
   {
      //Point to the current socket
      socket = socketTable + i;

#if (TCP_SUPPORT == ENABLED)
      //Connection-oriented socket?
      if(socket->type == SOCKET_TYPE_STREAM)
      {
         tcpUpdateEvents(socket);
      }
#endif
#if (UDP_SUPPORT == ENABLED)
      //Connectionless socket?
      if(socket->type == SOCKET_TYPE_DGRAM)
      {
         udpUpdateEvents(socket);
      }
#endif
#if (RAW_SOCKET_SUPPORT == ENABLED)
      //Raw socket?
      if(socket->type == SOCKET_TYPE_RAW_IP ||
         socket->type == SOCKET_TYPE_RAW_ETH)
      {
         rawSocketUpdateEvents(socket);
      }
#endif
   }

   //Release exclusive access to sockets
   osReleaseMutex(&socketMutex);

   //Get exclusive access to the device
   osAcquireMutex(&interface->nicDriverMutex);
   //Disable interrupts
   interface->nicDriver->disableIrq(interface);
}
