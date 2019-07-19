bool isBlynkConnected;
bool pendingTweet;

LiquidCrystal_I2C lcd(0x3f,16,2);

//Blynk Variables 
WidgetRTC rtc;
WidgetTerminal terminal(V2);
WidgetLED led1(V1);


void WriteMessage(String msg)
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  msg = "[" + currentDate  + " " + currentTime + "] \r\n " + msg;
  if (isBlynkConnected)
  {
    terminal.println(msg);
    terminal.flush();
  }
  Serial.println(msg);
}

void LcdWriteMessage(String msg)
{
  lcd.print(msg); 
}

void DisableTweetFlags(){
  pendingTweet = false;
  led1.off();
}

void EnableTweetFlags(){
  pendingTweet = true;
  led1.on();
}

void TweetWriteMessage(String msg)
{
  Blynk.tweet(msg);
  DisableTweetFlags();
}


void GetInfo() {
  WriteMessage("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
  WriteMessage("\r\nConnected To : " + WiFi.SSID() +
               "\r\nSignal :" + String(WiFi.RSSI()) +
               "\r\nIp Address : " +  WiFi.localIP().toString()  +
               "\r\nVersion : " + Version);
}


BLYNK_CONNECTED() {
  rtc.begin();
  WriteMessage("Blynk Connected");
  if (!isBlynkConnected)
  {
    isBlynkConnected = true;
    WriteMessage("Sync All");
    Blynk.syncAll();
  }
  else
  {
    WriteMessage("No Sync");
  }
  GetInfo();  
}

