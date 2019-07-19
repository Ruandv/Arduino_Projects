#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>   
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
    
#include "ProgramAuthToken.h"  
#include "GeneralMethods.h"  
#include "BlynkMethods.h"  
#include "Setups.h"  



void setup()
{ 
  Serial.begin(115200);
  SetupLcd();
  SetupPinMode();
  SetupWiFiManager();
  SetupOTA();
  Blynk.config(auth); 
} 

void loop()
{
  Blynk.run(); 
  httpServer.handleClient();   
}

