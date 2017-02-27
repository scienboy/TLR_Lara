#include "stdafx.h"
#include "cuda/CudaImageMorphology.h"
#include "cuda/CudaImageSergmentation.h"
#include "colorSegmentor.h"

void colorSegmentation(IplImage *src, IplImage *redMap, IplImage *greenMap, IplImage *yellowMap, IplImage *ryMap)
{

	float baseColor[3][3] = {
		//{ 53.4, 54.3, 160.4 },    // Red light
		////{ 230.0, 245.0, 80.0 },	  // Green light
		//{ 230.0, 245.0, 80.0 },		// Green light 2
		//{ 33.7, 196.2, 235.6 }  // Yellow light

		{ 30, 50, 230 },				// Red light
		{ 255, 255, 0 },				// Green light
		{ 0, 255, 255 }				// Yellow light
	};
	float *rgyRange = (float*)malloc(sizeof(float)* 9);
	memcpy(rgyRange, baseColor, sizeof(float)* 9);

	gpu_segmentation(src->imageData, redMap->imageData, greenMap->imageData, yellowMap->imageData, ryMap->imageData, rgyRange, src->width, src->height);

	//cvShowImage("GPU RYMap", ryMap);
	//cvShowImage("GPU RedMap", redMap);
	//cvShowImage("GPU YellowMap", yellowMap);
	//cvShowImage("GPU GreenMap", greenMap);

	gpu_erode(redMap->imageData, redMap->imageData, redMap->width, redMap->height, 3);
	gpu_dilate(ryMap->imageData, ryMap->imageData, ryMap->width, ryMap->height, 3);
	gpu_dilate(redMap->imageData, redMap->imageData, redMap->width, redMap->height, 3);
	gpu_dilate(redMap->imageData, redMap->imageData, redMap->width, redMap->height, 3);
	gpu_dilate(greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
	//gpu_dilate( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 5);
	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);
	//gpu_erode( greenMap->imageData, greenMap->imageData, greenMap->width, greenMap->height, 3);

	//cvShowImage("GPU RYMap_dilated", ryMap);
	//cvShowImage("GPU redMap_dilated", redMap);
	//cvShowImage("GPU GreenMap_dilated", greenMap);
	cvWaitKey(10);

	free(rgyRange);
}