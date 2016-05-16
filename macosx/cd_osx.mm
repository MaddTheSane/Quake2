//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// "cd_osx.m" - MacOS X audio CD driver.
//
// Written by:	awe			               	[mailto:awe@fruitz-of-dojo.de].
//	            ©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
// Quake IIª is copyrighted by id software	[http://www.idsoftware.com].
//
// Version History:
// v1.0.8: Rewritten. Uses now QuickTime for playback. Added support for MP3 and MP4 [AAC] playback.
// v1.0.3: Fixed an issue with requesting a track number greater than the max number.
// v1.0.1: Added "cdda" as extension for detection of audio-tracks [required by MacOS X v10.1 or later]
// v1.0.0: Initial release.
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Includes

#import <Cocoa/Cocoa.h>
#import <CoreAudio/AudioHardware.h>
#import <sys/mount.h>
#import <pthread.h>
#include "FDCDPlayer.hpp"
#include "FDCDDirectoryPlayer.hpp"

extern "C" {
#import "client.h"
#import "cd_osx.h"
#import "sys_osx.h"
#import "Quake2.h"
}

#pragma mark -

//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Variables

#pragma mark -

//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Function Prototypes

static  void		CDAudio_Pause (void);
static  void		CDAudio_Resume (void);
static 	void 		CD_f (void);

#pragma mark -

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Error (cderror_t theErrorNumber)
{
    if ([[NSApp delegate] mediaFolder] == NULL)
    {
        Com_Printf ("Audio-CD driver: ");
    }
    else
    {
        Com_Printf ("MP3/MP4 driver: ");
    }
    
    switch (theErrorNumber)
    {
        case CDERR_ALLOC_TRACK:
            Com_Printf ("Failed to allocate track!\n");
            break;
        case CDERR_MOVIE_DATA:
            Com_Printf ("Failed to retrieve track data!\n");
            break;
        case CDERR_AUDIO_DATA:
            Com_Printf ("File without audio track!\n");
            break;
        case CDERR_QUICKTIME_ERROR:
            Com_Printf ("QuickTime error!\n");
            break;
        case CDERR_THREAD_ERROR:
            Com_Printf ("Failed to initialize thread!\n");
            break;
        case CDERR_NO_MEDIA_FOUND:
            Com_Printf ("No Audio-CD found.\n");
            break;
        case CDERR_MEDIA_TRACK:
            Com_Printf ("Failed to retrieve media track!\n");
            break;
        case CDERR_MEDIA_TRACK_CONTROLLER:
            Com_Printf ("Failed to retrieve track controller!\n");
            break;
        case CDERR_EJECT:
            Com_Printf ("Can\'t eject Audio-CD!\n");
            break;
        case CDERR_NO_FILES_FOUND:
            if ([[NSApp delegate] mediaFolder] == NULL)
            {
                Com_Printf ("No audio tracks found.\n");
            }
            else
            {
                Com_Printf ("No files found with the extension \'.mp3\', \'.mp4\' or \'.m4a\'!\n");
            }
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

bool	CDAudio_GetTrackList (void)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        //aPlay->pause();
    }
    // release previously allocated memory:
    CDAudio_Shutdown ();
    
    @autoreleasepool {
        // Get the current MP3 listing or retrieve the TOC of the AudioCD:
        if ([[NSApp delegate] mediaFolder] != NULL)
        {
            NSString	*myMediaFolder = [[NSApp delegate] mediaFolder];
            FDCDDirectoryPlayer *dirPlayer = new FDCDDirectoryPlayer();
            Com_Printf ("Scanning for audio tracks. Be patient!\n");
            if (!dirPlayer->loadPath([myMediaFolder fileSystemRepresentation])) {
                delete dirPlayer;
            } else {
                FDCDPlayer::player = dirPlayer;
            }
        }
        else
        {
            struct statfs *	myMountList;
            UInt32			myMountCount;
            
            // get number of mounted devices:
            myMountCount = getmntinfo (&myMountList, MNT_NOWAIT);
            
            // zero devices? return.
            if (myMountCount <= 0)
            {
                CDAudio_Error (CDERR_NO_MEDIA_FOUND);
                return (0);
            }
            
            while (myMountCount--)
            {
                // is the device read only?
                if ((myMountList[myMountCount].f_flags & MNT_RDONLY) != MNT_RDONLY) continue;
                
                // is the device local?
                if ((myMountList[myMountCount].f_flags & MNT_LOCAL) != MNT_LOCAL) continue;
                
                // is the device "cdda"?
                if (strcmp (myMountList[myMountCount].f_fstypename, "cddafs")) continue;
                
                // is the device a directory?
                if (strrchr (myMountList[myMountCount].f_mntonname, '/') == NULL) continue;
                
                // we have found a Audio-CD!
                Com_Printf ("Found Audio-CD at mount entry: \"%s\".\n", myMountList[myMountCount].f_mntonname);
                
                // preserve the device name:
                FDCDCDPlayer *cdPlay = new FDCDCDPlayer();
                //CDAudio_SafePath (myMountList[myMountCount].f_mntonname);
                //myMountPath = [[NSFileManager defaultManager] stringWithFileSystemRepresentation:myMountList[myMountCount].f_mntonname length:strlen(myMountList[myMountCount].f_mntonname)];
                
                Con_Print ("Scanning for audio tracks. Be patient!\n");
                if (!cdPlay->loadPath(myMountList[myMountCount].f_mntonname)) {
                    delete cdPlay;
                } else {
                    FDCDPlayer::player = cdPlay;
                }
                //CDAudio_AddTracks2List (myMountPath, [NSArray arrayWithObjects: @"aiff", @"cdda", NULL]);
                
                break;
            }

        }
    }
    
    // just security:
    if (FDCDPlayer::player == NULL || FDCDPlayer::player->totalTrackNumber() == 0)
    {
        CDAudio_Error (CDERR_NO_FILES_FOUND);
        return (0);
    }
    
    return (1);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Play (int theTrack, qboolean theLoop)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        aPlay->play(theTrack, theLoop);
    }

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Stop (void)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        aPlay->stop();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Pause (void)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        aPlay->pause();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Resume (void)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        aPlay->resume();
    }
#if !defined( __LP64__ )
    
    if (gCDController != NULL && GetMovieActive (gCDController) == NO && IsMovieDone (gCDController) == NO)
    {
        SetMovieActive (gCDController, YES);
        StartMovie (gCDController);
    }
#else
#endif // !__LP64__
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Update (void)
{
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay) {
        aPlay->update();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Enable (bool theState)
{
    static BOOL	myCDIsEnabled = YES;
    
    if (myCDIsEnabled != theState)
    {
        static BOOL	myCDWasPlaying = NO;
        
        if (theState == NO)
        {
            if (FDCDCDPlayer::getPlayer() != NULL && FDCDCDPlayer::getPlayer()->isPlaying() == YES /*&& IsMovieDone (gCDController) == NO*/)
            {
                CDAudio_Pause ();
                myCDWasPlaying = YES;
            }
            else
            {
                myCDWasPlaying = NO;
            }
        }
        else
        {
            if (myCDWasPlaying == YES)
            {
                CDAudio_Resume ();
            }
        }
		
        myCDIsEnabled = theState;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

int	CDAudio_Init (void)
{
    // register the volume var:
    FDCDPlayer::cd_volume = Cvar_Get ("cd_volume", "1", CVAR_ARCHIVE);

#if 1
    
    // add "cd" and "mp3" console command:
    if ([[NSApp delegate] mediaFolder] != NULL)
    {
        Cmd_AddCommand ("mp3", CD_f);
        Cmd_AddCommand ("mp4", CD_f);
    }
    Cmd_AddCommand ("cd", CD_f);
    
    //gCurCDTrack = 0;
    
    if (FDCDCDPlayer::getPlayer() != NULL)
    {
        if ([[NSApp delegate] mediaFolder] == nil)
        {
            Con_Print ("CoreAudio CD driver initialized...\n");
        }
        else
        {
            Con_Print ("AVFoundation MP3/MP4 driver initialized...\n");
        }

        return (1);
    }
    
    if ([[NSApp delegate] mediaFolder] == nil)
    {
        Con_Print ("CoreAudio CD driver failed.\n");
    }
    else
    {
        Con_Print ("AVFoundation MP3/MP4 driver failed.\n");
    }
    
#endif // !__LP64__
    
    return (0);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CDAudio_Shutdown (void)
{
	CDAudio_Stop ();
    FDCDPlayer::closeCD();
#if !defined( __LP64__ )
    
    // shutdown the audio IO:
    CDAudio_Stop ();

    gCDController = NULL;
    gCDDevice[0] = 0x00;    
    gCurCDTrack = 0;

    if (gCDTrackList != NULL)
    {
       while ([gCDTrackList count])
        {
            NSMovie *	myMovie = [gCDTrackList objectAtIndex: 0];
            
            [gCDTrackList removeObjectAtIndex: 0];
            [myMovie release];
        }
        [gCDTrackList release];
		
        gCDTrackList = NULL;
        gCDTrackCount = 0;
    }
    
#else
    
#endif // !__LP64__
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

void	CD_f (void)
{
    char	*myCommandOption;

    // this command requires options!
    if (Cmd_Argc () < 2)
    {
        return;
    }

    // get the option:
    myCommandOption = Cmd_Argv (1);
    
    // turn CD playback on:
    if (Q_strcasecmp (myCommandOption, "on") == 0)
    {
        if (FDCDPlayer::getPlayer() == NULL)
        {
            CDAudio_GetTrackList();
        }
        CDAudio_Play(1, qfalse);
        
		return;
    }
    
    // turn CD playback off:
    if (Q_strcasecmp (myCommandOption, "off") == 0)
    {
        CDAudio_Shutdown ();
        
		return;
    }

    // just for compatibility:
    if (Q_strcasecmp (myCommandOption, "remap") == 0)
    {
        return;
    }

    // reset the current CD:
    if (Q_strcasecmp (myCommandOption, "reset") == 0)
    {
        CDAudio_Stop ();
		
        if (CDAudio_GetTrackList ())
        {
            if ([[NSApp delegate] mediaFolder] == NULL)
            {
                Con_Print ("CD");
            }
            else
            {
                Con_Print ("MP3/MP4 files");
            }
            FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
            if (aPlay) {
                Com_Printf (" found. %d tracks (\"%s\").\n", aPlay->totalTrackNumber(), aPlay->devicePath());
            }
		}
        else
        {
            CDAudio_Error (CDERR_NO_FILES_FOUND);
        }
        
	return;
    }
    
    // the following commands require a valid track array, so build it, if not present:
    FDCDPlayer *aPlay = FDCDPlayer::getPlayer();
    if (aPlay == NULL || aPlay->totalTrackNumber() == 0)
    {
        CDAudio_GetTrackList ();
        aPlay = FDCDPlayer::getPlayer();
        if (aPlay == NULL || aPlay->totalTrackNumber() == 0)
        {
            CDAudio_Error (CDERR_NO_FILES_FOUND);
			
            return;
        }
    }
    
    // play the selected track:
    if (Q_strcasecmp (myCommandOption, "play") == 0)
    {
        CDAudio_Play (atoi (Cmd_Argv (2)), qfalse);
        
		return;
    }
    
    // loop the selected track:
    if (Q_strcasecmp (myCommandOption, "loop") == 0)
    {
        CDAudio_Play (atoi (Cmd_Argv (2)), qtrue);
        
		return;
    }
    
    // stop the current track:
    if (Q_strcasecmp (myCommandOption, "stop") == 0)
    {
        CDAudio_Stop ();
        
		return;
    }
    
    // pause the current track:
    if (Q_strcasecmp (myCommandOption, "pause") == 0)
    {
        CDAudio_Pause ();
        
		return;
    }
    
    // resume the current track:
    if (Q_strcasecmp (myCommandOption, "resume") == 0)
    {
        CDAudio_Resume ();
        
		return;
    }
    
    // eject the CD:
    if ([[NSApp delegate] mediaFolder] == nil && Q_strcasecmp (myCommandOption, "eject") == 0)
    {
		const char* gCDDevice = aPlay ? aPlay->devicePath() : NULL;
        // eject the CD:
        if (gCDDevice[0] != 0x00)
        {
            NSString	*myDevicePath = [[NSFileManager defaultManager] stringWithFileSystemRepresentation:gCDDevice length:strlen(gCDDevice)];
            
            if (myDevicePath != NULL)
            {
                CDAudio_Shutdown ();
                
                if (![[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtPath: myDevicePath])
                {
                    CDAudio_Error (CDERR_EJECT);
                }
            }
            else
            {
                CDAudio_Error (CDERR_EJECT);
            }
        }
        else
        {
            CDAudio_Error (CDERR_NO_MEDIA_FOUND);
        }
        
		return;
    }
    
    // output CD info:
    if (Q_strcasecmp(myCommandOption, "info") == 0)
    {
        if (aPlay == NULL || aPlay->totalTrackNumber() == 0)
        {
            CDAudio_Error (CDERR_NO_FILES_FOUND);
        }
        else
        {
            if (/*gCDController != NULL && GetMovieActive (gCDController) == */ /* DISABLES CODE */ (YES))
            {
                Com_Printf ("Playing track %d of %d (\"%s\").\n", aPlay->currentTrackNumber(), aPlay->totalTrackNumber(), aPlay->devicePath());
            }
            else
            {
                Com_Printf ("Not playing. Tracks: %d (\"%s\").\n", aPlay->totalTrackNumber(), aPlay->devicePath());
            }
			
            Com_Printf ("Volume is: %.2f.\n", FDCDPlayer::getVolume());
        }
        
		return;
	}
}

//______________________________________________________________________________________________________________eOF
