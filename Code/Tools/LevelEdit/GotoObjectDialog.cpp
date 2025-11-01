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

// GotoObjectDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "gotoobjectdialog.h"
#include "Utils.h"
#include "node.h"
#include "cameramgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// GotoObjectDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GotoObjectDialogClass::GotoObjectDialogClass
(
	NodeClass *	node,
	CWnd *		parent
)
	: m_pNode (node),
	  CDialog(GotoObjectDialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(GotoObjectDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
GotoObjectDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GotoObjectDialogClass)
	DDX_Control(pDX, IDC_OBJECT_LIST, m_NodeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GotoObjectDialogClass, CDialog)
	//{{AFX_MSG_MAP(GotoObjectDialogClass)
	ON_CBN_EDITCHANGE(IDC_OBJECT_LIST, OnEditChangeObjectList)
	ON_CBN_SELCHANGE(IDC_OBJECT_LIST, OnSelChangeObjectList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GotoObjectDialogClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	
	// Fill the 'object list' combobox with a complete list of all the objects in the level.	
	::Fill_Node_Instance_Combo (::GetDlgItem (m_hWnd, IDC_OBJECT_LIST), m_pNode);	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
GotoObjectDialogClass::OnOK (void)
{
	// Get the currently selected node, and pass it onto the camera manager
	int index = Get_Current_Selection ();
	if (index != -1) {
		NodeClass *node = (NodeClass *)m_NodeList.GetItemData (index);
		if (node != NULL) {
			::Get_Camera_Mgr ()->Goto_Node (node);
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
/////////////////////////////////////////////////////////////////////////////
int
GotoObjectDialogClass::Get_Current_Selection (void)
{
	// Assume failure
	int index = -1;

	CString text;
	m_NodeList.GetWindowText (text);
	index = m_NodeList.FindStringExact (-1, text);

	// Return the index of the currently selected node
	return index;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnEditChangeObjectList
//
/////////////////////////////////////////////////////////////////////////////
void
GotoObjectDialogClass::OnEditChangeObjectList (void)
{
	// Enable/disable the OK button based on the validity of the user's entry
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), BOOL(Get_Current_Selection () != -1));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelChangeObjectList
//
/////////////////////////////////////////////////////////////////////////////
void
GotoObjectDialogClass::OnSelChangeObjectList (void)
{
	// Enable the OK button
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), true);
	return ;
}

