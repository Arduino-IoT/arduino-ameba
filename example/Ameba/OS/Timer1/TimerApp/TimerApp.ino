
#include "arduino.h"
#include "Timer.h"

Timer t;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
}



// the loop function runs over and over again forever
void loop() {
  t.start();
  delay(1000);
  t.stop();
  Serial.print("The time taken was ");
  Serial.print(t.read());
  Serial.println(" seconds \n");
}

