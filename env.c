// main application 

#include <avr/io.h>
#include <avr/interrupt.h>


// ADC latest values
struct adc_data {
	uint8_t cap_level, sust_level ;
} adc ;

// state machine tracking
#define RELEASING	1
#define ATTACKING	2
#define DEC_SUST	3

#define ATTACK_DONE_LVL  250	// at which ADC/capacitor level the attack phase should be done

void adc_setup() {
	// enable ADC. We'll use 8 bits results, so left adjust result register.
	
	// enable ADC, clock prescaler = /64
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0) ;
	
	// left adjust result, we'll use 8 bits results in high byte only.
	ADCSRB |= (1<<ADLAR);
	
	// disable digital IO on used ADC pins
	DIDR0 = 0b00000011;	
	
	// Go! =)
	ADCSRA |= (1<<ADSC) ;
}

void run_main_timer() {	// enable 16bit timer1 + interrupts
	
	// OCR1AH = 0x1f; // 8MHz, divider 8000 -> 1KHz timer
	// OCR1AL = 0x40;
	
	OCR1AH = 0x03; // 8MHz, divider 1000 -> 8KHz timer
	OCR1AL = 0xe8;
	
	// clear on OCR1A, clock prescaler 1. 
	TCCR1B |= (1<<WGM12) | (1<<CS10) ;
	// TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS11) ;  // presc. /64
	
	// enable OCR1A interrupt
	TIMSK1 |= (1<<OCIE1A);
}

void sw_set_release() {
	PORTB &= 0xf8;	// all switches off
	PORTB |= _BV(PB0);	
}
void sw_set_attack() {
	PORTB &= 0xf8;	// all switches off
	PORTB |= _BV(PB1);
}
void sw_set_dec_sust() {
	PORTB &= 0xf8;	// all switches off
	PORTB |= _BV(PB2);
}


void state_update() {
	
	uint8_t gate_went_high=0 ;	// event: gate went high
	uint8_t gate_went_low=0 ;	// event: gate went low
	
	static uint8_t prev_gate_level ; // saves previous gate level 
	
	uint8_t gate ; // gate input value

	static uint8_t stage = RELEASING ; 	// curremt ENV stage
	
	gate = ( ~PINA & 0x80 );	// check gate input pin
	
	
	if ( prev_gate_level == 0 ) {	
		if (gate) { // gate went low->high
			gate_went_high = 1 ;
			prev_gate_level = 1;
		}
	} else {	// prev state high, low now... 
		if (! gate) {
			gate_went_low = 1 ; 
			prev_gate_level = 0 ;
		}
	}
	

	// state machine 
	switch (stage) {
	
		case RELEASING:
			if (gate_went_high) {	// act on activated gate signal
				stage = ATTACKING;
				sw_set_attack();
			}
			break;
			
		case ATTACKING:
			// gate went low while attacking? -> go to release
			if (gate_went_low) {	
				stage = RELEASING;
				sw_set_release();
			}
			// capacitor level reached limit? -> go to decay/sustain
			if (adc.cap_level >= ATTACK_DONE_LVL) {
				stage = DEC_SUST;
				sw_set_dec_sust();
			}
			break;
			
		case DEC_SUST:
			// gate went low in decay/sustain? -> go to release
			if (gate_went_low) {
				stage = RELEASING;
				sw_set_release();
			}
			if (gate_went_high) {	// act on activated gate signal -> attack phase
				stage = ATTACKING;
				sw_set_attack();
			}
			break;
			
	}	
	
}


ISR(TIM1_COMPA_vect) {
	
	static uint8_t adc_chan=0;

	
	uint8_t chan_mask=0x01; // use 2 channels (0-1).
	uint8_t adc_in = ADCH;	// read 8 MSB of ACD results (8 bit resolution used)
	
	switch (adc_chan) {
		case 0 :	// chan 0, current envelope capapcitor level
			adc.cap_level = adc_in;
			break;
		case 1 :	// chan 1, currently set sustain level
			adc.sust_level = adc_in;
			break;

	}

	// next channel...
	adc_chan = ((adc_chan+1) & chan_mask ) ;
	
	// update ADMUX register
	ADMUX &= 0xf0;	// reset channel bits
	ADMUX |= adc_chan ;
	
	// start next conversion
	ADCSRA |= (1<<ADSC) ;
	
	// update ENV state machine 
	state_update();

}
