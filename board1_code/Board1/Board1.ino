
// Custom Definition
#include "IERG4230.h"

// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

//#include <Adafruit_MonoOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Beeper
#include "Buzzer.h"
Buzzer buzzer(14); // (Buzzer pin)

//RFID
#include "MFRC522_I2C.h"
#define RST_PIN 5 // GPIO-14(D5) Pin on ESP8266
MFRC522 mfrc522(0x28, RST_PIN);   // Create MFRC522 instance.

//WiFi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

String SERVER_IP="34.150.76.100:9876";

#ifndef STASSID
#define STASSID "1jwd293j2"
#define STAPSK  "u8}926C6"
#endif

// TimerInterrupt
//#include "ESP8266TimerInterrupt.h"
//#include "ESP8266_ISR_Timer.h"
//#define TIMER_INTERVAL_MS 1000
//ESP8266Timer ITimer;

//use millis instead
const long interval = 1000;
unsigned long previousMillis = 0;


// JSON
#include "ArduinoJson.h"


volatile int readflag=1;
volatile int readcard = 0;
//void TimerHandler()
//{
////  Serial.print("Readflag2:");
////  Serial.println(readflag);
////  Serial.print("Readcard2:");
////  Serial.println(readcard);
//  readflag=1;
//  if(readcard>0){
//    readcard--;
//    buzzer.sound(NOTE_C3,250);
//    buzzer.sound(NOTE_C3,250);
//    buzzer.sound(NOTE_C3,250);
//  }
//}


void bytearray_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

//************************************************************
void setup() {
  Serial.begin(9600);
  Wire.begin(2,0);

  // RFID
  mfrc522.PCD_Init();             // Init MFRC522
//  ShowReaderDetails();            // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
  
  // OLED
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(1000);
  display.clearDisplay();

  ///OLED diplay 1st line
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Please present your card");
  display.display();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

//  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS*1000, TimerHandler))
//  {
//    Serial.println(("Starting  ITimer OK"));
//  }
//  else
//    Serial.println(("Can't set ITimer correctly. Select another freq. or interval"));

}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    readflag = 1;
    if(readcard>0) readcard--;
    display.clearDisplay();
    display.display();
  }
  if (readflag) {
    if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClient client;
      HTTPClient http;
  
      Serial.print("[HTTP] begin...\n");
      // configure traged server and url
      String url = "http://"+SERVER_IP+"/command";
      if(http.begin(client, url)){
        int httpCode = http.GET();
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] POST... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.print("payload: ");
            Serial.println(payload);

            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
              Serial.print(F("deserialization() failed "));
              Serial.println(error.f_str());
            }
            else {
              String status = doc["Status"];
              Serial.print("Status: ");
              Serial.println(status);
              if (status == "OK") {
                  int command = doc["Data"]["command"];
                  Serial.print("COMMAND: ");
                  Serial.println(command);

                  String message;
                  display.clearDisplay();
                  if (command == 1){
                    message = "Access Granted!";
                  }
                  else if (command == 2){
                    message = "Invalid Card! Access Denied!";
                    
                  }
                  else if (command == 3){
                    message = "You don't have the access right, leave immediately!";
                  }
                  else if (command == 4){
                    String uname = doc["Data"]["uname"];
                    message = uname + " is playing.";
                  }
                  display.setCursor(0, 20);
                  // Display static text
                  display.println(message);
                  display.display();
                  
                  if (command == 1){
                    buzzer.sound(NOTE_C4,250);
                    buzzer.sound(NOTE_E4,250);
                    buzzer.sound(NOTE_G4,250);
                    buzzer.sound(NOTE_C5,1000);
                  }
                  else if (command == 2){
                    buzzer.begin(0);
                    buzzer.sound(NOTE_C3,250);
                    buzzer.sound(NOTE_C3,250);
                    buzzer.sound(NOTE_C3,250);
                    buzzer.end(10);
                  }
                  else if (command == 3){
                    buzzer.distortion(NOTE_C3,NOTE_C5);
                    buzzer.distortion(NOTE_C5,NOTE_C3);
                  }
                  Serial.println("End buzzer");
              }
            }
          }
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    }
    readflag=0;
  }
  // RFID
  // Look for new cards, and select one if present
  if(readcard>0) return;
  if (! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() )
  {
    delay(200);
    return;
  }
  char str[32] = "";
  bytearray_to_string(mfrc522.uid.uidByte,4,str);
  Serial.print("card uid in string: ");
  Serial.println(str);
  
  if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClient client;
      HTTPClient http;
      http.addHeader("Content-Type", "text/plain");
      Serial.print("[HTTP] begin...\n");
      // configure traged server and url
      String url = "http://"+SERVER_IP+"/access?id="+String(str);
      if(http.begin(client, url)){
//        String params = "id="+String(str);//"{\"id\":\""+String(str)+"\"}";
        int httpCode = http.GET();
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] POST... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    }
    readcard = 3;
}
