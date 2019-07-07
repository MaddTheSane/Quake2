//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// "Quake2.h"
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
// Quake II™ is copyrighted by id software  [http://www.idsoftware.com].
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Includes

#import <Cocoa/Cocoa.h>
#import <FruitzOfDojo/FDLinkView.h>

#pragma mark -

//------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma mark Defines

#define	SYS_ABOUT_TOOLBARITEM		@"Quake2 About ToolbarItem"
#define	SYS_AUDIO_TOOLBARITEM		@"Quake2 Sound ToolbarItem"
#define	SYS_PARAM_TOOLBARITEM		@"Quake2 Parameters ToolbarItem"
#define	SYS_START_TOOLBARITEM		@"Quake2 Start ToolbarItem"

#pragma mark -

NS_ASSUME_NONNULL_BEGIN

//------------------------------------------------------------------------------------------------------------------------------------------------------------

@interface Quake2 : NSObject <NSApplicationDelegate>
{
    IBOutlet NSWindow *				mediascanWindow;
    
    IBOutlet NSTextField *			mediascanText;
    IBOutlet NSProgressIndicator *	mediascanProgressIndicator;
    
    IBOutlet NSWindow *				startupWindow;
    
	IBOutlet NSView *				aboutView;
	IBOutlet NSView *				audioView;	
	IBOutlet NSView *				parameterView;
    IBOutlet FDLinkView	*			linkView;
			
    IBOutlet NSButton *				mp3CheckBox;
    IBOutlet NSButton *				mp3Button;
    IBOutlet NSTextField *			mp3TextField;
    
    IBOutlet NSButton *				optionCheckBox;
    IBOutlet NSButton *				parameterCheckBox;
    IBOutlet NSTextField *			parameterTextField;
    IBOutlet NSMenuItem *			pasteMenuItem;

    IBOutlet NSView *				mp3HelpView;

    NSView *						mEmptyView;

	NSTimer *						mFrameTimer;
	NSDate *						mDistantPast;
	
    NSString *						mMP3Folder;
	NSString *						mModFolder;

    NSMutableDictionary	*			mToolbarItems;									
	NSMutableArray *				mRequestedCommands;
	
	int								mLastFrameTime;
    BOOL							mOptionPressed;
	BOOL							mDenyDrag;
	BOOL							mAllowAppleScriptRun;
	BOOL							mHostInitialized;
	BOOL							mMediaScanCanceled;
}

+ (void) initialize;

#pragma mark NSApplicationDelegate
- (BOOL) application: (NSApplication *) theSender openFile: (NSString *) theFilePath;
- (void) applicationDidResignActive: (NSNotification *) theNote;
- (void) applicationDidBecomeActive: (NSNotification *) theNote;
- (void) applicationWillHide: (NSNotification *) theNote;
- (void) applicationDidFinishLaunching: (NSNotification *) theNote;
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication *) theSender;

- (void) setupDialog: (nullable NSTimer *) theTimer;
- (void) saveCheckBox: (NSButton *) theButton initial: (id) theInitial
              default: (NSString *) theDefault userDefaults: (NSUserDefaults *) theUserDefaults;
- (void) saveString: (NSString *) theString initial: (NSString *) theInitial
            default: (NSString *) theDefault userDefaults: (NSUserDefaults *) theUserDefaults;
- (void) stringToParameters: (NSString *) theString;
- (BOOL) isEqualTo: (nullable NSString *) theString;
- (void) installFrameTimer;
- (void) renderFrame: (NSTimer *) theTimer;
- (void) scanMediaThread: (id) theSender;
- (void) fireFrameTimer: (NSNotification *) theNotification;

- (IBAction) pasteString: (nullable id) theSender;
- (IBAction) startQuake2: (nullable id) theSender;
- (IBAction) visitFOD: (nullable id) theSender;
- (IBAction) toggleParameterTextField: (nullable id) theSender;
- (IBAction) toggleMP3Playback: (nullable id) theSender;
- (IBAction) selectMP3Folder: (nullable id) theSender;
- (IBAction) stopMediaScan: (nullable id) theSender;

- (void) connectToServer: (NSPasteboard *) thePasteboard userData:(NSString *) theData error:(NSString *_Nullable*_Nullable)theError;

@property BOOL hostInitialized;

@property (getter=allowAppleScriptRun, setter=enableAppleScriptRun:) BOOL allowAppleScriptRun;
- (void) requestCommand: (NSString *) theCommand;

@property (readonly, copy, nullable) NSString *modFolder;
@property (readonly, copy, nullable) NSString *mediaFolder;
@property (readonly) BOOL abortMediaScan;
@property (readonly) BOOL wasDragged;

@end

NS_ASSUME_NONNULL_END

//------------------------------------------------------------------------------------------------------------------------------------------------------------
