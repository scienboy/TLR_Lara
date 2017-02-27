#include <Windows.h>

class CpuTimer {
public:
	LARGE_INTEGER Frequency;
	LARGE_INTEGER BeginTime;
	LARGE_INTEGER Endtime;
	double elapsed_time_ms;
	CpuTimer()
	{
		QueryPerformanceFrequency( &Frequency );
	}

	~CpuTimer() {}

	void record()
	{
		QueryPerformanceCounter( &BeginTime );
	}

	double stop()
	{		
		QueryPerformanceCounter( &Endtime );
		elapsed_time_ms = (static_cast<float>(Endtime.QuadPart - BeginTime.QuadPart) / Frequency.QuadPart)*1000.0;
		return elapsed_time_ms;
	}
};