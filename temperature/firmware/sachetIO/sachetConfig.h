#ifndef sachetConfig
	#define sachetConfig

	// sachetSize;			//Number of bits in one packet
	// seqSize;				//Length of start and end sequence in number of bits (recommended >2)
	#define seqSize 3
	#define sachetSize 10
	#define inputBufferLen 1000

	struct structSachetProtocol
	{
		char startSeq[seqSize+1];
		char endSeq[seqSize+1];
		char ackSeq[seqSize+1];		
	} sachetProtocol = { "ART", "SOP", "CKW"};

#endif