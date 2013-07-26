/*==============================================================================
== FILE :
==    Program to test USB communication with OBDEV drivers
================================================================================
==
== FILE NAME :
==    main.c
==
== Description :
==    This is a test program to for USB communication between the PC and an
==    Atmega8 based device. A string sent to the atmega device is echoed
==    back with ascii values incremented by unity. A byte equal to no. of chars
==		read by the muC is prefixed to the string recd. Thus the byte count recd
==		is one greater than the bytes sent to the muC.
==
== DATE         NAME              REASON
== ----         ----              ------
== 13Jun2009   Ravi Mehrotra     Added test description to file header above
== 07May2009   Ravi Mehrotra     Original
==
==============================================================================*/

#include "DataTypes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "usbIO.h"

#ifdef WIN32
#include "windows.h"
#define sleep(t) Sleep((t)*1000)
#define usleep(t) Sleep((t)/1000)
#endif

S32Bit main(S32Bit argc, S8Bit **argv)
{
	U8Bit    aucBuffer[REPORT_LEN];    /* has room for dummy report ID */
	S32Bit	nLen;
	S32Bit	i, j,k;

	vInitUSB();

	memset( aucBuffer, 0, REPORT_LEN );
	fprintf( stderr, "Vendor  ->%s<-\n", stUSBdevice.acVendor );
	fprintf( stderr, "Product ->%s<-\n", stUSBdevice.acProduct );
	fprintf( stderr, "Serial  ->%s<-\n", stUSBdevice.acSerialNum );
	fprintf( stderr, "Version ->%0X.4x<-\n", stUSBdevice.sVersion );

	for ( i=0; i< strlen(stUSBdevice.acSerialNum); i++ )
		fprintf(stderr, "%d\n", stUSBdevice.acSerialNum[i] );

	for ( i = 0; i < 10; i++ )
	{
		//-------------- write data
		memset( aucBuffer, 0, REPORT_LEN );
		for ( j=0; j<14; j++ )
			aucBuffer[j] = 126+j;
		if ( (nLen = nWriteUSB( (U8Bit *)aucBuffer, 14 )) )
			fprintf( stderr, "================================\n");
			fprintf( stderr, "Wrote %d bytes\n", nLen );
		
		// ---------------- read data
		memset( aucBuffer, 0, REPORT_LEN );
		nLen = nReadUSB( aucBuffer );
		if ( nLen == 0 )
		{
			fprintf(stderr, "error reading data: \n" );
		}
		else
		{
			fprintf( stderr, "Read %d bytes: ", aucBuffer[0] );
			for ( j=0; j<aucBuffer[0]; j++ )
				fprintf( stderr, "%4d", aucBuffer[j+1] );
			fprintf( stderr, "\n" );
		}
		sleep(1);
	}
	vCloseUSB();
	return 0;
}
