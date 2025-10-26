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
 *                 Project Name : LightMap                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tool $* 
 *                                                                                             * 
 *                      $Author:: Ian_l               $* 
 *                                                                                             * 
 *                     $Modtime:: 9/06/00 5:06p       $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "SelectionDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


SelectionDialog::SelectionDialog (DynamicVectorClass <char*> *listptr, CWnd* pParent /*=NULL*/)
	: CDialog(SelectionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectionDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	ListPtr	 = listptr;
	Selection = -1;		// NOTE: Indicates no current selection.
}

void SelectionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectionDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectionDialog, CDialog)
	//{{AFX_MSG_MAP(SelectionDialog)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectionDialog message handlers
BOOL SelectionDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (ListPtr != NULL) {

		CListBox *listboxptr = (CListBox*) GetDlgItem (IDC_SELECTION_LIST);
	
		for (int i = 0; i < ListPtr->Count(); i++) {
			listboxptr->AddString ((*ListPtr) [i]);
		}
	
		// Set the initial selection.
		listboxptr->SetCurSel (0);
	}

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


void SelectionDialog::OnDestroy() 
{
	CDialog::OnDestroy();

	CListBox *listboxptr = (CListBox*) GetDlgItem (IDC_SELECTION_LIST);
	
	// Record the most recent list box selection.
	Selection = listboxptr->GetCurSel();
}
