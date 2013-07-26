/*
	filename: sachetIO.h
	description: Header for breaking down a large chunk of data into regular sachets for transfer over communication channels.
	language: C
	For use with C++, use 
		extern "C" {#include "sachetIO.h"} 
	in your C++ program. C was chosen to provide compatibility with microcontrollers that don't support C++.

	The protocol is simplistic and assumes all data consistency is done during the transmission and reception of the sachets.
	There's a start seq and an end seq and data in between. 
	The library provides methods to encode, viz convert data to sachets and decode, viz combine sachets to recover data

*/

#ifndef sachetIO
	#define sachetIO

	#define TRANSMIT
	#define RECIEVE
	
	#include "sachetConfig.h"
	//#include "sachetTest.h"
	#include <stdlib.h>
	#include <stdarg.h>
	#include <string.h>	

	// Common function(s)
	// void sachetRelease(char** sachets, int sachetLen);	//Simply deletes a 2D array

	#ifdef TRANSMIT
	//For the sender
		
		//Double buffering hasn't yet beein implemented for sending data
		// struct sachetT
		// {
			// bool i;
			// char* buf[2];
			// int len[2];

			// int (*sendData)( unsigned char* data, int len);	
			//pointer to a function you use to send data
			//eg. nWriteUSB
		// };
		
		//Returns the number of sachets to be transmitted
		//data - pointer to the data to be encoded into sachets
		//len  - length of the data array
		//sachets - pointer to an array of strings, each string of fixed length
		int sachetEncode(char* data, int len, char** sachets);

		//This is the highest level function. Just feed in the data and it's length
		//returns 0 for success
		//the last argument is the function that you use for sending changes
		//it should be of the form 
		//int send(unsigned char* data, int len)
		//NOTE: This will automatically create and release the sachets
		int sachetSend(char* data, int len, int (*send)( char*,int) );
	#endif

	#ifdef RECIEVE	
	//For the reciever
		////////NOT IMPLIMENTED YET
		struct structSachetR
		{
			int i;
			char* buf[2];
			int len[2];
		} sachetR={0,{NULL,NULL},{0,0}};
		////////////////////////////
		
		//Directly read (your recieve function should invoke poll if required)

		//data 			- The first value is address to the pointer which will hold the data
		//(*recieve) 	- is the function that reads the data 
		//timeout 		- the time delay would be the time it takes to get a value from the recieve function
		int sachetRecieve(char** data,int (*recieve)(char**),int timeout);
		//NOTE: Data must be freed by the user (This is not buffered)


		//NOT IMPLIMENTED YET
		//Bufferred Read (double buffered, while data is read into one, the other is made available)
		//Does a single iteration with the given recieve function
		//useful and can be used with poll
		int sachetRecieveIterate(char** data, int (*recieve)(char**));
		//invoked by sachetRecieveIterate
		int sachetResetBuffer(int i);
		///////////////////////////////

		//returns the data length else zero if data recieved is incomplete
		int sachetAppend(char* newData, char** appendedData, int* sizeAppendedData);
		//this is invoked by the sachetAppend function to test if all the data has been recieved
		int sachetDecode(char* data, int size);
	#endif

	//comment
#endif