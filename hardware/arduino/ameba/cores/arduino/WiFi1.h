/*
  WiFi.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WiFi1_h
#define WiFi1_h

#include <inttypes.h>


class WiFiClass1
{
public:

    WiFiClass1();

    void init();


    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int8_t scanNetworks();


	/* Start Wifi connection with passphrase
	 * the most secure supported mode will be automatically selected
	 *
	 * param ssid: Pointer to the SSID string.
	 * param passphrase: Passphrase. Valid characters in a passphrase
	 *		  must be between ASCII 32-126 (decimal).
	 */
	int begin(char* ssid);
	int begin(char* ssid, const char *passphrase);

};

extern WiFiClass1 WiFi1;

#endif
