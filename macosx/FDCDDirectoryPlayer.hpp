//
//  FDCDDirectoryPlayer.hpp
//  Quake II
//
//  Created by C.W. Betts on 5/13/16.
//
//

#ifndef FDCDDirectoryPlayer_hpp
#define FDCDDirectoryPlayer_hpp

#import <Cocoa/Cocoa.h>
#import <CoreAudio/AudioHardware.h>
#include "FDCDPlayer.hpp"

@class AVAudioPlayer;

class FDCDDirectoryPlayer: public FDCDPlayer {
    UInt16				gCDTrackCount;
    UInt16				gCurCDTrack;
    NSMutableArray<AVAudioPlayer*> *gCDTrackList;
    __unsafe_unretained AVAudioPlayer *gCDController;
    NSString            *CDDevice;
    BOOL				gCDLoop;
    BOOL				gCDNextTrack;
    
    void setCDDevice(NSString *str)
    {
        if ([CDDevice isEqualToString:str]) {
            return;
        }
        [str retain];
        [CDDevice release];
        CDDevice = str;
    }
    
public:
    FDCDDirectoryPlayer();
    virtual ~FDCDDirectoryPlayer();
    
    virtual void play(int theTrack, bool theLoop);
    virtual void stop();
    virtual void pause();
    virtual void resume();
    virtual void update();
    virtual bool loadPath(const char *directory);

#pragma mark getters
    virtual int currentTrackNumber();
    virtual int totalTrackNumber();
    virtual const char *devicePath();
    virtual bool isPlaying();
};

#endif /* FDCDDirectoryPlayer_hpp */
