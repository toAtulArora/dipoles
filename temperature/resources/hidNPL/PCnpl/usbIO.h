/*==============================================================================
== FILE :
==		Header file for USB API
================================================================================
==
== FILE NAME :
==    usbIO.h
==
== Description :
==    API for USB communication based on OBDEV drivers and an atmega muC
==
== DATE         NAME              REASON
== ----         ----              ------
== 13Jun2009	Ravi Mehrotra		prototype of nWriteUSB changed, see Changelog
== 26Dec2007	Ravi Mehrotra		Original
==
==============================================================================*/
#ifndef __usbIO_
#define __usbIO_

#include "DataTypes.h"
#include "hiddata.h"
#include "customConfig.h"  // for device VID, PID, vendor name and product name 

extern struct SUSBdevice stUSBDevice;

extern void 	vInitUSB();				// initialize connection between PC and muC
extern void 	vCloseUSB();			// close connection between PC and muC
extern S32Bit 	nReadUSB( U8Bit * );	// read string from muC, return no. bytes read
extern S32Bit 	nWriteUSB( U8Bit *, U8Bit );	// write string of given length
														// to muC, return no. bytes written
extern void 	vGetSerialNumber( U8Bit * );

#endif
