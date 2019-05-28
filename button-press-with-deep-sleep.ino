// this example uses one push button attached to pin 33
// push button attached to pin 33 sends message and puts it in to deep sleep
// again pushing the pushbutton attached to pin 33 it wakes up from sleep
// LED function kept as the this originated from my non deep-sleep code, it is not made working by code 

// written by Dr. Abhishek Ghosh, https://thecustomizewindows.com
// released under GNU GPL 3.0
// 

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex; pin 33 for push button
RTC_DATA_ATTR int bootCount = 0;

const byte BUTTON=33; // boot button pin (built-in on ESP32)
const byte LED=2; // onboard LED (built-in on ESP32)

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <base64.h>
#define USE_SERIAL Serial 

#define FORCE_MODE 3
 
unsigned long buttonPushedMillis; // when button was released
unsigned long ledTurnedOnAt; // when led was turned on
unsigned long turnOnDelay = 20; // wait to turn on LED
unsigned long turnOffDelay = 5000; // turn off LED after this time
bool ledReady = false; // flag for when button is let go
bool ledState = false; // for LED is on or not.


const char* ssid = "fill up";
const char* password = "fill up";

#define ORG "write here" // change
#define DEVICE_TYPE "change" // change
#define DEVICE_ID "change" // change
#define TOKEN "copy-paste" // change
#define EVENT "myEvent" // example

// <------- CHANGE PARAMETERS ABOVE THIS LINE ------------>

String urlPath = "/api/v0002/device/types/" DEVICE_TYPE "/devices/" DEVICE_ID "/events/" EVENT;
String urlHost = ORG ".messaging.internetofthings.ibmcloud.com";
int urlPort = 8883;
String authHeader;

void deep_sleep() {
    printf("Sleep at %d ms\n\n", millis());
    delay(20);
    // esp_sleep_enable_timer_wakeup(20000 * 1000); // Deep-Sleep time in microseconds
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low 
    esp_deep_sleep_start();
    // Serial.println("This will never be printed");

}
 
void setup() {
 Serial.begin(115200); Serial.println();
 delay(1000); //Take some time to open up the Serial Monitor
 //Increment boot number and print it every reboot
  ++bootCount;
Serial.println("Boot number: " + String(bootCount));
 pinMode(BUTTON, INPUT_PULLUP);
 pinMode(LED, OUTPUT);
 digitalWrite(LED, LOW);

initWifi();
authHeader = "Authorization: Basic " + base64::encode("use-token-auth:" TOKEN) + "\r\n";
doWiFiClientSecure();
loop();
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

void loop() {
 // get the time at the start of this loop()
 unsigned long currentMillis = millis(); 
 // check the button
 if (digitalRead(BUTTON) == LOW) {
  // update the time when button was pushed
  buttonPushedMillis = currentMillis;
  ledReady = true;
 }
  
 // make sure this code isn't checked until after button has been let go
 if (ledReady) {
   //this is typical millis code here:
   if ((unsigned long)(currentMillis - buttonPushedMillis) >= turnOnDelay) {
     // okay, enough time has passed since the button was let go.
     digitalWrite(LED, HIGH);
     doWiFiClientSecure();
     // setup our next "state"
     ledState = true;
     // save when the LED turned on
     ledTurnedOnAt = currentMillis;
     // wait for next button press
     ledReady = false;
   }
 }
  
 // see if we are watching for the time to turn off LED
 if (ledState) {
   // okay, led on, check for now long
   if ((unsigned long)(currentMillis - ledTurnedOnAt) >= turnOffDelay) {
     ledState = false;
     digitalWrite(LED, LOW); 
   }
 }
deep_sleep();               
}
