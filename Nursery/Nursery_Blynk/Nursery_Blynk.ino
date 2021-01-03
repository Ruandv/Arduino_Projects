#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>

#include "FirebaseESP8266.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>

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
  Blynk.run();
}

String availableCommands() {
  WriteMessage("You can only use the following commands :");
  WriteMessage("GetInfo");
  WriteMessage("Update");
}

BLYNK_WRITE(V2)
{
  String value = param.asStr() ;
  WriteMessage(value.substring(0, 10));
  if (String("GetInfo") == value)
  {
    GetInfo();
  }
  else if (String("Update") == value) {
    updateFirmware();
  }
  else
  {
    WriteMessage("I dont understand your command " + String(param.asStr()));
    availableCommands();
  }
}

BLYNK_WRITE(V3) {
  int value = param.asInt() ;
  digitalWrite(ledPin, value);
  SetValue(FIREBASEPATH, value);
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

String obj = "";
void setupFirebase() {
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
  wifi["FireBaseRoot"] = FIREBASEROOT;
  wifi["FireBasePath"] = FIREBASEPATH;

  serializeJson(doc, obj);
  obj.replace("\"", "'" );
  SetValue(host, obj);
}

void GetVersionInfo() {
  Firebase.getJSON(firebaseData, "Version/" + (String)host);

}
void updateFirmware() {
  GetVersionInfo();
  const String json = firebaseData.stringData();
  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  const char* DBVersion = doc["Version"];
  const char* FIRMWARE_URL = doc["URL"];
  WriteMessage("DBVersion " + (String) DBVersion);
  WriteMessage("Version " + (String)Version);
  if ((String)DBVersion != (String)Version)
  {
    WriteMessage("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    WriteMessage("Downloading file... " + (String)FIRMWARE_URL );
    t_httpUpdate_return ret = ESPhttpUpdate.update(FIRMWARE_URL);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        //String msg = "HTTP_UPDATE_FAILD Error " + (String)ESPhttpUpdate.getLastError() + ": " + ESPhttpUpdate.getLastErrorString().c_str();
        WriteMessage("Update Failed");
        WriteMessage((String)ESPhttpUpdate.getLastError());
        WriteMessage(ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        WriteMessage("HTTP UPDATE NO UPDATES");
        break;

      case HTTP_UPDATE_OK:
        WriteMessage("HTTP UPDATE OK");
        break;
    }
  }
  else
  {
    Serial.println("firmware up to date");
  }
}

void setup()
{
  Serial.begin(115200);
  setupPinMode();
  setupWiFiManager();
  SetupOTA();
  setupFirebase();
  Blynk.config(auth);
}

void loop()
{
  Blynk.run();
  httpServer.handleClient();
}

String GetValue(String path) {
  Firebase.getString(firebaseData, path);
  return firebaseData.stringData();
}

void SetValue(String path, int value) {
  Firebase.set(firebaseData, FIREBASEROOT + path, value);
}

void SetValue(String path, String value) {
  if (!Firebase.set(firebaseData, (String)FIREBASEROOT + "/" + (String)host , value)) {

    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
