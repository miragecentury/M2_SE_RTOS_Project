/**
 * @file dhcpv6_client.c
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
 * @section Description
 *
 * The Dynamic Host Configuration Protocol for IPv6 enables DHCP servers to
 * pass configuration parameters such as IPv6 network addresses to IPv6
 * nodes. This protocol is a stateful counterpart to IPv6 Stateless Address
 * Autoconfiguration (RFC 2462), and can be used separately or concurrently
 * with the latter to obtain configuration parameters. Refer to RFC 3315
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.6.4
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DHCPV6_TRACE_LEVEL

//Dependencies
#include <stdlib.h>
#include "core/net.h"
#include "dhcpv6/dhcpv6_client.h"
#include "dhcpv6/dhcpv6_common.h"
#include "dhcpv6/dhcpv6_debug.h"
#include "dns/dns_common.h"
#include "date_time.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && DHCPV6_CLIENT_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t dhcpv6ClientTickCounter;

//Requested DHCPv6 options
static const uint16_t dhcpv6OptionList[] =
{
   HTONS(DHCPV6_OPTION_DNS_SERVERS),
   HTONS(DHCPV6_OPTION_DOMAIN_LIST),
   HTONS(DHCPV6_OPTION_FQDN)
};


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains DHCPv6 client settings
 **/

void dhcpv6ClientGetDefaultSettings(Dhcpv6ClientSettings *settings)
{
   //Use default interface
   settings->interface = NULL;
   //No rapid commit
   settings->rapidCommit = FALSE;
   //Use the DNS servers provided by the DHCPv6 server
   settings->manualDnsConfig = FALSE;
   //DHCPv6 configuration timeout
   settings->timeout = 0;
   //DHCPv6 configuration timeout event
   settings->timeoutEvent = NULL;
   //Link state change event
   settings->linkChangeEvent = NULL;
   //FSM state change event
   settings->stateChangeEvent = NULL;
}


/**
 * @brief DHCPv6 client initialization
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] settings DHCPv6 client specific settings
 * @return Error code
 **/

error_t dhcpv6ClientInit(Dhcpv6ClientCtx *context, const Dhcpv6ClientSettings *settings)
{
   error_t error;
   Dhcpv6DuidLl *duid;
   Dhcpv6FqdnOption *fqdnOption;
   NetInterface *interface;

   //Debug message
   TRACE_INFO("Initializing DHCPv6 client...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;
   //A valid pointer to the interface being configured is required
   if(settings->interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Point to the underlying network interface
   interface = settings->interface;

   //Clear the DHCPv6 client context
   memset(context, 0, sizeof(Dhcpv6ClientCtx));
   //Save user settings
   context->settings = *settings;

   //Point to the client DUID
   duid = (Dhcpv6DuidLl *) context->clientId;

   //Generate a DUID-LL
   duid->type = HTONS(DHCPV6_DUID_LL);
   duid->hardwareType = HTONS(DHCPV6_HARDWARE_TYPE_ETH);
   duid->linkLayerAddr = interface->macAddr;

   //Length of the newly generated DUID
   context->clientIdLength = sizeof(Dhcpv6DuidLl);

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;
   //Set flags
   fqdnOption->mbz = 0;
   fqdnOption->n = FALSE;
   fqdnOption->o = FALSE;
   fqdnOption->s = FALSE;

   //Encode client's FQDN
   context->clientFqdnLength = dnsEncodeName(interface->hostname,
      fqdnOption->domainName);

   //Initialize mutex object
   if(!osCreateMutex(&context->mutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Callback function to be called when a DHCPv6 message is received
   error = udpAttachRxCallback(interface, DHCPV6_CLIENT_PORT,
      dhcpv6ClientProcessMessage, context);

   //Failed to register callback function?
   if(error)
   {
      //Clean up side effects
      osDeleteMutex(&context->mutex);
      //Report an error
      return error;
   }

   //DHCPv6 client is currently suspended
   context->running = FALSE;
   //Initialize state machine
   context->state = DHCPV6_STATE_INIT;

   //Attach the DHCPv6 client context to the network interface
   interface->dhcpv6ClientContext = context;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Start DHCPv6 client
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6ClientStart(Dhcpv6ClientCtx *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting DHCPv6 client...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Start DHCPv6 client
   context->running = TRUE;
   //Initialize state machine
   context->state = DHCPV6_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Stop DHCPv6 client
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6ClientStop(Dhcpv6ClientCtx *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping DHCPv6 client...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Stop DHCPv6 client
   context->running = FALSE;
   //Reinitialize state machine
   context->state = DHCPV6_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve current state
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Current DHCPv6 client state
 **/

Dhcpv6State dhcpv6ClientGetState(Dhcpv6ClientCtx *context)
{
   Dhcpv6State state;

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
 * @brief DHCPv6 client timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage DHCPv6 client operation
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/


void dhcpv6ClientTick(Dhcpv6ClientCtx *context)
{
   //Enter critical section
   osAcquireMutex(&context->mutex);

   //DHCPv6 client finite state machine
   switch(context->state)
   {
   //Process INIT state
   case DHCPV6_STATE_INIT:
      //This is the initialization state, where a client begins the process of
      //acquiring a lease. It also returns here when a lease ends, or when a
      //lease negotiation fails
      dhcpv6StateInit(context);
      break;
   //Process SOLICIT state
   case DHCPV6_STATE_SOLICIT:
      //The client sends a Solicit message to locate servers
      dhcpv6StateSolicit(context);
      break;
   //Process REQUEST state
   case DHCPV6_STATE_REQUEST:
      //The client sends a Request message to request configuration
      //parameters, including IP addresses, from a specific server
      dhcpv6StateRequest(context);
      break;
   //Process INIT-CONFIRM state
   case DHCPV6_STATE_INIT_CONFIRM:
      //When a client that already has a valid lease starts up after a
      //power-down or reboot, it starts here instead of the INIT state
      dhcpv6StateInitConfirm(context);
      break;
   //Process CONFIRM state
   case DHCPV6_STATE_CONFIRM:
      //The client sends a Confirm message to any available server
      //to determine whether the addresses it was assigned are still
      //appropriate to the link to which the client is connected
      dhcpv6StateConfirm(context);
      break;
   //Process BOUND state
   case DHCPV6_STATE_BOUND:
      //The client has a valid lease and is in its normal operating state
      dhcpv6StateBound(context);
      break;
   //Process RENEW state
   case DHCPV6_STATE_RENEW:
      //The client sends a Renew message to the server that originally
      //provided the client's addresses and configuration parameters to
      //extend the lifetimes on the addresses assigned to the client
      //and to update other configuration parameters
      dhcpv6StateRenew(context);
      break;
   //Process REBIND state
   case DHCPV6_STATE_REBIND:
      //The client sends a Rebind message to any available server to extend
      //the lifetimes on the addresses assigned to the client and to update
      //other configuration parameters. This message is sent after a client
      //receives no response to a Renew message
      dhcpv6StateRebind(context);
      break;
   //Invalid state...
   default:
      //Switch to the default state
      context->state = DHCPV6_STATE_INIT;
      break;
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);
}


/**
 * @brief Callback function for link change event
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6ClientLinkChangeEvent(Dhcpv6ClientCtx *context)
{
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check whether the DHCPv6 client is running
   if(context->running)
   {
      //The global address is no longer valid
      ipv6SetGlobalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
         IPV6_ADDR_STATE_INVALID);
   }

   //Check whether the client already has a valid lease
   if(context->state >= DHCPV6_STATE_INIT_CONFIRM)
   {
      //Switch to the INIT_CONFIRM state
      context->state = DHCPV6_STATE_INIT_CONFIRM;
   }
   else
   {
      //Switch to the INIT state
      context->state = DHCPV6_STATE_INIT;
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Invoke user-defined callback, if any
   if(context->settings.linkChangeEvent != NULL)
      context->settings.linkChangeEvent(context, interface, interface->linkState);
}


/**
 * @brief Process incoming DHCPv6 message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader UDP pseudo header
 * @param[in] udpHeader UDP header
 * @param[in] buffer Multi-part buffer containing the incoming DHCPv6 message
 * @param[in] offset Offset to the first byte of the DHCPv6 message
 * @param[in] params Pointer to the DHCPv6 client context
 **/

void dhcpv6ClientProcessMessage(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NetBuffer *buffer, size_t offset, void *params)
{
   size_t length;
   Dhcpv6ClientCtx *context;
   Dhcpv6Message *message;

   //Retrieve the length of the DHCPv6 message
   length = netBufferGetLength(buffer) - offset;

   //Make sure the DHCPv6 message is valid
   if(length < sizeof(Dhcpv6Message))
      return;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);
   //Sanity check
   if(!message) return;

   //Debug message
   TRACE_DEBUG("\r\n%s: DHCPv6 message received (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Point to the DHCPv6 client context
   context = (Dhcpv6ClientCtx *) params;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check current state
   switch(context->state)
   {
   //SOLICIT state?
   case DHCPV6_STATE_SOLICIT:
      //Parse Advertise message
      dhcpv6ParseAdvertise(context, message, length);
      break;
   //REQUEST, CONFIRM, RENEW, REBIND or DECLINE state?
   case DHCPV6_STATE_REQUEST:
   case DHCPV6_STATE_CONFIRM:
   case DHCPV6_STATE_RENEW:
   case DHCPV6_STATE_REBIND:
   case DHCPV6_STATE_DECLINE:
      //Parse Reply message
      dhcpv6ParseReply(context, message, length);
      break;
   //Any other state?
   default:
      //Drop incoming message
      break;
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);
}


/**
 * @brief INIT state
 *
 * This is the initialization state, where a client begins the process of
 * acquiring a lease. It also returns here when a lease ends, or when a
 * lease negotiation fails
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateInit(Dhcpv6ClientCtx *context)
{
   systime_t delay;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Check whether the DHCPv6 client is running
   if(context->running)
   {
      //Wait for the link to be up before starting DHCPv6 configuration
      if(interface->linkState)
      {
         //The first Solicit message from the client on the interface must be
         //delayed by a random amount of time between 0 and SOL_MAX_DELAY
         delay = dhcpv6RandRange(0, DHCPV6_CLIENT_SOL_MAX_DELAY);

         //Record the time at which the client started
         //the address acquisition process
         context->configStartTime = osGetSystemTime();
         //Clear flag
         context->timeoutEventDone = FALSE;

         //Switch to the SOLICIT state
         dhcpv6ChangeState(context, DHCPV6_STATE_SOLICIT, delay);
      }
   }
}


/**
 * @brief SOLICIT state
 *
 * A client uses the Solicit message to discover DHCPv6 servers
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateSolicit(Dhcpv6ClientCtx *context)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Send a Router Solicitation message
         ndpSendRouterSol(context->settings.interface);

         //Reset server preference value
         context->serverPreference = -1;
         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Solicit message
         dhcpv6SendSolicit(context);

         //Save the time at which the message was sent
         context->timestamp = time;
         //Initial retransmission timeout
         context->timeout = DHCPV6_CLIENT_SOL_TIMEOUT;

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //Check whether a valid Advertise message has been received
         if(context->serverPreference >= 0)
         {
            //Continue configuration procedure
            dhcpv6ChangeState(context, DHCPV6_STATE_REQUEST, 0);
         }
         else
         {
            //Send a Solicit message
            dhcpv6SendSolicit(context);

            //Save the time at which the message was sent
            context->timestamp = time;

            //The RT is doubled for each subsequent retransmission
            context->timeout *= 2;
            //MRT specifies an upper bound on the value of RT
            context->timeout = MIN(context->timeout, DHCPV6_CLIENT_SOL_MAX_RT);

            //Each of the computations of a new RT include a randomization factor,
            //which is a random number chosen with a uniform distribution between
            //-0.1 and +0.1
            context->timeout += dhcpv6Rand(context->timeout);

            //Increment retransmission counter
            context->retransmitCount++;
         }
      }
   }

   //Manage DHCPv6 configuration timeout
   dhcpv6CheckTimeout(context);
}


/**
 * @brief REQUEST state
 *
 * The client uses a Request message to populate IAs with addresses and obtain
 * other configuration information. The client includes one or more more IA
 * options in the Request message. The server then returns addresses and other
 * information about the IAs to the client in IA options in a Reply message
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateRequest(Dhcpv6ClientCtx *context)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Request message
         dhcpv6SendRequest(context);

         //Save the time at which the message was sent
         context->timestamp = time;
         //Initial timeout value
         context->timeout = DHCPV6_CLIENT_REQ_TIMEOUT;

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else if(context->retransmitCount < DHCPV6_CLIENT_REQ_MAX_RC)
      {
         //Send a Request message
         dhcpv6SendRequest(context);

         //Save the time at which the message was sent
         context->timestamp = time;

         //The RT is doubled for each subsequent retransmission
         context->timeout *= 2;
         //MRT specifies an upper bound on the value of RT
         context->timeout = MIN(context->timeout, DHCPV6_CLIENT_REQ_MAX_RT);

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //If the client does not receive a response within a reasonable
         //period of time, then it restarts the initialization procedure
         dhcpv6ChangeState(context, DHCPV6_STATE_INIT, 0);
      }
   }

   //Manage DHCPv6 configuration timeout
   dhcpv6CheckTimeout(context);
}


/**
 * @brief INIT-CONFIRM state
 *
 * When a client that already has a valid lease starts up after a
 * power-down or reboot, it starts here instead of the INIT state
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateInitConfirm(Dhcpv6ClientCtx *context)
{
   systime_t delay;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Check whether the DHCPv6 client is running
   if(context->running)
   {
      //Wait for the link to be up before starting DHCPv6 configuration
      if(interface->linkState)
      {
         //The first Confirm message from the client on the interface must be
         //delayed by a random amount of time between 0 and CNF_MAX_DELAY
         delay = dhcpv6RandRange(0, DHCPV6_CLIENT_CNF_MAX_DELAY);

         //Record the time at which the client started
         //the address acquisition process
         context->configStartTime = osGetSystemTime();
         //Clear flag
         context->timeoutEventDone = FALSE;

         //Switch to the CONFIRM state
         dhcpv6ChangeState(context, DHCPV6_STATE_CONFIRM, delay);
      }
   }
}


/**
 * @brief CONFIRM state
 *
 * Whenever a client may have moved to a new link, the prefixes from
 * the addresses assigned to the interfaces on that link may no longer
 * be appropriate for the link to which the client is attached. In such
 * the client must initiate a Confirm/Reply message exchange
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateConfirm(Dhcpv6ClientCtx *context)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Send a Router Solicitation message
         ndpSendRouterSol(context->settings.interface);

         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Confirm message
         dhcpv6SendConfirm(context);

         //Save the time at which the client sent the first message
         context->exchangeStartTime = time;
         context->timestamp = time;

         //Initial timeout value
         context->timeout = DHCPV6_CLIENT_CNF_TIMEOUT;

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //Send a Confirm message
         dhcpv6SendConfirm(context);

         //Save the time at which the message was sent
         context->timestamp = time;

         //The RT is doubled for each subsequent retransmission
         context->timeout *= 2;
         //MRT specifies an upper bound on the value of RT
         context->timeout = MIN(context->timeout, DHCPV6_CLIENT_CNF_MAX_RT);

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
   }
   else
   {
      //Check retransmission counter
      if(context->retransmitCount > 0)
      {
         //The message exchange fails once MRD seconds have elapsed since
         //the client first transmitted the message
         if(timeCompare(time, context->exchangeStartTime + DHCPV6_CLIENT_CNF_MAX_RD) >= 0)
         {
            //Restart the initialization procedure
            dhcpv6ChangeState(context, DHCPV6_STATE_INIT, 0);
         }
      }
   }

   //Manage DHCPv6 configuration timeout
   dhcpv6CheckTimeout(context);
}


/**
 * @brief BOUND state
 *
 * Client has a valid lease and is in its normal operating state
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateBound(Dhcpv6ClientCtx *context)
{
   systime_t t1;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //A client will never attempt to extend the lifetime of any
   //address in an IA with T1 set to 0xFFFFFFFF
   if(context->t1 != DHCPV6_INFINITE_TIME)
   {
      //Convert T1 to milliseconds
      t1 = context->t1 * 1000;

      //Check the time elapsed since the lease was obtained
      if(timeCompare(time, context->leaseStartTime + t1) >= 0)
      {
         //Record the time at which the client started the address renewal process
         context->configStartTime = time;

         //Enter the RENEW state
         dhcpv6ChangeState(context, DHCPV6_STATE_RENEW, 0);
      }
   }
}


/**
 * @brief RENEW state
 *
 * The client sends a Renew message to the server that originally
 * provided the client's addresses and configuration parameters to
 * extend the lifetimes on the addresses assigned to the client
 * and to update other configuration parameters
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateRenew(Dhcpv6ClientCtx *context)
{
   systime_t t2;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Renew message
         dhcpv6SendRenew(context);

         //Initial retransmission timeout
         context->timeout = DHCPV6_CLIENT_REN_TIMEOUT;
      }
      else
      {
         //Send a Renew message
         dhcpv6SendRenew(context);

         //The RT is doubled for each subsequent retransmission
         context->timeout *= 2;
         //MRT specifies an upper bound on the value of RT
         context->timeout = MIN(context->timeout, DHCPV6_CLIENT_REN_MAX_RT);
      }

      //Save the time at which the message was sent
      context->timestamp = time;

      //Each of the computations of a new RT include a randomization factor,
      //which is a random number chosen with a uniform distribution between
      //-0.1 and +0.1
      context->timeout += dhcpv6Rand(context->timeout);

      //Increment retransmission counter
      context->retransmitCount++;
   }
   else
   {
      //A client will never attempt to use a Rebind message to locate a
      //different server to extend the lifetime of any address in an IA
      //with T2 set to 0xFFFFFFFF
      if(context->t2 != DHCPV6_INFINITE_TIME)
      {
         //Convert T2 to milliseconds
         t2 = context->t2 * 1000;

         //Check whether T2 timer has expired
         if(timeCompare(time, context->leaseStartTime + t2) >= 0)
         {
            //Switch to the REBIND state
            dhcpv6ChangeState(context, DHCPV6_STATE_REBIND, 0);
         }
      }
   }
}


/**
 * @brief REBIND state
 *
 * The client sends a Rebind message to any available server to extend
 * the lifetimes on the addresses assigned to the client and to update
 * other configuration parameters. This message is sent after a client
 * receives no response to a Renew message
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateRebind(Dhcpv6ClientCtx *context)
{
   systime_t time;
   systime_t validLifetime;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Rebind message
         dhcpv6SendRebind(context);

         //Initial retransmission timeout
         context->timeout = DHCPV6_CLIENT_REB_TIMEOUT;
      }
      else
      {
         //Send a Rebind message
         dhcpv6SendRebind(context);

         //The RT is doubled for each subsequent retransmission
         context->timeout *= 2;
         //MRT specifies an upper bound on the value of RT
         context->timeout = MIN(context->timeout, DHCPV6_CLIENT_REB_MAX_RT);
      }

      //Save the time at which the message was sent
      context->timestamp = time;

      //Each of the computations of a new RT include a randomization factor,
      //which is a random number chosen with a uniform distribution between
      //-0.1 and +0.1
      context->timeout += dhcpv6Rand(context->timeout);

      //Increment retransmission counter
      context->retransmitCount++;
   }
   else
   {
      //Setting the valid lifetime of an address to 0xFFFFFFFF amounts
      //to a permanent assignment of an address to the client
      if(context->validLifetime != DHCPV6_INFINITE_TIME)
      {
         //Convert the valid lifetime to milliseconds
         validLifetime = context->validLifetime * 1000;

         //Check whether the valid lifetime has expired
         if(timeCompare(time, context->leaseStartTime + validLifetime) >= 0)
         {
            //The global address is no longer valid
            ipv6SetGlobalAddrEx(interface, &IPV6_UNSPECIFIED_ADDR,
               IPV6_ADDR_STATE_INVALID);

            //Restart DHCPv6 configuration
            dhcpv6ChangeState(context, DHCPV6_STATE_INIT, 0);
         }
      }
   }
}


/**
 * @brief DECLINE state
 *
 * If a client detects that one or more addresses assigned to it by a
 * server are already in use by another node, the client sends a Decline
 * message to the server to inform it that the address is suspect
 *
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6StateDecline(Dhcpv6ClientCtx *context)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Check retransmission counter
      if(context->retransmitCount == 0)
      {
         //Generate a 24-bit transaction ID
         context->transactionId = netGetRand() & 0x00FFFFFF;

         //Send a Decline message
         dhcpv6SendDecline(context);

         //Save the time at which the message was sent
         context->timestamp = time;
         //Initial timeout value
         context->timeout = DHCPV6_CLIENT_DEC_TIMEOUT;

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else if(context->retransmitCount < DHCPV6_CLIENT_DEC_MAX_RC)
      {
         //Send a Decline message
         dhcpv6SendDecline(context);

         //Save the time at which the message was sent
         context->timestamp = time;
         //The RT is doubled for each subsequent retransmission
         context->timeout *= 2;

         //Each of the computations of a new RT include a randomization factor,
         //which is a random number chosen with a uniform distribution between
         //-0.1 and +0.1
         context->timeout += dhcpv6Rand(context->timeout);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //If the client does not receive a response within a reasonable
         //period of time, then it restarts the initialization procedure
         dhcpv6ChangeState(context, DHCPV6_STATE_INIT, 0);
      }
   }
}


/**
 * @brief Send Solicit message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendSolicit(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   Dhcpv6FqdnOption *fqdnOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Solicit message
   message->msgType = DHCPV6_MSG_TYPE_SOLICIT;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Solicit message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //Prepare an IA_NA option for a the current interface
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes IA options for any IAs to which
   //it wants the server to assign addresses
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //The client should include an Option Request option to indicate
   //the options the client is interested in receiving
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ORO,
      &dhcpv6OptionList, sizeof(dhcpv6OptionList));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Check whether rapid commit is enabled
   if(context->settings.rapidCommit)
   {
      //Include the Rapid Commit option if the client is prepared
      //to perform the Solicit/Reply message exchange
      dhcpv6AddOption(message, &length, DHCPV6_OPTION_RAPID_COMMIT, NULL, 0);
   }

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;

   //The FQDN option can be used by the client to convey its
   //fully qualified domain name to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_FQDN,
      fqdnOption, sizeof(Dhcpv6FqdnOption) + context->clientFqdnLength);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send Request message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendRequest(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   Dhcpv6FqdnOption *fqdnOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Request message
   message->msgType = DHCPV6_MSG_TYPE_REQUEST;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Request message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //The client places the identifier of the destination
   //server in a Server Identifier option
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_SERVERID,
      context->serverId, context->serverIdLength);

   //Prepare an IA_NA option
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes IA options for any IAs to which
   //it wants the server to assign addresses
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //The client must include an Option Request option to indicate
   //the options the client is interested in receiving
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ORO,
      &dhcpv6OptionList, sizeof(dhcpv6OptionList));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;

   //The FQDN option can be used by the client to convey its
   //fully qualified domain name to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_FQDN,
      fqdnOption, sizeof(Dhcpv6FqdnOption) + context->clientFqdnLength);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send Confirm message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendConfirm(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6Option *option;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6IaAddrOption iaAddrOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   Dhcpv6FqdnOption *fqdnOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Confirm message
   message->msgType = DHCPV6_MSG_TYPE_CONFIRM;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Confirm message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //Prepare an IA_NA option
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes any IAs assigned to the interface
   //that may have moved to a new link
   option = dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //Prepare an IA Address option
   iaAddrOption.address = context->clientAddr;
   iaAddrOption.preferredLifetime = 0;
   iaAddrOption.validLifetime = 0;

   //Include the address currently assigned to the IA
   dhcpv6AddSubOption(option, &length, DHCPV6_OPTION_IAADDR,
      &iaAddrOption, sizeof(iaAddrOption));

   //The client must include an Option Request option to indicate
   //the options the client is interested in receiving
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ORO,
      &dhcpv6OptionList, sizeof(dhcpv6OptionList));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;

   //The FQDN option can be used by the client to convey its
   //fully qualified domain name to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_FQDN,
      fqdnOption, sizeof(Dhcpv6FqdnOption) + context->clientFqdnLength);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send Renew message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendRenew(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6Option *option;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6IaAddrOption iaAddrOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   Dhcpv6FqdnOption *fqdnOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Renew message
   message->msgType = DHCPV6_MSG_TYPE_RENEW;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Renew message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //The client places the identifier of the destination
   //server in a Server Identifier option
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_SERVERID,
      context->serverId, context->serverIdLength);

   //Prepare an IA_NA option
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes an IA option with all addresses
   //currently assigned to the IA in its Renew message
   option = dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //Prepare an IA Address option
   iaAddrOption.address = interface->ipv6Config.globalAddr;
   iaAddrOption.preferredLifetime = 0;
   iaAddrOption.validLifetime = 0;

   //Include the address currently assigned to the IA
   dhcpv6AddSubOption(option, &length, DHCPV6_OPTION_IAADDR,
      &iaAddrOption, sizeof(iaAddrOption));

   //The client must include an Option Request option to indicate
   //the options the client is interested in receiving
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ORO,
      &dhcpv6OptionList, sizeof(dhcpv6OptionList));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;

   //The FQDN option can be used by the client to convey its
   //fully qualified domain name to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_FQDN,
      fqdnOption, sizeof(Dhcpv6FqdnOption) + context->clientFqdnLength);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send Rebind message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendRebind(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6Option *option;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6IaAddrOption iaAddrOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   Dhcpv6FqdnOption *fqdnOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Rebind message
   message->msgType = DHCPV6_MSG_TYPE_REBIND;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Rebind message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //Prepare an IA_NA option
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes an IA option with all addresses
   //currently assigned to the IA in its Rebind message
   option = dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //Prepare an IA Address option
   iaAddrOption.address = interface->ipv6Config.globalAddr;
   iaAddrOption.preferredLifetime = 0;
   iaAddrOption.validLifetime = 0;

   //Include the address currently assigned to the IA
   dhcpv6AddSubOption(option, &length, DHCPV6_OPTION_IAADDR,
      &iaAddrOption, sizeof(iaAddrOption));

   //The client must include an Option Request option to indicate
   //the options the client is interested in receiving
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ORO,
      &dhcpv6OptionList, sizeof(dhcpv6OptionList));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Point to the client's fully qualified domain name
   fqdnOption = (Dhcpv6FqdnOption *) context->clientFqdn;

   //The FQDN option can be used by the client to convey its
   //fully qualified domain name to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_FQDN,
      fqdnOption, sizeof(Dhcpv6FqdnOption) + context->clientFqdnLength);

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send Decline message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return Error code
 **/

error_t dhcpv6SendDecline(Dhcpv6ClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   NetInterface *interface;
   Dhcpv6Message *message;
   Dhcpv6Option *option;
   Dhcpv6IaNaOption iaNaOption;
   Dhcpv6IaAddrOption iaAddrOption;
   Dhcpv6ElapsedTimeOption elapsedTimeOption;
   IpAddr destIpAddr;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCPv6 message
   buffer = udpAllocBuffer(DHCPV6_MAX_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCPv6 message
   message = netBufferAt(buffer, offset);

   //Format the Decline message
   message->msgType = DHCPV6_MSG_TYPE_DECLINE;
   //The transaction ID is chosen by the client
   STORE24BE(context->transactionId, message->transactionId);
   //Size of the Decline message
   length = sizeof(Dhcpv6Message);

   //The client must include a Client Identifier option
   //to identify itself to the server
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_CLIENTID,
      context->clientId, context->clientIdLength);

   //The client places the identifier of the server that
   //allocated the address in a Server Identifier option
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_SERVERID,
      context->serverId, context->serverIdLength);

   //Prepare an IA_NA option
   iaNaOption.iaId = htonl(interface->id);
   iaNaOption.t1 = 0;
   iaNaOption.t2 = 0;

   //The client includes an IA option with the address it is declining
   option = dhcpv6AddOption(message, &length, DHCPV6_OPTION_IA_NA,
      &iaNaOption, sizeof(Dhcpv6IaNaOption));

   //Prepare an IA Address option
   iaAddrOption.address = interface->ipv6Config.globalAddr;
   iaAddrOption.preferredLifetime = 0;
   iaAddrOption.validLifetime = 0;

   //Include the address the client is declining
   dhcpv6AddSubOption(option, &length, DHCPV6_OPTION_IAADDR,
      &iaAddrOption, sizeof(iaAddrOption));

   //Compute the time elapsed since the client sent the first message
   elapsedTimeOption.value = dhcpv6ComputeElapsedTime(context);

   //The client must include an Elapsed Time option in messages to indicate
   //how long the client has been trying to complete a DHCP message exchange
   dhcpv6AddOption(message, &length, DHCPV6_OPTION_ELAPSED_TIME,
      &elapsedTimeOption, sizeof(Dhcpv6ElapsedTimeOption));

   //Adjust the length of the multi-part buffer
   netBufferSetLength(buffer, offset + length);

   //Destination address
   destIpAddr.length = sizeof(Ipv6Addr);
   destIpAddr.ipv6Addr = DHCPV6_ALL_RELAY_AGENTS_AND_SERVERS_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCPv6 message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpv6DumpMessage(message, length);

   //Send DHCPv6 message
   error = udpSendDatagramEx(interface, DHCPV6_CLIENT_PORT,
      &destIpAddr, DHCPV6_SERVER_PORT, buffer, offset, 0);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Parse Advertise message
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] message Pointer to the incoming message to parse
 * @param[in] length Length of the incoming message
 * @return Error code
 **/

error_t dhcpv6ParseAdvertise(Dhcpv6ClientCtx *context,
   const Dhcpv6Message *message, size_t length)
{
   error_t error;
   int_t serverPreference;
   Dhcpv6Option *option;
   Dhcpv6Option *serverIdOption;

   //Check whether rapid commit is enabled
   if(context->settings.rapidCommit)
   {
      //Accept a Reply message with committed address assignments
      //and other resources in response to the Solicit message
      error = dhcpv6ParseReply(context, message, length);

      //The client terminates the waiting process as soon as a Reply
      //message with a Rapid Commit option is received
      if(!error) return NO_ERROR;
   }

   //Check the length of the DHCPv6 message
   if(length < sizeof(Dhcpv6Message))
      return ERROR_INVALID_MESSAGE;
   //Check the message type
   if(message->msgType != DHCPV6_MSG_TYPE_ADVERTISE)
      return ERROR_INVALID_MESSAGE;
   //Discard any received packet that does not match the transaction ID
   if(LOAD24BE(message->transactionId) != context->transactionId)
      return ERROR_INVALID_MESSAGE;

   //Get the length of the Options field
   length -= sizeof(Dhcpv6Message);

   //Search for the Client Identifier option
   option = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_CLIENTID);

   //Discard any received packet that does not include a Client Identifier option
   if(!option || ntohs(option->length) != context->clientIdLength)
      return ERROR_INVALID_MESSAGE;
   //Check whether the Client Identifier matches our identifier
   if(memcmp(option->value, context->clientId, context->clientIdLength))
      return ERROR_INVALID_MESSAGE;

   //Search for the Server Identifier option
   serverIdOption = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_SERVERID);

   //Discard any received packet that does not include a Server Identifier option
   if(!serverIdOption || !serverIdOption->length)
      return ERROR_INVALID_MESSAGE;
   //Check the length of the server DUID
   if(ntohs(serverIdOption->length) >= DHCPV6_MAX_DUID_SIZE)
      return ERROR_INVALID_MESSAGE;

   //Get the status code returned by the server
   error = dhcpv6ParseStatusCodeOption(message->options, length);
   //The client must ignore any Advertise message that includes
   //a Status Code option containing the value NoAddrsAvail
   if(error) return ERROR_INVALID_MESSAGE;

   //Search for the Preference option
   option = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_PREFERENCE);

   //Check whether the option has been found
   if(option && ntohs(option->length) == sizeof(Dhcpv6PreferenceOption))
   {
      //Server server preference value
      serverPreference = option->value[0];
   }
   else
   {
      //Any Advertise that does not include a Preference option
      //is considered to have a preference value of 0
      serverPreference = 0;
   }

   //Select the Advertise message that offers the highest server preference value
   if(serverPreference > context->serverPreference)
   {
      //Record the server preference value
      context->serverPreference = serverPreference;
      //Save the length of the DUID
      context->serverIdLength = ntohs(serverIdOption->length);
      //Record the server DUID
      memcpy(context->serverId, serverIdOption->value, context->serverIdLength);
   }

   //If the client receives an Advertise message that includes a
   //Preference option with a preference value of 255, the client
   //immediately completes the message exchange
   if(serverPreference == DHCPV6_MAX_SERVER_PREFERENCE)
   {
      //Continue configuration procedure
      dhcpv6ChangeState(context, DHCPV6_STATE_REQUEST, 0);
   }
   //The message exchange is not terminated before the first RT has elapsed
   else if(context->retransmitCount > 1)
   {
      //Continue configuration procedure
      dhcpv6ChangeState(context, DHCPV6_STATE_REQUEST, 0);
   }

   //The Advertise message was successfully parsed
   return NO_ERROR;
}


/**
 * @brief Parse Reply message
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] message Pointer to the incoming message to parse
 * @param[in] length Length of the incoming message
 * @return Error code
 **/

error_t dhcpv6ParseReply(Dhcpv6ClientCtx *context,
   const Dhcpv6Message *message, size_t length)
{
   error_t error;
   uint_t i;
   uint_t j;
   uint_t n;
   Dhcpv6Option *option;
   Dhcpv6Option *serverIdOption;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Check the length of the DHCPv6 message
   if(length < sizeof(Dhcpv6Message))
      return ERROR_INVALID_MESSAGE;
   //Check the message type
   if(message->msgType != DHCPV6_MSG_TYPE_REPLY)
      return ERROR_INVALID_MESSAGE;
   //Discard any received packet that does not match the transaction ID
   if(LOAD24BE(message->transactionId) != context->transactionId)
      return ERROR_INVALID_MESSAGE;

   //Get the length of the Options field
   length -= sizeof(Dhcpv6Message);

   //Search for the Client Identifier option
   option = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_CLIENTID);

   //Discard any received packet that does not include a Client Identifier option
   if(!option || ntohs(option->length) != context->clientIdLength)
      return ERROR_INVALID_MESSAGE;
   //Check whether the Client Identifier matches our identifier
   if(memcmp(option->value, context->clientId, context->clientIdLength))
      return ERROR_INVALID_MESSAGE;

   //Search for the Server Identifier option
   serverIdOption = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_SERVERID);

   //Discard any received packet that does not include a Server Identifier option
   if(!serverIdOption || !serverIdOption->length)
      return ERROR_INVALID_MESSAGE;
   //Check the length of the server DUID
   if(ntohs(serverIdOption->length) >= DHCPV6_MAX_DUID_SIZE)
      return ERROR_INVALID_MESSAGE;

   //The Reply message is received in response to a Solicit message?
   if(context->state == DHCPV6_STATE_SOLICIT)
   {
      //A Reply message is not acceptable when rapid commit is disallowed
      if(!context->settings.rapidCommit)
         return ERROR_INVALID_MESSAGE;

      //Search for the Rapid Commit option
      option = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_RAPID_COMMIT);

      //The client discards any message that does not include a Rapid Commit option
      if(!option || ntohs(option->length) != 0)
         return ERROR_INVALID_MESSAGE;

      //Save the length of the DUID
      context->serverIdLength = ntohs(serverIdOption->length);
      //Record the server DUID
      memcpy(context->serverId, serverIdOption->value, context->serverIdLength);
   }
   //The Reply message is received in response to a Request or a Renew message?
   else if(context->state == DHCPV6_STATE_REQUEST || context->state == DHCPV6_STATE_RENEW)
   {
      //Compare DUID lengths
      if(ntohs(serverIdOption->length) != context->serverIdLength)
         return ERROR_INVALID_MESSAGE;
      //Unexpected server DUID?
      if(memcmp(serverIdOption->value, context->serverId, context->serverIdLength))
         return ERROR_INVALID_MESSAGE;
   }
   //The Reply message is received in response to a Confirm or a Rebind message?
   else
   {
      //Do not check the server DUID when the Reply message is
      //received in response to a Confirm or a Rebind message
   }

   //Search for the Status Code option
   error = dhcpv6ParseStatusCodeOption(message->options, length);
   //Check the status code returned by the server
   if(error) return error;

   //Loop through DHCPv6 options
   for(i = 0; i < length; i += sizeof(Dhcpv6Option) + ntohs(option->length))
   {
      //Search for an IA_NA option
      option = dhcpv6GetOption(message->options + i, length - i, DHCPV6_OPTION_IA_NA);
      //Unable to find the specified option?
      if(!option) break;

      //Parse the contents of the IA_NA option
      error = dhcpv6ParseIaNaOption(context, option);

      //If an invalid option is received, the client discards
      //the option and process the rest of the message...
      if(!error)
      {
         //Save the length of the DUID
         context->serverIdLength = ntohs(serverIdOption->length);
         //Record the server DUID
         memcpy(context->serverId, serverIdOption->value, context->serverIdLength);

         //Search for DNS Servers option
         option = dhcpv6GetOption(message->options, length, DHCPV6_OPTION_DNS_SERVERS);

         //Check whether the message includes a DNS Servers option
         if(option && !(ntohs(option->length) % sizeof(Ipv6Addr)))
         {
            //Get the number of addresses provided in the response
            n = ntohs(option->length) / sizeof(Ipv6Addr);
            //Only a limited set of DNS servers is supported
            n = MIN(n, IPV6_MAX_DNS_SERVERS);

            //Loop through the list of addresses
            for(j = 0; j < n; j++)
            {
               //Record DNS server address
               ipv6CopyAddr(&interface->ipv6Config.dnsServer[j],
                  option->value + j * sizeof(Ipv6Addr));
            }
         }

         //Assign the IPv6 address to the interface
         ipv6SetGlobalAddrEx(interface, &context->clientAddr, IPV6_ADDR_STATE_VALID);

         //Save the time a which the lease was obtained
         context->leaseStartTime = osGetSystemTime();
         //Dump current DHCPv6 configuration for debugging purpose
         dhcpv6DumpConfig(context);
         //The client transitions to the BOUND state
         dhcpv6ChangeState(context, DHCPV6_STATE_BOUND, 0);

         //The Reply message was successfully parsed
         return NO_ERROR;
      }
   }

   //The Reply message contains no valid IA_NA option
   return ERROR_INVALID_MESSAGE;
}


/**
 * @brief Parse IA_NA option
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] option Pointer to the IA_NA option to parse
 * @return Error code
 **/

error_t dhcpv6ParseIaNaOption(Dhcpv6ClientCtx *context,
   const Dhcpv6Option *option)
{
   error_t error;
   size_t length;
   Dhcpv6Option *subOption;
   Dhcpv6IaNaOption *iaNaOption;
   Dhcpv6IaAddrOption *iaAddrOption;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Make sure the IA_NA option is valid
   if(ntohs(option->length) < sizeof(Dhcpv6IaNaOption))
      return ERROR_INVALID_OPTION;

   //Get the parameters associated with the IA_NA
   iaNaOption = (Dhcpv6IaNaOption *) option->value;
   //Compute the length of IA_NA Options field
   length = ntohs(option->length) - sizeof(Dhcpv6IaNaOption);

   //Check the IA identifier
   if(ntohl(iaNaOption->iaId) != interface->id)
      return ERROR_INVALID_OPTION;

   //If a client receives an IA_NA with T1 greater than T2, and both T1
   //and T2 are greater than 0, the client discards the IA_NA option and
   //processes the remainder of the message as though the server had not
   //included the invalid IA_NA option
   if(ntohl(iaNaOption->t1) > ntohl(iaNaOption->t2) && ntohl(iaNaOption->t2) > 0)
      return ERROR_INVALID_OPTION;

   //The client examines the status code in each IA individually
   error = dhcpv6ParseStatusCodeOption(iaNaOption->options, length);
   //If the status code is NoAddrsAvail, the client has received
   //no usable address in the IA
   if(error) return ERROR_INVALID_OPTION;

   //Get the address assigned to the IA
   subOption = dhcpv6GetOption(iaNaOption->options, length, DHCPV6_OPTION_IAADDR);
   //Failed to retrieve the IA Address option?
   if(!subOption || ntohs(subOption->length) < sizeof(Dhcpv6IaAddrOption))
      return ERROR_INVALID_OPTION;

   //Point to the contents of the IA Address option
   iaAddrOption = (Dhcpv6IaAddrOption *) subOption->value;
   //Compute the length of IA Address Options field
   length = ntohs(subOption->length) - sizeof(Dhcpv6IaAddrOption);

   //A client discards any addresses for which the preferred lifetime
   //is greater than the valid lifetime
   if(ntohl(iaAddrOption->preferredLifetime) > ntohl(iaAddrOption->validLifetime))
      return ERROR_INVALID_OPTION;

   //Record the IPv6 address assigned by the server
   context->clientAddr = iaAddrOption->address;

   //Record T1 and T2 times
   context->t1 = ntohl(iaNaOption->t1);
   context->t2 = ntohl(iaNaOption->t2);

   //Update preferred and valid lifetimes
   context->preferredLifetime = ntohl(iaAddrOption->preferredLifetime);
   context->validLifetime = ntohl(iaAddrOption->validLifetime);

   //If T1 or T2 is set to 0 by the server, the client may send a Renew
   //or Rebind message at the client's discretion
   if(!context->t1)
      context->t1 = context->preferredLifetime / 2;
   if(!context->t2)
      context->t2 = context->t1 + context->t1 / 2;

   //The IA_NA option was successfully parsed
   return NO_ERROR;
}


/**
 * @brief Update DHCPv6 FSM state
 * @param[in] context Pointer to the DHCPv6 client context
 * @param[in] newState New DHCPv6 state to switch to
 * @param[in] delay Initial delay
 **/

void dhcpv6ChangeState(Dhcpv6ClientCtx *context,
   Dhcpv6State newState, systime_t delay)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

#if (DHCPV6_TRACE_LEVEL >= TRACE_LEVEL_INFO)
   //Sanity check
   if(newState <= DHCPV6_STATE_DECLINE)
   {
      //DHCPv6 FSM states
      static const char_t *stateLabel[] =
      {
         "INIT",
         "SOLICIT",
         "REQUEST",
         "INIT-CONFIRM",
         "CONFIRM",
         "BOUND",
         "RENEW",
         "REBIND",
         "DECLINE"
      };

      //Debug message
      TRACE_INFO("%s: DHCPv6 client %s state\r\n",
         formatSystemTime(time, NULL), stateLabel[newState]);
   }
#endif

   //Set time stamp
   context->timestamp = time;
   //Set initial delay
   context->timeout = delay;
   //Reset retransmission counter
   context->retransmitCount = 0;
   //Switch to the new state
   context->state = newState;

   //Any user-defined event?
   if(context->settings.stateChangeEvent != NULL)
   {
      NetInterface *interface;

      //Point to the underlying network interface
      interface = context->settings.interface;

      //Leave critical section
      osReleaseMutex(&context->mutex);
      //Invoke callback function
      context->settings.stateChangeEvent(context, interface, newState);
      //Enter critical section
      osAcquireMutex(&context->mutex);
   }
}


/**
 * @brief Manage DHCPv6 configuration timeout
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6CheckTimeout(Dhcpv6ClientCtx *context)
{
   systime_t time;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Get current time
   time = osGetSystemTime();

   //Any user-defined event?
   if(context->settings.timeoutEvent != NULL)
   {
      //DHCPv6 configuration timeout?
      if(timeCompare(time, context->configStartTime + context->settings.timeout) >= 0)
      {
         //Ensure the callback function is only called once
         if(!context->timeoutEventDone)
         {
            //Leave critical section
            osReleaseMutex(&context->mutex);
            //Invoke callback function
            context->settings.timeoutEvent(context, interface);
            //Enter critical section
            osAcquireMutex(&context->mutex);

            //Set flag
            context->timeoutEventDone = TRUE;
         }
      }
   }
}


/**
 * @brief Compute the time elapsed since the client sent the first message
 * @param[in] context Pointer to the DHCPv6 client context
 * @return The elapsed time expressed in hundredths of a second
 **/

uint16_t dhcpv6ComputeElapsedTime(Dhcpv6ClientCtx *context)
{
   systime_t time = 0;

   //The elapsed time must be 0 for the first message
   if(context->retransmitCount > 0)
   {
      //Compute the time elapsed since the client sent the
      //first message (in hundredths of a second)
      time = (osGetSystemTime() - context->exchangeStartTime) / 10;

      //The value 0xFFFF is used to represent any elapsed time values
      //greater than the largest time value that can be represented
      if(time > 0xFFFF) time = 0xFFFF;
   }

   //Convert the 16-bit value to network byte order
   return htons((uint16_t) time);
}


/**
 * @brief Multiplication by a randomization factor
 *
 * Each of the computations of a new RT include a randomization factor
 * RAND, which is a random number chosen with a uniform distribution
 * between -0.1 and +0.1. The randomization factor is included to
 * minimize synchronization of messages transmitted by DHCPv6 clients
 *
 * @param[in] value Input value
 * @return Value resulting from the randomization process
 **/

int32_t dhcpv6Rand(int32_t value)
{
   //Use a randomization factor chosen with a uniform
   //distribution between -0.1 and +0.1
   return value * dhcpv6RandRange(-100, 100) / 1000;
}


/**
 * @brief Get a random value in the specified range
 * @param[in] min Lower bound
 * @param[in] max Upper bound
 * @return Random value in the specified range
 **/

int32_t dhcpv6RandRange(int32_t min, int32_t max)
{
   //Return a random value in the given range
   return min + netGetRand() % (max - min + 1);
}


/**
 * @brief Dump DHCPv6 configuration for debugging purpose
 * @param[in] context Pointer to the DHCPv6 client context
 **/

void dhcpv6DumpConfig(Dhcpv6ClientCtx *context)
{
   uint_t n;
   Ipv6Addr ipv6Addr;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Debug message
   TRACE_INFO("\r\n");
   TRACE_INFO("DHCPv6 configuration:\r\n");

   //Lease start time
   TRACE_INFO("  Lease Start Time = %s\r\n",
      formatSystemTime(context->leaseStartTime, NULL));

   //T1 parameter
   TRACE_INFO("  T1 = %" PRIu32 "s\r\n", context->t1);
   //T2 parameter
   TRACE_INFO("  T2 = %" PRIu32 "s\r\n", context->t2);

   //Global address
   ipv6GetGlobalAddr(interface, &ipv6Addr);
   TRACE_INFO("  IPv6 Global Address = %s\r\n", ipv6AddrToString(&ipv6Addr, NULL));

   //Preferred lifetime
   TRACE_INFO("  Preferred Lifetime = %" PRIu32 "s\r\n", context->preferredLifetime);
   //Valid lifetime
   TRACE_INFO("  Valid Lifetime = %" PRIu32 "s\r\n", context->validLifetime);

   //DNS servers
   for(n = 0; n < IPV6_MAX_DNS_SERVERS; n++)
   {
      ipv6GetDnsServer(interface, n, &ipv6Addr);
      TRACE_INFO("  DNS Server %u = %s\r\n", n + 1, ipv6AddrToString(&ipv6Addr, NULL));
   }

   //Debug message
   TRACE_INFO("\r\n");
}

#endif
