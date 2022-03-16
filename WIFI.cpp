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

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WIFI.h"



//Wifi vars, set in WiFi.h
const char* ssid     = SSID_TO_USE;
const char* password = PASS_TO_USE;
String IP_ADDR;

//logging server vars
const char* host = HOST_TO_LOG;
long updateTimer=0;
String urlbase = GMC_URI;
String url = "";

//Vars to hold data for webpage/server
boolean firstUpdate=false;
int CPMbufIndex=0;
int CPM_buffer[]={18,18,18,18,18,18,18,18,18,18,18,18}; //Fill buffer with average CPM
int CPM_avg = 0; //12Hour average
int CPM_curr = 0; //Running average CPM for the hour
String DOSE_avg = "0";
ESP8266WebServer server(80);

String getIPAddr()
{
  return IP_ADDR;
}

void setCPM(int a)
{
  CPM_buffer[CPMbufIndex]=(CPM_buffer[CPMbufIndex]+a)/2;
  CPM_curr=CPM_buffer[CPMbufIndex];
    CPM_avg=0;
    for(int i=0;i<12;i++)
    {
      CPM_avg+=CPM_buffer[i];
    }
    CPM_avg/=12;

}

void setDose(int a)
{
  float tmp=((float)a)/1000;
  DOSE_avg=String(tmp,3);
}


void wifiConnect()
{
  WiFi.begin(ssid, password);
}

int isWifiConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}

void serverOn() 
{

  IP_ADDR = WiFi.localIP().toString();

  server.on("/", handleBody);
  server.begin();
}

void wifiOff()
{
  //MDNS.end();
  server.stop();
  WiFi.disconnect();
}



void wifiLoop() {
  server.handleClient();

  if(!firstUpdate&&millis()>FIRST_UPDATE&&SEND_DATA_TO_WEB)
  {
    sendToServer();
    firstUpdate=true;
  }

 

  if(millis()-updateTimer>UPDATE_TIME&&SEND_DATA_TO_WEB)
  {
    sendToServer();
    updateTimer=millis();
  }
}

void sendToServer()
{
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    return;
  }
  
  buildURL();

  // This will send the request to the server
 client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  
}




//Server functions
void buildURL()
{
  url = urlbase + GMC_USERID;
  url = url + GMC_SENSORTAG;
  url = url + GMC_SENSORID;
  url = url + GMC_CPMTAG;
  url = url + CPM_curr;
  url = url + GMC_AVG_CPMTAG;
  url = url + CPM_avg;
  url = url + GMC_USVTAG;
  url = url + DOSE_avg;
}

void handleBody() { //Handler for the body path
  String html;

  html = "<html>";
  html += "<head>";
  html += "<meta http-equiv='refresh' content='15; url='/''/>";
  html += "<title>Geiger Info</title>";
  html += "<style>";
  html += "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Geiger CPM: ";
  html += "</h1>";
  html += "<p>Current: ";
  html += CPM_curr;
  html += " Average: ";
  html += CPM_avg;
  html += " uSv/hr: ";
  html += DOSE_avg;
  html += "</p>";
  html += "<hr>";
  html += "Local IP: ";
  html += IP_ADDR;
  html += "    Uptime: ";
  html += millis()/1000;
  html += "s <hr>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleRoot() {
  handleBody();
}
