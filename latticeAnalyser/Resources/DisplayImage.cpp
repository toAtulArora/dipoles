#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// #include <cv.h>
// #include <highgui.h>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	if(argc!=2)
	{
		cout<<" Usage: DisplayImage.c ImageToLoadAndDisplay"<<endl;
		return -1;
	}
	Mat image;
	image=imread(argv[1],CV_LOAD_IMAGE_COLOR);	// REad the file

	if(! image.data)
	{
		cout<<"Couldn't open the file or find it"<<endl;
		return -1;
	}

	namedWindow( "Display window", CV_WINDOW_AUTOSIZE);	//Create a window for display
	imshow("Display window",image);	//Put the image in it

	waitKey(0);
	return 0;
}