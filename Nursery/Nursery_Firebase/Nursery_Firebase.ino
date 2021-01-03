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
#include <TimeLib.h>

//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

bool checked = true;

void setClock() {
  configTime(TIME_ZONE, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < SECS_YR_2000) {
    delay(100);
    now = time(nullptr);
  }
  setTime(now);
  LogInfo("Time Sync: Success ");
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

void setupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  firebaseData.setBSSLBufferSize(1024, 1024);

  firebaseData.setResponseSize(1024);

  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  String obj = "";
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

void setup()
{
  Serial.begin(115200);
  setupPinMode();
  setupWiFiManager();
  SetupOTA();  
  setupFirebase();
  setClock();
  updateFirmware();
}

void updateFirmware() {
  GetVersionInfo();
  StaticJsonDocument<200> doc;
  deserializeJson(doc, firebaseData.stringData());
  const char* DBVersion = doc["Version"];
  String FIRMWARE_URL = doc["URL"];
  FIRMWARE_URL.replace("{version}",DBVersion);
  Serial.println("DBVersion " + (String) DBVersion);
  Serial.println("Version " + (String)Version);
  if ((String)DBVersion != (String)Version)
  {
    LogInfo("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    LogInfo("Downloading file... " + (String)FIRMWARE_URL );
    t_httpUpdate_return ret = ESPhttpUpdate.update(FIRMWARE_URL);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        LogInfo(((String)"Update failed  - HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()));
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        LogInfo("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        LogInfo("HTTP_UPDATE_OK");
        break;
    }
  }
  else
  {
    Serial.println("firmware up to date");
  }
}


void loop()
{
  int fbValue = GetValue(FIREBASEPATH);
  digitalWrite(ledPin, !fbValue);
  httpServer.handleClient();
  if (fbValue == 1 && checked == false)
  {
    updateFirmware();
  }
  checked = fbValue;
}

int GetValue(String path) {
  Firebase.get(firebaseData, (String)FIREBASEROOT+ path);
  return firebaseData.intData();
}

void GetVersionInfo() {
  Firebase.getJSON(firebaseData, "Version/" + (String)host);
}

void SetValue(String path, int value) {
  Firebase.set(firebaseData,FIREBASEROOT+ path, value);
}

void LogInfo(String json) {
  String logId = (String)year()+"_"+(String)month()+"_"+(String)day()+"/"+(String)hour()+"_"+(String)minute()+"_"+(String)second();
  Firebase.set(firebaseData,"LOGS/"+(String)host+"/"+logId, json);
}

void SetValue(String path, String value) {
  if (!Firebase.set(firebaseData, (String)FIREBASEROOT+"/"+(String)path , value)) {

    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
