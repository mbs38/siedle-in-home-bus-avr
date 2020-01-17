#include <avr/io.h>
#include "siedleBus.h"
#include <avr/interrupt.h>

volatile uint8_t dataOut[] = {0,0,0,0};
volatile uint8_t dataIn[] = {0,0,0,0};
volatile uint8_t bitCount = 0;

volatile uint8_t internalState = 0;
#define RECEIVE 1 //currently receiving, unable to transmit
#define SEND 2 //currently transmitting, unable to start another transmission
#define GAP_WAIT 3 //transmission or receiving finished
#define IDLE 4 //waiting for incoming transmission, able to transmit
#define RECEIVE_FIN 5 //frame has been received, able to transmit
#define WAIT_TM_FIN 6 //transmission has finished, no frame received yet

volatile uint8_t siedleSendState = 0;
#define SEND_FIN 2

volatile uint8_t siedleRecState = 0;
#define REC_FIN 2

void purgeTimer(void) {
	stopTimer;
	resetTimer;
	clearWaveformReg;
	enableCompareMatchInt;
}

inline void mblistSetBit(volatile uint8_t *list, uint8_t nr) {
	unsigned char ArC=0;
	ArC=nr/8;
	*(list+ArC)|=(1<<(nr-(ArC*8)));
}

inline unsigned char mblistReadBit(volatile uint8_t *list, uint8_t nr) {
	unsigned char ArC=0;
	ArC=nr/8;
	if (*(list+ArC)&(1<<(nr-(ArC*8))))
	{
		return 1;
	} else return 0;
}

inline void mblistClearBit(volatile uint8_t *list, uint8_t nr) {
	unsigned char ArC=0;
	ArC=nr/8;
	*(list+ArC)&=~(1<<(nr-(ArC*8)));
}

uint8_t siedleGetFrame(void) {
	if (siedleRecState==REC_FIN)
	{
		return 1;
	}
	return 0;
}

uint8_t siedleRecRestart(void) {
	siedleRecState=0;
	bitCount=0;
	internalState=IDLE;
	clearExtIntFlag;
	enableExtInt;
	return 0;
}

/* @brief: Call this once before using any of the other functions.
	
*/
void siedleSetup(void) {
	bitCount=0;
	/* Prepare external Interrupt */
	configureExtInt;
	/* Prepare timer */
	purgeTimer();
	/* Wait about 100ms */
	internalState=GAP_WAIT;
	setCompareMatchMax;
	startTimer;
}

/* @brief: sends a frame supplied in the argument.
		   Returns 0 if the bus is busy,
		   1 otherwise
	
*/
uint8_t siedleSendRaw(uint8_t *data) {
	if (internalState==IDLE)
	{
		disableExtInt;
		for (uint8_t c=0;c<4;c++)
		{
			dataOut[c]=data[c];
		}
		purgeTimer();
		internalState=SEND;
		setCompareMatch1ms;
		bitCount=0;
		startTimer;
		return 1;
	}
	return 0;
}

ISR(timerCompIntVect) {
	purgeTimer();
	switch(internalState) {
		case RECEIVE: {
			if (!(PIND&(1<<PIND3)))
			{
				mblistSetBit(dataIn,bitCount);
			} else mblistClearBit(dataIn,bitCount);
			bitCount++;
			if (bitCount==32)
			{
				
				if ((dataIn[0]&dataIn[1]&dataIn[2]&dataIn[3])==0xFF) //"crap detector"
				{
					/*
						Sometimes, at least in my house, there are glitches of unknown origin
						on the bus. Since these glitches seem to be shorter than 1ms, I can 
						filter them out like this. 
					*/
				} else {
					siedleRecState = REC_FIN;
				}
				/* Wait ~100ms */
				internalState=GAP_WAIT;
				bitCount=0;
				setCompareMatchMax;
				startTimer;
			} else {
				setCompareMatch2ms;
				startTimer;
			}
		}
		break;
		case SEND: {
			if (bitCount==32)
			{
				load10OhmOff;
				load200OhmOff;
				bitCount=0;
				/* Wait ~100ms */
				internalState=GAP_WAIT;
				setCompareMatchMax;	
				startTimer;		
			} else {
				load200OhmOn;
				if (mblistReadBit(dataOut,bitCount))
				{
					load10OhmOff;
					} else {
					load10OhmOn;
				}
				setCompareMatch2ms;
				startTimer;
				bitCount++;
			}
		}
		break;
		case GAP_WAIT: {
			if (bitCount==gapSize)
			{
				if (siedleRecState!=REC_FIN)
				{
					siedleRecRestart();
				} else {
					internalState=IDLE;
				}
			} else {
				bitCount++;
				setCompareMatchMax;
				startTimer;
			}
		}
		break;
	}
}

ISR(extIntVect) {
	siedleRecState = 0;
	bitCount = 0;
	internalState=RECEIVE;
	clearWaveformReg;
	resetTimer;
	setCompareMatch1ms;
	startTimer;
	disableExtInt;
}
