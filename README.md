# PPM_to_USB_Joystick_STM32

An adapter to  convert PPM RC signal to a Joystick - so it can be recognised by simulators (FMS, RCPhoenix etc.)

Based on Maple Mini CPU module (http://wiki.stm32duino.com/index.php?title=Maple_Mini) 

Connect a PPM signal to pin 2 (PB2). Note- 5v max. 

## Mapping:

   Joystick.X				(1)Aileron
   
   Joystick.Y				(2)Eelevator

   Joystick.Xrotate			(4)Rudder

   Joystick.Yrotate         (6)Ch6

   Joystick.sliderLeft      (7)Ch7

   Joystick.sliderRight     (3)Throttle 

   Joystick.button1         (5)Gear 

   Joystick.button2         (8)Ch8
   