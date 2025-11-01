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

// InertiaDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "InertiaDialog.h"
#include "rbody.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int MIN_IBODY = 0;
const int MAX_IBODY = 255;


/////////////////////////////////////////////////////////////////////////////
// CInertiaDialog dialog

CInertiaDialog::CInertiaDialog(CWnd* pParent,RigidBodyClass * obj) :
	CDialog(CInertiaDialog::IDD, pParent),
	Object(obj)
{
	//{{AFX_DATA_INIT(CInertiaDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	ASSERT(Object != NULL);
}


void CInertiaDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInertiaDialog)
	DDX_Control(pDX, IDC_IBODYZ_SPIN, m_IBodyZSpin);
	DDX_Control(pDX, IDC_IBODYY_SPIN, m_IBodyYSpin);
	DDX_Control(pDX, IDC_IBODYX_SPIN, m_IBodyXSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInertiaDialog, CDialog)
	//{{AFX_MSG_MAP(CInertiaDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInertiaDialog message handlers

BOOL CInertiaDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_IBodyXSpin.SetRange(MIN_IBODY * 100,MAX_IBODY * 100);
	m_IBodyYSpin.SetRange(MIN_IBODY * 100,MAX_IBODY * 100);
	m_IBodyZSpin.SetRange(MIN_IBODY * 100,MAX_IBODY * 100);
	
	Matrix3 ibody;
	Object->Get_Inertia(&ibody);
	m_IBodyXSpin.SetPos(ibody[0][0] * 100);
	m_IBodyYSpin.SetPos(ibody[1][1] * 100);
	m_IBodyZSpin.SetPos(ibody[2][2] * 100);

	SetDlgItemFloat(IDC_IBODYX_EDIT,ibody[0][0]);
	SetDlgItemFloat(IDC_IBODYY_EDIT,ibody[1][1]);
	SetDlgItemFloat(IDC_IBODYZ_EDIT,ibody[2][2]);

	return true; 
}


BOOL CInertiaDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_IBODYX_SPIN:
		case IDC_IBODYY_SPIN:
		case IDC_IBODYZ_SPIN:
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


void CInertiaDialog::OnOK() 
{
	Matrix3 ibody(1);
	ibody[0][0] = GetDlgItemFloat(IDC_IBODYX_EDIT);
	ibody[1][1] = GetDlgItemFloat(IDC_IBODYY_EDIT);
	ibody[2][2] = GetDlgItemFloat(IDC_IBODYZ_EDIT);
	Object->Set_Inertia(ibody);
	
	CDialog::OnOK();
}


float CInertiaDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CInertiaDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}
