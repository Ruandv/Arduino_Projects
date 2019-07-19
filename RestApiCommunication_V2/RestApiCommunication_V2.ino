#include "Adafruit_Sensor.h"
#include "DHT.h" 
#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
 
const String ServerIP = "192.168.1.106";
const String ServerPort = "8081";


#define DHTPIN 12     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11  

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

ESP8266WiFiMulti WiFiMulti;
SimpleTimer t ;
int myPorts[4] ={0,0,0,0};

void setup() {
    Serial.begin(115200);
    pinMode(5,OUTPUT);
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        digitalWrite(5,!digitalRead(5));
        delay(1000);
    }
    //WiFiMulti.addAP("Kransberg Turn office 2", "Mongoose");
    
    digitalWrite(5,LOW);
    while(WiFiMulti.run() != WL_CONNECTED){
      Serial.print(".");
    }  
   
}


void DoWork(){
   for (int pin=0; pin < 4; pin++) {
      if(myPorts[pin] !=0)
      {
        Serial.println(String(pin) + " " + String(myPorts[pin]));
        if(myPorts[pin]==12){
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          if (isnan(h) || isnan(t) ) {
            t=0;
            h=99;
          }
          String json = "{\"MacAddress\":\""+WiFi.macAddress()+"\",\"Pins\":[{\"PinId\":\""+String(myPorts[pin])+"\",\"Value\":\"{\\\"Temperature\\\":\\\""+t+"\\\",\\\"Humidity\\\":\\\""+h+"\\\"}\"}]}";
          PostRequest("Arduino/Update",json);
        }
        else
        {
          int val = digitalRead(myPorts[pin]);
          PostRequest("Arduino/Update","{\"MacAddress\":\""+WiFi.macAddress()+"\",\"Pins\":[{\"PinId\":\""+String(myPorts[pin])+"\",\"Value\":\""+String(val)+"\"}]");
        }
      }
   }
}

void loop() { 
  Serial.println("Sending...");
 PostRequest("api/temperature","22");
 Serial.println("Sending Done...");
 delay(3000);
}
 

JsonObject& GetRequest(String path){
  Serial.println("http://"+ServerIP+":"+ServerPort+"/api/"+path);
    String payload = "";
    if((WiFiMulti.run() == WL_CONNECTED)) {
      HTTPClient http;
      http.begin("http://"+ServerIP+":"+ServerPort+"/api/"+path); 
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "application/text");
      int httpCode = http.GET();
      
      if (httpCode > 0) {
        payload = http.getString();
      }
      http.end();
      return DeserializeJson(payload);
    }
    else{
      digitalWrite(5,HIGH);;
      Serial.println("No Network Connection");
    }
}

JsonObject& DeserializeJson(String json){
      Serial.println(json);
      Serial.flush();
      StaticJsonBuffer<2000> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(json);
      if (!root.success()) {
        digitalWrite(5,HIGH);
        Serial.println("ERROR PARSING THE JSON!!!!!!");
        Serial.println(json);
      }
      return root;
}

String SerializeDataArray(JsonArray& array1){
  String jsonStr;
  array1.printTo(jsonStr);
  return jsonStr;
}

String SerializeDataObject(JsonObject& obj){
  String jsonStr;
  obj.printTo(jsonStr);
  return jsonStr;
}

JsonObject& PostRequest(String path, String json){
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.println("http://"+ServerIP+":"+ServerPort+"/api/"+path);
        Serial.println(json);
        http.begin("http://"+ServerIP+":"+ServerPort+"/api/"+path); //HTTP
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Accept", "application/json");
        int result = http.POST(json); 
        String payload = http.getString(); 
        http.end();
        return DeserializeJson(payload);
    }
    else{
      digitalWrite(5,HIGH);
      Serial.println("No Network Connection");
    }
}


