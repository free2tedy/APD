#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alloca.h>
#include <sys/resource.h>
#include "mpi.h"

#define NUM_COLORS 256
#define C_ABS(a, b) sqrt(a * a + b * b)

int k, i, j;
int type, iterations;
double x_min, x_max, y_min, y_max;
double step;
double cx, cy;
int xx, yy;
		
int main(int argc, char** argv) {

	/* initializari specifice MPI */
	int rank, size;	
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Masterul se ocupa de citirea fisierului de intrare */
	if (rank == 0) {	
		/* ctirie fisier input */
		FILE *fr = fopen(argv[1], "r");
		fscanf(fr, "%d", &type);
		fscanf(fr, "%lf%lf%lf%lf", &x_min, &x_max, &y_min, &y_max);
		fscanf(fr, "%lf", &step);
		fscanf(fr, "%d", &iterations);
		if (type == 1) {
			fscanf(fr, "%lf %lf", &cx, &cy);
		}
		fclose(fr);
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	/* trimitere prin broadcast a datelor de intrare catre celalalte procese */
	MPI_Bcast(&type, 1, MPI_INT, 0, MPI_COMM_WORLD);			
	MPI_Bcast(&x_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&x_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&y_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&y_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&step, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (type == 1) {
		MPI_Bcast(&cx, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&cy, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}						
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	/* calcul rezolutie in pixeli */
	int width = (int) round ((x_max - x_min) / step);
	int height = (int) round ((y_max - y_min) / step);
	double length = (y_max - y_min) / size;

	/* alocare matrice */
	int **image = (int**) calloc(height, sizeof(int*));
	for (k = 0; k < height; k++) {
		image[k] = (int*) calloc(width, sizeof(int));	
	}

	double x, y;
	double a, b; // z = a + i * b
	double old_a, old_b;
	int iter, color;
		
	MPI_Barrier(MPI_COMM_WORLD);
	
	/* implementare algoritm */
	if (type == 0) {
		/* mandelbrot */
		i = (int) round ((float) (size - rank) / size * height) - 1; // iterez invers pe linii
		j = 0;
		for (x = x_min; x < x_max; x = x + step) {
			for (y = y_min + rank * length; y < y_min + (rank + 1) * length; y = y + step) {
				a = 0;
				b = 0;
				iter = 0;
				while (C_ABS(a, b) < 2 && iter < iterations) {
					old_a = a;	// salvez vechiul numaru complex
					old_b = b;
					a = old_a * old_a - old_b * old_b + x; // calculez noua valoare
					b = 2 * old_a * old_b + y;
					iter++;
				}
				if (i >= (int) round((float) (size - rank - 1) / size * height)) {
					// pun culoarea in matrice
					color = iter % NUM_COLORS;
					image[i][j] = color;
					i--;
				}
			}
			i = (int) round ((float) (size - rank) / size * height) - 1;
			j++;
			if (j == width) {
				break;
			}
		}
	} else if (type == 1) {
		/* julia */
		i = (int) round ((float) (size - rank) / size * height) - 1;
		j = 0;
		for (x = x_min; x < x_max; x = x + step) {
			for (y = y_min + rank * length; y < y_min + (rank + 1) * length; y = y + step) {
				a = x;
				b = y;
				iter = 0;
				while (C_ABS(a, b) < 2 && iter < iterations) {
					old_a = a;
					old_b = b;
					a = old_a * old_a - old_b * old_b + cx;
					b = 2 * old_a * old_b + cy;
					iter++;
				}
				if (i >= (int) round((float) (size - rank - 1) / size * height)) {
					color = iter % NUM_COLORS;
					image[i][j] = color;
					i--;
				}				
			}
			i = (int) round ((float) (size - rank) / size * height) - 1;
			j++;
			if (j == width) {
				break;
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	yy = (int) round(length / step);

	/* daca procesul este diferit de master 
	 * trimit portiunea de imagine prelucrata
	 * catre master
	 */	
	if (rank != 0) {
		xx = (int) round ((float) (size - rank - 1) / size * height);
		MPI_Send(image[xx], width * yy, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}

	/* daca procesul este master atunci asteapta
	 * sa primeasca portiunile de imagine 
	 * prelucrate de celalalte procese
	 */
	if (rank == 0) {
		for (k = 1; k < size; k++) {
			xx = (int) round ((float) (size - k - 1) / size * height);
			MPI_Recv(image[xx], width * yy, MPI_INT, k, 1, MPI_COMM_WORLD, &status);
		}
	} 

	MPI_Barrier(MPI_COMM_WORLD);
	
	/* Masterul printeaza in fisier */
	if (rank == 0) {					
		FILE *fw = fopen(argv[2], "w");
		fprintf(fw, "P2\n");
		fprintf(fw, "%d %d\n", width, height);
		fprintf(fw, "%d\n", NUM_COLORS - 1);
		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				fprintf(fw, "%d ", image[i][j]);
			}
			fprintf(fw, "\n");
		}
		fclose(fw);
	}
	
	free(image);
	
	MPI_Finalize();
	return 0;    
}
