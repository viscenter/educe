#include "mpi.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <teem/nrrd.h>
#include <unistd.h>

#define NN 2048
#define NA 4096
#define PI 3.1415927
#define SLICE_CHUNK 40

struct scan_data
{
   int firstRow;
   int rowInterval;
   int centerRow;
   int startRow;
   int endRow;
   int startSlice;
   int endSlice;
   float dz;
   unsigned int N;  //width
   float MINPOSS;   //1/zmax
   unsigned int K; //step
   float angle_step; //da
   unsigned int L; 
   float DD; //sourceToDetectorD
   float D;  //sourceToSampleD
   float a;  
   float COR; 
   float CORinterval;
   unsigned int M; // =10
   float xx;
};

void getTenSlices(char * filename, int jumpline, float *slices, float *MPOSSptr);

/* (thb) Unnecessary function call overhead. Remove */
void out_message(char *a)
{
    printf("%s", a);
}

/* (thb) ? */
void getFileName(char *filename, int i)
{
    int j;    
    int len = strlen(filename);

    j=i/100;
    filename[len-7] = (char)(j+48);
    i=i-j*100;
    j=i/10;
    filename[len-6] = (char)(j+48);
    i=i-j*10;
    filename[len-5] = (char)(i+48);
}

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

/*
	void four1(float data[], unsigned long nn, int isign) Fast Fourier
	Transform (FFT).  Replaces data[1..2*nn] by its discrete Fourier
	transform, if isign is input as 1; or replaces data[1..2*nn] by nn
	timesits inverse discrete Fourier transform, if isign is input as
	-1.  data is a complex array of length nn or, equivalenty, a real
	array of length 2*nn. nn must be an integer power of 2 (this is not
	checked for!).
*/


void four1(float *data,int nn,int isign)
{
    int n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta;
    float tempr,tempi;

    n=nn << 1;
    j=1;
    for (i=1;i<n;i+=2) 
    {
		if (j > i) 
		{
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) 
		{
			j -= m;
			m >>= 1;
		}
		j += m;
    }
    mmax=2;
    while (n > mmax) 
    {
		istep=2*mmax;
		theta=6.28318530717959/(isign*mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) 
		{
			for (i=m;i<=n;i+=istep) 
			{
				j=i+mmax;
				tempr = (float)(wr*data[j]-wi*data[j+1]);
				tempi = (float)(wr*data[j+1]+wi*data[j]);
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
    }
}
#undef SWAP
/* 
	void realft(float data[], unsigned long n, int isign) FFT of Single
	Real Function.  Calculates the Fourier Transform of a set of n
	real-valued data points.  Replaces this data (which is stored in
	array data[1..n]) by the positive frequency half of its complex
	Fourier transform. The real-valued first and last components of the
	complex transform are returned as elements data[1] and data[2],
	respectively. n must be a power of 2. This routine also calculates
	the inverse transform of a complex data array if it is the
	transform of real data. (Result in this case must be multiplied by
	2/n.)
*/


void realft(float *data,int n,int isign)
{
    int i,i1,i2,i3,i4,n2p3;
    float c1=0.5,c2,h1r,h1i,h2r,h2i;
    double wr,wi,wpr,wpi,wtemp,theta;

    theta=3.141592653589793/(double) n;
    if (isign == 1) 
    {
	c2 = -0.5;
	four1(data,n,1);
    } 
    else
    {
    	c2=0.5;
	theta = -theta;
    }
    wtemp=sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi=sin(theta);
    wr=1.0+wpr;
    wi=wpi;
    n2p3=2*n+3;
    for (i=2;i<=n/2;i++) 
    {
	i4=1+(i3=n2p3-(i2=1+(i1=i+i-1)));
	h1r=c1*(data[i1]+data[i3]);
	h1i=c1*(data[i2]-data[i4]);
	h2r = -c2*(data[i2]+data[i4]);
	h2i=c2*(data[i1]-data[i3]);
	data[i1] = (float) (h1r+wr*h2r-wi*h2i);
	data[i2] = (float) (h1i+wr*h2i+wi*h2r);
	data[i3] = (float) (h1r-wr*h2r+wi*h2i);
	data[i4] = (float) (-h1i+wr*h2i+wi*h2r);
	wr=(wtemp=wr)*wpr-wi*wpi+wr;
	wi=wi*wpr+wtemp*wpi+wi;
    }
    if (isign == 1)
    {
	data[1] = (h1r=data[1])+data[2];
	data[2] = h1r-data[2];
    }
    else 
    {
	data[1]=c1*((h1r=data[1])+data[2]);
	data[2]=c1*(h1r-data[2]);
	four1(data,n,-1);
    }
}
/*
	Given two real input arrays data1[1..n] and data2[1..n],this
	routine calls four1 and ret rns two complex o tp t
	arrays,fft1[1..2n] and fft2[1..2n],each of complex length n
	(i.e.,real length 2*n ,which contain the discrete Fourier
	transforms of the respective data arrays.n MUST be an integer power
	of 2.
*/

void twofft(float *data1,float *data2,float *fft1,float *fft2,int n)
{
    int nn3,nn2,jj,j;
    float rep,rem,aip,aim;

    nn3=1+(nn2=2+n+n);
    for (j=1,jj=2;j<=n;j++,jj+=2) 
    {
	fft1[jj-1]=data1[j];
	fft1[jj]=data2[j];
    }
    four1(fft1,n,1);
    fft2[1]=fft1[2];
    fft1[2]=fft2[2]=0.0;
    for (j=3;j<=n+1;j+=2) 
    {
	rep = (float) (0.5*(fft1[j]+fft1[nn2-j]));
	rem = (float) (0.5*(fft1[j]-fft1[nn2-j]));
	aip = (float) (0.5*(fft1[j+1]+fft1[nn3-j]));
	aim = (float) (0.5*(fft1[j+1]-fft1[nn3-j]));
	fft1[j]=rep;
	fft1[j+1]=aim;
	fft1[nn2-j]=rep;
	fft1[nn3-j] = -aim;
	fft2[j]=aip;
	fft2[j+1] = -rem;
	fft2[nn2-j]=aip;
	fft2[nn3-j]=rem;
    }
}

static float sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)
/* 
Convolves or deconvolves a real data set data[1..n] (including any
user-supplied zero padding) with a response function respns[1..n]. The
response function must be stored in wrap-around order in the first m
elements of respns,wherem is an odd integer ¡Ün. Wrap-around order means
that the first half of the array respns contains the impulse response
function at positive times, while the second half of the array contains the
impulse response function at negative times, counting down from the highest
element respns[m]. On input isign is +1 for convolution, .1 for
deconvolution. The answer is returned in the first n components of ans.
However, ans must be supplied in the calling program with dimensions
[1..2*n], for consistency with twofft. n MUST be an integer power of two.
*/
int convlv(float *data, int n, float *respns, int m, int isign, float *ans)
{
    int i,no2;
    float dum,mag2,*fft;

    fft = (float *)malloc((unsigned) 2*n*sizeof(float));
    if (!fft)
    {
        out_message("allocation failure in convlv() failed");
        return 1;
    }
    fft = fft - 1;

/*	for (i=1;i<=(m-1)/2;i++)
		respns[n+1-i]=respns[m+1-i];
	for (i=(m+3)/2;i<=n-(m-1)/2;i++)
		respns[i]=0.0;
*/
    twofft(data,respns,fft,ans,n);
    no2=n/2;
    for (i=2;i<=n+2;i+=2) 
    {
	if (isign == 1)
	{
		ans[i-1]=(fft[i-1]*(dum=ans[i-1])-fft[i]*ans[i])/no2;
		ans[i]=(fft[i]*dum+fft[i-1]*ans[i])/no2;
	}
	else if (isign == -1)
	{
	        if ((mag2=SQR(ans[i-1])+SQR(ans[i])) == 0.0)
            	{
                	out_message("Deconvolving at response zero in CONVLV");
                	return 1;
            	}
		ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/mag2/no2;
		ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/mag2/no2;
	}
	else
	{
        	out_message("No meaning for ISIGN in CONVLV");
            	return 1;
        }
    }
    ans[2]=ans[n+1];
    realft(ans,no2,-1);
    free((char*) (fft + 1));
    return 0;
}
#undef SQR

void fillin_phi_r(float *phi, float *r, int L, float tou)
{ // Since i and j are unreferenced variables, comment them out.
  //int i, j;
    int minc, maxc, x, y;
    float *mphi;
    float *mr;
  
    minc = L/2;
    maxc = (L/2) - 1;
  
    mphi = phi - 1;
    mr = r - 1;
  
    for(x= (-minc); x <= maxc; x++)
    	for(y = (-minc); y <= maxc; y++) 
    	{
	        /*increment pointers */
        	mphi++;
      		mr++;

      		/*r*/
      		(*mr) = (float)sqrt((double)(x*x + y*y)) * tou;
      
      		/* phi */
      		if(x==0 && y==0)
			(*mphi) = 0.0;
      		else
      			(*mphi) = (float)atan2((double)y, (double)x);
    	}
}

/* FAN BEAM BACKPROJECTION CODE FOR FLAT DETECTOR */
/* Input: pointer to normalized sinogram floating point data */
//int bp(float *sino, unsigned int N, float MINPOSS, unsigned int K, 
//float angle_step, float *image, unsigned int L, float DD, float D,
//float a, float COR, unsigned int M, scan_data *sd)
int bp(float *sino, float *image, unsigned int L, struct scan_data *sd, int myid)
{  
    float toualpha;
    float gna[NA], smt[NA];
    float rna[2*NA], srna[2*NA], qna[2*NA];
    float scale_factor, mag, tou;
    float U, s, beta, cr, cphi;
    int i, j, k;
    unsigned int x, y, n;
    float *sinoptr, *imageptr, *rptr, *phiptr, *phi, *r;
	float *UU;
	int *NNN;
//    char feedbackstr[80];
    unsigned int N;
    float MINPOSS,dz; 
    float zz=0;
    unsigned int K;
    float angle_step;
    float DD, D, a, COR, CORinterval; 
    unsigned int M;

    //transfer parameters
    N=sd->N;
    MINPOSS=sd->MINPOSS;
    K=sd->K;
    angle_step=sd->angle_step;
    DD=sd->DD;
    D=sd->D;
    a=sd->a; 
    COR=sd->COR;
    CORinterval=sd->CORinterval;
    M=sd->M; 
    dz=sd->dz*sd->rowInterval;
//   printf("%d %d %f %f %f %f %f %f\n", N, M, angle_step,DD, D, a, COR, MINPOSS);
 
    a = a*D/DD;
    toualpha =  (float)((N/2.0)-COR) * a;
    mag = DD/D;
    tou = (float)(a * N)/(float)L;
    if((r = (float *)malloc(L*L*sizeof(float))) == 0)
    {
    	out_message("Cannot allocate memory for bp function.\n");
        return 1;
    }
    if((phi = (float *)malloc(L*L*sizeof(float))) == 0)
    {
    	out_message("Cannot allocate memory for bp function.\n");
  	return 1;
    }
    if((UU = (float *)malloc(L*L*sizeof(float))) == 0)
    {
    	out_message("Cannot allocate memory for bp function.\n");
        return 1;
    }
    if((NNN = (int *)malloc(L*L*sizeof(int))) == 0)
    {
    	out_message("Cannot allocate memory for bp function.\n");
  		return 1;
    }

  
  // make kernel filter in space domain 
    for(i = 0; i < NA; i++)
    	gna[i] = 0.0;
    gna[0] = (float)1.0/(8*a*a);
    scale_factor = gna[0];      
    for(i = 1; i < NN/2; i+=2) 
    {
    	gna[i] = (float)(-1.0/(2*i*i*PI*PI*a*a));
    	scale_factor += gna[i];
    }
    for(i = NA-1; i > (NA - NN/2); i-=2) 
    { 
    	n = NA-i;
    	gna[i] = (float)(-1.0/(2*n*n*PI*PI*a*a));
    	scale_factor += gna[i];
  	}

  // make sure that the total sum is 1.0 
    for(i=0; i<NA; i++)
    	gna[i] = gna[i]/scale_factor;

  // Hamming filter in space-domain 
    for (i=0;i< NA;i++)
    	smt[i]=0.0;
    scale_factor = 0.0;
    j = NA - M/2;
    n = 0;
    for(i = 0; i < M; i++) 
    {
    	smt[j] = (float)(0.54 - 0.46 * cos((double)2 * PI * n / M));
    	scale_factor += smt[j];
    	if(++j == NA) j = 0;
    	n++;
    }
  
  // scale so that sum is 1.0 
    for(i = 0; i <NA; i++)
    	smt[i] = (float)(smt[i]/scale_factor);

  // predetermine phi and r for optimization 
    fillin_phi_r(phi, r, L, tou);

    imageptr = image;

   int imagenum;
   imagenum = sd->endSlice-sd->startSlice+1;
   int rownum = sd->endRow-sd->startRow+1;

      
    for(k=0; k<imagenum; k++)
       for(i = 0; i < L; i++)
            for( j = 0; j < L; j++)
    	    {
      		*imageptr = 0.0;
      		imageptr++;
    	    } 


    for(i = 0; i < NA; i++)
    	rna[i] = 0;
    int rr;
    // start of loop 
    float *QNA, *QNAptr;

    if((QNA = (float *)malloc(rownum*NA*sizeof(float))) == 0)
    {
        out_message("Cannot allocate memory for bp function.\n");
        return 1;
    }

//    printf("rownum = %d, imagenum = %d\n",rownum, imagenum);
    //printf("K=%d\n",K);


	float *puu; int* pnn;

    for(k=0; k<K; k++) 
    {
//	out_percent(sPercent + k*(ePercent-sPercent)/K);
//        sinoptr = sino + k*N;


     for(rr=0;rr<rownum; rr++)
     {
		sinoptr = sino+k*N+rr*K*N;

		//cr = 3.0;

        //zz=-(sd->startSlice+rr*sd->rowInterval-sd->centerRow)*
        //    sd->dz*(sd->D-cr)/sd->DD;


    	for(i=0; i<N; i++)
    	{
      		rna[i] = *sinoptr;
      		sinoptr++;
    	}

    	for(i = N; i < NA; i++)
      		rna[i] = 1.0;
    
    // dump the corrected projection 
    // log and then weigh the data 

    	for(i = 0; i < NA; i++) 
    	{
      		if(rna[i] < MINPOSS)
        		rna[i] = 1.0;
      		else
      			rna[i] = (float)((log((double)rna[i])) / log((double)MINPOSS));
    	}
    	for(i = 0; i < NA; i++) 
    	{
    		n = (int)(i - COR);
      		rna[i] = (float)(rna[i]*(D*D - toualpha*D)/(D*sqrt((double)(D*D + 
			n*n*a*a+zz*zz))));
    	}
    
    // dump the projection 


    // smooth 
    	if (convlv(rna-1, NA, smt-1, NA, 1, srna-1)) return 1;
    // dump the smoothed projection 


//    out_percent(sPercent + k*(ePercent-sPercent)/K + (ePercent-sPercent)/K/2);
		
     	if (convlv(srna-1, NA, gna-1, NA, 1, qna-1)) return 1;
 


	QNAptr = QNA+rr*NA;  
	for(i=0;i<NA; i++)
	{
	     *QNAptr=qna[i];
	     QNAptr++;
	}

	COR += CORinterval;
 }//end of Fourier transform of data



//prepare for backprojection

	beta = (float)(angle_step * k * PI / 180.0);
	phiptr = phi -1;
	rptr = r -1;

	//	cr = *rptr;


	puu=UU; pnn=NNN;

	for(i = 0; i < L; i++)
        for( j = 0; j < L; j++)
  	    {
   			rptr++;
   			phiptr++;
	 
	 		cr = *rptr;
	 		cphi = *phiptr;

	 		U = (float)(D + cr * sin((double)beta - cphi))/D;
	 		s = (float)(D * cr * cos((double)beta - cphi) - D*toualpha)/
	  			((float)(D + cr * sin(beta - cphi)));
	 
	                // find closest n 
	 		n = (unsigned int)((s / a) + (N/2.0) + 0.5);
			*puu=1.0/(U*U);
			*pnn=n;
			puu++;
			pnn++;
		} 



    // dump the projection 


    // backprojet 

     for(i=0; i<imagenum; i++)
      {



        phiptr = phi -1;
        rptr = r -1;
//      imageptr = image -1;

//	cr = *rptr;
	
//	cr =3.0;
//	zz=-(sd->firstRow+i*sd->rowInterval-sd->centerRow)*
//		sd->dz*(sd->D-cr)/sd->DD;
//             sd->dz*(sd->D-sd->xx)/sd->DD;
	puu = UU;
	pnn = NNN;
	imageptr = image-1+L*L*i;
	QNAptr = QNA+i*NA;  
	
    	for(x= 0; x < L; x++) 
    	{
    		for(y = 0 ; y < L; y++)
    		{
    			//rptr++;
    			//phiptr++;
	 			imageptr++;
	 
//	 		cr = *rptr;
//	 		cphi = *phiptr;
	 
//	 		U = (float)(D + cr * sin((double)beta - cphi))/D;
//	 		s = (float)(D * cr * cos((double)beta - cphi) - D*toualpha)/
//	  			((float)(D + cr * sin(beta - cphi)));
	 
	                // find closest n 
//	 		n = (unsigned int)((s / a) + (N/2.0) + 0.5);

			
//			rr=(sd->centerRow-(int)(zz/(U*D)*DD/sd->dz)-sd->firstRow)
//				/sd->rowInterval;

//         if(myid==1&&y==0&&i==0)printf("%f %d %f %f %f %d\n", zz,rr,U*D, 
//sd->D-sd->xx, cr, rownum);

			n =*pnn;
			//if(n>=0 && n < N && rr>=0 && rr<rownum)
			if(n>=0 && n < N)
	 		{	//*imageptr += (qna[n]/(U*U));
				//rr=rr-sd->startRow;
				//*imageptr +=(QNAptr QNA[rr*NA+n]/(U*U));
				*imageptr +=*(QNAptr+n)*(*puu);
			}
			pnn++; puu++;
       		} 
     	}
		

      }


//    sprintf(feedbackstr, "        backprojected %d", k);
//    out_message(feedbackstr);
} //end of k

//  out_percent(ePercent);
    if(r) free(r);		//added by jie to prevent memory leak
    if(phi) free(phi);		//added by jie to prevent memory leak
    if(QNA) free(QNA);
	if(NNN!=0) free(NNN);
    if(UU!=0) free(UU);

    return 0;

}

typedef struct {
    int x;
    int y;
    int z;
}  C_IntTuple;

	
int main( int argc, char *argv[] )
{
    int isOpenSuc = 1, iscalfail = 0, isMissCan = 0;
    int isSlicePerNodeChanged = 0;
    int i, j, sliceChunk;
    int myid, numprocs;
    int namelen, ImageSize;
    int width, step, jumpline;
    int fileNum;
    int sliceNum, slicePerNode, surplus, slicesSent;
    //first row number on the detector & interval,the row of the source 
    float dz,xx; //height of each row, size of sample, Chuanyu

    float xmin, xmax, ymin, ymax, zmin, zmax;
    float sourceToSampleD, sourceToDetectorD;
    float firstCOR, lastCOR, CORinterval, startCOR, COR, rotateD=0;
    float *slices, *images, *MINPOSS;
    float *sliceptr, *imageptr, *MPOSSptr;
    char YorN;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    char sinfileNameStr[100], volfileNameStr[70], iden[30], ofnamext[100];
    FILE *fp;
    int fileCounter = 0;
    int outstandingProcesses = 0;
    int *fileOffset;
    MPI_Status status;
    #ifdef TIMING
    double cStart, cTotalStart = 0; // Timing, for benchmarking
    #endif


    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    
    //root node prompts and read in the neccessary info for reconstruction.
    ///confirmation is needed to avoid the broken pipe error when info is not
    //provided correctly.
    if (myid == 0)
    {
	if (argc > 1)
	{
	    sliceChunk = atoi(argv[1]);
	    if ((sliceChunk % 10) != 0)
	    {
		printf("The number of slices per node must be evenly divisible by 10. %d is not valid\n", sliceChunk);
		exit(1);
	    }
	}
	else
	    sliceChunk = SLICE_CHUNK;

    	//read in the start info
    	printf("Enter the location of the first sinogram file:  ");
	fflush(stdout);
    	scanf("%s", sinfileNameStr);
    	
    	printf("Enter number of sinogram files within the same band:  ");
	fflush(stdout);
    	scanf("%d", &fileNum);
    	sliceNum = fileNum * 10;
    	slicePerNode = sliceNum/(numprocs-1);
    	surplus = sliceNum - (numprocs-1)*slicePerNode;
    	if(surplus!=0) slicePerNode++;
    	else isSlicePerNodeChanged = 1;
    	
    	printf("Enter the output vol file path and name:  ");
	fflush(stdout);
    	scanf("%s", volfileNameStr);
    	
	printf("Enter the image size:  ");
	fflush(stdout);
    	scanf("%d", &ImageSize); 	
    	
    	printf("Do you want to rotate the reconstructed images by a degree?(y/N) ");
	fflush(stdout);
    	scanf("%s", &YorN);
    	
    	if(YorN == 'Y' || YorN == 'y')
	{
	    printf("Enter the degree wish to rotate along clockwise:  ");
	    fflush(stdout);
	    scanf("%f", &rotateD); 
	    
	    rotateD = 360.0 - rotateD;	//to rotate the image clockwisely
	    while(rotateD<0)	    rotateD +=360;
	    while(rotateD>=360)	    rotateD -= 360;
	}
	YorN = 'A';
	
	
	// based on info of the sinogram file and output vol file, open the
	// files to make sure the it is right format and directoris is
	// correct.  read in the info of sino file header and tail.
    	if((fp = fopen(sinfileNameStr, "r")) == NULL)
    	{
    		out_message("Cannot open first sinogram file!\n");
    		isOpenSuc = 0;
    	}
    	else
    	{
    		out_message("\nOpen success!\n");
    		if(fscanf(fp, "%s", iden)!= 1)
    		{
    			out_message("Unexpected beginning of sinogram file!\n");
    			isOpenSuc = 0;
    		}
    		else
    		{
    			if(strcmp(iden, "DSAA")!=0)
    			{
    				out_message("Illegal identification of sinogram file!\n");
    				isOpenSuc = 0;
    			}
    			else
    			{
    				if(fscanf(fp, "%d %d", &width, &step) !=2)
    				{
    					isOpenSuc = 0;
    					out_message("Unexpected 2nd line in sinogram file!\n");
    				}
    				if(fscanf(fp, "%f %f", &xmin, &xmax) !=2)
    				{
    					isOpenSuc = 0;
    					out_message("Unexpected 3nd line in sinogram file!\n");
    				}
    				if(fscanf(fp, "%f %f", &ymin, &ymax) !=2)
    				{
    					isOpenSuc = 0;
    					out_message("Unexpected 4nd line in sinogram file!\n");
    				}
    				if(fscanf(fp, "%f %f", &zmin, &zmax) !=2)
    				{
    					isOpenSuc = 0;
    					out_message("Unexpected 5nd line in sinogram file!\n");	
    				}
    				printf("width = %d, step = %d\nxmin = %f, xmax = %f\nymin = %f, ymax = %f\nzmin = %f, zmax = %f\n",
    					width, step, xmin, xmax, ymin, ymax, zmin, zmax);
    			}
    		}
    		
    		//read the COR info and distance info at the tail of sinogram file
    		fseek(fp, -43, SEEK_END);
		if(fscanf(fp, "%s %f %f", iden, &firstCOR, &CORinterval) !=3)
    		{ 
		    	printf("Unexpected tail(1) in sinogram file!\n");
		    	isOpenSuc =0;
		}
    		else
    		{
    			//printf("the iden read from the tail is: %s\n", iden);
    			if(strcmp(iden, "COR_PARAMETERS")!=0)
    			{
    				//the sinogram file do not have the COR info, the data read into firstCOR and CORinterval
    				// are acturally FAN_BEAM_PARAMETERS
    				sourceToDetectorD = firstCOR;
    				sourceToSampleD = CORinterval;
    				
    				printf("sourceToDetectorD = %12.4f\tsourceToSampleD = %12.4f\n\n", sourceToDetectorD, sourceToSampleD);
    				
    				printf("The sinogram file does not have COR information!\n");
    				printf("Enter the COR of the first slice in this band:  ");
				scanf("%f", &firstCOR); 
				    	
				printf("Enter the COR of the last slice in this band:  ");
				scanf("%f", &lastCOR); 
		
				CORinterval = (lastCOR-firstCOR)/(fileNum*10-1);
			}
			else
			{
				//read the FAN_BEAM_PARAMETERS
				fseek(fp, -83, SEEK_END);
				if(fscanf(fp, "%s %f %f", iden, &sourceToDetectorD, &sourceToSampleD) !=3)
				{
					isOpenSuc =0;
    					out_message("Unexpected tail(2) in sinogram file!\n");
    				}
    				else
    				{
    					lastCOR = firstCOR + CORinterval*(fileNum*10-1);
    					
    					printf("sourceToDetectorD = %12.4f\tsourceToSampleD = %12.4f\n\n", sourceToDetectorD, sourceToSampleD);
    					printf("first slice COR is %12.6f and last slice COR is %12.6f\n", firstCOR, lastCOR);
    					printf("Do you want to change it?");
					fflush(stdout);
    					scanf("%s", &YorN);
    					
    					if(YorN == 'Y' || YorN == 'y')
					{
	    					printf("Enter the COR of the first slice in this band:  ");
						fflush(stdout);
						scanf("%f", &firstCOR); 
				    	
						printf("Enter the COR of the last slice in this band:  ");
						fflush(stdout);
						scanf("%f", &lastCOR); 
		
						CORinterval = (lastCOR-firstCOR)/(fileNum*10-1);
						
					}
					YorN = 'A';
    				}
    			}
    		}
    		fclose(fp);
    	}

	//open the output vol file for test
	if(isOpenSuc == 1)
	{
		sprintf(ofnamext, "%s.raw", volfileNameStr);
		if((fp = fopen(ofnamext, "w")) == NULL)
		{
			out_message("Cannot open output vol file!\n");
			isOpenSuc = 0;
		}
		else
			fclose(fp);
	}
    
    
 	if(isOpenSuc == 1)
 	{
 		printf("\n************* Recon Info ********************\n\n"); 
   	 	printf("The location of sinogram files (sinfileName): %s\n", 
			sinfileNameStr);
   	 	printf("Number of sinogram files in this band (fileNum) = %d\n",
			fileNum);
    		printf("The output vol file (volfileName): %s\n", 
			volfileNameStr);
   	 	printf("The image size (imagesize) = %d\n", ImageSize);
    		printf("The COR of the first slice (firstCOR) = %f\n", 
			firstCOR);
    		printf("The COR of the last slice (lastCOR) = %f\n", lastCOR);  
    		printf("CORinterval = %f\n\n", CORinterval);

		while(YorN != 'Y' && YorN != 'y' && YorN != 'N' && YorN != 'n')
		{
			printf("Is all those information correct?(Y/N):");
			fflush(stdout);
			scanf("%s", &YorN);
		}

		if(YorN == 'N' || YorN == 'n')
		{
			printf("Mission is Cancelled!\n");
			isMissCan = 1;
		}
		else
		{
			printf("Mission is Accepted!\n");
			isMissCan = 0;
		}
    	}
    	else
    		isMissCan = 1;
    }
    #ifdef TIMING
    if (myid == 0) 
	cTotalStart = MPI_Wtime();
    #endif
    MPI_Bcast(&isMissCan, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(isMissCan == 1)
    {
	MPI_Finalize();
	return 0;
    }

    // broadcast the neccessary info to all the nodes, and allocate memory in each node.  
    MPI_Bcast(&isOpenSuc, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&step, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&slicePerNode, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&surplus, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ImageSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&xmax, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&CORinterval, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sourceToSampleD, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sourceToDetectorD, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sliceNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sliceChunk, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for MPOSS stuff
    MINPOSS = (float *)malloc(sliceNum * sizeof(float));

    // Allocate memory for images
    images = (float *)malloc(ImageSize*ImageSize*sliceChunk*sizeof(float));

    // ?
    dz = xmax / width;
    xx = xmax * sourceToSampleD/sourceToDetectorD;

    // Loop for the master process. Read in sliceChunk slices, then send
    // that slice off to a compute node. Once a compute node is done,
    // it sends its chunk back and gets the next one.
    if (myid == 0)
    {
	jumpline = (int)rotateD*step/360;
    	startCOR = firstCOR;
	MPOSSptr = MINPOSS;
	slicesSent = 0;
	slices = (float *)malloc(width*step*sliceChunk*sizeof(float));
	fileOffset = (int *)malloc(numprocs * sizeof(int));
	/* This for loop hands out the initial data chunks to the
	 * slave nodes
	 */
    	for(i = 1; i < numprocs; i++)
    	{
	    slicePerNode = (sliceNum < sliceChunk) ? sliceNum : sliceChunk;
	    // Read in the requested number of slices

	    // Fill the slices buffer
	    #ifdef TIMING
	    cStart = MPI_Wtime();
	    #endif
	    sliceptr = slices;
	    for (j = 0; j < slicePerNode/10; j++)
	    {
	    	getFileName(sinfileNameStr, fileCounter);
	    	getTenSlices(sinfileNameStr, jumpline, sliceptr, MPOSSptr);

	    	fileCounter++;
		sliceptr += 10*step*width;
		MPOSSptr += 10;
	    }
	    #ifdef TIMING
	    if (slicePerNode > 0)
		printf("Reading %d slices took %lf seconds\n", slicePerNode, 
		    MPI_Wtime() - cStart);
	    #endif

	    //Send this slice to the next node
	    printf("Sending %d slices to node %d\n", slicePerNode, i);
	    #ifdef TIMING
	    cStart = MPI_Wtime();
	    #endif

	    MPI_Send(&slicePerNode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    MPI_Send(slices, width*step*slicePerNode, MPI_FLOAT, i, 0, 
	    	MPI_COMM_WORLD);
	    MPI_Send(&startCOR, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
	    MPI_Send(MINPOSS + slicesSent, slicePerNode, MPI_FLOAT, i, 0,
	    	MPI_COMM_WORLD);
	    MPI_Send(&slicesSent, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    #ifdef TIMING
	    printf("Sending %ld megabytes took %lf seconds\n\n", 
	      ((width*step*slicePerNode + slicePerNode + 1)*(long)sizeof(float) 
		    + 2*sizeof(int)) / 1048576, 
	    	MPI_Wtime() - cStart);
	    #else
	    printf("Send successful\n\n");
	    #endif

	    outstandingProcesses++;
	    fileOffset[i] = ImageSize*ImageSize*slicesSent;
	    startCOR += CORinterval * slicePerNode;
	    sliceNum -= slicePerNode;
	    slicesSent += slicePerNode;
    	} // End starting master loop

	/* Open the vol file, write data */
	FILE *ffp;
	int sliceThisTransaction;

        //open the vol file 
        sprintf(ofnamext,"%s.raw", volfileNameStr);
        if ((ffp = fopen(ofnamext, "w")) == NULL)
        {
              perror("Error opening vol output file");
              return(1);
        }

        printf("\nvol file is opened!\n");

	/* This loop waits for one of the slaves to come back. When it does,
	 * it writes the returned data to file and sends that slave the next
	 * chunk
	 */
	while (outstandingProcesses > 0)
	{
	    slicePerNode = (sliceNum < sliceChunk) ? sliceNum : sliceChunk;

	    // Fill the slices buffer
	    #ifdef TIMING
	    cStart = MPI_Wtime();
	    #endif
	    sliceptr = slices;
	    for (j = 0; j < slicePerNode/10; j++)
	    {
	    	getFileName(sinfileNameStr, fileCounter);
	    	getTenSlices(sinfileNameStr, jumpline, sliceptr, MPOSSptr);

	    	fileCounter++;
		sliceptr += 10*step*width;
		MPOSSptr += 10;
	    }
	    #ifdef TIMING
	    if (slicePerNode > 0)
		printf("Reading %d slices took %lf seconds\n", slicePerNode, 
		    MPI_Wtime() - cStart);
	    #endif

	    // Receive the data from the slave node that finishes next
	    MPI_Recv(&sliceThisTransaction, 1, MPI_INT, MPI_ANY_SOURCE, 0, 
	    	MPI_COMM_WORLD, &status);
	    MPI_Recv(images, ImageSize * ImageSize * sliceThisTransaction, 
		MPI_FLOAT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
	    printf("Received %d slices from node %d:\n", sliceThisTransaction, 
	    	status.MPI_SOURCE);
	    outstandingProcesses--;
	    
	    // Send the data to the slave node to keep it busy, if there
	    // is any more
	    if (slicePerNode != 0)
	    {
		printf("Sending %d slices to node %d\n", slicePerNode, 
			status.MPI_SOURCE);
		#ifdef TIMING
		cStart = MPI_Wtime();
		#endif
		MPI_Send(&slicePerNode, 1, MPI_INT, status.MPI_SOURCE, 
			0, MPI_COMM_WORLD);
		MPI_Send(slices, width*step*slicePerNode, MPI_FLOAT, 
		    status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		MPI_Send(&startCOR, 1, MPI_FLOAT, status.MPI_SOURCE, 0, 
		    MPI_COMM_WORLD);
		MPI_Send(MINPOSS + slicesSent, slicePerNode, MPI_FLOAT, 
		    status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		MPI_Send(&slicesSent, 1, MPI_INT, status.MPI_SOURCE, 
		    0, MPI_COMM_WORLD);
		#ifdef TIMING
		printf("Sending %ld megabytes took %lf seconds\n\n", 
		  ((width*step*slicePerNode+slicePerNode+1)*(long)sizeof(float) 
			+ 2*sizeof(int)) / 1048576, 
		    MPI_Wtime() - cStart);
		#else
		printf("Send successful\n");
		#endif

		fileOffset[0] = fileOffset[status.MPI_SOURCE];
		fileOffset[status.MPI_SOURCE] = ImageSize*ImageSize*slicesSent;
		outstandingProcesses++;
		startCOR += CORinterval * slicePerNode;
		sliceNum -= slicePerNode;
		slicesSent += slicePerNode;
	    }
	    else // We still need the correct offset
	        fileOffset[0] = fileOffset[status.MPI_SOURCE];
 
 	    #ifdef TIMING
	    printf("Writing data to file at offset %d ", fileOffset[0] * (int)sizeof(float));
	    #endif

	    if (fseek(ffp, fileOffset[0] * sizeof(float), SEEK_SET) == -1)
		perror("Error seeking");
	    if (fwrite(images, sizeof(float), 
	    	    ImageSize*ImageSize*sliceThisTransaction, ffp) == -1)
		perror("Error writing");
	    printf("...Done (%d outstanding processes)\n\n", outstandingProcesses);

	} // end of waiting master loop
	free(fileOffset);
	printf("Done writing voxel file\n");
	fclose(ffp);

	// Write the detached NRRD header
	char *bifferr;
	printf("Writing detached header..."); fflush(stdout);

	Nrrd *nval;
	NrrdIoState *nio;
	nval = nrrdNew();
	nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateDetachedHeader, AIR_TRUE);
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
	
	if (nrrdWrap_va(nval, slices, nrrdTypeFloat, 3, 
		(size_t)ImageSize, (size_t)ImageSize, (size_t)slicesSent))
	{
		bifferr = biffGetDone(NRRD);
		fprintf(stderr, "Error wrapping: %s\n", bifferr);
		free(bifferr);
	}
        sprintf(ofnamext,"%s.nhdr", volfileNameStr);
	if(nrrdSave(ofnamext, nval, nio))
	{
		bifferr = biffGetDone(NRRD);
		fprintf(stderr, "Error wrapping: %s\n", bifferr);
		free(bifferr);
	}
	printf("Done\n");

	free(slices);

    } // End master loop if
    else // Slave nodes
    {
    	for (;;)
	{
	    MPI_Recv(&slicePerNode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

	    if (slicePerNode == -1)
	        break;
	    slices = (float *)malloc(width*step*slicePerNode*sizeof(float));

	    MPI_Recv(slices, width*step*slicePerNode, MPI_FLOAT, 0, 0, 
		    MPI_COMM_WORLD, &status);
	    MPI_Recv(&startCOR, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
	    MPI_Recv(MINPOSS, slicePerNode, MPI_FLOAT, 0, 0, MPI_COMM_WORLD,
		    &status);
	    MPI_Recv(&slicesSent, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

	    sliceptr = slices;
	    imageptr = images;
	    COR = startCOR;
	    MPOSSptr = MINPOSS;
	    struct scan_data sd;
	    sd.firstRow = 120;		//(thb) Fix magic numbers
	    sd.centerRow = 960;
	    sd.rowInterval = 2;
	    sd.startRow = slicesSent;
	    sd.endRow = (slicesSent + slicePerNode) -1;
	    sd.startSlice = sd.startRow;
	    sd.endSlice = sd.endRow;
	    sd.dz = dz;

	    sd.N = width;
	    sd.MINPOSS = *MPOSSptr;
	    sd.K = step;
	    sd.angle_step = 360.0/step;
	    sd.DD=sourceToDetectorD;
	    sd.D=sourceToSampleD;
	    sd.a=xmax/width;
	    sd.COR=COR;
	    sd.CORinterval=CORinterval;
	    sd.M=10; // =10 (thb) yet another magic number
	    sd.xx=xx;

	    if (bp(sliceptr, imageptr,ImageSize,&sd,myid)==1)
	    	iscalfail = 1;
	    
	    MPI_Send(&slicePerNode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	    MPI_Send(images, ImageSize*ImageSize*slicePerNode, MPI_FLOAT,
	             0, 0, MPI_COMM_WORLD);
	    free(slices);
	} // End slave node loop
    } // End slave node else

    //Clean things up and get out of here
    j = -1;
    for (i = 1; i < numprocs; i++)
        MPI_Send(&j, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

    if(images!=NULL)    {free(images); images=NULL;}
    if(MINPOSS!=NULL)   {free(MINPOSS); MINPOSS=NULL;}

    if (myid == 0)
    {
	printf("Done\n");
	#ifdef TIMING
	printf("Total time: %lf seconds\n", MPI_Wtime() - cTotalStart);
	#endif
    }
    MPI_Finalize();
    return 0;

} //main

// Get the slices from the filename that is passed in
void getTenSlices(char * filename, int jumpline, float *slices, float *MPOSSptr)
{
	char iden[30];
	int width, step, sinoFileCount, j, k;
	unsigned short tmp;
	float xmin, xmax, ymin, ymax, zmin, zmax;
	float *sliceptr = slices; //Start writing into slices
	FILE *fp;
	printf("filename = %s\n", filename);
	if ((fp = fopen(filename, "r")) == NULL)
		perror("Cannot open file");
	else
	{
		fscanf(fp, "%s", iden);
		fscanf(fp, "%d %d", &width, &step);
		fscanf(fp, "%f %f", &xmin, &xmax);
		fscanf(fp, "%f %f", &ymin, &ymax);
		fscanf(fp, "%f %f", &zmin, &zmax);
		fseek(fp, 1, SEEK_CUR); //Skip the newline
		

		// Each file has ten slices
		for (sinoFileCount = 10; sinoFileCount > 0; sinoFileCount--)
		{
			// read in one slice info in one sino file. the
			// data has been modified (devided by zmax) to
			// facilitate the data manipulating in leaf node.
			if (jumpline != 0)
				sliceptr += (step - jumpline)*width;
			for (j = 0; j < step; j++)
			{
				for (k = 0; k < width; k++)
				{
					if (fread(&tmp, sizeof(short), 1, fp) != 1)
					    printf("\nError reading file");
					*sliceptr = tmp / zmax;
					sliceptr++;
				}
				if (j == jumpline - 1) 
				 	sliceptr -= step*width;
			}
			sliceptr += jumpline * width;
			
			*MPOSSptr = 1.0/zmax;
			MPOSSptr++;
		}
	}
	fclose(fp);
}

