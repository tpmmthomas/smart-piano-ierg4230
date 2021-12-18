/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "REALNEW"
#define STAPSK  "1155147592"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "djxmmx.net";
const uint16_t port = 17;

// Custom Definition
#include "IERG4230.h"

// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Beeper
#include "Buzzer.h"
Buzzer buzzer(14); // (Buzzer pin)

//RFID
#include "MFRC522_I2C.h"
#define RST_PIN 14 // GPIO-14(D5) Pin on ESP8266
MFRC522 mfrc522(0x28, RST_PIN);   // Create MFRC522 instance.

// Events
osEvent oled_event, beeper_event, rfid_event;

//************************************************************
void setup() {
  oled_event.timerSet(100);
  beeper_event.timerSet(100);
  rfid_event.timerSet(100);

  ESP.wdtDisable();
  ESP.wdtFeed();
  
  Serial.begin(9600);
  Wire.begin(2,0);

  // OLED
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // RFID
  mfrc522.PCD_Init();             // Init MFRC522
  ShowReaderDetails();            // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
  
  
  
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (osEvent::osTimer != millis()) timeStampUpdate();
//  if (oled_event.isSet()) oled_event_handler();
//  if (beeper_event.isSet()) beeper_event_handler();
//  if (rfid_event.isSet()) rfid_event_handler();

  // RFID
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
  display.println("Please present your card");
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
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    id += mfrc522.uid.uidByte[i];
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

  check_id(id);
  
  Serial.println();
  ESP.wdtFeed();
    
  //**********************************************
  static bool wait = false;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from ESP8266");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  if (wait) {
    delay(300000); // execute once every 5 minutes, don't flood remote service
  }
  wait = true;
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
  oled_event.timerUpdate(i);
  beeper_event.timerUpdate(i);
  rfid_event.timerUpdate(i);
}


// Event Handlers
void oled_event_handler(void){
  oled_event.clean();

  Serial.println("OLED");
  oled_event.timerSet(1000);
}


void beeper_event_handler(int mode, int duration, bool isLegal){
  beeper_event.clean();
  buzzer.begin(0);

  if (mode == 1) {
    buzzer.distortion(NOTE_C3, NOTE_C5);
    buzzer.distortion(NOTE_C5, NOTE_C3);
  }
  else if (mode == 2) {
    buzzer.fastDistortion(NOTE_C3, NOTE_C5);
    buzzer.fastDistortion(NOTE_C5, NOTE_C3);
  }

  buzzer.end(duration);

  Serial.println("BEEPER");
  beeper_event.timerSet(1000);
}


void rfid_event_handler(void){
  rfid_event.clean();

  Serial.println("RFID");
  rfid_event.timerSet(1000);
}

void check_id(String id) {
  // TODO
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
