
/**
 * Demonstrates how to send a command to a Watson IoT Platform device
 * 
 * You can use curl to test sending the commands:
 * curl -u <yourApiKey>:<yourApiPassword> -H "Content-Type: text/plain" -v -X 
 *     POST http://<yourOrg>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/
 *     application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/gpio -d "on" 
 * 
 * Modified for ESP32 by Dr. Abhishek Ghosh, https://thecustomizewindows.com , released under Apache License v2
 * 
 * Original Author: Anthony Elder
 * License: Apache License v2
 */


#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include <SPI.h>
 
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPass";
 
//-------- Customise these values -----------

#define ORG "change"
#define DEVICE_TYPE "change"
#define DEVICE_ID "change"
#define TOKEN "change"

//-------- Customise the above values --------

#define DEVICE_BUTTON 0
#define DEVICE_RELAY 19
#define DEVICE_GREEN_LED 18
#define DEVICE_RED_LED 4

Ticker ledBlinker;

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

int buttonPressDuration;

void setup() {
 Serial.begin(115200); Serial.println();

 pinMode(DEVICE_RELAY, OUTPUT);
 pinMode(DEVICE_GREEN_LED, OUTPUT);
 pinMode(DEVICE_RED_LED, OUTPUT);

 ledBlinker.attach(0.1, ledBlink); // fast blink indicates Wifi connecting
 wifiConnect();
 ledBlinker.attach(0.4, ledBlink); // slower blink indicates MQTT connecting
 mqttConnect();
 ledBlinker.detach();
 digitalWrite(DEVICE_GREEN_LED, LOW); // low is led on to show connected
 
 pinMode(DEVICE_BUTTON, INPUT);
 attachInterrupt(DEVICE_BUTTON, buttonPress, CHANGE); 
}

int lastHeartBeat;

void loop() {
 if (buttonPressDuration > 0) {
   doCommand(digitalRead(DEVICE_RELAY) ? "off" : "on");
   buttonPressDuration = 0;
 }

 if (!client.loop()) {
   mqttConnect();
 }
 
 if (millis()-lastHeartBeat > 10000) {
   Serial.print("loop: gpio "); Serial.print(DEVICE_RELAY); Serial.print(" current state "); 
   Serial.println(digitalRead(DEVICE_RELAY) ? "On" : "Off");
   digitalWrite(DEVICE_GREEN_LED, HIGH); // flicker LED to show its active
   delay(200);
   digitalWrite(DEVICE_GREEN_LED, LOW);
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
 int currentState = digitalRead(DEVICE_RELAY);
 int newState = (cmd == "on");
 digitalWrite(DEVICE_RELAY, newState);
 Serial.print("Relay switched from "); 
 Serial.print(currentState ? "On" : "Off");Serial.print(" to "); Serial.println(newState ? "On" : "Off");
}

unsigned long startPress = 0;

void buttonPress() {
 int currentState = digitalRead(DEVICE_BUTTON);
 if (currentState == 0) { // 0 is pressed, 1 is released
   startPress = millis();
 } else {
   int diff = millis() - startPress;
   if (diff > 100) { // debounce
     buttonPressDuration = diff;
   }
 }
 Serial.print("Button "); Serial.print(currentState ? "released" : "pressed");
 Serial.print(" duration="); Serial.println(buttonPressDuration);
}

void ledBlink() {
  digitalWrite(DEVICE_GREEN_LED, ! digitalRead(DEVICE_GREEN_LED));
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
