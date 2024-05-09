/*
 * HC-SR04_AT84.c
 *
 * Created: 2014-02-15 23:05:53
 *  Author: Wim
 */ 

#include "HC_SR04_AT85.h"

volatile uint8_t echoDone = 0;  // 1/0 flag to check if echo is over
static uint32_t countTimer1 = 0;  // current timer0 count



void calibrateClockTest(void)
{
    overclock();                    // This value is that value to be calibrated. Get it as close as possible to 10Mhz with PLL enabled.
    TCCR1 = _BV(CTC1) | _BV(CS10);  // CTC mode, /1
    GTCCR = _BV(COM1B0);            // Toggle OC1B
    PLLCSR = 0<<PCKE;               // System clock as clock source
    OCR1C = 0;
    DDRB |= _BV(PB4);
}

void plotValue(uint8_t val)
{
    int i,j;
    PORTB |= _BV(PB4);
    __asm__("nop");
    PORTB &= ~_BV(PB4);
    for (j=0; j<10; j++){__asm__("nop");}
    for(i = 0; i < 8; i++){
        if(val & _BV(7-i)) { PORTB |= _BV(PB4); }
        else { PORTB &= ~_BV(PB4); }
        for (j=0; j<10; j++){__asm__("nop");}
    }
    PORTB &= ~_BV(PB4);
    for (j=0; j<10; j++){__asm__("nop");}
    PORTB |= _BV(PB4);
    __asm__("nop");
    PORTB &= ~_BV(PB4);
    for (j=0; j<50; j++){__asm__("nop");}
}

void SONARinit()
{
	// initialize HC-SR04 pins
	// set Trigger pin as output
	// set Echo pin as input
	TRIGDDR |= (1<<TRIGPIN);
	ECHODDR &= ~(1<<ECHOPIN);
}

uint32_t SONARread()
{
	// Send a 10us HIGH pulse on the Trigger pin.
	// The sensor sends out a “sonic burst” of 8 cycles.
	// Listen to the Echo pin, and the duration of the next HIGH
	// signal will give you the time taken by the sound to go back
	// and forth from sensor to target.

	// return distance (mm)
	// enable pin-change interrupt on PCINT0:
	cli();
	// enable interrupt
	GIMSK |= (1 << PCIE);
	// enable pin
	PCMSK |= (1 << ECHOPCINT);
	sei();

	// set echo flag
	echoDone = 0;
	// reset counter
	countTimer1 = 0;

	// send 10us trigger pulse
	//    _
	// __| |__
	TRIGPRT &= ~(1 << TRIGPIN);
	_delay_us(20);
	TRIGPRT |= (1 << TRIGPIN);
	_delay_us(12);
	TRIGPRT &= ~(1 << TRIGPIN);
	_delay_us(20);

	// listen for echo and time it
	//       ____________
	// _____|            |___

	// loop till echo pin goes low
    //PORTB |= (1<<PB4);
	while(!echoDone);
    //PORTB = PINB & ~(1<<PB4);

	// disable pin-change interrupt:
	// disable interrupt
	GIMSK &= ~(1 << PCIE);
	// disable pin
	PCMSK &= ~(1 << ECHOPCINT);
	
	//  countTimer1 /= scaler;	// use 1 MHz, no need for scaler
	// in case of F_CPU = 8000000UL, set CS11 for timer1 prescaler 1/8
	return countTimer1;
}

uint32_t SONARavg()
{
	uint32_t val = 0;
	for (int ii = 0; ii<=2; ii++)
	{
		val += SONARread();
	}
	val /= 3;
	return val / 1311;
}	

ISR(TIM1_OVF_vect)
{
	// increment
	countTimer1 += 65535;
}

ISR(PCINT0_vect)
{
	// read Pin Change Interrupt 0 on PA0:
	if(ECHOPRT & (1 << ECHOPIN)) {
		// rising edge:
		// set overflow interrupt flag
		TIMSK |= (1<<TOIE1);
		// start 16-bit timer
		// Divide by 1 for 1 MHz, divide by 8 for 8 MHz (CS11)
		// reset counter in timer0
		TCNT1 = 0;
		TCCR1 |= (1<<CS10);	// use CS11 for 8 MHz clock
	}
	else {
		// falling edge

		// stop timer
		TCCR1 &= ~(1<<CS10);
		TIMSK &= ~(1<<TOIE1);
		// calculate time passed
		countTimer1 += TCNT1;


		// set flag
		echoDone = 1;
	}
	// probably not necessary to clear interrupt flag according to attiny docs
    // GIFR |= ( 1 << PCIF0 );    // clear interrupt flag 
}
