/*
Original library is from https://github.com/Nikkilae/PPM-reader
Updated by IF 
2021-03-05
- disable interrupts when read from volatile variables outside of ISR (https://github.com/Nikkilae/PPM-reader/pull/1)
2018-01-13
- includes support for Maple Mini board / STM32 (https://github.com/rogerclarkmelbourne/Arduino_STM32/)
- updated Interrupt Service Routine function 
- added a timestamp that indicates when the latest data was received
- channels renumbered to start from 1  
- debug functionality added 
- comments added 
- return a faisafe code in channel[0] (similar to SBUS) if a loss of signal is detected 
  (Walkera returns a 800 us pulses in this case) 
TODO:  
- Consider to check if value are within the max/min values and with channelValueMaxError tolerance.

 =================================================================
Copyright 2016 Aapo Nikkilä

This file is part of PPM Reader.

PPM Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PPM Reader.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PPMReader_H
#define PPMReader_H

#include <Arduino.h>
//#include <stdint.h> 

//define types
typedef enum signalPolarity {
    NORMAL, /**data pulse is from LOW to HIGH */
    INVERTED /**data pulse is from  HIGH to LOW */
}signalPolarity;


//Define thePPMReader class 
//I can create several instances of PPMReader to handle various pins: 
//PPMReader thing1;
//PPMReader thing2;
//And I can set the pins in the Arduino setup() routine: 
// thing1.setup(PB3);
// thing2.setup(PB4);
class PPMReader {

    public:
    
	//The range of a channel's min/max possible values, microseconds
	//default values are for +/-150% plus 100 us and minus 200us for contingency and for fail safe values
    uint16_t minChannelValue = 700;
    uint16_t maxChannelValue = 2200;

	//TODO not currently used. Consider to check if value are within the max/min values and with channelValueMaxError tolerance.
    //The maximum error to max/min values (in either direction) in channel value
    //with which the channel value is still considered valid */
    //uint16_t channelValueMaxError = 10;

    //The minimum value (time) after which the signal frame is considered to
    //be finished and we can start to expect a new signal frame.
	//Minimal blank time for 8 channels is:
	// PPM signal period - (150% max servo duration * 8 + 400us trailing pulse) 
	// 22000 us - (2100*8 + 400) = 4800us  */   
    uint16_t blankTime = 5000;

	//Calibration multipliers to apply to raw channel data values before 
	//it is returned as a normalised data (rawValues[i] * multiplierScale + multiplierBias;)
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
    float multiplierScale = 1.0f;
  	float multiplierBias = 0.0f;
	
	//Codes to return in case of failsafe condition is detected
	//Set the same as SBUS codes of Walkera DEVO 12E
	//Apparently Walkera returns 3 when the receiver is binded and signal is ok,  otherwise 0 is returned for failsafe 	
	uint16_t codeFailSafe=0;
    uint16_t codeNotFailSafe=3;
	//Min and Max pulse length in microseconds to detect a failsafe condition
    //Apparently Walkera returns an approx 800 us pulse on all channels when the receiver is binded but signal is lost
	uint16_t failSafeMinPulseLength = 770;
	uint16_t failSafeMaxPulseLength = 830; 
	
	
    private:

    //The pin from which to listen for interrupts
    uint8_t interruptPin = 0;

    //The amount of channels to be expected from the PPM signal.
    uint8_t channelAmount = 0;

    //Arrays for keeping track of channel values
    volatile uint16_t *rawValues; //= NULL;  //captured values 
    
	//A counter variable for determining which channel is being read next
    volatile uint8_t pulseCounter = 0;

    // A time variable to remember when the last pulse was read
    volatile uint32_t microsAtLastPulse = 0;

   // A static routine to the ISR function
   //http://www.stm32duino.com/viewtopic.php?f=9&t=1364&start=10#p19895
   //Working solution for interrupts inside C++ classes
	static void myIsrTrampoline(void *arg);
	
	//Indicates that PPM packet received and says when (in microseconds)
	volatile uint8_t isDataReady = 0;
	volatile uint32_t dataInputTimeStamp = 0;
	
	//Indicates that PPM packet contains data that can be recognised as a fail safe mode 
	volatile bool failSafe = false;

    public:

	//Set PPMReader object
	PPMReader(uint8_t channelAmount);
	
	//Delete PPMReader object
    ~PPMReader();
	
	//Set up interrupt 
  	void setupInterrupt(uint8_t interruptPin, signalPolarity PPMsignalPolarity = NORMAL); 
   	
	//Interrupt Service Routine function 
	void ISR();

    //Returns the latest raw (not necessarily valid) value for a channel
    //(starting from 0, Ch0 is a failsafe value, Ch1,2,etc. are the channels values). 
    uint16_t rawChannelValue(uint8_t channel);

 //   //Returns the latest received value that was considered valid for the channel (starting from 0).
 //   //Returns defaultValue if the given channel hasn't received any valid values yet. */
 //   uint16_t latestValidChannelValue(uint8_t channel, uint16_t defaultValue);
		
	//Returns status of current data packet 
	bool IsDataReady();
	
	
	//Returns time in microseconds when the last data packet was received 
	//or 0 if the current data packet is being received  
	uint32_t GetDataInputTimeStamp();
	
    
	//Functions to read the last available  data into an array. 
	//Returns a timestamp in microseconds to indicate when the data was received.
	//channels is an array from 0 to ChannelAmount+1 to cover the number  of channels from 1 to ChannelAmount  
	//forseRead is a flag to return the most latest values regardless of whether or not a data packet is received completely
    uint32_t readRaw(uint16_t* channels, bool forseRead = false);  //raw data
	uint32_t readNormalisedInteger(uint16_t* channels, bool forseRead = false);  //normalised data of Integer type
	uint32_t readNormalisedFloat(float* channels, bool forseRead = false);  //normalised data of Float type


	
};

#endif
