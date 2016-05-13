/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org

    This file based on Apple sample code. We haven't changed the file name, 
    so if you want to see the original search for it on apple.com/developer
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    AudioFilePlayer.h
*/
#ifndef __AudioFilePlayer_H__
#define __AudioFilePlayer_H__

#include <CoreServices/CoreServices.h>

#include <AudioUnit/AudioUnit.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1050
#include <AudioUnit/AUNTComponent.h>
#endif

const char* AudioFilePlayerErrorStr (OSStatus error);


void ThrowResult (OSStatus result, const char *str);

#define THROW_RESULT(str)                                       \
    if (result) {                                               \
        ThrowResult (result, str);                              \
    }


typedef void (*AudioFilePlayNotifier)(void          *inRefCon,
                                      OSStatus      inStatus);

enum {
    kAudioFilePlayErr_FilePlayUnderrun = -10000,
    kAudioFilePlay_FileIsFinished = -10001,
    kAudioFilePlay_PlayerIsUninitialized = -10002
};


class AudioFileManager;

#pragma mark __________ AudioFilePlayer
class AudioFilePlayer
{
public:
    bool            SetDestination(AudioUnit *inDestUnit);
    void            SetNotifier(AudioFilePlayNotifier inNotifier, void *inRefCon);
    void            SetStartFrame(int frame); /* seek in the file */
    int             GetCurrentFrame(); /* get the current frame position */
    void            SetStopFrame(int frame);   /* set limit in the file */
    int             Connect();
    void            Disconnect();
    void            DoNotification(OSStatus inError);
    bool            IsConnected();
    AudioUnit       GetDestUnit();
    void            Print();
    ~AudioFilePlayer();
    AudioFilePlayer(const FSRef *inFileRef);

private:
    AudioUnit                       mPlayUnit;
    FSIORefNum                      mForkRefNum;
    
    AURenderCallbackStruct          mInputCallback;

    AudioStreamBasicDescription     mFileDescription;
    
    bool                            mConnected;
    
    AudioFileManager*               mAudioFileManager;
    
    AudioFilePlayNotifier           mNotifier;
    void*                           mRefCon;
    
    int                             mStartFrame;
    
#pragma mark __________ Private_Methods
    
    int          OpenFile(const FSRef *inRef, SInt64 *outFileSize);
};


#pragma mark __________ AudioFileManager
class AudioFileManager
{
public:
        /* this method should NOT be called by an object of this class
           as it is called by the parent's Disconnect() method */
    void                Disconnect();
    int                 DoConnect();
    OSStatus            Read(char *buffer, ByteCount *len);
    const char*         GetFileBuffer();
    const AudioFilePlayer *GetParent();
    void                SetPosition(SInt64 pos);  /* seek/rewind in the file */
    int                 GetByteCounter();  /* return actual bytes streamed to audio hardware */
    void                SetEndOfFile(SInt64 pos);  /* set the "EOF" (will behave just like it reached eof) */
    AudioFileManager(AudioFilePlayer *inParent,
                     SInt16          inForkRefNum,
                     SInt64          inFileLength,
                     UInt32          inChunkSize);
    ~AudioFileManager();
    
protected:
    AudioFilePlayer*    mParent;
    SInt16              mForkRefNum;
    SInt64              mAudioDataOffset;
    
    char*               mFileBuffer;

    int                 mByteCounter;

    int                mReadFromFirstBuffer;
    int                mLockUnsuccessful;
    bool               mIsEngaged;
    
    int                 mNumTimesAskedSinceFinished;


	void*               mTmpBuffer;
	UInt32              mBufferSize;
	UInt32              mBufferOffset;
public:
    UInt32              mChunkSize;
    SInt64              mFileLength;
    SInt64              mReadFilePosition;
    int                 mWriteToFirstBuffer;
    int                 mFinishedReadingData;

protected:
    OSStatus            Render(AudioBufferList *ioData);
    OSStatus            GetFileData(void** inOutData, UInt32 *inOutDataSize);
    void                AfterRender();

public:
    static OSStatus     FileInputProc(void                            *inRefCon,
                                      AudioUnitRenderActionFlags      *ioActionFlags,
                                      const AudioTimeStamp            *inTimeStamp,
                                      UInt32                          inBusNumber,
                                      UInt32                          inNumberFrames,
                                      AudioBufferList                 *ioData);
};

#endif
