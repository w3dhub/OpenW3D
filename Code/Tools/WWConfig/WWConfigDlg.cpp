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

// WWConfigDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "WWConfig.h"
#include "WWConfigDlg.h"
#include "videoconfigdialog.h"
#include "audioconfigdialog.h"
#include "PerformanceConfigDialog.h"
#include "locale_api.h"
#include "wwconfig_ids.h"

extern int GlobalExitValue;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////
//
//	CWWConfigDlg
//
///////////////////////////////////////////////////////////////////
CWWConfigDlg::CWWConfigDlg (CWnd *pParent)
	:	CurrentTab (0),
		CDialog(CWWConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWWConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
///////////////////////////////////////////////////////////////////
void
CWWConfigDlg::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWWConfigDlg)
	DDX_Control(pDX, IDC_TAB_CTRL, m_TabCtrl);
	//}}AFX_DATA_MAP
	return ;
}

BEGIN_MESSAGE_MAP(CWWConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CWWConfigDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CTRL, OnSelchangeTabCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
///////////////////////////////////////////////////////////////////
BOOL
CWWConfigDlg::OnInitDialog (void)
{
	char string[ _MAX_PATH ];

	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon (m_hIcon, true);
	SetIcon (m_hIcon, false);

	//
	// Set Title of Dialog.
	//
	Locale_GetString( IDS_WWCONFIG_TITLE, string );
	SetWindowText((LPCTSTR) string );

	//
	// Set text on buttons.
	//
	Locale_GetString( IDS_OK, string );
	SetDlgItemText( IDOK, string  );

	Locale_GetString( IDS_CANCEL, string );
	SetDlgItemText( IDCANCEL, string  );

	//
	//	Add a tab to the dialog for video configuration
	//
	TC_ITEM tab_info	= { 0 };

	Locale_GetString( IDS_VIDEO_TAB, string );
	tab_info.mask		= TCIF_TEXT;
//	tab_info.pszText	= "Video";
	tab_info.pszText	= string;
	m_TabCtrl.InsertItem (0xFF, &tab_info);

	//
	//	Add a tab to the dialog for audio configuration
	//
	Locale_GetString( IDS_SOUND_TAB, string );
	tab_info.mask		= TCIF_TEXT;
//	tab_info.pszText	= "Sound";
	tab_info.pszText	= string;
	m_TabCtrl.InsertItem (0xFF, &tab_info);

	//
	//	Add a tab to the dialog for performance configuration
	//
	Locale_GetString( IDS_PERFORMANCE_TAB, string );
	tab_info.mask		= TCIF_TEXT;
//	tab_info.pszText	= "Performance";
	tab_info.pszText	= string;
	m_TabCtrl.InsertItem (0xFF, &tab_info);

	//
	// Get the display rectangle of the tab control
	//
	CRect tab_rect;
	m_TabCtrl.GetWindowRect (&tab_rect);
	m_TabCtrl.AdjustRect (false, &tab_rect);
	ScreenToClient (&tab_rect);

	VideoConfigDialogClass			*video_page = new VideoConfigDialogClass (this);
	AudioConfigDialogClass			*audio_page = new AudioConfigDialogClass (this);
	PerformanceConfigDialogClass	*performance_page = new PerformanceConfigDialogClass (this);

	video_page->SetWindowPos		( NULL, tab_rect.left, tab_rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);	
	audio_page->SetWindowPos		( NULL, tab_rect.left, tab_rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	performance_page->SetWindowPos	( NULL, tab_rect.left, tab_rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);	

	ConfigPages.Add ( video_page );
	ConfigPages.Add ( audio_page );
	ConfigPages.Add ( performance_page );
	
	//
	//	Display the first category page
	//
	ConfigPages[0]->ShowWindow( SW_SHOW );
	return true;
}


///////////////////////////////////////////////////////////////////
//
//	OnPaint
//
///////////////////////////////////////////////////////////////////
void
CWWConfigDlg::OnPaint (void) 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

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

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	OnQueryDragIcon
//
///////////////////////////////////////////////////////////////////
HCURSOR
CWWConfigDlg::OnQueryDragIcon (void)
{
	return (HCURSOR) m_hIcon;
}


///////////////////////////////////////////////////////////////////
//
//	OnOK
//
///////////////////////////////////////////////////////////////////
void
CWWConfigDlg::OnOK (void) 
{
	GlobalExitValue=0;
	//
	//	Loop over each page and ask them to save themselves
	//
	for (int index = 0; index < ConfigPages.Count (); index ++) {
		ConfigPages[index]->SendMessage (WM_USER + 101);
	}

	CDialog::OnOK ();
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	OnSelchangeTabCtrl
//
///////////////////////////////////////////////////////////////////
void
CWWConfigDlg::OnSelchangeTabCtrl
(
	NMHDR *		/* pNMHDR */,
	LRESULT *	pResult
)
{
	(*pResult) = 0;

	//
	//	Check to see if the user has selected a new tab
	//
	int newtab = m_TabCtrl.GetCurSel ();
	if (CurrentTab != newtab) {
		
		//
		// Hide the old tab
		//
		if (CurrentTab < ConfigPages.Count () && ConfigPages[CurrentTab] != NULL) {
			ConfigPages[CurrentTab]->ShowWindow (SW_HIDE);
		}

		//
		// Show the new tab
		//
		if (ConfigPages[newtab] != NULL) {
			ConfigPages[newtab]->ShowWindow (SW_SHOW);
		}

		//
		// Remember what our new current tab is
		//
		CurrentTab = newtab;
	}

	return ;
}


void CWWConfigDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	GlobalExitValue=1;
	CDialog::OnCancel();
}
