#include "variant.h"
#include "shtc1.h"

#include "wire.h"

//
// SHTC1
//

#define I2C_ADR_SHTC1 0x70
#define POLYNOMIAL 0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

// Sensor Commands
#define READ_ID              0xEFC8 // command: read ID register
#define SOFT_RESET           0x805D // soft resetSample Code for SHTC1
#define MEAS_T_RH_POLLING    0x7866 // meas. read T first, clock stretching disabled
#define MEAS_T_RH_CLOCKSTR   0x7CA2 // meas. read T first, clock stretching enabled
#define MEAS_RH_T_POLLING    0x58E0 // meas. read RH first, clock stretching disabled
#define MEAS_RH_T_CLOCKSTR   0x5C24 // meas. read RH first, clock stretching enabled

static int SHTC1_GetID(uint16_t *id);


int SHTC1_Init(uint16_t *pID)
{
  int error = NO_ERROR;
 
  Wire1.begin();
 
  if (pID == NULL ) return NULL_ERROR;
  
  error = SHTC1_GetID(pID);
  return error;
}

static int SHTC1_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
   uint8_t bit; // bit mask
   uint8_t crc = 0xFF; // calculated checksum
   uint8_t byteCtr; // byte counter

   // calculates 8-Bit checksum with given polynomial
   for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
   {
     crc ^= (data[byteCtr]);
     for(bit = 8; bit > 0; --bit)
     {
       if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
       else crc = (crc << 1);
     }
  }

   // verify checksum
   if(crc != checksum) return CHECKSUM_ERROR;
   else return NO_ERROR;
}

static void SHTC1_WriteCommand(uint16_t cmd)
{
  Wire1.beginTransmission(I2C_ADR_SHTC1);
  Wire1.write(cmd>>8);
  Wire1.write(cmd&0xFF);
  Wire1.endTransmission();
}  

static int SHTC1_Read2BytesAndCrc(uint16_t *data)
{
  int error;
  uint8_t bytes[2];
  uint8_t checksum;
  
  bytes[0] = Wire1.read();
  bytes[1] = Wire1.read();
  checksum = Wire1.read();

  error = SHTC1_CheckCrc(bytes, 2, checksum);
  *data = (bytes[0] << 8) | bytes[1];  
  
  return error;
}

static int SHTC1_GetID(uint16_t *id)
{
  int error = NO_ERROR;
  uint8_t bytes[2];
  uint8_t checksum;

  i2c_t *pI2CMaster = Wire1.getI2CMaster();

  SHTC1_WriteCommand(READ_ID);  

  Wire1.requestFrom(I2C_ADR_SHTC1, 3);

  error = SHTC1_Read2BytesAndCrc(id);

  return error;
}

static float SHTC1_CalcTemperature(uint16_t rawValue)
{
  return 175.0 * (float)rawValue / 65536.0 - 45.0;
}

static float SHTC1_CalcHumidity(uint16_t rawValue)
{
  return 100.0 * (float)rawValue / 65536.0;
} 


int SHTC1_GetTempAndHumi(float *temp, float *humi)
{
  int error;
  uint16_t rawValueTemp;
  uint16_t rawValueHumi;
  
  i2c_t *pI2CMaster = Wire1.getI2CMaster();

  SHTC1_WriteCommand(MEAS_T_RH_CLOCKSTR);  

  Wire1.requestFrom(I2C_ADR_SHTC1, 6); 

  error = NO_ERROR;
  error |= SHTC1_Read2BytesAndCrc(&rawValueTemp);
  error |= SHTC1_Read2BytesAndCrc(&rawValueHumi);

  //diag_printf("raw temp=0x%x, raw humidity=0x%x, error=%d\n", 
  //          rawValueTemp, rawValueHumi, error);
 
  if ( error == NO_ERROR ) {
    *temp = SHTC1_CalcTemperature(rawValueTemp);
    *humi = SHTC1_CalcHumidity(rawValueHumi);
  }
  
  return error;
}


