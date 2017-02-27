// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <iostream>
#include <string>
#include <io.h>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

__declspec(dllimport) void _initModule(int mode, int sizeMode, int srcWidth, int srcHeight, float sigma1 = 0.0f, float sigma2 = 0.0f, float sigma3 = 0.0f, int roi_x1 = -1, int roi_y1 = -1, int roi_x2 = -1, int roi_y2 = -1);
__declspec(dllimport) void _finalizeModule();
__declspec(dllimport) void _enhanceImage( IplImage *src, IplImage *dst, float nFactor = 1.4f, int intensityOffset = 0 );
__declspec(dllimport) void _bilateralFilter(IplImage *src, IplImage *dst, const float euclidean_delta = 3.0f, const int filter_radius = 3);