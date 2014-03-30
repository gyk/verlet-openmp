#pragma once

typedef struct _Scene Scene;
typedef double Vector[3];

const double MASS_MIN = 1.0;
const double MASS_RANGE = 4.0;

Scene* Scene_new(double size_x, double size_y, double size_z, int n_particles);
void Scene_update(Scene* scene, double dt);
void Scene_print(Scene* scene);
double* Scene_getMasses(Scene* scene);
Vector* Scene_getVertices(Scene* scene);
void Scene_free(Scene* scene);
