#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define N 1000
#define MASTER 0

// Tinand cont ca toate ex au fost facute sa mearga pe cazuri particulare,
// implementarea mea e pentru cazuri particulare cand 1000 se imparte perfect
// la nProcesses - 1	

void compareVectors(int * a, int * b) {
	// DO NOT MODIFY
	int i;
	for(i = 0; i < N; i++) {
		if(a[i]!=b[i]) {
			printf("Sorted incorrectly\n");
			return;
		}
	}
	printf("Sorted correctly\n");
}

void displayVector(int * v) {
	// DO NOT MODIFY
	int i;
	int displayWidth = 2;
	for(i = 0; i < N; i++) {
		printf("%i ", v[i]);
	}
	printf("\n");
}

int cmp(const void *a, const void *b) {
	// DO NOT MODIFY
	int A = *(int*)a;
	int B = *(int*)b;
	return A-B;
}
 
int main(int argc, char * argv[]) {
	int rank, i, j;
	int nProcesses;
	MPI_Init(&argc, &argv);
	int pos[N], recv_pos[N];
	int sorted = 0;
	int *v = (int*)malloc(sizeof(int)*N);
	int *vQSort = (int*)malloc(sizeof(int)*N);

	for (i = 0; i < N; i++)
		pos[i] = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
	printf("Hello from %i/%i\n", rank, nProcesses);

    if (rank == MASTER) {
        // generate random vector
		srandom(42);
		for(i = 0; i < N; i++)
			v[i] = random() % 200;
    }

    // send the vector to all processes
	MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);

	if(rank == 0) {
		// DO NOT MODIFY
		displayVector(v);

		// make copy to check it against qsort
		// DO NOT MODIFY
		for(i = 0; i < N; i++)
			vQSort[i] = v[i];
		qsort(vQSort, N, sizeof(int), cmp);

		// sort the vector v
		int dim_proc = (int)ceil((double)N / nProcesses);
		for (i = 0; i < dim_proc; i++) {
			for (j = 0; j < N; j++) {
				if (v[rank * dim_proc + i] > v[j] || (j < (rank * dim_proc + i) && v[rank * dim_proc + i] == v[j])) {
					pos[rank * dim_proc + i]++;
				}
			}
		}

        // recv the new pozitions
		MPI_Gather(pos + rank * dim_proc, dim_proc, MPI_INT, recv_pos, dim_proc, MPI_INT, 0, MPI_COMM_WORLD);

		int aux[N];
		for (i = 0; i < N; i++) {
			aux[recv_pos[i]] = v[i];
		}

		displayVector(aux);
		compareVectors(aux, vQSort);
	} else {
        // compute the positions
		int dim_proc = (int)ceil((double)N / nProcesses);
		for (i = 0; i < dim_proc; i++) {
			for (j = 0; j < N; j++) {
				if (v[rank * dim_proc + i] > v[j] || (j < (rank * dim_proc + i) && v[rank * dim_proc + i] == v[j])) {
					pos[rank * dim_proc + i]++;
				}
			}
		}

        // send the new positions to process MASTER
		if (rank == nProcesses - 1) {
			MPI_Gather(pos + rank * dim_proc, N - rank * dim_proc, MPI_INT, recv_pos, dim_proc, MPI_INT, 0, MPI_COMM_WORLD);
		} else {
			MPI_Gather(pos + rank * dim_proc, dim_proc, MPI_INT, recv_pos, dim_proc, MPI_INT, 0, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
	return 0;
}