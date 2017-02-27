#include <stdio.h>
#include <cuda_runtime.h>

class CudaTimer {
public:
	cudaEvent_t cudaEventStart, cudaEventStop;
	float elapsed_time_ms;
	CudaTimer()
	{
		cudaEventCreate(&cudaEventStart);
		cudaEventCreate(&cudaEventStop);
	}

	~CudaTimer()
	{
		cudaEventDestroy(cudaEventStart);
		cudaEventDestroy(cudaEventStop);
	}

	void record()
	{
		cudaEventRecord(cudaEventStart, 0);
	}

	double stop()
	{
		elapsed_time_ms = 0.0f;
		cudaEventRecord(cudaEventStop, 0);
		cudaEventSynchronize(cudaEventStop);
		cudaEventElapsedTime(&elapsed_time_ms, cudaEventStart, cudaEventStop);
		return elapsed_time_ms;
	}
};