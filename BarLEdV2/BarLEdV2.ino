#include <EEPROM.h>
#include <avr/sleep.h>
const byte redPin = 6;
const byte greenPin = 5;
const byte bluePin = 11;
int delayTime = 1;
unsigned long t = 0;
int maxDelayCounts = 10;
const byte interruptPin = 2;
int state = 1;
float del=0;
int stateAddress = 0;
int delayTimeAddress = 1;
unsigned long TimerA;
double debounce =0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), switchState, CHANGE);
  Serial.begin(115200);
  ReadValuesFromEprom();
}

void loop() {
  Serial.println( state);
  if(t>0)
  {
    Serial.println((millis() -t) / 1000.0);
  }
  switch(state){
    case 1:
      RgbSwitch();
    break;
    case 2:
      Fade();
    break;
    case 3:
      Rainbow();
    break;
    case 4:
      switchOff();
      delay(100);
      break;
    case 5:
      setColourRgb(255, 0, 0);
      break;
    case 6:
      setColourRgb(0, 255, 0);
      break;
    case 7:
      setColourRgb(0, 0, 255);
      break;
    default:
      state = 1;
      break;
  }
}
 

void Rainbow()
{
  Serial.println("Rainbow");
  while(state ==3)
  {
     unsigned int rgbColour[3];
    
    // Start off with red.
    rgbColour[0] = 255;
    rgbColour[1] = 0;
    rgbColour[2] = 0;  
    
    // Choose the colours to increment and decrement.
    for (int decColour = 0; decColour < 3; decColour += 1) {
      if(state != 3)
      {
        break;
      }
      int incColour = decColour == 2 ? 0 : decColour + 1;
    
      // cross-fade the two colours.
      int i = 0;
      TimerA = millis();
      while(i< 255) {        
        if(((millis()-TimerA))>(del/10))
        {
          if(state != 3)
          {
            break;
          }
          rgbColour[decColour] -= 1;
          rgbColour[incColour] += 1;
          
          setColourRgb(rgbColour[0], rgbColour[1], rgbColour[2]);
          
          TimerA = millis(); 
          i++;
        }    
      }
    }
  }
}

void Fade()
{  
  while(state==2)
  {
    TimerA = millis(); 
    Serial.println("Fade");
    switchOff(); 
    int i =0;
    while(i<255)
    {
      if(state!=2) 
      { 
        break;
      }
      if(((millis()-TimerA))>(del/10))
      {
        TimerA = millis(); 
        setColourRgb(i,0,0);
        i++;
      }    
    } 

    switchOff(); 
    i =0;
    while(i<255)
    {
      if(state!=2) 
      { 
        break;
      }
      if(((millis()-TimerA))>(del/10))
      {
        TimerA = millis(); 
        setColourRgb(0,i,0);
        i++;
      }    
    } 
    
   switchOff(); 
    i =0;
    while(i<255)
    {
      if(state!=2) 
      { 
        break;
      }
      if(((millis()-TimerA))>(del/10))
      {
        TimerA = millis(); 
        setColourRgb(0,0,i);
        i++;
      }    
    } 
  }
}

void setColourRgb(unsigned int red, unsigned int green, unsigned int blue) {
  if(t>0)
  {
    Serial.println((millis() -t) / 1000.0);
  }
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
 }

void RgbSwitch()
{
  int i = 0; 
  Serial.println("RGBSwitch");
  TimerA = millis(); 
  while(state==1)
  {
    if(((millis()-TimerA))> del)
    {
      switchOff(); 
      i++;
      TimerA = millis(); 
      if(i%3==0)
      {
        setColourRgb(0,0,254);
      }
      else if(i%3==1){
        setColourRgb(0,255,0);
      }
      else
      {
        setColourRgb(255,0,0);
      }
    }
  }
}

void switchState() {
  Serial.println("Button ");
  if(((millis()- debounce) /1000) > 0.100)
  {
    if(t <=0){
      t=millis();
    }
    else{
      double seconds = (millis() -t) / 1000.0;
      if(seconds <1)
      {
        updateState();
      }
      else if(seconds >1){
        updateDelayTime(seconds);
        if(delayTime>maxDelayCounts)
        {
          updateState();
          updateDelayTime(maxDelayCounts+1);
        }
        del =100*delayTime;
      }
      t=0;
    }
    debounce = millis();
  }
  else
  {
    debounce = millis(); 
  }
}

void updateDelayTime(int seconds){
  if(seconds>maxDelayCounts)
  {
    seconds =1;
    delayTime=0;
  }
  
  delayTime=seconds;
  EEPROM.write(delayTimeAddress, delayTime);
  Serial.println("delay Time Changed to : " + String(delayTime));
}

void updateState(){
  state = state+1;
  EEPROM.write(stateAddress, state);
  Serial.println("State Changed to : " + String(state));
}

void switchOff(){
  digitalWrite(redPin,LOW);
  digitalWrite(greenPin,LOW);
  digitalWrite(bluePin,LOW);
}

void ReadValuesFromEprom(){
  Serial.println("Getting values from Eeprom");
  EEPROM.get(delayTimeAddress, delayTime);
  int eeAddress = delayTimeAddress;
  eeAddress += sizeof(int);
  stateAddress =eeAddress;
  EEPROM.get(stateAddress,state); 
  delayTime = delayTime+256;
  state = state+256;
  Serial.println("delayTime : " +String(delayTime));
  Serial.println("state : " +String(state));
  del =100*delayTime;
}

