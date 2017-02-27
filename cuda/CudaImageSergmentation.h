#pragma once
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <math.h>



void initSegmentationModule(int width, int height);
void freeSegmentationModule();
void gpu_segmentation( char *host_src3c, char *host_redMap, char *host_greenMap, char *host_yellowMap, char *host_ryMap, float* rgyRange, int width, int height );
void gpu_hsvSegmentation( char *host_src3c, char *host_redMap, char *host_greenMap, char *host_yellowMap, char *host_ryMap, float* rgyRange, int width, int height );
