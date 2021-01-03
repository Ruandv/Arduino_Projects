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

void setClock() {
  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

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
  //Blynk.config(auth);
}
void updateFirmware() {
  GetVersionInfo();
  const String json = firebaseData.stringData();
  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  const char* DBVersion = doc["Version"];
  const char* FIRMWARE_URL = doc["URL"];
  Serial.println("DBVersion " + (String) DBVersion);
  Serial.println("Version " + (String)Version);
  if ((String)DBVersion != (String)Version)
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    Serial.println("Downloading file... " + (String)FIRMWARE_URL );
    t_httpUpdate_return ret = ESPhttpUpdate.update(FIRMWARE_URL);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }
  else
  {
    Serial.println("firmware up to date");
  }
}
bool checked = true;
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
  Firebase.get(firebaseData, path);
  return firebaseData.intData();
}

void GetVersionInfo() {
  Firebase.getJSON(firebaseData, "Version");

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
