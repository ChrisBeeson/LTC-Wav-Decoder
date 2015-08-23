#ifndef SMPTE_H_
#define SMPTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <sys/types.h>
#define round(x) ((x)<0?ceil((x)-0.5):floor((x)+0.5))
#define SIGN(x) ((x)<0?-1:1&&(x))
#define LtcABS(x) ((x)<0?-1.0f*(x):(x))
#define BITS_PER_FRAME 80
#ifdef __BIG_ENDIAN__
// Big Endian version, bytes are "upside down"
typedef struct SMPTEFrame {
	unsigned int user1:4;
	unsigned int frameUnits:4;
	
	unsigned int user2:4;
	unsigned int colFrm:1;
	unsigned int dfbit:1;
	unsigned int frameTens:2;
	
	unsigned int user3:4;
	unsigned int secsUnits:4;
	
	unsigned int user4:4;
	unsigned int biphaseMarkPhaseCorrection:1;
	unsigned int secsTens:3;
	
	unsigned int user5:4;
	unsigned int minsUnits:4;
	
	unsigned int user6:4;
	unsigned int binaryGroupFlagBit1:1;
	unsigned int minsTens:3;
	
	unsigned int user7:4;
	unsigned int hoursUnits:4;
	
	unsigned int user8:4;
	unsigned int binaryGroupFlagBit2:1;
	unsigned int reserved:1;
	unsigned int hoursTens:2;
	
	unsigned int syncWord:16;
} SMPTEFrame;

#else
// Little Endian version (default)
typedef struct SMPTEFrame {
	unsigned int frameUnits:4;
	unsigned int user1:4;
	
	unsigned int frameTens:2;
	unsigned int dfbit:1;
	unsigned int colFrm:1;
	unsigned int user2:4;
	
	unsigned int secsUnits:4;
	unsigned int user3:4;
	
	unsigned int secsTens:3;
	unsigned int biphaseMarkPhaseCorrection:1;
	unsigned int user4:4;
	
	unsigned int minsUnits:4;
	unsigned int user5:4;
	
	unsigned int minsTens:3;
	unsigned int binaryGroupFlagBit1:1;
	unsigned int user6:4;
	
	unsigned int hoursUnits:4;
	unsigned int user7:4;
	
	unsigned int hoursTens:2;
	unsigned int reserved:1;
	unsigned int binaryGroupFlagBit2:1;
	unsigned int user8:4;
	
	unsigned int syncWord:16;
} SMPTEFrame;

#endif
/* Extended SMPTE frame
 position is the position of the end of the SMPTE frame in the incoming
 sample stream, feeded to SMPTEDecoderWrite
 */ 
typedef struct SMPTEFrameExt
	{
        SMPTEFrame base;
		struct SMPTEFrameExt *next;
        int position;
	}SMPTEFrameExt;
typedef struct SMPTEDecoder
	{
        int sampleRate;
        int fps;
		
        SMPTEFrameExt* queue;
        int queueSize;
        int queueReadPos;
        int queueWritePos;
		
        int position;
		
        /* ... other internal variables needed by low level decoding
		 functions ... */
	}SMPTEDecoder; 
/**
 * Human readable time representation
 */
typedef struct SMPTETimecode {
	//these are only set when compiled with ENABLE_DATE
	char timezone[6];
	unsigned char years;
	unsigned char months;
	unsigned char days;
	// 
	unsigned char hours;
	unsigned char mins;
	unsigned char secs;
	unsigned char frame;
}SMPTETimecode;

//bits is a frame of 80 bits
int SMPTEDecoderRead(unsigned char *bits, SMPTEFrame *frame);

int SMPTEFrameToTime(SMPTEFrame* frame, SMPTETimecode* stime);

int ExtractSMPTEFromLTC(float *data, int sampleRate, 
						float frameRate, int size, SMPTETimecode *stime, float *seconds);


unsigned char *decode_sync_correlation(float *x, int Fs, int size, float *idx, float frameRate);
int decode_monostable(float *x, unsigned char *bits, int Fs, int size, float frameRate );
float max_sync_correlation(float *, int ,float, float  );
float var(float *x, float size);
float find_next_sync_word(float *,int ,float ,int, float );
int *sync_word(int Fs, int *signal_size, float frameRate);
int *generate_timecode(int *bits, int Fs, int size, int *signal_size, float frameRate);
float maximum(float *x, int size);

#endif /* SMPTE_H_ */