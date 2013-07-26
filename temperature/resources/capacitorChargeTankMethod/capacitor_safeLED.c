/*
	CAPACITIVE TANK BASED CAPACITANCE METER
*/
// Version: 0.1
//#include <global.h>

#include <avr/io.h>			//included so we can use the text label for ports, pins etc.
#include <avr/interrupt.h>	//include interrupt support
#include <util/delay.h>
//#include <inttypes.h>
//#include <avr/pgmspace.h>


#include <stdio.h>

//#include "LCD/lcd_hw.h"
//#include <LCD/lcd.h>
#include "lcd.h"

//#include <timer.h>
//#include <a2d.h>

//GLOBAL VARIABLES
volatile int drive_ctr;
volatile int wtemp;
static volatile int ac_captured;
static volatile uint16_t measured;
volatile int drive_length;
char test[10];

#define hi 4
#define lo 5
#define PULSE_PERIOD	5
#define DIRECTPUMP
#define FIXED_BURST	100
#define LED_ON PORTD|=(1<<4)
#define LED_OFF PORTD&=~(1<<4)

inline void init_all()
{
DDRD|=(1<<4);
//	lcd_init(LCD_DISP_ON);
/*
	DDRC=0x00;	// outputs
	DDRD=0xff;
	DDRB=0xff;
	PORTC = 0x00;
	PORTD = 0x00;
*/
	//lcd_init();	
	
/*
/////////////////
	PORTC = 0; // no pullups on port C, ever
	
	// ADC disable
	// ADC control and status register
	ADCSRA = 0;
	
	// Analog Comparator: input capture enable
	//okay bingo
	ACSR = _BV(ACIC); 
	
	TCCR1A = 0b00000000;			// Timer1	
	TCCR1B = _BV(ICNC1);			// noise canceller on, stopped (clock source disabled)
//CALIBRATION CODE
#ifdef FIXED_BURST
	drive_length = FIXED_BURST;			//okay this might be for the duration of charge bursts
#else
	drive_length = 16;
#endif	

*/	
/*
	sprintf(test,"Initialized");
	lcd_string(test,1);
	_delay_ms(10);
*/
for(int i=0;i<100;i+=10)
{
LED_ON;
_delay_ms(100-i);
LED_OFF;
_delay_ms(100-i);
}
}

int main(void)
{
init_all();
/*

lcd_gotoxy(0,0);
lcd_puts("Capacitance Meter");

lcd_gotoxy(0,1);
lcd_puts("XRS 2011");
*/
/*
	PORTD &= ~(1<<6);	//pullups off in INT0
	DDRD &= ~(1<<6);	//tristate INT0
	///////////////
*/
	DDRC &= ~(1<<0);
	PORTC &= ~(1<<0);

/*
//NOW SCANING
//Select Y
	PORTC = 0;
	DDRC = 0xFF;	
	
//Pumping
	PORTC = 0;
	PORTD &= ~_BV(2); 	// no slope //basically we don't wanna measure decay of capacitor now
	DDRD  &= ~_BV(2);  // PB1 = Z (tristate)
	drive_ctr = drive_length;	//drive counter is set
//Direct Pumping
	for (drive_ctr = drive_length; --drive_ctr >= 0; ) {
		DDRC &= ~(1 << hi);		//PC0..top is tristate
		PORTC |= (1 << lo); 	//PC1..bottom is ground
			
		PORTD |= (1 << 3);		//PD3..for drive on
		_delay_ms(1);
		
		DDRC &= ~(1 << lo);		//PC0..bottom is tristate
		PORTC |= (1 << hi); 	//PC1..top is ground

		PORTD &= ~(1 << 3);		//PD3..for drive off
		_delay_ms(1);
	}


//POST CHARGE sequence
for(wtemp=32767; --wtemp && drive_ctr != 0;);		// wait until pumped if in timer pump mode
*/

//ACTUAL MEASUREMENT
	ac_captured = 0;
/*	
	ADMUX = 5;			// select ADMUX for ADC 5, i.e. 0101
	
	SFIOR |= _BV(ACME);			// enable admux...confirmed
	
	// Analog Comparator input capture enable
	ACSR = _BV(ACIC); 			//confirmed
	
	ADCSRA |= (1<<ACME);		//no clue...
	
	ICR1 = 0;	//resetting done
	TCNT1 = 0;	//okay, timer 1 counter resetted
	
	TIFR &= ~_BV(ICF1);			// clear input capture bit	//okay
*/
////////////////
	//ADC Control & State Register
/*

	//SFIOR (special function I/O register)
	SFIOR |= (1<<ACME);	//to enable multiplexer

	ADCSRA &= ~(1<<ADEN);		//Simply disable ADC..we need to use comparitor
	//ADMUX (A2d Multiplexer)
	ADMUX |= (0<<MUX2)|(0<<MUX1)|(0<<MUX0);


	//ACSR (analog comparator control & state register)
	//Always clear interrupt enable (ACIE) before changing this
	//ACSR = (1<<ACIC) | (1<<ACIS1);	//Comparator Input Capture attaches to timer1, ACIS1 selects falling edge
	
	ACSR |= (0<<ACIC)|(0<<ACIS1)|(1<<ACBG)|(1<<ACIE);	//ACIS1 selects falling edge and internal band gap
	//ACSR |= (1<<ACIE);		//Comparator Interrupt Enable Now, after selection of right bits
*/
	SFIOR |=(1<<ACME);//enable multiplexer
	ADCSRA &=~(1<<ADEN);//make sure ADC is OFF
	ADMUX |=(0<<MUX2)|(0<<MUX1)|(0<<MUX0); //select ADC3 as negative AIN
	ACSR |=
	(0<<ACD)|	//Comparator ON
	(1<<ACBG)|	//Connect 1.23V reference to AIN0
	(1<<ACIE)|	//Comparator Interrupt enable
	(0<<ACIC)|	//input capture disabled
	(0<<ACIS1)| //set interrupt on output toggle
	(0<<ACIS0);
	
	sei();
	//while (~(ACSR & (1<<ACI)));
	//_delay_ms(10);
	//ACSR &= ~(1<<ACIE);		//Comparator Interrupt Enable Now, after selection of right bits
	
	//ACSR &= ~(1<<ACI);
	
/*	
	TIFR &= ~(1<<ICF1);	//Input Capture Flag cleared before chagning

	ICR1=0;			//Input Capture register
	TCNT1=0;		//Timer 1 register

	//Timer/Counter Control Register
	TCCR1A = 0;		//for normal mode, OCR1A immidiate update

	TCCR1B = (0<<ICNC1);	//for enabling input capture noise cancellation, edge falling
	TCCR1B |= (1<<CS10);	//for turning on the clock=enabling timer
	//Timer Interrupt Mask Register
	//TIMSK = (1<<TICIE1);	//Enable Interrupt for Input Capture	
	//TIFR &= ~(1<<ICF1);	//Input Capture Flag cleared before chagning//
	TIFR &= ~(1<<TOV1);	//Overflow cleared
	
	//TIMSK = (1<<TICIE1);	//Enable Interrupt for Input Capture
	TIMSK = (1<<TOIE1);	//Enable Overflow Interrupt
	
	////////////////	
	
	PORTC &= ~_BV(4);				// disable charging
	DDRC |= _BV(4);
	
	PORTD &= ~_BV(2);				// disable slope
	DDRD  |= _BV(2);  				// PD2 = +Vdd
*/
	/*
	//ORIGINALLY HERE
	//TIMSK = _BV(TICIE1); 			// enable input capture interrupt
	TCCR1A = 0b00000000;			// Timer1 normal mode	
	TCCR1B = _BV(ICNC1) | 0b001;	// noise canceller, input capture on negative edge, go	
	TIMSK = _BV(TICIE1); 			// enable input capture interrupt //confirmed
	*/


//now wait for measurement to complete	
//for(wtemp=32767; --wtemp && ac_captured == 1;);	// wait until capture	

//while (~(ACSR & (1<<ACO)));
//measured=TCNT1;
//TCCR1B=0;

//while (ac_captured==0) ;

//now answer is in the variable measured
//lcd_gotoxy(0,1);
for (;;)
{
;
/*
	if bit_is_clear(ACSR, ACO)
		measured=999;
	else
		measured=0;
*/
//while(ac_captured==0);
//ac_captured=0;
/*
sprintf(test,"Time:%d  %d",measured,ac_captured);
//sprintf(test,"DONE%D",measured);
//lcd_puts(test);
lcd_string(test,1);
_delay_ms(1000);

sprintf(test,"Damn");
//sprintf(test,"DONE%D",measured);
//lcd_puts(test);
lcd_string(test,1);
_delay_ms(1000);
*/
/*
LED_ON;
_delay_ms(10);
LED_OFF;
*/
}
}
//SIG_INPUT_CAPTURE1
/*
ISR(TIMER1_CAPT_vect){

//void SIG_INPUT_CAPTURE1( void ) __attribute__ ( ( signal ) );  
//void SIG_INPUT_CAPTURE1( void ) {

	SFIOR &= ~_BV(ACME);	// disable admux
	// magic
	ACSR &= ~_BV(ACIC); 	// disable AC capture input
	DDRC = 0xFF;			// drain all
	PORTD &= ~_BV(2); 		// slope to gnd PD2
	
	ac_captured = 1;		//set the captured flag
	
	measured = ICR1;		//save the value .. time elapsed
	//measured=1992;
	
	TIMSK &= ~_BV(TICIE1);	//disable input capture interrupt
	TCCR1B = 0b00000000;	//timer turned off (my guess)

}
*/
ISR(ANA_comp_vect){
	
	if bit_is_clear(ACSR, ACO)
		//PORTD &= ~(1<<4);
		//measured=100;
		LED_ON;
	else
		//PORTD |= (1<<4);
		//measured=1;
		LED_OFF;
	
	ac_captured=1;
	//measured=100;
	
}
/*
ISR(TIMER1_OVF_vect){

//void SIG_INPUT_CAPTURE1( void ) __attribute__ ( ( signal ) );  
//void SIG_INPUT_CAPTURE1( void ) {

	SFIOR &= ~_BV(ACME);	// disable admux
	// magic
	ACSR &= ~_BV(ACIC); 	// disable AC capture input
	DDRC = 0xFF;			// drain all
	PORTD &= ~_BV(2); 		// slope to gnd PD2
	
	ac_captured = 2;		//set the captured flag
	
	//measured = ICR1;		//save the value .. time elapsed
	measured=999;
	
	TIMSK &= ~_BV(TOIE1);	//disable timer overflow interrupt
	TCCR1B = 0b00000000;	//timer turned off (my guess)
	TCNT1=0;		//Timer 1 register
}
*/