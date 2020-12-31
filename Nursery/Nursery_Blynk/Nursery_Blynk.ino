#include "FirebaseESP8266.h"
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

BLYNK_WRITE(V3){
  int value = param.asInt() ;
  digitalWrite(ledPin,value);
  SetValue(value);
}

void GetInfo() {
  WriteMessage("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
  WriteMessage("\r\nConnected To : " + WiFi.SSID() +
               "\r\nSignal :" + String(WiFi.RSSI()) +
               "\r\nIp Address : " +  WiFi.localIP().toString()  +
               "\r\nVersions : " + Version);
}

void SetupOTA() {
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void setupPinMode()
{
  pinMode(sensorPin, INPUT);
}


void setupWiFiManager() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);
}

void SetupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void setup()
{
  Serial.begin(115200);
  setupPinMode();
  setupWiFiManager();
  SetupOTA();
  SetupFirebase();
  Blynk.config(auth);
}
void loop()
{
  if (GetValue(FIREBASEPATH) == "B") {
    int value = analogRead(ledPin);

    for (int i = 0; i < 4; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, HIGH);
      delay(500);
    }
    analogWrite(ledPin, value);
  }
  Blynk.run();
  httpServer.handleClient();
}

String GetValue(String path) {
  Firebase.getString(firebaseData, path);
  return firebaseData.stringData();
}
void SetValue(int value) {
  Firebase.set(firebaseData, FIREBASEPATH , value);
}
