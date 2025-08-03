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

// GotoLocationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "GotoLocationDialog.h"
#include "CameraMgr.h"
#include "Utils.h"
#include "euler.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// GotoLocationDialogClass
//
/////////////////////////////////////////////////////////////////////////////
GotoLocationDialogClass::GotoLocationDialogClass (CWnd* pParent /*=NULL*/)
	: CDialog(GotoLocationDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GotoLocationDialogClass)
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
GotoLocationDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GotoLocationDialogClass)
	DDX_Control(pDX, IDC_ZPOS_SPIN, m_ZPosSpin);
	DDX_Control(pDX, IDC_YPOS_SPIN, m_YPosSpin);
	DDX_Control(pDX, IDC_XPOS_SPIN, m_XPosSpin);
	DDX_Control(pDX, IDC_FACING_SPIN, m_FacingSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(GotoLocationDialogClass, CDialog)
	//{{AFX_MSG_MAP(GotoLocationDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
GotoLocationDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Turn all the edit controls into 'float' controls that will
	// allow the user to type only numbers (with decimals and signs)
	//
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_XPOS_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_YPOS_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_ZPOS_EDIT));
	::Make_Edit_Float_Ctrl (::GetDlgItem (m_hWnd, IDC_FACING_EDIT));
	
	//
	//	Set the float control ranges
	//
	m_XPosSpin.SetRange32 (-1000000, 1000000);
	m_YPosSpin.SetRange32 (-1000000, 1000000);
	m_ZPosSpin.SetRange32 (-1000000, 1000000);
	m_FacingSpin.SetRange32 (-1000000, 1000000);	

	//
	//	Get the camera's position and rotation
	//
	CameraClass *camera	= ::Get_Camera_Mgr ()->Get_Camera ();
	Vector3 position		= camera->Get_Position ();

	Matrix3D tm				= camera->Get_Transform ();
	Vector3 look_at_pos	= tm.Get_Z_Vector ();
	float facing			= WWMath::Wrap ((float)::atan2 (look_at_pos.Y, look_at_pos.X), 0.0F, DEG_TO_RADF (360));

	//
	//	Put the camera's current position and rotation into the
	// dialog controls.
	//
	::SetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT, position.X);
	::SetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT, position.Y);
	::SetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT, position.Z);
	::SetDlgItemFloat (m_hWnd, IDC_FACING_EDIT, RAD_TO_DEG (facing));
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
GotoLocationDialogClass::OnOK (void)
{
	//
	//	Get the new settings for the camera from the dialog
	//
	Vector3 pos (0, 0, 0);
	pos.X = ::GetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT, true);
	pos.Y = ::GetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT, true);
	pos.Z = ::GetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT, true);

	float facing = ::GetDlgItemFloat (m_hWnd, IDC_FACING_EDIT, true);

	//
	//	Create a transform for the camera using these new settings
	//
	float x_val = ::cos (DEG_TO_RADF (facing));
	float y_val = ::sin (DEG_TO_RADF (facing));
	Vector3 y_axis = Vector3 (0, 0, 1);
	Vector3 z_axis = Vector3 (x_val, y_val, 0);
	Vector3 x_axis = Vector3::Cross_Product (y_axis, z_axis);
	Matrix3D tm (x_axis, y_axis, z_axis, pos);

	//
	//	Pass the new transform onto the camera
	//
	::Get_Camera_Mgr ()->Set_Transform (tm);

	CDialog::OnOK ();
	return ;
}

