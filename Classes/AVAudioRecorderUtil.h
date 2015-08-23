//
//  AVAudioRecorderUtil.h
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.
//

// This class encapsulates AVAudioRecorder logic so that recording to a
// tmp file can be accomplished easily by the caller.

#import <Foundation/Foundation.h>

@class AVAudioRecorder;

@interface AVAudioRecorderUtil : NSObject 
{
	AVAudioRecorder *avAudioRecorder;
	NSString *path;
	float sampleRate; // sample rate (samples per second)
	BOOL isRecording;
	BOOL isResourcePath;
}

@property (nonatomic, strong) AVAudioRecorder *avAudioRecorder;
@property (nonatomic, copy) NSString *path;
@property (nonatomic, assign) float sampleRate;

@property (nonatomic, assign) BOOL isRecording;
@property (nonatomic, assign) BOOL isResourcePath;

+ (AVAudioRecorderUtil*) audioRecorderUtilWithTmpfile;

+ (AVAudioRecorderUtil*) audioRecorderUtilWithResource:(NSString*)resName;

- (void) startRecording;

- (void) stopRecording;


@end


