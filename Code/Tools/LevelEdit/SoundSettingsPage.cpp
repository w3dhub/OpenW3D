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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SoundSettingsPage.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "leveledit.h"
#include "soundsettingspage.h"
#include "utils.h"
#include "filemgr.h"
#include "audiblesound.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// SoundSettingsPageClass
//
SoundSettingsPageClass::SoundSettingsPageClass (void)	:
	Definition (NULL),
	DockableFormClass (SoundSettingsPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~SoundSettingsPageClass
//
/////////////////////////////////////////////////////////////////////////////
SoundSettingsPageClass::~SoundSettingsPageClass (void)
{
	return;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
SoundSettingsPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SoundSettingsPageClass)
	DDX_Control(pDX, IDC_VOLUME_SLIDER, m_VolumeSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(SoundSettingsPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(SoundSettingsPageClass)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_VOLUME_EDIT, OnChangeVolumeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SoundSettingsPageClass diagnostics

#ifdef _DEBUG
void SoundSettingsPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void SoundSettingsPageClass::Dump(CDumpContext& dc) const
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
SoundSettingsPageClass::HandleInitDialog (void)
{
	ASSERT (Definition != NULL);

	//
	// Set up the volume controls
	//
	m_VolumeSlider.SetRange (0, 100);
	m_VolumeSlider.SetPos (int(Definition->Get_Volume () * 100.0F));
	::SetDlgItemFloat (m_hWnd, IDC_VOLUME_EDIT, (Definition->Get_Volume () * 100.0F));

	//
	// Put the attenuation factors into the edit fields
	//
	::SetDlgItemFloat (m_hWnd, IDC_DROP_OFF_EDIT, Definition->Get_DropOff_Radius ());
	::SetDlgItemFloat (m_hWnd, IDC_MAX_VOL_EDIT, Definition->Get_Max_Vol_Radius ());

	//
	// Put the start offset into the edit field
	//
	::SetDlgItemFloat (m_hWnd, IDC_START_OFFSET_EDIT, Definition->Get_Start_Offset ());

	//
	// Put the pitch factor into the edit field
	//
	::SetDlgItemFloat (m_hWnd, IDC_PITCH_FACTOR_EDIT, Definition->Get_Pitch_Factor ());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
bool
SoundSettingsPageClass::Apply_Changes (void)
{
	//
	// Read the volume settings
	//
	float volume = ::GetDlgItemFloat (m_hWnd, IDC_VOLUME_EDIT);
	Definition->Set_Volume (volume / 100.0F);

	//
	// Read the attenuation factors
	//
	float dropoff	= ::GetDlgItemFloat (m_hWnd, IDC_DROP_OFF_EDIT);
	float maxvol	= ::GetDlgItemFloat (m_hWnd, IDC_MAX_VOL_EDIT);
	Definition->Set_DropOff_Radius (dropoff);
	Definition->Set_Max_Vol_Radius (maxvol);

	//
	// Read the start offset
	//
	float offset = ::GetDlgItemFloat (m_hWnd, IDC_START_OFFSET_EDIT);
	Definition->Set_Start_Offset (offset);

	//
	// Read the pitch factor
	//
	float factor = ::GetDlgItemFloat (m_hWnd, IDC_PITCH_FACTOR_EDIT);
	Definition->Set_Pitch_Factor (factor);	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Discard_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
SoundSettingsPageClass::Discard_Changes (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnHScroll
//
/////////////////////////////////////////////////////////////////////////////
void
SoundSettingsPageClass::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	//
	//	Update the edit control
	//
	::SetDlgItemFloat (m_hWnd, IDC_VOLUME_EDIT, m_VolumeSlider.GetPos ());
		
	DockableFormClass::OnHScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeVolumeEdit
//
/////////////////////////////////////////////////////////////////////////////
void
SoundSettingsPageClass::OnChangeVolumeEdit (void)
{
	//
	//	Update the edit control
	//
	float volume = ::GetDlgItemFloat (m_hWnd, IDC_VOLUME_EDIT);
	volume = WWMath::Clamp (volume, 0, 100.0F);
	m_VolumeSlider.SetPos (volume);	
	return ;
}

