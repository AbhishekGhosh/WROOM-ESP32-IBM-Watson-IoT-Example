#include "BLEDevice.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <base64.h>
#define USE_SERIAL Serial 

const char* ssid = "your-hotspot";
const char* password = "password-of-hotspot";

#define ORG "paste-it-here" 
#define DEVICE_TYPE "DevBoard" //change
#define DEVICE_ID "ESP32" // change
#define TOKEN "your-token"
#define EVENT "myEvent" // example

String urlPath = "/api/v0002/device/types/" DEVICE_TYPE "/devices/" DEVICE_ID "/events/" EVENT;
String urlHost = ORG ".messaging.internetofthings.ibmcloud.com";
int urlPort = 8883;
String authHeader;

static BLEAddress *pServerAddress;
BLEScan* pBLEScan;
BLEClient*  pClient;
bool deviceFound = false;
bool LEDoff = false;
bool BotonOff = false;
String knownAddresses[] = { "e0:a1:07:b7:0b:95"}; // change the MAC
unsigned long entry;

// <------- CHANGE PARAMETERS ABOVE THIS LINE ------------>

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice Device){
      // show the MAC of other BLE devices
      //Serial.print("BLE Advertised Device found: ");
      //Serial.println(Device.toString().c_str());
      pServerAddress = new BLEAddress(Device.getAddress()); 
      bool known = false;
      bool Master = false;
      for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++) {
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0) 
          known = true;
      }
      if (known) {
        Serial.print("Our device found!");
        Serial.print("Device distance:");
        Serial.println(Device.getRSSI());
        // adjust the value. -85 is medium distance
        // -60 is closer than -85
        if (Device.getRSSI() > -85) {
          deviceFound = true;
        }
        else {
          deviceFound = false;
        }
        Device.getScan()->stop();
        delay(100);
      }
    }
};
void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
   initWifi();
   authHeader = "Authorization: Basic " + base64::encode("use-token-auth:" TOKEN) + "\r\n";
  Serial.println("Done");
}
void Bluetooth() {
  Serial.println();
  Serial.println("BLE Scan restarted.....");
  deviceFound = false;
  BLEScanResults scanResults = pBLEScan->start(5);
  if (deviceFound) {
    Serial.println("Found");
    doWiFiClientSecure();
    delay(10000);
  }
  else{
    Serial.println("Device is not closer");
    doWiFiClientLost();
  }
}
void loop() { 
  Bluetooth();
}

void doWiFiClientSecure() {
 WiFiClientSecure client;
 Serial.print("connect: "); Serial.println(urlHost);
 while ( ! client.connect(urlHost.c_str(), urlPort)) {
    Serial.print(".");
 }
 Serial.println("Connected");
 String postData = String("{  \"d\": {\"Our device found!\": \"") + millis()/1000 + "\"}  }";
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

void doWiFiClientLost() {
 WiFiClientSecure client;
 Serial.print("connect: "); Serial.println(urlHost);
 while ( ! client.connect(urlHost.c_str(), urlPort)) {
    Serial.print(".");
 }
 Serial.println("Connected");
 String postData = String("{  \"d\": {\"Our device is away!\": \"") + millis()/1000 + "\"}  }";
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
