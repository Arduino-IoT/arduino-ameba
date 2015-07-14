/*
  wifi_drv.cpp - Library for Arduino Wifi shield.
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "variant.h"

#include "Arduino.h"


#include "lwip/err.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "rt_os_service.h"

#include "wifi_intfs.h"

#include "wifi_drv1.h"




/* TCP/IP and Network Interface Initialisation */
static char mac_addr[19];
//static char ip_addr[17] = "\0";
static char gateway[17] = "\0";
static char networkmask[17] = "\0";
static bool use_dhcp = false;
static bool if_enabled = false;

//static Semaphore tcpip_inited(0);



//static Semaphore netif_linked(0);
//static Semaphore netif_up(0);




// Private Methods

//



// TODO:  API to set mac address
// Hal_EfuseParseMACAddr_8195A
static void set_mac_address(char *mac) {
	mac[0] = 0x00;
	mac[1] = 0xE0;
	mac[2] = 0x4C;
	mac[3] = 0xB7;
	mac[4] = 0x23;
	mac[5] = 0x00;
}







// Public Methods


void WiFiDrv1::wifiDriverInit()
{
	use_dhcp = true;
	set_mac_address(mac_addr);
	wl_wifi_init(&padapter, &pnetif);
}



int8_t WiFiDrv1::startScanNetworks()
{
	Serial.println("WiFiDrv1.startScanNetworks()");

	wl_wifi_scan(padapter);
	return WL_SUCCESS;
}


uint8_t WiFiDrv1::getScanNetworks()
{
	Serial.println("WiFiDrv1.getScanNetworks()");
	return 1;

}

int8_t WiFiDrv1::wifiSetPassphrase(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t len)
{
	wl_err_t ret = WL_SUCCESS;

	if ( passphrase == NULL || len == 0 ) 
		ret = wl_wifi_connect(padapter, ssid, ssid_len,RTW_SECURITY_OPEN, passphrase, len);
	else
		ret = wl_wifi_connect(padapter, ssid, ssid_len,RTW_SECURITY_WPA2_AES_PSK, passphrase, len);
	
	if ( ret == WL_SUCCESS ) 
		if_enabled = true;
    return ret;
}



WiFiDrv1 wiFiDrv1;

