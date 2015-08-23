//
//  LTCRecorderViewController.m
//  LTCRecorder
//
//  Created by Chris Beeson on 9/10/09.

#import "LTCRecorderViewController.h"
#import "AVAudioRecorderUtil.h"
#import "LTCScanner.h"

@implementation LTCRecorderViewController

@synthesize sizeLabel;
@synthesize actionButton;
@synthesize textView;
@synthesize recorder;
@synthesize updateTimer;
@synthesize useRecordedInputSwitch;
@synthesize scanner;


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.

- (void)viewDidLoad {
    
    [super viewDidLoad];

	sizeLabel.text = @"Size: ? bytes";

	textView.text = @"";
	textView.delegate = self;
}

// Invoked when name text input should be put away, UITextViewDelegate

//- (void)textViewDidEndEditing:(UITextView *)_textView {
//	[textView resignFirstResponder];
//}

// Yuck!

- (BOOL)textView:(UITextView *)_textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text {
	if ([text isEqualToString:@"\n"]) {
		[textView resignFirstResponder];
		return NO;
	}
	return YES;
}

- (IBAction) buttonPressAction:(id)sender
{
	if (recorder.isRecording) {
		[self stopRecording];
	} else {
		[self startRecording];
	}

	return;
}

- (void) setButtonTitle:(NSString*)label
{
//	actionButton.titleLabel.text = label;

	[actionButton setTitle:label forState:UIControlStateNormal];
}

- (void) startRecording
{
	[self setButtonTitle:@"Stop"];
	
	if (recorder == nil) {
		BOOL useRecordedInput = useRecordedInputSwitch.on;

		if (useRecordedInput) {
			NSString *resFilename;

//			resFilename = @"ASrecording.wav";
//			resFilename = @"ASrecording.caf";

			if (1) {
				resFilename = @"ASrecording.caf";
			} else {
				resFilename = @"ASrecording_8k.caf";
			}
			
				resFilename = @"chunk.wav";
			
			self.recorder = [AVAudioRecorderUtil audioRecorderUtilWithResource:resFilename];
		} else {
			self.recorder = [AVAudioRecorderUtil audioRecorderUtilWithTmpfile];
		}

		NSAssert(recorder, @"recorder is nil");
	}

	// Create scanner that will read samples from recorded audio
	// and extract LTC timecode information

	self.scanner = [LTCScanner ltcScannerWithPath:recorder.path sampleRate:recorder.sampleRate];

	// Finish up 

	[recorder startRecording];

	[self startTimer];
}

- (void) stopRecording
{
	[self setButtonTitle:@"Record"];

	[recorder stopRecording];

	[self stopTimer];

	self.recorder = nil;
}

- (NSString*) fileSize:(NSString*)path {
	NSDictionary *fattrs = [[NSFileManager defaultManager] fileAttributesAtPath:path traverseLink:FALSE];	

	NSNumber *fsize = [fattrs objectForKey:NSFileSize];

	if (fsize == nil)
		return @"NA";

	unsigned long long size = [fsize unsignedLongLongValue];

	return [NSString stringWithFormat:@"%qu", size];	
}

- (void) updateTimerCallback: (NSTimer *)timer {
	if (scanner != nil) {
		// Scan a chunk of the file

		[scanner scanChunk];

		NSMutableString *mstr = [NSMutableString stringWithCapacity:100];

		// Grab the frame to time string mapping after scanning

		NSMutableDictionary *frameData = scanner.frameData;

		NSArray *sortedFrameKeys = [[frameData allKeys] sortedArrayUsingSelector:@selector(compare:)];

		for (NSNumber *key in sortedFrameKeys) {
			NSString *value = (NSString *) [frameData objectForKey:key];

			[mstr appendFormat:@"%d\t\t%@\n", [key integerValue], value];
		}

		textView.text = [NSString stringWithString:mstr];
	}

	// Get current file size

	NSString *path = recorder.path;
	NSString *size = [self fileSize:path];
	sizeLabel.text = [NSString stringWithFormat:@"Size %@ bytes (%d packets)", size, (int)scanner.numPackets];
	
	[self stopRecording];
	
	[self performSelector:@selector(startRecording) withObject:Nil afterDelay:0.5];
	
	
}

- (void) startTimer
{
	self.updateTimer = [NSTimer timerWithTimeInterval: 0.33
											   target: self
											 selector: @selector(updateTimerCallback:)
											 userInfo: NULL
											  repeats: TRUE];

    [[NSRunLoop currentRunLoop] addTimer: updateTimer forMode: NSDefaultRunLoopMode];		
}

- (void) stopTimer
{
	[updateTimer invalidate];	
}

@end
