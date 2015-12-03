/*
##Xively WiFi Sensor Tutorial##
This sketch is designed to take sensors (from photocell) and upload the values to Xively
at consistant intervals. At the same time it gets a setable value from Xively to adjust the brigthness
of an LED. This sketch is reusable and can be adapted for use with many different sensors.
Derived from Xively Ardino Sensor Client by Sam Mulube.
 
By Calum Barnes 3-4-2013
BSD 3-Clause License - [http://opensource.org/licenses/BSD-3-Clause]
Copyright (c) 2013 Calum Barnes
*/
#include "WiFi1.h"
#include <HttpClient.h>
#include <Xively.h>

char ssid[] = "NeoPhone"; //  your network SSID (name) 
char pass[] = "12345678";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Your Xively key to let you upload data
char xivelyKey[] = "HlwyIaGp8Dic1WzmpJCFxg07zoj4DaWks9KKCfH21qKyeiro";
//your xively feed ID
#define xivelyFeed 667621444
//#define xivelyFeed FEED_ID_HERE
//datastreams
char sensorID[] = "temp";


// Define the strings for our datastream IDs
XivelyDatastream datastreams[] = {
  XivelyDatastream(sensorID, strlen(sensorID), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(xivelyFeed, datastreams, 1 /* number of datastreams */);

WiFiClient1 client;
XivelyClient xivelyclient(client);

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi1.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi1.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi1.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm \n");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  Serial.println("Starting single datastream upload to Xively...");
  Serial.println();

  status = WiFi1.begin(ssid, pass);
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    // don't do anything else:
    while(true);
  } 
  else {
    Serial.println("Connected to wifi, delay 3 sec for DHCP");
    delay(3000);
     printWifiStatus();
  }
  

}

void loop() {
  int ret;
  static float temp = 21.0;
  
  
  Serial.print("temp= ");
  Serial.println(temp);
  datastreams[0].setFloat(temp);
  
  //print the sensor valye
  //Serial.print("Read sensor value ");
  //Serial.println(datastreams[0].getFloat());

  //send value to xively
  //Serial.println("Uploading it to Xively");
  ret = xivelyclient.put(feed, xivelyKey);
  //return message
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  Serial.println("");
  
  temp=temp+0.1;
  if (temp > 30 ) temp = 21;
  
  //delay between calls
  delay(2000);
}

