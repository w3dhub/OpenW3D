/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/Installer.cpp  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/17/02 7:43p                 $* 
 *                                                                                             * 
 *                    $Revision:: 18                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Include files.
#include "Installer.h"
#include "Argv.h"
#include "AssetMgr.h"
#include "AssetStatus.h"
#include "AudibleSound.h"
#include "BINKMovie.h"
#include "CardSelectionDialog.h"
#include "CodeControl.h"
#include "CopyDialog.h"
#include "DialogMgr.h"
#include "DirectoryDialog.h"
#include "DX8Wrapper.h"
#include "ErrorHandler.h"
#include "FinalDialog.h"
#include "FolderDialog.h"
#include "Ini.h"
#include "LicenseDialog.h"
#include "MixFile.h"
#include "Msgloop.h"
#include "registry.h"
#include "ReviewDialog.h"
#include "SafeTimer.h"
#include "SerialDialog.h"
#include "Timer.h"
#include "TransitionDialog.h"
#include "Translator.h"
#include "Utilities.h"
#include "WelcomeDialog.h"
#include "WhatToInstallDialog.h"
#include "Ww3D.h"
#include "Wwmath.h"
#include "WOL1Dialog.h"
#include "WOL2Dialog.h"
#include <io.h>
#include <objbase.h>
#include <shlobj.h>
#include <intshcut.h>

// Section names and entry names for IDS_GAME_INI_FILENAME.
#define INI_CD_LABEL_SECTION					"CDLabel"
#define INI_SUBDIRECTORY_SECTION				"SubDirectories"
#define INI_FILES_SECTION						"Files"
#define INI_VOLUME_LABEL_ENTRY				"VolumeLabel"		
#define INI_SUBDIRECTORY_ENTRY_PREFIX		"Dir"
#define INI_TOTAL_DISK_SPACE_NEEDED_ENTRY	"TotalDiskSpaceNeeded"
#define INI_NUM_FILES_ENTRY					"NumFiles"

// Section names and entry names for IDS_WOLAPI_INI_FILENAME.
#define INI_WOLAPI_SECTION						"Wolapi"
#define INI_WOLREGISTER_SECTION				"Register"
#define INI_WOLBROWSER_SECTION 				"Browser"
#define INI_VERSION_MAJOR_ENTRY				"Major"
#define INI_VERSION_MINOR_ENTRY				"Minor"

// Section names and entry names for IDS_SETUP_INI_FILENAME.
#define INI_SETUP_SECTION						"Setup"
#define INI_LANGUAGE_ENTRY						"Language"	
#define INI_SKU_ENTRY							"SKU"
#define INI_CAN_USE_IGR_SETTNGS_ENTRY		"CanUseIGRSettings"

// Miscellaneous.
#define ENCRYPTION_STRING_LENGTH				128	
#define INSTALLER_NOMINAL_BYTE_COUNT		0x40000

// Registry access.
#define APPLICATION_SUB_KEY_NAME				"Software\\Westwood\\Renegade"
#define APPLICATION_SUB_KEY_NAME_RENDER	"Software\\Westwood\\Renegade\\Render"
#define APPLICATION_SUB_KEY_NAME_OPTIONS	"Software\\Westwood\\Renegade\\Options"

// Macros.
#define END_DIALOG(dialog) \
	if (dialog != NULL) { \
		dialog->End_Dialog(); \
	}

#define REGISTER_DIALOG(dialogclass, dialog, args) \
  if (dialog == NULL) { \
		dialog = NEW_REF (dialogclass, args); \
		dialog->Start_Dialog();	\
	} else { \
		DialogMgrClass::Register_Dialog (dialog); \
	} 

#define UNREGISTER_DIALOG(dialog) \
	DialogMgrClass::UnRegister_Dialog (dialog); 

#define DO_CANCEL \
	CancelApplication = true; \
	PostQuitMessage (0); 


// Foward declarations.
void WWDebug_Message_Callback (DebugType type, const char *message);
void WWAssert_Callback (const char *message);


// Prototypes.
bool Default_On_Command (DialogBaseClass *dialog, int ctrl_id, int mesage_id, DWORD param);


// Singleton.
InstallerClass _Installer;


// Type definitions.
typedef HRESULT (*DLLREGISTERSERVER)(void);
typedef HRESULT (*DLLUNREGISTERSERVER)(void);


/***********************************************************************************************
 * InstallerClass::InstallerClass --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
InstallerClass::InstallerClass() 
	:	SetupIni (NULL),
		GameIni (NULL),
		WOLIni (NULL),
		AssetManager (NULL),
		FreshGameInstall (true),
		FreshWOLInstall (true),
		TargetWOLOlder (true),
		WwmathInitialized (false),
		DialogMgrInitialized (false),
		AudioSystem (NULL),
		TransitionMusic (NULL),
		InstallMusic (NULL),
		UIInput (NULL),
		CancelIntroduction (false),
		CancelApplication (false),
		WelcomeDialog (NULL),
		NDADialog (NULL),
		LicenseDialog (NULL),
		SerialDialog (NULL),
		WhatToInstallDialog (NULL),
		GameDirectoryDialog (NULL),
		GameFolderDialog (NULL),
		WOLDirectoryDialog (NULL),
		WOLFolderDialog (NULL),
		ReviewDialog (NULL),
		CopyDialog (NULL),
		WOL1Dialog (NULL),
		WOL2Dialog (NULL),
		FinalDialog (NULL),
		GameFileCount (0),
		GameSize (0),
		WOLFileCount (0),
		WOLSize (0),
		CardSelectionDialog (NULL),
		ProgEndAction (ACTION_NONE)
{
	SetupIni = new INIClass;
	GameIni	= new INIClass;
	WOLIni	= new INIClass;
}


/***********************************************************************************************
 * InstallerClass::~InstallerClass --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
InstallerClass::~InstallerClass()
{
	delete WOLIni;
	delete GameIni;
	delete SetupIni;
}


/***********************************************************************************************
 * InstallerClass::Install --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Install (MixFileFactoryClass *mixfilefactory)
{
	try {

		const Vector3 black (0.0f, 0.0f, 0.0f);

		MixFileFactory = mixfilefactory;

		// Install debug callbacks.
		WWDebug_Install_Message_Handler (WWDebug_Message_Callback);
		WWDebug_Install_Assert_Handler (WWAssert_Callback);

		// Check that the source data for the install is present.
		if (Check_Source()) {

			const char *intromoviename	= "InstallMovie.bik";
			const int	playouttime		= 2000;

			bool cancelapplication = false;	
			int  time;	

			Check_Existing_Install();
			Initialize();

			// If the user has more than one video card, run the video card selection dialog.
			CardSelectionDialog = NEW_REF (CardSelectionDialogClass, (false));
			while (CardSelectionDialog->Get_Card_Count() > 1) {

				int cardselection;	

				CardSelectionDialog->Start_Dialog();
				while (CardSelectionDialog->Is_Running()) {

					DialogMgrClass::On_Frame_Update();
					WW3D::Begin_Render (true, true, black);
					if (GameInFocus) {
						DialogMgrClass::Render();
					} else {
						Sleep (50);
					}
					WW3D::End_Render();
					Windows_Message_Handler();		
				}
				cardselection = CardSelectionDialog->Get_Card_Selection();
				cancelapplication = CardSelectionDialog->Cancel_Application();

				if (cancelapplication) break;
				
				// NOTE: Must release all dialogs before attempting initialization.
				REF_PTR_RELEASE (CardSelectionDialog);
				WW3D_Shutdown();
				if (WW3D_Initialize (cardselection)) break;

				// Re-initialize with the default video card and driver and inform the user
				// that their selected card would not initialize.
				WW3D_Shutdown();
				WW3D_Initialize();
				CardSelectionDialog = NEW_REF (CardSelectionDialogClass, (true));
			}
			REF_PTR_RELEASE (CardSelectionDialog);

			// Does the user wish to exit?
			if (!cancelapplication) {

				if (!Beta_Test()) {

					// Play BINK intro movie.
					BINKMovie::Init();
					BINKMovie::Play (intromoviename);
					while (!BINKMovie::Is_Complete()) {
						if (CancelIntroduction) {
							BINKMovie::Stop();
							break;
						}
						BINKMovie::Update();
						WW3D::Begin_Render (false, false, black);
						if (GameInFocus) {
							BINKMovie::Render();
						} else {
							Sleep (50);
						}
						WW3D::End_Render();
						Windows_Message_Handler();		
					}
					BINKMovie::Shutdown();

					// For artistic reasons, end the movie on a black frame.
					WW3D::Begin_Render (true, true, black);
					WW3D::End_Render();

					// If the user cancelled during the intro movie, then don't bother with the transition.
					if (!CancelIntroduction) {

						TransitionDialogClass *transition;

						// Create the transition animation.
						transition = NEW_REF (TransitionDialogClass, ());
						transition->Start_Dialog();

						// Start the transition music.
						if (TransitionMusic != NULL) TransitionMusic->Play();

						TTimerClass <SafeTimerClass> timer;
						
						// Play.
						while (true) {

							DialogMgrClass::On_Frame_Update();

							// Has the dialog 'played out'?
							// NOTE: Wait for both video and audio to finish.
							if (!transition->Is_Running()) {
								if (TransitionMusic != NULL) {
									if (!TransitionMusic->Is_Playing()) break;
								} else {
									break;
								}
							}

							WW3D::Begin_Render (true, true, black);
							if (GameInFocus) {
								DialogMgrClass::Render();
								if (TransitionMusic != NULL) {
									if (TransitionMusic->Get_State() == AudibleSoundClass::STATE_PAUSED) TransitionMusic->Resume();
								}
								if (!timer.Is_Active()) timer.Start();
							} else {
								if (TransitionMusic != NULL) {
									if (TransitionMusic->Get_State() == AudibleSoundClass::STATE_PLAYING) TransitionMusic->Pause();
								}
								if (timer.Is_Active()) timer.Stop();
								Sleep (50);
							}
							WW3D::End_Render();

							WW3D::Sync (timer.Value());
							WWAudioClass::Get_Instance()->On_Frame_Update();
							Windows_Message_Handler();		

							// Has the user cancelled the transition?
							if (CancelIntroduction) {
								transition->End_Dialog();
								if (TransitionMusic != NULL) TransitionMusic->Stop();
								break;
							}
						}
						REF_PTR_RELEASE (transition);
					}
				}

				// Set the command handler for all subsequent dialogs.
				DialogBaseClass::Set_Default_Command_Handler (Default_On_Command);

				// Put up the first dialog.
				Default_On_Command (NULL, 0, 0, 0);

				// Start the install music.
				if (InstallMusic != NULL) InstallMusic->Play();

				TTimerClass <SafeTimerClass> timer;

				// Run dialog loop.
				while (!CancelApplication) {

					DialogMgrClass::On_Frame_Update();

					WW3D::Begin_Render (true, true, black);
					if (GameInFocus) {
						DialogMgrClass::Render();
						if (InstallMusic != NULL) {
							if (InstallMusic->Get_State() == AudibleSoundClass::STATE_PAUSED) InstallMusic->Resume();
						}
						if (!timer.Is_Active()) timer.Start();
					} else {
						if (InstallMusic != NULL) {
							if (InstallMusic->Get_State() == AudibleSoundClass::STATE_PLAYING) InstallMusic->Pause();
						}
						if (timer.Is_Active()) timer.Stop();
						Sleep (50);
					}
					WW3D::End_Render();

					WW3D::Sync (timer.Value());
					WWAudioClass::Get_Instance()->On_Frame_Update();
					Windows_Message_Handler();
				}

				// Before shutting down, allow a short time for sound effects to 'play out'.
				time = timer.Value();
				while (timer.Value() < time + playouttime) {
					WW3D::Sync (timer.Value());
					WWAudioClass::Get_Instance()->On_Frame_Update();
					Windows_Message_Handler();
				}

				// Clear the frame buffer.
				WW3D::Begin_Render (true, true, black);
				WW3D::End_Render();
			}
			Shutdown();
		}
	}
	
	catch (const WideStringClass &errormessage) {

		Shutdown();
		throw (errormessage);
	}
}


/***********************************************************************************************
 * InstallerClass::Check_Source --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Check_Source()
{
	ArgvClass commandlineparser (false, true);			

	// Use the simple file factory to read INI files from the current working directory.
	_TheFileFactory = _TheSimpleFileFactory;

	#if PERMIT_INSTALL_FROM_NETWORK
	if (!commandlineparser.Find ("-N")) {
	#else
	if (true) {
	#endif
 	
		char sourcedriveletter;	

		// Prompt for a CD with correct volume label.
		if (Prompt_Install_CD (RxWideStringClass (IDS_INSTALL_CD_VOLUME_NAME), sourcedriveletter)) {
		
			char sourcedrive [] = "?:";
		
			sourcedrive [0] = tolower (sourcedriveletter);
			SourceGamePath  = sourcedrive;
			SourceWOLPath   = sourcedrive;

		} else {

			// No CD found.
			return (false);
		}

	} else {

		#if PERMIT_INSTALL_FROM_NETWORK

		Get_Current_Directory (SourceGamePath);
		Remove_Trailing_Name (SourceGamePath);
		SourceWOLPath = SourceGamePath;
		#endif
	}

	// Initialize the game and WOL source paths.
	SourceGamePath += L"\\";
	SourceGamePath += RxWideStringClass (IDS_GAME_DIRECTORY_NAME);
	SourceWOLPath	+= L"\\";
	SourceWOLPath  += RxWideStringClass (IDS_WOL_DIRECTORY_NAME);

	// Look in the current directory for Setup.ini file.
	if (!SetupIni->Is_Loaded()) {
		if (!SetupIni->Load (RxStringClass (IDS_SETUP_INI_FILENAME))) FATAL_APP_ERROR (IDS_INI_FILE_NOT_FOUND_OR_LOADED);
	}

	// Look in the current directory for <product name>.ini file.
	if (!GameIni->Is_Loaded()) {
		if (!GameIni->Load (RxStringClass (IDS_GAME_INI_FILENAME))) FATAL_APP_ERROR (IDS_INI_FILE_NOT_FOUND_OR_LOADED);
	}

	// Look in the WOL source directory for WOLAPI.ini file.
	if (!WOLIni->Is_Loaded()) {

		WideStringClass wolinipathname;
		StringClass		 multibytewolinipathname;

		wolinipathname  = SourceWOLPath;
		wolinipathname += L"\\";
		wolinipathname += RxWideStringClass (IDS_WOL_INI_FILENAME);
	
		multibytewolinipathname = wolinipathname;
		if (!WOLIni->Load (multibytewolinipathname)) FATAL_APP_ERROR (IDS_INI_FILE_NOT_FOUND_OR_LOADED);
	}

	// Count no. and total file size of contents of game directory.
	Count_Source_Files (SourceGamePath, GameFileCount, GameSize);
	
	//	Add game file count and size contained in CAB files.
	// WARNING: Only values < 2Gb will be reported correctly due to 32-bit limit on INIClass::Get_Int().
	//				However, result is returned in 64-bits so that all callers are ready to handle >= 2Gb when the need arises.
	GameFileCount += GameIni->Get_Int (INI_FILES_SECTION, INI_NUM_FILES_ENTRY);
	GameSize		  += GameIni->Get_Int (INI_FILES_SECTION, INI_TOTAL_DISK_SPACE_NEEDED_ENTRY);

	// Count no. and total file size of contents of WOL directory.
	// NOTE: WOL directory does not contain CAB files.
	Count_Source_Files (SourceWOLPath, WOLFileCount, WOLSize);

	// Return success.
	return (true);
}


/***********************************************************************************************
 * InstallerClass::Count_Source_Files --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Count_Source_Files (const WideStringClass &sourcepath, unsigned &filecount, int64_t &filesize)
{
	const wchar_t *wildcardname = L"*.*";

	WideStringClass sourcepathname (sourcepath);
	StringClass		 multibytesourcewolpath;	
	WIN32_FIND_DATA finddata;	
	HANDLE			 handle;
	bool				 done = false;

	sourcepathname += L"\\";
	sourcepathname += wildcardname;
	multibytesourcewolpath = sourcepathname;
	handle = FindFirstFile (multibytesourcewolpath, &finddata);
	if (handle != INVALID_HANDLE_VALUE) {
		while (!done) {
			
			WideStringClass filename (finddata.cFileName);
			
			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			
				// Filter out system diretories.
				if (filename [0] != L'.') {
				
					WideStringClass subsourcepath;

					// Recurse on subdirectory..
					subsourcepath  = sourcepath;
					subsourcepath += L"\\";
					subsourcepath += filename;
					Count_Source_Files (subsourcepath, filecount, filesize);
				}

			} else {
				
				const wchar_t *cabextension = L".cab";

  				wchar_t extension [_MAX_EXT];

				// Filter out CAB files.
				_wsplitpath (filename, NULL, NULL, NULL, extension);
				if (_wcsicmp (cabextension, extension) != 0) {
					filecount++;
					filesize += (finddata.nFileSizeHigh * ((int64_t) MAXDWORD) + 1) + finddata.nFileSizeLow;
				}
			}
			if (done = FindNextFile (handle, &finddata) == 0) {
				if (GetLastError() != ERROR_NO_MORE_FILES) FATAL_SYSTEM_ERROR;
			}
		}
		if (!FindClose (handle)) FATAL_SYSTEM_ERROR;
	}
}


/***********************************************************************************************
 * InstallerClass::Check_Existing_Install --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Check_Existing_Install()
{
	WideStringClass pathname;
	StringClass		 multibytepathname;	
	WIN32_FIND_DATA finddata;	
	HANDLE			 handle;
	bool				 fresh;
	bool				 older;

	// Check for existence of target game .exe.
	fresh = true;
	if (_RegistryManager.Get_Target_Game_Pathname (pathname)) {
		multibytepathname = pathname;
		handle = FindFirstFile (multibytepathname, &finddata);
		if (handle != INVALID_HANDLE_VALUE) {
			FindClose (handle);
			fresh = false;
		}
	}
	FreshGameInstall = fresh;

 	// Iterate over all WOL components. Install is a fresh install if one or more components are
	// missing from the user's system. The target is deemed older that the source if one or more
	// target version nos. is older than source.
	const char *sectionname [RegistryManagerClass::COMPONENT_COUNT] = {INI_WOLAPI_SECTION, INI_WOLREGISTER_SECTION, INI_WOLBROWSER_SECTION};

	fresh = false;
	older = false;
	for (int c = RegistryManagerClass::COMPONENT_FIRST; c < RegistryManagerClass::COMPONENT_COUNT; c++) {
		
		WideStringClass pathname;
		StringClass		 multibytepathname;	
		WIN32_FIND_DATA finddata;	
		HANDLE			 handle;
		DWORD				 major, minor, sourceversion, targetversion;

		// Check for existence of target .exe.
		if (!_RegistryManager.Get_Target_WOL_Pathname ((RegistryManagerClass::WOLComponentEnum) c, pathname)) {
			fresh = true;
			older = true;
			break;
		}
		multibytepathname = pathname;
		handle = FindFirstFile (multibytepathname, &finddata); 
		if (handle == INVALID_HANDLE_VALUE) {
			fresh = true;
			older = true;
			break;
		}
		FindClose (handle);

		// Compare source and target versions.
		if (!_RegistryManager.Get_Target_WOL_Version ((RegistryManagerClass::WOLComponentEnum) c, targetversion)) {
			older = true;
			break;
		}
		major = WOLIni->Get_Int (sectionname [c], INI_VERSION_MAJOR_ENTRY, 0x1);
		minor = WOLIni->Get_Int (sectionname [c], INI_VERSION_MINOR_ENTRY, 0x0);
		sourceversion = (major << 16) + minor;
		if (sourceversion > targetversion) {
			older = true;
			break;
		}
	}

	FreshWOLInstall = fresh;
	TargetWOLOlder  = older;	
}


/***********************************************************************************************
 * InstallerClass::Initialize --																					  *
 *																															  *			
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Initialize()
{
	const char *transitionmusicfilename	= "TransitionMusic.mp3";
	const char *installmusicfilename		= "InstallMusic.mp3";
	
	// All subsequent file factory access uses the mix file.
	_TheFileFactory = MixFileFactory;

	// Initialize WW systems.
	WWMath::Init();
	WwmathInitialized = true;
	if (!WW3D_Initialize()) FATAL_APP_ERROR (IDS_CANNOT_INITIALIZE_ANY_D3D_DEVICE);

	// Initialize the music.
	AudioSystem = WWAudioClass::Create_Instance();
	WWAudioClass::Get_Instance()->Initialize();
	WWAudioClass::Get_Instance()->Set_File_Factory (_TheFileFactory);
	TransitionMusic = WWAudioClass::Get_Instance()->Create_Sound_Effect (transitionmusicfilename);
	if (TransitionMusic != NULL) {
		TransitionMusic->Set_Type (AudibleSoundClass::TYPE_MUSIC);
	}
	InstallMusic = WWAudioClass::Get_Instance()->Create_Sound_Effect (installmusicfilename);
	if (InstallMusic != NULL) {
		InstallMusic->Set_Type (AudibleSoundClass::TYPE_MUSIC);
		InstallMusic->Set_Loop_Count (0);
	}
}


/***********************************************************************************************
 * InstallerClass::Shutdown --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Shutdown()
{
	// Close any open dialogs.
	END_DIALOG (WelcomeDialog)
	REF_PTR_RELEASE (WelcomeDialog)
	END_DIALOG (NDADialog)
	REF_PTR_RELEASE (NDADialog)
	END_DIALOG (LicenseDialog)
	REF_PTR_RELEASE (LicenseDialog)
	END_DIALOG (SerialDialog)
	REF_PTR_RELEASE (SerialDialog)
	END_DIALOG (WhatToInstallDialog)
	REF_PTR_RELEASE (WhatToInstallDialog)
	END_DIALOG (GameDirectoryDialog)
	REF_PTR_RELEASE (GameDirectoryDialog)
	END_DIALOG (GameFolderDialog)
	REF_PTR_RELEASE (GameFolderDialog)
	END_DIALOG (WOLDirectoryDialog)
	REF_PTR_RELEASE (WOLDirectoryDialog)
	END_DIALOG (WOLFolderDialog)
	REF_PTR_RELEASE (WOLFolderDialog)
	END_DIALOG (ReviewDialog)
	REF_PTR_RELEASE (ReviewDialog)
	END_DIALOG (CopyDialog)
	REF_PTR_RELEASE (CopyDialog)
	END_DIALOG (WOL1Dialog)
	REF_PTR_RELEASE (WOL1Dialog)
	END_DIALOG (WOL2Dialog)
	REF_PTR_RELEASE (WOL2Dialog)
	END_DIALOG (FinalDialog)
	REF_PTR_RELEASE (FinalDialog)
	END_DIALOG (CardSelectionDialog)
	REF_PTR_RELEASE (CardSelectionDialog)

	// Shutdown the music.
	if (TransitionMusic != NULL) {
		TransitionMusic->Stop();
		TransitionMusic->Release_Ref();
	}
	if (InstallMusic != NULL) {
		InstallMusic->Stop();
		InstallMusic->Release_Ref();
	}
	if (AudioSystem != NULL) {
		WWAudioClass::Get_Instance()->Shutdown();
		delete AudioSystem;
	}

	// Shutdown WW systems.
	WW3D_Shutdown();
	if (WwmathInitialized) WWMath::Shutdown();
}


/***********************************************************************************************
 * InstallerClass::WW3D_Initialize --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::WW3D_Initialize (int cardselection)
{
	const char *stylemgrini = "StyleMgr.ini";
	
	bool windowedmode;

	// Initialize WW3D.
	AssetManager = new WW3DAssetManager;
	AssetManager->Set_WW3D_Load_On_Demand (true);
	WW3D::Init (MainWindow);	
	WW3D::Enable_Sorting (true);
	WW3D::Set_Thumbnail_Enabled (false);
	AssetStatusClass::Peek_Instance()->Enable_Reporting (false);

	// All subsequent file factory access uses the mix file.
	_TheFileFactory = MixFileFactory;

	ArgvClass commandlineparser (false, true);
	
	windowedmode = (commandlineparser.Find ("-W") != NULL);

	// For each available video card...
	for (int d = 0; d < WW3D::Get_Render_Device_Count(); d++) {

		// Does this card index match the supplied card index or should we try all cards until success?
		if ((cardselection == -1) || (cardselection == d)) {
		
			// Set the card index and whether windowed mode in the registry.
			WW3D::Registry_Save_Render_Device (APPLICATION_SUB_KEY_NAME_RENDER, d, -1, -1, -1, 0, -1);
		
			// Configure video for the user.
			Auto_Configure();

			// Set UV bias according to registry.
			RegistryClass optionsregistry (APPLICATION_SUB_KEY_NAME_OPTIONS);
			if (optionsregistry.Is_Valid()) {
				WW3D::Set_Screen_UV_Bias (optionsregistry.Get_Int ("ScreenUVBias", 1) != 0);
			}

			// Override the windowed mode.
			RegistryClass renderregistry (APPLICATION_SUB_KEY_NAME_RENDER);
			if (optionsregistry.Is_Valid()) {
				renderregistry.Set_Int ("RenderDeviceWindowed", windowedmode ? 1 : 0);
			}

			// Attempt to initialize the device.
			if (WW3D::Registry_Load_Render_Device (APPLICATION_SUB_KEY_NAME_RENDER, true) == WW3D_ERROR_OK) {

				// Clear frame buffers.
				for (int frame = 0; frame < 3; frame++) {
					WW3D::Begin_Render (true, true, Vector3 (0.0f,0.0f,0.0f));
					WW3D::End_Render();
				}

				// Success! Initialize dialog system.
				DialogMgrClass::Initialize (stylemgrini);
				DialogMgrInitialized = true;
				UIInput = NEW_REF (InstallerUIInputClass, ());
				UIInput->InitIME (MainWindow);
				DialogMgrClass::Install_Input (UIInput);

				return (true);
			}
		}
	}

	// Failure.
	return (false);
}


/***********************************************************************************************
 * InstallerClass::WW3D_Shutdown --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::WW3D_Shutdown()
{
	// Clear frame buffers.
	for (int frame = 0; frame < 3; frame++) {
		WW3D::Begin_Render (true, true, Vector3 (0.0f,0.0f,0.0f));
		WW3D::End_Render();
	}

	// Shutdown WW3D (if previously initialized).
	REF_PTR_RELEASE (UIInput);
	if (DialogMgrInitialized) DialogMgrClass::Shutdown();
	if (WW3D::Is_Initted()) WW3D::Shutdown();
	if (AssetManager != NULL) {
		AssetManager->Free_Assets();
		delete AssetManager;
	}
}


/***********************************************************************************************
 * InstallerClass::Auto_Configure --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Auto_Configure()
{
   PROCESS_INFORMATION processinfo;
	STARTUPINFO			  startupinfo;

	memset (&startupinfo, 0, sizeof (STARTUPINFO));
	startupinfo.cb = sizeof (STARTUPINFO);

   // Start the child process. 
   if (CreateProcess (NULL,								// Address of module name
							 "WWConfig.exe -autoconfig",	// Address of command line
							 NULL,								// Address of process security attributes
							 NULL,								// Address of thread security attributes
							 false,								// Does new process inherit handles?
							 0,									// Creation flags
							 NULL,								// Address of new environment block
							 NULL,								// Address of current directory name
							 &startupinfo,						// Address of STARTUPINFO
							 &processinfo)) {					// Address of PROCESS_INFORMATION

		// Wait until child process exits.
	   WaitForSingleObject (processinfo.hProcess, INFINITE);

		// Close process and thread handles. 
	   CloseHandle (processinfo.hProcess);
	   CloseHandle (processinfo.hThread);
   }
}


/***********************************************************************************************
 * InstallerClass::Install_* --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Install_Game()
{
	WWASSERT (WhatToInstallDialog != NULL);
	return (WhatToInstallDialog->Install_Game());
}
	  
bool InstallerClass::Install_WOL()
{
	WWASSERT (WhatToInstallDialog != NULL);
	return (WhatToInstallDialog->Install_WOL());
}

bool InstallerClass::Install_Game_Shortcut()
{
	WWASSERT (WhatToInstallDialog != NULL);
	return (WhatToInstallDialog->Install_Game() && WhatToInstallDialog->Install_Game_Shortcut());
}


/***********************************************************************************************
 * InstallerClass::Get_Target_* --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
const wchar_t *InstallerClass::Get_Target_Game_Path (WideStringClass &path)
{
	WWASSERT (GameDirectoryDialog != NULL);
	return (GameDirectoryDialog->Get_Path (path));
}

const wchar_t *InstallerClass::Get_Target_WOL_Path (WideStringClass &path)
{
	WWASSERT (WOLDirectoryDialog != NULL);
	return (WOLDirectoryDialog->Get_Path (path));
}

const wchar_t *InstallerClass::Get_Target_Game_Folder (WideStringClass &folder)
{
	WWASSERT (GameFolderDialog != NULL);
	return (GameFolderDialog->Get_Folder (folder));
}

const wchar_t *InstallerClass::Get_Target_WOL_Folder (WideStringClass &folder)
{
	WWASSERT (WOLFolderDialog != NULL);
	return (WOLFolderDialog->Get_Folder (folder));
}


/***********************************************************************************************
 * InstallerClass::Get_Target_Sub_Path --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Get_Target_Sub_Path (unsigned index, WideStringClass &subpath)
{
	StringClass		 multibytesubdirectoryname, entryname;
	WideStringClass subdirectoryname, targetpath;
	char				 numberstring [MAX_NUMBER_STRING_LENGTH];	

	entryname  = INI_SUBDIRECTORY_ENTRY_PREFIX;
	entryname += ltoa (index, numberstring, 10);
	GameIni->Get_String (multibytesubdirectoryname, INI_SUBDIRECTORY_SECTION, entryname);
	subdirectoryname = multibytesubdirectoryname;
	if (subdirectoryname.Get_Length() > 0) {
		subpath  = Get_Target_Game_Path (targetpath);
		subpath += L"\\";
		subpath += subdirectoryname;
		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * InstallerClass::Get_Game_Size --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
int64_t InstallerClass::Get_Game_Size (bool ondisk)
{
	int64_t bytecount = 0;

	if (Install_Game()) {

		bytecount = GameSize;

		if (ondisk) {

			// Account for cluster padding.
			bytecount += Cluster_Padding (GameFileCount);

			// Account for any .key (Encryption) and .wsu (Uninstall log) files that will be created by the Installer.
			bytecount += INSTALLER_NOMINAL_BYTE_COUNT;
		}
	}

	return (bytecount);
}


/***********************************************************************************************
 * InstallerClass::Get_WOL_Size --																				  *		
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
int64_t InstallerClass::Get_WOL_Size (bool ondisk)
{
	int64_t bytecount = 0;

	if (Install_WOL()) {

		bytecount = WOLSize;

		if (ondisk) {
		
			// Account for cluster padding.
			bytecount += Cluster_Padding (WOLFileCount);

			// Account for any .key (Encryption) and .wsu (Uninstall log) files that will be created by the Installer.
			bytecount += INSTALLER_NOMINAL_BYTE_COUNT;
		}
	}

	return (bytecount);
}


/***********************************************************************************************
 * InstallerClass::Get_Total_Size --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
int64_t InstallerClass::Get_Total_Size (bool ondisk)
{
	return (Get_Game_Size (ondisk) + Get_WOL_Size (ondisk));
}


/***********************************************************************************************
 * InstallerClass::Get_WOL_Space_Available --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Get_WOL_Space_Available (const WideStringClass &path, int64_t &diskspace)
{
	WideStringClass targetgamepath, targetwolpath;

	// If the game is to be installed on the same drive, subtract game space needed from disk space available.
	if (Get_Disk_Space_Available (path, diskspace)) {
		if (Install_Game() && (Get_Target_Game_Path (targetgamepath) [0] == Get_Target_WOL_Path (targetwolpath) [0])) {
			diskspace -= Get_Game_Size (true);
			WWASSERT (diskspace >= 0);
		}
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * InstallerClass:Get_Serial_Number: --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
const char *InstallerClass::Get_Serial_Number (StringClass &serialnumber)
{
	bool valid;
	
	WWASSERT (SerialDialog != NULL);
	valid = SerialDialog->Get_Serial_Number (serialnumber);
	WWASSERT (valid);
	return (serialnumber);
}


/***********************************************************************************************
 * InstallerClass::Dialog_Callback --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Dialog_Callback (DialogBaseClass *dialog, int ctrl_id, int message_id, DWORD param)
{
	InstallMenuDialogClass *installmenudialog = (InstallMenuDialogClass*) dialog;
	
	// If no dialog is currently active display the welcome dialog.
	if (installmenudialog == NULL) {

		REGISTER_DIALOG (WelcomeDialogClass, WelcomeDialog, ())

	} else {

 		// Is the current dialog the Welcome dialog?
		if (installmenudialog->As_WelcomeDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					if (!Beta_Test()) {
						REGISTER_DIALOG (LicenseDialogClass, LicenseDialog, ())
					} else {
						REGISTER_DIALOG (NDADialogClass, NDADialog, ())
					}
					return;
			}
		}

		// Is the current dialog the NDA agreement?
		if (installmenudialog->As_NDADialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (LicenseDialogClass, LicenseDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WelcomeDialogClass, WelcomeDialog, ())
					return;
			}
		}

		// Is the current dialog the License dialog?
		if (installmenudialog->As_LicenseDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (SerialDialogClass, SerialDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					if (!Beta_Test()) {
						REGISTER_DIALOG (WelcomeDialogClass, WelcomeDialog, ())
					} else {
						REGISTER_DIALOG (NDADialogClass, NDADialog, ())
					}
					return;
			}
		}

		// Is the current dialog the Serial dialog?
		if (installmenudialog->As_SerialDialogClass()) {

			switch (ctrl_id) {
					
				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WhatToInstallDialogClass, WhatToInstallDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (LicenseDialogClass, LicenseDialog, ())
					return;
			}
		}

		// Is the current dialog the What To Install dialog?
		if (installmenudialog->As_WhatToInstallDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)

					// Does the user want to install the game? If not skip to WOL dialogs.	
					if (Install_Game()) {
						REGISTER_DIALOG (GameDirectoryDialogClass, GameDirectoryDialog, ())
					} else {
						REGISTER_DIALOG (WOLDirectoryDialogClass, WOLDirectoryDialog, ())
					}
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (SerialDialogClass, SerialDialog, ())
					return;
			}
		}

		// Is the current dialog the Game Directory dialog?
		if (installmenudialog->As_GameDirectoryDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (GameFolderDialogClass, GameFolderDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WhatToInstallDialogClass, WhatToInstallDialog, ())
					return;
			}
		}

		// Is the current dialog the Game Folder dialog?
		if (installmenudialog->As_GameFolderDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					
					// Does the user want to install WOL? If not skip to Review dialog.	
					if (Install_WOL()) {
						REGISTER_DIALOG (WOLDirectoryDialogClass, WOLDirectoryDialog, ())
					} else {
						REGISTER_DIALOG (ReviewDialogClass, ReviewDialog, ())
					}
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (GameDirectoryDialogClass, GameDirectoryDialog, ())
					return;
			}
		}

		// Is the current dialog the Online Directory dialog?
		if (installmenudialog->As_WOLDirectoryDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WOLFolderDialogClass, WOLFolderDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					
					// Does the user want to install the game? If not skip to the What To Install dialog.
					if (Install_Game()) {
						REGISTER_DIALOG (GameFolderDialogClass, GameFolderDialog, ())
					} else {
						REGISTER_DIALOG (WhatToInstallDialogClass, WhatToInstallDialog, ())
					}
					return;
			}
		}
			
		// Is the current dialog the Online Folder dialog?
		if (installmenudialog->As_WOLFolderDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (ReviewDialogClass, ReviewDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WOLDirectoryDialogClass, WOLDirectoryDialog, ())
					return;
			}
		}

		// Is the current dialog the Review dialog?
		if (installmenudialog->As_ReviewDialogClass()) {

			switch (ctrl_id) {

	 			case IDCANCEL:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					return;
	
				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (CopyDialogClass, CopyDialog, ())
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)
					
					// Does the user want to install WOL? If not skip to game folder.
					if (Install_WOL()) {
						REGISTER_DIALOG (WOLFolderDialogClass, WOLFolderDialog, ())
					} else {
						REGISTER_DIALOG (GameFolderDialogClass, GameFolderDialog, ())
					}
					return;
			}
		}

		// Is the current dialog the Copy dialog.
		if (installmenudialog->As_CopyDialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
					
					if (((CopyDialogClass*) installmenudialog->As_CopyDialogClass())->Was_Successful()) {
							
						// Copying was successful - advance straight to final dialog.
						UNREGISTER_DIALOG (dialog)
						REGISTER_DIALOG (FinalDialogClass, FinalDialog, (false))

					} else {

						// Copying was not successful - cancel as normal.
						DO_CANCEL
						UNREGISTER_DIALOG (dialog)
					}
					return;

				case IDOK:
					UNREGISTER_DIALOG (dialog)
					
					// Does the user have existing Westwood Online accounts?
					if (Has_WOL_Account()) {
						REGISTER_DIALOG (WOL1DialogClass, WOL1Dialog, ())
					} else {
						REGISTER_DIALOG (FinalDialogClass, FinalDialog, (false))
					}
					return;
			}
		}	

		// Is the current dialog the WOL1 dialog?
		if (installmenudialog->As_WOL1DialogClass()) {

			switch (ctrl_id) {

				case IDC_BUTTON_YES:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (WOL2DialogClass, WOL2Dialog, ())
					return;

				case IDC_BUTTON_NO:
				case IDCANCEL:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (FinalDialogClass, FinalDialog, (true))
					return;
			}
		}

		// Is the current dialog the WOL2 dialog?
		if (installmenudialog->As_WOL2DialogClass()) {

			switch (ctrl_id) {

				case IDCANCEL:
				case IDOK:
					UNREGISTER_DIALOG (dialog)
					REGISTER_DIALOG (FinalDialogClass, FinalDialog, (true))
					return;
			}
		}

		// Is the current dialog the Final dialog?
		if (installmenudialog->As_FinalDialogClass()) {

			switch (ctrl_id) {

				case IDOK:
					DO_CANCEL
					UNREGISTER_DIALOG (dialog)
					
					// Extract any information from dialogs that will be needed on program end.
					if (Install_Game()) {
						Get_Target_Game_Path (TargetGamePath);
						if (FinalDialog->Run_Game()) {
							ProgEndAction = ACTION_RUN_GAME;
						} else {
							if (FinalDialog->Display_Readme()) {
								ProgEndAction = ACTION_DISPLAY_README;
							}
						}
					}
					return;

				case IDC_BUTTON_BACK:
					UNREGISTER_DIALOG (dialog)

					// If WOL2Dialog exists return to it - otherwise return to WOL1Dialog.
					if (WOL2Dialog != NULL) {
						REGISTER_DIALOG (WOL2DialogClass, WOL2Dialog, ())
					} else {
						WWASSERT (WOL1Dialog != NULL);
						REGISTER_DIALOG (WOL1DialogClass, WOL1Dialog, ())
					}
					return;
			}
		}
	}
}


/***********************************************************************************************
 * Default_On_Command -- Callback for all menu dialogs.													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool Default_On_Command (DialogBaseClass *dialog, int ctrl_id, int message_id, DWORD param)
{
	_Installer.Dialog_Callback (dialog, ctrl_id, message_id, param);
	return (true);
}


/***********************************************************************************************
 * WWDebug_Message_Callback -- WWDebug message callback to define the behavior of WWDEBUG_SAY()*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void WWDebug_Message_Callback (DebugType type, const char *message)
{
	OutputDebugString (message);
}


/***********************************************************************************************
 * WWAssert_Callback -- WWAssert callback to define the behavior of WWASSERT().					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void WWAssert_Callback (const char *message)
{
	OutputDebugString (message);
	_asm int 0x03
}


/***********************************************************************************************
 * InstallerClass::Can_Use_IGR_Settings --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Can_Use_IGR_Settings()
{
	return (SetupIni->Get_Hex (INI_SETUP_SECTION, INI_CAN_USE_IGR_SETTNGS_ENTRY, 0x0) > 0x0);
}


/***********************************************************************************************
 * InstallerClass::Use_IGR_Settings --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Use_IGR_Settings()
{
	WWASSERT (WhatToInstallDialog != NULL);
	return (WhatToInstallDialog->Use_IGR_Settings());
}


/***********************************************************************************************
 * InstallerClass::Beta_Test --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Beta_Test()
{
	const char *ndafilename = "NDA.txt";

	HANDLE			 handle;
	WIN32_FIND_DATA finddata;	
	
	// Use existence of Beta NDA to indicate whether this is the Beta test version.
	handle = FindFirstFile (ndafilename, &finddata);
	if (handle != INVALID_HANDLE_VALUE) {
		FindClose (handle);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * InstallerClass::Update_Registry --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Update_Registry()
{
	if (Install_Game()) {

		WideStringClass installpathname, folderpath, shortcutpathname;
		StringClass		 serialnumber, encryptedserialnumber;
		DWORD				 languageid, sku, version;
		WideStringClass pathname, gamefolder, gamepath;
		DWORD				 major, minor;

		Get_Target_Game_Path (installpathname);
		installpathname += L"\\";
		installpathname += RxWideStringClass (IDS_GAME_EXECUTABLE_FILENAME);

		_RegistryManager.Get_Folder_Path (folderpath);
		folderpath += L"\\";
		folderpath += Get_Target_Game_Folder (gamefolder);
		folderpath += L"\\";
		folderpath += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);

		_RegistryManager.Get_Desktop_Path (shortcutpathname);
		shortcutpathname += L"\\";
		shortcutpathname += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);
		shortcutpathname += L".lnk";

		languageid = SetupIni->Get_Hex (INI_SETUP_SECTION, INI_LANGUAGE_ENTRY, 0x0);

		// Create encryption file in target directory.
		Get_Target_Game_Path (pathname);
		pathname += L"\\";
		pathname += RxWideStringClass (IDS_SERIAL_ENCRYPTION_FILENAME);
		Create_Encryption_File (pathname);

		// Encrypt the serial no.
		Encrypt (Get_Serial_Number (serialnumber), pathname, encryptedserialnumber);

		sku = SetupIni->Get_Int (INI_SETUP_SECTION, INI_SKU_ENTRY, 0x0);

		major	  = SetupIni->Get_Int (INI_SETUP_SECTION, INI_VERSION_MAJOR_ENTRY, 0x1);
		minor	  = SetupIni->Get_Int (INI_SETUP_SECTION, INI_VERSION_MINOR_ENTRY, 0x0);
		version = (major << 16) + minor;

		_RegistryManager.Register_Game (TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME),
												  installpathname,
												  folderpath,
												  shortcutpathname,
												  languageid,
												  encryptedserialnumber,
												  sku,
												  version);
	}

	if (Install_WOL()) {
	
		WideStringClass folderpath, wolapipathname, wolregisterpathname, wolbrowserpathname;
		DWORD				 sku, version;
		WideStringClass WOLfolder;
		DWORD				 major, minor;

		_RegistryManager.Get_Folder_Path (folderpath);
		folderpath += L"\\";
		folderpath += Get_Target_WOL_Folder (WOLfolder);
		folderpath += L"\\";
		folderpath += TxWideStringClass (IDS_WOL_PRODUCT_NAME, IDS_RESOURCE_WOL_PRODUCT_NAME);

		Get_Target_WOL_Path (wolapipathname);
		wolapipathname += L"\\";
		wolapipathname += RxWideStringClass (IDS_WOLAPI_DLL_FILENAME);

		sku = atol (RxStringClass (IDS_WOLAPI_SKU));

		major	  = WOLIni->Get_Int (INI_WOLAPI_SECTION, INI_VERSION_MAJOR_ENTRY, 0x1);
		minor	  = WOLIni->Get_Int (INI_WOLAPI_SECTION, INI_VERSION_MINOR_ENTRY, 0x0);
		version = (major << 16) + minor;

		_RegistryManager.Register_WOLAPI (folderpath,
													 wolapipathname,
													 TxWideStringClass (IDS_WOL_PRODUCT_NAME, IDS_RESOURCE_WOL_PRODUCT_NAME),
													 sku,
													 version,
													 Use_IGR_Settings());

		Get_Target_WOL_Path (wolregisterpathname);
		wolregisterpathname += L"\\";
		wolregisterpathname += RxWideStringClass (IDS_INTERNET_REGISTRATION_FILENAME);

		sku = atol (RxStringClass (IDS_WOLREGISTER_SKU));

		major	  = WOLIni->Get_Int (INI_WOLREGISTER_SECTION, INI_VERSION_MAJOR_ENTRY, 0x1);
		minor	  = WOLIni->Get_Int (INI_WOLREGISTER_SECTION, INI_VERSION_MINOR_ENTRY, 0x0);
		version = (major << 16) + minor;

		_RegistryManager.Register_WOLRegister (folderpath,
															wolregisterpathname,
															TxWideStringClass (IDS_WOLREGISTER_NAME, IDS_RESOURCE_WOLREGISTER_NAME),
															sku,
															version);

		Get_Target_WOL_Path (wolbrowserpathname);
		wolbrowserpathname += "\\";
		wolbrowserpathname += RxWideStringClass (IDS_WOLBROWSER_DLL_FILENAME);

		major	  = WOLIni->Get_Int (INI_WOLBROWSER_SECTION, INI_VERSION_MAJOR_ENTRY, 0x1);
		minor	  = WOLIni->Get_Int (INI_WOLBROWSER_SECTION, INI_VERSION_MINOR_ENTRY, 0x0);
		version = (major << 16) + minor;

		_RegistryManager.Register_WOLBrowser (wolbrowserpathname,
														  TxWideStringClass (IDS_WOLBROWSER_NAME, IDS_RESOURCE_WOLBROWSER_NAME),
														  version);

		// Now register the DLL's.
		Register_COM_Server (wolapipathname);
		Register_COM_Server (wolbrowserpathname);
	}

	// Update WOLAPI.
	_RegistryManager.Update_WOLAPI();
}


/***********************************************************************************************
 * InstallerClass::Create_Links --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Create_Links()
{
	if (CoInitialize (NULL) != S_OK) FATAL_APP_ERROR (IDS_COM_ERROR);

	if (Install_Game()) {

		WideStringClass folderpath, gamefolder, gamepathname, readmepathname;
		WideStringClass readmeiconpathname, uninstallpathname, internetpathname, wwconfigpathname;
		WideStringClass grabpatches ("GrabPatches");

		// Create subdirectory in folder path for menu item links.
		_RegistryManager.Get_Folder_Path (folderpath);
		folderpath += L"\\";
		folderpath += Get_Target_Game_Folder (gamefolder);
		folderpath += L"\\";
		folderpath += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);
		Create_Directory (folderpath);

		// Create Renegade menu item.
		Get_Target_Game_Path (gamepathname);
		gamepathname += L"\\";
		gamepathname += RxWideStringClass (IDS_GAME_EXECUTABLE_FILENAME);
	  	Create_File_Link (folderpath, TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME), gamepathname, gamepathname);

		// Create Auto Update menu item.
		Create_File_Link (folderpath, TxWideStringClass (IDS_AUTO_UPDATE_TITLE, IDS_RESOURCE_AUTO_UPDATE_TITLE), gamepathname, gamepathname, &grabpatches);

		// Create Readme menu item.
		Get_Target_Game_Path (readmepathname);
		readmepathname += L"\\";
		readmepathname += RxWideStringClass (IDS_WORD_README_FILENAME);
		Get_Target_Game_Path (readmeiconpathname);
		readmeiconpathname += L"\\";
		readmeiconpathname += RxWideStringClass (IDS_README_ICON_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_README_TITLE, IDS_RESOURCE_README_TITLE), readmepathname, readmeiconpathname);

		// Create Uninstall menu item.
		Get_Target_Game_Path (uninstallpathname);
		uninstallpathname += L"\\";
		uninstallpathname += RxWideStringClass (IDS_GAME_UNINSTALL_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_GAME_UNINSTALL_TITLE, IDS_RESOURCE_GAME_UNINSTALL_TITLE), uninstallpathname, uninstallpathname); 

		// Create Internet Registration menu item.
		Get_Target_Game_Path (internetpathname);
		internetpathname += L"\\";
		internetpathname += RxWideStringClass (IDS_INTERNET_REGISTRATION_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_INTERNET_REGISTRATION_TITLE, IDS_RESOURCE_INTERNET_REGISTRATION_TITLE), internetpathname, internetpathname);

		// Create GameSpy Arcade menu item.
		// LOCALIZATION NOTE: The French have decided not to include this link.
		if (SetupIni->Get_Hex (INI_SETUP_SECTION, INI_LANGUAGE_ENTRY, 0x0) != 0x3) {
			Create_URL_Link (folderpath, TxWideStringClass (IDS_PLAY_ONLINE_WITH_GAMESPY, IDS_RESOURCE_PLAY_ONLINE_WITH_GAMESPY), L"http://www.gamespyarcade.com/features/launch.asp?svcname=ccrenegade&distID=391");
		}

		// Create link to WWConfig.
		Get_Target_Game_Path (wwconfigpathname);
		wwconfigpathname += L"\\";
		wwconfigpathname += RxWideStringClass (IDS_WWCONFIG_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_WWCONFIG_TITLE, IDS_RESOURCE_WWCONFIG_TITLE), wwconfigpathname, wwconfigpathname);

		// Create desktop icon (if requested).
		if (Install_Game_Shortcut()) {
		
			WideStringClass shortcutpath;

			_RegistryManager.Get_Desktop_Path (shortcutpath);
			Create_File_Link (shortcutpath, TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME), gamepathname, gamepathname);
		}
	}

	if (Install_WOL()) {

		WideStringClass folderpath, wolfolder, registerpathname, registericonpathname, uninstallpathname;

		// Create subdirectory in folder path for menu item links.
		_RegistryManager.Get_Folder_Path (folderpath);
		folderpath += L"\\";
		folderpath += Get_Target_WOL_Folder (wolfolder);
		folderpath += L"\\";
		folderpath += TxWideStringClass (IDS_WOL_PRODUCT_NAME, IDS_RESOURCE_WOL_PRODUCT_NAME);
		Create_Directory (folderpath);

		// Create Internet Registration menu item.
		Get_Target_WOL_Path (registerpathname);
		registerpathname += L"\\";
		registerpathname += RxWideStringClass (IDS_INTERNET_REGISTRATION_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_INTERNET_REGISTRATION_TITLE, IDS_RESOURCE_INTERNET_REGISTRATION_TITLE), registerpathname, registerpathname);
		
		// Create Uninstall Registration menu item.
		Get_Target_WOL_Path (uninstallpathname);
		uninstallpathname += L"\\";
		uninstallpathname += RxWideStringClass (IDS_WOL_UNINSTALL_FILENAME);
		Create_File_Link (folderpath, TxWideStringClass (IDS_WOL_UNINSTALL_TITLE, IDS_RESOURCE_WOL_UNINSTALL_TITLE), uninstallpathname, uninstallpathname);
	}

	CoUninitialize();
}


/***********************************************************************************************
 * InstallerClass::Create_File_Link --																			  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Create_File_Link (const WideStringClass &linkpath, const WideStringClass &title, const WideStringClass &targetpathname, const WideStringClass &iconpathname, const WideStringClass *arguments)
{
	WideStringClass  linkpathname, trimmedtitle, targetpath;
	StringClass		  multibytelinkpathname, multibytetargetpath, multibytetitle, multibytetargetpathname, multibyteiconpathname;	
	IShellLink		 *psl;
	IPersistFile	 *ppf;
	bool				  success;	

	// The title is used in a pathname so it should be freed of any leading or trailing whitespace.
	trimmedtitle = title;
	trimmedtitle.Trim();

	linkpathname  = linkpath;
	linkpathname += L"\\";
	linkpathname += trimmedtitle;
	linkpathname += L".lnk";

	targetpath = targetpathname;
	Remove_Trailing_Name (targetpath);

	multibytelinkpathname	= linkpathname;
	multibytetargetpath		= targetpath;
	multibytetitle				= trimmedtitle;
	multibytetargetpathname = targetpathname;
	multibyteiconpathname	= iconpathname;

	// Remove previous icon (if it exists).  
	DeleteFile (multibytelinkpathname);

	success = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**) &psl) == S_OK;
	if (success) {

		try {

			if (psl->SetPath (multibytetargetpathname) != NOERROR) throw (IDS_COM_ERROR);
			if (psl->SetDescription (multibytetitle) != NOERROR) throw (IDS_COM_ERROR);
			if (psl->SetIconLocation (multibyteiconpathname, 0) != NOERROR) throw (IDS_COM_ERROR);
			if (arguments != NULL) {
		
				StringClass multibytearguments (*arguments);
		
				if (psl->SetArguments (multibytearguments) != NOERROR) throw (IDS_COM_ERROR);
			}

			if (psl->SetWorkingDirectory (multibytetargetpath) != NOERROR) throw (IDS_COM_ERROR);
			if (psl->SetShowCmd (SW_NORMAL) != NOERROR) throw (IDS_COM_ERROR);
			if (psl->SetHotkey (0) != NOERROR) throw (IDS_COM_ERROR);
			if (psl->QueryInterface (IID_IPersistFile, (void**) &ppf) == S_OK) {
				success = ppf->Save (linkpathname, true) == S_OK;
				ppf->Release();
			}

		} catch (...) {
			success = false;
		}
		psl->Release();
	}

	return (success);
}


/***********************************************************************************************
 * InstallerClass::Create_URL_Link --																			  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Create_URL_Link (const WideStringClass &linkpath, const WideStringClass &title, const WideStringClass &url)
{
	WideStringClass			 trimmedtitle, linkpathname;	
	StringClass					 multibytelinkpathname, multibyteurl;
	IUniformResourceLocator *purl;
	IPersistFile				*ppf;
  	bool							 success;	

	// The title is used in a pathname so it should be freed of any leading or trailing whitespace.
	trimmedtitle = title;
	trimmedtitle.Trim();

	linkpathname  = linkpath;
	linkpathname += L"\\";
	linkpathname += trimmedtitle;
	linkpathname += L".url";

	multibytelinkpathname = linkpathname;
	multibyteurl			 = url;

	// Remove previous icon (if it exists).  
	DeleteFile (multibytelinkpathname);

	// Create an IUniformResourceLocator object
	success = CoCreateInstance (CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**) &purl) == S_OK;
	if (success) {
		success = purl->SetURL (multibyteurl, 0) == S_OK;
		if (success) {
			success = purl->QueryInterface (IID_IPersistFile, (void**) &ppf) == S_OK;
			if (success) {
				success = ppf->Save (linkpathname, true) == S_OK;
				ppf->Release();
			}
		}
		purl->Release();
	}
	
	return (success);
}


/***********************************************************************************************
 * InstallerClass::Has_WOL_Account --																				  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Has_WOL_Account()
{
	WideStringClass name, password;

	return (_RegistryManager.Get_WOL_Account (0, name, password));
}


/***********************************************************************************************
 * InstallerClass::Get_WOL_Account --																				  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Get_WOL_Account (unsigned index, WideStringClass &name, WideStringClass &password)
{
	return (_RegistryManager.Get_WOL_Account (index, name, password));
}


/***********************************************************************************************
 * InstallerClass::Log --																							  *		
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Log (const WideStringClass &pathname, int64_t size)
{
	WideStringClass path;

	if (Install_Game()) {
	
		// Does the start of the pathname match either the game or WOL target path?
		if (wcsstr (pathname, Get_Target_Game_Path (path)) != NULL) {
		
			if (size == -1) {

				WideStringClass subdirectoryname (((WideStringClass) pathname).Peek_Buffer() + path.Get_Length() + 1);

				GameSubdirectories.Add (subdirectoryname);

			} else {

				FileLogClass filelog;

				filelog.RelativePathname = ((WideStringClass) pathname).Peek_Buffer() + path.Get_Length() + 1;
				filelog.Size = size;	
				GameFiles.Add (filelog);
			}

			return;
		}
	}
	
	if (Install_WOL()) {

		if (wcsstr (pathname, Get_Target_WOL_Path (path)) != NULL) {

			FileLogClass filelog;

			// NOTE: Logging WOL subdirectories is not supported.
			WWASSERT (size != -1);

			filelog.RelativePathname = ((WideStringClass) pathname).Peek_Buffer() + path.Get_Length() + 1;
			filelog.Size = size;	
			WOLFiles.Add (filelog);
		}
	}
}


/***********************************************************************************************
 * InstallerClass::Create_Uninstall_Logs --																	  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Create_Uninstall_Logs()
{
	if (Install_Game()) {
		Create_Game_Uninstall_Log();
	}
	if (Install_WOL()) {
		Create_WOL_Uninstall_Log();
	}
}


/***********************************************************************************************
 * InstallerClass::Create_Game_Uninstall_Log --																  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Create_Game_Uninstall_Log()
{
	WideStringClass log;
	WideStringClass installpathname, folderpath, gamefolder, shortcutpath;
	WideStringClass s;
	WideStringClass logfilepathname;
	WideStringClass gamefilepathname;
	HANDLE			 handle;	
	int				 i;
	DWORD				 byteswritten;
	StringClass		 multibytelogfilepathname, multibytelog;
	int64_t			 totalfilesize;
	
	_RegistryManager.Get_Folder_Path (folderpath);
	_RegistryManager.Get_Desktop_Path (shortcutpath);

	// Log temporary files that will/may be created by the game.
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"commands.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"_logfile.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"_asserts.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"_except.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"sysinfo.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"history.txt";
	Log (gamefilepathname, 0);
	Get_Target_Game_Path (gamefilepathname);
	gamefilepathname += L"\\";
	gamefilepathname += L"asset_report.txt";
	Log (gamefilepathname, 0);

	// Create log pathname and log this file so it gets written to the uninstall log.
	// NOTE: File size is not important - substitute zero.
	Get_Target_Game_Path (logfilepathname);
	logfilepathname += L"\\";
	logfilepathname += RxWideStringClass (IDS_GAME_UNINSTALL_LOG_FILENAME);
	Log (logfilepathname, 0);

	// Build the log.
	log += L"[Product]\n";
	log += L"Name=";
	log += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);
	log += L"\n";
	log += L"FullName=";
	log += TxWideStringClass (IDS_VERBOSE_GAME_PRODUCT_NAME, IDS_RESOURCE_VERBOSE_GAME_PRODUCT_NAME);
	log += L"\n";
	log += L"Key=";
	log += RxWideStringClass (IDS_GAME_PRODUCT_KEY);
	log += L"\n";
	log += L"Exe=";
	log += Get_Target_Game_Path (installpathname);
	log += L"\\";
	log += RxWideStringClass (IDS_GAME_EXECUTABLE_FILENAME);
	log += L"\n\n";
	log += L"[Icons]\n";
	log += L"LNK=";
	log += folderpath;
	log += L"\\";
	log += Get_Target_Game_Folder (gamefolder);
	log += L"\\";
	log += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);
	log += L"\n";
	log += L"Desktop=";
	log += shortcutpath;
	log += L"\\";
	log += TxWideStringClass (IDS_GAME_PRODUCT_NAME, IDS_RESOURCE_GAME_PRODUCT_NAME);
	log += L".lnk";
	log += L"\n\n";

	// Append subdirectory information.
	log += L"[SubDirectories]\n";
	for (i = 0; i < GameSubdirectories.Count(); i++) {
		s.Format (L"Dir%d=%s\n", i, GameSubdirectories [i]);
		log += s;
	}
	s.Format (L"NumFiles=%u\n\n", GameSubdirectories.Count());
	log += s;

	// Append relative pathname information.
	log += L"[Files]\n";
	for (i = 0; i < GameFiles.Count(); i++) {
		s.Format (L"F:%d=%s\n", i, GameFiles [i].RelativePathname);
		log += s;
	}
	s.Format (L"NumFiles=%u\n\n", GameFiles.Count());
	log += s;

	// Append relative file size information.
	log += L"[FileSizes]\n";
	totalfilesize = 0;
	for (i = 0; i < GameFiles.Count(); i++) {
		s.Format (L"S:%d=%I64d\n", i, GameFiles [i].Size);
		log += s;
		totalfilesize += GameFiles [i].Size;
	}
	s.Format (L"Total=%I64d\n", totalfilesize);
	log += s;

	// Write the log file.
	// NOTE: Convert strings from wide to multibyte.
	multibytelogfilepathname = logfilepathname;
	handle = CreateFile (multibytelogfilepathname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;
	multibytelog = log;
	if (!WriteFile (handle, multibytelog.Peek_Buffer(), multibytelog.Get_Length() * sizeof (TCHAR), &byteswritten, NULL)) FATAL_SYSTEM_ERROR;
	if (!CloseHandle (handle)) FATAL_SYSTEM_ERROR;
}


/***********************************************************************************************
 * InstallerClass::Create_WOL_Uninstall_Log --																  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Create_WOL_Uninstall_Log()
{
	WideStringClass log;
	WideStringClass s;
	WideStringClass logfilepathname;
	HANDLE			 handle;	
	int				 i;
	DWORD				 byteswritten;
	StringClass		 multibytelogfilepathname, multibytelog;

	// Create log pathname and log this file so it gets written to the uninstall log.
	// NOTE: File size is not important - substitute zero.
	Get_Target_WOL_Path (logfilepathname);
	logfilepathname += L"\\";
	logfilepathname += RxWideStringClass (IDS_WOL_UNINSTALL_LOG_FILENAME);
	Log (logfilepathname, 0);

	// Append relative pathname information.
	for (i = 0; i < WOLFiles.Count(); i++) {
		s.Format (L"%s\n", WOLFiles [i].RelativePathname);
		log += s;
	}

	// Write the log file.
	multibytelogfilepathname = logfilepathname;
	handle = CreateFile (multibytelogfilepathname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;
	multibytelog = log;
	if (!WriteFile (handle, multibytelog.Peek_Buffer(), multibytelog.Get_Length() * sizeof (TCHAR), &byteswritten, NULL)) FATAL_SYSTEM_ERROR;
	if (!CloseHandle (handle)) FATAL_SYSTEM_ERROR;
}


/***********************************************************************************************
 * InstallerClass::Create_Encryption_File --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Create_Encryption_File (const WideStringClass &pathname)
{
	StringClass multibytepathname (pathname); 
	char			stringbuffer [ENCRYPTION_STRING_LENGTH];
   HANDLE		handle;
	DWORD			byteswritten;

	handle = CreateFile (multibytepathname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;

	// Seed the random no. generator.
	srand (timeGetTime());

	for (unsigned i = 0; i < ENCRYPTION_STRING_LENGTH; i++) {

		char digit;
		
		digit	= rand() % 10;
		stringbuffer [i] = digit + '0';
	}

	if (!WriteFile (handle, stringbuffer, sizeof (stringbuffer), &byteswritten, NULL)) FATAL_SYSTEM_ERROR;
	if (!CloseHandle (handle)) FATAL_SYSTEM_ERROR;

	// Log the file so it gets written to the uninstall log.
	// NOTE: File size is not important - substitute zero.
	Log (pathname, 0);
}


/***********************************************************************************************
 * InstallerClass::Encrypt --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Encrypt (const char *number, const WideStringClass &pathname, StringClass &encryptednumber)
{
	StringClass  multibytepathname (pathname);
	char			*s;		
	int			 numberlength;
	HANDLE		 handle;	
	DWORD			 bytesread;
	char			 stringbuffer [ENCRYPTION_STRING_LENGTH];
	int			 p;
	
	numberlength = strlen (number);
	s = new char [numberlength + 1];
	memcpy (s, number, numberlength + 1);

	handle = CreateFile (multibytepathname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) FATAL_SYSTEM_ERROR;
	if (!ReadFile (handle, stringbuffer, sizeof (stringbuffer), &bytesread, NULL)) FATAL_SYSTEM_ERROR;
	
	p = 0;
	for (unsigned i = 0; i < ENCRYPTION_STRING_LENGTH; i++) {

		int  t;
		char c;

		t  = s [p] - '0';
		t %= 10;
		t += stringbuffer [i];
		t %= 10;
		c  = t + '0';
		s [p] = c;
		p++;
		if (p == numberlength) p = 0;
	}

	encryptednumber = s;

	// Clean-up.
	delete [] s;
}


/***********************************************************************************************
 * InstallerClass::Register_COM_Server --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
bool InstallerClass::Register_COM_Server (const WideStringClass &comdll)
{
	StringClass multibytecomdll (comdll);
	
	HMODULE module = LoadLibrary (multibytecomdll);

	if (module) {

		HRESULT hr = E_FAIL;

		DLLREGISTERSERVER dllRegister = (DLLREGISTERSERVER) GetProcAddress (module, "DllRegisterServer");

		if (dllRegister) {
			hr = dllRegister();
		}
		FreeLibrary (module);
		module = NULL;
		return (SUCCEEDED (hr));
	}
 
	return (false);
}


/***********************************************************************************************
 * InstallerClass::Finish --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::On_Prog_End()
{
	switch (ProgEndAction) {
	
		case ACTION_NONE:
			break;

		case ACTION_RUN_GAME:
			Run_Game();
			break;

		case ACTION_DISPLAY_README:
			Display_Readme();
			break;
	}
}


/***********************************************************************************************
 * InstallerClass::Run_Game --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Run_Game()
{
	WideStringClass	  modulepathname;
	StringClass			  multibytemodulepathname, multibytetargetgamepath;
	PROCESS_INFORMATION processinfo; 
	STARTUPINFO			  startupinfo;

	modulepathname  = TargetGamePath;
	modulepathname +=	L"\\";
	modulepathname += RxWideStringClass (IDS_GAME_EXECUTABLE_FILENAME);

	multibytemodulepathname = modulepathname;
	multibytetargetgamepath	= TargetGamePath;

	memset (&startupinfo, 0, sizeof (STARTUPINFO));
	startupinfo.cb = sizeof (STARTUPINFO);
	
	if (!CreateProcess (multibytemodulepathname.Peek_Buffer(),					// Address of module name
							  NULL,																// Address of command line
							  NULL,																// Address of process security attributes
							  NULL,																// Address of thread security attributes
							  false,																// Does new process inherit handles?
							  CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,	// Creation flags
							  NULL,																// Address of new environment block
							  multibytetargetgamepath.Peek_Buffer(),					// Address of current directory name
							  &startupinfo,													// Address of STARTUPINFO
							  &processinfo))	{												// Address of PROCESS_INFORMATION

		FATAL_SYSTEM_ERROR;
	}
}


/***********************************************************************************************
 * InstallerClass::Display_Readme --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void InstallerClass::Display_Readme()
{
	const unsigned  applicationcount								= 3;
	const wchar_t		*winwordprogramname							= L"WINWORD.EXE";
	const wchar_t		*wordprocessornames [applicationcount] = {L"Wordpad.exe", L"Notepad.exe", L"Write.exe"};

	WideStringClass		pathname;
	wchar_t						docapppathname [_MAX_PATH];
	wchar_t					  *t;
	unsigned				   i;	
	PROCESS_INFORMATION  processinfo; 
	STARTUPINFO			   startupinfo;
	WideStringClass	   commandline;
	StringClass				multibytecommandline;
	BOOL					   success = false;	

	// See if Winword is the owner of .doc files.  
	_RegistryManager.Get_Document_Application_Pathname (pathname);
	wcscpy (docapppathname, pathname);
	_wcsupr (docapppathname);
	t = wcsstr (docapppathname, winwordprogramname);
	if (t != NULL) {
		 
		t += wcslen (winwordprogramname);
		*t = L'\0';

		// Launch Winword with the Readme.doc file.
		commandline  = docapppathname;
		commandline +=	L" \"";
		commandline += TargetGamePath;
		commandline += L"\\";
		commandline += RxWideStringClass (IDS_WORD_README_FILENAME);
		commandline +=	L"\"";

		memset (&startupinfo, 0, sizeof (STARTUPINFO));
		startupinfo.cb = sizeof (STARTUPINFO);
		multibytecommandline = commandline;
		success = CreateProcess (NULL,															// Address of module name                
										 multibytecommandline.Peek_Buffer(), 					// Address of command line               
										 NULL,															// Address of process security attributes
										 NULL,															// Address of thread security attributes
										 false,															// Does new process inherit handles?
										 CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS, // Creation flags
										 NULL,															// Address of new environment block
										 NULL,															// Address of current directory name
										 &startupinfo,													// Address of STARTUPINFO
										 &processinfo);												// Address of PROCESS_INFORMATION
	}


	// No success? Try to launch any of the common text processor applications with the Readme.txt file.
	i = 0;
	while ((!success) && (i < applicationcount)) {

		commandline  = wordprocessornames [i];
		commandline +=	L" \"";
		commandline += TargetGamePath;
		commandline += L"\\";
		commandline += RxWideStringClass (IDS_TEXT_README_FILENAME);
		commandline +=	L"\"";

		memset (&startupinfo, 0, sizeof (STARTUPINFO));
		startupinfo.cb = sizeof (STARTUPINFO);
		multibytecommandline = commandline;
		success = CreateProcess (NULL,															// Address of module name
										 multibytecommandline.Peek_Buffer(),					// Address of command line
										 NULL,															// Address of process security attributes
										 NULL,															// Address of thread security attributes
										 false,															// Does new process inherit handles?
										 CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS, // Creation flags
										 NULL,															// Address of new environment block
										 NULL,															// Address of current directory name
										 &startupinfo,													// Address of STARTUPINFO
										 &processinfo);												// Address of PROCESS_INFORMATION

		i++;
	}

	// If still no success, throw error.
	if (!success) FATAL_SYSTEM_ERROR;
}


#undef END_DIALOG
#undef REGISTER_DIALOG
#undef UNREGISTER_DIALOG
#undef DO_CANCEL
