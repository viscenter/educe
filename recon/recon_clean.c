#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <unistd.h>

#include "mpi.h"

// magic numbers
#define SLICES_PER_SINOGRAM 10
#define NUMBER_BASE 10
#define MAXPATHLEN PATH_MAX
#define BUFFERLEN 255
#define SINOGRAM_ID "DSAA"
#define MASTER_NODE_ID 0
// TODO: fix sinogram tail seek so it doesn't need a magic size?
#define SINOGRAM_TAIL_SIZE 88
#define FAN_BEAM_ID "FAN_BEAM_PARAMETERS"

struct sinogram {
	int width;
	int number_of_steps;
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

struct global_parameters {
	float start_COR; // starting (currently only) center of rotation
	float min_intensity_possible; // minimum intensity possible...maybe pointless?
	float maximum_angle; // maximum angle of sinograms
	float xmax; // what is this, actually?

	float source_to_detector_dist; // distance from x-ray to detector
	float source_to_sample_dist; // distance from x-ray to sample (where?)

	int sinogram_width; // width of the sinogram sensor
	int number_of_steps; // number of angles / projections in the sinograms
	int image_size; // width and height of the reconstructed image
	int max_slices_per_node;
};

void init_program_arguments(struct reconstruction_arguments * program_arguments, int argc, char *argv[], int mpi_id);
void print_reconstruction_arguments(struct reconstruction_arguments * args, int mpi_id);
void check_failure(int failure_state);
int get_int_from_arg(const char * src, const char * varname, int mpi_id);
void get_path_from_arg(char * dst, const char * src, const char * varname, int mpi_id);
struct sinogram * parse_sinogram_file_header(const char * filename, int mpi_id);
void print_sinogram_info(struct sinogram * sgram, int mpi_id);
void test_output_filename(const char * filename, int mpi_id);
void run_reconstruction(struct reconstruction_arguments * args, struct sinogram * sgram, int mpi_id);
void run_reconstruction_master(struct reconstruction_arguments * args, struct sinogram * sgram);
void run_reconstruction_slave(void);
void compute_global_parameters(struct global_parameters * global_params, struct reconstruction_arguments * args, struct sinogram * sgram);
void bcast_global_parameters(struct global_parameters * global_params);

void fan_beam_convolution_recon(float * sino_slices, float * voxels, struct global_parameters * global_params, int num_slices);

void clear_float_array(float *array, size_t size, off_t offset);

int main( int argc, char *argv[] )
{
	// MPI state information
	int myid;

	struct reconstruction_arguments program_arguments;

	// init the MPI state
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	init_program_arguments(&program_arguments, argc, argv, myid);
	print_reconstruction_arguments(&program_arguments, myid);

	struct sinogram * first_sinogram = parse_sinogram_file_header(program_arguments.sinogram_start_filename, myid);
	print_sinogram_info(first_sinogram, myid);

	test_output_filename(program_arguments.output_vol_filename, myid);
	
	run_reconstruction(&program_arguments, first_sinogram, myid);

	// exit cleanly
	if(myid == MASTER_NODE_ID) {
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
	if(mpi_id == MASTER_NODE_ID) {
		if(argc != 7) {
			fprintf(stderr,
					"Usage: %s slicechunk sinogram number output imagesize cor\n",
					argv[0]);
			failure_state = 1;
		}
	}
	check_failure(failure_state);

	// initialize argument pointers
	if(mpi_id == MASTER_NODE_ID) {
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
	if((mpi_id == MASTER_NODE_ID) && ((program_arguments->max_slices_per_node % SLICES_PER_SINOGRAM) != 0)) {
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
	if(mpi_id == MASTER_NODE_ID) {
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

	if(mpi_id == MASTER_NODE_ID) {
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

	if(mpi_id == MASTER_NODE_ID) {
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

	if(mpi_id == MASTER_NODE_ID) {
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
								&parsed_sinogram->number_of_steps) != 2) {
						fprintf(stderr,
								"Unable to parse sinogram width and number_of_steps (2nd line)\n");
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
	if(mpi_id == MASTER_NODE_ID) {
		printf("**** Sinogram Parameters ****\n");
		printf("width: %d\n", sgram->width);
		printf("number_of_steps: %d\n", sgram->number_of_steps);
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

void test_output_filename(const char * filename, int mpi_id)
{
	int failure_state = 0;
	FILE * fp;

	if(mpi_id == MASTER_NODE_ID) {
		if(access(filename, F_OK) != -1) {
			fprintf(stderr,
					"Output filename \"%s\" already exists, exiting without clobbering\n",
					filename);
			failure_state = 1;
		}
		else if((fp = fopen(filename, "w")) == NULL) {
			fprintf(stderr, "Unable to open output vol file "
				 "with filename \"%s\" for writing: %s\n", filename,
				 strerror(errno));
			failure_state = 1;
		}
		else {
			fclose(fp);
		}
	}
	check_failure(failure_state);
}

void run_reconstruction(struct reconstruction_arguments * args, struct sinogram * sgram, int mpi_id)
{
	if(mpi_id == MASTER_NODE_ID) {
		run_reconstruction_master(args, sgram);
	}
	else {
		run_reconstruction_slave();
	}
}

void run_reconstruction_master(struct reconstruction_arguments * args, struct sinogram * sgram)
{
	int num_nodes;
	int num_slices;
	int slices_per_node;

	struct global_parameters global_params;

	MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);

	num_slices = args->number_of_sinograms * SLICES_PER_SINOGRAM;

	// compute global reconstruction parameters and send to all slaves
	compute_global_parameters(&global_params, args, sgram);
	bcast_global_parameters(&global_params);

	// tell all slaves we're done
	slices_per_node = -1;	
	for(int i = 1; i < num_nodes; i++) {
		MPI_Send(&slices_per_node, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD);
	}
}

void run_reconstruction_slave(void)
{
	int slices_this_chunk;

	struct global_parameters global_params;

	float *chunk_of_sino_slices = NULL;
	float *chunk_of_voxels = NULL;
	MPI_Status status;

	// get global parameters from master
	bcast_global_parameters(&global_params);

	// malloc buffers for slices once
	int sinogram_size_per_slice = global_params.sinogram_width * global_params.number_of_steps;
	chunk_of_sino_slices = (float *)malloc(sinogram_size_per_slice *
			global_params.max_slices_per_node * sizeof(float));
	chunk_of_voxels = (float *)malloc(global_params.image_size * global_params.image_size *
			global_params.max_slices_per_node * sizeof(float));

	while(1) { // loop until sinogram processing is done
		MPI_Recv(&slices_this_chunk, 1, MPI_INT, MASTER_NODE_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		if(slices_this_chunk == -1) { // processing done
			break;
		}

		// get sinos for processing from master
		MPI_Recv(chunk_of_sino_slices, sinogram_size_per_slice * slices_this_chunk,
				MPI_FLOAT, MASTER_NODE_ID, MPI_ANY_TAG,
				MPI_COMM_WORLD, &status);

		clear_float_array(chunk_of_voxels, global_params.image_size * global_params.image_size *
				slices_this_chunk, 0);
		fan_beam_convolution_recon(chunk_of_sino_slices, chunk_of_voxels, &global_params, slices_this_chunk);

		// tell the master how many slices we got and send back the recon'd voxel set
		MPI_Send(&slices_this_chunk, 1, MPI_INT, MASTER_NODE_ID, MPI_ANY_TAG, MPI_COMM_WORLD);
		MPI_Send(chunk_of_voxels, global_params.image_size * global_params.image_size *
				slices_this_chunk, MPI_FLOAT, MASTER_NODE_ID, MPI_ANY_TAG, MPI_COMM_WORLD);
	} // end sinogram processing

	free(chunk_of_sino_slices);
	free(chunk_of_voxels);
}

// initialize global parameters, should only be called by master
void compute_global_parameters(struct global_parameters * global_params, struct reconstruction_arguments * args, struct sinogram * sgram)
{
	global_params->image_size = args->image_size;
	global_params->start_COR = args->center_of_rotation;
	global_params->max_slices_per_node = args->max_slices_per_node;

	global_params->min_intensity_possible = 1.0/sgram->zmax;
	global_params->sinogram_width = sgram->width;
	global_params->number_of_steps = sgram->number_of_steps;
	global_params->maximum_angle = sgram->ymax; // RFB: use ymin as min angle?
	global_params->xmax = sgram->xmax;
	global_params->source_to_detector_dist = sgram->source_to_detector_dist;
	global_params->source_to_sample_dist = sgram->source_to_sample_dist;
}

void bcast_global_parameters(struct global_parameters * global_params)
{
	MPI_Bcast(&(global_params->image_size), 1, MPI_INT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->start_COR), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->max_slices_per_node), 1, MPI_INT, MASTER_NODE_ID, MPI_COMM_WORLD);

	MPI_Bcast(&(global_params->min_intensity_possible), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->sinogram_width), 1, MPI_INT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->number_of_steps), 1, MPI_INT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->maximum_angle), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->xmax), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->source_to_detector_dist), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
	MPI_Bcast(&(global_params->source_to_sample_dist), 1, MPI_FLOAT, MASTER_NODE_ID, MPI_COMM_WORLD);
}

void fan_beam_convolution_recon(float * sino_slices, float * voxels, struct global_parameters * global_params, int num_slices)
{
	for(int step = 0; step < global_params->number_of_steps; step++) {
		// convolve each sinogram slice by two FFT's
		for(int slice = 0; slice < num_slices; slice++) {
			// fill in RNA (source) array
			// convolve with SMT hamming filter (smoothing)
			// convolve with GNA ?? filter
		}

		// backprojection
		for(int i = 0; i < (global_parameters->image_size * global_parameters->image_size); i++) {
			// compute UU, NNN
		}

		for(int slice = 0; slice < num_slices; slice++) {
			for(int i = 0; i < (global_parameters->image_size * global_parameters->image_size); i++) {
				// if NNN is within the image
				// 	add FFTresult[NNN]*UU to the image
			}
		}
	}
}

void clear_float_array(float *array, size_t size, off_t offset)
{
#if defined(__STDC_IEC_559__)
	memset(array+offset, 0, size * sizeof(float));
#else
	for(size_t i = 0; i < size; i++) {
		(array+offset)[i] = 0.0;
	}
#endif // __STDC_IEC_559__	
}
