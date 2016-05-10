#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

// if you see an error message "Corrupted file or unsupported dataset type",
// you need to define the _LITTLE_ENDIAN as follows:
#define _LITTLE_ENDIAN   1
#define IndexVect(i, j, k) ((k) * xdim * ydim + (j) * xdim + (i))

void swap_buffer(char *buffer, int count, int typesize);
void ReadRawiv(int *xd, int *yd, int *zd, float **dataset, char *input_name);