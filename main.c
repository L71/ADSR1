#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "env.h"

int main() {

	// IO configuration:
	// PB0, PB1, PB2:	envelope capacitor switches (active: 1)
	// PA7:				gate signal input (active: 0)
	// PA0, PA1:		ADC inputs, capacitor level and sustain level.
	// PA2:				mode selection push button input (with pull-up)
	// PA3:				gate signal output (processed)
	// PA4/SCK:			clock out for UI counter ship
	// PA5/MISO:		reset out for UI counter chip
	// PA6/MOSI:		gate legato switch input

	// output pins for analog switches 
	DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB2);

	// output pin for gate signal, UI chip clock, UI chip reset
	DDRA |= _BV(PA3) | _BV(PA4) | _BV(PA5);
	
	// activate input pull-up on PA2 (mode switch) and PA6 (gate legato switch):
	PORTB |= _BV(PA2) | _BV(PA6);

	// activate release switch
	sw_set_release();
	
	pc_int_setup();
	adc_setup();
	run_main_timer();

	// reset UI counter chip
	PORTA |= _BV(PA5);
	PORTA &= ~_BV(PA5);
	PORTA |= _BV(PA4);

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
