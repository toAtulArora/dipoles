/*==============================================================================
== FILENAME :
==		Header file for Atmega <--> PC USB communication
================================================================================
==
== FILE NAME :
==    usbIO.h
==
== Description :
==    Header file to be included in top level program file containing the main()
==    function. It declares variables and allocates memory for the data buffers.
==
== DATE         NAME              REASON
== ----         ----              ------
== 01May2009	Ravi Mehrotra		Original
==
==============================================================================*/
#ifndef __usbIO_h_included__
#define __usbIO_h_included__

#include "DataTypesAVR.h"
#include "customConfig.h"	// should be included before util/delay.h to
									//   avoid compiler warning
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usbdrv.h"

// --------------- Functions to send and get data

extern void vSendUSBData( U8Bit );				// argument contains num bytes to
															//  be sent, actual data pointer is
															//  pucOutBuf
extern U8Bit ucGetUSBData( U8Bit ** );			// returns num bytes recd. argument
															//   points to data recd.
//---------------------------------------------------------------------------

volatile U8Bit aucOutBuf0[OUTBUFFER_LEN];		// buffer data to be sent to PC
volatile U8Bit aucOutBuf1[OUTBUFFER_LEN];		// buffer data to be sent to PC
volatile U8Bit *pucOutBuf = &aucOutBuf0[0];	// user always puts data in this
volatile U8Bit ucOutLen;							// no. of bytes to be sent to PC

volatile U8Bit aucInBuf[INBUFFER_LEN];			// buffer data recd. from PC

volatile U8Bit *pucReadyOutData;					// pointer to data to be sent out
															//   in a given usbPoll()
volatile U8Bit ucNumCharRead = 0;				// running num bytes transferred
															//   into data in in-buffer
volatile U8Bit ucNumCharWritten = 0;			// running num bytes transferred
															//   out of data in out-buffer

#endif  // __usbIO_h_included__
