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

// MakeMixFileDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "mixviewer.h"
#include "MakeMixFileDialog.h"
#include "wwstring.h"
#include "MainFrm.h"
#include "mixfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MakeMixFileDialogClass dialog


MakeMixFileDialogClass::MakeMixFileDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(MakeMixFileDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(MakeMixFileDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MakeMixFileDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MakeMixFileDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MakeMixFileDialogClass, CDialog)
	//{{AFX_MSG_MAP(MakeMixFileDialogClass)
	ON_BN_CLICKED(IDC_BROWSE_FILE, OnBrowseFile)
	ON_BN_CLICKED(IDC_BROWSE_DIR, OnBrowseDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MakeMixFileDialogClass message handlers

BOOL MakeMixFileDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
**
*/
unsigned MakeMixFileDialogClass::Add_Files (const StringClass &basepath, const StringClass &subpath, MixFileCreator &mixfile)
{
	const char wildcardname [] = "*.*";

	unsigned			 filecount;
	StringClass		 findfilepathname;	
	WIN32_FIND_DATA finddata;	
	HANDLE			 handle;

	filecount = 0;
	if (basepath.Get_Length() > 0) {
		findfilepathname  = basepath;
		findfilepathname += "\\";
	}
	if (subpath.Get_Length() > 0) {
		findfilepathname += subpath;
		findfilepathname += "\\";
	}
	findfilepathname += wildcardname;
	handle = FindFirstFile (findfilepathname, &finddata);
	if (handle != INVALID_HANDLE_VALUE) {
		
		bool done;

		done = false;
		while (!done) {

			// Filter out system files.
  			if (finddata.cFileName [0] != '.') {

				StringClass subpathname;

				if (subpath.Get_Length() > 0) {
					subpathname += subpath;
					subpathname += "\\";
				}
				subpathname += finddata.cFileName;

				// Is it a subdirectory?
	  			if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				
					// Recurse on subdirectory.
					filecount += Add_Files (basepath, subpathname, mixfile);

				} else {

					StringClass fullpathname;

					if (basepath.Get_Length() > 0) {
						fullpathname += basepath;
						fullpathname += "\\";
					}
					if (subpath.Get_Length() > 0) {
						fullpathname += subpath;
						fullpathname += "\\";
					}
					fullpathname += finddata.cFileName;
					if ( MixFilename != fullpathname ) {
						mixfile.Add_File (fullpathname, subpathname);
						filecount++;
					}
				}
			}
			done = !FindNextFile (handle, &finddata);
		}
	}
	return (filecount);
}

void MakeMixFileDialogClass::OnOK() 
{
	CString mix_name;
	GetDlgItemText( IDC_BROWSE_FILE_NAME, mix_name );
	CString dir_name;
	GetDlgItemText( IDC_BROWSE_DIR_NAME, dir_name );

	if ( mix_name.IsEmpty() ) {
		StringClass message;
		message.Format ("Invalid Mix File.");
		MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
	} else if ( dir_name.IsEmpty() ) {
		StringClass message;
		message.Format ("Invalid Source Directory.");
		MessageBox (message, "Mix File Error", MB_ICONERROR | MB_OK);
	} else {

		MixFilename = mix_name;
		MixFileCreator	mixfile (mix_name);
		StringClass basepath (dir_name);
		StringClass subpath;
		StringClass message;
		int filecount = Add_Files (basepath, subpath, mixfile);
		if (filecount > 0) {
			message.Format ("%u files added\n", filecount);
		} else {
			message.Format ("No files found in source directory\n");
		}
		MessageBox (message, "Mix File Created", MB_OK);

		CDialog::OnOK();
	}
}

void MakeMixFileDialogClass::OnBrowseFile() 
{
	CString old_name;
	GetDlgItemText( IDC_BROWSE_FILE_NAME, old_name );

	CFileDialog dialog (	TRUE,
								".mix",
								old_name,
								OFN_HIDEREADONLY | OFN_EXPLORER,
								"Mix File (*.mix)|*.mix||",
								this);

	dialog.m_ofn.lpstrTitle = "Pick Mix File to Create";
	if (dialog.DoModal () == IDOK) {
		SetDlgItemText( IDC_BROWSE_FILE_NAME, dialog.GetPathName () );
	}
}

void MakeMixFileDialogClass::OnBrowseDir() 
{
	CString old_name;
	GetDlgItemText( IDC_BROWSE_DIR_NAME, old_name );

	CFileDialog dialog (	TRUE,
								NULL, //".",
								old_name,
								OFN_HIDEREADONLY | OFN_EXPLORER,
								NULL, //"Mix File (*.mix)|*.mix||",
								this);

	dialog.m_ofn.lpstrTitle = "Pick A File In the Root Source Directory";
	if (dialog.DoModal () == IDOK) {
		StringClass filename	= static_cast<const char *>(dialog.GetPathName ());
		StringClass directory	= static_cast<const char *>(Strip_Filename_From_Path (filename));
		SetDlgItemText( IDC_BROWSE_DIR_NAME, directory );
	}
}
