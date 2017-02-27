#pragma once
#include "stdafx.h"

using namespace std;

/*
__declspec(dllimport) void _initTestNetwork(int moduleIdx, char* protoPath, char* caffeModelPath, int gpuMode = 1, bool reboot = false);
__declspec(dllimport) void _initNetwork(int netQty, bool reboot = false);
__declspec(dllimport) void _testImage(int moduleIdx, IplImage *graySrc, const int nClass, int width, int height, int &classNo, double &prob, bool verbose);
*/

void initCnn()
{
	_initNetwork(1);			// cnn이 3개면 (같은걸
	//_initTestNetwork(0,"../Bin64/cnn_lara_model/kylimnet_default_test.prototxt","../Bin64/cnn_lara_model/kylimnet_default_iter_2500.caffemodel");
	_initTestNetwork(
		0,			// 0번 네트워크
		"../Bin64/cnn_lara_model/kylimnet_default_test.prototxt",				// 네트웍 구조
		"../Bin64/cnn_lara_model/kylimnet_default_iter_2100.caffemodel");		// 학습된 웨이트
}

int classify(IplImage*roiImg)
{
	IplImage *colorRoi = cvCreateImage( cvSize(40, 40), 8, 3 );		// 이미지 크기를 바꿈
	IplImage *grayRoi = cvCreateImage( cvSize(40, 40), 8, 1 );		// 이미지 채널을 바꿈
	cvResize( roiImg, colorRoi );
	cvCvtColor( colorRoi, grayRoi, CV_BGR2GRAY );
	int classNo = -1;
	double prob = 0.0;

	_testImage(0, grayRoi, 5, 40, 40, classNo, prob, false );		// CNN 테스트 classNo: 번호 , 
	// 모듈 수 , roi, 클래스개수 , x크기, y크기, 클래스번호, 

	cvReleaseImage(&colorRoi);		// 해제
	cvReleaseImage(&grayRoi);		// 해제
		
	if( prob > 0.7 ) return classNo;	
	else return -1;
}