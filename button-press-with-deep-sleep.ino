// this is beta
// do not raise issue 
// final version will not bear the above warning

// connects once upon pressing ESP32 boot pushbutton (GPIO 0) button and sends a message, closes connection
// written by Dr. Abhishek Ghosh, https://thecustomizewindows.com
// released under GNU GPL 3.0


#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex, pin 33
RTC_DATA_ATTR int bootCount = 0;
#define TIME_TO_SLEEP  200       /* Time ESP32 will go to sleep (in seconds) */

const byte BUTTON=33; // pin 33
const byte LED=2; // onboard LED (built-in LED)

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <base64.h>
#define USE_SERIAL Serial 
 
unsigned long buttonPushedMillis; // when button was released
unsigned long ledTurnedOnAt; // when led was turned on
unsigned long turnOnDelay = 20; // wait to turn on LED
unsigned long turnOffDelay = 5000; // turn off LED after this time
bool ledReady = false; // flag for when button is let go
bool ledState = false; // for LED is on or not.

const char* ssid = "your wifi hotspot";
const char* password = "password of the above";

#define ORG "your org on ibm" 
#define DEVICE_TYPE "name you given"
#define DEVICE_ID "name you given"
#define TOKEN "your token"
#define EVENT "myEvent" // example

// <------- CHANGE PARAMETERS ABOVE THIS LINE ------------>

String urlPath = "/api/v0002/device/types/" DEVICE_TYPE "/devices/" DEVICE_ID "/events/" EVENT;
String urlHost = ORG ".messaging.internetofthings.ibmcloud.com";
int urlPort = 8883;
String authHeader;


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)

  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;

  }

}
 
void setup() {
 pinMode(BUTTON, INPUT_PULLUP);
 pinMode(LED, OUTPUT);
 digitalWrite(LED, LOW);
 Serial.begin(115200); Serial.println(); 
 delay(1000); //Take some time to open up the Serial Monitor
 //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
 
 initWifi();
 authHeader = "Authorization: Basic " + base64::encode("use-token-auth:" TOKEN) + "\r\n";
}

void anything() {
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
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
Serial.println("Loop will never be printed");
}
