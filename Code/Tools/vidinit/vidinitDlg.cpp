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

// vidinitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vidinit.h"
#include "vidinitDlg.h"
#include "SurrenderWnd.h"
#include "wwsaveload.h"
#include <ww3d.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define REGISTRY_KEY "Software\\Westwood Studios\\Earth and Beyond\\Render"
#define REGISTRY_KEY "Software\\Westwood Studios\\Renegade\\Render"

/////////////////////////////////////////////////////////////////////////////
// CVidinitDlg dialog

CVidinitDlg::CVidinitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVidinitDlg::IDD, pParent),
	m_TestCWnd(0)
{
	//{{AFX_DATA_INIT(CVidinitDlg)
	m_RunFullScreen = false;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVidinitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVidinitDlg)
	DDX_Control(pDX, IDC_RESOLUTIONS, m_Resolutions);
	DDX_Control(pDX, IDC_DRIVERS, m_Drivers);
	DDX_Check(pDX, IDC_CHECK1, m_RunFullScreen);
	//}}AFX_DATA_MAP
}															  

BEGIN_MESSAGE_MAP(CVidinitDlg, CDialog)
	//{{AFX_MSG_MAP(CVidinitDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_DRIVERS, OnSelchangeDrivers)
	ON_BN_CLICKED(IDTEST, OnTest)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER+1, OnDeadBeef)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVidinitDlg message handlers

BOOL CVidinitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, true);			// Set big icon
	SetIcon(m_hIcon, false);		// Set small icon
	
	// TODO: Add extra initialization here

	WW3D::Init(GetSafeHwnd());
	WWMath::Init();
	WWSaveLoad::Init();

	// HACK: Added throwaway variable for texture_depth to match new function interface. (DRM 6/01/00)
	// TODO: Add UI support for texture depth selection. (DRM, 6/1/00)
	int texture_depth;
	WW3D::Detect_Drivers(AfxGetInstanceHandle());
	WW3D::Registry_Load_Render_Device(REGISTRY_KEY, m_DeviceName, 255, m_DeviceWidth, m_DeviceHeight, m_DeviceDepth, m_DeviceWindowed, texture_depth);

	// Loop thought the different available render devices
	for (int lp = 0; lp < WW3D::Get_Render_Device_Count(); lp++) {
		const RenderDeviceDescClass &desc = WW3D::Get_Render_Device_Desc(lp);
		char temp[255];
		sprintf(temp, "%s", WW3D::Get_Render_Device_Name(lp));
		m_Drivers.AddString(temp);

	}
	int idx = m_Drivers.FindStringExact(0, m_DeviceName);
	if (idx != -1) {
		m_Drivers.SetCurSel(idx);
		OnSelchangeDrivers();

		char temp[255];
		sprintf(temp, "%d x %d in %d bit mode.", m_DeviceWidth, m_DeviceHeight, m_DeviceDepth);

		int idx = m_Resolutions.FindStringExact(0, temp);
		m_Resolutions.SetCurSel(idx);

		m_RunFullScreen = !m_DeviceWindowed;
		UpdateData(false);
	}

	return true;  // return true  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVidinitDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVidinitDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CVidinitDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();

	int driver	= m_Drivers.GetCurSel();
	if (driver == -1) return;

	int sel		= m_Resolutions.GetCurSel();
	if (sel == -1) return;

	const RenderDeviceDescClass &desc = WW3D::Get_Render_Device_Desc(driver);
	const DynamicVectorClass<ResolutionDescClass>& res = desc.Enumerate_Resolutions();

	// HACK: using constant 32 for texture depth. (DRM, 6/1/00)
	// TODO: Add UI support for texture depth selection. (DRM, 6/1/00)
	WW3D::Registry_Save_Render_Device( REGISTRY_KEY, driver, res[sel].Width, res[sel].Height, res[sel].BitDepth, !m_RunFullScreen, 32);

	WW3D::Shutdown();
	WWMath::Shutdown();
	WWSaveLoad::Shutdown();


	CDialog::OnOK();
}

void CVidinitDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	WW3D::Shutdown();
	WWMath::Shutdown();
	WWSaveLoad::Shutdown();

	CDialog::OnCancel();
}

void CVidinitDlg::OnSelchangeDrivers() 
{
	// TODO: Add your control notification handler code here
	int driver = m_Drivers.GetCurSel();

	const RenderDeviceDescClass &desc = WW3D::Get_Render_Device_Desc(driver);
	SetDlgItemText(IDC_HARDWARE_NAME, desc.Get_Hardware_Name());
	SetDlgItemText(IDC_HARDWARE_VENDOR, desc.Get_Hardware_Vendor());
	SetDlgItemText(IDC_HARDWARE_CHIPSET, desc.Get_Hardware_Chipset());
	SetDlgItemText(IDC_DRIVER_NAME, desc.Get_Device_Name());
	SetDlgItemText(IDC_DRIVER_VENDOR, desc.Get_Device_Vendor());
	
	const DynamicVectorClass<ResolutionDescClass>& res = desc.Enumerate_Resolutions();
	while (m_Resolutions.GetCount()) {
		m_Resolutions.DeleteString(0);
	}

	for (int lp = 0; lp < res.Count(); lp++) {
		char temp[255];
		sprintf(temp, "%d x %d in %d bit mode.", res[lp].Width, res[lp].Height, res[lp].BitDepth);
		m_Resolutions.AddString(temp);
	}


	
}

void CVidinitDlg::OnTest() 
{
	UpdateData(true);


	// TODO: Add your control notification handler code here
	int driver	= m_Drivers.GetCurSel();
	if (driver == -1) return;

	int sel		= m_Resolutions.GetCurSel();
	if (sel == -1) return;

	const RenderDeviceDescClass &desc = WW3D::Get_Render_Device_Desc(driver);
	const DynamicVectorClass<ResolutionDescClass>& res = desc.Enumerate_Resolutions();
	
	m_TestCWnd = new CSurrenderWnd(this, driver, res[sel].Width, res[sel].Height, res[sel].BitDepth, !m_RunFullScreen);

	CWnd *wnd = GetDlgItem(IDTEST);
	wnd->EnableWindow(false);
	wnd = GetDlgItem(IDOK);
	wnd->EnableWindow(false);
	wnd = GetDlgItem(IDCANCEL);
	wnd->EnableWindow(false);
}


LRESULT CVidinitDlg::OnDeadBeef(WPARAM wParam, LPARAM lParam) 
{
	if (m_TestCWnd && wParam == 0xDEADBEEF && lParam == 0xDEADBEEF) {
		WW3D::Restore_Display_Mode();
		m_TestCWnd->RemoveSubclass();
		m_TestCWnd->PostMessage(WM_CLOSE);
		m_TestCWnd = 0;
		CWnd *wnd = GetDlgItem(IDTEST);
		wnd->EnableWindow(true);
		wnd = GetDlgItem(IDOK);
		wnd->EnableWindow(true);
		wnd = GetDlgItem(IDCANCEL);
		wnd->EnableWindow(true);

	}
	return 0;
}
