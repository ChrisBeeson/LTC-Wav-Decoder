//
//  LTCScanner.m
//  LTCRecorder
//
//  Created by Chris Beeson on 9/10/09.


#import "LTCScanner.h"
#import "ltc_main.h"

@implementation LTCScanner

@synthesize path;
@synthesize sampleRate;
@synthesize startSample;
@synthesize scanNumSamples;
@synthesize numPackets, timecode;

- (void) dealloc {
    
    [self _closeAudioFile];
}

+ (LTCScanner*) ltcScannerWithPath:(NSString*)path sampleRate:(float)sampleRate {
    
    LTCScanner *obj = [[LTCScanner alloc] init];
    
    if (obj == nil) return nil;
    
    obj.path = path;
    
    // Scan 1 second at a time
    
    obj.startSample = 0;
    obj.scanNumSamples = sampleRate/10;
    obj.sampleRate = sampleRate;
    
    return obj;
}

- (void) open
{
    OSStatus result = [self _openAudioFile];
    NSAssert(result == 0, @"_openAudioFile result");
}

- (BOOL) scan
{
    if (audioFile == NULL)
        [self open];
    
    // Copy samples to a temp file, this is a small buffer of
    // data in a format known to the LTC library
    
    //NSString *tempDir = NSTemporaryDirectory();
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *tempDir= paths[0];
    NSString *tmpFilePath = [tempDir stringByAppendingPathComponent:@"chunk.wav"];
    OSStatus status = [self _copySamplesIntoWav:tmpFilePath];
    
    if (status == 0)
    {
        // Run LTC logic with .wav file input
        
        ltc_main(self.sampleRate, self.timecode);
        /*
         @try
         {
         ltc_main(self.sampleRate, self.frameData);
         }
         @catch (NSException *exception)
         {
         NSLog(@"main: Caught %@: %@", [exception name], [exception reason]);
         }
         @finally
         {
         
         }*/
    }
    
    // No need to delete the tmp .wav, it will be recreated from
    // an empty file on the next read operation.
    
    // Update number of packets in audio file
    
    self.numPackets = [self getNumPackets];
    
    // Close and reopen the audio file, for some reason the
    // headers are not being updated when the file grows.
    
    [self _closeAudioFile];
    
    return (status == 0);
}

- (void) scan:(NSUInteger)start numSamples:(NSUInteger)numSamples
{
    self.startSample = start;
    self.scanNumSamples = numSamples;
}

- (void) scanChunk
{
    BOOL scannedSomething = [self scan];
    
    if (scannedSomething) {
        self.startSample = startSample + scanNumSamples;
    }
}

- (BOOL) isDoneScanning
{
    return FALSE; // return TRUE if at EOF?
}

// Set default channel parameters

- (void) _setDefaultAudioFormatFlags:(AudioStreamBasicDescription*)audioFormatPtr
                         numChannels:(NSUInteger)numChannels
{
    bzero(audioFormatPtr, sizeof(AudioStreamBasicDescription));
    
    audioFormatPtr->mFormatID = kAudioFormatLinearPCM;
    audioFormatPtr->mSampleRate = self.sampleRate;
    audioFormatPtr->mChannelsPerFrame = numChannels;
    audioFormatPtr->mBytesPerPacket = 2 * numChannels;
    audioFormatPtr->mFramesPerPacket = 1;
    audioFormatPtr->mBytesPerFrame = 2 * numChannels;
    audioFormatPtr->mBitsPerChannel = 16;
    audioFormatPtr->mFormatFlags = kAudioFormatFlagsNativeEndian |
    kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
}

// Open a file via AudioFileOpenURL and save as audioFile

- (OSStatus) _openAudioFile
{
    OSStatus status;
    
    NSURL *url = [NSURL fileURLWithPath:path];
    
    AudioFileID inAudioFile = NULL;
    
#ifndef TARGET_OS_IPHONE
    // Why is this constant missing under Mac OS X?
# define kAudioFileReadPermission fsRdPerm
#endif
    
    status = AudioFileOpenURL((__bridge CFURLRef)url, kAudioFileReadPermission, 0, &inAudioFile);
    if (status)
    {
        goto reterr;
    }
    
    // Verify that file contains pcm data at 44 kHz
    
    AudioStreamBasicDescription inputDataFormat;
    UInt32 propSize = sizeof(inputDataFormat);
    
    bzero(&inputDataFormat, sizeof(inputDataFormat));
    status = AudioFileGetProperty(inAudioFile, kAudioFilePropertyDataFormat,
                                  &propSize, &inputDataFormat);
    
    if (status)
    {
        goto reterr;
    }
    
    if ((inputDataFormat.mFormatID == kAudioFormatLinearPCM) &&
        (inputDataFormat.mSampleRate == self.sampleRate) &&
        (inputDataFormat.mChannelsPerFrame == 1) &&
        (inputDataFormat.mBitsPerChannel == 16) &&
        (inputDataFormat.mFormatFlags == (kAudioFormatFlagsNativeEndian |
                                          kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger))
        ) {
        // no-op when the expected data format is found
    } else {
        status = kAudioFileUnsupportedFileTypeError;
        goto reterr;
    }
    
    self->audioFile = inAudioFile;
    
reterr:
    
    if (self->audioFile == NULL && inAudioFile != NULL) {
        // Close audio file when there is an error
        OSStatus close_status = AudioFileClose(inAudioFile);
        assert(close_status == 0);
    }
    
    return status;
}

- (void) _closeAudioFile
{
    if (audioFile != NULL) {
        int close_status = AudioFileClose(audioFile);
        assert(close_status == 0);
        self->audioFile = NULL;
    }
}

// Quick hack to copy samples from a .caf to a small .wav file

- (OSStatus) _copySamplesIntoWav:(NSString*)toTmpFile {
    OSStatus status;
    
    NSURL *outURL = [NSURL fileURLWithPath:toTmpFile];
    
    AudioFileID outFile = NULL;
    
#define BUFFER_SIZE (4096 * 8)
    char *buffer = NULL;
    
    AudioStreamBasicDescription outputDataFormat;
    
    [self _setDefaultAudioFormatFlags:&outputDataFormat numChannels:1];
    
    status = AudioFileCreateWithURL((__bridge CFURLRef)outURL, kAudioFileWAVEType, &outputDataFormat,
                                    kAudioFileFlags_EraseFile, &outFile);
    if (status)
    {
        goto reterr;
    }
    
    // Read samples from source .caf and write to .wav
    
    buffer = malloc(BUFFER_SIZE);
    assert(buffer);
    
    // Calculate where (which packet) the read operation will begin on
    
    SInt64 inPacketNum = (startSample / outputDataFormat.mChannelsPerFrame / outputDataFormat.mFramesPerPacket);
    SInt64 outPacketNum = 0;
    
    UInt32 numPackets1;
    UInt32 numPackets2;
    
    UInt32 numPacketsWritten = 0;
    UInt32 packetsWritten;
    
    UInt32 numSamplesToBeRead = scanNumSamples;
    
    while (TRUE) {
        // Read a chunk of input
        
        UInt32 bytesRead;
        
        numPackets1 = BUFFER_SIZE / outputDataFormat.mBytesPerPacket;
        
        UInt32 numPacketsToBeRead = (numSamplesToBeRead / outputDataFormat.mChannelsPerFrame / outputDataFormat.mFramesPerPacket);
        
        if (numPacketsToBeRead < numPackets1)
            numPackets1 = numPacketsToBeRead;
        
        if (numPackets1 == 0) {
            break; // Done reading samples from input file
        }
        
        status = AudioFileReadPackets(audioFile,
                                      false,
                                      &bytesRead,
                                      NULL,
                                      inPacketNum,
                                      &numPackets1,
                                      buffer);
        
        if (status) {
            goto reterr;
        }
        
        // If no frames were returned, read loop is finished
        
        if (numPackets1 == 0) {
            break;
        }
        
        // Advance the "current packet" value for the file being read from
        
        inPacketNum += numPackets1;
        
        int numSamples = (numPackets1 * outputDataFormat.mBytesPerPacket) / sizeof(int16_t);
        
        numSamplesToBeRead -= numSamples;
        
        // Write pcm data to output .wav file
        
        numPackets2 = numPackets1;
        
        packetsWritten = numPackets2;
        
        status = AudioFileWritePackets(outFile,
                                       FALSE,
                                       (numPackets2 * outputDataFormat.mBytesPerPacket),
                                       NULL,
                                       outPacketNum,
                                       &packetsWritten,
                                       buffer);
        
        if (status) {
            goto reterr;
        }
        
        if (packetsWritten != numPackets2) {
            status = kAudioFileInvalidPacketOffsetError;
            goto reterr;
        }
        
        outPacketNum += packetsWritten;
        
        numPacketsWritten += packetsWritten;
    }
    
reterr:
    if (outFile != NULL) {
        OSStatus close_status = AudioFileClose(outFile);
        assert(close_status == 0);
    }
    if (buffer != NULL) {
        free(buffer);
    }
    
    if (numPacketsWritten == 0) {
        // Don't create an empty .wav file since the ltc logic can't deal with this
        [[NSFileManager defaultManager] removeItemAtPath:toTmpFile error:nil];
        printf("did not write zero length .wav\n");
        
        status = kAudioFileInvalidPacketOffsetError;
    } else {
        printf("wrote %d samples to %s\n", (unsigned int)numPacketsWritten, [[toTmpFile lastPathComponent] UTF8String]);
    }
    
    return status;
}

- (UInt64) getNumPackets {
    UInt64 value;
    UInt32 size = sizeof(value);	
    
    AudioFileGetProperty(audioFile, kAudioFileStreamProperty_AudioDataPacketCount,
                         &size, &value);
    
    return value;	
}

@end
