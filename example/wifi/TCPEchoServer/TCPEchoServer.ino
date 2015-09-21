#include <WiFi1.h>
#include <TCPSocketServer.h>
#include <TCPSocketConnection.h>

char ssid[] = "NeoTPLink";          //  your network SSID (name) 
char pass[] = "myPassword";   // your network password

int status = WL_IDLE_STATUS;

TCPSocketServer server;

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
    Serial.println("Connected to wifi, delay 3 sec for DHCP");
    delay(3000);
    IPAddress ip = WiFi1.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    
  }
  
  server.bind(7);
  server.listen();
  
}

char buffer[256];
void loop() {
    Serial.println("Wait for new connection...");
    TCPSocketConnection client;
    server.accept(client);
    client.set_blocking(false, 15000); // Timeout after 15 s
    
    Serial.print("Connection from: ");
    Serial.println(client.get_address());
    
    
     while (true) {
        int n = client.receive(buffer, sizeof(buffer));
        if (n <= 0) break;
        
        server.write(buffer, n);
        if (n <= 0) break;
    }
        
        client.close();
}
