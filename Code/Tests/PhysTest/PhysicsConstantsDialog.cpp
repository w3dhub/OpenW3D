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

// PhysicsConstantsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "PhysicsConstantsDialog.h"
#include "physcon.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int MIN_DAMPING = 0;
const int MAX_DAMPING = 255;
const int MIN_GRAVITY = -100;
const int MAX_GRAVITY = 0;


/////////////////////////////////////////////////////////////////////////////
// CPhysicsConstantsDialog dialog


CPhysicsConstantsDialog::CPhysicsConstantsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPhysicsConstantsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhysicsConstantsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPhysicsConstantsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhysicsConstantsDialog)
	DDX_Control(pDX, IDC_LDAMPING_SPIN, m_LDampingSpin);
	DDX_Control(pDX, IDC_GRAVITYACCEL_SPIN, m_GravityAccelSpin);
	DDX_Control(pDX, IDC_ADAMPING_SPIN, m_ADampingSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPhysicsConstantsDialog, CDialog)
	//{{AFX_MSG_MAP(CPhysicsConstantsDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhysicsConstantsDialog message handlers

BOOL CPhysicsConstantsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_LDampingSpin.SetRange(MIN_DAMPING * 100,MAX_DAMPING * 100);
	m_ADampingSpin.SetRange(MIN_DAMPING * 100,MAX_DAMPING * 100);
	m_GravityAccelSpin.SetRange(MIN_GRAVITY * 100,MAX_GRAVITY * 100);
	
	m_LDampingSpin.SetPos(PhysicsConstants::LinearDamping * 100);
	m_LDampingSpin.SetPos(PhysicsConstants::LinearDamping * 100);
	m_GravityAccelSpin.SetPos(PhysicsConstants::GravityAcceleration.Z * 100);

	SetDlgItemFloat(IDC_LDAMPING_EDIT,PhysicsConstants::LinearDamping);
	SetDlgItemFloat(IDC_ADAMPING_EDIT,PhysicsConstants::AngularDamping);
	SetDlgItemFloat(IDC_GRAVITYACCEL_EDIT,PhysicsConstants::GravityAcceleration.Z);

	return true;	
}

BOOL CPhysicsConstantsDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// make the spin controls work...
	switch(wParam) 
	{
		case IDC_LDAMPING_SPIN:
		case IDC_ADAMPING_SPIN:
		case IDC_GRAVITYACCEL_SPIN:
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

void CPhysicsConstantsDialog::OnOK() 
{
	PhysicsConstants::LinearDamping = GetDlgItemFloat(IDC_LDAMPING_EDIT);
	PhysicsConstants::AngularDamping = GetDlgItemFloat(IDC_ADAMPING_EDIT);
	PhysicsConstants::GravityAcceleration.Z = GetDlgItemFloat(IDC_GRAVITYACCEL_EDIT);
	
	CDialog::OnOK();
}


float CPhysicsConstantsDialog::GetDlgItemFloat(int controlid)
{
	CString string;
	GetDlgItemText(controlid,string);
	return atof(string);
}

void CPhysicsConstantsDialog::SetDlgItemFloat(int controlid,float val)
{
	CString string;
	string.Format("%.2f",val);
	SetDlgItemText(controlid,string);
}
