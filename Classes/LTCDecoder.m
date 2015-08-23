//
//  LTCDecoder.m
//  LTCRecorder
//
//  Created by Chris Beeson on 02/10/2009.


#import "LTCDecoder.h"

static LTCDecoder *sharedLTCDelegate = nil;

@implementation LTCDecoder

@synthesize delegate;
@synthesize recorder;
@synthesize scanner;
@synthesize active, startTime;

- (void) startRecording
{
	if (recorder == nil) 
	{
		self.recorder = [AVAudioRecorderUtil audioRecorderUtilWithTmpfile];
	}
		
	// Create scanner that will read samples from recorded audio
	// and extract LTC timecode information
	
	self.scanner = [LTCScanner ltcScannerWithPath:recorder.path sampleRate:recorder.sampleRate];
	
	self.startTime = [NSDate timeIntervalSinceReferenceDate];
	
	[recorder startRecording];
	
	[self performSelector:@selector(process) withObject:Nil afterDelay:0.4];
}


- (void) stopRecording
{
	if(recorder !=nil)
	{
		[recorder stopRecording];
		self.recorder = nil;
	}
}

- (NSString*) fileSize:(NSString*)path 
{
    NSDictionary *fattrs = [[NSFileManager defaultManager] attributesOfItemAtPath:path error: nil];
	NSNumber *fsize = fattrs[NSFileSize];
	
	if (fsize == nil) return @"NA";
	
	unsigned long long size = [fsize unsignedLongLongValue];
	
	return [NSString stringWithFormat:@"%qu", size];	
}

- (void) process
{
	[self stopRecording];	
	
	if (scanner != nil) 
	{
		// Scan a chunk of the file
		[scanner scanChunk];
		
        //	double offset = [NSDate timeIntervalSinceReferenceDate] - startTime;
		
		if (scanner.timecode.mFlags ==  kSMPTETimeValid)
		{
		//	[delegate timecodeStampNow:scanner.time withOffset:offset];
		}
		else {
            
            NSAssert(nil,@"Timecode has error");
            //	[delegate timecodehaserror];
		}

	}
	
	// Get current file size
	/*
	NSString *path = recorder.path;
	NSString *size = [self fileSize:path];
	sizeLabel.text = [NSString stringWithFormat:@"Size %@ bytes (%d packets)", size, (int)scanner.numPackets];
	 */
	
	if (active) 
	{
		[self performSelector:@selector(startRecording) withObject:Nil afterDelay:0.5];
	}
}

-(void) setActive:(BOOL) value
{
	active = value;
	
	if(active) 
	{
		[self startRecording];
	}
	else 
	{
		[self stopRecording];
	}
}


#pragma mark ---- singleton object methods ----

// See "Creating a Singleton Instance" in the Cocoa Fundamentals Guide for more info

+ (LTCDecoder *)sharedInstance 
{
    @synchronized(self) 
	{
        if (sharedLTCDelegate == nil) 
		{
            [[self alloc] init]; // assignment not done here
        }
    }
    return sharedLTCDelegate;
}

+ (id)allocWithZone:(NSZone *)zone 
{
    @synchronized(self) 
	{
        if (sharedLTCDelegate == nil) 
		{
            sharedLTCDelegate = [super allocWithZone:zone];
            return sharedLTCDelegate;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}





@end
