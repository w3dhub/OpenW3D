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

// PhysTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PhysTest.h"

#include "MainFrm.h"
#include "PhysTestDoc.h"
#include "GraphicView.h"
#include "ww3d.h"
#include "assetmgr.h"
#include "refcount.h"
#include "wwdebug.h"
#include "wwphystrig.h"
#include "srRuntimeClass.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const char * LOGFILE_NAME				= "_logfile";
const char * LOGFILE_EXTENSION		= "txt";

/////////////////////////////////////////////////////////////////////////////
//
//	Local prototypes
//
BOOL CALLBACK fnTopLevelWindowSearch (HWND hwnd, LPARAM lParam);
void Debug_Refs(void);

void init_logfile(void);
void write_to_logfile(const char * string);
void wwdebug_message_handler(DebugType type,const char * message);
void wwdebug_assert_handler(const char * message);
bool wwdebug_trigger_handler(int trigger_num);


/////////////////////////////////////////////////////////////////////////////
// CPhysTestApp

BEGIN_MESSAGE_MAP(CPhysTestApp, CWinApp)
	//{{AFX_MSG_MAP(CPhysTestApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhysTestApp construction

CPhysTestApp::CPhysTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPhysTestApp object

CPhysTestApp theApp;




/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

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


BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

// App command to run the dialog
void CPhysTestApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CPhysTestApp initialization

BOOL CPhysTestApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Is there already an instance of the viewer running?
	HWND hprev_instance = NULL;
	::EnumWindows (fnTopLevelWindowSearch, (LPARAM)&hprev_instance);
	if (hprev_instance == NULL) {

		// Change the registry key under which our settings are stored.
		// You should modify this string to be something appropriate
		// such as the name of your company or organization.
		SetRegistryKey(_T("Westwood Studios"));

		LoadStdProfileSettings();  // Load standard INI file options (including MRU)

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		CSingleDocTemplate* pDocTemplate;
		pDocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CPhysTestDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CGraphicView));
		AddDocTemplate(pDocTemplate);

		// Parse command line for standard shell commands, DDE, file open
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		// Dispatch commands specified on the command line
		if (!ProcessShellCommand(cmdInfo))
			return false;

		// The one and only window has been initialized, so show and update it.
		m_pMainWnd->ShowWindow(SW_SHOW);
		m_pMainWnd->UpdateWindow();

	} else {

		// Make the previous instance in the foreground
		::ShowWindow (hprev_instance, SW_NORMAL);
		::BringWindowToTop (hprev_instance);
		::SetForegroundWindow (hprev_instance);
	}

	// Create the log file
   init_logfile();
	
	// Install message handler functions for the WWDebug messages
	// and assertion failures.
	WWDebug_Install_Message_Handler(wwdebug_message_handler);
	WWDebug_Install_Assert_Handler(wwdebug_assert_handler);
	//WWDebug_Install_Trigger_Handler(wwdebug_trigger_handler);

	WWDEBUG_SAY(("Logging Begin:\r\n"));
	
	return (hprev_instance == NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CPhysTestApp message handlers


int CPhysTestApp::ExitInstance() 
{
	// Free any resources the WW3D engine allocated
	WW3DAssetManager::Get_Instance()->Free_Assets();
	if (WW3D::Is_Initted()) {
		WW3D::Shutdown();
	}

	// Remove message handler functions for the WWDebug messages
	// and assertion failures.
	WWDebug_Install_Message_Handler(NULL);
	WWDebug_Install_Assert_Handler(NULL);
	//WWDebug_Install_Trigger_Handler(NULL);

	// Check Active Refs
	Debug_Refs();
	
	return CWinApp::ExitInstance();
}




//////////////////////////////////////////////////////////////////////////////
//
//	fnTopLevelWindowSearch
//
BOOL CALLBACK
fnTopLevelWindowSearch
(
	HWND hwnd,
	LPARAM lParam
)
{
	BOOL bcontinue = true;

	// Is this a viewer window?
	if (::GetProp (hwnd, "WW3DVIEWER") != 0) {
		bcontinue = false;
		(*((HWND *)lParam)) = hwnd;
	}

	// Return the true/false result code
	return bcontinue;
}

//////////////////////////////////////////////////////////////////////////////
//
//	Debug_Refs()
//
void Debug_Refs(void)
{
#ifndef NDEBUG
	RefBaseNodeClass * first = RefBaseClass::ActiveRefList.First();
	RefBaseNodeClass * node = first;
	while (node->Is_Valid())
	{
		RefBaseClass * obj = node->Get();
		ActiveRefStruct * ref = &(obj->ActiveRefInfo);

		bool display = true;
		int	count = 0;
		RefBaseNodeClass * search = first;
		while (search->Is_Valid()) {

			if (search == node) {	// if this is not the first one
				if (count != 0) {
					display = false;
					break;
				}
			}

			RefBaseClass * search_obj = search->Get();
			ActiveRefStruct * search_ref = &(search_obj->ActiveRefInfo);

			if ( ref->File && search_ref->File &&
				  !strcmp(search_ref->File, ref->File) &&
				  (search_ref->Line == ref->Line) ) {
				count++;
			} else if ( (ref->File == NULL) &&  (search_ref->File == NULL) ) {
				count++;
			}

			search = search->Next();
		}

		if ( display ) {
			char buf[256];
			sprintf(buf,"%d Active Ref: %s %d %p\n",count, ref->File,ref->Line,obj);

			::MessageBox(NULL,buf,"Unreleased Object!",MB_OK);

			static int num_printed = 0;
			if (++num_printed > 20) {
				::MessageBox(NULL,"And Many More......\n",NULL,MB_OK);
				break;
			}
		}

		node = node->Next();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
//	get_log_filename
//
const char * get_log_filename(void)
{
	static char log_name[_MAX_PATH];

	char exe_name[_MAX_PATH];
	::GetModuleFileName(::AfxGetInstanceHandle(),exe_name,sizeof(exe_name));
 
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath(exe_name,drive,dir,NULL,NULL);

	_makepath(log_name,drive,dir,LOGFILE_NAME,LOGFILE_EXTENSION);
	return log_name;
}

//////////////////////////////////////////////////////////////////////////////
//
//	init_logfile
//
void init_logfile(void)
{
   // Destroy contents
	FILE * file = fopen(get_log_filename(), "w");
	fclose(file);
}

//////////////////////////////////////////////////////////////////////////////
//
//	write_to_logfile
//
void write_to_logfile(const char * string)
{
   // open/close for each write so as to maximize integrity of this file.
   FILE * file = fopen(get_log_filename(), "a");
   if (file != NULL) {
		fwrite(string, 1, strlen(string), file);
	   fclose(file);
   }
}

//////////////////////////////////////////////////////////////////////////////
//
//	wwdebug_message_handler
//
void wwdebug_message_handler(DebugType type,const char * message)
{
	/*
	** dump the message into our log file
	*/
	write_to_logfile(message);
}

//////////////////////////////////////////////////////////////////////////////
//
//	wwdebug assert handler
//
void wwdebug_assert_handler(const char * message)
{
	/*
	** dump the assert into our log file
	*/
	write_to_logfile(message);

	/*
	** break into the debugger
	*/
	_asm int 0x03;
}

//////////////////////////////////////////////////////////////////////////////
//
//	wwdebug trigger handler
// Disables VIS!
//
bool wwdebug_trigger_handler(int trigger_num)
{
	switch( trigger_num ) 
	{
		case WWPHYS_TRIGGER_DISABLE_VIS:				return true;
	}
	return false;
}


