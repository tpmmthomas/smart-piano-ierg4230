#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "ESP8266TimerInterrupt.h"
#include "arduinoFFT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
// Temperature & Humidity
#include <Wire.h>
#include "Thinary_AHT10.h"

AHT10Class AHT10;

float Humidity;
float Temperature;

// Microphone
const int ledPin1 = 16;       // GPIO-16(D0)
const int ledPin2 = 2;        // GPIO-2(D4)
const int micPin_analog = A0; // ADC
const int micPin_digital = 12;// GPIO12(D6)

//FFT
arduinoFFT FFT = arduinoFFT();
volatile uint32_t samplei = 0;
const uint16_t samples = 512;
const double samplingFrequency = 8000;
volatile double sReal[samples];
double vReal[samples];
double vImag[samples];

void TimerHandler()
{
  if(samplei >= samples) return;
  sReal[samplei++] = (double)analogRead(A0);
}

#define TIMER_FREQ_HZ 8000

// Init ESP8266 timer 1
ESP8266Timer ITimer;

#define SERVER_IP "34.150.76.100:9876"
#ifndef STASSID
#define STASSID "1jwd293j2"
#define STAPSK  "u8}926C6"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


const int led = 13;

void setup(void) {
  Serial.begin(115200);
//  WiFi.begin(STASSID, STAPSK);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.print("Connected! IP address: ");
//  Serial.println(WiFi.localIP());
//  
//  if(AHT10.begin(eAHT10Address_Low))
//    Serial.println("Init AHT10 Sucess.");
//  else
//    Serial.println("Init AHT10 Failure.");
//  Serial.println("<<Thinary Eletronic AHT10 Module>>");
  delay(300);
  if (ITimer.attachInterrupt(TIMER_FREQ_HZ, TimerHandler))
  {
    Serial.println(("Starting  ITimer OK"));
  }
  else
    Serial.println(("Can't set ITimer correctly. Select another freq. or interval"));
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(micPin_analog, INPUT);
  pinMode(micPin_digital, INPUT);
  delay(10000000);
}

void loop(void) {
  if(samplei >= samples){
    for(int i=0;i<samples;i++){
      vReal[i] = sReal[i];
      vImag[i]=0.0;
    }
    FFT.Windowing(vReal,samples,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, samples);
    double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
    Serial.print("Calculated Frequency: ");
    Serial.println(x,6);
    delay(100);
    Humidity = AHT10.GetHumidity();
    Serial.println(String("") + "AHT10 | Humidity(%RH):\t" + Humidity + "%");
    samplei = 0;
  }
  delay(1000);
  ESP.wdtFeed();
}
