#include <stdlib.h>
#include <windows.h>
#include "stopwatch.h"

struct _Stopwatch
{
	bool stopped;
	double start_time_us;
	double end_time_us;
	LARGE_INTEGER frequency;
	LARGE_INTEGER startCount;
	LARGE_INTEGER endCount;
};

Stopwatch* Stopwatch_new()
{
	Stopwatch* stopwatch;
	stopwatch = (Stopwatch*)calloc(1, sizeof(*stopwatch));
	stopwatch->stopped = true;
	QueryPerformanceFrequency(&stopwatch->frequency);
	return stopwatch;
}

void Stopwatch_start(Stopwatch* stopwatch)
{
	QueryPerformanceCounter(&stopwatch->startCount);
	stopwatch->stopped = false;
}

void Stopwatch_stop(Stopwatch* stopwatch)
{
	QueryPerformanceCounter(&stopwatch->endCount);
	stopwatch->stopped = true;
}

double Stopwatch_elapsedTimeMicroSec(Stopwatch* stopwatch)
{
	if (!stopwatch->stopped) {
		QueryPerformanceCounter(&stopwatch->endCount);
	}

	double startTimeMicroSec = stopwatch->startCount.QuadPart * 
		(1000000.0 / stopwatch->frequency.QuadPart);
	double endTimeMicroSec = stopwatch->endCount.QuadPart * 
		(1000000.0 / stopwatch->frequency.QuadPart);
	return endTimeMicroSec - startTimeMicroSec;
}

double Stopwatch_elapsedTimeMilliSec(Stopwatch* stopwatch)
{
	return Stopwatch_elapsedTimeMicroSec(stopwatch) * 0.001;
}

void Stopwatch_free(Stopwatch* stopwatch)
{
	free(stopwatch);
}
