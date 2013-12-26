#include "Hypothesis.h"

int Hypothesis::objectNumber = 0;
int Hypothesis::img_height = 0;
int Hypothesis::img_width = 0;

Hypothesis::Hypothesis()
{
	killed = false;
	spatialId = rand();

	ex_x0 = 0;
	ex_y0 = 0;
	ex_w = 0;
	ex_h = 0;
	x0 = 0;
	y0 = 0;
	vx = 0;
	vy = 0;
	ex_ang = 0;
	ang = 0;

	width = 0;
	height = 0;
	area = 0;
	histSum = 0;
	state = HYP_OCCLUDED;
	hasMatch = false;
	//Allocate trajectory data
	trajectory = new int[2000];
	histogram = new float[4096];
	memset(histogram, 0, sizeof(float) * 4096 );
	time = 0;
	shellMask = create2dKernelBool(150,150, 0);

	objectState = SINGLE;
}
void Hypothesis::appear(Hypothesis & h)
{
	this->spatialId = h.spatialId;
	//printf("obj: %d spatialId is %d \n", id, h.spatialId);
	//Copy the region data... and histogram data...
	//Store last coordinate as ex
	ex_x0 = x0;
	ex_y0 = y0;
	//Store last hypothesis size as ex
	ex_h = height;
	ex_w = width;
	//Store hypothesis' last velocity as ex
	//ex_vx = vx;
	//ex_vy = vy;
	//Copy matched hypothesis' data
	if ( state != HYP_TRACKED ){
		x0 =  h.x0;
		y0 =  h.y0;
	}
	width = h.width;
	height = h.height;
	//Calculate last velocity (linear model here without a filter...)
	//vx = ex_x0 - x0;
	//vy = ex_y0 - y0;
	//ex_ang = ang;
	//ang = (atan2( vy, -vx)  * 180 / 3.14159265 ) ;  // Opencv Vx  (<-----) vector computation has sign (+), convert to (-)
	//velDirection = ang / 45;
	//velMag = vx * vx + vy * vy;
	//We saw the frame just now...
	lastSeenIndex = 0;
	//Add to trajectory ( x0 + half width ... )
	trajectory[2*time % 2000] = x0;
	trajectory[2*time+1 % 2000] = y0;
	//Store inner shell radius6
	innerShellRadius = sqrt( (width/2) * (width/2) + (height/2) * (height/2) ); 
	//outerShellRadius = innerShellRadius * 1.25;
	state = HYP_TRACKED;
}
void Hypothesis::updateColor(bool **mask, int ****pixHist,int *t)
{
	memset(histogram, 0, sizeof(float) * 4096 );
	pxInHistogram = 0;
	int r,g,b;
	for(int y = y0 - height/2 ; y < y0 + height/2 ; y++){
		for(int x = x0 - width/2 ; x < x0 + width/2 ; x++){
			//Extract only from foreground region in hypothesis
			if ( mask[y][x] ){
				//t is current bin-time on algorithm.
				b = pixHist[*t][y][x][1];
				g = pixHist[*t][y][x][2];
				r = pixHist[*t][y][x][3];
				//Now we have {b,g,r} (Already binned) to histogram
				//Fill
				histogram[ b + g*16 + r*256] += 1;
				pxInHistogram++;
			}
		}
	}/*
	for(int a = 0; a < 4096; a++){
		if ( histogram[a] < 0 || histogram[a] > pxInHistogram )
			std::cout << "BUG IN COLORHISTOGRAMFILL" << std::endl;
	}
	*/

}

void Hypothesis::updateColor(bool **mask, int ***bins)
{
	int yStart, xStart, xLim, yLim;
		//Boundary Checks
		if ( y0 - height/2  <= 0 )
			yStart = 0;
		else
			yStart = y0 - height/2;

		if ( y0 + height/2 >= Hypothesis::img_height - 1)
			yLim = Hypothesis::img_height - 1;
		else
			yLim = y0  + height/2;

		if ( x0  - width/2 < 0 )
			xStart = 0;
		else
			xStart = x0  - width/2;

		if ( x0  + width/2 >= Hypothesis::img_width - 1 )
			xLim = Hypothesis::img_width - 1;
		else
			xLim = x0  + width/2;
		//Boundary Checks

	memset(histogram, 0, sizeof(float) * 4096 );
	pxInHistogram = 0;
	int r,g,b;
	for(int y = yStart ; y < yLim; y++){
		for(int x = xStart ; x < xLim ; x++){
			//Extract only from foreground region in hypothesis
			if ( mask[y][x] ){
				//t is current bin-time on algorithm.
				b = bins[y][x][0];
				g = bins[y][x][1];
				r = bins[y][x][2];
				//Now we have {b,g,r} (Already binned) to histogram
				//Fill
				histogram[ b + g*16 + r*256] += 1;
				pxInHistogram++;
			}
		}
	}/*
	for(int a = 0; a < 4096; a++){
		if ( histogram[a] < 0 || histogram[a] > pxInHistogram )
			std::cout << "BUG IN COLORHISTOGRAMFILL" << std::endl;
	}
	*/

}

float Hypothesis::matchHist_Bhattarcharya_Coefficient(Hypothesis &t1)
{
	int a;
	float c,d;
	float bhat = 0;
	//Calc bhattarcharya coefficient
	for(a = 0; a < 4096; a++){
		c = histogram[a]; // / (pxInHistogram) + 1e-12;
		d = t1.histogram[a]; //  / (t1.pxInHistogram) + 1e-12;
		//std::cout << "a " << a << " c : " << c << " d: " << d << " bhat: " << bhat << std::endl;
		if ( !pxInHistogram || !t1.pxInHistogram ){
			std::cout << " bug in pxInHistogram amk" << std::endl;
		}
		bhat += sqrt( c * d );	// prevent sqrt(0) 
	}

	bhat = bhat / ( sqrt( pxInHistogram * t1.pxInHistogram) ); //Normalize
	/*
	if ( histogram[a] < 0 || histogram[a] > pxInHistogram )
		std::cout << "BUG IN COLORHISTOGRAM_MATCH" << std::endl;
	}
	*/

	return bhat;
}
void Hypothesis::disappear()
{
	lastSeenIndex++;
}

float Hypothesis::match(Hypothesis &t1)	// When a hypothesis_t matches with hypothesis_t+1, call this method with {ONLY ONE TO ONE SUPPORTEDNOW}
{
	float matchRat;
	float spatialCoordMatch = 1 - sqrt( pow(x0 - t1.x0,2) + pow(y0 - t1.y0 , 2)) / MAX_DIST_ALLOWED ; //Euclidian Distance
	// Update Velocity Match using cosine theorem
	float a = (this->velMag * this->velMag);
	float b = t1.velMag* t1.velMag;
	float alfa = MIN( ( this->velDirection - t1.velDirection ) , 8 - ( this->velDirection - t1.velDirection ) ) * 45.0;
	float velMetric =  (a *a + b * b - 2 * a * b * cos( alfa * 3.14159265 / 180 ) ) / (2*MAX_VELOC); //normalize also
	// Update Color Match using Bhattarcharya_Coefficient
	float colorMatch = matchHist_Bhattarcharya_Coefficient(t1);
	// costfunction implemented here. 
	std::cout << "SpatM : " << spatialCoordMatch << //" VelM: " << velMetric << 
				 " colorM: " << colorMatch << std::endl;
	matchRat = 0.20 * spatialCoordMatch + //0.10 * velMetric + 
			   + 0.80 * colorMatch;
	
	return matchRat;
}

void Hypothesis::printTrack(IplImage *img)
{
	for(int i = 0; i < time - 1; i++){
		cvLine( img, cvPoint( trajectory[ 2*i ], trajectory[ 2*i +1 ]),
			cvPoint( trajectory[ 2*(i+1) ], trajectory[ 2*(i+1) +1 ]),
			trajColor);

	}
}
int getAvailableHypothesisNumber()
{
	return Hypothesis::objectNumber;
}
void Hypothesis::updateCentroid(bool **mask)
{
	//printf("[DBGB] x0 : %f y0  : %f \n", x0, y0 );
	for(int y = y0; y < y0 + height ; y++){
		for(int x = x0; x < x0 + width ; x++){
			//Evaluate pixel membership hypo's		
		}
	}
	//Compute center of membered pixels
	int xSum = 0;
	int ySum = 0;
	if (y0 < 0 || y0 > 600 || x0 < 0 || x0 > 1200 ){
		printf("bug in centroids. \n");
		exit(-1);
	}
	//printf("%f %f\n", x0, y0 );
	int numPt = 0;
	for(int y = y0; y < y0 + height ; y++){
		for(int x = x0; x < x0 + width ; x++){
			/*
			if ( mask[y][x] != 0 && mask[y][x] != 1 ){
				printf("bug amg. \n");
			}
			*/
				xSum += (mask[y][x]*x);		// multiply with mask to prevent if check.
				ySum += (mask[y][x]*y);
				numPt += (mask[y][x] * 1);
		}
	}
	//printf("%d %d\n", xSum, ySum );
	ex_x0 = x0;
	ex_y0 = y0;
	x0 = xSum / numPt;
	y0 = ySum / numPt;
	//printf("%f %f\n", x0, y0 );
	//printf("[DBGA] x0 : %f y0  : %f \n", x0, y0 );
	
}


bool** Hypothesis::create2dKernelBool(int xSize, int ySize, bool scalar)
{
	int i;
	bool **kernel = new bool*[xSize]; 
	for(i = 0; i < xSize; i++){
		kernel[i] = new bool[ySize];
	}
	for(int i = 0; i < xSize; i++ ){
		for(int j = 0; j < ySize ; j++){
			kernel[i][j] = scalar;
		}
	}
	return kernel;
}
bool Hypothesis::hasInteract()
{
	return intEvtSet.size();
}

bool Hypothesis::hasMerge()
{
	return merEvtSet.size();	
}

void Hypothesis::updatePixelsCentroid(bool **mask)
{
	/*
	set<int>::iterator it;
	int roi = (innerShellRadius * 1.25 ) * 2 ; //Full radius
	int xSum = 0, ySum = 0, numPt = 0;

	int yLim, xLim, yStart, xStart;

	//Boundary Checks
	if ( y0 - roi < 0 )
		yStart = 0;
	else
		yStart = y0 - roi;

	if ( y0 + roi >= Hypothesis::img_height )
		yLim = Hypothesis::img_height - 1;
	else
		yLim = y0 + roi;

	if ( x0 - roi < 0 )
		xStart = 0;
	else
		xStart = x0 - roi;

	if ( x0 + roi >= Hypothesis::img_height )
		xLim = Hypothesis::img_height - 1;
	else
		xLim = x0 + roi;
	//Boundary Checks

	//Fill mask2 (Inside shell-outside shell mask)
	/*
	for(int y = 0 ; y < yLim - yStart; y++){
		for(int x = 0 ; x < xLim - xStart; x++){
			shellMask[y][x] = 
		}
	}
	*/

	//Implementation of argmax c
	/*
	classUnionSet.insert( intEvtSet.begin(), intEvtSet.end() );
	classUnionSet.insert( merEvtSet.begin(), merEvtSet.end() );
	for(int y = yStart ; y < yLim; y++){
		for(int x = xStart ; x < xLim ; x++){
			//Find max class
			
			xSum += (mask[y][x]*x);		// multiply with mask to prevent if check.
			ySum += (mask[y][x]*y);
			numPt += (mask[y][x] * 1);			
		}
	}	
	ex_x0 = x0;
	ex_y0 = y0;
	x0 = xSum / numPt;
	y0 = ySum / numPt;

	classUnionSet.clear();
	*/
}