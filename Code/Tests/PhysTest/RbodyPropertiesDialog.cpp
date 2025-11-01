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

// RbodyPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "RbodyPropertiesDialog.h"
#include "movephys.h"
#include "rbody.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int MIN_MASS = 1;
const int MAX_MASS = 255;
const int MIN_ELASTICITY = 0;
const int MAX_ELASTICITY = 1;
const int MIN_GRAVITY = 0;
const int MAX_GRAVITY = 1;
const int MIN_POS = -255;
const int MAX_POS = 255;
const int MIN_CONTACT_STIFFNESS = 0;
const int MAX_CONTACT_STIFFNESS = 255;
const int MIN_CONTACT_DAMPING = 0;
const int MAX_CONTACT_DAMPING = 255;
const int MIN_CONTACT_LENGTH = 0;
const int MAX_CONTACT_LENGTH = 2;


/////////////////////////////////////////////////////////////////////////////
// CRbodyPropertiesDialog dialog


CRbodyPropertiesDialog::CRbodyPropertiesDialog(CWnd* pParent,MoveablePhysClass * object) :
	CDialog(CRbodyPropertiesDialog::IDD, pParent),
	Object(object)
{
	//{{AFX_DATA_INIT(CRbodyPropertiesDialog)
	//}}AFX_DATA_INIT
	ASSERT(Object != NULL);
}


void CRbodyPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRbodyPropertiesDialog)
	DDX_Control(pDX, IDC_LENGTH_SPIN, m_LengthSpin);
	DDX_Control(pDX, IDC_STIFFNESS_SPIN, m_StiffnessSpin);
	DDX_Control(pDX, IDC_DAMPING_SPIN, m_DampingSpin);
	DDX_Control(pDX, IDC_POSITIONZ_SPIN, m_PositionZSpin);
	DDX_Control(pDX, IDC_POSITIONY_SPIN, m_PositionYSpin);
	DDX_Control(pDX, IDC_POSITIONX_SPIN, m_PositionXSpin);
	DDX_Control(pDX, IDC_MASS_SPIN, m_MassSpin);
	DDX_Control(pDX, IDC_GRAVITY_SPIN, m_GravitySpin);
	DDX_Control(pDX, IDC_ELASTICITY_SPIN, m_ElasticitySpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRbodyPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(CRbodyPropertiesDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRbodyPropertiesDialog message handlers

void CRbodyPropertiesDialog::OnOK() 
{
	Object->Set_Mass(GetDlgItemFloat(IDC_MASS_EDIT));	
	Object->Set_Gravity_Multiplier(GetDlgItemFloat(IDC_GRAVITY_EDIT));
	Object->Set_Elasticity(GetDlgItemFloat(IDC_ELASTICITY_EDIT));
	Vector3 pos;
	pos.X = GetDlgItemFloat(IDC_POSITIONX_EDIT);
	pos.Y = GetDlgItemFloat(IDC_POSITIONY_EDIT);
	pos.Z = GetDlgItemFloat(IDC_POSITIONZ_EDIT);
	Object->Set_Position(pos);

	float length = GetDlgItemFloat(IDC_LENGTH_EDIT);
	
	// not letting the user override stiffness and damping for now
	((RigidBodyClass *)Object)->Set_Contact_Parameters(length);

	CDialog::OnOK();
}

BOOL CRbodyPropertiesDialog::OnInitDialog() 
{
	ASSERT(Object != NULL);
	CDialog::OnInitDialog();
	
	m_MassSpin.SetRange(MIN_MASS * 100,MAX_MASS * 100);
	m_GravitySpin.SetRange(MIN_GRAVITY * 100,MAX_GRAVITY * 100);
	m_ElasticitySpin.SetRange(MIN_ELASTICITY * 100,MAX_ELASTICITY * 100);
	m_PositionXSpin.SetRange(MIN_POS * 100,MAX_POS * 100);
	m_PositionYSpin.SetRange(MIN_POS * 100,MAX_POS * 100);
	m_PositionZSpin.SetRange(MIN_POS * 100,MAX_POS * 100);
	m_StiffnessSpin.SetRange(MIN_CONTACT_STIFFNESS * 100,MAX_CONTACT_STIFFNESS * 100);
	m_DampingSpin.SetRange(MIN_CONTACT_DAMPING * 100,MAX_CONTACT_DAMPING * 100);
	m_LengthSpin.SetRange(MIN_CONTACT_LENGTH * 100,MAX_CONTACT_LENGTH * 100);

	m_MassSpin.SetPos(Object->Get_Mass() * 100);
	m_GravitySpin.SetPos(Object->Get_Gravity_Multiplier() * 100);
	m_ElasticitySpin.SetPos(Object->Get_Elasticity() * 100);

	Vector3 position = Object->Get_Transform().Get_Translation();
	m_PositionXSpin.SetPos(position.X * 100);
	m_PositionYSpin.SetPos(position.Y * 100);
	m_PositionZSpin.SetPos(position.Z * 100);

	float stiffness,damping,length;
	((RigidBodyClass *)Object)->Get_Contact_Parameters(&stiffness,&damping,&length);
	m_StiffnessSpin.SetPos(stiffness * 100);
	m_DampingSpin.SetPos(damping * 100);
	m_LengthSpin.SetPos(length * 100);

	SetDlgItemFloat(IDC_MASS_EDIT,Object->Get_Mass());	
	SetDlgItemFloat(IDC_GRAVITY_EDIT,Object->Get_Gravity_Multiplier());
	SetDlgItemFloat(IDC_ELASTICITY_EDIT,Object->Get_Elasticity());
	SetDlgItemFloat(IDC_POSITIONX_EDIT,position.X);
	SetDlgItemFloat(IDC_POSITIONY_EDIT,position.Y);
	SetDlgItemFloat(IDC_POSITIONZ_EDIT,position.Z);
	SetDlgItemFloat(IDC_STIFFNESS_EDIT,stiffness);
	SetDlgItemFloat(IDC_DAMPING_EDIT,damping);
	SetDlgItemFloat(IDC_LENGTH_EDIT,length);

	return true; 
}

float CRbodyPropertiesDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CRbodyPropertiesDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}


BOOL CRbodyPropertiesDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_MASS_SPIN:
		case IDC_GRAVITY_SPIN:
		case IDC_ELASTICITY_SPIN:
		case IDC_POSITIONX_SPIN:
		case IDC_POSITIONY_SPIN:
		case IDC_POSITIONZ_SPIN:
		case IDC_STIFFNESS_SPIN:
		case IDC_DAMPING_SPIN:
		case IDC_LENGTH_SPIN:
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
