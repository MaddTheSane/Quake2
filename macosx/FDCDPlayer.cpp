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
    
    /* check parameters */
    //require_action(NULL != vRefNum, BadParameter, result = paramErr);
    
    /* get the volume refNum from the FSRef */
    result = FSGetCatalogInfo(ref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL);
    require_noerr(result, FSGetCatalogInfo);
    
    /* return volume refNum from catalogInfo */
    vRefNum = catalogInfo.volume;
    
FSGetCatalogInfo:
//BadParameter:
    
    return ( result );
}

FDCDCDPlayer::FDCDCDPlayer() : tracks(NULL), currentTrack(0), nextTrackFrame(0), nextTrackFramesRemaining(0)
{
    theCD = new SDL2_CD();
    memset(gCDDevice, 0, sizeof(gCDDevice));
}

void FDCDCDPlayer::play(int theTrack, bool theLoop)
{
    looping = theLoop;
    if (theTrack > (theCD->numtracks + 1)) {
        return;
    }
    LoadFile(&tracks[theTrack], -1, -1);
    PlayFile();
}

void FDCDCDPlayer::stop()
{
    
}

void FDCDCDPlayer::pause()
{
    PauseFile();
}

void FDCDCDPlayer::resume()
{
    PlayFile();
}

void FDCDCDPlayer::update()
{
    
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

    //FSRef;
    
    
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
    
    currentTrack = i;
    
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
    return currentTrack;
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
    return false;
}
