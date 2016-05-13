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
    AudioFilePlayer.cpp
*/
#include "AudioFilePlayer.h"

void ThrowResult (OSStatus result, const char* str)
{
    printf ("Error: %s %d", str, result);
    throw result;
}

#if DEBUG
static void PrintStreamDesc (AudioStreamBasicDescription *inDesc)
{
    if (!inDesc) {
        printf ("Can't print a NULL desc!\n");
        return;
    }
    
    printf ("- - - - - - - - - - - - - - - - - - - -\n");
    printf ("  Sample Rate:%f\n", inDesc->mSampleRate);
    printf ("  Format ID:%s\n", (char*)&inDesc->mFormatID);
    printf ("  Format Flags:%X\n", (unsigned int)inDesc->mFormatFlags);
    printf ("  Bytes per Packet:%u\n", (unsigned int)inDesc->mBytesPerPacket);
    printf ("  Frames per Packet:%u\n", (unsigned int)inDesc->mFramesPerPacket);
    printf ("  Bytes per Frame:%u\n", (unsigned int)inDesc->mBytesPerFrame);
    printf ("  Channels per Frame:%u\n", (unsigned int)inDesc->mChannelsPerFrame);
    printf ("  Bits per Channel:%u\n", (unsigned int)inDesc->mBitsPerChannel);
    printf ("- - - - - - - - - - - - - - - - - - - -\n");
}
#endif


bool AudioFilePlayer::SetDestination (AudioUnit  *inDestUnit)
{
    if (mConnected) throw static_cast<OSStatus>(-1); /* can't set dest if already engaged */

    memcpy(&mPlayUnit, inDestUnit, sizeof (mPlayUnit));

    OSStatus result = noErr;
    

        /* we can "down" cast a component instance to a component */
    ComponentDescription desc;
    result = GetComponentInfo ((Component)*inDestUnit, &desc, 0, 0, 0);
    if (result) return 0; /*THROW_RESULT("GetComponentInfo")*/
        
        /* we're going to use this to know which convert routine to call
           a v1 audio unit will have a type of 'aunt'
           a v2 audio unit will have one of several different types. */
    if (desc.componentType != kAudioUnitType_Output) {
        result = badComponentInstance;
        /*THROW_RESULT("BAD COMPONENT")*/
        if (result) return 0;
    }

    /* Set the input format of the audio unit. */
    result = AudioUnitSetProperty (*inDestUnit,
                               kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Input,
                               0,
                               &mFileDescription,
                               sizeof (mFileDescription));
        /*THROW_RESULT("AudioUnitSetProperty")*/
    if (result) return 0;
    return 1;
}

void AudioFilePlayer::SetNotifier(AudioFilePlayNotifier inNotifier, void *inRefCon)
{
    mNotifier = inNotifier;
    mRefCon = inRefCon;
}

bool AudioFilePlayer::IsConnected()
{
    return mConnected;
}

AudioUnit AudioFilePlayer::GetDestUnit()
{
   return mPlayUnit;
}

void AudioFilePlayer::Print()
{
#if DEBUG    
    printf ("Is Connected:%s\n", (IsConnected() ? "true" : "false"));
    printf ("- - - - - - - - - - - - - - \n");
#endif
}

void    AudioFilePlayer::SetStartFrame (int frame)
{
    SInt64 position = frame * 2352;

    mStartFrame = frame;
    mAudioFileManager->SetPosition(position);
}

int AudioFilePlayer::GetCurrentFrame()
{
    return mStartFrame + (mAudioFileManager->GetByteCounter() / 2352);
}
    
void AudioFilePlayer::SetStopFrame (int frame)
{
    SInt64 position  = frame * 2352;
    
    mAudioFileManager->SetEndOfFile(position);
}

AudioFilePlayer::~AudioFilePlayer()
{
        Disconnect();
        
        if (mAudioFileManager) {
            delete mAudioFileManager;
            mAudioFileManager = 0;
        }
    
        if (mForkRefNum) {
            FSCloseFork (mForkRefNum);
            mForkRefNum = 0;
        }
}

int AudioFilePlayer::Connect()
{
#if DEBUG
    printf ("Connect:%lx, engaged=%d\n", (long)mPlayUnit, (mConnected ? 1 : 0));
#endif
    if (!mConnected)
    {           
        if (!mAudioFileManager->DoConnect())
            return 0;

        /* set the render callback for the file data to be supplied to the sound converter AU */
        mInputCallback.inputProc = mAudioFileManager->FileInputProc;
        mInputCallback.inputProcRefCon = mAudioFileManager;

        OSStatus result = AudioUnitSetProperty(mPlayUnit,
                            kAudioUnitProperty_SetRenderCallback,
                            kAudioUnitScope_Input, 
                            0,
                            &mInputCallback,
                            sizeof(mInputCallback));
        if (result)  THROW_RESULT("AudioUnitSetProperty");
        mConnected = 1;
    }

    return 1;
}

/* warning noted, now please go away ;-) */
/* #warning This should redirect the calling of notification code to some other thread */
void AudioFilePlayer::DoNotification (OSStatus inStatus)
{
    if (mNotifier) {
        (*mNotifier) (mRefCon, inStatus);
    } else {
        printf ("Notification posted with no notifier in place");
        
        if (inStatus == kAudioFilePlay_FileIsFinished)
            Disconnect();
        else if (inStatus != kAudioFilePlayErr_FilePlayUnderrun)
            Disconnect();
    }
}

void AudioFilePlayer::Disconnect()
{
#if DEBUG
    printf ("Disconnect:%lx,%d, engaged=%d\n", (long)mPlayUnit, 0, (mConnected ? 1 : 0));
#endif
    if (mConnected)
    {
        mConnected = 0;
            
        mInputCallback.inputProc = 0;
        mInputCallback.inputProcRefCon = 0;
        OSStatus result = AudioUnitSetProperty (mPlayUnit,
                                        kAudioUnitProperty_SetRenderCallback,
                                        kAudioUnitScope_Input, 
                                        0,
                                        &mInputCallback,
                                        sizeof(mInputCallback));
        if (result) 
            printf ("AudioUnitSetProperty:RemoveInputCallback:%d", (int)result);

        mAudioFileManager->Disconnect();
    }
}

typedef struct {
    UInt32 offset;
    UInt32 blockSize;
} SSNDData;

int AudioFilePlayer::OpenFile(const FSRef *inRef, SInt64 *outFileDataSize)
{
    ContainerChunk chunkHeader;
    ChunkHeader chunk;
    SSNDData ssndData;

    OSErr result;
    HFSUniStr255 dfName;
    ByteCount actual;
    SInt64 offset;

    /* Open the data fork of the input file */
    result = FSGetDataForkName(&dfName);
    if (result) THROW_RESULT("AudioFilePlayer::OpenFile(): FSGetDataForkName");

    result = FSOpenFork(inRef, dfName.length, dfName.unicode, fsRdPerm, &mForkRefNum);
    if (result) THROW_RESULT("AudioFilePlayer::OpenFile(): FSOpenFork");
 
    /* Read the file header, and check if it's indeed an AIFC file */
    result = FSReadFork(mForkRefNum, fsAtMark, 0, sizeof(chunkHeader), &chunkHeader, &actual);
    if (result)  THROW_RESULT("AudioFilePlayer::OpenFile(): FSReadFork");

    if (CFSwapInt32BigToHost(chunkHeader.ckID) != 'FORM') {
        result = -1;
        if (result) return 0; /*THROW_RESULT("AudioFilePlayer::OpenFile(): chunk id is not 'FORM'");*/
    }

    if (CFSwapInt32BigToHost(chunkHeader.formType) != 'AIFC') {
        result = -1;
        if (result) return 0; /*THROW_RESULT("AudioFilePlayer::OpenFile(): file format is not 'AIFC'");*/
    }

    /* Search for the SSND chunk. We ignore all compression etc. information
       in other chunks. Of course that is kind of evil, but for now we are lazy
       and rely on the cdfs to always give us the same fixed format.
       TODO: Parse the COMM chunk we currently skip to fill in mFileDescription.
    */
    offset = 0;
    do {
        result = FSReadFork(mForkRefNum, fsFromMark, offset, sizeof(chunk), &chunk, &actual);
        if (result) THROW_RESULT("AudioFilePlayer::OpenFile(): FSReadFork");

        chunk.ckID = CFSwapInt32BigToHost(chunk.ckID);
        chunk.ckSize = CFSwapInt32BigToHost(chunk.ckSize);

        /* Skip the chunk data */
        offset = chunk.ckSize;
    } while (chunk.ckID != 'SSND');

    /* Read the header of the SSND chunk. After this, we are positioned right
       at the start of the audio data. */
    result = FSReadFork(mForkRefNum, fsAtMark, 0, sizeof(ssndData), &ssndData, &actual);
    if (result) THROW_RESULT("AudioFilePlayer::OpenFile(): FSReadFork");

    ssndData.offset = CFSwapInt32BigToHost(ssndData.offset);

    result = FSSetForkPosition(mForkRefNum, fsFromMark, ssndData.offset);
    if (result) THROW_RESULT("AudioFilePlayer::OpenFile(): FSSetForkPosition");

    /* Data size */
    *outFileDataSize = chunk.ckSize - ssndData.offset - 8;

    /* File format */
    mFileDescription.mSampleRate = 44100;
    mFileDescription.mFormatID = kAudioFormatLinearPCM;
    mFileDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
    mFileDescription.mBytesPerPacket = 4;
    mFileDescription.mFramesPerPacket = 1;
    mFileDescription.mBytesPerFrame = 4;
    mFileDescription.mChannelsPerFrame = 2;
    mFileDescription.mBitsPerChannel = 16;

    return 1;
}

AudioFilePlayer::AudioFilePlayer(const FSRef *inFileRef)
{
    SInt64 fileDataSize  = 0;
    
    mPlayUnit = NULL;
    mForkRefNum = 0;
    memset(&mInputCallback, 0, sizeof(mInputCallback));
    memset(&mInputCallback, 0, sizeof(mInputCallback));
    memset(&mFileDescription, 0, sizeof(mFileDescription));
    mConnected = 0;
    mAudioFileManager = NULL;
    mNotifier = NULL;
    mRefCon = NULL;
    mStartFrame = 0;
    
    if (!OpenFile (inFileRef, &fileDataSize))
    {
        throw;
    }
    
    /* we want about 4 seconds worth of data for the buffer */
    int bytesPerSecond = (UInt32) (4 * mFileDescription.mSampleRate * mFileDescription.mBytesPerFrame);
    
#if DEBUG
    printf("File format:\n");
    PrintStreamDesc (&mFileDescription);
#endif
    
    mAudioFileManager = new AudioFileManager(this, mForkRefNum,
                                             fileDataSize,
                                             bytesPerSecond);
    
    if (mAudioFileManager == NULL)
    {
        delete this;
        return;
    }
}

