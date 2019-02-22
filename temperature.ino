/**
 * A simple IBM IoT example for testing connection
 * Base code written by http://www.iotsharing.com/
 * Improved with guide by Abhishek Ghosh, https://thecustomizewindows.com/
 * Needs below 2 steps :
 * (1) On IBM IoT dashboard, go to Security > Connection Security > TLS Optional
 * (2) Install PubSubClient library from Arduino IDE
 * Adding LED on pin 4 is optional
 * Open Serial Monitor to see output
 * Also check log on IBM IoT dashboard
 */
 
 
#include <WiFi.h>

#include <WiFiClient.h>

#include <PubSubClient.h> 

const char* ssid = "SamsungGalaxy";
const char* password = "fkop3288";

#define ORG "tko2rj"
#define DEVICE_TYPE "DevBoard"
#define DEVICE_ID "ESP32"
#define TOKEN "b(N4qP3@Lx!ydaoiEM"

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char pubTopic[] = "iot-2/evt/status/fmt/json";
char subTopic[] = "iot-2/cmd/test/fmt/String";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClient wifiClient;
PubSubClient client(server, 1883, NULL, wifiClient);

const char led = 4;

void receivedCallback(char* pubTopic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(pubTopic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /* we got '1' -> on */
  if ((char)payload[0] == '1') {
    digitalWrite(led, HIGH); 
  } else {
    /* we got '0' -> on */
    digitalWrite(led, LOW);
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    pinMode(led, OUTPUT);
    Serial.print("Connecting to "); 
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    } 
    Serial.println("");
    
    Serial.print("WiFi connected, IP address: "); 
    Serial.println(WiFi.localIP());

    if (!client.connected()) {
        Serial.print("Reconnecting client to ");
        Serial.println(server);
        while (!client.connect(clientId, authMethod, token)) {
            Serial.print(".");
            delay(500);
        }
        client.setCallback(receivedCallback);
        if (client.subscribe(subTopic)) {
            Serial.println("subscribe to cmd OK");
        } else {
            Serial.println("subscribe to cmd FAILED");
        }
        Serial.println("IBM Watson IoT connected");
    }
}

long lastMsg = 0;
long temperature = 0;

void loop() {
    client.loop();
    long now = millis();
    if (now - lastMsg > 3000) {
        lastMsg = now;
        temperature = random(0, 40);
        String payload = "{\"d\":{\"Name\":\"" DEVICE_ID "\"";
              payload += ",\"temperature\":";
              payload += temperature;
              payload += "}}";
        Serial.print("Sending payload: ");
        Serial.println(payload);

        if (client.publish(pubTopic, (char*) payload.c_str())) {
            Serial.println("Publish ok");
        } else {
            Serial.println("Publish failed");
        }
    }
}
