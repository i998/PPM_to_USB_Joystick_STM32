# PPM_to_USB_Joystick_STM32

An adapter to  convert PPM RC signal to a Joystick - so it can be recognised by simulators (FMS, RCPhoenix etc.)

Based on Maple Mini CPU module (http://wiki.stm32duino.com/index.php?title=Maple_Mini) 

Connect a PPM signal to pin 2 (PB2). 

Note - input signal is 5v max. Or use a resistor and a diode as
a signal converter to 3.3v as described in the documentation. 

## Mapping:

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