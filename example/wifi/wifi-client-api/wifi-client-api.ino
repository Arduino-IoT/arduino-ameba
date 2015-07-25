#include <WiFi1.h>

char ssid[] = "NeoPhone";          //  your network SSID (name) 
char pass[] = "myPassword";   // your network password

int status = WL_IDLE_STATUS;

// Initialize the client library
WiFiClient1 client;
IPAddress server(74,125,23,104);  // Google

void setup() {

  Serial.begin(38400);
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

    Serial.println("\nStarting connection...");
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
      Serial.println("connected");
      
          // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.0");
    client.println();
  
    // if there are incoming bytes available 
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    }
    
  }
}

void loop() {
}
