/**
 * @file private_mib_module.c
 * @brief Private MIB module
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
#define TRACE_LEVEL SNMP_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "snmp/mib_common.h"
#include "private_mib_module.h"
#include "private_mib_impl.h"
#include "crypto.h"
#include "asn1.h"
#include "oid.h"
#include "debug.h"


/**
 * @brief Private MIB base
 **/

PrivateMibBase privateMibBase;


/**
 * @brief Private MIB objects
 **/

const MibObject privateMibObjects[] =
{
   //testString object (1.3.6.1.4.1.8072.9999.9999.1.1)
   {
      "testString",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 1, 1},
      13,
      ASN1_CLASS_UNIVERSAL,
      ASN1_TYPE_OCTET_STRING,
      MIB_ACCESS_READ_WRITE,
      &privateMibBase.testString,
      &privateMibBase.testStringLen,
      PRIVATE_MIB_TEST_STRING_SIZE,
      NULL,
      NULL,
      NULL
   },
   //testInteger object (1.3.6.1.4.1.8072.9999.9999.1.2)
   {
      "testInteger",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 1, 2},
      13,
      ASN1_CLASS_UNIVERSAL,
      ASN1_TYPE_INTEGER,
      MIB_ACCESS_READ_WRITE,
      &privateMibBase.testInteger,
      NULL,
      sizeof(int32_t),
      NULL,
      NULL,
      NULL
   },
   //currentTime object (1.3.6.1.4.1.8072.9999.9999.1.3)
   {
      "currentTime",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 1, 3},
      13,
      ASN1_CLASS_APPLICATION,
      MIB_TYPE_TIME_TICKS,
      MIB_ACCESS_READ_ONLY,
      NULL,
      NULL,
      sizeof(uint32_t),
      NULL,
      privateMibGetCurrentTime,
      NULL
   },
   //ledCount object (1.3.6.1.4.1.8072.9999.9999.2.1)
   {
      "ledCount",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 2, 1},
      13,
      ASN1_CLASS_UNIVERSAL,
      ASN1_TYPE_INTEGER,
      MIB_ACCESS_READ_ONLY,
      &privateMibBase.ledCount,
      NULL,
      sizeof(int32_t),
      NULL,
      NULL,
      NULL
   },
   //ledColor object (1.3.6.1.4.1.8072.9999.9999.2.2.1)
   {
      "ledColor",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 2, 2, 1},
      14,
      ASN1_CLASS_UNIVERSAL,
      ASN1_TYPE_OCTET_STRING,
      MIB_ACCESS_READ_ONLY,
      NULL,
      NULL,
      PRIVATE_MIB_LED_COLOR_SIZE,
      NULL,
      privateMibGetLedEntry,
      privateMibGetNextLedEntry
   },
   //ledState object (1.3.6.1.4.1.8072.9999.9999.2.2.2)
   {
      "ledState",
      {43, 6, 1, 4, 1, 191, 8, 206, 15, 206, 15, 2, 2, 2},
      14,
      ASN1_CLASS_UNIVERSAL,
      ASN1_TYPE_INTEGER,
      MIB_ACCESS_READ_WRITE,
      NULL,
      NULL,
      sizeof(int32_t),
      privateMibSetLedEntry,
      privateMibGetLedEntry,
      privateMibGetNextLedEntry
   }
};


/**
 * @brief Private MIB module
 **/

const MibModule privateMibModule =
{
   privateMibObjects,
   arraysize(privateMibObjects),
   privateMibInit,
   privateMibLock,
   privateMibUnlock
};
