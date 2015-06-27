#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "env.h"

int main() {

	// setup input IO pins (PB0 / PB1 are inputs by default, just set PB1 pull-up)
	// PORTB |= 0x02;
	
	// also set PA7 as output (GATE THRU output signal)
	// DDRA |= _BV(PA7);

	// IO configuration:
	// PB0, PB1, PB2:	envelope capacitor switches (active: 1)
	// PA7:				gate signal input (active: 0)
	// PA0, PA1:		ADC inputs, capacitor level and sustain level.
	

	// output pins for analog switches 
	DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB2);

	// activate release switch
	// PORTB |= _BV(PB0);
	sw_set_release();
	
	adc_setup();
	run_main_timer();

    // Enable interrupts for all. GO! =)
    SREG |= 0x80 ;
	
    // main loop
    while ( 1 )  {
		// OCR0A = 0;
		// PORTA |= 0b10000000;
        _delay_ms(250);
		// OCR0A = 255;
		// PORTA &= 0b01111111;
		_delay_ms(250);
    }
  
}
