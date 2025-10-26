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

// MotorVehicleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "MotorVehicleDialog.h"
#include "motorvehicle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const float MIN_TORQUE = 1.0f;
const float MAX_TORQUE = 10000.0f;


/////////////////////////////////////////////////////////////////////////////
// CMotorVehicleDialog dialog


CMotorVehicleDialog::CMotorVehicleDialog(CWnd* pParent,MotorVehicleClass * obj)
	: CDialog(CMotorVehicleDialog::IDD, pParent),
	EditedObject(obj)
{
	//{{AFX_DATA_INIT(CMotorVehicleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMotorVehicleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotorVehicleDialog)
	DDX_Control(pDX, IDC_MVEHICLE_TORQUE_SPIN, m_TorqueSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotorVehicleDialog, CDialog)
	//{{AFX_MSG_MAP(CMotorVehicleDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotorVehicleDialog message handlers

float CMotorVehicleDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CMotorVehicleDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}

BOOL CMotorVehicleDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_TorqueSpin.SetRange(MIN_TORQUE * 100,MAX_TORQUE * 100);

	float mt = EditedObject->Get_Max_Engine_Torque();
	m_TorqueSpin.SetPos(mt * 100);
	SetDlgItemFloat(IDC_MVEHICLE_TORQUE_EDIT,mt);
	return true;
}

BOOL CMotorVehicleDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_MVEHICLE_TORQUE_SPIN:
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


void CMotorVehicleDialog::OnOK() 
{
	float mt = GetDlgItemFloat(IDC_MVEHICLE_TORQUE_EDIT);
	EditedObject->Set_Max_Engine_Torque(mt);
	CDialog::OnOK();
}
