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
#include "riic_rskrza1h.h"
#include "rsk_switches.h"
#include "rspi.h"
#include "adc.h"
#include "led.h"
#include "lcd_pmod.h"
#include "os_port.h"
#include "core/net.h"
#include "drivers/rza1_eth.h"
#include "drivers/upd60611.h"
#include "dhcp/dhcp_client.h"
#include "ipv6/slaac.h"
#include "icecast/icecast_client.h"
#include "mp3_decoder.h"
#include "radio_stations.h"
#include "max9856.h"
#include "ssif0.h"
#include "dmac0.h"
#include "str.h"
#include "path.h"
#include "date_time.h"
#include "resource_manager.h"
#include "debug.h"

//Application configuration
#define APP_MAC_ADDR "00-AB-CD-EF-00-A1"

#define APP_USE_DHCP ENABLED
#define APP_IPV4_HOST_ADDR "192.168.0.20"
#define APP_IPV4_SUBNET_MASK "255.255.255.0"
#define APP_IPV4_DEFAULT_GATEWAY "192.168.0.254"
#define APP_IPV4_PRIMARY_DNS "8.8.8.8"
#define APP_IPV4_SECONDARY_DNS "8.8.4.4"

#define APP_USE_SLAAC ENABLED
#define APP_IPV6_LINK_LOCAL_ADDR "fe80::a1"
#define APP_IPV6_PREFIX "2001:db8::"
#define APP_IPV6_PREFIX_LENGTH 64
#define APP_IPV6_GLOBAL_ADDR "2001:db8::a1"
#define APP_IPV6_ROUTER "fe80::1"
#define APP_IPV6_PRIMARY_DNS "2001:4860:4860::8888"
#define APP_IPV6_SECONDARY_DNS "2001:4860:4860::8844"

#define APP_USE_PROXY DISABLED
#define APP_PROXY_SERVER_NAME "192.168.0.254"
#define APP_PROXY_SERVER_PORT 8080

//Global variables
uint_t lcdLine = 0;
uint_t lcdColumn = 0;
uint_t volume = 0;

DhcpClientSettings dhcpClientSettings;
DhcpClientCtx dhcpClientContext;
SlaacSettings slaacSettings;
SlaacContext slaacContext;
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
      R_LED_On(LED0);
      osDelayTask(100);
      R_LED_Off(LED0);
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
   NetInterface *interface;
   OsTask *task;
   MacAddr macAddr;
#if (APP_USE_DHCP == DISABLED)
   Ipv4Addr ipv4Addr;
#endif
#if (APP_USE_SLAAC == DISABLED)
   Ipv6Addr ipv6Addr;
#endif

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
   TRACE_INFO("Target: RZ/A1H\r\n");
   TRACE_INFO("\r\n");

   //I2C initialization
   R_RIIC_rza1h_rsk_init();
   //SPI initialization
   R_SPI_Init();

   //Configure LEDs
   R_LED_Init();
   //Configure switches
   init_switches();

   //Initialize LCD display
   R_LCD_Init();
   //Turn on LCD display
   display_on();

   //Welcome message
   lcdSetCursor(0, 0);
   printf("Internet Radio Demo\r\n");

   //ADC initialization
   R_ADC_Open();

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
   netSetHostname(interface, "InternetRadio");
   //Select the relevant network adapter
   netSetDriver(interface, &rza1EthDriver);
   netSetPhyDriver(interface, &upd60611PhyDriver);
   //Set host MAC address
   macStringToAddr(APP_MAC_ADDR, &macAddr);
   netSetMacAddr(interface, &macAddr);

   //Initialize network interface
   error = netConfigInterface(interface);
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);
   }

#if (IPV4_SUPPORT == ENABLED)
#if (APP_USE_DHCP == ENABLED)
   //Get default settings
   dhcpClientGetDefaultSettings(&dhcpClientSettings);
   //Set the network interface to be configured by DHCP
   dhcpClientSettings.interface = interface;
   //Disable rapid commit option
   dhcpClientSettings.rapidCommit = FALSE;

   //DHCP client initialization
   error = dhcpClientInit(&dhcpClientContext, &dhcpClientSettings);
   //Failed to initialize DHCP client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize DHCP client!\r\n");
   }

   //Start DHCP client
   error = dhcpClientStart(&dhcpClientContext);
   //Failed to start DHCP client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start DHCP client!\r\n");
   }
#else
   //Set IPv4 host address
   ipv4StringToAddr(APP_IPV4_HOST_ADDR, &ipv4Addr);
   ipv4SetHostAddr(interface, ipv4Addr);

   //Set subnet mask
   ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &ipv4Addr);
   ipv4SetSubnetMask(interface, ipv4Addr);

   //Set default gateway
   ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &ipv4Addr);
   ipv4SetDefaultGateway(interface, ipv4Addr);

   //Set primary and secondary DNS servers
   ipv4StringToAddr(APP_IPV4_PRIMARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 0, ipv4Addr);
   ipv4StringToAddr(APP_IPV4_SECONDARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 1, ipv4Addr);
#endif
#endif

#if (IPV6_SUPPORT == ENABLED)
#if (APP_USE_SLAAC == ENABLED)
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
#else
   //Set link-local address
   ipv6StringToAddr(APP_IPV6_LINK_LOCAL_ADDR, &ipv6Addr);
   ipv6SetLinkLocalAddr(interface, &ipv6Addr);

   //Set IPv6 prefix
   ipv6StringToAddr(APP_IPV6_PREFIX, &ipv6Addr);
   ipv6SetPrefix(interface, &ipv6Addr, APP_IPV6_PREFIX_LENGTH);

   //Set global address
   ipv6StringToAddr(APP_IPV6_GLOBAL_ADDR, &ipv6Addr);
   ipv6SetGlobalAddr(interface, &ipv6Addr);

   //Set router
   ipv6StringToAddr(APP_IPV6_ROUTER, &ipv6Addr);
   ipv6SetRouter(interface, &ipv6Addr);

   //Set primary and secondary DNS servers
   ipv6StringToAddr(APP_IPV6_PRIMARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 0, &ipv6Addr);
   ipv6StringToAddr(APP_IPV6_SECONDARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 1, &ipv6Addr);
#endif
#endif

#if (APP_USE_PROXY == ENABLED)
   //Set proxy parameters
   netSetProxy(interface, APP_PROXY_SERVER_NAME, APP_PROXY_SERVER_PORT);
#endif

   //Get default settings
   icecastClientGetDefaultSettings(&icecastClientSettings);

   //Bind Icecast client to the desired interface
   icecastClientSettings.interface = &netInterface[0];
   //Icecast server name
   strcpy(icecastClientSettings.serverName, ICECAST_SERVER_NAME);
   //Icecast server port
   icecastClientSettings.serverPort = ICECAST_SERVER_PORT;
   //Requested resource
   strcpy(icecastClientSettings.resource, ICECAST_RESOURCE);
   //Streaming buffer size
   icecastClientSettings.bufferSize = 65536;

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
