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
   AudioFileManager.cpp
*/
#include "AudioFilePlayer.h"
#include <mach/mach.h> /* used for setting policy of thread */
#include "SDLOSXCAGuard.h"
#include <pthread.h>

#include <list>

/*typedef void *FileData;
typedef struct S_FileData
{
    AudioFileManager *obj;
    struct S_FileData *next;
} FileData;*/


class FileReaderThread {
public:
    FileReaderThread();
    ~FileReaderThread();

    SDLOSXCAGuard*                    GetGuard();
    void                        AddReader();
    void                        RemoveReader(AudioFileManager* inItem);
    int                         TryNextRead(AudioFileManager* inItem);

    int     mThreadShouldDie;
    
private:
    typedef std::list<AudioFileManager*> FileData;

    SDLOSXCAGuard	*mGuard;
    UInt32			mThreadPriority;
    
    int				mNumReaders;
    FileData		mFileData;


    void			ReadNextChunk();
    int				StartFixedPriorityThread();
    static UInt32	GetThreadBasePriority(pthread_t inThread);
    static void*	DiskReaderEntry(void *inRefCon);
};

SDLOSXCAGuard* FileReaderThread::GetGuard()
{
    return mGuard;
}

/* returns 1 if succeeded */
int FileReaderThread::TryNextRead(AudioFileManager* inItem)
{
    int didLock = 0;
    int succeeded = 0;
    if (mGuard->Try(&didLock))
    {
        mFileData.push_back(inItem);
        /* !!! FIXME: this could be faster with a "tail" member. --ryan. */

        mGuard->Notify();
        succeeded = 1;

        if (didLock)
            mGuard->Unlock();
    }
                
    return succeeded;
}

void FileReaderThread::AddReader()
{
    if (mNumReaders == 0)
    {
        mThreadShouldDie = 0;
        StartFixedPriorityThread();
    }
    mNumReaders++;
}

void FileReaderThread::RemoveReader (AudioFileManager* inItem)
{
    if (mNumReaders > 0)
    {
        int bNeedsRelease = mGuard->Lock();
        
        mFileData.remove(inItem);
        if (--mNumReaders == 0) {
            mThreadShouldDie = 1;
            mGuard->Notify(); /* wake up thread so it will quit */
            mGuard->Wait();   /* wait for thread to die */
        }

        if (bNeedsRelease)
            mGuard->Unlock();
    }   
}

int    FileReaderThread::StartFixedPriorityThread()
{
    pthread_attr_t      theThreadAttrs;
    pthread_t           pThread;

    OSStatus result = pthread_attr_init(&theThreadAttrs);
        if (result) return 0; /*THROW_RESULT("pthread_attr_init - Thread attributes could not be created.")*/
    
    result = pthread_attr_setdetachstate(&theThreadAttrs, PTHREAD_CREATE_DETACHED);
        if (result) return 0; /*THROW_RESULT("pthread_attr_setdetachstate - Thread attributes could not be detached.")*/
    
    result = pthread_create (&pThread, &theThreadAttrs, DiskReaderEntry, this);
        if (result) return 0; /*THROW_RESULT("pthread_create - Create and start the thread.")*/
    
    pthread_attr_destroy(&theThreadAttrs);
    
    /* we've now created the thread and started it
       we'll now set the priority of the thread to the nominated priority
       and we'll also make the thread fixed */
    thread_extended_policy_data_t       theFixedPolicy;
    thread_precedence_policy_data_t     thePrecedencePolicy;
    SInt32                              relativePriority;
    
    /* make thread fixed */
    theFixedPolicy.timeshare = 0;   /* set to 1 for a non-fixed thread */
    result = thread_policy_set (pthread_mach_thread_np(pThread), THREAD_EXTENDED_POLICY, (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);
        if (result) return 0; /*THROW_RESULT("thread_policy - Couldn't set thread as fixed priority.")*/
    /* set priority */
    /* precedency policy's "importance" value is relative to spawning thread's priority */
    relativePriority = mThreadPriority - GetThreadBasePriority(pthread_self());
        
    thePrecedencePolicy.importance = relativePriority;
    result = thread_policy_set (pthread_mach_thread_np(pThread), THREAD_PRECEDENCE_POLICY, (thread_policy_t)&thePrecedencePolicy, THREAD_PRECEDENCE_POLICY_COUNT);
        if (result) return 0; /*THROW_RESULT("thread_policy - Couldn't set thread priority.")*/

    return 1;
}

UInt32  FileReaderThread::GetThreadBasePriority (pthread_t inThread)
{
    thread_basic_info_data_t            threadInfo;
    policy_info_data_t                  thePolicyInfo;
    unsigned int                        count;
    
    /* get basic info */
    count = THREAD_BASIC_INFO_COUNT;
    thread_info (pthread_mach_thread_np (inThread), THREAD_BASIC_INFO, (integer_t*)&threadInfo, &count);
    
    switch (threadInfo.policy) {
        case POLICY_TIMESHARE:
            count = POLICY_TIMESHARE_INFO_COUNT;
            thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_TIMESHARE_INFO, (integer_t*)&(thePolicyInfo.ts), &count);
            return thePolicyInfo.ts.base_priority;
            break;
            
        case POLICY_FIFO:
            count = POLICY_FIFO_INFO_COUNT;
            thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_FIFO_INFO, (integer_t*)&(thePolicyInfo.fifo), &count);
            if (thePolicyInfo.fifo.depressed) {
                return thePolicyInfo.fifo.depress_priority;
            } else {
                return thePolicyInfo.fifo.base_priority;
            }
            break;
            
        case POLICY_RR:
            count = POLICY_RR_INFO_COUNT;
            thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_RR_INFO, (integer_t*)&(thePolicyInfo.rr), &count);
            if (thePolicyInfo.rr.depressed) {
                return thePolicyInfo.rr.depress_priority;
            } else {
                return thePolicyInfo.rr.base_priority;
            }
            break;
    }
    
    return 0;
}

void *FileReaderThread::DiskReaderEntry(void *inRefCon)
{
    FileReaderThread *frt = (FileReaderThread *)inRefCon;
    frt->ReadNextChunk();
    #if DEBUG
    printf ("finished with reading file\n");
    #endif
    
    return 0;
}

void FileReaderThread::ReadNextChunk()
{
    OSStatus result;
    ByteCount dataChunkSize;
    AudioFileManager* theItem = 0;
	
    for (;;)
    {
        { /* this is a scoped based lock */
            int bNeedsRelease = mGuard->Lock();
            
            if (mThreadShouldDie) {
                mGuard->Notify();
                if (bNeedsRelease)
                    mGuard->Unlock();
                return;
            }
            
            if (mFileData.empty())
            {
                mGuard->Wait();
            }
			
            /* kill thread */
            if (mThreadShouldDie) {
				
                mGuard->Notify();
                if (bNeedsRelease)
                    mGuard->Unlock();
                return;
            }
			
            theItem = mFileData.front();
            mFileData.pop_front();
			
            if (bNeedsRelease)
				mGuard->Unlock();
        }
		
        if ((theItem->mFileLength - theItem->mReadFilePosition) < theItem->mChunkSize)
            dataChunkSize = theItem->mFileLength - theItem->mReadFilePosition;
        else
            dataChunkSize = theItem->mChunkSize;
        
		/* this is the exit condition for the thread */
        if (dataChunkSize <= 0) {
            theItem->mFinishedReadingData = 1;
            continue;
        }
		/* construct pointer */
        char* writePtr = (char *) (theItem->GetFileBuffer() +
								   (theItem->mWriteToFirstBuffer ? 0 : theItem->mChunkSize));
		
		/* read data */
        result = theItem->Read(writePtr, &dataChunkSize);
        if (result != noErr && result != eofErr) {
            AudioFilePlayer *afp = (AudioFilePlayer *) theItem->GetParent();
            afp->DoNotification(result);
            continue;
        }
        
        if (dataChunkSize != theItem->mChunkSize)
        {
            writePtr += dataChunkSize;
			
            /* can't exit yet.. we still have to pass the partial buffer back */
            memset(writePtr, 0, (theItem->mChunkSize - dataChunkSize));
        }
        
        theItem->mWriteToFirstBuffer = !theItem->mWriteToFirstBuffer;   /* switch buffers */
        
        if (result == eofErr)
            theItem->mReadFilePosition = theItem->mFileLength;
        else
            theItem->mReadFilePosition += dataChunkSize;        /* increment count */
    }
}

void delete_FileReaderThread(FileReaderThread *frt)
{
	if (frt != NULL) {
		delete frt;
	}
}

FileReaderThread::~FileReaderThread()
{
    delete mGuard;
}

FileReaderThread *new_FileReaderThread()
{
    return new FileReaderThread();
}

FileReaderThread::FileReaderThread()
{
    mThreadShouldDie = 0;
    mNumReaders = 0;
    mFileData = FileData();

    mGuard = new SDLOSXCAGuard();

    mThreadPriority = 62;
}

static FileReaderThread *sReaderThread;

int AudioFileManager::DoConnect()
{
    if (!mIsEngaged) {
        OSStatus result;

        /*afm->mReadFilePosition = 0;*/
        mFinishedReadingData = 0;

        mNumTimesAskedSinceFinished = 0;
        mLockUnsuccessful = 0;
        
        ByteCount dataChunkSize;
        
        if ((mFileLength - mReadFilePosition) < mChunkSize)
            dataChunkSize = mFileLength - mReadFilePosition;
        else
            dataChunkSize = mChunkSize;
        
        result = Read(mFileBuffer, &dataChunkSize);
        if (result) THROW_RESULT("AudioFileManager::DoConnect(): Read");

        mReadFilePosition += dataChunkSize;
                
        mWriteToFirstBuffer = 0;
        mReadFromFirstBuffer = 1;

        sReaderThread->AddReader();
        
        mIsEngaged = 1;
    } else
        throw static_cast<OSStatus>(-1); /* thread has already been started */

    return 1;
}

void AudioFileManager::Disconnect()
{
    if (mIsEngaged) {
        sReaderThread->RemoveReader(this);
        mIsEngaged = 0;
    }
}

OSStatus AudioFileManager::Read(char *buffer, ByteCount *len)
{
    return FSReadFork(mForkRefNum,
                      fsFromStart,
                      mReadFilePosition + mAudioDataOffset,
                      *len,
                      buffer,
                      len);
}

OSStatus AudioFileManager::GetFileData(void** inOutData, UInt32 *inOutDataSize)
{
	if (mFinishedReadingData)
	{
		++mNumTimesAskedSinceFinished;
		*inOutDataSize = 0;
		*inOutData = 0;
		return noErr;
	}
	
	if (mReadFromFirstBuffer == mWriteToFirstBuffer) {
#if DEBUG
		printf ("* * * * * * * Can't keep up with reading file\n");
#endif
		
		mParent->DoNotification(kAudioFilePlayErr_FilePlayUnderrun);
		*inOutDataSize = 0;
		*inOutData = 0;
	} else {
		*inOutDataSize = mChunkSize;
		*inOutData = mReadFromFirstBuffer ? mFileBuffer : (mFileBuffer + mChunkSize);
	}
	
	mLockUnsuccessful = !sReaderThread->TryNextRead(this);
	
	mReadFromFirstBuffer = !mReadFromFirstBuffer;
	
	return noErr;
}

void AudioFileManager::AfterRender()
{
    if (mNumTimesAskedSinceFinished > 0)
    {
        int didLock = 0;
        SDLOSXCAGuard *guard = sReaderThread->GetGuard();
        if (guard->Try(&didLock)) {
            mParent->DoNotification(kAudioFilePlay_FileIsFinished);
            if (didLock)
                guard->Unlock();
        }
    }

    if (mLockUnsuccessful)
        mLockUnsuccessful = !sReaderThread->TryNextRead(this);
}

void AudioFileManager::SetPosition(SInt64 pos)
{
    if (pos < 0 || pos >= mFileLength) {
        printf ("AudioFileManager::SetPosition - position invalid: %d filelen=%d\n",
            (unsigned int)pos, (unsigned int)mFileLength);
        pos = 0;
    }
        
    mReadFilePosition = pos;
}
    
void AudioFileManager::SetEndOfFile(SInt64 pos)
{
    if (pos <= 0 || pos > mFileLength) {
        printf ("AudioFileManager::SetEndOfFile - position beyond actual eof\n");
        pos = mFileLength;
    }
    
    mFileLength = pos;
}

const char *AudioFileManager::GetFileBuffer()
{
    return mFileBuffer;
}

const AudioFilePlayer *AudioFileManager::GetParent()
{
    return mParent;
}

int AudioFileManager::GetByteCounter()
{
    return mByteCounter;
}

OSStatus AudioFileManager::FileInputProc(void                            *inRefCon,
										 AudioUnitRenderActionFlags      *ioActionFlags,
										 const AudioTimeStamp            *inTimeStamp,
										 UInt32                          inBusNumber,
										 UInt32                          inNumberFrames,
										 AudioBufferList                 *ioData)
{
    AudioFileManager* afm = (AudioFileManager*)inRefCon;
    return afm->Render(ioData);
}

OSStatus AudioFileManager::Render(AudioBufferList *ioData)
{
    OSStatus result = noErr;
    AudioBuffer *abuf;
    UInt32 i;

    for (i = 0; i < ioData->mNumberBuffers; i++) {
        abuf = &ioData->mBuffers[i];
        if (mBufferOffset >= mBufferSize) {
            result = GetFileData(&mTmpBuffer, &mBufferSize);
            if (result) {
                printf ("AudioConverterFillBuffer:%d\n", (int)result);
                mParent->DoNotification(result);
                return result;
            }

            mBufferOffset = 0;
        }

        if (abuf->mDataByteSize > mBufferSize - mBufferOffset)
            abuf->mDataByteSize = mBufferSize - mBufferOffset;
        abuf->mData = (char *)mTmpBuffer + mBufferOffset;
        mBufferOffset += abuf->mDataByteSize;
    
        mByteCounter += abuf->mDataByteSize;
        AfterRender();
    }
    return result;
}

AudioFileManager::~AudioFileManager()
{
	delete [] mFileBuffer;
}

AudioFileManager::AudioFileManager(AudioFilePlayer *inParent,
                                   SInt16          inForkRefNum,
                                   SInt64          inFileLength,
                                   UInt32          inChunkSize)
{
    if (sReaderThread == NULL)
    {
        sReaderThread = new_FileReaderThread();
        if (sReaderThread == NULL)
            throw;
    }

#if 0
    afm = (AudioFileManager *) SDL_malloc(sizeof (AudioFileManager));
    if (afm == NULL)
        return NULL;
    SDL_memset(afm, '\0', sizeof (*afm));
#endif

    mParent = inParent;
    mForkRefNum = inForkRefNum;
    mBufferSize = inChunkSize;
    mBufferOffset = inChunkSize;
    mChunkSize = inChunkSize;
    mFileLength = inFileLength;
	mFileBuffer = new char[mChunkSize * 2];
    FSGetForkPosition(mForkRefNum, &mAudioDataOffset);
    assert (mFileBuffer != NULL);
}

