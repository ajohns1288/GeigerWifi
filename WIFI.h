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
#ifndef _WIFI_H
#define _WIFI_H

//Wifi SSID/password
#define SSID_TO_USE "wifi"
#define PASS_TO_USE "hunter2"



//GMCMap settings - These will change for your specific application
#define SEND_DATA_TO_WEB false
#define HOST_TO_LOG "www.GMCMap.com"
#define GMC_URI "/log2.asp?AID="
#define GMC_USERTAG "AID="
#define GMC_USERID "12345"
#define GMC_SENSORTAG "&GID="
#define GMC_SENSORID "123456789"
#define GMC_CPMTAG "&CPM="
#define GMC_AVG_CPMTAG "&ACPM="
#define GMC_USVTAG "&USV="

//How often to send data to web. Default is 1 min after startup, every hour thereafter
#define FIRST_UPDATE 60000
#define UPDATE_TIME 3600000

void wifiConnect();
int isWifiConnected();
void serverOn();
void wifiOff();
void wifiLoop();
void setCPM(int);
void setDose(int);
void sendToServer();
void buildURL();
void handleBody();
String getIPAddr();
#endif
