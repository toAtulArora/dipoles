/*==============================================================================
== FILE :
==		API for USB communication 
================================================================================
==
== FILE NAME :
==    usbIO.c
==
== Description :
==    API for V-USB communication between PC and an atmega muC.
==
== DATE         NAME              REASON
== ----         ----              ------
== 01Aug2010	Ravi Mehrotra		Bytes written Linux(strlen), WINdoze(REPORT_LEN)
== 13Jun2009   Ravi Mehrotra     prototype of nWriteUSB changed, see Changelog
== 05May2009	Ravi Mehrotra		Original
==
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usbIO.h"


static usbDevice_t *dev = NULL;

/*******************************************************************************
** FUNCTION :
**       Initialize the USB Device
********************************************************************************
**
** Description :
**       Initializes the USB device connected to a USB port on the PC 
**			copied (almost) from obdev's function openDevice(). Hence the deviation
**			from naming conventions.
*******************************************************************************/
void vInitUSB()
{
	U8Bit		rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
	S8Bit		vendorName[] = {USB_CFG_VENDOR_NAME, 0}, productName[] = {USB_CFG_DEVICE_NAME, 0};
	S32Bit	vid = rawVid[0] + 256 * rawVid[1];
	S32Bit	pid = rawPid[0] + 256 * rawPid[1];
	S32Bit	nErr;

	if ( (nErr = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, 0)) != 0 )
		fprintf(stderr, "Error finding %s: %s\n", productName, usbErrorMessage(nErr));
}

/*******************************************************************************
** FUNCTION :
**       Read from the USB Device
********************************************************************************
**
** Description :
**       Reads data from the USB device in a string. The number of bytes read
**       is returned. The pointer to the string read is also returned. User
**			must allocate enough space (max. 254 bytes read in a call) for the
**			string to be read in the calling program.
*******************************************************************************/
S32Bit nReadUSB( U8Bit *paucStr )
{
	S32Bit 	nErr;
	S32Bit 	nBytes = REPORT_LEN;		// should be initialised to REPORT_LEN
	S8Bit		acStr[REPORT_LEN];

	memset( (void *)acStr, 0, sizeof(acStr) );
	nErr = usbhidGetReport( dev, 0, acStr, &nBytes );
	if ( nErr != 0 )
	{
		nBytes = 0;
		fprintf(stderr, "nError reading data: %s\n", usbErrorMessage(nErr));
	}
	else if ( nBytes > 0 )
	{
		nBytes--;
		memcpy( (void *)paucStr, (void *)(acStr + 1), nBytes );
	}
	else
	{
		nBytes = 0;
	}

	return nBytes;
}

/*******************************************************************************
** FUNCTION :
**       Sends the given string to the USB device
********************************************************************************
**
** Description :
**       Sends the given string pcStr to the USB device and returns the number 
**			of actually written out. Max. no. of bytes sent in a call can be
**			REPORT_LEN - 1.
*******************************************************************************/
S32Bit nWriteUSB( U8Bit *pucStr, U8Bit ucLen )
{
	S32Bit 	nErr;
	S8Bit		acBuffer[REPORT_LEN];
	S32Bit	nBytes = ucLen;

#ifdef WIN32
	nBytes = REPORT_LEN;
#endif

	acBuffer[0] = 0;		// room for report ID (not used)
	memcpy( (void *)(acBuffer + 1), (void *)pucStr, ucLen );
	if ( (nErr = usbhidSetReport( dev, acBuffer, nBytes )) != 0 )
	{
		fprintf(stderr, "Error writing data: %s\n", usbErrorMessage(nErr));
		return 0;
	}

	return nBytes;
}

/*******************************************************************************
** FUNCTION :
**       Close the USB connection
********************************************************************************
**
** Description :
**       Close the USB connection between PC and atmega muC.
**
*******************************************************************************/
void vCloseUSB()
{
	usbhidCloseDevice( dev );
}
/*
#include <usb.h>
void vGetSerialNumber( U8Bit *aucBuffer )
{
	struct usb_device *device = usb_device( (struct usb_dev_handle *)dev );
	usb_dev_handle *handle = (struct usb_dev_handle *)dev;
	char buf[128];
	int	len = 0;

	memset( buf, 0, 128 );
	len = usb_get_string_simple( handle, device->descriptor.iManufacturer, (S8Bit *)buf, sizeof(buf));
	fprintf(stderr, "my func Manufacturer iof len %d ->%s<-\n", len, buf);

	memset( buf, 0, 128 );
	len = usb_get_string_simple( handle, device->descriptor.iProduct, (S8Bit *)buf, sizeof(buf));
	fprintf(stderr, "my func Product of len %d ->%s<-\n", len, buf);

	memset( buf, 0, 128 );
	len = usb_get_string_simple( handle, device->descriptor.iSerialNumber, (S8Bit *)buf, sizeof(buf));
	fprintf(stderr, "my func Serial Num of len %d ->%s<-\n", len, buf);

	fprintf(stderr, "my func version ->%X<-\n", device->descriptor.bcdDevice);
}
*/
