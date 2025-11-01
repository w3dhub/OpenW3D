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
 *                     $Archive:: /Commando/Code/Installer/WinMain.cpp  $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/22/02 2:17p                 $* 
 *                                                                                             * 
 *                    $Revision:: 12                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// Include files.
#include "Argv.h"
#include "Bufffile.h"
#include "Chunkio.h"
#include "ErrorHandler.h"
#include "FFactory.h"
#include "Installer.h"
#include "MixFile.h"
#include "Msgloop.h"
#include "RAMFileFactory.h"
#include "resource.h"
#include "SafeTimer.h"
#include "SaveLoad.h"
#include "Timer.h"
#include "Translator.h"
#include "Win.h"
#include "WW3D.h"
#include "WWFile.h"
#include <malloc.h>
#include <dbt.h>


// Defines.
#define AUTORUN_MUTEX_OBJECT			TEXT("01AF9993-3492-11d3-8F6F-0060089C05B1")
#define APPLICATION_MUTEX_OBJECT		TEXT("C6D925A3-7A9B-4ca3-866D-8B4D506C3665")


// Static variables.


// Foward declarations.
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
ATOM				  MyRegisterClass (HINSTANCE hInstance);
bool				  Is_Autorun_Running();
bool				  Is_Application_Running();
bool				  Is_Win_95_Or_Above();
bool				  Is_Win_2K_Or_Above();
bool				  Running_As_Administrator();
void				  Prog_End();	


/***********************************************************************************************
 * WinMain -- Entry point to program.																			  *
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
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int result;

	// Initialize command line parser.
	ArgvClass::Init (lpCmdLine);
	TranslateDBClass::Initialize();

	try {

		// Set global copy of program instance.
		ProgramInstance = hInstance;

		// Ensure Autorun or another instance of this application is not running.
		if ((!Is_Autorun_Running()) && (!Is_Application_Running())) {

			// WARNING: Can only raise fatal system errors (not fatal application errors) until
			//				the translation table has been loaded.
			const char *installmixname			  = "InstallMix.dat";
			const char *installstringsfilename = "InstallStrings.tdb";

			RAMFileFactoryClass  ramfilefactory;
			MixFileFactoryClass  mixfilefactory (installmixname, &ramfilefactory);
			FileClass			  *file;

			// Perform application initialization.

			// An invalid mixfile factory indicates a read error.
			if (!mixfilefactory.Is_Valid()) FATAL_SYSTEM_ERROR;

			_TheFileFactory = &mixfilefactory;

			//	Load the translation table.
			file = _TheFileFactory->Get_File (installstringsfilename);
			if (file == NULL) {
				
				// Output an appropriate Windows error message.
				SetLastError (2);
				FATAL_SYSTEM_ERROR;
			}
			file->Open (FileClass::READ);
			if (file->Is_Available()) {

				ChunkLoadClass cload (file);
				
				SaveLoadSystemClass::Load (cload);
	
			} else {

				// Output an appropriate Windows error message.
				SetLastError (2);
				FATAL_SYSTEM_ERROR;
			}
			file->Close();
			_TheFileFactory->Return_File (file);

			// Check for valid OS.
			if (!(Is_Win_95_Or_Above() || Is_Win_2K_Or_Above())) FATAL_APP_ERROR (IDS_BAD_OS);

			// Check for Administrator rights under Win 2k or above.
			if (Is_Win_2K_Or_Above()) {
				if (!Running_As_Administrator()) FATAL_APP_ERROR (IDS_NOT_ADMINISTRATOR);
			}

			// Register function to be called at exit. 
			atexit (Prog_End);

			MyRegisterClass (hInstance);

			MainWindow = CreateWindow (RxStringClass (IDS_APPLICATION_MAIN_WINDOW),
												StringClass (TxWideStringClass (IDS_APPLICATION_NAME)),
												WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|WS_CLIPCHILDREN,
												0, 0, 0, 0,
												NULL, NULL,
												hInstance, NULL);

			if (MainWindow == NULL) FATAL_SYSTEM_ERROR;

			ShowCursor (false);
			_Installer.Install (&mixfilefactory);
			ShowCursor (true);
		}
		result = 1;

	} catch (const WideStringClass &errormessage) {

		// Catch handler for fatal errors.
		DestroyWindow (MainWindow);
		Windows_Message_Handler();
		Message_Box (TxWideStringClass (IDS_APPLICATION_ERROR), errormessage);
		result = 0;
	}

	TranslateDBClass::Shutdown();
	ArgvClass::Free();
	return (result);
}


/***********************************************************************************************
 * MyRegisterClass --																								  *
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
ATOM MyRegisterClass (HINSTANCE hInstance)
{
	static RxStringClass _classname (IDS_APPLICATION_MAIN_WINDOW);

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof (WNDCLASSEX); 

	wcex.style			 = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	 = (WNDPROC)WndProc;
	wcex.cbClsExtra	 = 0;
	wcex.cbWndExtra	 = 0;
	wcex.hInstance		 = hInstance;
	wcex.hIcon			 = LoadIcon(hInstance, (LPCTSTR)IDI_INSTALLER);
	wcex.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	 = (LPCSTR)IDS_INSTALLER;
	wcex.lpszClassName = _classname;
	wcex.hIconSm		 = LoadIcon (hInstance, (LPCTSTR)IDI_INSTALLER);

	return RegisterClassEx (&wcex);
}


/***********************************************************************************************
 * WndProc -- Windows message handler.																			  *
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
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//	Pass this message through to the input handler. If the message
	//	was processed and requires no further action, then return with
	//	this information.
	if (_Installer.Get_Input() != NULL) {

		LRESULT result = 0;

		if (_Installer.Get_Input()->ProcessMessage (hWnd, message, wParam, lParam, result)) {
			return (result);
		}
	}

	switch (message)
	{
		case WM_ACTIVATEAPP:
			if (wParam && !GameInFocus) {
				GameInFocus = true;
			} else {
				if (!wParam && GameInFocus) {
					GameInFocus = false;
				}
			}
			return (0);

		case WM_ERASEBKGND:
			return (1);

		case WM_PAINT:
			ValidateRect (hWnd, NULL);
			break;

		case WM_CREATE:
			break;

		case WM_DESTROY:
			PostQuitMessage (0);
			break;

		case WM_SYSCOMMAND:
			
			switch (wParam) {

				case SC_CLOSE:
					
					// Windows sent us a close message - probably in response to Alt-F4. Ignore it.
					return (0);

				case SC_SCREENSAVE:
					
					// Windows is about to start the screen saver. If we just return without passing
					// this message to DefWindowProc then the screen saver will not be allowed to start.
					return (0);

				case SC_KEYMENU:

					// Ignore all "menu-activation" commands.
					return (0);
			}
			break;

		case WM_KEYUP:
			
			// Test for SHIFT + ESC.
			if ((wParam & 0xff) == VK_ESCAPE && ((GetKeyState (VK_SHIFT) & 0x8000) != 0x0)) {
				_Installer.Cancel_Introduction();
			}

			#if !NDEBUG
			// Test for Print Screen (screenshot).
			if ((wParam & 0xff) == VK_SNAPSHOT) {
				WW3D::Make_Screen_Shot();
			}
			#endif

		case WM_MOUSEWHEEL:
		{
			if (_Installer.Get_Input() != NULL) {
				_Installer.Get_Input()->Add_Mouse_Wheel (HIWORD (wParam));
				return (0);
			}
			break;
		}

		case WM_DEVICECHANGE:
		{
			PDEV_BROADCAST_HDR pdbch;

			pdbch = (PDEV_BROADCAST_HDR) lParam;
			if (pdbch != NULL) {
				if (pdbch->dbch_devicetype == DBT_DEVTYP_VOLUME) {

					// Assume that this is the CD-ROM drive.
					if (wParam == DBT_DEVICEQUERYREMOVE) {
						ShowWindow (hWnd, SW_MINIMIZE);
					} else {
						if (wParam == DBT_DEVICEARRIVAL) {
							if (IsIconic (hWnd)) {
								ShowWindow (hWnd, SW_RESTORE);
							}
							SetForegroundWindow (hWnd);
						}
					}
					return (true);
				}
			}
			break;
		}					 
		
		default:
			break;
	}

	return (DefWindowProc (hWnd, message, wParam, lParam));
}


/***********************************************************************************************
 * InstallerClass::Is_Autorun_Running -- Determine whether Autorun (a sister application) is	  *
 *													  running.															  *
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
bool Is_Autorun_Running()
{
	HANDLE autorunmutex = OpenMutex (MUTEX_ALL_ACCESS & SYNCHRONIZE, false, AUTORUN_MUTEX_OBJECT);

	if (autorunmutex != NULL) {

		HWND window = FindWindow (RxStringClass (IDS_AUTORUN_MAIN_WINDOW), NULL);
		if (window) {

			CDTimerClass <SafeTimerClass> delaytimer (10 * 1000);

			// Hang around for a while to see if Autorun is about to quit...
			while ((delaytimer.Value() > 0) && (window != NULL)) {
				window = FindWindow (RxStringClass (IDS_AUTORUN_MAIN_WINDOW), NULL);
			}
		}

		CloseHandle (autorunmutex);
		
		// If the Autorun window still exists bring it to the foreground.
		if (window) {

			if (IsIconic (window)) {
				ShowWindow (window, SW_RESTORE);
			}
			SetForegroundWindow (window);
		}
	}

	return (autorunmutex != NULL);
}


/***********************************************************************************************
 * InstallerClass::Is_Application_Running -- Determine whether this application is running.	  *
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
bool Is_Application_Running()
{
	HANDLE appmutex = CreateMutex (NULL, false, APPLICATION_MUTEX_OBJECT);
	if (appmutex == NULL) FATAL_SYSTEM_ERROR;

	// See if the application was already running.
	HWND prev = FindWindow (RxStringClass (IDS_APPLICATION_MAIN_WINDOW), NULL);
	if (prev) {
		if (IsIconic (prev)) {
			ShowWindow (prev, SW_RESTORE);
		}
		SetForegroundWindow (prev);
		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * InstallerClass::Is_Win_95_Or_Above --																		  *
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
bool Is_Win_95_Or_Above()
{
	OSVERSIONINFO versioninfo;
	BOOL			  result;	
	bool			  validos = false;

	versioninfo.dwOSVersionInfoSize = sizeof (versioninfo);
	result = GetVersionEx (&versioninfo);
	if (result) {
		if (versioninfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
			validos = true;
		}
	}
	return (validos);
}


/***********************************************************************************************
 * InstallerClass::Is_Win_2K_Or_Above --																		  *
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
bool Is_Win_2K_Or_Above()
{
	OSVERSIONINFO versioninfo;
	BOOL			  result;	
	bool			  validos = false;

	versioninfo.dwOSVersionInfoSize = sizeof (versioninfo);
	result = GetVersionEx (&versioninfo);
	if (result) {
		if (versioninfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			validos = (versioninfo.dwMajorVersion >= 5) && (versioninfo.dwMinorVersion >= 0);
		}
	}
	return (validos);
}


/***********************************************************************************************
 * InstallerClass::Running_As_Administrator --																  *
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
bool Running_As_Administrator()
{
	bool			  fAdmin;
	HANDLE		  hThread;
	TOKEN_GROUPS *ptg = NULL;
	DWORD			  cbTokenGroups;
	DWORD			  dwGroup;
	PSID			  psidAdmin;

	SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

	// Open a handle to the access token for this thread.
	if (!OpenThreadToken (GetCurrentThread(), TOKEN_QUERY, false, &hThread)) {

		if (GetLastError() == ERROR_NO_TOKEN) {

			// If the thread does not have an access token, examine the access token associated with the process.
			if (! OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &hThread)) {
				return (false);
			}
		} else {
			return (false);
		}
	}

	// Query the size of the group information associated with the token. Note that we expect a false result from GetTokenInformation
	// because we've given it a NULL buffer. On exit cbTokenGroups will tell the size of the group information.
	if (GetTokenInformation (hThread, TokenGroups, NULL, 0, &cbTokenGroups)) {
		return (false);
	}

	// Verify that GetTokenInformation failed for lack of a large enough buffer.
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return (false);
	}

	// Allocate a buffer for the group information. Since _alloca allocates on the stack, we don't have
	// to explicitly deallocate it. That happens automatically when we exit this function.
	if (!(ptg = (TOKEN_GROUPS*) _alloca (cbTokenGroups))) {
		return (false);
	}

	// Ask for the group information again. This may fail if an administrator has added this account
	// to an additional group between our first call to GetTokenInformation and this one.
	if (!GetTokenInformation (hThread, TokenGroups, ptg, cbTokenGroups, &cbTokenGroups)) {
		return (false);
	}
	
	// Create a System Identifier for the Admin group.
	if (!AllocateAndInitializeSid (&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin)) {
		return (false);
	}

	// Iterate through the list of groups for this access token looking for a match against the SID we created above.
	fAdmin = false;

	for (dwGroup = 0; dwGroup < ptg->GroupCount; dwGroup++) {
		if (EqualSid (ptg->Groups[dwGroup].Sid, psidAdmin)) {
			fAdmin = true;
			break;
		}
	}

	// Explicity deallocate the SID we created.
	FreeSid (psidAdmin);
	return (fAdmin);
}


/***********************************************************************************************
 * Prog_End	--																											  *
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
void Prog_End()
{
	try {
		
		_Installer.On_Prog_End();

	} catch (const WideStringClass &errormessage) {

		// Catch handler for fatal errors.
		DestroyWindow (MainWindow);
		Windows_Message_Handler();
		Message_Box (TxWideStringClass (IDS_APPLICATION_ERROR), errormessage);
	}
}
