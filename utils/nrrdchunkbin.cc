#include <teem/nrrd.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>

#define ELEMENT_SIZE_IN_RAM 4
#define NUMBER_BASE 10
#define MAXPATHLEN PATH_MAX

Nrrd * load_nrrd_header(char * filename, NrrdIoState * nio)
{
	char *err;
	Nrrd *nin;

	// create a new nrrd
	nin = nrrdNew();

	// tell nrrdLoad to only read the header
	nrrdIoStateSet(nio, nrrdIoStateDetachedHeader, AIR_TRUE);
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

	if(nrrdLoad(nin, filename, nio)) {
		err = biffGetDone(NRRD);
		fprintf(stderr, "Trouble reading \"%s\" header:\n%s",
				filename, err);
		free(err); nio = nrrdIoStateNix(nio);
		return NULL;
	}

	return nin;
}

void save_nrrd_header(char * filename, Nrrd * nout, NrrdIoState * nio)
{
	FILE *fileOut;
	char *err;
	airArray *mop = airMopNew();

	nio->detachedHeader = AIR_TRUE;
	nio->skipData = AIR_TRUE;

	printf("Data file: %s\n", nio->dataFN[0]);
	if(!(fileOut = airFopen(filename, stdout, "wb"))) {
		fprintf(stderr, "Couldn't fopen(\"%s\",\"wb\"): %s\n",
				filename, strerror(errno));
		airMopError(mop); exit(1);
	}
	airMopAdd(mop, fileOut, (airMopper)airFclose, airMopAlways);

	if(nrrdFormatNRRD->write(fileOut, nout, nio)) {
		err = biffGetDone(NRRD);
		fprintf(stderr, "Trouble writing \"%s\" header:\n%s",
				filename, err);
		free(err); nio = nrrdIoStateNix(nio);
	}

	airMopOkay(mop);
}

void chunk_nrrd_by_ram_limit(char * filename, Nrrd * nin, NrrdIoState * nio, long ram_limit)
{
	int element_size_in_nrrd = (int)nrrdElementSize(nin);
	long size_in_ram = (long)nrrdElementNumber(nin)*(long)ELEMENT_SIZE_IN_RAM;
	int chunks = (int)ceilf((float)size_in_ram / (float)ram_limit);
	long slice_size = nin->axis[0].size * nin->axis[1].size * ELEMENT_SIZE_IN_RAM;
	char nhdrbuffer[MAXPATHLEN], nrrdbuffer[MAXPATHLEN];
	char *origDataFN = nio->dataFN[0];

	FILE *nrrdDataIn;

	memset((void*)nrrdbuffer, 0, MAXPATHLEN);
	snprintf(nrrdbuffer, MAXPATHLEN, "%s%s", nio->path, (nio->dataFN[0])+1);
	printf("Opening %s as data\n", nrrdbuffer);
	if((nrrdDataIn = fopen(nrrdbuffer,"r")) == NULL) {
		perror("Unable to open NRRD data file for reading");
		exit(1);
	}
	
	// seek the initial BS then reset it
	if(fseek(nrrdDataIn, nio->byteSkip, SEEK_SET) == -1) {
		perror("Unable to seek NRRD data file past header");
		exit(1);
	}
	nio->byteSkip = 0;
	
	printf("Splitting into %d chunks\n", chunks);
	for(int i = 0; i < chunks; i++) {
		memset((void*)nhdrbuffer, 0, MAXPATHLEN);
		memset((void*)nrrdbuffer, 0, MAXPATHLEN);
		snprintf(nhdrbuffer, MAXPATHLEN, "%s-%d.nhdr", filename, i);
		snprintf(nrrdbuffer, MAXPATHLEN, "%s-%d.raw", filename, i);

		long bytes_this_chunk = ram_limit > size_in_ram ? size_in_ram : ram_limit;
		int slices_this_chunk = (int)floor((double)bytes_this_chunk / slice_size);
		bytes_this_chunk = slices_this_chunk * slice_size;

		nin->axis[2].size = slices_this_chunk;
		nio->dataFN[0] = nrrdbuffer;
		
		save_nrrd_header(nhdrbuffer, nin, nio);
		
		// Write out the chunk
		char *databuffer;
		long bytes_this_chunk_in_input = bytes_this_chunk; // (bytes_this_chunk / (long)ELEMENT_SIZE_IN_RAM) * (long)element_size_in_nrrd;
	 	if((databuffer = (char*)malloc(bytes_this_chunk_in_input*sizeof(char))) == NULL) {
			perror("Couldn't allocate bytes_this_chunk of memory "
					"(your ram_limit is probably too high for this machine anyway)");
			exit(1);
		}

		FILE *nrrdDataOut;
		if((nrrdDataOut = fopen(nrrdbuffer,"w")) == NULL) {
			perror("Unable to open NRRD data file for writing");
			exit(1);
		}

		printf("Reading and writing %ld bytes\n", bytes_this_chunk_in_input);

		fread(databuffer, sizeof(char), bytes_this_chunk_in_input, nrrdDataIn);
		fwrite(databuffer, sizeof(char), bytes_this_chunk_in_input, nrrdDataOut);

		fclose(nrrdDataOut);
		free(databuffer);

		// fseek(nrrdDataIn, (bytes_this_chunk / (long)ELEMENT_SIZE_IN_RAM) *
		//	(long)element_size_in_nrrd, SEEK_CUR);
		size_in_ram -= bytes_this_chunk;
	}

	fclose(nrrdDataIn);
	nio->dataFN[0] = origDataFN;
}

int main(int argc, char * argv[])
{
	NrrdIoState * nio = nrrdIoStateNew();
	Nrrd * nin;
	int ram_limit = 0;

	if(argc !=	4) {
		fprintf(stderr, "Usage: %s input_nhdr output_nhdr ram_limit\n", argv[0]);
		exit(1);
	}

	if((nin = load_nrrd_header(argv[1], nio)) == NULL) {
		fprintf(stderr, "Could not load nrrd header\n");
		exit(1);
	}

	ram_limit = strtoimax(argv[3],
			NULL, NUMBER_BASE);

	chunk_nrrd_by_ram_limit(argv[2], nin, nio, ram_limit);

	nrrdNuke(nin);
	nrrdIoStateNix(nio);

	return 0;
}
