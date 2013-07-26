/*==============================================================================
== FILENAME : main.c
==    USB communication with Atmega - example main driver
================================================================================
==
== FILE NAME :
==    main.c
==
== Description :
==    Adapted from obdev, this is firmware for Atmega. Bi-directional data
==    communication is implemented. The names of the functions are as per the
==    obdev driver and should not be changed. The functions are called
==    automatically by the USB driver via usbPoll(). The only functions to
==    be directly called by user are usbInit() and usbPoll(). For read/write
==    data, the functions are ucGetUSBData() and vSendUSBData(), respectively.
==    See readme.txt for details.
==
==    A call to ucGetUSBData( U8Bit **pucData) returns the byte length of data
==    available with pucData pointing to the data buffer. Data must be copied/
==    consummed(read) by the user otherwise the buffer is overwritten whenever
==    data is received again by the muC.
==
==    Data to be sent out should be put into an array pointer pucOutData
==    vSendUSBData( ucNumBytes ) shoud be called to send data out. The data is
==    actually transmitted at the next call to usbPoll();
==
==		This test program reads a string sent by the host (PC) and echoes back
==		the length of string read (1 byte) and the the characters in the string
==		with ascii values incremented by unity.
==
==    On PC end the functions usbhidSetReport should get a first byte as a dummy
==    reportID byte which is discarded before transmission to muC. Function
==    usbhidGetReport adds a dummy reportID as first byte to data string recd.
==    from muC which does not contain any such ID byte. This has to be discarded.
==    These dummy report IDs are handled in NPL's wrapper functions nReadUSB and
==    nWriteUSB. The muC does not have to deal with any dummy reportID bytes.
==
== DATE         NAME              REASON
== ----         ----              ------
== 13Jun2009	Ravi Mehrotra		Added comment regarding test program above
== 01May2009	Ravi Mehrotra     Documentation and notes corrected
== 22Jan2009   Ravi Mehrotra     Original
==
==============================================================================*/

#include "usbIO.h"

/*******************************************************************************
** FUNCTION :
		Initialize usb and poll usb.
**
*******************************************************************************/
int main(void)
{
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
			pucOutBuf[0] = ucInDataLen;
			for ( i = 0; i < ucInDataLen; i++ )
				pucOutBuf[i+1] = pucInData[i] + 1;
			vSendUSBData( ucInDataLen+1 );
		}
	}
	return 0;
}
