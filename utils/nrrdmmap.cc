#include <teem/nrrd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAXPATHLEN PATH_MAX

Nrrd * mmap_load(char *filename)
{
	char *err;
	NrrdIoState *nio;
	Nrrd *nin;
	void *data = NULL;
	int fdin;
	char buffer[MAXPATHLEN];
	struct stat statbuf;

	nin = nrrdNew();

	nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

	if(nrrdLoad(nin, filename, nio)) {
		err = biffGetDone(NRRD);
		fprintf(stderr, "Trouble reading \"%s\" header:\n%s",
				filename, err);
		free(err); nio = nrrdIoStateNix(nio);
		return NULL;
	}

	snprintf(buffer, MAXPATHLEN, "%s/%s", nio->path, nio->dataFN[0]);
	printf("mmapping %s\n", buffer);

	if((fdin = open(buffer, O_RDONLY)) < 0) {
		perror("Error opening fd");
		exit(1);
	}

	if(fstat(fdin,&statbuf) < 0) {
		perror("Error stat'ing file");
		exit(1);
	}	

	/*	
	printf("Skipping %d bytes\n", nio->byteSkip);
	if(lseek(fdin, nio->byteSkip, SEEK_SET) == -1) {
		perror("Error seeking");
		exit(1);
	}
	*/

	if((data = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
			== (caddr_t)-1) {
		perror("Error mmapping");
		exit(1);
	}

	nio = nrrdIoStateNix(nio);

	nin->data = (char*)data+(nio->byteSkip);

	/*
	if(nrrdLoad(nin, filename, NULL)) {
		err = biffGetDone(NRRD);
		fprintf(stderr, "Trouble reading \"%s\" header:\n%s",
				filename, err);
		free(err); nio = nrrdIoStateNix(nio);
		return NULL;
	}
	*/

	return nin;
}

int main(int argc, char *argv[])
{
	Nrrd *nin;
	Nrrd *nout;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s infile\n", argv[0]);
		exit(1);
	}

	nin = mmap_load(argv[1]);

	nout = nrrdNew();
	nrrdSlice(nout, nin, 2, 0);
	nrrdSave("test.nrrd", nout, NULL);

	nrrdNix(nin);
	nrrdNuke(nout);

	return 0;
}
