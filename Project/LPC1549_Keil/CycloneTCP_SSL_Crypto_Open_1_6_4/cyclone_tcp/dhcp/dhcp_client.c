/**
 * @file dhcp_client.c
 * @brief DHCP client (Dynamic Host Configuration Protocol)
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
 * The Dynamic Host Configuration Protocol is used to provide configuration
 * parameters to hosts. Refer to the following RFCs for complete details:
 * - RFC 2131: Dynamic Host Configuration Protocol
 * - RFC 2132: DHCP Options and BOOTP Vendor Extensions
 * - RFC 4039: Rapid Commit Option for the DHCP version 4
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.6.4
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL DHCP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "dhcp/dhcp_client.h"
#include "dhcp/dhcp_common.h"
#include "dhcp/dhcp_debug.h"
#include "date_time.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV4_SUPPORT == ENABLED && DHCP_CLIENT_SUPPORT == ENABLED)

//Tick counter to handle periodic operations
systime_t dhcpClientTickCounter;


/**
 * @brief Initialize settings with default values
 * @param[out] settings Structure that contains DHCP client settings
 **/

void dhcpClientGetDefaultSettings(DhcpClientSettings *settings)
{
   //Use default interface
   settings->interface = NULL;
   //Use default host name
   strcpy(settings->hostname, "");
   //No rapid commit
   settings->rapidCommit = FALSE;
   //Use the DNS servers provided by the DHCP server
   settings->manualDnsConfig = FALSE;
   //DHCP configuration timeout
   settings->timeout = 0;
   //DHCP configuration timeout event
   settings->timeoutEvent = NULL;
   //Link state change event
   settings->linkChangeEvent = NULL;
   //FSM state change event
   settings->stateChangeEvent = NULL;
}


/**
 * @brief DHCP client initialization
 * @param[in] context Pointer to the DHCP client context
 * @param[in] settings DHCP client specific settings
 * @return Error code
 **/

error_t dhcpClientInit(DhcpClientCtx *context, const DhcpClientSettings *settings)
{
   error_t error;
   size_t n;
   NetInterface *interface;

   //Debug message
   TRACE_INFO("Initializing DHCP client...\r\n");

   //Ensure the parameters are valid
   if(context == NULL || settings == NULL)
      return ERROR_INVALID_PARAMETER;
   //A valid pointer to the interface being configured is required
   if(settings->interface == NULL)
      return ERROR_INVALID_PARAMETER;

   //Point to the underlying network interface
   interface = settings->interface;

   //Clear the DHCP client context
   memset(context, 0, sizeof(DhcpClientCtx));
   //Save user settings
   context->settings = *settings;

   //No DHCP host name defined?
   if(settings->hostname[0] == '\0')
   {
      //Use default host name
      n = strlen(interface->hostname);
      //Limit the length of the string
      n = MIN(n, DHCP_CLIENT_MAX_HOSTNAME_LEN);

      //Copy host name
      strncpy(context->settings.hostname, interface->hostname, n);
      //Properly terminate the string with a NULL character
      context->settings.hostname[n] = '\0';
   }

   //Initialize mutex object
   if(!osCreateMutex(&context->mutex))
   {
      //Failed to create mutex
      return ERROR_OUT_OF_RESOURCES;
   }

   //Callback function to be called when a DHCP message is received
   error = udpAttachRxCallback(interface, DHCP_CLIENT_PORT,
      dhcpClientProcessMessage, context);

   //Failed to register callback function?
   if(error)
   {
      //Clean up side effects
      osDeleteMutex(&context->mutex);
      //Report an error
      return error;
   }

   //DHCP client is currently suspended
   context->running = FALSE;
   //Initialize state machine
   context->state = DHCP_STATE_INIT;

   //Attach the DHCP client context to the network interface
   interface->dhcpClientContext = context;

   //Successful initialization
   return NO_ERROR;
}


/**
 * @brief Start DHCP client
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpClientStart(DhcpClientCtx *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Starting DHCP client...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Start DHCP client
   context->running = TRUE;
   //Initialize state machine
   context->state = DHCP_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Stop DHCP client
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpClientStop(DhcpClientCtx *context)
{
   //Check parameter
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;

   //Debug message
   TRACE_INFO("Stopping DHCP client...\r\n");

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Stop DHCP client
   context->running = FALSE;
   //Reinitialize state machine
   context->state = DHCP_STATE_INIT;

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Retrieve current state
 * @param[in] context Pointer to the DHCP client context
 * @return Current DHCP client state
 **/

DhcpState dhcpClientGetState(DhcpClientCtx *context)
{
   DhcpState state;

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
 * @brief DHCP client timer handler
 *
 * This routine must be periodically called by the TCP/IP stack to
 * manage DHCP client operation
 *
 * @param[in] context Pointer to the DHCP client context
 **/


void dhcpClientTick(DhcpClientCtx *context)
{
   //Enter critical section
   osAcquireMutex(&context->mutex);

   //DHCP client finite state machine
   switch(context->state)
   {
   //Process INIT state
   case DHCP_STATE_INIT:
      //This is the initialization state, where a client begins the process of
      //acquiring a lease. It also returns here when a lease ends, or when a
      //lease negotiation fails
      dhcpStateInit(context);
      break;
   //Process SELECTING state
   case DHCP_STATE_SELECTING:
      //The client is waiting to receive DHCPOFFER messages from one or more
      //DHCP servers, so it can choose one
      dhcpStateSelecting(context);
      break;
   //Process REQUESTING state
   case DHCP_STATE_REQUESTING:
      //The client is waiting to hear back from the server to which
      //it sent its request
      dhcpStateRequesting(context);
      break;
   //Process INIT REBOOT state
   case DHCP_STATE_INIT_REBOOT:
      //When a client that already has a valid lease starts up after a
      //power-down or reboot, it starts here instead of the INIT state
      dhcpStateInitReboot(context);
      break;
   //Process REBOOTING state
   case DHCP_STATE_REBOOTING:
      //A client that has rebooted with an assigned address is waiting for
      //a confirming reply from a server
      dhcpStateRebooting(context);
      break;
   //Process BOUND state
   case DHCP_STATE_BOUND:
      //Client has a valid lease and is in its normal operating state
      dhcpStateBound(context);
      break;
   //Process RENEWING state
   case DHCP_STATE_RENEWING:
      //Client is trying to renew its lease. It regularly sends DHCPREQUEST messages with
      //the server that gave it its current lease specified, and waits for a reply
      dhcpStateRenewing(context);
      break;
   //Process REBINDING state
   case DHCP_STATE_REBINDING:
      //The client has failed to renew its lease with the server that originally granted it,
      //and now seeks a lease extension with any server that can hear it. It periodically sends
      //DHCPREQUEST messages with no server specified until it gets a reply or the lease ends
      dhcpStateRebinding(context);
      break;
   //Invalid state...
   default:
      //Switch to the INIT state
      context->state = DHCP_STATE_INIT;
      break;
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);
}


/**
 * @brief Callback function for link change event
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpClientLinkChangeEvent(DhcpClientCtx *context)
{
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check whether the DHCP client is running
   if(context->running)
   {
      //The host address is no longer valid
      ipv4SetHostAddrEx(interface, IPV4_UNSPECIFIED_ADDR,
         IPV4_ADDR_STATE_INVALID);

      //Clear subnet mask
      ipv4SetSubnetMask(interface, IPV4_UNSPECIFIED_ADDR);
   }

   //Check whether the client already has a valid lease
   if(context->state >= DHCP_STATE_INIT_REBOOT)
   {
      //Switch to the INIT-REBOOT state
      context->state = DHCP_STATE_INIT_REBOOT;
   }
   else
   {
      //Switch to the INIT state
      context->state = DHCP_STATE_INIT;
   }

   //Leave critical section
   osReleaseMutex(&context->mutex);

   //Invoke user-defined callback, if any
   if(context->settings.linkChangeEvent != NULL)
      context->settings.linkChangeEvent(context, interface, interface->linkState);
}


/**
 * @brief Process incoming DHCP message
 * @param[in] interface Underlying network interface
 * @param[in] pseudoHeader UDP pseudo header
 * @param[in] udpHeader UDP header
 * @param[in] buffer Multi-part buffer containing the incoming DHCP message
 * @param[in] offset Offset to the first byte of the DHCP message
 * @param[in] params Pointer to the DHCP client context
 **/

void dhcpClientProcessMessage(NetInterface *interface,
   const IpPseudoHeader *pseudoHeader, const UdpHeader *udpHeader,
   const NetBuffer *buffer, size_t offset, void *params)
{
   size_t length;
   DhcpClientCtx *context;
   DhcpMessage *message;

   //Retrieve the length of the DHCP message
   length = netBufferGetLength(buffer) - offset;

   //Make sure the DHCP message is valid
   if(length < sizeof(DhcpMessage))
      return;
   if(length > DHCP_MAX_MSG_SIZE)
      return;

   //Point to the beginning of the DHCP message
   message = netBufferAt(buffer, offset);
   //Sanity check
   if(!message) return;

   //Debug message
   TRACE_DEBUG("\r\n%s: DHCP message received (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), length);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(message, length);

   //Point to the DHCP client context
   context = (DhcpClientCtx *) params;

   //Enter critical section
   osAcquireMutex(&context->mutex);

   //Check current state
   switch(context->state)
   {
   //SELECTING state?
   case DHCP_STATE_SELECTING:
      //Parse DHCPOFFER message
      dhcpParseOffer(context, message, length);
      break;
   //REQUESTING, REBOOTING, RENEWING or REBINDING state?
   case DHCP_STATE_REQUESTING:
   case DHCP_STATE_REBOOTING:
   case DHCP_STATE_RENEWING:
   case DHCP_STATE_REBINDING:
      //Parse DHCPACK or DHCPNAK message
      dhcpParseAckNak(context, message, length);
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
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateInit(DhcpClientCtx *context)
{
   systime_t delay;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Check whether the DHCP client is running
   if(context->running)
   {
      //Wait for the link to be up before starting DHCP configuration
      if(interface->linkState)
      {
         //The client should wait for a random time to
         //desynchronize the use of DHCP at startup
         delay = netGetRandRange(0, DHCP_CLIENT_INIT_DELAY);

         //Record the time at which the client started
         //the address acquisition process
         context->configStartTime = osGetSystemTime();
         //Clear flag
         context->timeoutEventDone = FALSE;

         //Switch to the SELECTING state
         dhcpChangeState(context, DHCP_STATE_SELECTING, delay);
      }
   }
}


/**
 * @brief SELECTING state
 *
 * The client is waiting to receive DHCPOFFER messages from
 * one or more DHCP servers, so it can choose one
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateSelecting(DhcpClientCtx *context)
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
         //A transaction identifier is used by the client to
         //match incoming DHCP messages with pending requests
         context->transactionId = netGetRand();

         //Send a DHCPDISCOVER message
         dhcpSendDiscover(context);

         //Initial timeout value
         context->retransmitTimeout = DHCP_CLIENT_DISCOVER_INIT_RT;
      }
      else
      {
         //Send a DHCPDISCOVER message
         dhcpSendDiscover(context);

         //The timeout value is doubled for each subsequent retransmission
         context->retransmitTimeout *= 2;

         //Limit the timeout value to a maximum of 64 seconds
         if(context->retransmitTimeout > DHCP_CLIENT_DISCOVER_MAX_RT)
            context->retransmitTimeout = DHCP_CLIENT_DISCOVER_MAX_RT;
      }

      //Save the time at which the message was sent
      context->timestamp = time;

      //The timeout value should be randomized by the value of a uniform
      //number chosen from the range -1 to +1
      context->timeout = context->retransmitTimeout +
         netGetRandRange(-DHCP_CLIENT_RAND_FACTOR, DHCP_CLIENT_RAND_FACTOR);

      //Increment retransmission counter
      context->retransmitCount++;
   }

   //Manage DHCP configuration timeout
   dhcpCheckTimeout(context);
}


/**
 * @brief REQUESTING state
 *
 * The client is waiting to hear back from the server
 * to which it sent its request
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateRequesting(DhcpClientCtx *context)
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
         //A transaction identifier is used by the client to
         //match incoming DHCP messages with pending requests
         context->transactionId = netGetRand();

         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //Initial timeout value
         context->retransmitTimeout = DHCP_CLIENT_REQUEST_INIT_RT;

         //Save the time at which the message was sent
         context->timestamp = time;

         //The timeout value should be randomized by the value of a uniform
         //number chosen from the range -1 to +1
         context->timeout = context->retransmitTimeout +
            netGetRandRange(-DHCP_CLIENT_RAND_FACTOR, DHCP_CLIENT_RAND_FACTOR);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else if(context->retransmitCount < DHCP_CLIENT_REQUEST_MAX_RC)
      {
         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //The timeout value is doubled for each subsequent retransmission
         context->retransmitTimeout *= 2;

         //Limit the timeout value to a maximum of 64 seconds
         if(context->retransmitTimeout > DHCP_CLIENT_REQUEST_MAX_RT)
            context->retransmitTimeout = DHCP_CLIENT_REQUEST_MAX_RT;

         //Save the time at which the message was sent
         context->timestamp = time;

         //The timeout value should be randomized by the value of a uniform
         //number chosen from the range -1 to +1
         context->timeout = context->retransmitTimeout +
            netGetRandRange(-DHCP_CLIENT_RAND_FACTOR, DHCP_CLIENT_RAND_FACTOR);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //If the client does not receive a response within a reasonable
         //period of time, then it restarts the initialization procedure
         dhcpChangeState(context, DHCP_STATE_INIT, 0);
      }
   }

   //Manage DHCP configuration timeout
   dhcpCheckTimeout(context);
}


/**
 * @brief INIT-REBOOT state
 *
 * When a client that already has a valid lease starts up after a
 * power-down or reboot, it starts here instead of the INIT state
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateInitReboot(DhcpClientCtx *context)
{
   systime_t delay;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Check whether the DHCP client is running
   if(context->running)
   {
      //Wait for the link to be up before starting DHCP configuration
      if(interface->linkState)
      {
         //The client should wait for a random time to
         //desynchronize the use of DHCP at startup
         delay = netGetRandRange(0, DHCP_CLIENT_INIT_DELAY);

         //Record the time at which the client started
         //the address acquisition process
         context->configStartTime = osGetSystemTime();
         //Clear flag
         context->timeoutEventDone = FALSE;

         //Switch to the REBOOTING state
         dhcpChangeState(context, DHCP_STATE_REBOOTING, delay);
      }
   }
}


/**
 * @brief REBOOTING state
 *
 * A client that has rebooted with an assigned address is
 * waiting for a confirming reply from a server
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateRebooting(DhcpClientCtx *context)
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
         //A transaction identifier is used by the client to
         //match incoming DHCP messages with pending requests
         context->transactionId = netGetRand();

         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //Initial timeout value
         context->retransmitTimeout = DHCP_CLIENT_REQUEST_INIT_RT;

         //Save the time at which the message was sent
         context->timestamp = time;

         //The timeout value should be randomized by the value of a uniform
         //number chosen from the range -1 to +1
         context->timeout = context->retransmitTimeout +
            netGetRandRange(-DHCP_CLIENT_RAND_FACTOR, DHCP_CLIENT_RAND_FACTOR);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else if(context->retransmitCount < DHCP_CLIENT_REQUEST_MAX_RC)
      {
         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //The timeout value is doubled for each subsequent retransmission
         context->retransmitTimeout *= 2;

         //Limit the timeout value to a maximum of 64 seconds
         if(context->retransmitTimeout > DHCP_CLIENT_REQUEST_MAX_RT)
            context->retransmitTimeout = DHCP_CLIENT_REQUEST_MAX_RT;

         //Save the time at which the message was sent
         context->timestamp = time;

         //The timeout value should be randomized by the value of a uniform
         //number chosen from the range -1 to +1
         context->timeout = context->retransmitTimeout +
            netGetRandRange(-DHCP_CLIENT_RAND_FACTOR, DHCP_CLIENT_RAND_FACTOR);

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //If the client does not receive a response within a reasonable
         //period of time, then it restarts the initialization procedure
         dhcpChangeState(context, DHCP_STATE_INIT, 0);
      }
   }

   //Manage DHCP configuration timeout
   dhcpCheckTimeout(context);
}


/**
 * @brief BOUND state
 *
 * Client has a valid lease and is in its normal operating state
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateBound(DhcpClientCtx *context)
{
   systime_t t1;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //A client will never attempt to extend the lifetime
   //of the address when T1 set to 0xFFFFFFFF
   if(context->t1 != DHCP_INFINITE_TIME)
   {
      //Convert T1 to milliseconds
      t1 = context->t1 * 1000;

      //Check the time elapsed since the lease was obtained
      if(timeCompare(time, context->leaseStartTime + t1) >= 0)
      {
         //Record the time at which the client started the address renewal process
         context->configStartTime = time;

         //Enter the RENEWING state
         dhcpChangeState(context, DHCP_STATE_RENEWING, 0);
      }
   }
}


/**
 * @brief RENEWING state
 *
 * Client is trying to renew its lease. It regularly sends
 * DHCPREQUEST messages with the server that gave it its current
 * lease specified, and waits for a reply
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateRenewing(DhcpClientCtx *context)
{
   systime_t t2;
   systime_t time;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Convert T2 to milliseconds
      t2 = context->t2 * 1000;

      //Check whether T2 timer has expired
      if(timeCompare(time, context->leaseStartTime + t2) < 0)
      {
         //First DHCPREQUEST message?
         if(context->retransmitCount == 0)
         {
            //A transaction identifier is used by the client to
            //match incoming DHCP messages with pending requests
            context->transactionId = netGetRand();
         }

         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //Save the time at which the message was sent
         context->timestamp = time;

         //Compute the remaining time until T2 expires
         context->timeout = context->leaseStartTime + t2 - time;

         //The client should wait one-half of the remaining time until T2, down to
         //a minimum of 60 seconds, before retransmitting the DHCPREQUEST message
         if(context->timeout > (2 * DHCP_CLIENT_REQUEST_MIN_DELAY))
            context->timeout /= 2;

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //If no DHCPACK arrives before time T2, the client moves to REBINDING
         dhcpChangeState(context, DHCP_STATE_REBINDING, 0);
      }
   }
}


/**
 * @brief REBINDING state
 *
 * The client has failed to renew its lease with the server that originally
 * granted it, and now seeks a lease extension with any server that can
 * hear it. It periodically sends DHCPREQUEST messages with no server specified
 * until it gets a reply or the lease ends
 *
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpStateRebinding(DhcpClientCtx *context)
{
   systime_t time;
   systime_t leaseTime;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Get current time
   time = osGetSystemTime();

   //Check current time
   if(timeCompare(time, context->timestamp + context->timeout) >= 0)
   {
      //Convert the lease time to milliseconds
      leaseTime = context->leaseTime * 1000;

      //Check whether the lease has expired
      if(timeCompare(time, context->leaseStartTime + leaseTime) < 0)
      {
         //First DHCPREQUEST message?
         if(context->retransmitCount == 0)
         {
            //A transaction identifier is used by the client to
            //match incoming DHCP messages with pending requests
            context->transactionId = netGetRand();
         }

         //Send a DHCPREQUEST message
         dhcpSendRequest(context);

         //Save the time at which the message was sent
         context->timestamp = time;

         //Compute the remaining time until the lease expires
         context->timeout = context->leaseStartTime + leaseTime - time;

         //The client should wait one-half of the remaining lease time, down to a
         //minimum of 60 seconds, before retransmitting the DHCPREQUEST message
         if(context->timeout > (2 * DHCP_CLIENT_REQUEST_MIN_DELAY))
            context->timeout /= 2;

         //Increment retransmission counter
         context->retransmitCount++;
      }
      else
      {
         //The host address is no longer valid...
         ipv4SetHostAddrEx(interface, IPV4_UNSPECIFIED_ADDR,
            IPV4_ADDR_STATE_INVALID);

         //Clear subnet mask
         ipv4SetSubnetMask(interface, IPV4_UNSPECIFIED_ADDR);

         //If the lease expires before the client receives
         //a DHCPACK, the client moves to INIT state
         dhcpChangeState(context, DHCP_STATE_INIT, 0);
      }
   }
}


/**
 * @brief Send DHCPDISCOVER message
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpSendDiscover(DhcpClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   DhcpMessage *message;
   NetInterface *interface;
   IpAddr destIpAddr;

   //DHCP message type
   const uint8_t messageType = DHCP_MESSAGE_TYPE_DISCOVER;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCP message
   buffer = udpAllocBuffer(DHCP_MIN_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCP message
   message = netBufferAt(buffer, offset);
   //Clear memory buffer contents
   memset(message, 0, DHCP_MIN_MSG_SIZE);

   //Format DHCPDISCOVER message
   message->op = DHCP_OPCODE_BOOTREQUEST;
   message->htype = DHCP_HARDWARE_TYPE_ETH;
   message->hlen = sizeof(MacAddr);
   message->xid = htonl(context->transactionId);
   message->secs = dhcpComputeElapsedTime(context);
   message->flags = HTONS(DHCP_FLAG_BROADCAST);
   message->ciaddr = IPV4_UNSPECIFIED_ADDR;
   message->chaddr = interface->macAddr;

   //Write magic cookie before setting any option
   message->magicCookie = HTONL(DHCP_MAGIC_COOKIE);
   //Properly terminate options field
   message->options[0] = DHCP_OPT_END;

   //DHCP Message Type option
   dhcpAddOption(message, DHCP_OPT_DHCP_MESSAGE_TYPE,
      &messageType, sizeof(messageType));

   //Retrieve the length of the host name
   length = strlen(context->settings.hostname);

   //Any host name defined?
   if(length > 0)
   {
      //The Host Name option specifies the name of the client
      dhcpAddOption(message, DHCP_OPT_HOST_NAME,
         context->settings.hostname, length);
   }

   //Check whether rapid commit is enabled
   if(context->settings.rapidCommit)
   {
      //Include the Rapid Commit option if the client is prepared
      //to perform the DHCPDISCOVER-DHCPACK message exchange
      dhcpAddOption(message, DHCP_OPT_RAPID_COMMIT, NULL, 0);
   }

   //Set destination IP address
   destIpAddr.length = sizeof(Ipv4Addr);
   destIpAddr.ipv4Addr = IPV4_BROADCAST_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCP message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), DHCP_MIN_MSG_SIZE);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(message, DHCP_MIN_MSG_SIZE);

   //Broadcast DHCPDISCOVER message
   error = udpSendDatagramEx(interface, DHCP_CLIENT_PORT, &destIpAddr,
      DHCP_SERVER_PORT, buffer, offset, IPV4_DEFAULT_TTL);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send DHCPREQUEST message
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpSendRequest(DhcpClientCtx *context)
{
   error_t error;
   size_t length;
   size_t offset;
   NetBuffer *buffer;
   DhcpMessage *message;
   NetInterface *interface;
   IpAddr destIpAddr;

   //DHCP message type
   const uint8_t messageType = DHCP_MESSAGE_TYPE_REQUEST;

   //Requested DHCP options
   const uint8_t optionList[] =
   {
      DHCP_OPT_SUBNET_MASK,
      DHCP_OPT_ROUTER,
      DHCP_OPT_DNS_SERVER,
      DHCP_OPT_INTERFACE_MTU,
      DHCP_OPT_IP_ADDRESS_LEASE_TIME,
      DHCP_OPT_RENEWAL_TIME_VALUE,
      DHCP_OPT_REBINDING_TIME_VALUE
   };

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCP message
   buffer = udpAllocBuffer(DHCP_MIN_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCP message
   message = netBufferAt(buffer, offset);
   //Clear memory buffer contents
   memset(message, 0, DHCP_MIN_MSG_SIZE);

   //Format DHCPREQUEST message
   message->op = DHCP_OPCODE_BOOTREQUEST;
   message->htype = DHCP_HARDWARE_TYPE_ETH;
   message->hlen = sizeof(MacAddr);
   message->xid = htonl(context->transactionId);
   message->secs = dhcpComputeElapsedTime(context);

   //The client IP address must be included if the client
   //is fully configured and can respond to ARP requests
   if(context->state == DHCP_STATE_RENEWING ||
      context->state == DHCP_STATE_REBINDING)
   {
      message->flags = 0;
      message->ciaddr = interface->ipv4Config.addr;
   }
   else
   {
      message->flags = HTONS(DHCP_FLAG_BROADCAST);
      message->ciaddr = IPV4_UNSPECIFIED_ADDR;
   }

   //Client hardware address
   message->chaddr = interface->macAddr;
   //Write magic cookie before setting any option
   message->magicCookie = HTONL(DHCP_MAGIC_COOKIE);
   //Properly terminate options field
   message->options[0] = DHCP_OPT_END;

   //DHCP Message Type option
   dhcpAddOption(message, DHCP_OPT_DHCP_MESSAGE_TYPE,
      &messageType, sizeof(messageType));

   //Retrieve the length of the host name
   length = strlen(context->settings.hostname);

   //Any host name defined?
   if(length > 0)
   {
      //The Host Name option specifies the name of the client
      dhcpAddOption(message, DHCP_OPT_HOST_NAME,
         context->settings.hostname, length);
   }

   //Server Identifier option
   if(context->state == DHCP_STATE_REQUESTING)
   {
      dhcpAddOption(message, DHCP_OPT_SERVER_IDENTIFIER,
         &context->serverIpAddr, sizeof(Ipv4Addr));
   }

   //Requested IP Address option
   if(context->state == DHCP_STATE_REQUESTING ||
      context->state == DHCP_STATE_REBOOTING)
   {
      dhcpAddOption(message, DHCP_OPT_REQUESTED_IP_ADDRESS,
         &context->requestedIpAddr, sizeof(Ipv4Addr));
   }

   //Parameter Request List option
   dhcpAddOption(message, DHCP_OPT_PARAM_REQUEST_LIST,
      optionList, sizeof(optionList));

   //IP address is being renewed?
   if(context->state == DHCP_STATE_RENEWING)
   {
      //The client transmits the message directly to the
      //server that initially granted the lease
      destIpAddr.length = sizeof(Ipv4Addr);
      destIpAddr.ipv4Addr = context->serverIpAddr;
   }
   else
   {
      //Broadcast the message
      destIpAddr.length = sizeof(Ipv4Addr);
      destIpAddr.ipv4Addr = IPV4_BROADCAST_ADDR;
   }

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCP message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), DHCP_MIN_MSG_SIZE);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(message, DHCP_MIN_MSG_SIZE);

   //Send DHCPREQUEST message
   error = udpSendDatagramEx(interface, DHCP_CLIENT_PORT, &destIpAddr,
      DHCP_SERVER_PORT, buffer, offset, IPV4_DEFAULT_TTL);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Send DHCPDECLINE message
 * @param[in] context Pointer to the DHCP client context
 * @return Error code
 **/

error_t dhcpSendDecline(DhcpClientCtx *context)
{
   error_t error;
   size_t offset;
   NetBuffer *buffer;
   DhcpMessage *message;
   NetInterface *interface;
   IpAddr destIpAddr;

   //DHCP message type
   const uint8_t messageType = DHCP_MESSAGE_TYPE_DECLINE;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Allocate a memory buffer to hold the DHCP message
   buffer = udpAllocBuffer(DHCP_MIN_MSG_SIZE, &offset);
   //Failed to allocate buffer?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Point to the beginning of the DHCP message
   message = netBufferAt(buffer, offset);
   //Clear memory buffer contents
   memset(message, 0, DHCP_MIN_MSG_SIZE);

   //Format DHCPDECLINE message
   message->op = DHCP_OPCODE_BOOTREQUEST;
   message->htype = DHCP_HARDWARE_TYPE_ETH;
   message->hlen = sizeof(MacAddr);
   message->xid = htonl(context->transactionId);
   message->secs = 0;
   message->flags = 0;
   message->ciaddr = IPV4_UNSPECIFIED_ADDR;
   message->chaddr = interface->macAddr;

   //Write magic cookie before setting any option
   message->magicCookie = HTONL(DHCP_MAGIC_COOKIE);
   //Properly terminate options field
   message->options[0] = DHCP_OPT_END;

   //DHCP Message Type option
   dhcpAddOption(message, DHCP_OPT_DHCP_MESSAGE_TYPE,
      &messageType, sizeof(messageType));
   //Server Identifier option
   dhcpAddOption(message, DHCP_OPT_SERVER_IDENTIFIER,
      &context->serverIpAddr, sizeof(Ipv4Addr));
   //Requested IP Address option
   dhcpAddOption(message, DHCP_OPT_REQUESTED_IP_ADDRESS,
      &context->requestedIpAddr, sizeof(Ipv4Addr));

   //Set destination IP address
   destIpAddr.length = sizeof(Ipv4Addr);
   destIpAddr.ipv4Addr = IPV4_BROADCAST_ADDR;

   //Debug message
   TRACE_DEBUG("\r\n%s: Sending DHCP message (%" PRIuSIZE " bytes)...\r\n",
      formatSystemTime(osGetSystemTime(), NULL), DHCP_MIN_MSG_SIZE);

   //Dump the contents of the message for debugging purpose
   dhcpDumpMessage(message, DHCP_MIN_MSG_SIZE);

   //Broadcast DHCPDECLINE message
   error = udpSendDatagramEx(interface, DHCP_CLIENT_PORT, &destIpAddr,
      DHCP_SERVER_PORT, buffer, offset, IPV4_DEFAULT_TTL);

   //Free previously allocated memory
   netBufferFree(buffer);
   //Return status code
   return error;
}


/**
 * @brief Parse DHCPOFFER message
 * @param[in] context Pointer to the DHCP client context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 **/

void dhcpParseOffer(DhcpClientCtx *context,
   const DhcpMessage *message, size_t length)
{
   DhcpOption *option;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //The DHCP server shall respond with a BOOTREPLY opcode
   if(message->op != DHCP_OPCODE_BOOTREPLY)
      return;
   //Enforce hardware type
   if(message->htype != DHCP_HARDWARE_TYPE_ETH)
      return;
   //Check the length of the hardware address
   if(message->hlen != sizeof(MacAddr))
      return;
   //Discard any received packet that does not match the transaction ID
   if(ntohl(message->xid) != context->transactionId)
      return;
   //Make sure the IP address offered to the client is valid
   if(message->yiaddr == IPV4_UNSPECIFIED_ADDR)
      return;
   //Check MAC address
   if(!macCompAddr(&message->chaddr, &interface->macAddr))
      return;
   //Check magic cookie
   if(message->magicCookie != HTONL(DHCP_MAGIC_COOKIE))
      return;

   //Retrieve DHCP Message Type option
   option = dhcpGetOption(message, length, DHCP_OPT_DHCP_MESSAGE_TYPE);

   //Failed to retrieve specified option?
   if(!option || option->length != 1)
      return;
   //Check message type
   if(option->value[0] != DHCP_MESSAGE_TYPE_OFFER)
      return;

   //Retrieve Server Identifier option
   option = dhcpGetOption(message, length, DHCP_OPT_SERVER_IDENTIFIER);

   //Failed to retrieve specified option?
   if(!option || option->length != 4)
      return;

   //Record the DHCP server IP address
   ipv4CopyAddr(&context->serverIpAddr, option->value);

   //Record the IP address offered to the client
   context->requestedIpAddr = message->yiaddr;

   //Switch to the REQUESTING state
   dhcpChangeState(context, DHCP_STATE_REQUESTING, 0);
}


/**
 * @brief Parse DHCPACK or DHCPNAK message
 * @param[in] context Pointer to the DHCP client context
 * @param[in] message Pointer to the incoming DHCP message
 * @param[in] length Length of the incoming message to parse
 * @return Error code
 **/

void dhcpParseAckNak(DhcpClientCtx *context,
   const DhcpMessage *message, size_t length)
{
   uint_t i;
   uint_t n;
   DhcpOption *option;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //The DHCP server shall respond with a BOOTREPLY opcode
   if(message->op != DHCP_OPCODE_BOOTREPLY)
      return;
   //Enforce hardware type
   if(message->htype != DHCP_HARDWARE_TYPE_ETH)
      return;
   //Check the length of the hardware address
   if(message->hlen != sizeof(MacAddr))
      return;
   //Discard any received packet that does not match the transaction ID
   if(ntohl(message->xid) != context->transactionId)
      return;
   //Check MAC address
   if(!macCompAddr(&message->chaddr, &interface->macAddr))
      return;
   //Check magic cookie
   if(message->magicCookie != HTONL(DHCP_MAGIC_COOKIE))
      return;

   //Retrieve DHCP Message Type option
   option = dhcpGetOption(message, length, DHCP_OPT_DHCP_MESSAGE_TYPE);

   //Failed to retrieve specified option?
   if(!option || option->length != 1)
      return;

   //Check message type (DHCPACK or DHCPNAK)
   if(option->value[0] == DHCP_MESSAGE_TYPE_NAK)
   {
      //The host address is no longer appropriate for the link
      ipv4SetHostAddrEx(interface, IPV4_UNSPECIFIED_ADDR,
         IPV4_ADDR_STATE_INVALID);

      //Clear subnet mask
      ipv4SetSubnetMask(interface, IPV4_UNSPECIFIED_ADDR);

      //Restart DHCP configuration
      dhcpChangeState(context, DHCP_STATE_INIT, 0);
   }
   else if(option->value[0] == DHCP_MESSAGE_TYPE_ACK)
   {
      //Retrieve Server Identifier option
      option = dhcpGetOption(message, length, DHCP_OPT_SERVER_IDENTIFIER);

      //Failed to retrieve specified option?
      if(!option || option->length != 4)
         return;
      //Unexpected server identifier?
      if(!ipv4CompAddr(option->value, &context->serverIpAddr))
         return;

      //Retrieve IP Address Lease Time option
      option = dhcpGetOption(message, length, DHCP_OPT_IP_ADDRESS_LEASE_TIME);

      //Failed to retrieve specified option?
      if(!option  || option->length != 4)
         return;

      //Record the lease time
      context->leaseTime = LOAD32BE(option->value);

      //Retrieve Renewal Time Value option
      option = dhcpGetOption(message, length, DHCP_OPT_RENEWAL_TIME_VALUE);

      //Specified option found?
      if(option && option->length == 4)
      {
         //This option specifies the time interval from address assignment
         //until the client transitions to the RENEWING state
         context->t1 = LOAD32BE(option->value);
      }
      else if(context->leaseTime != DHCP_INFINITE_TIME)
      {
         //By default, T1 is set to 50% of the lease time
         context->t1 = context->leaseTime / 2;
      }
      else
      {
         //Infinite lease
         context->t1 = DHCP_INFINITE_TIME;
      }

      //Retrieve Rebinding Time value option
      option = dhcpGetOption(message, length, DHCP_OPT_REBINDING_TIME_VALUE);

      //Specified option found?
      if(option && option->length == 4)
      {
         //This option specifies the time interval from address assignment
         //until the client transitions to the REBINDING state
         context->t2 = LOAD32BE(option->value);
      }
      else if(context->leaseTime != DHCP_INFINITE_TIME)
      {
         //By default, T2 is set to 87.5% of the lease time
         context->t2 = context->leaseTime * 7 / 8;
      }
      else
      {
         //Infinite lease
         context->t2 = DHCP_INFINITE_TIME;
      }

      //Retrieve Subnet Mask option
      option = dhcpGetOption(message, length, DHCP_OPT_SUBNET_MASK);

      //The specified option has been found?
      if(option && option->length == sizeof(Ipv4Addr))
      {
         //Record subnet mask
         ipv4CopyAddr(&interface->ipv4Config.subnetMask, option->value);
      }

      //Retrieve Router option
      option = dhcpGetOption(message, length, DHCP_OPT_ROUTER);

      //The specified option has been found?
      if(option && !(option->length % sizeof(Ipv4Addr)))
      {
         //Save default gateway
         if(option->length >= sizeof(Ipv4Addr))
            ipv4CopyAddr(&interface->ipv4Config.defaultGateway, option->value);
      }

      //Use the DNS servers provided by the DHCP server?
      if(!context->settings.manualDnsConfig)
      {
         //Retrieve DNS Server option
         option = dhcpGetOption(message, length, DHCP_OPT_DNS_SERVER);

          //The specified option has been found?
         if(option && !(option->length % sizeof(Ipv4Addr)))
         {
            //Get the number of addresses provided in the response
            n = option->length / sizeof(Ipv4Addr);
            //Only a limited set of DNS servers is supported
            n = MIN(n, IPV4_MAX_DNS_SERVERS);

            //Loop through the list of addresses
            for(i = 0; i < n; i++)
            {
               //Record DNS server address
               ipv4CopyAddr(&interface->ipv4Config.dnsServer[i],
                  option->value + i * sizeof(Ipv4Addr));
            }
         }
      }

      //Retrieve MTU option
      option = dhcpGetOption(message, length, DHCP_OPT_INTERFACE_MTU);

      //The specified option has been found?
      if(option && option->length == 2)
      {
         //This option specifies the MTU to use on this interface
         n = LOAD16BE(option->value);
         //Save MTU
         ipv4SetMtu(interface, n);
      }

      //Record the IP address assigned to the client
      ipv4SetHostAddrEx(interface, message->yiaddr, IPV4_ADDR_STATE_VALID);

      //Save the time a which the lease was obtained
      context->leaseStartTime = osGetSystemTime();
      //Dump current DHCP configuration for debugging purpose
      dhcpDumpConfig(context);
      //The client transitions to the BOUND state
      dhcpChangeState(context, DHCP_STATE_BOUND, 0);
   }
}


/**
 * @brief Update DHCP FSM state
 * @param[in] context Pointer to the DHCP client context
 * @param[in] newState New DHCP state to switch to
 * @param[in] delay Initial delay
 **/

void dhcpChangeState(DhcpClientCtx *context,
   DhcpState newState, systime_t delay)
{
   systime_t time;

   //Get current time
   time = osGetSystemTime();

#if (DHCP_TRACE_LEVEL >= TRACE_LEVEL_INFO)
   //Sanity check
   if(newState <= DHCP_STATE_REBINDING)
   {
      //DHCP FSM states
      static const char_t *stateLabel[] =
      {
         "INIT",
         "SELECTING",
         "REQUESTING",
         "INIT-REBOOT",
         "REBOOTING",
         "BOUND",
         "RENEWING",
         "REBINDING"
      };

      //Debug message
      TRACE_INFO("%s: DHCP client %s state\r\n",
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
 * @brief Manage DHCP configuration timeout
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpCheckTimeout(DhcpClientCtx *context)
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
      //DHCP configuration timeout?
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
 * @brief Compute the appropriate secs field
 *
 * Compute the number of seconds elapsed since the client began
 * address acquisition or renewal process
 *
 * @param[in] context Pointer to the DHCP client context
 * @return The elapsed time expressed in seconds
 **/

uint16_t dhcpComputeElapsedTime(DhcpClientCtx *context)
{
   systime_t time;

   //Compute the time elapsed since the DHCP configuration process started
   time = (osGetSystemTime() - context->configStartTime) / 1000;

   //The value 0xFFFF is used to represent any elapsed time values
   //greater than the largest time value that can be represented
   if(time > 0xFFFF) time = 0xFFFF;

   //Convert the 16-bit value to network byte order
   return htons((uint16_t) time);
}


/**
 * @brief Dump DHCP configuration for debugging purpose
 * @param[in] context Pointer to the DHCP client context
 **/

void dhcpDumpConfig(DhcpClientCtx *context)
{
   uint_t i;
   size_t mtu;
   Ipv4Addr ipv4Addr;
   NetInterface *interface;

   //Point to the underlying network interface
   interface = context->settings.interface;

   //Debug message
   TRACE_INFO("\r\n");
   TRACE_INFO("DHCP configuration:\r\n");

   //Lease start time
   TRACE_INFO("  Lease Start Time = %s\r\n",
      formatSystemTime(context->leaseStartTime, NULL));
   //Lease time
   TRACE_INFO("  Lease Time = %" PRIu32 "s\r\n", context->leaseTime);
   //Renewal time
   TRACE_INFO("  T1 = %" PRIu32 "s\r\n", context->t1);
   //Rebinding time
   TRACE_INFO("  T2 = %" PRIu32 "s\r\n", context->t2);

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

   //Maximum transmit unit
   ipv4GetMtu(interface, &mtu);
   TRACE_INFO("  MTU = %" PRIuSIZE "\r\n", mtu);
   TRACE_INFO("\r\n");
}

#endif
