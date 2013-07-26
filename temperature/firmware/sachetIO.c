#include "sachetIO.h"

void sachetRelease(char** sachets, int sachetLen)
{
	int i;
	for (i = 0; i < sachetLen; ++i) 
	{
	    free(sachets[i]);
	}
	free(sachets);	
}

#ifdef RECIEVE
	int sachetRecieve(char** data, int (*recieve)(char**),int timeout)
	{
		char finalDataLength=0,dataLength=0;
		free(*data);		//free up whatever data was already present
		*data=0;			//null pointer

		while(timeout--)
		{
			if((*recieve)(&buff)>0)
			{
				finalDataLength = sachetAppend(buff,data,&dataLength);
				if(dataLength>0)
				{
					//multiplication by char is not required
					char* finalData=malloc((finalDataLength*sizeof(char)) + 1); //make the final data as long as it was found out to be
					memcpy(finalData,(*data+seqSize),finalDataLength);			//extract data from the appended data to the final data
					free(*data);												//release memory occupied by the appended data
					*data=finalData;											//Update the data pointer
					return finalDataLength;										//Return the length of the data
					// cout<<"Got the data"<<data;
				}	
			}
		}

		//this contains the errorflag in which case
		return finalDataLength;
		//timed out :(
	}

	//returns the length if data extracted
	//else
	//	-1	:   data allocation failed
	//		:	data collected so far doesn't have the start and end sequence (for error messages refer to sachetDecode)
	int sachetAppend(char* newData, char** appendedData,int* sizeAppendedData)
	{
		int sizeData=*sizeAppendedData;					//used later; starting point of the new data in the appended data
		int newSize=sizeData+sachetSize;				//used later
		*sizeAppendedData+=newSize;						//Update this value for the function that called it
		*appendedData=realloc(appendedData,newSize);	//One more sachet of data has been recieved,
		//so make space for it in the memory
		//exit if it fails
		if(*appendedData==NULL) return -1;
		
		//append the data now
		int i;
		for(i=sizeData-1,k=0;k<sachetSize;i++,k++)
			*appendedData[i]=newData[k];

		return sachetDecode(*appendedData,*sizeAppendedData);
	}

	// -2 : Start sequence missing
	// -3 : End Sequence missing
	int sachetDecode(char* data, int size)
	{
		int i;
		for(i=0;i<seqSize;i++)
		{
			if(data[i]!=startSeq[i])
				return -2;
		}

		char* loc=strstr(data,endSeq);	//location of start of endSequence
		if(loc==NULL)
			return -3;

		int length=(loc - (data+seqSize)) //+ 1;	//find the length from the difference. Note 1 has been removed because
		//loc points to the first character of the end sequence
		return length;
	}

#endif

#ifdef TRANSMIT
	int sachetSend(char* data, int len, int (*send)(char*,int))
	{
		// va_list=pointers;	//pointers to the unknown arguments
		// va_start(len,pointers);	//gets the pointer to the first argument
		// va_arg(pointers,0);	//gets the first pointer

		char** sachets;

		int i,sachetCount=sachetEncode(data, len, sachets);

		for(i=0;i<sachetCount;i++)
		{
			if((*send)( (char *) sachets[i],sachetSize)==0)
			{
				sachetRelease(sachets,sachetCount);
				//release memory before quitting
				return 1;	//error in sending
			}				
		}
		sachetRelease(sachets,sachetCount);
		return 0;			
	}

	int sachetEncode(char* data, int len, char** sachets)
	{		
		int i;
		int effLen=len+(2*seqSize);	//Add space for the start and stop sequences
		int sachetCount=(effLen / sachetSize) + 1; //simple math, division :)

///////////////////////////
		//PART 1
		//Allocate memory
		//Make an array of pointers to strings, of size
		//sachetCount times the size of one pointer
		sachets = (char**)malloc(sachetCount*sizeof(char*));

		if(sachets==0) return 0;
		//Now for each string pointer, create a fixed sized string
		for(i=0;i<sachetCount;i++)
		{
			//sizeof(char) is rather redundant, there for clarity
			sachets[i]=(char*)malloc(seqSize*sizeof(char));
			
			if(sachets[i]==0) return 0;
		}

///////////////////////////
		//PART 2
		//Break the string into pieces to send
		
		//Append the start seq
		for(i=0;i<seqSize;i++)
		{
			sachets[i / sachetSize][i % sachetSize]=sachetProtocol.startSeq[i];
		}

		//Now append the data
		for(i=0;i<len;i++)
		{
			int iP = i+seqSize;

			sachets[iP / sachetSize][iP % sachetSize]=data[i];
		}

		//Now append the end seq
		for(i=0;i<seqSize;i++)
		{
			int iP = i + (seqSize+len);

			sachets[iP / sachetSize][iP % sachetSize]=sachetProtocol.endSeq[i];
		}

		return sachetCount;
	}
#endif