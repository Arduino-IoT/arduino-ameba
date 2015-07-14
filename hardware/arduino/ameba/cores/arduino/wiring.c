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
#include "rt_time.h"


void delay( uint32_t ms )
{

	//NeoJou
	
	osStatus ret;

	//HalDelayUs(ms*1000);
	ret = osDelay(ms);
	if ( (ret != osEventTimeout) && (ret != osOK) ) {
		DiagPrintf("delay : ERROR : 0x%x \n", ret);
	}

}

void delayMicroseconds(uint32_t usec)
{
	HalDelayUs(usec);
}

uint32_t millis( void )
{
    // OS_TICK = 1000, so the same as ticks
    return rt_time_get() ;
}




#ifdef __cplusplus
}
#endif
