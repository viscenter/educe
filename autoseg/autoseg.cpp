#include <teem/nrrd.h>

#include "cv.h"
#include "highgui.h"

#define PI 3.14159
#define TORAD 1.0/PI

#define RADIAL_SAMPLES 360

#define SOBEL_THRESH 100
#define MAX_LAYERS 2

int cx, cy;

CvPoint calc_ray_outer(float theta, CvPoint center)
{
	CvPoint result;
	float hyp = (float)center.x;
	float dx = hyp * cos(theta);
	float dy = hyp * sin(theta);

	result.x = center.x - dx;
	result.y = center.y - dy;

	return result;
}

void radial_sample(int width, int height, char* data, IplImage *unwrapped, int slice)
{
	IplImage *cvcast = cvCreateImageHeader(cvSize(width, height),
			IPL_DEPTH_8U, 1);
	cvcast->imageData = data;

	// cvSaveImage("slice.png",cvcast);
	
	CvPoint center = cvPoint(cx,cy);

	unsigned char* linebuf;
	for(int sample = 0; sample < RADIAL_SAMPLES; sample++) {
		float theta = ((float)sample)*((2.0*PI)/(float)RADIAL_SAMPLES);
		CvPoint outer = calc_ray_outer(theta, center);

		// printf("%g:\t%d,%d\n", theta*(180.0/PI), outer.x, outer.y);
		cvClipLine(cvSize(width, height), &outer, &center);
		int linesize = abs(center.x-outer.x)+abs(center.y-outer.y)+1;
		linebuf = (unsigned char*)malloc(linesize);
		cvSampleLine(cvcast,outer,center,linebuf,4);
		
		IplImage *castline = cvCreateImageHeader(cvSize(linesize,1), IPL_DEPTH_8U, 1);
		castline->imageData = (char*)linebuf;

		IplImage *sobel = cvCreateImage(cvSize(linesize,1), IPL_DEPTH_8U, 1);

		cvSobel(castline, sobel, 1, 0, 3);

		int layer = 0;
		for(int i = 0; (i < linesize) && (layer < MAX_LAYERS); i++) {
			// printf(" %d,", (int)cvGetReal1D(sobel,i));
			if((int)cvGetReal1D(sobel,i) > SOBEL_THRESH) {
				int max = 0, max_i = 0;
				for(; i < linesize; i++) {
					int curval = (int)cvGetReal1D(sobel,i);
					if(curval == 0) break;
					if(curval > max) {
						max = curval;
						max_i = i;
					}
				}
				cvSetReal2D(unwrapped,slice,(layer*RADIAL_SAMPLES)+sample,cvGetReal1D(castline,max_i));
				// printf("%d\t",max);
				layer++;
			}
		}
		// printf("\n");
	
		/*	
		char filename[] = "line000.png";
		sprintf(filename,"line%03d.png",(int)(theta*(180.0/PI)));
		cvSaveImage(filename,sobel);
		*/
		
		cvReleaseImageHeader(&castline);
		cvReleaseImage(&sobel);

		free(linebuf);
	}
}

void process_slices(Nrrd *nin)
{
	int width = nin->axis[0].size,
			height = nin->axis[1].size,
			slices = nin->axis[2].size;

	IplImage *unwrapped = cvCreateImage(cvSize(RADIAL_SAMPLES*MAX_LAYERS, slices), IPL_DEPTH_8U, 1);
	
	for(int i = 0; i < slices; i++) {
		radial_sample(width, height, ((char*)(nin->data))+(width * height * i), unwrapped, i);
	}

	cvSaveImage("unwrapped.png",unwrapped);
	cvReleaseImage(&unwrapped);
}

int main(int argc, char * argv[])
{
	Nrrd *nin = nrrdNew();
	Nrrd *nquantized = nrrdNew();
	char *biffErr;

	if(argc < 4) {
		fprintf(stderr,"Usage: %s centerx centery nrrd\n",argv[0]);
		return 1;
	}

	cx = atoi(argv[1]);
	cy = atoi(argv[2]);

	if(nrrdLoad(nin, argv[3], NULL)) {
		biffErr = biffGetDone(NRRD);
		fprintf(stderr, "Error reading file: %s\n", biffErr);
		free(biffErr);
		return 1;
	}

	printf("Read %d-dimensional nrrd of type %d (%s)\n",
			nin->dim, nin->type, airEnumStr(nrrdType, nin->type));

	printf("Axis dimensions: %d x %d x %d\n", nin->axis[0].size,
			nin->axis[1].size, nin->axis[2].size);

	NrrdRange *range = nrrdRangeNewSet(nin, 0);
	printf("min: %g\tmax: %g\n", range->min, range->max);

	if(nrrdQuantize(nquantized, nin, range, 8)) {
		biffErr = biffGetDone(NRRD);
		fprintf(stderr, "Error quantizing: %s\n", biffErr);
		return 1;
	}

	process_slices(nquantized);

	nrrdRangeNix(range);
	nrrdNuke(nin);
	nrrdNuke(nquantized);

	return 0;
}
