/*
Original library is from https://github.com/Nikkilae/PPM-reader
Updated by IF 
2022-02-23
- removed unnecessary comparison 
2021-03-05
- disable interrupts when read from volatile variables outside of ISR (https://github.com/Nikkilae/PPM-reader/pull/1)
2018-04-09
- added extra conditions to ISR
2018-01-13
- includes support for Maple Mini board / STM32 (https://github.com/rogerclarkmelbourne/Arduino_STM32/)
- updated Interrupt Service Routine function 
- added a timestamp that indicates when the latest data was received
- channels renumbered to start from 1  
- debug functionality added 
- comments added 
- return a faisafe code in channel[0] (similar to SBUS) if a loss of signal is detected 
  (Walkera returns a 800 us pulses in this case)
TODO - see the .h file 
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

// Set to true to print some debug messages, or false to disable them.
//#define ENABLE_DEBUG_OUTPUT_PPMReader

#include "PPMReader.h"

/* Set PMReader object */
PPMReader::PPMReader(uint8_t channelAmount) {
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::PPMReader started"); 
#endif
	 
    if ( channelAmount > 0) {
        // Setup an array for storing channel values
		// Channels is an array from 0 to ChannelAmount+1 to cover the number  of channels from 1 to Channelamount
        this->channelAmount = channelAmount;
        //change {0..15} to {1.16}
		//rawValues = new uint16_t[channelAmount];
        //validValues = new uint16_t[channelAmount];
        rawValues = new uint16_t[channelAmount + 1];
		
        for (uint8_t i = 0; i <= channelAmount + 1 ; ++i) {
            rawValues[i] = 0;

        }
    }
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::PPMReader completed"); 
#endif
}

/* Delete PMReader object */
PPMReader::~PPMReader() {
    detachInterrupt(interruptPin);
	delete [] rawValues;
	
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::~PPMReader completed"); 
#endif
}

/* Function to setup interrupt */
void PPMReader::setupInterrupt(uint8_t pin, signalPolarity PPMsignalPolarity)
{
  ExtIntTriggerMode mode;
  interruptPin=pin;
  
  //attach interrupt as per the signal polarity
   switch (PPMsignalPolarity) {
    case NORMAL:
        mode = RISING;
    case INVERTED:
        mode = FALLING;
   }
   
  attachInterrupt(pin, myIsrTrampoline, this, mode);
  
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::setupInterrupt completed"); 
#endif
}

/* Interrupt Service Routine */
void PPMReader::ISR() {  
//just to check that ISR is called - it  is a bad practice to debug ISR with serial print!
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::ISR() called"); 
#endif

  // Remember the current micros() and calculate the time since the last pulseReceived()
    uint32_t previousMicros = microsAtLastPulse;
    microsAtLastPulse = micros();
    uint16_t time = microsAtLastPulse - previousMicros;
    
    if (time > blankTime) {
        /* If the time between pulses was long enough to be considered an end
         * of a signal frame, prepare to read channel values from the next pulses */
        pulseCounter = 0;
		failSafe=false;
    }
    else {
            //Proceed only if a captured impulse looks valid - 
            //that prevents from recording too short or too long pulses,  
            //missing or multiple interrupt firing etc. so the rest of 
            //the logic is not disturbed.   
            //Otherwise simply ignore detected impulses that are too short or too long 
            if (time >= minChannelValue && time <= maxChannelValue)  {  

                // Set DataReady flag  to 0 as the data is being acquired AND
                //Store times between pulses as channel values
                if (pulseCounter < channelAmount) {
                    isDataReady=false;
                    dataInputTimeStamp=0;
                    rawValues[pulseCounter+1] = time;  // pulseCounter+1 is for change {0..15} to {1.16}
                    //Check if value are in failsafe  range (Walkera DEVO 12E returns 800 us approx)
                    if (time >= failSafeMinPulseLength && time <= failSafeMaxPulseLength) {
                        failSafe=true;
                    }
                }
				++pulseCounter;
                
				// if all pulses counted then set flag that data is ready 
                if (pulseCounter==channelAmount) {
                    isDataReady=true;
                    dataInputTimeStamp=micros();
                }
		}
	}

	
}

/* Function to return the latest raw (not necessarily valid) value for the  * channel (starting from 0) */
uint16_t PPMReader::rawChannelValue(uint8_t channel) {
    // Check for channel's validity and return the latest raw channel value or 0
    uint16_t value = 0;
    if (channel <= channelAmount) {
        value = rawValues[channel];
    }
    return value;
}
 


/* Function to read the last available raw data into an array. 
Returns a timestamp in microseconds to indicate when the data was received.
Channels is an array from 0 to ChannelAmount+1 to cover the number  of channels from 1 to Channelamount
forseRead is a flag to return the most latest values regardless of whether or not they received completely */
uint32_t PPMReader::readRaw(uint16_t* channels, bool forseRead) {
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::readRaw started"); 
#endif

	if (isDataReady) {
		for (uint8_t i = 1; i <= channelAmount; ++i) { 
			noInterrupts();
			channels[i] = rawValues[i];
			interrupts();
		}
		// Set fail safe value to Channel 0 
		if (failSafe) {
		  channels[0]=codeFailSafe;
		}
		else
		{
		  channels[0]=codeNotFailSafe; 
		}
		
		
	    return this->dataInputTimeStamp;	
	}
    else 
	{
	   if (forseRead) { //still update the channels with the latest raw data available
			for (uint8_t i = 1; i <= channelAmount; ++i) { 
				noInterrupts();
				channels[i] = rawValues[i];
				interrupts();
			}
		
		    // Set fail safe value to Channel 0 
		    if (failSafe) {
		      channels[0]=codeFailSafe;
		    }
		    else
		    {
		       channels[0]=codeNotFailSafe; 
		    }
		}	
	  //but return timestamp as 0 
	  return 0;	
	}      

}

/* Function to read the last available normalised data into an array (integer values)   
Returns a timestamp in microseconds to indicate when the data was received.
Channels is an array from 0 to ChannelAmount+1 to cover the number  of channels from 1 to ChannelAmount
forseRead is a flag to return the most latest values regardless of whether or not a data packet is received completely */
uint32_t PPMReader::readNormalisedInteger(uint16_t* channels, bool forseRead) {
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::readNormalisedInteger started"); 
#endif

	if (isDataReady) {
		for (uint8_t i = 1; i <= channelAmount; ++i) { 
		    //apply multipliers only  
			//channels[i] = (uint16_t) rawValues[i] * multiplierScale + multiplierBias;
			
			//apply multipliers AND constraints 
			noInterrupts();
            channels[i] = (uint16_t) constrain((uint16_t) rawValues[i] * multiplierScale + multiplierBias, minChannelValue, maxChannelValue);
			interrupts();
		}
        
		// Set fail safe value to Channel 0 
		if (failSafe) {
		  channels[0]=codeFailSafe;
		}
		else
		{
		  channels[0]=codeNotFailSafe; 
		}
		//return function value
	    return this->dataInputTimeStamp;
	}
    else 
	{
	   if (forseRead) { //still update the channels with the latest raw data available
			for (uint8_t i = 1; i <= channelAmount; ++i) { 
				//apply multipliers only  
				//channels[i] = (uint16_t) rawValues[i] * multiplierScale + multiplierBias;
			
				//apply multipliers AND constraints 
				noInterrupts();
                channels[i] = (uint16_t) constrain((uint16_t) rawValues[i] * multiplierScale + multiplierBias, minChannelValue, maxChannelValue);
				interrupts();
			}
			// Set fail safe value to Channel 0 
			if (failSafe) {
				channels[0]=codeFailSafe;
			}
			else
			{
				channels[0]=codeNotFailSafe; 
			}

		}	
	  //but return timestamp as 0 
	  return 0;	
	}      

}





/* Function to read the last available normalised data into an array (float values)   
Returns a timestamp in microseconds to indicate when the data was received.
Channels is an array from 0 to ChannelAmount+1 to cover the number  of channels from 1 to ChannelAmount
forseRead is a flag to return the most latest values regardless of whether or not a data packet is received completely */
uint32_t PPMReader::readNormalisedFloat(float* channels, bool forseRead) {
#ifdef ENABLE_DEBUG_OUTPUT_PPMReader
  Serial.println("PPMReader::readNormalisedFloat started"); 
#endif


	if (isDataReady) {
		for (uint8_t i = 1; i <= channelAmount; ++i) { 
		    //apply multipliers only  
			//channels[i] = (float) rawValues[i] * multiplierScale + multiplierBias;
			
			//apply multipliers AND constraints 
			noInterrupts();
			channels[i] = (float) constrain((float) rawValues[i] * multiplierScale + multiplierBias, minChannelValue, maxChannelValue);
            interrupts();   
		}
		
		// Set fail safe value to Channel 0 
		if (failSafe) {
		  channels[0]=codeFailSafe;
		}
		else
		{
		  channels[0]=codeNotFailSafe; 
		}
		//return function value
	    return this->dataInputTimeStamp;	
	}
    else 
	{
	   if (forseRead) { //still update the channels with the latest raw data available
			for (uint8_t i = 1; i <= channelAmount; ++i) { 
				//apply multipliers only  
				//channels[i] = (float) rawValues[i] * multiplierScale + multiplierBias;
			
				//apply multipliers AND constraints 
				noInterrupts();
				channels[i] = (float) constrain((float) rawValues[i] * multiplierScale + multiplierBias, minChannelValue, maxChannelValue);
				interrupts();
			}
			
			// Set fail safe value to Channel 0 
			if (failSafe) {
				channels[0]=codeFailSafe;
			}
			else
			{
				channels[0]=codeNotFailSafe; 
			}
	
		}	
	  //but return timestamp as 0 
	  return 0;	
	}      
}


/* Function to return an indicator that PPM packet received */
bool PPMReader::IsDataReady() {
return this->isDataReady;
}

/* Function to return a timestamp when PPM packet received 
or 0 if the current data packet is being received  */
uint32_t PPMReader::GetDataInputTimeStamp() {
return this->dataInputTimeStamp;
}


 /*A static routine to the ISR function
   http://www.stm32duino.com/viewtopic.php?f=9&t=1364&start=10#p19895
   Working solution for interrupts inside C++ classes */
void PPMReader::myIsrTrampoline(void *arg)
{
  ((PPMReader *)arg)->ISR();
}


