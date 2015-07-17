#include "arduino.h"
#include "Wire.h"
#include "DS1307.h"

#define DS1307_CTRL_ID 0x68 

uint8_t rtc_bcd[7];


void DS1307_init()
{
  Wire1.begin();
}

void DS1307_get(int *rtc, boolean refresh)
{
  if (refresh) DS1307_read();
  for(int i=0;i<7;i++)  // cycle through each component, create array of data
  {
	rtc[i]= DS1307_get_data(i, 0);
  }  
}

int DS1307_get_data(int c, boolean refresh)  // aquire individual RTC item from buffer, return as int, refresh buffer if required
{
  if(refresh) DS1307_read();
  int v=-1;
  switch(c)
  {
  case DS1307_SEC:
    v=(10*((rtc_bcd[DS1307_SEC] & DS1307_HI_SEC)>>4))+(rtc_bcd[DS1307_SEC] & DS1307_LO_BCD);
	break;
  case DS1307_MIN:
    v=(10*((rtc_bcd[DS1307_MIN] & DS1307_HI_MIN)>>4))+(rtc_bcd[DS1307_MIN] & DS1307_LO_BCD);
	break;
  case DS1307_HR:
    v=(10*((rtc_bcd[DS1307_HR] & DS1307_HI_HR)>>4))+(rtc_bcd[DS1307_HR] & DS1307_LO_BCD);
	break;
  case DS1307_DOW:
    v=rtc_bcd[DS1307_DOW] & DS1307_LO_DOW;
	break;
  case DS1307_DATE:
    v=(10*((rtc_bcd[DS1307_DATE] & DS1307_HI_DATE)>>4))+(rtc_bcd[DS1307_DATE] & DS1307_LO_BCD);
	break;
  case DS1307_MTH:
    v=(10*((rtc_bcd[DS1307_MTH] & DS1307_HI_MTH)>>4))+(rtc_bcd[DS1307_MTH] & DS1307_LO_BCD);
	break;
  case DS1307_YR:
    v=(10*((rtc_bcd[DS1307_YR] & DS1307_HI_YR)>>4))+(rtc_bcd[DS1307_YR] & DS1307_LO_BCD)+DS1307_BASE_YR;
	break;
  } // end switch
  return v;
}

void DS1307_read(void)
{
  // use the Wire lib to connect to tho rtc
  // reset the resgiter pointer to zero
  Wire1.beginTransmission(DS1307_CTRL_ID);
  Wire1.write(0x00);
  Wire1.endTransmission();

  // request the 7 bytes of data    (secs, min, hr, dow, date. mth, yr)
  Wire1.requestFrom(DS1307_CTRL_ID, 7);
  for(int i=0; i<7; i++)
  {
    // store data in raw bcd format
    rtc_bcd[i]=Wire1.read();
  }
}

void DS1307_stop(void)
{
	// set the ClockHalt bit high to stop the rtc
	// this bit is part of the seconds byte
    rtc_bcd[DS1307_SEC]=rtc_bcd[DS1307_SEC] | DS1307_CLOCKHALT;
    DS1307_save();
}

void DS1307_save(void)
{
  Wire1.beginTransmission(DS1307_CTRL_ID);
  Wire1.write(0x00); // reset register pointer
  for(int i=0; i<7; i++)
  {
    Wire1.write(rtc_bcd[i]);
  }
  Wire1.endTransmission();
}

void DS1307_set_data(int c, int v)  // Update buffer, then update the chip
{
  switch(c)
  {
  case DS1307_SEC:
    if(v<60 && v>-1)
    {
	//preserve existing clock state (running/stopped)
	int state=rtc_bcd[DS1307_SEC] & DS1307_CLOCKHALT;
	rtc_bcd[DS1307_SEC]=state | ((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_MIN:
    if(v<60 && v>-1)
    {
	rtc_bcd[DS1307_MIN]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_HR:
  // TODO : AM/PM  12HR/24HR
    if(v<24 && v>-1)
    {
	rtc_bcd[DS1307_HR]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_DOW:
    if(v<8 && v>-1)
    {
	rtc_bcd[DS1307_DOW]=v;
    }
    break;
  case DS1307_DATE:
    if(v<31 && v>-1)
    {
	rtc_bcd[DS1307_DATE]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_MTH:
    if(v<13 && v>-1)
    {
	rtc_bcd[DS1307_MTH]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_YR:
    if(v<13 && v>-1)
    {
	rtc_bcd[DS1307_YR]=((v / 10)<<4) + (v % 10);
    }
    break;
  } // end switch
  DS1307_save();
}

void DS1307_start(void)
{
	// unset the ClockHalt bit to start the rtc
	// TODO : preserve existing seconds
    rtc_bcd[DS1307_SEC]=rtc_bcd[DS1307_SEC] & (~DS1307_CLOCKHALT);
    DS1307_save();
}

void DS1307_SetOutput(int c)  
{
  int out;
  switch(c)
  {
  case HIGH :
	out=DS1307_HIGH_BIT;
  break;
  case LOW :
	out=DS1307_LOW_BIT;
  break;
  case DS1307_SQW1HZ :
	out=DS1307_SQW1HZ_BIT;
  break;
  case DS1307_SQW4KHZ :
	out=DS1307_SQW4KHZ_BIT;
  break;
  case DS1307_SQW8KHZ :
	out=DS1307_SQW8KHZ_BIT;
  break;
  case DS1307_SQW32KHZ :
	out=DS1307_SQW32KHZ_BIT;
  break;
  default: 
	out=DS1307_LOW_BIT;
  }
  
  Wire1.beginTransmission(DS1307_CTRL_ID);
  Wire1.write(0x07); 
  Wire1.write(out);
  Wire1.endTransmission();
}

