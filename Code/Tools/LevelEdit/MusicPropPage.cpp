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

// MusicPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "MusicPropPage.h"
#include "AudibleSound.h"
#include "FileMgr.h"
#include "Phys.h"
#include "SceneEditor.h"
#include "SoundBuffer.h"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MusicPropPage property page

IMPLEMENT_DYNCREATE(MusicPropPageClass, CPropertyPage)

MusicPropPageClass::MusicPropPageClass() : CPropertyPage(MusicPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(MusicPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

MusicPropPageClass::~MusicPropPageClass()
{
}

void MusicPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MusicPropPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MusicPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(MusicPropPageClass)
	ON_BN_CLICKED(IDC_MUSIC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MusicPropPageClass message handlers

BOOL MusicPropPageClass::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	//
	//	Put the background music pathname into the edit control.
	//
	SceneEditorClass *sceneeditor = ::Get_Scene_Editor();
	CString filename = sceneeditor->Get_Background_Music_Filename ();
	if (filename.GetLength () > 0) {
		CString pathname = ::Get_File_Mgr()->Make_Full_Path (filename);
		SetDlgItemText (IDC_MUSIC_PATHNAME, pathname);
	}
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


void MusicPropPageClass::OnBrowse() 
{
	// Determine what filename and path to initially display in the dialog.
	CString defaultpathname;
	CString pathname;

	if (GetDlgItemText (IDC_MUSIC_PATHNAME, defaultpathname) > 0) {
		defaultpathname = ::Get_File_Mgr()->Make_Full_Path (defaultpathname);
		pathname = ::Strip_Filename_From_Path (defaultpathname);
	} else {
		pathname = ::Get_File_Mgr()->Get_Base_Path();
	}

	CFileDialog dialog (true, ".wav", defaultpathname,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER, "All Sound Files|*.wav;*.mp3|WAV File (*.wav)|*.wav|MP3 File (*.mp3)|*.mp3||", this);

	// Set the pathname so it opens in the correct directory.
	dialog.m_ofn.lpstrInitialDir = pathname;

	// Ask the user which new asset file to use.
	if (dialog.DoModal () == IDOK) {

		// Is the new path valid?
		if (::Get_File_Mgr ()->Is_Path_Valid (dialog.GetPathName())) {

			// Convert the new asset file path to a relative path and pass it onto the base.
			CString relativepathname = ::Get_File_Mgr ()->Make_Relative_Path (dialog.GetPathName());
			SetDlgItemText (IDC_MUSIC_PATHNAME, relativepathname);

		} else {
			
			// Let the user know that this pathname is invalid.
			CString message;
			CString title;
			message.Format (IDS_INVALID_MODEL_PATH_MSG, (LPCTSTR)::Get_File_Mgr()->Get_Base_Path());
			title.LoadString (IDS_INVALID_MODEL_PATH_TITLE);			
			::MessageBox (m_hWnd, message, title, MB_ICONERROR | MB_OK);
		}		
	}
}


void MusicPropPageClass::OnOK() 
{
	//	Pass the music pathname onto the scene editor.
	CString pathname;

	GetDlgItemText (IDC_MUSIC_PATHNAME, pathname);
	SceneEditorClass *sceneeditor = ::Get_Scene_Editor();
	sceneeditor->Set_Background_Music (pathname);

	// Check this file into VSS (if possible).
	if (::Get_File_Mgr()->Is_VSS_Read_Only() == false) {
		::Get_File_Mgr ()->Add_File_To_Database (pathname);
	}

	CPropertyPage::OnOK();
	return ;
}
