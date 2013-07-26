/*==============================================================================
== FUNCTION :
==    Header file for USB hardware configuration for AVR microcontroller
================================================================================
==
== FILE NAME :
==    customConfig.h
==
== Description :
==		Define maximum in and out data buffer lengths (REPORT_LEN) and device
==		description. The description mast match the description in the firmware.
==		USB_CFG_SERIAL_NUMBER and USB_CFG_SERIAL_NUMBER_LEN are optional. The PC
==		end customConfig.h file	does not	require the --Hardware Config-- section.
==
==	NOTE :
==		Only half the Serial Number string is OK, the latter half is transmitted
==		as junk !!! Reason unknown.
==
== DATE         NAME              REASON
== ----         ----              ------
== 01Aug2010	Ravi Mehrotra		Added USB_CFG_SERIAL_NUMBER and version facility
== 07May2009	Ravi Mehrotra		Original
==
==============================================================================*/
#ifndef __customConfig_h_included__
#define __customConfig_h_included__

/* ---------------------------- Memory and Data buffers -------------------*/

#define REPORT_LEN	128		// indata and outdata maximum length 

#define INBUFFER_LEN		128	// The buffer lengths can be unequal and less 
#define OUTBUFFER_LEN	128	//   than REPORT_LEN

/* --------------- These defs over ride those in usbconfig.h --------------- */
/* ---------------------------- Hardware Config ---------------------------- */

#define  F_CPU 12000000UL

#define USB_CFG_IOPORTNAME    B     // port use for USB
#define USB_CFG_DMINUS_BIT    0     // USB D- line must be 0
#define USB_CFG_DPLUS_BIT     1     // USB D+ line must also be connected to INT0

/* --------------------------- Functional Range ---------------------------- */

#define USB_CFG_IS_SELF_POWERED  0     // 1 if device has own power supply
#define USB_CFG_MAX_BUS_POWER    100    // in mA

/* -------------------------- Device Description --------------------------- */

#define USB_CFG_VENDOR_NAME		'N', 'P', 'L'
#define USB_CFG_VENDOR_NAME_LEN	3
#define USB_CFG_DEVICE_NAME		't', 'e', 'm','p','e','r','a','t','u','r','e'
#define USB_CFG_DEVICE_NAME_LEN  11

#define USB_CFG_VENDOR_ID       0xc0, 0x16
#define USB_CFG_DEVICE_ID       0xdf, 0x05 
#define USB_CFG_DEVICE_VERSION  0x0a, 0x0b

#define USB_CFG_SERIAL_NUMBER   'v', 'e', 'r', 's', 'i', 'o' , 'n', ' ', '0', '.' , '1'
#define USB_CFG_SERIAL_NUMBER_LEN   12

#endif // __customConfig_h_included__ 
