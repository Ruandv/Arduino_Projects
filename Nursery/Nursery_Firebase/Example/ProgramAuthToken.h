#include "Arduino.h"

char* Version = "3.9";
char* host = "xyz";
const int sensorPin = 12;
const int ledPin = 13;

#define FIREBASE_HOST ".firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH ""
#define FIREBASEROOT "ROOT"
#define FIREBASEPATH "/xyz_Status"
FirebaseData firebaseData;