//
//  LTCDecoder.h
//  LTCRecorder
//
//  Created by Chris Beeson on 02/10/2009.


#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "LTCDecoder.h"
#import "AVAudioRecorderUtil.h"
#import "LTCScanner.h"


@class AVAudioRecorderUtil;
@class LTCScanner;

@interface LTCDecoder : NSObject 

{
	id __weak delegate;
	
	AVAudioRecorderUtil *recorder;
	LTCScanner *scanner;
	double startTime;
	
	BOOL active;
}

@property (nonatomic,weak) id delegate;
@property (nonatomic, strong) AVAudioRecorderUtil *recorder;

@property (nonatomic, strong) LTCScanner *scanner;
@property (nonatomic,assign) BOOL active;
@property (nonatomic,assign) double startTime;

+ (LTCDecoder *)sharedInstance;

- (void) startRecording;
- (void) stopRecording;

@end
