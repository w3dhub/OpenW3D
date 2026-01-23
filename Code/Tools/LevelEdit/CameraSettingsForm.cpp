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

// ameraSettingsFormClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "CameraSettingsForm.H"
#include "Utils.h"
#include "CameraMgr.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CameraSettingsFormClass
//
CameraSettingsFormClass::CameraSettingsFormClass (void)
	: DockableFormClass(CameraSettingsFormClass::IDD)
{
	//{{AFX_DATA_INIT(CameraSettingsFormClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~CameraSettingsFormClass
//
CameraSettingsFormClass::~CameraSettingsFormClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
CameraSettingsFormClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CameraSettingsFormClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(CameraSettingsFormClass, DockableFormClass)
	//{{AFX_MSG_MAP(CameraSettingsFormClass)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DEPTH_SPIN, OnDeltaPosDepthSpin)
	ON_EN_UPDATE(IDC_DEPTH_EDIT, OnUpdateDepthEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CameraSettingsFormClass diagnostics

#ifdef _DEBUG
void CameraSettingsFormClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void CameraSettingsFormClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
void
CameraSettingsFormClass::HandleInitDialog (void)
{
	// Set the depth range
	SendDlgItemMessage (IDC_DEPTH_SPIN, UDM_SETRANGE, 0, MAKELPARAM (4000, 20));

	// Put the current camera information into the controls
	Update_Controls ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Controls
//
void
CameraSettingsFormClass::Update_Controls (void)
{
	CameraMgr *pcamera_mgr = ::Get_Camera_Mgr ();
	if (pcamera_mgr != NULL) {		
		const Matrix3D &transform = pcamera_mgr->Get_Camera ()->Get_Transform ();
		CString temp_string;
		
		// Put the x position of the camera into the controls
		temp_string.Format ("%.2f", transform.Get_X_Translation ());
		SetDlgItemText (IDC_POSX_EDIT, temp_string);

		// Put the y position of the camera into the controls
		temp_string.Format ("%.2f", transform.Get_Y_Translation ());
		SetDlgItemText (IDC_POSY_EDIT, temp_string);

		// Put the z position of the camera into the controls
		temp_string.Format ("%.2f", transform.Get_Z_Translation ());
		SetDlgItemText (IDC_POSZ_EDIT, temp_string);

		// Put the x rotation of the camera into the controls
		temp_string.Format ("%.2f", (float)RAD_TO_DEG (transform.Get_X_Rotation ()));
		SetDlgItemText (IDC_ROTATEX_EDIT, temp_string);

		// Put the y rotation of the camera into the controls
		temp_string.Format ("%.2f", (float)RAD_TO_DEG (transform.Get_Y_Rotation ()));
		SetDlgItemText (IDC_ROTATEY_EDIT, temp_string);

		// Put the z rotation of the camera into the controls
		temp_string.Format ("%.2f", (float)RAD_TO_DEG (transform.Get_Z_Rotation ()));
		SetDlgItemText (IDC_ROTATEZ_EDIT, temp_string);

		// Put the current depth of the camera into its dialog control
		float near_plane = 0;
		float far_plane = 0;
		pcamera_mgr->Get_Camera ()->Get_Clip_Planes (near_plane, far_plane);
		SetDlgItemInt (IDC_DEPTH_EDIT, (int)far_plane);
		SendDlgItemMessage (IDC_DEPTH_SPIN, 0, MAKELPARAM ((short)far_plane, 0));
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosDepthSpin
//
void
CameraSettingsFormClass::OnDeltaPosDepthSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown) {
		Set_Depth (pNMUpDown->iPos);
	}
	
	*pResult = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnUpdateDepthEdit
//
void
CameraSettingsFormClass::OnUpdateDepthEdit (void)
{
	Set_Depth (GetDlgItemInt (IDC_DEPTH_EDIT));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Depth
//
void
CameraSettingsFormClass::Set_Depth (int /* new_depth */)
{

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
CameraSettingsFormClass::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
	// Is this the message we are expecting?
	if ((message == WM_SHOWWINDOW) || (message == WM_ACTIVATE)) {
		
		// Make sure the controls reflect the current state when we are
		// shown
		if ((BOOL)LOWORD (wParam)) {
			Update_Controls ();
		}
	} 

	// Allow the base class to process this message	
	return DockableFormClass::WindowProc(message, wParam, lParam);
}

