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

/////////////////////////////////////////////////////////////////////////////
//
//  Utils.CPP
//
//  Module containing usefull misc. utility functions
//


#include "StdAfx.h"
#include "mainfrm.h"
#include "leveleditdoc.h"
#include "LevelEditView.h"
#include "LevelEdit.h"
#include "Utils.h"
#include "outputform.h"
#include "sceneeditor.h"
#include "nodemgr.h"
#include "node.h"
#include "groupmgr.h"
#include "updatingdbdialog.h"
#include "icons.h"
#include "regkeys.h"
#include "presetmgr.h"
#include "HLod.h"
#include "AABox.h"
#include "definition.h"
#include "verchk.h"
#include "editorsaveload.h"
#include "generatingvisdialog.h"
#include "boxrobj.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "nodecategories.h"
#include "conversationpage.h"


////////////////////////////////////////////////////////////////////////////
//
//  Get_Current_Document
//
////////////////////////////////////////////////////////////////////////////
CLevelEditDoc *
Get_Current_Document (void)
{
   // Assume failure
	CLevelEditDoc *pdoc = NULL;

   // Get a pointer to the main window
	CMainFrame *pmainwnd = (CMainFrame *)theApp.GetMainWnd ();
    
	// Did we successfully get a pointer to the main window?
	if (pmainwnd != NULL) {
		// Use the main window pointer to get a pointer
		// to the current doc.
		//pdoc = (CLevelEditDoc *)pmainwnd->GetActiveDocument ();
		pdoc = pmainwnd->Get_Current_Document ();
		ASSERT (pdoc);
	}

	// Return the doc pointer
	return pdoc;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Node_Mgr
//
////////////////////////////////////////////////////////////////////////////
NodeMgrClass &
Get_Node_Mgr (void)
{
	// Return the node manager pointer
	return _TheNodeMgr;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Scene_Editor
//
////////////////////////////////////////////////////////////////////////////
SceneEditorClass *
Get_Scene_Editor (void)
{
	SceneEditorClass *peditor = NULL;
	CLevelEditDoc *pdoc  = ::Get_Current_Document ();
	
	// Were we successful in getting the document pointer?
	ASSERT (pdoc != NULL);
	if (pdoc != NULL) { 
		
		// Get a pointer to the editor
		peditor = pdoc->Get_Scene ();
	}

	// Return the editor pointer
	return peditor;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Camera_Mgr
//
////////////////////////////////////////////////////////////////////////////
CameraMgr *
Get_Camera_Mgr (void)
{
	CameraMgr *pcameramgr = NULL;

	// Get a pointer to the current view
	CLevelEditView *pview  = ::Get_Main_View ();
	
	// Were we successful in getting the view pointer?
	ASSERT (pview != NULL);
	if (pview != NULL) { 
		// Get a pointer to the camera maanager
		pcameramgr = pview->Get_Camera_Mgr ();
	}

	// Return the camera manager pointer
	return pcameramgr;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Mouse_Mgr
//
////////////////////////////////////////////////////////////////////////////
MouseMgrClass *
Get_Mouse_Mgr (void)
{
	MouseMgrClass *pmousemgr = NULL;

	// Get a pointer to the current view
	CLevelEditView *pview  = ::Get_Main_View ();
	
	// Were we successful in getting the view pointer?
	ASSERT (pview != NULL);
	if (pview != NULL) { 
		// Get a pointer to the mouse maanager
		pmousemgr = pview->Get_Mouse_Mgr ();
	}

	// Return the mouse manager pointer
	return pmousemgr;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Selection_Mgr
//
////////////////////////////////////////////////////////////////////////////
SelectionMgrClass &
Get_Selection_Mgr (void)
{
	// Return the selction manager pointer
	return ::Get_Scene_Editor ()->Get_Selection_Mgr ();
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Main_View
//
////////////////////////////////////////////////////////////////////////////
CLevelEditView *
Get_Main_View (void)
{
	// Assume failure
	CLevelEditView *pview = NULL;

   // Get a pointer to the main window
	CMainFrame *pmainwnd = (CMainFrame *)theApp.GetMainWnd ();
    
	// Did we successfully get a pointer to the main window?
	ASSERT (pmainwnd != NULL);
	if (pmainwnd != NULL) {
		// Use the main window pointer to get a pointer
		// to the current view.
		pview = (CLevelEditView *)pmainwnd->GetActiveView ();
		ASSERT (pview != NULL);
	}

	// Return the view pointer
	return pview;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_File_Mgr
//
////////////////////////////////////////////////////////////////////////////
FileMgrClass *
Get_File_Mgr (void)
{
	FileMgrClass *pfilemgr = NULL;

	// Get a pointer to the current document
	CLevelEditDoc *pdoc  = ::Get_Current_Document ();
	
	// Were we successful in getting the document pointer?
	ASSERT (pdoc != NULL);
	if (pdoc != NULL) { 
		// Get a pointer to the file maanager
		pfilemgr = pdoc->Get_File_Mgr ();
	}

	// Return the file manager pointer
	return pfilemgr;
}


////////////////////////////////////////////////////////////////////////////
//
//  Refresh_Main_View
//
////////////////////////////////////////////////////////////////////////////
void
Refresh_Main_View (void)
{
	// Force the view to repaint itself,
	// but only if auto updating is turned off
	CLevelEditView *pview = Get_Main_View ();
	if ((pview != NULL) &&
	    (pview->Is_Updating () == false) &&
		 ::IsWindow (pview->m_hWnd)) {
		pview->Repaint_View ();
		pview->Repaint_View ();
	}
	
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Paint_Gradient
//
////////////////////////////////////////////////////////////////////////////
void
Paint_Gradient
(
    HWND hwnd,
    BYTE base_red,
    BYTE base_green,
    BYTE base_blue
)
{
	// Get the bounding rectangle so we know how much to paint
	RECT rect;
	::GetClientRect (hwnd, &rect);

	// Determine the width, height, and width per each shade
	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;
	float width_per_shade = ((float)width) / 256.00F;

	// Pull a hack to get the CDC for the window
	HDC hdc = ::GetDC (hwnd);
	CDC cdc;
	cdc.Attach (hdc);

	// Loop through each shade and paint its sliver
	float posx = 0.00F;
	for (int shade = 0; shade < 256; shade ++) {       
		// Paint this sliver
		cdc.FillSolidRect ((int)posx,
								 0,
								 (width_per_shade >= 1.00F) ? (int)width_per_shade : 1,
								 height,
								 RGB (shade*base_red, shade*base_green, shade*base_blue));

		// Increment the current position
		posx += width_per_shade;
	}

	// Release the DC
	cdc.Detach ();    
	::ReleaseDC (hwnd, hdc);

	// Validate the contents of the window so the control won't paint itself
	::ValidateRect (hwnd, NULL);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Startup_Directory
//
////////////////////////////////////////////////////////////////////////////
CString
Get_Startup_Directory (void)
{
	TCHAR path[MAX_PATH];
	::GetModuleFileName (NULL, path, sizeof (path));

	LPTSTR filename = ::strrchr (path, '\\');
	if (filename)
	{
		// Strip ff the filename
		filename[0] = 0;
	}

	// Return the path to the caller
	return CString (path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Data_Directory
//
////////////////////////////////////////////////////////////////////////////
CString
Get_Data_Directory (void)
{
	// Get the path we were run from
	CString path = ::Get_Startup_Directory ();

	// Concat the 'data' subdir onto that path
	path += "\\Data";

	// Return the path to the caller
	return path;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Filename_From_Path
//
////////////////////////////////////////////////////////////////////////////
CString
Get_Filename_From_Path (LPCTSTR path)
{
	// Find the last occurance of the directory deliminator
	LPCTSTR filename = ::strrchr (path, '\\');
	if (filename != NULL) {
		// Increment past the directory deliminator
		filename ++;
	} else {
		filename = path;
	}

	// Return the filename part of the path
	return CString (filename);
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
	} else if ((temp_path[1] != ':') && (temp_path[1] != '\\')) {
		temp_path[0] = 0;
	}

	// Return the path only
	return CString (temp_path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Subdir_From_Full_Path
//
////////////////////////////////////////////////////////////////////////////
CString
Get_Subdir_From_Full_Path (LPCTSTR path)
{
	int last_delim = 0;
	int prev_delim = 0;

	for (int index = 0; path[index] != 0; index++) {
		if (path[index] == '\\') {
			prev_delim = last_delim;
			last_delim = index + 1;
		}
	}

	// Return the sub-dir name
	CString sub_dir = &path[prev_delim];
	sub_dir = sub_dir.Left ((last_delim-prev_delim)-1);
	return sub_dir;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Subdir_And_Filename_From_Path
//
////////////////////////////////////////////////////////////////////////////
CString
Get_Subdir_And_Filename_From_Path (LPCTSTR path)
{
	int last_delim = 0;
	int prev_delim = 0;

	for (int index = 0; path[index] != 0; index++) {
		if (path[index] == '\\') {
			prev_delim = last_delim;
			last_delim = index + 1;
		}
	}

	// Return the sub-dir and filename
	CString sub_dir = &path[prev_delim];
	return sub_dir;
}


////////////////////////////////////////////////////////////////////////////
//
//  Up_One_Directory
//
////////////////////////////////////////////////////////////////////////////
CString
Up_One_Directory (LPCTSTR path)
{
	// Copy the path to a buffer we can modify
	TCHAR local_copy[MAX_PATH];
	::lstrcpy (local_copy, path);

	// Strip off the last subdir in the path
	LPTSTR plast_dir = ::strrchr (local_copy, '\\');
	if (plast_dir != NULL) {
		plast_dir[0] = 0;
	}

	// Return the new path
	return CString (local_copy);
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_File_Size
//
////////////////////////////////////////////////////////////////////////////
DWORD
Get_File_Size (LPCTSTR path)
{
	DWORD file_size = 0L;

	ASSERT (path != NULL);
	if (path != NULL) {
		
		// Attempt to open the file
		HANDLE hfile = ::CreateFile (path,
											  0,
											  0,
											  NULL,
											  OPEN_EXISTING,
											  0L,
											  NULL);
		
		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			// Get the size in bytes of the file and return it
			// to the caller.
			file_size = ::GetFileSize (hfile, NULL);

			// Close the file
			::CloseHandle (hfile);
			hfile = INVALID_HANDLE_VALUE;
		}		
	}

	// Return the size in bytes of the file to the caller.
	return file_size;
}


////////////////////////////////////////////////////////////////////////////
//
//  Output_Message
//
////////////////////////////////////////////////////////////////////////////
void
Output_Message (LPCTSTR message)
{
   // Get a pointer to the main window
	CMainFrame *pmainwnd = (CMainFrame *)theApp.GetMainWnd ();
    
	// Did we successfully get a pointer to the main window?
	ASSERT (pmainwnd != NULL);
	if (pmainwnd != NULL) {
		FormToolbarClass &toolbar = pmainwnd->Get_Output_Toolbar ();

		// Get a pointer to the output window
		OutputFormClass *form = (OutputFormClass *)toolbar.Get_Form ();

		// Print the message to the window
		if ((form != NULL) && IsWindow (form->m_hWnd)) {			
			form->Output_Message (message);
		}
	}

	TRACE (message);			
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Path_Relative
//
////////////////////////////////////////////////////////////////////////////
bool
Is_Path_Relative (LPCTSTR path)
{
	bool relative = false;

	// Param valid?
	ASSERT (path != NULL);
	if (path != NULL && path[0] != 0) {
		
		// Check for drive designation
		relative = bool(path[1] != ':');

		// Check for network path
		relative &= bool((path[0] != '\\') && (path[1] != '\\'));
	}

	// Return the true/false result code
	return relative;
}


////////////////////////////////////////////////////////////////////////////
//
//  Find_File
//
////////////////////////////////////////////////////////////////////////////
bool
Find_File
(
	LPCTSTR filename,
	LPCTSTR start_dir,
	CString &full_path
)
{
	bool bfound = false;

	// Params valid?
	ASSERT (filename != NULL);
	ASSERT (start_dir != NULL);
	if ((filename != NULL) && (start_dir != NULL)) {

		// Make sure the current directory name doesn't end in a
		// directory delimiter.
		CString current_dir = start_dir;
		if (current_dir[::lstrlen (current_dir)-1] == '\\') {
			current_dir = current_dir.Left (::lstrlen (current_dir)-1);
		}
		
		// Is the file in this directory?
		CString dir_test = current_dir + CString ("\\") + filename;
		if (::GetFileAttributes (dir_test) != 0xFFFFFFFF) {
			
			// Found it!
			bfound = true;
			full_path = dir_test;
		} else {
			
			WIN32_FIND_DATA find_info = { 0 };
			BOOL bcontinue = true;

			CString search_mask = current_dir + "\\*.*";

			// Loop through all the files in the current directory
			HANDLE hfilefind;
			for (hfilefind = ::FindFirstFile (search_mask, &find_info);
				  (hfilefind != INVALID_HANDLE_VALUE) && bcontinue && !bfound;
				  bcontinue = ::FindNextFile (hfilefind, &find_info)) {

				// Is this 'file' a subdir of the current directory?
				if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				    (find_info.cFileName[0] != '.')) {

					// Build the full path of this sub-dir name
					CString sub_dir_path = current_dir + CString ("\\") + find_info.cFileName;

					// Recursively call this find function for that subdir
					bfound = Find_File (filename, sub_dir_path, full_path);
				}
			}

			// Close the handle we needed for the file find routines
			if (hfilefind != INVALID_HANDLE_VALUE) {
				::FindClose (hfilefind);
			}
		}
	}

	// Return the true/false result code
	return bfound;
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
	int icompare = 0;

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
			icompare = ::CompareFileTime (&file1_info.ftLastWriteTime, &file2_info.ftLastWriteTime);
		} else if ((hfile1 == INVALID_HANDLE_VALUE) && (hfile2 != INVALID_HANDLE_VALUE)) {
			icompare = -1;
		} else if ((hfile1 != INVALID_HANDLE_VALUE) && (hfile2 == INVALID_HANDLE_VALUE)) {
			icompare = 1;
		}
		
		if (hfile1 != INVALID_HANDLE_VALUE) {
			::CloseHandle (hfile1);
		}

		if (hfile2 != INVALID_HANDLE_VALUE) {
			::CloseHandle (hfile2);
		}
	}

	// Same as strcmp, -1 if file1 is older than file2, 0 if equal, 1 if file1 is newer than file2
	return icompare;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_List_From_String
//
////////////////////////////////////////////////////////////////////////////
int
Build_List_From_String
(
	LPCTSTR buffer,
	LPCTSTR delimiter,
	CString **pstring_list
)
{
	// Start with zero list entries
	int count = 0;

	ASSERT (buffer != NULL);
	ASSERT (delimiter != NULL);
	ASSERT (pstring_list != NULL);
	if ((buffer != NULL) &&
		 (delimiter != NULL) &&
		 (pstring_list != NULL)) {

		size_t delim_len = ::strlen (delimiter);

		// Determine how many entries there will be in the list
		LPCTSTR entry;
		for (entry = buffer;
			  (entry != NULL) && (entry[1] != 0);
			  entry = ::strstr (entry, delimiter)) {
			
			// Move past the current delimiter (if necessary)
			if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
				entry += delim_len;
			}

			// Increment the count of entries
			count ++;
		}
	
		if (count > 0) {

			// Allocate enough CString objects to hold all the strings in the list
			(*pstring_list) = new CString[count];
		
			// Parse the string and pull out its entries.
			count = 0;
			for (entry = buffer;
				  (entry != NULL) && (entry[1] != 0);
				  entry = ::strstr (entry, delimiter)) {
				
				// Move past the current delimiter (if necessary)
				if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
					entry += delim_len;
				}

				// Copy this entry into its own string
				CString entry_string = entry;
				int delim_index = entry_string.Find (delimiter);
				entry_string = (delim_index >= 0) ? entry_string.Left (delim_index) : entry_string;

				//
				// Add this entry to our list (even if its an empty string)
				//
				(*pstring_list)[count++] = entry_string;
			}
		} else if (delim_len > 0) {
			count = 1;
			(*pstring_list) = new CString[count];
			(*pstring_list)[0] = buffer;
		}
				
	}

	// Return the number of entries in our list
	return count;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_String_From_List
//
////////////////////////////////////////////////////////////////////////////
CString
Build_String_From_List
(
	const CString *pstring_list,
	int count,
	LPCTSTR delimiter
)
{	
	// Start an empty string
	CString composite_string;

	ASSERT (delimiter != NULL);
	ASSERT (pstring_list != NULL);
	if ((delimiter != NULL) &&
		 (pstring_list != NULL)) {

		// Loop through all the entries in our list and add them
		// to the composite string
		for (int entry_index = 0; entry_index < count; entry_index ++) {
			composite_string += pstring_list[entry_index] + CString (delimiter);
		}
	}

	// Return the composite string
	return composite_string;
}


////////////////////////////////////////////////////////////////////////////
//
//  Constrain_Point_To_Aspect_Ratio
//
////////////////////////////////////////////////////////////////////////////
void
Constrain_Point_To_Aspect_Ratio
(
	float &xpos,
	float &ypos
)
{
	// Get the aspect ratios for the x and y axis
	float xaspect = ::Get_Main_View ()->Get_Cursor_Aspect_RatioX ();
	float yaspect = ::Get_Main_View ()->Get_Cursor_Aspect_RatioY ();

	// Convert the 'window' point to a full screen point.
	xpos *= xaspect;
	ypos *= yaspect;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Asset_Name_From_Filename
//
////////////////////////////////////////////////////////////////////////////
CString
Asset_Name_From_Filename (LPCTSTR filename)
{
	// Get the filename from this path
	CString asset_name = ::Get_Filename_From_Path (filename);
	
	// Find the index of the extension (if exists)
	int extension = asset_name.ReverseFind ('.');	
	
	// Strip off the extension
	if (extension != -1) {
		asset_name = asset_name.Left (extension);
	}

	// Return the name of the asset
	return asset_name;
}


////////////////////////////////////////////////////////////////////////////
//
//  Filename_From_Asset_Name
//
CString
Filename_From_Asset_Name (LPCTSTR asset_name)
{
	// The filename is simply the asset name plus the .w3d extension
	CString filename = asset_name + CString (".w3d");

	// Return the filename
	return filename;
}


////////////////////////////////////////////////////////////////////////////
//
//  Message_Box
//
////////////////////////////////////////////////////////////////////////////
UINT
Message_Box
(
	HWND hparentwnd,
	UINT message_id,
	UINT title_id,
	UINT style
)
{
	UINT ret_code = IDOK;

	if (Is_Silent_Mode () == false) {

		// Load the message from the strings table
		CString message;
		CString title;
		message.LoadString (message_id);
		title.LoadString (title_id);

		// Show the message box to the user and return the user's response
		ret_code = ::MessageBox (hparentwnd, message, title, style);
	}
	
	return ret_code;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rotate_Matrix
//
////////////////////////////////////////////////////////////////////////////
void
Rotate_Matrix
(
	const Matrix3D &input_mat,
	const Matrix3D &rotation_mat,
	const Matrix3D &coord_system,
	Matrix3D *poutput_mat
)
{
	/*
	** Translate this node in the specified coordinate system
	*/
	Matrix3D coord_inv;					// inverse of coordinate transform
	Matrix3D coord_to_obj;				// transform from coord to object
	
	coord_system.Get_Orthogonal_Inverse (coord_inv);
	Matrix3D::Multiply (coord_inv, input_mat, &coord_to_obj);

	Matrix3D::Multiply (coord_system, rotation_mat, poutput_mat);
	Matrix3D::Multiply (*poutput_mat, coord_to_obj, poutput_mat);

	if (!poutput_mat->Is_Orthogonal ()) {
		poutput_mat->Re_Orthogonalize ();
		TRACE ("Matrix wasn't orthogonal.\n");
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_LOD_File_Count
//
////////////////////////////////////////////////////////////////////////////
int
Get_LOD_File_Count
(
	LPCTSTR first_lod_filename,
	CString *pbase_filename
)
{
	CString base_filename = first_lod_filename;
	int lod_count = 0;

	if ((base_filename.GetLength () > 6) &&
		 ::strcmpi (&first_lod_filename[::lstrlen (first_lod_filename)-6], "L1.W3D") == 0) {
		base_filename = base_filename.Left (base_filename.GetLength () - 6);
			
		// Loop through the files in the directory and count up the ones that
		// match our LOD-syntax for this object
		bool bfound = true;
		int lod_index = 1;
		while (bfound) {
			CString lod_filename;
			lod_filename.Format ("%sL%d.w3d", (LPCTSTR)base_filename, lod_index);
			lod_index ++;
			bfound = (::GetFileAttributes (lod_filename) != 0xFFFFFFFF);
			if (bfound) {
				lod_count ++;
			}
		}
	}
	
	if (pbase_filename != NULL) {
		*pbase_filename = base_filename;
	}

	// Return the count of LODs we found
	return lod_count;
}


////////////////////////////////////////////////////////////////////////////
//
//  Browse_For_Folder
//
////////////////////////////////////////////////////////////////////////////
bool
Browse_For_Folder
(	
	CString &folder,
	HWND hparentwnd,
	LPCTSTR /* default_path */,
	LPCTSTR title,
	UINT flags
)
{
	bool retval = false;

	// Browse for the folder
	BROWSEINFO browse_info = { 0 };
	browse_info.hwndOwner = hparentwnd;
	browse_info.lpszTitle = title;
	browse_info.ulFlags = flags;
	LPITEMIDLIST pidl = ::SHBrowseForFolder (&browse_info);
	if (pidl != NULL) {
		
		// Convert the 'PIDL' into a string
		char path[MAX_PATH];
		retval = (::SHGetPathFromIDList (pidl, path) != 0);
		if (retval) {
			folder = path;
		}

		// Fre the 'PIDL'
		LPMALLOC pmalloc = NULL;
		if (SUCCEEDED (::SHGetMalloc (&pmalloc))) {
			pmalloc->Free (pidl);
			COM_RELEASE (pmalloc);
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  General_Pump_Messages
//
////////////////////////////////////////////////////////////////////////////
void
General_Pump_Messages (void)
{
	// Process any paint messages currently pending in the queue
	MSG msg = { 0 };
	while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {		
		::DispatchMessage (&msg);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Pump_Messages
//
////////////////////////////////////////////////////////////////////////////
void
Pump_Messages (void)
{
	// Process any paint messages currently pending in the queue
	MSG msg = { 0 };
	while (::PeekMessage (&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE) ||
			 ::PeekMessage (&msg, NULL, WM_NCPAINT, WM_NCPAINT, PM_REMOVE) ||
			 ::PeekMessage (&msg, NULL, WM_ERASEBKGND, WM_ERASEBKGND, PM_REMOVE) ||
			 ::PeekMessage (&msg, NULL, WM_SYNCPAINT, WM_SYNCPAINT, PM_REMOVE)) {
		
		::DispatchMessage (&msg);
	}

	return ;
}


typedef struct
{
	LPVOID ThreadProc;
	DWORD dwparam1;
	DWORD dwparam2;
	DWORD dwparam3;
	HRESULT *presult;
	HWND *phmain_wnd;
	HANDLE hevent;
} THREAD_PARAMS;


////////////////////////////////////////////////////////////////////////////
//
//  fnWorkerThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnWorkerThread (LPVOID pParam)
{
	THREAD_PARAMS *pthread_params = (THREAD_PARAMS *)pParam;
	if (pthread_params != NULL) {

		// Call the user function
		MY_THREADPROC thread_proc = (MY_THREADPROC)pthread_params->ThreadProc;
		(thread_proc) (pthread_params->dwparam1,
							pthread_params->dwparam2,
							pthread_params->dwparam3,
							pthread_params->presult);

		// Free the thread params
		SAFE_DELETE (pthread_params);
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_Worker_Thread
//
////////////////////////////////////////////////////////////////////////////
void
Create_Worker_Thread
(
	MY_THREADPROC fnthread_proc,
	DWORD dwparam1,
	DWORD dwparam2,
	DWORD dwparam3,
	HRESULT *presult
)
{
	// Create a structure we can pass to the thread proc
	THREAD_PARAMS *pthread_params = new THREAD_PARAMS;
	pthread_params->ThreadProc = (LPVOID)fnthread_proc;
	pthread_params->dwparam1 = dwparam1;
	pthread_params->dwparam2 = dwparam2;
	pthread_params->dwparam3 = dwparam3;
	pthread_params->presult = presult;
	pthread_params->phmain_wnd = NULL;
	pthread_params->hevent = NULL;

	// Kick off our own thread proc
	::AfxBeginThread (fnWorkerThread, (LPVOID)pthread_params);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fnUIThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnUIThread (LPVOID pParam)
{
	THREAD_PARAMS *pthread_params = (THREAD_PARAMS *)pParam;
	if (pthread_params != NULL) {

		// Call the user function
		MY_UITHREADPROC thread_proc = (MY_UITHREADPROC)pthread_params->ThreadProc;
		(thread_proc) (pthread_params->dwparam1,
							pthread_params->dwparam2,
							pthread_params->dwparam3,
							pthread_params->presult,
							pthread_params->phmain_wnd);

		// Let the calling thread return
		::SetEvent (pthread_params->hevent);

		// Now pump messages until its time to quit the thread
		MSG msg = { 0 };
		while (::GetMessage (&msg, NULL, 0, 0) > 0) {
			::TranslateMessage (&msg);
			::DispatchMessage (&msg);
		}

		// Free the thread params
		SAFE_DELETE (pthread_params);
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_UI_Thread
//
////////////////////////////////////////////////////////////////////////////
void
Create_UI_Thread
(
	MY_UITHREADPROC fnthread_proc,
	DWORD_PTR dwparam1,
	DWORD_PTR dwparam2,
	DWORD_PTR dwparam3,
	HRESULT *presult,
	HWND *phmain_wnd
)
{	
	// Create a structure we can pass to the thread proc
	THREAD_PARAMS *pthread_params = new THREAD_PARAMS;
	pthread_params->ThreadProc = (LPVOID)fnthread_proc;
	pthread_params->dwparam1 = dwparam1;
	pthread_params->dwparam2 = dwparam2;
	pthread_params->dwparam3 = dwparam3;
	pthread_params->presult = presult;
	pthread_params->phmain_wnd = phmain_wnd;;
	pthread_params->hevent = ::CreateEvent (NULL, false, false, NULL);

	// Kick off our own thread proc
	::AfxBeginThread (fnUIThread, (LPVOID)pthread_params);	

	// Now pump messages until the thread has initialized
	while (::MsgWaitForMultipleObjects (1,
												  &(pthread_params->hevent),
												  false,
												  INFINITE,
												  QS_ALLINPUT | QS_ALLEVENTS | QS_ALLPOSTMESSAGE) != WAIT_OBJECT_0) {
		

		// Dispatch all messages in the queue
		MSG msg = { 0 };
		while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {			
			::TranslateMessage (&msg);
			::DispatchMessage (&msg);
		}
	}

	// Make sure we close the event handle
	::CloseHandle (pthread_params->hevent);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Frame_Count
//
////////////////////////////////////////////////////////////////////////////
void
Update_Frame_Count
(
	int frame,
	int max_frames
)
{
   // Get a pointer to the main window
	CMainFrame *pmainwnd = (CMainFrame *)theApp.GetMainWnd ();
    
	// Did we successfully get a pointer to the main window?
	ASSERT (pmainwnd != NULL);
	if (pmainwnd != NULL) {
		pmainwnd->Update_Ani_Frame (frame, max_frames);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetDlgItemFloat
//
////////////////////////////////////////////////////////////////////////////
void
SetDlgItemFloat
(
	HWND hdlg,
	UINT child_id,
	float value
)
{
	// Convert the float to a string
	CString text;
	text.Format ("%.3f", value);

	// Pass the string onto the dialog control
	::SetDlgItemText (hdlg, child_id, text);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetWindowFloat
//
////////////////////////////////////////////////////////////////////////////
void
SetWindowFloat (HWND hwnd, float value)
{
	// Convert the float to a string
	CString text;
	text.Format ("%.3f", value);

	// Pass the string onto the window
	::SetWindowText (hwnd, text);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetWindowFloat
//
////////////////////////////////////////////////////////////////////////////
float
GetWindowFloat (HWND hwnd, bool interpret)
{
	// Get the string from the window
	TCHAR string_value[20];
	::GetWindowText (hwnd, string_value, sizeof (string_value));

	//
	// Convert the string to a float and return the value
	//
	float value = 0;
	if (interpret) {
		value = ((float)::atol (string_value)) / 100.0F;
	} else {
		value = ::atof (string_value);
	}
	
	return value;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetDlgItemFloat
//
////////////////////////////////////////////////////////////////////////////
float
GetDlgItemFloat
(
	HWND hdlg,
	UINT child_id,
	bool interpret
)
{
	// Get the string from the window
	TCHAR string_value[20];
	::GetDlgItemText (hdlg, child_id, string_value, sizeof (string_value));

	//
	// Convert the string to a float and return the value
	//
	float value = 0;
	if (interpret) {
		value = ((float)::atol (string_value)) / 100.0F;
	} else {
		value = ::atof (string_value);
	}
	
	return value;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Conversation_Form
//
////////////////////////////////////////////////////////////////////////////
ConversationPageClass *
Get_Conversation_Form (void)
{
	//
	// Return the form pointer
	//
	return ConversationPageClass::Get_Instance ();
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Overlap_Form
//
////////////////////////////////////////////////////////////////////////////
OverlapPageClass *
Get_Overlap_Form (void)
{
	OverlapPageClass *form = NULL;

	//
	// Attempt to get a pointer to the form from the dialog bar
	//	
	CMainFrame *main_wnd					= (CMainFrame *)::AfxGetMainWnd ();
	if (main_wnd != NULL) {
		MainDialogBarClass &toolbar	= main_wnd->Get_Main_Dialog_Bar ();
		form									= toolbar.Get_Overlap_Form ();
	}

	// Return the form pointer
	return form;
}

			
////////////////////////////////////////////////////////////////////////////
//
//  Get_Instances_Form
//
////////////////////////////////////////////////////////////////////////////
InstancesPageClass *
Get_Instances_Form (void)
{
	InstancesPageClass *form = NULL;

	//
	// Attempt to get a pointer to the form from the dialog bar
	//	
	CMainFrame *main_wnd				= (CMainFrame *)::AfxGetMainWnd ();
	if (main_wnd != NULL) {
		MainDialogBarClass &toolbar	= main_wnd->Get_Main_Dialog_Bar ();
		form									= toolbar.Get_Instances_Form ();
	}

	// Return the form pointer
	return form;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Global_Presets_Form
//
////////////////////////////////////////////////////////////////////////////
PresetsFormClass *
Get_Presets_Form (void)
{
	//
	// Attempt to get a pointer to the form from the dialog bar
	//	
	MainDialogBarClass &toolbar	= ((CMainFrame *)::AfxGetMainWnd ())->Get_Main_Dialog_Bar ();
	PresetsFormClass *form			= toolbar.Get_Presets_Form ();

	// Return the form pointer
	return form;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Global_Group_List
//
////////////////////////////////////////////////////////////////////////////
GROUP_LIST &
Get_Global_Group_List (void)
{
	// Return the list pointer
	return ::Get_Scene_Editor ()->Get_Group_List ();
}


////////////////////////////////////////////////////////////////////////////
//
//  Fill_Node_Instance_Combo
//
////////////////////////////////////////////////////////////////////////////
void
Fill_Node_Instance_Combo
(
	HWND			hcombobox,
	NodeClass *	default_node
)
{	
	// Loop through all the nodes in the level, and add them to the combobox
	NodeMgrClass &node_mgr = ::Get_Node_Mgr ();
	for (NodeClass *node = node_mgr.Get_First ();
		  node != NULL;
		  node = node_mgr.Get_Next (node)) {
		
		// Add this node to the combobox
		int index = ::SendMessage (hcombobox, CB_ADDSTRING, (WPARAM)0, (LPARAM)node->Get_Name ());
		if (index != CB_ERR) {
			::SendMessage (hcombobox, CB_SETITEMDATA, (WPARAM)index, (LPARAM)node);

			// If this is the default node, then select it...
			if (node == default_node) {
				::SendMessage (hcombobox, CB_SETCURSEL, (WPARAM)index, 0L);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Fill_Group_Combo
//
////////////////////////////////////////////////////////////////////////////
void
Fill_Group_Combo
(
	HWND				hcombobox,
	GroupMgrClass *pdefault
)
{	
	// Loop through all the groups in the level, and add them to the combobox
	GROUP_LIST &group_list = ::Get_Global_Group_List ();
	for (int index = 0; index < group_list.Count (); index ++) {

		GroupMgrClass *pgroup = group_list[index];
		if (pgroup != NULL) {

			// Add this group to the combobox
			int index = ::SendMessage (hcombobox, CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)pgroup->Get_Name ());
			if (index != CB_ERR) {
				::SendMessage (hcombobox, CB_SETITEMDATA, (WPARAM)index, (LPARAM)pgroup);

				// If this is the default group, then select it...
				if (pgroup == pdefault) {
					::SendMessage (hcombobox, CB_SETCURSEL, (WPARAM)index, 0L);
				}
			}
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Ambient_Light_Form
//
////////////////////////////////////////////////////////////////////////////
LightAmbientFormClass *
Get_Ambient_Light_Form (void)
{
	// Attempt to get a pointer to the form from the library object
	FormToolbarClass &toolbar = ((CMainFrame *)::AfxGetMainWnd ())->Get_Ambient_Light_Toolbar ();
	LightAmbientFormClass *pform = (LightAmbientFormClass *)toolbar.Get_Form ();

	// Return the form pointer
	return pform;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Global_Image_List
//
////////////////////////////////////////////////////////////////////////////
CImageList *
Get_Global_Image_List (void)
{
	// Return the imagelist pointer
	return ((CMainFrame *)::AfxGetMainWnd ())->Get_Image_List ();
}


////////////////////////////////////////////////////////////////////////////
//
//  Copy_File
//
////////////////////////////////////////////////////////////////////////////
bool
Copy_File
(
	LPCTSTR	existing_filename,
	LPCTSTR	new_filename,
	bool		bforce_copy
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
			retval = (::CopyFile (existing_filename, new_filename, false) != 0);
		}
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnUpdatingVSSThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnUpdatingVSSThread
(
	DWORD_PTR dwparam1,
	DWORD_PTR /*dwparam2*/,
	DWORD_PTR /*dwparam3*/,
	HRESULT* /*presult*/,
	HWND* phmain_wnd
)
{
	// Create a new instance of the 'updating' dialog
	UpdatingDBDialogClass *pdialog = new UpdatingDBDialogClass ((HWND)dwparam1);
	pdialog->ShowWindow (SW_SHOW);

	// Return the window handle of the main wnd to the calling thread
	if (phmain_wnd != NULL) {
		(*phmain_wnd) = pdialog->m_hWnd;
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////
//
//  Show_VSS_Update_Dialog
//
////////////////////////////////////////////////////////////////////////////
HWND
Show_VSS_Update_Dialog (HWND hparent_wnd)
{
	// Kick off a UI thread that will display the 'updating' dialog and animation for us
	HWND hthread_wnd = NULL;
	::Create_UI_Thread (fnUpdatingVSSThread, (DWORD_PTR)hparent_wnd, 0, 0, NULL, &hthread_wnd);
	return hthread_wnd;
}


////////////////////////////////////////////////////////////////////////////
//
//  Kill_VSS_Update_Dialog
//
////////////////////////////////////////////////////////////////////////////
void
Kill_VSS_Update_Dialog (HWND hdlg)
{
	// Close out the window
	if (::IsWindow (hdlg)) {
		::PostMessage (hdlg, WM_USER+101, 0, 0L);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_File_Time
//
////////////////////////////////////////////////////////////////////////////
bool
Get_File_Time
(
	LPCTSTR path,
	LPFILETIME pcreation_time,
	LPFILETIME paccess_time,
	LPFILETIME pwrite_time
)
{
	// Assume failure
	bool retval = false;

	// Attempt to open the file
	HANDLE hfile = ::CreateFile (path,
										  0,
										  0,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);
	
	if (hfile != INVALID_HANDLE_VALUE) {

		// Get the mod times for this file
		retval = (::GetFileTime (hfile, pcreation_time, paccess_time, pwrite_time) != 0);

		// Close the file
		SAFE_CLOSE (hfile);
	}		
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//	Type_To_Icon
//
////////////////////////////////////////////////////////////////////////////
int
Type_To_Icon
(
	NODE_TYPE type,
	bool btemp
)
{
	int index = TEMP_ICON;

	// If we aren't looking for a temp icon, then determine
	// which icon to use based on the type
	if (btemp == false) {		
		switch (type)
		{
			case NODE_TYPE_OBJECT:
				index = OBJECT_ICON;
				break;

			case NODE_TYPE_TERRAIN_SECTION:
			case NODE_TYPE_TERRAIN:
				index = TERRAIN_ICON;
				break;
			
			case NODE_TYPE_TILE:
				index = TILE_ICON;
				break;

			case NODE_TYPE_ZONE:
			case NODE_TYPE_DAMAGE_ZONE:
				index = ZONE_ICON;
				break;

			case NODE_TYPE_COVER_SPOT:
				index = OBJECT_ICON;
				break;

			case NODE_TYPE_WAYPATH:
			case NODE_TYPE_WAYPOINT:
				index = WAYPATH_ICON;
				break;

			case NODE_TYPE_LIGHT:
				index = LIGHT_ICON;
				break;

			case NODE_TYPE_SOUND:
				index = SOUND_ICON;
				break;

			case NODE_TYPE_TRANSITION:
			case NODE_TYPE_TRANSITION_CHARACTER:
				index = TRANSITION_ICON;
				break;

			case NODE_TYPE_BUILDING:
				index = BUILDING_ICON;
				break;
		}
	}

	// Return the icon's index
	return index;
}


/////////////////////////////////////////////////////////////////////////////
//
//	FileAccessRightsClass
//
/////////////////////////////////////////////////////////////////////////////
FileAccessRightsClass::FileAccessRightsClass
(
	LPCTSTR filename,
	ACCESS_TYPE type,
	bool should_restore
)
	:	m_Filename (filename),
		m_FileAttrs (0xFFFFFFFF),
		m_bNeedsRestoring (false)
{
	// Does the file exist?
	m_FileAttrs = ::GetFileAttributes (m_Filename);
	if (m_FileAttrs != 0xFFFFFFFF) {
		
		if ((type == WANTS_READONLY) &&
			 (m_FileAttrs & FILE_ATTRIBUTE_READONLY) != FILE_ATTRIBUTE_READONLY) {
			
			// Assign the readonly bit to the file
			::SetFileAttributes (m_Filename, m_FileAttrs | FILE_ATTRIBUTE_READONLY);
			m_bNeedsRestoring = should_restore;
		} else if ((type == WANTS_WRITEABLE) &&
					  (m_FileAttrs & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY) {
			
			// Strip the readonly bit from the file
			::SetFileAttributes (m_Filename, m_FileAttrs & (~FILE_ATTRIBUTE_READONLY));
			m_bNeedsRestoring = should_restore;			
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	~FileAccessRightsClass
//
/////////////////////////////////////////////////////////////////////////////
FileAccessRightsClass::~FileAccessRightsClass (void)
{
	// Restore the file's attributes if necessary
	if (m_bNeedsRestoring) {
		::SetFileAttributes (m_Filename, m_FileAttrs);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	fnEditToFloatProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
fnEditToFloatProc
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	WNDPROC old_proc = (WNDPROC)::GetProp (hwnd, "OLD_WND_PROC");
	LRESULT result = 0L;

	if (message == WM_SETTEXT) {
		
		//
		//	Convert the textual value to a int, convert
		// the int to a float, and conver the float to
		// a string.
		//
		LPCTSTR string		= (LPCTSTR)lparam;
		if (::strchr (string, '.') != 0) {
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
		} else {
			int value			= ::atol ((LPCTSTR)lparam);
			float float_value	= value / 100.0F;
			CString new_text;
			new_text.Format ("%.3f", float_value);
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, (LPARAM)(LPCTSTR)new_text);
		}

	} else if (message == WM_GETTEXT) {
		
		//
		//	Get the value (as text) from the control,
		// convert it to a float, convert the float
		// to a int, then convert the int back to
		// a string.
		//
		result				= ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);		
		LPCTSTR string		= (LPCTSTR)lparam;
		if (::strchr (string, '.') != 0) {
			float float_value	= ::atof (string);
			int int_value		= int(float_value * 100);
			::itoa (int_value, (LPTSTR)lparam, 10);			
		} else {
			int int_value		= ::atol (string) * 100;
			::itoa (int_value, (LPTSTR)lparam, 10);						
		}

		result = ::lstrlen ((LPTSTR)lparam);

	} else if (message == WM_CHAR) {
		
		//
		//	Check to see if this is one of the characters we allow
		// the user to type
		//
		if (	(wparam >= '0' && wparam <= '9') ||
				wparam == '.' ||
				wparam == VK_BACK ||
				wparam == '-')
		{
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
		}

	} else if (old_proc != NULL) {
		result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
	}

	return result;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Restore_Edit_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
Restore_Edit_Ctrl (HWND edit_wnd)
{
	DWORD_PTR orig_proc = (DWORD_PTR)::GetProp (edit_wnd, "OLD_WND_PROC");
	if (orig_proc != 0) {
		::SetWindowLongPtr (edit_wnd, GWLP_WNDPROC, (LONG_PTR)orig_proc);
		::RemoveProp (edit_wnd, "OLD_WND_PROC");
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
//	Make_Edit_Float_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
Make_Edit_Float_Ctrl (HWND edit_wnd)
{
	Restore_Edit_Ctrl (edit_wnd);

	LONG_PTR old_proc = ::SetWindowLongPtr (edit_wnd, GWLP_WNDPROC, (LONG_PTR)fnEditToFloatProc);
	SetProp (edit_wnd, "OLD_WND_PROC", (HANDLE)old_proc);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	fnEditToIntProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
fnEditToIntProc
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	WNDPROC old_proc = (WNDPROC)::GetProp (hwnd, "OLD_WND_PROC");
	LRESULT result = 0L;

	if (message == WM_CHAR) {
		
		//
		//	Check to see if this is one of the characters we allow
		// the user to type
		//
		if (	(wparam >= '0' && wparam <= '9') ||
				wparam == VK_BACK ||
				wparam == '-')
		{
			result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
		}

	} else if (old_proc != NULL) {
		result = ::CallWindowProc (old_proc, hwnd, message, wparam, lparam);
	}

	return result;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Make_Edit_Int_Ctrl
//
/////////////////////////////////////////////////////////////////////////////
void
Make_Edit_Int_Ctrl (HWND edit_wnd)
{
	Restore_Edit_Ctrl (edit_wnd);

	LONG_PTR old_proc = ::SetWindowLongPtr (edit_wnd, GWLP_WNDPROC, (LONG_PTR)fnEditToIntProc);
	SetProp (edit_wnd, "OLD_WND_PROC", (HANDLE)old_proc);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Modified
//
/////////////////////////////////////////////////////////////////////////////
void
Set_Modified (bool modified)
{
	::Get_Current_Document ()->SetModifiedFlag (modified);
	return ;
}


bool _IsSilent = false;
//////////////////////////////////////////////////////////////////////////////
//
//	Is_Silent_Mode
//
/////////////////////////////////////////////////////////////////////////////
bool
Is_Silent_Mode (void)
{
	return _IsSilent;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Silent_Mode
//
/////////////////////////////////////////////////////////////////////////////
void
Set_Silent_Mode (bool is_silent)
{
	_IsSilent = is_silent;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Next_Temp_ID
//
/////////////////////////////////////////////////////////////////////////////
int
Get_Next_Temp_ID (void)
{
	//
	//	Read the next temporary preset ID from the registry
	//
	int temp_id = theApp.GetProfileInt (CONFIG_KEY, TEMP_ID_VALUE, TEMP_DEF_ID_START);

	//
	//	Make sure the temp ID is in the right range
	//
	if (temp_id < TEMP_DEF_ID_START) {
		temp_id = TEMP_DEF_ID_START;
	}

	//
	//	Store the next temp ID value in the registry
	//
	theApp.WriteProfileInt (CONFIG_KEY, TEMP_ID_VALUE, temp_id+1);	
	return temp_id;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Collision_Box
//
/////////////////////////////////////////////////////////////////////////////
bool
Get_Collision_Box (const RenderObjClass *model, AABoxClass &box)
{
	bool retval = false;

	if (model != NULL) {
		
		//
		//	Get the collision box for this render obj
		//
		RenderObjClass *world_box = model->Get_Sub_Object_By_Name ("WORLDBOX");

		// If we didn't finde WorldBox, try to find the LOD named "WorldBox" 
		// The LOD code generates a unique name for the mesh by appending A,B,C, etc to the name.
		// A is the lowest LOD, B is the next, and so on.  Our worldbox is specified in the highest
		// LOD so we have to construct the name by appending 'A'+LodCount to the name... icky
		if ((world_box == NULL) && (model->Class_ID() == RenderObjClass::CLASSID_HLOD))
		{			
			char namebuffer[64];
			sprintf(namebuffer,"WorldBox%c",'A' + ((HLodClass *)model)->Get_Lod_Count() - 1);

			world_box = model->Get_Sub_Object_By_Name(namebuffer);
			/*if (box_obj && box_obj->Class_ID() == RenderObjClass::CLASSID_OBBOX) {
				world_box = box_obj;
			}*/
		}
		
		if (world_box != NULL) {
			box = world_box->Get_Bounding_Box ();
			world_box->Release_Ref ();
			retval = true;
		}
	}
	

	return retval;
}


//////////////////////////////////////////////////////////////////
//
//	Get_Collision_Box
//
//	Note:  This function returns true if it returns an AABox or
// false if it returns an OBBox
//
//////////////////////////////////////////////////////////////////
bool
Get_Collision_Box (RenderObjClass *render_obj, AABoxClass &aabox, OBBoxClass &obbox)
{
	bool is_aabox = true;

	aabox.Center.Set (0, 0, 0);
	aabox.Extent.Set (0, 0, 0);
	obbox.Center.Set (0, 0, 0);
	obbox.Extent.Set (0, 0, 0);

	if (render_obj != NULL) {

		//
		// Try to get the "WorldBox" from the model
		//
		RenderObjClass *world_box = render_obj->Get_Sub_Object_By_Name ("WorldBox");

		//
		// If we didn't finde WorldBox, try to find the LOD named "WorldBox" 
		// The LOD code generates a unique name for the mesh by appending A,B,C, etc to the name.
		// A is the lowest LOD, B is the next, and so on.  Our worldbox is specified in the highest
		// LOD so we have to construct the name by appending 'A'+LodCount to the name... icky
		//
		if ((world_box == NULL) && (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
			
			char namebuffer[64];
			sprintf(namebuffer,"WorldBox%c",'A' + ((HLodClass *)render_obj)->Get_Lod_Count() - 1);
			world_box = render_obj->Get_Sub_Object_By_Name (namebuffer);
		}

		if (world_box != NULL) {
			
			//
			//	Determine which type of bounding box to return and OBBox or an AABox
			//
			if (world_box->Class_ID() == RenderObjClass::CLASSID_OBBOX) {
				obbox		= ((OBBoxRenderObjClass *)world_box)->Get_Box ();
				is_aabox	= false;
			} else {
				aabox		= world_box->Get_Bounding_Box ();
				is_aabox	= true;
			}
		}

		REF_PTR_RELEASE (world_box);
	}

	return is_aabox;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Instance_Definition
//
/////////////////////////////////////////////////////////////////////////////
PersistClass *
Instance_Definition (DefinitionClass *definition)
{
	PersistClass *new_object = NULL;
	
	//
	//	Create the new object
	//
	if (definition != NULL) {
		new_object = definition->Create ();
	}

	if (new_object == NULL) {
		
		LPCTSTR name = definition->Get_Name ();
		CString message;
		message.Format ("Unable to create an instance of %s.\nCheck to make sure the preset is properly configured with a model and/or physics object.", name);
			
		//
		//	Warn the user that we were unable to create the object
		//
		HWND parent_wnd = ::AfxGetMainWnd ()->m_hWnd;
		::MessageBox (parent_wnd, message, "Preset Error", MB_ICONERROR | MB_OK);
	}

	return new_object;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Check_Editor_Version
//
/////////////////////////////////////////////////////////////////////////////
bool
Check_Editor_Version (void)
{
#ifndef PUBLIC_EDITOR_VER
	char curr_filename[MAX_PATH];
	::GetModuleFileName (NULL, curr_filename, MAX_PATH);

	CString filename = "\\\\mobius\\project7\\projects\\renegade\\programming\\tools\\level edit\\";
	filename += ::Get_Filename_From_Path (curr_filename);

	//
	//	Check the version of the level editor that is out on the network
	// against the version we are running.  We don't want people making modifications
	// with and older version
	//
	return (::Compare_EXE_Version (::AfxGetInstanceHandle (), filename) >= 0);
#else
	return true;
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
// Perform_Job
//
/////////////////////////////////////////////////////////////////////////////
void
Perform_Job (LPCTSTR filename, bool delete_on_completion)
{
	int total = ::GetPrivateProfileInt ("Job Description", "Total", 1, filename);
	int index = ::GetPrivateProfileInt ("Job Description", "Index", 0, filename);
	
	CString granularity_string;
	::GetPrivateProfileString ("Job Description", "Granularity", "1", granularity_string.GetBufferSetLength (20), 20, filename);
	float granularity = ::atof (granularity_string);

	CString level_file;
	::GetPrivateProfileString ("Job Description", "LVL", "", level_file.GetBufferSetLength (MAX_PATH), MAX_PATH, filename);

	CString output_file;
	::GetPrivateProfileString ("Job Description", "Output", "", output_file.GetBufferSetLength (MAX_PATH), MAX_PATH, filename);

	if (level_file.GetLength () > 0) {
		
		//
		//	Load the requested level
		//
		::Get_Main_View ()->Allow_Repaint (false);
		EditorSaveLoadClass::Load_Level (level_file);
		::Get_Main_View ()->Allow_Repaint (true);

		//
		//	Start VIS
		//
#if (1)		
		::Get_Scene_Editor ()->Generate_Uniform_Sampled_Vis (granularity,false,false,true,index,total);
#else
		::Get_Scene_Editor ()->Generate_Edge_Sampled_Vis (granularity,false,true,index,total);
#endif
		::Get_Scene_Editor ()->Generate_Manual_Vis (true,index,total);
		::Get_Scene_Editor ()->Generate_Light_Vis (true,index,total);

		//
		//	Export VIS to the given file
		//
		::Get_Scene_Editor ()->Export_VIS (output_file);
	}

	//
	//	Delete the job file if necessary
	//
	if (delete_on_completion) {
		::DeleteFile (filename);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Factory_Name
//
/////////////////////////////////////////////////////////////////////////////
LPCTSTR
Get_Factory_Name (uint32 class_id)
{
	LPCTSTR name = NULL;

	//
	//	Can we find the requested factory?
	//
	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (class_id);
	if (factory == NULL) {
		
		//
		//	Check to see if this is an abstract factory
		//
		for (int index = 0; index < PRESET_CATEGORY_COUNT; index ++) {
			if (PRESET_CATEGORIES[index].clsid == (int)class_id) {
				name = PRESET_CATEGORIES[index].name;
				break;
			}
		}

	} else {
		name = factory->Get_Name ();
	}

	return name;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Newline_To_Chars
//
/////////////////////////////////////////////////////////////////////////////
void
Convert_Newline_To_Chars (CString &string)
{
	CString retval;

	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.GetLength ();

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (string[index] == '\n') {
			retval += "\\n";
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Chars_To_Newline
//
/////////////////////////////////////////////////////////////////////////////
void
Convert_Chars_To_Newline (CString &string)
{
	CString retval;
	
	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.GetLength ();

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (index + 1 < count && string[index] == '\\' && string[index + 1] == 'n') {
			retval += '\n';
			index ++;
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}
