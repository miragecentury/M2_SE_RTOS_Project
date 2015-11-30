/**
 * @file http_server_callbacks.c
 * @brief HTTP server custom callbacks
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
#include "core/ping.h"
#include "http/http_server.h"
#include "http/mime.h"
#include "http_server_callbacks.h"
#include "settings.h"
#include "str.h"
#include "path.h"
#include "date_time.h"
#include "resource_manager.h"
#include "debug.h"


/**
 * @brief URI not found callback
 * @param[in] connection Handle referencing a client connection
 * @param[in] uri NULL-terminated string containing the path to the requested resource
 * @return Error code
 **/

error_t httpServerUriNotFoundCallback(HttpConnection *connection,
   const char_t *uri)
{
   error_t error;

   //Check the requested URI
   if(!strcasecmp(uri, "/get_config.xml"))
   {
      //Process get_config.xml file
      error = httpServerProcessGetConfig(connection);

   }
   else if(!strcasecmp(uri, "/set_config.xml"))
   {
      //Process set_config.xml file
      error = httpServerProcessSetConfig(connection);
   }
   else
   {
      //URI not found...
      error = ERROR_NOT_FOUND;
   }

   //Return status code
   return error;
}


/**
 * @brief Process get_config.xml request
 * @param[in] connection Handle referencing a client connection
 * @return Error code
 **/

error_t httpServerProcessGetConfig(HttpConnection *connection)
{
   error_t error;
   uint_t i;
   size_t n;
   char_t *buffer;
   char_t temp[40];

   //Allocate a memory buffer
   buffer = osAllocMem(2048);
   //Failed to allocate memory?
   if(!buffer) return ERROR_OUT_OF_MEMORY;

   //Format XML data
   n = sprintf(buffer, "<settings>\r\n");

   //Icecast settings
   n += sprintf(buffer + n, "  <icecast>\r\n");
   //Icecast resource
   n += sprintf(buffer + n, "    <url>%s</url>\r\n",
      appSettings.icecast.url);
   //Icecast server port
   n += sprintf(buffer + n, "    <port>%" PRIu16 "</port>\r\n",
      appSettings.icecast.port);
   //End of Icecast settings
   n += sprintf(buffer + n, "  </icecast>\r\n");

   //LAN settings
   n += sprintf(buffer + n, "  <lan>\r\n");
   //MAC address
   n += sprintf(buffer + n, "    <macAddr>%s</macAddr>\r\n",
      macAddrToString(&appSettings.lan.macAddr, temp));
   //Host name
   n += sprintf(buffer + n, "    <hostName>%s</hostName>\r\n",
      appSettings.lan.hostname);
   //Enable DHCP
   n += sprintf(buffer + n, "    <enableDhcp>%u</enableDhcp>\r\n",
      appSettings.lan.enableDhcp);
   //IPv4 host address
   n += sprintf(buffer + n, "    <hostAddr>%s</hostAddr>\r\n",
      ipv4AddrToString(appSettings.lan.hostAddr, temp));
   //Subnet mask
   n += sprintf(buffer + n, "    <subnetMask>%s</subnetMask>\r\n",
      ipv4AddrToString(appSettings.lan.subnetMask, temp));
   //Default gateway
   n += sprintf(buffer + n, "    <defaultGateway>%s</defaultGateway>\r\n",
      ipv4AddrToString(appSettings.lan.defaultGateway, temp));
   //Primary DNS
   n += sprintf(buffer + n, "    <primaryDns>%s</primaryDns>\r\n",
      ipv4AddrToString(appSettings.lan.primaryDns, temp));
   //Secondary DNS
   n += sprintf(buffer + n, "    <secondaryDns>%s</secondaryDns>\r\n",
      ipv4AddrToString(appSettings.lan.secondaryDns, temp));
   //End of LAN settings
   n += sprintf(buffer + n, "  </lan>\r\n");

   //Proxy settings
   n += sprintf(buffer + n, "  <proxy>\r\n");
   //Enable proxy server
   n += sprintf(buffer + n, "    <enable>%u</enable>\r\n",
      appSettings.proxy.enable);
   //Proxy server name
   n += sprintf(buffer + n, "    <name>%s</name>\r\n",
      appSettings.proxy.name);
   //Proxy server port
   n += sprintf(buffer + n, "    <port>%" PRIu16 "</port>\r\n",
      appSettings.proxy.port);
   //End of proxy settings
   n += sprintf(buffer + n, "  </proxy>\r\n");

  //End of settings
   n += sprintf(buffer + n, "</settings>\r\n");

   //Format HTTP response header
   connection->response.version = connection->request.version;
   connection->response.statusCode = 200;
   connection->response.keepAlive = connection->request.keepAlive;
   connection->response.noCache = TRUE;
   connection->response.contentType = mimeGetType(".xml");
   connection->response.chunkedEncoding = FALSE;
   connection->response.contentLength = n;

   //Send the header to the client
   error = httpWriteHeader(connection);

   //Check status code
   if(!error)
   {
      //Send response body
      error = httpWriteStream(connection, buffer, n);
   }

   //Check status code
   if(!error)
   {
      //Properly close output stream
      error = httpCloseStream(connection);
   }

   //Free previously allocated memory
   osFreeMem(buffer);
   //Return status code
   return error;
}


/**
 * @brief Process get_config.xml request
 * @param[in] connection Handle referencing a client connection
 * @return Error code
 **/

error_t httpServerProcessSetConfig(HttpConnection *connection)
{
   error_t error;
   uint_t i;
   size_t n;
   char_t *p;
   char_t *buffer;
   char_t *separator;
   char_t *property;
   char_t *value;
   Settings *newSettings;

   //Point to the scratch buffer
   buffer = connection->buffer;

   //Allocate a memory buffer to hold the new configuration
   newSettings = osAllocMem(sizeof(Settings));
   //Failed to allocate memory?
   if(!newSettings) return ERROR_OUT_OF_MEMORY;

   //Start of exception handling block
   do
   {
      //Retrieve default settings
      error = getDefaultSettings(newSettings);
      //Any error to report?
      if(error) break;

      //Process HTTP request body
      while(1)
      {
         //Read the HTTP request body until an ampersand is encountered
         error = httpReadStream(connection, buffer,
            HTTP_SERVER_BUFFER_SIZE - 1, &n, HTTP_FLAG_BREAK('&'));
         //End of stream detected?
         if(error) break;

         //Properly terminate the string with a NULL character
         buffer[n] = '\0';

         //Remove the trailing ampersand
         if(n > 0 && buffer[n - 1] == '&')
            buffer[--n] = '\0';

         //Decode the percent-encoded string
         error = httpDecodePercentEncodedString(buffer, buffer, HTTP_SERVER_BUFFER_SIZE);
         //Any error detected?
         if(error) break;

         //Check whether a separator is present
         separator = strchr(buffer, '=');

         //Separator found?
         if(separator)
         {
            //Split the line
            *separator = '\0';
            //Get property name and value
            property = strTrimWhitespace(buffer);
            value = strTrimWhitespace(separator + 1);

            //Debug message
            TRACE_DEBUG("[%s]=%s\r\n", property, value);

            //Icecast settings
            if(!strcasecmp(property, "icecastSettingsUrl"))
            {
               //Check resource length
               if(strlen(value) >= sizeof(newSettings->icecast.url))
               {
                  //Report an error
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }

               //Save resource
               strcpy(newSettings->icecast.url, value);
            }
            else if(!strcasecmp(property, "icecastSettingsPort"))
            {
               //Save Icecast server port
               newSettings->icecast.port = strtoul(value, &p, 10);

               //Invalid port number?
               if(*p != '\0')
               {
                  //Report an error
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }
            }
            //LAN settings
            else if(!strcasecmp(property, "lanSettingsMacAddr"))
            {
               //Save MAC address
               error = macStringToAddr(value, &newSettings->lan.macAddr);
               //Invalid address?
               if(error) break;
            }
            else if(!strcasecmp(property, "lanSettingsHostName"))
            {
               //Check the length of the host name
               if(strlen(value) >= sizeof(newSettings->lan.hostname))
               {
                  //Report an error
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }

               //Save host name
               strcpy(newSettings->lan.hostname, value);
            }
            else if(!strcasecmp(property, "lanSettingsEnableDhcp"))
            {
               //Check flag value
               if(!strcasecmp(value, "off"))
               {
                  //DHCP client is disabled
                  newSettings->lan.enableDhcp = FALSE;
               }
               else if(!strcasecmp(value, "on"))
               {
                  //DHCP client is enabled
                  newSettings->lan.enableDhcp = TRUE;
               }
               else
               {
                  //Invalid value
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }
            }
            else if(!strcasecmp(property, "lanSettingsHostAddr"))
            {
               //Save IPv4 host address
               error = ipv4StringToAddr(value, &newSettings->lan.hostAddr);
               //Invalid address?
               if(error) break;
            }
            else if(!strcasecmp(property, "lanSettingsSubnetMask"))
            {
               //Save subnet mask
               error = ipv4StringToAddr(value, &newSettings->lan.subnetMask);
               //Invalid mask?
               if(error) break;
            }
            else if(!strcasecmp(property, "lanSettingsDefaultGateway"))
            {
               //Save default gateway
               error = ipv4StringToAddr(value, &newSettings->lan.defaultGateway);
               //Invalid address?
               if(error) break;
            }
            else if(!strcasecmp(property, "lanSettingsPrimaryDns"))
            {
               //Save primary DNS
               error = ipv4StringToAddr(value, &newSettings->lan.primaryDns);
               //Invalid address?
               if(error) break;
            }
            else if(!strcasecmp(property, "lanSettingsSecondaryDns"))
            {
               //Save secondary DNS
               error = ipv4StringToAddr(value, &newSettings->lan.secondaryDns);
               //Invalid address?
               if(error) break;
            }
            //Proxy settings
            else if(!strcasecmp(property, "proxySettingsEnable"))
            {
               //Check flag value
               if(!strcasecmp(value, "off"))
               {
                  //Proxy server is disabled
                  newSettings->proxy.enable = FALSE;
               }
               else if(!strcasecmp(value, "on"))
               {
                  //Proxy server is enabled
                  newSettings->proxy.enable = TRUE;
               }
               else
               {
                  //Invalid value
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }
            }
            else if(!strcasecmp(property, "proxySettingsName"))
            {
               //Check the length of proxy server name
               if(strlen(value) >= sizeof(newSettings->proxy.name))
               {
                  //Report an error
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }

               //Save proxy server name
               strcpy(newSettings->proxy.name, value);
            }
            else if(!strcasecmp(property, "proxySettingsPort"))
            {
               //Save proxy server port
               newSettings->proxy.port = strtoul(value, &p, 10);

               //Invalid port number?
               if(*p != '\0')
               {
                  //Report an error
                  error = ERROR_INVALID_SYNTAX;
                  break;
               }
            }
         }
      }

      //Check status code
      if(error == NO_ERROR || error == ERROR_END_OF_STREAM)
      {
         //Commit changes
         appSettings = *newSettings;
         //Write settings to non-volatile memory
         error = saveSettings(newSettings);
      }
      else if(error != ERROR_INVALID_SYNTAX)
      {
         //Propagate exception
         break;
      }

      //Point to the scratch buffer
      buffer = connection->buffer + 384;
      //Format XML data
      n = sprintf(buffer, "<data>\r\n  <status>");

      if(error == ERROR_INVALID_SYNTAX)
         n += sprintf(buffer + n, "Invalid configuration!\r\n");
      else if(error != NO_ERROR)
         n += sprintf(buffer + n, "Failed to save settings to non-volatile memory!\r\n");
      else
         n += sprintf(buffer + n, "Settings successfully saved. Please reboot the board.\r\n");

      //Terminate XML data
      n += sprintf(buffer + n, "</status>\r\n</data>\r\n");

      //Format HTTP response header
      connection->response.version = connection->request.version;
      connection->response.statusCode = 200;
      connection->response.keepAlive = connection->request.keepAlive;
      connection->response.noCache = TRUE;
      connection->response.contentType = mimeGetType(".xml");
      connection->response.chunkedEncoding = FALSE;
      connection->response.contentLength = n;

      //Send the header to the client
      error = httpWriteHeader(connection);
      //Any error to report?
      if(error) break;

      //Send response body
      error = httpWriteStream(connection, buffer, n);
      //Any error to report?
      if(error) break;

      //Properly close output stream
      error = httpCloseStream(connection);
      //Any error to report?
      if(error) break;

      //End of exception handling block
   } while(0);

   //Free previously allocated memory
   osFreeMem(newSettings);
   //Return status code
   return error;
}
