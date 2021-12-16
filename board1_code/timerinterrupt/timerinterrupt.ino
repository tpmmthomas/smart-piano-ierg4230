/****************************************************************************************************************************
  Argument_None.ino
  For ESP8266 boards
  Written by Khoi Hoang
  Built by Khoi Hoang https://github.com/khoih-prog/ESP8266TimerInterrupt
  Licensed under MIT license
  The ESP8266 timers are badly designed, using only 23-bit counter along with maximum 256 prescaler. They're only better than UNO / Mega.
  The ESP8266 has two hardware timers, but timer0 has been used for WiFi and it's not advisable to use. Only timer1 is available.
  The timer1's 23-bit counter terribly can count only up to 8,388,607. So the timer1 maximum interval is very short.
  Using 256 prescaler, maximum timer1 interval is only 26.843542 seconds !!!
  Now with these new 16 ISR-based timers, the maximum interval is practically unlimited (limited only by unsigned long miliseconds)
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.
*****************************************************************************************************************************/

/* Notes:
   Special design is necessary to share data between interrupt code and the rest of your program.
   Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
   variable can not spontaneously change. Because your function may change variables while your program is using them,
   the compiler needs this hint. But volatile alone is often not enough.
   When accessing shared variables, usually interrupts must be disabled. Even with volatile,
   if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
   If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
   or the entire sequence of your code which accesses the data.
*/

#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "ESP8266TimerInterrupt.h"
#include "arduinoFFT.h"

#ifndef LED_BUILTIN
  #define LED_BUILTIN       2         // Pin D4 mapped to pin GPIO2/TXD1 of ESP8266, NodeMCU and WeMoS, control on-board LED
#endif

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

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  
  delay(300);
  // Interval in microsecs
  if (ITimer.attachInterrupt(TIMER_FREQ_HZ, TimerHandler))
  {
    Serial.println(("Starting  ITimer OK"));
  }
  else
    Serial.println(("Can't set ITimer correctly. Select another freq. or interval"));
}

void loop()
{
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
    delay(1000);
    samplei = 0;
  }
}
