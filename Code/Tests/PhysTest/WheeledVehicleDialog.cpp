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

// WheeledVehicleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "WheeledVehicleDialog.h"
#include "wheelvehicle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const float MIN_KS = 0.01f;
const float MAX_KS = 1000.0f;
const float MIN_KD = 0.01f;
const float MAX_KD = 1000.0f;
const float MIN_LENGTH = 0.1f;
const float MAX_LENGTH = 32.0f;


/////////////////////////////////////////////////////////////////////////////
// CWheeledVehicleDialog dialog


CWheeledVehicleDialog::CWheeledVehicleDialog(CWnd* pParent,WheeledVehicleClass * obj)
	: CDialog(CWheeledVehicleDialog::IDD, pParent),
	EditedObject(obj)
{
	//{{AFX_DATA_INIT(CWheeledVehicleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWheeledVehicleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWheeledVehicleDialog)
	DDX_Control(pDX, IDC_WVEHICLE_LENGTH_SPIN, m_LengthSpin);
	DDX_Control(pDX, IDC_WVEHICLE_KS_SPIN, m_KSSpin);
	DDX_Control(pDX, IDC_WVEHICLE_KD_SPIN, m_KDSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWheeledVehicleDialog, CDialog)
	//{{AFX_MSG_MAP(CWheeledVehicleDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWheeledVehicleDialog message handlers

BOOL CWheeledVehicleDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_KSSpin.SetRange(MIN_KS * 100,MAX_KS * 100);
	m_KDSpin.SetRange(MIN_KD * 100,MAX_KD * 100);
	m_LengthSpin.SetRange(MIN_LENGTH * 100,MAX_LENGTH * 100);
	
	float ks,kd,len;
	EditedObject->Get_Suspension_Parameters(&ks,&kd,&len);
	m_KSSpin.SetPos(ks * 100);
	m_KDSpin.SetPos(kd * 100);
	m_LengthSpin.SetPos(len * 100);

	SetDlgItemFloat(IDC_WVEHICLE_KS_EDIT,ks);
	SetDlgItemFloat(IDC_WVEHICLE_KD_EDIT,kd);
	SetDlgItemFloat(IDC_WVEHICLE_LENGTH_EDIT,len);

	return true; 
}

BOOL CWheeledVehicleDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_WVEHICLE_KS_SPIN:
		case IDC_WVEHICLE_KD_SPIN:
		case IDC_WVEHICLE_LENGTH_SPIN:
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


void CWheeledVehicleDialog::OnOK() 
{
	float ks,kd,len;
	ks = GetDlgItemFloat(IDC_WVEHICLE_KS_EDIT);
	kd = GetDlgItemFloat(IDC_WVEHICLE_KD_EDIT);
	len = GetDlgItemFloat(IDC_WVEHICLE_LENGTH_EDIT);
	EditedObject->Set_Suspension_Parameters(ks,kd,len);
	CDialog::OnOK();
}



float CWheeledVehicleDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CWheeledVehicleDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}



