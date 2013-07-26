// Version: 0.1
#include <global.h>

#include <avr/io.h>			//included so we can use the text label for ports, pins etc.
#include <avr/interrupt.h>	//include interrupt support
#include <util/delay.h>

#include <stdio.h>

//#include "LCD/lcd_hw.h"
//#include <LCD/lcd.h>
#include "lcd.h"

//#include <timer.h>
//#include <a2d.h>

//GLOBAL VARIABLES
int drive_ctr;
int wtemp;
int ac_captured;
int measured;
int drive_length;
char test[10];
#define hi 1
#define lo 0
#define PULSE_PERIOD	5
#define DIRECTPUMP
#define FIXED_BURST	100
inline void init_all()
{
//	lcd_init(LCD_DISP_ON);

	DDRC=0xff;	// outputs
	DDRD=0xff;
	DDRB=0xff;
	PORTC = 0x00;
	PORTD = 0x00;
	lcd_init();	
	
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

for (;;)
{

//NOW SCANING
//Select Y
	PORTC = 0;
	DDRC = 0xff;	
//Pumping
	PORTC = 0;
	PORTB &= ~_BV(1); 	// no slope //basically we don't wanna measure decay of capacitor now
	DDRB  &= ~_BV(1);  // PB1 = Z (tristate)
	drive_ctr = drive_length;	//drive counter is set
//Direct Pumping
	for (drive_ctr = drive_length; --drive_ctr > 0; ) {
		DDRC &= ~(1 << hi);		//PC0..top is tristate
		PORTC |= (1 << lo); 	//PC1..bottom is ground
			
		PORTB |= (1 << 0);		//PB0..for drive on

		DDRC &= ~(1 << lo);		//PC0..bottom is tristate
		PORTC |= (1 << hi); 	//PC1..top is ground

		PORTB &= ~(1 << 0);		//PB0..for drive off

	}

//POST CHARGE sequence
for(wtemp=32767; --wtemp && drive_ctr != 0;);		// wait until pumped if in timer pump mode

//ACTUAL MEASUREMENT
	ac_captured = 0;
	
	ADMUX = 0;			// select ADMUX for ADC 0, i.e. 0000
	SFIOR |= _BV(ACME);			// enable admux
	// Analog Comparator input capture enable
	ACSR = _BV(ACIC); 
	
	ICR1 = 0;
	TCNT1 = 0;
	TIFR &= ~_BV(ICF1);			// clear input capture bit	
	TIMSK = _BV(TICIE1); 			// enable input capture interrupt
	TCCR1A = 0b00000000;			// Timer1 normal mode
	TCCR1B = _BV(ICNC1) | 0b001;	// noise canceller, input capture on negative edge, go
	
	PORTB |= _BV(1);				// enable slope
	DDRB  |= _BV(1);  				// PB1 = +Vdd
//now wait for measurement to complete	
for(wtemp=32767; --wtemp && ac_captured == 0;);	// wait until capture	
//now answer is in the variable measured
//lcd_gotoxy(0,1);
sprintf(test,"%D",measured);
//lcd_puts(test);
lcd_string(test,1);
_delay_ms(10);
}
}

void SIG_INPUT_CAPTURE1( void ) __attribute__ ( ( signal ) );  
void SIG_INPUT_CAPTURE1( void ) {
	SFIOR &= ~_BV(ACME);	// disable admux
	// magic
	ACSR &= ~_BV(ACIC); 	// disable AC capture input
	DDRC = 0xFF;			// drain all
	PORTB &= ~_BV(1); 		// slope to gnd
	
	ac_captured = 1;		//set the captured flag
	measured = ICR1;		//save the value .. time elapsed
	TIMSK &= ~_BV(TICIE1);	//disable input capture interrupt
	TCCR1B = 0b00000000;	//timer turned off (my guess)
}

