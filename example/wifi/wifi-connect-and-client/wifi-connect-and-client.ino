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

char ssid[] = "NeoTPLink";     // the name of your network
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient1 client;
IPAddress server(192,168,0,101);  // 

byte numSsid;

void show_enc_type(uint8_t type) {
  switch (type) {
    case 2 : 
        client.print(" TKIP ");
        break;
    case 4 : 
        client.print(" CCMP ");
        break;
    case 5 : 
        client.print(" WEP ");
        break;
    case 7 : 
        client.print(" None ");
        break;
    case 8 : 
        client.print(" Auto ");
        break;
    default : 
        client.print(" wrong ttype ");
        break;
  }
}

void listNetworks() {

  // print the list of networks seen:
  client.print("number of available networks:");
  client.println(numSsid);


  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    client.print(thisNet);
    client.print(") ");
    client.print(WiFi1.SSID(thisNet));

    client.print("\tSignal: ");
    client.print(WiFi1.RSSI(thisNet));
    client.println(" dBm");

    client.print("\tEncryption: ");
    show_enc_type(WiFi1.encryptionType(thisNet));
    
    client.println(" ");
  }
 
}

void wifi_connect_and_report()
{
  static int i = 0;
 //status = WiFi.begin(ssid, pass);
  status = WiFi1.begin(ssid);
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    return;
  } 
  else {
    delay(3000);
    IPAddress ip = WiFi1.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    Serial.println("\nStarting connection...");
    // if you get a connection, report back via serial:
    if (client.connect(server, 2095)) {
      Serial.println("connected");
    } else {
      Serial.println("can not connect");
      return;
    }
  }
  
  listNetworks();
  
  delay(1000);
  client.stop();
  WiFi1.disconnect();
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(38400); 
  
}

void loop() {
    // scan for existing networks:
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  numSsid = WiFi1.scanNetworks();

  wifi_connect_and_report();
  // check the network connection once every 10 seconds:
  delay(1000);
}


