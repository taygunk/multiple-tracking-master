#ifndef BLOB_ENGINE_H
#define BLOB_ENGINE_H
#include "Environment.h"
#include "Hypothesis.h"
#include "BgSubEngine.h"
#include <cxcore.h>
#include <highgui.h>
#include <iostream>
#include <stdio.h>

#define NO_REGION -1

class BlobDetectionEngine{
	//Variables
public:
	CvFont font;
	char text[50];
	IplImage *visualImg; // Output Image for drawing
	//Blobs
	CvPoint cent1;
	CvPoint cent2;
	CvMemStorage *mem;
	static const int maxBlobNum = 200;
	CvSeq *contours, *ptr;
	int numOfFiltCont;
	CvSeq **filtCont;
	//Regions
	CvRect regRect[30];
	short regionList[maxBlobNum];
	int numOfRegions;
	float r1, r2;
	int minAreaFilter;
	//Hypothesis and Correspondence
	vector<Hypothesis> hyp_t;
	vector<Hypothesis> hyp_t_1;
	float **corMatrix;	// Correspondence Matrix
	int curHypNum;  // at time t  , we have some new hypothesis
	int preHypNum;	// at time t-1, we have some old hypothesis
	CvScalar colors[10];
	//Methods
private:
	FILE *trackFp;
	bool hypClicked;
	bool hypUpdated;
	int tx,ty;
public:
	int frameIdx;
	BlobDetectionEngine(IplImage *visualImg);
	void setColors();
	void fillZero(float **mat, int size);
	void userClick(bool *hypClick, bool *hypUpdate, int *tx, int *ty);
	double BlobDetectionEngine::find_eu_distance(const CvPoint& point_one, const CvPoint& point_two);
	void setAreaConstraint(int area);
	void findBlobs(IplImage *grayImg, bool drawBlobs);
	void blobToRegions();
	//Experimental Methods {Ongoing Research}
	void runMatching(bool **bgMask, int ****bins, int *time); //Update color needs intel from background subtraction histogram bins
	void fillColorToRegion(int regionId, float *hist1, float *hist2); //Paint all pixels to Blue(hist1match) or Red(Hist2match)
	void BlobDetectionEngine::alternateMatching(bool **bgMask, int ***bins);
	void BlobDetectionEngine::createHypothesis(bool **bgMask, int ****bins, int *time);
	void BlobDetectionEngine::createHypothesis(bool **bgMask, int ***bins);
	void BlobDetectionEngine::runBasicMatching(bool **bgMask, int ****bins, int *time);
	int findIdxOfHyp(int idd);
	void eventResolve();
	void setLogger(FILE *logger);
	//void pixelClassEvaluate(bool **bgMask, int ****bins, int *time);
	void BlobDetectionEngine::pixelClassEvaluate(bool **mask, int ***bins);
	void printIterator (int i);
	void BlobDetectionEngine::presentate(IplImage *hypOnly);
	void BlobDetectionEngine::debugPixel(int y, int x, int ***bins);
};

#endif