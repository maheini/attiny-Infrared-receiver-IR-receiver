#include <avr/io.h>
#include <avr/interrupt.h>

//ATTENTION: Timer0 is active while receiving - don't use him! (only while receiving, not while waiting)
//			 System clock is 8 Mhz.
//			 Checkcode must be exactly like the code witch was sendt


#define RCPIN 0 //choose between PB 0 -4 (to set one of PCINT 0-7)

volatile int8_t bit=26;
volatile uint16_t puffer_adress=0;
volatile uint16_t puffer_message=0;

int main()
{
	ir_set_state(1);

	DDRB |= (1<<1);
	
	while(1)
	{		
		while(bit==26);
		while (bit != 26);
		if (puffer_adress == 0b0101010101) PORTB |= (1<<1);
	}
}

ISR(PCINT0_vect)
{ //for receive start or synchronysation of receiver
	//Timer rücksetzen und Messung starten
	if (bit!=26) TCNT0 = 0;
	else
	{
		//start timer (prescaler 64) and activate match A interrupt (free running)
		puffer_adress=0;
		puffer_message=0;
		TIMSK |= (1<<4);		//compare A
		TCCR0B |= (0b11<<0);	//prescaler 64
	}
	OCR0A = 55;					//set time
}

ISR(TIMER0_COMPA_vect) //receiving now
{
	//set for next measure
	TCNT0 = 0;
	OCR0A = 111;
	if (bit<0) //if message finished? then...
	{//save result reset timer, puffer and bit
		bit=26;
		TIMSK &= ~(1<<4);
		OCR0A = 0;
		TCCR0B &= ~(0b11<<0);
		return;
	}
	if (bit>11 && bit<22) puffer_adress |= (PINB & (1<<RCPIN))<<(bit-12);
	if (bit<12) puffer_message |= (PINB & (1<<RCPIN))<<bit;
	bit--;
	return;
}

void ir_set_state(uint8_t status)
{
	if (status)
	{
		//Set up for RX pin (enable PC interrupt) + activate internal pullup resistor
		GIMSK |= (1<<5);		//enables PC interrupt
		PCMSK |= (1<<RCPIN);	//enable the correct Pin
		//PORTB |= (1<<RCPIN);	//enable pullup resistor
		sei();
	}
	else
	{
		//Set up for RX pin (enable PC interrupt) + deactivate internal pullup resistor
		GIMSK &= ~(1<<0);
		PCMSK &= ~(1<<RCPIN);
		PORTB &= ~(1<<RCPIN);
	}
	return;
}
