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

// FilePickerClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "filepicker.h"
#include "Utils.h"
#include "filemgr.h"


///////////////////////////////////////////////////////////
//
//	FilePickerClass
//
///////////////////////////////////////////////////////////
FilePickerClass::FilePickerClass (void)
	: m_AssetTreeOnly (true),
	  PickerClass ()
{
	m_ExtensionString = "*.*";
	m_FilterString = "All Files (*.*)|*.*||";
	return ;
}


///////////////////////////////////////////////////////////
//
//	~FilePickerClass
//
///////////////////////////////////////////////////////////
FilePickerClass::~FilePickerClass (void)
{
	return ;
}


///////////////////////////////////////////////////////////
//
//	On_Pick
//
///////////////////////////////////////////////////////////
void
FilePickerClass::On_Pick (void)
{
	CString window_text;
	GetWindowText (window_text);
	CString full_path	= ::Get_File_Mgr ()->Make_Full_Path (window_text);
	CString filename	= ::Get_Filename_From_Path (full_path);
	CString path;
	
	DWORD attribs = ::GetFileAttributes (full_path);
	if ((attribs != 0xFFFFFFFF) && !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
		path = ::Strip_Filename_From_Path (full_path);
	} else if ((attribs == 0xFFFFFFFF) && (::strchr (filename, '*') != NULL)) {
		path = ::Strip_Filename_From_Path (full_path);
	} else if ((attribs == 0xFFFFFFFF) && m_AssetTreeOnly) {
		path = ::Get_File_Mgr ()->Get_Base_Path ();
	} else {
		path = full_path;
	}

	CFileDialog dialog (TRUE,
							  m_ExtensionString,
							  filename,
							  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
							  m_FilterString,
							  this);

	//
	// Set the path, so it opens in the correct directory
	//
	if (path.GetLength () > 0) {
		dialog.m_ofn.lpstrInitialDir = path;
	}

	//
	// Show the dialog to the user
	//
	if (dialog.DoModal () == IDOK) {

		path = dialog.GetPathName ();
		if (::Get_File_Mgr ()->Is_Empty_Path (path) == false)
		{
			if (m_AssetTreeOnly == false || ::Get_File_Mgr ()->Is_Path_Valid (path)) {
				CString rel_path = ::Get_File_Mgr ()->Make_Relative_Path (dialog.GetPathName ());
				SetWindowText (rel_path);
			}
		}
	}		

	PickerClass::On_Pick ();
	return ;
}

