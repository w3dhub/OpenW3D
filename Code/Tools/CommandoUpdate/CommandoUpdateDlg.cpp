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

// CommandoUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CommandoUpdate.h"
#include "CommandoUpdateDlg.h"
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
	bool						is_recursive;
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
	bool		is_recursive;
} APP_INFO;

typedef enum
{
	APP_EDITOR		= 0,
	APP_GAME_ENGINE,
	APP_GAME_LEVELS,
	APP_GAME_MOVIES,
	APP_LIGHTMAP,
	APP_VISFARM,
	APP_MIXVIEW,
	APP_MULTIPLAY,
	APP_MAX,
} APPLICATION_IDS;

const APP_INFO APPLICATIONS[APP_MAX] =
{
	{ "Level Editor", "\\\\mobius\\project7\\projects\\renegade\\programming\\tools\\level edit",	"c:\\commando\\leveledit",	"LevelEdit\\Install",	IDC_EDITOR_CHECK,			IDC_EDITOR_CLEAN_CHECK,		IDC_EDITOR_DIR_BUTTON,		true,		false },
	{ "Game Engine",	"\\\\havoc\\rock\\projects\\renegade\\asset management\\current build",			"c:\\commando\\run",			"Commando\\Install",		IDC_GAME_CHECK,			IDC_GAME_CLEAN_CHECK,		IDC_GAME_DIR_BUTTON,			false,	true },
	{ "Game Levels",	"\\\\havoc\\rock\\projects\\renegade\\asset management\\current levels",		"c:\\commando\\run",			"Commando\\Install",		IDC_GAME_LEVELS_CHECK,	IDC_GAME_CLEAN_CHECK,		IDC_GAME_DIR_BUTTON,			false,	true },
	{ "Game Movies",	"\\\\havoc\\rock\\projects\\renegade\\asset management\\cinematics",				"c:\\commando\\run",			"Commando\\Install",		IDC_GAME_MOVIES_CHECK,	IDC_GAME_CLEAN_CHECK,		IDC_GAME_DIR_BUTTON,			false,	true },
	{ "Lightmap",		"\\\\mobius\\project7\\projects\\renegade\\programming\\tools\\lightmap",		"c:\\commando\\lightmap",	"LightMap\\Install",		IDC_LIGHTMAP_CHECK,		IDC_LIGHTMAP_CLEAN_CHECK,	IDC_LIGHTMAP_DIR_BUTTON,	true,		false },
	{ "Vis Farm",		"\\\\mobius\\project7\\projects\\renegade\\programming\\tools\\vis farm",		"c:\\commando\\vis farm",	"VisFarm\\Install",		IDC_VISFARM_CHECK,		IDC_VISFARM_CLEAN_CHECK,	IDC_VISFARM_DIR_BUTTON,		false,	false },
	{ "Mix Viewer",	"\\\\mobius\\project7\\projects\\renegade\\programming\\tools\\mixview",		"c:\\commando\\mixview",	"MixView\\Install",		IDC_MIXVIEW_CHECK,		IDC_MIXVIEW_CLEAN_CHECK,	IDC_MIXVIEW_DIR_BUTTON,		false,	false },
	{ "Multiplay",		"\\\\tanya\\game\\projects\\renegade\\run",												"c:\\renegade\\mplay",		"MPlay\\Install",			IDC_MULTIPLAY_CHECK,		IDC_MULTIPLAY_CLEAN_CHECK,	IDC_MULTIPLAY_DIR_BUTTON,	false,	true }
};


const char * const INSTALL_REG_VALUE		= "Path";
const char * const DATA_SUB_DIR				= "\\Data";
const char * const MOVIE_SUB_DIR				= "\\Data\\Movies";
const char * const SRDLL_SUB_DIR				= "\\SrDLL";


/////////////////////////////////////////////////////////////////////////////
//
//	Local prototypes
//
/////////////////////////////////////////////////////////////////////////////
bool	Update_App (CWnd *parent_wnd, LPCTSTR title, LPCTSTR src_dir, LPCTSTR dest_dir, bool is_recursive);
bool	Clean_Directory (LPCTSTR local_dir, bool is_recursive);
bool	Get_Install_Directory (HWND hparent_wnd, LPCTSTR title, CString &folder);
bool	Delete_File (LPCTSTR filename);


/////////////////////////////////////////////////////////////////////////////
// CCommandoUpdateDlg dialog

CCommandoUpdateDlg::CCommandoUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommandoUpdateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommandoUpdateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommandoUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandoUpdateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCommandoUpdateDlg, CDialog)
	//{{AFX_MSG_MAP(CCommandoUpdateDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandoUpdateDlg message handlers

BOOL CCommandoUpdateDlg::OnInitDialog()
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
			SendDlgItemMessage (app_info.ctrl_id, BM_SETCHECK, (WPARAM)true);
			
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
		SetDlgItemText (app_info.dir_ctrl_id, path);

		//
		//	Check the clean option (by default) if necessary
		//
		if (app_info.clean_default) {
			SendDlgItemMessage (app_info.clean_ctrl_id, BM_SETCHECK, (WPARAM)true);
		}
	}

	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCommandoUpdateDlg::OnPaint() 
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
HCURSOR CCommandoUpdateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
//////////////////////////////////////////////////////////////////////////////////
void
CCommandoUpdateDlg::OnOK (void)
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
			HKEY hreg_key = NULL;
			CString reg_key_name;
			reg_key_name.Format ("Software\\Westwood Studios\\%s", app_info.reg_key);

			//
			//	Get the path where this application is installed
			//
			CString local_path;
			GetDlgItemText (app_info.dir_ctrl_id, local_path);

			// Ensure the path doesn't contain a terminating delimiter
			if (local_path[::lstrlen (local_path) - 1] == '\\') {
				local_path.Delete (::lstrlen (local_path) - 1, 1);
			}			

			if (success) {
				CWaitCursor wait_cursor;

				//
				//	Clean the old version if necessary
				//
				if (SendDlgItemMessage (app_info.clean_ctrl_id, BM_GETCHECK) == 1) {									

					//
					//	Special case the "game engine only" option.  Note: we only want
					// to clean the data subdir if we aren't cleaning the game directory
					//
					if (index == APP_GAME_LEVELS || index == APP_GAME_MOVIES) {
						if (SendDlgItemMessage (APPLICATIONS[APP_GAME_ENGINE].ctrl_id, BM_GETCHECK) == 0) {
							
							if (index == APP_GAME_MOVIES) {
								CString data_dir = local_path + MOVIE_SUB_DIR;
								::Clean_Directory (data_dir, app_info.is_recursive);
							} else {
								CString data_dir = local_path + DATA_SUB_DIR;
								::Clean_Directory (data_dir, app_info.is_recursive);
							}
						}
					} else {
						::Clean_Directory (local_path, app_info.is_recursive);
					}
				}
								
				//
				//	Special case the "game levels" option
				//
				if (index == APP_GAME_LEVELS) {
					CString data_dir		= local_path + DATA_SUB_DIR;
					CString src_data_dir	= app_info.src;
					CString data_title	= "Game Levels";
					success &= ::Update_App (this, data_title, src_data_dir, data_dir, app_info.is_recursive);
				} else if (index == APP_GAME_MOVIES) {
					CString data_dir		= local_path + MOVIE_SUB_DIR;
					CString src_data_dir	= app_info.src;
					CString data_title	= "Game Movies";
					success &= ::Update_App (this, data_title, src_data_dir, data_dir, app_info.is_recursive);
				} else {
				
					//
					//	Update the application's files
					//
					success &= ::Update_App (this, app_info.title, app_info.src, local_path, app_info.is_recursive);					
				}
				
				//
				//	Write the app's installation dir to the registry if we
				//	installed it correctly.
				//
				if (success) {
					success &= (::RegCreateKeyEx (	HKEY_CURRENT_USER,
																reg_key_name,
																0L,
																NULL,
																REG_OPTION_NON_VOLATILE,
																KEY_ALL_ACCESS,
																NULL,
																&hreg_key,
																NULL) == ERROR_SUCCESS);
					if (success) {
						::RegSetValueEx (	hreg_key,
												INSTALL_REG_VALUE,
												0L,
												REG_SZ,
												(BYTE *)(LPCTSTR)local_path,
												local_path.GetLength () + 1);
						::RegCloseKey (hreg_key);
					}
				}
			}
		}
	}
	
	CDialog::OnOK ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Install_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
Get_Install_Directory (HWND hparent_wnd, LPCTSTR title, CString &folder)
{
	bool retval = false;

	// Browse for the folder
	BROWSEINFO browse_info = { 0 };
	browse_info.hwndOwner = hparent_wnd;
	browse_info.lpszTitle = title;
	browse_info.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidl = ::SHBrowseForFolder (&browse_info);
	if (pidl != NULL) {
		
		// Convert the 'PIDL' into a string
		char path[MAX_PATH];
		retval = (::SHGetPathFromIDList (pidl, path) == true);
		if (retval) {
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
	//retval = (dialog.DoModal () == IDOK);

	// Return the true/false result code
	return retval;
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
	bool retval = false;

	ASSERT (filename != NULL);
	if (filename != NULL) {

		// Strip the readonly bit off if necessary
		DWORD attributes = ::GetFileAttributes (filename);
		if ((attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY))
		{
			::SetFileAttributes (filename, attributes & (~FILE_ATTRIBUTE_READONLY));
		}

		// Perform the delete operation!
		if ((attributes != 0xFFFFFFFF) &&
			 ((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
		{
			retval = ::RemoveDirectory (filename) == true;
		} else {
			retval = ::DeleteFile (filename) == true;
		}
		
	}

	// Return the true/false result code
	return retval;
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
	bool retval = false;

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
			retval = (::CopyFile (existing_filename, new_filename, false) == true);
		}
	}

	// Return the true/false result code
	return retval;
}


__inline void Delimit_Path (CString &path)
{
	if (path[::lstrlen (path) - 1] != '\\') {
		path += CString ("\\");
	}
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Clean_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
Clean_Directory (LPCTSTR local_dir, bool is_recursive)
{
	bool retval = true;

	// Build a search mask from the directory
	CString search_mask = CString (local_dir) + "\\*.*";
	
	// Loop through all the files in this directory and add them
	// to our list
	CStringList file_list;
	BOOL bcontinue = true;
	WIN32_FIND_DATA find_info = { 0 };
	for (HANDLE hfind = ::FindFirstFile (search_mask, &find_info);
		  (hfind != INVALID_HANDLE_VALUE) && bcontinue;
		  bcontinue = ::FindNextFile (hfind, &find_info)) {
		
		// If this file isn't a directory, add it to the list
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			CString filename = find_info.cFileName;
			file_list.AddTail (filename);
		} else if (is_recursive && find_info.cFileName[0] != '.') {
			
			//
			//	Recurse into this subdirectory
			//
			CString full_path = local_dir;
			::Delimit_Path (full_path);
			full_path += find_info.cFileName;
			Clean_Directory (full_path, is_recursive);
			
			//
			//	Add this directory to the list so it will get
			// deleted with the files...
			//
			CString filename = find_info.cFileName;
			file_list.AddTail (filename);
		}
	}

	// Close the search handle
	if (hfind != NULL) {
		::FindClose (hfind);
	}
	
	//
	//	Now loop through all the files and delete them
	//
	for (POSITION pos = file_list.GetHeadPosition (); pos != NULL; ) {
		CString &filename = file_list.GetNext (pos);
		CString full_path = local_dir + CString ("\\") + filename;
		if (::Delete_File (full_path) == false) {
			CString message;
			message.Format ("Cannot delete %s.  This may result in an incomplete update.  Please make sure no applications are running before running the update.", full_path);
			::MessageBox (NULL, message, "Delete Error", MB_SETFOREGROUND | MB_TOPMOST | MB_ICONEXCLAMATION | MB_OK);
			retval = false;
		}
	}

	return retval;
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
//	Build_File_List
//
//////////////////////////////////////////////////////////////////////////////////
void
Build_File_List (LPCTSTR search_path, CStringList &file_list)
{
	//
	// Loop through all the files in this directory and add them
	// to our list
	//
	BOOL keep_going				= true;
	WIN32_FIND_DATA find_info	= { 0 };

	for (HANDLE hfind = ::FindFirstFile (search_path, &find_info);
		  (hfind != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (hfind, &find_info))
	{		
		//
		// If this file isn't a directory, add it to the list
		//
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			CString filename = find_info.cFileName;
			file_list.AddTail (filename);
		}
	}

	// Close the search handle
	if (hfind != NULL) {
		::FindClose (hfind);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Quick_Compare_Files
//
////////////////////////////////////////////////////////////////////////////
int
Quick_Compare_Files
(
	LPCTSTR file1,
	LPCTSTR file2
)
{
	// Assume they are the same
	int compare = 0;

	// Params OK?
	ASSERT (file1 != NULL);
	ASSERT (file2 != NULL);
	if ((file1 != NULL) && (file2 != NULL)) {

		// Attempt to open file1
		HANDLE hfile1 = ::CreateFile (file1,
											   0,
											   0,
											   NULL,
											   OPEN_EXISTING,
											   0L,
											   NULL);

		// Attempt to open file2
		HANDLE hfile2 = ::CreateFile (file2,
											   0,
											   0,
											   NULL,
											   OPEN_EXISTING,
											   0L,
											   NULL);

		if ((hfile1 != INVALID_HANDLE_VALUE) && (hfile2 != INVALID_HANDLE_VALUE)) {

			// Get information about these 2 files.
			BY_HANDLE_FILE_INFORMATION file1_info = { 0 };
			BY_HANDLE_FILE_INFORMATION file2_info = { 0 };			
			::GetFileInformationByHandle (hfile1, &file1_info);
			::GetFileInformationByHandle (hfile2, &file2_info);

			// Compare the time these files were last written to
			compare = ::CompareFileTime (&file1_info.ftLastWriteTime, &file2_info.ftLastWriteTime);
		} else if ((hfile1 == INVALID_HANDLE_VALUE) && (hfile2 != INVALID_HANDLE_VALUE)) {
			compare = -1;
		} else if ((hfile1 != INVALID_HANDLE_VALUE) && (hfile2 == INVALID_HANDLE_VALUE)) {
			compare = 1;
		}
		
		if (hfile1 != INVALID_HANDLE_VALUE) {
			::CloseHandle (hfile1);
		}

		if (hfile2 != INVALID_HANDLE_VALUE) {
			::CloseHandle (hfile2);
		}
	}

	// Same as strcmp, -1 if file1 is older than file2, 0 if equal, 1 if file1 is newer than file2
	return compare;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	FileNeedsCopying
//
//////////////////////////////////////////////////////////////////////////////////
bool
FileNeedsCopying (LPCTSTR src_path, LPCTSTR dest_path)
{
	return (Quick_Compare_Files (src_path, dest_path) != 0);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Update_App_Directory
//
//////////////////////////////////////////////////////////////////////////////////
bool
Update_App_Directory
(
	FileCopyDialogClass *	dialog,
	LPCTSTR						src_dir,
	LPCTSTR						dest_dir,
	bool							is_recursive
)
{
	bool retval = true;

	//
	// Build search masks from the src and dest directories
	//
	CString remote_search_mask	= src_dir;
	remote_search_mask += "\\*.*";

	//
	// Loop through all the files in this directory and add them
	// to our list
	//
	BOOL keep_going				= true;
	WIN32_FIND_DATA find_info	= { 0 };
	CStringList file_list;

	for (HANDLE hfind = ::FindFirstFile (remote_search_mask, &find_info);
		  (hfind != INVALID_HANDLE_VALUE) && keep_going && retval;
		  keep_going = ::FindNextFile (hfind, &find_info))
	{		
		//
		// If this file isn't a directory, add it to the list
		//
		if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			CString filename = find_info.cFileName;
			file_list.AddTail (filename);
		} else if (is_recursive && find_info.cFileName[0] != '.') {
			
			//
			//	Recurse into this subdirectory
			//
			CString full_src_path	= src_dir;
			CString full_dest_path	= dest_dir;			
			::Delimit_Path (full_src_path);
			::Delimit_Path (full_dest_path);
			full_src_path	+= find_info.cFileName;
			full_dest_path	+= find_info.cFileName;
			
			retval &= ::Update_App_Directory (	dialog,
															full_src_path,
															full_dest_path,
															is_recursive);
		}
	}

	// Close the search handle
	if (hfind != NULL) {
		::FindClose (hfind);
	}
	
	//
	//	Now loop through all the files and copy them to the src
	//
	for (POSITION pos = file_list.GetHeadPosition (); pos != NULL && retval; ) {

		CString &filename = file_list.GetNext (pos);

		CString src_file	= src_dir + CString ("\\") + filename;
		CString dest_file	= dest_dir + CString ("\\") + filename;			
		
		//
		//	Make sure the destination directory exists
		//
		::Create_Dir_If_Necessary (dest_dir);

		//
		//	Do a file comparison to determine whether or not to copy the file
		//
		if (::FileNeedsCopying (src_file, dest_file)) {

			//
			//	Update the dialog
			//
			dialog->Set_Current_File (filename);

			//
			//	Copy the file
			//
			if (::Copy_File (src_file, dest_file, true) == false) {
				CString message;
				message.Format ("Cannot copy %s to %s.  Please make sure no applications are running before running the update.", src_file, dest_file);
				::MessageBox (NULL, message, "Copy Error", MB_SETFOREGROUND | MB_TOPMOST | MB_ICONEXCLAMATION | MB_OK);
				retval = false;
			}
		}
	}
	
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	fnUpdateAppDirectory
//
//////////////////////////////////////////////////////////////////////////////////
UINT
fnUpdateAppDirectory (LPVOID pParam)
{
	UPDATE_INFO *info	= (UPDATE_INFO *)pParam;
	ASSERT (info != NULL);
	if (info != NULL) {
				
		//
		//	Begin updating this directory...
		//
		info->result = ::Update_App_Directory (	info->dialog,
																info->src_dir,
																info->dest_dir,
																info->is_recursive);

		//
		// Kill the updating dialog
		//
		::PostMessage (info->dialog->m_hWnd, WM_USER + 101, 0, 0L);
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
	LPCTSTR	dest_dir,
	bool		is_recursive
)
{
	CWaitCursor wait_cursor;

	FileCopyDialogClass dialog (parent_wnd);
	dialog.Set_Current_Application (title);
	
	//
	//	Kick off a worker thread to do the actual file copy
	//
	UPDATE_INFO *info		= new UPDATE_INFO;
	info->dialog			= &dialog;
	info->src_dir			= src_dir;
	info->dest_dir			= dest_dir;
	info->is_recursive	= is_recursive;
	::AfxBeginThread (fnUpdateAppDirectory, (LPVOID)info);
	
	//
	//	Display a dialog to monitor the progress
	//
	dialog.DoModal ();
	bool retval = info->result;
	delete info;

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnCommand
//
//////////////////////////////////////////////////////////////////////////////////
BOOL
CCommandoUpdateDlg::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	bool found = false;
	for (int index = 0; (index < APP_MAX) && (found == false); index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];

		if (LOWORD (wParam) == app_info.ctrl_id) {

			bool enable = (SendDlgItemMessage (LOWORD(wParam), BM_GETCHECK) == 1);

			if (index == APP_GAME_ENGINE) {
				enable |= (SendDlgItemMessage (APPLICATIONS[APP_GAME_LEVELS].ctrl_id, BM_GETCHECK) == 1);
				enable |= (SendDlgItemMessage (APPLICATIONS[APP_GAME_MOVIES].ctrl_id, BM_GETCHECK) == 1);
			} else if (index == APP_GAME_LEVELS || index == APP_GAME_MOVIES) {
				enable |= (SendDlgItemMessage (APPLICATIONS[APP_GAME_ENGINE].ctrl_id, BM_GETCHECK) == 1);
			}

			//
			//	Enable or disable the clean and directory buttons
			//
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

			found = true;
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
CCommandoUpdateDlg::OnDefaults (void)
{
	for (int index = 0; index < APP_MAX; index ++) {
		const APP_INFO &app_info = APPLICATIONS[index];

		if (SendDlgItemMessage (app_info.ctrl_id, BM_GETCHECK) != 0) {
			SetDlgItemText (app_info.dir_ctrl_id, app_info.default_dir);
		}
	}
	
	return ;
}

