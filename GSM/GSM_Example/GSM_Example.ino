#include <SoftwareSerial.h>
#include "Variables.h"
#include "Common.h"
#include <FastLED.h>

SoftwareSerial GsmSerial(arduinoRx, arduinoTx); //RX(14), TX(16)
CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  setFastLedColour(CRGB::Black);
  pinMode(8, INPUT);
  setBaudRate();
   
  while(!isReady())
  {
    Serial.println("Trying again");
  }
  printCommands();
}

void printCommands() {
  Serial.println("Available Commands :");
  Serial.println("0. List all commands");
  Serial.println("1. Get sim number");
  Serial.println("2. Send sms to {number,message}");
  Serial.println("3. Get the modules time");
  Serial.println("4. Set the time {yy/MM/dd,HH:mm:ss}");
  Serial.println("5. Get signal strength");
  Serial.println("6. Add new contact number{index|contactNumber|name}");
  Serial.println("7. Set characterSet to GSM");
  Serial.println("8. Query characterSet");
  Serial.println("9. Power off module");
  Serial.println("10. View Contact {index}");
  Serial.println("11. Check mobile status");
  Serial.println("12. Make a call {contactNumber}");
  Serial.println("13. Hang up call");
  Serial.println("14. View contact list");
  Serial.println("15. Clear contact list");
  Serial.println("16. Set default sms storage location {storageLocation}");
  Serial.println("17. Read sms message");
  
}

void loop() {
  //if (isReady())
  //{ 
    delay(1000);
    String data = readSerialResponse();
    String content= Split(data, "|", 0); 
    content.trim();
    if (content != "" && content != "WTF") {
      
      if (content == "0")
      {
        printCommands();
      }
      else if (content == "1")
      {
        Serial.println("The sim number is : " + getContactNumber());
      }
      else if (content == "2")
      {
        Serial.println("Sending sms " + data);
        String numberToSms,   message;
        numberToSms= Split(data, "|", 1);
        message= Split(data, "|", 2);
        SmsMe(numberToSms, message);
      }
      else if (content == "3")
      {
        result = Split(data, "|", 1);
        String d = setNewTime(result, false);
        Serial.println("Time : " + d);
      }
      else if (content == "4")
      {
        result = Split(data, "|", 1);
        setNewTime(result, true);
        result = Split(data, "|", 1);
        String d = setNewTime(result, false);
        Serial.println("Time : " + d);
      }
      else if (content == "5")
      {
        Serial.println("Signal : " + String(SignalStrength()));
      }
      else if (content == "6")
      {
        String index,contactNumber,contactName;
        index = Split(data, "|", 1);
        contactNumber = Split(data, "|", 2);
        contactName = Split(data, "|", 3);
        Serial.println(addContactNumber( index.toInt(),contactNumber,contactName ));
      }
      else if (content == "7")
      {
        Serial.println(CharacterSet(defaultCharacterset, true));
      }
      else if (content == "8")
      {
        Serial.println(CharacterSet(defaultCharacterset, false));
      }
      else if (content == "9")
      {
        Serial.println(powerOff());
      }
      else if (content == "10")
      {
        result = Split(data, "|", 1);
        result = Split(viewContactNumber(result.toInt()),"|",1);
        String contactInfo = result;
        result = Split(contactInfo,",",1);
        String cellNumber = result;
        result = Split(contactInfo,",",3);
         String cellName =result;
        Serial.println(cellName + " : " + cellNumber);
      }
      else if (content == "11")
      {
        Serial.println(CheckMobileStatus());
      }
      else if (content == "12")
      {
        result = Split(data, "|", 1);
        Serial.println(MakeCall(result));
      }
      else if (content == "13")
      {
        Serial.println(TerminateCall());
      }
      else if (content == "14") {
        setFastLedColour(CRGB::Blue);
        for (int i = 1; i < 11; i++) {
          result = Split(viewContactNumber(i),"|",1);
          String contactInfo = result;
          result = Split(contactInfo,",",1);
          String cellNumber = result;
          result = Split(contactInfo,",",3);
          String cellName = result;
          Serial.println(String(i) + " " + cellName + " : " + cellNumber);
        }
      }
      else if (content == "15") {
        setFastLedColour(CRGB::Red);
        for (int i = 1; i < 11; i++) {
          String cellNumber = "0";
          String cellName = "Unknown";
          addContactNumber( i, cellNumber, cellName);
          Serial.println(String(i) + " " + cellName + " : " + cellNumber);
        }
      }
      else if (content == "16")
      {
        result = Split(data, "|", 1);
        Serial.println(SmsStorageLocation(result,true));
      }
     
      else if (content != "")
      {
        result = Split(data, "|", 0);
        result=result+ "\r";
        Serial.println(result);
        SubmitATCommand(result);
        delay(100);
        String resp = readSimResponse();
        Serial.println("resp 1:" + resp);
        result = Split(resp, "|", 0);
        Serial.println("Response 0:" + result);
        result = Split(resp, "|", 1);
        Serial.println("Response 1:" + result);
        result = Split(resp, "|", 2);
        Serial.println("Response 2:" + result);
        result = Split(resp, "|", 3);
        Serial.println("Response 3:" + result);
      }
    }
  //}
}

String readSerialResponse() {
  String content = "";
  if (Serial.available())
  {
    content = Serial.readString();
  }
  content.replace("\r\n", "|");
  content.replace("\r", "|");
  return content ;
}
void SubmitATCommand(String cmd)
{
  if (!cmd.endsWith("\r")) {
    cmd = cmd + "\r";
  }
   Serial.println(cmd);
  if ( digitalRead(8) == HIGH && (!cmd.startsWith("AT+CPAS") && !cmd.startsWith("AT+CSQ") ||debugATCommands))
  {
    Serial.println(cmd);
  }
  
  GsmSerial.write(cmd.c_str());
  delay(100);
}
String readSimResponse() {
  String cmd = "";
  if (GsmSerial.available())
  {
    cmd = GsmSerial.readString();
  }
  cmd.replace("\r\n", "|");

  if ( digitalRead(8) == HIGH && (!cmd.startsWith("AT+CPAS") && !cmd.startsWith("AT+CSQ") ||debugATCommands))
  { 
    Serial.println("Sim Response:" + cmd);
    /*Serial.println("Response 0:" + Split(content, "|", 0));
    Serial.println("Response 1:" + Split(content, "|", 1));
    Serial.println("Response 2:" + Split(content, "|", 2));
    Serial.println("Response 3:" + Split(content, "|", 3));*/
  } 
  return cmd ;
}

String powerOff() {
  String val = "AT+CPWROFF\r";
  SubmitATCommand(val);
  return readSimResponse() ;
}

String MakeCall(String mobileNumber)
{
  if (mobileNumber == "")
  {
    mobileNumber = myNumber;
  }

  String val = "ATD" + myNumber + ";\r";
  SubmitATCommand(val);
  return readSimResponse() ;
}

String TerminateCall()
{
  String val = "ATH0\r";
  SubmitATCommand(val);
  return readSimResponse() ;
}

String SmsStorageLocation(String storageLocation, bool setIt){
  String val = "AT+CPMS";
  if(storageLocation==""||storageLocation=="WTF"){
    storageLocation = defaultStorageLocation;
  }
  if (setIt)
  {
    val = val + "=" + storageLocation + "\r";
  }
  else
  {
    val = val + "?\r";
  }
  SubmitATCommand(val);
  return readSimResponse() ;
}


String CharacterSet(String characterSet, bool setIt) {
  String val = "AT+CSCS";
  if (setIt)
  {
    val = val + "=\"" + characterSet + "\"\r";
  }
  else
  {
    val = val + "?\r";
  }
  SubmitATCommand(val);
  return readSimResponse() ;
}

String addContactNumber(int index, String number, String contactName) {
  if (number == "")
  {
    number = myNumber;
  }
  if (contactName == "")
  {
    contactName = myName;
  }
  if (index == 0)
  {
    index = defaultPBIndex;
  }

  result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
  if (result != "+CSCS: \"GSM\"")
  {
    result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
    String msg = "Incorrect Characterset - Try again" + result;
    CharacterSet(defaultCharacterset, true);
    return msg;
  }
  else
  {
    String val = "AT+CPBW=" + String(index) + ",\"" + number + "\",129,\"" + contactName + "\"\r";
    SubmitATCommand(val);
    return readSimResponse() ;
  }
}

String viewContactNumber(int index) {
  if (index == 0)
  {
    index = defaultPBIndex;
  }
  result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
  if (result != "+CSCS: \"GSM\"")
  {
     result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
     String msg = "Incorrect Characterset - Try again" + result;
    CharacterSet(defaultCharacterset, true);
    return msg;
  }
  else
  {
    String val = "AT+CPBR=" + String(index) + "\r";
    SubmitATCommand(val);
    return readSimResponse() ;
  }
}

String setNewTime(String newTime, bool mustSetTime) {
  if (newTime.indexOf("/") < 2)
  {
    newTime = defaultDateTime;
  }

  String val = "AT+CCLK=\"" + newTime + "\"\r";

  if (!mustSetTime)
  {
    val = "AT+CCLK?\r";
  }
  SubmitATCommand(val);

  val = readSimResponse();
  int first = val.indexOf("CCLK: \"") + 7;
  int two = val.indexOf("\"", first);

  return val.substring(first, two);
}

String getContactNumber() {
  String val = "AT+CNUM\r";
  SubmitATCommand(val);
  val = "";
  val = readSimResponse();
  int first = val.indexOf(",");
  return val.substring(first + 2, first + 13);
}

int SignalStrength() {
  String val = "AT+CSQ\r";
  SubmitATCommand(val); 
  val = readSimResponse();  
  result = Split(val, "|", 1); 
  result.replace("+CSQ: ", ""); 
  val = Split(result, ",", 0);
  return val.toInt();
}

bool isReady() {
  bool isConnected = false;
  int i = 0;
  int mobileStatus = 0;

  while (!isConnected && i < 20) {
    delay(2000);
    int v = SignalStrength();
    Serial.println(String(i) + "Mobile Sinal " +String(v));
    mobileStatus = CheckMobileStatus();
    Serial.println(String(i) + "Mobile Satus " +String(mobileStatus));
    
    isConnected = (mobileStatus == 0 || mobileStatus == 3 || mobileStatus == 4) && (v > 4 && v < 99);
    setFastLedColour(CRGB::Black);
    setFastLedColour(CRGB::Tan, i % 3);
    i++;
  }

  if (isConnected && i < 20 && mobileStatus == 0)
  {
    setFastLedColour(CRGB::Green);
  }
  else if (isConnected && i < 20 && (mobileStatus == 3 || mobileStatus == 4))
  {
    Serial.println("Mobile Satus " +String(mobileStatus));
    if (mobileStatus == 3)
    {
      String val = "At+clip=1\r";
      SubmitATCommand(val);
      val = readSimResponse();
      result = Split(val, "|", 5) ;
      result.replace("+CLIP: ", "");
      result = Split(result, ",", 0);
      Serial.println("Incomming call from " + result);
    }
    //if(isKnownNumber(val))
    //{
    //  setFastLedColour(CRGB::LightCyan);
    //}
    //else
    //{
    setFastLedColour(CRGB::Indigo);
    //}
  }
  else
  {
    setFastLedColour(CRGB::Red);
  }
  return isConnected && i < 20;
}

int CheckMobileStatus()
{
  String val = "AT+CPAS\r";
  SubmitATCommand(val);
  result  = Split(readSimResponse(), "|", 1);
  result.replace("+CPAS: ", "");
  return result.toInt();
}

void setBaudRate() {
  Serial.begin(57600);
  Serial.println("Test GSM SMS Project");
  GsmSerial.begin(57600);
  /*String val = "AT+IPR="+String(57600);
    SubmitATCommand(val);*/
}
void setFastLedColour(CRGB colour)
{
  for ( int i = 0; i < NUM_LEDS; i++) {
    setFastLedColour(colour, i);
  }
}

void setFastLedColour(CRGB colour, int index)
{
  leds[index] = colour;
  FastLED.show();
}

void SmsMe(String numberToSms, String message)
{
  if (numberToSms == "") {
    numberToSms = myNumber;
  }
  if (message == "") {
    message = defaultMessage;
  }
  result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
  if ( result!= "+CSCS: \"GSM\"")
  {
    result = Split(CharacterSet(defaultCharacterset, false), "|", 1);
    String msg = "Incorrect Characterset - Try again" + result;
    CharacterSet(defaultCharacterset, true);
    return msg;
  }
  else
  {
    SubmitATCommand("AT+CMGF=1\r");
    Serial.println(readSimResponse());
    SubmitATCommand(("AT+CMGS=\"+" + numberToSms  + "\"\r"));
    Serial.println(readSimResponse());
    SubmitATCommand(message + "\r");
    delay(1000);
    Serial.println(readSimResponse());
    GsmSerial.write(0x1A);
    delay(1000);
    Serial.println(readSimResponse());
  }
}

