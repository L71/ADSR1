// main application (header file)


// setup of timer and ADC etc
void adc_setup(void);		// configure ADC HW
void run_main_timer(void);	// start main timer
void pc_int_setup(void);	// pin change ISR for GATE IN

// set capacitor switches ... single functions.
// only ONE switch must ever be enabled at any one time. 
// (failing here may fry the switch IC)
void sw_set_release(void);
void sw_set_attack(void);
void sw_set_dec_sust(void);
