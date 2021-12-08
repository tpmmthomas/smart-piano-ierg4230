#include "Buzzer.h"
/*
  IERG4230 IoT Testing Program
  Buzzer module
  Matter Needing attention:
  On Boot/Reset/wakeup,
  GPIO15(D8) keep LOW and GPIO2(D4)keep HIGH
  GPIO0(D3) HIGH = Run mode / LOW = Flash mode
  On Board LEDs (active LOW):
    GPIO16(D0)
    GPIO2(D4)

 Connections:
 Buzzer       ESP8266-12E
 ---------    ---------
 VCC          3.3V
 I/O          GPIO 14(D5)
 GND          GND
*/

Buzzer buzzer(14); // (Buzzer pin)

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP8266-12E/F Buzzer testing program");
  Serial.println("Build-in LED at GPIO-16(D0)");
  Serial.println("Buzzer pin at GPIO-14(D5)");
}

void loop() {
  buzzer.begin(0);

  /* Method - It creates a "normal distortion" effect on the Buzzer */
  buzzer.distortion(NOTE_C3, NOTE_C5);
  buzzer.distortion(NOTE_C5, NOTE_C3);

  /* Method - It creates a "fast distortion" effect on the Buzzer */
  //  buzzer.fastDistortion(NOTE_C3, NOTE_C5);
  //  buzzer.fastDistortion(NOTE_C5, NOTE_C3);

  /* Method - It creates a "slow distortion" effect on the Buzzer */
  //  buzzer.slowDistortion(NOTE_C3, NOTE_C5);
  //  buzzer.slowDistortion(NOTE_C5, NOTE_C3);

  buzzer.end(1000);
}
