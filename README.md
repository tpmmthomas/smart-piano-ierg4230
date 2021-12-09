# smart-piano-ierg4230

This repo might be useful if we successfully get MAX4466.
https://github.com/adafruit/Adafruit_Learning_System_Guides/tree/main/Tiny_Music_Visualizer

👍👍👍🙏🙏🙏


## Sensors to be used, and required functions on controller chip (ESP8266)
- MAX4466 Amplifier (Purchasing) -- Detect the frequency, send FFT spectrum to server. 
- MIC : Decide if have someone playing piano. If so, use the MAX4466.
- AHT10 Humidity -- Upload the humidity and temp info to server (temp info only for reference, no actual use for now). 
- Beeper -- Activate when receive signal from server. repeatedly play annoying sound when play piano without proper access control. Also play responsive sounds when users tap the RFID card.  
- OLED：Show welcome message or warning message as prompted.
- RFID: Tap cards to disable access control.  

## Backend Processing


## Hardware configuration (also the project code separation)
We can use two ESP8266 boards.
Humidity: send reminder if above certain threshold.  


## Frontend (Web)

## Frontend (mobile) (Notifications only)