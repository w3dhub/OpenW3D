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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LightSettingsPage.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.H"
#include "LightSettingsPage.H"
#include "Utils.H"
#include "ColorSelectionDialog.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// LightSettingsPageClass
//
LightSettingsPageClass::LightSettingsPageClass (void)
	: m_pLight (NULL),
	  DockableFormClass (LightSettingsPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// LightSettingsPageClass
//
LightSettingsPageClass::LightSettingsPageClass (LightSettingsClass *plight)
	: m_pLight (plight),
	  DockableFormClass (LightSettingsPageClass::IDD)
{
	//{{AFX_DATA_INIT(LightSettingsPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
// ~LightSettingsPageClass
//
LightSettingsPageClass::~LightSettingsPageClass (void)
{
	return;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
LightSettingsPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightSettingsPageClass)
	DDX_Control(pDX, IDC_INTENSITY_SLIDER, m_IntensitySlider);
	DDX_Control(pDX, IDC_OUTER_ATTEN_SLIDER, m_OuterAttenSlider);
	DDX_Control(pDX, IDC_INNER_ATTEN_SLIDER, m_InnerAttenSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(LightSettingsPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(LightSettingsPageClass)
	ON_WM_DRAWITEM()
	ON_EN_CHANGE(IDC_INNER_ATTEN_EDIT, OnChangeInnerAttenEdit)
	ON_EN_CHANGE(IDC_OUTER_ATTEN_EDIT, OnChangeOuterAttenEdit)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_OUTER_ATTEN_EDIT, OnKillfocusOuterAttenEdit)
	ON_EN_KILLFOCUS(IDC_INNER_ATTEN_EDIT, OnKillfocusInnerAttenEdit)
	ON_BN_CLICKED(IDC_AMBIENT_BUTTON, OnAmbientButton)
	ON_BN_CLICKED(IDC_DIFFUSE_BUTTON, OnDiffuseButton)
	ON_BN_CLICKED(IDC_SPECULAR_BUTTON, OnSpecularButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LightSettingsPageClass diagnostics

#ifdef _DEBUG
void LightSettingsPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void LightSettingsPageClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG



/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
void
LightSettingsPageClass::HandleInitDialog (void)
{
	ASSERT (m_pLight != NULL);

	// Record the light's original settings so we can
	// restore them if we need to.
	m_OrigAmbient = m_pLight->Get_Ambient_Color ();
	m_OrigDiffuse = m_pLight->Get_Diffuse_Color ();
	m_OrigSpecular = m_pLight->Get_Specular_Color ();
	m_OrigIntensity = m_pLight->Get_Intensity ();
	m_pLight->Get_Attenuation (m_OrigInnerAttenuation, m_OrigOuterAttenuation);

	// Set up the sliders
	m_IntensitySlider.SetRange (0, 100);
	m_InnerAttenSlider.SetRange (0, 100000);
	m_OuterAttenSlider.SetRange (0, 100000);
	m_IntensitySlider.SetPos (int(m_OrigIntensity * 100.00F));
	m_InnerAttenSlider.SetPos (int(m_OrigInnerAttenuation * 100.00F));
	m_OuterAttenSlider.SetPos (int(m_OrigOuterAttenuation * 100.00F));

	// Put the attenuation values into the edit fields
	::SetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT, m_OrigInnerAttenuation);
	::SetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT, m_OrigOuterAttenuation);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
bool
LightSettingsPageClass::Apply_Changes (void)
{
	// Return true to allow the dialog to close
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// Discard_Changes
//
void
LightSettingsPageClass::Discard_Changes (void)
{
	// Give the original values back to the light
	m_pLight->Set_Ambient_Color (m_OrigAmbient);
	m_pLight->Set_Diffuse_Color (m_OrigDiffuse);
	m_pLight->Set_Specular_Color (m_OrigSpecular);
	m_pLight->Set_Intensity (m_OrigIntensity);
	m_pLight->Set_Attenuation (m_OrigInnerAttenuation, m_OrigOuterAttenuation);	

	// Now update the light with these changes
	m_pLight->Update_Light ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDrawItem
//
void
LightSettingsPageClass::OnDrawItem
(
	int nIDCtl,
	LPDRAWITEMSTRUCT lpDrawItemStruct
) 
{
	// Determine what color to paint the button
	Vector3 color (0, 0, 0);
	if (nIDCtl == IDC_AMBIENT_BUTTON) {
		color = m_pLight->Get_Ambient_Color ();
	} else if (nIDCtl == IDC_DIFFUSE_BUTTON) {
		color = m_pLight->Get_Diffuse_Color ();
	} else if (nIDCtl == IDC_SPECULAR_BUTTON) {
		color = m_pLight->Get_Specular_Color ();
	}

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
	temp_dc.FillSolidRect (&rect, RGB (int(color.X * 255), int(color.Y * 255), int(color.Z * 255)));
	temp_dc.Detach ();
	
	// Draw the focus rectangle if necessary
	if (lpDrawItemStruct->itemState & ODS_FOCUS) {
		::DrawFocusRect (lpDrawItemStruct->hDC, &rect);
	}

	// Allow the base class to process this message
	//DockableFormClass::OnDrawItem (nIDCtl, lpDrawItemStruct);
	return; 
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeInnerAttenEdit
//
void
LightSettingsPageClass::OnChangeInnerAttenEdit (void) 
{
	float inner = ::GetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT);
	float outer = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT);
	m_InnerAttenSlider.SetPos (int(inner * 100.00F));
	
	// Update the light with these new settings
	m_pLight->Set_Attenuation (inner, outer);	
	m_pLight->Update_Light ();
	::Refresh_Main_View ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeOuterAttenEdit
//
void
LightSettingsPageClass::OnChangeOuterAttenEdit (void)
{
	float inner = ::GetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT);
	float outer = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT);
	m_OuterAttenSlider.SetPos (int(outer * 100.00F));

	// Update the light with these new settings
	m_pLight->Set_Attenuation (inner, outer);	
	m_pLight->Update_Light ();
	::Refresh_Main_View ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeOuterAttenEdit
//
void
LightSettingsPageClass::OnHScroll
(
	UINT nSBCode,
	UINT nPos,
	CScrollBar *pScrollBar
) 
{
	if (pScrollBar == GetDlgItem (IDC_INTENSITY_SLIDER)) {
		
		// Pass the new intensity onto the UI and the light
		int position = m_IntensitySlider.GetPos ();
		m_pLight->Set_Intensity ((((float)position) / 100.00F));
		
	} else if (pScrollBar == GetDlgItem (IDC_OUTER_ATTEN_SLIDER)) {
		
		// Determine the radii of the inner and outer attenuation spheres
		int position = m_OuterAttenSlider.GetPos ();
		float outer = (((float)position) / 100.00F);
		float inner = ::GetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT);

		if (outer < inner) {
			outer = inner;
			m_OuterAttenSlider.SetPos (int(outer * 100.00F));
		}

		// Reset the UI and the light to match these settings
		::SetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT, outer);
		m_pLight->Set_Attenuation (inner, outer);

	} else {
		
		// Determine the radii of the inner and outer attenuation spheres
		int position = m_InnerAttenSlider.GetPos ();
		float inner = (((float)position) / 100.00F);
		float outer = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT);

		if (outer < inner) {
			inner = outer;
			m_InnerAttenSlider.SetPos (int(inner * 100.00F));
		}

		// Reset the UI and the light to match these settings
		::SetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT, inner);		
		m_pLight->Set_Attenuation (inner, outer);
	}

	// Now update the light with these changes
	m_pLight->Update_Light ();

	// Make sure the scene is painted
	::Refresh_Main_View ();

	// Allow the base class to process this message
	DockableFormClass::OnHScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKillfocusOuterAttenEdit
//
void
LightSettingsPageClass::OnKillfocusOuterAttenEdit (void) 
{
	float inner = ::GetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT);
	float outer = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT);

	// If we need to normalize the data, then do so now...
	if (outer < inner) {
		
		// Reset the UI
		outer = inner;
		::SetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT, outer);
		m_OuterAttenSlider.SetPos (int(outer * 100.00F));

		// Update the light with these new settings
		m_pLight->Set_Attenuation (inner, outer);	
		m_pLight->Update_Light ();
		::Refresh_Main_View ();
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKillfocusInnerAttenEdit
//
void
LightSettingsPageClass::OnKillfocusInnerAttenEdit (void)
{
	float inner = ::GetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT);
	float outer = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_ATTEN_EDIT);

	// If we need to normalize the data, then do so now...
	if (outer < inner) {
		
		// Reset the UI
		inner = outer;
		::SetDlgItemFloat (m_hWnd, IDC_INNER_ATTEN_EDIT, inner);
		m_InnerAttenSlider.SetPos (int(outer * 100.00F));

		// Update the light with these new settings
		m_pLight->Set_Attenuation (inner, outer);	
		m_pLight->Update_Light ();
		::Refresh_Main_View ();
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAmbientButton
//
void
LightSettingsPageClass::OnAmbientButton (void)
{
	// Display a dialog to the user that will allow them to select a color
	ColorSelectionDialogClass dialog (m_pLight->Get_Ambient_Color (), this);
	if (dialog.DoModal () == IDOK) {
		m_pLight->Set_Ambient_Color (dialog.Get_Color ());
		m_pLight->Update_Light ();

		// Repaint the views
		::InvalidateRect (::GetDlgItem (m_hWnd, IDC_AMBIENT_BUTTON), NULL, TRUE);
		::Refresh_Main_View ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDiffuseButton
//
void
LightSettingsPageClass::OnDiffuseButton (void) 
{
	// Display a dialog to the user that will allow them to select a color
	ColorSelectionDialogClass dialog (m_pLight->Get_Diffuse_Color (), this);
	if (dialog.DoModal () == IDOK) {
		m_pLight->Set_Diffuse_Color (dialog.Get_Color ());
		m_pLight->Update_Light ();

		// Repaint the views
		::InvalidateRect (::GetDlgItem (m_hWnd, IDC_DIFFUSE_BUTTON), NULL, TRUE);
		::Refresh_Main_View ();
	}

	return; 
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSpecularButton
//
void
LightSettingsPageClass::OnSpecularButton (void)
{
	// Display a dialog to the user that will allow them to select a color
	ColorSelectionDialogClass dialog (m_pLight->Get_Specular_Color (), this);
	if (dialog.DoModal () == IDOK) {
		m_pLight->Set_Specular_Color (dialog.Get_Color ());
		m_pLight->Update_Light ();

		// Repaint the views
		::InvalidateRect (::GetDlgItem (m_hWnd, IDC_SPECULAR_BUTTON), NULL, TRUE);
		::Refresh_Main_View ();
	}

	return; 
}
