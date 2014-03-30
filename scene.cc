#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include "scene.h"

/* Private */
static void Scene_initialize(Scene* scene);
static void Scene_compute(Scene* scene);

struct _Scene
{
	double space[3];
	int nParticles;
	double* mass;
	Vector* position;
	Vector* velocity;
	Vector* force;
	Vector* acceleration;
	double initEnergy;
	double wholeEnergy;
	double potEnergy;
	double kinEnergy;
};

Scene* Scene_new(double sizeX, double sizeY, double sizeZ, int nParticles)
{
	Scene* scene = (Scene*)malloc(sizeof(*scene));
	scene->space[0] = sizeX * 0.5;
	scene->space[1] = sizeY * 0.5;
	scene->space[2] = sizeZ * 0.5;
	scene->nParticles = nParticles;

	scene->mass = (double*)calloc(nParticles, sizeof(double));
	scene->position = (Vector*)calloc(nParticles, sizeof(Vector));
	scene->velocity = (Vector*)calloc(nParticles, sizeof(Vector));
	scene->force = (Vector*)calloc(nParticles, sizeof(Vector));
	scene->acceleration = (Vector*)calloc(nParticles, sizeof(Vector));

// zero bytes don't necessarily mean 0.0 in C99
#if defined(__STDC__)
# if (__STDC_VERSION__ >= 199409L)
	assert(scene->position[0][0] == 0.0);
# endif
#endif

	Scene_initialize(scene);
	return scene;
}

void Scene_initialize(Scene* scene)
{
	for (int i=0; i<scene->nParticles; i++) {
		scene->mass[i] = MASS_MIN + (double)rand()/RAND_MAX * MASS_RANGE;
		for (int d=0; d<3; d++) {
			scene->position[i][d] = 
				((double)rand()/RAND_MAX - 0.5) * scene->space[d] * 2.0;
			scene->velocity[i][d] = ((double)rand()/RAND_MAX - 0.5) * 2.0;
		}
	}
	Scene_compute(scene);
	scene->initEnergy = scene->wholeEnergy;
}

double distance(double p[], double q[], double scales[])
{
	double dis = 0.0;
	for (int i=0; i<3; i++) {
		scales[i] = p[i] - q[i];
		dis += scales[i] * scales[i];
	}
	dis = sqrt(dis);
	for (int i=0; i<3; i++) {
		scales[i] /= dis;
	}
	return dis;
}

void Scene_compute(Scene* scene)
{
	int i, j, d;
	double dis, scales[3];
	double potEnergy = 0.0, kinEnergy = 0.0;

	Vector* pos = scene->position;
	Vector* vel = scene->velocity;
	Vector* frc = scene->force;

	memset(frc, 0, scene->nParticles * sizeof(*frc));
#pragma omp parallel \
	default (shared) private (i, j, d, dis, scales)
#pragma omp for reduction (+ : potEnergy, kinEnergy)
	for (i=0; i<scene->nParticles; i++) {
		for (j=i+1; j<scene->nParticles; j++) {
			dis = distance(pos[i], pos[j], scales);
			double force;

		#if defined(LENNARD_JONES_POTENTIAL)
			const double SIGMA = 1.0, EPSILON = 1.0;
			double t = SIGMA / dis;
			t = t * t;
			t = t * t * t;
			force = -24.0 * EPSILON / dis * (2.0 * t * t - t);
			potEnergy += 4.0 * EPSILON * (t * t - t);
		#elif defined(SINE_POTENTIAL)
			const double PI_OVER_2 = 2.0 * atan(1.0);
			if (dis > PI_OVER_2) dis = PI_OVER_2;
			force = sin(2.0 * dis) * (MASS_MIN + MASS_RANGE);
			double sinDis = sin(dis);
			potEnergy += sinDis * sinDis;
		#else
			#error "Please specify the potential function"
		#endif

			for (d=0; d<3; d++) {
				double f = force * scales[d];
				frc[i][d] -= f;
				frc[j][d] += f;
			}
		}

		for (d=0; d<3; d++) {
			kinEnergy += vel[i][d] * vel[i][d] * scene->mass[i];
		}
	}
	scene->potEnergy = potEnergy;
	scene->kinEnergy = kinEnergy * 0.5;
	scene->wholeEnergy = scene->potEnergy + scene->kinEnergy;
}


void Scene_update(Scene* scene, double dt)
{
	Scene_compute(scene);
	double r_ratio = scene->kinEnergy / 
			(scene->kinEnergy + (scene->wholeEnergy - scene->initEnergy));

	const int COUNTER_MAX = 5000;
	static int counter = COUNTER_MAX;
	if (++counter > COUNTER_MAX) {
		printf("Energy = %g (potential = %g, kinectic = %g)\n", 
			scene->wholeEnergy, scene->potEnergy, scene->kinEnergy);
		counter = 0;
	}

	Vector* pos = scene->position;
	Vector* vel = scene->velocity;
	Vector* acc = scene->acceleration;
	Vector* frc = scene->force;

	int i, d;
#pragma omp parallel \
	default (shared) private (i, d)
#pragma omp for
	for (i=0; i<scene->nParticles; i++) {
		double r_mass = 1.0 / scene->mass[i];
		for (d=0; d<3; d++) {
			double vel_i_d = vel[i][d] * r_ratio;
			pos[i][d] = pos[i][d] + vel_i_d * dt + 0.5 * acc[i][d] * dt * dt;
			vel[i][d] = vel_i_d + 
				0.5 * dt * (frc[i][d] * r_mass + acc[i][d]);
			acc[i][d] = frc[i][d] * r_mass;
			if (pos[i][d] > scene->space[d]) {
				vel[i][d] = -abs(vel[i][d]);
			} else if (pos[i][d] < -scene->space[d]) {
				vel[i][d] = abs(vel[i][d]);
			}
		}
	}
}

void Scene_print(Scene* scene)
{
	for (int i=0; i<5; i++) {
		for (int d=0; d<3; d++) {
			printf("%g ", scene->position[i][d]);
		}
		printf("\n");
	}
}

double* Scene_getMasses(Scene* scene)
{
	return scene->mass;
}

Vector* Scene_getVertices(Scene* scene)
{
	return scene->position;
}

void Scene_free(Scene* scene)
{
	free(scene->mass);
	free(scene->position);
	free(scene->velocity);
	free(scene->force);
	free(scene->acceleration);
	free(scene);
}
