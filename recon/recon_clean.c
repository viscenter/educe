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
#define BUFFERLEN 255
#define SINOGRAM_ID "DSAA"
#define SINOGRAM_TAIL_SIZE 88
#define FAN_BEAM_ID "FAN_BEAM_PARAMETERS"

struct sinogram {
	int width;
	int number_of_angles;
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;

	float source_to_detector_dist;
	float source_to_sample_dist;
};

struct reconstruction_arguments {
	int max_slices_per_node,
			number_of_sinograms,
			image_size,
			center_of_rotation;
	char sinogram_start_filename[MAXPATHLEN],
			 output_vol_filename[MAXPATHLEN];
};

void init_program_arguments(struct reconstruction_arguments * program_arguments, int argc, char *argv[], int mpi_id);
void print_reconstruction_arguments(struct reconstruction_arguments * args, int mpi_id);
void check_failure(int failure_state);
int get_int_from_arg(const char * src, const char * varname, int mpi_id);
void get_path_from_arg(char * dst, const char * src, const char * varname, int mpi_id);
struct sinogram * parse_sinogram_file_header(const char * filename, int mpi_id);
void print_sinogram_info(struct sinogram * sgram, int mpi_id);

int main( int argc, char *argv[] )
{
	// MPI state information
	int myid, numprocs;

	struct reconstruction_arguments program_arguments;

	// init the MPI state
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	init_program_arguments(&program_arguments, argc, argv, myid);
	print_reconstruction_arguments(&program_arguments, myid);

	struct sinogram * first_sinogram = parse_sinogram_file_header(program_arguments.sinogram_start_filename, myid);
	print_sinogram_info(first_sinogram, myid);

	// exit cleanly
	if(myid == 0) {
		free(first_sinogram);
	}
	MPI_Finalize();

	return 0;
} // main

void init_program_arguments(struct reconstruction_arguments * program_arguments, int argc, char *argv[], int mpi_id)
{
	int failure_state = 0;
	
	// program arguments
	char * arg_max_slices_per_node = NULL,
			* arg_sinogram_start_filename = NULL,
			* arg_number_of_sinograms = NULL,
			* arg_output_vol_filename = NULL,
			* arg_image_size = NULL,
			* arg_center_of_rotation = NULL;

	// check for the correct number of command-line arguments
	if(mpi_id == 0) {
		if(argc != 7) {
			fprintf(stderr,
					"Usage: %s slicechunk sinogram number output imagesize cor\n",
					argv[0]);
			failure_state = 1;
		}
	}
	check_failure(failure_state);

	// initialize argument pointers
	if(mpi_id == 0) {
		arg_max_slices_per_node = argv[1];
		arg_sinogram_start_filename = argv[2];
		arg_number_of_sinograms = argv[3];
		arg_output_vol_filename = argv[4];
		arg_image_size = argv[5];
		arg_center_of_rotation = argv[6];
	}

	// get max_slices_per_node
	program_arguments->max_slices_per_node = get_int_from_arg(arg_max_slices_per_node,
			"max_slices_per_node", mpi_id);
	if((mpi_id == 0) && ((program_arguments->max_slices_per_node % SLICES_PER_SINOGRAM) != 0)) {
			fprintf(stderr, "The number of slices per node must"
					"be divisible by %d. %d is not valid\n",
					SLICES_PER_SINOGRAM, program_arguments->max_slices_per_node);
			failure_state = 1;
	}
	check_failure(failure_state);

	// get sinogram_start_filename
	get_path_from_arg(program_arguments->sinogram_start_filename, arg_sinogram_start_filename,
			"Sinogram file", mpi_id);

	// get number_of_sinograms
	program_arguments->number_of_sinograms = get_int_from_arg(arg_number_of_sinograms,
			"number_of_sinograms", mpi_id);

	// get output_vol_filename
	get_path_from_arg(program_arguments->output_vol_filename, arg_output_vol_filename,
			"Output filename", mpi_id);

	// get image_size
	program_arguments->image_size = get_int_from_arg(arg_image_size, "image_size", mpi_id);

	// get center_of_rotation
	program_arguments->center_of_rotation = get_int_from_arg(arg_center_of_rotation,
			"center_of_rotation", mpi_id);
} // init_program_arguments

void print_reconstruction_arguments(struct reconstruction_arguments * args, int mpi_id)
{
	if(mpi_id == 0) {
		printf("\n********* Recon Info *********\n");
		printf("max_slices_per_node: %d\n", args->max_slices_per_node);
		printf("sinogram_start_filename: %s\n", args->sinogram_start_filename);
		printf("number_of_sinograms: %d\n", args->number_of_sinograms);
		printf("output_vol_filename: %s\n", args->output_vol_filename);
		printf("image_size: %d\n", args->image_size);
		printf("center_of_rotation: %d\n", args->center_of_rotation);
		printf("\n");
	}
} // print_reconstruction_arguments

// broadcast the failure state and exit accordingly
void check_failure(int failure_state)
{
	MPI_Bcast(&failure_state, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if(failure_state) {
		MPI_Finalize();
		exit(1);
	}
} // check_failure

int get_int_from_arg(const char * src, const char * varname, int mpi_id)
{
	int failure_state = 0;
	int dst = 0;

	if(mpi_id == 0) {
		dst = strtoimax(src,
				NULL, NUMBER_BASE);
		if(dst == 0) {
			fprintf(stderr, "Error setting %s: %s\n", varname, strerror(errno));
			failure_state = 1;
		}
	}
	check_failure(failure_state);
	return dst;
} // get_int_from_arg

// get a path from an arg
// dst must be char dst[MAXPATHLEN]
void get_path_from_arg(char * dst, const char * src, const char * varname, int mpi_id)
{
	int failure_state = 0;

	if(mpi_id == 0) {
		if(strlcpy(dst,
			src,
			MAXPATHLEN) >=
				MAXPATHLEN) {
			fprintf(stderr, "%s exceeds maximum path length\n", varname);
			failure_state = 1;
		}
	}
	check_failure(failure_state);
} // get_path_from_arg

struct sinogram * parse_sinogram_file_header(const char * filename, int mpi_id)
{
	int failure_state = 0;
	char buffer[BUFFERLEN];
	struct sinogram * parsed_sinogram = NULL; 
	FILE * fp;

	if(mpi_id == 0) {
		if((fp = fopen(filename, "r")) == NULL) {
			fprintf(stderr, "Unable to open sinogram with filename \"%s\"\n", filename);
			failure_state = 1;
		}
		else { // sinogram open succeeded
			// read in SINOGRAM_ID
			fgets(buffer, strlen(SINOGRAM_ID)+1, fp);
			if(strncmp(buffer, SINOGRAM_ID, strlen(SINOGRAM_ID)) != 0) {
				fprintf(stderr, "Incorrect sinogram identifier: %s\n", buffer);
				failure_state = 1;
			}
			else { // valid sinogram id
				// alloc sinogram info
				if((parsed_sinogram = malloc(sizeof(struct sinogram))) == NULL) {
					perror("Unable to allocate sinogram info");
					failure_state = 1;
				}
				else { // parse rest of header
					// skip the newline after SINOGRAM_ID
					fseek(fp, 1, SEEK_CUR);

					if(fscanf(fp, "%d %d", 
								&parsed_sinogram->width,
								&parsed_sinogram->number_of_angles) != 2) {
						fprintf(stderr,
								"Unable to parse sinogram width and number_of_angles (2nd line)\n");
						failure_state = 1;
					}
					else if(fscanf(fp, "%f %f",
								&parsed_sinogram->xmin,
								&parsed_sinogram->xmax) != 2) {
						fprintf(stderr,
								"Unable to parse sinogram xmin and xmax (3rd line)\n");
						failure_state = 1;
					}
					else if(fscanf(fp, "%f %f",
								&parsed_sinogram->ymin,
								&parsed_sinogram->ymax) != 2) {
						fprintf(stderr,
								"Unable to parse sinogram ymin and ymax (4th line)\n");
						failure_state = 1;
					}
					else if(fscanf(fp, "%f %f",
								&parsed_sinogram->zmin,
								&parsed_sinogram->zmax) != 2) {
						fprintf(stderr,
								"Unable to parse sinogram zmin and zmax (3rd line)\n");
						failure_state = 1;
					}
					else { // parse info at the tail
						fseek(fp, -SINOGRAM_TAIL_SIZE, SEEK_END);
						fgets(buffer, strlen(FAN_BEAM_ID)+1, fp);
						if(strncmp(buffer, FAN_BEAM_ID, strlen(FAN_BEAM_ID)) != 0) {
							fprintf(stderr, "Incorrect fan beam identifier: %s\n", buffer);
							failure_state = 1;
						}
						else {
							// skip the newline after FAN_BEAM_ID
							fseek(fp, 1, SEEK_CUR);

							if(fscanf(fp, "%f %f",
										&parsed_sinogram->source_to_detector_dist,
										&parsed_sinogram->source_to_sample_dist) != 2) {
								fprintf(stderr, "Unable to parse fan beam parameters\n");
							}
						}
					}

					if(failure_state) {
						free(parsed_sinogram);
					}
				}
			}
			fclose(fp);
		}
	}
	check_failure(failure_state);

	return parsed_sinogram;
} // parse_sinogram_file_header

void print_sinogram_info(struct sinogram * sgram, int mpi_id) {
	if(mpi_id == 0) {
		printf("**** Sinogram Parameters ****\n");
		printf("width: %d\n", sgram->width);
		printf("number_of_angles: %d\n", sgram->number_of_angles);
		printf("xmin: %g\n", sgram->xmin);
		printf("xmax: %g\n", sgram->xmax);
		printf("ymin: %g\n", sgram->ymin);
		printf("ymax: %g\n", sgram->ymax);
		printf("zmin: %g\n", sgram->zmin);
		printf("zmax: %g\n", sgram->zmax);

		printf("source_to_detector_dist: %g\n", sgram->source_to_detector_dist);
		printf("source_to_sample_dist: %g\n", sgram->source_to_sample_dist);
	}
}
