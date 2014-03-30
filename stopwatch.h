#pragma once

typedef struct _Stopwatch Stopwatch;

Stopwatch* Stopwatch_new();
void Stopwatch_start(Stopwatch* stopwatch);
void Stopwatch_stop(Stopwatch* stopwatch);
double Stopwatch_elapsedTimeMicroSec(Stopwatch* stopwatch);
double Stopwatch_elapsedTimeMilliSec(Stopwatch* stopwatch);
void Stopwatch_free(Stopwatch* stopwatch);
