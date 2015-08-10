#line 1 "Blink.ino"
  
       
                                                                         

                                                                    
                                                                   
                                                                   
                                        

                                            

                     
                     
   


                                                                       
#include "Arduino.h"
void setup();
void loop();
#line 18
void setup() {
                                            
  pinMode(13, OUTPUT);
}

                                                     
void loop() {
  digitalWrite(13, HIGH);                                                 
  delay(1000);                                  
  digitalWrite(13, LOW);                                                 
  delay(1000);                                  
}

