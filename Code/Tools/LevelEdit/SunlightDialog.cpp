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

// SunlightDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "SunlightDialog.h"
#include "Utils.H"
#include "Light.H"
#include "SceneEditor.H"
#include "Utils.h"
#include "colorpickerdialogclass.h"
#include "phys.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SunlightDialogClass dialog


SunlightDialogClass::SunlightDialogClass(CWnd* pParent /*=NULL*/)
	:	m_pSunlight (NULL),
		CDialog(SunlightDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(SunlightDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

SunlightDialogClass::~SunlightDialogClass (void)
{
	MEMBER_RELEASE (m_pSunlight);
	return ;
}

void SunlightDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SunlightDialogClass)
	DDX_Control(pDX, IDC_YAW_SLIDER, m_YawSlider);
	DDX_Control(pDX, IDC_PITCH_SLIDER, m_PitchSlider);
	DDX_Control(pDX, IDC_INTENSITY_SLIDER, m_IntensitySlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SunlightDialogClass, CDialog)
	//{{AFX_MSG_MAP(SunlightDialogClass)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_COLOR, OnColor)
	ON_WM_DRAWITEM()
	ON_EN_UPDATE(IDC_PITCH_EDIT, OnUpdatePitchEdit)
	ON_EN_KILLFOCUS(IDC_PITCH_EDIT, OnKillfocusPitchEdit)
	ON_EN_KILLFOCUS(IDC_YAW_EDIT, OnKillfocusYawEdit)
	ON_EN_UPDATE(IDC_YAW_EDIT, OnUpdateYawEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SunlightDialogClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	
	m_pSunlight = ::Get_Scene_Editor ()->Get_Sun_Light ();
	ASSERT (m_pSunlight != NULL);
	m_pSunlight->Get_Diffuse (&m_Color);
	
	::Get_Scene_Editor ()->Get_Sun_Light_Orientation (&m_Yaw, &m_Pitch);
	m_Intensity = m_pSunlight->Get_Intensity ();
	m_Yaw		= WWMath::Wrap (m_Yaw, 0, DEG_TO_RADF (360));
	m_Pitch	= WWMath::Wrap (m_Pitch, 0, DEG_TO_RADF (90));

	m_YawSlider.SetRange (0, 360);
	m_PitchSlider.SetRange (0, 90);
	m_IntensitySlider.SetRange (0, 100);

	m_YawSlider.SetPos ((int)(RAD_TO_DEG (m_Yaw) + 0.5F));
	m_PitchSlider.SetPos ((int)(RAD_TO_DEG (m_Pitch) + 0.5F));
	m_IntensitySlider.SetPos ((m_Intensity * 100));

	::SetDlgItemFloat (m_hWnd, IDC_YAW_EDIT, (float)m_YawSlider.GetPos ());
	::SetDlgItemFloat (m_hWnd, IDC_PITCH_EDIT, (float)m_PitchSlider.GetPos ());
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnHScroll
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnHScroll
(
	UINT nSBCode,
	UINT nPos, 
	CScrollBar* pScrollBar
) 
{
	if (pScrollBar == GetDlgItem (IDC_YAW_SLIDER)) {		
		m_Yaw = DEG_TO_RAD ((float)m_YawSlider.GetPos ());
		::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
		::Get_Scene_Editor ()->Update_Lighting ();
		::SetDlgItemFloat (m_hWnd, IDC_YAW_EDIT, (float)m_YawSlider.GetPos ());
		::Get_Scene_Editor ()->Update_Lighting ();

	} else if (pScrollBar == GetDlgItem (IDC_PITCH_SLIDER)) {
		m_Pitch = DEG_TO_RAD ((float)m_PitchSlider.GetPos ());
		::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
		::Get_Scene_Editor ()->Update_Lighting ();
		::SetDlgItemFloat (m_hWnd, IDC_PITCH_EDIT, (float)m_PitchSlider.GetPos ());
		::Get_Scene_Editor ()->Update_Lighting ();

	} else if (pScrollBar == GetDlgItem (IDC_INTENSITY_SLIDER)) {
		int pos = m_IntensitySlider.GetPos ();
		m_Intensity = ((float)pos) / 100.0F;
		m_pSunlight->Set_Intensity (m_Intensity);
		::Get_Scene_Editor ()->Update_Lighting ();
	}
	
	::Refresh_Main_View ();	

	// Allow the base class to process this message
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnColor
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnColor (void)
{
	int red		= m_Color.X * 255;
	int green	= m_Color.Y * 255;
	int blue		= m_Color.Z * 255;
	
	//
	// Display a dialog to the user that will allow them to select a color
	//
	if (::Show_Color_Picker (&red, &green, &blue)) {
		
		//
		//	Get the color from the picker and pass it onto the manager
		//
		m_Color.X = ((float)red) / 255.0F;
		m_Color.Y = ((float)green) / 255.0F;
		m_Color.Z = ((float)blue) / 255.0F;
		m_pSunlight->Set_Diffuse (m_Color);

		//
		// Repaint the views
		//
		::InvalidateRect (::GetDlgItem (m_hWnd, IDC_BK_COLOR), NULL, TRUE);
		::Get_Scene_Editor ()->Update_Lighting ();
		::Refresh_Main_View ();
	}


	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDrawItem
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnDrawItem
(
	int nIDCtl,
	LPDRAWITEMSTRUCT lpDrawItemStruct
) 
{
	// Determine what state to draw the button in (pushed or normal)
	UINT state = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		state |= DFCS_PUSHED;
	}

	// Draw the button's outline
	CRect rect = lpDrawItemStruct->rcItem;
	::DrawFrameControl (lpDrawItemStruct->hDC, rect, DFC_BUTTON, state);

	// Fill the button with the appropriate color
	CDC temp_dc;
	temp_dc.Attach (lpDrawItemStruct->hDC);
	temp_dc.FillSolidRect (&rect, RGB (int(m_Color.X * 255), int(m_Color.Y * 255), int(m_Color.Z * 255)));
	temp_dc.Detach ();
	
	// Draw the focus rectangle if necessary
	if (lpDrawItemStruct->itemState & ODS_FOCUS) {
		::DrawFocusRect (lpDrawItemStruct->hDC, &rect);
	}
	
	// Allow the base class to process this message
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnUpdatePitchEdit
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnUpdatePitchEdit (void)
{
	float pitch_deg	= ::GetDlgItemFloat (m_hWnd, IDC_PITCH_EDIT);	
	m_Pitch				= DEG_TO_RAD (pitch_deg);
	::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
	::Get_Scene_Editor ()->Update_Lighting ();
	m_PitchSlider.SetPos ((int)pitch_deg);

	::Refresh_Main_View ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKillfocusPitchEdit
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnKillfocusPitchEdit (void)
{
	float pitch_deg = ::GetDlgItemFloat (m_hWnd, IDC_PITCH_EDIT);	
	if (pitch_deg > 90.0F) {
		pitch_deg = 90.0F;
		::SetDlgItemFloat (m_hWnd, IDC_PITCH_EDIT, pitch_deg);

		m_Pitch = DEG_TO_RAD (pitch_deg);
		::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
		::Get_Scene_Editor ()->Update_Lighting ();
		::Refresh_Main_View ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKillfocusYawEdit
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnKillfocusYawEdit (void)
{
	float yaw_deg = ::GetDlgItemFloat (m_hWnd, IDC_YAW_EDIT);	
	if (yaw_deg > 360.0F) {
		yaw_deg = 360.0F;
		::SetDlgItemFloat (m_hWnd, IDC_YAW_EDIT, yaw_deg);

		m_Yaw = DEG_TO_RAD (yaw_deg);
		::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
		::Get_Scene_Editor ()->Update_Lighting ();
		::Refresh_Main_View ();
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// OnUpdateYawEdit
//
/////////////////////////////////////////////////////////////////////////////
void
SunlightDialogClass::OnUpdateYawEdit (void)
{
	float yaw_deg = ::GetDlgItemFloat (m_hWnd, IDC_YAW_EDIT);	
	m_Yaw = DEG_TO_RAD (yaw_deg);
	::Get_Scene_Editor ()->Set_Sun_Light_Orientation (m_Yaw, m_Pitch);
	::Get_Scene_Editor ()->Update_Lighting ();
	m_YawSlider.SetPos ((int)yaw_deg);

	::Refresh_Main_View ();
	return ;
}

