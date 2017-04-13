//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// "in_osx.h"
//
// Written by:	awe                         [mailto:awe@fruitz-of-dojo.de].
//		        ©2001-2006 Fruitz Of Dojo   [http://www.fruitz-of-dojo.de].
//
// Quake II™ is copyrighted by id software  [http://www.idsoftware.com].
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Variables

extern UInt8		gInSpecialKey[];
extern UInt8		gInNumPadKey[];

#pragma mark Function Prototypes

extern void			IN_SetKeyboardRepeatEnabled (BOOL theState);
extern void			IN_SetF12EjectEnabled (BOOL theState);
extern void			IN_ShowCursor (BOOL theState);
extern void			IN_CenterCursor (void);
extern void			IN_ReceiveMouseMove (int32_t theDeltaX, int32_t theDeltaY);

//------------------------------------------------------------------------------------------------------------------------------------------------------------
