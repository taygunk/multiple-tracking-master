#include "BgSubEngine.h"

//Implementation
void BgSubEngine::maskToImage(bool** mask, IplImage* img)
{
	for(int i = 0 ; i < img->height; i++)
		for(int j = 0; j < img->width; j++){
			if ( !mask[i][j] )
				cvSet2D( img, i , j, cvScalar(0,0,0,0));
			else
				cvSet2D( img, i , j, cvScalar(255,255,255,0));
		}
}

void BgSubEngine::ImageToMask(IplImage* img, bool** mask)
{
	for(int i = 0 ; i < img->height; i++)
		for(int j = 0; j < img->width; j++){

			if ( cvGet2D(img, i,j).val[0] )
				mask[i][j] = 1;
			else
				mask[i][j] = 0;
		}
}
void BgSubEngine::setHistogram(float *****hist, float val)
{
	int i,j,k,x,y;
	for(i = 0; i < colorBin ; i++)
		for(j = 0; j < colorBin ; j++)
			for(k = 0; k < colorBin; k++)
				for(x = 0; x < xSpaceBin; x++)
					for(y = 0; y < ySpaceBin; y++){
						hist[i][j][k][x][y] = val;
					}

}
void BgSubEngine::setBins(int ****bins, float val)
{
	int i,j,k,x;
	for(i = 0; i < frameNoStored ; i++)
		for(j = 0; j < height ; j++)
			for(k = 0; k < width; k++)
				for(x = 0; x < 6; x++){
					bins[i][j][k][x] = val;
				}

}
void BgSubEngine::makeImageBins(IplImage *img, int**** bins)
{
	for(int i = 0 ; i < img->height; i++)
		for(int j = 0; j < img->width; j++){
			makeVector( cvPtr2D(img, i, j, NULL), i, j,	&bins[t][i][j][1] );
		}
}
void BgSubEngine::makeVector(uchar *rawData, int x, int y, int *vector)
{
	vector[0] = round( (rawData[0] / 255.0)  * ( colorBin - 1 ) );
	vector[1] = round( (rawData[1] / 255.0)  * ( colorBin - 1 ) );
	vector[2] = round( (rawData[2] / 255.0)  * ( colorBin - 1 ) );
	vector[3] = round( (x / (float)height) * ( xSpaceBin - 1 ) );
	vector[4] = round( (y / (float)width)  * ( ySpaceBin - 1 ) );

}
void BgSubEngine::augmentBgModelLinear(float *****bgHist, int ****bins)
{
	for(int i = 0 ; i < height; i++){
		for(int j = 0; j < width; j++){
			bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa1;
			if ( bins[t][i][j][1] > 0 && bins[t][i][j][2] > 0 && bins[t][i][j][3] > 0 ){
				bgHist[ bins[t][i][j][1] - 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] -1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] -1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
			}
			if ( bins[t][i][j][1] < colorBin -1  && bins[t][i][j][2] < colorBin -1 && bins[t][i][j][3] < colorBin -1 ){
				bgHist[ bins[t][i][j][1] + 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] + 1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] +1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
			}
		}
	}

}
double BgSubEngine::computeSum(float *****hist)
{
	double sum = 0;
	int i,j,k,x,y;
	for(i = 0; i < colorBin ; i++)
		for(j = 0; j < colorBin ; j++)
			for(k = 0; k < colorBin; k++)
				for(x = 0; x < xSpaceBin; x++)
					for(y = 0; y < ySpaceBin; y++){
						sum += hist[i][j][k][x][y];
					}
	return sum;
}
void BgSubEngine::updatePxProbs(int**** bins, float *****bgHist, float *****fgHist, float **Px_PsiB, float **Px_PsiF, double bgSum, double fgSum)
{
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			Px_PsiB[i][j] =  bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ];
			Px_PsiB[i][j] /= (bgSum+epsilon) ;
			
			Px_PsiF[i][j] =  fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ];
			Px_PsiF[i][j] = learnRat * uniLikelihood + ( 1 - learnRat ) * (Px_PsiF[i][j] / (fgSum+epsilon) );

			Px_PsiB[i][j] =   log( Px_PsiB[i][j] + epsilon);
			Px_PsiF[i][j] =   log( Px_PsiF[i][j] + epsilon);
		}
	}	
}
void BgSubEngine::computeMaskImage(float **Px_PsiB, float **Px_PsiF, double bgSum, double fgSum, int ****bins, bool **mask)
{
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
				
				if ( Bg_thresholding ){
					Px_PsiB[i][j]  < bg_thresh  ? mask[i][j] = 1 : mask[i][j] = 0; // 1 
				}
				if ( LogLikelihood ){
					Px_PsiB[i][j] - Px_PsiF[i][j] < kappa? mask[i][j] = 1  :  mask[i][j] = 0;
				}
		}
	}

	
}
void BgSubEngine::augmentModelsWithMaskFullBgLinear(int ****bins, float *****bgHist, float *****fgHist, bool **mask)
{
	for(int i = 0; i < height; i++)
		for(int j = 0; j < width; j++){
			//Add to background model always ( Does not depend on the mask )
			bins[t][i][j][0] = 0; //Mark as background append at time t
			bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa1;
			if ( bins[t][i][j][1] > 0 && bins[t][i][j][2] > 0 && bins[t][i][j][3] > 0){
				bgHist[ bins[t][i][j][1] - 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] -1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] -1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
			}
			if ( bins[t][i][j][1] < colorBin - 1 && bins[t][i][j][2] < colorBin - 1 && bins[t][i][j][3] < colorBin - 1 ){
				bgHist[ bins[t][i][j][1] + 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] + 1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				bgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] +1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
			}

			if ( mask[i][j] ){
				bins[t][i][j][0] = 1; //Mark as foreground append at time t
				fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa1;
				if ( bins[t][i][j][1] > 0 && bins[t][i][j][2] > 0 && bins[t][i][j][3] > 0){
					fgHist[ bins[t][i][j][1] - 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
					fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] -1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
					fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] -1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				}
				if ( bins[t][i][j][1] < colorBin - 1 && bins[t][i][j][2] < colorBin - 1 && bins[t][i][j][3] < colorBin - 1 ){
					fgHist[ bins[t][i][j][1] + 1][ bins[t][i][j][2] ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
					fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] + 1 ][ bins[t][i][j][3] ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
					fgHist[ bins[t][i][j][1] ][ bins[t][i][j][2] ][ bins[t][i][j][3] +1 ][ bins[t][i][j][4] ][ bins[t][i][j][5] ] += alfa2;
				}
			}
		}
}
void BgSubEngine::deductBgModelLinear(int time, int ****bins, float *****bgHist)
{
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			bgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa1;
			if ( bins[time][i][j][1] > 0 && bins[time][i][j][2] > 0 && bins[time][i][j][3] > 0 ){
				bgHist[ bins[time][i][j][1] - 1][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				bgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] -1 ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				bgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] -1 ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
			}
			if ( bins[time][i][j][1] < colorBin - 1 && bins[time][i][j][2] < colorBin - 1 && bins[time][i][j][3] < colorBin - 1  ){
				bgHist[ bins[time][i][j][1] + 1][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				bgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] + 1 ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				bgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] +1 ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
			}
		}
	}	
}
void BgSubEngine::deductFgModelLinear(int time, int ****bins, float *****fgHist)
{
	int numPx = 0;
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			if ( bins[time][i][j][0] == 1 ){
				fgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa1;
				if ( bins[time][i][j][1] > 0 && bins[time][i][j][2] > 0 && bins[time][i][j][3] > 0 ){
					fgHist[ bins[time][i][j][1] - 1][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
					fgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] -1 ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
					fgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] -1 ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				}
				if ( bins[time][i][j][1] < colorBin - 1 && bins[time][i][j][2] < colorBin - 1 && bins[time][i][j][3] < colorBin - 1  ){
					fgHist[ bins[time][i][j][1] + 1][ bins[time][i][j][2] ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
					fgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] + 1 ][ bins[time][i][j][3] ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
					fgHist[ bins[time][i][j][1] ][ bins[time][i][j][2] ][ bins[time][i][j][3] +1 ][ bins[time][i][j][4] ][ bins[time][i][j][5] ] -= alfa2;
				}
				numPx++;
			}
		}
	}
}

//Interface
BgSubEngine::BgSubEngine(int width, int height, int colorBin, int xSpaceBin, int ySpaceBin, IplImage *output){
	this->output = output;
	this->colorBin = colorBin;
	this->xSpaceBin = xSpaceBin;
	this->ySpaceBin = ySpaceBin;
	learnRat = 0.01;
	alfa1 = 1;
	alfa2 = 0.5;
	learnSec = 8;
	bg_thresh = -13.9971;//38.0301; //13.9905 * exp(1);
	kappa = 0.0;
	Bg_thresholding = false;	
	LogLikelihood = true;
	useMrf = false;
	frameNoStored = 50;
	fgUpdInterval = 3;
	bgUpdInterval = frameNoStored;
	learnFrameNum = learnSec * 30;
	maskedOutput = FALSE;
	videoOutput = true;
	this->width = width;
	this->height = height;
	epsilon = 1e-12;
	t = 0;
	logeps = log(epsilon);
	uniLikelihood = 1.0 / (colorBin * colorBin * colorBin * xSpaceBin * ySpaceBin);
	//Memory Allocation
	bins = create4dKernel(frameNoStored, height, width, 6); //1 For PixelAdd/NotAdd, 5 Bins
	bgHist = create5dKernel(colorBin, colorBin, colorBin, xSpaceBin, ySpaceBin);
	fgHist = create5dKernel(colorBin, colorBin, colorBin, xSpaceBin, ySpaceBin);
	Px_Psib = create2dKernel( height, width);
	Px_Psif = create2dKernel( height, width);
	mask = create2dKernelBool( height, width, false);
	bgSum = 0;
	fgSum = 0;
	//MRF
	lambda = 10;

	/* Data */
	if ( !bins || !bgHist || !fgHist || !Px_Psib || !Px_Psif ){
		cout << "[BgSubEngine] Not enough memory. " << endl;
		exit(-1);
	}
	setBins(bins, 0);
	setHistogram(fgHist, 1);
	setHistogram(bgHist, 0);

	frameIdx = 0;

	Bg_thresholding = false;
	LogLikelihood = true;
	useMrf = false;
	debugText = false;
}

void BgSubEngine::learnFrames(IplImage *input_frame)
{
	t = frameIdx % frameNoStored;
	cout << "Learning frame " << frameIdx << " :stored in bin " << t << endl;
	makeImageBins(input_frame, bins);
	augmentBgModelLinear( bgHist, bins);
	frameIdx++;
}
void BgSubEngine::learnFromFile(int learnFrameNum)
{
	frameIdx += learnFrameNum;
	t = frameIdx % frameNoStored;
}
void BgSubEngine::run(IplImage *input_frame, IplImage *output)
{
	t = frameIdx % frameNoStored;
	deductBgModelLinear(t, bins, bgHist);
	if ( debugText ){
	cout << "[BG] Removed frame " << frameIdx - frameNoStored << " :stored in bin " << t << endl;
	}
	int slidingWindowTail = ( frameIdx - fgUpdInterval ) % frameNoStored;
	deductFgModelLinear( slidingWindowTail, bins, fgHist);
	if ( debugText ){
	cout << "[FG] Removed frame " << frameIdx - fgUpdInterval << " :stored in bin " << slidingWindowTail << endl;
	}
	cout << "Processing frame " << frameIdx << " :stored in bin " << t << endl;
	/* Decision Step */
	makeImageBins(input_frame, bins);
	bgSum = computeSum(bgHist);
	fgSum = computeSum(fgHist);
	updatePxProbs(bins, bgHist, fgHist, Px_Psib, Px_Psif, bgSum, fgSum);
	computeMaskImage( Px_Psib, Px_Psif, bgSum, fgSum, bins, mask);
	augmentModelsWithMaskFullBgLinear(bins, bgHist, fgHist, mask); 
	maskToImage(mask, output);
	frameIdx++;
}
void BgSubEngine::finalize()
{
	if ( Bg_thresholding ){
		cout << "Background Model : Parameters ColorBin : " << colorBin << " xSpaceBin: " << xSpaceBin << " ySpaceBin: " << ySpaceBin << " Log Threshold: " << bg_thresh << endl; 
	}
	else if ( LogLikelihood && !useMrf ){
		cout << "BG+FG LogLikelihood Model : Parameters ColorBin : " << colorBin << " xSpaceBin: " << xSpaceBin << " ySpaceBin: " << ySpaceBin << " Log Threshold: " << kappa << " LearnRate : " << learnRat << endl; 
	}
}
void BgSubEngine::useBGmodelOnly(double thresh)
{
	Bg_thresholding = true;
	LogLikelihood = false;
	bg_thresh = thresh;
}
void BgSubEngine::useBGandFGmodel(double thresh)
{
	kappa = thresh;
}
//Debugging
void BgSubEngine::serialize(char* filename)
{
	FILE *fp = fopen(filename, "w+");
	int i,j,k,x,y;
	for(i = 0; i < colorBin ; i++)
		for(j = 0; j < colorBin ; j++)
			for(k = 0; k < colorBin; k++)
				for(x = 0; x < xSpaceBin; x++)
					for(y = 0; y < ySpaceBin; y++){
							fprintf(fp, "%f ", bgHist[i][j][k][x][y] );
					}

	for(i = 0; i < frameNoStored ; i++)
		for(j = 0; j < height ; j++)
			for(k = 0; k < width; k++)
				for(x = 0; x < 6; x++){
					fprintf(fp, "%d ", bins[i][j][k][x] );
				}

	fclose(fp);
}
void BgSubEngine::deserialize(char* filename)
{
	FILE *fp = fopen(filename, "r");
	if ( fp == NULL ){
		cout << "Error: BgSubEngine::deserialize" << endl;
		exit(-1);
	}
	int i,j,k,x,y;
	for(i = 0; i < colorBin ; i++)
		for(j = 0; j < colorBin ; j++)
			for(k = 0; k < colorBin; k++)
				for(x = 0; x < xSpaceBin; x++)
					for(y = 0; y < ySpaceBin; y++){
							fscanf(fp, "%f", &bgHist[i][j][k][x][y] );
					}

	for(i = 0; i < frameNoStored ; i++)
		for(j = 0; j < height ; j++)
			for(k = 0; k < width; k++)
				for(x = 0; x < 6; x++){
					fscanf(fp, "%d", &bins[i][j][k][x] );
				}	

	fclose(fp);
}