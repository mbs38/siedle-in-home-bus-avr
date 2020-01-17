siedle-in-home-bus-avr
======================

Atmel AVR compatible library implementing an interface to the Siedle "in-home-bus", which Siedle uses in many of their door intercom systems. In main.c you can find a usage example for the lib.

the example file main.c
-----------------------

The example implements a bridge device interfacing the Siedle-Bus to a standard UART interface (19200 Baud, 8n1).

how to use the example
----------------------

Once connected to the bus the bridge will send an info string over the UART every time a transmission (example: a frame containing four bytes of data 123, 123, 123, 123 with 123 being a byte in decimal representation) is happening on the bus (info string will be "RECD:123,123,123,123/r"). You can transmit a frame containing arbitrary data onto the bus by sending a string with the same structure: "SEND:456,456,456,456\r".

example circuit
---------------

Example circuits necessary for the physical interface are included below. Thx to the users of the mikrocontroller.net forum for finding the right load resistor values.

This circuit provides a physical interface to the bus:
![interface circuit](/interface.png)

This library has been tested with an attiny2313, but it can easily be adapted to work on other Attiny and ATmega devices. Using the circuit below, the whole device can be powered by the bus (if you manage to stay below 15mA). Note the comments at the serial interface!
![attiny and supply](/attiny2313example.png)
