#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <SPI.h>
 
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPass";

#define ORG "change"
#define DEVICE_TYPE "change"
#define DEVICE_ID "change"
#define TOKEN "change"

#define LED 2

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

#define CMD_STATE "/gpio/" 


// use the '+' wildcard so it subscribes to any command with any message format
const char commandTopic[] = "iot-2/cmd/+/fmt/+";

void gotMsg(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, gotMsg, wifiClient);

void setup() {
 Serial.begin(115200); Serial.println();
 pinMode(LED, OUTPUT);
 wifiConnect();
 mqttConnect(); 
}

int lastHeartBeat;

void loop() {

 if (!client.loop()) {
   mqttConnect();
 }
 
 if (millis()-lastHeartBeat > 10000) {
   Serial.print("loop: gpio "); Serial.print(LED); Serial.print(" current state "); 
   Serial.println(digitalRead(LED) ? "On" : "Off");
   lastHeartBeat = millis();
 }
}

void gotMsg(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("gotMsg: invoked for topic: "); Serial.println(topic);
 
 if (String(topic).indexOf(CMD_STATE) > 0) {
   String cmd = "";
   for (int i=0; i<payloadLength; i++) {
     cmd += (char)payload[i];
   }
   doCommand(cmd);
 } else {
   Serial.print("gotMsg: unexpected topic: "); Serial.println(topic); 
 }
}

void doCommand(String cmd) {
 int currentState = digitalRead(LED);
 int newState = (cmd == "on");
 digitalWrite(LED, newState);
 Serial.print("LED switched from "); 
 Serial.print(currentState ? "On" : "Off");Serial.print(" to "); Serial.println(newState ? "On" : "Off");
}

void wifiConnect() {
 Serial.print("Connecting to "); Serial.print(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 } 
 Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
 if (!!!client.connected()) {
   Serial.print("Reconnecting MQTT client to "); Serial.println(server);
   while (!!!client.connect(clientId, authMethod, token)) {
     Serial.print(".");
     delay(500);
   }
   Serial.println();
 }

 subscribeTo(commandTopic);
}

void subscribeTo(const char* topic) {
 Serial.print("subscribe to "); Serial.print(topic);
 if (client.subscribe(topic)) {
   Serial.println(" OK");
 } else {
   Serial.println(" FAILED");
 }
}
