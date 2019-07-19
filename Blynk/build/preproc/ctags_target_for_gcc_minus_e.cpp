# 1 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino"
# 1 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino"

# 3 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 4 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 5 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 6 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 7 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 8 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 9 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 10 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 11 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 12 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 13 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 14 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2
# 15 "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" 2

int iCount = 0;

Ticker ticker;

//OTA Setup Variables
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiClient espClient;
PubSubClient client(espClient);


WidgetRTC rtc;
WidgetTerminal terminal(2);
WidgetLED LED (4);
bool isConnected = false;
bool notificationSend = false;

//SystemParameters
const String Version = "2.65";
int geyserPin = 13;
bool shouldSaveConfig = false;

void BlynkOnConnected() {
  WriteMessage("Blynk Connected");
  if (!isConnected)
  {
    WriteMessage("Sync All");
    Blynk.syncAll();

    isConnected = true;
  }
  else
  {
    WriteMessage("No Sync");
  }
  rtc.begin();
  GetInfo();
}

void callback(char* topic, byte* payload, unsigned int length) {
  WriteMessage("\r\nMessage arrived [" + String(topic) + "] ");
  String msg = "";
  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    msg += receivedChar;
  }
  WriteMessage(msg);
  DynamicJsonDocument doc(2048);
  String input = msg;
  auto error = deserializeJson(doc, input);
  if (error) {
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom.text." "e:\\Git\\arduino\\Blynk\\Nursery_MQTT\\Nursery_MQTT.ino" "." "68" "." "19" "\""))) = ("deserializeJson() failed with code "); &__c[0];}))))));
    Serial.println(error.c_str());
    return;
  }
  String mac = doc["macAddress"];
  int rlength = strlen(rootTopic);
  if ( String(topic).substring(0, rlength) == rootTopic && String(mac) == WiFi.macAddress()) {
    if (String(topic) == String(rootTopic) + "/Time")
    {
      int h = doc["h"];
      int m = doc["m"];
      int s = doc["s"];
      int d = doc["d"];
      int mt = doc["m"];
      int y = doc["y"];
      setTime(h, m, s, d, mt, y);
      WriteMessage("Time Set!");
    }
    else
    {
      WriteMessage("UNKNOWN Topic for me " + String(topic));
    }
  }
  else
  {
    WriteMessage("Not Mine");
  }
}

void reconnect() {
  while (!client.connected()) {
    WriteMessage("Attempting MQTT connection...\r\n");
    // Attempt to connect
    char myClientId[WiFi.macAddress().length() + 1];
    WiFi.macAddress().toCharArray(myClientId, WiFi.macAddress().length() + 1);

    char lwtTopic[(String(rootTopic) + "/home/bedroom/"+host+"/availability").length() + 1];
    (String(rootTopic) + "/home/bedroom/"+host+"/availability").toCharArray(lwtTopic, (String(rootTopic) + "/home/bedroom/"+host+"/availability").length() + 1);

    if (client.connect(myClientId, lwtTopic, qosLevel, true, lwtMessage2.c_str())) {
      WriteMessage("connected");
      publisher("home/bedroom/"+String(host)+"/availability", "online");
    } else {
      WriteMessage("failed, rc=");
      WriteMessage(String(client.state()));
      WriteMessage(" try again in 5 seconds\r\n");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

void BlynkWidgetRead0 (BlynkReq __attribute__((__unused__)) &request)
{
  long rssi = WiFi.RSSI();
  WriteMessage("Update RSSI");

  publisher("home/bedroom/"+String(host)+"","{\"state\":\""+String(digitalRead(12))+"\",\"RSSI\":"+ String(rssi) + "\"}");
  Blynk.virtualWrite(0, rssi);
}

void availableCommands() {
  WriteMessage("You can only use the following commands :");
  WriteMessage("GetInfo");
}

void BlynkWidgetWrite12 (BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param)
{
  digitalWrite(12,!digitalRead(12));
  publisher("home/bedroom/"+String(host)+"","{\"state\":\""+String(digitalRead(12))+"\",\"RSSI\":"+ String(WiFi.RSSI()) + "}");
}

void BlynkWidgetWrite3 (BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param)
{
  iCount++;
  publisher("home/bedroom/"+String(host)+"","ON");
  WriteMessage((String)iCount);
  switch (iCount) {
    case 1:
      analogWrite(12, 100);
      break;
    case 2:
      analogWrite(12, 175);
      break;
    case 3:
      analogWrite(12, 245);
      break;
    default:
      analogWrite(12, 0);
      publisher("home/bedroom/"+String(host)+"","OFF");
      iCount = 0;
      break;
  }
}

void BlynkWidgetWrite2 (BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param)
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

void GetInfo() {
  WriteMessage("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
  WriteMessage("\r\nConnected To : " + WiFi.SSID() +
               "\r\nSignal :" + String(WiFi.RSSI()) +
               "\r\nIp Address : " + WiFi.localIP().toString() +
               "\r\nMac Address: " + WiFi.macAddress() +
               "\r\nVersion : " + Version );
}

void SetupOTA() {
  WriteMessage("OTA START");
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.on("/download", handleDownload);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  WriteMessage("OTA Done");
}

void setupPinMode(){
  pinMode(geyserPin, 0x01);
}

void saveConfig() {
   if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  StaticJsonDocument<200> doc;

  doc["mqtt_server"] = mqtt_server;

  doc["auth"] = auth;

  doc["rootTopic"] = rootTopic;

  doc["mqtt_port"] = mqtt_port;
  doc["host"] = host;

  WriteMessage("mqtt_server :" + String(mqtt_server));
  WriteMessage("auth :" +String(auth) );
  WriteMessage("Saving rootTopic :" +String(rootTopic));
  WriteMessage("Saving mqtt_port :" + String(mqtt_port));
  WriteMessage("Saving Host :" + String(host));

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return ;
  }

  String s;
  serializeJson(doc, s);
  configFile.println(s);
  WriteMessage(s);
  configFile.close();
}

bool setupConfigs() {

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  strcpy(mqtt_server , doc["mqtt_server"]);
  //mqtt_server = doc["mqtt_server"];
  mqtt_port = doc["mqtt_port"] ;
  strcpy(auth, doc["auth"]);

  strcpy(rootTopic, doc["rootTopic"]);
  strcpy(host, doc["host"]);

  Serial.println("Loaded Host: " + String(host));
  Serial.println("Loaded mqtt_server: " + String(mqtt_server));
  Serial.println("Loaded rootTopic: " +String(rootTopic));
  Serial.print("Blynk Key: " + String(auth));
  configFile.close();
  return true;
}

void handleDownload() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS failed to mount !\r\n");
  }
  else {
    String str = "";
    File f = SPIFFS.open(httpServer.arg(0), "r");
    if (!f) {
      Serial.println("Can't open SPIFFS file !\r\n");
    }
    else {
      WriteMessage("Start reading");
      char buf[1024];
      int siz = f.size();
      while (siz > 0) {
        size_t len = std::min((int)(sizeof(buf) - 1), siz);
        f.read((uint8_t *)buf, len);
        buf[len] = 0;
        str += buf;
        siz -= sizeof(buf) - 1;
      }
      f.close();
      WriteMessage("Value : " + str);
      httpServer.send(200, "text/plain", str);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  WriteMessage("Setup - START");
  WriteMessage("Serial Set");
  setupPinMode();
  WriteMessage("Pin modes Set");
  setupConfigs();
  WriteMessage("Configs set");
  setupWiFiManager();
  WriteMessage("WiFi Manager Set");
  SetupOTA();
  WriteMessage("OTA Complete");

  Serial.println(auth);
  Blynk.config(auth);
  Serial.println(auth);
  WriteMessage("Blynk Complete");
  client.setServer(mqtt_server, 1883);
  WriteMessage("MQTT Setup Complete");
  client.setCallback(callback);
}
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void tick()
{
  int state = digitalRead(2); // get the current state of GPIO1 pin
  digitalWrite(2, !state); // set pin to the opposite state
}

void configModeCallback (WiFiManager *myWiFiManager) {
  WriteMessage("Entered config mode");
  //if you used auto generated SSID, print it
  WriteMessage(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setupWiFiManager() {
  WiFiManager wifiManager;
  if(digitalRead(13)==0x1){
    wifiManager.resetSettings();
  }
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", auth, 32);
  WiFiManagerParameter custom_mqtt_root("rootTopic", "mqtt Root", rootTopic, 40);
  WiFiManagerParameter custom_host("host", "host", host, 40);

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.addParameter(&custom_mqtt_root);
  wifiManager.addParameter(&custom_host);

  wifiManager.setConfigPortalTimeout(240);
  wifiManager.autoConnect(host);

  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(host, custom_host.getValue());
  mqtt_port, custom_mqtt_port.getValue();
  strcpy(auth, custom_blynk_token.getValue());
  strcpy(rootTopic, custom_mqtt_root.getValue());

  if (shouldSaveConfig) {
    saveConfig();
  }
}


void loop()
{
  if (!client.connected()) {
    reconnect() ;
  }
  client.loop();
  Blynk.run();
  httpServer.handleClient();
}


void publisher(String theTopic, String msg) {
  char msgBuf[msg.length() + 1];
  msg.toCharArray(msgBuf, msg.length() + 1);
  theTopic = String(rootTopic) + "/" + theTopic;
  WriteMessage("Publishing " + msg + " to Topic " + theTopic);
  char topicBuf[theTopic.length() + 1];
  theTopic.toCharArray(topicBuf, theTopic.length() + 1);
  client.publish(topicBuf, msgBuf);
  client.loop();
}
