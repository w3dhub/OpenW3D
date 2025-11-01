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

// VisualOptionsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "VisualOptionsDialog.h"
#include "Utils.h"
#include "WW3D.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////
const int MAX_REDUCTION		= 5;


//////////////////////////////////////////////////////////////
//
//	VisualOptionsDialogClass
//
//////////////////////////////////////////////////////////////
VisualOptionsDialogClass::VisualOptionsDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(VisualOptionsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VisualOptionsDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VisualOptionsDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VisualOptionsDialogClass)
	DDX_Control(pDX, IDC_TEXTURE_SIZE_SLIDER, m_SizeSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VisualOptionsDialogClass, CDialog)
	//{{AFX_MSG_MAP(VisualOptionsDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
//////////////////////////////////////////////////////////////
BOOL
VisualOptionsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();
	m_SizeSlider.SetRange (0, MAX_REDUCTION);

	// Set the current texture resolution
	int reduction = WW3D::Get_Texture_Reduction ();
	m_SizeSlider.SetPos (MAX_REDUCTION - reduction);

	// Set the wireframe check state
	//SendDlgItemMessage (IDC_WIREFRAME_CHECK, BM_SETCHECK, (WPARAM) WW3D::Get_Polygon_Mode () == WW3D::LINE);	
	return true;
}


//////////////////////////////////////////////////////////////
//
//	OnOK
//
//////////////////////////////////////////////////////////////
void
VisualOptionsDialogClass::OnOK (void)
{
	CWaitCursor wait_cursor;

	// Reduce textures
	int reduction = MAX_REDUCTION - m_SizeSlider.GetPos ();
	WW3D::Set_Texture_Reduction (reduction);

	// Switch between wireframe
	if (SendDlgItemMessage (IDC_WIREFRAME_CHECK, BM_GETCHECK)) {
		//WW3D::Set_Polygon_Mode (WW3D::LINE);
	} else {
		//WW3D::Set_Polygon_Mode (WW3D::FILL);
	}

	CDialog::OnOK ();
	return ;
}
