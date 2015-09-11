//
//    FILE: dht.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.13
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// 0.1.13 fix negative temperature
// 0.1.12 support DHT33 and DHT44 initial version
// 0.1.11 renamed DHTLIB_TIMEOUT
// 0.1.10 optimized faster WAKEUP + TIMEOUT
// 0.1.09 optimize size: timeout check + use of mask
// 0.1.08 added formula for timeout based upon clockspeed
// 0.1.07 added support for DHT21
// 0.1.06 minimize footprint (2012-12-27)
// 0.1.05 fixed negative temperature bug (thanks to Roseman)
// 0.1.04 improved readability of code using DHTLIB_OK in code
// 0.1.03 added error values for temp and humidity when read failed
// 0.1.02 added error codes
// 0.1.01 added support for Arduino 1.0, fixed typos (31/12/2011)
// 0.1.0 by Rob Tillaart (01/04/2011)
//
// inspired by DHT11 library
//
// Released to the public domain
//

#include "DHT1.h"

extern "C" {
#include "rt_os_service.h"
}

/////////////////////////////////////////////////////
//
// PUBLIC
//



// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
int DHT1::read(uint8_t pin)
{
    // READ VALUES
    int rv = _readSensor(pin, DHTLIB_DHT_WAKEUP);
    if (rv != DHTLIB_OK)
    {
        humidity    = DHTLIB_INVALID_VALUE;  // invalid value, or is NaN prefered?
        temperature = DHTLIB_INVALID_VALUE;  // invalid value
        return rv; // propagate error value
    }

    // CONVERT AND STORE
    humidity = word(bits[0], bits[1]) * 0.1;
    temperature = word(bits[2] & 0x7F, bits[3]) * 0.1;
    if (bits[2] & 0x80)  // negative temperature
    {
        temperature = -temperature;
    }

    // TEST CHECKSUM
    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
    if (bits[4] != sum)
    {
        return DHTLIB_ERROR_CHECKSUM;
    }
    return DHTLIB_OK;
}

#define DHTLIB_TIMEOUT (16700*2)

/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_TIMEOUT
IMAGE2_TEXT_SECTION
int DHT1::_readSensor(uint8_t pin, uint8_t wakeupDelay)
{
    // INIT BUFFERVAR TO RECEIVE DATA
    uint8_t mask = 128;
    uint8_t idx = 0;
	uint32_t start_time, cur_time;
	uint16_t loop_cnt;

    // EMPTY BUFFER
    for (uint8_t i = 0; i < 5; i++) bits[i] = 0;

	cli();
	
    // REQUEST SAMPLE
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(wakeupDelay*1000);
    digitalWrite(pin, HIGH);
	delayMicroseconds(40);
    pinMode(pin, INPUT);

	// keep low for 80 us
	loop_cnt = DHTLIB_TIMEOUT;
    while(digitalRead(pin) == LOW)
    {
		if (--loop_cnt == 0) {
			sti();
			return -2;
		}
    }

	// keep high for 80 us
	loop_cnt = DHTLIB_TIMEOUT;
    while(digitalRead(pin) == HIGH)
    {
		if (--loop_cnt == 0) {
			sti();
			return -3;
		}
    }

    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (uint8_t i = 40; i != 0; i--)
    {
		loop_cnt = DHTLIB_TIMEOUT;
        while(digitalRead(pin) == LOW)
        {
			if (--loop_cnt == 0) {
				sti();
				return -4;
			}
        }

		start_time = micros();		
		loop_cnt = DHTLIB_TIMEOUT;
        while(digitalRead(pin) == HIGH)
        {
			if (--loop_cnt == 0) {
				sti();
				return -5;
			}
        }

        if ((micros() - start_time) > 40)
        { 
            bits[idx] |= mask;
        }
        mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx++;
        }
    }
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);

	sti();
    return DHTLIB_OK;
}
//
// END OF FILE
//

