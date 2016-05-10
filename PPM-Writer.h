#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGBDATA;

void WritePPM(int XDIM, int YDIM, RGBDATA *dataset, FILE* fp);