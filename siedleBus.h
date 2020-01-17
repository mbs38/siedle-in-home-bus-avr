#ifndef siedleBus_H
#define siedleBus_H
#endif


/*

	Title: Library implementing Siedle's "in-home-bus" on the avr
	Author: Max Brueggemann
	Hardware: developed for the attiny2313 running at 3.6864MHz
	
	Knowledge about the signal levels, timing and so on have been acquired by reverse engineering.
	Keep that in mind when things don't work right away.
			
	This library uses Timer0 and external Interrupt int1.
	
	

*/
volatile uint8_t dataOut[4];
volatile uint8_t dataIn[4];

extern uint8_t siedleGetFrame(void);
extern uint8_t siedleRecRestart(void);
/*
	Call once.
*/
extern void siedleSetup(void);

/* @brief: Put a frame on the bus.

*/
extern uint8_t siedleSendRaw(uint8_t *data);

/* @brief: returns true if a frame has been received
	
*/
extern uint8_t siedleGetFrame(void);

/*
	These are the pins used to put loads onto the bus.
	Use a suitable npn transistor and to drive the 10Ohm and 200Ohm resistors.
*/
#define PORT200Ohm PORTD
#define PIN200Ohm PORTD5

#define PORT10Ohm PORTD
#define PIN10Ohm PORTD4

/*
	Adapt these definitions to the frequency your avr is running at.
	With the attiny2313 at 3.6864 and a prescaler of 64 we get the compare match after
	about 1.01ms which is 1% off but seems to work anyway (at least with my Siedle system).
	
	oneMilliSec = (F_CPU/64)/1000 = 57.6 => 57
	
*/
#define oneMilliSec 57
#define startTimer TCCR0B = (1<<CS01)|(1<<CS00)
#define maxVal 255
#define gapSize 22

/*
	You may have to adapt these definitions to the controller you are using.
*/
#define extIntVect INT1_vect
#define timerCompIntVect TIMER0_COMPA_vect
#define configureExtInt MCUCR |= (1<<ISC10)|(1<<ISC11)
#define enableExtInt GIMSK |= (1<<INT1)
#define clearExtIntFlag EIFR|=(1<<INTF1)
#define disableExtInt GIMSK &= ~(1<<INT1)
#define enableCompareMatchInt TIMSK |= (1<<OCIE0A)
#define stopTimer TCCR0B = 0
#define resetTimer TCNT0 = 0
#define setCompareMatch2ms OCR0A = 2*oneMilliSec
#define setCompareMatch1ms OCR0A = oneMilliSec
#define setCompareMatchMax OCR0A = maxVal
#define clearWaveformReg TCCR0A=0 //probably not necessary, just want to make sure that the slate is clean.

/*
	
*/
#define load10OhmOn PORT10Ohm|=(1<<PIN10Ohm)
#define load10OhmOff PORT10Ohm&=~(1<<PIN10Ohm)
#define load200OhmOn PORT200Ohm|=(1<<PIN200Ohm)
#define load200OhmOff PORT200Ohm&=~(1<<PIN200Ohm)



