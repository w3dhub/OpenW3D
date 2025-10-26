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

// RenderDeviceDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "RenderDeviceDialog.h"
#include "ww3d.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderDeviceDialog dialog


CRenderDeviceDialog::CRenderDeviceDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRenderDeviceDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenderDeviceDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRenderDeviceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRenderDeviceDialog)
	DDX_Control(pDX, IDC_RENDER_DEVICE_COMBO, m_RenderDeviceCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRenderDeviceDialog, CDialog)
	//{{AFX_MSG_MAP(CRenderDeviceDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenderDeviceDialog message handlers

BOOL CRenderDeviceDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// plug all of the render device names into the combo box
	for (int i=0; i<WW3D::Get_Render_Device_Count(); i++) {
		const char * name = WW3D::Get_Render_Device_Name(i);
		m_RenderDeviceCombo.AddString(name);
	}
	m_RenderDeviceCombo.SetCurSel(WW3D::Get_Render_Device());

	return true;
}

void CRenderDeviceDialog::OnOK() 
{
	// get the index of the currently selected render device and set ww3d to use it
	int device = m_RenderDeviceCombo.GetCurSel();
	if (device != CB_ERR) {

		// keep trying to set a device until one works
		WW3DErrorType err = WW3D_ERROR_GENERIC;
		int count = 0;
		while ((err != WW3D_ERROR_OK) && (count < WW3D::Get_Render_Device_Count())) {
			err = WW3D::Set_Render_Device(device);
			count++;
			if (err != WW3D_ERROR_OK) {
				device = (device + 1) % WW3D::Get_Render_Device_Count();
			}
		}
		assert(err == WW3D_ERROR_OK);
	
	}
	CDialog::OnOK();
}

