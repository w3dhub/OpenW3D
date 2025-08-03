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

// PresetGeneralTab.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "PresetGeneralTab.h"
#include "definition.h"
#include "preset.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// PresetGeneralTabClass
//
/////////////////////////////////////////////////////////////////////////////
PresetGeneralTabClass::PresetGeneralTabClass (PresetClass *preset)
	:	m_Preset (preset),
		m_IsReadOnly (false),
		DockableFormClass(PresetGeneralTabClass::IDD)
{
	//{{AFX_DATA_INIT(PresetGeneralTabClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


PresetGeneralTabClass::~PresetGeneralTabClass (void)
{
	return ;
}


void
PresetGeneralTabClass::DoDataExchange (CDataExchange *pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PresetGeneralTabClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PresetGeneralTabClass, DockableFormClass)
	//{{AFX_MSG_MAP(PresetGeneralTabClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PresetGeneralTabClass diagnostics

#ifdef _DEBUG
void PresetGeneralTabClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void PresetGeneralTabClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
PresetGeneralTabClass::HandleInitDialog (void)
{
	ASSERT (m_Preset != NULL);

	DefinitionClass *definition = m_Preset->Get_Definition ();
	if (definition != NULL) {

		//
		// Set the text of the respective dialog controls
		//
		SetDlgItemText (IDC_NAME_EDIT, definition->Get_Name ());
		SetDlgItemInt (IDC_PRESET_ID_EDIT, definition->Get_ID ());		
		SetDlgItemText (IDC_COMMENTS_EDIT, m_Preset->Get_Comments ());
	}	

	if (m_IsReadOnly) {
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_NAME_EDIT), false);
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_COMMENTS_EDIT), false);
	}

	// Put the focus into the edit control
	::SetFocus (::GetDlgItem (m_hWnd, IDC_NAME_EDIT));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
PresetGeneralTabClass::Apply_Changes (void)
{
	// Assume success
	bool retval = true;

	DefinitionClass *definition = m_Preset->Get_Definition ();
	if (definition != NULL) {

		// Pass the new name onto the base
		CString name;
		GetDlgItemText (IDC_NAME_EDIT, name);
		
		// Is the new name valid?
		if (name.GetLength () > 0) {
			definition->Set_Name (name);
		} else {

			// Let the user know they MUST enter a name for the base
			::Message_Box (m_hWnd, IDS_BASE_INFO_NONAME_MSG, IDS_BASE_INFO_NONAME_TITLE);
			
			// Put the focus back to the control
			::SetFocus (::GetDlgItem (m_hWnd, IDC_NAME_EDIT));
			retval = false;
		}

		// Pass the new comments onto the base
		CString comments;
		GetDlgItemText (IDC_COMMENTS_EDIT, comments);
		m_Preset->Set_Comments (comments);
	}
	
	// Return true to allow the dialog to close
	return retval;
}

