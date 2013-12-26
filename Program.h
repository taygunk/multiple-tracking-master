#ifndef H_PROG_H
#define H_PROG_H

#include "Environment.h"
#include "VideoWrapper.h"
#include "Hypothesis.h"
#include <stdio.h>
#include <cmath>
#include <vector>
#include <highgui.h>
#include "BgSubEngine.h"
#include "BlobDetectionEngine.h"
#include "AdaptiveMedianBGS.hpp"
#include "GrimsonGMM.hpp"
#include "ZivkovicAGMM.hpp"
#include "MeanBGS.hpp"
#include "WrenGA.hpp"
#include "PratiMediodBGS.hpp"
#include "Eigenbackground.hpp"


FILE *trackFp;

bool humanTrack = true;
bool autoTrack = false;
bool debugFlag = false;
bool videoOutput = true;
bool autoRemoveHyp = true;

bool hypClicked = false;
bool hypUpdated = false;

BlobDetectionEngine *beng;
int ***colorzz;
int tx,ty;
#pragma warning ( disable : 4800 ) 
enum RESULT_TYPE { IMAGE_DMS, IMAGE_SM_WALLFLOWER, VIDEO };
void mouseHypothesisCreate(int event, int x, int y, int flags, void* param)
{
	if (event == CV_EVENT_LBUTTONDOWN && (flags & CV_EVENT_FLAG_CTRLKEY)){
		if ( humanTrack ){
			cout << "-------------------" << endl;
			//cout << "Pixel click at x y " << x << " " << y << endl;
			tx = x;
			ty = y;
			beng->debugPixel(y,x , colorzz);
			cout << "-------------------" << endl;
			
		}
	}
	else if (event == CV_EVENT_LBUTTONDOWN){
		if ( humanTrack ){
			cout << "mouse click at x y " << x << " " << y << endl;
			hypClicked = true;
			tx = x;
			ty = y;
			
		}
	}
	else if (event == CV_EVENT_RBUTTONDOWN){
		if ( humanTrack ){
			cout << "update click at x y " << x << " " << y << endl;
			hypClicked = false;
			hypUpdated = true;
			tx = x;
			ty = y;
			
		}
	}
}

void balls_video_autoclicks(VideoWrapper *vid)
{
	if ( vid->frameIdx == 352){
		tx = 57;
		ty = 103;
		hypClicked = true;
	}
	if ( vid->frameIdx == 358){
		tx = 69;
		ty = 32;
		hypClicked = true;
	}
	if ( vid->frameIdx == 367){
		tx = 89;
		ty = 47;
		hypClicked = true;
	}
	if ( vid->frameIdx == 368){
		tx = 77;
		ty = 86;
		hypClicked = true;
	}
	if ( vid->frameIdx == 370){
		tx = 143;
		ty = 67;
		hypClicked = true;
	}
}


void pets2_video_autoclicks(VideoWrapper *vid)
{
	if ( vid->frameIdx == 241){
		tx = 52;
		ty = 136;
		hypClicked = true;
	}	
	if ( vid->frameIdx == 468){
		tx = 296;
		ty = 209;
		hypClicked = true;
	}
	if ( vid->frameIdx == 691){
		tx = 11;
		ty = 135;
		hypClicked = true;
	}	
}
void aqua2_video_autoclicks(VideoWrapper *vid)
{
	if ( vid->frameIdx == 2010){
		tx = 103;
		ty = 49;
		hypClicked = true;
	}
	if ( vid->frameIdx == 2011){
		tx = 58;
		ty = 55;
		hypClicked = true;
	}
	if ( vid->frameIdx == 2012){
		tx = 165;
		ty = 36;
		hypClicked = true;
	}	
	if ( vid->frameIdx == 2031){
		tx = 18;
		ty = 116;
		hypClicked = true;
	}	
	if ( vid->frameIdx == 2032){
		tx = 40;
		ty = 203;
		hypClicked = true;
	}
	if ( vid->frameIdx == 2033){
		tx = 59;
		ty = 172;
		hypClicked = true;
	}
	if ( vid->frameIdx == 2136){
		tx = 54;
		ty = 73;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 2139){
		tx = 192;
		ty = 155;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 2142){
		tx = 125;
		ty = 38;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 2152){
		tx = 33;
		ty = 121;
		hypUpdated = true;
	}
}


void aqua1_video_autoclicks(VideoWrapper *vid)
{
	if ( vid->frameIdx == 902){
		tx = 161;
		ty = 107;
		hypClicked = true;
	}
	if ( vid->frameIdx == 903){
		tx = 147;
		ty = 160;
		hypClicked = true;
	}
	if ( vid->frameIdx == 950){
		tx = 278;
		ty = 123;
		hypClicked = true;
	}
	if ( vid->frameIdx == 1095){
		tx = 295;
		ty = 94;
		hypClicked = true;
	}	
	if ( vid->frameIdx == 1098){
		tx = 266;
		ty = 16;
		hypClicked = true;
	}
	if ( vid->frameIdx == 1198){
		tx = 176;
		ty = 77;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 1199){
		tx = 136;
		ty = 148;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 1273){
		tx = 195;
		ty = 113;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 1274){
		tx = 244;
		ty = 103;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 1360){
		tx = 225;
		ty = 58;
		hypUpdated = true;
	}
	if ( vid->frameIdx == 1650){
		vid->finishRecord();
		exit(0);
	}
}


void caviarFight_video_autoclicks(VideoWrapper *vid)
{
	if ( vid->frameIdx == 210){
		tx = 235;
		ty = 134;
		hypClicked = true;
	}
	if ( vid->frameIdx == 211){
		tx = 204;
		ty = 84;
		hypClicked = true;
	}
	if ( vid->frameIdx == 900){
		vid->finishRecord();
		exit(0);
	}
}

void engineMain(int argc, char* argv[])
{
	trackFp = fopen("trackingLog.txt", "w+");
	static int learnSec = 8;
	static int learnFrameNum = learnSec * 30;
	int width, height;
	int colorBin = 16;
	int xSpaceBin = 25;
	int ySpaceBin = 25;
	/* Parameter Stuff */
	if ( argc > 2)
		learnFrameNum = atoi( argv[2] ) * 30;
	if ( argc > 5){
		cout << "set colorBin to " << atoi( argv[3] ) << " xSpaceBin to : " << atoi( argv[4] ) << " ySpaceBin to : " << atoi( argv[5] ) << endl;
		colorBin = atoi( argv[3] );
		xSpaceBin = atoi( argv[4] );
		ySpaceBin = atoi( argv[5] );
	}
	/* Variables Stuff */
	VideoWrapper *vid = new VideoWrapper(argc, argv);
	vid->setMultipleOutput(); // We ll see a video with 4 videos...
	vid->startRecord("EngineMain.avi");
	IplImage *img, *visualImg, *grayImg, *presentationImg;
	char key = 'u';
	bool wait;
	int shortDelay = 20;
	int longDelay = 200000;
	width = vid->width; height = vid->height; img = vid->img; visualImg = vid->visualImg;
	clock_t startAll, endAll; double durationAll;	
	grayImg = cvCreateImage( cvSize( img->width, img->height), 8, 1);
	presentationImg = cvCreateImage( cvSize( img->width, img->height), 8, 3);
	cvNamedWindow("MaskImage",CV_WINDOW_AUTOSIZE );
	cvSetMouseCallback("MaskImage", mouseHypothesisCreate, NULL);

	//Train or rememorize background
	BgSubEngine *bgsub = new BgSubEngine(img->width, img->height, colorBin, xSpaceBin, ySpaceBin, visualImg);
	bgsub->useBGandFGmodel(0.5);
	char train[255];
	sprintf(train, "%s", vid->getVideoFileName());
	sprintf(&train[strlen(vid->getVideoFileName() ) - 4], "%s", ".bin");

	/*Zivkovic GMM variables */
	// setup marks to hold results of low and high thresholding
	BwImage low_threshold_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	low_threshold_mask.Ptr()->origin = IPL_ORIGIN_BL;
	RgbImage frame_data;
	frame_data.ReleaseMemory(false);	// AVI frame data is released by with the AVI capture device

	BwImage high_threshold_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	high_threshold_mask.Ptr()->origin = IPL_ORIGIN_BL;



	
	/*
	Algorithms::BackgroundSubtraction::GrimsonGMM bgs;
	Algorithms::BackgroundSubtraction::GrimsonParams params;
	params.SetFrameSize(width, height);
	params.LowThreshold() = 3.0f*3.0f;
	params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
	params.Alpha() = 0.001f;
	params.MaxModes() = 5;
	*/

	/* zivkovic gmm */
	Algorithms::BackgroundSubtraction::ZivkovicAGMM bgs;
	Algorithms::BackgroundSubtraction::ZivkovicParams params;
	params.SetFrameSize(width, height);
	params.LowThreshold() = 1.4f*1.4f;
	params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
	params.Alpha() = 0.001f;
	params.MaxModes() = 1;

	bgs.Initalize(params);
	frame_data = img;
	bgs.InitModel( frame_data);
	bgs.Subtract(0, frame_data, low_threshold_mask, high_threshold_mask);
	IplImage* flipped = cvCreateImage( cvSize(width, height), 8, 1);
	bool shahBGS = true;
	if (shahBGS){
		cout << "Searching cache file : " << train << endl;
		FILE *fp = fopen(train, "r");
		if ( fp == NULL){
			cout << "No cached learning frames has been found. Algorithm will learn input frames. " << endl;
			//Learn background model from video and cache the learned frames
			for(int i = 0; i < learnFrameNum ; i++){
				cvShowImage("MaskImage", img);
				bgsub->learnFrames(img);
				img = cvQueryFrame( vid->capture );
				vid->frameIdx++;
				cvWaitKey(5);
			}
			bgsub->serialize(train);
		}
		else{
			fclose(fp);
			cout << "Rememorizing cached (already learned) " << learnFrameNum << " frames " << endl;
			bgsub->deserialize(train);
			bgsub->learnFromFile(learnFrameNum);
			//img = cvQueryFrame( vid->capture );
			cvSetCaptureProperty( vid->capture, CV_CAP_PROP_POS_FRAMES , learnFrameNum);
			vid->frameIdx += learnFrameNum;
		}
	}
	else{
		for(int i = 1; i < learnFrameNum ; i++){
			img = cvQueryFrame( vid->capture );
			cout << "Learning frame: " << vid->frameIdx << endl;
			cvShowImage("MaskImage", img);
			frame_data = img;
			bgs.Subtract(i, frame_data, low_threshold_mask, high_threshold_mask);
			vid->frameIdx++;
			cvWaitKey(5);
		}
	}

	beng = new BlobDetectionEngine(visualImg);
	beng->setAreaConstraint(75);
	beng->setLogger(trackFp);
	beng->frameIdx = vid->frameIdx;


	startAll = clock();
	bool **maskPointer ;
	bool **custommasK = create2dKernelBool(height, width, 0) ;
	colorzz = create3dKernel(img->height, img->width, 3);
	while ( vid->numFrames > ( vid->frameIdx + 5) && key != 'q' )
	{	
		if ( !img)
			break;

		vid->WriteOutputChannel(0, img); //Store original image
		cout << "Processing frame: " << vid->frameIdx << endl;
		//Background Subtraction
		if  (shahBGS){
			bgsub->run(img, visualImg);
			maskPointer = bgsub->mask;
			for(int i = 0; i < img->height; i++){
				for(int j = 0; j < img->width ; j++)
				{
					
					colorzz[i][j][0] = bgsub->bins[bgsub->t][i][j][1];
					colorzz[i][j][1] = bgsub->bins[bgsub->t][i][j][2];
					colorzz[i][j][2] = bgsub->bins[bgsub->t][i][j][3];
				}
			}
			//Median Filter
			cvSmooth(visualImg, visualImg, CV_MEDIAN);
			//UNCOMMENT AFTER CLEARING OUT THIS
			bgsub->ImageToMask(visualImg, bgsub->mask);
		}
		else{
			
			//GMM
			frame_data = img;
			bgs.Subtract(vid->frameIdx, frame_data, low_threshold_mask, high_threshold_mask);
			for(int row = 0; row < height; row++){
				for(int col=0; col < width; col++){
					flipped->imageData[ (row)*flipped->widthStep+col] = low_threshold_mask(row, col);
					if ( flipped->imageData[ (row)*flipped->widthStep+col] ){
						cvSet2D( visualImg, row , col, cvScalar(255,255,255,0));
					}
					else{
						cvSet2D( visualImg, row , col, cvScalar(0,0,0,0) );
					}
				}
			}	
			cvSmooth(visualImg, visualImg, CV_MEDIAN);
			for(int row = 0; row < height; row++){
				for(int col=0; col < width; col++){
					if ( cvGet2D(visualImg, row,col).val[0] ){
						custommasK[row][col] = 1;
					}
					else{
						custommasK[row][col] = 0; 
					}
				}
			}	

			maskPointer = custommasK;		
			for(int i = 0; i < img->height; i++)
				for(int j = 0; j < img->width ; j++){
					colorzz[i][j][0] = round((cvGet2D(img, i,j).val[0] / 255.0) * ( colorBin - 1 )) ; 
					colorzz[i][j][1] = round((cvGet2D(img, i,j).val[1] / 255.0) * ( colorBin - 1 ));
					colorzz[i][j][2] = round((cvGet2D(img, i,j).val[2] / 255.0) * ( colorBin - 1 )) ;
				}
		}

		vid->WriteOutputChannel(1, visualImg); //Store BGSub result
		//Connected Components
		cvCvtColor(visualImg, grayImg, CV_BGR2GRAY);
		cvThreshold(grayImg, grayImg, 127, 255, CV_THRESH_BINARY); 
		//Blob Detection
		beng->findBlobs(grayImg, true);
		//Region Detection
		beng->blobToRegions();
		//vid->WriteOutputChannel(2, visualImg); //Store Regional result

		//balls_video_autoclicks(vid);
		//pets2_video_autoclicks(vid);
		//aqua2_video_autoclicks(vid);
		//aqua1_video_autoclicks(vid);
		//caviarFight_video_autoclicks(vid);

		beng->userClick(&hypClicked, &hypUpdated, &tx, &ty );
		beng->alternateMatching(maskPointer, colorzz);
		hypClicked = false;

		vid->WriteOutputChannel(2, visualImg); //Write Algorithm Debug Output totally
		//Prepare presentationImg
		cvCopy(img, presentationImg);
		beng->presentate(presentationImg);
		vid->WriteOutputChannel(3, presentationImg); //Write presentation Output totally

		img = cvQueryFrame( vid->capture );
		cvShowImage("MaskImage", visualImg);	
		cvShowImage("realImage", presentationImg);	
		if ( key == 'p') {
			key = cvWaitKey(longDelay);
			wait = true;
		}
		else if ( key =='u'){
			key = cvWaitKey(shortDelay);
			wait = false;
		}

		if ( wait ){
			cvWaitKey(longDelay);
		}
		else{
		cvWaitKey(shortDelay);
		}
		/* Output */

		vid->frameIdx++;
		beng->frameIdx = vid->frameIdx;

		if ( videoOutput )
		vid->WriteFrame(visualImg);


	}

	bgsub->finalize();
	vid->finishRecord();
	endAll = clock();
	durationAll = double((endAll-startAll)) / CLOCKS_PER_SEC;
	cout << "Average (FPS):   " << (vid->frameIdx - learnFrameNum ) / durationAll << endl;
	vid->finishRecord();
	fclose(trackFp);
}

#endif