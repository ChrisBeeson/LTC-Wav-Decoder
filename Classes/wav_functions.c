#include"wav_functions.h"
#include"smpte.h"
/*********************************************************************************
Function Name: 
    ReadWavInit
Function Syntax:
    w=ReadWavInit(char *filename)
Function Description:
    Reads the .wav file header and fills in the relevant fields of the structure wavefmt
Parameters:
>Input:
    filename = filename of the .wav file to be read
>Outputs:
   pwave = pointer to the wavefmt structure (holds all information about .wav file)
********************************************************************************/
wavefmt *ReadWavInit(char *filename)
{
  	wavefmt *w;
    w = (wavefmt *)malloc(sizeof(wavefmt));
    if(w == NULL)
    {
        printf("Unable to allocate memory\n");
        return (wavefmt *)-1;
    }
	w->f1 = fopen(filename, "rb");
    if(w->f1 == NULL)
    {
        printf("Unable to open the wave file\n");
        return (wavefmt *)-1;
    }
	w->filename = (char *)malloc(sizeof(char)*(strlen(filename)+1));
	if(w->filename == NULL)
    {
        printf("Unable to allocate memory\n");
        return (wavefmt *)-1;
    }
	strcpy(w->filename, filename);
    fseek(w->f1,4,SEEK_SET);
    fread((char *) &(w->ChunkSize), 1, 4, w->f1);
    fseek(w->f1,16,SEEK_SET);
    fread((char *) &(w->SubChunk1Size), 1, 4, w->f1);
    fread((char *) &(w->Format), 1, sizeof(short), w->f1);
    fread((char *) &(w->Channels), 1, sizeof(short), w->f1);
    fread((char *) &(w->SampleRate), 1, sizeof(int), w->f1);
    fread((char *) &(w->ByteRate), 1, sizeof(int), w->f1);
    fread((char *) &(w->BlockAlign), 1, sizeof(short), w->f1);
    fread((char *) &(w->BitsPerSample), 1,sizeof(short), w->f1);
    fseek(w->f1, 40, SEEK_SET);
	fread((char *)&(w->DataSize), 1, sizeof(int), w->f1);
	return w;
}
/*********************************************************************************
Function Name: 
    ReadWavBuf
Function Syntax:
    ReadWavBuf(wavefmt *w, float **buf, int *t)
Function Description:
    Reads the data from the .wav file into buf for a particular duration of time samples
	specified by t. t is an array of [start_time end_time]
Parameters:
>Input:
    w = pointer to the wavefmt structure
	buf = data buffer pointer buffer
	t =  pointer to the sample interval for reading the samples
>Outputs/return value:
   0 = successful read
   -1 = error in memory allocation
********************************************************************************/
int ReadWavBuf(wavefmt *w, float **buf, int *t)
{
	int noBytes, i, j;
	char *data;
	int size;
//	float maxi;
	//careful works only for 16bit, 8 bit, 24, 32 bits
	noBytes = w->BitsPerSample/8;
	size = t[1]-t[0]+1;
    data = (unsigned char *)malloc(noBytes*(t[1]-t[0]+1)*sizeof(char));
	if(*buf == NULL)
        *buf = (float *)malloc((t[1]-t[0]+1)*sizeof(float));
	else
		realloc(buf, (t[1]-t[0]+1)*sizeof(float));
	if((*buf == NULL) || (data == NULL))
    {
        printf("Unable to allocate memory\n");
        return -1;
    }
	w->data = *buf;
	fseek(w->f1, noBytes*t[0], SEEK_CUR);
    fread(data, 1, noBytes*(t[1]-t[0]+1), w->f1);
    for(i=0; i < (t[1]-t[0]); i++)
    {
        short temp = 0;
        for(j=0; j < noBytes; j++)
			temp = temp | *((unsigned char *)(data + j + 2*i))<< 8*j;
		//repeat the sign in here..
		*(*buf + i) = temp/(float)pow(2,w->BitsPerSample-1);
		if(*(*buf + i)==0)
			continue;
        printf("%d=%12.10f\n",i,*(*buf + i));
    }
    free(data);
	
	/*
	//normalise the read data
	maxi = maximum(*buf,size);
	for(i=0; i < size; i++)
        *(*buf+i) = 0.9f*(*(*buf+i))/maxi;
	 */
	
	return 0;
}

/*********************************************************************************
Function Name: 
    ReadWavClose
Function Syntax:
    ReadWavClose(wavefmt *w)
Function Description:
    Realease and close all memory allocations related to the wave file being read
Parameters:
>Input:
    w = pointer to the wavefmt structure
>Outputs/return value:
   None
********************************************************************************/
void ReadWavClose(wavefmt *w)
{
	fclose(w->f1);
	free(w->filename);
	free(w->data);
	free(w);
}