#include <iostream>

using namespace std;

int main()
{
	int a=1;
	int b=1;
	long c= (long) a | (b<<8);
	cout<<c;
	return 0;
}