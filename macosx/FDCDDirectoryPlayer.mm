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
    gCDTrackList = [[NSMutableArray alloc] init];
}

FDCDDirectoryPlayer::~FDCDDirectoryPlayer()
{
    [gCDTrackList release];
}

void FDCDDirectoryPlayer::play(int theTrack, bool theLoop)
{
    gCDNextTrack = NO;
    
    if (gCDTrackList != NULL && gCDTrackCount != 0) {
        AVAudioPlayer *myMovie;
        
        // check for mismatching CD track number:
        if (theTrack > gCDTrackCount || theTrack <= 0) {
            theTrack = 1;
        }
        gCurCDTrack = 0;
        
        if (gCDController != NULL && [gCDController currentTime] != [gCDController duration]) {
            [gCDController stop];
            gCDController = NULL;
        }
        
        myMovie = [gCDTrackList objectAtIndex: theTrack - 1];
        
        if (myMovie != NULL) {
            gCDController = myMovie;
            
            if (gCDController != NULL) {
                gCurCDTrack	= theTrack;
                gCDLoop		= theLoop;
                
                [gCDController setCurrentTime:0];
                [gCDController play];
                
                //if (GetMoviesError () != noErr)
                //{
                //    CDAudio_Error (CDERR_QUICKTIME_ERROR);
                //}
            } else {
                CDAudio_Error (CDERR_MEDIA_TRACK);
            }
        } else {
            CDAudio_Error (CDERR_MEDIA_TRACK_CONTROLLER);
        }
    }

}
void FDCDDirectoryPlayer::stop()
{
    [gCDController stop];
}
void FDCDDirectoryPlayer::pause()
{
    [gCDController pause];
}
void FDCDDirectoryPlayer::resume()
{
    [gCDController play];
}

void FDCDDirectoryPlayer::update()
{
    
}

bool FDCDDirectoryPlayer::loadPath(const char *directory)
{
    return false;
}

