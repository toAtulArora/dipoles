/*
+------------------------------------+
|+----------------------------------+|
||									||
||	[][][]		[][]]		[][]	||
||	[]			[]			[] []	||
||	[][]		[][][]		[][] 	||
||	[]		  	    []		[] []	||
||	[]			[][][]		[]  []	||
||	                            	||
||	For 		Simplicity 	Reasons	||
|+----------------------------------+|
+------------------------------------+
[]
[]	File: 		fsr_io.c
[]	Author:		Atul Singh Arora
[]	Version: 	0.1
[]	Desc:		Provides simple controls
[]				for input output operations
[]				without wasting space.
[]
[]
[]
/\/\/\/\/\/\/\/\/\/\/\/\//\/\/\/\/\/\\
*/
#include <avr/io.h>

// Function prototype
int     set_PORTB_bit(int position, int value);

void ON(int Port,int Pin)
{
	if 		(Port==0)	PORTA |= (1 << Pin);       
	else if (Port==1)	PORTB |= (1 << Pin);       
	else if (Port==2)	PORTC |= (1 << Pin);       
	else if (Port==3)	PORTD |= (1 << Pin);       
	
}
void ON(int Port)
{
	if		(Port==0) PORTA = 0xFF;
	else if (Port==1) PORTB = 0xFF;
	else if (Port==2) PORTC = 0xFF;
	else if (Port==3) PORTD = 0xFF;	
}
void OFF(int Port)
{
	if 		(Port==0)	PORTA = 0xFF;
	else if (Port==1)	PORTB = 0xFF;
	else if (Port==2)	PORTC = 0xFF;
	else if (Port==3)	PORTD = 0xFF;
}
void OFF(int Port,int Pin)
{
	if 		(Port==0)	PORTA &= ~(1 << Pin);       
	else if (Port==1)	PORTB &= ~(1 << Pin);       
	else if (Port==2)	PORTC &= ~(1 << Pin);       
	else if (Port==3)	PORTD &= ~(1 << Pin);       
}
