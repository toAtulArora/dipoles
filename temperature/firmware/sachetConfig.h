#ifndef sachetConfig
	#define sachetConfig
	
	struct sachetProtocol
	{
		char startSeq[seqSize+1]="ART";
		char endSeq[seqSize+1]="SOP";
		char ackSeq[seqSize+1]="CKW";		
	};

	// sachetSize;			//Number of bits in one packet
	// seqSize;				//Length of start and end sequence in number of bits (recommended >2)
	#define seqSize 3
	#define sachetSize 10

#endif