#include "arduino.h"
#include "shtc1.h"
#include "rtl_lib.h"


void setup()
{
  int error;
  uint16_t shtc1_id;
 
  delay(10000);  
  DiagPrintf("setup \r\n"); 
  error = SHTC1_Init(&shtc1_id);
  if ( error == NO_ERROR ) {
    DiagPrintf("SHTC1 init ok, id=%d\r\n", shtc1_id);
  } else {
    DiagPrintf("SHTC1 init FAILED! \r\n");
    for(;;);
  }
}


void loop()
{
  int   error;
  float temperature = 0.0f;
  float humidity = 0.0f;
 

  error = SHTC1_GetTempAndHumi(&temperature, &humidity);
  
  rtl_printf("temp=%f, humidity=%f, error=%d\n", 
            temperature, humidity, error);
  
   delay(1000);
}

