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

// WaypathInfoPage.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "waypathinfopage.h"
#include "waypathnode.h"
#include "waypointnode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// WaypathInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
WaypathInfoPageClass::WaypathInfoPageClass (void)
	:	m_Waypoint (NULL),
		DockableFormClass (WaypathInfoPageClass::IDD)
{
	//{{AFX_DATA_INIT(WaypathInfoPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WaypathInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
WaypathInfoPageClass::WaypathInfoPageClass (WaypointNodeClass *waypoint)
	:	m_Waypoint (waypoint),
		DockableFormClass (WaypathInfoPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~WaypathInfoPageClass
//
/////////////////////////////////////////////////////////////////////////////
WaypathInfoPageClass::~WaypathInfoPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
WaypathInfoPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WaypathInfoPageClass)
	DDX_Control(pDX, IDC_SPEED_SLIDER, m_SpeedSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(WaypathInfoPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(WaypathInfoPageClass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
/////////////////////////////////////////////////////////////////////////////
void
WaypathInfoPageClass::HandleInitDialog (void)
{
	ASSERT (m_Waypoint != NULL);
	WaypathNodeClass *path = m_Waypoint->Peek_Waypath ();
	
	//
	//	Set the initial state of the 'path' controls
	//
	CheckDlgButton (IDC_TWOWAY_CHECK, path->Get_Flag (WaypathClass::FLAG_TWO_WAY));
	CheckDlgButton (IDC_LOOPING_CHECK, path->Get_Flag (WaypathClass::FLAG_LOOPING));
	CheckDlgButton (IDC_HUMAN_CHECK, path->Get_Flag (WaypathClass::FLAG_HUMAN));
	CheckDlgButton (IDC_GROUND_VEHICLE_CHECK, path->Get_Flag (WaypathClass::FLAG_GROUND_VEHICLE));
	CheckDlgButton (IDC_AIR_VEHICLE_CHECK, path->Get_Flag (WaypathClass::FLAG_FLYING_VEHICLE));
	CheckDlgButton (IDC_INNATE_PATHFIND_CHECK, path->Get_Flag (WaypathClass::FLAG_INNATE_PATHFIND));
	
	//	
	//	Set the initial state of the 'point' controls
	//
	m_SpeedSlider.SetRange (1, 100);
	m_SpeedSlider.SetPos (int(m_Waypoint->Get_Speed () * 100));
	CheckDlgButton (IDC_JUMP_CHECK, m_Waypoint->Get_Flag (WaypointNodeClass::FLAG_REQUIRES_JUMP));

	//
	//	Update the enabled state of the checkboxes
	//
	Update_Enable_State ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
WaypathInfoPageClass::Apply_Changes (void)
{
	WaypathNodeClass *path = m_Waypoint->Peek_Waypath ();
	
	//
	//	Pass the flags back to the path object
	//
	path->Set_Flag (WaypathClass::FLAG_TWO_WAY, IsDlgButtonChecked (IDC_TWOWAY_CHECK) == 1);
	path->Set_Flag (WaypathClass::FLAG_LOOPING, IsDlgButtonChecked (IDC_LOOPING_CHECK) == 1);
	path->Set_Flag (WaypathClass::FLAG_HUMAN, IsDlgButtonChecked (IDC_HUMAN_CHECK) == 1);
	path->Set_Flag (WaypathClass::FLAG_GROUND_VEHICLE, IsDlgButtonChecked (IDC_GROUND_VEHICLE_CHECK) == 1);
	path->Set_Flag (WaypathClass::FLAG_FLYING_VEHICLE, IsDlgButtonChecked (IDC_AIR_VEHICLE_CHECK) == 1);
	path->Set_Flag (WaypathClass::FLAG_INNATE_PATHFIND, IsDlgButtonChecked (IDC_INNATE_PATHFIND_CHECK) == 1);

	//
	//	Pass the waypoint settings onto the waypoint object
	//
	int speed = m_SpeedSlider.GetPos ();
	m_Waypoint->Set_Speed (((float)speed) / 100.0F);
	m_Waypoint->Set_Flag (WaypointNodeClass::FLAG_REQUIRES_JUMP, IsDlgButtonChecked (IDC_JUMP_CHECK) == 1);

		
	// Return true to allow the dialog to close
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
WaypathInfoPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
) 
{
	if (LOWORD (wParam) == IDC_INNATE_PATHFIND_CHECK && HIWORD (wParam) == BN_CLICKED) {
		Update_Enable_State ();
	}

	return DockableFormClass::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Enable_State
//
/////////////////////////////////////////////////////////////////////////////
void
WaypathInfoPageClass::Update_Enable_State (void)
{
	bool enabled = (IsDlgButtonChecked (IDC_INNATE_PATHFIND_CHECK) == 1);

	::EnableWindow (::GetDlgItem (m_hWnd, IDC_HUMAN_CHECK), enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_GROUND_VEHICLE_CHECK), enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_AIR_VEHICLE_CHECK), enabled);
	return ;
}
