#include "stdafx.h"
/*
#include "cuda/CudaImageMorphology.h"
#include "cuda/CudaImageSergmentation.h"
#include "CudaTimer.h"
#include "CpuTimer.h"
#include "BlobLabeling.h"
#include "../ObjectDetectorLib/util/ImageUtil.h"
#include "KylimWrapper.h"


//#define _SAVE_RESULT

using namespace std;

CpuTimer cpuTimer;
CudaTimer cudaTimer;
ImageUtil iUtil;
CBlobLabeling blobDetector;


bool checkAspectRatio( CvRect rt );
CvRect getTrafficLightRect( CvRect rt, int mode );
bool checkBulbValidation( IplImage *src, CvRect rt, int mode );
IplImage* getBulbRoiImage( IplImage *src, CvRect rt, int mode );
void saveBulbRoiImage( IplImage *src, CvRect rt, int frameSeq, int mode );
void colorSegmentation( IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap );
vector<CvRect> blobDetection( IplImage *ryMap );


int _tmain(int argc, _TCHAR* argv[])
{

#ifdef _SAVE_RESULT
	char resFileName[1000];
	sprintf(resFileName,"D:/LARA_RESULT.csv");
	FILE *resFp;
	fopen_s( &resFp, resFileName, "w" );
#endif

	char buf[100];
	int stx = 1900;
	IplImage *src = NULL;
	IplImage *res = NULL;
	IplImage *redMap = NULL;
	IplImage *greenMap = NULL;
	IplImage *yellowMap = NULL;
	IplImage *ryMap = NULL;
	const double scaleFactor = 1.5;

	CvFont				m_Font;
	CvPoint				m_txtPoint(10,30);
	char				m_errTxt[100];

	cvInitFont( &m_Font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1 );	

	// Step :: Init CNN Module
	initCnn();

	for( int frameIdx = stx; frameIdx <= 4059; frameIdx++ ) {	

		sprintf_s( buf, "D:/TSET/VIVA_TLR/DAY_TESTSET/%.5d.jpg", frameIdx);
		IplImage *srcImg = cvLoadImage( buf );
		IplImage *croppedImg = iUtil.getRoiFrame(srcImg,0,0,srcImg->height*0.5,srcImg->width,0);

		if( frameIdx == stx ) {
			_initModule( 1, 1, croppedImg->width, croppedImg->height, 162, 82, 2 );
			src = cvCreateImage( cvSize(croppedImg->width*scaleFactor, croppedImg->height*scaleFactor), 8, 3 );
			res = cvCreateImage( cvSize(srcImg->width, srcImg->height), 8, 3 );
			redMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			greenMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			yellowMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			ryMap = cvCreateImage( cvGetSize( src ), 8, 1 );
			initSegmentationModule( src->width, src->height );
		}		
		_enhanceImage( croppedImg, croppedImg, 1.6 , 0 );
		cvResize( croppedImg, src );		
		cvResize( srcImg, res );
		cvReleaseImage(&srcImg);
		cvShowImage("C", src);
		cpuTimer.record();

		colorSegmentation( src, redMap, greenMap, yellowMap, ryMap );
		vector<CvRect> redBlobs = blobDetection( ryMap );
		vector<CvRect> greenBlobs = blobDetection( greenMap );
		bool isRedOn = false;
		bool isWarningOn = false;
		bool isGreenOn = false;
		double redAreaSum = 0.0;
		double warningAreaSum = 0.0;
		double greenAreaSum = 0.0;

		for( int i = 0; i < redBlobs.size(); i++ ) {			
			CvPoint p1, p2;
			p1.x = redBlobs.at(i).x / scaleFactor;
			p1.y = redBlobs.at(i).y / scaleFactor;
			p2.x = p1.x + redBlobs.at(i).width / scaleFactor;
			p2.y = p1.y + redBlobs.at(i).height / scaleFactor;

			//if( p1.y >= src->height*0.35 ) continue;
			if( !checkAspectRatio( redBlobs.at(i) ) ) continue;
			if( !checkBulbValidation( src, redBlobs.at(i), 1 ) ) continue;

			IplImage *roiImg = getBulbRoiImage( src, redBlobs.at(i), 1);
			int classNo = classify( roiImg );
			cvReleaseImage(&roiImg);

			if( classNo == 1 ) {

				saveBulbRoiImage( src, redBlobs.at(i), frameIdx, 1 );
				CvPoint pCircle;
				pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
				pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(255,0,0), 1);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(255,0,0), 2);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(0,0,0), 4);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(255,0,0), 1);
				isRedOn = true;
				redAreaSum += p2.x * p2.y;
#ifdef _SAVE_RESULT			
				CvRect tlRect = getTrafficLightRect(redBlobs.at(i), 1);
				fprintf_s( resFp,"%d,%d,%d,%d,%d,stop\n", 
					frameIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor)
					);				
				fflush( resFp );
#endif	

			} else if( classNo == 4 ) {

				saveBulbRoiImage( src, redBlobs.at(i), frameIdx, 5 );
				CvPoint pCircle;
				pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
				pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(255,255,0), 1);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(255,255,0), 2);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(0,0,0), 4);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(255,255,0), 1);
				isWarningOn = true;
				warningAreaSum += p2.x * p2.y;
#ifdef _SAVE_RESULT			
				CvRect tlRect = getTrafficLightRect(redBlobs.at(i), 1);
				fprintf_s( resFp,"%d,%d,%d,%d,%d,warning\n", 
					frameIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor)
					);				
				fflush( resFp );
#endif	

			}
			cvReleaseImage(&roiImg);


			//cvDrawRect( res, p1, p2, CV_RGB(0,0,0), 3 );
			//cvDrawRect( res, p1, p2, CV_RGB(255,0,0), 2 );

		}

		for( int i = 0; i < greenBlobs.size(); i++ ) {			
			CvPoint p1, p2;
			p1.x = greenBlobs.at(i).x / scaleFactor;;
			p1.y = greenBlobs.at(i).y / scaleFactor;;
			p2.x = p1.x + greenBlobs.at(i).width / scaleFactor;;
			p2.y = p1.y + greenBlobs.at(i).height / scaleFactor;;

			//if( p1.y >= src->height*0.35 ) continue;
			if( !checkAspectRatio( greenBlobs.at(i) ) ) continue;
			if( !checkBulbValidation( src, greenBlobs.at(i), 2 ) ) continue;

			IplImage *roiImg = getBulbRoiImage( src, greenBlobs.at(i), 2);
			int classNo = classify( roiImg );
			cvReleaseImage(&roiImg);
			if( classNo == 2 ) {

				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 2 );
				CvPoint pCircle;
				pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
				pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,255,0), 1);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 3);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x ), CV_RGB(0,255,0), 2);

				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(0,0,0), 4);
				cvDrawCircle( res, pCircle, ( p2.x - p1.x )*2, CV_RGB(0,255,0), 2);

				isGreenOn = true;
				greenAreaSum += p2.x * p2.y;
#ifdef _SAVE_RESULT
				CvRect tlRect = getTrafficLightRect(greenBlobs.at(i), 2);
				fprintf_s( resFp,"%d,%d,%d,%d,%d,go\n", 
					frameIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor)
					);				
				fflush( resFp );
#endif
			} else if( classNo == 3 ) {
				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 4 );					
			} else {				
				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 6 );
			}
			//cvDrawRect( res, p1, p2, CV_RGB(0,0,0), 3 );
			//cvDrawRect( res, p1, p2, CV_RGB(0,255,0), 2 );

		}
		cpuTimer.stop();



		const int cLength = 30;
		int boxWidth = cLength * 6;
		int boxHeight = cLength * 2;
		CvPoint pBox1( res->width / 2 - (boxWidth/2), res->height/2 );
		CvPoint pBox2( res->width / 2 + (boxWidth/2), res->height/2 + boxHeight );

		cvDrawRect( res, pBox1, pBox2, CV_RGB(20,20,20), CV_FILLED );
		cvDrawRect( res, pBox1, pBox2, CV_RGB(100,100,100), 1 );

		CvPoint pRed( pBox1.x + cLength*1, pBox1.y + cLength);
		CvPoint pYellow( pRed.x + cLength*2, pRed.y );
		CvPoint pGreen( pYellow.x + cLength*2, pYellow.y );

		cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
		cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
		cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);


		if( redAreaSum > warningAreaSum && redAreaSum > greenAreaSum )  {
			cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), CV_FILLED);
			cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
			cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
		} 
		if( warningAreaSum > redAreaSum && warningAreaSum > greenAreaSum )  {
			cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
			cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), CV_FILLED);
			cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
		} 
		if( greenAreaSum > redAreaSum && greenAreaSum > warningAreaSum )  {
			cvDrawCircle( res, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
			cvDrawCircle( res, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
			cvDrawCircle( res, pGreen, cLength*0.9, CV_RGB(0,255,0), CV_FILLED);
		}

		// Display fps
		sprintf_s( m_errTxt, "[%5d/%5d] %3d fps , %2.2fms", 
			(frameIdx+1), 11178,
			(int)(1000.0 / cpuTimer.elapsed_time_ms), 
			cpuTimer.elapsed_time_ms);
		cvPutText( res, m_errTxt, m_txtPoint, &m_Font, CV_RGB(0,255,0) );

		printf("\r[%5d/%5d] %3d fps , %2.2fms", 
			(frameIdx+1), 11178,
			(int)(1000.0 / cpuTimer.elapsed_time_ms), 
			cpuTimer.elapsed_time_ms);
#ifdef _SAVE_RESULT
		sprintf_s( buf, "D:/Lara_Result/%.6d.jpg", frameIdx);
		cvSaveImage(buf, res);

#endif		
		cvShowImage( "Result", res);
		cvWaitKey(1);

		cvReleaseImage( &croppedImg );
	}

#ifdef _SAVE_RESULT
	fclose( resFp );
#endif
	freeSegmentationModule();
	cvReleaseImage( &src );
	cvReleaseImage( &res );
	cvReleaseImage( &redMap );
	cvReleaseImage( &greenMap );
	cvReleaseImage( &yellowMap );
	cvReleaseImage( &ryMap );

	return 0;
}

CvRect getTrafficLightRect( CvRect rt, int mode )
{
	int x, y, width, height;
	const double marginRate = 0.2;
	if( mode == 1 ) {
		// Case :: Red
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	} else if( mode == 2 ) {
		// Case :: Green
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * 2 - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	}
	return CvRect(x,y,width,height);
}

IplImage* getBulbRoiImage( IplImage *src, CvRect rt, int mode )
{
	int x, y, width, height;
	const double marginRate = 0.2;
	if( mode == 1 ) {
		// Case :: Red
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	} else if( mode == 2 ) {
		// Case :: Green
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * 2 - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	}
	IplImage *roi = iUtil.getRoiFrame( src, y, x, y+height, x+width, 3 );
	return roi;
}

void saveBulbRoiImage( IplImage *src, CvRect rt, int frameSeq, int mode )
{
	static int roiIdx = 0;
	int x, y, width, height;
	if( mode == 1 || mode == 3 || mode == 5 ) {
		// Case :: Red
		x = rt.x - rt.width * 0.2; 
		y = rt.y - rt.height * 0.2; 
		width = rt.width + rt.width * 0.4; 
		height = rt.height * 3 + rt.height * 0.4;
	} else if( mode == 2 || mode == 4 || mode == 6 ) {
		// Case :: Green
		x = rt.x - rt.width * 0.2; 
		y = rt.y - rt.height * 2 - rt.height * 0.2; 
		width = rt.width + rt.width * 0.4; 
		height = rt.height * 3 + rt.height * 0.4;
	}
	IplImage *roi = iUtil.getRoiFrame( src, y, x, y+height, x+width, 3 );
	char buf[100];
	if( mode == 1 )
		sprintf_s(buf,"d:/tset/lara_rois/red/%d_%d.jpg", frameSeq, roiIdx++);
	if( mode == 3 )
		sprintf_s(buf,"d:/tset/lara_rois/red-am/%d_%d.jpg", frameSeq, roiIdx++);
	else if( mode == 2 )
		sprintf_s(buf,"d:/tset/lara_rois/green/%d_%d.jpg", frameSeq, roiIdx++);
	else if( mode == 4 )
		sprintf_s(buf,"d:/tset/lara_rois/green-am/%d_%d.jpg", frameSeq, roiIdx++);
	else if( mode == 5 )
		sprintf_s(buf,"d:/tset/lara_rois/yellow/%d_%d.jpg", frameSeq, roiIdx++);
	else if( mode == 6 )
		sprintf_s(buf,"d:/tset/lara_rois/etc/%d_%d.jpg", frameSeq, roiIdx++);

	cvSaveImage(buf,roi);
	cvReleaseImage(&roi);
}

bool checkBulbValidation( IplImage *src, CvRect rt, int mode )
{
	int x, y, width, height;
	double marginRate = 0.2;
	if( mode == 1 ) {
		// Case :: Red
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	} else if( mode == 2 ) {
		// Case :: Green
		x = rt.x - rt.width * marginRate; 
		y = rt.y - rt.height * 2 - rt.height * marginRate; 
		width = rt.width + rt.width * marginRate*2; 
		height = rt.height * 3 + rt.height * marginRate*2;
	}
	if( y < 0 ) y = 0;
	if( y + height > src->height ) height -= y + height - src->height;	
	const int avgVal = iUtil.getAvgRGB( src, x, y, width, height );
	//printf("Mode=%d, (%d,%d) = %d\n", mode, rt.x, rt.y, avgVal );
	if( mode == 1 && avgVal <= 100 ) return true;
	if( mode == 2 && avgVal <= 100 ) return true;
	return false;
}

bool checkAspectRatio( CvRect rt )
{
	if( rt.width <= 9 || rt.height <= 9 ) return false;
	if( rt.width > rt.height ) {
		if( abs( (float)rt.height / (float)rt.width ) <= 0.6 ) return false;
	} else {
		if( abs( (float)rt.width / (float)rt.height ) <= 0.6 ) return false;
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

void colorSegmentation( IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap )
{
	float baseColor[3][3] = { 
		{  80.0, 80.0, 240.0 },    // Red light
		//{ 230.0, 245.0, 80.0 },	  // Green light
		{ 230.0, 245.0, 80.0 },		// Green light 2
		{  33.7, 196.2, 235.6 } };  // Yellow light

	float *rgyRange = (float*)malloc( sizeof(float) * 9 );	
	memcpy(rgyRange, baseColor, sizeof(float)*9);

	gpu_segmentation( src->imageData, redMap->imageData, greenMap->imageData, yellowMap->imageData, ryMap->imageData, rgyRange, src->width, src->height );

	gpu_dilate( ryMap->imageData, ryMap->imageData, ryMap->width, ryMap->height, 9);

	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
	gpu_dilate( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 9);
	//gpu_dilate( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 5);

	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);

	cvShowImage( "GPU RYMap", ryMap);	
	//cvShowImage( "GPU GreenMap", greenMap);	

	free( rgyRange );
}
*/