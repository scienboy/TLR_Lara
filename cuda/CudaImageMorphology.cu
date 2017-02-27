#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>

__global__ void _erode(char *grayIn, char *grayOut, int width, int height, int kernelSize, int *mask)
{
	const unsigned int offset = blockIdx.x * blockDim.x + threadIdx.x;	
	const int x = offset % width;
	const int y = (offset-x) / width;

	if( x - kernelSize / 2 < 0 || y - kernelSize / 2 < 0 || x + kernelSize / 2 > width || y + kernelSize / 2 > height ) return;

	int intensitySum = 0;	
	int subImageSize = kernelSize * kernelSize;
	
	for( int i = 0; i < subImageSize; i++ ) {		
		if( (unsigned char)grayIn[ offset + mask[i] ] > 100 ) intensitySum++;
	}
	
	if( intensitySum == subImageSize )
		grayOut[ offset ] = (unsigned char)255;
	else
		grayOut[ offset ] = (unsigned char)0;

}

__global__ void _dilate(char *grayIn, char *grayOut, int width, int height, int kernelSize, int *mask)
{
	const unsigned int offset = blockIdx.x * blockDim.x + threadIdx.x;	
	const int x = offset % width;
	const int y = (offset-x) / width;

	if( x - kernelSize / 2 < 0 || y - kernelSize / 2 < 0 || x + kernelSize / 2 > width || y + kernelSize / 2 > height ) return;

	int intensitySum = 0;	
	int subImageSize = kernelSize * kernelSize;
	
	for( int i = 0; i < subImageSize; i++ ) {		
		if( (unsigned char)grayIn[ offset + mask[i] ] > 100 ) intensitySum++;
	}
	
	if( intensitySum >= 1 )
		grayOut[ offset ] = (unsigned char)255;
	else
		grayOut[ offset ] = (unsigned char)0;
	
}

int* _getRangeMask(int kernelSize, int width)
{	
	int *mask = (int*)malloc(sizeof(int)*kernelSize*kernelSize);
	int subImageSize = kernelSize*kernelSize;
	int centerPixel = subImageSize / 2;
	int centerPixelCol = centerPixel / ( subImageSize / kernelSize );
	int centerPixelRow = centerPixel % ( subImageSize / kernelSize );	
	for( int i = 0; i < subImageSize; i++ ) {
		int rowIdx = i / ( subImageSize / kernelSize );
		int colIdx = i % ( subImageSize / kernelSize );		
		mask[i] = ( rowIdx - centerPixelRow ) * width + ( colIdx - centerPixelCol );
	}

	return mask;
}

void gpu_erode( char *host_src, char *host_dst, int width, int height, int kernelSize = 3 )
{
	cudaDeviceProp prop;
	int nMaxThreadsPerBlock;
	int count = 1;
	cudaGetDeviceCount( &count );
	for( int i = 0; i < count; i++  ) {
		cudaGetDeviceProperties( &prop, i );
		nMaxThreadsPerBlock = prop.maxThreadsPerBlock;		
	}
	const int nBlocks = ( width * height + nMaxThreadsPerBlock - 1 )  / nMaxThreadsPerBlock;	
	char *dev_src, *dev_dst;
	cudaMalloc( (void**)&dev_src, sizeof(char)*width*height );
	cudaMalloc( (void**)&dev_dst, sizeof(char)*width*height );
	cudaMemcpy( dev_src, host_src, sizeof(char)*width*height, cudaMemcpyHostToDevice );

	int *host_mask = _getRangeMask( kernelSize, width );
	int *dev_mask;
	cudaMalloc( (void**)&dev_mask, sizeof(int)*kernelSize*kernelSize);
	cudaMemcpy( dev_mask, host_mask, sizeof(int)*kernelSize*kernelSize, cudaMemcpyHostToDevice );

	_erode<<< nBlocks, nMaxThreadsPerBlock >>>( dev_src, dev_dst, width, height, kernelSize, dev_mask );
	
	cudaMemcpy( host_dst, dev_dst, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	
	cudaFree( dev_src );
	cudaFree( dev_dst );
	cudaFree( dev_mask );
	free( host_mask );
}

void gpu_dilate( char *host_src, char *host_dst, int width, int height, int kernelSize = 3 )
{
	cudaDeviceProp prop;
	int nMaxThreadsPerBlock;
	int count = 1;
	cudaGetDeviceCount( &count );
	for( int i = 0; i < count; i++  ) {
		cudaGetDeviceProperties( &prop, i );
		nMaxThreadsPerBlock = prop.maxThreadsPerBlock;		
	}
	const int nBlocks = ( width * height + nMaxThreadsPerBlock - 1 )  / nMaxThreadsPerBlock;	
	char *dev_src, *dev_dst;
	cudaMalloc( (void**)&dev_src, sizeof(char)*width*height );
	cudaMalloc( (void**)&dev_dst, sizeof(char)*width*height );
	cudaMemcpy( dev_src, host_src, sizeof(char)*width*height, cudaMemcpyHostToDevice );
	
	int *host_mask = _getRangeMask( kernelSize, width );
	int *dev_mask;
	cudaMalloc( (void**)&dev_mask, sizeof(int)*kernelSize*kernelSize);
	cudaMemcpy( dev_mask, host_mask, sizeof(int)*kernelSize*kernelSize, cudaMemcpyHostToDevice );
		
	_dilate<<< nBlocks, nMaxThreadsPerBlock >>>( dev_src, dev_dst, width, height, kernelSize, dev_mask );
	
	cudaMemcpy( host_dst, dev_dst, sizeof(char)*width*height, cudaMemcpyDeviceToHost );
	
	
	cudaFree( dev_src );
	cudaFree( dev_dst );
	cudaFree( dev_mask );
	free( host_mask );
}