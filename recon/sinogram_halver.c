#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>

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


int main(int argc, char **argv)
{
	char *filename, *binfilename;
	char idenFan[45];
	char iden[35];
	char junk[35];
	int numfiles, i, j, k, var1, var2, width, step;
	float f1, f2, firstCOR, CORinterval, temp;
	short readnum;
	float sourceToDetectorD = -1;
	float sourceToSampleD = -1;
	FILE *fp;
	FILE *bfp;

	idenFan[0] = '\0';

	if (argc < 3)
	{
		printf("Usage: %s filename numfiles\n", argv[0]);
		exit(1);
	}
	numfiles = atoi(argv[2]);
	filename = argv[1];

	binfilename = (char *)malloc((strlen(filename) + 5) * sizeof(char));
	for (i = 0; i < numfiles; i++)
	{
		sprintf(binfilename, "half_");
		strncat(binfilename, filename, strlen(filename));
		printf("Converting %s to binary file %s, %d files left\n", 
			filename, binfilename, numfiles - i);

		if((fp = fopen(filename, "r")) == NULL)
	        {
	                perror("Cannot open file");
	                exit(1);
	        }
		if((bfp = fopen(binfilename, "w")) == NULL)
		{
			perror("Cannot open binary file");
			exit(1);
		}

		if(fscanf(fp, "%s", iden)!= 1 || strcmp(iden, "DSAA") != 0)
		{
			printf("Illegal sinogram file beginning\n");
			exit(1);
		}
		fprintf(bfp, "%s\n", iden);
		
		if(fscanf(fp, "%d %d", &var1, &var2) !=2)
		{
			printf("Unexpected 2nd line in sinogram file!\n");
			exit(1);
		}
		fprintf(bfp, "%d\t%d\n", var1, var2/2);

		if(fscanf(fp, "%f %f", &f1, &f2) !=2)
		{
			printf("Unexpected 3nd line in sinogram file!\n");
			exit(1);
		}
		fprintf(bfp, "%.1f\t%.1f\n", f1, f2);

		if(fscanf(fp, "%f %f", &f1, &f2) !=2)
		{
			printf("Unexpected 4th line in sinogram file!\n");
			exit(1);
		}
		fprintf(bfp, "%.1f\t%.1f\n", f1, f2);

		if(fscanf(fp, "%f %f", &f1, &f2) !=2)
		{
			printf("Unexpected 5th line in sinogram file!\n");
			exit(1);
		}
		fprintf(bfp, "%.1f\t%.1f\n", f1, f2);

		fseek(fp, -43, SEEK_END);
		if(fscanf(fp, "%s %f %f", iden, &firstCOR, &CORinterval) !=3)
		{
			printf("Unexpected tail(1) in sinogram file!\n");
			exit(1);
		}

		if (strcmp(iden, "COR_PARAMETERS") == 0) // COR_Parameters exist
		{
			// Get FAN_BEAM_PARAMETERS
			fseek(fp, -90, SEEK_END);
			if(fscanf(fp, "%s %f %f", idenFan, &sourceToDetectorD, &sourceToSampleD) !=3)
			{
				printf("Unexpected tail(2) in sinogram file!\n");
				exit(1);
			}
		}

		// Read in and write out floating point values
		if(fseek(fp, 0, SEEK_SET) == -1)
			perror("Error seeking");
		fscanf(fp, "%s", junk);
                fscanf(fp, "%d %d", &width, &step);
                fscanf(fp, "%f %f", &f1, &f2);
                fscanf(fp, "%f %f", &f1, &f2);
                fscanf(fp, "%f %f", &f1, &f2);
	
		// Skip the newline
		fseek(fp, 1, SEEK_CUR);

		for (var1 = 10; var1 > 0; var1--)
		{
			for(j=0; j<step; j++)
			{
				if ((j % 2) != 0) // Skip every other rotation
					continue;
				for(k=0; k<width; k++)
                                {
					if(fread(&temp, sizeof(float), 1, fp) !=1)
					{
						printf("Error reading file\n");
						exit(1);
					}
					readnum = (short)temp;
					fwrite(&readnum, sizeof(short), 1, bfp);
				}
			}
		}




		// Write out file tail
		if (strlen(idenFan) > 0 && sourceToDetectorD > -1)
			fprintf(bfp, "\n\n%s\n%12.4f\t%12.4f\n\n%s\n%12.6f\t%12.6f\n", idenFan, sourceToDetectorD, sourceToSampleD, iden, firstCOR, CORinterval);
		else
			fprintf(bfp, "\n\n%s\n%12.6f\t%12.6f\n", iden, firstCOR, CORinterval);

		fclose(fp);
		fclose(bfp);
		getFileName(filename, i + 1);
	}
}

