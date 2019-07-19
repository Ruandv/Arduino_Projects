/*
  If the ESP does not want to connect to the correct WIFI or you have connection trouble
  use this file to reset the settings on the ESP and then upload your Application again.
*/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
void setup() {
    Serial.begin(115200);
    WiFiManager wifiManager;
    //reset saved settings
    wifiManager.resetSettings();
    
    wifiManager.autoConnect("AutoConnectAP");
    
    Serial.println("connected...yeey :)");
}

void loop() {
    // put your main code here, to run repeatedly:
    
}
