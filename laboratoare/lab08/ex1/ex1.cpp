#include "mpi.h"
#include <iostream>
#include <time.h>
// #include <stdio.h>
// #include <stdlib.h>

using namespace std;

int main (int argc, char *argv[])
{
    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int recv_num;
    // First process starts the circle.
    if (rank == 0) {
    	srand(time(NULL));
    	int random_num = rand();
        // First process starts the circle.
        // Generate a random number.
        // Send the number to the next process.
		MPI_Status status;

        MPI_Send(&random_num, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv_num, 1, MPI_INT, numtasks - 1, 0, MPI_COMM_WORLD, &status);
                printf("Process with rank %d, received %d.\n",
                rank, recv_num);


    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the number from the previous process.
        // Increments the number.
        // Sends the number to the first process.
        MPI_Status status;
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
		printf("Process with rank %d, received %d with tag %d.\n",
                rank, recv_num, status.MPI_TAG);
		recv_num++;
        MPI_Send(&recv_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    } else {
        // Middle process.
        // Receives the number from the previous process.
        // Increments the number.
        // Sends the number to the next process.
        MPI_Status status;
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        printf("Process with rank %d, received %d with tag %d.\n",
                rank, recv_num, status.MPI_TAG);
 		recv_num++;
		MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

}

