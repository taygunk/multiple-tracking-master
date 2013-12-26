#ifndef BGSUB_ENGINE_H
#define BGSUB_ENGINE_H
#include "Environment.h"
class BgSubEngine{
public:
	//Pointers to shared memory
	IplImage* output;
	//Variables
	int colorBin;
	int xSpaceBin;
	int ySpaceBin;
	float learnRat;
	float alfa1;
	float alfa2;
	int learnSec;
	int frameNoStored;
	int fgUpdInterval;
	int bgUpdInterval;
	int learnFrameNum;
	float bg_thresh;
	float kappa;
	bool maskedOutput;
	bool videoOutput;
	int width, height;
	float epsilon;
	int t;
	int frameIdx;
	float logeps;
	float uniLikelihood;
	bool Bg_thresholding;	
	bool LogLikelihood;
	bool useMrf;
	//Data Structures
	int ****bins;
	float *****bgHist;
	float *****fgHist;
	float **Px_Psib;
	float **Px_Psif;
	bool **mask;
	double bgSum, fgSum;
	//Variables for MRF & Graph Cut based approach
	float id;			// Returns node id of pixel
	float cap, revcap;	// Capacities of edges
	float lambda;		// Lambda for pixel clique connection ratio
	float maxflow;		// Computation of maxflow
	//Debugging Options
	bool debugText;


	//Methods
	private:
		void maskToImage(bool** mask, IplImage* img);
		void setBins(int ****bins, float val);
		void setHistogram(float *****hist, float val);
		void makeVector(uchar *rawData, int x, int y, int *vector);
		void makeImageBins(IplImage *img, int**** bins);
		void augmentBgModelLinear( float***** bgHist, int**** bins);
		double computeSum(float *****hist);
		void computeSum(float *****hist, double *sum);
		void updatePxProbs(int**** bins, float *****bgHist, float *****fgHist, float **Px_PsiB, float **Px_PsiF, double bgSum, double fgSum);
		void computeMaskImage(float **Px_PsiB, float **Px_PsiF, double bgSum, double fgSum, int ****bins, bool **mask);
		void augmentModelsWithMaskFullBgLinear(int ****bins, float *****bgHist, float *****fgHist, bool **mask);
		void deductBgModelLinear(int time, int ****bins, float *****bgHist);
		void deductFgModelLinear(int time, int ****bins, float *****fgHist);
	public:
		BgSubEngine(int width, int height, int colorBin, int xSpaceBin, int ySpaceBin, IplImage *output);
		//BgSubEngine(int width, int height, int cBin, int xBin, int yBin, IplImage *output);
		//setLearningSeconds(int t);						//Algorithm will learn input frames for t seconds.
		void learnFrames(IplImage *input_frame);			//Algorithm will learn input frames
		void learnFromFile(int learnFrameNum);				//!!!You need to call this if you read the learned frame data from file.
		//Use BGModel, BG+FG Model or BG+FG+MRF Model
		void useBGmodelOnly(double thresh);					//Uses only background model for the process with threshold.
		void useBGandFGmodel(double thresh);				//Uses both models for the process with threshold. (This is default)
		void useMRF();										//Apply MRF to the process. (This is default)
		//Call this at each subsequent frame
		void run(IplImage *input_frame, IplImage *output);	//Applies background subtraction to input frame.
		void finalize();									//Clear resources.
		//Debugging Purposes
		void serialize(char* filename);						//Writes learned frames to a .txt file
		void deserialize(char* filename);					//Reads learned frames from a .txt file
		//DELETE THIS AFTER. FOR MODIFYING MASK FOR PIXEL ACCESS
		void ImageToMask(IplImage* img, bool** mask);
		
		
};
#endif