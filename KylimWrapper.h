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
	_initNetwork(1);			// cnn�� 3���� (������
	//_initTestNetwork(0,"../Bin64/cnn_lara_model/kylimnet_default_test.prototxt","../Bin64/cnn_lara_model/kylimnet_default_iter_2500.caffemodel");
	_initTestNetwork(
		0,			// 0�� ��Ʈ��ũ
		"../Bin64/cnn_lara_model/kylimnet_default_test.prototxt",				// ��Ʈ�� ����
		"../Bin64/cnn_lara_model/kylimnet_default_iter_2100.caffemodel");		// �н��� ����Ʈ
}

int classify(IplImage*roiImg)
{
	IplImage *colorRoi = cvCreateImage( cvSize(40, 40), 8, 3 );		// �̹��� ũ�⸦ �ٲ�
	IplImage *grayRoi = cvCreateImage( cvSize(40, 40), 8, 1 );		// �̹��� ä���� �ٲ�
	cvResize( roiImg, colorRoi );
	cvCvtColor( colorRoi, grayRoi, CV_BGR2GRAY );
	int classNo = -1;
	double prob = 0.0;

	_testImage(0, grayRoi, 5, 40, 40, classNo, prob, false );		// CNN �׽�Ʈ classNo: ��ȣ , 
	// ��� �� , roi, Ŭ�������� , xũ��, yũ��, Ŭ������ȣ, 

	cvReleaseImage(&colorRoi);		// ����
	cvReleaseImage(&grayRoi);		// ����
		
	if( prob > 0.7 ) return classNo;	
	else return -1;
}