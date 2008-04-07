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

	printf("Skip: %d\n",nio->byteSkip);
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

void chunk_nrrd_by_ram_limit(char * filename, Nrrd * nin, NrrdIoState * nio, int ram_limit)
{
	int element_size_in_nrrd = (int)nrrdElementSize(nin);
	int size_in_ram = (int)nrrdElementNumber(nin)*ELEMENT_SIZE_IN_RAM;
	int chunks = (int)ceilf((float)size_in_ram / (float)ram_limit);
	int slice_size = nin->axis[0].size * nin->axis[1].size * ELEMENT_SIZE_IN_RAM;
	char buffer[MAXPATHLEN];

	printf("Splitting into %d chunks\n", chunks);
	for(int i = 0; i < chunks; i++) {
		memset((void*)buffer, 0, MAXPATHLEN);
		snprintf(buffer, MAXPATHLEN, "%s-%d.nhdr", filename, i);

		int bytes_this_chunk = ram_limit > size_in_ram ? size_in_ram : ram_limit;
		int slices_this_chunk = bytes_this_chunk / slice_size;

		nin->axis[2].size = slices_this_chunk;

		save_nrrd_header(buffer, nin, nio);
		
		nio->byteSkip += (bytes_this_chunk / ELEMENT_SIZE_IN_RAM) *
			element_size_in_nrrd;
		size_in_ram -= bytes_this_chunk;
	}
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
