#include <WiFi1.h>

char ssid[] = "NeoPhone";          //  your network SSID (name) 
char pass[] = "myPassword";   // your network password

int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  //status = WiFi.begin(ssid, pass);
  status = WiFi1.begin(ssid);
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    // don't do anything else:
    while(true);
  } 
  else {
    Serial.println("Connected to wifi, delay 10 sec for DHCP");
    delay(10000);
   IPAddress ip = WiFi1.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

  }
}

char targetHost[] = "njiot.blogspot.tw";

void loop() {
  IPAddress ip;
   WiFi1.hostByName(targetHost, ip);
   Serial.print("host name : ");
   Serial.println(targetHost);
   Serial.print("IP : ");
   Serial.println(ip);
   delay(10000);
}
