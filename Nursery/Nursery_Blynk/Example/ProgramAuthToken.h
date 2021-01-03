#include "Arduino.h"

char* auth = "1r0-xxx";
char* Version = "3.2";
char* host = "xxx Blynk";
const int sensorPin = 12;
const int ledPin = 13;

#define FIREBASE_HOST ".firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH ""

#define FIREBASEROOT "xxr"
#define FIREBASEPATH "/Light_xxx"
FirebaseData firebaseData;
