#include <stdio.h>
//#include "sachetIO.h"
#include "sachetTest.h"


int main()
{	
	char SendThese[38]="This should get broken into sachets";
	char* data;
	int i,len;
	len=sachetRecieve(&data,boom,100);
	printf("Data Recieve Test \nData Length:%d \nData Recieved: ",len);
	for(i=0;i<len;i++)
		printf("%c",data[i]);
	printf("\n\nData Transmission Test\n");
	i=sachetSend(SendThese,36,bam);
	//sendData(SendThese,30);
	//printf("This is a test for sachetIO transmission\n");
	printf("\n\nBuffered Input Test");
	for(i=0;i<10;i++)
	{
		len=sachetRecieveIterate(&data,busted);		
		if(len>0)
		{
			printf("Length: %d \nData Recieved: ",len);
			for(i=0;i<len;i++)
			{
				printf("%c",data[i]);
			}
			i=10;
		}
	}

	getchar();
	return 0;
}

