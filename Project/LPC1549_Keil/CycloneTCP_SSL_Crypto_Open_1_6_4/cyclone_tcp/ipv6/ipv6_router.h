/**
 * @file ipv6_router.h
 * @brief IPv6 router
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

#ifndef _IPV6_ROUTING_H
#define _IPV6_ROUTING_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//IPv6 router support
#ifndef IPV6_ROUTER_SUPPORT
   #define IPV6_ROUTER_SUPPORT DISABLED
#elif (IPV6_ROUTER_SUPPORT != ENABLED && IPV6_ROUTER_SUPPORT != DISABLED)
   #error IPV6_ROUTER_SUPPORT parameter is not valid
#endif

//IPv6 router tick interval
#ifndef IPV6_ROUTER_TICK_INTERVAL
   #define IPV6_ROUTER_TICK_INTERVAL 100
#elif (IPV6_ROUTER_TICK_INTERVAL < 100)
   #error IPV6_ROUTER_TICK_INTERVAL parameter is not valid
#endif

//Size of the IPv6 routing table
#ifndef IPV6_ROUTING_TABLE_SIZE
   #define IPV6_ROUTING_TABLE_SIZE 8
#elif (IPV6_ROUTING_TABLE_SIZE < 1)
   #error IPV6_ROUTING_TABLE_SIZE parameter is not valid
#endif


/**
 * @brief IPv6 prefix information
 **/

typedef struct
{
   Ipv6Addr prefix;
   uint8_t length;
   bool_t onLinkFlag;
   bool_t autonomousFlag;
   uint32_t validLifetime;
   uint32_t preferredLifetime;
} Ipv6PrefixInfo;


/**
 * @brief IPv6 prefix information
 **/

typedef struct
{
   uint8_t cid;
   Ipv6Addr prefix;
   uint8_t length;
   bool_t compression;
   uint16_t validLifetime;
} Ipv6ContextInfo;


/**
 * @brief IPv6 router settings
 **/

typedef struct
{
   NetInterface *interface;
   systime_t maxRtrAdvInterval;
   systime_t minRtrAdvInterval;
   uint8_t curHopLimit;
   bool_t managedFlag;
   bool_t otherConfigFlag;
   uint16_t defaultLifetime;
   uint32_t reachableTimer;
   uint32_t retransTimer;
   uint32_t linkMtu;
   Ipv6PrefixInfo *prefixList;
   uint_t prefixListLength;
   Ipv6ContextInfo *contextInfo;
   uint_t contextInfoLength;
} Ipv6RouterSettings;


/**
 * @brief IPv6 router context
 **/

typedef struct
{
   Ipv6RouterSettings settings; ///<IPv6 router settings
   OsMutex mutex;               ///<Mutex preventing simultaneous access to IPv6 router
   bool_t running;              ///<This flag tells whether the IPv6 router is running
   systime_t timestamp;         ///<Timestamp to manage retransmissions
   systime_t timeout;           ///<Timeout value
   uint_t routerAdvCount;       ///<Router Advertisement message counter
} Ipv6RouterContext;


/**
 * @brief Routing table entry
 **/

typedef struct
{
   Ipv6Addr prefix;         ///<Destination
   uint_t prefixLength;     ///<IPv6 prefix length
   NetInterface *interface; ///<Outgoing network interface
   Ipv6Addr nextHop;        ///<Next hop
} Ipv6RoutingTableEntry;


//Tick counter to handle periodic operations
extern systime_t ipv6RouterTickCounter;

//IPv6 router related functions
void ipv6RouterGetDefaultSettings(Ipv6RouterSettings *settings);
error_t ipv6RouterInit(Ipv6RouterContext *context, const Ipv6RouterSettings *settings);
error_t ipv6RouterStart(Ipv6RouterContext *context);
error_t ipv6RouterStop(Ipv6RouterContext *context);

void ipv6RouterTick(Ipv6RouterContext *context);
void ipv6RouterLinkChangeEvent(Ipv6RouterContext *context);

void ipv6ProcessRouterSol(NetInterface *interface, Ipv6PseudoHeader *pseudoHeader,
   const NetBuffer *buffer, size_t offset, uint8_t hopLimit);

error_t ipv6SendRouterAdv(Ipv6RouterContext *context, uint16_t routerLifetime);

error_t ipv6InitRoutingTable(void);
error_t ipv6ClearRoutingTable(void);

error_t ipv6AddRoute(const Ipv6Addr *prefix, uint_t prefixLength,
   NetInterface *interface, const Ipv6Addr *nextHop);

error_t ipv6ForwardPacket(NetInterface *srcInterface,
   NetBuffer *srcBuffer, size_t srcOffset);

#endif
