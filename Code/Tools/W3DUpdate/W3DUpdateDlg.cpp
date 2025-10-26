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

// W3DUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "W3DUpdate.h"
#include "W3DUpdateDlg.h"
#include "FileCopyDialog.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	Data Tables
//
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
	FileCopyDialogClass *dialog;
	CString					src_dir;
	CString					dest_dir;
	bool						result;
} UPDATE_INFO;

typedef struct
{
	LPCTSTR	title;
	LPCTSTR	src;
	LPCTSTR	default_dir;
	LPCTSTR	reg_key;	
	UINT		ctrl_id;
	UINT		clean_ctrl_id;
	UINT		dir_ctrl_id;
	bool		clean_default;
	int		copy_to_app_dir1;
	int		copy_to_app_dir2;
} APP_INFO;

typedef enum
{
	APP_VIEWER		= 0,
	APP_PLUGIN,
	APP_WDUMP,
	APP_MAX4_PLUGIN,
	APP_RENEGADE_DAZZLES,
	APP_W3DSHELLEXT,
	APP_GMAXRENX,
	APP_MAX,
} APPLICATION_IDS;

const APP_INFO APPLICATIONS[APP_MAX] =
	// --- Title ---						---Source ---                            --Default Dir---                      ---RegKey---             ---Ctrl_ID---      ---Clean CtrlID--     --Dir CtrlID---       --Clean default- copy to appdir1, copy to appdir2
{
	{ "W3DViewer",				"\\\\cabal\\mis\\r&d\\w3d\\w3dview",				"c:\\w3d\\w3dview",						"W3DUpdate\\W3DView",		IDC_VIEWER_CHECK,	IDC_VIEWER_CLEAN_CHECK,	IDC_VIEWER_DIR_BUTTON,	true,		-1, -1 },
	{ "Max3 Plugin",			"\\\\cabal\\mis\\r&d\\w3d\\maxplugins\\max3",	"c:\\3dsmax3\\plugins\\westwood",	"W3DUpdate\\MaxPlugin30",	IDC_PLUGIN_CHECK,	IDC_PLUGIN_CLEAN_CHECK,	IDC_PLUGIN_DIR_BUTTON,	false,	-1, -1 },
	{ "WDump",					"\\\\cabal\\mis\\r&d\\w3d\\wdump",					"c:\\w3d\\wdump",							"W3DUpdate\\WDump",			IDC_WDUMP_CHECK,	IDC_WDUMP_CLEAN_CHECK,	IDC_WDUMP_DIR_BUTTON,	true,		-1, -1 },
	{ "Max4 Plugin",			"\\\\cabal\\mis\\r&d\\w3d\\maxplugins\\max4",	"c:\\3dsmax4\\plugins\\westwood",	"W3DUpdate\\MaxPlugin40",	IDC_MAX4_PLUGIN_CHECK,	IDC_MAX4_PLUGIN_CLEAN_CHECK,	IDC_MAX4_PLUGIN_DIR_BUTTON,	false,	-1, -1 },
	{ "Renegade Dazzles",	"\\\\mobius\\project7\\projects\\renegade\\asset management\\assets\\dazzle",	"",	"W3DUpdate\\Reneg Dazzle",	IDC_RENEGDAZZLE_CHECK,	-1,					-1							,	false,	APP_VIEWER, APP_PLUGIN },
	{ "W3D Shell Extension","\\\\cabal\\mis\\r&d\\w3d\\W3DShellExt",					"",									"W3DUpdate\\W3DShellExt",	IDC_W3DSHELLEXT,	-1							,	-1							,	false,	-1, -1 },
	{"Gmax Settings",			"\\\\cabal\\mis\\r&d\\w3d\\GmaxRenX",					"",										"W3DUpdate\\GmaxRenX",		IDC_GMAX_CHECK,	IDC_GMAX_CLEAN_CHECK2							,	-1							,	false,	-1,	-1}
};


const char * const INSTALL_REG_VALUE		= "Path";


/////////////////////////////////////////////////////////////////////////////
//
//	Local prototypes
//
/////////////////////////////////////////////////////////////////////////////
bool	Update_App (CWnd *parent_wnd, LPCTSTR title, LPCTSTR src_dir, LPCTSTR dest_dir);
bool	Clean_Directory (LPCTSTR local_dir);
bool	Get_Install_Directory (HWND hparent_wnd, LPCTSTR title, CString &folder);
bool	Delete_File (LPCTSTR filename);


/////////////////////////////////////////////////////////////////////////////
// CW3DUpdateDlg dialog

CW3DUpdateDlg::CW3DUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CW3DUpdateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CW3DUpdateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CW3DUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CW3DUpdateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CW3DUpdateDlg, CDialog)
	//{{AFX_MSG_MAP(CW3DUpdateDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CW3DUpdateDlg message handlers

BOOL CW3DUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, true);			// Set big icon
	SetIcon(m_hIcon, false);		// Set small icon

	//
	//	Check/uncheck the applications by default
	//
	for (int index = 0; index < APP_MAX; index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];
		HKEY hreg_key = NULL;
		CString reg_key_name;
		reg_key_name.Format ("Software\\Westwood Studios\\%s", app_info.reg_key);

		TCHAR path[MAX_PATH] = { 0 };


		//
		//	Is this application installed?
		//
		if (::RegOpenKeyEx (HKEY_CURRENT_USER, reg_key_name, 0L, KEY_READ, &hreg_key) == ERROR_SUCCESS) {
			//Moumine- 
			//SendDlgItemMessage (app_info.ctrl_id, BM_SETCHECK, (WPARAM)true);
			
			//
			//	Read the installation directory from the registry
			//
			DWORD size = sizeof (path);
			::RegQueryValueEx (hreg_key, INSTALL_REG_VALUE, 0L, NULL, (BYTE *)path, &size);
			::RegCloseKey (hreg_key);
		} else {
			::EnableWindow (::GetDlgItem (m_hWnd, app_info.clean_ctrl_id), false);
			::EnableWindow (::GetDlgItem (m_hWnd, app_info.dir_ctrl_id), false);
		}

		//
		//	Set the text of the install directory buttons
		//
		if (path[0] == 0) {
			::lstrcpy (path, app_info.default_dir);
		}
		
		if (app_info.dir_ctrl_id != -1) {
			SetDlgItemText (app_info.dir_ctrl_id, path);
		}

		//
		//	Check the clean option (by default) if necessary
		//
		if (app_info.clean_default && app_info.clean_ctrl_id != -1) {
			SendDlgItemMessage (app_info.clean_ctrl_id, BM_SETCHECK, (WPARAM)true);
		}
	}

	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CW3DUpdateDlg::OnPaint() 
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
HCURSOR CW3DUpdateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
//////////////////////////////////////////////////////////////////////////////////
void
CW3DUpdateDlg::OnOK (void)
{
	//
	//	Install/upgrade each application
	//
	bool success = true;
	for (int index = 0; (index < APP_MAX) && success; index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];

		//
		//	Does the user want to install this application?
		//
		if (SendDlgItemMessage (app_info.ctrl_id, BM_GETCHECK) == 1) {

			//
			//	Build a string containing the name registry key we will keep information
			// about this application in.
			//
			CString reg_key_name;
			reg_key_name.Format ("Software\\Westwood Studios\\%s", app_info.reg_key);

			//
			//	Should we install this application to the same directory as another
			// application in the list?
			//
			if (app_info.copy_to_app_dir1 != -1) {
				CString local_path;				

				//
				//	Install the application to the same directory as App1
				//
				Get_Destination_Path (app_info.copy_to_app_dir1, local_path);
				success = Install_App (app_info.title, app_info.src, local_path, reg_key_name, false);

				if (success) {
					if (app_info.copy_to_app_dir2 != -1) {
						
						//
						//	Install the application to the same directory as App2
						//
						Get_Destination_Path (app_info.copy_to_app_dir2, local_path);
						success = Install_App (app_info.title, app_info.src, local_path, reg_key_name, false);
					}
				}
				
				
			} else {

				//
				//	Do we need to clean the source directory for this application?
				//
				bool clean = (SendDlgItemMessage (app_info.clean_ctrl_id, BM_GETCHECK) == 1);

				//
				//	Get the path where this application is installed
				//
				CString local_path;
				Get_Destination_Path (index, local_path);
				
				//
				//	Install the application
				//
				success = Install_App (app_info.title, app_info.src, local_path, reg_key_name, clean);
			}
			if(index == APP_W3DSHELLEXT){ //Need more registration
				RegisterShellExt();
			}else{
				if(index == APP_VIEWER){
					//Set Viewer as default application 
					RegisterViewer(index);
				}
			}
		}
	}

	CDialog::OnOK ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Destination_Path
//
//////////////////////////////////////////////////////////////////////////////////
void
CW3DUpdateDlg::Get_Destination_Path (int app_id, CString &dest_path)
{
	//
	//	Read the destination directory from the dialog
	//
	switch(app_id){
		case APP_W3DSHELLEXT:{
			//SHGetFolderPath(NULL,CSIDL_SYSTEM, NULL,SHGFP_TYPE_CURRENT,dest_path.GetBuffer(MAX_PATH) );
			SHGetSpecialFolderPath(NULL,dest_path.GetBuffer(MAX_PATH), CSIDL_SYSTEM, true);
			dest_path.ReleaseBuffer();
			break;
		}
		case APP_GMAXRENX:{
			GetEnvironmentVariable("GMaxLoc", dest_path.GetBuffer(MAX_PATH),MAX_PATH);
			dest_path.ReleaseBuffer();
			CString str(MAKEINTRESOURCE(IDS_GMAXRENX_PATH));
			dest_path += str;
			break;
		}
		default:{
			GetDlgItemText (APPLICATIONS[app_id].dir_ctrl_id, dest_path);
			break;
		}
	}

	//
	// Ensure the path doesn't contain a terminating delimiter
	//
	if (dest_path[::lstrlen (dest_path) - 1] == '\\') {
		dest_path.Delete (::lstrlen (dest_path) - 1, 1);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Install_App
//
//////////////////////////////////////////////////////////////////////////////////
bool
CW3DUpdateDlg::Install_App
(
	const CString &title,
	const CString &src_path,
	const CString &dest_path,
	const CString &reg_key_name,
	bool				clean
)
{
	CWaitCursor wait_cursor;

	//
	//	Clean the old version if necessary
	//
	if (clean) {
		::Clean_Directory (dest_path);
	}
	
	//
	//	Update the application's files
	//
	bool ret_val = ::Update_App (this, title, src_path, dest_path);
	
	//
	//	Write the app's installation dir to the registry if we
	//	installed it correctly.
	//
	if (ret_val) {
		HKEY reg_key = NULL;
		ret_val &= (::RegCreateKeyEx (	HKEY_CURRENT_USER,
													reg_key_name,
													0L,
													NULL,
													REG_OPTION_NON_VOLATILE,
													KEY_ALL_ACCESS,
													NULL,
													&reg_key,
													NULL) == ERROR_SUCCESS);
		if (ret_val) {
			::RegSetValueEx (	reg_key,
									INSTALL_REG_VALUE,
									0L,
									REG_SZ,
									(BYTE *)(LPCTSTR)dest_path,
									dest_path.GetLength () + 1);
			::RegCloseKey (reg_key);
		}
	}

	return ret_val;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Install_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
Get_Install_Directory (HWND hparent_wnd, LPCTSTR title, CString &folder)
{
	bool ret_val = false;

	// Browse for the folder
	BROWSEINFO browse_info = { 0 };
	browse_info.hwndOwner = hparent_wnd;
	browse_info.lpszTitle = title;
	browse_info.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidl = ::SHBrowseForFolder (&browse_info);
	if (pidl != NULL) {
		
		// Convert the 'PIDL' into a string
		char path[MAX_PATH];
		ret_val = (::SHGetPathFromIDList (pidl, path) == true);
		if (ret_val) {
			folder = path;
		}

		// Free the 'PIDL'
		LPMALLOC pmalloc = NULL;
		if (SUCCEEDED (::SHGetMalloc (&pmalloc))) {
			pmalloc->Free (pidl);
			pmalloc->Release ();
		}
	}

	
	//CFileDialog dialog (true, ".pth", "test.pth", OFN_PATHMUSTEXIST | OFN_EXPLORER, "files *.*|*.*||", CWnd::FromHandle(hparent_wnd));
	//CFileDialog dialog (true, ".pth", "test.pth", OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLETEMPLATEHANDLE, "files *.*|*.*||", CWnd::FromHandle(hparent_wnd));
	//dialog.m_ofn.lpTemplateName = MAKEINTRESOURCE (IDD_DIR_SELECT_DIALOG);
	//dialog.m_ofn.hInstance = ::AfxGetInstanceHandle ();
	//HRSRC resource = ::FindResource (::AfxGetInstanceHandle (), MAKEINTRESOURCE (IDD_DIR_SELECT_DIALOG), RT_DIALOG);	
	//dialog.m_ofn.hInstance = (HINSTANCE)::LoadResource (::AfxGetInstanceHandle (), resource);

	//dialog.DoModal ();
	/*TCHAR path[MAX_PATH] = { 0 };

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof (ofn);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLETEMPLATE;
	ofn.hInstance = ::AfxGetInstanceHandle ();
	ofn.lpTemplateName = MAKEINTRESOURCE (IDD_DIR_SELECT_DIALOG);
	ofn.hwndOwner = hparent_wnd;
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof (path);
	GetOpenFileName (&ofn);
	::CommDlgExtendedError ();*/
	//ret_val = (dialog.DoModal () == IDOK);

	// Return the true/false result code
	return ret_val;
}


////////////////////////////////////////////////////////////////////////////
//
//  Strip_Filename_From_Path
//
////////////////////////////////////////////////////////////////////////////
CString
Strip_Filename_From_Path (LPCTSTR path)
{
	// Copy the path to a buffer we can modify
	TCHAR temp_path[MAX_PATH];
	::lstrcpy (temp_path, path);

	// Find the last occurance of the directory deliminator
	LPTSTR filename = ::strrchr (temp_path, '\\');
	if (filename != NULL) {
		// Strip off the filename
		filename[0] = 0;
	}

	// Return the path only
	return CString (temp_path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Delete_File
//
////////////////////////////////////////////////////////////////////////////
bool
Delete_File (LPCTSTR filename)
{
	// Assume failure
	bool ret_val = false;

	ASSERT (filename != NULL);
	if (filename != NULL) {

		// Strip the readonly bit off if necessary
		DWORD attributes = ::GetFileAttributes (filename);
		if ((attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)) {
			::SetFileAttributes (filename, attributes & (~FILE_ATTRIBUTE_READONLY));
		}

		// Perform the delete operation!
		ret_val = ::DeleteFile (filename) == true;
	}

	// Return the true/false result code
	return ret_val;
}


////////////////////////////////////////////////////////////////////////////
//
//  Copy_File
//
////////////////////////////////////////////////////////////////////////////
bool
Copy_File
(
	LPCTSTR existing_filename,
	LPCTSTR new_filename,
	bool bforce_copy
)
{
	// Assume failure
	bool ret_val = false;

	ASSERT (existing_filename != NULL);
	ASSERT (new_filename != NULL);
	if ((existing_filename != NULL) &&
		 (new_filename != NULL)) {

		// Make sure we aren't copying over ourselves
		bool allow_copy = (::lstrcmpi (existing_filename, new_filename) != 0);
		
		// Strip the readonly bit off if necessary
		DWORD attributes = ::GetFileAttributes (new_filename);
		if (allow_copy &&
		    (attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)) {
			if (bforce_copy) {
				::SetFileAttributes (new_filename, attributes & (~FILE_ATTRIBUTE_READONLY));
			} else {
				allow_copy = false;
			}
		}

		// Perform the copy operation!
		if (allow_copy) {
			ret_val = (::CopyFile (existing_filename, new_filename, false) == true);
		}
	}

	// Return the true/false result code
	return ret_val;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Clean_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
Clean_Directory (LPCTSTR local_dir)
{
	bool ret_val = true;
	// Build a search mask from the directory
	CString search_mask = CString (local_dir) + "\\*.*";
	// Loop through all the files in this directory and add them
	// to our list
	WIN32_FIND_DATA find_info = { 0 };
	HANDLE hfind = ::FindFirstFile (search_mask, &find_info);
	if(hfind){
		while(FindNextFile(hfind, &find_info)){
			// If this file isn't a directory, add it to the list
			if(!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				CString full_path = local_dir + CString ("\\") + find_info.cFileName;
				if (::Delete_File (full_path) == false) {
					CString message;
					message.Format ("Cannot delete %s.  This may result in an incomplete update.  Please make sure no applications are running before running the update.", full_path);
					::MessageBox (NULL, message, "Delete Error", MB_SETFOREGROUND | MB_TOPMOST | MB_ICONEXCLAMATION | MB_OK);
					ret_val = false;
				}
			}else{
				if( find_info.cFileName[0] != '.'){
					CString new_dir =  CString(local_dir) + "\\" + CString(find_info.cFileName);
					Clean_Directory(new_dir.GetBuffer(new_dir.GetLength()));
					new_dir.ReleaseBuffer();
					RemoveDirectory(new_dir.GetBuffer(new_dir.GetLength()));
					new_dir.ReleaseBuffer();
				}
			}
		}
		::FindClose (hfind);
	}
	return ret_val;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create_Dir_If_Necessary
//
//////////////////////////////////////////////////////////////////////////////////
void
Create_Dir_If_Necessary (LPCTSTR path)
{
	if (::GetFileAttributes (path) == 0xFFFFFFFF) {
		Create_Dir_If_Necessary (::Strip_Filename_From_Path (path));
		::CreateDirectory (path, NULL);
	}
	
	return ;
}
//////////////////////////////////////////////////////////////////////////////////
//
//	fnUpdateAppDirectory
//
//////////////////////////////////////////////////////////////////////////////////
UINT
fnUpdateAppDirectory (LPVOID pParam)
{
	static call_depth(0);
	UPDATE_INFO *info	= (UPDATE_INFO *)pParam;
	ASSERT (info != NULL);
	if (info != NULL) {
		info->result = true;

		// Build a search mask from the directory
		CString search_mask = info->src_dir + "\\*.*";
		
		// Loop through all the files in this directory and add them
		// to our list
		WIN32_FIND_DATA find_info = { 0 };
		HANDLE hfind = ::FindFirstFile (search_mask, &find_info);
		if(hfind){
			while(FindNextFile(hfind, &find_info)){
				// If this file isn't a directory, add it to the list
				if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					CString src_path = info->src_dir + CString ("\\") + find_info.cFileName;
					CString dest_path = info->dest_dir + CString ("\\") + find_info.cFileName;
					::Create_Dir_If_Necessary (info->dest_dir);
					//info->dialog->Set_Current_File (find_info.cFileName);
					if (::Copy_File (src_path, dest_path, true) == false) {
						CString message;
						message.Format (IDS_COPYFAIL, src_path, dest_path);
						::MessageBox (NULL, message, "Copy Error", MB_SETFOREGROUND | MB_TOPMOST | MB_ICONEXCLAMATION | MB_OK);
						info->result = false;
					}
				}else{
					if( find_info.cFileName[0] != '.'){
						UPDATE_INFO info2 = *info;
						CString folder = "\\";
						folder += find_info.cFileName;
						info2.dest_dir +=  folder;
						info2.src_dir += folder;
						call_depth ++;
						fnUpdateAppDirectory(&info2);
						call_depth --;
					}
				}
			}
			::FindClose (hfind);
		}
	}
	if(!call_depth){
		// Kill the updating dialog
		::PostMessage(info->dialog->m_hWnd, WM_USER + 101, 0, 0L);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	Update_App
//
//////////////////////////////////////////////////////////////////////////////////
bool
Update_App
(
	CWnd *	parent_wnd,
	LPCTSTR	title,
	LPCTSTR	src_dir,
	LPCTSTR	dest_dir
)
{
	CWaitCursor wait_cursor;

	FileCopyDialogClass dialog (parent_wnd);
	dialog.Set_Current_Application (title);

	//
	//	Create the destination directory if necessary
	//
	if (::GetFileAttributes (dest_dir) == 0xFFFFFFFF) {
		::CreateDirectory (dest_dir, NULL);
	}
	
	//
	//	Kick off a worker thread to do the actual file copy
	//
	UPDATE_INFO *info	= new UPDATE_INFO;
	info->dialog		= &dialog;
	info->src_dir		= src_dir;
	info->dest_dir		= dest_dir;
	::AfxBeginThread (fnUpdateAppDirectory, (LPVOID)info);
	// Kill the updating dialog
	::PostMessage( dialog.m_hWnd, WM_USER + 101, 0, 0L);
	
	//
	//	Display a dialog to monitor the progress
	//
	dialog.DoModal ();
	bool ret_val = info->result;
	delete info;
	return ret_val;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnCommand
//
//////////////////////////////////////////////////////////////////////////////////
BOOL
CW3DUpdateDlg::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	for (int index = 0; index < APP_MAX; index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];

		if (LOWORD (wParam) == app_info.ctrl_id) {
			
			//
			//	Enable or disable the clean and directory buttons
			//
			bool enable = (SendDlgItemMessage (LOWORD(wParam), BM_GETCHECK) == 1);
			::EnableWindow (::GetDlgItem (m_hWnd, app_info.clean_ctrl_id), enable);
			::EnableWindow (::GetDlgItem (m_hWnd, app_info.dir_ctrl_id), enable);
			
		} else if (LOWORD (wParam) == app_info.dir_ctrl_id) {
			
			//
			//	Change the installation directoy
			//
			CString directory;
			CString dlg_title;
			dlg_title.Format ("Select a directory to install the %s to.", app_info.title);
			if (Get_Install_Directory (m_hWnd, dlg_title, directory)) {
				SetDlgItemText (app_info.dir_ctrl_id, directory);
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnDefaults
//
//////////////////////////////////////////////////////////////////////////////////
void
CW3DUpdateDlg::OnDefaults (void)
{
	for (int index = 0; index < APP_MAX; index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];

		if (SendDlgItemMessage (app_info.ctrl_id, BM_GETCHECK) != 0) {
			SetDlgItemText (app_info.dir_ctrl_id, app_info.default_dir);
		}
	}
	
	return ;
}

void CW3DUpdateDlg::RegisterShellExt()
{
	HKEY reg_key;
	CString reg_key_name(MAKEINTRESOURCE(IDS_SHELLEXT_CLSID));
	CString reg_value(MAKEINTRESOURCE(IDS_SHELLEXT_NAME));
	CString value_name;
	//[HKEY_CLASSES_ROOT\CLSID\{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}]
	DWORD ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//   @="W3D Shell Extension"
		ret_val = ::RegSetValueEx(reg_key,"",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	//[HKEY_CLASSES_ROOT\CLSID\{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}\InProcServer32]
	::RegCloseKey (reg_key);
	reg_key_name += "\\InProcServer32";
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val){
		//@="W3Dshellext.dll"
		reg_value.LoadString(IDS_SHELLEXT_DLLNAME);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
		//"ThreadingModel"="Apartment"
		reg_value.LoadString(IDS_W3DSHELLEXT_APARTMENT);
		value_name.LoadString(IDS_W3DSHELLEXT_THMODEL);
		ret_val = ::RegSetValueEx(reg_key, value_name,0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\.w3d]
	reg_key_name.LoadString(IDS_W3DFILEEXT);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//@="W3DFile"
		reg_value.LoadString(IDS_W3DFILETEXT);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile]
	reg_key_name.LoadString(IDS_W3DFILEKEY);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
   //@="Shell Extension file"
		reg_value.LoadString(IDS_W3DSHELLEXT_TEXT);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile\shellex\ContextMenuHandlers]
	reg_key_name.LoadString(IDS_CTXMENUHANDLERS);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//@="W3DCtxMenu"
		reg_value.LoadString(IDS_W3DMENU_TEXT);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile\shellex\ContextMenuHandlers\W3DCtxMenu]
	reg_key_name.LoadString(IDS_W3DCTXMENU_KEY);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//@="{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}"
		reg_value.LoadString(IDS_W3DSHELLEXT_GUID);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile\shellex\PropertySheetHandlers]
	reg_key_name.LoadString(IDS_W3DPROPSHEETHANDLERS);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//@="W3DPropertyPage"
		reg_value.LoadString(IDS_W3DPROPPAGE_TEXT);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile\shellex\PropertySheetHandlers\W3DPropertyPage]
	reg_key_name.LoadString(IDS_W3DPROPPAGE_KEY);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
   //@="{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}"
		reg_value.LoadString(IDS_W3DSHELLEXT_GUID);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved]
	reg_key_name.LoadString(IDS_MSEXTENSIONS_KEY);
	ret_val = ::RegCreateKeyEx (HKEY_CURRENT_USER,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
   //"{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}"="W3D Shell Extension"
		value_name.LoadString(IDS_W3DSHELLEXT_GUID);
		reg_value.LoadString(IDS_W3DSHELLEXT_TEXT);
		ret_val = ::RegSetValueEx(reg_key, value_name,0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	::RegCloseKey (reg_key);
	//[HKEY_CLASSES_ROOT\W3DFile\\DefaultIcon
	reg_key_name.LoadString(IDS_W3DDEFAULTICON_KEY);
	ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
   //%SystemRoot%\\system32\\W3DShellExt.dll,0
		reg_value.LoadString(IDS_W3DDEFAULTICON_TEXT);
		ret_val = ::RegSetValueEx(reg_key, "",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST ,NULL,NULL);	
}

void CW3DUpdateDlg::RegisterViewer(int index){
	HKEY reg_key;
	CString reg_key_name(MAKEINTRESOURCE(IDS_OPENWITH));
	CString dest_path;
	Get_Destination_Path(index, dest_path);
	CString reg_value(MAKEINTRESOURCE(IDS_VIEWERCOMMAND));
	reg_value = dest_path + reg_value; 
	//[HKEY_CLASSES_ROOT\CLSID\{556F8779-49C4-4e88-9CEF-0AC2CFD6B763}]
	DWORD ret_val = ::RegCreateKeyEx (HKEY_CLASSES_ROOT,reg_key_name,0L,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&reg_key,NULL);
	if (ERROR_SUCCESS == ret_val) {
		//   @="W3D Shell Extension"
		ret_val = ::RegSetValueEx(reg_key,"",0L,REG_SZ,(BYTE *)(LPCTSTR)reg_value,reg_value.GetLength () + 1);
	}

}
