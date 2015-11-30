/**
 * @file radio_stations.h
 * @brief List of Internet radio station
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

#ifndef _RADIO_STATIONS_H
#define _RADIO_STATIONS_H

//Capital FM (UK)
#if 1
   //Icecast server name
   #define ICECAST_SERVER_NAME "ice-sov.musicradio.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/CapitalMP3"
#endif

//Classic FM (UK)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "ice-sov.musicradio.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/ClassicFMMP3"
#endif

//RTL (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "streaming.radio.rtl.fr"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/rtl-1-44-96"
#endif

//RTL2 (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "streaming.radio.rtl2.fr"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/rtl2-1-44-96"
#endif

//OUI FM (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "ouifm.ice.infomaniak.ch"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/ouifm-high.mp3"
#endif

//Le Mouv' (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "mp3lg-out-b.tdf-cdn.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/lemouv/all/lemouvhautdebit.mp3"
#endif

//France Info (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "mp3lg-out-b.tdf-cdn.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/franceinter/all/franceinterhautdebit.mp3"
#endif

//FIP (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "mp3lg-out-b.tdf-cdn.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/fip/all/fiphautdebit.mp3"
#endif

//Couleur 3 (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "streaming.swisstxt.ch"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/m/couleur3/mp3_128"
#endif

//Alpes 1 (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "alpes1grenoble.ice.infomaniak.ch"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/alpes1grenoble-high.mp3"
#endif

//Alouette Radio (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "broadcast.infomaniak.net"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/alouette-high.mp3"
#endif

//Radio Classique (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "str20.creacast.com"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/classique1"
#endif

//Radio Nova (FR)
#if 0
   //Icecast server name
   #define ICECAST_SERVER_NAME "ice3.infomaniak.ch"
   //Icecast server port
   #define ICECAST_SERVER_PORT 80
   //Requested resource
   #define ICECAST_RESOURCE "/radionova-high.mp3"
#endif

#endif
