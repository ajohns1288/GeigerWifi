# GeigerWifi
Arduino sketch for a Wifi and display for Geiger counter pulse outputs 

This was just a fun project to add a display to the MightyOhm Geiger kit found here: https://mightyohm.com/blog/products/geiger-counter/

# Requirements
See the .h files for a list of includes. All libraries are availale through the Arduino IDE.

# Hardware 
I used the Adafruit Huzzah Feather ESP8266, OLED display, and LiPo battery back. You could probably use whatever you have laying around with just a bit of tinkering.

# Function
This system detects the pulse output of a geiger counter and uses the time between pulses as well as the number of pulses per second to calculate a counts per minute (CPM). This is then used to calculate an equivalent dose based on the geiger tube used. 

The calculated values are then sent to the display. The menu buttons (A and B on the adarfruit board) cycle through the different display formats. These values are also sent to a monitoring web server every hour. Additionally, the ESP hosts a small webpage where the values can be seen remotely over Wifi. To save battery life, wifi can be turned off with button C.

# Disclaimer
CAUTION:
 * THIS IN NO WAY SHOULD BE USED IN A CRITICAL/LIFE SAFETY MONITORING SYSTEM.
 * DO NOT RELY ON THIS TO BE AN ACCURATE MEASURE OF RADIATION EXPOSURE
 * IT IS FOR EDUCATIONAL PURPOSES ONLY.
