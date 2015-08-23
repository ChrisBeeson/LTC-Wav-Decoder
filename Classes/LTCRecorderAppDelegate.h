//
//  LTCRecorderAppDelegate.h
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.


#import <UIKit/UIKit.h>

@class LTCRecorderViewController;

@interface LTCRecorderAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    LTCRecorderViewController *viewController;
}

@property (nonatomic, strong) IBOutlet UIWindow *window;
@property (nonatomic, strong) IBOutlet LTCRecorderViewController *viewController;

@end

