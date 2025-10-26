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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "StdAfx.h"
#include "mixviewer.h"
#include "mainfrm.h"
#include "wwstring.h"
#include "duplicatecombiner.h"
#include "wwfile.h"
#include "ffactory.h"
#include "mixfile.h"
#include "avassetsuck.h"
#include "mixpatchmaker.h"
#include "ffactory.h"
#include "mixfile.h"
#include "mixviewerdoc.h"
#include "makemixfiledialog.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(IDM_COMBINE_DUPLICATES, OnCombineDuplicates)
	ON_COMMAND(IDM_CREATE_MIX_FILE, OnCreateMixFile)
	ON_COMMAND(IDM_EXPORT_FILES, OnExportFiles)
	ON_COMMAND(IDM_REMOVE_AV_ASSETS, OnRemoveAVAssets)
	ON_COMMAND(IDM_MAKE_MIX_PATCH, OnMakeMixPatch)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return false;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



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
	return temp_path;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnCombineDuplicates
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCombineDuplicates (void)
{
	CFileDialog dialog (	true,
								".dat",
								"always.dat",
								OFN_HIDEREADONLY | OFN_EXPLORER,
								"Mix File (*.dat, *.mix)|*.dat;*.mix||",
								this);

	dialog.m_ofn.lpstrTitle = "Pick Shared Mix File";
	if (dialog.DoModal () == IDOK) {
		DuplicateRemoverClass combiner;

		//
		//	Determine what directory to search
		//
		StringClass full_path	= static_cast<const char *>(dialog.GetPathName ());
		StringClass directory	= static_cast<const char *>(Strip_Filename_From_Path (full_path));
		combiner.Set_Destination_File (full_path);

		WIN32_FIND_DATA find_info	= { 0 };
		BOOL keep_going				= true;

		CString search_mask = static_cast<const char *>(directory + "\\*.mix");

		//
		// Loop over all the mix files in the search directory
		//
		int count = 0;
		HANDLE find_handle = ::FindFirstFile (search_mask, &find_info);
		for (;(find_handle != INVALID_HANDLE_VALUE) && keep_going;
			  keep_going = ::FindNextFile (find_handle, &find_info))
		{
			//
			// Check to make sure this isn't a directory
			//
			if (!(find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				StringClass full_path	= directory;
				full_path					+= "\\";
				full_path					+= find_info.cFileName;
				combiner.Add_Mix_File (full_path);
				count ++;
			}
		}

		//
		// Close the handle we needed for the file find routines
		//
		if (find_handle != INVALID_HANDLE_VALUE) {
			::FindClose (find_handle);
		}

		//
		//	Combine the files (if necessary)
		//
		if (count > 0) {
			combiner.Process ();
		}
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
//	
//	OnCreateMixFile
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCreateMixFile (void)
{
	MakeMixFileDialogClass dialog( this );
	dialog.DoModal();
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//	
//	OnExportFiles
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportFiles (void)
{
	StringClass	current_filename;

	CDocument * doc = GetActiveDocument();
	if ( doc != NULL ) {
		current_filename = doc->GetPathName();
	}

	if ( !current_filename.Is_Empty() ) {

		DynamicVectorClass<StringClass>	filenames;
		MixFileFactoryClass mix_factory (current_filename, _TheFileFactory);
		if ( mix_factory.Build_Filename_List( filenames ) ) {

			// Make the destination folder
			StringClass export_path = current_filename;
			export_path += " Files";
			int result = (int)::CreateDirectory( export_path, NULL );
			int error = ::GetLastError();
			if ( result != 0 || error == ERROR_ALREADY_EXISTS ) {

				bool	read_error = false;
				bool	write_error = false;

				for (int index = 0; index < filenames.Count (); index ++) {
					StringClass source_name = filenames[index];
					StringClass dest_name;
					dest_name = export_path;
					dest_name += "\\";
					dest_name += filenames[index];

					// If the filename contains a /, create the directory
					if ( ::strchr( filenames[index], '\\' ) != NULL ) {
						StringClass dest_folder = dest_name;
						int length = ::strrchr( dest_folder, '\\' ) - dest_folder;
						dest_folder.Erase( length, dest_folder.Get_Length() - length );
						int result = (int)::CreateDirectory( dest_folder, NULL );
						int error = ::GetLastError();
						if ( result == 0 && error != ERROR_ALREADY_EXISTS ) {
							StringClass message;
							message.Format ("Failed to create folder %s.", dest_folder );
							MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
							return;
						}
					}

					FileClass * source_file = mix_factory.Get_File( source_name );
					FileClass * dest_file = _TheFileFactory->Get_File( dest_name );

					if ( dest_file ) {
						if ( dest_file->Open( FileClass::WRITE ) == 0 ) {
							StringClass message;
							message.Format ("Failed to open %s for writing.", dest_name );
							MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
							return;
						}
					}
					if ( source_file ) {
						if ( source_file->Open() == 0 ) {
							StringClass message;
							message.Format ("Failed to open %s for reading.", source_name );
							MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
							return;
						}
					}


					if ( source_file && source_file->Is_Available() && 
							dest_file && dest_file->Is_Available() ) {

						int length = source_file->Size();
						unsigned char file_buffer[4096];

						while ( length > 0 ) {
							int amount = std::min ( (int)length, (int)sizeof( file_buffer ) );
							if ( source_file->Read( &(file_buffer[0]), amount ) != amount ) {
								read_error = true;
								length = 0;
							}
							if ( dest_file->Write( &file_buffer[0], amount ) != amount ) {
								write_error = true;
								length = 0;
							}
							length -= amount;
						}

					}

					if ( source_file != NULL ) {
						mix_factory.Return_File( source_file );
					}
					if ( dest_file != NULL ) {
						_TheFileFactory->Return_File( dest_file );
					}

					if ( read_error ) {
						StringClass message;
						message.Format ("Read Error on %s.", source_name );
						MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
						return;
					}
					if ( write_error ) {
						StringClass message;
						message.Format ("Write Error on %s.", dest_name );
						MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
						return;
					}

//					filenames[index];
				}

			} else {
				StringClass message;
				message.Format ("Failed to create folder %s.", export_path);
				MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
			}

		} else {
			StringClass message;
			message.Format ("Error reading the filename list from %s.", current_filename);
			MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
		}		
	} else {
		StringClass message;
		message.Format ("No Current Mix File.");
		MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
	}		

	return ;
}






/////////////////////////////////////////////////////////////////////////////
//
//	OnRemoveAVAssets
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRemoveAVAssets (void)
{
	CFileDialog dialog(true, ".mix", NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_EXPLORER, "Mix File (*.dat, *.mix)|*.dat;*.mix||", this);
	dialog.m_ofn.lpstrTitle = "Pick Mix Files";

	char *file_name_buffer = new char [65536];
	*file_name_buffer = 0;

	dialog.m_ofn.lpstrFile = file_name_buffer;
	dialog.m_ofn.nMaxFile = 65535;

	if (dialog.DoModal () == IDOK) {

		//
		// Process each mix file and put the stripped mix file into a 'temp' subdirectory under the current one.
		//
		POSITION pos = dialog.GetStartPosition();

		while (pos != NULL) {
			StringClass file_name = static_cast<const char *>(dialog.GetNextPathName(pos));

			//
			// Get just the path portion.
			//
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char path[_MAX_PATH];
			char name[_MAX_PATH];
			char ext[_MAX_PATH];
			_splitpath(file_name, drive, dir, name, ext);

			//
			// Make sure the temp directory exists.
			//
			strcat(dir, "temp\\");
			_makepath(path, drive, dir, NULL, NULL);
			CreateDirectory(path, NULL);

			//
			// Make the output file name.
			//
			char output_file[_MAX_PATH];
			_makepath(output_file, drive, dir, name, ext);

			AVAssetSuckerClass sucker;
			sucker.Suck(file_name.Peek_Buffer(), output_file);
		}
	}
	delete [] file_name_buffer;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnMakeMixPatch
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnMakeMixPatch(void)
{
	//
	// Ask for the name of the old source mix file.
	//
	CFileDialog dialog(true, ".mix", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, "Mix File (*.dat, *.mix)|*.dat;*.mix||", this);
	dialog.m_ofn.lpstrTitle = "Pick Old Source Mix File";

	char file_name_buffer[1024];
	file_name_buffer[0] = 0;

	dialog.m_ofn.lpstrFile = file_name_buffer;
	dialog.m_ofn.nMaxFile = 1024;

	if (dialog.DoModal () == IDOK) {

		//
		// Pull the name of the old source mix file.
		//
		POSITION pos = dialog.GetStartPosition();
		StringClass old_file_name = static_cast<const char *>(dialog.GetNextPathName(pos));

		//
		// Ask for the name of the new source mix file.
		//
		CFileDialog dialog2(true, ".mix", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, "Mix File (*.dat, *.mix)|*.dat;*.mix||", this);
		dialog2.m_ofn.lpstrTitle = "Pick New Source Mix File";

		file_name_buffer[0] = 0;

		dialog2.m_ofn.lpstrFile = file_name_buffer;
		dialog2.m_ofn.nMaxFile = 1024;

		if (dialog2.DoModal () == IDOK) {

			//
			// Pull the name of the new source mix file.
			//
			pos = dialog2.GetStartPosition();
			StringClass new_file_name = static_cast<const char *>(dialog2.GetNextPathName(pos));


			//
			// Ask for the name of the output mix file.
			//
			CFileDialog dialog3(true, ".mix", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, "Mix File (*.dat, *.mix)|*.dat;*.mix||", this);
			dialog3.m_ofn.lpstrTitle = "Pick Output Patch Mix File";

			file_name_buffer[0] = 0;

			dialog3.m_ofn.lpstrFile = file_name_buffer;
			dialog3.m_ofn.nMaxFile = 1024;

			if (dialog3.DoModal () == IDOK) {

				//
				// Pull the name of the new source mix file.
				//
				pos = dialog3.GetStartPosition();
				StringClass out_file_name = static_cast<const char *>(dialog3.GetNextPathName(pos));

				//
				// Ask for the directory containing the old source art.
				//
				char path[_MAX_PATH + 256];
				BROWSEINFO binfo;
				binfo.hwndOwner = NULL;
				binfo.pidlRoot = NULL;
				binfo.pszDisplayName = path;
    			binfo.lpszTitle = "Select Old Source Art Directory";
				binfo.ulFlags = 0;
				binfo.lpfn = NULL;
				binfo.lParam = 0;
				binfo.iImage = 0;

				LPITEMIDLIST itemptr = SHBrowseForFolder(&binfo);

				if (itemptr) {

					char old_art_dir[_MAX_PATH + 256];

					if (SHGetPathFromIDList(itemptr, old_art_dir)) {

						//
						// Ask for the directory containing the new source art.
						//
						char path[_MAX_PATH + 256];
						BROWSEINFO binfo;
						binfo.hwndOwner = NULL;
						binfo.pidlRoot = NULL;
						binfo.pszDisplayName = path;
    					binfo.lpszTitle = "Select New Source Art Directory";
						binfo.ulFlags = 0;
						binfo.lpfn = NULL;
						binfo.lParam = 0;
						binfo.iImage = 0;

						itemptr = SHBrowseForFolder(&binfo);

						if (itemptr) {
							char new_art_dir[_MAX_PATH + 256];
							if (SHGetPathFromIDList(itemptr, new_art_dir)) {
								MixPatchMakerClass patcher;
								patcher.Make_Patch(old_file_name.Peek_Buffer(), new_file_name.Peek_Buffer(), out_file_name.Peek_Buffer(), old_art_dir, new_art_dir);
							}
						}
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
CMainFrame::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
) 
{
	return CFrameWnd::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnDropFiles
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDropFiles (HDROP hDropInfo) 
{
	SetActiveWindow ();

	CWinApp *win_app = AfxGetApp ();
	ASSERT (win_app != NULL);

	//
	//	Get the count of files from the drop query
	//
	int file_count = (int)::DragQueryFile (hDropInfo, (UINT)-1, NULL, 0);
	if (file_count > 0) {

		CMixViewerDoc *doc = (CMixViewerDoc *)GetActiveDocument ();

		if (file_count == 1) {

			//
			//	Get the filename...
			//
			TCHAR filename[_MAX_PATH];
			::DragQueryFile (hDropInfo, 0, filename, _MAX_PATH);

			//
			//	Get the extension from the filename
			//
			char extension[_MAX_EXT] = { 0 };
			::_splitpath (filename, NULL, NULL, NULL, extension);

			//
			//	Is this a mix file, or should we be adding this file to the mix?
			//
			if (	::lstrcmpi (extension, ".mix") == 0 ||
					::lstrcmpi (extension, ".dat") == 0 ||
					::lstrcmpi (extension, ".dbs") == 0)
			{
				win_app->OpenDocumentFile (filename);
			} else {
				Add_To_Mix_File (doc->GetPathName (), filename);
				doc->Reload_Views ();
			}
		
		} else {

			DynamicVectorClass<StringClass> file_list;

			//
			//	Loop over each of the dropped files and add them to the current mix file
			//
			for (int index = 0; index < file_count; index ++) {
				
				//
				//	Add this file to a list so we can batch add all the files...
				//
				TCHAR filename[_MAX_PATH];
				::DragQueryFile (hDropInfo, index, filename, _MAX_PATH);				
				file_list.Add (filename);
			}
			::DragFinish(hDropInfo);

			//
			//	Add the mix files to the list
			//
			Add_To_Mix_File (doc->GetPathName (), file_list);
			doc->Reload_Views ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Mix_File
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::Add_To_Mix_File (const char *mix_filename, const char *filename) 
{
	DynamicVectorClass<StringClass> file_list;	
	file_list.Add (filename);
	Add_To_Mix_File (mix_filename, file_list);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Mix_File
//
/////////////////////////////////////////////////////////////////////////////
void
CMainFrame::Add_To_Mix_File (const char *mix_filename, DynamicVectorClass<StringClass> &new_file_list)
{
	//
	//	Set the current directory...
	//
	CString path = Strip_Filename_From_Path (mix_filename);
	if (path.GetLength () > 0) {	
		::SetCurrentDirectory (path);
	}

	//
	//	Get access to the mix file in qestion
	//
	MixFileFactoryClass *mix_factory = new MixFileFactoryClass (mix_filename, _TheFileFactory);
	if (mix_factory->Is_Valid () && mix_factory->Build_Internal_Filename_List ()) {

		//
		//	Now add the new files into the mix file...
		//
		for (int index = 0; index < new_file_list.Count (); index ++) {
			CString filename = ::Get_Filename_From_Path (new_file_list[index]);
			mix_factory->Delete_File (filename);
			mix_factory->Add_File (new_file_list[index], filename);
		}

		//
		//	Do it!
		//
		mix_factory->Flush_Changes ();
	}

	//
	//	Free the mix factory
	//
	delete mix_factory;
	mix_factory = NULL;
	return ;
}
