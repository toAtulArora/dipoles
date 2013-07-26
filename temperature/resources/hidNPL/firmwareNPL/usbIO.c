/*==============================================================================
== FILENAME : usbIO.c
==    USB communication with Atmega
================================================================================
==
== FILE NAME :
==    usbIO.c
==
== Description :
==    Adapted from obdev, this is firmware for Atmega. Bi-directional data
==    communication is implemented. The names of the functions are as per the
==    obdev driver and should not be changed. The functions are called
==    automatically by the USB driver via usbPoll(). The only functions to
==    be directly called by user are usbInit(), usbPoll(), vSendUSBData(), and
==    ucGetUSBData().
==
== DATE         NAME              REASON
== ----         ----              ------
== 07Feb2009   Ravi Mehrotra     Renamed file to be consistent with usbIO.h
==                               Added function ucGetUSBData()
== 22Jan2009   Ravi Mehrotra     Original
==
==============================================================================*/
#include "DataTypesAVR.h"
#include "customConfig.h"  // should be included before util/delay.h to avoid
									//   compiler warning
#include <avr/io.h>
#include <avr/interrupt.h>  // for sei()
#include <util/delay.h>     // for _delay_ms()

#include <avr/pgmspace.h>   // required by usbdrv.h 
#include "usbdrv.h"

// We use double buffering for out data so that if processing while
// usbPoll is incomplete and more data comes in, it is not lost

extern volatile U8Bit aucOutBuf0[];       // buffer data to be sent to PC
extern volatile U8Bit aucOutBuf1[];       // buffer data to be sent to PC
extern volatile U8Bit *pucOutBuf;         // user always puts data in this
extern volatile U8Bit ucOutLen;           // no. of bytes to be sent to PC

extern volatile U8Bit aucInBuf[];         // buffer data recd. from PC

extern volatile U8Bit *pucReadyOutData;   // pointer to data to be sent out
                                          //   in a given usbPoll()
extern volatile U8Bit ucNumCharRead;      // running num bytes transferred
                                          //   into data in in-buffer
volatile U8Bit ucInLen;

static BOOLEAN bInBusy;

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

PROGMEM char usbHidReportDescriptor[22] = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, REPORT_LEN,              //   REPORT_COUNT from customConfig.h
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};
/* Since we define only one feature report, we don't use report-IDs (which
 * would be the first byte of the report). The entire report consists of 
 * REPORT_LEN opaque data bytes.
 */

/*******************************************************************************
** FUNCTION :
**       Get data written by the USB controller
********************************************************************************
**
** Description :
**       This function reads data sent(written) to the atmega8 by a PC
**       The incoming data is copied from pucData to aucInBuf. The number of
**       bytes to be copied in the call to this function is given in ucLen.
**       The total no. of bytes available for reading is ucInLen.
**
*******************************************************************************/
U8Bit usbFunctionWrite( U8Bit *pucData, U8Bit ucLen )
{
	U8Bit  i;

	if ( ucLen > ucInLen - ucNumCharRead )
		ucLen = ucInLen - ucNumCharRead;
	for ( i = 0; i < ucLen; i++ )
		aucInBuf[ ucNumCharRead++ ] = pucData[i];
	if ( ucNumCharRead < ucInLen )   // not all bytes read into aucInBuf so far
	{
		return 0;
	}
	else
	{
		bInBusy = Z_FALSE;
		return 1;
	}
}

// --------------------------- OBDEV function

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	U8Bit	i;

	if ( (rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS )
	{    // HID class request 
		if(rq->bRequest == USBRQ_HID_GET_REPORT)
		{  // wValue: ReportType (highbyte), ReportID (lowbyte) 
			// since we have only one report type, we can ignore the report-ID
			usbMsgPtr = (U8Bit *)pucReadyOutData;
			i = ucOutLen;
			ucOutLen = 0;
			return i;
		}
		else if(rq->bRequest == USBRQ_HID_SET_REPORT)
		{
			// since we have only one report type, we can ignore the report-ID
			bInBusy = Z_TRUE;
			ucInLen = data[6];
			/*
			if ( ucInLen > INBUFFER_LEN )
				ucInLen = INBUFFER_LEN;
			*/
			ucNumCharRead = 0;
			return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
		}
	}
	else
	{
		// ignore vendor type requests, we don't use any 
	}
	return 0;
}

/*******************************************************************************
** FUNCTION :
**       Send data to PC
********************************************************************************
**
** Description :
**       Data pointed to by pucOutBuf is prepared to be sent to the pC at the
**       next call to usbPoll(). As double buffering is used the pointer
**       pucOutBuf is set to the other out data buffer.
**       ucNumByte is an in argumant containing number of bytes to be sent out
*******************************************************************************/
void vSendUSBData( U8Bit ucNumByte )
{
	pucReadyOutData = pucOutBuf;     // save pointer to data ready to be sent out
	ucOutLen = ucNumByte;            // save number of bytes to be sent out

	// --------- use double buffering for next set of dat to be sent out
	if ( pucOutBuf == aucOutBuf0 )
		pucOutBuf = aucOutBuf1;
	else
		pucOutBuf = aucOutBuf0;
}

/*******************************************************************************
** FUNCTION :
**       Receive data from PC
********************************************************************************
**
** Description :
**       This is just a convenience function for the user. It sets the argument
**       pointer to the data in buffer and returns the no. of bytes read from
**       the PC. Data should be consummed (copied) by the user otherwise in the
**       next call to usbPoll(), the buffer is overwritten.
*******************************************************************************/
U8Bit ucGetUSBData( U8Bit **paucData )
{
	if ( bInBusy == Z_TRUE )
		return 0;

	U8Bit ucDataLen = ucInLen;

	*paucData = (U8Bit *)aucInBuf;
	ucInLen = 0;
	return ucDataLen;
}
