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

// EditRemarkDialog.cpp : implementation file
//


#include "StdAfx.h"

#if 0

#include "LevelEdit.h"
#include "editremarkdialog.h"
#include "dialogue.h"
#include "translatedb.h"
#include "translateobj.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_TEXT		= 0
};


/////////////////////////////////////////////////////////////////////////////
//
// EditRemarkDialogClass
//
/////////////////////////////////////////////////////////////////////////////
EditRemarkDialogClass::EditRemarkDialogClass(CWnd* pParent /*=NULL*/)
	:	m_Remark (NULL),
		CDialog(EditRemarkDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditRemarkDialogClass)
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
EditRemarkDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditRemarkDialogClass)
	DDX_Control(pDX, IDC_WEIGHT_SPIN, m_WeightSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EditRemarkDialogClass, CDialog)
	//{{AFX_MSG_MAP(EditRemarkDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
EditRemarkDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();
	ASSERT (m_Remark != NULL);

	//
	//	Calculate the rectangle where we are to display the string picker
	//
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_PICKER_AREA), &rect);
	ScreenToClient (&rect);

	//
	//	Create the string picker
	//
	StringPicker.Set_Selection (m_Remark->Get_Text_ID ());
	StringPicker.Create (this);
	StringPicker.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
	StringPicker.ShowWindow (SW_SHOW);

	//
	//	Configure the weight controls
	//
	m_WeightSpin.SetRange (0, 100);
	m_WeightSpin.SetPos ((int)m_Remark->Get_Weight ());
	SetDlgItemInt (IDC_WEIGHT_EDIT, (int)m_Remark->Get_Weight ());	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
EditRemarkDialogClass::OnOK (void)
{
	//
	//	Read the silence configuration
	//
	int new_weight = GetDlgItemInt (IDC_WEIGHT_EDIT);
	m_Remark->Set_Weight (new_weight);

	//
	//	Get the selected string from the picker object
	//
	m_Remark->Set_Text_ID (StringPicker.Get_Selection ());

	CDialog::OnOK ();
	return ;
}

#endif //0