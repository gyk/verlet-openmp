all: verlet.exe verlet_openmp.exe verletDemo.exe

CFLAGS = -Wall -O2 -o
# CFLAGS = -Wall -O2 -g -o
# CFLAGS = -Wall -O0 -pg -fno-inline-small-functions -o
CFLAGS_OBJ = -c $(CFLAGS)
GL_LIBS = -lopengl32 -lglu32 -lfreeglut

verlet.exe: verlet.o scene.o stopwatch.o
	gcc $(CFLAGS) $@ $^

verlet_openmp.exe: verlet.o scene_openmp.o stopwatch.o
	gcc $(CFLAGS) $@ $^ -fopenmp

verletDemo.exe: verletDemo.o PPMReader.o scene_openmp.o
	gcc $(CFLAGS) $@ $^ -Llib $(GL_LIBS) -fopenmp

verlet.o: verlet.cc
	gcc $(CFLAGS_OBJ) $@ $^

verletDemo.o: verletDemo.cc
	gcc $(CFLAGS_OBJ) $@ -c $^ -Iinclude

PPMReader.o: PPMReader.cc
	gcc $(CFLAGS_OBJ) $@ $^

scene.o: scene.cc
	gcc $(CFLAGS_OBJ) $@ $^ -DLENNARD_JONES_POTENTIAL

scene_openmp.o: scene.cc
	gcc $(CFLAGS_OBJ) $@ $^ -DLENNARD_JONES_POTENTIAL -fopenmp

stopwatch.o: stopwatch.cc
	gcc $(CFLAGS_OBJ) $@ $^

clean:
	del *.exe
