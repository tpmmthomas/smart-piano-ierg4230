/*
   IERG4230 IoT Testing Program
   MFRC522 RFID
   Matter Needing attention:
   On Boot/Reset/wakeup,
   GPIO15(D8) keep LOW and GPIO2(D4)keep HIGH
   GPIO0(D3) HIGH = Run mode / LOW = Flash mode
   On Board LEDs (active LOW):
    GPIO16(D0)
    GPIO2(D4)
  
  Connections:
  MFRC522      ESP8266-12E
  ---------    -----------
  IRQ          Not connected
  SDA_NSS_RX   GPIO 2(D4)
  SCK          Not connected
  MOSI         Not connected
  SCL_MISO_TX  GPIO 0(D3)
  PCTRL        Not connected
  GND          GND
  nRESET       Not connected
  VIN(+5)      VIN
*/
#include <Wire.h>
#include "MFRC522_I2C.h"

#include <Adafruit_GFX.h>     //OLED
#include <Adafruit_SSD1306.h> //OLED

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// OLED
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, i2c Addr, Reset share with 8266 reset);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define RST_PIN 14 // GPIO-14(D5) Pin on ESP8266

// 0x3F is I2C address on SDA.
//MFRC522 mfrc522(0x3F, RST_PIN);   // Create MFRC522 instance.
MFRC522 mfrc522(0x28, RST_PIN);   // Create MFRC522 instance.

void setup() {
  ESP.wdtDisable();
  ESP.wdtFeed();

  Serial.begin(115200);           // Initialize serial communications with the PC
  Serial.println("ESP8266-12E/F MFRC5522 RFID test program\n");
  Serial.println("Build-in LED at GPIO-16(D0)");
  Serial.println("I2C SDA Pin at GPIO-2(D4)");
  Serial.println("I2C SCL Pin at GPIO-0(D3)");
//  Wire.begin();                   // Initialize I2C
  Wire.begin(2,0);                  // Initialize I2C
  mfrc522.PCD_Init();             // Init MFRC522
  ShowReaderDetails();            // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

  // OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(1000);
  display.clearDisplay();

  ///OLED diplay 1st line
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("IERG 4230 IoT MFRC522");
  display.display();
}

void loop() {
  // Look for new cards, and select one if present
  if (! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() )
  {
    ESP.wdtFeed();  // keep FEED the watchdog!
    delay(200);
    return;
  }
  display.clearDisplay();
  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  display.setCursor(0, 10);
  // Display static text
  display.println("IERG 4230 IoT MFRC522");
  display.display();

  // Dump UID
  Serial.println();
  Serial.print(F("Card UID Length: "));
  Serial.println(mfrc522.uid.size, HEX);
  //OLED 3rd line
  display.setCursor(0, 30);
  display.print("Card UID Length: ");
  display.println(mfrc522.uid.size, HEX);
  display.display();

  Serial.print(F("Card UID:"));
  //OLED 5rd line
  display.setCursor(0, 50);
  display.print("Card UID: ");

  display.display();
  display.setCursor(55, 50);
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    // OLED 5rd line
    // display.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    if (mfrc522.uid.uidByte[i] < 0x10)
    {
      display.print(" 0");
    }
    else
    {
      display.print(" ");
    }
    display.print(mfrc522.uid.uidByte[i], HEX);
    display.display();
  }
  Serial.println();
  ESP.wdtFeed();
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  ESP.wdtFeed();
  Serial.println("");
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}
