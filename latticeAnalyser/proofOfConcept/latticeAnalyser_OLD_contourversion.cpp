/*
  filename: latticeAnalyser.cpp
  description: This is the main application which analyses the lattice and 
                  1. controls `temperature'
                  2. records dipole position (thus angular velocity) as a function of time
  baby steps (TM):
    1. Proof of concept stage
        a. Find a suitable algorithm
        b. Make the required modifications
            i. Add a colour filter
              Implement two colours [done]
              Add two sliders for adjusting tolerance [done, but need to refresh something!]
              Add GUI for selecting the colours [done, but not a gui so to speak]
              save settings [not doing it]
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src; Mat src_gray; Mat srcColorFilter; Mat src_process; Mat srcColorA; Mat srcColorB;
Mat_<Vec3b> srcTemp = src;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

// Colour read
Point origin;

// This is a colour filter for improving accuracy
  // 20, 28, 41 [dark]
  // TODO: Allow the user to select the colour
  Scalar colorB=Scalar(41,28,20);
  Scalar colorA=Scalar(145,128,115);
  int colorATol=10;
  int colorBTol=10;


static void onMouse( int event, int x, int y, int, void* )
{
    switch( event )
    {
    case CV_EVENT_LBUTTONUP:
        colorA=Scalar(src.at<Vec3b>(x,y)[0],src.at<Vec3b>(x,y)[1],src.at<Vec3b>(x,y)[2]);
        cout<<"Color A's been changed to "<<endl<<colorA.val[0]<<endl<<colorA.val[1]<<endl<<colorA.val[2]<<endl;
        // displayOverlay("Source","haha",2000);
        cout<<colorA.val;
        break;
    case CV_EVENT_RBUTTONUP:
        colorB=Scalar(src.at<Vec3b>(x,y)[0],src.at<Vec3b>(x,y)[1],src.at<Vec3b>(x,y)[2]);
        cout<<"Color B's been changed to "<<endl<<colorB.val[0]<<endl<<colorB.val[1]<<endl<<colorB.val[2]<<endl;
        break;
    }
}



/// Function header
void thresh_callback(int, void* );

/**
 * @function main
 */
int main( int, char** argv )
{
  /// Load source image and convert it to gray
  src = imread( argv[1], 1 );
  
  Scalar lowerBound;
  Scalar upperBound;

  lowerBound = colorA-Scalar::all(colorATol);
  upperBound = colorA+Scalar::all(colorATol);
  // Now we want a mask for the these ranges
  inRange(src,lowerBound,upperBound, srcColorA);

  lowerBound = colorB-Scalar::all(colorBTol);
  upperBound = colorB+Scalar::all(colorBTol);  
  inRange(src,lowerBound,upperBound, srcColorB);

  addWeighted(srcColorA, 1, srcColorB, 1, 0, srcColorFilter);
  
  //You may not it here, like so
  // srcColorFilter=Mat(srcColorFilter.rows, srcColorFilter.cols, srcColorFilter.type(), Scalar(255,255,255))-srcColorFilter;
  // srcColorA=Mat(srcColorA.rows, srcColorA.cols, srcColorA.type(), Scalar(255,255,255))-srcColorA;

  // not(srcColorFilter,srcColorFilter);

  /// Convert image to gray and blur it
  cvtColor( src, src_process, COLOR_BGR2GRAY );
  // cvtColor( src, src_gray, COLOR_BGR2GRAY );
  // multiply(src_process,srcColorFilter,src_gray,1);
  // src_gray=srcColorFilter.mul(src_process/255);
  src_gray=srcColorFilter;

  blur( src_gray, src_gray, Size(3,3) );

  /// Create Window
  const char* source_window = "Source";
  namedWindow( source_window, WINDOW_AUTOSIZE );
  setMouseCallback( "Source", onMouse, 0 );
  imshow( source_window, src );


  //Show the filtered image too
  const char* filter_window = "Color Filter";
  namedWindow( filter_window, WINDOW_AUTOSIZE );
  imshow( filter_window, src_gray);


  createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
  
  //Show the settings window
  const char* settings_window="Settings";
  namedWindow(settings_window,WINDOW_AUTOSIZE  | CV_GUI_NORMAL);
  createTrackbar( "ColorA Tolerance", settings_window, &colorATol, 256, 0 );
  createTrackbar( "ColorB Tolerance", settings_window, &colorBTol, 256, 0 );

  thresh_callback( 0, 0 );

  waitKey(0);
  return(0);
}

/**
 * @function thresh_callback
 */
void thresh_callback(int, void* )
{
  

  Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using Threshold
  threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
  /// Find contours
  findContours( threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Find the rotated rectangles and ellipses for each contour
  vector<RotatedRect> minRect( contours.size() );
  vector<RotatedRect> minEllipse( contours.size() );

  for( size_t i = 0; i < contours.size(); i++ )
     { minRect[i] = minAreaRect( Mat(contours[i]) );
       if( contours[i].size() > 5 )
         { minEllipse[i] = fitEllipse( Mat(contours[i]) ); }
     }

  /// Draw contours + rotated rects + ellipses
  Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  for( size_t i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       // contour
       drawContours( drawing, contours, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       // ellipse
       // ellipse( drawing, minEllipse[i], color, 2, 8 );

       // rotated rectangle
       // Point2f rect_points[4]; minRect[i].points( rect_points );
       // for( int j = 0; j < 4; j++ )
       //    line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
     }

  /// Show in a window
  namedWindow( "Contours", WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
}
