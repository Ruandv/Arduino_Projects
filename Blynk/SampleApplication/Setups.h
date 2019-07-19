
//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
  
void SetupLcd(){
  lcd.init();
  lcd.setBacklight(44);
}
void SetupOTA() {
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void SetupPinMode()
{
  
}

void SetupWiFiManager(){
  WiFiManager wifiManager; 
  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);
} 
