#ifndef RG_ENVIRONMENT_H
#define RG_ENVIRONMENT_H
//Libraries
#include <iostream>
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include <ctime>
using namespace std;
//Environment Declarations
#define caviarFile1 ("../../../data/caviar/LeftBag.mpg")
#define caviarFile2 ("../../../data/caviar/Walk2.mpg")
#define useCaviarDb FALSE
#define useShahDb TRUE
#define useTestDb TRUE
#define testDbFile1 ("../../../data/testDatabase/sample10x10.mp4")
#define testDbFile2 ("../../../data/testDatabase/sample50x10.mp4")
#define testDbFile3 ("../../../data/testDatabase/sample50x50.mp4")
#define testDbFile4 ("../../../data/testDatabase/sample100x50.mp4")
#define testDbFile5 ("../../../data/testDatabase/sample100x100.mp4")
#define testDbFile6 ("../../../data/testDatabase/sample200x100.mp4")
#define testDbFile7 ("../../../data/testDatabase/sample320x240.mp4")
#define testDbFile8 ("../../../data/testDatabase/BALLS.avi")
#define testDbFile9 ("../../../data/akvaryum/320x240/video1_cam67.avi")
#define testDbFile10 ("../../../data/testDatabase/pets2001/camera1.avi")
#define testDbFile11 ("../../../data/testDatabase/pets2001/camera2.avi")
#define shahDbFile1 ("../../../data/testDatabase/shah/carscene.avi")
#define shahDbFile2 ("../../../data/testDatabase/shah/fountainscene.avi")
#define shahDbFile3 ("../../../data/testDatabase/shah/ducks.avi")
// Debugging Declarations
#define DEBUG_FLAG false


// Function Declarations
float** create2dKernel(int xSize, int ySize);
bool** create2dKernelBool(int xSize, int ySize, bool scalar);
int** create2dKernel(int xSize, int ySize, int scalar);
void delete2dKernel(float** kernel, int xSize);
int*** create3dKernel(int numOfKernel, int xSize, int ySize);
int**** create4dKernel(int numOfKernel, int xSize, int ySize, int lastSize);
float***** create5dKernel(int numOfKernel, int xSize, int ySize, int lastSize, int lastlastSize);
void delete3dKernel(float*** kernel, int numOfKernel, int xSize);
float round(float d);
#endif