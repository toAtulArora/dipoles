/*==============================================================================
== FILENAME :
==		USB communication between Atmega muC and PC
================================================================================
==
== FILE NAME :
==    readme.txt
==
== Description :
==    These are details of how to set up USB data communication between an 
==		Atmega microcontroller and a PC. Ths is based on OBDEV's V-USBpackage 
==		(15Apr2009). These are instructions for the microcontroller end software.
==		For the PC	end code see the directory ../PC. This version of firmware
==		implements a double buffer for outgoing data. 
==
== Note :
==		The files under directory usbdrv are iuntouched OBDEV's original files.
==		File usbconfig.h is minimally edited. Other files have been set up at NPL.
==
== DATE         NAME              REASON
== ----         ----              ------
== 13Jun2009	Ravi Mehrotra		Edited this file header
== 05May2009	Ravi Mehrotra		Original
==
==============================================================================*/

NPL adaptation of obdev's firmware USB driver for atmega controllers

This package encapsulates obdev's firmware based V-USB driver. I found
that one has to spend considerable amount of time in getting  the  USB
communication deployed using the obdev package. Hence, I prepared this
package which makes life very simple for the user.

Here are the steps to be followed:

1. Edit customConfig.h (the only file which needs to be edited by  the
   user, aside from the top level program  file  containing  the  main
   function) to set up the data in and out buffer lengths. Keep  these
   to a minimum if you want to save SRAM space in  the  muC.  Describe
   your USB port and pins and vendor and device names and use  of  USB
   power.

2. See main.c for an example of the top level driver.

3. The names of the functions are as per the obdev driver  and  should
   not be changed. The functions are called automatically by  the  USB
   driver via usbPoll(). The only functions to be directly  called  by
   user are usbInit() and usbPoll(). Optionally, re-enumeration of the
   device  can  be  done  via  functions   usbDeviceDisconnect()   and
   usbDeviceDisconnect() as shown in main.c.

4. The function ucGetUSBData(U8Bit **pucData) returns number of  bytes
   recd. from the PC and available in the buffer. The pointer  pucData
   is set to point to the buffer aucInBuf[].

   WARNING !!!
   The data available must be consummed(read and copied) the data. The
   buffer contents are overwritten at the next call to usbPoll().

5. Data to be sent out should be put into an array pointer  pucOutData
   vSendUSBData( ucNumBytes ) shoud be called to send  data  out.  The
   data is actually transmitted at the next  call  to  usbPoll().  The
   variable pucOutData is already defined.

   Double buffering for data to be  sent  out  is  used.  This  is  to
   prevent new set of data overwriting the last set of  data  just  in
   case it is still being transmitted by usbPoll().

Enjoy USB communication !!

Ravi Mehrotra
CCF, NPL
05May2009
