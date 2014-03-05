#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include "mpi.h"

#define MIN(a, b) a < b ? a : b

typedef struct {
	int s, d;
	char message[100];
}msg;

int main(int argc, char **argv) {

	int rank, size;
	MPI_Status status, status1, status2;    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    
	int i, j, k; 
	int send_type, recv_type, ok = 1, parent = -1;
	int topologie[size][size], muchii_cicluri[size][size];
	int recv_top[size][size], d[size][size], next_hop[size]; 
	int node, neighbour, message_count = 0;    
	char buffer[100], *next;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			topologie[i][j] = 0;
			muchii_cicluri[i][j] = 0;
			d[i][j] = 0;
		}
		next_hop[i] = -1;
	}
	
	// Citire din fisier linia aferenta fiecarui nod din topologie
    FILE *f = fopen(argv[1], "r");
	while (!feof(f)) {	
		fgets(buffer, 100, f);
		next = strtok(buffer, " -");
		node = atoi(next);
		if (node == rank) {
			next = strtok(NULL, " -");
			while (next != NULL) {
				neighbour = atoi(next);
				topologie[node][neighbour] = 1;
				topologie[neighbour][node] = 1;
				next = strtok(NULL, " -");
			}
		}
	}
	fclose(f);
	
	
	if (rank == 0) {
		for (i = 0; i < size; i++) {
			send_type = 1;
			if (topologie[rank][i]) {
				MPI_Send (&send_type, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
				//printf("[%d] Trimit sonda catre: %d\n", rank, i);
				message_count++;
			}
		}
	}
	
	while (ok) {
		MPI_Recv (&recv_type, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int source = status.MPI_SOURCE;
		//printf("[%d] de la: %d\n", rank, source);
		// receptie mesaj de tip sonda
		if (recv_type == 1) {
			if (parent == -1) {
				parent = source;
				for (i = 0; i < size; i++) {
					send_type = 1;
					if (i != parent && topologie[rank][i]) {
						MPI_Send (&send_type, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
						//printf("[%d] Trimit sonda\n", rank);
						message_count++;
					}
				}
			} else {
				muchii_cicluri[rank][source] = 1;
				muchii_cicluri[source][rank] = 1;
				message_count--;
			}
		}
		
		if (recv_type == 2) {
			for (i = 0; i < size; i++) {
				MPI_Recv (&recv_top[i], size, MPI_INT, source, 1, MPI_COMM_WORLD, &status1);
				for (j = 0; j < size; j++) {
					topologie[i][j] = topologie[i][j] | recv_top[i][j];
				}
			}
			message_count--;
		}
	
		if (message_count == 0 && parent != -1) {
			send_type = 2;
			MPI_Send (&send_type, 1, MPI_INT, parent, 1, MPI_COMM_WORLD);
			for (i = 0; i < size; i++) {
				MPI_Send (&topologie[i], size, MPI_INT, parent, 1, MPI_COMM_WORLD);
			}
			break;
		}
		
		if (message_count == 0 && rank == 0) {
			break;
		}
		
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	
	// Am obtinut topologia in procesul cu rank 0	
	
	if (rank == 0) {
		// trimitere topologie la vecinii lui 0
		for (i = 0; i < size; i++) {
			if (topologie[rank][i]) {
				for (j = 0; j < size; j++) {
					MPI_Send (&topologie[j], size, MPI_INT, i, 1, MPI_COMM_WORLD);
				}
			}
		}
	} else {
		
		// receptionare topologie
		for (i = 0; i < size; i++) {
			MPI_Recv (&topologie[i], size, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status2);
		}
		
		// trimitere topologie mai departe la vecini cu exceptia celui de la care a fost receptionata
		for (i = 0; i < size; i++) {
			if (topologie[rank][i] && i != status2.MPI_SOURCE && muchii_cicluri [rank][i] != 1) {
				for (j = 0; j < size; j++) {
					MPI_Send (&topologie[j], size, MPI_INT, i, 1, MPI_COMM_WORLD);
				}
			}
		}
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	
	// calcul tabela de rutare
	
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			d[i][j] = topologie[i][j];
	
	for (k = 0; k < size; k++)
		for (i = 0; i < size; i++)
			for (j = 0; j < size; j++)
				if (i != j && d[i][k] && d[k][j])
					if (d[i][j] > d[i][k] + d[k][j] || d[i][j] == 0)
						d[i][j] = d[i][k] + d[k][j];

	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			if (topologie[rank][j] && d[rank][i] - 1 == d[j][i]) {
				next_hop[i] = j;
				break;
			}
    
    for (i = 0; i < size; i++) {
 		MPI_Barrier (MPI_COMM_WORLD);
 		if (i == rank) {
 			printf("Buncar %d\t", rank);
 			for (j = 0; j < size; j++) {
 				printf("%d ", next_hop[j]);
 			}
 			printf("\n");
 		}
    }
    
    MPI_Barrier (MPI_COMM_WORLD);
    if (rank == 0) printf("\n");
    MPI_Barrier (MPI_COMM_WORLD);  	
   	if (rank == 0) {
   		for (i = 0; i < size; i++) {
   			for (j = 0; j < size; j++) {
   				printf("%d ", topologie[i][j]);
   			}
   			printf("\n");
   		}
   		printf("\n\n");
   	}
    MPI_Barrier (MPI_COMM_WORLD);
    
    
    // citire fisier de mesaje
    
    FILE *g = fopen(argv[2], "r");
    char mesaje[100][100], buffer1[10];
    int msg_number;
    
    fscanf(g, "%d", &msg_number);
    msg s_msg[msg_number];
    
    for (i = 0; i < msg_number; i++) {
    	fscanf(g, "%d %s", &s_msg[i].s, buffer1);
    	if (strcmp(buffer1, "B") == 0) 
    		s_msg[i].d = -1;
    	else
    		s_msg[i].d = atoi(buffer1);
    	fgets(s_msg[i].message, 100, g);
    	s_msg[i].message[strlen(s_msg[i].message) - 1] = 0;
    }
    fclose(g);
    
    MPI_Barrier (MPI_COMM_WORLD);
    
	// trimitere broadcast inainte de a incepe comunicatia
		
	int b_msg[2], b_recv[2];
	b_msg[0] = rank;
	b_msg[1] = -2;
	MPI_Request req = MPI_REQUEST_NULL;
	
	for (i = 0; i < size; i++) {
		if (topologie[i][rank] && muchii_cicluri[i][rank] != 1) {
			MPI_Isend (&b_msg, 2, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
		}
	}

	for (i = 0; i < size - 1; i++) {
		MPI_Recv (&b_recv, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		printf("[B%d] Etapa 1 finalizata -Buncar %d- (sursa B%d) \n", rank, b_recv[0], status.MPI_SOURCE);
		for (j = 0; j < size; j++) {
			if (topologie[j][rank] && muchii_cicluri[j][rank] != 1 && j != status.MPI_SOURCE) {
				MPI_Isend (&b_recv, 2, MPI_INT, j, 1, MPI_COMM_WORLD, &req);
			}
		}
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	if (rank == 0) printf("\n\n");
	MPI_Barrier (MPI_COMM_WORLD);
	
	// incepere trimitere mesaje
	char buffer2[100];
	
	for (i = 0; i < msg_number; i++) {
		if (s_msg[i].s == rank) {
			if (s_msg[i].d != -1) {
			
				// trimitere mesaje normale
				sprintf(buffer2, "%d %d %s", s_msg[i].d, s_msg[i].s, s_msg[i].message);
				printf("[B%d] Trimit mesajul: \"%s \" catre B%d prin next hop: B%d\n", rank, s_msg[i].message, s_msg[i].d, next_hop[s_msg[i].d]);
				MPI_Isend (buffer2, 100, MPI_CHAR, next_hop[s_msg[i].d], 1, MPI_COMM_WORLD, &req);
				
			} else {
			
				// trimitere mesaje de tip broadcast
				sprintf(buffer2, "%d %d %s", s_msg[i].d, s_msg[i].s, s_msg[i].message);
				printf("[B%d] [BROADCAST] %s\n", rank, s_msg[i].message);
				for (j = 0; j < size; j++) {
					if (topologie[j][rank] && muchii_cicluri[j][rank] != 1) {
						MPI_Isend (buffer2, 100, MPI_CHAR, j, 1, MPI_COMM_WORLD, &req);
					}
				}
				
			}
		}
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	if (rank == 0) printf("\n");
	MPI_Barrier (MPI_COMM_WORLD);
	
	// trimitere mesaj broadcast pentru a semnaliza ca toti si-au trimis mesajele
	// aceste mesaje de broadcast vor avea in campul destinatie valoarea -2 pentru
	// a le putea diferentia de celalalte mesaje
	
	char broadcast[100];
	sprintf(broadcast, "-2 %d Broadcast", rank);
	for (i = 0; i < size; i++) {
		if (topologie[i][rank] && muchii_cicluri[i][rank] != 1) {
			MPI_Isend (broadcast, 100, MPI_CHAR, i, 1, MPI_COMM_WORLD, &req);
		}
	}
	
	// inceperea rutarii mesajelor pana la destinatie 
	
	int bcast_count = 0;
	int src, dst, space = 0;
	
	while (ok) {
		char buf[100] = "", buffer3[100] = "";
		MPI_Recv (buffer3, 100, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status2);
		
		// parsare destinatie, sursa si mesaj din stringul primit
		sscanf(buffer3, "%d%d", &dst, &src);
		space = 0;
		while (buffer3[space++] != ' ') {}
		while (buffer3[space++] != ' ') {}
		strcpy (buf, buffer3 + space);
		
		//printf("[Rutare] [B%d] Mesaj: \"%s \" \n", rank, buf);
		// tratare mesaje in functie de destinatie
		if (dst == -2) {
			// mesaje broadcast finalizare
			bcast_count++;
			for (k = 0; k < size; k++) {
				if (topologie[k][rank] && muchii_cicluri[k][rank] != 1 && k != src) {
					sprintf(buffer3, "%d %d %s", -2, rank, buf);
					MPI_Isend(buffer3, 100, MPI_CHAR, k, 1, MPI_COMM_WORLD, &req);
				}
			}
		} 
		else if (dst == -1) {
			// mesaje broadcast
			printf("[RUTARE] [B%d] [BROADCAST] %s\n", rank, buf);
			for (k = 0; k < size; k++) {
				if (topologie[k][rank] && muchii_cicluri[k][rank] != 1 && k != src) {
					sprintf(buffer3, "%d %d %s", -1, rank, buf);
					MPI_Isend(buffer3, 100, MPI_CHAR, k, 1, MPI_COMM_WORLD, &req);
				}
			}
		} 
		else if (dst == rank) {
			// mesajul a ajuns la destinatie
			printf("[B%d] Am primit mesajul: \"%s \" de la B%d\n", rank, buf, src);
		} 
		else {
			// ruteaza mai departe mesajul
			printf("[RUTARE] [B%d] Rutez mai departe mesajul primit de la B%d catre B%d prin next hop: B%d\n", rank, src, dst, next_hop[dst]);
			sprintf(buffer3, "%d %d %s", dst, rank, buf);
			MPI_Isend(buffer3, 100, MPI_CHAR, next_hop[dst], 1, MPI_COMM_WORLD, &req);
		}
			
		if (bcast_count == size - 1) {
			// conditia de iesire din while este sa fi primit broadcast de finalizare
			// de la toate celalalte buncare
			break;
		}
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	if (rank == 0) printf("\n");
	MPI_Barrier (MPI_COMM_WORLD);
	
	int nr_vecini = 0, wake_up_signal = -42, recv_signal;
	int first_signal = 0;
	
	for (i = 0; i < size; i++) {
		if (topologie[i][rank] && muchii_cicluri[i][rank] != 1)
			nr_vecini++;
	}
	
	
	// algoritm wake up
	
	if (nr_vecini == 1) {
		MPI_Isend(&wake_up_signal, 1, MPI_INT, parent, 1, MPI_COMM_WORLD, &req);
		MPI_Recv(&recv_signal, 1, MPI_INT, parent, 1, MPI_COMM_WORLD, &status);
	} else {
		for (i = 0; i < nr_vecini; i++) {
			MPI_Recv(&recv_signal, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
			if (first_signal == 0 && recv_signal == -42) {
				first_signal = 1;
				for (j = 0; j < size; j++) {
					if (topologie[j][rank] && muchii_cicluri[j][rank] != 1) {
						MPI_Isend(&wake_up_signal, 1, MPI_INT, j, 1, MPI_COMM_WORLD, &req);
					}
				}
			}
		}
	}
	
	MPI_Barrier (MPI_COMM_WORLD);
	
	// incepem algoritmul de alegere a liderului
	
	srand(time(NULL));
	int my_vote[2], recv_vecini[size], recv_vote[2], last_src;
	my_vote[0] = rand() % size;
	my_vote[1] = 1;
	
	for (i = 0; i < size; i++) {
		recv_vecini[i] = topologie[i][rank];
	}
	
	for (i = 0; i < nr_vecini - 1; i++) {
		MPI_Recv (&recv_vote[0], 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		recv_vecini[status.MPI_SOURCE] = 0;
		my_vote[0] = MIN(my_vote[0], recv_vote[0]);
		my_vote[1] += recv_vote[1];
	}
	
	for (i = 0; i < size; i++) {
		if (recv_vecini[i] == 1) {
			last_src = i;
			MPI_Isend (&my_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
			MPI_Recv (&recv_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
			break;
		}
	}
	my_vote[0] = MIN(recv_vote[0], my_vote[0]);
	
	for (i = 0; i < size; i++) {
		if (topologie[i][rank] && muchii_cicluri[i][rank] != 1 && i != last_src) {
			MPI_Isend (&my_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
		}
	}

	int leader = my_vote[0];
	
	// s-a terminat alegerea liderului
	
	MPI_Barrier (MPI_COMM_WORLD);
	
	// incepem alegerea adjunctului
	
	srand(time(NULL));

	my_vote[1] = 0;
	my_vote[0] = rand() % size;
	
	while (my_vote[0] == leader) {
		srand(time(NULL));
		my_vote[0] = rand() % size;
	}
	
	for (i = 0; i < size; i++) {
		recv_vecini[i] = topologie[i][rank];
	}
	
	for (i = 0; i < nr_vecini - 1; i++) {
		MPI_Recv (&recv_vote[0], 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		recv_vecini[status.MPI_SOURCE] = 0;
		my_vote[0] = MIN(my_vote[0], recv_vote[0]);
		my_vote[1] += recv_vote[1];
	}
	
	for (i = 0; i < size; i++) {
		if (recv_vecini[i] == 1) {
			last_src = i;
			MPI_Isend (&my_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
			MPI_Recv (&recv_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
			break;
		}
	}
	my_vote[0] = MIN(recv_vote[0], my_vote[0]);
	
	for (i = 0; i < size; i++) {
		if (topologie[i][rank] && muchii_cicluri[i][rank] != 1 && i != last_src) {
			MPI_Isend (&my_vote[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
		}
	}
	
	int second = my_vote[0];
	
	// s-a terminat alegerea adjunctului
	
	MPI_Barrier (MPI_COMM_WORLD);
	
	printf("[B%d] ID lider: %d -- ID adjunct: %d\n", rank, leader, second);
	
	MPI_Finalize();
	return 0;
}
