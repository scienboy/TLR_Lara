#include "stdafx.h"
//#include "cuda/CudaImageMorphology.h"
//#include "cuda/CudaImageSergmentation.h"
//#include "CudaTimer.h"
//#include "CpuTimer.h"
//#include "BlobLabeling.h"
//#include "../ObjectDetectorLib/util/ImageUtil.h"
//#include "KylimWrapper.h"
//
//#define _SAVE_RESULT				// save할 때 값을 저장 => 어느위치에 있다.
//#define _SAVE_RESULT_IMAGE			// 디스플레이된걸 하드에 저장하는거
//#define _SAVE_ROI_IMAGE			// 잡히는걸(ROI) 하드에 저장
//
//using namespace std;
//
//CpuTimer cpuTimer;				// CPU 타이머 쓰기 위함
//CudaTimer cudaTimer;			// CUDA 타이머 쓰기 위함
//ImageUtil iUtil;
//CBlobLabeling blobDetector;
//
//bool checkAspectRatio( CvRect rt );
//CvRect getTrafficLightRect( CvRect rt, int mode );
//bool checkBulbValidation( IplImage *src, CvRect rt, int mode );
//IplImage* getBulbRoiImage( IplImage *src, CvRect rt, int mode );
//void saveBulbRoiImage( IplImage *src, CvRect rt, int frameSeq, int mode );
//void colorSegmentation( IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap );
//vector<CvRect> blobDetection( IplImage *ryMap );
//void drawBulbCircle( IplImage *img, CvRect rt, float scaleFactor, CvScalar color );
//void drawBulbRect( IplImage *img, CvRect rt, float scaleFactor, int signalType );
//void drawTrafficLightResult( IplImage *img, int flag, int isTracked );
//vector<CvRect> getTrackedResult( int frameIdx, int &isTracked );
//void saveResults( int fIdx );
//void performanceTest( int mode );
//
//vector<CvRect> m_validBlobs[12000];
//int m_validBlobClass[12000] = { 0, };
//double m_procMs[12000] = { 0.0, };
//int gt_class[12000] = {-1,};
//int res_class[12000] = {-1,};
//int gt_width[12000] = {-1,};
//int res_width[12000] = {-1,};
//
//CvFont				m_Font;
//CvPoint				m_txtPoint(10,30);
//char				m_errTxt[100];
//FILE *resFp;
//const int stx = 7562;
//const int etx = 11178;
//const double scaleFactor = 1.5;
//
//int _tmain(int argc, _TCHAR* argv[])		// _tmain: unicode를 지원하기 위해 main을 변경한 것
//{
//	int mode = 0;
//	main:
//	printf("Traffic Light Recognition Machine\n 1: Processing\n 2: Performance Test\n");
//	scanf("%d",&mode);
//	
//	if( mode == 2 )			// 2. Performance Test
//	{
//		performanceTest( 2 );	// GT와 테스트결과를 비교하여 성능측정. 1번 프로세싱을 한번은 해서 csv가 떨어져야만 그걸 토대로 비교
//		goto main;
//		return 0;
//	}
//
//#ifdef _SAVE_RESULT
//	char resFileName[1000];
//	sprintf(resFileName,"E:/ExperimentResult/LARA_RESULT_TRACKED.csv");
//	fopen_s( &resFp, resFileName, "w" );
//#endif
//
//	char buf[100];
//	
//	IplImage *src = NULL;
//	IplImage *res = NULL;
//	IplImage *redMap = NULL;
//	IplImage *greenMap = NULL;
//	IplImage *yellowMap = NULL;
//	IplImage *ryMap = NULL;
//	double totalProcessingTime = 0.0;
//	cvInitFont( &m_Font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1 );	
//
//	// Step :: Init CNN Module	--> stdafx.h로 광용형이 미리 컴파일해둠
//	initCnn();
//	//11178
//	for( int frameIdx = stx; frameIdx <= etx; frameIdx++ ) {	
//		
//		sprintf_s( buf, "E:/Lara_TLR_BenchmardkDB/Lara3D_UrbanSeq1_JPG/frame_%d.jpg", frameIdx);
//		IplImage *srcImg = cvLoadImage( buf );
//		IplImage *croppedImg = iUtil.getRoiFrame(srcImg,0,0,srcImg->height*0.38,srcImg->width,0);
//		
//		if( frameIdx == stx ) {
//			src = cvCreateImage( cvSize(croppedImg->width*scaleFactor, croppedImg->height*scaleFactor), 8, 3 );
//			res = cvCreateImage( cvSize(srcImg->width, srcImg->height), 8, 3 );
//			redMap = cvCreateImage( cvGetSize( src ), 8, 1 );
//			greenMap = cvCreateImage( cvGetSize( src ), 8, 1 );
//			yellowMap = cvCreateImage( cvGetSize( src ), 8, 1 );
//			ryMap = cvCreateImage( cvGetSize( src ), 8, 1 );
//			initSegmentationModule( src->width, src->height );
//		}		
//
//		cvResize( croppedImg, src );		
//		cvResize( srcImg, res );
//		cvReleaseImage(&srcImg);
//
//		cpuTimer.record();
//
//		colorSegmentation( src, redMap, greenMap, yellowMap, ryMap );			// gpu segmentation함수
//		vector<CvRect> redBlobs = blobDetection( ryMap );			
//		vector<CvRect> greenBlobs = blobDetection( greenMap );
//		vector<CvRect> validRedBlobs; 
//		vector<CvRect> validGreenBlobs;
//		vector<CvRect> validYellowBlobs;
//
//		//cvShowImage("redBlobs", &redBlobs);
//		//cvShowImage("greenBlobs", &greenBlobs);
//		
//		// Step :: RED, YELLOW 블롭에 대한 인식 및 검증
//		for( int i = 0; i < redBlobs.size(); i++ ) {			
//			if( !checkAspectRatio( redBlobs.at(i) ) ) continue;
//			//if( !checkBulbValidation( src, redBlobs.at(i), 1 ) ) continue;
//			
//			IplImage *roiImg = getBulbRoiImage( src, redBlobs.at(i), 1);
//			int classNo = classify( roiImg );		// cnn
//			cvReleaseImage(&roiImg);
//
//			if( classNo == 1 ) {			// 클래시파이 끝내서 나온거가지고 classNo  --> 1:red, 2:yellow, 3:green
//#ifdef _SAVE_ROI_IMAGE					
//				saveBulbRoiImage( src, redBlobs.at(i), frameIdx, 1 );			// roi 저장 할지여부
//#endif
//				validRedBlobs.push_back( redBlobs.at(i) );
//			} else if( classNo == 4 ) {			
//#ifdef _SAVE_ROI_IMAGE
//				saveBulbRoiImage( src, redBlobs.at(i), frameIdx, 5 );
//#endif
//				validYellowBlobs.push_back( redBlobs.at(i) );
//			}
//			cvReleaseImage(&roiImg);
//		}
//
//		// Step :: GREEN 블롭에 대한 인식 및 검증
//		for( int i = 0; i < greenBlobs.size(); i++ ) {			
//			if( !checkAspectRatio( greenBlobs.at(i) ) ) continue;			
//			//if( !checkBulbValidation( src, greenBlobs.at(i), 2 ) ) continue;
//			IplImage *roiImg = getBulbRoiImage( src, greenBlobs.at(i), 2);
//			int classNo = classify( roiImg );
//			cvReleaseImage(&roiImg);
//			if( classNo == 2 ) {			
//#ifdef _SAVE_ROI_IMAGE
//				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 2 );
//#endif
//				validGreenBlobs.push_back( greenBlobs.at(i) );
//			} else if( classNo == 3 ) {
//#ifdef _SAVE_ROI_IMAGE
//				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 4 );					
//#endif
//			} else {				
//#ifdef _SAVE_ROI_IMAGE
//				saveBulbRoiImage( src, greenBlobs.at(i), frameIdx, 6 );
//#endif
//			}
//			
//		}
//
//		int redArea = 0;			// 블롭들의 width height 면적누적
//		int greenArea = 0;
//		int yellowArea = 0;
//		for( int i = 0; i < validRedBlobs.size(); i++ )				// 여기서 누산함. 여기서 맥스 찾아서 신호인식 
//			redArea += validRedBlobs.at(i).width * validRedBlobs.at(i).height;		
//		for( int i = 0; i < validGreenBlobs.size(); i++ ) 
//			greenArea += validGreenBlobs.at(i).width * validGreenBlobs.at(i).height;		
//		for( int i = 0; i < validYellowBlobs.size(); i++ ) 
//			yellowArea += validYellowBlobs.at(i).width * validYellowBlobs.at(i).height;
//		
//		//여기서 어느 bulb신호가 가장 면적이 큰지 판별
//		if( redArea > greenArea && redArea > yellowArea ) {
//			// Case :: RED			
//			m_validBlobClass[frameIdx] = 1;
//			for( int i = 0; i < validRedBlobs.size(); i++ ) {
//				m_validBlobs[frameIdx].push_back( validRedBlobs.at(i) );				
//			}
//		} else if( greenArea > redArea && greenArea > yellowArea ) {
//			// Case :: GREEN			
//			m_validBlobClass[frameIdx] = 3;
//			for( int i = 0; i < validGreenBlobs.size(); i++ ) {
//				m_validBlobs[frameIdx].push_back( validGreenBlobs.at(i) );				
//			}
//		} else if( yellowArea > redArea && yellowArea > greenArea ) {
//			// Case :: YELLOW			
//			m_validBlobClass[frameIdx] = 2;
//			for( int i = 0; i < validYellowBlobs.size(); i++ ) {
//				m_validBlobs[frameIdx].push_back( validYellowBlobs.at(i) );				
//			}
//		} 
//
//		cpuTimer.stop();
//		m_procMs[ frameIdx ] = cpuTimer.elapsed_time_ms;
//		printf("\r[%5d/%5d] %3d fps , %2.2fms", 
//			(frameIdx+1), etx,
//			(int)(1000.0 / cpuTimer.elapsed_time_ms), 
//			cpuTimer.elapsed_time_ms);
//		totalProcessingTime += cpuTimer.elapsed_time_ms;
//		fflush(stdout);
//		cvReleaseImage( &croppedImg );
//#ifdef _SAVE_RESULT
//		if( frameIdx == 0)
//			saveResults( 0 );
//		else if( frameIdx > 1 )
//			saveResults( frameIdx - 1 );
//#endif
//	}
//
//#ifdef _SAVE_RESULT
//	saveResults( etx - 2 );			// 인덱스를 제거. 전후는 있는데 가운데가 없으면 가운데 메꿔줌. 약식트래킹 세장봤을때 가운데 하나만 없으면 채워줌.--> 보수적으로 땜빵
//	saveResults( etx - 1 );
//
//	fclose( resFp );
//#endif
//	
//	freeSegmentationModule();			// gpu 메모리 해제
//	cvReleaseImage( &src );
//	cvReleaseImage( &res );
//	cvReleaseImage( &redMap );
//	cvReleaseImage( &greenMap );
//	cvReleaseImage( &yellowMap );
//	cvReleaseImage( &ryMap );
//
//	printf("\n\nAvg Processing time : %f\n", totalProcessingTime / (double)etx );
//	return 0;
//}
//
//void saveResults( int fIdx )
//{
//	char buf[500];
//	sprintf_s( buf, "E:/Lara_TLR_BenchmardkDB/Lara3D_UrbanSeq1_JPG/frame_%d.jpg", fIdx);
//	IplImage *finalImg = cvLoadImage( buf );
//	CvScalar color;
//	int isTracked = 0;
//	// Step :: 전후 관계를 이용하여 비어있는 프레임의 결과를 보정
//	if( fIdx >= 1 ) {						
//		vector<CvRect> tracked = getTrackedResult( fIdx, isTracked );
//		res_class[fIdx]=m_validBlobClass[fIdx];
//		// Step :: 중앙 신호등 결과 출력
//		drawTrafficLightResult(finalImg, m_validBlobClass[fIdx], isTracked);
//		if( m_validBlobClass[ fIdx ] == 1 ) color = CV_RGB(255,0,0);
//		else if( m_validBlobClass[ fIdx ] == 2 ) color = CV_RGB(255,255,0);
//		else if( m_validBlobClass[ fIdx ] == 3 ) color = CV_RGB(0,255,0);
//
//		for( int i = 0; i < tracked.size(); i++ ) {
//			//drawBulbCircle( finalImg, tracked.at(i), scaleFactor, color );
//			drawBulbRect( finalImg, tracked.at(i), scaleFactor, m_validBlobClass[ fIdx ] );
//#ifdef _SAVE_RESULT			
//			CvRect tlRect = getTrafficLightRect(tracked.at(i), 1);			
//			if( m_validBlobClass[ fIdx ] == 1 )
//				fprintf_s( resFp,"%d,%d,%d,%d,%d,1\n", fIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor) );
//			else if( m_validBlobClass[ fIdx ] == 2 )
//				fprintf_s( resFp,"%d,%d,%d,%d,%d,2\n", fIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor) );
//			else if( m_validBlobClass[ fIdx ] == 3 )
//				fprintf_s( resFp,"%d,%d,%d,%d,%d,3\n", fIdx, (int)((double)tlRect.x/scaleFactor), (int)((double)tlRect.y/scaleFactor), (int)((double)tlRect.width/scaleFactor), (int)((double)tlRect.height/scaleFactor) );
//			fflush( resFp );
//#endif
//		}
//	} else {
//		// Step :: 중앙 신호등 결과 출력
//		drawTrafficLightResult(finalImg, 0, isTracked);
//	}
//	// Display fps
//	if( isTracked == 1 ) {
//		sprintf_s( m_errTxt, "[%5d/%5d] %3d fps , %2.2fms  [TRACKED]", 
//			(fIdx+1), etx, (int)(1000.0 / m_procMs[ fIdx ]), m_procMs[ fIdx ]);
//	} else {
//		sprintf_s( m_errTxt, "[%5d/%5d] %3d fps , %2.2fms", 
//			(fIdx+1), etx, (int)(1000.0 / m_procMs[ fIdx ]), m_procMs[ fIdx ]);
//	}
//	cvPutText( finalImg, m_errTxt, m_txtPoint, &m_Font, CV_RGB(0,255,0) );
//
//	cvShowImage("RESULT", finalImg);
//	cvWaitKey(0);
//#ifdef _SAVE_RESULT_IMAGE
//	sprintf_s( buf, "E:/ExperimentResult/LaraResult/frame_%d.jpg", fIdx);
//	cvSaveImage(buf, finalImg);
//#endif	
//	cvReleaseImage(&finalImg);
//}
//
//vector<CvRect> getMachedRoiIdx( vector<CvRect> &prevRects, vector<CvRect> &nextRects )
//{
//	vector<CvRect> tracked;
//	for( int i = 0; i < prevRects.size(); i++ ) {
//		CvRect rp = prevRects.at(i);
//		for( int j = 0; j < nextRects.size(); j++ ) {
//			CvRect np = nextRects.at(j);
//			if( abs( rp.x - np.x ) < rp.width &&
//				abs( rp.y - np.y ) < rp.height ) {
//				// Case :: Matched!
//				CvRect rtObj;
//				rtObj.x = ( rp.x + np.x ) / 2;
//				rtObj.y = ( rp.y + np.y ) / 2;
//				rtObj.width = ( rp.width + np.width ) / 2;
//				rtObj.height = ( rp.height + np.height ) / 2;
//				tracked.push_back( rtObj );
//				//printf("\t X:%d, Y:%d, WIDTH:%d, HEIGHT:%d\n", rtObj.x, rtObj.y, rtObj.width, rtObj.height );
//
//				break;
//			}
//		}
//	}
//	return tracked;
//}
//
//vector<CvRect> getTrackedResult( int frameIdx, int &isTracked )
//{	
//	static int trackedQty = 0;
//	vector<CvRect> tracked;
//	
//	if( m_validBlobClass[ frameIdx-1 ] == m_validBlobClass[ frameIdx + 1 ] &&  
//		m_validBlobClass[ frameIdx-1 ] != 0 && 
//		m_validBlobClass[ frameIdx ] == 0 &&		
//		m_validBlobClass[ frameIdx+1 ] != 0 )			
//	{
//		// Case :: Track이 필요한 경우
//		isTracked = 1;
//		m_validBlobClass[ frameIdx ] = m_validBlobClass[ frameIdx+1 ];
//		tracked = getMachedRoiIdx(  m_validBlobs[ frameIdx-1 ],  m_validBlobs[ frameIdx+1 ] );		
//		printf("\n\t%d frame is tracked (Total: %3d)\n", frameIdx, trackedQty++ );
//		fflush(stdout);
//	} else {
//		// Case :: Track이 필요하지 않은 경우
//		isTracked = 0;
//		for(int i = 0; i < m_validBlobs[frameIdx].size(); i++ )
//			tracked.push_back( m_validBlobs[frameIdx].at(i) );
//	}
//
//	return tracked;
//}
//
//void drawTrafficLightResult( IplImage *img, int flag, int isTracked )
//{
//	const int cLength = 25;
//	int boxWidth = cLength * 2;
//	int boxHeight = cLength * 6;
//	/*
//	CvPoint pBox1( img->width / 2 - (boxWidth/2), img->height/2 );
//	CvPoint pBox2( img->width / 2 + (boxWidth/2), img->height/2 + boxHeight );
//	*/
//
//	CvPoint pBox1( img->width / 2 - (boxWidth/2), img->height - boxHeight );
//	CvPoint pBox2( img->width / 2 + (boxWidth/2), img->height );
//
//	cvDrawRect( img, pBox1, pBox2, CV_RGB(20,20,20), CV_FILLED );
//	if( isTracked )
//		cvDrawRect( img, pBox1, pBox2, CV_RGB(100,100,100), 2 );
//	else 
//		cvDrawRect( img, pBox1, pBox2, CV_RGB(100,100,100), 1 );
//
//	/*
//	CvPoint pRed( pBox1.x + cLength*1, pBox1.y + cLength);
//	CvPoint pYellow( pRed.x + cLength*2, pRed.y );
//	CvPoint pGreen( pYellow.x + cLength*2, pYellow.y );
//	*/
//
//	CvPoint pRed( pBox1.x + cLength*1, pBox1.y + cLength);
//	CvPoint pYellow( pRed.x, pRed.y + cLength*2 );
//	CvPoint pGreen( pRed.x, pYellow.y +  cLength*2);
//
//	cvDrawCircle( img, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
//	cvDrawCircle( img, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
//	cvDrawCircle( img, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
//
//	if( flag == 0 ) {
//		cvDrawCircle( img, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
//		cvDrawCircle( img, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
//		cvDrawCircle( img, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
//	} else if( flag == 1 )  {		
//		cvDrawCircle( img, pRed, cLength*0.9, CV_RGB(255,0,0), CV_FILLED);
//		cvDrawCircle( img, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
//		cvDrawCircle( img, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
//	} else if( flag == 2 )  {
//		cvDrawCircle( img, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
//		cvDrawCircle( img, pYellow, cLength*0.9, CV_RGB(255,255,0), CV_FILLED);
//		cvDrawCircle( img, pGreen, cLength*0.9, CV_RGB(0,255,0), 1);
//	} else if( flag == 3 )  {
//		cvDrawCircle( img, pRed, cLength*0.9, CV_RGB(255,0,0), 1);
//		cvDrawCircle( img, pYellow, cLength*0.9, CV_RGB(255,255,0), 1);
//		cvDrawCircle( img, pGreen, cLength*0.9, CV_RGB(0,255,0), CV_FILLED);
//	}
//}
//
//void drawBulbCircle( IplImage *img, CvRect rt, float scaleFactor, CvScalar color )
//{
//	CvPoint p1, p2;
//	p1.x = rt.x / scaleFactor;;
//	p1.y = rt.y / scaleFactor;;
//	p2.x = p1.x + rt.width / scaleFactor;;
//	p2.y = p1.y + rt.height / scaleFactor;;
//	CvPoint pCircle;
//	pCircle.x = p1.x + ( p2.x - p1.x ) / 2;
//	pCircle.y = p1.y + ( p2.y - p1.y ) / 2;
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x )/2, CV_RGB(0,0,0), 3);
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x )/2, color, 1);
//
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x ), CV_RGB(0,0,0), 3);
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x ), color, 2);
//
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x )*2, CV_RGB(0,0,0), 4);
//	cvDrawCircle( img, pCircle, ( p2.x - p1.x )*2, color, 1);
//
//}
//
//void drawBulbRect( IplImage *img, CvRect rt, float scaleFactor, int signalType )
//{
//	CvPoint p1, p2;
//	p1.x = rt.x / scaleFactor - rt.width/2;
//	p2.x = p1.x + rt.width / scaleFactor + rt.width;
//
//	if( signalType == 1 ) {
//		// Case :: Red
//		p1.y = rt.y / scaleFactor - rt.height / scaleFactor;	
//		p2.y = p1.y + rt.height / scaleFactor + rt.height / scaleFactor * 4;
//		cvDrawRect( img, p1, p2, CV_RGB(255,0,0), 2);
//	} else if( signalType == 2 ) {
//		// Case :: Yellow
//		p1.y = rt.y / scaleFactor - rt.height / scaleFactor * 2;	
//		p2.y = p1.y + rt.height / scaleFactor + rt.height / scaleFactor * 4;
//		cvDrawRect( img, p1, p2, CV_RGB(255,255,0), 2);
//	} else if( signalType == 3 ) {
//		// Case :: Green
//		p1.y = rt.y / scaleFactor - rt.height / scaleFactor * 3;	
//		p2.y = p1.y + rt.height / scaleFactor + rt.height / scaleFactor * 4;
//		cvDrawRect( img, p1, p2, CV_RGB(0,255,0), 2);
//	}
//	
//
//}
//
//CvRect getTrafficLightRect( CvRect rt, int mode )
//{
//	int x, y, width, height;
//	const double marginRate = 0.2;
//	if( mode == 1 ) {
//		// Case :: Red
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	} else if( mode == 2 ) {
//		// Case :: Green
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * 2 - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	}
//	return CvRect(x,y,width,height);
//}
//
//IplImage* getBulbRoiImage( IplImage *src, CvRect rt, int mode )
//{
//	int x, y, width, height;
//	const double marginRate = 0.2;
//	if( mode == 1 ) {
//		// Case :: Red
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	} else if( mode == 2 ) {
//		// Case :: Green
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * 2 - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	}
//	IplImage *roi = iUtil.getRoiFrame( src, y, x, y+height, x+width, 3 );
//	return roi;
//}
//
//void saveBulbRoiImage( IplImage *src, CvRect rt, int frameSeq, int mode )
//{
//	static int roiIdx = 0;
//	int x, y, width, height;
//	if( mode == 1 || mode == 3 || mode == 5 ) {
//		// Case :: Red
//		x = rt.x - rt.width * 0.2; 
//		y = rt.y - rt.height * 0.2; 
//		width = rt.width + rt.width * 0.4; 
//		height = rt.height * 3 + rt.height * 0.4;
//	} else if( mode == 2 || mode == 4 || mode == 6 ) {
//		// Case :: Green
//		x = rt.x - rt.width * 0.2; 
//		y = rt.y - rt.height * 2 - rt.height * 0.2; 
//		width = rt.width + rt.width * 0.4; 
//		height = rt.height * 3 + rt.height * 0.4;
//	}
//	IplImage *roi = iUtil.getRoiFrame( src, y, x, y+height, x+width, 3 );
//	char buf[100];
//	if( mode == 1 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/red/%d_%d.jpg", frameSeq, roiIdx++);
//	if( mode == 3 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/%d_%d.jpg", frameSeq, roiIdx++);
//	else if( mode == 2 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/green/%d_%d.jpg", frameSeq, roiIdx++);
//	else if( mode == 4 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/green-am/%d_%d.jpg", frameSeq, roiIdx++);
//	else if( mode == 5 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/yellow/%d_%d.jpg", frameSeq, roiIdx++);
//	else if( mode == 6 )
//		sprintf_s(buf,"E:/ExperimentResult/LaraResult/etc/%d_%d.jpg", frameSeq, roiIdx++);
//
//	cvSaveImage(buf,roi);
//	cvReleaseImage(&roi);
//}
//
//bool checkBulbValidation( IplImage *src, CvRect rt, int mode )
//{
//	int x, y, width, height;
//	double marginRate = 0.2;
//	if( mode == 1 ) {
//		// Case :: Red
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	} else if( mode == 2 ) {
//		// Case :: Green
//		x = rt.x - rt.width * marginRate; 
//		y = rt.y - rt.height * 2 - rt.height * marginRate; 
//		width = rt.width + rt.width * marginRate*2; 
//		height = rt.height * 3 + rt.height * marginRate*2;
//	}
//	if( y < 0 ) y = 0;
//	if( y + height > src->height ) height -= y + height - src->height;	
//	const int avgVal = iUtil.getAvgRGB( src, x, y, width, height );
//	//printf("Mode=%d, (%d,%d) = %d\n", mode, rt.x, rt.y, avgVal );
//	if( mode == 1 && avgVal <= 100 ) return true;
//	if( mode == 2 && avgVal <= 100 ) return true;
//	return false;
//}
//
//bool checkAspectRatio( CvRect rt )
//{
//	if( rt.width < 7 || rt.height < 7 ) return false;
//	if( rt.width > rt.height ) {
//		if( abs( (float)rt.height / (float)rt.width ) <= 0.6 ) return false;
//	} else {
//		if( abs( (float)rt.width / (float)rt.height ) <= 0.6 ) return false;
//	}
//	return true;
//}
//
//vector<CvRect> blobDetection( IplImage *ryMap )
//{
//	blobDetector.SetParam( ryMap, 1 );
//	blobDetector.DoLabeling();
//	vector<CvRect> blobs;
//	for( int i = 0; i < blobDetector.m_nBlobs; i++ ) {
//		blobs.push_back( blobDetector.m_recBlobs[i] );		
//	}
//
//	return blobs;
//}
//
//void colorSegmentation( IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap )
//{
//	
//	float baseColor[3][3] = { 
//		{  53.4,  54.3, 160.4 },    // Red light
//		//{ 230.0, 245.0, 80.0 },	  // Green light
//		{ 230.0, 245.0, 80.0 },		// Green light 2
//		{  33.7, 196.2, 235.6 } };  // Yellow light
//		
//	float *rgyRange = (float*)malloc( sizeof(float) * 9 );	
//	memcpy(rgyRange, baseColor, sizeof(float)*9);
//
//	gpu_segmentation( src->imageData, redMap->imageData, greenMap->imageData, yellowMap->imageData, ryMap->imageData, rgyRange, src->width, src->height );
//
//	cvShowImage("GPU RYMap", ryMap);
//	cvShowImage("GPU RedMap", redMap);
//	cvShowImage("GPU YellowMap", yellowMap);
//	cvShowImage("GPU GreenMap", greenMap);
//
//	gpu_dilate( ryMap->imageData, ryMap->imageData, ryMap->width, ryMap->height, 3);
//	gpu_dilate(greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 9);
//	//gpu_dilate( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 5);
//	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
//	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
//
//	cvShowImage( "GPU RYMap_dilated", ryMap);	
//	cvShowImage( "GPU GreenMap_dilated", greenMap);	
//
//	free( rgyRange );
//}
//
//
//void procIntArray( string &line, int* array, int nQty ) {
//	//store inputs		
//	char* cstr = new char[line.size()+1];
//	char* t;
//	strcpy(cstr, line.c_str());
//
//	//tokenise
//	int i = 0;
//	t = strtok (cstr,",");
//
//	while( t!=NULL && i < nQty )
//	{	
//		array[i] = atoi(t);			
//
//		//move token onwards
//		t = strtok(NULL,",");
//		i++;			
//	}
//}
//
//void loadResult()
//{
//	char fName[200];
//	fstream inputFile;
//	sprintf_s(fName,"E:/ExperimentResult/LARA_RESULT_TRACKED.csv");
//	//sprintf_s(fName,"D:/LARA_RESULT_D90.csv");
//	inputFile.open( fName, ios::in );
//	int data[6];
//	if( inputFile.is_open() ) {
//		string line = "";
//		while( !inputFile.eof() ) {
//			getline( inputFile, line );
//			procIntArray( line, data, 6 );
//			res_class[ data[0] ] = data[5];
//			res_width[ data[0] ] = data[3];
//
//		}
//	}
//	inputFile.close();
//}
//
//
//void loadGT()		// 실제 dataset의 Ground truth 읽기
//{
//	char fName[200];
//	fstream inputFile;
//	sprintf_s(fName,"E:/ExperimentResult/Lara_GT_revised.csv");		// GT 파일
//	inputFile.open( fName, ios::in );
//	int data[6];
//	if( inputFile.is_open() ) {
//		string line = "";
//		while( !inputFile.eof() ) {
//			getline( inputFile, line );
//			procIntArray( line, data, 6 );			
//			gt_class[ data[0] ] = data[5];
//			gt_width[ data[0] ] = data[3];
//		}
//	}
//	inputFile.close();
//}
//
//void performanceTest( int mode )
//{
//	// 모든 값 -1로 세팅
//	for( int i = 0; i < 12000; i++ ) {
//		gt_class[i] = -1;
//		res_class[i] = -1;
//		gt_width[i] = -1;
//		res_width[i] = -1;
//	}
//	
//	int minWidth = 6;
//	loadGT();			// GT받아옴. getline, procIntArray, gt_class, gt_width
//
//	if( mode == 2 ) {
//		loadResult();	// Result를 받아옴. getline, procIntArray, res_class, res_width
//		printf("Minimum GT Width (Default 6): ");
//		scanf("%d", &minWidth);
//	} 
//
//	int TP = 0;
//	int FP = 0;
//	int FN = 0;
//	int GT_QTY = 0;
//	// TP
//	for( int i = 0; i < 12000; i++ ) {
//		if( gt_class[i] != -1 ) {
//			// Case :: GT 있음	
//			if( gt_width[i] < minWidth ) continue;
//			GT_QTY++;
//			if( res_class[i] != -1 ) {
//				if( res_class[i] == gt_class[i] ) {
//					// Case :: TP
//					TP++;
//				} else {
//					FP++;
//				}
//			} else {
//				FN++;
//			}
//		} 
//	}
//	// 추가 FP
//	for( int i = 0; i < 12000; i++ ) {
//		if( res_class[i] != -1 ) {
//			// Case :: 결과 있음			
//			if( gt_class[i] == -1 ) {
//				if( gt_width[i] != -1 && gt_width[i] < minWidth ) continue;
//				FP++;
//			}
//		} 
//	}
//
//	double recall = (double)TP / (double)( TP + FN );
//	double precision = (double)TP / (double)( TP + FP );
//	double f1Score = 2 * ( ( precision * recall ) / ( precision + recall ) );
//	printf("GT QTY = %d\n", GT_QTY);
//	printf("TP = %d\n", TP);
//	printf("FP = %d\n", FP);
//	printf("FN = %d\n", FN);
//	printf("Precision = %f\n", precision );
//	printf("Recall = %f\n", recall );
//	printf("F1 = %f\n", f1Score );
//}