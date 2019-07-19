#include <SoftwareSerial.h>
#include <SimpleTimer.h>
SimpleTimer balanceTimer;
const char BalanceUssdCode[] = "AT+CUSD=1,\"*141#\",15\r";
const char CloseBalanceUssdCode[] = "AT+CUSD=1,\"00\",15";

const int arduinoRx = 6;
const int arduinoTx = 5;
SoftwareSerial GsmSerial(arduinoRx, arduinoTx); //RX(14), TX(16)
String globalData = "";
bool gsmUp = false;

void setup()
{
  Serial.begin(57600);
  GsmSerial.begin(57600);  //Baud rate of the GSM/GPRS Module
  
  while (!gsmUp)
  {
    String d = "";
  
    Serial.print(".");
    while (GsmSerial.available()) {
      d += char(GsmSerial.read());
      Serial.println(d);
    }
    
    if (d.indexOf("ODEM") > 0)
    {
      Serial.println("odem");
    }
    if (d.indexOf("PBREADY") > 0)
    {
      gsmUp = true;
    }
    delay(1000);
  }
  GsmSetup();
  int timerId = balanceTimer.setInterval(60000, checkBalance);
  PrintCommands();
}

void checkBalance(){
  Serial.println("Checking your balance");
  String data = GetBalance();
  String balance = GetSplit(data,'*',0);
  Serial.println("Your balance is : " + balance);
}
void PrintCommands() {
  Serial.println("\r\r\r\r");
  Serial.println("Available Commands : ");
  Serial.println();
  Serial.println("GetBalance();");
  Serial.println("SendingSMS~[27number]~[message]");
  Serial.println("ReadExistingNumbers");
  Serial.println("AddNewNumber~[index]~[27number]~[contactName]");
  Serial.println("AT+{Command}\r\r\r");
}

void GsmSetup() {
  Serial.println("GSM Setup : Start");
  GsmSerial.print("AT+ENPWRSAVE=0\r");
  delay(1000);
  GsmSerial.print("AT+CMGF=1\r");
  delay(1000);
  GsmSerial.print("AT+CSCS=\"GSM\"\r");
  delay(1000);
  GetResponse(5000);
  Serial.println("GSM Setup : Done");
}

void loop()
{
   //balanceTimer.run();
  if (GsmSerial.available()) {
    String str = GetResponse(5000);
    if (str.indexOf("RING") > 0)
    {
      //We can do something later when we get a call
    }
    else
    {
      Serial.println("MODEM REPLIED : " + str);
    }
  }

  if (Serial.available() > 0) {
    char incomingByte = Serial.read(); 
    if ((incomingByte == '\n' || incomingByte == '\r') && globalData != "")
    {
      String cmd = GetSplit(globalData , '~', 0);// Get the Command Name

      if (cmd == "ReadExistingNumbers") {
        Serial.println(ReadExistingNumbers());
      }
      else if (cmd == "AddNewNumber")
      {
        String index = GetSplit(globalData , '~', cmd.length() + 1);
        String number = GetSplit(globalData , '~', cmd.length() + 1 + index.length() + 1);
        String contactName = GetSplit(globalData , '~', cmd.length() + 1 + index.length() + 1 + number.length() + 1);
        
        if (AddNewNumber(index, number, contactName))
        {
          //After adding the New Number then SMS the person.
          SendingSMS(number, "Welcome " + contactName + " to the Notification System");
        }
      }
      else if (cmd == "SendingSMS")
      {
        String number = GetSplit(globalData , '~', cmd.length() + 1);
        String message = GetSplit(globalData , '~', number.length() + 1 + cmd.length() + 1);
        SendingSMS(number, message);
      }
      else if (cmd == "GetBalance") //GetBalance
      {
        GetBalance();
      }
      else
      {
        //WE dont have this command in yet so lets just pass it to the Modem and see what comes back
        GsmSerial.print(globalData  + "\r");
      }
      globalData  = "";
    }
    else
    {
      globalData  += String(incomingByte);
    }
  }
}

String GetBalance() {
  GsmSetup();
  GsmSerial.print(BalanceUssdCode);
  Serial.println("going to wait for rsp "+ String(millis()));
  String resp = GetResponse(15000);
  Serial.println(resp);
  Serial.println(resp.length());

  int startAt = resp.indexOf("*Airtime") +9 ;
  int endAt = resp.indexOf("*SMS");
  String airtime = resp.substring(startAt,endAt); 
  
  startAt = resp.indexOf("*SMS") + 5;
  endAt = resp.indexOf("*Data");
  String sms = resp.substring(startAt,endAt);
 
  startAt = resp.indexOf("*Data")+6;
  endAt = resp.indexOf("**Select");  
  String myData = resp.substring(startAt,endAt);

  return airtime + "*" + sms + "*" + myData ;
}

void WriteMessage(String msg, char searchFor, int skip){
 
  String internal = msg.substring(skip);
  int i = internal.indexOf(searchFor);
  Serial.println(msg + "-ABCDEF");
}

String ReadExistingNumbers()
{
  String myString = "";
  GsmSetup();
  GsmSerial.print("AT+CPBS=\"SM\"\r");
  GetResponse(5000);
  for (int i = 1; i < 10; i++) {
    Serial.println("Reading Number : " + String(i));
    GsmSerial.print("AT+CPBR=" + String(i) + "\r");
    String resp = GetResponse(5000);
    String index  = String(i);
    String number = GetSplit(resp, '\"', String("AT+CPBR=" + index + "~~*+CPBR: " + index + ",\"").length());
    String type = GetSplit(resp, ',', String("AT+CPBR=" + index + "~~*+CPBR: " + index + ",\"").length() + 2 + number.length());
    String contactName = GetSplit(resp, '\"', String("AT+CPBR=" + index + "~~*+CPBR: " + index + ",\"").length() + 2 + number.length() + type.length() + 2);
    myString += index + "," + contactName + "," + number + "*";
  }
  return myString;
}

bool GetResponseIsOk(int waitingTime)
{
  String resp = GetResponse(waitingTime);
  bool b =  IsOk(resp);
  if(!b)
  {
    Serial.println("GetResponseIsOk ERROR : " + resp);
  }
} 

String GetResponse(int waitingTime) {
  String response = "";

  unsigned long lastRead = millis();   // last time a char was available
  while (millis() - lastRead < waitingTime){
    while (GsmSerial.available()) {
      response += char(GsmSerial.read());
      lastRead = millis();
    }
    if(IsOk(response)){
      Serial.println("Breaking out because all was received");
      break;
    }
  }
  response.replace("\r", "~");
  response.replace("\n", "*");

  return response;
}

//SendingSMS~27[NUMBER]~Testing the system
void SendingSMS(String number, String message)
{
  Serial.println("Sending message " + message );
  GsmSetup();   
  GsmSerial.print("AT+CMGS=\"+" + String(number) + "\"\r");    //Number to which you want to send the sms
  delay(1000);
  GsmSerial.println(message);   //The text of the message to be sent    
  delay(1000);
  GsmSerial.write(0x1A);
  delay(1000);
  String resp = GetResponse(5000);
  Serial.println("Sending Response " + resp);
}

bool IsOk(String response)
{
  return response.endsWith("~*OK~*");
}

//AddNewNumber~X~27[NUMBER]~JACK
bool AddNewNumber(String index, String number, String contactName)
{
  GsmSetup();
  //Set storage to sim
  GsmSerial.print("AT+CPBS=\"SM\"\r");
  GetResponse(5000);
  GsmSerial.print("AT+CPBW=" + index + ",\"" + number + "\",129,\"" + contactName + "\"\r");
  String response = GetResponse(5000);
  return IsOk(response);
}

String GetSplit(String results, char searchFor, int skip) 
{
  String internal = results.substring(skip);
  int i = internal.indexOf(searchFor);
  return internal.substring(0, i);
}

