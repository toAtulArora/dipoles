/*==============================================================================
==
== FILE NAME:		DataTypesAVR.h 
==
== DESCRIPTION:	Global defines and typedefs for Data types for AVR
==  
== DATE				NAME					REASON
== ----				----					------
==	22Jan2009		Ravi Mehrotra		Original
==
==============================================================================*/

#ifndef __DataTypesAVR_h__
#define __DataTypesAVR_h__

typedef unsigned char	BOOLEAN;
typedef unsigned char	U8Bit;
typedef char				S8Bit;
typedef unsigned int		U16Bit;
typedef signed int		S16Bit;
typedef unsigned long	U32Bit;
typedef long				S32Bit;
typedef float				FLOAT4;
typedef double				FLOAT8;

#define Z_TRUE				1
#define Z_FALSE			0

#define Z_SUCCESS			1
#define Z_FAILURE			0

#define Z_ON				1
#define Z_OFF				0

#define Z_ACTIVE			1
#define Z_INACTIVE		0

#define Z_CHANGE  		1
#define Z_NO_CHANGE  	0

#define Z_YES           1
#define Z_NO            0

#endif // __DataTypesAVR_h__
