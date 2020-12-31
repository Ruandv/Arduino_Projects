#include "FirebaseESP8266.h"

#include <ESP8266WiFi.h>

#include <ESP8266HTTPUpdateServer.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "ProgramAuthToken.h"


//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void WriteMessage(String msg)
{
  Serial.println(msg);
}

String availableCommands() {
  WriteMessage("You can only use the following commands :");
  WriteMessage("GetInfo");
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
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
}


void setupWiFiManager() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);
}

void SetupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  firebaseData.setBSSLBufferSize(1024, 1024);

  firebaseData.setResponseSize(1024);

  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void setup()
{
  Serial.begin(115200);
  setupPinMode();
  setupWiFiManager();
  SetupOTA();
  SetupFirebase();
  //Blynk.config(auth);
}
void loop()
{

  int fbValue = GetValue(FIREBASEPATH);
  Serial.println(fbValue);
  digitalWrite(ledPin, !fbValue);
  httpServer.handleClient();

}

int GetValue(String path) {
  Firebase.get(firebaseData, path);
  return firebaseData.intData();
}
void SetValue(int value) {
  Firebase.set(firebaseData, FIREBASEPATH , value);
}
