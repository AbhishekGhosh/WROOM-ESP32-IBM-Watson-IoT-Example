/**
 * Demonstrates how to control two relays with Watson IoT Platform connected ESP32 device with plain cURL commands. 
 * 
 * You can use curl to test sending the commands:
 * 
 * curl -u <yourApiKey>:<yourApiPassword> -H "Content-Type: text/plain" -v -X 
 *     POST http://<yourOrg>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/
 *     application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/pin1 -d "on" 
 * 
 * curl -u <yourApiKey>:<yourApiPassword> -H "Content-Type: text/plain" -v -X 
 *     POST http://<yourOrg>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/
 *     application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/pin1 -d "off" 
 *     
 * curl -u <yourApiKey>:<yourApiPassword> -H "Content-Type: text/plain" -v -X 
 *     POST http://<yourOrg>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/
 *     application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/pin2 -d "on" 
 * 
 * curl -u <yourApiKey>:<yourApiPassword> -H "Content-Type: text/plain" -v -X 
 *     POST http://<yourOrg>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/
 *     application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/pin2 -d "off" 
 * 
 * Modified for ESP32 by Dr. Abhishek Ghosh, https://thecustomizewindows.com , released under Apache License v2
 * 
 * Author: Dr. Abhishek Ghosh
 * License: Apache License v2
 */


#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include <SPI.h>

const char* ssid = "change";
const char* password = "change";
 
//-------- Customise these values -----------

#define ORG "change"
#define DEVICE_TYPE "change"
#define DEVICE_ID "change"
#define TOKEN "change"

//-------- Customise the above values --------

// first relay with two status LEDs

#define DEVICE_BUTTON 5
#define DEVICE_RELAY 19
#define DEVICE_GREEN_LED 18
#define DEVICE_RED_LED 4


// second relay with two status LEDs

#define DEVICE_BUTTON_1 13
#define DEVICE_RELAY_1 21
#define DEVICE_GREEN_LED_1 22
#define DEVICE_RED_LED_1 23

// nothing to modify for adding more relays 

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

// you are sending this url format to IBM's server

#define CMD_STATE_1 "/pin1/" 
#define CMD_STATE_2 "/pin2/" 

// use the '+' wildcard so it subscribes to any command with any message format
// you are sending this url format to IBM's server
// changing the format will change cURL command
// you can add more relays in the same way

const char commandTopic1[] = "iot-2/cmd/pin1/fmt/+";
const char commandTopic2[] = "iot-2/cmd/pin2/fmt/+";

Ticker ledBlinker;

// IBM's setup does not need strcmd for HTTP command

void gotMsg(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, gotMsg, wifiClient);

// you can add more relays by appending 2, 3 where I used 1 beside keywords & strings

int buttonPressDuration;
int buttonPressDuration1;

void setup() {
 Serial.begin(115200); Serial.println();

 pinMode(DEVICE_RELAY, OUTPUT);
 pinMode(DEVICE_GREEN_LED, OUTPUT);
 pinMode(DEVICE_RED_LED, OUTPUT);

 pinMode(DEVICE_RELAY_1, OUTPUT);
 pinMode(DEVICE_GREEN_LED_1, OUTPUT);
 pinMode(DEVICE_RED_LED_1, OUTPUT);

 ledBlinker.attach(0.1, ledBlink); // fast blink indicates Wifi connecting
 wifiConnect();
 ledBlinker.attach(0.4, ledBlink); // slower blink indicates MQTT connecting
 mqttConnect();
 ledBlinker.detach();

// we have two status LEDs
 
 digitalWrite(DEVICE_GREEN_LED, LOW); // low is led on to show connected
 digitalWrite(DEVICE_GREEN_LED_1, LOW); // low is led on to show connected

// we have two buttons 
 
 pinMode(DEVICE_BUTTON, INPUT);
 attachInterrupt(DEVICE_BUTTON, buttonPress, CHANGE); 
 
 pinMode(DEVICE_BUTTON_1, INPUT);
 attachInterrupt(DEVICE_BUTTON_1, buttonPress1, CHANGE); 
}

// isolate the relay devices and HTTP commands

int lastHeartBeat;
int lastHeartBeat1;

void loop() {
 if (buttonPressDuration > 0) {
   doCommand(digitalRead(DEVICE_RELAY) ? "off" : "on");
   buttonPressDuration = 0;
 }

  if (buttonPressDuration1 > 0) {
   doCommand(digitalRead(DEVICE_RELAY_1) ? "off" : "on");
   buttonPressDuration1 = 0;
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

  if (millis()-lastHeartBeat1 > 10000) {
   Serial.print("loop: gpio1 "); Serial.print(DEVICE_RELAY_1); Serial.print(" current state "); 
   Serial.println(digitalRead(DEVICE_RELAY_1) ? "On" : "Off");
   digitalWrite(DEVICE_GREEN_LED_1, HIGH); // flicker LED to show its active
   delay(200);
   digitalWrite(DEVICE_GREEN_LED_1, LOW);
   lastHeartBeat1 = millis();
 }
}

void gotMsg(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("gotMsg: invoked for topic: "); Serial.println(topic);

// we defined CMD_STATE 1 and 2 before 
// it is URLs on IBM's server

// seek on first URL
 
 if (String(topic).indexOf(CMD_STATE_1) > 0) {
   String cmd = "";
   for (int i=0; i<payloadLength; i++) {
     cmd += (char)payload[i];
   }
   doCommand(cmd);
 } else {
    // unexpected error was for one relay setup
    // uncommenting will evoke false warning message
   // Serial.print("gotMsg: unexpected topic: "); 
   Serial.println(topic); 
 }

// seek on second URL 

  if (String(topic).indexOf(CMD_STATE_2) > 0) {
   String cmd1 = "";
   for (int i=0; i<payloadLength; i++) {
     cmd1 += (char)payload[i];
   }
   doCommand1(cmd1);
 } else {
      // unexpected error was for one relay setup
    // uncommenting will evoke false warning message
   // Serial.print("gotMsg: unexpected topic: "); 
   Serial.println(topic); 
 }
}

// doCommand is a variable name
// it is for the first relay

void doCommand(String cmd) {
 int currentState = digitalRead(DEVICE_RELAY);
 int newState = (cmd == "on");
 digitalWrite(DEVICE_RELAY, newState);
 Serial.print("Relay switched from "); 
 Serial.print(currentState ? "On" : "Off");Serial.print(" to "); Serial.println(newState ? "On" : "Off");
}

// it is for the second relay

void doCommand1(String cmd1) {
 int currentState = digitalRead(DEVICE_RELAY_1);
 int newState = (cmd1 == "on");
 digitalWrite(DEVICE_RELAY_1, newState);
 Serial.print("Relay switched from "); 
 Serial.print(currentState ? "On" : "Off");Serial.print(" to "); Serial.println(newState ? "On" : "Off");
}

// first and second buttons with debounce

unsigned long startPress = 0;
unsigned long startPress1 = 0;

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

void buttonPress1() {
 int currentState1 = digitalRead(DEVICE_BUTTON_1);
 if (currentState1 == 0) { // 0 is pressed, 1 is released
   startPress1 = millis();
 } else {
   int diff = millis() - startPress1;
   if (diff > 100) { // debounce
     buttonPressDuration1 = diff;
   }
 }
 Serial.print("Button 2"); Serial.print(currentState1 ? "released" : "pressed");
 Serial.print(" duration="); Serial.println(buttonPressDuration1);
}

void ledBlink() {
  digitalWrite(DEVICE_GREEN_LED, ! digitalRead(DEVICE_GREEN_LED));
    digitalWrite(DEVICE_GREEN_LED_1, ! digitalRead(DEVICE_GREEN_LED_1));
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

// add more relays here to subscribe 

 subscribeTo(commandTopic1);
 subscribeTo(commandTopic2);
}

void subscribeTo(const char* topic) {
 Serial.print("subscribe to "); Serial.print(topic);
 if (client.subscribe(topic)) {
   Serial.println(" OK");
 } else {
   Serial.println(" FAILED");
 }
}
