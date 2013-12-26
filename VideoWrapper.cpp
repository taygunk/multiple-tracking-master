#include "VideoWrapper.h"
VideoWrapper::VideoWrapper(int argc, char**argv)
{
	multiOutput = false;
	/* Dataset Settings and Dynamic Load of Video */
	if ( argc < 2 && useCaviarDb){
		cout << "Using Caviar Dataset" << endl;
		strncpy(filename, caviarFile1, strlen(caviarFile1)+1 );
		cout << "File : " << caviarFile1 << endl;
		capture = cvCaptureFromAVI(filename);
		fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		if (capture == NULL || fps < 5) {printf("Video not found [TERMINATION].\n"); return; }
	}
	else if (argc < 2 && useShahDb){
		cout << "Using Test Database" << endl;
		strncpy(filename, testDbFile10, strlen(testDbFile10)+1 );
		cout << "File : " << testDbFile10 << endl;
		//system("dir");
		capture = cvCaptureFromAVI(filename);
		fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		if (capture == NULL || fps < 5) {printf("Video not found [TERMINATION].\n"); return; }
	}
	else if (argc < 2 && useTestDb){
		cout << "Using Test Database" << endl;
		strncpy(filename, testDbFile11, strlen(testDbFile11)+1 );
		cout << "File : " << testDbFile11 << endl;
		capture = cvCaptureFromAVI(filename);
		fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		if (capture == NULL || fps < 5) {printf("Video not found [TERMINATION].\n"); return; }
	}
	else{
		strncpy(filename, argv[1], strlen(argv[1])+1 );
		cout << "Using custom Dataset : " << filename << endl;
		capture = cvCaptureFromAVI(argv[1]);
		fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		if (capture == NULL || fps < 5) {printf("Video not found [TERMINATION].\n"); return; }
	}
	//Initialize frames
	img = cvQueryFrame(capture);
	img2 = cvCreateImage( cvSize(img->width, img->height), img->depth, img->nChannels);
	visualImg = cvCreateImage( cvSize(img->width, img->height), img->depth, img->nChannels);
	//Add padding +8 pixel for creating border between images
	bufferImg = cvCreateImage( cvSize( 8 + img->width*2, 8 + img->height*2), img->depth, img->nChannels); // Serious business bro.
	cout << "[Frame Information] Height : " << img->height << " Width : " << img->width << endl;
	/* Dataset Settings and Dynamic Load of Video */
	//Initialize publicData
	width = img->width;
	height = img->height;
	/* Initialize font data. */
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.33, 0.33, 0, 1, CV_AA);
	numFrames = (int) cvGetCaptureProperty(capture,  CV_CAP_PROP_FRAME_COUNT);
	frameIdx = 0;
	/* Initialize Video Recording */

}

void VideoWrapper::startRecord(char* filename, int fps, int isColor)
{


	strcpy(filenameBuffer, "videoOutputs/");
	strcat(filenameBuffer, filename);
	if ( fps == NULL )
		fps = this->fps;
	if ( isColor == NULL )
		isColor = img->nChannels > 1 ? 1 : 0;
	
	if ( multiOutput ){
		//10 Pixels between videos
		recCapture = cvCreateVideoWriter(filenameBuffer, CV_FOURCC('X','V','I','D'), fps, cvSize( bufferImg->width, bufferImg->height), isColor);
	}
	else{
		recCapture = cvCreateVideoWriter(filenameBuffer, CV_FOURCC('X','V','I','D'), fps, cvSize( width, height), isColor);
	}
	if (!recCapture)
		printf("Video writing is not enabled\n");
	if (filename == NULL) {
		printf("Recording is disabled. There is no filename \n");
		exit(0);
	}
	
}

void VideoWrapper::WriteFrame(IplImage *img)
{
	if ( multiOutput ){
		//int padding = 20;
		//cvPutText(bufferImg, "Frame", cvPoint(padding,padding), &font, cvScalar(255,255,255,255) );
		//cvPutText(bufferImg, "Background Subtraction", cvPoint(padding+width,padding), &font, cvScalar(255,255,255,255) );
		//cvPutText(bufferImg, "Blob Detection", cvPoint(padding,padding+height), &font, cvScalar(255,255,255,255) );
		//cvPutText(bufferImg, "Tracking", cvPoint(padding+width,padding+height), &font, cvScalar(255,255,255,255) );
		cvWriteFrame(recCapture, bufferImg);
		//cvSetZero(bufferImg);
		cvSet(bufferImg, cvScalar(255,255,255,255));
	}
	else
		cvWriteFrame(recCapture, img);
}

void VideoWrapper::finishRecord()
{
	cvReleaseVideoWriter(&recCapture);
}

void VideoWrapper::passNFrames(int numOfFrames)
{
	for(int i = 0; i < numOfFrames; i++){
		img = cvQueryFrame(capture);
		frameIdx++;
	}
}
char* VideoWrapper::getVideoFileName()
{
	char *ptr;
	ptr = &filename[0];
	return ptr;
}
void VideoWrapper::setMultipleOutput()
{
	multiOutput = true;
}
void VideoWrapper::WriteOutputChannel(int ind, IplImage *img)
{
	if ( ind < 0 || ind > 3){
		cout << "[WrongIndex: VideoWrapper::WriteOutputChannel]  : "<< endl;
		exit(-1);
	}
	
	int widthOffset = (ind & 1) * width ;
	int heightOffset = (ind & 2)/2  * height;

	int widthDelim = (widthOffset != 0) ? 8 : 0;
	int heightDelim = (heightOffset != 0) ? 8 : 0; 
	//cout << "[heightDelim " << heightDelim << " : widthDelim:] " << widthDelim << endl;

	cvSetImageROI(bufferImg, cvRect( widthOffset + widthDelim , heightOffset + heightDelim , width, height) );
	cvCopy(img, bufferImg);
	cvResetImageROI(bufferImg);
	//cout << "[copied : ind:] " << ind << endl;

}