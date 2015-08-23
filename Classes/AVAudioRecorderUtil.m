//
//  AVAudioRecorderUtil.m
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.
//

#import "AVAudioRecorderUtil.h"

#import <AVFoundation/AVFoundation.h>

#import <CoreAudio/CoreAudioTypes.h>


@implementation AVAudioRecorderUtil

@synthesize avAudioRecorder;
@synthesize path;
@synthesize sampleRate;
@synthesize isRecording;
@synthesize isResourcePath;


- (instancetype) init
{
	self = [super init];
	
	if (self == nil)
		return nil;
/*
#if 0
	self.sampleRate = 8000.0;
#else
	self.sampleRate = 44100.0;
#endif
*/
	self.sampleRate = 22050.0;
	
	return self;
}

+ (AVAudioRecorderUtil*) audioRecorderUtilWithTmpfile
{
	AVAudioRecorderUtil *obj = [[AVAudioRecorderUtil alloc] init];

	if (obj	== nil)
		return nil;

	NSString *path;

	if (FALSE) {
		path = @"/dev/null"; // toss recorded data
	} else {
		NSString *tempDir = NSTemporaryDirectory();
		path = [tempDir stringByAppendingPathComponent:@"sound.caf"];
	}

	obj.path = path;

	if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
		[[NSFileManager defaultManager] removeItemAtPath:path error:nil];
	}

	NSURL *url = [NSURL fileURLWithPath:path];

  	NSError *error;

	// 44kHz, mono, linear PCM

	int numChannels = 1;

	#if TARGET_IPHONE_SIMULATOR
		// The simulator seems to explode if you try to record mono audio from
		// the mic. So, tell the system to record in stereo and ignore the
		// right channel

		numChannels = 2;
	#endif

	NSAssert(obj.sampleRate != 0, @"sampleRate");

  	NSDictionary *settings = @{AVSampleRateKey: @(obj.sampleRate),
							  AVFormatIDKey: @(kAudioFormatLinearPCM),
							  AVNumberOfChannelsKey: @(numChannels),
							  AVEncoderAudioQualityKey: @(AVAudioQualityMax)};

	obj.avAudioRecorder = [[AVAudioRecorder alloc] initWithURL:url settings:settings error:&error];
	NSAssert(obj.avAudioRecorder, @"avAudioRecorder is nil");

	AVAudioSession *audioSession = [AVAudioSession sharedInstance];

	BOOL audioHWAvailable = audioSession.inputIsAvailable;

	NSAssert(audioHWAvailable, @"audioHWAvailable");

// FIXME: Fixup delagate logic so that interruption works properly
//    audioSession.delegate = self;

    [audioSession setActive: YES error: nil];	

	return obj;
}

+ (AVAudioRecorderUtil*) audioRecorderUtilWithResource:(NSString*)resName
{
	AVAudioRecorderUtil *obj = [[AVAudioRecorderUtil alloc] init];
	
	if (obj	== nil)
		return nil;

	obj.isResourcePath = TRUE;

	NSBundle* appBundle = [NSBundle mainBundle];

	NSString* resPath = [appBundle pathForResource:resName ofType:nil];	

	obj.path = resPath;

	return obj;
}

- (void) startRecording
{
	isRecording = TRUE;

	[avAudioRecorder prepareToRecord];

	[avAudioRecorder record];
}

- (void) stopRecording
{
	isRecording = FALSE;

	[avAudioRecorder stop];
}


// FIXME: Audio recorder should implement delagate methods and respond to interruptions correctly

@end
