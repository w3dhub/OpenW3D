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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetPropSheet.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/13/00 2:53p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "PresetPropSheet.h"
#include "parameterinheritancedialog.h"


/////////////////////////////////////////////////////////////////////////////
// Local constants
/////////////////////////////////////////////////////////////////////////////
static const int BORDER_BUTTON_X		= 6;
static const int BORDER_BUTTON_Y		= 6;
static const int BORDER_TAB_X			= 6;
static const int BORDER_TAB_Y			= 6;


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PresetPropSheetClass::OnInitDialog (void)
{
	//
	//	Get the dimensions of the buttons
	//
	CRect button_rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDCANCEL), &button_rect);
	ScreenToClient (&button_rect);		

	//
	//	Create a new button
	//
	::CreateWindow (	"BUTTON",
							"OK && Propagate...",
							WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
							0, 0,
							(button_rect.Width () * 3) / 2, button_rect.Height (),
							m_hWnd,
							(HMENU)IDC_PROPAGATE,
							::AfxGetInstanceHandle (),
							NULL);
	//
	//	Set the new button's font
	//
	SendDlgItemMessage (IDC_PROPAGATE, WM_SETFONT, ::SendMessage (m_hWnd, WM_GETFONT, 0, 0L));

	//
	//	Disable the button if we are in read-only mode
	//
	if (Is_Read_Only ()) {
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_PROPAGATE), FALSE);
	}

	return EditorPropSheetClass::OnInitDialog ();
}


/////////////////////////////////////////////////////////////////////////////
//
// Reposition_Buttons
//
/////////////////////////////////////////////////////////////////////////////
void
PresetPropSheetClass::Reposition_Buttons (int cx, int cy)
{
	//
	//	Get the dimensions of the buttons
	//
	CRect button_rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDCANCEL), &button_rect);
	ScreenToClient (&button_rect);		

	//
	//	Reposition the OK and Cancel buttons
	//
	int first_width = ((button_rect.Width () * 3) / 2);
	::SetWindowPos (::GetDlgItem (m_hWnd, IDC_PROPAGATE),
						 NULL,
						 (cx - first_width) - BORDER_BUTTON_X,
						 (cy - button_rect.Height ()) - BORDER_BUTTON_Y,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);
						 	
	::SetWindowPos (::GetDlgItem (m_hWnd, IDCANCEL),
						 NULL,
						 (cx - (button_rect.Width () + first_width)) - (BORDER_BUTTON_X * 3),
						 (cy - button_rect.Height ()) - BORDER_BUTTON_Y,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);

	::SetWindowPos (::GetDlgItem (m_hWnd, IDC_OK),
						 NULL,
						 (cx - ((button_rect.Width () * 2) + first_width)) - (BORDER_BUTTON_X * 4),
						 (cy - button_rect.Height ()) - BORDER_BUTTON_Y,
						 0,
						 0,
						 SWP_NOZORDER | SWP_NOSIZE);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PresetPropSheetClass::OnCommand (WPARAM wParam, LPARAM lParam)
{
	//
	//	If the user clicked the propagate button, then save the
	// preset changes and display the dialog
	//
	if (	LOWORD (wParam) == IDC_PROPAGATE &&
			HIWORD (wParam) == BN_CLICKED)
	{
		if (Apply_Changes ()) {
			
			//
			//	Show the propagation dialog
			//
			ParameterInheritanceDialogClass dialog (this);
			dialog.Set_Preset (m_Preset);
			dialog.DoModal ();

			//
			//	Close the property sheet
			//
			EndDialog (IDOK);
		}
	}

	return EditorPropSheetClass::OnCommand (wParam, lParam);
}
