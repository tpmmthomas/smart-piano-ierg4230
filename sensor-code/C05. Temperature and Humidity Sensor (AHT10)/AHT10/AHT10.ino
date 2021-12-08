#include <Wire.h>
#include "Thinary_AHT10.h"

#include <Adafruit_GFX.h>  //OLED
#include <Adafruit_SSD1306.h> //OLED

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//OLED
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, i2c Addr, Reset share with 8266 reset);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

AHT10Class AHT10;

float Humidity;
float Temperature;

void setup() {
  // put your setup code here, to run once:
  ESP.wdtDisable();
  ESP.wdtFeed();
  
  Serial.begin(9600);
//  Wire.begin();
  Wire.begin(2,0);
  if(AHT10.begin(eAHT10Address_Low))
    Serial.println("Init AHT10 Sucess.");
  else
    Serial.println("Init AHT10 Failure.");
    Serial.println("<<Thinary Eletronic AHT10 Module>>");

  //OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  //OLED diplay 1st line
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("IERG4230 AHT10 Module"); // Display static text
  display.display();

  delay(500);
}

void loop() {
//  put your main code here, to run repeatedly:
//  Serial.println("//Thinary Eletronic AHT10 Module//");
//  Serial.println("https://thinaryelectronic.aliexpress.com");
  Humidity = AHT10.GetHumidity();
  Temperature = AHT10.GetTemperature();

  Serial.println(String("") + "Humidity(%RH):\t" + Humidity + "%");
  Serial.println(String("") + "Temperature(℃):\t" + Temperature + "℃");
//  Serial.println(String("") + "Dewpoint(℃):\t" + AHT10.GetDewPoint() + "℃");
  Serial.println();

  display.clearDisplay();
  //OLED diplay 1st line
  display.setCursor(0, 10);
  display.println("IERG4230 AHT10 Module"); // Display static text
  
  //OLED diplay 3rd line
  display.setCursor(0, 30);
  display.println("Humidity(%RH):     %");      // Display static text
  display.setCursor(84, 30);
  display.print(Humidity);

  //OLED diplay 5rd line
  display.setCursor(0, 50);
  display.print("Temperature(C):     C");       // Display static text
  display.setCursor(90, 50);
  display.print(Temperature);

  display.display();
  
  ESP.wdtFeed();
  
  delay(500);
}
