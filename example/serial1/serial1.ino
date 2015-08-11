#include "arduino.h"

int baud_rate = 9600;

void setup() {
  Serial1.begin(9600);
  Serial1.println("Hello, world");
}

void loop() {
  static int i=0;
  Serial1.print("hello ");
  Serial1.println(i);
  while ( Serial1.available() ) {
    Serial.write(Serial1.read());
  }
  i++;
  delay(1000);
}
