#include "smpte.h"
#include "wav_functions.h"

/*********************************************************************************
 Function Name: 
 ExtractSMPTEFromLTC
 Function Syntax:
 int ExtractSMPTEFromLTC(void *data, int sampleRate, 
 float frameRate, int size, SMPTETime *stime, float *seconds)
 Function Description:
 Extracts the smpte time code form the raw wav data.
 Parameters:
 >Input:
 data = pointer to the wavedata stream
 sampleRate = sampling frequency of the .wav file
 frameRate = frame rate of the .wav file
 size = No of samples to be read from the input buf (data), note this is not number of bytes
 but it is the number of samples in time of the wav data
 stime = pointer the time stamp
 seconds = pointer the time instant of the first occurance of smpte frame
 >Outputs/return values:
 1 when a smpte frame is found successfully
 0 when no frame is found
 ********************************************************************************/
int ExtractSMPTEFromLTC(float *data, int sampleRate, 
						float frameRate, int size, SMPTETimecode *stime, float *seconds)
{
	unsigned char *frame_bits = NULL;
	SMPTEFrame *frame;
	float F0, N;
	F0 = frameRate*BITS_PER_FRAME;    // Frequency of a train of '0's = 2.4kHz for NTSC
    N = sampleRate/F0 ;              // Duration of one symbol, in samples
	if(size < 2*BITS_PER_FRAME*N)
	{
		printf("Not enough samples to decode one full frame\n");
		return 0;
	}
	frame_bits = decode_sync_correlation((float *)data , sampleRate , 
										 size, seconds, frameRate);
	if(frame_bits == NULL)
		return 0;
	frame = (SMPTEFrame *)malloc(sizeof(SMPTEFrame));
	SMPTEDecoderRead(frame_bits, frame);
	SMPTEFrameToTime(frame, stime);
	free(frame);
	free(frame_bits);
	return 1;
}
/*********************************************************************************
 Function Name: 
 SMPTEDecoderRead
 Function Syntax:
 SMPTEDecoderRead(unsigned char *bits, SMPTEFrame *frame)
 Function Description:
 Reads the relevant bit information on to the smpte frame structure
 Parameters:
 >Input:
 bits = pointer the 80 bit smpte frame
 frame = contains the relevant information of the smpte frame
 >Outputs/return values:
 always returns 1
 ********************************************************************************/
int SMPTEDecoderRead(unsigned char *bits, SMPTEFrame *frame)
{
	//int frame_len = BITS_PER_FRAME;
	memset(frame, 0, sizeof(SMPTEFrame));
	memcpy(frame, bits, 10);
	
	return 1;
}
/*********************************************************************************
 Function Name: 
 SMPTEFrameToTime
 Function Syntax:
 SMPTEFrameToTime(SMPTEFrame* frame, SMPTETime* stime)
 Function Description:
 Converts the smpte frame information into time stamp
 Parameters:
 >Input:
 frame = contains the relevant information of the smpte frame
 stime = pointer to the time stamp info
 >Outputs/return values:
 always returns 1
 ********************************************************************************/

int SMPTEFrameToTime(SMPTEFrame* frame, SMPTETimecode* stime)
{
	stime->hours = frame->hoursUnits + frame->hoursTens*10;
	stime->mins = frame->minsUnits + frame->minsTens*10;
	stime->secs = frame->secsUnits + frame->secsTens*10;
	stime->frame = frame->frameUnits + frame->frameTens*10;
	return 1;
}
/**********************************************************************************
 Function Name: 
 decode_sync_correlation
 Function Syntax:
 unsigned char *bits = decode_sync_correlation(int *x, int Fs, int size, float *idx
 float frameRate)
 Function Description:
 Decoder based on correlation with sync word of the smpte frame
 Parameters:
 >Input:
 x = pointer to the input stream
 Fs = Sampling frequency
 size = size of the input stream
 idx = pointer to the index where each frame starts
 frameRate = frame rate of the input stream
 >Outputs:
 frame_bits = pointer to the smpte frame
 >NOTE-- 
 ********************************************************************************/
unsigned char *decode_sync_correlation(float *x, int Fs, int size, float *idx, float frameRate)
{
	float F0, N, N_frame;
	float i_frame_start = 0;
	float i = 0;
	float i_sync_start, i_sync_end;
	//int total_frame_bits =0;
	unsigned char *frame_bits = NULL;
	float temp;
	F0 = frameRate * BITS_PER_FRAME;    // Frequency of a train of '0's = 2.4kHz for NTSC
    N = Fs/F0 ;							// Duration of one symbol, in samples
    N_frame = BITS_PER_FRAME;           // 80 bits in one frame
	
	i = 0;
	
	while( i < size)
	{
		i_sync_start = find_next_sync_word(x, Fs, i, size, frameRate );
		if((i_sync_start + 2*N + 300) > size)
		{
			printf("Not enough samples to decode the smpte frame\n");
            return frame_bits;
		}
		i_sync_end   = max_sync_correlation(x, Fs, i_sync_start, frameRate);
		temp = i_sync_end - i_frame_start - N_frame*N;
		if((LtcABS(temp) < N_frame*N/2) && (temp > 4*N))
		{
			// A frame is between the end of the previous sync word ..
			// .. and the end of this sync word
			// The frame is decoded by splitting it into 80 equal bits
			frame_bits = (unsigned char *)malloc( (BITS_PER_FRAME/8) * sizeof(char) );
			if(!decode_monostable( (x+(int)floor(i_frame_start-2*N)), frame_bits, Fs, 
								  (int)(floor(i_sync_end-i_frame_start+1)+2*N), frameRate))
			{
				free(frame_bits);
				frame_bits = 0;
			}
			*idx = i_frame_start;
			//   printf("%f\n",*(idx));
			return frame_bits;
		}
		i_frame_start = i_sync_end;
		i = i_sync_end + 1;
    }
	return frame_bits;
}
/************************************************************************************
 Function Name: 
 max_sync_correlation
 Function Syntax:
 float *sync_end = max_sync_correlation(int *x, int Fs,float i_sync_start0, float framerate )
 Function Description:
 Find the end of the sync by finding max correlation:
 Correlate signal with the ideal sync word
 The beginning of the sync word coincides with the
 place where the correlation is the largest (can be + or -)
 Parameters:
 >Input:
 x = pointer to the input stream
 Fs = sampling frequency
 i_sync_start0 = sync start obtained from find_next_sync_word()(initial sync start)
 frame rate = frame rate of the wave file
 >Outputs:
 sync_end = index of the end of the sync_word
 >NOTE-- 
 
 ********************************************************************************/
float max_sync_correlation(float *x, int Fs,float i_sync_start0, float frameRate )
{
    float F0, N, sync_end;
    float temp;
    int *signal_size1, *sync0, i, t;
    float mean, prod, c_max;
    int span_start, span_end;
    float *xc, *c;
    int i_max;
	
    signal_size1 = (int *)malloc(sizeof(int));
	F0 = frameRate * BITS_PER_FRAME;               // Frequency of a train of '0's = 2.4kHz
    
    N = (float)Fs/F0;         // Duration of one symbol, in samples
	
    sync0 = sync_word(Fs, signal_size1, frameRate);
	
    span_start = (int)floor(i_sync_start0 - 2*N) ;
    span_end = (int)floor(i_sync_start0 + 2*N) ;
    xc = (float *)malloc((*signal_size1)*sizeof(float));
    c = (float *)malloc(((int)round(span_end - span_start) + 1)* sizeof(float));
    for( i = span_start-1; i< span_end ;i++ )
    {
        mean=0;prod=0;
        for(t=0;t<*signal_size1;t++)
            mean += x[i+t];
        mean = mean/(*signal_size1);
        for(t=0; t<*signal_size1; t++)
        {
            temp = *(x+i+t);
            *(xc+t) = temp*1.0f-mean;
			// printf("%d\t",*(x+t+i));
        }
        for(t=0; t < *signal_size1 ;t++)
            prod = prod + xc[t]*sync0[t];
        c[i+1-span_start] = prod;
    }
    c_max = LtcABS(c[0]);
    i_max = 1;
    for( i = 1; i< span_end-span_start+1 ;i++ )
    {
    	if(c_max < (LtcABS(c[i])))
    	{
    		c_max = LtcABS(c[i]);
    		i_max = i+1;
    	}
    }
    
    // i_max points to the point of highest correlation.
    // If the initial guess at the begining of the sync word (i_sync_start0)
    // is correct, then [i_max] points to the center of [span] array (round(2*N)+1)
    // Finally add the length of the sync word to return pointer to the end of
    // the detected sync word.
	
    sync_end = i_sync_start0 + i_max + *signal_size1;
	free(signal_size1);
	free(xc);
	free(c);
    return sync_end;
}
/************************************************************************************
 Function Name: 
 find_next_sync_word
 Function Syntax:
 float sync_start = find_next_sync_word(float *x,int Fs,int i,int size );
 Function Description:
 The start of the sync word 001111... is detected
 Parameters:
 >Input:
 x = pointer to the input stream to detect the sync word
 Fs = sampling frequency
 i = start from where you want to detect sync word
 size = Total size of the vector x
 >Outputs:
 sync_start = Index of the sync start word
 >NOTE-- 
 
 ********************************************************************************/
float find_next_sync_word(float *x, int Fs, float i, int size, float frameRate )
{
	int bit;
	int consecutive_ones, count, s;
    float F0, sync_start, N;
    float t1, t2;
	F0 = frameRate * BITS_PER_FRAME; // Frequency of a train of '0's = 2.4kHz
    N = (float)Fs/F0;               // Duration of one symbol, in samples
    consecutive_ones = 12;   // Consecutive '1's to detect the sync word
    count = 0;               // Counter of consecutive '1's
    sync_start = -1;         // Index of the sync_word_start
	
	s = SIGN(x[(int)floor(i)]);

	while(i < size)
	{
		if(SIGN(x[(int)floor(i)]) != s)
		{
            t1 = i + 0.25f*N;
            t2 = i + 0.75f*N;
			
			bit = 1 * (SIGN(x[(int)floor(t1)]) != SIGN(x[(int)floor(t2)]));
			if(bit == 1)
			{
				count++;
				if(count == 1)
                    sync_start = i*1.0f;
				if(count == consecutive_ones)
					break;
			}
			else
				count = 0;
			i = (i + 0.75f*N);
            s = SIGN(x[(int)floor(i)]);
		}
		i++;
	}
	
	
	if(sync_start==-1)       // If the next sync word not found
		sync_start = 1.0f*size + 1;
    else
        // Sync word is "00 11111...". The algorithm above detects only a train of "11111..."
        // Start of sync word is moved back by 2 leading zeros.
        sync_start = sync_start - 2*N;
	
	return sync_start;
}

//
// This algorithm decodes Biphase Mark modulation
// by triggering off an edge (rising or falling)
// and ignoring the any transitions for the next 3/4*T.
// Properties of the modulation are such that this
// algorithm is self-synchronizing.
//

int decode_monostable(float *x, unsigned char *bits, int Fs, int size, float frameRate )
{
	float F0, N;
	//float i_frame_start = 0;
	int k=0, s;
	int bit;
	int flag = 0;
	//int total_frame_bits =0;
	float frame_duration, threshold, KT, last_frame, i;
	int t1, t2, t3, bit_num = 0;
	F0 = frameRate * BITS_PER_FRAME;           // Frequency of a train of '0's = 2.4kHz
    N = Fs/F0 ;              // Duration of one symbol, in samples
   	memset(bits, 0, BITS_PER_FRAME/8); 
	i = 0;
	
	
	s = SIGN(x[0]);
    
	// Setting the hysteresis threshold relative to the standard deviation of the signal
    frame_duration = BITS_PER_FRAME * N + 2*N;
    KT = (float)1/2;
    threshold = KT * var(x,frame_duration);
    last_frame = frame_duration;
	if( size < frame_duration )
		return 0;
	
	s = 1;
	i = 1;
	k = 0;
    while( i < (size ) )
	{
		t1 = (int)floor(i);
		if( ((s>0) && (x[t1]<-1*threshold)) || ((s<0) && (x[t1]>threshold)) )
		{
			bit_num++;
			t2 = (int)floor(i+0.25f*N);
			t3 = (int)floor(i+0.75f*N);
			bit = 1 * ( SIGN(x[t2]) != SIGN(x[t3]) );
			if(bit_num > 2)
			{
				int temp;
				
				temp = (int)(k/8);
				bits[temp] = bits[temp] | bit<<(k%8);
				k++;
				//	printf("%d  ",bit);
				//	if((k%8) == 0)
				//		printf("\n");
				//    idx  = [idx,  i];
			}
            i = (i + 0.75f*N);
            s = SIGN(x[(int)floor(i)]);
			
            if( i > last_frame && ((i + frame_duration) < size ))
			{
				threshold = KT * var((x+(int)last_frame),frame_duration);
				last_frame = last_frame + frame_duration;
			}
			flag = 1;
		}
        i = i+1;
	}
	printf("\n");
	return flag;
}
/************************************************************************************
 Function Name: 
 sync_word
 Function Syntax:
 int *signal = sync_word(int Fs, int *signal_size)
 Function Description:
 generate time code for sync_bits[] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
 Parameters:
 >Input:
 Fs = sampling frequency
 signal_size = size of the generated time code signal
 >Outputs:
 signal = pointer to the generated time code signal
 >NOTE-- 
 
 ********************************************************************************/
int *sync_word(int Fs, int *signal_size, float frameRate)
{
    int *signal, i;
    int sync_bits[] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
    signal = generate_timecode(sync_bits, Fs, 16,  signal_size, frameRate);
    for(i=0; i<*signal_size; i++)
        signal[i] = -1*signal[i];
    return signal;
}

float var(float *x, float size)
{
	float mean = 0.0f, var =0.0f;
	int i;
	//	size = (int)floor(size);
	for(i = 0; i < size; i++)
		mean += x[i];
	mean = mean/size;
	for(i = 0; i < size; i++)
		var += (x[i] - mean);
	var = var/size;
	return var;
}

float maximum(float *x, int size)
{
	float maxi = 0;
	int i;
	maxi = *x;
	for(i=0; i < size; i++)
	{
		if(*(x + i)>maxi)
			maxi = x[i];
	}
	return maxi;
}
/************************************************************************************
 Function Name: 
 generate_timecode
 Function Syntax:
 int *x = generate_timecode(int *bits, int Fs, int size, int *signal_size, float framerate)
 Function Description:
 Generates time code for the input data pointed by bits.
 Parameters:
 >Input:
 bits = pointer to array(vector) of 1s and 0s
 Fs = sampling frequency of the input data. Fs = 22050(audio file sampling frequency)
 size = size of the input data (No of bits in bits array)
 signal_size = the pointer storing the size of the array pointed by x (the number of bits generated).
 >Outputs:
 x = pointer to the time code generated.
 >NOTE-- 
 American NTSC standard uses 30 frames per seconds.
 SMPTE frame contains 80 bits
 Total: 30 x 80 = 2400 bits per second =2400 baud
 Train of zeros = 2400Hz
 Train of ones  = 4800Hz
 ********************************************************************************/
int *generate_timecode(int *bits, int Fs, int size, int *signal_size, float frameRate)
{
	int sign_prev, i;
	int bit, t, j=0;
	int *x;
	float F0, N, t0, prev_i, F1;
    int temp = 0, temp1, temp2;
	
    F0 = frameRate * BITS_PER_FRAME;      // Frequency of a train of '0's = 2.4kHz
    F1 = 2*F0;	   // Frequency of a train of '1's = 4.8kHz
    N = 2*(float)Fs/F1;   // number of samples in one bit
	
    sign_prev = -1;
    prev_i = 0;
	x = (int *)malloc(sizeof(int));
    for( i = 0 ; i < size ; i++)
	{
		
		t0 = (i + 1)*N;
        bit = bits[i];
        if( bit == 1 )
		{
            temp1 = (int)round(t0-N/2)-(int)round(prev_i+1)+1;
            temp2 = (int)round(t0)-(int)round(t0-N/2+1)+1;
            temp =j + temp1 + temp2;
            //printf("%d\n", (int)temp);
			x = realloc(x, sizeof(int)*temp);
			
			for(t=(int)round(prev_i+1); t<=(int)round(t0-N/2); t++)
            {
				*(x + j) = -sign_prev;
                j++;
            }
            prev_i = t0-N/2;
           	for(t=(int)round(prev_i+1); t <= (int)round(t0); t++)
            {
				*(x + j) = sign_prev;
                j++;
            }
		}
        else
		{
            // '0' has single transition and changes sign
            temp =j + (int)round(t0)-(int)round(prev_i);
            //printf("%d\n",(int)temp);
			x = realloc(x, sizeof(int)*(int)temp);
           	for(t=(int)round(prev_i); t < (int)round(t0); t++)
            {
           		*(x + j) = -sign_prev;
                j++;
            }
            sign_prev = -sign_prev;
		}
        prev_i = t0;
	}
    *signal_size = j;
    return x;
}