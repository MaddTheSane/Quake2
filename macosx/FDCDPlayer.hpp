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

class FDCDPlayer {
public:
    virtual void play(int theTrack, bool theLoop) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void update() = 0;
    virtual bool loadPath(const char *directory) = 0;
    //virtual static bool scan() = 0;
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
