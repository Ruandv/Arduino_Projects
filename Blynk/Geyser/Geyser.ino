//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <SimpleTimer.h>
#include "ProgramAuthToken.h"



boolean dayOfWeek[7];

//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* host = "MySmartGeyser";


//Blynk Variables

WidgetRTC rtc;
WidgetTerminal terminal(V2);
WidgetLED geyserLED (V4);
bool isConnected = false;
bool notificationSend = false;

//SystemParameters
const String Version = "2.65";  
int durationHour = 3;
int geyserPin  = 13;
int geyserStatus = 0;
int switchOn = 0;
int buttonState = false;
int timerId;
String geyserTimeoutTime = "";
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 3500;

SimpleTimer t;

BLYNK_CONNECTED() {
  WriteMessage("Blynk Connected");
  if (!isConnected)
  {
    WriteMessage("Sync All");
    Blynk.syncAll();
     
    isConnected = true;
  }
  else
  {
    WriteMessage("No Sync");
  }
  rtc.begin();
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

BLYNK_READ(V0)
{
  long rssi = WiFi.RSSI();
  Blynk.virtualWrite(0, rssi);
}

String availableCommands() {
  WriteMessage("You can only use the following commands :");
  WriteMessage("GetInfo");
}

BLYNK_WRITE(V6)
{
  durationHour = param.asInt();
  WriteMessage("Timeout Duration is now set to : " + String(durationHour));
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

BLYNK_WRITE(V3)//Geyser Out
{
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int reading = param.asInt();
    lastDebounceTime = millis();
    toggleGeyser();
  }
}


BLYNK_WRITE(V5)//Time to switch on
{
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int reading = param.asInt();
    lastDebounceTime = millis();
    switchOn = reading;
    WriteMessage ("Geyser will switch on at : " + String(reading));
  }
}

void toggleGeyser() {
  buttonState = !buttonState;
  digitalWrite(geyserPin, buttonState );
  geyserStatus = digitalRead(geyserPin);
  if(geyserStatus ==HIGH)
  {
    timerId = t.setTimeout(3600000 * durationHour, durationTimeout);
    geyserTimeoutTime = String(hour() + durationHour) + ":" + String(minute()) + ":" + String(second());
    geyserLED.on();
  }
  else
  {
    t.deleteTimer(timerId);
    geyserLED.off();
  } 
  printGeyserStatus();
}

void GetInfo() {
  WriteMessage("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
  WriteMessage("\r\nConnected To : " + WiFi.SSID() +
               "\r\nSignal :" + String(WiFi.RSSI()) +
               "\r\nIp Address : " +  WiFi.localIP().toString()  +
               "\r\nVersion : " + Version +
               "\r\nDuration Time : " + String(durationHour)+
               "\r\Scheduled Time : " + String(switchOn));
  printGeyserStatus();
}

void printGeyserStatus() {
  if ( geyserStatus == HIGH) {
    WriteMessage("Geyser Status : ON ");
    WriteMessage("Geyser will switch off at  : " + geyserTimeoutTime);;
  }
  else
  {
    WriteMessage("Geyser Status : OFF ");
  }
}

void SetupOTA() {
  WriteMessage("OTA START");
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  WriteMessage("OTA Done");
}

void setupPinMode()
{
  pinMode(geyserPin, OUTPUT);
  digitalWrite(geyserPin, buttonState);
}

void setup()
{
  
  Serial.begin(115200);
  WriteMessage("Setup - START");
  WriteMessage("Serial Set");
  setupPinMode();
  WriteMessage("Pin modes Set");
  setupWiFiManager();
  WriteMessage("WiFi Manager Set");
  SetupOTA();
  WriteMessage("Blynk Begin");
  Blynk.config(auth);
  WriteMessage("Blynk Complete");
}

void setupWiFiManager() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);
}

void durationTimeout()
{
  if ( geyserStatus == HIGH)
  {
    toggleGeyser();
  }
}
  
void loop()
{
  if(switchOn >0 ){
    if ( geyserStatus != HIGH && switchOn == hour())
    {
      toggleGeyser();
    }
  }
  Blynk.run();
  httpServer.handleClient(); 
  t.run();
}
