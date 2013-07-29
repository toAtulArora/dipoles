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
    2. Look at it grow!
        a. Enable screen cropping [done]
        b. Write an algorithm for ellipse to dipole conversion [completed]
        c. Save data for each frame using a circular array of sorts [done]
        d. Output the data perhaps in a text file  [done]
    3. Testing
        a. Test OpenCV's computation time [done]
		    b. Compiling it on Windows [done and as it turns out, useless]
    4. Optimizing
        a. Multi-threading frame fethcing [done]
        b. Multi-threading display update [done, but failed to improve, infact made it worse]
        c. Multi-thread using mutex [done, no real difference but not slow either][it is actually very slow]
		    d. Atomic Sync for frame fetching [done]
        f. Atomic Sync for display update [done, and apparently faster! Yey]
            [There's something wrong though, since the frame jitters like a bad codec.]
            [resolved]
        g. Double Buffer for display [skipped]
    5. Hardware Interface
        a. Modify the CLI to include menus  [done]
        b. Fagocytosis of USB demo program [done]
        c. Working on the Proof of Concept for temperature
          i. Velocity calculation [done]
          ii. Realtime graphs [library installed; plplot; basic functionality tested]
            I. Test separate  [Done]
            II. Fagocytosis   [Done]
        d. Find the axis of the lattice to find the coil anlge [after experimentation, discontinued]
    6. Finalizing
        a. Temperature Interface generic
          i. Proof of concept [Done] 
          ii. Import configurations form file [Done]
          iii. Test proper firing [Done]
          iv. Algorithm to find the best candidate for pumping energy [Done]
          v. Test the algorithm with 2 dipoles (not even a lattice :) [Done]
          vi. Ensure the temperature communication doesn't make the program wait [Done]
          vii. Test with a 2x2 lattice [Done]
        b. Improved colour filter based on HSV
          i. implement [Done]
          ii. allow colour picking (debug exisiting problem) [Done]
        c. Stricter ellipse detection (rejects most unwanted ellipses) [Done]
        d. Temperature Calculation and corresponding proof of concept triger [Done]

*/


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>

#include <atomic>

using namespace cv;
using namespace std;


// #include <string.h>
// #include <array> 

#define GRAPHS_ENABLED

#ifdef GRAPHS_ENABLED
    #include <plplot/plplot.h>
    #include <plplot/plstream.h>        
    plstream *pls=new plstream();
    const char *legline[4];
    int colline[4],styline[4];
    int id1;
    bool plotSqKE=true;
#endif


//Configuration
//#define ATOMIC
// #define MULTI_THREAD_DISPLAY
// #define ATOMIC_DISPLAY
// #define MULTI_THREAD_CAMERA_UPDATE


#define TEMPERATURE_ENABLED


//TODO: Either calculate it at runtime, or allow the user to input
//This is the detected angle of the dipole when it is aligned with the coil (least energy configuration)
// #define COILANGLE 0 
float coilAngle = 90;
#define preciseAngleTol 20
    //This is slightly twisted to explain; it is the difference allowed between the atan2 angle and the ellipse angle to resolve the mod, if this is not clear, refer to the code
const double version=0.6;
// #define MINANGULARVELOCITY 10000000
int minAngularVelocity=100;

int tempCandidate=0;  //This is the dipole that is accelerated
bool blind=false;           //This is the blind option, meaning hardware tracking is turned off
bool invertPush=false;      //This is to invert the moment of pushing
bool useCalibration=true;
#ifdef TEMPERATURE_ENABLED
  #ifdef TEMPERATURE_SINGLDIPOLE
    inline void fireElectro(long frame, int id);
  #else
    void fireElectro(int id, int intensity);
  #endif

  // for USB interface
  extern "C"
  {
    #include "DataTypes.h"
    #include "usbIO.h"
  }
  //
  vector<char> dipolePort;
  vector<char> dipoleBit;
#endif

#ifdef ATOMIC_DISPLAY
  #ifndef MULTI_THREAD_DISPLAY
    #define MULTI_THREAD_DISPLAY
  #endif
#endif

//#if (defined(ATOMIC) || defined(ATOMIC_DISPLAY) || defined(MULTI_THREAD_DISPLAY) || defined(MULTI_THREAD_CAMERA_UPDATE))
  //#include<atomic>
//#endif


#define CALIBRATION_ENABLED

#ifdef CALIBRATION_ENABLED
    Mat cameraMatrix;
    // = Mat(3,3,CV_32FC1);
    Mat distCoeffs;
    // = Mat(5,1,CV_32FC1);    
#endif  

#ifdef ATOMIC
	//#include <atomic>
  atomic<bool> threadsEnabled=false;
#else
  bool threadsEnabled=false;
#endif


#ifdef MULTI_THREAD_DISPLAY
  #ifndef ATOMIC_DISPLAY
    mutex drawnow;
  #endif
#endif

#ifdef ATOMIC_DISPLAY
    atomic<bool> updateDisplayRequested;
    atomic<bool> updateDisplayCompleted;
#endif

mutex processingImage;
mutex grabbingFrame;

//For computation time
double tCstart,tCdelta,tCend; //time for computation
vector <double> computationTime;

vector <Mat>  buf;
// /////////////////////
//     Mat_<double> cameraMatrix(3,3), distCoeffs(5,1);
//     cameraMatrix =[ 5.6712925674714052e+02, 0., 3.1716566879559707e+02, 0., 5.6556813512152769e+02, 2.1037221807058236e+02, 0., 0., 1. ];
// //////////////////////


Mat grabbedFrame; 
#ifdef MULTI_THREAD_CAMERA_UPDATE
  atomic<bool> frameGrabbed,frameRequested;
#else
  bool frameGrabbed=false,frameRequested=false;
#endif

#ifdef CALIBRATION_ENABLED
  void getCameraCalibrationParameters()
  {
    char calibrationFile[]="configurations/logitech2";
    FileStorage fs2(calibrationFile, FileStorage::READ);

    // first method: use (type) operator on FileNode.
    // int frameCount = (int)fs2["frameCount"];

    // std::string date;
    // second method: use FileNode::operator >>
    // fs2["calibrationDate"] >> date;

    // Mat cameraMatrix2, distCoeffs2;

    fs2["camera_matrix"] >> cameraMatrix;
    fs2["distortion_coefficients"] >> distCoeffs;


    // cameraMatrix = Mat(3,3,CV_32FC1);
    // distCoeffs = Mat(5,1,CV_32FC1);    
    // //TODO: Input them off of a file instead..
    // //Not very certain how the pointer thing is working..found this on an implementation on the net
    //   cameraMatrix.ptr<float>(0)[0] = 5.6712925674714052e+02;
    //   cameraMatrix.ptr<float>(0)[1] = 0;
    //   cameraMatrix.ptr<float>(0)[2] = 3.1716566879559707e+02;
    //   cameraMatrix.ptr<float>(1)[0] = 0;
    //   cameraMatrix.ptr<float>(1)[1] = 5.6556813512152769e+02;
    //   cameraMatrix.ptr<float>(1)[2] = 2.1037221807058236e+02;
    //   cameraMatrix.ptr<float>(2)[0] = 0;
    //   cameraMatrix.ptr<float>(2)[1] = 0;
    //   cameraMatrix.ptr<float>(2)[2] = 1;

    //   distCoeffs.ptr<float>(0)[0]=1.0093652191470437e-01;
    //   distCoeffs.ptr<float>(1)[0]=-3.9155163783030478e-01;
    //   distCoeffs.ptr<float>(2)[0]=-8.1203753896884399e-04;
    //   distCoeffs.ptr<float>(3)[0]=4.7881016467320944e-03;
    //   distCoeffs.ptr<float>(4)[0]=2.8593695994355700e-01;
    cout<<"Camera Calibration Enabled"<<endl<<"Using file: "<<calibrationFile<<endl;
    cout<<cameraMatrix<<endl<<distCoeffs<<endl;
  }
#endif
void initializeMultithreadResources()
{
#ifdef MULTI_THREAD_CAMERA_UPDATE
  frameGrabbed=false,frameRequested=false;
#endif  

#ifdef ATOMIC_DISPLAY
    updateDisplayRequested=true;
    updateDisplayCompleted=false;
#endif

#ifdef ATOMIC
  //#include <atomic>
  threadsEnabled=false;
#endif    
}

Mat srcPreCrop; Mat cimg; Mat src; Mat src_gray; Mat srcColorFilter; Mat src_process; Mat srcColorA; Mat srcColorB;Mat drawing;
//////////////
// bool isGood(float val,float expected,float tol)
// {
//   if(abs(val-expected)
// }
//////////////
//the following two functions are from http://www.cs.rit.edu/~ncs/color/t_convert.html
//0-360 and 0-1 (originally)
//r g b is also from zero to one
void RGBtoHSV( double r, double g, double b, double *h, double *s, double *v )
{
  r/=255;
  g/=255;
  b/=255;

  double min, max, delta;

  min = MIN( r, MIN(g, b) );
  max = MAX( r, MAX(g, b) );
  *v = max;       // v

  delta = max - min;

  if( max != 0 )
    *s = delta / max;   // s
  else {
    // r = g = b = 0    // s = 0, v is undefined
    *s = 0;
    *h = -1;
    return;
  }

  if( r == max )
    *h = ( g - b ) / delta;   // between yellow & magenta
  else if( g == max )
    *h = 2 + ( b - r ) / delta; // between cyan & yellow
  else
    *h = 4 + ( r - g ) / delta; // between magenta & cyan

  *h *= 60;       // degrees
  if( *h < 0 )
    *h += 360;

  (*h) *= (180.0/360.0);
  (*s) *= 255.0;
  (*v) *= 255.0;
}

void HSVtoRGB( double *r, double *g, double *b, double h, double s, double v )
{
  h *= (360.0/180.0);
  s /= 255.0;
  v /= 255.0;

  int i;
  double f, p, q, t;

  if( s == 0 ) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }

  h /= 60;      // sector 0 to 5
  i = floor( h );
  f = h - i;      // factorial part of h
  p = v * ( 1 - s );
  q = v * ( 1 - s * f );
  t = v * ( 1 - s * ( 1 - f ) );

  switch( i ) {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:    // case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
  }

  (*r)*=255;
  (*g)*=255;
  (*b)*=255;
}
//////////////////////
float findPrinciple(float val,float modVal)
{
  while(val<0)
    val-=modVal;
  return val;
}

//Think of a clock. I give you two positions on the clock. You've to tell me which ones ahead
/////basically modular arithmetic in some sense
bool IsClockwise(float final, float initial, float modVal)
{
  float delta,deltaA,deltaB;
  delta=final-initial;
  
  deltaA=delta;
  while(deltaA<0)
    deltaA+=modVal;
  
  deltaB=-delta;
  while(deltaB<0)
    deltaB+=modVal;

  if(deltaA<deltaB)
    return true;
  else
    return false;
}
//This is to find the shortest distance in two numbers in a modular arithmetic system
float shortestDistance(float final, float initial, float modVal)
{
  float delta,deltaA,deltaB;
  delta=final-initial;

  deltaA=delta;
  while(deltaA<0)
    deltaA+=modVal;

  deltaB=-delta;
  while(deltaB<0)
    deltaB+=modVal;

  return (deltaA<deltaB) ? deltaA: deltaB;
}


// int lastBuf=1;
//for the cropping
int cropped = 0;
Point origin;
Rect selection;
bool selectRegion;

// Mat cimg;
Mat_<Vec3b> srcTemp = src;
int thresh = 100;
int max_thresh = 255;
int canny=100;
int centre=30;

int minorAxis=7;
int majorAxis=35;
int radius=15;
int ellipseTolerance=8;


int mode=0;
double theta=3.14159;

//mode 
//0 is screen select
//1 is colour select

RNG rng(12345);

/////////////DIPOLE DETECTION

class dipole
{
public:
  double angle,order; //angle is the angle, order gives a rough size of the dipole detected
  int x,y,id;  //centre, id tells where its mapped
  int e1,e2; //index number of ellipse
  static int count[2]; //double buffer
  static int current;  
};

int dipole::count[2] = {0};
int dipole::current=0;

//Not using a dynamic array, doesn't matter for now
//TODO: Use a dynamic aray
// array<dipole,500> dipoles;
// array<dipole,500> lastDipoles;
#define DmaxDipoleCount 1000
// #define DtemperatureAverageOverPeriod 1
int temperatureAverageOverPeriod=1;
dipole dipoles[2][DmaxDipoleCount];
// Colour read
// Point origin;



///////////////DIPOLE INFORMATION STORAGE IN RAM
bool dipoleRec=false;

class dipoleSkel
{
public:
    double angle;
    int id; //For the dipoleData, this refers to the id of seedDipole
    //in seedDipole, this should refer to the hardware ID
    vector<int> neighour;
    vector<double> neAngle;
    int x,y;
    double instAngularVelocity;
    bool detected;  //stores whether the dipole was detected at all
};

//This class is for storing dipole data of a given frame
class dipoleFrame
{
public:
  double time;   //time elapsed since the seed frame
  double order;  //gives the rough size of the dipoles
  int count,velValidCount;    //number of dipole detected in the frame, number of dipoles for which inst angular velocity could be calculated (essentially, that it was detected in two consecutive frames)
  double meanSquaredAngularVelocity,temperature; //mean of squares of angular velocities of each of the dipoles
  vector<dipoleSkel> data;
};

// #define DframeBufferLen  5000
dipoleFrame seedDipole;
//NOTE: You have to fix the numbering problem right here.
vector <dipoleFrame> dipoleData;

#define DframeBufferLen 5000
////THIS IS FOR STORING IN FILE
FILE * pFile;
char fileName[50];
/////////////////////////////
#ifdef TEMPERATURE_ENABLED
  void getTemperaturePins()
  {

    char temperaturePinsFile[]="configurations/temperaturePins";    
    FileStorage fs2(temperaturePinsFile, FileStorage::READ);
    cout<<endl<<"Temperature Enabled, using file: "<<temperaturePinsFile<<endl;

    char tempString[5];    
    string extractedString;
    dipolePort.clear();
    dipoleBit.clear();

    int iMax;
    fs2["count"]>>iMax;

    // FileNodeIterator it = fs2.begin(), it_end = fs2.end();
    // int idx = 0;
    // std::vector<uchar> lbpval;

    // // iterate through a sequence using FileNodeIterator
    // for( ; it != it_end; ++it, idx++ )
    // {
    //     // cout << "feature #" << idx << ": ";
    //     cout << "x=" << (int)(*it)["x"] << ", y=" << (int)(*it)["y"] << ", lbp: (";
    //     // you can also easily read numerical arrays using FileNode >> std::vector operator.
    //     (*it)["lbp"] >> lbpval;
    //     for( int i = 0; i < (int)lbpval.size(); i++ )
    //         cout << " " << (int)lbpval[i];
    //     cout << ")" << endl;
    // }


    for(int i=0;i<iMax;i++)
    {
      sprintf(tempString,"p%d",i);
      fs2[tempString] >> extractedString;
      // cout<<tempString<<" "<<extractedString<<endl;
      if(extractedString.size()>1)
      {
        dipolePort.push_back(extractedString[0]);
        dipoleBit.push_back((extractedString[1]-'0'));
        //Not using dipole pin because obviously char corresponding to 0 is not '0'
        cout<<"Dipole "<<i<<" \t -- \t"<<dipolePort[i]<<extractedString[1]<<" (Port"<<dipolePort[i]<<" Bit"<<extractedString[1]<<")"<<endl;
      }
      else
      {
        dipolePort.push_back('Z');
        dipoleBit.push_back(9);
      }
    }
    
  }
#endif



///////////////////////
float candidateAptitude( int id)
{
  // int candidate=posPhysicalToDetected(id);
  int cf=dipoleData.size()-1;
  int candidate=id;
  float distanceFromCoil=shortestDistance(dipoleData[cf].data[candidate].angle,coilAngle,360);
  // if((dipoleData[cf].data[tempCandidate].angle - COILANGLE) > 0)
  if(IsClockwise(dipoleData[cf].data[candidate].angle,coilAngle,360))
  {
    if (dipoleData[cf].data[candidate].instAngularVelocity>0) //if it is going in the opposite direction
    {
      if (invertPush)
        // fireElectro(cf,tempCandidate);
        return distanceFromCoil;
    }
    else
    {
      if(!invertPush)
         // fireElectro(cf,tempCandidate);  //to increase speed, because the force will be towards the coil              
        return distanceFromCoil;
    }             
      
     
  }
  else  //if the angle is negaative, then
  {
    if (dipoleData[cf].data[candidate].instAngularVelocity<0) //if it is going twoards the coil
    {
      if(invertPush)
        // fireElectro(cf,tempCandidate);
        return distanceFromCoil;
    }
    else
    {
      if(!invertPush)
        // fireElectro(cf,tempCandidate);
        return distanceFromCoil;
    }
      // fireElectro(cf);
  }
  return 0; //this means the given dipole can't be used to pump in energy
}


///////////

int posPhysicalToDetected(int phyID)
{
  for(int i=0;i<seedDipole.data.size();i++)
  {
    if(seedDipole.data[i].id==phyID)
      return i;
  }
  cout<<endl<<"INVALID physical ID";
  return 0;
}

// This is a colour filter for improving accuracy
  // 20, 28, 41 [dark]
  // TODO: Allow the user to select the colour
  // Scalar colorB=Scalar(245,245,10);
  // Scalar colorB=Scalar(126,88,47);
  // Scalar colorA=Scalar(10,245,245);

  // HSV



  //This is experimental :P
  Scalar colorA((float)103.953, (float)151.59, (float)217);
  
  //The following's from GIMP
  // Scalar colorA((float)206*(360/180),74*(255/100),83*(255/100));

  // int colorATol=30;
  // int colorBTol=35;
  // int brightInv=10;  //this is to increase the brightness after processing
  // H: 0 - 180, S: 0 - 255, V: 0 - 255

  int hueTol=20;
  int valueTol=193;
  int saturationTol=85;

  int colorATol=255;
  int colorBTol=255;
  int brightInv=74;  //this is to increase the brightness after processing

//
  const char* source_window = "Source";
  const char* filter_window = "Color Filter";
  const char* settings_window="Settings";


//////////TIMING
  long t,tLast,tickWhenGrabbed;
  double deltaT;
static void onMouse( int event, int x, int y, int, void* )
{
  if(mode==0)
  {
    if( selectRegion )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, src.cols, src.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        cropped=0;
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectRegion = true;
        break;
    case CV_EVENT_LBUTTONUP:
        cropped=0;
        selectRegion = false;
        if( selection.width > 0 && selection.height > 0 )
            cropped = -1;
        break;
    }    
  }
  else if(mode==1)
  {
    switch( event )
    {
    case CV_EVENT_LBUTTONUP:
        cout<<"Color A is currently (HSV) "<<endl<<(float)colorA.val[0]<<endl<<(float)colorA.val[1]<<endl<<(float)colorA.val[2]<<endl;
        cout<<endl<<"x="<<x<<",y="<<y<<endl;
        // colorA=Scalar(srcPreCrop.at<Vec3b>(y,x)[0],srcPreCrop.at<Vec3b>(y,x)[1],srcPreCrop.at<Vec3b>(y,x)[2]);    
        //THIS IS RIGHT, it had to be y,x and not x,y!!! damn..
        RGBtoHSV((int)srcPreCrop.at<Vec3b>(y,x)[2],(int)srcPreCrop.at<Vec3b>(y,x)[1],(int)srcPreCrop.at<Vec3b>(y,x)[0],&(colorA.val[0]),&(colorA.val[1]),&(colorA.val[2]));
        cout<<"BGR colours are "<<(int)srcPreCrop.at<Vec3b>(y,x)[2]<<","<<(int)srcPreCrop.at<Vec3b>(y,x)[1]<<","<<(int)srcPreCrop.at<Vec3b>(y,x)[0]<<endl;
        // printf("RGB colours are %f %f %f",srcPreCrop.at<Vec3b>(y,x)[0],srcPreCrop.at<Vec3b>(y,x)[1],srcPreCrop.at<Vec3b>(y,x)[2]);
        cout<<"Color A's been changed to "<<endl<<(float)colorA.val[0]<<endl<<(float)colorA.val[1]<<endl<<(float)colorA.val[2]<<endl;
        break;
    // case CV_EVENT_RBUTTONUP:
    //     cout<<x<<","<<y<<endl;
    //     colorB=Scalar(src.at<Vec3b>(y,x)[0],src.at<Vec3b>(y,x)[1],src.at<Vec3b>(y,x)[2]);
    //     cout<<"Color B's been changed to "<<endl<<colorB.val[0]<<endl<<colorB.val[1]<<endl<<colorB.val[2]<<endl;
    //     break;
    }
  }
}

#ifdef __GNUC__
	void tGrabFrame(VideoCapture&& capture)
#else
	void tGrabFrame(VideoCapture& capture)
#endif
{
  while(threadsEnabled)
  {
    
    capture>>grabbedFrame;    
    if(frameRequested)
    {		
      tickWhenGrabbed=getTickCount();
    	grabbedFrame.copyTo(srcPreCrop);
    	frameGrabbed=true;
    }

	//buf.push_back(frameGrabbed);

    // if(processingImage.try_lock())
    // {
    //   buf.copyTo(srcPreCrop);      
    //   processingImage.unlock();
    // }
  }
  
}

void updateDisplay()
{
  if(!drawing.empty())
    imshow("Contours", drawing );
  if(!cimg.empty())
    imshow("Debug", cimg);
  if(!src_gray.empty())
    imshow( filter_window, src_gray);
  if(!srcPreCrop.empty())
    imshow( source_window, srcPreCrop );

}

#ifdef MULTI_THREAD_DISPLAY
  #ifndef ATOMIC_DISPLAY
    void tUpdateDisplay()
    {
      while(threadsEnabled)
      {
        drawnow.lock(); //this is to ensure it updates only once the processing has been done and not repetatively the same frame
        drawnow.unlock();

        updateDisplay();

      }
    }
  #else
    void tAtomicDisplay()
    {
      while(threadsEnabled)
      {
        
          // && updateDisplayCompleted==false)
      	if(updateDisplayRequested)
        {
          updateDisplay();
          // this_thread::sleep_for(chrono::milliseconds(10));
          // updateDisplayCompleted=true;          
          updateDisplayRequested=false;
        }
		    // else
          // this_thread::sleep_for(chrono::milliseconds (1));
			 // this_thread::yield();

      }
    }
  #endif
#endif
#ifdef GRAPHS_ENABLED

    void clearGraph()
    {
      pls->col0(1);
      // cout<<"1"<<endl;
      double xmin2d = -2.5;
      double xmax2d =  2.5;
      double ymin2d = -2.5;
      double ymax2d =  4.0;
      // pls->wind( 0.0, 1.0, 0.0, 1.0 );
      // pls->env(xmin2d, xmax2d, ymin2d, ymax2d, 0, -2);
      pls->adv(1);
      pls->clear();
      pls->vpor( 0.0, 1.0, 0.0, 1.0 );
      pls->wind( -2.5, 2.5, -3.0, 3.0 );

      double basex = 2.0;
      double basey = 4.0;
      double height = 3.0;
      double xmin = 0.0;
      double xmax = 10.0;
      double ymin = 0.0;
      double ymax = 1000.0;
      double zmin = 0.0;
      double zmax = 360.0;
      double alt = 45.0;
      double az = 30.0;
      double side = 1;        
      pls->w3d(basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az);
      pls->box3( "bnstu", "Dipole Count", 0.0, 0,
              "bnstu", "Frame Count", 0.0, 0,
              "bcdmnstuv", "Angular Position", 0.0, 4 );
      
      


      // This is for selecting the second view
      // Following are to get the points in the right location!
      pls->adv(2);
      pls->clear();
      pls->vpor( 0.0, 1.0, 0.0, 1.0 );
      pls->wind( -2.5, 2.5, -3.0, 3.0 );
      // pls->env(xmin2d, xmax2d, ymin2d, ymax2d, 0, -2);
      // pls->wind( 0.0, 1.0, 0.0, 1.0 );
      zmin=-360;
      zmax=360;
      pls->w3d(basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az);
      pls->box3( "bnstu", "Dipole Count", 0.0, 0,
              "bnstu", "Frame Count", 0.0, 0,
              "bcdmnstuv", "Angular Velocity", 0.0, 4 );

      pls->adv(3);


      //the second false is the one!

      legline[0] = "total energy";                       // pens legend
      legline[1] = "not assigned";
      legline[2] = "not assigned";
      legline[3] = "not assigned";

      pls->stripc( &id1, "bcnst", "bcnstv",
          0, 100, 0.3, 0, 10,
          0, 0,
          true, false,
          1, 3,
          colline, styline, legline,
          "t", "", "Average Kinetic Energy" );
    }
#endif

int process(VideoCapture& capture)
{

  #ifdef GRAPHS_ENABLED
    styline[0] = colline[0] = 2;      // pens color and line style
    styline[1] = colline[1] = 3;
    styline[2] = colline[2] = 4;
    styline[3] = colline[3] = 5;        
    
    pls->init();
    pls->ssub( 1, 3 );
    // pls->adv(1);
    cout<<endl<<"Initializing the interface for graphing"<<endl;
    clearGraph();

  #endif


  #ifdef TEMPERATURE_ENABLED
      vInitUSB();
  #endif      

  /// Create Window
  namedWindow( source_window, WINDOW_AUTOSIZE );
  setMouseCallback( "Source", onMouse, 0 );
  //Show the filtered image too

  namedWindow( filter_window, WINDOW_AUTOSIZE );

  //Show the settings window

  namedWindow(settings_window,WINDOW_AUTOSIZE  | CV_GUI_NORMAL);
  

  createTrackbar( "Min Ang Vel Sq", settings_window, &minAngularVelocity, 100000, 0 );
  createTrackbar( "Averaging Time", settings_window, &temperatureAverageOverPeriod, 10, 0 );
  // H: 0 - 180, S: 0 - 255, V: 0 - 255  
  createTrackbar( "Hue Tolerance", settings_window, &hueTol, 180, 0 );
  createTrackbar( "Saturation Tolerance", settings_window, &saturationTol, 255, 0 );
  createTrackbar( "Value Tolerance", settings_window, &valueTol, 255,0 );

  // createTrackbar( "ColorB Tolerance", settings_window, &colorBTol, 256, 0 );
  createTrackbar( "Minor Axis", settings_window, &minorAxis, 100, 0 );
  createTrackbar( "Major Axis", settings_window, &majorAxis, 200, 0 ); 
  createTrackbar( "Radius", settings_window, &radius, 200, 0 ); 
  createTrackbar( "Ellipse Tolerance", settings_window, &ellipseTolerance, 200, 0 ); 

  createTrackbar( "Brightness Inverse",settings_window, &brightInv, 255, 0);
  createTrackbar( "Canny (Hough)", settings_window, &canny, 200, 0 );  
  createTrackbar( "Centre (Hough)", settings_window, &centre, 200, 0 );    
  // createTrackbar( "Theta", settings_window, &thetaD, 3.141591, 0 );    

  /// Show in a window
  namedWindow( "Contours", WINDOW_AUTOSIZE );
  namedWindow( "Debug", WINDOW_AUTOSIZE );


  ////Voodoo intializations
  // dipoles[0][0].current=0;
  // dipoles[0][0].count[0]=0;
  // dipoles[0][0].count[1]=0;
  /// Load source image
  // src = imread( argv[1], 1 );
  
  // std::string arg = argv[1];      
////////////////

  // cout<<capture.get(CV_CAP_PROP_FRAME_HEIGHT);
  // capture.set(CV_CAP_PROP_FRAME_HEIGHT,1080);
  // capture.set(CV_CAP_PROP_FRAME_WIDTH,1920);
    
  // capture.set(CV_CAP_PROP_FRAME_HEIGHT,720);
  // capture.set(CV_CAP_PROP_FRAME_WIDTH,1280);

  capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);
  capture.set(CV_CAP_PROP_FRAME_WIDTH,640);
  
  thread t1(tGrabFrame,capture);
  #ifdef MULTI_THREAD_DISPLAY
    #ifndef ATOMIC_DISPLAY
      thread t2(tUpdateDisplay);
    #else
      thread t2(tAtomicDisplay);
    #endif
  #endif




  for(;;)
  {
    // processingImage.lock();
    //if the video feed has over 1 frame

    // if(buf.size()>1)
    // {
    //   buf.erase(buf.begin()); //remove the oldest frame
    //   srcPreCrop=buf[buf.size()-1]; //grab the latest frame
    // }

    // frameGrabbed.copyTo(srcPreCrop);
	frameRequested=true;

    if(frameGrabbed==true && !srcPreCrop.empty())
		// #ifdef ATOMIC_DISPLAY
		// 	// if(updateDisplayCompleted)
		// #endif
      
    {
      frameRequested=false;	//this is so that the frame is not processed unless required
  	  frameGrabbed=false;	//this is so that we know the next time a frame is grabbed

      #ifdef MULTI_THREAD_DISPLAY
        // drawnow=true;
        #ifndef ATOMIC_DISPLAY
          drawnow.lock();          
        #endif
      #endif

      {  //IMAGE CAPTURE and CROP  
        // capture>>srcPreCrop;
        ///////////////COMPUTATION TIME CALCULATION
        tCstart=getTickCount();
        ///////////
        tLast=t;
        // t=getTickCount()/getTickFrequency();   //This is give time in seconds
        // t=getTickCount(); 
        t=tickWhenGrabbed;
        deltaT=(t-tLast)/getTickFrequency();

        if(dipoleRec)
        {
          //if this is not the last frame, add a frame
          dipoleData.push_back(seedDipole);
          //This is to avoid overflows
          // if (dipoleData.size()>DframeBufferLen)
            // dipoleData.erase(dipoleData.begin());
          //for the last frame
          dipoleData[dipoleData.size()-1].time=dipoleData[dipoleData.size()-2].time+deltaT;
          dipoleData[dipoleData.size()-1].count=0;  //No dipoles found before analysis!
          dipoleData[dipoleData.size()-1].meanSquaredAngularVelocity=0; //Initially zero
          dipoleData[dipoleData.size()-1].velValidCount=0;              //This is to set the number of dipoles for which velocity was calculated to zero. Very important
        }

        // long cfInit=dipoleData.size()-1;  //last frame
        
        // //test for current frame
        // if(dipoleData[cfInit].time!=t)
        // {
        //   //if this is not the last frame, add a frame
        //   dipoleData.push_back(seedDipole);
        //   //This is to avoid overflows
        //   if (dipoleData.size()>DframeBufferLen)
        //     dipoleData.erase(dipoleData.begin());
        //   //for the last frame
        //   cfInit=dipoleData.size()-1;
        //   dipoleData[cfInit].time=t;
        // }



        if(srcPreCrop.empty())
        {
          cout<<"Didn't get an image";
          break;
        }
        if(!cropped==0)
        {
          src=srcPreCrop(selection);
        }
        else
          src=srcPreCrop;
        
        #ifdef CALIBRATION_ENABLED
          if(useCalibration)
          {
            Mat temp=src.clone();
            undistort(temp, src, cameraMatrix, distCoeffs);            
          }
        #endif
        // imshow( source_window, srcPreCrop );

      }

      ////////////////////////
      {//COLOR FILTER
        //Input src, output src_gray
        Scalar lowerBound;
        Scalar upperBound;
        Mat srcCloneTemp=src.clone();
        

        cvtColor(src,src,CV_BGR2HSV);
        // lowerBound = colorA-Scalar::all(colorATol);
        // upperBound = colorA+Scalar::all(colorATol);
        upperBound=colorA+Scalar((float)hueTol,(float)saturationTol,(float)valueTol);
        lowerBound=colorA-Scalar((float)hueTol,(float)saturationTol,(float)valueTol);

        // Now we want a mask for the these ranges
        inRange(src,lowerBound,upperBound, srcColorA);

        // lowerBound = colorB-Scalar::all(colorBTol);
        // upperBound = colorB+Scalar::all(colorBTol);  
        // We do it for both the colours 
        // cvtColor(src,)
        inRange(src,lowerBound,upperBound, srcColorB);

        // Now we create a combined filter for them
        // addWeighted(srcColorA, 1, srcColorB, 1, 0, srcColorFilter);
        addWeighted(srcColorA, 1, srcColorB, 0, 0, srcColorFilter);
        
        cvtColor(src,src,CV_HSV2BGR);
        // cvtColor(srcColorA,srcColorA,CV_HSV2BGR);
        /// Convert image to gray
        cvtColor( src, src_process, COLOR_BGR2GRAY );

        /// Now keep only the required areas in the image  
        // // // multiply(src_process,srcColorFilter,src_gray,1);
        


        //Change the following to use greyscaled output
        src_gray=srcColorFilter.mul(src_process/brightInv);
        // src_gray=srcColorFilter;

        // // // src_gray=srcColorFilter;

        // NOw blur it
        blur( src_gray, src_gray, Size(3,3) );

        // imshow( filter_window, src_gray);
      }

      ////////////////////////////

      // BLANK PROCESSING
      // medianBlur( src, src, 5 );
      // cvtColor( src, src_gray, COLOR_BGR2GRAY );

      // // // blur( src_gray, src_gray, Size(3,3) );

      /////////////////////////////    
      // This is contour Detection
      /////////////////////////////////
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

      for( size_t i = 0; i< minEllipse.size(); i++ )
         {
          //You can add aditional conditions to eliminate detected ellipses
          float minor = (minEllipse[i].size.height<minEllipse[i].size.width)?minEllipse[i].size.height:minEllipse[i].size.width;
          float major = (minEllipse[i].size.height>minEllipse[i].size.width)?minEllipse[i].size.height:minEllipse[i].size.width;
          bool good=false;
          
          if(minor/major<1.3 && minor/major>0.7)  //if close enough to a circle
          {
            if((abs(minor-radius)<ellipseTolerance) && (abs(major-radius)<ellipseTolerance))
              good=true;  //VALID CIRCLE DETECTED
          }
          else //IF its an ellipse then
          {
            if(abs(minor-minorAxis)<ellipseTolerance && abs(major-majorAxis)<ellipseTolerance)
              good=true;  //VALID ELLIPSE FOUND
          }
          // if(
          //   !(
          //     (
          //     (minEllipse[i].size.height>minMinorAxis && minEllipse[i].size.width>minMinorAxis) 
          //     &&
          //     (minEllipse[i].size.height<maxMajorAxis && minEllipse[i].size.width<maxMajorAxis)
          //     )
          //   )            

          //   )
          if(!good)
          {
            // minEllipse[i]=RotatedRect(Point2f(0,0),Point2f(0,0),0);
            minEllipse.erase(minEllipse.begin()+i--);
          }
         }

      

      ///////////////////////////
      //  Dipole Detection Algorithm
      ////////////////////////////////////////
      // This detects form the ellipses detected, the dipoles!!
      // This doesn't act on the dipoles itself!!!
      // You yourself got confused about this :D
      ///////////////////////////    
      vector<bool> detected (minEllipse.size(),false);

      int k = !(dipoles[0][0].current);
      dipoles[0][0].current=k;
      dipoles[0][0].count[k]=0; //this count is different from that used in frames, they correspond to a subset of these which are position wise close enough to the seed frame
      
      // dipolesA[0].lastcount=0;
      for (int i=0; i<minEllipse.size();i++)
      {
        if(detected[i]==false)  //This detected corresponds to having been paired earlier
        {
          for (int j=0; j<minEllipse.size();j++)
          {
            if((i!=j) && detected[j]==false)  //This is so that you don't test with yourself and with others that got paired
            {
                // if(abs(minEllipse[i].angle-minEllipse[j].angle)< 15)  //if the orientation matches (less than 5 degrees), then 
                {
                  //Find the distance between minEllipses

                  Point differenceVector=Point(minEllipse[i].center.x - minEllipse[j].center.x, minEllipse[i].center.y - minEllipse[j].center.y);  //find the difference vector
                  double distanceSquared=differenceVector.x*differenceVector.x + differenceVector.y*differenceVector.y; //find the distance squared

                  //Find the major axis length
                  double majorAxis=MAX( MAX(minEllipse[i].size.width, minEllipse[i].size.height) , MAX(minEllipse[j].size.width, minEllipse[j].size.height)); //find the max dimension
                  
                  //The ratio is the ratio between the distance between the ellipse and the small circle and the length of the major axis
                  double errorPlusOne = distanceSquared / ((11.348/30)*(11.348/30)*majorAxis*majorAxis) ; //now to compare, just divide and see if it's close enough to one

                  if (errorPlusOne>0.5 && errorPlusOne<2)  //if the error is small enough, then its a match
                  {
                      // This is to ensure these don't get paired
                      detected[i]=true;
                      detected[j]=true;

                      // this is collection of the final result
                      int c=dipoles[k][0].count[k]++; //dont get confused, count is static, so even dipoles[0][0] would've worked, ro for that matter, any valid index
                      // Note the ++ is after because the count is always one greater than the index of the last element!
                      

                      // dipoles[k][c].angle=(minEllipse[i].angle + minEllipse[j].angle)/2.0;
                      // dipoles[k][c].angle=(minEllipse[i].angle);

                      //We're using two shapes, one ellipse and one circle.                    
                      RotatedRect largerEllipse =  ( MAX(minEllipse[i].size.width, minEllipse[i].size.height) > MAX(minEllipse[j].size.width, minEllipse[j].size.height)  )?minEllipse[i]:minEllipse[j];
                      RotatedRect smallerEllipse =  ( MAX(minEllipse[i].size.width, minEllipse[i].size.height) <= MAX(minEllipse[j].size.width, minEllipse[j].size.height)  )?minEllipse[i]:minEllipse[j];
                      

                      //CENTRE BASED ANGLE ESTIMATION
                      // double dCenterX=largerEllipse.center.x-smallerEllipse.center.x;
                      // double dCenterY=largerEllipse.center.y-smallerEllipse.center.y;

                      // dipoles[k][c].angle=((180/3.1415926535)*atan2(dCenterY,dCenterX)) + 180;
                      //////////////////////////////

                      //UNCOMMENT THIS PART FOR THE OLD ANGLE ESTIMATOR
                      // dipoles[k][c].angle=(largerEllipse.angle);

                      //Now we use the circle to remove the mod 180 problem and get the complete 360 degree position
                      // if((smallerEllipse.center.y -largerEllipse.center.y) < 0)
                      //   dipoles[k][c].angle+=180;
                      ////////////////////////


                      //////////////////////////////////////

                      //THIS IS ATAN ASSISTED RESOLOLVING ELLIPSE ANGLE
                      //This is because the dipole angle is more accurate!
                      double preciseAngle=(largerEllipse.angle);

                      //This is calculated using the centres of the ellipse and circle
                      
                      double dCenterX=largerEllipse.center.x-smallerEllipse.center.x;
                      double dCenterY=largerEllipse.center.y-smallerEllipse.center.y;

                      //IN CODE NOTES:
                      //ELLIPSE is zero/180 when straight
                      //atan2 gives slope of the line, above the axis is positive, below is negative

                      //The first + 180 is because atan2 returns between plus and minus pi,
                      /////and the minus ninty is because the ellipse is perpendicular to the line joining centres of the two ellipses (ellipse and circle)
                      double roughAngle=(((180/3.1415926535)*atan2(dCenterY,dCenterX)) + 180);

                      //THIS DOESN'T GIVE PROPER RESULTS!!
                      // //THIS IS INTERESTING..
                      // // if (roughAngle>0 && roughAngle<180)
                      // if(roughAngle>350 && preciseAngle<10)
                      // {
                      //   // preciseAngle+=180;
                      //   ;
                      //   //do NOTHING!
                      //   //don't remove this bbecause there are else cases also!
                      // }
                      // else if(roughAngle<10 && preciseAngle>170)
                      // {
                      //   preciseAngle+=180;
                      // }
                      // else
                      // {
                      //   double equivalentAngle=roughAngle;  //will always be between 0 and 180
                      //   //if the precise angle is anyway close enough then dont do anything, else
                      //   //The commented didn't work
                      //   // if(( ((int)abs(preciseAngle-equivalentAngle)) % 360)>preciseAngleTol && ( ((int) abs((preciseAngle+180)-equivalentAngle)) % 360)<preciseAngleTol)
                      //   if(abs(preciseAngle-equivalentAngle)>preciseAngleTol && abs((preciseAngle+180)-equivalentAngle)<=preciseAngleTol)
                      //   {
                      //     preciseAngle += 180;
                      //   }
                      // }                      
                      //THIS IS MATH POWER (actually miniscule manifestation of math's power)
                      if((shortestDistance(roughAngle,preciseAngle,360)-shortestDistance(roughAngle,preciseAngle+180,360))>5)
                        preciseAngle+=180;

                      dipoles[k][c].angle=findPrinciple(preciseAngle,360);
                      // dipoles[k][c].angle=roughAngle;
                      ///////////////////


                      // dipoles[k][c].x=(minEllipse[i].center.x + minEllipse[j].center.x)/2.0;
                      // dipoles[k][c].y=(minEllipse[i].center.y + minEllipse[j].center.y)/2.0;

                      dipoles[k][c].order=MAX(largerEllipse.size.height, largerEllipse.size.width);

                      dipoles[k][c].x=largerEllipse.center.x; //(minEllipse[i].center.x + minEllipse[j].center.x)/2.0;
                      dipoles[k][c].y=largerEllipse.center.y; //(minEllipse[i].center.y + minEllipse[j].center.y)/2.0;



                      dipoles[k][c].e1=i; //don't know why this is required
                      dipoles[k][c].e2=j;



                      // A NEW DIPOLE HAS BEEN DETECTED (HOPEFULLY)
                      // TIME TO PUT IT IN PLACE (IF ASKED TO)
                      //////////////THIS IS FOR RECORDING/SAVING THE DIPOLE MOVEMENT/////////////
                      if (dipoleRec==true)
                      {

                        long cf=dipoleData.size()-1;  //last frame


                        for(int q=0;q<seedDipole.data.size();q++)
                        {
                          //This is to test which dipole belongs where in accordance with the seedDipole frame
                          // if(MAX(abs(seedDipole.data[q].x - dipoles[k][c].x), abs(seedDipole.data[q].y - dipoles[k][c].y)) < (seedDipole.order/2.0) )
                          //Or you could use the last fraame for this
                          //CAVEAT: YOU MAY THINK WITH THE LAST FRAME, THERE MIGHT ALREADY HAVE BEEN MISSES!! BUT THAT'S ALRIGHT, EVERY LAST FRAME COMES FORM THE SEED FRAME, SO AT WORST, IT WILL BE 
                          //MATCHED TO THE DIPOLE IN THE SEED FRAME
                          if(
                            (MAX(abs(dipoleData[cf-1].data[q].x - dipoles[k][c].x), abs(dipoleData[cf-1].data[q].y - dipoles[k][c].y)) < (dipoleData[cf-1].order/4.0) )
                            && 
                            (dipoleData[cf].data[q].detected==false)
                            )
                          {
                            dipoles[k][c].id=q;
                            // dipoleData.data[q] = dipoles[k][c]
                            //TODO: Make a function for converting      
                            dipoleData[cf].data[q].id=q;                       
                            dipoleData[cf].data[q].x=dipoles[k][c].x; //Copy the relavent data from the dipole data collected into the temp dipole
                            dipoleData[cf].data[q].y=dipoles[k][c].y;
                            dipoleData[cf].data[q].angle=dipoles[k][c].angle;
                            dipoleData[cf].count+=1;
                            if(cf==0)
                              dipoleData[cf].data[q].instAngularVelocity=0;
                            else
                            {
                              if(dipoleData[cf-1].data[q].detected==true)
                              {
                                double deltaAngle=(dipoleData[cf].data[q].angle - dipoleData[cf-1].data[q].angle);
                                if(abs(deltaAngle)>300)  //eg. 359 - 2
                                {
                                  if(dipoleData[cf].data[q].angle<30) //roughly speaking, it couldn't couldn't have crossed 20!
                                  {
                                    deltaAngle=(dipoleData[cf].data[q].angle+360)-dipoleData[cf-1].data[q].angle;
                                  }
                                  else  //the other one must be close to zero!
                                  {
                                    deltaAngle=dipoleData[cf].data[q].angle-(dipoleData[cf-1].data[q].angle+360);
                                  }
                                }
                                dipoleData[cf].data[q].instAngularVelocity=deltaAngle/deltaT;
                                dipoleData[cf].velValidCount+=1;
                              }
                              else
                              {
                                dipoleData[cf].data[q].instAngularVelocity=0; //this is NOT TRUE! but doen't matter, because the usual analysis will use angle vs time
                                //this is used for angular velocity caclulation, in which it will be added, but not counted while dividing...so it is harmless
                              }
                            }
                              
                            
                            // if(cf>1)
                              // if(dipoleData[cf-1].data[q].detected)
                            dipoleData[cf].meanSquaredAngularVelocity += (dipoleData[cf].data[q].instAngularVelocity*dipoleData[cf].data[q].instAngularVelocity) ;  //Add the sqr of inst angular velocity, averaging is done later
                            
                            dipoleData[cf].data[q].detected=true;   //This is true only when the dipole's
                            
                            dipoleData[cf].order=dipoles[k][c].order; //This is bad programming..i should average, but doens't matter
                            //Now that it has matched, terminate the loop
                            q=seedDipole.data.size();
                          }
                        }


                      }


                  }
                  // magnitude(differenceVector.x,differenceVector.y,distance);

                  // point positionVector ((minEllipse[i].x + minEllipse[j].x)/2.0,(minEllipse[i].y + minEllipse[j].y)/2.0);                

                }
            }
          } 
        }
      }
      //////////////AFter the frame has been processed, evaluate physical quantities of interest
      if(dipoleRec==true)
      {
        long cf=dipoleData.size()-1;  //last frame
        if(dipoleData[cf].velValidCount>0)
          dipoleData[cf].meanSquaredAngularVelocity/=dipoleData[cf].velValidCount;
        //else it would be zero, the meanSquaredAngularVelocity
        // dipoleData[cf].meanSquaredAngularVelocity=sqrt(dipoleData[cf].meanSquaredAngularVelocity);
        
        int temperatureFrameCount=0;
        double temperatureFrame=0;
        for(int k=cf;k>0;k--)
        {
          double dTime=(dipoleData[cf].time-dipoleData[k].time);
          // cout<<dTime;
          if(dTime>=0 && dTime<(double)temperatureAverageOverPeriod)
          {
            temperatureFrame+=dipoleData[k].meanSquaredAngularVelocity;
            // cout<<temperatureFrame;
            temperatureFrameCount++;
          }
          else
            k=0;  //terminate the loop
        }
        if(temperatureFrameCount>0)
          temperatureFrame/=temperatureFrameCount;
        else
          temperatureFrame=0;
        dipoleData[cf].temperature=temperatureFrame;
        

        #ifdef TEMPERATURE_ENABLED

          //If you want to put pid, put it here..
          #ifndef TEMPERATURE_SINGLDIPOLE
          if(dipoleData[cf].temperature<(minAngularVelocity))
          {
            static int lastFireIntensity=0;
            static long lastFireTime=0;
            int bestCandidate=0;    //Gotto pump some dipole! :P
            // float lastMinDist=360;  //This is the how far it is from the coil, can't be greater than 180
            float lastMaxSin=0;
            for(int i=0;i<dipoleData[cf].data.size();i++)
            {
              // float minDist=candidateAptitude(i);
              float maxSin=abs(sin(candidateAptitude(i) * (3.1415926535/180)));
              // if(minDist<lastMinDist)
              if(maxSin>lastMaxSin)
              {                
                // lastMinDist=minDist;
                lastMaxSin=maxSin;
                bestCandidate=i;
              }
            // static int lastFireIntensity=0;
            // static long lastFireTime=0;
            // int bestCandidate=0;    //Gotto pump some dipole! :P
            // float lastMinDist=360;  //This is the how far it is from the coil, can't be greater than 180
            // for(int i=0;i<dipoleData[cf].data.size();i++)
            // {
            //   float minDist=candidateAptitude(i);
            //   if(minDist<lastMinDist)
            //   {                
            //     lastMinDist=minDist;
            //     bestCandidate=i;
            //   }
            }

            int dipoleToFire=seedDipole.data[ dipoleData[cf].data[bestCandidate].id ].id;
            // fireElectro(dipoleToFire,abs(dipoleData[cf].data[bestCandidate].instAngularVelocity/10));
            cout<<( (tickWhenGrabbed - lastFireTime )/getTickFrequency() ) <<","<<(((float)lastFireIntensity)/1000)<<endl;
            if( ( (tickWhenGrabbed - lastFireTime )/getTickFrequency() )  > (((float)lastFireIntensity)/1000) )
            {
              lastFireIntensity=30;
              fireElectro(dipoleToFire,lastFireIntensity);  
              lastFireTime=tickWhenGrabbed;              
            }            
          }
          #endif

          #ifdef TEMPERATURE_SINGLDIPOLE
          //THE SINGLE DIPOLE ALGORITHM
          if(!blind)
          {
            if(dipoleData[cf].temperature<(minAngularVelocity))
            {
              int candidate=posPhysicalToDetected(tempCandidate);

              // if((dipoleData[cf].data[tempCandidate].angle - COILANGLE) > 0)
              if(IsClockwise(dipoleData[cf].data[candidate].angle,coilAngle,360))
              {
                if (dipoleData[cf].data[candidate].instAngularVelocity>=0) //if it is going in the opposite direction
                {
                  if (invertPush)
                    fireElectro(cf,tempCandidate);
                }
                else
                {
                  if(!invertPush)
                     fireElectro(cf,tempCandidate);  //to increase speed, because the force will be towards the coil              
                }             
                  
                 
              }
              else  //if the angle is negaative, then
              {
                if (dipoleData[cf].data[candidate].instAngularVelocity<=0) //if it is going twoards the coil
                {
                  if(invertPush)
                    fireElectro(cf,tempCandidate);
                }
                else
                {
                  if(!invertPush)
                    fireElectro(cf,tempCandidate);
                }
                  // fireElectro(cf);
              }
              // fireElectro();
            }            
          }
          else
            fireElectro(cf,tempCandidate);
          #endif
        #endif
      }
      /////////////
      #ifdef GRAPHS_ENABLED
        if(dipoleRec==true)
        {
          // cout<<endl<<"DID SOMETHING";
          static long cfRe=0;
          long cf=dipoleData.size()-1;  //last frame
          long t=(cf-cfRe);
          if(t>=1000)
          {
            cfRe=cf;
            clearGraph();
            t=0;
            // pls->bop();
            // pls->adv(1);
            // pls->clear();
            // pls->adv(2);
            // pls->clear();
          }
            
          for(int i=0;i<dipoleData[cf].count;i++)
          {
            if(dipoleData[cf].data[i].detected)
            {
              pls->adv(1);
              
              pls->vpor( 0.0, 1.0, 0.0, 1.0 );
              pls->wind( -2.5, 2.5, -3.0, 3.0 );
              pls->w3d(2,4,3,0,10,0,1000,0,360,45,30);
              double x = dipoleData[cf].data[i].id;
              double z = dipoleData[cf].data[i].angle;
              // double x = i;
              // double z=i;
              double y = t;
              pls->col0((i+1)%10);
              pls->poin3(1,&x, &y, &z,1);
              


              pls->adv(2);
              pls->vpor( 0.0, 1.0, 0.0, 1.0 );
              pls->wind( -2.5, 2.5, -3.0, 3.0 );
              pls->w3d(2,4,3,0,10,0,1000,-360,360,45,30);
              // x = dipoleData[cf].data[i].id;
              z = dipoleData[cf].data[i].instAngularVelocity;
              // double x = i;
              // double z=i;
              // y = cf;
              pls->col0((i+1)%10);
              pls->poin3(1,&x, &y, &z,1);
            }
          }

          pls->adv(3);
          pls->stripa(id1,0,cf,minAngularVelocity);
          //NOTE: The library apparently has issues if you stop printing both! it crashes at runtime :(
          if(plotSqKE)
            pls->stripa(id1,1,cf,(dipoleData[cf].meanSquaredAngularVelocity));
          else
            pls->stripa(id1,1,cf,(dipoleData[cf].temperature));
          
        }        
      #endif

      //////////////DRAWING THE CONTOUR AND DIPOLE
      /// Draw contours + rotated rects + ellipses
      drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
      for( size_t i = 0; i< contours.size(); i++ )
         {
           // Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
          Scalar color = Scalar(0,0,255 );
           // contour
           drawContours( drawing, contours, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point() );

           // ellipse
           if(i<minEllipse.size())
  			       ellipse( drawing, minEllipse[i], color, 2, 8 );

           // rotated rectangle
           // Point2f rect_points[4]; minRect[i].points( rect_points );
           // for( int j = 0; j < 4; j++ )
           //    line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
          // }


          // int xx=dipoles[k][i].x;
          // int yy=dipoles[k][i].y;
          // int theta=dipoles[k][i].angle;

          // line(drawing, Point2f(xx,yy),Point2f(xx + 5*cos(theta), yy + 5*sin(theta)), Scalar(0,0,255),1,8);

         }

      for( int i=0;i<dipoles[0][0].count[k];i++)
      {

        int xx=dipoles[k][i].x;
        int yy=dipoles[k][i].y;
        double theta = (3.1415926535/180) * dipoles[k][i].angle;

        line(drawing, Point2f(xx - 5*cos(theta), yy - 5*sin(theta)),Point2f(xx + 5*cos(theta), yy + 5*sin(theta)), Scalar(0,255,255),5,8);



        // Use "y" to show that the baseLine is about
        char text[30];
        // dipoles[0][0].count[0]=1;
        // sprintf(text,"%f",dipoles[0][dipoles[0][0].count[k]-1].angle);
        // sprintf(text,"%f",dipoleData[dipoleData.size()-1].meanSquaredAngularVelocity);
        int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontScale = 0.5;
        int thickness = 1;

        int baseline=0;
        Size textSize = getTextSize(text, fontFace,
                                    fontScale, thickness, &baseline);
        baseline += thickness;

        // center the text
        Point textOrg((drawing.cols - textSize.width)/2,
                      (drawing.rows + textSize.height)/2);
          // // draw the box
          // rectangle(drawing, textOrg + Point(0, baseline),
          //           textOrg + Point(textSize.width, -textSize.height),
          //           Scalar(0,0,255));
          // // ... and the baseline first
          // line(drawing, textOrg + Point(0, thickness),
          //      textOrg + Point(textSize.width, thickness),
          //      Scalar(0, 0, 255));

          // then put the text itself
          // putText(drawing, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8);
        sprintf(text,"%1.1f",dipoles[k][i].angle);
        putText(drawing, text, Point(dipoles[k][i].x,dipoles[k][i].y), fontFace, fontScale, Scalar::all(0), thickness*3, 8);
        putText(drawing, text, Point(dipoles[k][i].x,dipoles[k][i].y), fontFace, fontScale, Scalar::all(255), thickness, 8);

        
        sprintf(text,"%d,%d",dipoles[k][i].id,i);

        if(dipoleRec==true)
        {
          if(dipoles[k][i].id < seedDipole.data.size())
            sprintf(text,"[%d],%d",seedDipole.data[dipoles[k][i].id].id,i);
        }
          

        putText(drawing, text, Point(dipoles[k][i].x,dipoles[k][i].y-10), fontFace, fontScale, Scalar::all(0), thickness*3, 8);
        putText(drawing, text, Point(dipoles[k][i].x,dipoles[k][i].y-10), fontFace, fontScale, Scalar(255,255,0), thickness, 8);

        //DEBUG ONLY
        if(i==0)
        {
          cimg = Mat(src.rows,src.cols+500, CV_8UC3, Scalar(0,0,0));           
          // sprintf(text,"%1.1f",dipoles[k][i].angle);          
          sprintf(text,"Press p to seed");
          // sprintf(text,"%1.1f",dipoleData[dipoleData.size()-1].data[0].instAngularVelocity);

          // if(dipoleData[dipoleData.size()-1].meanSquaredAngularVelocity >0)
          if(dipoleRec==true)
            sprintf(text,"%1.1f",dipoleData[dipoleData.size()-1].meanSquaredAngularVelocity);
          putText(cimg, text, Point(src.rows/4,src.cols/4), fontFace, fontScale*12, Scalar::all(255), thickness*4, 8);        
        }
      }
      

      ///////////////////////////    
      //  THIS IS HOUGH
      ///////////////////////////
      // // cvtColor(img, cimg, CV_GRAY2BGR);
      // // cimg=src_gray;
      // // Mat cimg();
      // Mat cimg(src.rows,src.cols, CV_8UC3, Scalar(255,255,255));

      // vector<Vec3f> circles;
      // HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, 10,
      //              canny, centre, minMinorAxis, maxMajorAxis // change the last two parameters
      //                             // (min_radius & max_radius) to detect larger circles
      //              );

      // // src_gray:s Input image (grayscale)
      // // circles: A vector that stores sets of 3 values: x_{c}, y_{c}, r for each detected circle.
      // // CV_HOUGH_GRADIENT: Define the detection method. Currently this is the only one available in OpenCV
      // // dp = 1: The inverse ratio of resolution
      // // min_dist = src_gray.rows/8: Minimum distance between detected centers
      // // param_1 = 200: Upper threshold for the internal Canny edge detector
      // // param_2 = 100*: Threshold for center detection.
      // // min_radius = 0: Minimum radio to be detected. If unknown, put zero as default.
      // // max_radius = 0: Maximum radius to be detected. If unknown, put zero as default

      // for( size_t i = 0; i < circles.size(); i++ )
      // {
      //     Vec3i c = circles[i];
      //     // Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );        
      //     Scalar color = Scalar( 255,255,0 );        
      //     circle( cimg, Point(c[0], c[1]), c[2], color, 3, CV_AA);
      //     circle( cimg, Point(c[0], c[1]), 2, color, 3, CV_AA);
      // }

      // imshow("Debug", cimg);

      #ifndef MULTI_THREAD_DISPLAY
        updateDisplay();
      #endif
      
      #ifdef MULTI_THREAD_DISPLAY
        // drawnow=true;
        #ifndef ATOMIC_DISPLAY
          drawnow.unlock();
        #else
          updateDisplayRequested=true;  //Both are required to trigger an update
          // updateDisplayCompleted=false; //The request flag is used to indicate processing is done, the second indicates the same here
          //however, once the frame has been updated, the update flag avoids unecessary refreshing of the frames.
        #endif

      #endif
    

      //////////////////////
      // CLI
      //////////////////////
      char key = (char) waitKey(5); //delay N millis, usually long enough to display and capture input
      int kMax; //sorry, bad programming, but relatively desparate for results..
      switch (key)
      {
          #ifdef GRAPHS_ENABLED
            case '0':
            {
              plotSqKE=!plotSqKE;
              cout<<endl<<"Plot measure of inst. average kinetic energy:"<<plotSqKE;
              break;
            }
          #endif
          case 'o':
          {
            cout<<endl<<"Input the coil angle"<<endl;
            cin>>coilAngle;
            cout<<endl<<"CoilAngle updated to "<<coilAngle;
            break;
          }
          case 'i': //Initialize the indices of the seed
          { 
            //you've to use brackets in case if you want local variables!
            if(dipoleRec==true)
            {
              cout<<endl<<"Input the indices in order to calibrate "<<endl;            
              for (int newLocation=0;newLocation<seedDipole.data.size();newLocation++)
              {
                  int val;
                  cin>>val;
                  seedDipole.data[val].id=newLocation;
              }
            }
            else
            {
              cout<<endl<<"Start dipole recording using the key `p' and try again";
            }
            break;

          }
          case 'c':
            #ifdef GRAPHS_ENABLED
              clearGraph();
            #endif
            break;
          case 'C':
            mode=1;
            cout<<"Mouse will capture color now. Right click for one, left for the other"<<endl;
            break;
          case 'S':
            mode=0;
            cout<<"Screen crop mode selected. Mouse will capture start point at left click and the other point at right click"<<endl;
            break;
          case 'p':
            {
              cout<<"Look what you've done!"<<endl<<"Just kidding: This frame will be used as a seed"<<endl;
              dipoleRec=true; //Enable dipole recording
              seedDipole.data.clear();  //clear the data
              dipoleSkel tempDipole;  //create a temporary dipole skeleton
              k=dipoles[0][0].current;  //find the current buffer of dipoles detected (double buffered for possible multithreading)
              kMax=dipoles[0][0].count[k]; //find the number of dipoles detected in the last scan
  
              for(int c=0;c<kMax;c++)
              {
                tempDipole.x=dipoles[k][c].x; //Copy the relavent data from the dipole data collected into the temp dipole
                tempDipole.y=dipoles[k][c].y;
                tempDipole.angle=dipoles[k][c].angle;
                tempDipole.instAngularVelocity=0;
                tempDipole.detected=false;   //This is to ensure the dipole was detected, but for the seed frame, it is left false.
                tempDipole.id=c;
                seedDipole.data.push_back(tempDipole);  //Add the data in the seedframe's data stream
  
                seedDipole.order+=dipoles[k][c].order;  //to get teh average order
                if(c>0)
                {
                  seedDipole.order/=2.0;
                }
              }
              seedDipole.time=0; //Initial time is to be stored as zero
              dipoleData.push_back(seedDipole);
              break;
            }
          case 'w':
            {            
              cout<<endl<<"Writing angle vs time for the first dipole to file"<<endl;
              if(dipoleRec==true)
              {
                sprintf(fileName,"latticeAnalyserBeta_%d",getTickCount());
                pFile = fopen (fileName,"w");
                
                //Loop through all the frames
                for (vector<dipoleFrame>::iterator dD = dipoleData.begin() ; dD != dipoleData.end(); ++dD)
                {
                  //Within each frame, loop through all dipoles?
                  // for(vector<dipoleSkel>::iterator dS = dD.data.begin() ; dS!=dD.data.end() ; ++dS)
                  // {

                  // }
                  //or just print the first dipole
                  if(dD->data[0].detected)
                    fprintf (pFile, "%f,%f,%f\n",dD->data[0].angle,dD->time,dD->meanSquaredAngularVelocity);
                      //->data[0].instAngularVelocity);
                  // fprintf (pFile, "%d,%d\n",dD->data[0].angle,dD->time);
                }
                  

                // for (int p=0;p<dipoleData.size();p++)
                // {
                //   fprintf(pFile,"%d,%d\n",dipoleData[p].data.size(),dipoleData[p].time);
                // }
                fclose (pFile);
                // fprintf (pFile, "Name %d [%-10.10s]\n",n,name);

              }
              break;
            }
          case 'F':
          {
            // Table Description
            //  Time    Dipole 0    Dipole 1    Dipole 2 ...
            // cout<<endl<<"Writing angle of all dipoles for the given frame"
            
            if(dipoleRec==true)
            {
              cout<<endl<<"Writing all the data collected so far into a file";              
              sprintf(fileName,"latticeAnalyserRENAMEorLOSEme");
              pFile=fopen(fileName,"w");
              //Write the column names (very important to figure out which dipole corresponds to which spatial location)
              fprintf(pFile,"Time");
              for(int fi=0;fi<seedDipole.data.size();fi++)
              {
                fprintf(pFile,"\t Dipole %d",seedDipole.data[fi].id);
              }
              fprintf(pFile,"\t Mean Sq Ang Vel Per \t Temperature");
              fprintf(pFile,"\n");

              //Now write the data              
              for (vector<dipoleFrame>::iterator dD=dipoleData.begin(); dD!=dipoleData.end();dD++)
              {
                //The first entry is time
                fprintf(pFile,"%f",dD->time);
                for(vector<dipoleSkel>::iterator dData=dD->data.begin(); dData!=dD->data.end(); dData++)
                { 
                  //Second entry onwrds, we have the dipole angles
                  if(dData->detected==true)
                    fprintf(pFile,"\t %f",dData->angle);
                  else
                    fprintf(pFile,"\t");
                }
                fprintf(pFile,"\t %f \t %f",dD->meanSquaredAngularVelocity,dD->temperature);
                fprintf(pFile,"\n");
              }
              fclose (pFile);
              cout<<endl<<"Written! Unless something broke..";
            }
            else
              cout<<endl<<"Data recording is off. Turn it on using `p'.";
            break;
          }
          case 'b':
            //This is to make blind
            blind=!blind;
            cout<<"Blind:"<<blind<<endl;
            break;
          case 'd':
            useCalibration=!useCalibration;
            cout<<endl<<"Calibration Use:"<<useCalibration<<endl;
            break;
          case 'I':
            //invert push
            invertPush=!invertPush;
            cout<<"Push"<<invertPush<<endl;
            break;
          case 'W':
            {
              pFile=fopen("TestComputation","w");
              for(vector<double>::iterator d=computationTime.begin();d!=computationTime.end();++d)
              {
                fprintf(pFile,"%f\n",*d);
              }
              fclose(pFile);
              break;              
            }
          case 'q':
          case 'Q':
          case 27: //escape key
              destroyWindow(source_window);
              destroyWindow(filter_window);
              destroyWindow(settings_window);
              destroyWindow("Contours");
              destroyWindow("Debug");

              #ifdef TEMPERATURE_ENABLED
                  vCloseUSB();
              #endif 
              threadsEnabled=false;
              t1.join();
              #ifdef MULTI_THREAD_DISPLAY
                t2.join();
              #endif
              // destroyAllWindows();
              // this_thread::sleep_for( chrono::milliseconds(5000) );
              // waitKey(1000);
              return 0;
          // case ' ': //Save an image
          //     sprintf(filename, "filename%.3d.jpg", n++);
          //     imwrite(filename, frame);
          //     cout << "Saved " << filename << endl;
          //     break;
          default:
              break;
      }
      tCend=getTickCount();
      tCdelta=tCend-tCstart;
      computationTime.push_back(tCdelta/getTickFrequency());
    }    
    // processingImage.unlock();
  }
  #ifdef GRAPHS_ENABLED
    delete pls;
  #endif
  return 0;


}

// double distSq(int x1,int y1,int x2,int y2)
// {
//   return (pow(x1-x2,2) + pow(y1-y2,2));
// }
// void latticeAxis(vector<dipoleSkel>& data,vector<double>& angles)
// {
//   int lastDistance=10000;
//   // Calculating the shortest distance squared, between the first point and all other points
//   for(int i=1;i<data.size();i++)    
//   {    
//     int distance=pow((data[0].x-data[i].x),2) + pow((data[0].y-data[i].y),2);
//     lastDistance=MIN(distance,lastDistance);
//   }


//   double tolLower=0.5;
//   double tolHigher=1.5;
//   for(int i=0;i<data.size();i++)
//   {
//     for(int j=0;j<data.size();j++)
//     {
//       if(i!=j)
//       {
//         double neiDist=distSq(data[i].x,data[i].y,data[j].x,data[j].y)/(lastDistance)
//         if( neiDist > tolLower && neiDist<tolHigher ) //Yup, found a neighbour!
//         {
//           double angleFound=atan2( (data[i].y - data[j].y) / (data[i].x - data[j].x) );
//           angles.push_back(angleFound); //Added the angle found to an array
//         }
//       }
//     }
//   }
// }

// inline void latticeAxisTest()
// {
//   // Calculate shortest distance squared between the first point and all other pointss
//   // for each point, find all neighbours, store the angle determined in an array
//   // TODO: In the previous step, add a method to avoid incorrect counting
  
// }

// void latticeAxis(vector<dipoleSkel>& data)
// {
  //Find neighbours
  //Find the ones with only 3 neighbours
  //Within these, find the ones with a particular slope missing
  

// double a=3.14;  // The value you seek
// std::find_if(v.begin(),v.end(),[a](double b) { return a>b-epsilon && a<b+epsilon; });  

// }
#ifdef TEMPERATURE_ENABLED
#ifdef TEMPERATURE_SINGLDIPOLE
  inline void fireElectro(long frame,int id)
  {
    //this is to avoid too many fires within a short time
    static long lastFrame=0;
    // static bool alternate=false;
    if(frame-lastFrame>3)
    {
      // alternate=!alternate;
      // if(alternate)
      // {
        char usbBuf[REPORT_LEN]={dipolePort[id],dipoleBit[id],0,200};

        nWriteUSB((unsigned char*)usbBuf,14);
        lastFrame=frame;
        cout<<endl<<">> Temperature: Electro Fired for dipole "<<id;

      // }
    }
  }
#else
  void fireElectro(int id, int intensity)
  {
        char usbBuf[REPORT_LEN]={dipolePort[id],dipoleBit[id],0,intensity};

        nWriteUSB((unsigned char*)usbBuf,14);    
        // lastFrame=frame;
        cout<<endl<<">> Temperature: Electro Fired for dipole "<<id<<" with intensity "<<intensity<<" ";
  }
#endif
#endif

void temperatureTest()
{
#ifdef TEMPERATURE_ENABLED
  cout<<"Which dipole to fire?";
  int i=0;
  cin>>i;

  char usbBuf[REPORT_LEN];
  for(int q=0;q<REPORT_LEN;q++)
  {
    usbBuf[q]=0;
  }

  // usbBuf[0]='B';
  // usbBuf[1]=5;
  usbBuf[0]=dipolePort[i];
  usbBuf[1]=dipoleBit[i];
  usbBuf[2]=255;
  usbBuf[3]=255;


  cout<<"temperature Test"<<endl<<endl;  
  cout<<"Initializing Hardware"<<endl;
  vInitUSB();
  cout<<endl;
  // cout<<"Initialization Successful"<<endl<<endl;
  
  cout<<"Writing to hardware:"<<usbBuf<<endl;  
  int usbLen;
  if( (usbLen=nWriteUSB( (unsigned char *) usbBuf,14)) )
  {
    cout<<"Writing Successful"<<endl<<endl;
  }

  cout<<"Reading from hardware"<<endl;
  usbLen=nReadUSB( (unsigned char*) usbBuf);
  if(usbLen==0)
    cout<<"Failed!"<<endl<<endl;
  else
  {
    usbBuf[usbLen]= '\0';
    cout<<"Data Read: "<<usbBuf<<endl;
    for(int i=0;i<usbLen;i++)
      printf("%4d",usbBuf[i]);
    cout<<endl<<endl;
  }
  vCloseUSB();

#else
	cout<<"Temperature Support not enabled";
#endif
}

/**
 * @function main
 */
int main( int ac, char** argv )
{
  initializeMultithreadResources();
  #ifdef TEMPERATURE_ENABLED
    getTemperaturePins();
  #endif

  cout<<"Loading";
    cout<<endl<<endl
        <<"Lattice Analyser | version "<<version<<endl
        <<"------------"<<endl
        <<"Created at the National Physical Laboratory, New Delhi"<<endl            
        <<endl
        <<"Project Repository Folder: github.com/toAtulArora/IISER_repo/Summers_2013/NPL"<<endl    
        <<endl
        <<"For help type"<<endl
        <<"help"<<endl
        <<"(Like you couldn't guess!)"<<endl<<endl
        <<"\t now what?  ";


  for(;;)
  {
    string a;
    cin>>a;

    if(!a.compare("help"))
    {
      cout<<endl; //for multi line, beauty stuff
      
      cout<<"Command \t Description"<<endl
      <<"------- \t -----------"<<endl
      <<"temp    \t Launches hardware test"<<endl
      <<"temperature \t same as temp"<<endl      
      <<"<number> \t Initiates analysis of dipoles using the corresponding camera"<<endl
      <<"q        \t exit or quit"<<endl;

      cout<<endl; //again for multi line console outputs, to maintain beuty
    }
    else if(!a.compare("exit") || !a.compare("quit") || !a.compare("q"))
    {
      break;
    }
    else if(!a.compare("temperature") || !a.compare("temp"))
    {
      temperatureTest();
    }
    else 
    {
      // if(atoi(a.c_str())!=0 || !a.compare("0"))
      cout<<"Commands for this mode:"<<endl
      <<"c \t Will clear the graphs"<<endl
      <<"C \t Enable mouse capture of colour"<<endl
      <<"S \t Screen crop is selected, viz Drag with left click to select a sub window"<<endl
      <<"p \t This frame will be selected as the seed frame"<<endl
      <<"w \t Write the angles and dipoles, of the first dipole to file"<<endl
      <<"W \t Write computation times to file"<<endl<<endl;

      threadsEnabled=true;
      getCameraCalibrationParameters();
      VideoCapture capture; //try to open string, this will attempt to open it as a video file
      // if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
      
      capture.open(a);
      if(!capture.isOpened())
      {
          capture.open(atoi(a.c_str()));  
      }
      
      if (capture.isOpened())
      {
        process(capture);

      }
      else
      {
          cerr << "Failed to open the video device specified" << endl;      
          // return 1;
      }
    }
    // else if(!a.compare("latticeAxis"))
    // {
    //   latticeAxisTest();
    // }
    cout<<endl<<"\t now what? ";
  }



  return 0;
}
