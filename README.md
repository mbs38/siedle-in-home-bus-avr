# siedle-in-home-bus-avr
# The code is work in progress!
Atmel AVR compatible library implementing an interface to the Siedle "in-home-bus", which Siedle uses in many of their door intercom systems. Example circuits necessary for the physical interface included. Thx to the users of the mikrocontroller.net forum for finding the right load resistor values.

This circuit provides a physical interface to the bus:
![interface circuit](/interface.png)

This library has been tested with an attiny2313, but it can easily be adapted to work on other Attiny and ATmega devices. Using the circuit below, the whole device can be powered by the bus. Note the comments at the serial interface!
![attiny and supply](/attiny2313example.png)
