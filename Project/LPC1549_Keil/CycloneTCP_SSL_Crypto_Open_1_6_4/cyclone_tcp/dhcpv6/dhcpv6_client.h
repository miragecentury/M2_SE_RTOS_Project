/**
 * @file dhcpv6_client.h
 * @brief DHCPv6 client (Dynamic Host Configuration Protocol for IPv6)
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

#ifndef _DHCPV6_CLIENT_H
#define _DHCPV6_CLIENT_H

//Dependencies
#include "dhcpv6/dhcpv6_common.h"
#include "core/socket.h"

//DHCPv6 client support
#ifndef DHCPV6_CLIENT_SUPPORT
   #define DHCPV6_CLIENT_SUPPORT DISABLED
#elif (DHCPV6_CLIENT_SUPPORT != ENABLED && DHCPV6_CLIENT_SUPPORT != DISABLED)
   #error DHCPV6_CLIENT_SUPPORT parameter is not valid
#endif

//DHCPv6 client tick interval
#ifndef DHCPV6_CLIENT_TICK_INTERVAL
   #define DHCPV6_CLIENT_TICK_INTERVAL 200
#elif (DHCPV6_CLIENT_TICK_INTERVAL < 100)
   #error DHCPV6_CLIENT_TICK_INTERVAL parameter is not valid
#endif

//Maximum size of the client's FQDN
#ifndef DHCPV6_CLIENT_MAX_FQDN_SIZE
   #define DHCPV6_CLIENT_MAX_FQDN_SIZE 16
#elif (DHCPV6_CLIENT_MAX_FQDN_SIZE < 1)
   #error DHCPV6_CLIENT_MAX_FQDN_SIZE parameter is not valid
#endif

//Max delay of first Solicit
#ifndef DHCPV6_CLIENT_SOL_MAX_DELAY
   #define DHCPV6_CLIENT_SOL_MAX_DELAY 1000
#elif (DHCPV6_CLIENT_SOL_MAX_DELAY < 1000)
   #error DHCPV6_CLIENT_SOL_MAX_DELAY parameter is not valid
#endif

//Initial Solicit timeout
#ifndef DHCPV6_CLIENT_SOL_TIMEOUT
   #define DHCPV6_CLIENT_SOL_TIMEOUT 1000
#elif (DHCPV6_CLIENT_SOL_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_SOL_TIMEOUT parameter is not valid
#endif

//Max Solicit timeout value
#ifndef DHCPV6_CLIENT_SOL_MAX_RT
   #define DHCPV6_CLIENT_SOL_MAX_RT 120000
#elif (DHCPV6_CLIENT_SOL_MAX_RT < 1000)
   #error DHCPV6_CLIENT_SOL_MAX_RT parameter is not valid
#endif

//Initial Request timeout
#ifndef DHCPV6_CLIENT_REQ_TIMEOUT
   #define DHCPV6_CLIENT_REQ_TIMEOUT 1000
#elif (DHCPV6_CLIENT_REQ_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_REQ_TIMEOUT parameter is not valid
#endif

//Max Request timeout value
#ifndef DHCPV6_CLIENT_REQ_MAX_RT
   #define DHCPV6_CLIENT_REQ_MAX_RT 30000
#elif (DHCPV6_CLIENT_REQ_MAX_RT < 1000)
   #error DHCPV6_CLIENT_REQ_MAX_RT parameter is not valid
#endif

//Max Request retry attempts
#ifndef DHCPV6_CLIENT_REQ_MAX_RC
   #define DHCPV6_CLIENT_REQ_MAX_RC 10
#elif (DHCPV6_CLIENT_REQ_MAX_RC < 1)
   #error DHCPV6_CLIENT_REQ_MAX_RC parameter is not valid
#endif

//Max delay of first Confirm
#ifndef DHCPV6_CLIENT_CNF_MAX_DELAY
   #define DHCPV6_CLIENT_CNF_MAX_DELAY 1000
#elif (DHCPV6_CLIENT_CNF_MAX_DELAY < 1000)
   #error DHCPV6_CLIENT_CNF_MAX_DELAY parameter is not valid
#endif

//Initial Confirm timeout
#ifndef DHCPV6_CLIENT_CNF_TIMEOUT
   #define DHCPV6_CLIENT_CNF_TIMEOUT 1000
#elif (DHCPV6_CLIENT_CNF_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_CNF_TIMEOUT parameter is not valid
#endif

//Max Confirm timeout
#ifndef DHCPV6_CLIENT_CNF_MAX_RT
   #define DHCPV6_CLIENT_CNF_MAX_RT 4000
#elif (DHCPV6_CLIENT_CNF_MAX_RT < 1000)
   #error DHCPV6_CLIENT_CNF_MAX_RT parameter is not valid
#endif

//Max Confirm duration
#ifndef DHCPV6_CLIENT_CNF_MAX_RD
   #define DHCPV6_CLIENT_CNF_MAX_RD 10000
#elif (DHCPV6_CLIENT_CNF_MAX_RD < 1000)
   #error DHCPV6_CLIENT_CNF_MAX_RD parameter is not valid
#endif

//Initial Renew timeout
#ifndef DHCPV6_CLIENT_REN_TIMEOUT
   #define DHCPV6_CLIENT_REN_TIMEOUT 10000
#elif (DHCPV6_CLIENT_REN_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_REN_TIMEOUT parameter is not valid
#endif

//Max Renew timeout value
#ifndef DHCPV6_CLIENT_REN_MAX_RT
   #define DHCPV6_CLIENT_REN_MAX_RT 600000
#elif (DHCPV6_CLIENT_REN_MAX_RT < 1000)
   #error DHCPV6_CLIENT_REN_MAX_RT parameter is not valid
#endif

//Initial Rebind timeout
#ifndef DHCPV6_CLIENT_REB_TIMEOUT
   #define DHCPV6_CLIENT_REB_TIMEOUT 10000
#elif (DHCPV6_CLIENT_REB_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_REB_TIMEOUT parameter is not valid
#endif

//Max Rebind timeout value
#ifndef DHCPV6_CLIENT_REB_MAX_RT
   #define DHCPV6_CLIENT_REB_MAX_RT 600000
#elif (DHCPV6_CLIENT_REB_MAX_RT < 1000)
   #error DHCPV6_CLIENT_REB_MAX_RT parameter is not valid
#endif

//Max delay of first Information-request
#ifndef DHCPV6_CLIENT_INF_MAX_DELAY
   #define DHCPV6_CLIENT_INF_MAX_DELAY 1000
#elif (DHCPV6_CLIENT_INF_MAX_DELAY < 1000)
   #error DHCPV6_CLIENT_INF_MAX_DELAY parameter is not valid
#endif

//Initial Information-request timeout
#ifndef DHCPV6_CLIENT_INF_TIMEOUT
   #define DHCPV6_CLIENT_INF_TIMEOUT 1000
#elif (DHCPV6_CLIENT_INF_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_INF_TIMEOUT parameter is not valid
#endif

//Max Information-request timeout value
#ifndef DHCPV6_CLIENT_INF_MAX_RT
   #define DHCPV6_CLIENT_INF_MAX_RT 120000
#elif (DHCPV6_CLIENT_INF_MAX_RT < 1000)
   #error DHCPV6_CLIENT_INF_MAX_RT parameter is not valid
#endif

//Initial Release timeout
#ifndef DHCPV6_CLIENT_REL_TIMEOUT
   #define DHCPV6_CLIENT_REL_TIMEOUT 1000
#elif (DHCPV6_CLIENT_REL_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_REL_TIMEOUT parameter is not valid
#endif

//Max Release attempts
#ifndef DHCPV6_CLIENT_REL_MAX_RC
   #define DHCPV6_CLIENT_REL_MAX_RC 5
#elif (DHCPV6_CLIENT_REL_MAX_RC < 1)
   #error DHCPV6_CLIENT_REL_MAX_RC parameter is not valid
#endif

//Initial Decline timeout
#ifndef DHCPV6_CLIENT_DEC_TIMEOUT
   #define DHCPV6_CLIENT_DEC_TIMEOUT 1000
#elif (DHCPV6_CLIENT_DEC_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_DEC_TIMEOUT parameter is not valid
#endif

//Max Decline attempts
#ifndef DHCPV6_CLIENT_DEC_MAX_RC
   #define DHCPV6_CLIENT_DEC_MAX_RC 5
#elif (DHCPV6_CLIENT_DEC_MAX_RC < 1)
   #error DHCPV6_CLIENT_DEC_MAX_RC parameter is not valid
#endif

//Initial Reconfigure timeout
#ifndef DHCPV6_CLIENT_REC_TIMEOUT
   #define DHCPV6_CLIENT_REC_TIMEOUT 2000
#elif (DHCPV6_CLIENT_REC_TIMEOUT < 1000)
   #error DHCPV6_CLIENT_REC_TIMEOUT parameter is not valid
#endif

//Max Reconfigure attempts
#ifndef DHCPV6_CLIENT_REC_MAX_RC
   #define DHCPV6_CLIENT_REC_MAX_RC 8
#elif (DHCPV6_CLIENT_REC_MAX_RC < 1)
   #error DHCPV6_CLIENT_REC_MAX_RC parameter is not valid
#endif

//Forward declaration of Dhcpv6ClientCtx structure
struct _Dhcpv6ClientCtx;
#define Dhcpv6ClientCtx struct _Dhcpv6ClientCtx


/**
 * @brief DHCPv6 client FSM states
 **/

typedef enum
{
   DHCPV6_STATE_INIT         = 0,
   DHCPV6_STATE_SOLICIT      = 1,
   DHCPV6_STATE_REQUEST      = 2,
   DHCPV6_STATE_INIT_CONFIRM = 3,
   DHCPV6_STATE_CONFIRM      = 4,
   DHCPV6_STATE_BOUND        = 5,
   DHCPV6_STATE_RENEW        = 6,
   DHCPV6_STATE_REBIND       = 7,
   DHCPV6_STATE_DECLINE      = 8
} Dhcpv6State;


/**
 * @brief DHCPv6 configuration timeout callback
 **/

typedef void (*Dhcpv6TimeoutCallback)(Dhcpv6ClientCtx *context,
   NetInterface *interface);


/**
 * @brief Link state change callback
 **/

typedef void (*Dhcpv6LinkChangeCallback)(Dhcpv6ClientCtx *context,
   NetInterface *interface, bool_t linkState);


/**
 * @brief FSM state change callback
 **/

typedef void (*Dhcpv6StateChangeCallback)(Dhcpv6ClientCtx *context,
   NetInterface *interface, Dhcpv6State state);


/**
 * @brief DHCPv6 client settings
 **/

typedef struct
{
   NetInterface *interface;                    ///<Network interface to configure
   bool_t rapidCommit;                         ///<Quick configuration using rapid commit
   bool_t manualDnsConfig;                     ///<Force manual DNS configuration
   systime_t timeout;                          ///<DHCPv6 configuration timeout
   Dhcpv6TimeoutCallback timeoutEvent;         ///<DHCPv6 configuration timeout event
   Dhcpv6LinkChangeCallback linkChangeEvent;   ///<Link state change event
   Dhcpv6StateChangeCallback stateChangeEvent; ///<FSM state change event
} Dhcpv6ClientSettings;


/**
 * @brief DHCPv6 client context
 **/

struct _Dhcpv6ClientCtx
{
   Dhcpv6ClientSettings settings;                   ///<DHCPv6 client settings
   OsMutex mutex;                                   ///<Mutex preventing simultaneous access to DHCPv6 client
   bool_t running;                                  ///<This flag tells whether the DHCP client is running or not
   Dhcpv6State state;                               ///<Current state of the FSM
   bool_t timeoutEventDone;                         ///<Timeout callback function has been called
   systime_t timestamp;                             ///<Timestamp to manage retransmissions
   systime_t timeout;                               ///<Timeout value
   uint_t retransmitCount;                          ///<Retransmission counter
   uint8_t clientId[DHCPV6_MAX_DUID_SIZE];          ///<Client DUID
   size_t clientIdLength;                           ///<Length of the client DUID
   uint8_t clientFqdn[DHCPV6_CLIENT_MAX_FQDN_SIZE]; ///<Client's fully qualified domain name
   size_t clientFqdnLength;                         ///<Length of the client's FQDN
   Ipv6Addr clientAddr;                             ///<IPv6 address assigned by to the client
   uint8_t serverId[DHCPV6_MAX_DUID_SIZE];          ///<Server DUID
   size_t serverIdLength;                           ///<Length of the server DUID
   int_t serverPreference;                          ///<Preference value for the server
   uint32_t transactionId;                          ///<Value to match requests with replies
   systime_t configStartTime;                       ///<Address acquisition or renewal process start time
   systime_t exchangeStartTime;                     ///<Time at which the client sent the first message
   systime_t leaseStartTime;                        ///<Lease start time
   uint32_t t1;                                     ///<T1 parameter
   uint32_t t2;                                     ///<T2 parameter
   uint32_t preferredLifetime;                      ///<Preferred lifetime
   uint32_t validLifetime;                          ///<Valid lifetime
};


//Tick counter to handle periodic operations
extern systime_t dhcpv6ClientTickCounter;

//DHCPv6 client related functions
void dhcpv6ClientGetDefaultSettings(Dhcpv6ClientSettings *settings);
error_t dhcpv6ClientInit(Dhcpv6ClientCtx *context, const Dhcpv6ClientSettings *settings);
error_t dhcpv6ClientStart(Dhcpv6ClientCtx *context);
error_t dhcpv6ClientStop(Dhcpv6ClientCtx *context);
Dhcpv6State dhcpv6ClientGetState(Dhcpv6ClientCtx *context);

void dhcpv6ClientTick(Dhcpv6ClientCtx *context);
void dhcpv6ClientLinkChangeEvent(Dhcpv6ClientCtx *context);

void dhcpv6ClientProcessMessage(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NetBuffer *buffer, size_t offset, void *params);

void dhcpv6StateInit(Dhcpv6ClientCtx *context);
void dhcpv6StateSolicit(Dhcpv6ClientCtx *context);
void dhcpv6StateRequest(Dhcpv6ClientCtx *context);
void dhcpv6StateInitConfirm(Dhcpv6ClientCtx *context);
void dhcpv6StateConfirm(Dhcpv6ClientCtx *context);
void dhcpv6StateBound(Dhcpv6ClientCtx *context);
void dhcpv6StateRenew(Dhcpv6ClientCtx *context);
void dhcpv6StateRebind(Dhcpv6ClientCtx *context);
void dhcpv6StateDecline(Dhcpv6ClientCtx *context);

error_t dhcpv6SendSolicit(Dhcpv6ClientCtx *context);
error_t dhcpv6SendRequest(Dhcpv6ClientCtx *context);
error_t dhcpv6SendConfirm(Dhcpv6ClientCtx *context);
error_t dhcpv6SendRenew(Dhcpv6ClientCtx *context);
error_t dhcpv6SendRebind(Dhcpv6ClientCtx *context);
error_t dhcpv6SendDecline(Dhcpv6ClientCtx *context);

error_t dhcpv6ParseAdvertise(Dhcpv6ClientCtx *context,
   const Dhcpv6Message *message, size_t length);

error_t dhcpv6ParseReply(Dhcpv6ClientCtx *context,
   const Dhcpv6Message *message, size_t length);

error_t dhcpv6ParseIaNaOption(Dhcpv6ClientCtx *context,
   const Dhcpv6Option *option);

void dhcpv6ChangeState(Dhcpv6ClientCtx *context,
   Dhcpv6State newState, systime_t delay);

void dhcpv6CheckTimeout(Dhcpv6ClientCtx *context);

uint16_t dhcpv6ComputeElapsedTime(Dhcpv6ClientCtx *context);

int32_t dhcpv6Rand(int32_t value);
int32_t dhcpv6RandRange(int32_t min, int32_t max);

void dhcpv6DumpConfig(Dhcpv6ClientCtx *context);

#endif
