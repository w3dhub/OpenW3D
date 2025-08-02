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

// LevelEdit.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "leveledit.h"

#include "mainfrm.h"
#include "leveleditdoc.h"
#include "leveleditview.h"
#include "assetmgr.h"
#include "ww3d.h"
#include "wwdebug.h"
#include "utils.h"
#include "editorassetmgr.h"
#include "_assetmgr.h"
#include "wwphystrig.h"
#include "regkeys.h"
#include "_wwaudio.h"
#include "win.h"
#include "sceneeditor.h"
#include "editorsaveload.h"
#include "generatingvisdialog.h"
#include "splash.h"
#include "ffactory.h"
#include "wwsaveload.h"
#include "wwmath.h"
#include "wwphys.h"
#include "translatedb.h"
#include "presetmgr.h"
#include "phys.h"
#include "utils.h"
#include "colorutils.h"
#include "assetpackagemgr.h"
#include "editorbuild.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
void	Register_Light_Icon (void);


/////////////////////////////////////////////////////////////////////////////
// CLevelEditApp

BEGIN_MESSAGE_MAP(CLevelEditApp, CWinApp)
	//{{AFX_MSG_MAP(CLevelEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelEditApp construction

CLevelEditApp::CLevelEditApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLevelEditApp object

CLevelEditApp theApp;


/////////////////////////////////////////////////////////////////////////////
//
// fnMessage_Print_Func
//
/////////////////////////////////////////////////////////////////////////////
void
fnMessage_Print_Func (DebugType /*type*/, const char *message)
{
	::OutputDebugString (message);
	::Output_Message (message);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnAssert_Print_Func
//
/////////////////////////////////////////////////////////////////////////////
void
fnAssert_Print_Func (const char *message)
{
	::Output_Message (message);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnTrigger_Func
//
/////////////////////////////////////////////////////////////////////////////
bool
fnTrigger_Func (int trigger_num)
{
	// Assume not triggered
	bool retval = false;

	// Are we checking for inverted 'vis'?
	if (trigger_num == WWPHYS_TRIGGER_INVERT_VIS) {
		retval = bool(::GetKeyState (VK_F9) < 0);
	}

	// Return the true/false result code
	return retval;
}


bool File_Exists (LPCSTR filename)
{
	WIN32_FIND_DATA find_info;
   HANDLE file_handle = ::FindFirstFile (filename, &find_info);
	
	if (file_handle != INVALID_HANDLE_VALUE) {
		::FindClose (file_handle);
		return true;
	} else {
		return false;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// InitInstance
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CLevelEditApp::InitInstance (void)
{
	//
	//	Get the installation directory of this application
	//
	TCHAR install_path[MAX_PATH] = { 0 };
	::GetModuleFileName (::AfxGetInstanceHandle (), install_path, sizeof (install_path));
	if (install_path[::lstrlen(install_path)-1] == '\\') {
		install_path[::lstrlen(install_path)-1] = 0;
	}

	_pThe3DAssetManager = new EditorAssetMgrClass;

	//
	//	Let the user select which asset package they wish to work with
	//
	AssetPackageMgrClass::Initialize ();
	AssetPackageMgrClass::Show_Package_Selection_UI ();

	//
	//	Display the splash screen
	//
	SplashClass splash_screen;
	splash_screen.Show (true);

	//
	//	Initialize the libraries
	//
	WWMath::Init ();
	WWPhys::Init ();
	WWSaveLoad::Init ();
	TranslateDBClass::Initialize ();	

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	RegisterColorPicker (::AfxGetInstanceHandle ());
	RegisterColorBar (::AfxGetInstanceHandle ());

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Westwood Studios"));	

	//
	//	Handle the command line
	//
	LPCTSTR cmd_line = ::strstr (m_lpCmdLine, "-j=");
	if (cmd_line != NULL) {
		cmd_line += 3;

		Set_Silent_Mode (true);
	}


	LoadStdProfileSettings(9);  // Load standard INI file options (including MRU)

	::WWDebug_Install_Message_Handler (fnMessage_Print_Func);
	::WWDebug_Install_Assert_Handler (fnAssert_Print_Func);
	::WWDebug_Install_Trigger_Handler (fnTrigger_Func);

	//
	//	Disable the 3DFX logo
	//
	_putenv ("FX_GLIDE_NO_SPLASH=1");

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

#ifdef PUBLIC_EDITOR_VER

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME2,
		RUNTIME_CLASS(CLevelEditDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CLevelEditView));
	AddDocTemplate(pDocTemplate);

#else

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLevelEditDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CLevelEditView));
	AddDocTemplate(pDocTemplate);

#endif //PUBLIC_EDITOR_VER

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	ProgramInstance	= m_hInstance;
	MainWindow			= m_pMainWnd->m_hWnd;	
	PresetMgrClass::Initialize ();

	//
	//	Update the installation key in the registry
	//
	const char *INSTALL_KEY		= "Software\\Westwood Studios\\LevelEdit\\Install";
	const char *INSTALL_VALUE	= "Exe";
	HKEY reg_key = NULL;
	if (::RegCreateKeyEx (	HKEY_CURRENT_USER,
									INSTALL_KEY,
									0L,
									NULL,
									0L,
									KEY_ALL_ACCESS,
									NULL,
									&reg_key,
									NULL) == ERROR_SUCCESS)
	{		
		::RegSetValueEx (	reg_key,
								INSTALL_VALUE,
								0L,
								REG_SZ,
								(BYTE *)install_path,
								::lstrlen (install_path) + 1);
		::RegCloseKey (reg_key);
	}

	//
	//	Register an icon with the light file
	//
	Register_Light_Icon ();

	//
	//	Handle the command line
	//
	if (cmd_line != NULL) {
		::Perform_Job (cmd_line, true);
		Set_Modified (false);
		m_pMainWnd->PostMessage (WM_CLOSE);
	}

	return TRUE;
}

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
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CLevelEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CLevelEditApp commands

/*
**
*/
void Debug_Refs(void)
{
#ifndef NDEBUG
	TRACE("Detecting Active Refs...\r\n");
   //ODS("At time %s", cMiscUtil::Get_Text_Time());
	RefCountNodeClass * first = RefCountClass::ActiveRefList.First();
	RefCountNodeClass * node = first;
	while (node->Is_Valid())
	{
		RefCountClass * obj = node->Get();
		ActiveRefStruct * ref = &(obj->ActiveRefInfo);

		bool display = true;
		int	count = 0;
		RefCountNodeClass * search = first;
		while (search->Is_Valid()) {

			if (search == node) {	// if this is not the first one
				if (count != 0) {
					display = false;
					break;
				}
			}

			RefCountClass * search_obj = search->Get();
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
			TRACE ( "%d Active Ref: %s %d %p\n", count, ref->File,ref->Line,obj);

			static int num_printed = 0;
			if (++num_printed > 20) {
				TRACE( "And Many More......\n");
				break;
			}
		}

		node = node->Next();
	}
	TRACE("Done.\r\n");
   //ODS("At time %s", cMiscUtil::Get_Text_Time());
#endif
}


/////////////////////////////////////////////////////////////
//
//	ExitInstance
//
int
CLevelEditApp::ExitInstance (void)
{
	// Remove the debug handlers we installed earlier
	::WWDebug_Install_Message_Handler (NULL);
	::WWDebug_Install_Assert_Handler (NULL);
	::WWDebug_Install_Trigger_Handler (NULL);

	//
	// Free the audio manager
	//
	SAFE_DELETE (_pTheAudioManager);	

	//
	//	Shutdown the libraries
	//
	WWMath::Shutdown ();
	WWPhys::Shutdown ();
	WWSaveLoad::Shutdown ();
	TranslateDBClass::Shutdown ();
	PresetMgrClass::Shutdown ();

	//
	// Check for active refs
	//
	Debug_Refs ();
	
	// Allow the base class to process this message
	return CWinApp::ExitInstance();
}


/////////////////////////////////////////////////////////////
//
//	OnFileOpen
//
void
CLevelEditApp::OnFileOpen (void) 
{
	CFileDialog dialog (TRUE,
							  ".lvl",
							  NULL,
							  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
							  "Level Files|*.lvl||",
							  ::AfxGetMainWnd ());
	
	//
	//	Setup the initial directory
	//
	CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_SAVE_DIR_VALUE);
	if (full_path.GetLength () != 0) {
		dialog.m_ofn.lpstrInitialDir = full_path;
	}

	//
	//	Let the user pick a file
	//
	if (dialog.DoModal () == IDOK) {
		OpenDocumentFile (dialog.GetPathName ());
	}

	// Determine which directory we should use as a default for the open
	/*CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_SAVE_DIR_VALUE);
	if (full_path.GetLength () != 0) {
		::SetCurrentDirectory (full_path);
		::Get_Current_Document ()->Set_Current_Path (full_path);
	}

	// Allow the base class to process this message
	CWinApp::OnFileOpen ();*/
	return ;	
}


/////////////////////////////////////////////////////////////
//
//	Register_Light_Icon
//
/////////////////////////////////////////////////////////////
void
Register_Light_Icon (void)
{
	DWORD disp		= 0;
	HKEY reg_key	= NULL;

	const char *LIGHT_ICON_APP_NAME	= "W3D Light";
	const char *LIGHT_ICON_KEY			= "W3D Light\\DefaultIcon";

	//
	//	Open the file extension association registry key
	//
	if (::RegCreateKeyEx (	HKEY_CLASSES_ROOT,
									".wlt",
									0,
									NULL,
									REG_OPTION_NON_VOLATILE,
									KEY_ALL_ACCESS,
									NULL,
									&reg_key,
									&disp) == ERROR_SUCCESS)
	{
		//
		//	Write the name of the application under this extension's default value
		//
		::RegSetValueEx (reg_key, NULL, 0, REG_SZ, (const BYTE *)LIGHT_ICON_APP_NAME, ::lstrlen (LIGHT_ICON_APP_NAME) + 1);
		::RegCloseKey (reg_key);

		//
		//	Open the application key
		//
		if (::RegCreateKeyEx (	HKEY_CLASSES_ROOT,
										LIGHT_ICON_KEY,
										0,
										NULL,
										REG_OPTION_NON_VOLATILE,
										KEY_ALL_ACCESS,
										NULL,
										&reg_key,
										&disp) == ERROR_SUCCESS)
		{
			//
			//	Build a string representing the path to the level editor and
			// the icon's index.
			//
			TCHAR path[MAX_PATH] = { 0 };
			::GetModuleFileName (NULL, path, MAX_PATH);

			CString icon_path;
			icon_path.Format ("%s,-%d", path, IDI_LIGHT);

			//
			//	Write the icon's location under this key
			//
			::RegSetValueEx (reg_key, NULL, 0, REG_SZ, (const BYTE *)(LPCTSTR)icon_path, ::lstrlen (icon_path) + 1);
			::RegCloseKey (reg_key);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	PreTranslateMessage
//
//	This is a hack to ensure that translator keys are
// processed even if we are in one of the sidebar windows.
//
/////////////////////////////////////////////////////////////
BOOL
CLevelEditApp::PreTranslateMessage (MSG *pMsg) 
{
	BOOL retval = false;

	//
	//	Is this window registered for global translation?
	//
	if (pMsg->hwnd != NULL && ::GetProp (pMsg->hwnd, "TRANS_ACCS") == (HANDLE)1) {

		//
		//	Translate the accelerator
		//
		CMainFrame *main_wnd = (CMainFrame *)::AfxGetMainWnd ();
		if (main_wnd !=  NULL) {
			HACCEL accel_table = main_wnd->GetDefaultAccelerator ();
			if (::TranslateAccelerator (main_wnd->m_hWnd, accel_table, pMsg)) {
				retval = true;
			}		
		}
	} 
	
	if (retval == false) {
		retval = CWinApp::PreTranslateMessage (pMsg);
	}
		
	return retval;
}


