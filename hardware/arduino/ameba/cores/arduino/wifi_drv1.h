/*
  wifi_drv.h - Library for Arduino Wifi shield.
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

#ifndef WiFi_Drv1_h
#define WiFi_Drv1_h

#include <inttypes.h>

#include "lwip/err.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"

#include "wifi_intfs.h"
#include "wl_definitions.h"

class WiFiDrv1
{
public:


	void* padapter;
	void* pnetif;

    /*
     * Driver initialization
     */
    void wifiDriverInit();


    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int8_t startScanNetworks();

    /*
     * Get the networks available
     *
     * return: Number of discovered networks
     */
    uint8_t getScanNetworks();

	char* getSSIDNetoworks(uint8_t networkItem);
	int32_t getRSSINetoworks(uint8_t networkItem);
	
	uint8_t getEncTypeNetowrks(uint8_t networkItem);

    /* Start Wifi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param ssid_len: Lenght of ssid string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     * param len: Lenght of passphrase string.
     * return: WL_SUCCESS or WL_FAILURE
     */
   int8_t wifiSetPassphrase(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t len);

   uint8_t* getMacAddress();

};

extern WiFiDrv1 wiFiDrv1;

#endif
