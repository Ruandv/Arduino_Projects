#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "ProgramAuthToken.h"

//flag for saving data
bool shouldSaveConfig = false;


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
const String Version = "2020";
int durationHour = 3;
int geyserPin  = 13;
int geyserStatus = 0;
int switchOn = 0;
int buttonState = false;
int timerId;
String geyserTimeoutTime = "";
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 3500;

BlynkTimer t;

BLYNK_CONNECTED() {
  // WriteMessage("Blynk Connected");
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


BLYNK_WRITE(V6)
{
  durationHour = param.asInt();
  WriteMessage("Timeout Duration is now set to : " + String(durationHour));
}

BLYNK_WRITE(V2)
{
  String value = param.asStr() ;
  value.toLowerCase();
  terminal.clear();
  if (String("getinfo") == value)
  {
    GetInfo();
  }
  else if (String("reset") == value)
  {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    WriteMessage("Wifi Settings Reset");
    ESP.reset();
  }
  else if (String("restart") == value)
  {
    ESP.restart();
  }
  else {
    WriteMessage("getinfo");
    WriteMessage("reset");
    WriteMessage("restart");
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

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void toggleGeyser() {
  buttonState = !buttonState;
  digitalWrite(geyserPin, buttonState );
  geyserStatus = digitalRead(geyserPin);
  if (geyserStatus == HIGH)
  {
    timerId = t.setInterval(3600000 * durationHour, durationTimeout);
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
               "\r\nDuration Time : " + String(durationHour) +
               "\r\nBlynkToken : " + String(blynk_token) +
               "\r\nGeyserPin : " + String(geyserPin) +
               "\r\nScheduled Time : " + String(switchOn));
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
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void setupPinMode()
{
  pinMode(geyserPin, OUTPUT);
  digitalWrite(geyserPin, buttonState);
}

void setup()
{
  Serial.begin(115200);
  setupFileSystem();
  setupWiFiManager();
  setupPinMode();
  SetupOTA();
  Blynk.config(blynk_token);
}

void setupFileSystem() {
  //clean FS, for testing
  SPIFFS.format();

  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, buf.get());
        if (!error) {
          WriteMessage("parsed json");
          strcpy(blynk_token, doc["blynk_token"]);
          geyserPin = doc[geyserPin].as<int>();
        } else {
          WriteMessage("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    WriteMessage("failed to mount FS");
  }
}
void setupWiFiManager() {
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);

  char cstr[16];
  itoa(geyserPin, cstr, 10);
  WiFiManagerParameter custom_geyserPin("GeyserPin", "Geyser Pin", cstr, 16);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(20);
  
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.addParameter(&custom_geyserPin);
  wifiManager.autoConnect(host);
  if(WiFi.status() != WL_CONNECTED) {
    ESP.reset();
  }
  strcpy(blynk_token, custom_blynk_token.getValue());
  char cust[2];
  strcpy(cust,custom_geyserPin.getValue());
  geyserPin = atoi(cust);
  if (shouldSaveConfig) {
    WriteMessage("saving config");
    DynamicJsonDocument doc(1024);

    doc["blynk_token"] = blynk_token;
    doc["geyserPin"] = geyserPin;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      WriteMessage("failed to open config file for writing");
    }
    serializeJson(doc, Serial);
    serializeJson(doc, configFile);
    configFile.close();
  }
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
  if (switchOn > 0 ) {
    if ( geyserStatus != HIGH && switchOn == hour())
    {
      toggleGeyser();
    }
  }
  Blynk.run();
  httpServer.handleClient();
  t.run();
}
