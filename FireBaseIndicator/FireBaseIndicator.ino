#include "FirebaseESP8266.h"
#include "secrets.h"    //MAKE SURE YOU COPY THIS FILE FROM EXAMPLES
#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>
#include <FastLED.h>
#include <Wire.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//flag for saving data

Ticker blinker;
int blinkerCount = 0;
WiFiManager wifiManager;

#define DATA_PIN 12
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];
FirebaseData firebaseData;


void setLED(int r, int g, int b) {
  leds[0] = CRGB( g, r, b);
  FastLED.show();
}

void noWiFi()
{
  for (int i = 0; i < 200; i++) {
    setLED(i, 0, 0);
    delay(100);
  }
}
void sucessfulDeployment()
{
  if (blinkerCount % 2 == 0)
  {
    setLED(0, 255, 0);
  }
  else
  {
    setLED(0, 0, 0);
  }
  blinkerCount++;
  if (blinkerCount > 8) {
    setLED(0, 255, 0);
    blinker.detach();
    blinkerCount = 0;
  }
}

void failedDeployment()
{
  if (blinkerCount % 2 == 0)
  {
    setLED(255, 0, 0);
  }
  else
  {
    setLED(0, 0, 0);
  }
  blinkerCount++;
  if (blinkerCount > 8) {
    setLED(255, 0, 0);
    blinker.detach();
    blinkerCount = 0;
  }
}

void enableSPIFF() {
  setLED(125, 90, 180);
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
          Serial.println("\nparsed json");
          strcpy(firebase_Root, json["firebase_Root"]);
          strcpy(firebase_Secret, json["firebase_Secret"]);
          strcpy(AP_Names, json["AP_Names"]);
          strcpy(firebase_Organization, json["firebase_Organization"]);
          strcpy(firebase_SystemName, json["firebase_SystemName"]);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  Serial.begin(115200);
  enableSPIFF();
  //wifiManager.resetSettings();
  configureWiFiManagerParameters();
  blinker.detach();
  setLED(0, 0, 0);
  Firebase.begin(firebase_Root, firebase_Secret);

}

void configureWiFiManagerParameters()
{
  WiFiManagerParameter test_param("APName", "APName", AP_Names, 40);
  WiFiManagerParameter firebase_Root_param("FireBaseRoot", "FireBase", firebase_Root, 40);
  WiFiManagerParameter firebase_Secret_param("Secret", "Secret", firebase_Secret, 60);
  WiFiManagerParameter firebase_Organization_param("Organization", "Organization", firebase_Organization, 60);
  WiFiManagerParameter firebase_SystemName_param("SystemName", "SystemName", firebase_SystemName , 60);
  wifiManager.addParameter(&test_param);
  wifiManager.addParameter(&firebase_Root_param);
  wifiManager.addParameter(&firebase_Secret_param);
  wifiManager.addParameter(&firebase_Organization_param);
  wifiManager.addParameter(&firebase_SystemName_param);
  wifiManager.setTimeout(120);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect(AP_Names);
}

void saveConfigCallback () {
  Serial.println("saving config");
  DynamicJsonDocument json(1024);
  json["firebase_Root"] = firebase_Root;
  json["firebase_Secret"] = firebase_Secret;
  json["AP_Names"] = AP_Names;
  json["firebase_Organization"] = firebase_Organization;
  json["firebase_SystemName"] = firebase_SystemName;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }
  serializeJson(json, Serial);
  serializeJson(json, configFile);
  configFile.close();
}

void configModeCallback (WiFiManager *myWiFiManager) {
  blinker.attach(10, noWiFi);
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  
  Firebase.getJSON(firebaseData, "/" + (String)firebase_Organization + "/" + (String)firebase_SystemName);
  if (firebaseData.dataType() == "json")
  {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, getKey(true));
    JsonObject obj = doc.as<JsonObject>();
    Serial.println("eventDate : " + obj["eventDate"].as<String>());
    Serial.println("status : " + obj["status"].as<String>());
    if ( obj["status"].as<String>() == "succeeded") {
      blinker.attach(1, sucessfulDeployment);
    }
    else
    {
      blinker.attach(1, failedDeployment);
    }
    Firebase.deleteNode(firebaseData,  "/" + (String)firebase_Organization + "/" + (String)firebase_SystemName + "/" + getKey(false));
  }
  delay(1000);
}

String getKey(bool valueOnly) {
  FirebaseJson &json = firebaseData.jsonObject();
  size_t len = json.iteratorBegin();
  String key, value = "";
  int type = 0;
  for (size_t i = 0; i < 1; i++)
  {
    json.iteratorGet(i, type, key, value);
    if (valueOnly == false)
      return key;
    else
      return value;
  }
  json.iteratorEnd();
}
