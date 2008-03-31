#include <teem/nrrd.h>
#include <math.h>

int main(int argc, char * argv[])
{
	Nrrd *nin = nrrdNew();
	char *biffErr;

	if(argc < 4) {
		fprintf(stderr,"Usage: %s radius nrrd nrrdout\n",argv[0]);
		return 1;
	}

	int max_radius = atoi(argv[1]);

	if(nrrdLoad(nin, argv[2], NULL)) {
		biffErr = biffGetDone(NRRD);
		fprintf(stderr, "Error reading file: %s\n", biffErr);
		free(biffErr);
		return 1;
	}

	printf("Read %d-dimensional nrrd of type %d (%s)\n",
			nin->dim, nin->type, airEnumStr(nrrdType, nin->type));

	printf("Axis dimensions: %d x %d x %d\n", (int)nin->axis[0].size,
			(int)nin->axis[1].size, (int)nin->axis[2].size);

	NrrdRange *range = nrrdRangeNewSet(nin, 0);
	printf("min: %g\tmax: %g\n", range->min, range->max);

	int width = nin->axis[0].size,
			height = nin->axis[1].size,
			slices = nin->axis[2].size;

	int cx = width/2;
	int cy = height/2;
	
	int index = 0;
	float (*ins)(void *, size_t I, float v) = nrrdFInsert[nin->type];
	for(int slice = 0; slice < slices; slice++) {
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				if(((int)sqrtf(powf((float)(x-cx),2.0)+powf((float)(y-cy),2.0))) > max_radius) {
					ins(nin->data,index,0.0);
				}
				index++;
			}
		}
	}

	Nrrd *ncrop = nrrdNew();

	size_t min[3], max[3];

	min[0] = cx-max_radius;
	min[1] = cy-max_radius;
	min[2] = 0;
	
	max[0] = cx+max_radius;
	max[1] = cy+max_radius;
	max[2] = slices-1;

	nrrdCrop(ncrop,nin,min,max);

	nrrdSave(argv[3], ncrop, NULL);	

	nrrdNuke(nin);
	nrrdNuke(ncrop);

	return 0;
}
