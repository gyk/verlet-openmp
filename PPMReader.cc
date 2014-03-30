#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PPMReader.h"

PPMTexture PPMReader_load(char* fileName)
{
	FILE* fp = fopen(fileName, "rb");
	PPMTexture ppm = {0};
	char header[3];
	// TODO: fix security issue here.
	fscanf(fp, "%2s\n%d %d\n%d\n", header, 
		&ppm.width, &ppm.height, &ppm.range);

	if (strcmp(header, "P6")) {
		fprintf(stderr, "Only P6 PPM file is supported.\n");
		fclose(fp);
		return ppm;
	}

	if (ppm.range != 255) {
		fprintf(stderr, "Range must be 255.\n");
		fclose(fp);
		return ppm;
	}

	int length = ppm.width * ppm.height * 3;
	ppm.data = (unsigned char*)calloc(length, sizeof(unsigned char));

	fread(ppm.data, 1, length, fp);
	fclose(fp);
	return ppm;
}