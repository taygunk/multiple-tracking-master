#include "BlobDetectionEngine.h"
BlobDetectionEngine::BlobDetectionEngine(IplImage *visualImg) 
{
	hypClicked = false;

	corMatrix = new float*[50];
	for(int row = 0; row < 50; row++){
		corMatrix[row] = new float[50];
	}

	this->visualImg = visualImg;

	strcpy(text, "...");
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, CV_AA);

	filtCont = new CvSeq*[maxBlobNum];
	mem = cvCreateMemStorage(0); 

	hyp_t.reserve(50);	// Reserve memory for 50 elements
	for(int i = 0; i < 50; i++)
		hyp_t.push_back( Hypothesis() );

	hyp_t_1.reserve(50);
	for(int i = 0; i < 50; i++)
		hyp_t_1.push_back( Hypothesis() );

	curHypNum = 0;
	preHypNum = 0;

	minAreaFilter = 75;
	//add image info to hypt
	Hypothesis *test = new Hypothesis();
	test->img_height = visualImg->height;
	test->img_width = visualImg->width;
	cout << "W : " << test->img_width << " H : " << test->img_height << endl;
	delete test;

	this->setColors();
}


double BlobDetectionEngine::find_eu_distance(const CvPoint& point_one, const CvPoint& point_two)
{
	return sqrt(double((point_one.x - point_two.x) * (point_one.x - point_two.x) +
		(point_one.y - point_two.y) * (point_one.y - point_two.y)) );
}

void BlobDetectionEngine::fillZero(float **mat, int size)
{
	for(int dim1= 0; dim1 < size; dim1++)
		for(int dim2=0; dim2 < size; dim2++)
			mat[dim1][dim2] = 0;

}
void BlobDetectionEngine::setColors()
{
	colors[0] = cvScalar(255,0,0,255);	//blue
	colors[1] = cvScalar(0,255,255,255);  //yellow
	colors[2] = cvScalar(30,105,210,255);	//chocolate
	colors[3] = cvScalar(147,20,255,255);	//deep-pink
	colors[4] = cvScalar(113,179,60,255); //medium-sea green
	colors[5] = cvScalar(230,216,173,255);//light-blue
	colors[6] = cvScalar(205,250,255,255);//lemon-chiffon
	colors[7] = cvScalar(143,143,188,255);//rosy-brown
}

void BlobDetectionEngine::userClick(bool *hypClick, bool *hypUpdate, int *tx, int *ty)
{
	this->hypClicked = *hypClick;
	this->hypUpdated = *hypUpdate;
	this->tx = *tx;
	this->ty = *ty;
}
void BlobDetectionEngine::findBlobs(IplImage *grayImg, bool drawBlobs)
{
	cvFindContours(grayImg, mem, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	int i = 0;
	for (ptr = contours; ptr != NULL; ptr = ptr->h_next) {
		//Filter small contours
		CvRect rect = cvBoundingRect(ptr);
		if (  rect.height * rect.width < minAreaFilter ){
			continue;
		}
		filtCont[i] = ptr;

		//CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
		CvScalar color = CV_RGB( 255, 255, 255 );
		cvDrawContours(visualImg, ptr, color, CV_RGB(0,0,0), 0, CV_FILLED, 8, cvPoint(0,0));
		cvRectangle(visualImg, cvPoint(rect.x +3, rect.y +3), cvPoint(rect.x + rect.width, rect.y + rect.height), color, 1);
		//sprintf(text, "B%d [%d,%d]", i, rect.x, rect.y);
		sprintf(text, "Blob %d", i);
		//cvPutText(visualImg, text, cvPoint(rect.x, rect.y), &font, color); 
		i++;
	}
	numOfFiltCont = i;
}

void BlobDetectionEngine::blobToRegions()
{
	for(int blob = 0; blob < maxBlobNum ; blob++){
		regionList[blob] = NO_REGION; // Blob is not corresponded with any region	 
	}
	int k = 0; // Number of regions found
	int j; // Match iterator

	//Use circle world collision to merge disconnected blobs
	/*
	for(int i = 0; i < numOfFiltCont; i++){
	j = 0; 
	// Get blob's surrounding circle's centers and radius'
	CvRect rectA = cvBoundingRect(filtCont[i]);
	r1 = find_eu_distance( cvPoint( rectA.x, rectA.y ), cvPoint(rectA.x + rectA.width , rectA.y + rectA.height) ) / 2;
	//r1 = max( rectA.width/2, rectA.height/2 );
	cent1.x = rectA.x + rectA.width / 2;
	cent1.y = rectA.y + rectA.height / 2;
	for(j = 0 ; j < numOfFiltCont ; j++){
	if ( i == j || (regionList[j] != NO_REGION) )
	continue;
	//Check for neighbourhood blobs
	CvRect rectB = cvBoundingRect(filtCont[j]);
	r2 = find_eu_distance( cvPoint( rectB.x, rectB.y ), cvPoint(rectB.x + rectB.width , rectB.y + rectB.height) ) / 2;
	//r2 = max( rectB.width/2, rectB.height/2 );
	cent2.x = rectB.x + rectB.width / 2;
	cent2.y = rectB.y + rectB.height / 2;
	//Maybe we shouldnt make inner circles model.??
	if ( find_eu_distance( cent1, cent2) <= ( 5 ) ){
	//Blob worlds are in collision.
	if ( regionList[i] != NO_REGION ){
	// Add this blob to their region chain
	regionList[j] = regionList[i];
	}
	else{
	// Create a new region
	regionList[i] = k;
	regionList[j] = k;
	regRect[k].x = min(rectA.x, rectB.x);
	regRect[k].y = min(rectA.y, rectB.y);
	regRect[k].width  = min(rectA.x + rectA.width , rectB.x + rectB.width ) - regRect[k].x;
	regRect[k].height = min(rectA.y + rectA.height, rectB.y + rectB.height) - regRect[k].y;
	k++;
	}
	//Update region's coordinates
	int regIdx = regionList[i];
	cvCircle(visualImg, cvPoint(cent1.x, cent1.y), r1, CV_RGB(255,0,0), 1);
	cvCircle(visualImg, cvPoint(cent2.x, cent2.y), r2, CV_RGB(255,0,0), 1);
	float x1 = min( min(rectA.x, rectB.x) , regRect[regIdx].x ); 
	float y1 = min( min(rectA.y, rectB.y) , regRect[regIdx].y ); 
	regRect[regIdx].width += regRect[regIdx].x - x1;
	regRect[regIdx].height += regRect[regIdx].y - y1;
	regRect[regIdx].x = x1;
	regRect[regIdx].y = y1;
	float x2 = max( regRect[regIdx].x + regRect[regIdx].width , max( rectA.x + rectA.width  , rectB.x + rectB.width ) );
	float y2 = max( regRect[regIdx].y + regRect[regIdx].height, max( rectA.y + rectA.height , rectB.y + rectB.height) );
	regRect[regIdx].width = x2 - regRect[regIdx].x;
	regRect[regIdx].height = y2 - regRect[regIdx].y;
	}
	}
	}
	*/
	numOfRegions = k;
	//Output Regions
	for(int i = 0; i < numOfFiltCont; i++){
		int regIdx = regionList[i];
		if ( regionList[i] != NO_REGION){
			cvRectangle(visualImg, cvPoint(regRect[regIdx].x , regRect[regIdx].y ), cvPoint(regRect[regIdx].x + regRect[regIdx].width, regRect[regIdx].y + regRect[regIdx].height), CV_RGB(0,255,0), 1);
			//sprintf(text, "B%d [%d,%d]", i, rect.x, rect.y);			
		}
		// A single blob, assign this blob to a region
		else{
			CvRect rect = cvBoundingRect(filtCont[i]);
			regionList[i] = numOfRegions; // new region.
			regIdx = regionList[i];
			regRect[regIdx].x = rect.x;
			regRect[regIdx].y = rect.y;
			regRect[regIdx].width  = (rect.x + rect.width) - regRect[regIdx].x;
			regRect[regIdx].height = (rect.y + rect.height) - regRect[regIdx].y;
			numOfRegions++;
		}
		cvRectangle(visualImg, cvPoint(regRect[regIdx].x , regRect[regIdx].y ), cvPoint(regRect[regIdx].x + regRect[regIdx].width, regRect[regIdx].y + regRect[regIdx].height), CV_RGB(0,255,0), 1);
		//cout << "Region " << regIdx << "[" << regRect[regIdx].x << "," << regRect[regIdx].y << "] "
		//	<< " w: " << regRect[regIdx].width << " h: " << regRect[regIdx].height << " has area : " << regRect[regIdx].height * regRect[regIdx].width << endl;
		sprintf(text, "Region %d", regIdx);
		//cvPutText(visualImg, text, cvPoint(regRect[regIdx].x -5 , regRect[regIdx].y -5 ), &font, CV_RGB(0,0,255));
	}

}
void BlobDetectionEngine::setAreaConstraint(int area)
{
	minAreaFilter = area;
}
void BlobDetectionEngine::runMatching(bool **bgMask, int ****bins, int *time)
{
	curHypNum = numOfRegions;
	//@At that frame, accumulate region information on temporary hypothesises
	for(int hyp = 0; hyp < curHypNum ; hyp++){
		//Fill hyp_t_1 (matrix column HYP)
		hyp_t_1[hyp].ex_x0 = regRect[hyp].x;
		hyp_t_1[hyp].ex_y0 = regRect[hyp].y;
		hyp_t_1[hyp].ex_h = regRect[hyp].height;
		hyp_t_1[hyp].ex_w = regRect[hyp].width;
		hyp_t_1[hyp].x0 =	regRect[hyp].x;
		hyp_t_1[hyp].y0 = regRect[hyp].y;
		hyp_t_1[hyp].width = regRect[hyp].width;
		hyp_t_1[hyp].height = regRect[hyp].height;
		hyp_t_1[hyp].hasMatch = false;
		//Uncomment if we dont need color histograms
		hyp_t_1[hyp].updateColor( bgMask, bins, time );
	}
	//Tag as previous hypothesis as no match yet.
	for(int hyp = 0; hyp < preHypNum ; hyp++){
		hyp_t[hyp].hasMatch = false;		
	}
	//Determine Correspondence Matrix Matches
	fillZero(corMatrix, 50); //Fill zeros
	//Check for matches
	for(int row = 0; row < preHypNum ; row++){
		for(int col = 0; col < curHypNum ; col++){
			float matchRatio = hyp_t[row].match( hyp_t_1[col] );	//Check match i,j
			//cout << "colorHist match : (" << hyp_t[row].id << ","<< col  << ") : " << hyp_t[row].matchHist_Bhattarcharya_Coefficient(hyp_t_1[col] ) << endl;
			corMatrix[row][col] = matchRatio;
			//cout << "match rat : " << matchRatio << endl;
			if ( matchRatio > 0.7 ){
				//cout << "traj change : hyp" << row << " as : " << hyp_t_1[col].x0 << endl;
				hyp_t[row].appear( hyp_t_1[col] );
				hyp_t_1[col].hasMatch = true;
				hyp_t[row].hasMatch = true;
				hyp_t[row].state = HYP_TRACKED;
			}
		}
		if (hyp_t[row].hasMatch == false){
			cout << "No match for hyp : " << hyp_t[row].id << " (disappear) " << endl;
			hyp_t[row].state = HYP_OCCLUDED;
		}
	}
	//Check for still non-matched new hypothesis (Appear Events automaticily)
	//Create hypothesis with mouse clicks
	if ( hypClicked ){
		hypClicked = false;
		int trackedX = tx;
		int trackedY = ty;
		//Find which region user clicked
		int i = 0;
		while( i < curHypNum ){
			//Found correct region user clicked
			if ( tx >= hyp_t_1[i].x0 && ty >= hyp_t_1[i].y0 
				&& tx <= hyp_t_1[i].x0 + hyp_t_1[i].width && ty <= hyp_t_1[i].y0 + hyp_t_1[i].height ){
					//Create a new hypothesis, appear event...
					hyp_t[preHypNum].trajColor = colors[ Hypothesis::objectNumber];
					hyp_t[preHypNum].id = Hypothesis::objectNumber;
					hyp_t[preHypNum].appear( hyp_t_1[i] );
					hyp_t[preHypNum].updateColor( bgMask, bins, time);
					float matchRatio = hyp_t[preHypNum].match( hyp_t_1[i] );
					corMatrix[preHypNum][i] = matchRatio;
					cout << "Created new Hypothesis. code (" << hyp_t[preHypNum].id <<") " << endl;
					preHypNum++;
					Hypothesis::objectNumber++;
					break;
			}
			i++;
		}
	}


	//Output Phase
	//Output correspondence matrix
	cout << "***[_CorrespondenceMat_]***" << endl;
	cout.precision(2);
	for(int row = 0; row < preHypNum ; row++){
		cout << endl;
		//cout << "[" << hyp_t[row].id << "] "; 
		for(int col = 0; col < curHypNum ; col++){
			cout << corMatrix[row][col] << " ";
		}
	}
	cout << endl;

	//Advance all hypothesis on time, (if occluded long time remove them )
	for(int row = 0; row < preHypNum ; row++){
		cout << "Hyp " << hyp_t[row].id << " :  vx " << hyp_t[row].vx << " vy  " << hyp_t[row].vy << " ang: " << hyp_t[row].ang << 
			" velmag veldir : " << hyp_t[row].velMag << " " << hyp_t[row].velDirection <<  endl;
		hyp_t[row].printTrack(visualImg);
		sprintf(text, "Hyp %d", hyp_t[row].id);
		cvPutText(visualImg, text, cvPoint(hyp_t[row].x0 + hyp_t[row].width /2 , hyp_t[row].y0 + hyp_t[row].height /2), &font, colors[hyp_t[row].id] );
		if ( hyp_t[row].lastSeenIndex == MAX_FRAME_OCCLUDED ){
			//Remove hypothesis from list...
		}
		else if(hyp_t[row].state == HYP_OCCLUDED){
			hyp_t[row].lastSeenIndex ++;	// 1 more frame we couldnt see the shit object.
		}
		else{ // If still tracking hypothesis
			hyp_t[row].time++;
		}
	}
}


void BlobDetectionEngine::setLogger(FILE *logger)
{
	trackFp = logger;
}
void BlobDetectionEngine::alternateMatching(bool **bgMask, int ***bins)
{
	curHypNum = numOfRegions;
	//@At that frame, accumulate region information on temporary hypothesises
	for(int hyp = 0; hyp < curHypNum ; hyp++){
		//Fill hyp_t_1 (matrix column HYP)
		hyp_t_1[hyp].ex_x0 = regRect[hyp].x + regRect[hyp].width/2;
		hyp_t_1[hyp].ex_y0 = regRect[hyp].y + regRect[hyp].height/2;
		hyp_t_1[hyp].ex_h = regRect[hyp].height;
		hyp_t_1[hyp].ex_w = regRect[hyp].width;
		hyp_t_1[hyp].x0 =	regRect[hyp].x + regRect[hyp].width/2;
		hyp_t_1[hyp].y0 = regRect[hyp].y + regRect[hyp].height/2;
		hyp_t_1[hyp].width = regRect[hyp].width;
		hyp_t_1[hyp].height = regRect[hyp].height;
		hyp_t_1[hyp].hasMatch = false;
		hyp_t_1[hyp].spatialId = rand(); //means new hyp
		//Uncomment if we dont need color histograms
		hyp_t_1[hyp].updateColor( bgMask, bins );
	}
	//Tag as previous hypothesis as no match yet.
	for(int hyp = 0; hyp < preHypNum ; hyp++){
		hyp_t[hyp].hasMatch = false;		
	}

	//Check for still non-matched new hypothesis (Appear Events automaticily)
	BlobDetectionEngine::createHypothesis(bgMask, bins);	

	//Check for matches
	for(int row = 0; row < preHypNum ; row++){
		for(int col = 0; col < curHypNum ; col++){
			if ( hyp_t[row].x0 >= (hyp_t_1[col].x0 - hyp_t_1[col].width / 2 ) && (hyp_t[row].y0 >= hyp_t_1[col].y0 - hyp_t_1[col].height/2) 
				&&  hyp_t[row].x0 <= (hyp_t_1[col].x0 + hyp_t_1[col].width / 2 ) && (hyp_t[row].y0 <= hyp_t_1[col].y0 + hyp_t_1[col].height/2) ){
					hyp_t[row].appear( hyp_t_1[col] );
					//printf("%d innershell modified...\n", row); 
					hyp_t_1[col].hasMatch = true;
					hyp_t[row].hasMatch = true;
					hyp_t[row].state = HYP_TRACKED;
					//cout << "match for currently tracked hyp " << hyp_t[row].id << " to : " << col << endl;
					//cout << " hyp " << hyp_t[row].x0 << " , " << hyp_t[row].y0 << endl;
			}
		}
		if (hyp_t[row].hasMatch == false){
			//cout << "No match for hyp : " << hyp_t[row].id << " (disappear) " << endl;
			//cout << "center : " << hyp_t[row].x0 << " "<<  hyp_t[row].y0  << " (disappear) " << endl;
			//cout << "center : " << hyp_t[row].ex_x0 << " "<<  hyp_t[row].ex_y0  << " (disappear) " << endl;
			hyp_t[row].state = HYP_OCCLUDED;
		}
	}
	//Event Resolution Analysis
	eventResolve();

	//if (Hypothesis::objectNumber == 2){
	//	cent1.x = hyp_t[0].x0;
	//	cent1.y = hyp_t[0].y0;
	//	cent2.x = hyp_t[1].x0;
	//	cent2.y = hyp_t[1].y0;
	//	bool intersecting = find_eu_distance( cent1, cent2) < ( hyp_t[0].innerShellRadius * 1.25 + hyp_t[1].innerShellRadius * 1.25);
	//	cout << " o2o distance : " << find_eu_distance( cent1, cent2) <<  "shelltotal " << ( hyp_t[0].innerShellRadius * 1.25 + hyp_t[1].innerShellRadius * 1.25) << endl;
	//}

	//Pixel Membership Evaluation
	pixelClassEvaluate(bgMask, bins);
	for(int row = 0; row < preHypNum ; row++){
		if ( !hyp_t[row].killed ){
			if ( hyp_t[row].hasInteract() )
				cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.25, CV_RGB(127,0,127), 1);
			if ( hyp_t[row].hasMerge() )
				cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.35, CV_RGB(0,0,255), 1);
			if ( !hyp_t[row].hasMerge() && !hyp_t[row].hasInteract() )	
				cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.25, CV_RGB(0,255,0), 1);
		}
	}
	//Advance all hypothesis on time, (if occluded long time remove them )
	for(int row = 0; row < preHypNum ; row++){
		//cout << "Hyp " << hyp_t[row].id << " :  vx " << hyp_t[row].vx << " vy  " << hyp_t[row].vy << " ang: " << hyp_t[row].ang << 
		//" velmag veldir : " << hyp_t[row].velMag << " " << hyp_t[row].velDirection <<  endl;
		cout << "Hyp " << hyp_t[row].id << " :  x " << hyp_t[row].x0 << " y  " << hyp_t[row].y0 << " innerShellRadius: " << hyp_t[row].innerShellRadius <<  endl;
		fprintf(trackFp, "%d ,%d ,%.2f ,%.2f \n", frameIdx, hyp_t[row].id, hyp_t[row].x0, hyp_t[row].y0 );
		//hyp_t[row].printTrack(visualImg);
		sprintf(text, "O%d", hyp_t[row].id);
		cvPutText(visualImg, text, cvPoint(hyp_t[row].x0 + hyp_t[row].width /2 , hyp_t[row].y0 + hyp_t[row].height /2), &font, colors[hyp_t[row].id] );

		if ( hyp_t[row].lastSeenIndex == MAX_FRAME_OCCLUDED ){
			//Remove hypothesis from list...
			memset(hyp_t[row].histogram, 0, sizeof(float) * 4096 );
			hyp_t[row].killed = true;
		}
		else if(hyp_t[row].state == HYP_OCCLUDED){
			hyp_t[row].lastSeenIndex ++;	// 1 more frame we couldnt see the shit object.
		}
		else{ // If still tracking hypothesis
			hyp_t[row].time++;
		}
	}

}
void BlobDetectionEngine::runBasicMatching(bool **bgMask, int ****bins, int *time)
{
	curHypNum = numOfRegions;
	//@At that frame, accumulate region information on temporary hypothesises
	for(int hyp = 0; hyp < curHypNum ; hyp++){
		//Fill hyp_t_1 (matrix column HYP)
		hyp_t_1[hyp].ex_x0 = regRect[hyp].x + regRect[hyp].width/2;
		hyp_t_1[hyp].ex_y0 = regRect[hyp].y + regRect[hyp].height/2;
		hyp_t_1[hyp].ex_h = regRect[hyp].height;
		hyp_t_1[hyp].ex_w = regRect[hyp].width;
		hyp_t_1[hyp].x0 =	regRect[hyp].x + regRect[hyp].width/2;
		hyp_t_1[hyp].y0 = regRect[hyp].y + regRect[hyp].height/2;
		hyp_t_1[hyp].width = regRect[hyp].width;
		hyp_t_1[hyp].height = regRect[hyp].height;
		hyp_t_1[hyp].hasMatch = false;
		hyp_t_1[hyp].spatialId = rand(); //means new hyp
		//Uncomment if we dont need color histograms
		hyp_t_1[hyp].updateColor( bgMask, bins, time );
	}
	//Tag as previous hypothesis as no match yet.
	for(int hyp = 0; hyp < preHypNum ; hyp++){
		hyp_t[hyp].hasMatch = false;		
	}

	//Check for still non-matched new hypothesis (Appear Events automaticily)
	BlobDetectionEngine::createHypothesis(bgMask, bins, time);

	//Check for matches
	for(int row = 0; row < preHypNum ; row++){
		for(int col = 0; col < curHypNum ; col++){
			if ( hyp_t[row].x0 >= (hyp_t_1[col].x0 - hyp_t_1[col].width / 2 ) && (hyp_t[row].y0 >= hyp_t_1[col].y0 - hyp_t_1[col].height/2) 
				&&  hyp_t[row].x0 <= (hyp_t_1[col].x0 + hyp_t_1[col].width / 2 ) && (hyp_t[row].y0 <= hyp_t_1[col].y0 + hyp_t_1[col].height/2) ){
					hyp_t[row].appear( hyp_t_1[col] );
					//printf("%d innershell modified...\n", row); 
					hyp_t_1[col].hasMatch = true;
					hyp_t[row].hasMatch = true;
					hyp_t[row].state = HYP_TRACKED;
					//cout << "match for currently tracked hyp " << hyp_t[row].id << " to : " << col << endl;
					//cout << " hyp " << hyp_t[row].x0 << " , " << hyp_t[row].y0 << endl;
			}
		}
		if (hyp_t[row].hasMatch == false){
			//cout << "No match for hyp : " << hyp_t[row].id << " (disappear) " << endl;
			//cout << "center : " << hyp_t[row].x0 << " "<<  hyp_t[row].y0  << " (disappear) " << endl;
			//cout << "center : " << hyp_t[row].ex_x0 << " "<<  hyp_t[row].ex_y0  << " (disappear) " << endl;
			hyp_t[row].state = HYP_OCCLUDED;
		}
	}

	//Event Resolution Analysis
	eventResolve();


	//checkShellInteractions();
	//checkStateUpdates();

	//Pixel Membership Evaluation
	//pixelClassEvaluate(bgMask, bins, time);

	//Track using color histogram correspondence and outer shell

	//Print outer shells 
	for(int row = 0; row < preHypNum ; row++){
		if ( hyp_t[row].hasInteract() )
			cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.25, CV_RGB(127,0,127), 1);
		if ( hyp_t[row].hasMerge() )
			cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.35, CV_RGB(0,0,255), 1);
		if ( !hyp_t[row].hasMerge() && !hyp_t[row].hasInteract() )	
			cvCircle(visualImg, cvPoint(hyp_t[row].x0, hyp_t[row].y0), hyp_t[row].innerShellRadius * 1.25, CV_RGB(0,255,0), 1);

	}


	for(int row = 0; row < preHypNum ; row++){
		//Make color likelihood for determining pixels...
		//hyp_t[row].updateCentroid(bgMask); 
		//hyp_t[row].updatePixelsCentroid(bgMask);
	}


	//Advance all hypothesis on time, (if occluded long time remove them )
	for(int row = 0; row < preHypNum ; row++){
		//cout << "Hyp " << hyp_t[row].id << " :  vx " << hyp_t[row].vx << " vy  " << hyp_t[row].vy << " ang: " << hyp_t[row].ang << 
		//" velmag veldir : " << hyp_t[row].velMag << " " << hyp_t[row].velDirection <<  endl;
		cout << "Hyp " << hyp_t[row].id << " :  x " << hyp_t[row].x0 << " y  " << hyp_t[row].y0 << " innerShellRadius: " << hyp_t[row].innerShellRadius <<  endl;
		//hyp_t[row].printTrack(visualImg);
		sprintf(text, "O%d", hyp_t[row].id);
		cvPutText(visualImg, text, cvPoint(hyp_t[row].x0 + hyp_t[row].width /2 , hyp_t[row].y0 + hyp_t[row].height /2), &font, colors[hyp_t[row].id] );

		if ( hyp_t[row].lastSeenIndex == MAX_FRAME_OCCLUDED ){
			//Remove hypothesis from list...
		}
		else if(hyp_t[row].state == HYP_OCCLUDED){
			hyp_t[row].lastSeenIndex ++;	// 1 more frame we couldnt see the shit object.
		}
		else{ // If still tracking hypothesis
			hyp_t[row].time++;
		}
	}
}


void BlobDetectionEngine::createHypothesis(bool **bgMask, int ****bins, int *time)
{
	//Create hypothesis with mouse clicks
	if ( hypClicked ){
		hypClicked = false;
		int trackedX = tx;
		int trackedY = ty;
		//Find which region user clicked
		int i = 0;
		while( i < curHypNum ){
			//Found correct region user clicked
			if ( tx >= (hyp_t_1[i].x0 - hyp_t_1[i].width / 2) && ty >= (hyp_t_1[i].y0 - hyp_t_1[i].height/2  )
				&& tx <= (hyp_t_1[i].x0 + hyp_t_1[i].width / 2) && ty <= (hyp_t_1[i].y0 + hyp_t_1[i].height/2 ) ){
					//Create a new hypothesis, appear event...
					hyp_t[preHypNum].trajColor = colors[ Hypothesis::objectNumber];
					hyp_t[preHypNum].id = Hypothesis::objectNumber;
					hyp_t[preHypNum].appear( hyp_t_1[i] );
					hyp_t[preHypNum].updateColor( bgMask, bins, time);
					//float matchRatio = hyp_t[preHypNum].match( hyp_t_1[i] );
					//corMatrix[preHypNum][i] = matchRatio;
					cout << "Created new Hypothesis. code (" << hyp_t[preHypNum].id <<") " << endl;
					preHypNum++;
					Hypothesis::objectNumber++;
					break;
			}
			i++;
		}
	}
}
void BlobDetectionEngine::createHypothesis(bool **bgMask, int ***bins)
{
	//Create hypothesis with mouse clicks
	if ( hypClicked ){
		hypClicked = false;
		int trackedX = tx;
		int trackedY = ty;
		//Find which region user clicked
		int i = 0;
		while( i < curHypNum ){
			//Found correct region user clicked
			if ( tx >= (hyp_t_1[i].x0 - hyp_t_1[i].width / 2) && ty >= (hyp_t_1[i].y0 - hyp_t_1[i].height/2  )
				&& tx <= (hyp_t_1[i].x0 + hyp_t_1[i].width / 2) && ty <= (hyp_t_1[i].y0 + hyp_t_1[i].height/2 ) ){
					//Create a new hypothesis, appear event...
					hyp_t[preHypNum].trajColor = colors[ Hypothesis::objectNumber];
					hyp_t[preHypNum].id = Hypothesis::objectNumber;
					hyp_t[preHypNum].appear( hyp_t_1[i] );
					hyp_t[preHypNum].updateColor( bgMask, bins);
					//float matchRatio = hyp_t[preHypNum].match( hyp_t_1[i] );
					//corMatrix[preHypNum][i] = matchRatio;
					cout << "Created new Hypothesis. code (" << hyp_t[preHypNum].id <<") " << endl;
					preHypNum++;
					Hypothesis::objectNumber++;
					break;
			}
			i++;
		}
	}
	if ( hypUpdated){
		hypUpdated = false;
		int trackedX = tx;
		int trackedY = ty;
		//Find which region user clicked
		int i = 0;
		while( i < preHypNum ){	
			//Found correct hyp user clicked
			if ( tx >= (hyp_t[i].x0 - hyp_t[i].width / 2) && ty >= (hyp_t[i].y0 - hyp_t[i].height/2  )
				&& tx <= (hyp_t[i].x0 + hyp_t[i].width / 2) && ty <= (hyp_t[i].y0 + hyp_t[i].height/2 ) ){
					hyp_t[i].updateColor( bgMask, bins);
					cout << "Updated ColorModel of object id : " << hyp_t[i].id <<") " << endl;
			}
			i++;
		}
	}
}
int BlobDetectionEngine::findIdxOfHyp(int idd)
{
	for(int q = 0; q < preHypNum; q++){
		if ( hyp_t[q].id == idd ){
			return q;
		}
	}
	return -1;
}

void BlobDetectionEngine::eventResolve()
{
	bool intersecting;
	int compId; //Compared hypothesis id
	set<int>::iterator it;
	for(int row = 0; row < preHypNum ; row++){
		for(int next = 0; next < preHypNum ; next++){
			compId = hyp_t[next].id;
			//jcout << "compId " <<compId << endl;
			if ( row != next ){
				if ( hyp_t[row].spatialId != hyp_t[next].spatialId ){
					//Compute shell interaction
					cent1.x = hyp_t[row].x0;
					cent1.y = hyp_t[row].y0;
					cent2.x = hyp_t[next].x0;
					cent2.y = hyp_t[next].y0;
					intersecting = find_eu_distance( cent1, cent2) < ( hyp_t[row].innerShellRadius * 1.25 + hyp_t[next].innerShellRadius * 1.25);
					if ( intersecting ){
						//Determine 4.0 or 2.0
						//int pos = std::find(hyp_t[row].merEvtSet.begin(), hyp_t[row].merEvtSet.end(), compId) - hyp_t[row].merEvtSet.;
						it = hyp_t[row].merEvtSet.find( compId );
						if( it != hyp_t[row].merEvtSet.end() ){
							cout << "obj " << hyp_t[row].id <<" stopped merge with " << hyp_t[next].id << endl;
							//std::cout << compId << " [i] is at position " << pos << '\n';
							//Equation 4.0 (M_i,j,t-1 found)
							hyp_t[row].intEvtSet.insert( compId );
							hyp_t[row].merEvtSet.erase( compId );

						}
						else{
							cout << "obj " << hyp_t[row].id <<" is in interaction with " << hyp_t[next].id << endl;
							//std::cout << compId << " [i] not found.\n";
							//Equation 2.0 (M_i,j,t-1 not found)
							hyp_t[row].intEvtSet.insert( compId );

							/* correct usage
							std::for_each(hyp_t[row].intEvtSet.begin(), hyp_t[row].intEvtSet.end(), printIterator);
							*/
							/*Shitty usage
							for (it = hyp_t[row].intEvtSet.begin(); it != hyp_t[row].intEvtSet.end(); it++){
							cout << *it << " " ;
							}
							cout << endl;
							*/

						}
					}
					else{
						it = hyp_t[row].intEvtSet.find( compId );
						if( it != hyp_t[row].intEvtSet.end() ){
							//std::cout << compId << " [m] is at position " << pos << '\n';
							//Eq 5.0 (I_i,j,t-1 found)
							cout << "obj " << hyp_t[row].id <<" stopped interaction with " << hyp_t[next].id << endl;
							hyp_t[row].intEvtSet.erase( compId );
						}
						else{
							//std::cout << compId << " [m] not found.\n";
							//Eq 1.0 (I_i,j,t-1 not found)
							//No action taken.
						}
					}
				}
				else{
					it =hyp_t[row].merEvtSet.find(compId);
					if( it != hyp_t[row].merEvtSet.end() ){
						//everything is ok. //merge continue
						//cout << "Object " << row << " and " << hyp_t[next].id << " is in mergestate ! " << endl;
					}
					else{
						it = hyp_t[row].intEvtSet.find(compId);
						//cout << "Searching " << compId << " in " << hyp_t[row].id << "'s interaction list ! " << endl;
						if( it != hyp_t[row].intEvtSet.end() ){
							//eq 3.0
							cout << "Object " << row << " and " << hyp_t[next].id << " merged ! " << endl;
							hyp_t[row].intEvtSet.erase( compId );
							hyp_t[row].merEvtSet.insert( compId );
						}	
						else{
							//Experimental
							hyp_t[row].merEvtSet.insert( compId );
							cout << "3.0 UNDEFINED STATE(Main assumption violated) obj pair {"<<hyp_t[row].id<<","<<hyp_t[next].id << "}" << endl;
						}
					}
				}

			}
		}
	}

}
void BlobDetectionEngine::printIterator (int i) {
	cout << " " << i;
}

void BlobDetectionEngine::pixelClassEvaluate(bool **mask, int ***bins)
{
	float shellRad;
	int r,g,b, cInd;
	set<int>::iterator it;
	for(int row = 0; row < preHypNum; row++){
		//CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
		int roi = ( hyp_t[row].innerShellRadius * 1.25 ) ;
		int xSum = 0, ySum = 0, numPt = 0;
		int yLim, xLim, yStart, xStart;
		float dist;
		float prior,likelihood, posterior;

		//Boundary Checks
		if ( hyp_t[row].y0  - roi <= 0 )
			yStart = 0;
		else
			yStart = hyp_t[row].y0 - roi;

		if ( hyp_t[row].y0 + roi >= Hypothesis::img_height - 1)
			yLim = Hypothesis::img_height - 1;
		else
			yLim = hyp_t[row].y0  + roi;

		if ( hyp_t[row].x0  - roi < 0 )
			xStart = 0;
		else
			xStart = hyp_t[row].x0  - roi;

		if ( hyp_t[row].x0  + roi >= Hypothesis::img_width - 1 )
			xLim = Hypothesis::img_width - 1;
		else
			xLim = hyp_t[row].x0  + roi;
		//Boundary Checks

		//Implementation of argmax c
		hyp_t[row].classUnionSet.insert( hyp_t[row].intEvtSet.begin(), hyp_t[row].intEvtSet.end() );
		hyp_t[row].classUnionSet.insert( hyp_t[row].merEvtSet.begin(), hyp_t[row].merEvtSet.end() );
		//cout << "Gonna search from {"<<yStart<<","<<xStart<<"} to {"<<yLim << "," << xLim << "} . " << endl;
		//cout << "ActualImageLimits {"<<0<<","<<0<<"} to {"<<Hypothesis::img_height << "," << Hypothesis::img_width << "} . " << endl;	
		//cvRectangle(visualImg, cvPoint( xStart, yStart ), cvPoint( xLim , yLim ), CV_RGB(0,255,0), 1);

		/*int step      = visualImg->widthStep;
		int channels  = visualImg->nChannels;
		uchar *data   = (uchar *)visualImg->imageData;*/
		for(int y = yStart ; y < yLim; y++){
			for(int x = xStart ; x < xLim ; x++){
				if ( mask[y][x] ){
					//Ask whether inside shell?
					dist = find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi);
					//Ask whether inside shell?
					if ( dist > 1 )
						continue;
					b = bins[y][x][0];
					g = bins[y][x][1];
					r = bins[y][x][2];
					cInd = b + g*16 + r*256;
					//Find max matched class
					int biggestIdx = hyp_t[row].id;
					//float priorDef = (float)(1.00 - (find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi))) ;
					float priorDef = (float)(1 - pow( find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi) , 2.0 ));
					float LikelihoodDef = (hyp_t[row].histogram[ cInd ]+ 1e-6) / hyp_t[row].pxInHistogram ;
					float biggestPost;
					//useposterior.
					biggestPost = LikelihoodDef * priorDef;

					for(int clas = 0; clas < preHypNum; clas++ ){
						it = hyp_t[row].classUnionSet.find( hyp_t[clas].id );
						if ( it != hyp_t[row].classUnionSet.end() ){ //Implementation of Index and C_i
							//Get pixel likelihood
							//cout <<"lulz " << endl;
							shellRad = hyp_t[clas].innerShellRadius * 1.25;
							likelihood = (hyp_t[clas].histogram[ cInd ]+ 1e-6) / hyp_t[clas].pxInHistogram;
							//prior = (float)(1.00 - find_eu_distance( cvPoint( hyp_t[clas].x0, hyp_t[clas].y0), cvPoint(x,y) ) / (shellRad)) ;
							prior = (float)(1 - pow( find_eu_distance( cvPoint( hyp_t[clas].x0, hyp_t[clas].y0), cvPoint(x,y) ) / (shellRad) , 2.0 ));

							//if (prior > 2.00 )
							//cout << "prior: " << prior << endl;

							if ( prior < 0)
								prior = 0;

							//posterior = likelihood ;
							//cout << "likelihood: " << likelihood << endl;
							posterior = likelihood * prior;
							if ( posterior > biggestPost ){
								biggestPost = posterior;
								biggestIdx = hyp_t[clas].id;
							}
						}
					}
					if ( biggestIdx == hyp_t[row].id ){
						//cout << "pix : {"<<y<<","<<x<<"} is class " << biggestIdx << " prob: " << LikelihoodDef <<  endl;
						//cvSet2D( visualImg, y , x, color);
						xSum += (x);		// multiply with mask to prevent if check.
						ySum += (y);
						numPt += (1);								
					}
					if (biggestIdx == 0)
						cvSet2D( visualImg, y , x, cvScalar(255,0,0,0));
					if (biggestIdx == 1)
						cvSet2D( visualImg, y , x, cvScalar(0,0,255,0));
					if (biggestIdx == 2)
						cvSet2D( visualImg, y , x, cvScalar(0,255,0,0));
					if (biggestIdx == 3)
						cvSet2D( visualImg, y , x, cvScalar(255,0,255,0));
					if (biggestIdx == 4)
						cvSet2D( visualImg, y , x, cvScalar(255,255,0,0));
					if (biggestIdx == 5)
						cvSet2D( visualImg, y , x, cvScalar(0,255,255,0));
				}

			}
		}
		hyp_t[row].ex_x0 = hyp_t[row].x0;
		hyp_t[row].ex_y0 = hyp_t[row].y0;
		if ( numPt != 0){
			hyp_t[row].x0 = xSum / numPt;
			hyp_t[row].y0 = ySum / numPt;
		}
		hyp_t[row].classUnionSet.clear();

	}

}


void BlobDetectionEngine::presentate(IplImage *hypOnly)
{
	for(int row = 0; row < preHypNum ; row++){
		//cout << "Hyp " << hyp_t[row].id << " :  x " << hyp_t[row].x0 << " y  " << hyp_t[row].y0 << " innerShellRadius: " << hyp_t[row].innerShellRadius <<  endl;
		//hyp_t[row].printTrack(visualImg);
		sprintf(text, "O%d", hyp_t[row].id);
		cvRectangle(hypOnly, cvPoint(hyp_t[row].x0 - hyp_t[row].width /2 , hyp_t[row].y0 - hyp_t[row].height /2), cvPoint(hyp_t[row].x0 + hyp_t[row].width /2 , hyp_t[row].y0 + hyp_t[row].height /2), hyp_t[row].trajColor );
		cvPutText(hypOnly, text, cvPoint(hyp_t[row].x0  , hyp_t[row].y0 ), &font, colors[hyp_t[row].id] );
	}

}
void BlobDetectionEngine::debugPixel(int y, int x, int ***bins)
{
	float shellRad;
	int r,g,b, cInd;
	set<int>::iterator it;
	for(int row = 0; row < preHypNum; row++){
		int roi = ( hyp_t[row].innerShellRadius * 1.25 ) ;
		float dist;
		float prior,likelihood, posterior;
		//Index function implementation
		hyp_t[row].classUnionSet.insert( hyp_t[row].intEvtSet.begin(), hyp_t[row].intEvtSet.end() );
		hyp_t[row].classUnionSet.insert( hyp_t[row].merEvtSet.begin(), hyp_t[row].merEvtSet.end() );
		dist = find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi);
		//Ask whether inside shell?
		if ( dist > 1 )
			continue;
		b = bins[y][x][0];
		g = bins[y][x][1];
		r = bins[y][x][2];
		cInd = b + g*16 + r*256;
		//Find max matched class
		int biggestIdx = hyp_t[row].id;
		//float priorDef = (float)(1.00 - (find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi))) ;
		float priorDef = (float)(1 - pow( find_eu_distance( cvPoint( hyp_t[row].x0, hyp_t[row].y0), cvPoint(x,y) ) / (roi) , 2.0 ));
		float LikelihoodDef = (hyp_t[row].histogram[ cInd ] + 1e-6) / hyp_t[row].pxInHistogram;
		float biggestPost;
		//usepost.
		biggestPost = LikelihoodDef * priorDef;
		cout << "pixel x,y : " << x << " " << y << " " << "bgr: " << b << " " << g << " " << r << " dist shellcore: " << dist << endl;	
		cout << "(ITER)class: " << hyp_t[row].id << " posterior: " << biggestPost << " likelihood: " << LikelihoodDef << " prior: " << priorDef <<  endl;

		for(int clas = 0; clas < preHypNum; clas++ ){
			it = hyp_t[row].classUnionSet.find( hyp_t[clas].id );
			if ( it != hyp_t[row].classUnionSet.end() ){ //Implementation of Index and C_i
				//Get pixel likelihood
				//cout <<"lulz " << endl;
				shellRad = hyp_t[clas].innerShellRadius * 1.25;
				likelihood = (hyp_t[clas].histogram[ cInd ]+ 1e-6) / hyp_t[clas].pxInHistogram;
				//prior = (float)(1.00 - find_eu_distance( cvPoint( hyp_t[clas].x0, hyp_t[clas].y0), cvPoint(x,y) ) / (shellRad)) ;
				prior = (float)(1 - pow( find_eu_distance( cvPoint( hyp_t[clas].x0, hyp_t[clas].y0), cvPoint(x,y) ) / (shellRad) , 2.0 ));

				//if (prior > 2.00 )
				//cout << "prior: " << prior << endl;

				if ( prior < 0)
					prior = 0;

				posterior = likelihood * prior;
				//cout << "likelihood: " << likelihood << endl;
				//posterior = likelihood * prior;
				cout << "class: " << hyp_t[clas].id << " posterior: " << posterior << " likelihood: " << likelihood << " prior: " << prior <<  endl;
				if ( posterior > biggestPost ){
					biggestPost = posterior;
					//LikelihoodDef = likelihood;
					biggestIdx = hyp_t[clas].id;
				}
			}
		}
		cout << "class " << biggestIdx << " has right to take that pixel..." << endl;
	}

	for(int row = 0; row < preHypNum; row++){
		hyp_t[row].classUnionSet.clear();
	}


}
