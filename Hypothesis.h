#ifndef H_HYPOTHESIS_H
#define H_HYPOTHESIS_H
//Spatial Possibilities
#define MAX_VELOC 50		// Teleportation of objects are not invented yet for 21th century.

#define HYP_TRACKED 0
#define HYP_OCCLUDED 1
#define MAX_DIST_ALLOWED 100	// at most 100px distance for match.
#define MAX_FRAME_OCCLUDED 75  // at most 100frame, the object is alive.

#include <cmath>
#include <cxcore.h>
#include <iostream>
#include <vector>
#include <set>
using namespace std;

#if defined (__WIN32__)
  // Windows time stuff
else
  // Linux time Stuff
#endif

#define SINGLE 0
#define INTERACTING 1
#define MERGED 2


class Hypothesis
{
public:
	static int objectNumber;
	static int img_width;
	static int img_height;

public:
	int id;
	int *trajectory;
	CvScalar trajColor;
	int time;						// Keep track of time for trajectory printing
	float ex_x0, ex_y0, ex_w, ex_h; // t-1 positions, Boundaries
	float x0, y0, width, height;    // t positions, Boundaries
	float ex_vx, ex_vy,vx,vy;		// t velocities
	float ex_ang, ang;				// angle of velocities
	float velDirection;				// encoded velocity with (8 dog of angle)
	float velMag;					// encoded velocity with magnitude( <= MAX_VELOC)
	float area;				        // Computed with width * height;
	float *histogram;
	float pxInHistogram;			// # of pixels in histogram ( normalizer)
	float histSum;
	int state;					  // {being HYP_TRACKED, HYP_OCCLUDED, removed}
	int lastSeenIndex;			  // How many frames ago we see this hypothesis last time ?
	bool hasMatch;
	float innerShellRadius;
	int spatialId;
	bool **shellMask;

	short objectState;
	short killed;
	//vector<int> intEvtList;
	//vector<int> merEvtList;
	set<int> intEvtSet;
	set<int> merEvtSet;
	set<int> classUnionSet;

public:
	Hypothesis();
	void appear(Hypothesis & h);			// When hypothesis first appeared at the scene, call this method.
	void disappear();						// When hypothesis is not seen in the scene, call this method. {Occlusion or scene exit}
	void updateColor(bool **mask, int ****pixHist, int *t);	// When hypothesis' color histogram needs to be updated, call this method (On creation and after split/merge)
	void updateColor(bool **mask, int ***bins);
	float match(Hypothesis & t1);			// When a hypothesis_t matches with hypothesis_t+1, call this method with {ONLY ONE TO ONE SUPPORTEDNOW}
	float matchHist_Bhattarcharya_Coefficient(Hypothesis &t1);		
	void updateCentroid(bool **mask);
	//Debugging
	void printTrack(IplImage *img);			// Hypothesis' track will be printed
	int getAvailableHypothesisNumber();
	int getPrintTrackColor();
	bool isInteracting(int *interactArray);
	bool isMerged(int *mergeArray);
	bool hasInteract();
	bool hasMerge();
	void updatePixelsCentroid(bool **mask);
	bool **create2dKernelBool(int xSize, int ySize, bool scalar);

};

#endif