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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ParameterCtrls.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/27/01 5:16p                                              $*
 *                                                                                             *
 *                    $Revision:: 36                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "parameterctrls.h"
#include "simpleparameter.h"
#include "Utils.h"
#include "filemgr.h"
#include "vector3.h"
#include "PhysObjectEditDialog.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "presetmgr.h"
#include "preset.h"
#include "icons.h"
#include "ScriptEditDialog.h"
#include "PresetListDialog.h"
#include "EditScript.h"
#include "ScriptMgr.h"
#include "combatchunkid.h"
#include "parameterctrls.h"
#include "ZoneEditDialog.h"
#include "specsheet.h"
#include "phys.h"
#include "definitionutils.h"
#include "editfilenamelistdialog.h"
#include "colorpickerdialogclass.h"


///////////////////////////////////////////////////////////////////////////////////////////
//	Local inlines
///////////////////////////////////////////////////////////////////////////////////////////
inline int
Get_Ctrl_Height (HWND hdlg, int dlg_units)
{
	RECT temp_rect;
	temp_rect.left		= 0;
	temp_rect.top		= 0;
	temp_rect.right	= dlg_units;
	temp_rect.bottom	= dlg_units;
	::MapDialogRect (::GetParent (hdlg), &temp_rect);
	return temp_rect.bottom - temp_rect.top;
}

inline void
Get_String_Size (HWND parent_wnd, LPCTSTR text, LPSIZE text_size)
{
	HDC hdc = ::GetDC (parent_wnd);
	HFONT hold_font = (HFONT)::SelectObject (hdc, (HFONT)::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	GetTextExtentPoint32 (hdc, text, ::lstrlen (text), text_size);
	::SelectObject (hdc, hold_font);
	::ReleaseDC (parent_wnd, hdc);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//	Local prototypes
///////////////////////////////////////////////////////////////////////////////////////////
void Create_Float_Ctrls (CWnd *parent_wnd, CEdit &edit_ctrl, CSpinButtonCtrl &spin_ctrl, const CRect &rect, int id1, int id2);


/////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////
static const int BORDER_WIDTH		= 10;
static const int SPACE_WIDTH		= 5;
static const int TEXT_HEIGHT		= 10;
static const int CTRL_HEIGHT		= 13;


//*******************************************************************************************//
//
//	Local utility functions
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create_Static_Text_Controls
//
//////////////////////////////////////////////////////////////////////
void
Create_Static_Text_Controls
(
	HWND				parent_wnd,
	const CRect &	control_rect,
	CStatic *		left_text,
	LPCTSTR			param_name,
	CStatic *		right_text,
	LPCTSTR			units_name,
	CRect *			remaining_rect
)
{
	//
	//	Determine the dimensions of the parameter name
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);

	//
	//	Build a rectangle which will contain the parameter name
	//
	int ctrl_height	= ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= control_rect;
	text_rect.right	= text_rect.left + text_size.cx + 2;
	text_rect.top		+= ((ctrl_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;


	//
	//	Create the left static text control (showing the param name)
	//
	left_text->Create (	param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));

	left_text->SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	
	//
	//	Build a rectangle that represents the left over control area
	//
	remaining_rect->left		= text_rect.right + SPACE_WIDTH;
	remaining_rect->right	= control_rect.right;
	remaining_rect->top		= control_rect.top;
	remaining_rect->bottom	= control_rect.top + ctrl_height;
	
	//
	//	Do we need to create a 'units' control?
	//
	if (right_text != NULL) {		

		//
		//	Calculate the width of the units string
		//
		Get_String_Size (parent_wnd, units_name, &text_size);
		CRect text2_rect	= text_rect;
		text2_rect.left	= control_rect.right - (text_size.cx + 2);
		text2_rect.right	= control_rect.right;

		//
		//	Create the right static text control (showing the param units)
		//
		right_text->Create (	units_name,
									WS_CHILD | WS_VISIBLE | SS_LEFT,
									text2_rect,
									CWnd::FromHandle (parent_wnd));

		right_text->SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

		//
		//	Shorten the remaining rectangle
		//
		remaining_rect->right	= text2_rect.left - SPACE_WIDTH;
	}

	return; 
}

//////////////////////////////////////////////////////////////////////
//
//	Resize_Controls
//
//////////////////////////////////////////////////////////////////////
void
Resize_Controls
(
	const CRect &	rect,
	CWnd &			left_wnd,
	CWnd &			middle_wnd,
	CWnd &			right_wnd
)
{
	//
	//	Get the current bounding boxes of the controls
	//
	CRect old_rect1;
	CRect old_rect2;
	CRect old_rect3;
	left_wnd.GetWindowRect (&old_rect1);	
	middle_wnd.GetWindowRect (&old_rect2);
	right_wnd.GetWindowRect (&old_rect3);

	HWND parent_wnd = ::GetParent (left_wnd);
	::ScreenToClient (parent_wnd, (LPPOINT)&old_rect2);
	::ScreenToClient (parent_wnd, ((LPPOINT)&old_rect2)+1);	
	::ScreenToClient (parent_wnd, (LPPOINT)&old_rect3);
	::ScreenToClient (parent_wnd, ((LPPOINT)&old_rect3)+1);

	//
	//	Resize the middle control and reposition the right control
	//
	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH + old_rect3.Width () + SPACE_WIDTH);
	middle_wnd.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	right_wnd.SetWindowPos (NULL, old_rect2.left + new_width + SPACE_WIDTH, old_rect3.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	return ;
}


//*******************************************************************************************//
//
//	Start of FileParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
FileParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int picker_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect text_rect	= *pos;
	text_rect.bottom	+= text_height;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (m_Parameter->Get_Name (),
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	CRect picker_rect		= *pos;
	picker_rect.top		= text_rect.bottom;
	picker_rect.bottom	= picker_rect.top + picker_height;

	//
	//	Create the file picker control
	//
	m_FilePicker.Create_Picker (	WS_CHILD | WS_TABSTOP | WS_VISIBLE,
											picker_rect,
											CWnd::FromHandle (parent_wnd),
											id_start);
	
	m_FilePicker.SetWindowText (m_Parameter->Get_String ());
	
	CString filter_string;
	filter_string.Format ("%s (*%s)|*%s|All Files (*.*)|*.*||", m_Parameter->Get_Description (), m_Parameter->Get_Extension (), m_Parameter->Get_Extension ());
	m_FilePicker.Set_Filter_String (filter_string);	

	pos->bottom = picker_rect.bottom;

	if (m_IsReadOnly) {
		m_FilePicker.Set_Read_Only (true);
		m_FilePicker.EnableWindow (false);
	}

	id_start ++;
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
FileParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_FilePicker.GetWindowRect (&old_rect2);

	m_StaticText.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect1.Height (), SWP_NOZORDER | SWP_NOMOVE);
	m_FilePicker.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Is_Modified
//
//////////////////////////////////////////////////////////////////////
bool
FileParameterCtrlClass::Is_Modified (void) const
{
	bool retval = false;

	if (m_Parameter != NULL) {
		
		//
		//	Simply compare the current and original values
		//
		CString curr_value;
		Get_Current_Value (curr_value);
		CString orig_value = m_Parameter->Get_String ();
		retval = (orig_value.CompareNoCase (curr_value) != 0);
	}	

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
FileParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	CString path;
	Get_Current_Value (path);

	if (::Get_File_Mgr ()->Is_Empty_Path (path) == false) {
		m_Parameter->Set_String ((LPCTSTR)path);
	} else {
		m_Parameter->Set_String ("");
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Current_Value
//
//////////////////////////////////////////////////////////////////////
void
FileParameterCtrlClass::Get_Current_Value (CString &value) const
{
	CString filename;
	m_FilePicker.GetWindowText (filename);

	value = ::Get_File_Mgr ()->Make_Full_Path (filename);
	value = ::Get_File_Mgr ()->Make_Relative_Path (value);
	return ;
}


//*******************************************************************************************//
//
//	Start of StringsDBEntryParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
StringsDBEntryParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int picker_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT) + 4;

	CRect text_rect	= *pos;
	text_rect.bottom	+= text_height;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (m_Parameter->Get_Name (),
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	CRect picker_rect		= *pos;
	picker_rect.top		= text_rect.bottom;
	picker_rect.bottom	= picker_rect.top + picker_height;

	//
	//	Create the file picker control
	//
	m_StringPicker.Create_Picker (	WS_CHILD | WS_TABSTOP | WS_VISIBLE,
												picker_rect,
												CWnd::FromHandle (parent_wnd),
												id_start);
	
	m_StringPicker.Set_Entry (m_Parameter->Get_Value ());
	m_StringPicker.Set_Read_Only (true);
	
	pos->bottom = picker_rect.bottom;

	if (m_IsReadOnly) {
		m_StringPicker.EnableWindow (false);
	}

	id_start ++;
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
StringsDBEntryParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_StringPicker.GetWindowRect (&old_rect2);

	m_StaticText.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect1.Height (), SWP_NOZORDER | SWP_NOMOVE);
	m_StringPicker.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
StringsDBEntryParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	m_Parameter->Set_Value (m_StringPicker.Get_Entry ());
	return ;
}


//*******************************************************************************************//
//
//	Start of SoundFileParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
SoundFileParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	FileParameterCtrlClass::Create (parent_wnd, id_start, pos);
	m_FilePicker.Set_Filter_String ("Sound Files (*.wav,*.mp3)|*.wav;*.mp3|All Files (*.*)|*.*||");
	return id_start;
}


//*******************************************************************************************//
//
//	Start of StringParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
StringParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int edit_height	= ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect text_rect	= *pos;
	text_rect.bottom	+= text_height;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (m_Parameter->Get_Name (),
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	CRect edit_rect	= *pos;
	edit_rect.top		= text_rect.bottom;
	edit_rect.bottom	= edit_rect.top + edit_height;

	//
	//	Create the edit control
	//
	m_EditCtrl.Create (	WS_CHILD | WS_TABSTOP | WS_VISIBLE,
								edit_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_EditCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_EditCtrl.ModifyStyleEx (0, WS_EX_CLIENTEDGE, 0);	
	m_EditCtrl.SetWindowText (m_Parameter->Get_String ());

	pos->bottom = edit_rect.bottom;

	if (m_IsReadOnly) {
		m_EditCtrl.EnableWindow (false);
	}

	id_start ++;
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
StringParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_EditCtrl.GetWindowRect (&old_rect2);

	m_StaticText.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect1.Height (), SWP_NOZORDER | SWP_NOMOVE);
	m_EditCtrl.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
StringParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	CString text;	
	m_EditCtrl.GetWindowText (text);
	m_Parameter->Set_String ((LPCTSTR)text);
	return ;
}


//*******************************************************************************************//
//
//	Start of IntParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
IntParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name	= m_Parameter->Get_Name ();
	CString units_name	= m_Parameter->Get_Units_Name ();

	//
	//	Create the static text controls
	//
	CRect edit_rect;
	Create_Static_Text_Controls (	parent_wnd,
											*pos,
											&m_StaticText1,
											param_name,
											&m_StaticText2,
											units_name,
											&edit_rect);

	//
	//	Create the edit control
	//
	m_EditCtrl.Create (	WS_CHILD | WS_TABSTOP | WS_VISIBLE,
								edit_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_EditCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_EditCtrl.ModifyStyleEx (0, WS_EX_CLIENTEDGE, 0);
	::Make_Edit_Int_Ctrl (m_EditCtrl);

	//
	//	Create the spin control
	//
	m_SpinCtrl.Create (	UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_NOTHOUSANDS | WS_CHILD | WS_VISIBLE,
								CRect (0, 0, 50, 20),
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_SpinCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	m_SpinCtrl.SetRange32 (m_Parameter->Get_Min (), m_Parameter->Get_Max ());
	
	CString text;
	text.Format ("%d", m_Parameter->Get_Value ());
	m_EditCtrl.SetWindowText (text);

	if (m_IsReadOnly) {
		m_EditCtrl.EnableWindow (false);
		m_SpinCtrl.EnableWindow (false);
	}

	pos->bottom = edit_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
IntParameterCtrlClass::Resize (const CRect &rect)
{
	Resize_Controls (rect, m_StaticText1, m_EditCtrl, m_StaticText2);
	m_SpinCtrl.SetBuddy (&m_EditCtrl);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
IntParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	CString text_value;
	m_EditCtrl.GetWindowText (text_value);
	m_Parameter->Set_Value (::atol (text_value));
	return ;
}


//*******************************************************************************************//
//
//	Start of FloatParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
FloatParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name	= m_Parameter->Get_Name ();
	CString units_name	= m_Parameter->Get_Units_Name ();

	//
	//	Create the static text controls
	//
	CRect edit_rect;
	Create_Static_Text_Controls (	parent_wnd,
											*pos,
											&m_StaticText1,
											param_name,
											&m_StaticText2,
											units_name,
											&edit_rect);

	//
	//	Create the float controls
	//
	::Create_Float_Ctrls (	CWnd::FromHandle (parent_wnd),
									m_EditCtrl,
									m_SpinCtrl,
									edit_rect,
									id_start++,
									id_start ++);

	
	::SetWindowFloat (m_EditCtrl, m_Parameter->Get_Value ());
	m_SpinCtrl.SetRange32 (m_Parameter->Get_Min () * 100, m_Parameter->Get_Max () * 100);

	if (m_IsReadOnly) {
		m_EditCtrl.EnableWindow (false);
		m_SpinCtrl.EnableWindow (false);
	}

	pos->bottom = edit_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
FloatParameterCtrlClass::Resize (const CRect &rect)
{
	Resize_Controls (rect, m_StaticText1, m_EditCtrl, m_StaticText2);
	m_SpinCtrl.SetBuddy (&m_EditCtrl);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
FloatParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	m_Parameter->Set_Value (GetWindowFloat (m_EditCtrl, true));
	return ;
}



//*******************************************************************************************//
//
//	Start of AngleParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
AngleParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name	= m_Parameter->Get_Name ();
	CString units_name	= m_Parameter->Get_Units_Name ();

	//
	//	Create the static text controls
	//
	CRect edit_rect;
	Create_Static_Text_Controls (	parent_wnd,
											*pos,
											&m_StaticText1,
											param_name,
											&m_StaticText2,
											units_name,
											&edit_rect);

	//
	//	Create the float controls
	//
	::Create_Float_Ctrls (	CWnd::FromHandle (parent_wnd),
									m_EditCtrl,
									m_SpinCtrl,
									edit_rect,
									id_start++,
									id_start ++);
	
	//
	//	Set the initial value of the control
	//
	::SetWindowFloat (m_EditCtrl, RAD_TO_DEG (m_Parameter->Get_Value ()));
	
	//
	//	Set the ranges on the spinner
	//
	float min_ang = RAD_TO_DEG (m_Parameter->Get_Min ());
	float max_ang = RAD_TO_DEG (m_Parameter->Get_Max ());
	m_SpinCtrl.SetRange32 (min_ang * 100, max_ang * 100);

	//
	//	Make the controls read-only if necessary
	//
	if (m_IsReadOnly) {
		m_EditCtrl.EnableWindow (false);
		m_SpinCtrl.EnableWindow (false);
	}

	pos->bottom = edit_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
AngleParameterCtrlClass::Resize (const CRect &rect)
{
	Resize_Controls (rect, m_StaticText1, m_EditCtrl, m_StaticText2);
	m_SpinCtrl.SetBuddy (&m_EditCtrl);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
AngleParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	m_Parameter->Set_Value (DEG_TO_RAD (::GetWindowFloat (m_EditCtrl, true)));
	return ;
}


//*******************************************************************************************//
//
//	Start of BoolParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
BoolParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the checkbox's dimensions
	//
	CRect check_rect	= *pos;
	check_rect.bottom	= check_rect.top + ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	//
	//	Create the edit control
	//
	m_CheckBoxCtrl.Create (	param_name,
									WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX,
									check_rect,
									CWnd::FromHandle (parent_wnd),
									id_start ++);
	m_CheckBoxCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_CheckBoxCtrl.SetCheck (m_Parameter->Get_Value ());

	if (m_IsReadOnly) {
		m_CheckBoxCtrl.EnableWindow (false);
	}

	pos->bottom = check_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
BoolParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect;
	m_CheckBoxCtrl.GetWindowRect (&old_rect);

	//int new_width = rect.Width () - (old_rect.Width () + SPACE_WIDTH);
	m_CheckBoxCtrl.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
BoolParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	m_Parameter->Set_Value (bool(m_CheckBoxCtrl.GetCheck () == 1));
	return ;
}


//*******************************************************************************************//
//
//	Start of Vector3ParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
Vector3ParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int edit_height	= ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect group_rect	= *pos;
	group_rect.bottom	+= edit_height + text_height + SPACE_WIDTH + SPACE_WIDTH;

	//
	//	Create the static text control
	//	
	m_GroupBox.Create (	m_Parameter->Get_Name (),
								WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_GROUPBOX,
								group_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_GroupBox.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the labels, don't worry about their position, they will be moved later.
	//
	m_TextCtrls[0].Create ("X:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[1].Create ("Y:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[2].Create ("Z:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[0].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[1].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[2].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the float controls
	//
	int y_pos = group_rect.top + SPACE_WIDTH + (group_rect.Height () >> 1) - (edit_height >> 1);
	for (int index = 0; index < 3; index ++) {

		::Create_Float_Ctrls (	CWnd::FromHandle (parent_wnd),
										m_EditCtrls[index],
										m_SpinCtrls[index],
										CRect (0, y_pos, 10, y_pos + edit_height),
										id_start++,
										id_start ++);
	}

	Vector3 value = m_Parameter->Get_Value ();
	::SetWindowFloat (m_EditCtrls[0], value.X);
	::SetWindowFloat (m_EditCtrls[1], value.Y);
	::SetWindowFloat (m_EditCtrls[2], value.Z);

	CRect parent_rect;
	::GetClientRect (parent_wnd, &parent_rect);
	Resize (parent_rect);

	if (m_IsReadOnly) {
		m_EditCtrls[0].EnableWindow (false);
		m_EditCtrls[1].EnableWindow (false);
		m_EditCtrls[2].EnableWindow (false);
		m_SpinCtrls[0].EnableWindow (false);
		m_SpinCtrls[1].EnableWindow (false);
		m_SpinCtrls[2].EnableWindow (false);
	}

	pos->bottom = group_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
Vector3ParameterCtrlClass::Resize (const CRect &rect)
{
	//
	//	Resize the group box
	//
	CRect group_rect;
	m_GroupBox.GetClientRect (&group_rect);
	group_rect.left	= 0;
	group_rect.right	= rect.Width ();
	m_GroupBox.SetWindowPos (NULL, 0, 0, group_rect.Width (), group_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);


	//
	//	Determine the dimensions of the labels
	//
	HWND parent_wnd = ::GetParent (m_GroupBox);
	CSize text_size;
	Get_String_Size (parent_wnd, "XX: ", &text_size);

	//
	//	Resize the edit controls
	//
	CRect edit_rect;
	m_EditCtrls[0].GetWindowRect (&edit_rect);
	::ScreenToClient (parent_wnd, (LPPOINT)&edit_rect);
	::ScreenToClient (parent_wnd, ((LPPOINT)&edit_rect)+1);
	
	int width = (group_rect.Width () - (BORDER_WIDTH * 2) - (text_size.cx * 3)) / 3;
	edit_rect.left		= group_rect.left + BORDER_WIDTH;
	edit_rect.right	= edit_rect.left + text_size.cx + width;

	for (int index = 0; index < 3; index ++) {
		
		int y = edit_rect.top + (edit_rect.Height () >> 1) - (text_size.cy >> 1);
		m_TextCtrls[index].SetWindowPos (NULL, edit_rect.left, y, text_size.cx, text_size.cy, SWP_NOZORDER);
		
		edit_rect.left	+= text_size.cx;
		m_EditCtrls[index].SetWindowPos (NULL, edit_rect.left, edit_rect.top, edit_rect.Width (), edit_rect.Height (), SWP_NOZORDER);
		m_SpinCtrls[index].SetBuddy (&m_EditCtrls[index]);

		edit_rect.left		+= width;
		edit_rect.right	+= (text_size.cx + width);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
Vector3ParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	Vector3 value;
	value.X	= ::GetWindowFloat (m_EditCtrls[0], true);
	value.Y	= ::GetWindowFloat (m_EditCtrls[1], true);
	value.Z	= ::GetWindowFloat (m_EditCtrls[2], true);
	
	m_Parameter->Set_Value (value);
	return ;
}


//*******************************************************************************************//
//
//	Start of Vector2ParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
Vector2ParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int edit_height	= ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect group_rect	= *pos;
	group_rect.bottom	+= edit_height + text_height + SPACE_WIDTH + SPACE_WIDTH;

	//
	//	Create the static text control
	//	
	m_GroupBox.Create (	m_Parameter->Get_Name (),
								WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_GROUPBOX,
								group_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_GroupBox.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the labels, don't worry about their position, they will be moved later.
	//
	m_TextCtrls[0].Create ("X:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[1].Create ("Y:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[0].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[1].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the float controls
	//
	int y_pos = group_rect.top + SPACE_WIDTH + (group_rect.Height () >> 1) - (edit_height >> 1);
	for (int index = 0; index < 2; index ++) {

		::Create_Float_Ctrls (	CWnd::FromHandle (parent_wnd),
										m_EditCtrls[index],
										m_SpinCtrls[index],
										CRect (0, y_pos, 10, y_pos + edit_height),
										id_start++,
										id_start ++);
	}

	Vector2 value = m_Parameter->Get_Value ();
	::SetWindowFloat (m_EditCtrls[0], value.X);
	::SetWindowFloat (m_EditCtrls[1], value.Y);

	CRect parent_rect;
	::GetClientRect (parent_wnd, &parent_rect);
	Resize (parent_rect);

	if (m_IsReadOnly) {
		m_EditCtrls[0].EnableWindow (false);
		m_EditCtrls[1].EnableWindow (false);
		m_SpinCtrls[0].EnableWindow (false);
		m_SpinCtrls[1].EnableWindow (false);
	}

	pos->bottom = group_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
Vector2ParameterCtrlClass::Resize (const CRect &rect)
{
	//
	//	Resize the group box
	//
	CRect group_rect;
	m_GroupBox.GetClientRect (&group_rect);
	group_rect.left	= 0;
	group_rect.right	= rect.Width ();
	m_GroupBox.SetWindowPos (NULL, 0, 0, group_rect.Width (), group_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);


	//
	//	Determine the dimensions of the labels
	//
	HWND parent_wnd = ::GetParent (m_GroupBox);
	CSize text_size;
	Get_String_Size (parent_wnd, "XX: ", &text_size);

	//
	//	Resize the edit controls
	//
	CRect edit_rect;
	m_EditCtrls[0].GetWindowRect (&edit_rect);
	::ScreenToClient (parent_wnd, (LPPOINT)&edit_rect);
	::ScreenToClient (parent_wnd, ((LPPOINT)&edit_rect)+1);
	
	int width = (group_rect.Width () - (BORDER_WIDTH * 3) - (text_size.cx * 2)) / 2;
	edit_rect.left		= group_rect.left + BORDER_WIDTH;
	edit_rect.right	= edit_rect.left + text_size.cx + width;

	for (int index = 0; index < 2; index ++) {
		
		int y = edit_rect.top + (edit_rect.Height () >> 1) - (text_size.cy >> 1);
		m_TextCtrls[index].SetWindowPos (NULL, edit_rect.left, y, text_size.cx, text_size.cy, SWP_NOZORDER);
		
		edit_rect.left	+= text_size.cx;
		m_EditCtrls[index].SetWindowPos (NULL, edit_rect.left, edit_rect.top, edit_rect.Width (), edit_rect.Height (), SWP_NOZORDER);
		m_SpinCtrls[index].SetBuddy (&m_EditCtrls[index]);

		edit_rect.left		+= width;
		edit_rect.right	+= (text_size.cx + width);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
Vector2ParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	Vector2 value;
	value.X	= ::GetWindowFloat (m_EditCtrls[0], true);
	value.Y	= ::GetWindowFloat (m_EditCtrls[1], true);
	
	m_Parameter->Set_Value (value);
	return ;
}







//*******************************************************************************************//
//
//	Start of RectParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
RectParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	int text_height	= ::Get_Ctrl_Height (parent_wnd, TEXT_HEIGHT);
	int edit_height	= ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect group_rect	= *pos;
	group_rect.bottom	+= edit_height + text_height + SPACE_WIDTH + SPACE_WIDTH;

	//
	//	Create the static text control
	//	
	m_GroupBox.Create (	m_Parameter->Get_Name (),
								WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_GROUPBOX,
								group_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_GroupBox.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the labels, don't worry about their position, they will be moved later.
	//
	m_TextCtrls[0].Create ("X1:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[1].Create ("Y1:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[2].Create ("X2:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[3].Create ("Y2:", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect (0, 0, 10, 10), CWnd::FromHandle (parent_wnd));
	m_TextCtrls[0].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[1].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[2].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	m_TextCtrls[3].SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the float controls
	//
	int y_pos = group_rect.top + SPACE_WIDTH + (group_rect.Height () >> 1) - (edit_height >> 1);
	for (int index = 0; index < 4; index ++) {

		::Create_Float_Ctrls (	CWnd::FromHandle (parent_wnd),
										m_EditCtrls[index],
										m_SpinCtrls[index],
										CRect (0, y_pos, 10, y_pos + edit_height),
										id_start++,
										id_start ++);
	}

	RectClass value = m_Parameter->Get_Value ();
	::SetWindowFloat (m_EditCtrls[0], value.Left);
	::SetWindowFloat (m_EditCtrls[1], value.Top);
	::SetWindowFloat (m_EditCtrls[2], value.Right);
	::SetWindowFloat (m_EditCtrls[3], value.Bottom);

	CRect parent_rect;
	::GetClientRect (parent_wnd, &parent_rect);
	Resize (parent_rect);

	if (m_IsReadOnly) {
		m_EditCtrls[0].EnableWindow (false);
		m_EditCtrls[1].EnableWindow (false);
		m_EditCtrls[2].EnableWindow (false);
		m_EditCtrls[3].EnableWindow (false);
		m_SpinCtrls[0].EnableWindow (false);
		m_SpinCtrls[1].EnableWindow (false);
		m_SpinCtrls[2].EnableWindow (false);
		m_SpinCtrls[3].EnableWindow (false);
	}

	pos->bottom = group_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
RectParameterCtrlClass::Resize (const CRect &rect)
{
	//
	//	Resize the group box
	//
	CRect group_rect;
	m_GroupBox.GetClientRect (&group_rect);
	group_rect.left	= 0;
	group_rect.right	= rect.Width ();
	m_GroupBox.SetWindowPos (NULL, 0, 0, group_rect.Width (), group_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);


	//
	//	Determine the dimensions of the labels
	//
	HWND parent_wnd = ::GetParent (m_GroupBox);
	CSize text_size;
	Get_String_Size (parent_wnd, "XX:  ", &text_size);

	//
	//	Resize the edit controls
	//
	CRect edit_rect;
	m_EditCtrls[0].GetWindowRect (&edit_rect);
	::ScreenToClient (parent_wnd, (LPPOINT)&edit_rect);
	::ScreenToClient (parent_wnd, ((LPPOINT)&edit_rect)+1);
	
	int width = (group_rect.Width () - (BORDER_WIDTH * 5) - (text_size.cx * 4)) / 4;
	edit_rect.left		= group_rect.left + BORDER_WIDTH;
	edit_rect.right	= edit_rect.left + text_size.cx + width;

	for (int index = 0; index < 4; index ++) {
		
		int y = edit_rect.top + (edit_rect.Height () >> 1) - (text_size.cy >> 1);
		m_TextCtrls[index].SetWindowPos (NULL, edit_rect.left, y, text_size.cx, text_size.cy, SWP_NOZORDER);
		
		edit_rect.left	+= text_size.cx;
		m_EditCtrls[index].SetWindowPos (NULL, edit_rect.left, edit_rect.top, edit_rect.Width (), edit_rect.Height (), SWP_NOZORDER);
		m_SpinCtrls[index].SetBuddy (&m_EditCtrls[index]);

		edit_rect.left		+= width + BORDER_WIDTH;
		edit_rect.right	+= (text_size.cx + width) + BORDER_WIDTH;
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
RectParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	RectClass value;
	value.Left		= ::GetWindowFloat (m_EditCtrls[0], true);
	value.Top		= ::GetWindowFloat (m_EditCtrls[1], true);
	value.Right		= ::GetWindowFloat (m_EditCtrls[2], true);
	value.Bottom	= ::GetWindowFloat (m_EditCtrls[3], true);
	
	m_Parameter->Set_Value (value);
	return ;
}





//////////////////////////////////////////////////////////////////////
//
//	Create_Float_Ctrls
//
//////////////////////////////////////////////////////////////////////
void
Create_Float_Ctrls
(
	CWnd *				parent_wnd,
	CEdit &				edit_ctrl,
	CSpinButtonCtrl &	spin_ctrl,
	const CRect &		rect,
	int					id1,
	int					id2
)
{
	//
	//	Create the edit control
	//
	edit_ctrl.Create (WS_CHILD | WS_TABSTOP | WS_VISIBLE, rect, parent_wnd, id1);
	edit_ctrl.SendMessage (WM_SETFONT, ::SendMessage (*parent_wnd, WM_GETFONT, 0, 0L));
	edit_ctrl.ModifyStyleEx (0, WS_EX_CLIENTEDGE, 0);
	::Make_Edit_Float_Ctrl (edit_ctrl);

	//
	//	Create the spin control
	//
	spin_ctrl.Create (	UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_AUTOBUDDY | UDS_NOTHOUSANDS | UDS_ALIGNRIGHT | WS_CHILD | WS_VISIBLE,
								CRect (0, 0, 50, 20),
								parent_wnd,
								id2);
	spin_ctrl.SendMessage (WM_SETFONT, ::SendMessage (*parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Set the float control ranges
	//
	spin_ctrl.SetRange32 (-1000000, 1000000);	
	return ;
}


//*******************************************************************************************//
//
//	Start of EnumParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
EnumParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);

	int combo_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 2;
	text_rect.top		+= ((combo_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the edit control's dimensions
	//
	CRect combo_rect	= *pos;
	combo_rect.left	= text_rect.right + SPACE_WIDTH;
	combo_rect.bottom	= combo_rect.top + (combo_height * 10);

	//
	//	Create the edit control
	//
	m_ComboBox.Create (	WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
								combo_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_ComboBox.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));
	//m_ComboBox.ModifyStyleEx (0, WS_EX_CLIENTEDGE, 0);

	//
	//	Fill in the list of enumerated values
	//
	int count = m_Parameter->Get_Count ();
	for (int index = 0; index < count; index ++) {
		LPCTSTR name	= m_Parameter->Get_Entry_Name (index);
		int value		= m_Parameter->Get_Entry_Value (index);

		int combo_index = m_ComboBox.AddString (name);
		m_ComboBox.SetItemData (combo_index, value);

		if (value == m_Parameter->Get_Selected_Value ()) {
			m_ComboBox.SetCurSel (combo_index);
		}
	}

	if (m_IsReadOnly) {
		m_ComboBox.EnableWindow (false);
	}

	pos->bottom = combo_rect.top + combo_height;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
EnumParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_ComboBox.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_ComboBox.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
EnumParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	int cur_sel = m_ComboBox.GetCurSel ();
	if (cur_sel >= 0) {
		int value = m_ComboBox.GetItemData (cur_sel);
		m_Parameter->Set_Selected_Value (value);
	}

	return ;
}


//*******************************************************************************************//
//
//	Start of ColorParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
ColorParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 2;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	m_Button.Create (	"",
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	if (m_IsReadOnly) {
		m_Button.EnableWindow (false);
	}

	pos->bottom = button_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
ColorParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
ColorParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
ColorParameterCtrlClass::On_Command
(
	HWND		parent_wnd,
	WPARAM	wparam,
	LPARAM	/* lparam */
)
{
	bool retval = false;

	if (	LOWORD (wparam) == m_ButtonID &&
			HIWORD (wparam) == BN_CLICKED	)
	{
		//
		//	Get the current color fron the control
		//
		Vector3 color = m_Parameter->Get_Value ();
		int red		= color.X * 255;
		int green	= color.Y * 255;
		int blue		= color.Z * 255;
				
		//
		//	Show the color picker to the user
		//
		if (::Show_Color_Picker (&red, &green, &blue)) {
			
			//
			//	Store the new color
			//
			color.X = ((float)red) / 255.0F;
			color.Y = ((float)green) / 255.0F;
			color.Z = ((float)blue) / 255.0F;
			m_Parameter->Set_Value (color);
			
			::InvalidateRect (::GetDlgItem (parent_wnd, m_ButtonID), NULL, true);
			::UpdateWindow (::GetDlgItem (parent_wnd, m_ButtonID));
		}

		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	On_DrawItem
//
//////////////////////////////////////////////////////////////////////
bool
ColorParameterCtrlClass::On_DrawItem
(
	HWND		/* parent_wnd */,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	bool retval = false;
	
	//
	//	Is this the right control?
	//
	if ((int)wparam == m_ButtonID) {
		LPDRAWITEMSTRUCT drawitem_info = (LPDRAWITEMSTRUCT)lparam;
		
		// Determine what color to paint the button
		Vector3 color = m_Parameter->Get_Value ();

		// Determine what state to draw the button in (pushed or normal)
		UINT state = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
		if (drawitem_info->itemState & ODS_SELECTED) {
			state |= DFCS_PUSHED;
		}

		// Draw the button's outline
		CRect rect = drawitem_info->rcItem;
		::DrawFrameControl (drawitem_info->hDC, rect, DFC_BUTTON, state);

		// Fill the button with the appropriate color
		CDC temp_dc;
		temp_dc.Attach (drawitem_info->hDC);
		temp_dc.FillSolidRect (&rect, RGB (int(color.X * 255), int(color.Y * 255), int(color.Z * 255)));
		temp_dc.Detach ();
		
		// Draw the focus rectangle if necessary
		if (drawitem_info->itemState & ODS_FOCUS) {
			::DrawFocusRect (drawitem_info->hDC, &rect);
		}
				
		retval = true;
	}

	return retval;
}


//*******************************************************************************************//
//
//	Start of PhysDefParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
PhysDefParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 2;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	m_Button.Create (	"Edit Phyiscs Object...",
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	if (m_IsReadOnly) {
		m_Button.EnableWindow (false);
	}

	pos->bottom = button_rect.bottom;	
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
PhysDefParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
PhysDefParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
PhysDefParameterCtrlClass::On_Command
(
	HWND		parent_wnd,
	WPARAM	wparam,
	LPARAM	/* lparam */
)
{
	bool retval = false;

	if (	LOWORD (wparam) == m_ButtonID &&
			HIWORD (wparam) == BN_CLICKED)
	{
		//
		//	Setup the dialog's parameters
		//

		PhysObjectEditDialogClass dialog (CWnd::FromHandle (parent_wnd));
		dialog.Set_Definition_ID (m_Parameter->Get_Value ());
		dialog.Set_Filter (m_Parameter->Get_Base_Class ());
		dialog.Set_Is_Temp (m_IsTemp);

		//
		//	Show the dialog
		//
		if (dialog.DoModal () == IDOK) {
			m_Parameter->Set_Value (dialog.Get_Definition_ID ());
		}
				
		retval = true;
	}

	return retval;
}


//*******************************************************************************************//
//
//	Start of DefParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
DefParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = Get_Parameter ()->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int picker_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT) + 4;
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 2;
	text_rect.top		+= ((picker_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));


	//
	//	Determine the edit control's dimensions
	//
	CRect picker_rect		= *pos;
	picker_rect.left		= text_rect.right + SPACE_WIDTH;
	picker_rect.bottom	= picker_rect.top + picker_height;

	//
	//	Create the preset picker control
	//
	m_PresetPicker.Create_Picker (	WS_CHILD | WS_TABSTOP | WS_VISIBLE,
												picker_rect,
												CWnd::FromHandle (parent_wnd),
												id_start);
	
	//
	//	Pass the current preset onto the control
	//
	PresetClass *preset = PresetMgrClass::Find_Preset (m_Parameter->Get_Value ());
	m_PresetPicker.Set_Preset (preset);
	m_PresetPicker.Set_Read_Only (true);
	m_PresetPicker.Set_Class_ID (Get_Def_Class_ID ());	
	m_PresetPicker.Set_Icon_Index (Get_Def_Icon ());
	
	//
	//	Make the controls read-only (if necessary)
	//
	if (m_IsReadOnly) {		
		m_PresetPicker.EnableWindow (false);
	}

	pos->bottom = picker_rect.top + picker_height;
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
DefParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_PresetPicker.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_PresetPicker.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
//////////////////////////////////////////////////////////////////////
void
DefParameterCtrlClass::On_Destroy (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
DefParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	PresetClass *preset = (PresetClass *)m_PresetPicker.Get_Preset ();
	if (preset != NULL) {
		m_Parameter->Set_Value (preset->Get_ID ());
	} else {
		m_Parameter->Set_Value (0);
	}

	return ;
}


//*******************************************************************************************//
//
//	Start of ScriptParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
ScriptParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 32;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_CheckBoxCtrl.Create (	param_name,
									WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX,
									text_rect,
									CWnd::FromHandle (parent_wnd),
									id_start ++);
	m_CheckBoxCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	CString button_text;
	button_text.Format ("Edit Script (%s)...", m_Parameter->Get_Script_Name ());
	m_Button.Create (	button_text,
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	pos->bottom = button_rect.bottom;
	m_CheckBoxCtrl.SetCheck (::lstrlen (m_Parameter->Get_Script_Name ()) > 0);
	m_Button.EnableWindow (m_CheckBoxCtrl.GetCheck ());

	if (m_IsReadOnly) {
		m_CheckBoxCtrl.EnableWindow (false);
		m_Button.EnableWindow (false);
	}

	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
ScriptParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_CheckBoxCtrl.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
ScriptParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	//
	//	Clear the data if necessary
	//
	if (m_CheckBoxCtrl.GetCheck () == false) {
		m_Parameter->Set_Script_Name ("");
		m_Parameter->Set_Params ("");
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
ScriptParameterCtrlClass::On_Command
(
	HWND		parent_wnd,
	WPARAM	wparam,
	LPARAM	/* lparam */
)
{
	bool retval = false;

	if (	LOWORD (wparam) == m_ButtonID &&
			HIWORD (wparam) == BN_CLICKED)
	{
		//
		//	Setup the dialog's parameters
		//
		ScriptEditDialogClass dialog (CWnd::FromHandle (parent_wnd));
		
		//
		//	Initialize the dialog with the current script settings
		//
		EditScriptClass *template_script = ScriptMgrClass::Find_Script (m_Parameter->Get_Script_Name ());
		if (template_script != NULL) {
			EditScriptClass temp_script = *template_script;
			temp_script.Set_Composite_Values (m_Parameter->Get_Params ());
			dialog.Set_Script (temp_script);
		}		

		//
		//	Show the dialog
		//
		if (dialog.DoModal () == IDOK) {
			
			//
			//	Record the settings
			//
			EditScriptClass &script = dialog.Get_Script ();
			m_Parameter->Set_Script_Name (script.Get_Name ());
			m_Parameter->Set_Params (script.Get_Composite_String ());

			//
			//	Rename the button
			//
			CString button_text;
			button_text.Format ("Edit Script (%s)...", script.Get_Name ());
			m_Button.SetWindowText (button_text);

			//
			//	Update the window
			//
			m_CheckBoxCtrl.SetCheck (::lstrlen (m_Parameter->Get_Script_Name ()) > 0);			
		}
				
		retval = true;
	} else if (	LOWORD (wparam) == ::GetWindowLong (m_CheckBoxCtrl, GWL_ID) &&
					HIWORD (wparam) == BN_CLICKED)
	{
		m_Button.EnableWindow (m_CheckBoxCtrl.GetCheck ());
	}


	return retval;
}


//*******************************************************************************************//
//
//	Start of DefIDListParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
DefIDListParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 32;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	m_Button.Create (	"Pick Presets...",
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	pos->bottom = button_rect.bottom;		

	if (m_IsReadOnly) {
		m_StaticText.EnableWindow (false);
		m_Button.EnableWindow (false);
	}

	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
DefIDListParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
DefIDListParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
DefIDListParameterCtrlClass::On_Command
(
	HWND		parent_wnd,
	WPARAM	wparam,
	LPARAM	/* lparam */
)
{
	bool retval = false;

	if (	LOWORD (wparam) == m_ButtonID &&
			HIWORD (wparam) == BN_CLICKED)
	{
		//
		//	Setup the dialog's parameters
		//
		PresetListDialogClass dialog (CWnd::FromHandle (parent_wnd));
		
		//
		//	What class-id should we let the user pick from?
		//
		uint32 root_class_id			= m_Parameter->Get_Class_ID ();
		uint32 selected_class_id	= 0;
		uint32 *class_id_ptr			= m_Parameter->Get_Selected_Class_ID ();
		if (class_id_ptr != NULL) {
			selected_class_id = (*class_id_ptr);
		}

		//
		//	Configure the dialog
		//
		DynamicVectorClass<int> &list = m_Parameter->Get_List ();
		dialog.Set_Preset_List (&list);
		dialog.Set_Class_ID (root_class_id);
		dialog.Set_Selected_Class_ID (selected_class_id);
		dialog.Enable_Type_Selection (class_id_ptr != NULL);

		//
		//	Display the dialog
		//
		if (dialog.DoModal () == IDOK) {
			if (class_id_ptr != NULL) {
				(*class_id_ptr) = dialog.Get_Selected_Class_ID ();
				m_Parameter->Set_Modified ();
			}
		}

		retval = true;
	}

	return retval;
}


//*******************************************************************************************//
//
//	Start of ZoneParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
ZoneParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 32;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	m_Button.Create (	"Edit Zone...",
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	pos->bottom = button_rect.bottom;		

	if (m_IsReadOnly) {
		m_StaticText.EnableWindow (false);
		m_Button.EnableWindow (false);
	}

	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
ZoneParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
ZoneParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
ZoneParameterCtrlClass::On_Command
(
	HWND		/* parent_wnd */,
	WPARAM	/* wparam */,
	LPARAM	/* lparam */
)
{
	bool retval = false;
	return retval;
}


//*******************************************************************************************//
//
//	Start of FilenameListParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
FilenameListParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the dimensions of the title
	//
	CSize text_size;
	Get_String_Size (parent_wnd, param_name, &text_size);
	
	int button_height = ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);
	CRect text_rect	= *pos;
	text_rect.right	= text_rect.left + text_size.cx + 32;
	text_rect.top		+= ((button_height >> 1) - (text_size.cy >> 1));
	text_rect.bottom	= text_rect.top + text_size.cy;

	//
	//	Create the static text control
	//	
	m_StaticText.Create (param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								text_rect,
								CWnd::FromHandle (parent_wnd));
	m_StaticText.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Determine the control's dimensions
	//
	CRect button_rect		= *pos;
	button_rect.left		= text_rect.right + SPACE_WIDTH;
	button_rect.bottom	= button_rect.top + button_height;

	//
	//	Create the edit control
	//
	m_ButtonID = id_start ++;
	m_Button.Create (	"Edit File List...",
							WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
							button_rect,
							CWnd::FromHandle (parent_wnd),
							m_ButtonID);
	m_Button.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	pos->bottom = button_rect.bottom;		

	if (m_IsReadOnly) {
		m_StaticText.EnableWindow (false);
		m_Button.EnableWindow (false);
	}

	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
FilenameListParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect1;
	CRect old_rect2;
	m_StaticText.GetWindowRect (&old_rect1);
	m_Button.GetWindowRect (&old_rect2);

	int new_width = rect.Width () - (old_rect1.Width () + SPACE_WIDTH);
	m_Button.SetWindowPos (NULL, 0, 0, new_width, old_rect2.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Read_Data
//
//////////////////////////////////////////////////////////////////////
void
FilenameListParameterCtrlClass::Read_Data (HWND /*parent_wnd*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
bool
FilenameListParameterCtrlClass::On_Command
(
	HWND		parent_wnd,
	WPARAM	wparam,
	LPARAM	/* lparam */
)
{
	bool retval = false;

	if (	LOWORD (wparam) == m_ButtonID &&
			HIWORD (wparam) == BN_CLICKED)
	{
		//
		//	Setup the dialog's parameters
		//
		EditFilenameListDialogClass dialog (CWnd::FromHandle (parent_wnd));		
		DynamicVectorClass<StringClass> &filename_list = m_Parameter->Get_List ();
		dialog.Set_List (filename_list);

		//
		//	Display the dialog
		//
		if (dialog.DoModal () == IDOK) {
			filename_list = dialog.Get_List ();
			m_Parameter->Set_Modified ();
		}

		retval = true;
	}

	return retval;
}


//*******************************************************************************************//
//
//	Start of SeparatorParameterCtrlClass
//
//*******************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
int
SeparatorParameterCtrlClass::Create
(
	HWND		parent_wnd,
	int		id_start,
	LPRECT	pos
)
{
	CString param_name = m_Parameter->Get_Name ();

	//
	//	Determine the ctrl's dimensions
	//
	CRect ctrl_rect	= *pos;
	ctrl_rect.top		+= BORDER_WIDTH / 2;
	ctrl_rect.bottom	= ctrl_rect.top + ::Get_Ctrl_Height (parent_wnd, CTRL_HEIGHT);

	//
	//	Determine the text's dimensions
	//
	CSize size;
	::Get_String_Size (parent_wnd, param_name, &size);


	//
	//	Create the frame control
	//
	CRect frame_rect = ctrl_rect;
	frame_rect.top		= ctrl_rect.top + (size.cy / 2);
	frame_rect.bottom	= frame_rect.top + 5;
	m_FrameCtrl.Create (	param_name,
								WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
								frame_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_FrameCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	//
	//	Create the static text control
	//
	ctrl_rect.left		+= BORDER_WIDTH;
	ctrl_rect.right	= ctrl_rect.left + size.cx + 1;
	m_TextCtrl.Create (	param_name,
								WS_CHILD | WS_VISIBLE | SS_LEFT,
								ctrl_rect,
								CWnd::FromHandle (parent_wnd),
								id_start ++);
	m_TextCtrl.SendMessage (WM_SETFONT, ::SendMessage (parent_wnd, WM_GETFONT, 0, 0L));

	pos->bottom = ctrl_rect.bottom;
	return id_start;
}


//////////////////////////////////////////////////////////////////////
//
//	Resize
//
//////////////////////////////////////////////////////////////////////
void
SeparatorParameterCtrlClass::Resize (const CRect &rect)
{
	CRect old_rect;
	
	m_FrameCtrl.GetWindowRect (&old_rect);
	m_FrameCtrl.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);

	//m_TextCtrl.GetWindowRect (&old_rect);
	//m_TextCtrl.SetWindowPos (NULL, 0, 0, rect.Width (), old_rect.Height (), SWP_NOZORDER | SWP_NOMOVE);
	return ;
}

