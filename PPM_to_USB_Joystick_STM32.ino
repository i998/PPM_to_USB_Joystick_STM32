/*
v03 - PPM to USB Joystick

Mapping:
   Joystick.X            <->      (1)Aileron 
   Joystick.Y            <->      (2)Eelev
   Joystick.Xrotate      <->      (4)Rudder
   Joystick.Yrotate      <->      (5)Gear
   Joystick.sliderLeft   <->      (6)Ch6 (flaps)
   Joystick.sliderRight  <->      (3)Throttle  
   Joystick.button       <->      (7)Ch7
   Joystick.button       <->      (8)Ch8 

 
Original idea:  https://github.com/voroshkov/Leonardo-USB-RC-Adapter 
USB HID library: https://github.com/arpruss/USBHID_stm32f1
PPM Reader Library: https://github.com/i998/FlyByWire
  
Status:  Works OK

Change list:
v0.3:
- added Median filter which shall reduce effect of potential jitter/outlier values for RC channels. 5-point median filtering is used  
- minor bugfixes
v0.2  
- updated PPM to USB Joystick to adapt to the changes in the USB HID library  
- updated PPMReader  - interrupts disabled when reading from volatile variables outside of ISR (https://github.com/Nikkilae/PPM-reader/pull/1)
- modified to use Maple Mini and a PPM reader library   

Notes:
- Compiled with Fastest (-O3) settings 
- With minimal modifications can be changed to SBUS to USB Joystick

TODO:  
- detect or read from hardware a number of input PPM channels   

=================================================================
(C)2022,2021,2018 ifh  
This file is part of PPM to USB Joystick.

PPM to USB Joystick is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM to USB Joystick is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

*/

//use global copies of the libraries 
#include <USBComposite.h>

//use local copies of the libraries 
#include "src\PPMReader.h"
#include "src\MedianFilter.h"



//Uncomment to print some debug messages. 
//Note that in the debug mode the Maple Mini's USB will be configured in a serial mode 
//so the USB Joystick wil not be available - the PC will detect a serial port instead.
//#define ENABLE_DEBUG_LOOP_IN


//====Constants and global Variables==========================
//the number of the LED pin
const uint8_t ledPin =  LED_BUILTIN;

//RC input channel parameters, microseconds  
uint16_t minNormalChannelValue = 1100;
uint16_t maxNormalChannelValue = 1900;
uint16_t channelMidPoint = 1500;

//USB Joystick channel parameters 
uint16_t minJoystickChannelValue = 0;
uint16_t maxJoystickChannelValue = 1023;

//timestamp variables
uint16_t timestampOld =0;
uint16_t timestampNew =0;

uint16_t timestampDataSentToUsb =0;
uint16_t minDelayToSendToUsb = 1; //miliseconds  


//=================Set Up PPM receiver ======================
//set a pin number for PPM input 
int PPMinputPin=2;

// Initialize a PPMReader on digital pin 3 with 8 expected channels. 
//Note interrupt will be attached separately in Setup()
int channelAmountIn = 8;
PPMReader ppm(channelAmountIn);

    //number of values to cover amount of channels, indexed {1..channelAmount}
    uint16_t channelsIN[9];  // for PPM, 8 channels, 1..8, 9 values indexed {0..8}
    //float channelsIN[9];  // for PPM, 8 channels, 1..8,9 values indexed {0..8}

     uint16_t channelsIN_MF[9];  // for Median Filter  - contains input channels after filter is applied 
	
//========Set Up Median Filter =====================
MedianFilter Filter;

	
//=================Set Up Joystick ======================

USBHID HID;
HIDJoystick Joystick(HID);


//=================SETUP()===================================
void setup() {
  
   // set the digital pin as output:
  pinMode(ledPin, OUTPUT);

//==============================
#ifdef ENABLE_DEBUG_LOOP_IN
       Serial.begin(9600);
    //The program will wait for serial to be ready up to 10 sec then it will contunue anyway  
    for (int i=1; i<=10; i++){
        delay(1000);
        if (Serial){
          break;
        }
    }
Serial.println("Setup() started ");
#endif
//===============================


//=====setup Median Filter ===============
  Filter.channelAmountIn = channelAmountIn;
  Filter.channelAmountOut = channelAmountIn;  //use same number of channels for both input and output
  Serial.println("Median Filter setup completed");




//begin the PPM communication
  //=======PPM setup=========
  //set the PPMinputPin as input,  pulled up for inverted polarity , pulled down for normal  
  pinMode(PPMinputPin, INPUT_PULLUP); 
  //attach interrupt  to the input pin.  The function is in the PPMReader class and it sets the interrupt pin and signal polarity  
  ppm.setupInterrupt(PPMinputPin, INVERTED);
  
  // correct the default values  
  // The range of a channel's possible values, microseconds
    ppm.minChannelValue = 700;
    ppm.maxChannelValue = 2200;

 // The minimum value (time) after which the signal frame is considered to
 // be finished and we can start to expect a new signal frame.
	//Minimal blank time for 8 channels is:
	// PPM signal period - (150% max servo duration * 8 + 400us trailing pulse) 
	// 22000 us - (2100*8 + 400) = 4800us  */   
    ppm.blankTime = 5000;

  //Calibration multipliers to apply to raw channel data values before 
  //they are returned as a normalised data (rawValues[i] * multiplierScale + multiplierBias;)
    //Walkera DEVO 12E values:
    //to uS (1100..1900):
	//  Scale=1.0f
	//  Bias=-9.0f 
	//to percentage (-100..+100):
	//  Scale=0.250f
	//  Bias=-377.250f 
	//to float (-1..+1):
	//  Scale=0.00250f
	//  Bias=-3.77250f
    ppm.multiplierScale = 1.0f;
    ppm.multiplierBias = 0.0f;
//====================================


//=====Set Up Joystick ===============
HID.begin(HID_JOYSTICK);

/* joystick reference:
X
Y
Xrotate
Yrotate
sliderLeft
sliderRight
hat(direction in dergees)
button(number,value) 
*/

}
//=====END OF SETUP ()=================================================


//==================LOOP()==============================================
void loop() {

//acquire the data into a local array
timestampNew = ppm.readNormalisedInteger(&channelsIN[0]);

if(timestampNew!=0 && timestampNew!=timestampOld){ //data is ready and it is a new data 
  //update timestamp
    timestampOld = timestampNew;

    
   //optional - blinking /serial debug
        // set the LED with the ledState of the variable:
        //http://wiki.stm32duino.com/index.php?title=API
        //This is a function you can call, that doesn't take the pinnumber
        //but the GPIOPort, and which pin of the port you want to access.
        // It's a lot faster than the digitalWrite function
        //example: gpio_write_bit(GPIOB, 0, LOW); 
        // builtin led:  gpio_write_bit(GPIOB,1,ledState);
        gpio_write_bit(GPIOB,1,HIGH); 


  //=======Debug Input values==============================================
  #ifdef ENABLE_DEBUG_LOOP_IN
        // Print latest valid values from all channels
        for (int i = 1; i <= channelAmountIn; ++i) {
        Serial.print("Ch"+String(i)+":"+String(channelsIN[i]) + " ");
        }
        Serial.print(" Ch0:");Serial.print(channelsIN[0]);  //"Byte 23 of SBUS protocol or PPM failsafe value"        
        Serial.println();
  #endif
  //==========================================================================

  //Apply Median Filter   
    Filter.ApplyFilter(channelsIN, channelsIN_MF);
    //Filter.Passthrough(channelsIN, channelsIN_MF);
  
    
  // Convert PPM values to USB joystick values and send them to USB 
  if (millis()- timestampDataSentToUsb >= minDelayToSendToUsb) { //delay if needed
    timestampDataSentToUsb  = millis(); 
  
   Joystick.X(PpmToJoystickValue(channelsIN_MF[1]));            //      (1)Aileron 
   Joystick.Y(PpmToJoystickValue(channelsIN_MF[2]));            //      (2)Eelev
   Joystick.Xrotate(PpmToJoystickValue(channelsIN_MF[4]));      //      (4)Rudder
   Joystick.Yrotate(PpmToJoystickValue(channelsIN_MF[5]));      //      (5)Gear
   Joystick.sliderLeft(PpmToJoystickValue(channelsIN_MF[6]));   //      (6)Ch6 (flaps)
   Joystick.sliderRight(PpmToJoystickValue(channelsIN_MF[3]));  //      (3)Throttle  
   Joystick.button(1,(channelsIN_MF[7] > channelMidPoint));      //     (7)Ch7
   Joystick.button(2,(channelsIN_MF[8] > channelMidPoint));      //     (8)Ch8 
   //Joystick.hat(0);
   
   Joystick.send();
  }



} 
else
{ 
  // data not ready yet, do something else in this loop
       
        if (timestampNew==0){ //data is being received 
         // do something
        }
        if (timestampNew==timestampOld){ // looping too fast, the same old data is available 
        // do something
   
        }

        
       //optional - blinking /serial debug     
       gpio_write_bit(GPIOB,1,LOW);
      
   }




 
}
//============ END OF LOOP() =============================================







//=========== Helper Functions =========================================== 

//This function converts an input value 
//in the range of [minNormalChannelValue - maxNormalChannelValue] 
//to the range of [minJoystickChannelValue - maxJoystickChannelValue] 
//  reference: map(value, fromLow, fromHigh, toLow, toHigh)
uint16_t PpmToJoystickValue(uint16_t ppmValue) { 
  return constrain( 
                    map(ppmValue, minNormalChannelValue, maxNormalChannelValue, minJoystickChannelValue, maxJoystickChannelValue), 
                    minJoystickChannelValue, 
                    maxJoystickChannelValue 
                  ); 
 } 


