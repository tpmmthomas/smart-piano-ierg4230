/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

// Custom Definition
#include "IERG4230.h"

// Temperature & Humidity
#include <Wire.h>
#include "Thinary_AHT10.h"

AHT10Class AHT10;

float Humidity;
float Temperature;

// Mic
//const int micPin_analog = A0; // ADC
//const int micPin_digital = 12;// GPIO12(D6)

// MAX4466
// TODO



// Events
osEvent th_event, mic_event, fft_event, wifi_event;
osEvent test_event;


//************************************************************
void setup() {
  th_event.timerSet(100);
//  mic_event.timerSet(100);
//  fft_event.timerSet(100);
//  wifi_event.timerSet(100);

  test_event.timerSet(100);

  // init for ATH10 (temperature & humidity)
  ESP.wdtDisable();
  ESP.wdtFeed();

  Serial.begin(115200);
  Wire.begin(2,0);

  if(AHT10.begin(eAHT10Address_Low))
    Serial.println("Init AHT10 Sucess.");
  else
    Serial.println("Init AHT10 Failure.");
  Serial.println("<<Thinary Eletronic AHT10 Module>>");

  delay(500);

  // init for mic
//  pinMode(micPin_analog, INPUT);
//  pinMode(micPin_digital, INPUT);

  // init for MAX4466 (FFT) 
  // TODO



  // Serial.setDebugOutput(true);

//  Serial.println();
//  Serial.println();
//  Serial.println();
//
//  for (uint8_t t = 4; t > 0; t--) {
//    Serial.printf("[SETUP] WAIT %d...\n", t);
//    Serial.flush();
//    delay(1000);
//  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("REALNEW", "1155147592");
}

void loop() {
  
  if (osEvent::osTimer != millis()) timeStampUpdate();
  if (th_event.isSet()) th_event_handler();
//  if (mic_event.isSet()) mic_event_handler();
//  if (fft_event.isSet()) fft_event_handler();
//  if (wifi_event.isSet()) wifi_event_handler();
  
  if (test_event.isSet()) test_event_handler();
  
  

//  delay(10000);
}

void timeStampUpdate(void)   // no need to modify this function unless you know what you are doing.
{
  int i;
  unsigned long temp;
  temp = millis();
  if (osEvent::osTimer > temp) i = 1;
  else i = (int)(temp - osEvent::osTimer);
  osEvent::osTimer = temp;
  //---- user add their own tasks if necessary
  th_event.timerUpdate(i);
//  mic_event.timerUpdate(i);
//  fft_event.timerUpdate(i);
//  wifi_event.timerUpdate(i);

  test_event.timerUpdate(i);
}

// Event Handlers
void th_event_handler(void){
  th_event.clean();
  
  // get the temperature & humidity
  Temperature = AHT10.GetTemperature();
  Humidity = AHT10.GetHumidity();
  
  Serial.println(String("") + "Temperature(℃):\t" + Temperature + "℃");
  Serial.println(String("") + "Humidity(%RH):\t\t" + Humidity + "%");
  Serial.println();

  ESP.wdtFeed();

  // send data to server
  // TODO
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://34.150.76.100:9876")) {  // HTTP
      
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
//      int httpCode = http.GET();
//      int httpCode = http.POST("temp="+to_string(Temperature)+"?humid="+to_string(Humidity));
//      int httpCode = http.POST("123");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  // monitor every 15 seconds
  th_event.timerSet(15000);
}


//void mic_event_handler(void){
//  mic_event.clean();
//  
//  // get the sound
//  int micA_Status = analogRead(micPin_analog); // Analog value 0~1023
//  int micD_Status = digitalRead(micPin_digital); // Digital value 0 = volulme lager than threshold
//
//  Serial.print("Mic(Analog)=");
//  Serial.println(micA_Status);        // Display analog value
//  Serial.print("Mic(Digital)=");
//  Serial.println(micD_Status);        // Display digital value
//
//  // do something when volume exceeds threshold
//  if (micA_Status >= 535) {
//    Serial.println("Sound detected");
//  }
//  else {
//    Serial.println("No sound detected");
//  }
//
//  // monitor every 0.5 seconds
//  mic_event.timerSet(500);
//}


//void fft_event_handler(void){
//  fft_event.clean();
//
//  Serial.println("FFT");
//  fft_event.timerSet(1000);
//}


void wifi_event_handler(void){
//  wifi_event.clean();
  Serial.println("Wi-Fi");

  // PROBLEM: Must run the below then other code, no multitasking

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // HTTP
      
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

//  wifi_event.timerSet(10000);
}


void test_event_handler(void){
  test_event.clean();

  Serial.println("HELLOP");
  test_event.timerSet(1000);
}
