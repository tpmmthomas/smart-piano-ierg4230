/*
  IERG4230 IoT Testing Program
  MIC module
  Matter Needing attention:
  On Boot/Reset/wakeup,
  GPIO15(D8) keep LOW and GPIO2(D4)keep HIGH
  GPIO0(D3) HIGH = Run mode / LOW = Flash mode
  On Board LEDs (active LOW):
    GPIO16(D0)
    GPIO2(D4)

 Connections:
 Mic          ESP8266-12E
 ---------    ---------
 AO           A0
 G            GND
 +            3.3V
 DO           GPIO12 (D6)
*/
const int ledPin1 = 16;       // GPIO-16(D0)
const int ledPin2 = 2;        // GPIO-2(D4)
const int micPin_analog = A0; // ADC
const int micPin_digital = 12;// GPIO12(D6)

void setup()
{
  Serial.begin(115200);
  Serial.println("ESP8266-12E/F MIC testing program\n");
  Serial.println("Build-in LED1 at GPIO-16(D0)");
  Serial.println("Build-in LED2 at GPIO-2(D4)");
  Serial.println("Mic analog Pin at A0");
  Serial.println("Mic digital Pin at GPIO-12(D6)");
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(micPin_analog, INPUT);
  pinMode(micPin_digital, INPUT);
}

void loop()
{
  int micA_Status = analogRead(micPin_analog); // Analog value 0~1023
  int micD_Status = digitalRead(micPin_digital); // Digital value 0 = volulme lager than threshold

  Serial.print("Mic(Analog)=");
  Serial.println(micA_Status);        // Display analog value
  Serial.print("Mic(Digital)=");
  Serial.println(micD_Status);        // Display digital value
  digitalWrite(ledPin2, micD_Status); // DO = LOW => On board LED GPIO-2(D4) ON

  // Mic Analog status AO(Value) > threshold (535) => turn on ESP8266 onboard LED GPIO16(D0)
  // Mic Analog status AO > threshold (VR) => DO = LOW => MIC module LED2 OFF
  if (micA_Status >= 535) {
    digitalWrite(ledPin1, LOW);
    Serial.println("Sound detected, LED is ON");
  }
  else {
    digitalWrite(ledPin1, HIGH);
    Serial.println("LED OFF");
  }

  delay(100);
}
