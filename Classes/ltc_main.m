//
//  ltc_main.m
//  LtcWavDecoder
//
//  Copyright 2009 Chris Beeson All rights reserved.


#import "ltc_main.h"
#include "smpte.h"
#include "wav_functions.h"

#define BUF_SIZE 32768

void ltc_main(float sampleRate, SMPTETime timecode)
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *tempDir= paths[0];
	NSString *path = [tempDir stringByAppendingPathComponent:@"chunk.wav"];
	assert(path);

    int t[] = {0, sampleRate};
	SMPTETimecode stime;
	wavefmt *w;
	float *buf = NULL;
	int buf_size;
	float seconds;
	char *filenameCStr = (char*) [path UTF8String];

	w = ReadWavInit(filenameCStr);
	
	if (w == (wavefmt *)-1) return;

	buf_size = 16000;
	
	if(buf_size > (w->ChunkSize - 50))
		buf_size = w->ChunkSize - 50;
	
	ReadWavBuf(w, &buf, t);
	
	if(ExtractSMPTEFromLTC(w->data, w->SampleRate, 30, 5000, &stime, &seconds) == 1)
	{
		//printf("frame = %d, hour = %d, min = %d, sec = %d\n",stime.frame,stime.hours,stime.mins,stime.secs);
		timecode.mHours = stime.hours;
		timecode.mMinutes = stime.mins;
		timecode.mSeconds = stime.secs;
		timecode.mFrames = stime.frame;
		timecode.mFlags = kSMPTETimeValid;
	}
	
	else
		timecode.mFlags = 999;
	
	ReadWavClose(w);
}
