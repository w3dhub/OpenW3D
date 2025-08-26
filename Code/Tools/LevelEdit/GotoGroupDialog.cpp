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

// GotoGroupDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "GotoGroupDialog.h"
#include "Utils.h"
#include "GroupMgr.H"
#include "CameraMgr.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// GotoGroupDialogClass
//
GotoGroupDialogClass::GotoGroupDialogClass
(
	GroupMgrClass *pgroup,
	CWnd *pParent
)
	: m_pGroup (pgroup),
	  CDialog(GotoGroupDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GotoGroupDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
GotoGroupDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GotoGroupDialogClass)
	DDX_Control(pDX, IDC_GROUP_LIST, m_GroupList);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GotoGroupDialogClass, CDialog)
	//{{AFX_MSG_MAP(GotoGroupDialogClass)
	ON_CBN_EDITCHANGE(IDC_GROUP_LIST, OnEditChangeGroupList)
	ON_CBN_SELCHANGE(IDC_GROUP_LIST, OnSelChangeGroupList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
BOOL
GotoGroupDialogClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	
	// Fill the 'group list' combobox with a complete list of all the group in the level.	
	::Fill_Group_Combo (::GetDlgItem (m_hWnd, IDC_OBJECT_LIST), m_pGroup);	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
void
GotoGroupDialogClass::OnOK (void)
{
	// Get the currently selected node, and pass it onto the camera manager
	int index = Get_Current_Selection ();
	if (index != -1) {
		GroupMgrClass *pgroup = (GroupMgrClass *)m_GroupList.GetItemData (index);
		if (pgroup != NULL) {
			::Get_Camera_Mgr ()->Goto_Group (pgroup);
		}
	}
	
	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Current_Selection
//
int
GotoGroupDialogClass::Get_Current_Selection (void)
{
	// Assume failure
	int index = -1;

	CString text;
	m_GroupList.GetWindowText (text);
	index = m_GroupList.FindStringExact (-1, text);

	// Return the index of the currently selected group
	return index;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnEditChangeGroupList
//
void
GotoGroupDialogClass::OnEditChangeGroupList (void)
{
	// Enable/disable the OK button based on the validity of the user's entry
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), BOOL(Get_Current_Selection () != -1));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelChangeGroupList
//
void
GotoGroupDialogClass::OnSelChangeGroupList (void)
{
	// Enable the OK button
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), TRUE);
	return ;
}

