/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.


	MainUsingNSSpeechRecognizer.m
	fauxTunes
	
	Copyright (c) 2001-2007 Apple Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>

#import "FauxTunes.h"
#import "Main.h"
#import "SpeechRoutines.h"

// Prototypes
NSDictionary * CreateCommandsDictionary();

// Globals
NSDictionary * 			gCommandsDictionary = NULL;
NSSpeechRecognizer *	gSpeechRecognizer	= NULL;

@implementation SpeechEnabledAppDelegate

/* 
    applicationDidFinishLaunching:
    
    This method is called by Cocoa when our application has been loaded and is ready to run.
    
*/

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    
    // Create a dictionary containing keys as command phrases and values as selectors.
	gCommandsDictionary = CreateCommandsDictionary();

	// Instaniate the speech recognizer object, and set ourselves as the delegate.
	gSpeechRecognizer = [NSSpeechRecognizer new];
	[gSpeechRecognizer setDelegate:self];
    
	// Add commands
	[gSpeechRecognizer setCommands:[gCommandsDictionary allKeys]];

	// Start listening
	[gSpeechRecognizer startListening];
    
}

- (void)speechRecognizer:(NSSpeechRecognizer *)sender didRecognizeCommand:(id)command
{
	[_fauxTunes performSelector:(SEL)[[gCommandsDictionary objectForKey:command] longValue]];
}

@end

/*
    SetupCommandNameTable
    
*/

NSDictionary * CreateCommandsDictionary()
{

	return [[NSDictionary alloc] initWithObjectsAndKeys:
				// File Menu
				[NSNumber numberWithLong:(long)@selector(fileMenuNewPlaylist:)], @"Make new playlist",
				[NSNumber numberWithLong:(long)@selector(fileMenuNewPlaylistFromSelection:)], @"New playlist from selection",
				[NSNumber numberWithLong:(long)@selector(fileMenuAddToLibrary:)], @"Add to library",
				[NSNumber numberWithLong:(long)@selector(fileMenuHideOrShowWindow:)], @"Show fauxTunes Window",
				[NSNumber numberWithLong:(long)@selector(fileMenuHideOrShowWindow:)], @"Hide fauxTunes Window",
				[NSNumber numberWithLong:(long)@selector(fileMenuCloseWindow:)], @"Close this window",
				[NSNumber numberWithLong:(long)@selector(fileMenuGetInfo:)], @"Get song info",
				[NSNumber numberWithLong:(long)@selector(fileMenuShowSongFile:)], @"Show song file",
				[NSNumber numberWithLong:(long)@selector(fileMenuShopForProducts:)], @"Shop for fauxTunes products",
				// Edit Menu
				[NSNumber numberWithLong:(long)@selector(editMenuUndo:)], @"Cancel last command",
				[NSNumber numberWithLong:(long)@selector(editMenuCopy:)], @"Copy this to the clipboard",
				[NSNumber numberWithLong:(long)@selector(editMenuPaste:)], @"Paste the clipboard here",
				[NSNumber numberWithLong:(long)@selector(editMenuSelectAll:)], @"Select all songs",
				[NSNumber numberWithLong:(long)@selector(editMenuSelectNone:)], @"Select none",
				[NSNumber numberWithLong:(long)@selector(editMenuShowCurrentSong:)], @"Show current song",
				[NSNumber numberWithLong:(long)@selector(editMenuViewOptions:)], @"Show view options",
				// Controls Menu
				[NSNumber numberWithLong:(long)@selector(controlsMenuPlayOrStopSong:)], @"Play this song",
				[NSNumber numberWithLong:(long)@selector(controlsMenuPlayOrStopSong:)], @"Stop playing song",
				[NSNumber numberWithLong:(long)@selector(controlsMenuNextSong:)], @"Go to next song",
				[NSNumber numberWithLong:(long)@selector(controlsMenuPreviousSong:)], @"Go to previous song",
				[NSNumber numberWithLong:(long)@selector(controlsMenuShuffle:)], @"Shuffle songs",
				[NSNumber numberWithLong:(long)@selector(controlsMenuRepeatOff:)], @"Turn repeating off",
				[NSNumber numberWithLong:(long)@selector(controlsMenuRepeatAll:)], @"Repeat all songs",
				[NSNumber numberWithLong:(long)@selector(controlsMenuRepeatOne:)], @"Repeat one song",
				[NSNumber numberWithLong:(long)@selector(controlsMenuVolumeUp:)],  @"Turn volume up",
				[NSNumber numberWithLong:(long)@selector(controlsMenuVolumeDown:)], @"Turn volume down",
				[NSNumber numberWithLong:(long)@selector(controlsMenuMute:)], @"Mute volume",
				[NSNumber numberWithLong:(long)@selector(controlsMenuEjectCD:)], @"Eject the CD",
				// Visuals Menu
				[NSNumber numberWithLong:(long)@selector(visualsMenuToggleVisuals:)], @"Display visual",
				[NSNumber numberWithLong:(long)@selector(visualsMenuToggleVisuals:)], @"Turn visual off",
				[NSNumber numberWithLong:(long)@selector(visualsMenuSmall:)], @"Show visual at small size",
				[NSNumber numberWithLong:(long)@selector(visualsMenuMedium:)], @"Show visual at meduim size",
				[NSNumber numberWithLong:(long)@selector(visualsMenuLarge:)], @"Show visual at large size",
				[NSNumber numberWithLong:(long)@selector(visualsMenuFullScreen:)], @"Display full screen",
				[NSNumber numberWithLong:(long)@selector(visualsMenuFullScreen:)], @"Turn full screen off",
				// Advanced Menu
				[NSNumber numberWithLong:(long)@selector(advancedMenuOpenStream:)], @"Open stream",
				[NSNumber numberWithLong:(long)@selector(advancedMenuConvertToMP3:)], @"Convert to MP3",
				[NSNumber numberWithLong:(long)@selector(advancedMenuExportSongList:)], @"Export song list",
				[NSNumber numberWithLong:(long)@selector(advancedMenuGetCDTrackNames:)], @"Get CD track names",
				[NSNumber numberWithLong:(long)@selector(advancedMenuSubmitCDTrackNames:)], @"Submit CD track names",
				[NSNumber numberWithLong:(long)@selector(visualsMenuFullScreen:)], @"Convert ID3 tags",
				// Help Menu
				[NSNumber numberWithLong:(long)@selector(helpMenuGetHelp:)], @"Get help",
		NULL];
}


/*
    main
    
    This is the standard main routine for Cocoa applications
*/

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}

