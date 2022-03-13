# PPM_to_USB_Joystick_STM32

An adapter to  convert PPM RC signal to a Joystick - so it can be recognised by simulators (FMS, RCPhoenix etc.) A median filter shall reduce effect of potential jitter/outlier values for RC channels. 5-point median filtering is used.

Based on the following: 
 
  - Maple Mini CPU module (https://stm32duinoforum.com/forum/wiki_subdomain/index_title_Maple_Mini.html) 
  - Arduino for STM32 (https://github.com/rogerclarkmelbourne/Arduino_STM32)
  - PPM Reader library for STM32 from FlyByWire (https://github.com/i998/FlyByWire)
  - USB Composite library (https://github.com/arpruss/USBComposite_stm32f1)
   
   
## Connection:   
Connect a PPM signal to pin 2 (PB2). 

Note - input signal is 5v max. Or use a resistor and a diode as a signal converter to 3.3v as described in the documentation. 

## Signal Mapping:

   Joystick.X            <->      (1)Aileron
   
   Joystick.Y            <->      (2)Eelev
   
   Joystick.Xrotate      <->      (4)Rudder
   
   Joystick.Yrotate      <->      (5)Gear
   
   Joystick.sliderLeft   <->      (6)Ch6 (flaps)
   
   Joystick.sliderRight  <->      (3)Throttle  
   
   Joystick.button       <->      (7)Ch7
   
   Joystick.button       <->      (8)Ch8 
  
   
## License:
PPM to USB Joystick is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM to USB Joystick is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.  

Libraries that included as a part of this software package may have
different licenses,  please check the libraries source code and notes. 

## Donation:
If you like this project or it helps you to reduce your development effort, you can buy me a cup of coffee :) 

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?business=ifhone777-hub%40yahoo.com&currency_code=USD)