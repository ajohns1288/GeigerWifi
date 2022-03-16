/*
 * CAUTION:
 * THIS IN NO WAY SHOULD BE USED IN A CRITICAL/LIFE SAFETY MONITORING SYSTEM.
 * DO NOT RELY ON THIS TO BE AN ACCURATE MEASURE OF RADIATION EXPOSURE
 * IT IS FOR EDUCATIONAL PURPOSES ONLY.
 * 
 * GeigerWifi: Geiger counter display, webserver, and uploader
 * Copyright (C) 2022 A. Johnson (ajohns1288@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "WIFI.h"
#include "graphics.h"

#define GEIGER_DOSE_FCTR 57 //really .0057, this avoids floats.
#define FILT_CONST 0.7 //How much filtering to apply 
#define LONG_PER 60 //Long period for averaging CPM
#define SHORT_PER 5 //Short period for averaging CPM

#define NO_BATT_VOLTAGE 780 //Voltage above which no battery is considered connected
#define CHARGE_VOLTAGE 770 //Voltage above which battery is considered charging
#define WIFI_MIN_VOLT 600 //Votage below which Wifi is disabled
#define ALLOW_DBG_MENU 0 //Allow debug display on OLED (must add into to screen to use)
#define WIFI_TIMEOUT 10000 //Time to wait for wifi connection before aborting connection

//Electrical configuration
#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2
#define BATT_A2D A0
#define COUNT_PIN  13

#define setBit(reg,x) reg|=(1UL<<x)
#define clearBit(reg,x) reg&=~(1UL<<x)
#define checkBit(reg,x) (reg&(1UL<<x))
#define BUTTON_A_BIT 0
#define BUTTON_B_BIT 1
#define BUTTON_C_BIT 2
#define ALLOW_WIFI_BIT 3
#define WIFI_ON_BIT 4
int statusReg = 0;

//Instantaneous CPM calculation variables
volatile long prevCountTime = 0;
volatile long countTime = 0;
volatile int count = 0;
volatile int count2 = 0;
int CPM_inst = 0; //CPM determined by time between counts
int CPM_filt = 0; //Instantaneous CPM filtered

//Average CPM calculation variables
int fastcpm = 0;
int slowcpm = 0;
int CPS = 0;
int dataBuf[LONG_PER];
int idx = 0;

//Dose (usv/h) variables
int dose = 0;
int doseFilt = 0;

//Battery variables
int scaledBatt = 0;
int rawBatt = 0;
int filtBatt=700;

//Display vars
int dispMode = MODE_CPM;
long tmr500 = 0;
int tmr1000 = 0;



void setWifi(int stat)
{
  long startWifiTmr=millis();
  clearDisplay();
  if (stat==1 && !checkBit(statusReg, WIFI_ON_BIT))
  {
    textSize(1);
    homeCursor();
    bool timedOut = false;
    if (checkBit(statusReg, ALLOW_WIFI_BIT))
    {
      clearDisplay();
      sendTextToDisplay("Connecting to WiFi",1);
      updateDisplay();
      wifiConnect();
      while (!isWifiConnected() & !timedOut)
      {
        sendTextToDisplay(".", 0);
        updateDisplay();
        delay(500);
        (millis()-startWifiTmr) < WIFI_TIMEOUT ? timedOut = false : timedOut = true;
      }
      if (timedOut)
      {
        sendTextToDisplay("WiFi connection timed out", 1);
        updateDisplay();
        delay(1000);
      }
      else
      {
        sendTextToDisplay(".", 1);
        sendTextToDisplay("Connection successful", 1);
        updateDisplay();
        serverOn();
        sendTextToDisplay("IP Address: ", 1);
        sendTextToDisplay(getIPAddr(),0);
        updateDisplay();
        delay(1000);
        setBit(statusReg, WIFI_ON_BIT);
      }
    }
    else
    {
      sendTextToDisplay("Battery too low for WiFi", 1);
      updateDisplay();
      delay(1000);
    }
  }
  else if ((stat==0) && checkBit(statusReg, WIFI_ON_BIT))
  {
    textSize(1);
    homeCursor();
    sendTextToDisplay("Turning Wifi Off...", 1);
    updateDisplay();
    wifiOff();
    sendTextToDisplay("WifiOff", 1);
    updateDisplay();
    clearBit(statusReg, WIFI_ON_BIT);
    delay(1000);
  }
  else
  {
    textSize(1);
    homeCursor();
    sendTextToDisplay("Wifi Error :(", 1);
    updateDisplay();
    delay(1000);
  }
}


void setup() {

  Serial.begin(115200);
  battery_level();
  initDisplay();

  setWifi(1);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BATT_A2D, INPUT);
  pinMode(COUNT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(COUNT_PIN), handleCount, FALLING);

}

void loop() {
  if (!digitalRead(BUTTON_A)) setBit(statusReg, BUTTON_A_BIT);
  if (!digitalRead(BUTTON_B)) setBit(statusReg, BUTTON_B_BIT);
  if (!digitalRead(BUTTON_C)) setBit(statusReg, BUTTON_C_BIT);
  delay(10);
  yield();
  if (checkBit(statusReg, WIFI_ON_BIT))
  {
    wifiLoop();
  }

  

  if (millis() - tmr500 > 500)
  {
    battery_level();
    if (checkBit(statusReg, BUTTON_A_BIT))
    {
      //conditional compile to allow debug menu
#if ALLOW_DBG_MENU
      dispMode < 2 ? dispMode = 0 : dispMode--;
#else
      dispMode < 2 ? dispMode = 1 : dispMode--;
#endif
      clearBit(statusReg, BUTTON_A_BIT);
    }
    if (checkBit(statusReg, BUTTON_B_BIT))
    {
      dispMode > 3 ? dispMode = 4 : dispMode++;
      clearBit(statusReg, BUTTON_B_BIT);
    }
    if (checkBit(statusReg, BUTTON_C_BIT))
    {
      checkBit(statusReg,WIFI_ON_BIT)?setWifi(0):setWifi(1);
      clearBit(statusReg, BUTTON_C_BIT);
    }
    calcCPMInst();
    int c = 0;
    int a = 0;
    switch (dispMode)
    {
      case MODE_DBG:
        //Add stuff here if needed for debug
        break;
      case MODE_CPM:
        c = CPM_filt;
        a = slowcpm;
        break;
      case MODE_USV:
        c = dose;
        a = doseFilt;
        break;
      case MODE_CNT:
        c = count2;
        break;
      case MODE_VIS:
        c = CPM_filt;
        a = slowcpm;
        break;
    }
    clearDisplay();
    drawStatusBar(checkBit(statusReg,WIFI_ON_BIT),getIPAddr(),scaledBatt);
    writeDisplay(dispMode, c, a);
    updateDisplay();
    tmr500 = millis();
  }

  if (millis() - tmr1000 > 1000)
  {
    tmr1000 = millis();
    calcCPMAvg();
    calcDose();

    //Send data to Wifi Server
    setCPM(slowcpm);
    setDose(dose);
  }
}

void battery_level() {

  rawBatt = analogRead(A0);
  if (rawBatt < WIFI_MIN_VOLT)
  {
    clearBit(statusReg,ALLOW_WIFI_BIT);
  }
  else
  {
    setBit(statusReg,ALLOW_WIFI_BIT);
  }

  filtBatt=(rawBatt*0.4+filtBatt*0.6);
  
  if (filtBatt > NO_BATT_VOLTAGE)
  {
    scaledBatt = 10;
  }
  else if (filtBatt > CHARGE_VOLTAGE)
  {
    scaledBatt = 9;
  }
  else
  {
    scaledBatt = map(rawBatt, 580, 765, 0, 8);
  }
  
}


void calcCPMInst()
{
  //Calculates the instantaneous CPM based on time between counts
  //If time between has not changed, instantaneous filters to the fast average
  //CPM_filt is a further filtered version of the instantaneous for use in display
  static int prevDelT;
  int deltaT = countTime - prevCountTime;
  if (deltaT && deltaT != prevDelT)
  {
    CPM_inst = (60000 / (deltaT));
    prevDelT = deltaT;
  }
  else
  {
    CPM_inst = (CPM_inst * FILT_CONST) + (fastcpm * (1 - FILT_CONST));
  }

  CPM_filt = (CPM_filt * FILT_CONST) + (CPM_inst * (1 - FILT_CONST));

}

void calcDose()
{
  dose = (GEIGER_DOSE_FCTR * slowcpm) / 10;
  doseFilt = (doseFilt * FILT_CONST) + (dose * (1 - FILT_CONST));
}

void calcCPMAvg()
{
  //Calculates CPM averaged over samples taken at 1 second.
  //This code is from the MightyOhm geiger counter kit by Jeff Keyzer
  CPS = count;
  slowcpm -= dataBuf[idx];   // subtract oldest sample in sample buffer

  slowcpm += count;     // add current sample
  dataBuf[idx] = count;  // save current sample to buffer (replacing old value)

  // Compute CPM based on the last SHORT_PERIOD samples
  fastcpm = 0;
  for (int i = 0; i < SHORT_PER; i++) {
    int x = idx - i;
    if (x < 0)
      x = LONG_PER + x;
    fastcpm += dataBuf[x]; // sum up the last 5 CPS values
  }
  fastcpm = fastcpm * (LONG_PER / SHORT_PER); // convert to CPM

  // Move to the next entry in the sample buffer
  idx++;
  if (idx >= LONG_PER)
    idx = 0;
  count = 0;  // reset counter
}

ICACHE_RAM_ATTR void handleCount()
{
  count++;
  count2++;
  prevCountTime = countTime;
  countTime = millis();
}
