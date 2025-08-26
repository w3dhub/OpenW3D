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

// PositionPage.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "positionpage.h"
#include "node.h"
#include "Utils.h"
#include "sceneeditor.h"
#include "mover.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// PositionPageClass
//
/////////////////////////////////////////////////////////////////////////////
PositionPageClass::PositionPageClass (void)
	: m_pNode (NULL),
	  m_bInclueRotation (false),
	  DockableFormClass (PositionPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// PositionPageClass
//
/////////////////////////////////////////////////////////////////////////////
PositionPageClass::PositionPageClass (NodeClass *node)
	: m_pNode (node),
	  m_bInclueRotation (false),
	  DockableFormClass (PositionPageClass::IDD)
{
	//{{AFX_DATA_INIT(PositionPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~PositionPageClass
//
/////////////////////////////////////////////////////////////////////////////
PositionPageClass::~PositionPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PositionPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PositionPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(PositionPageClass)
	ON_NOTIFY(UDN_DELTAPOS, IDC_XPOS_SPIN, OnDeltaPosXPosSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_XROT_SPIN, OnDeltaPosXRotSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_YPOS_SPIN, OnDeltaPosYPosSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_YROT_SPIN, OnDeltaPosYRotSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ZPOS_SPIN, OnDeltaPosZPosSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ZROT_SPIN, OnDeltaPosZRotSpin)
	ON_EN_CHANGE(IDC_ZROT_EDIT, OnChangeZRotEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PositionPageClass diagnostics

#ifdef _DEBUG
void PositionPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void PositionPageClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::HandleInitDialog (void)
{
	ASSERT (m_pNode != NULL);

	// Get the node's position
	Matrix3D transform = m_pNode->Get_Transform ();
	Vector3 translation = transform.Get_Translation ();

	// Fill the position controls
	::SetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT, translation.X);
	::SetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT, translation.Y);
	::SetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT, translation.Z);

	// Fill the rotation controls
	::SetDlgItemFloat (m_hWnd, IDC_XROT_EDIT, RAD_TO_DEG (transform.Get_X_Rotation ()));
	::SetDlgItemFloat (m_hWnd, IDC_YROT_EDIT, RAD_TO_DEG (transform.Get_Y_Rotation ()));
	::SetDlgItemFloat (m_hWnd, IDC_ZROT_EDIT, RAD_TO_DEG (transform.Get_Z_Rotation ()));

	// Enable/disable the 'restrict rotation' checkbox, and set its check state
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_RESTRICT_CHECK), m_pNode->Can_Be_Rotated_Freely ());
	SendDlgItemMessage (IDC_RESTRICT_CHECK, BM_SETCHECK, (WPARAM)m_pNode->Is_Rotation_Restricted ());

	// Set the ranges for the spin controls
	SendDlgItemMessage (IDC_XPOS_SPIN, UDM_SETRANGE, (WPARAM)0, MAKELPARAM (UD_MAXVAL, UD_MINVAL));
	SendDlgItemMessage (IDC_YPOS_SPIN, UDM_SETRANGE, (WPARAM)0, MAKELPARAM (UD_MAXVAL, UD_MINVAL));
	SendDlgItemMessage (IDC_ZPOS_SPIN, UDM_SETRANGE, (WPARAM)0, MAKELPARAM (UD_MAXVAL, UD_MINVAL));
	SendDlgItemMessage (IDC_ZROT_SPIN, UDM_SETRANGE, (WPARAM)0, MAKELPARAM (UD_MAXVAL, UD_MINVAL));
	
	// Assume we aren't messing with the rotation
	m_bInclueRotation = false;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
PositionPageClass::Apply_Changes (void)
{
	Matrix3D transform (1);
	Vector3 translation (0, 0, 0);

	// Get the rotation angle
	if (m_bInclueRotation) {
		float zrot = ::GetDlgItemFloat (m_hWnd, IDC_ZROT_EDIT);
		transform.Rotate_Z (DEG_TO_RAD (zrot));
	} else {
		transform = m_pNode->Get_Transform ();
	}

	// Pass the translation onto the transform
	translation.X = ::GetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT);
	translation.Y = ::GetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT);
	translation.Z = ::GetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT);
	transform.Set_Translation (translation);

	// Pass the newly constructed transform directly to the node
	MoverClass::Transform_Node (m_pNode, transform);
	
	// Pass the rotation restriction onto the node (if necessary)
	if (m_pNode->Can_Be_Rotated_Freely ()) {
		m_pNode->Restrict_Rotation (bool(SendDlgItemMessage (IDC_RESTRICT_CHECK, BM_GETCHECK) == 1));
	}

	// Return true to allow the dialog to close
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosXPosSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosXPosSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	// Get the current value, increment it, and put it back into the control
	float xpos = ::GetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT);
	xpos += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_XPOS_EDIT, xpos);
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosXRotSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosXRotSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// Get the current value, increment it, and put it back into the control
	float xrot = ::GetDlgItemFloat (m_hWnd, IDC_XROT_EDIT);
	xrot += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_XROT_EDIT, xrot);
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosYPosSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosYPosSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// Get the current value, increment it, and put it back into the control
	float ypos = ::GetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT);
	ypos += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_YPOS_EDIT, ypos);
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosYRotSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosYRotSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// Get the current value, increment it, and put it back into the control
	float yrot = ::GetDlgItemFloat (m_hWnd, IDC_YROT_EDIT);
	yrot += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_YROT_EDIT, yrot);
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosZPosSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosZPosSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// Get the current value, increment it, and put it back into the control
	float zpos = ::GetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT);
	zpos += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_ZPOS_EDIT, zpos);
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDeltaPosZRotSpin
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnDeltaPosZRotSpin
(
	NMHDR* pNMHDR,
	LRESULT* pResult
) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// Get the current value, increment it, and put it back into the control
	float zrot = ::GetDlgItemFloat (m_hWnd, IDC_ZROT_EDIT);
	zrot += (((float)pNMUpDown->iDelta) / 100.0F);
	::SetDlgItemFloat (m_hWnd, IDC_ZROT_EDIT, zrot);

	// Modify the transform's rotation on exit
	m_bInclueRotation = true;
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeZRotEdit
//
/////////////////////////////////////////////////////////////////////////////
void
PositionPageClass::OnChangeZRotEdit (void)
{
	// Modify the transform's rotation on exit
	m_bInclueRotation = true;	
	return ;
}

