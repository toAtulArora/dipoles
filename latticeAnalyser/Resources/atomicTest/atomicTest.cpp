#include <iostream>
#include <atomic>
using namespace std;

atomic<bool> threadsEnabled;

int main()
{
	threadsEnabled=true;
	cout<<"Yey! I just declared an atomic bool";
	return 0;
}