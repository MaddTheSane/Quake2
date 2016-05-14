//
//  FDCDPlayer.cpp
//  Quake II
//
//  Created by C.W. Betts on 5/13/16.
//
//

#include "CDPlayer.h"
#include "FDCDPlayer.hpp"

cvar_t *FDCDPlayer::cd_volume = NULL;
FDCDPlayer *FDCDPlayer::player = NULL;
