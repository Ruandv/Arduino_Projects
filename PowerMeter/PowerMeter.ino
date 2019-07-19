// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"             // Include Emon Library
 
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x3F,2,1,0,4,5,6,7);


#define VOLT_CAL 148.7
#define CURRENT_CAL 54.6

EnergyMonitor emon1;

void setup()
{  
  Serial.begin(9600);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.begin (16,2); // for 16 x 2 LCD module
  printToScreen("Initializing","Device");
  emon1.current(0, CURRENT_CAL);       // Current: input pin, calibration.
}

void loop()
{
  emon1.calcVI(20,1800);         // Calculate all. No.of half wavelengths (crossings), time-out
  float currentDraw = emon1.Irms;             //extract Irms into Variable
  float supplyVoltage =220;
  printToScreen("Current: " + String(currentDraw),
                "Watts: " + String(float(currentDraw) * float(supplyVoltage)));
}

void printToScreen(String line1,String line2){
  lcd.setCursor (0,0);   // set cursor to 0,0
  lcd.print(line1); 
  lcd.setCursor (0,1);        
  lcd.print(line2);
}
