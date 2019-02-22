/**
* A simple HTTPS IBM IoT example for testing connection
* his is a modified version of the sketch that goes with the developerWorks recipe: https://developer.ibm.com/recipes/tutorials/use-http-to-send-data-to-the-ibm-iot-foundation-from-an-esp8266/
* This sketch is modified to show how to use a secure https connection between the ESP32 and the Watson IoT Platform.
* Improved with guide by Abhishek Ghosh, https://thecustomizewindows.com/
* Open Serial Monitor to see output
* Also check log on IBM IoT dashboard
* ESP32 will connect once & get disconnected
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <base64.h>

#define USE_SERIAL Serial 


// <------- CHANGE PARAMETERS BELOW THIS LINE ------------>

const char* ssid = "your hotspot";
const char* password = "abcdefgh";

#define ORG "abishek678"
#define DEVICE_TYPE "yourtype"
#define DEVICE_ID "your id"
#define TOKEN "your token"
#define EVENT "myEvent" 

// <------- CHANGE PARAMETERS ABOVE THIS LINE ------------>

String urlPath = "/api/v0002/device/types/" DEVICE_TYPE "/devices/" DEVICE_ID "/events/" EVENT;
String urlHost = ORG ".messaging.internetofthings.ibmcloud.com";
int urlPort = 8883;
String authHeader;

void setup() {
  Serial.begin(115200); Serial.println(); 
  initWifi();
  Serial.println("View the published data on Watson at: "); 
  if (ORG == "quickstart") {
    Serial.println("https://quickstart.internetofthings.ibmcloud.com/#/device/" DEVICE_ID "/sensor/");
  } else {
    Serial.println("https://" ORG ".internetofthings.ibmcloud.com/dashboard/#/devices/browse/drilldown/" DEVICE_TYPE "/" DEVICE_ID);
  }  
  if (ORG == "quickstart") {
    authHeader = "";
  } else {
    authHeader = "Authorization: Basic " + base64::encode("use-token-auth:" TOKEN) + "\r\n";
  }  
}

void loop() {
  doWiFiClientSecure();
  delay(10000);
}

void doWiFiClientSecure() {
 WiFiClientSecure client;
 Serial.print("connect: "); Serial.println(urlHost);
 while ( ! client.connect(urlHost.c_str(), urlPort)) {
    Serial.print(".");
 }
 Serial.println("Connected");
 String postData = String("{  \"d\": {\"aMessage\": \"") + millis()/1000 + "\"}  }";
 String msg = "POST " + urlPath + " HTTP/1.1\r\n"
                "Host: " + urlHost + "\r\n"
                "" + authHeader + ""
                "Content-Type: application/json\r\n"
                "Content-Length: " + postData.length() + "\r\n"
                "\r\n" + postData;
                
 client.print(msg);
 Serial.print(msg);

 Serial.print("\n*** Request sent, receiving response...");
 while (!!!client.available()) {
    delay(50);
 Serial.print(".");
  }
  
Serial.println();
Serial.println("Got response");  
  while(client.available()){
  Serial.write(client.read());
  }
Serial.println(); Serial.println("closing connection");
  client.stop();
}

void initWifi() {
  Serial.print("Connecting to: "); Serial.print(WiFi.SSID());
  WiFi.mode(WIFI_STA);   
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
     delay(250);
     Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());

}
