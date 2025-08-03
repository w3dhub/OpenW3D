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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PhysObjectEditDialog.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/18/00 4:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "PhysObjectEditDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// PhysObjectEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
PhysObjectEditDialogClass::PhysObjectEditDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(PhysObjectEditDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(PhysObjectEditDialogClass)
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
PhysObjectEditDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PhysObjectEditDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PhysObjectEditDialogClass, CDialog)
	//{{AFX_MSG_MAP(PhysObjectEditDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PhysObjectEditDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_EDIT_AREA), &rect);
	ScreenToClient (&rect);
	::DestroyWindow (::GetDlgItem (m_hWnd, IDC_EDIT_AREA));

	//
	//	Setup the form we use to edit the physics object
	//
	m_ObjectEditForm.Create (this, 101);
	m_ObjectEditForm.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), 0);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
PhysObjectEditDialogClass::OnOK (void) 
{
	if (m_ObjectEditForm.Apply_Changes ()) {
		CDialog::OnOK ();
	}

	return ;
}
