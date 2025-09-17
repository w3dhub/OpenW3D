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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Splash.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/07/99 5:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "Splash.h"
#include "SplashScreen.h"
#include "Utils.h"


///////////////////////////////////////////////////////////////////////
//	Forward declarations
///////////////////////////////////////////////////////////////////////
UINT fnSplashScreenThread (DWORD_PTR, DWORD_PTR, DWORD_PTR, HRESULT *, HWND *phmain_wnd);


///////////////////////////////////////////////////////////////////////
//
//	SplashClass
//
///////////////////////////////////////////////////////////////////////
SplashClass::SplashClass (void)
	:	m_hThreadWnd (NULL)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~SplashClass
//
///////////////////////////////////////////////////////////////////////
SplashClass::~SplashClass (void)
{
	Close ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Status_Text
//
///////////////////////////////////////////////////////////////////////
void
SplashClass::Set_Status_Text (LPCTSTR text)
{
	if (m_hThreadWnd != NULL) {
		::PostMessage (m_hThreadWnd, WM_USER+102, 0, (LPARAM)text);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Show
//
///////////////////////////////////////////////////////////////////////
void
SplashClass::Show (bool show)
{
	//
	//	Kick off a thread that will display the splash screen
	//
	if (m_hThreadWnd == NULL) {
		::Create_UI_Thread (fnSplashScreenThread, 0, 0, 0, NULL, &m_hThreadWnd);
	}

	::ShowWindow (m_hThreadWnd, show ? SW_SHOW : SW_HIDE);

	//
	//	Register ourselves as the current status window
	//
	if (show) {
		ProgressUIMgrClass::Set_Current_Progress_UI (this);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Close
//
///////////////////////////////////////////////////////////////////////
void
SplashClass::Close (void)
{
	//
	// Close out the window
	//
	if (::IsWindow (m_hThreadWnd)) {
		::PostMessage (m_hThreadWnd, WM_USER+101, 0, 0L);
		m_hThreadWnd = NULL;
	}

	if (ProgressUIMgrClass::Get_Current_Progress_UI () == this) {
		ProgressUIMgrClass::Set_Current_Progress_UI (NULL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnSplashScreenThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnSplashScreenThread
(
	DWORD_PTR /*dwparam1*/,
	DWORD_PTR /*dwparam2*/,
	DWORD_PTR /*dwparam3*/,
	HRESULT* /*presult*/,
	HWND* phmain_wnd
)
{
	//
	// Create a new instance of the 'updating' dialog
	//
	SplashScreenClass *splash_wnd = new SplashScreenClass;
	splash_wnd->CreateEx (0, "STATIC", "Level Edit", WS_POPUP | WS_VISIBLE, 0, 0, 100, 100, NULL, NULL);
	splash_wnd->ShowWindow (SW_SHOW);

	//
	// Return the window handle of the main wnd to the calling thread
	//
	if (phmain_wnd != NULL) {
		(*phmain_wnd) = splash_wnd->m_hWnd;
	}

	return 1;
}
