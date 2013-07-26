/*	
	filename: temperature
	description: This is the firmware of the hardware that simulates temperature on the dipole lattice	
	
	baby steps(TM):
		1. Bare Minimum Tests: Communication Test with the lattice analyser	[done]
		2. Proof of Concept:
			The target of this would be to keep a dipole continously rotating
			a. Test the kind of currents and voltages required to fire up the magnet
			b. Make a program and fire up the magnet from the lattic analyser
	
	communication protocol [clear text]
		fixed length, 10 characters (8 bit each)
		
		xxxxx xxxxx
		\---/ \---/
		  |     |
		  |     +------- Field Intensity
		  +--------------Dipole ID
		for
	Binary Protocol for testing
		fixed length, 10 charactes, 8 bit each
		0 1 2 3 4   5 6 7 8 9 
		x x x x x   x x x x x 
		| | | |     \ - - - / 
		| | | |         | 
 	 	| | \ |         +------------Undefined for now
		\ |  -|---------------------Dipole Intensity (65536 max)
		 -|------------------------Dipole ID (65536 max)
	Binary Protocol V0.2
		fixed length, 4 characters for now
		0 1 2 3 4  
		x x x x x  
		| | | |    
		| | | |    
 	 	| | \ |    
		\ |  -|---------------------Dipole Intensity (65536 max)
		 -|------------------------Dipole ID
		 								|
		 							   / \
								Port 		Bit

*/

#include "usbIO.h"
#include "avriomacros.h"
// #include "temperatureConfig.h"
// #define DEBUG_STAGE_ZERO

// This is to ease programming, reduce codesize
// #define port(x) port##x
// #define bit(x) bit##x
#define fireElectro(port,bit,duration) \
{ \
	out(bit,port);	\ 
	off(bit,port);	\ 
	for(U16Bit j=0;j<duration;j++) \
		_delay_us(1); \
	in(bit,port);	\
	on(bit,port);	\
}

#ifndef DEBUG_STAGE_ZERO
	// U16Bit acknowledge[10]="Ok";
	U16Bit*	dipoleID;
	U16Bit* intensity;
	U8Bit	len;
	U8Bit*	data=0;
#endif

int main(void)
{
	#ifdef DEBUG_STAGE_ZERO
		U8Bit   i;
		U8Bit   ucInDataLen;
		U8Bit   *pucInData = 0;

		usbInit();

		// ---------------- enforce re-enumeration.
		usbDeviceDisconnect();  // enforce re-enumeration, do this while interrupts are disabled!
		i = 255;
		while(--i)					// fake USB disconnect for > 250 ms
			_delay_ms(1);
		usbDeviceConnect();

		sei();
		for(;;)
		{
			usbPoll();
			ucInDataLen = ucGetUSBData( &pucInData );
			if ( ucInDataLen > INBUFFER_LEN )
				ucInDataLen = INBUFFER_LEN;
			if ( ucInDataLen )
			{						// copy indata and send back ascii values incremented by 1
									// also, make the first value, the length of data recieved
				pucOutBuf[0] = ucInDataLen;
				for ( i = 0; i < ucInDataLen; i++ )
					pucOutBuf[i+1] = pucInData[i] + 1;
				vSendUSBData( ucInDataLen+1 );				
			}
		}
		return 0;
	#else
		// for(;;)
		// {
		// 	DDRB |= (1<<5);
		// 	PORTB |= (1<<5);
		// 	_delay_ms(1000);
		// 	PORTB &= ~(1<<5);
		// 	_delay_ms(1000);

		// 	// DDRB=0xff;
		// 	// // DDRC=0xff;
		// 	// // PORTD=0xff;	
		// 	// PORTB=0xff;
		// 	// _delay_ms(1000);
		// 	// // DDRB=0x0;
		// 	// // DDRC=0x0;
		// 	// // PORTD=0x0;			
		// 	// PORTB=0x0;
		// 	// _delay_ms(1000);
		// }
		DDRB=0;
		DDRB &= ~(1<<5);				//Make it high impedence again
		PORTB |= (1<<5);				//

		usbInit();								//Initialize USB		
		usbDeviceDisconnect();					//Disconnect and re-connect
		_delay_ms(255);
		usbDeviceConnect();

		sei();									//Enable Interrupts
		for(;;)
		{
			usbPoll();								//This has to be called frequently enough
			len=ucGetUSBData(&data);				//Get data
			if(len>INBUFFER_LEN)
				len=INBUFFER_LEN;					//truncate if overflew
			if(len)									//Got data?
			{				
				dipoleID=(U16Bit*)&data[0];			//point the dipoleID to its location in the data 
													//(according to the binary protocol)
				intensity=(U16Bit*)&data[2];		//point the intensity pointer to its location

				if(*dipoleID==256)					//if the id is for the zeroth dipole (testing)
				{
					pucOutBuf[0]=(U8Bit) (*intensity);
					// pucOutBuf[0]=(U8Bit) (*(intensity[0]))
					vSendUSBData(1);				//Acknowledge receiving data

					//TODO: ADD PORT CODE			//Fire up the magnet for a time proportional to the intensity
					
					DDRB |= (1<<5);					//Define as output					
					PORTB &= ~(1<<5);				//Make port D.4 low, to fire the magnet
					for(U16Bit j=0;j<(*intensity);j++)
						_delay_us(1);
					DDRB &= ~(1<<5);				//Make it high impedence again
					PORTB |= (1<<5);				//

					// _delay_us((U8Bit) (*intensity/2) );
					// _delay_us((U8Bit) (*intensity/2) );
					//TODO: ADD CODE TO TURN OFF					
				}
				else
				{
					char pin=data[1];

					pucOutBuf[0]=':';
					pucOutBuf[1]='(';

					// if(data[0]=='A')
						// fireElectro(A,pin,*intensity)
					if(data[0]=='B')
					{
						fireElectro(B,pin,*intensity)
					}
					else if(data[0]=='C')
					{	
						fireElectro(C,pin,*intensity)
						pucOutBuf[0]='C';
						pucOutBuf[1]=data[1];
					}
					else if(data[0]=='D')
						fireElectro(D,pin,*intensity)
					// don't even think of missing the brackets, the macro is a set of statements!

					vSendUSBData(2);				//Acknowledge receiving data

				}
			}
		}
		return 0;
	#endif


}