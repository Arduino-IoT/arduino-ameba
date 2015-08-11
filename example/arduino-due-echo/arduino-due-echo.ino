void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

}

void loop() {
  int rc;
  if ( Serial.available() > 0 ) {
    rc = Serial.read();
    if ( rc > 0 ) Serial.write(rc);
  }
}

