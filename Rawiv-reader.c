#include "Rawiv-reader.h"

/*
 * ***************************************************************************
 * Routine:  swap_buffer    < ... >
 *
 * Purpose:  swap the bytes when LITTLE_ENDIAN is enabled
 * ***************************************************************************
 */
void swap_buffer(char *buffer, int count, int typesize)
{
	char sbuf[4];
	int i;
	int temp = 1;
	unsigned char* chartempf = (unsigned char*)&temp;
	if (chartempf[0] > '\0') {

		// swapping isn't necessary on single byte data
		if (typesize == 1)
			return;

		for (i = 0; i < count; i++)
		{
			memcpy(sbuf, buffer + (i*typesize), typesize);

			switch (typesize)
			{
			case 2:
			{
				buffer[i*typesize] = sbuf[1];
				buffer[i*typesize + 1] = sbuf[0];
				break;
			}
			case 4:
			{
				buffer[i*typesize] = sbuf[3];
				buffer[i*typesize + 1] = sbuf[2];
				buffer[i*typesize + 2] = sbuf[1];
				buffer[i*typesize + 3] = sbuf[0];
				break;
			}
			default:
				break;
			}
		}
	}
}



/*
 * ***************************************************************************
 * Routine:  ReadRawiv    < ... >
 *
 * Purpose:  Read the volume in rawiv format
 * ***************************************************************************
 */
void ReadRawiv(int *xd, int *yd, int *zd, float **dataset, char *input_name)
{
	float c_float;
	unsigned char c_unchar;
	unsigned short c_unshort;
	int i, j, k;
	float *data;
	int xdim, ydim, zdim;
	float maxraw;
	float minraw;
	float minext[3], maxext[3];
	int nverts, ncells;
	unsigned int dim[3];
	float orig[3], span[3];
	struct stat filestat;
	size_t size[3];
	int datatype;
	int found;
	FILE *fp;



	if ((fp = fopen(input_name, "rb")) == NULL) {
		printf("read error...\n");
		exit(0);
	}
	stat(input_name, &filestat);

	/* reading RAWIV header */
	fread(minext, sizeof(float), 3, fp);
	fread(maxext, sizeof(float), 3, fp);
	fread(&nverts, sizeof(int), 1, fp);
	fread(&ncells, sizeof(int), 1, fp);
#ifdef _LITTLE_ENDIAN
	swap_buffer((char *)minext, 3, sizeof(float));
	swap_buffer((char *)maxext, 3, sizeof(float));
	swap_buffer((char *)&nverts, 1, sizeof(int));
	swap_buffer((char *)&ncells, 1, sizeof(int));
#endif  

	size[0] = 12 * sizeof(float) + 2 * sizeof(int) + 3 * sizeof(unsigned int) +
		nverts * sizeof(unsigned char);
	size[1] = 12 * sizeof(float) + 2 * sizeof(int) + 3 * sizeof(unsigned int) +
		nverts * sizeof(unsigned short);
	size[2] = 12 * sizeof(float) + 2 * sizeof(int) + 3 * sizeof(unsigned int) +
		nverts * sizeof(float);

	found = 0;
	for (i = 0; i < 3; i++)
		if (size[i] == (unsigned int)filestat.st_size)
		{
			if (found == 0)
			{
				datatype = i;
				found = 1;
			}
		}
	if (found == 0)
	{
		printf("Corrupted file or unsupported dataset type\n");
		exit(5);
	}

	fread(dim, sizeof(unsigned int), 3, fp);
	fread(orig, sizeof(float), 3, fp);
	fread(span, sizeof(float), 3, fp);

#ifdef _LITTLE_ENDIAN
	swap_buffer((char *)dim, 3, sizeof(unsigned int));
	swap_buffer((char *)orig, 3, sizeof(float));
	swap_buffer((char *)span, 3, sizeof(float));
#endif

	xdim = dim[0];
	ydim = dim[1];
	zdim = dim[2];

	data = (float *)malloc(sizeof(float)*xdim*ydim*zdim);

	maxraw = -999999.f;
	minraw = 999999.f;
	if (datatype == 0) {
		printf("data type: unsigned char \n");
		for (i = 0; i < zdim; i++)
			for (j = 0; j < ydim; j++)
				for (k = 0; k < xdim; k++) {
					fread(&c_unchar, sizeof(unsigned char), 1, fp);
					data[IndexVect(k, j, i)] = (float)c_unchar;
					if (c_unchar > maxraw)
						maxraw = c_unchar;
					if (c_unchar < minraw)
						minraw = c_unchar;
				}
	}
	else if (datatype == 1) {
		printf("data type: unsigned short \n");
		for (i = 0; i < zdim; i++)
			for (j = 0; j < ydim; j++)
				for (k = 0; k < xdim; k++) {
					fread(&c_unshort, sizeof(unsigned short), 1, fp);
#ifdef _LITTLE_ENDIAN
					swap_buffer((char *)&c_unshort, 1, sizeof(unsigned short));
#endif 
					data[IndexVect(k, j, i)] = (float)c_unshort;

					if (c_unshort > maxraw)
						maxraw = c_unshort;
					if (c_unshort < minraw)
						minraw = c_unshort;
				}
	}
	else if (datatype == 2) {
		printf("data type: float \n");
		for (i = 0; i < zdim; i++)
			for (j = 0; j < ydim; j++)
				for (k = 0; k < xdim; k++) {
					fread(&c_float, sizeof(float), 1, fp);
#ifdef _LITTLE_ENDIAN
					swap_buffer((char *)&c_float, 1, sizeof(float));
#endif 
					data[IndexVect(k, j, i)] = c_float;

					if (c_float > maxraw)
						maxraw = c_float;
					if (c_float < minraw)
						minraw = c_float;
				}
	}
	else {
		printf("error\n");
		fclose(fp);
		exit(1);
	}

	fclose(fp);
	printf("minimum = %f,   maximum = %f \n", minraw, maxraw);

	for (i = 0; i < zdim; i++)
		for (j = 0; j < ydim; j++)
			for (k = 0; k < xdim; k++) {
				data[IndexVect(k, j, i)] = 255 * (data[IndexVect(k, j, i)] - minraw) / (maxraw - minraw);
			}

	printf("dimension: %d X %d X %d\n", xdim, ydim, zdim);
	*xd = xdim;
	*yd = ydim;
	*zd = zdim;
	*dataset = data;
}
