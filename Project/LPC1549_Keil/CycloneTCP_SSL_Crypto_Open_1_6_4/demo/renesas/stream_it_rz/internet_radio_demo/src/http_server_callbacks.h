/**
 * @file http_server_callbacks.h
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

#ifndef _HTTP_SERVER_CALLBACKS_H
#define _HTTP_SERVER_CALLBACKS_H

//Dependencies
#include "core/net.h"
#include "http/http_server.h"

//HTTP server callbacks
error_t httpServerUriNotFoundCallback(HttpConnection *connection,
   const char_t *uri);

error_t httpServerProcessGetConfig(HttpConnection *connection);
error_t httpServerProcessSetConfig(HttpConnection *connection);

#endif
