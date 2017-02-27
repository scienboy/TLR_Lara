#include "CudaImageSergmentation.h"

cudaDeviceProp prop;
int nMaxThreadsPerBlock = 1024;
char *dev_src3c, *dev_redMap, *dev_greenMap, *dev_yellowMap, *dev_ryMap;

__global__ void _segmentation(char *bgrIn, char *dev_redMap, char *dev_greenMap, char *dev_yellowMap,char *dev_ryMap, float *rgyRange, int width, int height)
{
	const unsigned int offset = blockIdx.x * blockDim.x + threadIdx.x;	
	const int x = offset % width;
	const int y = (offset-x) / width;
		
	const int idx3c = y * ( width * 3 ) + x * 3;
	float bb = (unsigned char)bgrIn[ idx3c ];
	float gg = (unsigned char)bgrIn[ idx3c + 1];
	float rr = (unsigned char)bgrIn[ idx3c + 2];

	for( int i = 0; i < 3; i++ ) {		
		float diff = abs( rgyRange[(3*i)+0] - bb ) + abs( rgyRange[(3*i)+1] - gg ) + abs( rgyRange[(3*i)+2] - rr );		
		if( i == 0 ) {		// Red와 비교
			if( diff <= 80) { // 80
				dev_redMap[ offset ] = (unsigned char)255;
				dev_ryMap[ offset ] = (unsigned char)255;
			} else {
				dev_redMap[ offset ] = (unsigned char)0;
				dev_ryMap[ offset ] = (unsigned char)0;
			}
		} else if( i == 1 ) { // Cyan과 비교, 90
			if( diff <= 180.0f ) dev_greenMap[ offset ] = (unsigned char)255;
			else dev_greenMap[ offset ] = (unsigned char)0;
		
		} else if( i == 2 ) {
			if( diff <= 85 ) { // Yellow와 비교, 80
				dev_yellowMap[ offset ] = (unsigned char)255;				
				dev_ryMap[ offset ] = (unsigned char)255;
			} else {
				dev_yellowMap[ offset ] = (unsigned char)0;
				//dev_ryMap[ offset ] = (unsigned char)0;
			}
		}
	}
	
}

__global__ void _hsvSegmentation(char *bgrIn, char *dev_redMap, char *dev_greenMap, char *dev_yellowMap,char *dev_ryMap, float *rgyRange, int width, int height)
{
	const unsigned int offset = blockIdx.x * blockDim.x + threadIdx.x;	
	const int x = offset % width;
	const int y = (offset-x) / width;
		
	const int idx3c = y * ( width * 3 ) + x * 3;
	float hue = (unsigned char)bgrIn[ idx3c ];
	float sat = (unsigned char)bgrIn[ idx3c + 1];
	float bright = (unsigned char)bgrIn[ idx3c + 2];

	if( bright < 200 ) {
		dev_redMap[ offset ] = (unsigned char)0;
		dev_ryMap[ offset ] = (unsigned char)0;
		dev_greenMap[ offset ] = (unsigned char)0;
		return;
	} else if( bright >= 250 ) {
		dev_redMap[ offset ] = (unsigned char)0;
		dev_ryMap[ offset ] = (unsigned char)0;
		dev_greenMap[ offset ] = (unsigned char)0;
		return;
	}

	for( int i = 0; i < 4; i++ ) {		
		
		float diff = abs( rgyRange[(3*i)+0] - hue ) + abs( rgyRange[(3*i)+1] - sat );
		float hueDiff = abs( rgyRange[(3*i)+0] - hue );
		float satDiff = abs( rgyRange[(3*i)+1] - sat );
		if( i == 0 ) {									
			if( hueDiff <= 40 ) {
				dev_redMap[ offset ] = (unsigned char)255;
				dev_ryMap[ offset ] = (unsigned char)255;
			} else {
				dev_redMap[ offset ] = (unsigned char)0;
				dev_ryMap[ offset ] = (unsigned char)0;
			}
		} else if( i == 2 ) {
			if( diff <= 40 ) dev_greenMap[ offset ] = (unsigned char)255;
			else dev_greenMap[ offset ] = (unsigned char)0;
		
		} else if( i == 3 ) {
			if( hueDiff <= 20 ) {
				dev_yellowMap[ offset ] = (unsigned char)255;				
				dev_ryMap[ offset ] = (unsigned char)255;
			} else {
				dev_yellowMap[ offset ] = (unsigned char)0;
				dev_ryMap[ offset ] = (unsigned char)0;
			}
		}
	}
	
}

void initSegmentationModule(int width, int height)
{	
	cudaDeviceProp prop;
	int nMaxThreadsPerBlock;

	int count = 1;
	cudaGetDeviceCount( &count );
	for( int i = 0; i < count; i++  ) {
		cudaGetDeviceProperties( &prop, i );
		nMaxThreadsPerBlock = prop.maxThreadsPerBlock;		
	}

	cudaMalloc( (void**)&dev_src3c, sizeof(char)*width*height*3 );
	cudaMalloc( (void**)&dev_redMap, sizeof(char)*width*height );
	cudaMalloc( (void**)&dev_greenMap, sizeof(char)*width*height );
	cudaMalloc( (void**)&dev_yellowMap, sizeof(char)*width*height );
	cudaMalloc( (void**)&dev_ryMap, sizeof(char)*width*height );
}

void freeSegmentationModule()
{	
	cudaFree( dev_src3c );
	cudaFree( dev_redMap );
	cudaFree( dev_greenMap );
	cudaFree( dev_yellowMap );
}

void gpu_segmentation( char *host_src3c, char *host_redMap, char *host_greenMap, char *host_yellowMap, char *host_ryMap, float* rgyRange, int width, int height )
{	
	const int nBlocks = ( width * height + nMaxThreadsPerBlock - 1 )  / nMaxThreadsPerBlock;	
	
	float *dev_rgyRange;
	cudaMalloc( (void**)&dev_rgyRange, sizeof(float)*9 );
	cudaMemcpy( dev_src3c, host_src3c, sizeof(char)*width*height*3, cudaMemcpyHostToDevice );
	cudaMemcpy( dev_rgyRange, rgyRange, sizeof(float)*9, cudaMemcpyHostToDevice );

	_segmentation<<< nBlocks, nMaxThreadsPerBlock >>>( dev_src3c, dev_redMap, dev_greenMap, dev_yellowMap, dev_ryMap, dev_rgyRange, width, height );

	cudaMemcpy( host_redMap, dev_redMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_greenMap, dev_greenMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_yellowMap, dev_yellowMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_ryMap, dev_ryMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	
	cudaFree( dev_rgyRange );

}

void gpu_hsvSegmentation( char *host_src3c, char *host_redMap, char *host_greenMap, char *host_yellowMap, char *host_ryMap, float* rgyRange, int width, int height )
{	
	const int nBlocks = ( width * height + nMaxThreadsPerBlock - 1 )  / nMaxThreadsPerBlock;	
	
	float *dev_rgyRange;
	cudaMalloc( (void**)&dev_rgyRange, sizeof(float)*12 );
	cudaMemcpy( dev_src3c, host_src3c, sizeof(char)*width*height*3, cudaMemcpyHostToDevice );
	cudaMemcpy( dev_rgyRange, rgyRange, sizeof(float)*9, cudaMemcpyHostToDevice );

	_hsvSegmentation<<< nBlocks, nMaxThreadsPerBlock >>>( dev_src3c, dev_redMap, dev_greenMap, dev_yellowMap, dev_ryMap, dev_rgyRange, width, height );

	cudaMemcpy( host_redMap, dev_redMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_greenMap, dev_greenMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_yellowMap, dev_yellowMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	cudaMemcpy( host_ryMap, dev_ryMap, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	
	cudaFree( dev_rgyRange );

}
