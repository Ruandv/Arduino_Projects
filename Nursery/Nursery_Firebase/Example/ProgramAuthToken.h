#include "Arduino.h"

char* auth = "";
char* Version = "3.1";
char* host = "Nursery Firebase";
const int sensorPin = 12;
const int ledPin = 13;

#define FIREBASE_HOST "" //Without http:// or https:// schemes
#define FIREBASE_AUTH ""

#define FIREBASEPATH ""
FirebaseData firebaseData;
FirebaseJson json;

void printResult(FirebaseData &data);
