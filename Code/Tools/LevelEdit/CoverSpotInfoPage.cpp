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

// CoverSpotInfoPage.cpp : implementation file
//

#include "StdAfx.h"
#include "CoverSpotInfoPage.h"
#include "LevelEdit.h"
#include "coverspotnode.h"
#include "node.h"
#include "utils.h"
#include "sceneeditor.h"
#include "mousemgr.h"
#include "coverattackpointnode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CoverSpotInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
CoverSpotInfoPageClass::CoverSpotInfoPageClass (void)
	:	m_CoverSpot (NULL),
		DockableFormClass (CoverSpotInfoPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// CoverSpotInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
CoverSpotInfoPageClass::CoverSpotInfoPageClass (CoverSpotNodeClass *cover_spot)
	:	m_CoverSpot (cover_spot),
		DockableFormClass (CoverSpotInfoPageClass::IDD)
{
	//{{AFX_DATA_INIT(CoverSpotInfoPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~CoverSpotInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
CoverSpotInfoPageClass::~CoverSpotInfoPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
CoverSpotInfoPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(CoverSpotInfoPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(CoverSpotInfoPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(CoverSpotInfoPageClass)
	ON_BN_CLICKED(IDC_ADD_ATTACK_LOCATION, OnAddAttackLocation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CoverSpotInfoPageClass diagnostics

#ifdef _DEBUG
void CoverSpotInfoPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void CoverSpotInfoPageClass::Dump(CDumpContext& dc) const
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
CoverSpotInfoPageClass::HandleInitDialog (void)
{
	ASSERT (m_CoverSpot != NULL);

	SendDlgItemMessage (IDC_CROUCH_CHECK, BM_SETCHECK, m_CoverSpot->Requires_Crouch ());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
CoverSpotInfoPageClass::Apply_Changes (void)
{
	BOOL crouch = SendDlgItemMessage (IDC_CROUCH_CHECK, BM_GETCHECK);
	m_CoverSpot->Set_Requires_Crouch (bool(crouch == 1));

	// Return true to allow the dialog to close
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAddAttackLocation
//
/////////////////////////////////////////////////////////////////////////////
void
CoverSpotInfoPageClass::OnAddAttackLocation (void)
{
	//
	//	Add a new attack point to the world
	//	
	CoverAttackPointNodeClass *attack_point = m_CoverSpot->Add_Attack_Point (Matrix3D(1));
	if (attack_point != NULL) {
		::Get_Mouse_Mgr ()->Move_Node (attack_point);
	}

	//
	//	Simulate pressing the OK button
	//
	::PostMessage (::GetParent (m_hWnd), WM_COMMAND, MAKELPARAM(IDC_OK, BN_CLICKED), 0L);
	return ;
}
