//***********************************************************************************
//
// avriomacros.h	(rev 1.1)
//
// Author:	Jeremy Greenwood
//
// Description:
//			Includes basic macro functions for input and output tasks. Intended for
//		use with AVR microcontrollers compiled with GCC.
//
//		To use this in a project:
//												
//		A)	1. Add this file to project	  or	B)	1.	Add this file to avr/include
//			2. #include "avriomacros.h"				2.	#include <avriomacros.h>
//
//***********************************************************************************
// I couldn't find my own file where I'd done this, and this guy came to save some time! :)
// Revision comment: The in command was wrong!!! Actually had to spend more time figuring what was going wrong
// than would've to have written the whole thing! But not guarenteed without errors..
///////////////////////////////////

 /*Macro function to declare an output pin */
#define out(bit,port)	DDR##port |= (1 << bit)

/* Macro function to declare an input pin */
#define in(bit,port)	DDR##port &= ~(1 << bit)

/* Macro function to set an output pin high */
#define on(bit,port)	PORT##port |= (1 << bit)

/* Macro function to set an output pin low */
#define off(bit,port)	(PORT##port &= ~(1 << bit))

 /*Macro function to toggle an output pin */
#define flip(bit,port)	PORT##port ^= (1 << bit)

/* Macro function to set internal pullup resistor of input pin (same as on macro)*/
#define pullup(bit,port)	PORT##port |= (1 << bit)	// helps distinguish whats being done without comments

/* Macro function to get state of input pin */
#define get(bit,port)	(PIN##port & (1 << bit))

// /* Macro function to declare an output pin */
// #define out(x)			_out(x)
// #define _out(bit,port)	DDR##port |= (1 << bit)

// /* Macro function to declare an input pin */
// #define in(x)			_in(x)
// #define _in(bit,port)	DDR##port &= ~(1 << bit)

// /* Macro function to set an output pin high */
// #define on(x)			_on(x)
// #define _on(bit,port)	PORT##port |= (1 << bit)

// /* Macro function to set an output pin low */
// #define off(x)			_off(x)
// #define _off(bit,port)	PORT##port &= ~(1 << bit)

// /* Macro function to toggle an output pin */
// #define flip(x)			_flip(x)
// #define _flip(bit,port)	PORT##port ^= (1 << bit)

// /* Macro function to set internal pullup resistor of input pin (same as "on" macro)*/
// // #define pullup(x)		_on(x)

// /* Macro function to get state of input pin */
// #define get(x)			_get(x)
// #define _get(bit,port)	(PIN##port & (1 << bit))
