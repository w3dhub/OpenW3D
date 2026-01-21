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

// EditStringDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "editstringdialog.h"
#include "translateobj.h"
#include "definitionclassids.h"
#include "presetmgr.h"
#include "preset.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
bool		Is_Valid_ID_Char (char ch);


/////////////////////////////////////////////////////////////////////////////
//
// EditStringDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditStringDialogClass::EditStringDialogClass(CWnd* pParent /*=NULL*/)
	:	m_Object (NULL),
		CDialog(EditStringDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditStringDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditStringDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditStringDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditStringDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditStringDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Get information about the location where we want to create the
	// sound preset picker control
	//
	HWND placeholder_wnd = ::GetDlgItem (m_hWnd, IDC_SOUND_PRESET_EDIT);
	CRect rect;
	::GetWindowRect (placeholder_wnd, &rect);
	ScreenToClient (&rect);
	::DestroyWindow (placeholder_wnd);

	//
	//	Create the preset picker control
	//
	m_PresetPicker.Create_Picker (WS_CHILD | WS_TABSTOP | WS_VISIBLE, rect, this, 101);
	m_PresetPicker.Set_Read_Only (true);

	//
	//	Configure the preset picker control
	//
	m_PresetPicker.Set_Class_ID (CLASSID_SOUND);	
	if (m_Object != NULL && m_Object->Get_Sound_ID () > 0) {
		PresetClass *preset = PresetMgrClass::Find_Preset (m_Object->Get_Sound_ID ());
		m_PresetPicker.Set_Preset (preset);
	}

	//
	//	Fill in the text controls
	//
	if (m_Object != NULL) {
		CString text = static_cast<const char *>(m_Object->Get_English_String ());
		Convert_Newline_To_Chars (text);
		
		SetDlgItemText (IDC_CODEID_EDIT, m_Object->Get_ID_Desc ());
		SetDlgItemText (IDC_STRING_EDIT, text);
		SetDlgItemText (IDC_ANIMATION_NAME_EDIT, m_Object->Get_Animation_Name ());

		//
		//	Select the string so the user can readily modify its contents
		//
		::SetFocus (::GetDlgItem (m_hWnd, IDC_STRING_EDIT));
		SendDlgItemMessage (IDC_STRING_EDIT, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	} else {
		SetDlgItemText (IDC_CODEID_EDIT, "IDS_");
		
		//
		//	Select the CODE ID so the user can enter a valid ID
		//
		::SetFocus (::GetDlgItem (m_hWnd, IDC_CODEID_EDIT));
		SendDlgItemMessage (IDC_CODEID_EDIT, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
	}
	
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Newline_To_Chars
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringDialogClass::Convert_Newline_To_Chars (CString &string)
{
	CString retval;

	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.GetLength ();

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (string[index] == '\n') {
			retval += "\\n";
		} else if (string[index] == '\t') {
			retval += "\\t";
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Chars_To_Newline
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringDialogClass::Convert_Chars_To_Newline (CString &string)
{
	CString retval;
	
	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.GetLength ();
	//retval.GetBufferSetLength (count);

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (index + 1 < count && string[index] == '\\' && string[index + 1] == 'n') {
			retval += '\n';
			index ++;
		} else if (index + 1 < count && string[index] == '\\' && string[index + 1] == 't') {
			retval += '\t';
			index ++;
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
EditStringDialogClass::OnOK (void)
{
	if (Validate_ID ()) {
		
		//
		//	Create a new translation object (if necessary)
		//		
		if (m_Object == NULL) {						
			m_Object = new TDBObjClass;
		}

		//
		//	Read the strings the user entered
		//
		CString code_id;
		GetDlgItemText (IDC_CODEID_EDIT, code_id);

		CString text;
		GetDlgItemText (IDC_STRING_EDIT, text);
		Convert_Chars_To_Newline (text);

		CString anim_name;
		GetDlgItemText (IDC_ANIMATION_NAME_EDIT, anim_name);

		//
		//	Pass the new strings onto the translation object
		//
		m_Object->Set_ID_Desc (code_id);
		m_Object->Set_English_String (text);
		m_Object->Set_Animation_Name (anim_name);
		
		//
		//	Pass the sound preset ID onto the translation object
		//
		int sound_id = -1;
		PresetClass *sound_preset = m_PresetPicker.Get_Preset ();
		if (sound_preset != NULL) {
			sound_id = sound_preset->Get_ID ();
		}
		m_Object->Set_Sound_ID (sound_id);
		
		CDialog::OnOK ();
	}
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Validate_ID
//
/////////////////////////////////////////////////////////////////////////////
bool
EditStringDialogClass::Validate_ID (void)
{
	//
	//	Get the string ID from the control
	//
	CString code_id;
	GetDlgItemText (IDC_CODEID_EDIT, code_id);

	//
	//	Validate the ID
	//
	return Validate_String_ID (m_hWnd, code_id);
}


/////////////////////////////////////////////////////////////////////////////
//
// Validate_String_ID
//
/////////////////////////////////////////////////////////////////////////////
bool
Validate_String_ID (HWND wnd, const char *string_id)
{
	bool retval = false;
	CString code_id = string_id;

	if (code_id.GetLength () == 0) {
		
		//
		//	Warn the user they need to enter some ID
		//
		::MessageBox (wnd, "Please enter a descriptive token for the code identifier.", "Invalid Code ID", MB_ICONERROR | MB_OK);

	} else if (code_id.GetLength () < 4) {

		//
		//	Warn the user they should enter a descriptive name
		//
		::MessageBox (wnd, "Code identifier is too short. Please enter a more descriptive token for the code identifier.", "Invalid Code ID", MB_ICONERROR | MB_OK);

	} else if (code_id[0] < 0x41 || code_id[0] > 0x7A || (code_id[0] >= 0x5B && code_id[0] <= 0x60)) {
		
		//
		//	Warn the user the name must start with a character
		//
		::MessageBox (wnd, "The code identifier must begin with a character (not a number), please enter a valid identifier.", "Invalid Code ID", MB_ICONERROR | MB_OK);

	} else {
		
		//
		//	Check each character in the string to make sure its exceptable
		//
		int count		= code_id.GetLength ();
		bool is_valid	= true;
		for (int index = 1; is_valid && index < count; index ++) {			
			is_valid &= Is_Valid_ID_Char (code_id[index]);
		}

		if (is_valid == false) {
			
			//
			//	Warn the user that the code ID contains an illegal character
			//
			::MessageBox (wnd, "The code identifier can only contain characters (a-z, A-Z), numbers (0-9), and the underscore character (_), please enter a valid identifier.", "Invalid Code ID", MB_ICONERROR | MB_OK);

		} else {
			retval = true;
		}
	}


	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Is_Valid_ID_Char
//
/////////////////////////////////////////////////////////////////////////////
bool
Is_Valid_ID_Char (char ch)
{
	bool is_valid = false;
	
	//
	//	The character if valid if its either a character, a number, or an underscore
	//
	is_valid |= (ch >= '0' && ch <= '9');
	is_valid |= (ch >= 'A' && ch <= 'Z');
	is_valid |= (ch >= 'a' && ch <= 'z');
	is_valid |= (ch == '_');
	
	return is_valid;
}

