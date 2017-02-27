#include "stdafx.h"
// TLR_Lara.cpp : Defines the entry point for the console application.
//
/*
#include "stdafx.h"
#include "cuda/CudaImageMorphology.h"
#include "cuda/CudaImageSergmentation.h"
#include "CudaTimer.h"
#include "CpuTimer.h"
#include "BlobLabeling.h"
#include "../ObjectDetectorLib/util/ImageUtil.h"

using namespace std;

CpuTimer cpuTimer;
CudaTimer cudaTimer;
ImageUtil iUtil;
CBlobLabeling blobDetector;

bool checkAspectRatio( CvRect rt );
bool checkBulbValidation( IplImage *src, CvRect rt, int mode );
void colorSegmentation( IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap );
vector<CvRect> blobDetection( IplImage *ryMap );

int _tmain(int argc, _TCHAR* argv[])
{
	char buf[100];
	int stx = 4840;
	IplImage *src = NULL;
	IplImage *hsv = NULL;
	IplImage *res = NULL;
	IplImage *redMap = NULL;
	IplImage *greenMap = NULL;
	IplImage *yellowMap = NULL;
	IplImage *ryMap = NULL;

	for( int i = stx; i <= 11178; i++ ) {
		sprintf_s( buf, "D:/TSET/Lara/%.6d.jpg", i);
		IplImage *img = cvLoadImage( buf );
		if( i == stx ) {
			src = cvCreateImage( cvSize(img->width*2, img->height*2), 8, 3 );
			hsv = cvCreateImage( cvSize(img->width*2, img->height*2), 8, 3 );
			res = cvCreateImage( cvSize(img->width*2, img->height*2), 8, 3 );
			redMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			greenMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			yellowMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			ryMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			initSegmentationModule( src->width, src->height );
		}		
		
		cvResize( img, src );
		cvCvtColor( src, hsv, CV_BGR2HSV );
		
		cvResize( img, res );
		cpuTimer.record();

		colorSegmentation( hsv, redMap, greenMap, yellowMap, ryMap );
		vector<CvRect> redBlobs = blobDetection( ryMap );
		vector<CvRect> greenBlobs = blobDetection( greenMap );
		bool isRedOn = false;
		bool isGreenOn = false;

		for( int i = 0; i < redBlobs.size(); i++ ) {			
			CvPoint p1, p2;
			p1.x = redBlobs.at(i).x;
			p1.y = redBlobs.at(i).y;
			p2.x = p1.x + redBlobs.at(i).width;
			p2.y = p1.y + redBlobs.at(i).height;

			if( p1.y >= src->height*0.35 ) continue;
			if( !checkAspectRatio( redBlobs.at(i) ) ) continue;
			if( !checkBulbValidation( src, redBlobs.at(i), 1 ) ) continue;

			CvPoint pCircle;
			pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
			pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
			cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
			cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(255,0,0), 2);

			cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 2);
			cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(255,0,0), 1);
			isRedOn = true;
			//cvDrawRect( res, p1, p2, CV_RGB(0,0,0), 3 );
			//cvDrawRect( res, p1, p2, CV_RGB(255,0,0), 2 );
		}

		for( int i = 0; i < greenBlobs.size(); i++ ) {			
			CvPoint p1, p2;
			p1.x = greenBlobs.at(i).x;
			p1.y = greenBlobs.at(i).y;
			p2.x = p1.x + greenBlobs.at(i).width;
			p2.y = p1.y + greenBlobs.at(i).height;

			if( p1.y >= src->height*0.35 ) continue;
			if( !checkAspectRatio( greenBlobs.at(i) ) ) continue;
			if( !checkBulbValidation( src, greenBlobs.at(i), 2 ) ) continue;
			CvPoint pCircle;
			pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
			pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
			cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
			cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,255,0), 2);

			cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 2);
			cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,255,0), 1);

			isGreenOn = true;
			//cvDrawRect( res, p1, p2, CV_RGB(0,0,0), 3 );
			//cvDrawRect( res, p1, p2, CV_RGB(0,255,0), 2 );
		}
		cpuTimer.stop();


		const int cLength = 50;
		int boxWidth = cLength*6;
		int boxHeight = cLength * 2;
		CvPoint pBox1( res->width / 2 - (boxWidth/2), 400 );
		CvPoint pBox2( res->width / 2 + (boxWidth/2), 400 + boxHeight );

		cvDrawRect( res, pBox1, pBox2, CV_RGB(20,20,20), CV_FILLED );
		cvDrawRect( res, pBox1, pBox2, CV_RGB(100,100,100), 1 );


		CvPoint pRed( pBox1.x + cLength*1, pBox1.y + cLength);
		CvPoint pYellow( pRed.x + cLength*2, pRed.y );
		CvPoint pGreen( pYellow.x + cLength*2, pYellow.y );

		cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
		cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
		cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);


		if( isRedOn ) {
			cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), CV_FILLED);
			cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
			cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
		} 
		if( isGreenOn ) {
			cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
			cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
			cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), CV_FILLED);
		}

		printf("\r%2.2fms [%d/%d]", cpuTimer.elapsed_time_ms, (i+1), 11178);
		sprintf_s( buf, "D:/Lara_Result/%.6d.jpg", i);
		cvSaveImage(buf, res);
		cvShowImage( "Result", res);
		cvWaitKey(1);

		cvReleaseImage( &img );
	}

	freeSegmentationModule();
	cvReleaseImage( &src );
	cvReleaseImage( &res );
	cvReleaseImage( &redMap );
	cvReleaseImage( &greenMap );
	cvReleaseImage( &yellowMap );
	cvReleaseImage( &ryMap );

	return 0;
}

bool checkBulbValidation( IplImage *src, CvRect rt, int mode )
{
	int x, y, width, height;
	if( mode == 1 ) {
		// Case :: Red
		x = rt.x + rt.width * 0.2; 
		y = rt.y + rt.height; 
		width = rt.width *0.6; 
		height = rt.height * 2;
	} else if( mode == 2 ) {
		// Case :: Green
		x = rt.x; 
		y = rt.y - rt.height * 2; 
		width = rt.width; 
		height = rt.height * 2;
	}
	if( y < 0 ) y = 0;
	if( y + height > src->height ) height -= y + height - src->height;	
	const int avgVal = iUtil.getAvgRGB( src, x, y, width, height );
	//printf("Mode=%d, (%d,%d) = %d\n", mode, rt.x, rt.y, avgVal );
	if( avgVal <= 100 ) return true;
	return false;
}

bool checkAspectRatio( CvRect rt )
{
	if( rt.width <= 9 || rt.height <= 9 ) return false;
	if( rt.width > rt.height ) {
		if( abs( (float)rt.height / (float)rt.width ) <= 0.7 ) return false;
	} else {
		if( abs( (float)rt.width / (float)rt.height ) <= 0.7 ) return false;
	}
	return true;
}

vector<CvRect> blobDetection( IplImage *ryMap )
{
	blobDetector.SetParam( ryMap, 1 );
	blobDetector.DoLabeling();
	vector<CvRect> blobs;
	for( int i = 0; i < blobDetector.m_nBlobs; i++ ) {
		blobs.push_back( blobDetector.m_recBlobs[i] );		
	}

	return blobs;
}

void colorSegmentation( IplImage *hsv, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap )
{
	float baseColor[4][3] = { 
		{ 171.8, 193.5 },	// RED
		{ 89.5, 58.8 },		// GREEN1
		{ 71.2, 151.2 },	// GREEN2
		{ 19.0, 221.8 }		// YELLOW
	};

	float *rgyRange = (float*)malloc( sizeof(float) * 12 );	
	memcpy(rgyRange, baseColor, sizeof(float)*12);

	gpu_hsvSegmentation( hsv->imageData, redMap->imageData, greenMap->imageData, yellowMap->imageData, ryMap->imageData, rgyRange, hsv->width, hsv->height );

	gpu_erode( ryMap->imageData, ryMap->imageData, ryMap->width, ryMap->height, 3);
	gpu_dilate( ryMap->imageData, ryMap->imageData, ryMap->width, ryMap->height, 9);

	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
	gpu_dilate( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 9);	
	

	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);

	cvShowImage( "GPU RYMap", ryMap);	
	//cvShowImage( "GPU RedMap", redMap);	
	cvShowImage( "GPU GreenMap", greenMap);	

	free( rgyRange );


}
*/