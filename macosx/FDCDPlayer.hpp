//
//  FDCDPlayer.hpp
//  Quake II
//
//  Created by C.W. Betts on 5/13/16.
//
//

#ifndef FDCDPlayer_hpp
#define FDCDPlayer_hpp

#include <stdio.h>
#include "cd_osx.h"
extern "C" {
#include "q_shared.h"
#include "cdaudio.h"
}

void		CDAudio_Error (cderror_t theErrorNumber);


class FDCDPlayer {
public:
    virtual void play(int theTrack, bool theLoop) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void update() = 0;
    virtual bool loadPath(const char *directory) = 0;
    virtual int currentTrackNumber() = 0;
    virtual int totalTrackNumber() = 0;
    virtual const char *devicePath() = 0;
    virtual bool isPlaying() = 0;
    //virtual static bool scan() = 0;
    
    static FDCDPlayer* getPlayer() {
        return player;
    }
    
    static float getVolume() {
        return cd_volume->value;
    }
    
    static void closeCD() {
        delete player;
        player = NULL;
    }
    
    virtual ~FDCDPlayer() {}
    
protected:
    static cvar_t *cd_volume;

#pragma mark friends
    friend int ::CDAudio_Init (void);
    friend bool	::CDAudio_GetTrackList (void);
    
private:
    static FDCDPlayer *player;
};

class FDCDCDPlayer: public FDCDPlayer {
    struct SDL2_CD *theCD;
    struct FSRef *tracks;
    int currentTrack;
    int nextTrackFrame;
    int nextTrackFramesRemaining;
    char gCDDevice[PATH_MAX];
    bool looping;

public:
    FDCDCDPlayer();
    
    virtual void play(int theTrack, bool theLoop);
    virtual void stop();
    virtual void pause();
    virtual void resume();
    virtual void update();
    virtual bool loadPath(const char *directory);
    
    virtual ~FDCDCDPlayer();
    
#pragma mark getters
    virtual int currentTrackNumber();
    virtual int totalTrackNumber();
    virtual const char *devicePath();
    virtual bool isPlaying();

private:
    
    struct FSRef* GetFileForOffset(int start, int length,  int &outStartFrame, int &outStopFrame);
    void safePath (const char *thePath);

};


#endif /* FDCDPlayer_hpp */
