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

// GenerateBaseDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "GenerateBaseDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GenerateBaseDialogClass dialog


GenerateBaseDialogClass::GenerateBaseDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(GenerateBaseDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GenerateBaseDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void GenerateBaseDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GenerateBaseDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GenerateBaseDialogClass, CDialog)
	//{{AFX_MSG_MAP(GenerateBaseDialogClass)
	ON_EN_UPDATE(IDC_PRESET_NAME, OnUpdatePresetName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GenerateBaseDialogClass message handlers

void
GenerateBaseDialogClass::OnOK (void) 
{
	GetDlgItemText (IDC_PRESET_NAME, m_PresetName);
	CDialog::OnOK();
	return ;
}

void
GenerateBaseDialogClass::OnUpdatePresetName (void)
{
	// Enable/disable the OK button based on the entry in the name editfield
	BOOL benable = (::GetWindowTextLength (::GetDlgItem (m_hWnd, IDC_PRESET_NAME)) > 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDOK), benable);
	return ;
}
