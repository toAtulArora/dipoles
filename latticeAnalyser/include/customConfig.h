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
==		USB_CFG_SERIAL_NUMBER, and USB_CFG_SERIAL_NUMBER_LEN are optional.
==
== NOTE :
==		REPORT_LEN must be larger by unity than the one defined in firmware to
==		allow for the dummy report id.
==
== DATE         NAME              REASON
== ----         ----              ------
==	12Jun2010	Ravi Mehrotra		Fixed REPORT_LEN for Windows
== 07May2009	Ravi Mehrotra		Original
==
==============================================================================*/
#ifndef __customConfig_h_included__
#define __customConfig_h_included__

/* ---------------------------- Memory and Data buffers -------------------*/

#define REPORT_LEN	129		// indata and outdata maximum length 

/* -------------------------- Device Description --------------------------- */

#define USB_CFG_VENDOR_NAME		'N', 'P', 'L'
#define USB_CFG_VENDOR_NAME_LEN	3
#define USB_CFG_DEVICE_NAME		't', 'e', 'm', 'p', 'e', 'r', 'a', 't', 'u', 'r', 'e'
#define USB_CFG_DEVICE_NAME_LEN  11

#define USB_CFG_VENDOR_ID       0xc0, 0x16
#define USB_CFG_DEVICE_ID       0xdf, 0x05 
#define USB_CFG_DEVICE_VERSION  0x00, 0x01

#define USB_CFG_SERIAL_NUMBER   'v', 'e', 'r', 's' , 'i', 'o', 'n',' ', '0', '.', '1'
 //"J", "u", "n", "k", "k"
#define USB_CFG_SERIAL_NUMBER_LEN   10 

#endif // __customConfig_h_included__ 
