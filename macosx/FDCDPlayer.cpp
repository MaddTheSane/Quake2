//
//  FDCDPlayer.cpp
//  Quake II
//
//  Created by C.W. Betts on 5/13/16.
//
//

#include "CDPlayer.h"
#include "FDCDPlayer.hpp"
#include <CoreServices/CoreServices.h>

cvar_t *FDCDPlayer::cd_volume = NULL;
FDCDPlayer *FDCDPlayer::player = NULL;

using namespace SDL2CDPlayerInternal;

static OSErr FSGetVRefNum(const FSRef *ref,
                          FSVolumeRefNum &vRefNum)
{
    OSErr			result;
    FSCatalogInfo	catalogInfo;
    
    /* get the volume refNum from the FSRef */
    result = FSGetCatalogInfo(ref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL);
    require_noerr(result, FSGetCatalogInfo);
    
    /* return volume refNum from catalogInfo */
    vRefNum = catalogInfo.volume;
    
FSGetCatalogInfo:
    
    return ( result );
}

FDCDCDPlayer::FDCDCDPlayer() : tracks(NULL), nextTrackFrame(0), nextTrackFramesRemaining(0)
{
    theCD = new SDL2_CD();
    memset(theCD, 0, sizeof(SDL2_CD));
    theCD->status = CD_TRAYEMPTY;
    memset(gCDDevice, 0, sizeof(gCDDevice));
    SetCompletionProc(callBack, theCD, this);
}

void FDCDCDPlayer::play(int theTrack, bool theLoop)
{
    if (theTrack > (theCD->numtracks)) {
        theTrack = 0;
    }
    SDL2CDPlayerInternal::Lock();
    int startFrame = 0, stopFrame = 0;
    FSRef * aFile = GetFileForOffset(theTrack, theCD->track[theTrack].length, startFrame, stopFrame);
    looping = theLoop;
    if (LoadFile(aFile, startFrame, stopFrame) < 0) {
        PlayFile();
        theCD->status = CD_PLAYING;
    } else {
        
    }
    SDL2CDPlayerInternal::Unlock();
}

void FDCDCDPlayer::stop()
{
    theCD->status = CD_STOPPED;
}

void FDCDCDPlayer::pause()
{
    SDL2CDPlayerInternal::Lock();
    theCD->status = CD_PAUSED;
    PauseFile();
    SDL2CDPlayerInternal::Unlock();
}

void FDCDCDPlayer::resume()
{
    SDL2CDPlayerInternal::Lock();
    PlayFile();
    theCD->status = CD_PLAYING;
    SDL2CDPlayerInternal::Unlock();
}

void FDCDCDPlayer::update()
{
    SDL2CDPlayerInternal::Lock();

    SDL2CDPlayerInternal::Unlock();
}

bool FDCDCDPlayer::loadPath(const char *directory)
{
    safePath(directory);
    CFURLRef aURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)directory, strlen(directory), true);
    FSRef pathRef;
    FSVolumeRefNum vRef;
    if (!CFURLGetFSRef(aURL, &pathRef)) {
        CFRelease(aURL);
        goto failure;
    }
    CFRelease(aURL);
    if (FSGetVRefNum(&pathRef, vRef) != noErr) {
        goto failure;
    }
    if (ReadTOCData(vRef, theCD) != 0) {
        goto failure;
    }
    tracks = new FSRef[theCD->numtracks];
    //cdrom->numtracks
    if (ListTrackFiles (vRef, tracks, theCD->numtracks) < 0) {
        goto failure;
    }

    theCD->status = CD_STOPPED;
    return true;
    
    
failure:
    return false;
}

FSRef* FDCDCDPlayer::GetFileForOffset(int start, int length,  int &outStartFrame, int &outStopFrame)
{
    int i;
    
    for (i = 0; i < theCD->numtracks; i++) {
        
        if (theCD->track[i].offset <= start &&
            start < (theCD->track[i].offset + theCD->track[i].length))
            break;
    }
    
    if (i == theCD->numtracks)
        return NULL;
    
    theCD->cur_track = i;
    
    outStartFrame = start - theCD->track[i].offset;
    
    if ((outStartFrame + length) < theCD->track[i].length) {
        outStopFrame += length;
        length = 0;
        nextTrackFrame = -1;
        nextTrackFramesRemaining = -1;
    } else {
        outStopFrame = -1;
        length -= theCD->track[i].length - outStartFrame;
        nextTrackFrame = theCD->track[i+1].offset;
        nextTrackFramesRemaining = length;
    }
    
    return &tracks[i];
}

FDCDCDPlayer::~FDCDCDPlayer()
{
    SetCompletionProc(NULL, NULL, NULL);

    delete theCD;
    if (tracks) {
        delete [] tracks;
    }
    
    ReleaseFile();
}

void FDCDCDPlayer::safePath(const char *thePath)
{
    size_t	myStrLength = 0;
    
    if (thePath != NULL) {
        size_t		i;
        
        myStrLength = strlen (thePath);
        
        if (myStrLength > MAX_OSPATH - 1) {
            myStrLength = MAX_OSPATH - 1;
        }
#if 0
        strncpy(gCDDevice, thePath, myStrLength);
#else
        for (i = 0; i < myStrLength; i++) {
            gCDDevice[i] = thePath[i];
        }
#endif
    }
    
    gCDDevice[myStrLength] = 0x00;
}


int FDCDCDPlayer::currentTrackNumber()
{
    return theCD->cur_track;
}

int FDCDCDPlayer::totalTrackNumber()
{
    return theCD->numtracks;
}

const char *FDCDCDPlayer::devicePath()
{
    return gCDDevice;
}

bool FDCDCDPlayer::isPlaying()
{
    return theCD->status == CD_PLAYING;
}

void FDCDCDPlayer::callBack(void* inRefCon, SDL2_CD* cdrom)
{
    FDCDCDPlayer *ourSelf = (FDCDCDPlayer *)inRefCon;
    Lock ();
    
    if (ourSelf->nextTrackFrame > 0 && ourSelf->nextTrackFramesRemaining > 0) {
        
        /* Load the next file to play */
        int startFrame, stopFrame;
        FSRef *file;
        
        PauseFile ();
        ReleaseFile ();
        
        file = ourSelf->GetFileForOffset (ourSelf->nextTrackFrame,
                                 ourSelf->nextTrackFramesRemaining, startFrame, stopFrame);
        
        if (file == NULL) {
            ourSelf->theCD->status = CD_STOPPED;
            Unlock ();
            return;
        }
        
        LoadFile (file, startFrame, stopFrame);
        
        SetCompletionProc (callBack, cdrom, ourSelf);
        
        PlayFile ();
    }
    else {
        
        /* Release the current file */
        PauseFile ();
        ReleaseFile ();
        ourSelf->theCD->status = CD_STOPPED;
    }
    
    Unlock ();
}

