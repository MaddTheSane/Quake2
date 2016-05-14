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

FDCDCDPlayer::FDCDCDPlayer()
{
    
}

void FDCDCDPlayer::play(int theTrack, bool theLoop)
{
    
}

void FDCDCDPlayer::stop()
{
    
}

void FDCDCDPlayer::pause()
{
    
}

void FDCDCDPlayer::resume()
{
    
}

void FDCDCDPlayer::update()
{
    
}

bool FDCDCDPlayer::loadPath(const char *directory)
{
    CFURLRef aURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)directory, strlen(directory), true);
    FSRef pathRef;
    FSVolumeRefNum vRef;
    if (!CFURLGetFSRef(aURL, &pathRef)) {
        CFRelease(aURL);
        return false;
    }
    CFRelease(aURL);
    if (FSGetVRefNum(&pathRef, vRef) != noErr) {
        return false;
    }
    ReadTOCData(vRef, theCD);
    //FSRef;
    
    return false;
}

FDCDCDPlayer::~FDCDCDPlayer()
{
    
}
