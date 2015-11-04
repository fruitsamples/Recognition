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


	InstallSpeakableItems.c
	
	Copyright (c) 2001-2007 Apple Inc. All rights reserved.
*/

#include <Carbon/Carbon.h>

// Prototypes
OSErr NewAliasFile(FSRef * inParentContainerFSRef, CFStringRef inAliasFileName, FSRef * inTargetFileFSRef, Boolean inReplaceIfExists, Boolean inMakeInvisible, FSRef * outAliasFileFSRef);

void InstallSpeakableItemsFolderContents(const FSRef * inSourceFolderFSRef, const FSRef * inTargetFolderFSRef, OSType inFileType, OSType inFileCreator, Boolean inRsrcForkInDataFork);


/*
    InstallSpeakableItemsForThisApplication
    
    This routines creates an application-specific folder inside the user's Speakable Items directory hierarchy, then
    installs Command and AppleScript data files from the application's bundle into the application-specific folder.
    Installing the items at run time instead of during install time is preferrable because the application may be
    self-contained and not need a separate installer.  It also allows the application to easily accomodate new
    users being created after the initial installation.
    
	This routine only creates and installs the application's items if the folder doesn't already exist and the user
    has turned on Speakable Items at least once, thereby creating the "Speakable Items" folder hierarchy in their 
    "Library/Speech/" folder.  You can optionally force a reinstall of items if the application's items folder
    already exists, for example to reinstall items the user may have thrown away.  This allows you to either, call
    this routine during every launch to create the folder if necessary, or call only when the user indicates that
    they want the spoken command feature enabled
    
	When using XCode, the Command and AppleScript data files are added to the project and specified to be
    copied (using a Copy File Build Phase) into separate folders inside the "Resources" folder.  Since AppleScript 
    files have resource forks, the resource fork must be moved to the data fork before adding the file to the project.  
    This routine then recreates the AppleScript resource fork using the data in the data fork.
    
*/

Boolean InstallSpeakableItemsForThisApplication(CFStringRef inCommandFilesResourcePath, CFStringRef inCompliedASDataFilesResourcePath, Boolean inForceReinstallation)
{
    #define kInvisibleFileName	CFSTR("Target Application Alias\r")

	Boolean					successfullyCreated		= false;
    CFURLRef				theSIAppDirCFURLRef		= NULL;
    CFURLRef				thisAppsSIDirCFURLRef	= NULL;
    CFStringRef				thisAppsDisplayName		= NULL;
    ProcessSerialNumber		thisAppsPSN = {kNoProcess, kNoProcess};

    GetCurrentProcess(&thisAppsPSN);

    // Grab info about this process
	CFDictionaryRef processDictionary = ProcessInformationCopyDictionary(&thisAppsPSN, kProcessDictionaryIncludeAllInformationMask);
    if (processDictionary) {
        
        if (CopyProcessName(&thisAppsPSN, &thisAppsDisplayName) == noErr && thisAppsDisplayName) {

            // Get the user's library folder.
			FSRef userLibraryFolderFSRef;
			if (FSFindFolder(kUserDomain, kDomainLibraryFolderType, true, &userLibraryFolderFSRef) == noErr) {
			
				CFURLRef userLibraryFolderCFURL = CFURLCreateFromFSRef(kCFAllocatorDefault, (const struct FSRef *)&userLibraryFolderFSRef);
				if (userLibraryFolderCFURL) {
				
					theSIAppDirCFURLRef = CFURLCreateCopyAppendingPathComponent(NULL, userLibraryFolderCFURL, CFSTR("/Speech/Speakable Items/Application Speakable Items/"), true);
					thisAppsSIDirCFURLRef = CFURLCreateCopyAppendingPathComponent(NULL, theSIAppDirCFURLRef, thisAppsDisplayName, true);
					
					CFRelease(userLibraryFolderCFURL);
                }
            }
        }
		CFRelease(processDictionary);
    }
    
    // If we found the user's "Application Speakable Items" folder, then look for this app's folder 
    FSRef theSIAppDirFSRef;
    FSRef thisAppsSIDirFSRef;
    if (theSIAppDirCFURLRef && thisAppsSIDirCFURLRef && CFURLGetFSRef(theSIAppDirCFURLRef, &theSIAppDirFSRef)) {
    
        // We'll create a new folder for our application's spekable items if it doesn't already exist.
        // NOTE:  If the "Speakable Items" folder hasn't be created, then the user hasn't run SI yet so there's no need to create our folder yet.
        if (! CFURLGetFSRef(thisAppsSIDirCFURLRef, &thisAppsSIDirFSRef)) {
    
            const CFIndex		maxNameLen = 300;
            UniChar				appNameAsUniChar[maxNameLen];
            UniCharCount      	appNameLength;

            appNameLength = CFStringGetLength(thisAppsDisplayName);
            if (appNameLength > maxNameLen)
                appNameLength = maxNameLen;
                
            CFStringGetCharacters(thisAppsDisplayName, CFRangeMake(0, appNameLength), appNameAsUniChar); 
        
            // Create the Directory
            if (FSCreateDirectoryUnicode(&theSIAppDirFSRef, appNameLength, appNameAsUniChar, 0, NULL, &thisAppsSIDirFSRef, NULL, NULL) == noErr) {
        
                // Since GetProcessInformation returns the actual executable of bundled applications, we must use
                // GetProcessBundleLocation to get the application bundle FSSpec.
                
                // Create the invisible alias file
                FSRef	bundleLocationFSRef;
                if (GetProcessBundleLocation(&thisAppsPSN, &bundleLocationFSRef) == noErr
					&& NewAliasFile(&thisAppsSIDirFSRef, kInvisibleFileName, &bundleLocationFSRef, true, true, NULL) == noErr) {
                    successfullyCreated = true;
				}
            }
            else if (inForceReinstallation) {
                successfullyCreated = true;
            }
        }
		else {
			// We reinstall our commands at every launch because Speakable Items in later Mac OS X releases automatically creates our application folder.
			// You can optimize this if you look in the folder first before deciding the install the commands.
			successfullyCreated = true;
		}
    } 
	
	if (thisAppsDisplayName) {
		CFRelease(thisAppsDisplayName);
	}
	if (theSIAppDirCFURLRef) {
		CFRelease(theSIAppDirCFURLRef);
	}
	if (thisAppsSIDirCFURLRef) {
		CFRelease(thisAppsSIDirCFURLRef);
	}
   
    if (successfullyCreated) {
    
        FSRef 		sourceFolderFSRef;
        CFURLRef	sourceCFURLRef;
        FSCatalogInfo  catalogInfo;

        // Install Command Files
        sourceCFURLRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), inCommandFilesResourcePath, NULL, NULL);
        if (sourceCFURLRef) {
            if (CFURLGetFSRef(sourceCFURLRef, &sourceFolderFSRef)) {
                InstallSpeakableItemsFolderContents(&sourceFolderFSRef, &thisAppsSIDirFSRef, 'sicf', 'siax', false);
            }
            CFRelease(sourceCFURLRef);
        }
    
        // Install AppleScript Files
        sourceCFURLRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), inCompliedASDataFilesResourcePath, NULL, NULL);
        if (sourceCFURLRef) {
            if (CFURLGetFSRef(sourceCFURLRef, &sourceFolderFSRef)) {
                InstallSpeakableItemsFolderContents(&sourceFolderFSRef, &thisAppsSIDirFSRef, 'osas', 'ToyS', true);
            }
            CFRelease(sourceCFURLRef);
        }
        
        // Touch the mod date of the application's folder so Speakable Item will update itself.
		CFTimeZoneRef systemTimeZone = CFTimeZoneCopySystem();
		if (systemTimeZone) {
			if (UCConvertCFAbsoluteTimeToUTCDateTime(CFTimeZoneGetSecondsFromGMT(systemTimeZone, CFAbsoluteTimeGetCurrent()), &(catalogInfo.contentModDate)) == noErr) {
				FSSetCatalogInfo(&thisAppsSIDirFSRef, kFSCatInfoContentMod, &catalogInfo);
			}
			CFRelease(systemTimeZone);
		}
    }
    
	return successfullyCreated;
}


/*
    NewAliasFile
    
    A utility routine used by InstallSpeakableItemsForThisApplication to create an alias file.
*/
    
OSErr NewAliasFile(FSRef * inParentContainerFSRef, CFStringRef inAliasFileName, FSRef * inTargetFileFSRef, Boolean inReplaceIfExists, Boolean inMakeInvisible, FSRef * outAliasFileFSRef)
{
	OSErr		theErr	= noErr;

	FSRef		theAliasFileFSRef;
	
	FSIORefNum	savedResFile = CurResFile();


    const UniCharCount		maxNameLen = 300;

    UniChar				aliasFileNameAsUniChar[maxNameLen];
    UniCharCount      	aliasFileNameLength;

    // Convert name to unicode
    aliasFileNameLength = CFStringGetLength(inAliasFileName);
    if (aliasFileNameLength > maxNameLen)
        aliasFileNameLength = maxNameLen;
        
    CFStringGetCharacters(inAliasFileName, CFRangeMake(0, aliasFileNameLength), aliasFileNameAsUniChar); 
    
    // If we're allowed to replace an existing copy, delete the existing one first
    // Otherwise, the FSCreateFile will fail below and we'll skip adding the resource.
    if (inReplaceIfExists && FSMakeFSRefUnicode(inParentContainerFSRef, aliasFileNameLength, aliasFileNameAsUniChar, 0, &theAliasFileFSRef) == noErr)
        FSDeleteObject(&theAliasFileFSRef);
    
    // Create the file.
    FSCreateResFile(inParentContainerFSRef, aliasFileNameLength, aliasFileNameAsUniChar, 0, NULL, &theAliasFileFSRef, NULL);
	theErr = ResError();
    
	if(! theErr) {
		
        FSIORefNum theAliasResFileNum = FSOpenResFile(&theAliasFileFSRef, fsRdWrPerm);
		if (theAliasResFileNum != -1){
			
            // Add the alias resource
            AliasHandle	theAliasHandle;
            theErr = FSNewAlias(NULL, inTargetFileFSRef, &theAliasHandle);
			if (! theErr) {
				AddResource((Handle)theAliasHandle, rAliasType, 0, "\p");
				theErr = ResError();
			}
			
			CloseResFile(theAliasResFileNum);
		}
	}
	
	UseResFile(savedResFile);
	
	
	//
	// Set Finder flags to an invisible alias file
	//
    if (! theErr) {
    
        FSCatalogInfo	theCatalogInfo;
        
        theErr = FSGetCatalogInfo(&theAliasFileFSRef, kFSCatInfoFinderInfo, &theCatalogInfo, NULL, NULL, NULL);
        if (! theErr) {

            ((FileInfo *)theCatalogInfo.finderInfo)->finderFlags |= kIsAlias;

            if (inMakeInvisible)
                ((FileInfo *)theCatalogInfo.finderInfo)->finderFlags |= kIsInvisible;

            theErr = FSSetCatalogInfo(&theAliasFileFSRef, kFSCatInfoFinderInfo, &theCatalogInfo);
        }
		
		if (outAliasFileFSRef) {
			*outAliasFileFSRef = theAliasFileFSRef;
		}
        
    }
	
	return theErr;
}

/*
    InstallSpeakableItemsFolderContents
    
    A utility routine used by InstallSpeakableItemsForThisApplication to copy the contents of a folder.
*/

void InstallSpeakableItemsFolderContents(const FSRef * inSourceFolderFSRef, const FSRef * inTargetFolderFSRef, OSType inFileType, OSType inFileCreator, Boolean inRsrcForkInDataFork)
{

    const	ItemCount	kMaxInteratorCount = 100;

    OSStatus		status = noErr;
    FSIterator      sourceIterator;
    ItemCount		sourceDirObjectCount = 0;    
    FSRef *			sourceFSRefArray = NULL;    
    FSCatalogInfo *	sourceFSCatInfoArray = NULL;
    HFSUniStr255 *	sourceHFSUniNameArray = NULL;
    UInt32			sourceFileIndex = 0;

    sourceFSRefArray = malloc(kMaxInteratorCount * sizeof(FSRef));
    sourceFSCatInfoArray = malloc(kMaxInteratorCount * sizeof(FSCatalogInfo));
    sourceHFSUniNameArray = malloc(kMaxInteratorCount * sizeof(HFSUniStr255));
    
    status = FSOpenIterator(inSourceFolderFSRef, kFSIterateFlat, &sourceIterator);
   
    if (!status && sourceFSRefArray && sourceFSCatInfoArray && sourceHFSUniNameArray) {
            
        do {
    
            // Grab a batch of source files to process from the source directory
            status = FSGetCatalogInfoBulk(sourceIterator, kMaxInteratorCount, &sourceDirObjectCount, NULL, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo | kFSCatInfoDataSizes, sourceFSCatInfoArray, sourceFSRefArray, NULL, sourceHFSUniNameArray);
            if ((status == errFSNoMoreItems || status == noErr) && sourceDirObjectCount) {
                status = noErr;
    
                for (sourceFileIndex = 0; sourceFileIndex < sourceDirObjectCount; sourceFileIndex++) {
                                    
                    // Only copy files, not directories
                    if (! (sourceFSCatInfoArray[sourceFileIndex].nodeFlags & kFSNodeIsDirectoryMask)) {
            
                        FSRef			targetFileFSRef;
                        FSCatalogInfo	targetFileCat;
                        FSIORefNum		itemToBeCopiedDataForkRefNum = -1;
            
                        // Open data and resource fork of "from" file
                        status = FSOpenFork(&(sourceFSRefArray[sourceFileIndex]), 0, NULL, fsRdPerm, &itemToBeCopiedDataForkRefNum);
                        
                        if (! status) {
                            memcpy(targetFileCat.finderInfo, sourceFSCatInfoArray[sourceFileIndex].finderInfo, sizeof(UInt8) * 16);
                            
                            ((FileInfo *)targetFileCat.finderInfo)->fileType		= inFileType; 
                            ((FileInfo *)targetFileCat.finderInfo)->fileCreator 	= inFileCreator;

                            status = FSCreateFileUnicode(inTargetFolderFSRef, sourceHFSUniNameArray[sourceFileIndex].length, sourceHFSUniNameArray[sourceFileIndex].unicode,kFSCatInfoFinderInfo, &targetFileCat, &targetFileFSRef, NULL);
                        }
                                
                        if (! status && itemToBeCopiedDataForkRefNum != -1) {
                            
                            Size	theDataCount = sourceFSCatInfoArray[sourceFileIndex].dataLogicalSize;
                            void *	theDataBuffer = malloc(theDataCount);
                            if (theDataBuffer) {
                            
                                FSIORefNum		newForkRefNum = -1;
                                HFSUniStr255	newForkName;
            
                                // Create the fork specified my the given flag
                                if (inRsrcForkInDataFork)
                                    status = FSGetResourceForkName(&newForkName);
                                else
                                    status = FSGetDataForkName(&newForkName);
                        
                                if (! status)
                                    status = FSCreateFork(&targetFileFSRef, newForkName.length, newForkName.unicode);
                                
                                if (! status)
                                    status = FSOpenFork(&targetFileFSRef, newForkName.length, newForkName.unicode, fsRdWrPerm, &newForkRefNum);
                                    
                                if (! status)
                                    status = FSReadFork(itemToBeCopiedDataForkRefNum, fsFromStart, 0, theDataCount, theDataBuffer, (ByteCount *) &theDataCount);
                                    
                                if (! status)
                                    status = FSWriteFork(newForkRefNum, fsFromStart, 0, theDataCount, theDataBuffer, (ByteCount *) &theDataCount);
                    
                                if (newForkRefNum != -1)
                                    FSCloseFork(newForkRefNum);
                            
                                free(theDataBuffer);
                            }
                        }
                        
                        // Close up our source files.
                        if (itemToBeCopiedDataForkRefNum != -1)
                            FSCloseFork(itemToBeCopiedDataForkRefNum);
                    }
                
                }
                
            }
            
        } while (! status);
        
        free(sourceFSRefArray);
        free(sourceFSCatInfoArray);
        free(sourceHFSUniNameArray);
    }
}
