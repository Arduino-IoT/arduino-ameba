// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// Version 3.7.2 modified on Dec 6, 2011 to support Arduino 1.0
// See Includes...
// Modified by Jordan Hochenbaum

#include "DallasTemperature.h"

#include "Arduino.h"   

DallasTemperature::DallasTemperature()
{
}


// initialise the bus
void DallasTemperature::begin(OneWire* _oneWire)
{
  DeviceAddress deviceAddress;

  DiagPrintf("DallasTemperature begin \r\n");
  
  _wire = _oneWire;
  devices = 0;
  parasite = false;
  bitResolution = 9;
  waitForConversion = true;
  checkForConversion = true;


  _wire->reset_search();
  devices = 0; // Reset the number of devices when we enumerate wire devices

  while (_wire->search(deviceAddress))
  {
    if (validAddress(deviceAddress))
    {
      if (!parasite && readPowerSupply(deviceAddress)) parasite = true;

      ScratchPad scratchPad;

      readScratchPad(deviceAddress, scratchPad);

	  bitResolution = max(bitResolution, getResolution(deviceAddress));

      devices++;
    }
  }
}

// returns the number of devices found on the bus
uint8_t DallasTemperature::getDeviceCount(void)
{
  return devices;
}

// returns true if address is valid
bool DallasTemperature::validAddress(uint8_t* deviceAddress)
{
  return (_wire->crc8(deviceAddress, 7) == deviceAddress[7]);
}

// finds an address at a given index on the bus
// returns true if the device was found
bool DallasTemperature::getAddress(uint8_t* deviceAddress, uint8_t index)
{
  uint8_t depth = 0;

  _wire->reset_search();

  while (depth <= index && _wire->search(deviceAddress))
  {
    if (depth == index && validAddress(deviceAddress)) return true;
    depth++;
  }

  return false;
}

// attempt to determine if the device at the given address is connected to the bus
bool DallasTemperature::isConnected(uint8_t* deviceAddress)
{
  ScratchPad scratchPad;
  return isConnected(deviceAddress, scratchPad);
}

// attempt to determine if the device at the given address is connected to the bus
// also allows for updating the read scratchpad
bool DallasTemperature::isConnected(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  readScratchPad(deviceAddress, scratchPad);
  for (int i=0; i<8; i++) 
  	DiagPrintf("scratchPad[%d] = 0x%x \r\n", i, scratchPad[i]);
  DiagPrintf(" CRC cal=0x%x, scratch=0x%x \r\n", _wire->crc8(scratchPad, 8), scratchPad[SCRATCHPAD_CRC]); 
  return (_wire->crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

// read device's scratch pad
void DallasTemperature::readScratchPad(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  // send the command
  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(READSCRATCH);

  // TODO => collect all comments &  use simple loop
  // byte 0: temperature LSB  
  // byte 1: temperature MSB
  // byte 2: high alarm temp
  // byte 3: low alarm temp
  // byte 4: DS18S20: store for crc
  //         DS18B20 & DS1822: configuration register
  // byte 5: internal use & crc
  // byte 6: DS18S20: COUNT_REMAIN
  //         DS18B20 & DS1822: store for crc
  // byte 7: DS18S20: COUNT_PER_C
  //         DS18B20 & DS1822: store for crc
  // byte 8: SCRATCHPAD_CRC
  //
  // for(int i=0; i<9; i++)
  // {
  //   scratchPad[i] = _wire->read();
  // }

  
  // read the response

  // byte 0: temperature LSB
  scratchPad[TEMP_LSB] = _wire->read();

  // byte 1: temperature MSB
  scratchPad[TEMP_MSB] = _wire->read();

  // byte 2: high alarm temp
  scratchPad[HIGH_ALARM_TEMP] = _wire->read();

  // byte 3: low alarm temp
  scratchPad[LOW_ALARM_TEMP] = _wire->read();

  // byte 4:
  // DS18S20: store for crc
  // DS18B20 & DS1822: configuration register
  scratchPad[CONFIGURATION] = _wire->read();

  // byte 5:
  // internal use & crc
  scratchPad[INTERNAL_BYTE] = _wire->read();

  // byte 6:
  // DS18S20: COUNT_REMAIN
  // DS18B20 & DS1822: store for crc
  scratchPad[COUNT_REMAIN] = _wire->read();

  // byte 7:
  // DS18S20: COUNT_PER_C
  // DS18B20 & DS1822: store for crc
  scratchPad[COUNT_PER_C] = _wire->read();

  // byte 8:
  // SCTRACHPAD_CRC
  scratchPad[SCRATCHPAD_CRC] = _wire->read();
  DiagPrintf("%s : CRC : %d \r\n", __FUNCTION__, scratchPad[SCRATCHPAD_CRC]);

  _wire->reset();
}

// writes device's scratch pad
void DallasTemperature::writeScratchPad(uint8_t* deviceAddress, const uint8_t* scratchPad)
{
  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(WRITESCRATCH);
  _wire->write(scratchPad[HIGH_ALARM_TEMP]); // high alarm temp
  _wire->write(scratchPad[LOW_ALARM_TEMP]); // low alarm temp
  // DS18S20 does not use the configuration register
  if (deviceAddress[0] != DS18S20MODEL) _wire->write(scratchPad[CONFIGURATION]); // configuration
  _wire->reset();
  // save the newly written values to eeprom
  _wire->write(COPYSCRATCH, parasite);
  if (parasite) delay(10); // 10ms delay
  _wire->reset();
}

// reads the device's power requirements
bool DallasTemperature::readPowerSupply(uint8_t* deviceAddress)
{
  bool ret = false;
  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(READPOWERSUPPLY);
  if (_wire->read_bit() == 0) ret = true;
  _wire->reset();
  return ret;
}


// set resolution of all devices to 9, 10, 11, or 12 bits
// if new resolution is out of range, it is constrained.
void DallasTemperature::setResolution(uint8_t newResolution)
{
  bitResolution = constrain(newResolution, 9, 12);
  DeviceAddress deviceAddress;
  for (int i=0; i<devices; i++)
  {
    getAddress(deviceAddress, i);
	setResolution(deviceAddress, bitResolution);
  }
}

// set resolution of a device to 9, 10, 11, or 12 bits
// if new resolution is out of range, 9 bits is used. 
bool DallasTemperature::setResolution(uint8_t* deviceAddress, uint8_t newResolution)
{
  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad))
  {
    // DS18S20 has a fixed 9-bit resolution
    if (deviceAddress[0] != DS18S20MODEL)
    {
      switch (newResolution)
      {
        case 12:
          scratchPad[CONFIGURATION] = TEMP_12_BIT;
          break;
        case 11:
          scratchPad[CONFIGURATION] = TEMP_11_BIT;
          break;
        case 10:
          scratchPad[CONFIGURATION] = TEMP_10_BIT;
          break;
        case 9:
        default:
          scratchPad[CONFIGURATION] = TEMP_9_BIT;
          break;
      }
      writeScratchPad(deviceAddress, scratchPad);
    }
	return true;  // new value set
  }
  return false;
}

// returns the global resolution
uint8_t DallasTemperature::getResolution()
{
	return bitResolution;
}

// returns the current resolution of the device, 9-12
// returns 0 if device not found
uint8_t DallasTemperature::getResolution(uint8_t* deviceAddress)
{
  if (deviceAddress[0] == DS18S20MODEL) return 9; // this model has a fixed resolution

  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad))
  {
	switch (scratchPad[CONFIGURATION])
    {
      case TEMP_12_BIT:
        return 12;
        
      case TEMP_11_BIT:
        return 11;
        
      case TEMP_10_BIT:
        return 10;
        
      case TEMP_9_BIT:
        return 9;
        
	}
  }
  return 0;
}


// sets the value of the waitForConversion flag
// TRUE : function requestTemperature() etc returns when conversion is ready
// FALSE: function requestTemperature() etc returns immediately (USE WITH CARE!!)
// 		  (1) programmer has to check if the needed delay has passed 
//        (2) but the application can do meaningful things in that time
void DallasTemperature::setWaitForConversion(bool flag)
{
	waitForConversion = flag;
}

// gets the value of the waitForConversion flag
bool DallasTemperature::getWaitForConversion()
{
	return waitForConversion;
}


// sets the value of the checkForConversion flag
// TRUE : function requestTemperature() etc will 'listen' to an IC to determine whether a conversion is complete
// FALSE: function requestTemperature() etc will wait a set time (worst case scenario) for a conversion to complete
void DallasTemperature::setCheckForConversion(bool flag)
{
	checkForConversion = flag;
}

// gets the value of the waitForConversion flag
bool DallasTemperature::getCheckForConversion()
{
	return checkForConversion;
}

bool DallasTemperature::isConversionAvailable(uint8_t* deviceAddress)
{
	// Check if the clock has been raised indicating the conversion is complete
  	ScratchPad scratchPad;
  	readScratchPad(deviceAddress, scratchPad);
	return scratchPad[0];
}	


// sends command for all devices on the bus to perform a temperature conversion
void DallasTemperature::requestTemperatures()
{
  _wire->reset();
  _wire->skip();
  _wire->write(STARTCONVO, parasite);

  // ASYNC mode?
  if (!waitForConversion) return; 
  blockTillConversionComplete(&bitResolution, 0);

  return;
}

// sends command for one device to perform a temperature by address
// returns FALSE if device is disconnected
// returns TRUE  otherwise
bool DallasTemperature::requestTemperaturesByAddress(uint8_t* deviceAddress)
{

  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(STARTCONVO, parasite);
  
    // check device
  ScratchPad scratchPad;
  if (!isConnected(deviceAddress, scratchPad)) return false;
  
  
  // ASYNC mode?
  if (!waitForConversion) return true;   
  uint8_t bitResolution = getResolution(deviceAddress);
  blockTillConversionComplete(&bitResolution, deviceAddress);
  
  return true;
}


void DallasTemperature::blockTillConversionComplete(uint8_t* bitResolution, uint8_t* deviceAddress)
{
	if(deviceAddress != 0 && checkForConversion && !parasite)
	{
	  	// Continue to check if the IC has responded with a temperature
	  	// NB: Could cause issues with multiple devices (one device may respond faster)
	  	unsigned long start = millis();
		while(!isConversionAvailable(0) && ((millis() - start) < 750));	
	}
	
  	// Wait a fix number of cycles till conversion is complete (based on IC datasheet)
	  switch (*bitResolution)
	  {
	    case 9:
	      delay(94);
	      break;
	    case 10:
	      delay(188);
	      break;
	    case 11:
	      delay(375);
	      break;
	    case 12:
	    default:
	      delay(750);
	      break;
	  }

}

// sends command for one device to perform a temp conversion by index
bool DallasTemperature::requestTemperaturesByIndex(uint8_t deviceIndex)
{
  DeviceAddress deviceAddress;
  getAddress(deviceAddress, deviceIndex);
  return requestTemperaturesByAddress(deviceAddress);
}

// Fetch temperature for device index
float DallasTemperature::getTempCByIndex(uint8_t deviceIndex)
{
  DeviceAddress deviceAddress;
  getAddress(deviceAddress, deviceIndex);
  return getTempC((uint8_t*)deviceAddress);
}

// Fetch temperature for device index
float DallasTemperature::getTempFByIndex(uint8_t deviceIndex)
{
  return toFahrenheit(getTempCByIndex(deviceIndex));
}

// reads scratchpad and returns the temperature in degrees C
float DallasTemperature::calculateTemperature(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  int16_t rawTemperature = (((int16_t)scratchPad[TEMP_MSB]) << 8) | scratchPad[TEMP_LSB];

  switch (deviceAddress[0])
  {
    case DS18B20MODEL:
    case DS1822MODEL:
      switch (scratchPad[CONFIGURATION])
      {
        case TEMP_12_BIT:
          return (float)rawTemperature * 0.0625;
          break;
        case TEMP_11_BIT:
          return (float)(rawTemperature >> 1) * 0.125;
          break;
        case TEMP_10_BIT:
          return (float)(rawTemperature >> 2) * 0.25;
          break;
        case TEMP_9_BIT:
          return (float)(rawTemperature >> 3) * 0.5;
          break;
      }
      break;
    case DS18S20MODEL:
      /*

      Resolutions greater than 9 bits can be calculated using the data from
      the temperature, COUNT REMAIN and COUNT PER �C registers in the
      scratchpad. Note that the COUNT PER �C register is hard-wired to 16
      (10h). After reading the scratchpad, the TEMP_READ value is obtained
      by truncating the 0.5�C bit (bit 0) from the temperature data. The
      extended resolution temperature can then be calculated using the
      following equation:

                                       COUNT_PER_C - COUNT_REMAIN
      TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                               COUNT_PER_C
      */

      // Good spot. Thanks Nic Johns for your contribution
      return (float)(rawTemperature >> 1) - 0.25 +((float)(scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) / (float)scratchPad[COUNT_PER_C] );
      break;
  }
}

// returns temperature in degrees C or DEVICE_DISCONNECTED if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
float DallasTemperature::getTempC(uint8_t* deviceAddress)
{
  // TODO: Multiple devices (up to 64) on the same bus may take 
  //       some time to negotiate a response
  // What happens in case of collision?

  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad)) return calculateTemperature(deviceAddress, scratchPad);
  return DEVICE_DISCONNECTED;
}

// returns temperature in degrees F
// TODO: - when getTempC returns DEVICE_DISCONNECTED 
//        -127 gets converted to -196.6 F
float DallasTemperature::getTempF(uint8_t* deviceAddress)
{
  return toFahrenheit(getTempC(deviceAddress));
}

// returns true if the bus requires parasite power
bool DallasTemperature::isParasitePowerMode(void)
{
  return parasite;
}


// Convert float celsius to fahrenheit
float DallasTemperature::toFahrenheit(float celsius)
{
  return (celsius * 1.8) + 32;
}

// Convert float fahrenheit to celsius
float DallasTemperature::toCelsius(float fahrenheit)
{
  return (fahrenheit - 32) / 1.8;
}

