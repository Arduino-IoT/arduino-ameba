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

#include "pinNames.h"
#include "Objects.h"



static gpio_t gpio_pin_struct[14];

extern void pinMode( uint32_t ulPin, uint32_t ulMode )
{
	//NeoJou
	PinName pinname;

#if 1 //
	gpio_t *pGpio_t;


	//DiagPrintf("[pinMode] ulPin=%d, ulMode=%d\r\n", ulPin, ulMode);
	if ( ulPin < 0 || ulPin > 13 ) return;

	pGpio_t = &gpio_pin_struct[ulPin];

	if ( g_APinDescription[ulPin].ulPinType == PIO_GPIO && 
		 g_APinDescription[ulPin].ulPinMode == ulMode ) {
		 return;
	}

	if ( g_APinDescription[ulPin].ulPinType != PIO_GPIO ) {
		pinname = g_APinDescription[ulPin].pinname;
		//pinname = digital_pinMap[ulPin];
		gpio_init(pGpio_t, pinname);
		g_APinDescription[ulPin].ulPinType = PIO_GPIO;
	}


	switch ( ulMode )
	{
		case INPUT:
			gpio_dir(pGpio_t, PIN_INPUT);	// Direction: Input
			gpio_mode(pGpio_t, PullNone); 	// No pull
			break ;

		case INPUT_PULLUP:
			gpio_dir(pGpio_t, PIN_INPUT);	// Direction: Input
			gpio_mode(pGpio_t, PullUp); 	// Pull Up
			break ;

		case OUTPUT:
			gpio_dir(pGpio_t, PIN_OUTPUT);	// Direction: Output
			gpio_mode(pGpio_t, PullNone); 	// No pull
		break ;

		default:
		break ;
	}
	g_APinDescription[ulPin].ulPinMode = ulMode;
	
#else // orig
	if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
    {
        return ;
    }

	switch ( ulMode )
    {
        case INPUT:
            /* Enable peripheral for clocking input */
            pmc_enable_periph_clk( g_APinDescription[ulPin].ulPeripheralId ) ;
            PIO_Configure(
            	g_APinDescription[ulPin].pPort,
            	PIO_INPUT,
            	g_APinDescription[ulPin].ulPin,
            	0 ) ;
        break ;

        case INPUT_PULLUP:
            /* Enable peripheral for clocking input */
            pmc_enable_periph_clk( g_APinDescription[ulPin].ulPeripheralId ) ;
            PIO_Configure(
            	g_APinDescription[ulPin].pPort,
            	PIO_INPUT,
            	g_APinDescription[ulPin].ulPin,
            	PIO_PULLUP ) ;
        break ;

        case OUTPUT:
            PIO_Configure(
            	g_APinDescription[ulPin].pPort,
            	PIO_OUTPUT_1,
            	g_APinDescription[ulPin].ulPin,
            	g_APinDescription[ulPin].ulPinConfiguration ) ;

            /* if all pins are output, disable PIO Controller clocking, reduce power consumption */
            if ( g_APinDescription[ulPin].pPort->PIO_OSR == 0xffffffff )
            {
                pmc_disable_periph_clk( g_APinDescription[ulPin].ulPeripheralId ) ;
            }
        break ;

        default:
        break ;
    }
#endif
}

extern void digitalWrite( uint32_t ulPin, uint32_t ulVal )
{
//NeoJou
#if 1

	gpio_t *pGpio_t;


	if ( ulPin < 0 || ulPin > 13 ) return;

	/* Handle */
	if ( g_APinDescription[ulPin].ulPinType != PIO_GPIO )
	{
	  return ;
	}

	pGpio_t = &gpio_pin_struct[ulPin];

	gpio_write(pGpio_t, ulVal);


#else // original
  /* Handle */
	if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
  {
    return ;
  }

  if ( PIO_GetOutputDataStatus( g_APinDescription[ulPin].pPort, g_APinDescription[ulPin].ulPin ) == 0 )
  {
    PIO_PullUp( g_APinDescription[ulPin].pPort, g_APinDescription[ulPin].ulPin, ulVal ) ;
  }
  else
  {
    PIO_SetOutput( g_APinDescription[ulPin].pPort, g_APinDescription[ulPin].ulPin, ulVal, 0, PIO_PULLUP ) ;
  }
#endif
}

extern int digitalRead( uint32_t ulPin )
{
//NeoJou
#if 1
	
	gpio_t *pGpio_t;
	
	
	if ( ulPin < 0 || ulPin > 13 ) return;

	/* Handle */
	if ( g_APinDescription[ulPin].ulPinType != PIO_GPIO )
	{
	  return ;
	}

	pGpio_t = &gpio_pin_struct[ulPin];
	
	return gpio_read(pGpio_t);
	
	
#else // original
	if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
    {
        return LOW ;
    }

	if ( PIO_Get( g_APinDescription[ulPin].pPort, PIO_INPUT, g_APinDescription[ulPin].ulPin ) == 1 )
    {
        return HIGH ;
    }

	return LOW ;
#endif
}

#ifdef __cplusplus
}
#endif

