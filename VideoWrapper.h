#ifndef VIDEO_WRAPPER_H
#define VIDEO_WRAPPER_H
#include "Environment.h"
class VideoWrapper{

private:
	// OpenCV VideoFile 
	char filename[255];	// Videofle's name and extension
	char filenameBuffer[255];
	CvFont font;		// OpenCV Font for appending textual information to image
	int fps;
	// OpenCV Frame Data
public:
	IplImage *img, *img2, *visualImg, *bufferImg;
	int width;
	int height;
	int frameIdx;
	int numFrames;
	CvCapture* capture;	// OpenCV VideoWrapper
	CvVideoWriter* recCapture; // OpenCV Video Recorder
	bool multiOutput;
	//Methods
	VideoWrapper(int argc, char**argv);
	// Initialize frames
	char* getVideoFileName();
	void passNFrames(int numOfFrames);
	// Start Recording
	void setMultipleOutput(); // Will output 4 videos
	void WriteOutputChannel(int ind, IplImage *img); // Will write to channel X, (between 0-3)
	void startRecord(char* filename, int fps = NULL , int isColor = NULL);
	void WriteFrame(IplImage *img);
	void finishRecord();
};
#endif