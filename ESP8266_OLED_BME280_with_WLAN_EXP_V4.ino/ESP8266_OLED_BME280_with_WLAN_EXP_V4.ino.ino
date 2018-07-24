/*
 OLED 0.96 inch+ BME280 + Nextion Display
for NodeMCU 1.0(ESP-12E Module)
From:
https://gist.github.com/kuc-arc-f/c7d902283e62cba5e8318316ed624329
 
 OLED:
 https://github.com/squix78/esp8266-oled-ssd1306



Wiring to OLED Display
D1 (GPIO5) to SDK of OLED
D2 (GPIO4) to SDA of OLED
3V to VDD of OLED
G to GND of OLED

Wiring to BME280
3V to VIN of BME280
G to GND of BME280
D1 (GPIO4) to SDL of BMP180
D2 (GPIO5) to SCA of BMP280
*/

#include <Wire.h>
#include <BME280_MOD-1022.h>
#include "SSD1306.h"


//
SSD1306   display(0x3c, 4, 5);



double mTemp=0;
double mHumidity=0;
double mPressure=0;

uint32_t mNextHttp= 30000;
uint32_t mTimerTmpInit=30000;
uint32_t mTimerDisp;
uint32_t mTimerTime;
uint32_t mTimerTemp;

// Arduino needs this to pring pretty numbers
void printFormattedFloat(float x, uint8_t precision) {
char buffer[10];

  dtostrf(x, 7, precision, buffer);
  
}

// print out the measurements
void printCompensatedMeasurements(void) {
    mTemp      = BME280.getTemperatureMostAccurate();
    mHumidity  = BME280.getHumidityMostAccurate();
    mPressure  = BME280.getPressureMostAccurate();
}

//
void init_BME280(){
  uint8_t chipID;
  
    chipID = BME280.readChipId();
  
   
  // need to read the NVM compensation parameters
  BME280.readCompensationParams();
  
  // Need to turn on 1x oversampling, default is os_skipped, which means it doesn't measure anything
  BME280.writeOversamplingPressure(os1x);  // 1x over sampling (ie, just one sample)
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os1x);
  
  // example of a forced sample.  After taking the measurement the chip goes back to sleep
  BME280.writeMode(smForced);
    while (BME280.isMeasuring()) {
    delay(50);
  }
 
  
  // read out the data - must do this before calling the getxxxxx routines
  BME280.readMeasurements();
  // Example for "indoor navigation"
  // We'll switch into normal mode for regular automatic samples  
  BME280.writeStandbyTime(tsb_0p5ms);        // tsb = 0.5ms
  BME280.writeFilterCoefficient(fc_16);      // IIR Filter coefficient 16
  BME280.writeOversamplingPressure(os16x);    // pressure x16
  BME280.writeOversamplingTemperature(os16x);  // temperature x2
  BME280.writeOversamplingHumidity(os16x);     // humidity x1
  
  BME280.writeMode(smNormal);
}


//
void setup()   {                
    mTimerTemp= mTimerTmpInit; 
  Wire.begin();
  Serial.begin(9600);
  Serial.println("test");
  init_BME280();
  
    
  //OLED
  display.init();
  display.flipScreenVertically();
  display.displayOn();
  display.clear();
}

//
void display_OLED(String sTmp, String sHum, String sPre){

  sTmp="Temp:  "+ sTmp+ " C";
  sHum="Hum :  "+ sHum+ " %";
  sPre="P   :"+ sPre+ " hPa"; 
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString( 0, 10, sTmp );
  display.drawString( 0, 26, sHum );
  display.drawString( 0, 42, sPre );
  
  display.display();
  delay(2000);
  display.clear();

  Serial.print("t11.txt=\"" + sTmp + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("t12.txt=\"" + sHum + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("t13.txt=\"" + sPre + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  
}




//
void loop() {
    delay(100);
    while (BME280.isMeasuring()) {
    }
    String sTmp="";
    String sHum="";
    String sPre="";  
    if (millis() > mTimerDisp) {
        mTimerDisp = millis()+ 3000;    
        // read out the data - must do this before calling the getxxxxx routines
        BME280.readMeasurements();
        printCompensatedMeasurements();
        int itemp  =(int)mTemp;   
        int iHum   = (int)mHumidity;   
        int iPress = (int)mPressure;          
        char cTemp[10+1]; 
        char cHum[10+1]; 
        char cPres[10+1]; 
        sTmp= String(mTemp,2);
        sHum= String(mHumidity,2);
        sPre= String(mPressure,2);
        display_OLED(sTmp, sHum, sPre);
         } // if_timeOver
    if (millis() > mTimerTemp) {
      mTimerTemp = millis()+ mNextHttp+ mTimerTmpInit;
      int itemp  =(int)mTemp;   
      int iHum   = (int)mHumidity;   
      int iPress = (int)mPressure;          
      sTmp=String(itemp,2);
      sHum=String(iHum,2);
      sPre=String(iPress,2);      
     
      delay(100 );
    }          
}

