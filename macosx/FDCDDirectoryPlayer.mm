//
//  FDCDDirectoryPlayer.cpp
//  Quake II
//
//  Created by C.W. Betts on 5/13/16.
//
//

#include "FDCDDirectoryPlayer.hpp"
#import <AVFoundation/AVFoundation.h>

FDCDDirectoryPlayer::FDCDDirectoryPlayer()
{
    
}

FDCDDirectoryPlayer::~FDCDDirectoryPlayer()
{
    [gCDTrackList release];
    [gCDOriginalAssets release];
}

void FDCDDirectoryPlayer::play(int theTrack, bool theLoop)
{
    
}
void FDCDDirectoryPlayer::stop()
{
    
}
void FDCDDirectoryPlayer::pause()
{
    
}
void FDCDDirectoryPlayer::resume()
{
    
}
void FDCDDirectoryPlayer::update()
{
    
}
bool FDCDDirectoryPlayer::loadPath(const char *directory)
{
    return false;
}

