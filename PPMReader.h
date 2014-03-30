

typedef struct _PPMTexture
{
	int width;
	int height;
	int range;
	unsigned char* data;
} PPMTexture;

PPMTexture PPMReader_load(char* fileName);