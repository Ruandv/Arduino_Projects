#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "DHTesp.h" 

const char* ssid = "Kransberg Turn office 2";
const char* password = "Mongoose";
const char* host = "http://myiotcorewebapi.azurewebsites.net";

DHTesp dht;
HTTPClient http;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
} 

void loop() {
  
  dht.setup(13, DHTesp::DHT11);
  delay(dht.getMinimumSamplingPeriod());
 
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  if(dht.getStatusString() !="TIMEOUT")
  {
    http.begin(String(host)+"/api/myesp/PinModes");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST("[{\"pinType\": \"Input\", \"pinId\": \"12\", \"value\": \""+String(temperature)+"\" }]");
    String payload = http.getString();
    Serial.println("payLoad " + payload);
    http.end();
    Serial.println(httpCode);
    delay(900000);//15Minutes
  }
}
