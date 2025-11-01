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

// CopyLockedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CopyLocked.h"
#include "CopyLockedDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyLockedDlg dialog

CCopyLockedDlg::CCopyLockedDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyLockedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyLockedDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCopyLockedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyLockedDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCopyLockedDlg, CDialog)
	//{{AFX_MSG_MAP(CCopyLockedDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyLockedDlg message handlers

BOOL CCopyLockedDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, true);			// Set big icon
	SetIcon(m_hIcon, false);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return true;  // return true  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCopyLockedDlg::OnPaint() 
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
HCURSOR CCopyLockedDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LPCTSTR _filenames[] = {
"Msvcrtd.dll",
"Mfc42d.dll",
"Msvcp50.dll",
"Msvcp50d.dll",
"Msvcp60.dll",
"Msvcp60d.dll",
"Msvcrt.dll",
"Mfc42.dll" };


void
CCopyLockedDlg::OnOK() 
{
	// TODO: Add extra validation here

	CWaitCursor wait_cursor;

	TCHAR temp_path[MAX_PATH];
	::GetTempPath (sizeof (temp_path), temp_path);

	for (int index = 0; index < sizeof (_filenames)/sizeof (LPCTSTR); index ++) {

		HRSRC hresource = ::FindResource (::AfxGetInstanceHandle (), _filenames[index], "FILE");
		HGLOBAL hglobal = ::LoadResource (::AfxGetInstanceHandle (), hresource);
		LPVOID pbuffer = ::LockResource (hglobal);
		if(pbuffer != NULL) {

			CString full_path = CString (temp_path);
			if (temp_path[::lstrlen (temp_path)-1] != '\\') {
				full_path += "\\";
			}			 
			full_path += CString (_filenames[index]);

			HANDLE hfile = ::CreateFile (full_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				DWORD dwbyteswritten = 0L;
				DWORD dwsizeofres = ::SizeofResource (::AfxGetInstanceHandle (), hresource);
				::WriteFile (hfile, pbuffer, dwsizeofres, &dwbyteswritten, NULL);
				::CloseHandle (hfile);

				if (dwbyteswritten == dwsizeofres) {
					TCHAR system_path[MAX_PATH];
					::GetSystemDirectory (system_path, sizeof (system_path));
					
					CString new_path = CString (system_path);
					if (new_path[::lstrlen (new_path)-1] != '\\') {
						new_path += "\\";
					}			 
					new_path += CString (_filenames[index]);

					::MoveFileEx (full_path, new_path, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING);
					::MoveFileEx (full_path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
	}

	::MessageBox (m_hWnd, "In order to complete the install, you need to reboot your machine.", "Reboot", MB_OK | MB_ICONEXCLAMATION);
		//::ExitWindowsEx (EWX_REBOOT, 0);
	
	
	CDialog::OnOK();
	return ;
}
