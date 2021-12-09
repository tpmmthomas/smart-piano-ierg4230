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

## Hardware configuration 
We can use two ESP8266 boards. (Also separate project code into two boards).
No need direct communication between the two boards. Make use of the web server for all communications.
### Board 1 (Facing front to users)
Include only the OLED, RFID, beeper. (This part only do the access control.)  
Once users tap an RFID card ==> Send code to web server by REST API.   
Preset different response messages in the OLED ==> Also set up a web request server in ESP8266 (is this possible?)   
Then upon receiving web request, show corresponding thing (message) immediately + make corresponding sound in beeper.   
(If cannot set up ESP8266 as web server externally, do periodic web reqeusts to server to active fetch info instead.)
### Board 2 (Install inside the piano)  
This include the MIC , MAX4466, AHT10.



## Backend Processing (Inside flask /gravana?)



## Frontend (Web)

## Frontend (mobile) (Notifications only)
Use a telegram alert tool.  
Alert if:   
1.  Humidity exceeds threshold + Dehumidify tube not connected.
2.  All access records (different msg for valid or invalid access).
3.  Pitch is not right. 

## Flowchart
![abc](flow.png)