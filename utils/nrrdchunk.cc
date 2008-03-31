#include <teem/nrrd.h>
#include <errno.h>

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

	nrrdNuke(nout);
}

int main(int argc, char * argv[])
{
	NrrdIoState * nio = nrrdIoStateNew();
	Nrrd * nin;

	if(argc !=	3) {
		fprintf(stderr, "Usage: %s input_nhdr output_nhdr\n", argv[0]);
		exit(1);
	}

	if((nin = load_nrrd_header(argv[1], nio)) == NULL) {
		fprintf(stderr, "Could not load nrrd header\n");
		exit(1);
	}

	save_nrrd_header(argv[2], nin, nio);

	nrrdIoStateNix(nio);

	return 0;
}
