// Version: 0.1
#include <global.h>

#include <avr/io.h>			//included so we can use the text label for ports, pins etc.
#include <avr/interrupt.h>	//include interrupt support
#include <util/delay.h>

#include <stdio.h>
#include <math.h>			//for the sin function

#include <FSR/fsr_io.h>			//For Simplicity Reasons File for easy i/o
#include <LCD/lcd_hw.h>
#include <LCD/lcd.h>

//#include <timer.h>
//#include <a2d.h>

//GLOBAL VARIABLES
int drive_ctr;
int wtemp;
#define hi 1
#define lo 0
#define PULSE_PERIOD	5
#define DIRECTPUMP
inline void init_all()
{
//	lcd_init(LCD_DISP_ON);

	DDRC=0xff;	// outputs
	DDRD=0xff;
	DDRB=0xff;
	PORTC = 0x00;
	PORTD = 0x00;
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
#ifdef DIRECTPUMP
	for (drive_ctr = drive_length; --drive_ctr > 0; ) {
		DDRC &= ~(1 << hi);		//PC0..top is tristate
		PORTC |= (1 << lo); 	//PC1..bottom is ground
			
		PORTB |= (1 << 0);		//PB0..for drive on

		DDRC &= ~(1 << lo);		//PC0..bottom is tristate
		PORTC |= (1 << hi); 	//PC1..top is ground

		PORTB &= ~(1 << 0);		//PB0..for drive off

	}
#else
//we're not doing direct pumping..we use a timer instead
//so..
//Timer Start Pumping
	TCCR1A = 0b00000000; 				// OC1A/OC1B off, WGM11:WGM10 = 0
	TCCR1B = _BV(ICNC1) | _BV(WGM12); 	// CTC mode// clear timer on compare
	OCR1A = PULSE_PERIOD;
	TIFR |= _BV(TOV1);		// clear overflow bit
	TIMSK = _BV(OCIE1A); 	//_BV(TOIE1); 	// enable overflow interrupt

	TCCR1B |= 0b001; 		// enable clock source, start it
#endif	

//POST CHARGE sequence
for(wtemp=32767; --wtemp && drive_ctr != 0;);		// wait until pumped if in timer pump mode

//ACTUAL MEASUREMENT
	ac_captured = 0;
	
	//TCCR1B = _BV(ICNC1);		// input capture noise canceler, stop timer

	ADMUX = y_admux;			// select ADMUX
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
}
}
void SIG_OUTPUT_COMPARE1A( void ) __attribute__ ( ( signal ) );  
void SIG_OUTPUT_COMPARE1A( void ) {
//	PORTC &= ~(1 << hi);
//	PORTC &= ~(1 << lo);
#ifdef DIRECTPUMP
#else	
	DDRC &= ~(1 << hi);		//PC0..top is tristate
	PORTC |= (1 << lo); 	//PC1..bottom is ground
		
	PORTB |= (1 << 0);		//PB0..for drive on

	DDRC &= ~(1 << lo);		//PC0..bottom is tristate
	PORTC |= (1 << hi); 	//PC1..top is ground

	PORTB &= ~(1 << 0);		//PB0..for drive off

	if (--drive_ctr == 0) {
		// end of burst: end drive, enable slope, enter capture mode			
		TIMSK &= ~_BV(OCIE1A); 	// disable overflow interrupt
	}
#endif
}
