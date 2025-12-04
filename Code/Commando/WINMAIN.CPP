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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/WINMAIN.CPP                         $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/17/02 11:08a                                              $*
 *                                                                                             *
 *                    $Revision:: 85                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WinMain -- Win32 Program Entry Point!                                                     *
 *   WIN_resize -- Surrender-required function which resizes the main window                   *
 *   WIN_set_fullscreen -- Surrender-required function for toggling full-screen mode           *
 *   Main_Window_Proc -- Windows Proc for the main game window                                 *
 *   Create_Main_Window -- Creates the main game window                                        *
 *   On_Focus_Loss -- this function is called when the application loses focus                 *
 *   On_Focus_Restore -- This function is called when the application gets focus               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "WINMAIN.H"
#define _WIN32_WINDOWS 0x0401
#include "winmain_compat.h"
#include "win.h"
#include "resource.h"
#include "ww3d.h"

#include "miscutil.h"
#include "input.h"
#include "useroptions.h"

#include "WWAudio.h"
#include "ffactory.h"

#include "debug.h"
#include "verchk.h"
#include "devoptions.h"
#include "dialogmgr.h"
#include "renegadedialogmgr.h"
#include "Except.h"

#include "directinput.h"
#include "WebBrowser.h"
#include "wwmemlog.h"

#include "datasafe.h"

#include "combatgmode.h"
#include "registry.h"
#include "init.h"
#include "_globals.h"
#include "buildnum.h"
#include "dx8wrapper.h"
#include "formconv.h"
#include "AutoStart.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include "slavemaster.h"

#include "ServerSettings.h"
#include "openw3d.h"
#include "ini.h"

#ifdef _DEBUG
#include	<crtdbg.h>
#endif //_DEBUG
#include <iostream>
#include "singletoninstancekeeper.h"
#include "packetmgr.h"

//----------------------------------------------------------------------------
//	Globals
//----------------------------------------------------------------------------
extern "C"
{
	HWND		hWndMain;
	bool		WIN_fullscreen = true;
}


//----------------------------------------------------------------------------
//	Local functions
//----------------------------------------------------------------------------
static BOOL Create_Main_Window(void);
LRESULT CALLBACK Main_Window_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void On_Focus_Loss(void);
void On_Focus_Restore(void);
int Start_Application( void );
bool Set_Working_Directory(void);

//#include "packet.h"

#if 1 //ndef _MSC_VER
int Top_Level_Exception_Filter(EXCEPTION_POINTERS *e_info)
{
	return(Exception_Handler(e_info->ExceptionRecord->ExceptionCode, e_info));
}
#endif //_MSC_VER


/***********************************************************************************************
 * WinMain -- Win32 Program Entry Point!                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  																														  *
 * 	Standard WinMain inputs :-)																				  *
 * 																														  *
 * OUTPUT:																												  *
 *  																														  *
 * 	Standard WinMain output																						  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int main(int argc, char *argv[])
{
	WWMemoryLogClass::Init();	// This switches memlog from static to dynamic allocations mode

#ifdef _DEBUG
	/*
	**	Setup to track memory heap integrity and check for
	**	memory leaks. Output is dumped to the debug string console.
	*/
	if (cDevOptions::CrtDbgEnabled.Is_True()) {

		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	/*
	** Leak test
	*/
	//char *flibble = new char [1024];
	//unsigned int *flibble2 = new unsigned int;
	//void *flibble3 = malloc(128);

	//flibble = flibble;
	//flibble2 = flibble2;
	//flibble3 = flibble3;
#endif //_DEBUG

#ifdef WWDEBUG
	//
	// If necessary, disable packet optimizations
	//
	if (cDevOptions::PacketOptimizationsEnabled.Is_False()) {
		PacketManager.Disable_Optimizations();
	}
#endif //WWDEBUG

	//
	// There's a couple of options we need for the FDS
	//
#ifdef FREEDEDICATEDSERVER
	if (!SlaveMaster.Am_I_Slave() && !ServerSettingsClass::Is_Server_Settings_File_Set()) {
		cUserOptions::Set_Server_INI_File("server.ini");
	}
	ConsoleBox.Set_Exclusive(true);
#endif //FREEDEDICATEDSERVER

	switch (cUserOptions::Parse_Command_Line(argc, argv)) {
	case cUserOptions::ParseResult::SUCCESS:
		break;
	case cUserOptions::ParseResult::FAILURE:
        fprintf(stderr, "Error during parsing arguments\n");
		cUserOptions::Print_Command_Line_Help(true);
		return 1;
	case cUserOptions::ParseResult::PRINT_HELP:
		cUserOptions::Print_Command_Line_Help(false);
		return 0;
	}

#ifdef _DEBUG // Denzil - DO NOT COMPILE INTO FINAL BUILD
	bool webInstalled = WebBrowser::InstallPrerequisites();

	if (!webInstalled) {
		::MessageBoxA(NULL, "Embedded browser prerequisites missing\n\nBrowser functionality questionable.\n\n(Please call Denzil @ 27272)",
				"Renegade Warning!", MB_ICONWARNING|MB_OK);
	}
#endif // WWDEBUG

	//
	//	Verify that we can execute (i.e. make sure there are no other instances running)
	//
	SingletonInstanceKeeperClass instance_keeper;
	if (instance_keeper.Verify_Safe_To_Execute () == false) {
		return 0;
	}

	//
	//	Start the game!
	//
	int retval = Start_Application();
	return retval;
}

static bool Graphics_Settings_Trouble_Shooting()
{
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if (!registry.Is_Valid()) return true;

	int progress=registry.Get_Int( VALUE_NAME_GAME_INITIALIZATION_IN_PROGRESS, 0 );
	if (progress) {
		StringClass options="wwconfig.exe ";
		char* opts=options.Peek_Buffer();
		STARTUPINFOA startup_info;
		ZeroMemory(&startup_info,sizeof(STARTUPINFOA));
		startup_info.cb=sizeof(STARTUPINFOA);
		PROCESS_INFORMATION process_info;
		CreateProcessA(
			NULL,
			opts,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&startup_info,
			&process_info);

		DWORD exit_code=STILL_ACTIVE;
		unsigned res=0;
		do {
			res=GetExitCodeProcess(process_info.hProcess,&exit_code);
			if (!res) {
				return true;
			}
			Sleep(100);
		}
		while (exit_code==STILL_ACTIVE);
		return !exit_code;
	}
	return true;
}

typedef IDirect3D9* (WINAPI *Direct3DCreate8Type) (UINT SDKVersion);
static Direct3DCreate8Type	Direct3DCreate8Ptr = NULL;
static HMODULE D3D8Lib = NULL;

static bool Video_Card_Driver_Check()
{
	INIClass ini(W3D_CONF_FILE);

	int disabled=ini.Get_Int(W3D_SECTION_RENDER, "DriverVersionCheckDisabled", 0);
	if (disabled>=87) return true;

	IDirect3D9* d3d=NULL;
	D3DCAPS9 tmp_caps;
	const D3DCAPS9* d3dcaps=NULL;
	D3DADAPTER_IDENTIFIER9 adapter_id;

	// Init D3D
	Init_D3D_To_WW3_Conversion();

	D3D8Lib = LoadLibraryA("D3D9.DLL");

	if (D3D8Lib != NULL) {
		Direct3DCreate8Ptr = (Direct3DCreate8Type) GetProcAddress(D3D8Lib, "Direct3DCreate9");
		if (Direct3DCreate8Ptr) {
			d3d=Direct3DCreate8Ptr(D3D_SDK_VERSION);		// TODO: handle failure cases...
			if (!d3d) {
				FreeLibrary(D3D8Lib);
				return true;
			}
		} else {
			FreeLibrary(D3D8Lib);
			return(true);
		}
	} else {
		return(true);
	}

	// Select device. If there is already a device selected in the registry, use it.

	int current_adapter_index=D3DADAPTER_DEFAULT;

	//
	//	Load the render device settings from the registry
	//
	char device_name[256] = { 0 };
	ini.Get_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, "", device_name, sizeof(device_name));
	
	int adapter_count = d3d->GetAdapterCount();
	for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {
		D3DADAPTER_IDENTIFIER9 id;
		::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER9));
		HRESULT res = d3d->GetAdapterIdentifier(adapter_index,0/*D3DENUM_WHQL_LEVEL*/,&id);
		// If device ok, check if it matches the currently set adapter name
		if (res == D3D_OK) {
			StringClass name(id.Description,true);
			if (name==device_name) {
				current_adapter_index=adapter_index;
				break;
			}
		}
	}

	if (FAILED(d3d->GetDeviceCaps(
		current_adapter_index,
		D3DDEVTYPE_HAL,
		&tmp_caps))) {
		d3d->Release();
		FreeLibrary(D3D8Lib);
		return true;
	}

	::ZeroMemory(&adapter_id, sizeof(D3DADAPTER_IDENTIFIER9));
	if (FAILED( d3d->GetAdapterIdentifier(
		current_adapter_index,
		0/*D3DENUM_WHQL_LEVEL*/,
		&adapter_id))) {
		d3d->Release();
		FreeLibrary(D3D8Lib);
		return true;
	}

	d3dcaps=&tmp_caps;
	DX8Caps caps(d3d,*d3dcaps,WW3D_FORMAT_UNKNOWN,adapter_id);

	DX8Caps::DriverVersionStatusType status=caps.Get_Driver_Version_Status();

	d3d->Release();
	FreeLibrary(D3D8Lib);

	switch (status) {
	default:
	case DX8Caps::DRIVER_STATUS_GOOD:
	case DX8Caps::DRIVER_STATUS_OK:
	case DX8Caps::DRIVER_STATUS_UNKNOWN:
		ini.Put_Int(W3D_SECTION_RENDER, "DriverVersionCheckDisabled", 87);
		ini.Save(W3D_CONF_FILE);
		return true;
	case DX8Caps::DRIVER_STATUS_BAD:
		break;
	}

	StringClass options="wwconfig.exe -driverversion";
	char* opts=options.Peek_Buffer();
	STARTUPINFOA startup_info;
	ZeroMemory(&startup_info,sizeof(STARTUPINFOA));
	startup_info.cb=sizeof(STARTUPINFOA);
	PROCESS_INFORMATION process_info;
	if (!CreateProcessA(
		NULL,
		opts,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startup_info,
		&process_info)) {
		return true;
	}

	DWORD exit_code=STILL_ACTIVE;
	unsigned res=0;
	do {
		res=GetExitCodeProcess(process_info.hProcess,&exit_code);
		if (!res) {
			return true;
		}
		Sleep(100);
	}
	while (exit_code==STILL_ACTIVE);
	return !exit_code;
}


/***********************************************************************************************
 * Start_Application -- Handles main execution.	                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * 	Standard main output                                                                       *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/2001 PDS  : Created.                                                                *
 *=============================================================================================*/
int Start_Application(void)
{
	{
		WWMEMLOG(MEM_GAMEINIT);
		//LPSTR	command	= lpCmdLine;

		/*
		** Set the working directory.
		*/
		if (!Set_Working_Directory()) {
			MessageBoxA(NULL,"Set working folder and try again...","Invalid working folder",MB_OK);
			return 0;
		}

		//
		// TEMP Dev code - check the working folder is correct!
		//
		char tmp_buffer[256];
		char* tmp_ptr;
		if (!SearchPathA(
			"data",
			"always.dat",
			NULL,
			sizeof(tmp_buffer),
			tmp_buffer,
			&tmp_ptr)) {
			MessageBoxA(NULL,"Set working folder and try again...","Invalid working folder",MB_OK);
			return 0;
		}

		/*
		** Only do these checks if this isn't an auto-restart. If we are restarting then we must have run once OK already. Right?
		*/
		if (AutoRestart.Get_Restart_Flag() == false && !ConsoleBox.Is_Exclusive()) {
			if (!Video_Card_Driver_Check()) return 0;
			if (!Graphics_Settings_Trouble_Shooting()) return 0;
		}

		//Debug_Say(("Started logging at time %s", cMiscUtil::Get_Text_Time()));

		if (!Create_Main_Window()) return 0;

		Register_Thread_ID(GetCurrentThreadId(), "Main Thread", true);
	}

	int exitCode = EXIT_SUCCESS;

#ifdef WWDEBUG
	if (cDevOptions::EnableExceptionHandler.Is_False()) {
		exitCode = Game_Main_Loop();
	} else {
#endif //WWDEBUG

		Register_Application_Exception_Callback(&Application_Exception_Callback);
		Register_Application_Version_Callback(&BuildInfoClass::Composite_Build_Info);

#if 0 //def _MSC_VER
		/*
		** The __try/__except construct is part of the WIN32 interface. This is not the same
		** as C++ exception handling. There is no additional code overhead required
		** with this interface and the compilers exception handling options should be disabled
		** to prevent the compiler generating additional stack unwinding code.
		*/
		__try {
#else
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) &Top_Level_Exception_Filter);
#endif
			WWDEBUG_SAY(("Game_Main_Loop\n"));
			exitCode = Game_Main_Loop();

#if 0 //def _MSC_VER
		} __except(Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {};

		/*
		** Set the exception filter to use on shutdown.
		*/
#else
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) &Top_Level_Exception_Filter);
#endif
#ifdef WWDEBUG
	}
#endif //WWDEBUG

	Unregister_Thread_ID(GetCurrentThreadId(), "Main Thread");

   //Debug_Say(("Finished logging at time %s", cMiscUtil::Get_Text_Time()));

	return exitCode;
}

/***********************************************************************************************
 * Main_Window_Proc -- Windows Proc for the main game window                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * 																														  *
 * 	Standard Windows Proc inputs																				  *
 * 																														  *
 * OUTPUT:																												  *
 * 																														  *
 * 	Standard Windows Proc output																				  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
LRESULT CALLBACK Main_Window_Proc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static bool _reset = false;

	/*
	**	Pass this message through to the input handler. If the message
	**	was processed and requires no further action, then return with
	**	this information.
	*/
	if (_TheWWUIInput && Input::Is_Console_Enabled () == false) {
		LRESULT result = 0;

		//DialogWalker->Process_Message(hwnd, message, wParam, lParam, result);

		if (_TheWWUIInput->ProcessMessage(hwnd, message, wParam, lParam, result)) {
			return result;
		}
	}

	switch (message )
	{
		/*
		** basic management messages
		*/
		case WM_ACTIVATEAPP:
			if (wParam && !GameInFocus) {
				WWDEBUG_SAY(("***** FOCUS GAINED *****\n"));
				GameInFocus = true;
				On_Focus_Restore();
			} else if (!wParam && GameInFocus) {
				WWDEBUG_SAY(("***** FOCUS LOST *****\n"));
				GameInFocus = false;
				On_Focus_Loss();
			}
			return(0);

		case WM_ERASEBKGND:
			return 1;

		case WM_PAINT:
			ValidateRect(hwnd, NULL);
			break;

		/*
		** minimize/maximize
		*/
		case WM_SYSKEYDOWN:

			if (wParam == VK_RETURN && ((lParam>>16) & KF_ALTDOWN) && !((lParam>>16) & KF_REPEAT))
			{
				if (WIN_fullscreen) {
					WIN_fullscreen = false;
				} else {
					WIN_fullscreen = true;
				}
			}

			break;

		/*
		** getch()
		*/
		case WM_CHAR:
			//Debug_Say(("WM_CHAR %d\n", wParam));
			Input::Console_Add_Key( wParam );
			break;

		/*
		** Main window creation
		*/
		case WM_CREATE:
			break;

		/*
		** Main window destruction
		*/
		case WM_DESTROY:
			ReleaseCapture ();
			PostQuitMessage (0);
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {

				case SC_CLOSE:
					/*
					** Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
					** pretending to handle the message and returning true;
					*/
					return (0);

				case SC_KEYMENU:
					/*
					** Ignore all "menu-activation" commands.
					*/
					return (0);

				case SC_SCREENSAVE:
					/*
					** Windoze is about to start the screen saver. If we just return without passing
					** this message to DefWindowProc then the screen saver will not be allowed to start.
					*/
					return (0);
			}
			break;

// Denzil - Using WM_xxxFOCUS messages to switch to windowed / fullscreen fails.
#if(0)
		case WM_KILLFOCUS:
		{
			if (WW3D::Is_Initted ()) {

				// Get the current state of the redendering device
				int width = 0;
				int height = 0;
				int bits = 0;
				bool bwindowed = false;
				WW3D::Get_Device_Resolution (width, height, bits, bwindowed);

				// If we are running fullscreen, then toggle to 'windowed'
				// and minimze the window.
				if (bwindowed == false) {
					//Debug_Say(("WM_KILLFOCUS minimize"));
					WW3D::Set_Device_Resolution (-1, -1, -1, true);
					::ShowWindow (hwnd, SW_MINIMIZE);
					_reset = true;
				}
			}
		}
		break;

		case WM_SETFOCUS:

			// If we need to reset the app to 'fullscreen' mode then
			// do so an restore the window.
			if (_reset == true) {
				WW3D::Set_Device_Resolution (-1, -1, -1, false);
				::ShowWindow (hwnd, SW_RESTORE);
				_reset = false;
			}
			break;
#endif

		case WM_COMMAND:
			if (LOWORD (wParam) == IDM_TOGGLE_FULLSCREEN) {

				// Ask WW3D to toggle the fullscreen mode for us.
				WW3D::Toggle_Windowed ();
			}

			break;

		default:
			break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


/***********************************************************************************************
 * Create_Main_Window -- Creates the main game window                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  																														  *
 * 																														  *
 * OUTPUT:																												  *
 * 																														  *
 * 	TRUE = success, FALSE = failure																			  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static BOOL Create_Main_Window(void)
{
	WNDCLASSA    wc;
	BOOL        rc;

	ProgramInstance = (HINSTANCE)GetModuleHandleA(NULL);

	if (!ConsoleBox.Is_Exclusive()) {
		wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wc.lpfnWndProc		= Main_Window_Proc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= ProgramInstance;
		wc.hIcon				= LoadIcon( NULL, IDI_APPLICATION);
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)GetStockObject( BLACK_BRUSH);
		wc.lpszMenuName	= NULL;
		wc.lpszClassName	= SingletonInstanceKeeperClass::Get_GUID ();

		rc = RegisterClassA( &wc);
		if (!rc ) return FALSE;

		// Assume windowed mode
		MainWindow = CreateWindowExA(0, SingletonInstanceKeeperClass::Get_GUID(), "Renegade",
			WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|WS_CLIPCHILDREN,
				0, 0, 0, 0, NULL, NULL, ProgramInstance, NULL);

		if (!MainWindow) {
			return FALSE;
		}

		SetFocus(MainWindow);
	}

	return TRUE;
}


/***********************************************************************************************
 * On_Focus_Loss -- this function is called when the application loses focus                   *
 *                                                                                             *
 * INPUT:		Nothing																								  *
 * 																														  *
 * OUTPUT:		Nothing																								  *
 * 																														  *
 * WARNINGS:	None																									  *
 * 																														  *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void On_Focus_Loss(void)
{
	DirectInput::Unacquire();
}


/***********************************************************************************************
 * On_Focus_Restore -- This function is called when the application gets focus                 *
 *                                                                                             *
 * INPUT:		Nothing																								  *
 * 																														  *
 * OUTPUT:		Nothing																								  *
 * 																														  *
 * WARNINGS:	None																									  *
 * 																														  *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void On_Focus_Restore(void)
{
	if (WebBrowser::IsWebPageDisplayed() == false) {
		DirectInput::Acquire();
	}

	GameModeManager::Hide_Render_Frames(1);	// Hide the first rendered frame
}


void Prog_End(void)
{
}


/***********************************************************************************************
 * Set_Working_Directory -- Sets current directory as configured in the registry, set through  *
 *                          command line options or to the same directory as this .exe.        *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Program instance                                                                  *
 *                                                                                             *
 * OUTPUT:   true if the directory was changed succesfully, else false.                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/2001 12:11PM ST : Created                                                            *
 *=============================================================================================*/
bool Set_Working_Directory(void)
{
	char path_buffer[_MAX_PATH];
	const char *gamedir = cUserOptions::GameDir.Get();
	if (gamedir == NULL || gamedir[0] == '\0') {
		char path_to_exe[256];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		GetModuleFileNameA(NULL, path_to_exe, sizeof(path_to_exe));
		_splitpath(path_to_exe, drive, dir, NULL, NULL);
		_makepath(path_buffer, drive, dir, NULL, NULL);
		gamedir = path_buffer;
	}
	return !!SetCurrentDirectoryA(gamedir);
}
