/**
* A simple IBM IoT example for testing connection
* Base code written by Anthony Elder, IBM && released under Apache License v2
* Improved with guide by Abhishek Ghosh, https://thecustomizewindows.com/
* Needs below 2 steps :
* (1) On IBM IoT dashboard, go to Security > Connection Security > TLS Optional
* (2) Install PubSubClient library from Arduino IDE
* Open Serial Monitor to see output
* Also check log on IBM IoT dashboard
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h> 

const char* ssid = "your hotspot";
const char* password = "abcdefgh";

#define ORG "abishek678"
#define DEVICE_TYPE "yourtype"
#define DEVICE_ID "your id"
#define TOKEN "your token"

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char pubTopic[] = "iot-2/evt/status/fmt/json";
char subTopic[] = "iot-2/cmd/test/fmt/String";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClient wifiClient;
PubSubClient client(server, 1883, NULL, wifiClient);

void setup() {
 Serial.begin(115200); delay(1); Serial.println();

 initWiFi();
}

void loop() {

 if (!!!client.connected()) {
   Serial.print("Reconnecting client to "); Serial.println(server);
   while (!!!client.connect(clientId, authMethod, token)) {
     Serial.print(".");
     delay(500);
   }
   Serial.println();
 }

 String payload = "{ \"d\" : {\"counter\":";
 payload += millis()/1000;
 payload += "}}";
 
Serial.print("Sending payload: "); Serial.println(payload);
 
 if (client.publish(pubTopic, (char*) payload.c_str())) {
   Serial.println("Publish ok");
 } else {
   Serial.println("Publish failed");
 }

 delay(3000);
}

void initWiFi() {
 Serial.print("Connecting to "); Serial.print(ssid);
 if (strcmp (WiFi.SSID().c_str(), ssid) != 0) {
   WiFi.begin(ssid, password);
 }
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 } 
 Serial.println(""); Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());
}
