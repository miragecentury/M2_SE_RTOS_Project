/**
 * @file main.c
 * @brief Main routine
 *
 * @section License
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
#include <string.h>
#include "iodefine.h"
#include "rspi.h"
#include "adc.h"
#include "lcd_pmod.h"
#include "os_port.h"
#include "core/net.h"
#include "drivers/rza1_eth.h"
#include "drivers/upd60611.h"
#include "dhcp/dhcp_client.h"
#include "ipv6/slaac.h"
#include "http/http_server.h"
#include "icecast/icecast_client.h"
#include "eeprom.h"
#include "settings.h"
#include "dhcp_client_callbacks.h"
#include "http_server_callbacks.h"
#include "mp3_decoder.h"
#include "max9856.h"
#include "ssif0.h"
#include "dmac0.h"
#include "str.h"
#include "path.h"
#include "date_time.h"
#include "resource_manager.h"
#include "debug.h"

//Application configuration
#define APP_HTTP_MAX_CONNECTIONS 8

//Global variables
uint_t lcdLine = 0;
uint_t lcdColumn = 0;
uint_t volume = 0;

DhcpClientSettings dhcpClientSettings;
DhcpClientCtx dhcpClientContext;
SlaacSettings slaacSettings;
SlaacContext slaacContext;
HttpServerSettings httpServerSettings;
HttpServerContext httpServerContext;
HttpConnection httpConnections[APP_HTTP_MAX_CONNECTIONS];
IcecastClientSettings icecastClientSettings;
IcecastClientContext icecastClientContext;


/**
 * @brief Set cursor location
 * @param[in] line Line number
 * @param[in] column Column number
 **/

void lcdSetCursor(uint_t line, uint_t column)
{
   lcdLine = MIN(line, 16);
   lcdColumn = MIN(column, 21);
}


/**
 * @brief Write a character to the LCD display
 * @param[in] c Character to be written
 **/

void lcdPutChar(char_t c)
{
   uint8_t buffer[2];

   if(c == '\r')
   {
      lcdColumn = 0;
   }
   else if(c == '\n')
   {
      lcdColumn = 0;
      lcdLine++;
   }
   else if(lcdLine < 16 && lcdColumn < 21)
   {
      buffer[0] = c;
      buffer[1] = '\0';

      //Display current character
      display_set_cursor(lcdColumn, lcdLine);
      display_str(buffer);

      //Advance the cursor position
      if(++lcdColumn >= 21)
      {
         lcdColumn = 0;
         lcdLine++;
      }
   }
}


/**
 * @brief I/O initialization
 **/

void ioInit(void)
{
   //Configure LED (P7_8)
   PORT7.PMCn.BIT.PMCn8 = 0;
   PORT7.PIPCn.BIT.PIPCn8 = 0;
   PORT7.PMn.BIT.PMn8 = 0;
   //Clear LED
   PORT7.Pn.BIT.Pn8 = 0;

   //Configure SW1 (P1_11) as an input
   PORT1.PMCn.BIT.PMCn11 = 0;
   PORT1.PIBCn.BIT.PIBCn11 = 1;
   PORT1.PMn.BIT.PMn11 = 1;

   //Configure PMOD_RST (P3_15) as an output
   PORT3.PMCn.BIT.PMCn15 = 0;
   PORT3.PIPCn.BIT.PIPCn15 = 0;
   PORT3.PMn.BIT.PMn15 = 0;
   //Set PMOD_RST to default state
   PORT3.Pn.BIT.Pn15 = 1;

   //Configure PMOD_CS (P6_13) as an output
   PORT6.PMCn.BIT.PMCn13 = 0;
   PORT6.PIPCn.BIT.PIPCn13 = 0;
   PORT6.PMn.BIT.PMn13 = 0;
   //Set CS to default state
   PORT6.Pn.BIT.Pn13 = 1;

   //Configure PMOD_GPIO (P9_5) as an output
   PORT9.PMCn.BIT.PMCn5 = 0;
   PORT9.PIPCn.BIT.PIPCn5 = 0;
   PORT9.PMn.BIT.PMn5 = 0;
   //Set PMOD_GPIO to default state
   PORT9.Pn.BIT.Pn5 = 1;

   //Configure PHY_RST (P2_7)
   PORT2.PMCn.BIT.PMCn7 = 0;
   PORT2.PIPCn.BIT.PIPCn7 = 0;
   PORT2.PMn.BIT.PMn7 = 0;

   //Reset PHY transceiver
   PORT2.Pn.BIT.Pn7 = 0;
}


/**
 * @brief User task
 **/

void userTask(void *param)
{
   char_t buffer[40];

   //Point to the network interface
   NetInterface *interface = &netInterface[0];

   //Initialize LCD display
   lcdSetCursor(2, 0);
   printf("IPv4 Addr\r\n");
   lcdSetCursor(5, 0);
   printf("IPv6 Link-Local Addr\r\n");
   lcdSetCursor(9, 0);
   printf("IPv6 Global Addr\r\n");

   //Endless loop
   while(1)
   {
      //Start A/D conversion
      R_ADC_Read();
      //Get conversion result
      volume = g_adc_result * 100 / 1023;

#if (IPV4_SUPPORT == ENABLED)
      //Display IPv4 host address
      lcdSetCursor(3, 0);
      printf("%-16s\r\n", ipv4AddrToString(interface->ipv4Config.addr, buffer));
#endif

#if (IPV6_SUPPORT == ENABLED)
      //Display IPv6 link-local address
      lcdSetCursor(6, 0);
      printf("%-40s\r\n", ipv6AddrToString(&interface->ipv6Config.linkLocalAddr, buffer));

      //Display IPv6 global address
      lcdSetCursor(10, 0);
      printf("%-40s\r\n", ipv6AddrToString(&interface->ipv6Config.globalAddr, buffer));
#endif

      //Display volume
      lcdSetCursor(13, 0);
      printf("Headphone Volume %u%%  \r\n", volume);

      //Update volume
      max9856SetVolume(volume, volume);

      //Loop delay
      osDelayTask(200);
   }
}


/**
 * @brief LED blinking task
 **/

void blinkTask(void *parameters)
{
   //Endless loop
   while(1)
   {
      PORT7.Pn.BIT.Pn8 = 1;
      osDelayTask(100);
      PORT7.Pn.BIT.Pn8 = 0;
      osDelayTask(900);
   }
}


/**
 * @brief Main entry point
 * @return Unused value
 **/

int_t main(void)
{
   error_t error;
   size_t n;
   char_t *p;
   NetInterface *interface;
   OsTask *task;

   //Initialize kernel
   osInitKernel();
   //Configure debug UART
   debugInit(115200);

   //Start-up message
   TRACE_INFO("\r\n");
   TRACE_INFO("**************************************\r\n");
   TRACE_INFO("*** CycloneTCP Internet Radio Demo ***\r\n");
   TRACE_INFO("**************************************\r\n");
   TRACE_INFO("Copyright: 2010-2015 Oryx Embedded SARL\r\n");
   TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
   TRACE_INFO("Target: RZ/A1L\r\n");
   TRACE_INFO("\r\n");

   //Configure I/Os
   ioInit();

   //SPI initialization
   R_SPI_Init();
   //Initialize LCD display
   R_LCD_Init();
   //Turn on LCD display
   display_on();

   //ADC initialization
   R_ADC_Open();

   //EEPROM initialization
   eepromInit();

   //Check whether SW1 is pressed
   if(!PORT1.PPRn.BIT.PPRn11)
   {
      //Debug message
      TRACE_INFO("Restoring factory settings...\r\n");

      //Load default settings
      getDefaultSettings(&appSettings);
   }
   else
   {
      //Debug message
      TRACE_INFO("Loading user settings...\r\n");

      //Load application settings
      error = loadSettings(&appSettings);
      //Any error to report?
      if(error)
      {
         //Debug message
         TRACE_ERROR("Failed to load user settings!\r\n");
      }
   }

   //Welcome message
   lcdSetCursor(0, 0);
   printf("%s\r\n", appSettings.lan.hostname);

   //MAX9856 audio CODEC initialization
   error = max9856Init();
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize MAX9856!\r\n");
   }

   //Initialize SSIF0 interface
   ssif0Init();
   //Initialize DMAC0 controller
   dmac0Init();

   //TCP/IP stack initialization
   error = netInit();
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
   }

   //Configure the first Ethernet interface
   interface = &netInterface[0];

   //Set interface name
   netSetInterfaceName(interface, "eth0");
   //Set host name
   netSetHostname(interface, appSettings.lan.hostname);
   //Select the relevant network adapter
   netSetDriver(interface, &rza1EthDriver);
   netSetPhyDriver(interface, &upd60611PhyDriver);
   //Set host MAC address
   netSetMacAddr(interface, &appSettings.lan.macAddr);

   //Initialize network interface
   error = netConfigInterface(interface);
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);
   }

#if (IPV4_SUPPORT == ENABLED)
   //Check whether DHCP is enabled
   if(appSettings.lan.enableDhcp)
   {
      //Get default settings
      dhcpClientGetDefaultSettings(&dhcpClientSettings);
      //Set the network interface to be configured by DHCP
      dhcpClientSettings.interface = interface;
      //Disable rapid commit option
      dhcpClientSettings.rapidCommit = FALSE;
      //DHCP configuration timeout (30s)
      dhcpClientSettings.timeout = 30000;
      //DHCP configuration timeout event
      dhcpClientSettings.timeoutEvent = dhcpClientTimeoutCallback;
      //Link state change event
      dhcpClientSettings.linkChangeEvent = dhcpClientLinkChangeCallback;

      //DHCP client initialization
      error = dhcpClientInit(&dhcpClientContext, &dhcpClientSettings);
      //Failed to initialize DHCP client?
      if(error)
      {
         //Debug message
         TRACE_ERROR("Failed to initialize DHCP client!\r\n");
      }
   }
   else
   {
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
      TRACE_INFO("  IPv4 Address = %s\r\n", ipv4AddrToString(appSettings.lan.hostAddr, NULL));
      //Subnet mask
      TRACE_INFO("  Subnet Mask = %s\r\n", ipv4AddrToString(appSettings.lan.subnetMask, NULL));
      //Default gateway
      TRACE_INFO("  Default Gateway = %s\r\n", ipv4AddrToString(appSettings.lan.defaultGateway, NULL));
      //DNS servers
      TRACE_INFO("  DNS Server 1 = %s\r\n", ipv4AddrToString(appSettings.lan.primaryDns, NULL));
      TRACE_INFO("  DNS Server 2 = %s\r\n", ipv4AddrToString(appSettings.lan.secondaryDns, NULL));
   }
#endif

#if (IPV6_SUPPORT == ENABLED)
   //Get default settings
   slaacGetDefaultSettings(&slaacSettings);
   //Set the network interface to be configured
   slaacSettings.interface = interface;

   //SLAAC initialization
   error = slaacInit(&slaacContext, &slaacSettings);
   //Failed to initialize SLAAC?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize SLAAC!\r\n");
   }

   //Start IPv6 address autoconfiguration process
   error = slaacStart(&slaacContext);
   //Failed to start SLAAC process?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start SLAAC!\r\n");
   }
#endif

   //Check whether proxy server is enabled
   if(appSettings.proxy.enable)
   {
      //Set proxy parameters
      netSetProxy(interface, appSettings.proxy.name, appSettings.proxy.port);
   }

   //Get default settings
   httpServerGetDefaultSettings(&httpServerSettings);
   //Bind HTTP server to the desired interface
   httpServerSettings.interface = &netInterface[0];
   //Listen to port 80
   httpServerSettings.port = HTTP_PORT;
   //Client connections
   httpServerSettings.maxConnections = APP_HTTP_MAX_CONNECTIONS;
   httpServerSettings.connections = httpConnections;
   //Specify the server's root directory
   strcpy(httpServerSettings.rootDirectory, "/www/");
   //Set default home page
   strcpy(httpServerSettings.defaultDocument, "index.html");
   //Callback functions
   httpServerSettings.uriNotFoundCallback = httpServerUriNotFoundCallback;

   //HTTP server initialization
   error = httpServerInit(&httpServerContext, &httpServerSettings);
   //Failed to initialize HTTP server?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize HTTP server!\r\n");
   }

   //Start HTTP server
   error = httpServerStart(&httpServerContext);
   //Failed to start HTTP server?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start HTTP server!\r\n");
   }

   //Get default settings
   icecastClientGetDefaultSettings(&icecastClientSettings);
   //Bind Icecast client to the desired interface
   icecastClientSettings.interface = &netInterface[0];
   //Icecast server port
   icecastClientSettings.serverPort = appSettings.icecast.port;
   //Streaming buffer size
   icecastClientSettings.bufferSize = 65536;

   //Split URL string
   p = strchr(appSettings.icecast.url, '/');

   //Make sure the Icecast resource is valid
   if(p != NULL)
   {
      //Retrieve the length of the server name
      n = p - appSettings.icecast.url;

      //Icecast server name
      strncpy(icecastClientSettings.serverName, appSettings.icecast.url, n);
      //Properly terminate the string with a NULL character
      icecastClientSettings.serverName[n] = '\0';

      //Requested resource
      strcpy(icecastClientSettings.resource, p);
   }

   //Icecast client initialization
   error = icecastClientInit(&icecastClientContext, &icecastClientSettings);
   //Failed to initialize Icecast client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize Icecast client!\r\n");
   }

   //Start Icecast client
   error = icecastClientStart(&icecastClientContext);
   //Failed to start Icecast client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start Icecast client!\r\n");
   }

   //Start MP3 decoder
   error = mp3DecoderStart(&icecastClientContext);
   //Failed to start Icecast client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start MP3 decoder!\r\n");
   }

   //Create user task
   task = osCreateTask("User Task", userTask, NULL, 800, 1);
   //Failed to create the task?
   if(task == OS_INVALID_HANDLE)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }

   //Create a task to blink the LED
   task = osCreateTask("Blink", blinkTask, NULL, 500, 1);
   //Failed to create the task?
   if(task == OS_INVALID_HANDLE)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }

   //Start the execution of tasks
   osStartKernel();

   //This function should never return
   return 0;
}
