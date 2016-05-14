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
    [gCDController stop];
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

#import "Quake2.h"

bool FDCDDirectoryPlayer::loadPath(const char *directory)
{
    NSFileManager *	myFileManager = [NSFileManager defaultManager];
    NSArray *theExtensions = @[ @"mp3", @"mp4", @"m4a"];
    NSString *theMountPath = [myFileManager stringWithFileSystemRepresentation:directory length:strlen(directory)];
    
    if (myFileManager != NULL)
    {
        NSDirectoryEnumerator *	myDirEnum = [myFileManager enumeratorAtPath: theMountPath];
        
        if (myDirEnum != NULL)
        {
            NSString *	myFilePath;
            
            // get all audio tracks:
            while ((myFilePath = [myDirEnum nextObject]) && [[NSApp delegate] abortMediaScan] == NO) {
                for (NSString *ext in theExtensions) {
                    if ([[myFilePath pathExtension] isEqualToString: ext]) {
                        NSString *	myFullPath	= [theMountPath stringByAppendingPathComponent: myFilePath];
                        NSURL *		myMoviePath	= [NSURL fileURLWithPath: myFullPath];
                        AVAudioPlayer *myMovie	= [[AVAudioPlayer alloc] initWithContentsOfURL: myMoviePath error:NULL];
                        
                        if (myMovie != nil) {
                            // add only movies with audiotacks and use only the audio track:
                            if ((1)/*CDAudio_StripVideoTracks (myQTMovie) > 0*/) {
                                //myMovie
                                [gCDTrackList addObject: myMovie];
                            } else {
                                CDAudio_Error (CDERR_AUDIO_DATA);
                            }
                        } else {
                            CDAudio_Error (CDERR_MOVIE_DATA);
                        }
                    }
                }
            }
        }
    }
    
    gCDTrackCount = [gCDTrackList count];
    return gCDTrackCount != 0;
}

