#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scene.h"
#include "stopwatch.h"

int main(int argc, char const *argv[])
{
	int nParticles;
	nParticles = 800;
	Scene* scene = Scene_new(10.0, 10.0, 8.0, nParticles);
	Stopwatch* stopwatch = Stopwatch_new();

	Stopwatch_start(stopwatch);
	for (int i=0; i<1000; i++) {
		Scene_update(scene, 0.001);
		// Scene_print(scene);
		// getchar();
	}
	Stopwatch_stop(stopwatch);
	printf("Elapsed time = %gms\n", Stopwatch_elapsedTimeMilliSec(stopwatch));
	Stopwatch_free(stopwatch);
	Scene_free(scene);

	return 0;
}