#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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
	
	int old_senat[n][n], new_senat[n][n];
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			fscanf(fr, "%d", &old_senat[i][j]); // citire culoarea fiecarui senator
		}
	}
	
	
	
	int dmin[k], number_sen[k], zero[k], dist;	
	for(r = 0; r < k; r++) {
		number_sen[r] = 0;
		zero[k] = 0;
	}
	
	for(w = 0; w < weeks; w++) {
	
		for(i = 0 ; i < n; i++) {
			for(j = 0; j < n; j++) {
			
				// setez distanta minima fata de fiecare culoare
				// cu o valoare destul de mare ce nu poate fi atinsa
				for(r = 0; r < k; r++) {
					dmin[r] = n;
					if(zero[r] == 1)
						dmin[r] = 0;
				}
		
				// verific fiecare senator (i,j) cu restul de senatori
				// pentru a determina distanta minima catre fiecare culoare
				for(i1 = 0 ; i1 < n; i1++) {
					for(j1 = 0; j1 < n; j1++) {
						if(i != i1 || j != j1){
							dist = MAX(abs(i-i1), abs(j-j1));
							dmin[old_senat[i1][j1]] = MIN(dmin[old_senat[i1][j1]], dist);
						}
					}
				}
			
				// determin maximul dintre distantele minime
				// si retin culoarea pentru care s-a gasit maximul
				max_col = -1;
				for(r = 0; r < k; r++) {
					if(max_col < MAX(max_col, dmin[r]) && MAX(max_col, dmin[r]) != n){
						max_col = MAX(max_col, dmin[r]);
						new_color = r;
					}
				}
			
				// noua culoare a senatorului (i,j) este culoarea
				// pentru care s-a gasit maximul
				new_senat[i][j] = new_color;
				number_sen[new_color]++;
			}
		}
		
		// copiere configuratie noua a senatorilor
		// dintr-o matrice in alta	
		for(i = 0 ; i < n; i++) {
			for(j = 0; j < n; j++) {
				old_senat[i][j] = new_senat[i][j];
			}
		}
		
		// printare in fisier numar de senatori din fiecare culoare
		// si resetarea numaratoarii senatorilor
		for(r = 0; r < k; r++) {
			fprintf(fw, "%d ", number_sen[r]);
			if(number_sen[r] == 0)
				zero[r] = 1;
			number_sen[r] = 0;
		}
		fprintf(fw, "\n");
	}
	
	// printare matrice senatori la sfarsitul algoritmului
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
