#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "mpi.h"


#define NUM_COLORS 256
#define C_ABS(a, b) sqrt(a * a + b * b)

int k, i, j;

int main(int argc, char** argv) {

	int type;
	float x_min, x_max, y_min, y_max;
	float step;
	int iterations;
	float cx, cy;
	
	/* ctirie fisier input */
	FILE *fr = fopen(argv[1], "r");
	fscanf(fr, "%d", &type);
	fscanf(fr, "%f%f%f%f", &x_min, &x_max, &y_min, &y_max);
	fscanf(fr, "%f", &step);
	fscanf(fr, "%d", &iterations);
	if(type == 1) {
		fscanf(fr, "%f %f", &cx, &cy);
	}
	fclose(fr);

	/* calcul rezolutie in pixeli */
	int width = (int) round((x_max - x_min) / step);
	int height = (int) round((y_max - y_min) / step);

	int **image = (int**) malloc(height * sizeof(int*));
	for(k = 0; k < height + 1; k++) {
		image[k] = (int*) malloc(width * sizeof(int));	
	}


	float x, y;
	float a, b; // z = a + i * b
	float old_a, old_b;
	int iter, color;
	
	/* implementare algoritm */
	if(type == 0) {
		i = height - 1;
		j = 0;
		for(x = x_min; x < x_max; x = x + step) {
			for(y = y_min; y < y_max; y = y + step) {
				a = 0;
				b = 0;
				iter = 0;
				while(C_ABS(a, b) < 2 && iter < iterations) {
					old_a = a;
					old_b = b;
					a = old_a * old_a - old_b * old_b + x;
					b = 2 * old_a * old_b + y;
					iter++;
				}
				if(i >= 0) {
					color = iter % NUM_COLORS;
					image[i][j] = color;
					i--;
				}
			}
			j++;
			i = height - 1;
		}
	} else if(type == 1) {
		i = height - 1;
		j = 0;
		for(x = x_min; x < x_max; x = x + step) {
			for(y = y_min; y < y_max; y = y + step) {
				a = x;
				b = y;
				iter = 0;
				while(C_ABS(a, b) < 2 && iter < iterations) {
					old_a = a;
					old_b = b;
					a = old_a * old_a - old_b * old_b + cx;
					b = 2 * old_a * old_b + cy;
					iter++;
				}
				if(i >= 0) {
					color = iter % NUM_COLORS;
					image[i][j] = color;
					i--;
				}
			}
			j++;
			i = height - 1;
		}
	}
	
	/* scriere in fisier */	
	FILE *fw = fopen(argv[2], "w");
	fprintf(fw, "P2\n");
	fprintf(fw, "%d %d\n", height, width);
	fprintf(fw, "255\n");
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			fprintf(fw, "%d ", image[i][j]);
		}
		fprintf(fw, "\n");
	}
	fclose(fw);
	
	
	return 0;    
}
