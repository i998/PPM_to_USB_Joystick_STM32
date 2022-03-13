/*
RC Signal Median filter 

The median filter shall reduce effect of potential jitter/outlier values for RC channels

Original idea: https://github.com/iNavFlight/inav/blob/44c494af43b90d8a8fbce7afaad5a3334687d2f4/src/main/common/maths.c#L307
               https://github.com/iNavFlight/inav/blob/master/src/main/rx/rx.c
			   
    // Assuming a step transition from 1000 -> 2000 different filters will yield the following output:
    //  No filter:              1000, 2000, 2000, 2000, 2000        - 0 samples delay
    //  3-point moving average: 1000, 1333, 1667, 2000, 2000        - 2 samples delay
    //  3-point median:         1000, 1000, 2000, 2000, 2000        - 1 sample delay
    //  5-point median:         1000, 1000, 1000, 2000, 2000        - 2 sample delay

    // For the same filters - noise rejection capabilities (2 out of 5 outliers
    //  No filter:              1000, 2000, 1000, 2000, 1000, 1000, 1000
    //  3-point MA:             1000, 1333, 1333, 1667, 1333, 1333, 1000    - noise has reduced magnitude, but spread over more samples
    //  3-point median:         1000, 1000, 1000, 2000, 1000, 1000, 1000    - high density noise is not removed
    //  5-point median:         1000, 1000, 1000, 1000, 1000, 1000, 1000    - only 3 out of 5 outlier noise will get through

    // Apply 5-point median filtering. This filter has the same delay as 3-point moving average, but better noise rejection			   
			   
			   
			   

TODO - see the .h file 
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
  
 // Set to true to print some debug messages, or false to disable them.
//#define ENABLE_DEBUG_OUTPUT_FILTER 

#include "Arduino.h"
#include "MedianFilter.h"
 
   
  // Set MedianFilter object 
MedianFilter::MedianFilter() {

	
	//Fill up arrays for historical values with the default value 	
	for (uint8_t i=1; i<=channelAmountIn; i++) {
		_queuePosition1[i]=DefaultInputValue;	
		_queuePosition2[i]=DefaultInputValue;
		_queuePosition3[i]=DefaultInputValue;		
		_queuePosition4[i]=DefaultInputValue;
		_queuePosition5[i]=DefaultInputValue;
		
    }	
		
	//Set the pointer 	
	_queuePointer = 1;	
}


// Delete MedianFilter object 
MedianFilter::~MedianFilter() {
    //delete [] chIN;
    //delete [] chOUT;
}
  
   
 //This function updates applies 5-point median filter.  
// parameter chIN[] - an array of input values from receiver, pulse length in us 
// parameter chOUT[] - an array of output to servo driver, pulse length in us
// function output - chOUT[] array updated  
void MedianFilter::ApplyFilter(uint16_t chIN[0], uint16_t chOUT[0]){ 
#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
  Serial.println("MedianFilter::ApplyFilter started"); 
#endif
 
    //===Print debug values======================================================
#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
        // Print values from all input channels
        for (int i = 1; i <= channelAmountIn; ++i) {
        Serial.print("Ch"+String(i)+":"+String(chIN[i]) + " ");
        }
        Serial.println();
#endif
 
 _timestamp=micros();

//=======CALCULATIONS STARTED========================================================= 
 
		   //Set the array for historical values per channel  
           uint16_t v[5];
 
 // Quick and dirty compilation of historical values for a channel from arrays of historical values based on the queue pointer
 // for 5 point filter only
#ifdef ENABLE_DEBUG_OUTPUT_FILTER  
        Serial.println("queuePointer=" + _queuePointer);
		Serial.println(AA);
		Serial.println(_queuePointer);
 #endif
 
 switch(_queuePointer)
    {
        case 1:
		   //Put the input values into the appropriate queue array
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
		  _queuePosition1[i]=chIN[i];	
          }

		  
		  //For each channel 
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
	          
		     //Set the array for historical values, oldest value has array index as 0
             v[4]=_queuePosition1[i]; //the latest data
		     v[3]=_queuePosition5[i];
		     v[2]=_queuePosition4[i];
			 v[1]=_queuePosition3[i];
			 v[0]=_queuePosition2[i]; //the oldest data
			 
			 //Apply median filter and put the value into output array 
			 
			 	#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
				  Serial.println("quickMedianFilter5_16 about to started"); 
						  // Print values from all input channels
						for (int i = 0; i <= 4; ++i) {
						Serial.print("v"+ String(i)+":"+String(v[i]) + " ");
						}
						Serial.println();
				#endif
			 chOUT[i] = quickMedianFilter5_16(v);
		  }
            break;

        case 2:
		   //Put the input values into the appropriate queue array
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
		  _queuePosition2[i]=chIN[i];	
          }

		  
		  //For each channel 
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
	          
		     //Set the array for historical values
             v[4]=_queuePosition2[i]; //the latest data
		     v[3]=_queuePosition1[i];
		     v[2]=_queuePosition5[i];
			 v[1]=_queuePosition4[i];
			 v[0]=_queuePosition3[i]; //the oldest data
			 
			 //Apply median filter and put the value into output array 
			 chOUT[i] = quickMedianFilter5_16(v);
		  }
            break;

        case 3:
		   //Put the input values into the appropriate queue array
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
		  _queuePosition3[i]=chIN[i];	
          }

		  
		  //For each channel 
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
	          
		     //Set the array for historical values
             v[4]=_queuePosition3[i]; //the latest data
		     v[3]=_queuePosition2[i];
		     v[2]=_queuePosition1[i];
			 v[1]=_queuePosition5[i];
			 v[0]=_queuePosition4[i]; //the oldest data
			 
			 //Apply median filter and put the value into output array 
			 chOUT[i] = quickMedianFilter5_16(v);
		  }
            break;

        case 4:
		   //Put the input values into the appropriate queue array
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
		  _queuePosition4[i]=chIN[i];	
          }

		  
		  //For each channel 
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
	          
		     //Set the array for historical values
             v[4]=_queuePosition4[i]; //the latest data
		     v[3]=_queuePosition3[i];
		     v[2]=_queuePosition2[i];
			 v[1]=_queuePosition1[i];
			 v[0]=_queuePosition5[i]; //the oldest data
			 
			 //Apply median filter and put the value into output array 
			 chOUT[i] = quickMedianFilter5_16(v);
		  }
            break;
			
        case 5:
		   //Put the input values into the appropriate queue array
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
		  _queuePosition5[i]=chIN[i];	
          }

		  
		  //For each channel 
		  for (uint8_t i=1; i<=channelAmountIn; i++) {
	          
		     //Set the array for historical values
             v[4]=_queuePosition5[i]; //the latest data
		     v[3]=_queuePosition4[i];
		     v[2]=_queuePosition3[i];
			 v[1]=_queuePosition2[i];
			 v[0]=_queuePosition1[i]; //the oldest data
			 
			 //Apply median filter and put the value into output array 
			 chOUT[i] = quickMedianFilter5_16(v);
		  }
            break;
	   #ifdef ENABLE_DEBUG_OUTPUT_FILTER
        default:
	
          Serial.println("FUCK" ) ;
		#endif  
    }


 

	
  //set queue pointer to the next array so new input values can be recorded there 
  _queuePointer=_queuePointer +1;
  if(_queuePointer > 5) {_queuePointer=1;}
  
  
 // Serial.println("queuePointer2=" + _queuePointer);
  
  
 //=======CALCULATIONS COMPLETED=====================================================
 
  CalculationTime = micros() - _timestamp;
  
#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
  Serial.println("MedianFilter::ApplyFilter completed"); 
  Serial.print("Calc time, us: " + CalculationTime);
#endif
    //===Print debug values======================================================
#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
        // Print values from all input channels
        for (int i = 1; i <= channelAmountOut; ++i) {
        Serial.print("Ch"+String(i)+":"+String(chOUT[i]) + " ");
        }
        Serial.println();
#endif
} 





//This function passes the input  without changes
// parameter chIN[] - an array of input values 
// parameter chOUT[] - an array of output values
// function output - chOUT[] array updated 
void MedianFilter::Passthrough(uint16_t chIN[0], uint16_t chOUT[0]){ 
#ifdef ENABLE_DEBUG_OUTPUT_FILTER
  Serial.println("MedianFilter::Passthrough started"); 
#endif
  for (uint8_t i=1; i<=channelAmountOut; i++) {
    if (i<=channelAmountIn) { 
       chOUT[i]=chIN[i];
	}
    else
    {
     chOUT[i]=DefaultInputValue;
    }	
  }

#ifdef ENABLE_DEBUG_OUTPUT_FILTER 
  Serial.println("MedianFilter::Passthrough completed"); 
#endif
} 


//These functions are median filters 
// parameter * v - an array of input values,  assume oldest value has array index as 0 
// function output - calculated median value 

// Quick median filter implementation
// (c) N. Devillard - 1998
// http://ndevilla.free.fr/median/median.pdf
#define QMF_SORT(type,a,b) { if ((a)>(b)) QMF_SWAP(type, (a),(b)); }
#define QMF_SWAP(type,a,b) { type temp=(a);(a)=(b);(b)=temp; }
#define QMF_test(type,a,b) { if ((a)=(b)) a=a+10; }
uint32_t MedianFilter::quickMedianFilter3_32(uint32_t * v)
{
    uint32_t p[3];
    memcpy(p, v, sizeof(p));

    QMF_SORT(uint32_t, p[0], p[1]); QMF_SORT(uint32_t, p[1], p[2]); QMF_SORT(uint32_t, p[0], p[1]) ;
    return p[1];
}

uint16_t MedianFilter::quickMedianFilter3_16(uint16_t * v)
{
    uint16_t p[3];
    memcpy(p, v, sizeof(p));

    QMF_SORT(uint16_t, p[0], p[1]); QMF_SORT(uint16_t, p[1], p[2]); QMF_SORT(uint16_t, p[0], p[1]) ;
    return p[1];
}

uint32_t MedianFilter::quickMedianFilter5_32(uint32_t * v)
{
    uint32_t p[5];
    memcpy(p, v, sizeof(p));

    QMF_SORT(uint32_t, p[0], p[1]); QMF_SORT(uint32_t, p[3], p[4]); QMF_SORT(uint32_t, p[0], p[3]);
    QMF_SORT(uint32_t, p[1], p[4]); QMF_SORT(uint32_t, p[1], p[2]); QMF_SORT(uint32_t, p[2], p[3]);
    QMF_SORT(uint32_t, p[1], p[2]);
    return p[2];
}

uint16_t MedianFilter::quickMedianFilter5_16(uint16_t * v)
{

	#ifdef ENABLE_DEBUG_OUTPUT_FILTER1 
      Serial.println("quickMedianFilter5_16 started"); 
          // Print values from all input channels
        for (int i = 0; i <= 4; ++i) {
        Serial.print("v"+ String(i)+":"+String(v[i]) + " ");
        }
        Serial.println();
    #endif

    uint16_t p[5];
    memcpy(p, v, sizeof(p));

		#ifdef ENABLE_DEBUG_OUTPUT_FILTER1 
      Serial.println("quickMedianFilter5_16 started"); 
          // Print values from all input channels
        for (int i = 0; i <= 4; ++i) {
        Serial.print("p"+ String(i)+":"+String(p[i]) + " ");
        }
        Serial.println();
    #endif
	//p[0]=p[0]+10;
	//QMF_test(uint16_t, p[0], p[0])
    QMF_SORT(uint16_t, p[0], p[1]); QMF_SORT(uint16_t, p[3], p[4]); QMF_SORT(uint16_t, p[0], p[3]);
    QMF_SORT(uint16_t, p[1], p[4]); QMF_SORT(uint16_t, p[1], p[2]); QMF_SORT(uint16_t, p[2], p[3]);
    QMF_SORT(uint16_t, p[1], p[2]);
	
		#ifdef ENABLE_DEBUG_OUTPUT_FILTER1 
        Serial.println("p[0]" + p[0]); 
        #endif
	
    return p[2];
	//return p[0];
}

uint32_t MedianFilter::quickMedianFilter7_32(uint32_t * v)
{
    uint32_t p[7];
    memcpy(p, v, sizeof(p));

    QMF_SORT(uint32_t, p[0], p[5]); QMF_SORT(uint32_t, p[0], p[3]); QMF_SORT(uint32_t, p[1], p[6]);
    QMF_SORT(uint32_t, p[2], p[4]); QMF_SORT(uint32_t, p[0], p[1]); QMF_SORT(uint32_t, p[3], p[5]);
    QMF_SORT(uint32_t, p[2], p[6]); QMF_SORT(uint32_t, p[2], p[3]); QMF_SORT(uint32_t, p[3], p[6]);
    QMF_SORT(uint32_t, p[4], p[5]); QMF_SORT(uint32_t, p[1], p[4]); QMF_SORT(uint32_t, p[1], p[3]);
    QMF_SORT(uint32_t, p[3], p[4]);
    return p[3];
}

uint32_t MedianFilter::quickMedianFilter9_32(uint32_t * v)
{
    uint32_t p[9];
    memcpy(p, v, sizeof(p));

    QMF_SORT(uint32_t, p[1], p[2]); QMF_SORT(uint32_t, p[4], p[5]); QMF_SORT(uint32_t, p[7], p[8]);
    QMF_SORT(uint32_t, p[0], p[1]); QMF_SORT(uint32_t, p[3], p[4]); QMF_SORT(uint32_t, p[6], p[7]);
    QMF_SORT(uint32_t, p[1], p[2]); QMF_SORT(uint32_t, p[4], p[5]); QMF_SORT(uint32_t, p[7], p[8]);
    QMF_SORT(uint32_t, p[0], p[3]); QMF_SORT(uint32_t, p[5], p[8]); QMF_SORT(uint32_t, p[4], p[7]);
    QMF_SORT(uint32_t, p[3], p[6]); QMF_SORT(uint32_t, p[1], p[4]); QMF_SORT(uint32_t, p[2], p[5]);
    QMF_SORT(uint32_t, p[4], p[7]); QMF_SORT(uint32_t, p[4], p[2]); QMF_SORT(uint32_t, p[6], p[4]);
    QMF_SORT(uint32_t, p[4], p[2]);
    return p[4];

}
