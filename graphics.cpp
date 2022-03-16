/*
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

#include "graphics.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

uint8_t  BATT_BMP[] = {0x38,0xFE,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0xFE};
uint8_t  NOBAT_BMP[] = {0x24,0x24,0x7E,0x7E,0x7E,0x3C,0x18,0x18,0x18,0x18,0x18};
uint8_t  PWR_BMP[] = {0x01,0x06,0x0C,0x18,0x30,0x3E,0x06,0x0C,0x18,0x30,0x40};

void initDisplay()
{
  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(250);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  // text display tests

  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
}

void homeCursor()
{
  display.setCursor(0, 0);
}

void sendTextToDisplay(String s, int newline)
{
  if (newline)
  {
    display.println(s);
  }
  else
  {
    display.print(s);
  }
}

void sendIntToDisplay(int s, int newline)
{
  if (newline)
  {
    display.println(s, DEC);
  }
  else
  {
    display.print(s, DEC);
  }
}

void textSize(int i)
{
  display.setTextSize(i);
}

void clearDisplay()
{
  display.clearDisplay();
}

void updateDisplay()
{
  display.display();
}

void drawStatusBar(bool wifiOn, String IP, int batt)
{
  display.drawBitmap(120, 0, getBattBmp(batt), 8, 11, 1);
  display.setCursor(0, 0);
  display.setTextSize(1);
  if (wifiOn)
  {
    display.println("Wifi ON");
    display.print("IP:");
    display.print(IP);
  }
  else
  {
    display.print("Wifi OFF");
  }
}
void writeDisplay(int m, int c, int a)
{

  display.setCursor(0, 16);
  display.setTextSize(2);
  switch (m)
  {
    case MODE_DBG:
      //Add stuff here if needed for debug    
      break;
    case MODE_CPM:
      display.println("Unit: CPM");
      display.print("CUR: ");
      display.println(c, DEC);
      display.print("AVG: ");
      display.println(a, DEC);
      break;
    case MODE_USV:
      display.println("Unit:uSv/h");
      display.print("CUR: 0.");
      if (c < 10)
      {
        display.print("00");
      }
      else if (c < 100)
      {
        display.print("0");
      }
      display.println(c, DEC);
      display.print("AVG: 0.");
      if (a < 10)
      {
        display.print("00");
      }
      else if (a < 100)
      {
        display.print("0");
      }
      display.println(a, DEC);
      break;
    case MODE_CNT:
          display.println("Tot Count:");
          display.println(c, DEC);
      break;
    case MODE_VIS:
      display.println("CPM Meter:");
      display.print("C");
      int tmp=map(c,0,150,0,9);
      for(int i=0;i<tmp;i++)
      {
        display.print("=");
      }
      display.print("\nA");
      tmp=map(a,0,40,0,9);
      for(int i=0;i<tmp;i++)
      {
        display.print("=");
      }
      break;

  }

}

uint8_t * getBattBmp(int a)
{

  if (a == 10)
  {
    return NOBAT_BMP;
  }
  if (a == 9)
  {
    return PWR_BMP;
  }

  for (int i = 9; i > 1; i--)
  {
    if (i > (9 - a))
    {
      BATT_BMP[i] = 0xFE;
    }
    else
    {
      BATT_BMP[i] = 0x82;
    }
  }
  return BATT_BMP;
}
