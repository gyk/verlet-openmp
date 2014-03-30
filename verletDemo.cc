#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include "scene.h"
#include "PPMReader.h"

// GLUT CALLBACK functions
void render();
void reshape(int w, int h);
void idle();
void mouseClick(int button, int stat, int x, int y);
void mouseMove(int x, int y);

// constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float CAMERA_DISTANCE = 15.0f;
const int N_PARTICLES = 100;

int screenWidth = SCREEN_WIDTH;
int screenHeight = SCREEN_HEIGHT;
int mouseLeftDown = GLUT_UP;
int mouseRightDown = GLUT_UP;
float mouseX = 0, mouseY = 0;
float cameraAngleX = 0.0f, cameraAngleY = 0.0f;
float cameraDistance = CAMERA_DISTANCE;

GLuint textureHandle = -1;

PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB  = NULL;
PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;

Scene* scene;
GLfloat (*colors)[3];
Vector* positions;


void render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0, 0, -cameraDistance);
	glRotatef(cameraAngleX, 1, 0, 0);   // pitch
	glRotatef(cameraAngleY, 0, 1, 0);   // heading

	glEnable(GL_POINT_SPRITE_ARB);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, (GLdouble*)positions);
	glColorPointer(3, GL_FLOAT, 0, (GLfloat*)colors);

	// draw point sprites
	glDrawArrays(GL_POINTS, 0, N_PARTICLES);

	glDisable(GL_POINT_SPRITE_ARB);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glPopMatrix();
	glutSwapBuffers();
}

static void prepare()
{
	glShadeModel(GL_SMOOTH);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 0);

	char* ext = (char*)glGetString(GL_EXTENSIONS);
	if(strstr(ext, "GL_ARB_point_parameters") == NULL) {
		fprintf(stderr, "GL_ARB_point_parameters extension was not found.\n");
		exit(-1);
	}

	glPointParameterfARB  = (PFNGLPOINTPARAMETERFARBPROC)
		wglGetProcAddress("glPointParameterfARB");
	glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)
		wglGetProcAddress("glPointParameterfvARB");
	if (!glPointParameterfARB || !glPointParameterfvARB) {
		fprintf(stderr, "GL_ARB_point_parameters functions were not available.\n");
		exit(-1);
	}

	PPMTexture ppm = PPMReader_load((char*)"particle.ppm");
	if (!ppm.data) {
		fprintf(stderr, "particle.ppm is corrupt.\n");
		exit(-1);
	}
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ppm.width, ppm.height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, ppm.data);

	glDepthMask(GL_FALSE);
	// set up point sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float quadratic[] =  { 1.0f, 0.0f, 0.01f };
	glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);

	float maxSize = 0.0f;
	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxSize);
	if (maxSize > 100.0f) maxSize = 100.0f;
	
	glPointSize(maxSize);
	glPointParameterfARB(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f);
	glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, maxSize);

	glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
	// glBindTexture(GL_TEXTURE_2D, textureHandle);

}

int main(int argc, char** argv)
{
	scene = Scene_new(12.0, 9.0, 12.0, N_PARTICLES);
	double* masses = Scene_getMasses(scene);
	colors = (float (*)[3])malloc(N_PARTICLES * sizeof(float) * 3);
	for (int i=0; i<N_PARTICLES; i++) {
		float scale = (float)((masses[i] - MASS_MIN) / MASS_RANGE);
		colors[i][0] = scale;
		colors[i][1] = 1 - scale;
		colors[i][2] = 0.0;
	}
	positions = Scene_getVertices(scene);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Verlet demo");

	prepare();
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);

	glutMainLoop();
	// while (true) {
	// 	glutMainLoopEvent();
	// 	for (int i=0; i<10; i++) {
	// 		Scene_update(scene, 0.001);
	// 	}
	// }
	return 0;
}

void toPerspective()
{
	// set viewport to be the entire window
	glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

	// set perspective viewing frustum
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// FOV, AspectRatio, NearClip, FarClip
	gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 0.5f, 1000.0f);

	// switch to modelview matrix in order to set scene
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void reshape(int w, int h)
{
	screenWidth = w;
	screenHeight = h;
	toPerspective();
	glutPostRedisplay();
}

void idle()
{
	for (int i=0; i<20; i++) {
		Scene_update(scene, 0.0001);
	}
	glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y)
{
	mouseX = x;
	mouseY = y;

	if(button == GLUT_LEFT_BUTTON) {
		mouseLeftDown = (state == GLUT_DOWN);
	} else if(button == GLUT_RIGHT_BUTTON) {
		mouseRightDown = (state == GLUT_DOWN);
	}
}

void mouseMove(int x, int y)
{
	if(mouseLeftDown) {
		cameraAngleY += x - mouseX;
		cameraAngleX += y - mouseY;
		mouseX = x;
		mouseY = y;
	} else if (mouseRightDown) {
		cameraDistance -= (y - mouseY) * 0.2f;
		mouseY = y;
	} else {
		return;
	}
	glutPostRedisplay();
}