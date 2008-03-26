#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>

#include "mpi.h"

// magic numbers
#define SLICES_PER_SINOGRAM 10
#define NUMBER_BASE 10
#define MAXPATHLEN PATH_MAX

void check_failure(int failure_state);

int main( int argc, char *argv[] )
{
	// MPI state information
	int myid, numprocs;
	int failure_state = 0;

	// program arguments
	char * arg_max_slices_per_node = NULL,
			* arg_sinogram_start_filename = NULL,
			* arg_number_of_sinograms = NULL,
			* arg_output_vol_filename = NULL,
			* arg_image_size = NULL,
			* arg_center_of_rotation = NULL;

	int max_slices_per_node = 0;
	char sinogram_start_filename[MAXPATHLEN];

	// init the MPI state
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	// check for the correct number of command-line arguments
	if(myid == 0) {
		if(argc != 7) {
			fprintf(stderr,
					"Usage: %s slicechunk sinogram number output imagesize cor\n",
					argv[0]);
			failure_state = 1;
		}
	}
	check_failure(failure_state);

	// initialize argument pointers
	if(myid == 0) {
		arg_max_slices_per_node = argv[1];
		arg_sinogram_start_filename = argv[2];
		arg_number_of_sinograms = argv[3];
		arg_output_vol_filename = argv[4];
		arg_image_size = argv[5];
		arg_center_of_rotation = argv[6];
	}

	// get max_slices_per_node
	if(myid == 0) {
		max_slices_per_node = strtoimax(arg_max_slices_per_node,
				NULL, NUMBER_BASE);
		if(max_slices_per_node == 0) {
			perror("Error setting max_slices_per_node");
			failure_state = 1;
		}
		else if((max_slices_per_node % SLICES_PER_SINOGRAM) != 0) {
			fprintf(stderr, "The number of slices per node must"
					"be divisible by %d. %d is not valid\n",
					SLICES_PER_SINOGRAM, max_slices_per_node);
			failure_state = 1;
		}
	}
	check_failure(failure_state);

	// get sinogram_start_filename
	if(myid == 0) {
		if(strlcpy(sinogram_start_filename,
			arg_sinogram_start_filename,
			sizeof(sinogram_start_filename)) >=
				sizeof(sinogram_start_filename)) {
			fprintf(stderr, "Sinogram filename exceeds maximum path length\n");
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
