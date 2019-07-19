#include "Arduino.h"

//char auth[] = "3960e6d41d634e18ae94b0af9a023fcc"; //This is where you must put your BLYNK Token
char auth[] = "5afc683551db46fcac159605436f6a75";
//char auth[] = "2798debfb776493a8483a1d1b8a15f66";
const char* mqtt_server = "192.168.1.60";
const char* host = "ESP8266SmartGeyser" ;
const String rootTopic = "kransberg";
const int qosLevel = 1;
String lwtMessage2 =  "offline";
