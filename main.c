#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Rawiv-reader.h"
#include "PPM-writer.h"

#define IndexVect(i,j,k) ((k) * xdim * ydim + (j) * xdim + (i))

float viewplane = 160;
float viewpoint[3] = { 64, 64, 300 };
float intensity[4] = { 20, 50, 80, 110 };
float opacity[5] = { .2, .3, .4, .3, .2 };
float color[5][3] = { {255, 128, 128}, {128, 255, 128}, {128, 128, 255}, {255, 255, 128}, {255, 128, 255} };

// normalize a vectorby shrinking the length to 1
void normalize(float *vec)
{
	float len = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

// different implementations for finding intensity for the point
float find_intensity(float *point, float* volume, int xdim, int ydim, int zdim)
{
	int i = floor(point[0]);
	int j = floor(point[1]);
	int k = floor(point[2]);

	/*
	// closest value
	if (point[0] - i > .5)
		i++;
	if (point[1] - j > .5)
		j++;
	if (point[2] - k > .5)
		k++;

	return volume[IndexVect(i, j, k)];
	*/

	// trilinear interpolation
	float x = point[0] - i;
	float y = point[1] - j;
	float z = point[2] - k;

	return volume[IndexVect(i, j, k)] * (1 - x) * (1 - y) * (1 - z)
		+ volume[IndexVect(i + 1, j, k)] * x * (1 - y) * (1 - z)
		+ volume[IndexVect(i, j + 1, k)] * (1 - x) * y * (1 - z)
		+ volume[IndexVect(i, j, k + 1)] * (1 - x) * (1 - y) * z
		+ volume[IndexVect(i + 1, j, k + 1)] * x * (1 - y) * z
		+ volume[IndexVect(i, j + 1, k + 1)] * (1 - x) * y * z
		+ volume[IndexVect(i + 1, j + 1, k)] * x * y * (1 - z)
		+ volume[IndexVect(i + 1, j + 1, k + 1)] * x * y * z;
}

int main(int argc, char *argv[])
{
	int xdim, ydim, zdim;
	float *volume;
	FILE *fp;
	
	/*
	if (argc != 3){
	  printf("Usage: test <input_filename> <output> \n");
	  printf("       <input_filename>:   RAWIV file \n");
	  printf("       <output>:   PPM file \n");
	  exit(0);
	}
	*/

	printf("begin reading rawiv.... \n");
	//ReadRawiv(&xdim, &ydim, &zdim, &volume, argv[1]);
	ReadRawiv(&xdim, &ydim, &zdim, &volume, "heart-128.rawiv");

	// define you output dimension
	int outXDIM = 128;
	int outYDIM = 128;
	RGBDATA *output;
	output = (RGBDATA*)malloc(sizeof(RGBDATA) * outXDIM * outYDIM);

	float vec[3], point[3];
	int i, j, t;
	for (i = 0; i < outXDIM; i++) {
		for (j = 0; j < outYDIM; j++) {
			vec[0] = i - viewpoint[0];
			vec[1] = j - viewpoint[1];
			vec[2] = viewplane - viewpoint[2];

			normalize(vec);

			float den;
			float red = 0, green = 0, blue = 0, alpha = 0;
			for (t = 1; t <= 300; t++) {
				point[0] = viewpoint[0] + vec[0] * t;
				point[1] = viewpoint[1] + vec[1] * t;
				point[2] = viewpoint[2] + vec[2] * t;

				if (point[0] < 0 || point[1] < 0 || point[2] < 0 || point[0] >= xdim - 1 || point[1] >= ydim - 1 || point[2] >= zdim - 1)
					continue;

				den = find_intensity(point, volume, xdim, ydim, zdim);

				int index = 0;
				while (den > intensity[index])
					index++;

				red += (1 - alpha) * color[index][0] * opacity[index];
				green += (1 - alpha) * color[index][1] * opacity[index];
				blue += (1 - alpha) * color[index][2] * opacity[index];
				alpha += (1 - alpha) * opacity[index];
				
				/*
				red = red * alpha + (1 - alpha) * color[index][0];
				green = green * alpha + (1 - alpha) * color[index][1];
				blue = blue * alpha + (1 - alpha) * color[index][2];
				alpha += (1 - alpha) * opacity[index];
				*/
			}

			output[i * outYDIM + j].r = red;
			output[i * outYDIM + j].g = green;
			output[i * outYDIM + j].b = blue;
		}
	}

	// scale the output image to[0, 255]
	unsigned char maxr = 0, maxg = 0, maxb = 0;
	unsigned char minr = 255, ming = 255, minb = 255;

	for (i = 0; i < outXDIM * outYDIM; i++) {
		if (output[i].r < minr)
			minr = output[i].r;
		if (output[i].r > maxr)
			maxr = output[i].r;
		if (output[i].g < ming)
			ming = output[i].g;
		if (output[i].g > maxg)
			maxg = output[i].g;
		if (output[i].b < minb)
			minb = output[i].b;
		if (output[i].b > maxb)
			maxb = output[i].b;
	}

	unsigned char min = minr < ming ? minr : ming;
	min = minb < min ? minb : min;
	unsigned char max = maxr > maxg ? maxr : maxg;
	max = maxg > max ? maxg : max;
	float dis = max - min;
	for (i = 0; i < outXDIM * outYDIM; i++) {
		output[i].r = (unsigned char)((output[i].r - min) / dis * 255);
		output[i].g = (unsigned char)((output[i].g - min) / dis * 255);
		output[i].b = (unsigned char)((output[i].b - min) / dis * 255);
	}

	/* Begin writing PPM.... This is your output picture */
	printf("Begin writing PPM.... \n");

	//if ((fp=fopen(argv[2], "wb")) == NULL){
	if ((fp = fopen("test.ppm", "wb")) == NULL) {
		printf("write ppm error....\n");
		exit(0);
	}

	WritePPM(outXDIM, outYDIM, output, fp);

	free(output);
	free(volume);

	return(0);
}