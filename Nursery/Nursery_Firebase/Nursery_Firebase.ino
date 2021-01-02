#include "FirebaseESP8266.h"
#include <ArduinoJson.h>
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
String obj = "";
void SetupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  firebaseData.setBSSLBufferSize(1024, 1024);

  firebaseData.setResponseSize(1024);

  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  DynamicJsonDocument doc(1024);
  JsonObject wifi  = doc.createNestedObject("Information");
  wifi["IPAddress"] = WiFi.localIP().toString();
  wifi["Version"] = (String)Version;
  wifi["LEDPin"] = (String)ledPin;
  wifi["FireBasePath"] = FIREBASEPATH;

  serializeJson(doc, obj);
  obj.replace("\"", "'" );
  SetValue(host, obj);
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
  digitalWrite(ledPin, !fbValue);
  httpServer.handleClient();

}

int GetValue(String path) {
  Firebase.get(firebaseData, path);
  return firebaseData.intData();
}
void SetValue(String path, int value) {
  Firebase.set(firebaseData, path, value);
}
void SetValue(String path, String value) {
  if (!Firebase.set(firebaseData, host , value)) {

    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
