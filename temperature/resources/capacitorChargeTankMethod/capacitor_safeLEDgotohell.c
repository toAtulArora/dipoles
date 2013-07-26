
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
int i;
void Init()
{
	DDRC &= ~(1<<0);
	PORTC &= ~(1<<0);
	DDRD|=(1<<4);
	
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
	
/*	
for(i=0;i<100;i+=10)
{
LED_ON;
_delay_ms(100-i);
LED_OFF;
_delay_ms(100-i);
}
*/
	lcd_init();
	sprintf(test,"RESETTED");
	lcd_string(test,1);
	_delay_ms(1000);

sei();
}

int main(void) {
     Init();
	 while(1) {     // Infinite loop; interrupts do the rest

	sprintf(test,"WORKING");
	lcd_string(test,1);
	_delay_ms(10);
	 
   }
}

ISR(ANA_COMP_vect) {
if bit_is_clear(ACSR, ACO)
	PORTD&=~(1<<4);//LED is ON
	else 	PORTD|=(1<<4);//LED is OFF
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