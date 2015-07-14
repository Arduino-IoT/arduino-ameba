
#include "arduino.h"
#include "Timeout.h"

Timeout flipper;
int led1_status, led2_status;
void flip()  {
  led2_status = !led2_status;
  digitalWrite(12, led2_status);
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  led1_status = HIGH;
  pinMode(13, OUTPUT);

   //
  led2_status = HIGH;
  pinMode(12, OUTPUT);
  digitalWrite(12, led2_status);
   
  flipper.attach(&flip, 2.0);
}

// the loop function runs over and over again forever
void loop() {
  led1_status = !led1_status;
  digitalWrite(13, led1_status);
  delay(200);
}

