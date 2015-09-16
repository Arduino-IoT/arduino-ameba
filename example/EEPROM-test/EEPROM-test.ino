#include "flashEEPROM.h"

// start reading from the first byte (address 0) of the EEPROM
int address = 0;
uint32_t value;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  flashEEPROM.read_from_flashEEPROM();
}

void read_flashEEPROM(void)
{
  address = 0;
  while (address<flashEEPROM.length()) {
    value = flashEEPROM.data[address];
  
    Serial.print(address);
    Serial.print("\t");
    Serial.print(value, HEX);
    Serial.println();
    
    address = address +1;
  }
}

void write_flashEEPROM(uint8_t val)
{
   address = 0;
   while (address<flashEEPROM.length()) {
      flashEEPROM.data[address] = (address)&0xFF;
      address = address + 1;     
   }
   flashEEPROM.write_to_flashEEPROM();
}

void loop() {
  
  Serial.println("Start to Read");
  read_flashEEPROM();
  Serial.println("wait 500ms then Start to Write");
  delay(500);
  write_flashEEPROM(0xAA);
  Serial.println("Start to Read");
  read_flashEEPROM();  
  delay(1000);
}
