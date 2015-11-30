/**
 * @file slaac.c
 * @brief IPv6 Stateless Address Autoconfiguration
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
 * @section Description
 *
 * Stateless Address Autoconfiguration is a facility to allow devices to
 * configure themselves independently. Refer to the following RFCs for
 * complete details:
 * - RFC 4862: IPv6 Stateless Address Autoconfiguration
 * - RFC 6106: IPv6 Router Advertisement Options for DNS Configuration
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.6.4
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL SLAAC_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ethernet.h"
#include "ipv6/ndp.h"
#include "ipv6/slaac.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && SLAAC_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t slaacTickCounter;


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains SLAAC settings
 **/

void slaacGetDefaultSettings(SlaacSettings *settings)
{
   //Use default interface
   settings->interface = NULL;
   //Minimum delay before transmitting the first RS message
   settings->minRtrSolicitationDelay = 0;
   //Maximum delay before transmitting the first RS message
   settings->maxRtrSolicitationDelay = NDP_MAX_RTR_SOLICITATION_DELAY;
   //Time interval between retransmissions of RS messages
   settings->rtrSolicitationInterval = NDP_RTR_SOLICITATION_INTERVAL;
   //Number of retransmissions for RS messages
   settings->maxRtrSolicitations = NDP_MAX_RTR_SOLICITATIONS;
   //Maximum number of NS messages sent while performing DAD
   settings->dupAddrDetectTransmits = NDP_DUP_ADDR_DETECT_TRANSMITS;
   //Use the DNS servers specified by the RDNSS option
   settings->manualDnsConfig = FALSE;
   //Router Advertisement parsing callback
   settings->parseRouterAdvCallback = NULL;
}


/**
 * @brief SLAAC initialization
 * @param[in] context Pointer to the SLAAC context
 * @param[in] settings SLAAC specific settings
 * @return Error code
 **/

error_t slaacInit(SlaacContext *context, const SlaacSettings *settings)
{
   NetInterface *interface;

   //Debug message
   TRACE_INFO("Initializing SLAAC...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;
   //A valid pointer to the interface being configured is required
   if(settings->interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Point to the underlying network interface
   interface = settings->interface;

   //Clear the SLAAC context
   memset(context, 0, sizeof(SlaacContext));
   //Save user settings
   context->settings = *settings;

   //Initialize mutex object
   if(!osCreateMutex(&context->mutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //SLAAC operation is currently suspended
   context->running = FALSE;
   //Initialize state machine
   context->state = SLAAC_STATE_INIT;

   //Attach the SLAAC context to the network interface
   interface->slaacContext = context;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Start SLAAC process
 * @param[in] context Pointer to the SLAAC context
 * @return Error code
 **/

error_t slaacStart(SlaacContext *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting SLAAC...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Start SLAAC operation
   context->running = TRUE;
   //Initialize state machine
   context->state = SLAAC_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Stop SLAAC process
 * @param[in] context Pointer to the SLAAC context
 * @return Error code
 **/

error_t slaacStop(SlaacContext *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping SLAAC...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Suspend SLAAC operation
   context->running = FALSE;
   //Reinitialize state machine
   context->state = SLAAC_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve current state
 * @param[in] context Pointer to the SLAAC context
 * @return Current SLAAC state
 **/

SlaacState slaacGetState(SlaacContext *context)
{
   SlaacState state;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Get current state
   state = context->state;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Return current state
   return state;
}


/**
 * @brief SLAAC timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage SLAAC operation
 *
 * @param[in] context Pointer to the SLAAC context
 **/

void slaacTick(SlaacContext *context)
{
   systime_t time;
   NetInterface *interface;

   //Get current time
   time = osGetSystemTime();
   //Point to the underlying network interface
   interface = context->settings.interface;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check current state
   if(context->state == SLAAC_STATE_INIT)
   {
      //Wait for the link to be up before starting SLAAC
      if(context->running && interface->linkState)
      {
         Eui64 interfaceId;
         Ipv6Addr linkLocalAddr;

         //Generate the 64-bit interface identifier
         macAddrToEui64(&interface->macAddr, &interfaceId);

         //A link-local address is formed by combining the well-known
         //link-local prefix fe80::/10 with the interface identifier
         linkLocalAddr.w[0] = htons(0xFE80);
         linkLocalAddr.w[1] = htons(0x0000);
         linkLocalAddr.w[2] = htons(0x0000);
         linkLocalAddr.w[3] = htons(0x0000);
         linkLocalAddr.w[4] = interfaceId.w[0];
         linkLocalAddr.w[5] = interfaceId.w[1];
         linkLocalAddr.w[6] = interfaceId.w[2];
         linkLocalAddr.w[7] = interfaceId.w[3];

         //Use the link-local address as a tentative address
         ipv6SetLinkLocalAddrEx(interface, &linkLocalAddr,
               IPV6_ADDR_STATE_TENTATIVE);

         //Set time stamp
         context->timestamp = time;
         //Reset timeout value
         context->timeout = 0;
         //Reset retransmission counter
         context->retransmitCount = 0;

         //Verify the uniqueness of the link-local address
         context->state = SLAAC_STATE_LINK_LOCAL_ADDR_DAD;
      }
   }
   else if(context->state == SLAAC_STATE_LINK_LOCAL_ADDR_DAD)
   {
      //Check current time
      if(timeCompare(time, context->timestamp + context->timeout) >= 0)
      {
         //Duplicate Address Detection failed?
         if(interface->ipv6Config.linkLocalAddrDup)
         {
            //A tentative address that is determined to be a duplicate
            //must not be assigned to an interface
            ipv6SetLinkLocalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
               IPV6_ADDR_STATE_INVALID);

            //Address autoconfiguration failed
            context->state = SLAAC_STATE_DAD_FAILURE;

            //Dump current IPv6 configuration for debugging purpose
            slaacDumpConfig(context);
         }
         //Duplicate Address Detection is on-going?
         else if(context->retransmitCount < context->settings.dupAddrDetectTransmits)
         {
            //Send a multicast Neighbor Solicitation message
            ndpSendNeighborSol(interface, &interface->ipv6Config.linkLocalAddr, TRUE);

            //Save the time at which the message was sent
            context->timestamp = time;
            //Set timeout value
            context->timeout = interface->ipv6Config.retransTimer;
            //Increment retransmission counter
            context->retransmitCount++;
         }
         //Duplicate Address Detection is complete?
         else
         {
            //The use of the link-local address is now unrestricted
            interface->ipv6Config.linkLocalAddrState = IPV6_ADDR_STATE_PREFERRED;

            //Set time stamp
            context->timestamp = time;

            //Delay before transmitting the first Router Solicitation message
            context->timeout = netGetRandRange(context->settings.minRtrSolicitationDelay,
               context->settings.maxRtrSolicitationDelay);

            //Reset retransmission counter
            context->retransmitCount = 0;

            //To obtain an advertisement quickly, a host sends out
            //Router Solicitations
            context->state = SLAAC_STATE_ROUTER_SOLICIT;
         }
      }
   }
   else if(context->state == SLAAC_STATE_ROUTER_SOLICIT)
   {
      //Check current time
      if(timeCompare(time, context->timestamp + context->timeout) >= 0)
      {
         //Check retransmission counter
         if(context->retransmitCount < context->settings.maxRtrSolicitations)
         {
            //Send Router Solicitation message
            ndpSendRouterSol(interface);

            //Save the time at which the message was sent
            context->timestamp = time;
            //Set timeout value
            context->timeout = context->settings.rtrSolicitationInterval;
            //Increment retransmission counter
            context->retransmitCount++;
         }
         else
         {
            //A link has no routers if no Router Advertisements are received
            //after having sent a small number of Router Solicitations
            context->state = SLAAC_STATE_NO_ROUTER;

            //Dump current IPv6 configuration for debugging purpose
            slaacDumpConfig(context);
         }
      }
   }
   else if(context->state == SLAAC_STATE_GLOBAL_ADDR_DAD)
   {
      //Check current time
      if(timeCompare(time, context->timestamp + context->timeout) >= 0)
      {
         //Duplicate Address Detection failed?
         if(interface->ipv6Config.globalAddrDup)
         {
            //A tentative address that is determined to be a duplicate
            //must not be assigned to an interface
            ipv6SetGlobalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
               IPV6_ADDR_STATE_INVALID);

            //Address autoconfiguration failed
            context->state = SLAAC_STATE_DAD_FAILURE;

            //Dump current IPv6 configuration for debugging purpose
            slaacDumpConfig(context);
         }
         //Duplicate Address Detection is on-going?
         else if(context->retransmitCount < context->settings.dupAddrDetectTransmits)
         {
            //Send a multicast Neighbor Solicitation message
            ndpSendNeighborSol(interface, &interface->ipv6Config.globalAddr, TRUE);

            //Save the time at which the message was sent
            context->timestamp = time;
            //Set timeout value
            context->timeout = interface->ipv6Config.retransTimer;
            //Increment retransmission counter
            context->retransmitCount++;
         }
         //Duplicate Address Detection is complete?
         else
         {
            //The use of the global address is now unrestricted
            interface->ipv6Config.globalAddrState = IPV6_ADDR_STATE_PREFERRED;
            //Successful address autoconfiguration
            context->state = SLAAC_STATE_CONFIGURED;

            //Dump current IPv6 configuration for debugging purpose
            slaacDumpConfig(context);
         }
      }
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);
}


/**
 * @brief Callback function for link change event
 * @param[in] context Pointer to the SLAAC context
 **/

void slaacLinkChangeEvent(SlaacContext *context)
{
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check whether SLAAC is enabled
   if(context->running)
   {
      //The IPv6 link-local address is not longer valid
      ipv6SetLinkLocalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
         IPV6_ADDR_STATE_INVALID);

      //The IPv6 global address is no longer valid
      ipv6SetGlobalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
         IPV6_ADDR_STATE_INVALID);

      //Clear IPv6 prefix
      ipv6SetPrefix(interface, &IPV6_UNSPECIFIED_ADDR, 0);
   }

   //Reinitialize state machine
   context->state = SLAAC_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);
}


/**
 * @brief Router Advertisement message processing
 * @param[in] context Pointer to the SLAAC context
 * @param[in] message Pointer to the Router Advertisement message
 * @param[in] length Length of the message, in bytes
 **/

void slaacProcessRouterAdv(SlaacContext *context,
   NdpRouterAdvMessage *message, size_t length)
{
   uint_t i;
   uint_t n;
   NetInterface *interface;
   NdpPrefixInfoOption *prefixInfoOption;
   NdpRdnssOption *rdnssOption;
   Eui64 interfaceId;
   Ipv6Addr globalAddr;

   //Invoke callback function, if any
   if(context->settings.parseRouterAdvCallback != NULL)
      context->settings.parseRouterAdvCallback(message, length);

   //Check current state
   if(context->state != SLAAC_STATE_ROUTER_SOLICIT &&
      context->state != SLAAC_STATE_NO_ROUTER)
   {
      //Discard incoming Router Advertisement message
      return;
   }

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Calculate the length of the Options field
   length -= sizeof(NdpRouterAdvMessage);

   //Search for the Prefix Information option
   prefixInfoOption = ndpGetOption(message->options, length, NDP_OPT_PREFIX_INFORMATION);

   //Prefix Information option not found?
   if(prefixInfoOption == NULL || prefixInfoOption->length != 4)
      return;

   //If the Autonomous flag is not set, silently ignore the Prefix
   //Information option
   if(!prefixInfoOption->a)
      return;

   //If the prefix is the link-local prefix, silently ignore the
   //Prefix Information option
   if(ipv6CompPrefix(&prefixInfoOption->prefix, &IPV6_LINK_LOCAL_ADDR_PREFIX, 64))
      return;

   //Check whether the valid lifetime is zero
   if(!ntohl(prefixInfoOption->validLifetime))
      return;

   //If the preferred lifetime is greater than the valid lifetime,
   //silently ignore the Prefix Information option
   if(ntohl(prefixInfoOption->preferredLifetime) > ntohl(prefixInfoOption->validLifetime))
      return;

   //If the sum of the prefix length and interface identifier length does
   //not equal 128 bits, the Prefix Information option must be ignored
   if(prefixInfoOption->prefixLength != 64)
      return;

   //Save IPv6 prefix
   ipv6SetPrefix(interface, &prefixInfoOption->prefix,
      prefixInfoOption->prefixLength);

   //Generate the 64-bit interface identifier
   macAddrToEui64(&interface->macAddr, &interfaceId);

   //Form an address by combining the advertised prefix
   //with the interface identifier
   globalAddr.w[0] = prefixInfoOption->prefix.w[0];
   globalAddr.w[1] = prefixInfoOption->prefix.w[1];
   globalAddr.w[2] = prefixInfoOption->prefix.w[2];
   globalAddr.w[3] = prefixInfoOption->prefix.w[3];
   globalAddr.w[4] = interfaceId.w[0];
   globalAddr.w[5] = interfaceId.w[1];
   globalAddr.w[6] = interfaceId.w[2];
   globalAddr.w[7] = interfaceId.w[3];

   //Use the global address as a tentative address
   ipv6SetGlobalAddrEx(interface, &globalAddr, IPV6_ADDR_STATE_TENTATIVE);

   //Use the DNS servers provided by the router?
   if(!context->settings.manualDnsConfig)
   {
      //Search for the Recursive DNS Server (RDNSS) option
      rdnssOption = ndpGetOption(message->options, length,
         NDP_OPT_RECURSIVE_DNS_SERVER);

      //RDNSS option found?
      if(rdnssOption != NULL && rdnssOption->length >= 1)
      {
         //Retrieve the number of addresses
         n = (rdnssOption->length - 1) / 2;

         //Loop through the list of addresses
         for(i = 0; i < n; i++)
         {
            //Record DNS server address
            ipv6SetDnsServer(interface, i, &rdnssOption->address[i]);
         }
      }
   }

   //Set time stamp
   context->timestamp = osGetSystemTime();
   //Reset timeout value
   context->timeout = 0;
   //Reset retransmission counter
   context->retransmitCount = 0;

   //Verify the uniqueness of the global address
   context->state = SLAAC_STATE_GLOBAL_ADDR_DAD;
}


/**
 * @brief Dump SLAAC configuration for debugging purpose
 * @param[in] context Pointer to the SLAAC context
 **/

void slaacDumpConfig(SlaacContext *context)
{
   uint_t n;
   size_t mtu;
   Ipv6Addr ipv6Addr;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Debug message
   TRACE_INFO("\r\n");
   TRACE_INFO("SLAAC configuration:\r\n");

   //Link-local address
   ipv6GetLinkLocalAddr(interface, &ipv6Addr);
   TRACE_INFO("  Link-local Address = %s\r\n", ipv6AddrToString(&ipv6Addr, NULL));

   //IPv6 prefix
   ipv6GetPrefix(interface, &ipv6Addr, &n);
   TRACE_INFO("  Prefix = %s/%" PRIu8 "\r\n", ipv6AddrToString(&ipv6Addr, NULL), n);

   //Global address
   ipv6GetGlobalAddr(interface, &ipv6Addr);
   TRACE_INFO("  Global Address = %s\r\n", ipv6AddrToString(&ipv6Addr, NULL));

   //Router address
   ipv6GetRouter(interface, &ipv6Addr);
   TRACE_INFO("  Router = %s\r\n", ipv6AddrToString(&ipv6Addr, NULL));

   //DNS servers
   for(n = 0; n < IPV6_MAX_DNS_SERVERS; n++)
   {
      ipv6GetDnsServer(interface, n, &ipv6Addr);
      TRACE_INFO("  DNS Server %u = %s\r\n", n + 1, ipv6AddrToString(&ipv6Addr, NULL));
   }

   //Maximum transmit unit
   ipv6GetMtu(interface, &mtu);
   TRACE_INFO("  MTU = %" PRIuSIZE "\r\n", mtu);
   TRACE_INFO("\r\n");
}


/**
 * @brief Map a MAC address to the IPv6 modified EUI-64 identifier
 * @param[in] macAddr Host MAC address
 * @param[out] interfaceId IPv6 modified EUI-64 identifier
 **/

void macAddrToEui64(const MacAddr *macAddr, Eui64 *interfaceId)
{
   //Copy the Organization Unique Identifier (OUI)
   interfaceId->b[0] = macAddr->b[0];
   interfaceId->b[1] = macAddr->b[1];
   interfaceId->b[2] = macAddr->b[2];

   //The middle 16 bits are given the value 0xFFFE
   interfaceId->b[3] = 0xFF;
   interfaceId->b[4] = 0xFE;

   //Copy the right-most 24 bits of the MAC address
   interfaceId->b[5] = macAddr->b[3];
   interfaceId->b[6] = macAddr->b[4];
   interfaceId->b[7] = macAddr->b[5];

   //Modified EUI-64 format interface identifiers are
   //formed by inverting the Universal/Local bit
   interfaceId->b[0] ^= MAC_ADDR_FLAG_LOCAL;
}

#endif
