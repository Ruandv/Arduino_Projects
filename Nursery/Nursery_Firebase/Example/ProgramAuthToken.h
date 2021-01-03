#include "Arduino.h"

char* Version = "3.7";
char* host = "YourHost";
const int sensorPin = 12;
const int ledPin = 13;

#define FIREBASE_HOST ".firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH ""

#define FIREBASEPATH "NB"
FirebaseData firebaseData;