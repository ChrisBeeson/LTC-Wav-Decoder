//
//  LTCScanner.h
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.

// This class implements a .caf file scanner, the code will open an
// existing audio file and scan the contents for LTC encoded
// sync information.

#import <Foundation/Foundation.h>

#import <AudioToolbox/AudioFile.h>
#import <AudioToolbox/AudioToolbox.h>

@interface LTCScanner : NSObject 
{
	NSString *path;
	float sampleRate; // sample rate (samples per second)
	NSUInteger startSample;
	NSUInteger scanNumSamples;
	UInt64 numPackets;
	AudioFileID audioFile;
	NSMutableDictionary *frameData;
	SMPTETime timecode;
}

@property (nonatomic, copy) NSString *path;
@property (nonatomic, assign) float sampleRate;
@property (nonatomic, assign) NSUInteger startSample;
@property (nonatomic, assign) NSUInteger scanNumSamples;
@property (nonatomic, assign) SMPTETime timecode;

+ (LTCScanner*) ltcScannerWithPath:(NSString*)path sampleRate:(float)sampleRate;

- (void) scanChunk;

@property (NS_NONATOMIC_IOSONLY, getter=isDoneScanning, readonly) BOOL doneScanning;

- (void) open;

@property (NS_NONATOMIC_IOSONLY, readonly) OSStatus _openAudioFile;

- (void) _closeAudioFile;

- (OSStatus) _copySamplesIntoWav:(NSString*)toTmpFile;

- (void) _setDefaultAudioFormatFlags:(AudioStreamBasicDescription*)audioFormatPtr
						 numChannels:(NSUInteger)numChannels;

@property (NS_NONATOMIC_IOSONLY, getter=getNumPackets) UInt64 numPackets;

@end
