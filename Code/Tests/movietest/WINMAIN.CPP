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

/* $Header: /Commando/Code/Tests/movietest/WINMAIN.CPP 2     3/21/98 12:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tests/movietest/WINMAIN.CPP                  $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/06/98 2:47p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   WinMain -- Win32 Program Entry Point!                                                     * 
 *   WIN_resize -- Surrender-required function which resizes the main window                   * 
 *   WIN_set_fullscreen -- Surrender-required function for toggling full-screen mode           * 
 *   Main_Window_Proc -- Windows Proc for the main game window                                 * 
 *   Create_Main_Window -- Creates the main game window                                        * 
 *   Focus_Loss -- this function is called when the application loses focus                    * 
 *   Focus_Restore -- This function is called when the application gets focus                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "winmain.h"
#include <sr.hpp>
#include "win.h"
#include "wwmouse.h"
#include "init.h"
#include "mainloop.h"
#include "shutdown.h"
#include "_globals.h"


//----------------------------------------------------------------------------
//	Globals
//----------------------------------------------------------------------------
extern "C" 
{
	HWND		hWndMain;
	bool		WIN_fullscreen = true;
	
//PORT130	SRBOOL SRCALL WIN_resize(SRLONG width, SRLONG height);
//PORT130	SRBOOL SRCALL WIN_set_fullscreen(SRBOOL state);
}

//----------------------------------------------------------------------------
//	Local functions
//----------------------------------------------------------------------------
static BOOL Create_Main_Window(HANDLE hInstance, int nCmdShow);
LRESULT CALLBACK Main_Window_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void Focus_Loss(void);
void Focus_Restore(void);
void Split_Command_Line_Args(HINSTANCE instance, char *path_to_exe, char *command_line);
void Set_Working_Directory(char *old_path, char *new_path);



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
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	LPSTR		command;
	HANDLE	prev;
	char		path_to_exe[_MAX_PATH];
	char		oldpath[_MAX_PATH];

	command = lpCmdLine;
	prev	= hPrevInstance;

	if (!Create_Main_Window(hInstance, nCmdShow)) return 0;

	// Setup the keyboard system
	Keyboard = new WWKeyboardClass();

	// Setup the mouse system and take over the mouse.
	MouseCursor = new WWMouseClass(NULL, MainWindow);

	Split_Command_Line_Args(hInstance, &path_to_exe[0], lpCmdLine);
	Set_Working_Directory(oldpath, &path_to_exe[0]);

	Init();

	Main_Loop();
	
	Shutdown();

	delete Keyboard;
	delete MouseCursor;

	return(EXIT_SUCCESS);
} 

/*********************************************************************************************** 
 * WIN_resize -- Surrender-required function which resizes the main window                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * 																														  * 
 * 	width -- desired new width																					  * 
 * 	height -- desired new height																				  * 
 * 																														  * 
 * OUTPUT:                                                                                     * 
 * 																														  * 
 * 	SRTRUE = success SRFALSE = error																			  * 
 * 																														  * 
 * WARNINGS:																											  * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/18/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
#ifdef PORT130
SRBOOL SRCALL WIN_resize( SRLONG width, SRLONG height )
{
	RECT			rc;
	RECT			rcWork;
	SRLONG		xp,yp;

	SetRect( &rc, 0,0, width,height);
	AdjustWindowRectEx( &rc, GetWindowStyle(hWndMain), 
						GetMenu(hWndMain)!=NULL,
						GetWindowExStyle(hWndMain));

	xp = (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))/2;
	yp = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top))/2;

	SetWindowPos( hWndMain, HWND_NOTOPMOST, xp, yp, rc.right-rc.left, rc.bottom-rc.top,
				            SWP_NOZORDER | SWP_NOACTIVATE);

	// make sure the window does not hang outside work area
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
	GetWindowRect(hWndMain, &rc);

	if (rc.left < rcWork.left) rc.left = rcWork.left;
	if (rc.top  < rcWork.top)  rc.top  = rcWork.top;

	SetWindowPos( hWndMain, NULL, rc.left, rc.top, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	UpdateWindow( hWndMain);

	return SRTRUE;
}
#endif

/*********************************************************************************************** 
 * WIN_set_fullscreen -- Surrender-required function for toggling full-screen mode             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *  																														  * 
 * 	state -- if SRTRUE, go into fullscreen mode, else go to windowed mode						  * 
 * 																														  * 
 * OUTPUT:																												  * 
 * 																														  * 
 * 	SRTRUE = sucess SRFALSE = failure																		  * 
 * 																														  * 
 * WARNINGS:																											  * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/18/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
#ifdef PORT130
SRBOOL SRCALL WIN_set_fullscreen( SRBOOL state )
{
	DWORD			dwStyle;

	if (state == SRFALSE)	{

		// change window style to popup
		dwStyle = GetWindowStyle( hWndMain);
		dwStyle |= WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX;
		SetWindowLong( hWndMain, GWL_STYLE, dwStyle);

	} else {

		// change window style to exclusive
		dwStyle = GetWindowStyle( hWndMain);
		dwStyle |= WS_POPUP | WS_VISIBLE;
		dwStyle &= ~(WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX);
		SetWindowLong( hWndMain, GWL_STYLE, dwStyle);
	}

	// set state flag accordingly
	WIN_fullscreen = state;

	return SRTRUE;
}
#endif
				
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
	PAINTSTRUCT ps;
	HDC         hdc;

	/*
	**	Pass this message through to the keyboard handler. If the message
	**	was processed and requires no further action, then return with
	**	this information.
	*/
//	if (Keyboard && Keyboard->Message_Handler(hwnd, message, wParam, lParam)) {
//		return(1);
//	}
	if (Keyboard) {
		Keyboard->Message_Handler(hwnd, message, wParam, lParam);
	}


	switch (message )
	{
		/*
		** basic management messages
		*/
		case WM_ACTIVATEAPP:

			if (WIN_fullscreen) {

				GameInFocus = (wParam != 0);
				if (!GameInFocus) {
					Focus_Loss();
				} else {
					Focus_Restore();
				}

			} else {

				GameInFocus = true;
				if (wParam != 0) {
					if (MouseCursor != NULL) MouseCursor->Capture_Mouse();
				} else {
					if (MouseCursor != NULL) MouseCursor->Release_Mouse();
				}

			}
			return(0);

		case WM_SETCURSOR:
			SetCursor(NULL);
			return 1;

		case WM_ERASEBKGND:
			return 1;

		case WM_PAINT:
			hdc = BeginPaint( hwnd, &ps);
			EndPaint( hwnd, &ps);
			return 1;

		/*
		** minimize/maximize
		*/
		case WM_SYSKEYDOWN:

			if (wParam == VK_RETURN && ((lParam>>16) & KF_ALTDOWN) && !((lParam>>16) & KF_REPEAT))
			{
				WIN_fullscreen = !WIN_fullscreen;
			}
			break;

		/*
		** interface open and close
		*/
		case WM_CREATE:
			break;

		case WM_DESTROY:
			ReleaseCapture();
			PostQuitMessage( 0);
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {

				case SC_CLOSE:
					/*
					** Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
					** pretending to handle the message and returning true;
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
 * 	hInstance -- Instance handle of the application														  * 
 * 	nCmdShow -- how the window is to be shown																  * 
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
static BOOL Create_Main_Window(HANDLE hInstance, int nCmdShow)
{
	WNDCLASS    wc;
	BOOL        rc;

	ProgramInstance = hInstance;

	wc.style				= CS_DBLCLKS;
	wc.lpfnWndProc		= Main_Window_Proc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon				= LoadIcon( NULL, IDI_APPLICATION);
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW);
	wc.hbrBackground	= GetStockObject( BLACK_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= "SRCLASS";

	rc = RegisterClass( &wc);
	if (!rc ) return FALSE;

	MainWindow = hWndMain = CreateWindowEx(
		0,											// WS_EX_TOPMOST,
		"SRClass",
		"Commando",
		WS_VISIBLE |							// so we don't have to call ShowWindow
		WS_POPUP |								// non-app window
		WS_SYSMENU,								// so we get an icon in the tray
		0, 0,										// top left corner
		GetSystemMetrics(SM_CXSCREEN),	// bottom right corner
		GetSystemMetrics(SM_CYSCREEN),
		NULL,										// no parent handle
		NULL,										// no menu handle
		ProgramInstance,						// main program instance
		NULL);									// creation parameters

	if (!hWndMain) {
		return FALSE;
	}

	return TRUE;
}


/*********************************************************************************************** 
 * Focus_Loss -- this function is called when the application loses focus                      * 
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
void Focus_Loss(void)
{
}


/*********************************************************************************************** 
 * Focus_Restore -- This function is called when the application gets focus                    * 
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
void Focus_Restore(void)
{
}


void Prog_End(void)
{
//	Sound_End();
	MouseCursor->Release_Mouse();
	delete MouseCursor;
	MouseCursor = NULL;
}


void Split_Command_Line_Args(HINSTANCE instance, char *path_to_exe, char *command_line)
{
	// first arguement is the path to the executable including file name
	GetModuleFileName (instance, &path_to_exe[0], 132);
	Argv[0] = path_to_exe;	

	char * token = strtok(command_line, " ");
	Argc = 1;
	while (Argc < ARRAY_SIZE(Argv) && token != NULL) {
		Argv[Argc++] = token;
		token = strtok(NULL, " ");
	}
}

void Set_Working_Directory(char *old_path, char *new_path)
{
	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char dir[_MAX_DIR];
	/*
	**	Remember the current working directory and drive.
	*/
	GetCurrentDirectory(_MAX_PATH, old_path);

	/*
	**	Change directory to the where the executable is located. Handle the
	**	case where there is no path attached to argv[0].
	*/
	_splitpath(new_path, drive, dir, NULL, NULL);
	_makepath(path, drive, dir, NULL, NULL);
	SetCurrentDirectory(path);
}

