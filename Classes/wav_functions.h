#ifndef WAV_FUNCTIONS_H_
#define WAV_FUNCTIONS_H_

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

typedef struct {
	char *filename;
	FILE *f1;
    int ChunkSize;
    int SubChunk1Size;
    short Format;
    short Channels;
    int SampleRate;
    int ByteRate;
    short BlockAlign;
    short BitsPerSample;
    int   DataSize;
    float *data;
}wavefmt;

wavefmt *ReadWavInit(char *filename);
void ReadWavClose(wavefmt *w);
int ReadWavBuf(wavefmt *w, float **buf, int *t);


#endif