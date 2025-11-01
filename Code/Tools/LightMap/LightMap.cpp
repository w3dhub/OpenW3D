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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : LightMap                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tool $* 
 *                                                                                             * 
 *                      $Author:: Ian_l               $* 
 *                                                                                             * 
 *                     $Modtime:: 7/19/01 8:13p       $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "LightMapView.h"
#include "MainFrm.h"
#include "verchk.h"
#include <stdlib.h>
#include <direct.h>

//	Static functions.
BOOL CALLBACK TopLevelWindowSearch (HWND hwnd, LPARAM lParam);

// The following is maintained by MFC tools.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(LightMapApp, CWinApp)
	//{{AFX_MSG_MAP(LightMapApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// The one and only LightMapApp object
LightMapApp theApp;


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***********************************************************************************************
 * LightMapApp:: --																									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapApp::LightMapApp()
{
	// NOTE: Place all significant initialization in InitInstance
}


/***********************************************************************************************
 * LightMapApp:: --																									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
BOOL LightMapApp::InitInstance()
{
	char computername [MAX_COMPUTERNAME_LENGTH + 1];
	char processidname [33];

	DWORD computernamesize;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// specific initialization routines you do not need.

	Do_Version_Check();

	// Set the working path. 
	_getdcwd (0, WorkingPath, sizeof (WorkingPath));
	strcat (WorkingPath, "\\");

	// Set the instance name.
	computernamesize = sizeof (computername);
	GetComputerName (computername, &computernamesize);
	itoa (GetCurrentProcessId(), processidname, 16); 
	strcpy (InstanceName, computername);
	strcat (InstanceName, processidname);

	// Version 0.1 by default.
	ApplicationVersion = 1;

	// Get the name and path of the currently executing application.
	TCHAR filename [MAX_PATH];
	::GetModuleFileName (NULL, filename, sizeof (filename));

	// Get the version information for this application.
	DWORD dummy_var = 0;
	DWORD version_size = ::GetFileVersionInfoSize (filename, &dummy_var);
	if (version_size > 0) {

		// Get the file version block
		LPBYTE pblock = new BYTE [version_size];
		if (::GetFileVersionInfo (filename, 0L, version_size, pblock)) {

			// Query the block for the file version information.
			UINT version_len = 0;
			VS_FIXEDFILEINFO *pversion_info = NULL;
			if (::VerQueryValue (pblock, "\\", (LPVOID*) &pversion_info, &version_len)) {
				ApplicationVersion = pversion_info->dwFileVersionMS;
			}
		}
		delete [] pblock;
	}

	SetRegistryKey ("Westwood Studios");
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	// serve as the connection between documents, frame windows and views.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(LightMapDoc),
		RUNTIME_CLASS(CMainFrame),       
		RUNTIME_CLASS(LightMapView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open.
	CCommandLineInfo cmdInfo;

	ParseCommandLine (cmdInfo);

	EnableShellOpen();
	RegisterShellFileTypes (true);

	// Dispatch commands specified on the command line.
	if (!ProcessShellCommand (cmdInfo)) return (false);
	
	m_pMainWnd->DragAcceptFiles();

	return (true);
}


/***********************************************************************************************
 * LightMapApp::Do_Version_Check --	 Check if this version of lightmap is newer or older than  *
 *												 a version at a designated location on the network and if  *
 *												 older then report this fact to user.							  *
 *																															  *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightMapApp::Do_Version_Check()
{	
	const char *networkpath		= "\\\\Mobius\\Project7\\Projects\\Renegade\\Programming\\Tools\\Lightmap";
	const char *newversiontext = "There is a newer version of the Lightmap tool. Please run 'Renegade Update' to update your local copy.";

	char pathname [_MAX_PATH];
	char filename [_MAX_FNAME];
	char extension [_MAX_EXT];
	
	::GetModuleFileName (NULL, pathname, MAX_PATH);
	_splitpath (pathname, NULL, NULL, filename, extension);
	strcpy (pathname, "\\\\Mobius\\Project7\\Projects\\Renegade\\Programming\\Tools\\Lightmap\\");
	strcat (pathname, filename);
	strcat (pathname, extension);

	if (Compare_EXE_Version (::AfxGetInstanceHandle(), pathname) < 0) {
		::MessageBox (NULL, newversiontext, "Version Information", MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
	}
}


/***********************************************************************************************
 * LightMapApp::TopLevelWindowSearch --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/15/00    IML : Created.                                                                *
 *=============================================================================================*/
BOOL CALLBACK TopLevelWindowSearch (HWND hwnd, LPARAM lParam)
{
	BOOL bcontinue = true;

	// Is this a viewer window?
	if (::GetProp (hwnd, "LIGHTMAP") != 0) {
		bcontinue = false;
		(*((HWND*)lParam)) = hwnd;
	}

	return (bcontinue);
}


/***********************************************************************************************
 * LightMapApp:: --																									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
LightMapDoc* LightMapApp::GetDoc() 
{
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate *pDocTemplate = GetNextDocTemplate (pos);

	pos = pDocTemplate->GetFirstDocPosition();
	LightMapDoc* pDoc = (LightMapDoc*)pDocTemplate->GetNextDoc (pos);
	ASSERT (pDoc->IsKindOf(RUNTIME_CLASS(LightMapDoc)));

	return (pDoc);
}


/***********************************************************************************************
 * LightMapApp:: --																									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	
	aboutDlg.DoModal();
}


/***********************************************************************************************
 * LightMapApp::OnFileOpen --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightMapApp::OnFileOpen() 
{
	CFileDialog dialog (true, NULL, NULL, File_Dialog_Flags(), File_Dialog_Filter());

	if (dialog.DoModal() == IDOK) {
		if (GetDoc()->OnOpenDocument (dialog.GetPathName())) {
			GetDoc()->SetPathName (dialog.GetPathName());
			GetDoc()->UpdateAllViews (NULL);
		}
	}
}


/***********************************************************************************************
 * LightMapApp::Explorer_Style --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
bool LightMapApp::Explorer_Style()
{
	OSVERSIONINFO info;

	info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	::GetVersionEx (&info);
 	return ((info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) || ((info.dwPlatformId == VER_PLATFORM_WIN32_NT) && (info.dwMajorVersion >= 4)));
}


/***********************************************************************************************
 * LightMapApp::File_Dialog_Flags --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
DWORD LightMapApp::File_Dialog_Flags()
{
	DWORD	flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (Explorer_Style()) flags |= OFN_EXPLORER;
	return (flags);
}


/***********************************************************************************************
 * CAboutDlg::OnInitDialog --																						  *		
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
BOOL CAboutDlg::OnInitDialog() 
{
	// Allow the base class to process this message.
	CDialog::OnInitDialog();
	
	// Put the version string into the dialog.
	CString version_string;
	version_string.Format ("Version %d.%d", (theApp.Application_Version() >> 16), (theApp.Application_Version() & 0xffff));
	GetDlgItem (IDC_VERSION_TEXT)->SetWindowText (version_string);
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}
