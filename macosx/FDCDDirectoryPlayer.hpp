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

@class AVAsset;

class FDCDDirectoryPlayer: public FDCDPlayer {
    UInt16				gCDTrackCount;
    UInt16				gCurCDTrack;
    NSMutableArray<AVAsset*> *gCDTrackList;
    NSMutableArray<AVAsset*> *gCDOriginalAssets;
    char					gCDDevice[PATH_MAX];
    BOOL					gCDLoop;
    BOOL					gCDNextTrack;
public:
    FDCDDirectoryPlayer();
    virtual ~FDCDDirectoryPlayer();
    
    virtual void play(int theTrack, bool theLoop);
    virtual void stop();
    virtual void pause();
    virtual void resume();
    virtual void update();
    virtual bool loadPath(const char *directory);
};

#endif /* FDCDDirectoryPlayer_hpp */
