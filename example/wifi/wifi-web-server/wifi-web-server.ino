/*
 
 This example connects to an unencrypted Wifi network. 
 Then it prints the  MAC address of the Wifi shield,
 the IP address obtained, and other network details.

 Circuit:
 * WiFi shield attached
 
 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */
 #include <WiFi1.h>
 #include "WiFiServer1.h"

char ssid[] = "NeoTPLink";     // the name of your network
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiServer1 server(80);

void setup() {
  Serial.begin(38400);
  Serial.print("SSID: ");
  Serial.println(ssid);

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

    // you're connected now, so print out the data:
    Serial.print("You're connected to the network");

    Serial.println("Server begin");
    server.begin();
     
  }   
  
}

static int counts=1;

void loop() {
  // check the network connection once every 10 seconds:
  // listen for incoming clients
  WiFiClient1 client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
            client.print("counts ");
            client.print(counts++);
            client.println("<br />");       
          client.println("</html>");
           break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}


