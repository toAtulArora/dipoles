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
Mat cimg;
Mat_<Vec3b> srcTemp = src;
int thresh = 100;
int max_thresh = 255;

int minRadius=1, maxRadius=30;
RNG rng(12345);

// Colour read
Point origin;

// This is a colour filter for improving accuracy
  // 20, 28, 41 [dark]
  // TODO: Allow the user to select the colour
  Scalar colorB=Scalar(245,245,10);
  Scalar colorA=Scalar(10,245,245);
  int colorATol=30;
  int colorBTol=30;
//
  const char* source_window = "Source";
  const char* filter_window = "Color Filter";
  const char* settings_window="Settings";

static void onMouse( int event, int x, int y, int, void* )
{
    switch( event )
    {        
    case CV_EVENT_LBUTTONUP:
        cout<<x<<","<<y<<endl;
        colorA=Scalar(src.at<Vec3b>(x,y)[0],src.at<Vec3b>(x,y)[1],src.at<Vec3b>(x,y)[2]);        
        cout<<"Color A's been changed to "<<endl<<colorA.val[0]<<endl<<colorA.val[1]<<endl<<colorA.val[2]<<endl;
        // displayOverlay("Source","haha",2000);
        // cout<<colorA.val;
        break;
    case CV_EVENT_RBUTTONUP:
        cout<<x<<","<<y<<endl;
        colorB=Scalar(src.at<Vec3b>(x,y)[0],src.at<Vec3b>(x,y)[1],src.at<Vec3b>(x,y)[2]);
        cout<<"Color B's been changed to "<<endl<<colorB.val[0]<<endl<<colorB.val[1]<<endl<<colorB.val[2]<<endl;
        break;
    }
}



int process(VideoCapture& capture)
{
  
  for(;;)
  {    
    capture>>src;
    if(src.empty())
    {
      cout<<"Didn't get an image";
      break;
    }
    imshow( source_window, src );

    /////////////////////////
    //COLOR FILTER
    ////////////////////////
    // //Input src, output src_gray

    Scalar lowerBound;
    Scalar upperBound;

    lowerBound = colorA-Scalar::all(colorATol);
    upperBound = colorA+Scalar::all(colorATol);
    // Now we want a mask for the these ranges
    inRange(src,lowerBound,upperBound, srcColorA);

    lowerBound = colorB-Scalar::all(colorBTol);
    upperBound = colorB+Scalar::all(colorBTol);  
    // We do it for both the colours 
    inRange(src,lowerBound,upperBound, srcColorB);

    // Now we create a combined filter for them
    addWeighted(srcColorA, 1, srcColorB, 1, 0, srcColorFilter);
    

    /// Convert image to gray
    cvtColor( src, src_process, COLOR_BGR2GRAY );

    /// Now keep only the required areas in the image  
    // // // multiply(src_process,srcColorFilter,src_gray,1);
    src_gray=srcColorFilter.mul(src_process/255);
    // // // src_gray=srcColorFilter;

    // NOw blur it
    blur( src_gray, src_gray, Size(3,3) );

    imshow( filter_window, srcColorFilter);
    ////////////////////////////

    //BLANK PROCESSING
    // cvtColor( src, src_gray, COLOR_BGR2GRAY );
    // blur( src_gray, src_gray, Size(3,3) );

    /////////////////////////////
    // This is contour Detection
    ////////////////////////////
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

    imshow( "Contours", drawing );
    
    ///////////////////////////    
    //  THIS IS HOUGH
    ///////////////////////////
    // cvtColor(img, cimg, CV_GRAY2BGR);
    cimg=src_gray;

    vector<Vec3f> circles;
    HoughCircles(cimg, circles, CV_HOUGH_GRADIENT, 1, 10,
                 100, 30, minRadius, maxRadius // change the last two parameters
                                // (min_radius & max_radius) to detect larger circles
                 );
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        // Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );        
        Scalar color = Scalar( 0,255,255 );        
        circle( cimg, Point(c[0], c[1]), c[2], color, 3, CV_AA);
        circle( cimg, Point(c[0], c[1]), 2, color, 3, CV_AA);
    }

    imshow("Hough", cimg);


    //////////////////////
    // CLI
    //////////////////////
    char key = (char) waitKey(5); //delay N millis, usually long enough to display and capture input
    switch (key)
    {
        case 'q':
        case 'Q':
        case 27: //escape key
            return 0;
        // case ' ': //Save an image
        //     sprintf(filename, "filename%.3d.jpg", n++);
        //     imwrite(filename, frame);
        //     cout << "Saved " << filename << endl;
        //     break;
        default:
            break;
    }
  }
  return 0;


}

/**
 * @function main
 */
int main( int ac, char** argv )
{


  /// Create Window

  namedWindow( source_window, WINDOW_AUTOSIZE );
  setMouseCallback( "Source", onMouse, 0 );
  // createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback);
  //CAN BE ENABLED, but causes problems, the following lines, to the color detection
  // createTrackbar( " Threshold:", "Source", &thresh, max_thresh, 0);


  //Show the filtered image too

  namedWindow( filter_window, WINDOW_AUTOSIZE );

  //Show the settings window

  namedWindow(settings_window,WINDOW_AUTOSIZE  | CV_GUI_NORMAL);
  createTrackbar( "ColorA Tolerance", settings_window, &colorATol, 256, 0 );
  createTrackbar( "ColorB Tolerance", settings_window, &colorBTol, 256, 0 );
  createTrackbar( "Min Radius (Hough)", settings_window, &minRadius, 10, 0 );
  createTrackbar( "Max Radius (Hough)", settings_window, &maxRadius, 200, 0 );  


  /// Show in a window
  namedWindow( "Contours", WINDOW_AUTOSIZE );
  namedWindow( "Hough", WINDOW_AUTOSIZE );


  /// Load source image
  // src = imread( argv[1], 1 );
  std::string arg = argv[1];
  VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file
  if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
      capture.open(atoi(arg.c_str()));
  if (!capture.isOpened())
  {
      cerr << "Failed to open a video device or video file!\n" << endl;      
      return 1;
  }
  
  return process(capture);  
}
