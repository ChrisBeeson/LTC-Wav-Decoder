//
//  LTCRecorderViewController.h
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.


#import <UIKit/UIKit.h>

@class AVAudioRecorderUtil;
@class LTCScanner;

@interface LTCRecorderViewController : UIViewController
	<UITextViewDelegate>
{
	IBOutlet UIButton *actionButton;
	IBOutlet UILabel *sizeLabel;
	IBOutlet UISwitch *useRecordedInputSwitch;
	IBOutlet UITextView *textView;
	AVAudioRecorderUtil *recorder;
	
	NSTimer *updateTimer;
	LTCScanner *scanner;
}

@property (nonatomic, retain) UILabel *sizeLabel;
@property (nonatomic, retain) UIButton *actionButton;
@property (nonatomic, retain) UISwitch *useRecordedInputSwitch;
@property (nonatomic, retain) UITextView *textView;
@property (nonatomic, retain) AVAudioRecorderUtil *recorder;
@property (nonatomic, retain) NSTimer *updateTimer;
@property (nonatomic, retain) LTCScanner *scanner;

-(IBAction) buttonPressAction:(id)sender;

- (void) startRecording;

- (void) stopRecording;

- (void) startTimer;

- (void) stopTimer;

@end

