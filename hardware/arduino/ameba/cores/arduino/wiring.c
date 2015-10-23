/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"
#include "rt_Time.h"

#include "us_ticker_api.h"
#include "section_config.h"

IMAGE2_TEXT_SECTION
void delay( uint32_t ms )
{
	osStatus ret;

	//HalDelayUs(ms*1000);
	ret = osDelay(ms);
	if ( (ret != osEventTimeout) && (ret != osOK) ) {
		DiagPrintf("delay : ERROR : 0x%x \n", ret);
	}
}


IMAGE2_TEXT_SECTION
void delayMicroseconds(uint32_t us)
{
	rtw_udelay_os(us);
}


IMAGE2_TEXT_SECTION
uint32_t millis( void )
{
    // OS_TICK = 1000, so the same as ticks
    return rt_time_get() ;
}


IMAGE2_TEXT_SECTION
uint32_t micros( void ) 
{
	return us_ticker_read();
}


#ifdef __cplusplus
}
#endif
