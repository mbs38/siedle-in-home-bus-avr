#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/eeprom.h>
#include "siedleBus.h"

inline void ledOn(void) {
	PORTD&=~(1<<PORTD6);
}

inline void ledOff(void) {
	PORTD|=(1<<PORTD6);
}

void convertIntToChars(uint8_t inInt,uint8_t* ptrChArray) {
	ptrChArray[0]=(inInt/100)+48;
	inInt=inInt-((inInt/100)*100);
	ptrChArray[1]=(inInt/10)+48;
	inInt=inInt-((inInt/10)*10);
	ptrChArray[2]=inInt+48;
}

uint8_t convertCharsToInt(volatile uint8_t * inChars){
	uint8_t res = 0;
	res=(inChars[0]-48)*100;
	res=res+(inChars[1]-48)*10;
	res=res+(inChars[2]-48);
	return res;
}

volatile uint8_t uartRxCount = 0;
volatile uint8_t uartRxComplete = 0;
volatile uint8_t uartRxArray[21];


ISR(USART_RX_vect)
{
	uint8_t in = UDR;
	if (!uartRxComplete)
	{
		if (in=='S')
		{
			uartRxCount=0;
		} else if (in=='\r' && uartRxCount==20)
		{
			uartRxComplete=1;
		}
		if (uartRxCount<21)
		{
			uartRxArray[uartRxCount]=in;
		}
		uartRxCount++;
		
	}		
}


/* @brief: sends an info string to using the serial port.
*/
void uartSendInfo(uint8_t recd, volatile uint8_t *sent) {
	uint8_t uartOut[21];
	/*
	 * 'S','E','N','T',':',0,0,0,',',0,0,0,',',0,0,0,',',0,0,0,'\r'
	 * 'R','E','C','D',':',0,0,0,',',0,0,0,',',0,0,0,',',0,0,0,'\r'
	*/
	uartOut[0]='S';
	uartOut[1]='E';
	uartOut[2]='N';
	uartOut[3]='T';
	uartOut[4]=':';
	uartOut[8]=',';
	uartOut[12]=',';
	uartOut[16]=',';
	uartOut[20]='\r';
	if (recd)
	{
		uartOut[0]='R';
		uartOut[2]='C';
		uartOut[3]='D';
	}
	convertIntToChars(sent[0],uartOut+5);
	convertIntToChars(sent[1],uartOut+9);
	convertIntToChars(sent[2],uartOut+13);
	convertIntToChars(sent[3],uartOut+17);
	for (uint8_t cc=0;cc<21;cc++)
	{
		UDR=uartOut[cc];
		while ( !( UCSRA & (1<<UDRE)) );
	}
}

int main(void)
{
	PORTD=0;
	DDRD = (1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6);
	DDRD &= ~(1<<PORTD3);

	sei();
	
	#define baud 11 //19200
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);

	ledOff();
	_delay_ms(10);
	siedleSetup();

    while(1)
    {
		if (siedleGetFrame())
		/* @brief: handle frames from bus. Send an info
		 *         string containing the frame
		 *         via the serial port of the AVR.
		*/
		{
			
			_delay_ms(400);
			ledOn();
			uartSendInfo(1,dataIn);
			ledOff();
			siedleRecRestart();
		}
		if (uartRxComplete)
		/* @brief: Handle commands from the user (via serial
		 * 	   port). Extract frame from string and transmit
		 * 	   it to the bus.
		*/
		{
			uint8_t toSend[4];
			toSend[0]=convertCharsToInt(uartRxArray+5);
			toSend[1]=convertCharsToInt(uartRxArray+9);
			toSend[2]=convertCharsToInt(uartRxArray+13);
			toSend[3]=convertCharsToInt(uartRxArray+17);

			if (siedleSendRaw(toSend))
			{
				ledOn();
				uartSendInfo(0,dataOut);
				ledOff();
			}
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				uartRxComplete=0;
			}
			
		}
    }
}
