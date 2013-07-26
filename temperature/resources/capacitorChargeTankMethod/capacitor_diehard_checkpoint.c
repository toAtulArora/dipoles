/*
	CAPACITIVE TANK BASED CAPACITANCE METER
*/
// Version: 0.1
#include <global.h>

#include <avr/io.h>			//included so we can use the text label for ports, pins etc.
#include <avr/interrupt.h>	//include interrupt support
#include <util/delay.h>
#include <inttypes.h>
#include <avr/pgmspace.h>


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
inline void init_all()
{
//	lcd_init(LCD_DISP_ON);

	DDRC=0x00;	// outputs
	DDRD=0xff;
	DDRB=0xff;
	PORTC = 0x00;
	PORTD = 0x00;
	ADCSRA &= ~(1<<ADEN);		//Simply disable ADC..we need to use comparitor	
	lcd_init();	
	
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
*/
//CALIBRATION CODE
#ifdef FIXED_BURST
	drive_length = FIXED_BURST;			//okay this might be for the duration of charge bursts
#else
	drive_length = 16;
#endif	

	
	sprintf(test,"Initialized");
	lcd_string(test,1);
	_delay_ms(10);


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
int keepcount=0;
while(1)
{
	keepcount++;
//	DDRC = 0x00;
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
	//ADC Control & State Register
	//right now, only top (hi) is ground..everything else is tristate
	DDRC &= ~(1<<0);	//just to confirm//lo is tristate, ready for input
	PORTC &= ~(1<<0);	//	
	//SFIOR (special function I/O register)
	SFIOR |= (1<<ACME);	//to enable multiplexer
	ADMUX |= (0<<MUX2)|(0<<MUX1)|(0<<MUX0);	//ADMUX (A2d Multiplexer)//equivalent of lo here

	ACSR = (1<<ACIC)|(0<<ACIS1)|(1<<ACBG);	//ACIS1 selects falling edge and internal band gap	//ACSR (analog comparator control & state register) 	//Always clear interrupt enable (ACIE) before changing this
	ACSR &= ~(1<<ACIE);		//disable Comparator Interrupt Enable Now, after selection of right bits


	ICR1=0;			//Input Capture register
	TCNT1=0;		//Timer 1 register

	TIFR |= (1<<ICF1);	//Input Capture Flag cleared before chagning
	TIMSK |= (1<<TICIE1);	//Enable Interrupt for Input Capture	//Timer Interrupt Mask Register
	
	TCCR1A = 0;		//for normal mode, OCR1A immidiate update	//Timer/Counter Control Register
	TCCR1B = (0<<ICNC1);	//for enabling input capture noise cancellation, edge falling
	TCCR1B |= (1<<CS10);	//for turning on the clock=enabling timer

	
	//Draining and checking for zero volt
	/*
	PORTD |= _BV(2);				// enable slope..make it 
	DDRD  |= _BV(2);  				// PD2 = +Vdd
	*/
	//and set interrupts
	//TIFR |= (1<<ICF1);	//Input Capture Flag cleared before chagning//Timer Interrupt Flag Register
	

	sei();	
	while(ac_captured==0);	//Wait while zero is crossed
	//cli();
	
	//DDRC = 0xFF;			// drain all
	//PORTD &= ~_BV(2); 		// slope to gnd PD2
	
//EXPERIMENTING
/*
	TIFR &= ~(1<<ICF1);	//clear input capture flag, timer 1
	ACSR &= ~(1<<ACIE);	//disable comparator interrupt enable
	ACSR |= _BV(ACD); 		// disable analog comparator
	ACSR &= ~_BV(ACIC); 	// disable AC capture input capture..timer1 relation
	SFIOR &= ~_BV(ACME);	// disable admux
	TCCR1B = 0;
	
	DDRC = 0xFF;			// drain all
	PORTD &= ~_BV(2); 		// slope to gnd PD2
*/

	//Now just display the time measured
	sprintf(test,"Time:%d ",measured);
	lcd_string(test,1);
	sprintf(test,"Count:%d ",keepcount);
	lcd_string(test,16);
	_delay_ms(10);
	
if bit_is_clear(ACSR, ACO)
		PORTD&=~(1<<4);//LED is ON
	else 	PORTD|=(1<<4);//LED is OFF
	
}	
	////////////////	
	
/*
	PORTC &= ~_BV(4);				// disable charging
	DDRC |= _BV(4);
	
	PORTD &= ~_BV(2);				// disable slope
	DDRD  |= _BV(2);  				// PD2 = +Vdd
*/


//now wait for measurement to complete	
//for(wtemp=32767; --wtemp && ac_captured == 1;);	// wait until capture	

//while (~(ACSR & (1<<ACO)));
//measured=TCNT1;
//TCCR1B=0;

//while (ac_captured==0) ;

//now answer is in the variable measured
//lcd_gotoxy(0,1);
//for (;;)
//{



//sprintf(test,"Damn");
//sprintf(test,"DONE%D",measured);
//lcd_puts(test);
//lcd_string(test,1);
//_delay_ms(1000);

//}
}
//SIG_INPUT_CAPTURE1

ISR(TIMER1_CAPT_vect){

//void SIG_INPUT_CAPTURE1( void ) __attribute__ ( ( signal ) );  
//void SIG_INPUT_CAPTURE1( void ) {


	//
	// magic

	//ACSR|= _BV(ACD); 	// disable AC capture input
	//ACSR&= ~_BV(ACIC); 	// disable AC capture input
	//SFIOR &= ~_BV(ACME);	// disable admux
	
	//DDRC = 0xFF;			// drain all
	//PORTD &= ~_BV(2); 		// slope to gnd PD2
	

	
	ac_captured = 1;		//set the captured flag
	
	if(ICR1 != 0) measured = ICR1;		//save the value .. time elapsed
	//measured=1992;
/*
	TIFR |= (1<<ICF1);	//clear input capture flag, timer 1
	ACSR &= ~(1<<ACIE);	//disable comparator interrupt enable
	ACSR |= _BV(ACD); 		// disable analog comparator
	ACSR &= ~_BV(ACIC); 	// disable AC capture input capture..timer1 relation
	SFIOR &= ~_BV(ACME);	// disable admux
	TIMSK &= ~_BV(TICIE1);	//disable input capture interrupt
	TCCR1B = 0b00000000;				//disable timer0
*/
}

ISR(ANA_COMP_vect){
if bit_is_clear(ACSR, ACO)
		PORTD&=~(1<<4);//LED is ON
	else 	PORTD|=(1<<4);//LED is OFF
		
	//ac_captured=1;
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
	//TCNT1=0;		//Timer 1 register
}*/
