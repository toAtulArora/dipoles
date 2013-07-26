
/*
	CAPACITIVE TANK BASED CAPACITANCE METER
*/

// Version: 0.1
//#include <global.h>

#include <avr/io.h>			//included so we can use the text label for ports, pins etc.
#include <avr/interrupt.h>	//include interrupt support
#include <util/delay.h>


#include <stdio.h>

#include "lcd.h"

//GLOBAL VARIABLES

char test[10];

#define hi 4
#define lo 5
#define PULSE_PERIOD	5
#define DIRECTPUMP
#define FIXED_BURST	100
#define LED_ON PORTD|=(1<<4)
#define LED_OFF PORTD&=~(1<<4)
volatile int i,waiting;
volatile uint16_t measured;
unsigned short b;
volatile uint16_t high_word=0;
volatile uint16_t measured_high=0;

volatile int drive_ctr;
volatile int wtemp;
static volatile int ac_captured;

volatile int drive_length=50;

int count=0;
void Init()
{
	
	lcd_init();
	sprintf(test,"RESETTED");
	lcd_string(test,1);
	_delay_ms(1000);
	TCCR1A = 0;		//for normal mode, OCR1A immidiate update	//Timer/Counter Control Register
	ADCSRA &=~(1<<ADEN);//make sure ADC is OFF
	
	DDRD|=(1<<4);	//THE LED
sei();
}

int main(void) {
     Init();
	 while(1) {     // Infinite loop; interrupts do the rest
	 count++;
///////////////	
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
			//asm("nop");
			//asm("nop");
			//_delay_ms(1);
			
			DDRC &= ~(1 << lo);		//PC0..bottom is tristate
			PORTC |= (1 << hi); 	//PC1..top is ground

			PORTD &= ~(1 << 3);		//PD3..for drive off
			//asm("nop");
			//asm("nop");
			//_delay_ms(1);
		}

	//POST CHARGE sequence
	for(wtemp=32767; --wtemp && drive_ctr != 0;);		// wait until pumped if in timer pump mode
*/

		PORTC = 0;			//All earthed
		DDRC = 0xFF;		//All outputs

		//SLope Tristate
		PORTD &= ~_BV(2); 	// no slope //basically we don't wanna measure decay of capacitor now
		DDRD  &= ~_BV(2);  // PB1 = Z (tristate)
		DDRD  |= _BV(3);	//Port D 3 is output enabled
		
		_delay_ms(40);		//give time to discharge
///////////////////////////////////////////
//COMMON SO FAR
	for(int b=0;b<200;b++)
	{
		DDRC &= ~(1<<hi); 		// top is tris
		DDRC |= (1<<lo); 		// bottom is gnd
			
		PORTD |= (1<<3);		//Turn on Drive

		DDRC &= ~(1<<lo);		// bottom is tris
		DDRC |= (1<<hi);		// top is gnd
		
		PORTD &= ~(1<<3);		//Turn off drive
	}
//////////////	
	
	waiting=1;
	DDRC &= ~(1<<5);
	PORTC &= ~(1<<5);
	
	
	SFIOR |=(1<<ACME);//enable multiplexer
	ADMUX |=(1<<MUX2)|(0<<MUX1)|(1<<MUX0); //select ADC3 as negative AIN
	ACSR |=
	(0<<ACD)|	//Disable Comparator
	(0<<ACBG)|	//Connect 1.23V reference to AIN0
	(1<<ACIE)|	//Comparator Interrupt enable
	(0<<ACIC)|	//input capture disabled
	(1<<ACIS1)| //toggle set interrupt on falling edge
	(0<<ACIS0);
	
	TCNT1=0;
	high_word=0;
	TIMSK |= (1<<TOIE1);	//Timer Input Mask Register..for Timer 1 Overflow
/////////////////
	
	PORTC &= ~(1 << hi); 	//PC4 or hi is earthed
	DDRD |= (1<<2);		//drive enable output
	PORTD |= (1<<2);		//drive +V
	
	TCCR1B |= (1<<CS10);	//for turning on the clock=enabling timer	
	

	while(waiting);
	
	//measured=measured>>1;

	SFIOR &= ~(1<<ACME);//disable multiplexer	
	/*
	ACSR |=
	(1<<ACD)|	//Disable Comparator
	(1<<ACBG)|	//Connect 1.23V reference to AIN0
	(0<<ACIE)|	//Comparator Interrupt enable
	(0<<ACIC)|	//input capture disabled
	(0<<ACIS1)| //set interrupt on output toggle
	(0<<ACIS0);
	*/
	
	DDRC = 0xFF;			// drain all
	PORTC = 0;
	PORTB &= ~_BV(2); 		// slope to gnd
	
	sprintf(test,"WORKING %u   ",measured_high);
	lcd_string(test,1);	
	sprintf(test,"%d   ",measured);
	lcd_string(test,16);	
	
	_delay_ms(100);
	PORTD |= (1<<4);
	
   }
	
}


ISR(ANA_COMP_vect) {
//	if bit_is_clear(ACSR,ACO)
//	{
	waiting=0;
	measured=TCNT1;
	measured_high=high_word;
	TCCR1B = 0;
//	}

//	if bit_is_clear(ACSR, ACO)
		PORTD &= ~(1<<4);
//	else
//		PORTD |= (1<<4);
}

ISR(TIMER1_OVF_vect){
	high_word++;
}
/*
ISR(ANA_comp_vect){
	
	if bit_is_clear(ACSR, ACO)
		PORTD &= ~(1<<4);
	else
		PORTD |= (1<<4);
	
	//ac_captured=1;
	//measured=100;
	
}
*/

/*
// ***********************************************************
// Project:  Analog Comparator example	
// Author: winavr.scienceprog.com	
// Module description: Analog comparator example with positive comparator
// input connected to Vref 1.23V. When compared voltage exceed 1.23V LED lighst on.
// When voltage drops bellow - LED turns OFF. Comparator inputis connected to ADC3 pin.
// ***********************************************************


#include <avr/io.h>       
#include <avr/interrupt.h>  
#include "lcd.h"
#include <util/delay.h>
#include <stdio.h>

#define AINpin 0
#define LED 4
char test [10];
void Init()
{
	DDRC &=~(1<<AINpin);//as input
	PORTC &=~(1<<AINpin);//no Pull-up
	
	DDRD |=(1<<LED); //Led pin as output
	PORTD |=(1<<LED);//Initally LED is OFF
	
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
	lcd_init();
	sprintf(test,"RESETTED");
	lcd_string(test,1);
	_delay_ms(1000);
	
	sei();//enable global interrupts
}
// Interrupt handler for ANA_COMP_vect
//
ISR(ANA_COMP_vect) {
if bit_is_clear(ACSR, ACO)
	PORTD&=~(1<<LED);//LED is ON
	else 	PORTD|=(1<<LED);//LED is OFF
}
// ***********************************************************
// Main program
//
int main(void) {
     Init();
	 while(1) {     // Infinite loop; interrupts do the rest

	sprintf(test,"WORKING");
	lcd_string(test,1);
	_delay_ms(10);
	 
   }
}
*/