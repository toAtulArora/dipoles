/*==============================================================================
== FILENAME :
==		USB communication between Atmega muC and PC
================================================================================
==
== FILE NAME :
==    readme.txt
==
== Description :
==    These are details for setting up USB data communication between an Atmega
==		microcontroller and a PC. Ths is based on OBDEV's old PowerSwitch package.
==		These are instructions for the PC end software on Linux. For the ATmega
==		firmware	see the directory ../firmware
==
==	Note :
==		Files hidsdi.h is unedited from OBDEV vusb package. Files hiddata.h and
==		hiddata.c are minimally edited (for function usbErrorMessage which is
==		include in NPL's version. It is taken from vusb's hidtool.c file). All
==		other files have been written at NPL.
==
==		The driver functions usbhidSetReport should get a first byte as a dummy
==		reportID byte which is discarded before transmission to muC. Function
==		usbhidGetReport adds a dummy reportID as first byte to data string recd.
==		from muC which does not contain any such ID byte. This has to be discarded.
==		These dummy report IDs are handled in NPL's wrapper functions nReadUSB and
==		nWriteUSB. The muC does not have to deal with any dummy reportID bytes.
==
==		Windows has stringent requirements for REPORT_LEN. The data sent is always
==		equal to REPORT_LEN. In Linux the actual length of data can be smaller 
==		than REPORT_LEN.
==
==		Hence, on Windows, if actual data length is smaller than REPORT_LEN, some
==		mechanism to send the actual data length along with data is desirable.
==		This can be done by sending data in frames. The receiving end can then
==		decipher the actual data length and ignore extra bytes.
==
== DATE         NAME              REASON
== ----         ----              ------
==	12Jun2010	Ravi Mehrotra		Added note regarding REPORT_LEN in Windows
== 13Jun2009	Ravi Mehrotra		Edited above file header
== 07May2009	Ravi Mehrotra		Original
==
==============================================================================*/

NPL PC end program for USB communication with Atmega controllers.

This package an example program to set up the PC end software for data
communication with Atmega controllers over USB. I found that  one  has
to spend considerable amount of time in getting the USB  communication
deployed using the obdev package (PowerSwitch). Hence, I prepared this
package which makes life very simple for the user.

Here are the steps to be followed:

1. Edit customConfig.h (the only file which needs to be edited by  the
   user, aside from the top level program  file  containing  the  main
   function) to define vendor and device names. These should match the
   ones  defined  in  the  microcontroller  firmware  for   successful
   communication. Note that the customConfig.h files for firmware and
   PC are not the same.

2. See main.c for an example of the top level driver.

3. There are only four user functions as  given  in  the  header  file
   usbIO.h, namely, vInitUSB(),  vCloseUSB,  nWriteUSB(),  nReadUSB().
   The names are self explanatory.

Enjoy USB communication !!

Ravi Mehrotra
CCF, NPL
07May2009
