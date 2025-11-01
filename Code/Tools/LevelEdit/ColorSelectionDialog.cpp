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

// ColorSelectionDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "ColorSelectionDialog.H"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////
//
//	ColorSelectionDialogClass
//
ColorSelectionDialogClass::ColorSelectionDialogClass
(
	const Vector3 &def_color,
	CWnd *pParent
)
	: m_Color (def_color),
	  m_PaintColor (def_color),
	  CDialog(ColorSelectionDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ColorSelectionDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
void
ColorSelectionDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ColorSelectionDialogClass)
	DDX_Control(pDX, IDC_COLOR_WINDOW, m_ColorWindow);
	DDX_Control(pDX, IDC_SLIDER_BLUE, m_BlueSlider);
	DDX_Control(pDX, IDC_SLIDER_GREEN, m_GreenSlider);
	DDX_Control(pDX, IDC_SLIDER_RED, m_RedSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ColorSelectionDialogClass, CDialog)
	//{{AFX_MSG_MAP(ColorSelectionDialogClass)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GRAYSCALE_CHECK, OnGrayscaleCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////
//
//	ColorSelectionDialogClass
//
BOOL
ColorSelectionDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Set the ranges of the slider controls
	m_RedSlider.SetRange (0, 100);
	m_GreenSlider.SetRange (0, 100);
	m_BlueSlider.SetRange (0, 100);

	// Determine the initial settings (in integers)
	int red_value = int(m_Color.X * 100.00F);
	int green_value = int(m_Color.Y * 100.00F);
	int blue_value = int(m_Color.Z * 100.00F);

	if ((red_value == green_value) &&
	    (red_value == blue_value)) {

		// Check the grayscale checkbox
		SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_SETCHECK, (WPARAM)true);
	}

	// Set the initial slider positions
	m_RedSlider.SetPos (red_value);
	m_GreenSlider.SetPos (green_value);
	m_BlueSlider.SetPos (blue_value);
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	ColorSelectionDialogClass
//
void
ColorSelectionDialogClass::OnOK (void)
{
	// Record the color
	m_Color = m_PaintColor;

	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	OnHScroll
//
void
ColorSelectionDialogClass::OnHScroll
(
	UINT nSBCode,
	UINT nPos,
	CScrollBar *pScrollBar
)
{
	// Are the sliders moving together?
	if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK)) {
		int position = 0;
		
		// Determine which slider sent this message and
		// use its current positions
		if (pScrollBar == GetDlgItem (IDC_SLIDER_RED)) {
			position = m_RedSlider.GetPos ();
		} else if (pScrollBar == GetDlgItem (IDC_SLIDER_GREEN)) {
			position = m_GreenSlider.GetPos ();
		} else {
			position = m_BlueSlider.GetPos ();
		}

		// Make all the sliders the same pos
		m_RedSlider.SetPos (position);
		m_GreenSlider.SetPos (position);
		m_BlueSlider.SetPos (position);
	}

	// Record the selected color for later use
	m_PaintColor.X = float(m_RedSlider.GetPos ()) / 100.00F;
	m_PaintColor.Y = float(m_GreenSlider.GetPos ()) / 100.00F;
	m_PaintColor.Z = float(m_BlueSlider.GetPos ()) / 100.00F;

	// Update the window that displays the color the user has selected
	Paint_Color_Window ();

	// Allow the base class to process this message
	CDialog::OnHScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	OnPaint
//
void
ColorSelectionDialogClass::OnPaint (void) 
{
	CPaintDC dc (this);
        
	// Paint the gradients for each color
	::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_RED_GRADIENT), 1, 0, 0);
	::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_GREEN_GRADIENT), 0, 1, 0);
	::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_BLUE_GRADIENT), 0, 0, 1);

	// Update the window that displays the color the user has selected
	Paint_Color_Window ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Paint_Color_Window
//
void
ColorSelectionDialogClass::Paint_Color_Window (void)
{
	// Get the client coords of the 'color' window
	CRect rect;
	m_ColorWindow.GetClientRect (&rect);
	
	// Fill the window with the selected color
	CDC *pdc = m_ColorWindow.GetDC ();
	::FrameRect (*pdc, &rect, (HBRUSH)::GetStockObject (BLACK_BRUSH));
	rect.DeflateRect (1, 1);
	pdc->FillSolidRect (&rect, RGB (int(m_PaintColor.X * 255), int(m_PaintColor.Y * 255), int(m_PaintColor.Z * 255)));
	m_ColorWindow.ReleaseDC (pdc);
	
	// Let the window know it doesn't need to be repainted
	m_ColorWindow.ValidateRect (NULL);
	return; 
}


/////////////////////////////////////////////////////////////////
//
//	OnGrayscaleCheck
//
void
ColorSelectionDialogClass::OnGrayscaleCheck (void)
{
	// Is the checkbox checked?
	if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK)) {
		
		// Make the green and blue sliders the same as red
		m_GreenSlider.SetPos (m_RedSlider.GetPos ());
		m_BlueSlider.SetPos (m_RedSlider.GetPos ());

		Vector3 light_settings;
		light_settings.X = float(m_RedSlider.GetPos ()) / 100.00F;
		light_settings.Y = float(m_GreenSlider.GetPos ()) / 100.00F;
		light_settings.Z = float(m_BlueSlider.GetPos ()) / 100.00F;

		// Update the window that displays the color the user has selected
		Paint_Color_Window ();		
	}

	return ;
}
