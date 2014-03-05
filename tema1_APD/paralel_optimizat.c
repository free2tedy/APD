#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<omp.h>


#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

int main(int argc, char *argv[]){
	
	
	int weeks = atoi(argv[1]);
	FILE *fr = fopen(argv[2], "r");
	FILE *fw = fopen(argv[3], "w");
	
	int n, k;
	int w, i, j, i1, j1, r;
	int new_color, max_col;
	
	fscanf(fr, "%d", &n); // citire dimensiune matrice patratica
	fscanf(fr, "%d", &k); // citire numar partide politice
	
	int rem_colors = k, ver_colors = 0;
	int dmin[k], number_sen[k], zero[k], v[k], dist;	
	for(r = 0; r < k; r++) {
		number_sen[r] = 0;
		zero[r] = 0;
	}
	
	int old_senat[n][n], new_senat[n][n];
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			fscanf(fr, "%d", &old_senat[i][j]); // citire culoarea fiecarui senator
			number_sen[old_senat[i][j]]++;
		}
	}
	
	for(r = 0; r < k; r++) {
		if(number_sen[r] == 0)
			zero[r] = 1;
		else if(number_sen[r] == 1)
			zero[r] = 2;
		else
			zero[r] = 0;
		number_sen[r] = 0;
	}
	

	for(w = 0; w < weeks; w++) {
		#pragma omp parallel for \
		shared(k, n, old_senat, new_senat, number_sen, zero) \
		private(i, j, rem_colors, ver_colors, dist, r, dmin, v, i1, j1, new_color, max_col) \
		schedule(dynamic, 2)
		for(i = 0 ; i < n; i++) {		
			for(j = 0; j < n; j++) {
				
				rem_colors = k;
				ver_colors = 0;
				dist = 1;
		
				for(r = 0; r < k; r++) {
					dmin[r] = n;
					v[r] = 0;
					if(zero[r] == 1){
						rem_colors--;
						dmin[r] = 0;
						v[r] = 1;
					}	
				}
				if(zero[old_senat[i][j]] == 2){
					dmin[old_senat[i][j]] = 0;
					v[old_senat[i][j]] = 1;
					ver_colors++;
				}
				
				while(ver_colors < rem_colors - 1){
					for(i1 = MAX(0, i - dist); i1 <= MIN(i + dist, n - 1); i1++){
						if(j - dist >= 0){
							if(v[old_senat[i1][j - dist]] == 0){
								dmin[old_senat[i1][j - dist]] = dist;
								v[old_senat[i1][j - dist]]	= 1;
								ver_colors++;				
							}	
						}
						if(j + dist < n){
							if(v[old_senat[i1][j + dist]] == 0){
								dmin[old_senat[i1][j + dist]] = dist;	
								v[old_senat[i1][j + dist]]	= 1;
								ver_colors++;						
							}
						}
					}
							
					for(j1 = MAX(j - dist + 1, 0); j1 < MIN(j + dist, n); j1++){
						if(i - dist >= 0){
							if(v[old_senat[i - dist][j1]] == 0){
								dmin[old_senat[i - dist][j1]] = dist;
								v[old_senat[i - dist][j1]]	= 1;
								ver_colors++;				
							}
						}
						if(i + dist < n){
							if(v[old_senat[i + dist][j1]] == 0){
								dmin[old_senat[i + dist][j1]] = dist;	
								v[old_senat[i + dist][j1]]	= 1;
								ver_colors++;						
							}
						}
					}
					dist++;
				}
				if(ver_colors == rem_colors - 1){
					for(r = 0; r < k; r++){
						if(v[r] == 0)
							new_color = r;
					}
				} else {
					max_col = -1;
					for(r = 0; r < k; r++) {
						if(max_col < MAX(max_col, dmin[r]) && MAX(max_col, dmin[r]) != n){
							max_col = MAX(max_col, dmin[r]);
							new_color = r;
						}
					}	
				}

				new_senat[i][j] = new_color;
				#pragma omp atomic
				number_sen[new_color]++;
							
			}
		}
	
		for(i = 0 ; i < n; i++) {
			for(j = 0; j < n; j++) {
				old_senat[i][j] = new_senat[i][j];
			}
		}
		
	
		for(r = 0; r < k; r++) {
			fprintf(fw, "%d ", number_sen[r]);
			zero[r] = 0;
			if(number_sen[r] == 0)
				zero[r] = 1;
			else if(number_sen[r] == 1)
				zero[r] = 2;
			number_sen[r] = 0;
		}
		fprintf(fw, "\n");
	}


	for(i = 0; i < n; i++) {
		for(j = 0; j < n; j++) {
			fprintf(fw, "%d ", new_senat[i][j]);
		}
		fprintf(fw, "\n");
	}


	fclose(fr);
	fclose(fw);

	
	return 0;
}
