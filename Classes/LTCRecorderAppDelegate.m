//
//  LTCRecorderAppDelegate.m
//  LTCRecorder
//
//  Copyright 2009 Chris Beeson All rights reserved.


#import "LTCRecorderAppDelegate.h"
#import "LTCRecorderViewController.h"

@implementation LTCRecorderAppDelegate

@synthesize window;
@synthesize viewController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
}




@end
