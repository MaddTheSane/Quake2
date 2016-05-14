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
    //virtual static bool scan() = 0;
    
    static FDCDPlayer* GetPlayer() {
        return player;
    }
    
    static float volumeValue() {
        return cd_volume->value;
    }
    
protected:
    static cvar_t *cd_volume;

    friend int ::CDAudio_Init (void);
    
private:
    static FDCDPlayer *player;
};

class FDCDCDPlayer: public FDCDPlayer {
public:
    virtual void play(int theTrack, bool theLoop);
    virtual void stop();
    virtual void pause();
    virtual void resume();
    virtual void update();
    virtual bool loadPath(const char *directory);
};


#endif /* FDCDPlayer_hpp */
