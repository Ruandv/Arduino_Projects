#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "ProgramAuthToken.h"

//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//Blynk Variables
WidgetRTC rtc;
WidgetTerminal terminal(V2);
bool isConnected = false;

int triggerTime = 0;
int dim =0;
int LEDPIN =13;
BLYNK_CONNECTED() {
  rtc.begin();
  WriteMessage("Blynk Connected");
  if (!isConnected)
  {
    isConnected = true;
    WriteMessage("Sync All");
    Blynk.syncAll();
  }
  else
  {
    WriteMessage("No Sync");
  }
  GetInfo();
}

void WriteMessage(String msg)
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  msg = currentTime + " - " + msg;
  if (isConnected)
  {
    terminal.println(msg);
    terminal.flush();
  }
  Serial.println(msg);
}

String availableCommands() {
  WriteMessage("You can only use the following commands :");
  WriteMessage("GetInfo");
}

BLYNK_WRITE(V2)
{
  String value = param.asStr() ;
  WriteMessage(value.substring(0, 10));
  if (String("GetInfo") == value)
  {
    GetInfo();
  }
  else
  {
    WriteMessage("I dont understand your command " + String(param.asStr()));
    availableCommands();
  }
}

void GetInfo() {
  WriteMessage("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
  WriteMessage("\r\nConnected To : " + WiFi.SSID() +
               "\r\nSignal :" + String(WiFi.RSSI()) +
               "\r\nIp Address : " +  WiFi.localIP().toString()  +
               "\r\nVersion : " + Version);
}

void SetupOTA() {
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void setupPinMode()
{


}

void toggleSwitch() {
  WriteMessage("Toggle");
}


void setupWiFiManager() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);
}

void setup()
{
  Serial.begin(115200);
  setupPinMode();
  setupWiFiManager();
  SetupOTA();
  Blynk.config(auth);
}

void loop()
{
  Blynk.run();
  httpServer.handleClient();
  if (digitalRead(12) == HIGH || triggerTime > 0) {
    if (triggerTime == 0)
    {
      triggerTime = millis();
    }

    if (triggerTime - millis() > 180000)
    {
      //switch off
      triggerTime = 0;
    }
    
    if (dim < 250) {
      WriteMessage("increase dim to " + String(dim));
      dim++;
    }
    digitalWrite(LEDPIN, dim);
    //switch on the lights for 3 minutes
  }
  else
  {
    //switch off
    WriteMessage("SWITCH OFF");
    triggerTime = 0;
    dim=0;
  }
  delay(150);
}
