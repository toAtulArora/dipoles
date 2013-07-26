
int bam(char* data, int length);

int bam(char* data,int length)
{
	static int trace=0;
	int i;
	
	trace++;
	printf("[%d]",trace);
	for(i=0;i<length;i++)
	{
		printf("%c",data[i]);
	}
	printf("\n");
	return 10;
}
int boom(char** data);

int boom(char** data)
{
	static char testString[100]="ARTThis was sent as saches to your system SOP junk won't show ";
	static int start=0;
	*data=testString+start;
	start+=10;
	return 10;
}

int busted(char** data);

int busted(char** data)
{
	static char testString[100]="ARTCongratulations if you can read this!SOP I take that back.. ";
	static int start=0;
	*data=testString+start;
	start+=10;
	return 10;

}

