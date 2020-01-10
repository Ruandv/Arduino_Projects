#include "Arduino.h"

char auth[] = "5f63523";
const char* mqtt_server = "192.168.1.60";
const char* host = "ESP8266SmartGeyser" ;
const String rootTopic = "d";
const int qosLevel = 1;
String lwtMessage2 =  "offline";
