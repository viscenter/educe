#include <stdio.h>
#include <stdlib.h>

#include "mpi.h"

void check_failure(int failure_state);

int main( int argc, char *argv[] )
{
	int myid, numprocs; // MPI state information
	int failure_state = 0;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	// check for the correct number of command-line arguments
	if(myid == 0) {
		if(argc != 7) {
			printf("Usage: %s slicechunk sinogram number output imagesize cor\n",argv[0]);
			failure_state = 1;
		}
	}
	check_failure(failure_state);

	return 0;
}

// broadcast the failure state and exit accordingly
void check_failure(int failure_state)
{
	MPI_Bcast(&failure_state, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if(failure_state) {
		MPI_Finalize();
		exit(1);
	}
}
