/*
RC Signal Median filter 

The median filter shall reduce effect of potential jitter/outlier values for RC channels. 
5-point median filtering is used. 

Original idea: https://github.com/iNavFlight/inav/blob/44c494af43b90d8a8fbce7afaad5a3334687d2f4/src/main/common/maths.c#L307
               https://github.com/iNavFlight/inav/blob/master/src/main/rx/rx.c
			   
TODO:

=================================================================
(C) 2021,2018 ifh  
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

#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include "Arduino.h"


class MedianFilter {
	public:
		//Set MedianFilter object
		MedianFilter();
	
		//Delete MedianFilter object
		~MedianFilter();
	
		//The code is written with an assumption that  
		//channelAmountIn <= 16 and channelAmountOut <= 16. 
		//The amount of channels to be expected from the input signal.
		uint8_t channelAmountIn = 16;
	
		//The amount of channels to be expected for the output signal.
		uint8_t channelAmountOut = 16;
	
		//This function applies median filter 
		// parameter chIN[] - an array of input values from receiver, pulse length in us 
		// parameter chOUT[] - an array of output values with the median filter applied, pulse length in us
		// function output - chOUT[] array updated  		 
		void ApplyFilter (uint16_t chIN[0], uint16_t chOUT[0]);
	
	
        //This function passes the input to servos without changes
        // parameter chIN[] - an array of input values from receiver, pulse length in us 
        // parameter chOUT[] - an array of output to servo driver, pulse length in us
        // function output - chOUT[] array updated 
       void Passthrough(uint16_t chIN[0], uint16_t chOUT[0]);



	
		//CalcTime, micros
		uint32_t CalculationTime = 0;
		
		//Default value for input signal 
		uint16_t  DefaultInputValue = 1000; //999; 
		

	private:
		uint32_t _timestamp = 0;


		// A set of arrays to hold the historical values for RC channels. 
	 	int _queuePointer = 1; 	
		int AA = 1;
		
		uint16_t  _queuePosition1[17];	
		uint16_t  _queuePosition2[17];	
		uint16_t  _queuePosition3[17];
		uint16_t  _queuePosition4[17];
		uint16_t  _queuePosition5[17];
	 
	 
		//These functions are median filters 
		// parameter * v - an array of input values,  assume the oldest value has array index as 0 
		// function output - calculated median value 
		uint32_t quickMedianFilter3_32(uint32_t * v);
		uint16_t quickMedianFilter3_16(uint16_t * v);
		uint32_t quickMedianFilter5_32(uint32_t * v);
		uint16_t quickMedianFilter5_16(uint16_t * v);
		uint32_t quickMedianFilter7_32(uint32_t * v);
		uint32_t quickMedianFilter9_32(uint32_t * v);
		
	};

#endif