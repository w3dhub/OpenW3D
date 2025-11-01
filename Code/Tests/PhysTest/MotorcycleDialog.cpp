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

// MotorcycleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "MotorcycleDialog.h"
#include "motorcycle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const float MIN_K = 0.01f;
const float MAX_K = 100.0f;


/////////////////////////////////////////////////////////////////////////////
// CMotorcycleDialog dialog


CMotorcycleDialog::CMotorcycleDialog(CWnd* pParent,MotorcycleClass * obj)
	: CDialog(CMotorcycleDialog::IDD, pParent),
	EditedObject(obj)
{
	//{{AFX_DATA_INIT(CMotorcycleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMotorcycleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotorcycleDialog)
	DDX_Control(pDX, IDC_LEAN_K1_SPIN, m_LeanK1Spin);
	DDX_Control(pDX, IDC_LEAN_K0_SPIN, m_LeanK0Spin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotorcycleDialog, CDialog)
	//{{AFX_MSG_MAP(CMotorcycleDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotorcycleDialog message handlers

float CMotorcycleDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CMotorcycleDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}

BOOL CMotorcycleDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_LeanK0Spin.SetRange(MIN_K * 100,MAX_K * 100);
	m_LeanK1Spin.SetRange(MIN_K * 100,MAX_K * 100);

	float k;
	k = EditedObject->LeanK0;
	m_LeanK0Spin.SetPos(k * 100);
	SetDlgItemFloat(IDC_LEAN_K0_EDIT,k);
	
	k = EditedObject->LeanK1;
	m_LeanK1Spin.SetPos(k * 100);
	SetDlgItemFloat(IDC_LEAN_K1_EDIT,k);

	return true;
}

BOOL CMotorcycleDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_LEAN_K0_SPIN:
		case IDC_LEAN_K1_SPIN:
			LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;
			if (lpnmud->hdr.code == UDN_DELTAPOS) {
				HWND hwnd = (HWND)SendDlgItemMessage(LOWORD(wParam),UDM_GETBUDDY);
				float curval = GetDlgItemFloat(GetWindowLong(hwnd,GWL_ID));
				curval += (float)lpnmud->iDelta / 100.0f;
				SetDlgItemFloat(GetWindowLong(hwnd,GWL_ID), curval);
			}
			break;
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CMotorcycleDialog::OnOK() 
{
	float k;
	k = GetDlgItemFloat(IDC_LEAN_K0_EDIT);
	EditedObject->LeanK0 = k;
	k = GetDlgItemFloat(IDC_LEAN_K1_EDIT);
	EditedObject->LeanK1 = k;

	CDialog::OnOK();
}
