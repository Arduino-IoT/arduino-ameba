
#include "arduino.h"
#include "Ticker.h"

Ticker flipper;
int led1_status;

void test_flip()  {
  Serial.println("flip");
  led1_status = !led1_status;
  digitalWrite(13, led1_status);
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(38400);
  // initialize digital pin 13 as an output.
  led1_status = HIGH;
  pinMode(13, OUTPUT);

  flipper.attach(&test_flip, 2.0);
}

// the loop function runs over and over again forever
void loop() {
  delay(1000);
}

