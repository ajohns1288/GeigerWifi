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

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <stdint.h>
#include <Arduino.h>

//graphics.h/.cpp is an abstraction layer for the graphics library. 
//This is done so that if other hardware is used, only the .cpp needs to be changed

//Display mode definitions
#define MODE_CPM 1
#define MODE_USV 2
#define MODE_CNT 3
#define MODE_VIS 4
#define MODE_DBG 0


void initDisplay();
void homeCursor();
void sendTextToDisplay(String, int);
void sendIntToDisplay(int , int);
void textSize(int );
void clearDisplay();
void updateDisplay();
void drawStatusBar(bool, String, int);
void writeDisplay(int, int, int);
uint8_t * getBattBmp(int);

#endif
