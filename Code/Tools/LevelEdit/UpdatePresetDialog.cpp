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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/UpdatePresetDialog.cpp                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "leveledit.h"
#include "updatepresetdialog.h"
#include "utils.h"
#include "filemgr.h"
#include "filelocations.h"
#include "editorini.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "ffactory.h"
#include "regkeys.h"
#include "specsheet.h"
#include "preset.h"
#include "parameter.h"
#include "definitionutils.h"
#include "assetdatabase.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// UpdatePresetDialogClass
//
/////////////////////////////////////////////////////////////////////////////
UpdatePresetDialogClass::UpdatePresetDialogClass
(
	PresetClass *	preset,
	CWnd *			parent
)
	: m_Preset (preset),
	  CDialog (UpdatePresetDialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(UpdatePresetDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// UpdatePresetDialogClass
//
/////////////////////////////////////////////////////////////////////////////
void
UpdatePresetDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(UpdatePresetDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(UpdatePresetDialogClass, CDialog)
	//{{AFX_MSG_MAP(UpdatePresetDialogClass)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_FILENAME_EDIT, OnChangeFilenameEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
UpdatePresetDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	SANITY_CHECK (m_Preset != NULL && m_Preset->Get_Definition () != NULL) {
		return TRUE;
	}
	
	//
	//	Lookup the model-definition for this preset
	//
	DefinitionClass *definition = ::Find_Physics_Definition (m_Preset->Get_Definition ());
	if (definition == NULL) {
		definition = m_Preset->Get_Definition ();
	}
	
	//
	//	Get the rectangle where we will be creating the 'spec-sheet'.
	//
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_FILE_GROUP), &rect);		
	ScreenToClient (&rect);
	rect.top += 12;
	rect.bottom -= 5;
	rect.left += 10;
	rect.right -= 10;

	//
	//	Create the spec-sheet that lists only file parameters
	//
	m_ParamSheet = new SpecSheetClass (definition);
	m_ParamSheet->Show_File_Only (true);
	m_ParamSheet->Set_Asset_Tree_Only (false);
	m_ParamSheet->Create ("static", "", WS_CHILD | WS_VISIBLE, rect, this, 101);

	//
	//	Build a list of local filenames
	//
	int count = m_ParamSheet->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *parameter = m_ParamSheet->Get_Parameter (index);
		if (parameter != NULL) {
			m_LocalFileList.Add (((StringParameterClass *)parameter)->Get_String ());
		}
	}

	//
	//	Provide some default comments
	//
	CString comments;
	comments.Format ("Modified preset: %s.", m_Preset->Get_Name ());
	SetDlgItemText (IDC_COMMENTS_EDIT, comments);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowse
//
/////////////////////////////////////////////////////////////////////////////
void
UpdatePresetDialogClass::OnBrowse (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnChangeFilenameEdit
//
/////////////////////////////////////////////////////////////////////////////
void
UpdatePresetDialogClass::OnChangeFilenameEdit (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
UpdatePresetDialogClass::OnOK (void)
{
	int count = m_ParamSheet->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
				
		//
		//	Get the original and new filenames
		//
		CString orig_full_path	= ::Get_File_Mgr ()->Make_Full_Path (m_LocalFileList[index]);
		CString orig_path			= ::Strip_Filename_From_Path (orig_full_path);
		CString new_path;
		m_ParamSheet->Get_Current_Filename_Value (index, new_path);
		new_path = ::Get_File_Mgr ()->Make_Full_Path (new_path);

		//
		//	Only update
		//
		CString orig_filename	= ::Get_Filename_From_Path (orig_full_path);
		CString new_filename		= ::Get_Filename_From_Path (new_path);
		if (orig_filename.CompareNoCase (new_filename) == 0) {

			//
			//	If the user changed the path, then ask the file manager to
			//	update the file.
			//
			if (orig_full_path.CompareNoCase (new_path) != 0) {
				::Get_File_Mgr ()->Update_Model (orig_path, new_path);
				
				//
				//	Update the INI file so it 'knows' about this latest update
				//
				CString rel_folder = ::Get_File_Mgr ()->Make_Relative_Path (orig_path);
				Update_INI (rel_folder);
			}
		} else {
			
			//
			//	Warn the user that we cannot do this operation...
			//
			ParameterClass *parameter = m_ParamSheet->Get_Parameter (index);
			CString message;
			message.Format ("Cannot update %s, new filename MUST match original filename.", parameter->Get_Name ());
			MessageBox (message, "Invalid Entry", MB_ICONEXCLAMATION | MB_OK);
		}
	}
	
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_INI
//
/////////////////////////////////////////////////////////////////////////////
void
UpdatePresetDialogClass::Update_INI (LPCTSTR rel_folder)
{		
	//
	// Attempt to check out the INI file
	//
	CString path						= ::Get_File_Mgr ()->Make_Full_Path (UPDATE_INI_PATH);
	AssetDatabaseClass &asset_db	= ::Get_File_Mgr ()->Get_Database_Interface ();

	if (asset_db.Retry_Check_Out (path, 10, 1000)) {
				
		//
		// Get a pointer to the INI file from the asset manager		
		//
		EditorINIClass *pini = _pThe3DAssetManager->Get_INI (path);		
		ASSERT (pini != NULL);	
		if (pini != NULL) {

			//
			// Update the current update ID in the INI file
			//
			int curr_update_id = pini->Get_Int ("General", "ID");
			curr_update_id ++;
			pini->Put_Int ("General", "ID", curr_update_id);

			//
			//	Write the (relative) folder name of the updated asset out to the INI.
			//
			CString number_string;
			number_string.Format ("%d", curr_update_id);
			pini->Put_String ("Details", number_string, rel_folder);
			
			//
			// Update the current time/date in the INI file
			//
			CTime current_time = CTime::GetCurrentTime ();
			CString time_string = current_time.Format ("%I:%M%p");
			CString date_string = current_time.Format ("%A, %B %d, %Y");
			pini->Put_String ("General", "Time", time_string);
			pini->Put_String ("General", "Date", date_string);

			//
			// Update the comments in the INI file
			//
			TCHAR comments[512];
			GetDlgItemText (IDC_COMMENTS_EDIT, comments, sizeof (comments));
			for (LPSTR newline = ::strchr (comments, '\r');
			     newline != NULL;
				  newline = ::strchr (newline, '\r'))
			{
				newline[0] = '|';
				newline[1] = '|';
			}
			pini->Put_TextBlock ("Comments", comments);

			//
			// Save the INI file
			//
			FileClass *pini_file = _TheFileFactory->Get_File (path);
			if (pini_file) {
				pini->Save (*pini_file);
				_TheFileFactory->Return_File (pini_file);
			}

			//
			// If the user was up-to-date before this operation, then write
			// his new update ID in the registry.
			//
			int update_id = theApp.GetProfileInt (CONFIG_KEY, LAST_UPDATE_VALUE, 0);
			if (update_id == (curr_update_id - 1)) {
				theApp.WriteProfileInt (CONFIG_KEY, LAST_UPDATE_VALUE, curr_update_id);
			}

			// Free the INI object
			SAFE_DELETE (pini);			
		}

		// Now try to check in the file
		asset_db.Retry_Check_In (path, 10, 250);
	}

	return ;
}

