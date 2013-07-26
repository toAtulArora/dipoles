/*==============================================================================
==
== FILE NAME:		DataTypes.h 
==
== DESCRIPTION:	Global defines and typedefs for Data types 
==  
== DATE				NAME					REASON
== ----				----					------
==	06Jul2005		Ravi Mehrotra		Original
==
==============================================================================*/

#ifndef __DataTypes_h__
#define __DataTypes_h__



typedef unsigned char	U8Bit;
typedef char				S8Bit;
typedef unsigned short	U16Bit;
typedef signed short		S16Bit;
typedef unsigned int		U32Bit;
typedef signed int		S32Bit;
typedef float				FLOAT4;
typedef double				FLOAT8;

typedef	unsigned char	BOOLEAN;

typedef struct SUSBdevice 
{
	S8Bit 	acVendor[32];
	S8Bit 	acProduct[32];
	S8Bit 	acSerialNum[32];
	S16Bit 	sVersion;
} susb;

struct SUSBdevice	stUSBdevice;

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

#endif // __DataTypes_h__
