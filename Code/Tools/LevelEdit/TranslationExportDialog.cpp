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

// TranslationExportDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "translationexportdialog.h"
#include "translatedb.h"
#include "export.h"
#include "Utils.h"
#include "stringsmgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TranslationExportDialogClass dialog


TranslationExportDialogClass::TranslationExportDialogClass(CWnd* pParent /*=NULL*/)	:
	IsInstaller (false),
	CDialog(TranslationExportDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(TranslationExportDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


void
TranslationExportDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TranslationExportDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TranslationExportDialogClass, CDialog)
	//{{AFX_MSG_MAP(TranslationExportDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TranslationExportDialogClass message handlers


///////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
///////////////////////////////////////////////////////////////////////////
BOOL
TranslationExportDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();
	
	//
	//	Select english by default
	//
	SendDlgItemMessage (IDC_LANG_COMBO, CB_SETCURSEL, TranslateDBClass::LANGID_ENGLISH);	
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
///////////////////////////////////////////////////////////////////////////
void
TranslationExportDialogClass::OnOK (void) 
{
	//
	//	Export the data...
	//
	int lang_id = SendDlgItemMessage (IDC_LANG_COMBO, CB_GETCURSEL);
	if (lang_id >= 0) {

		//
		//	Make sure the translate database is loaded into memory
		//
		StringsMgrClass::Load_Translation_Database ();
		
		//
		//	Switch the database to the requested language...
		//
		TranslateDBClass::Set_Current_Language (lang_id);
		TranslateDBClass::Enable_Single_Language_Export (true);

		//
		//	Export the installer string separate from the game strings
		//
		if (IsInstaller == false) {
			Export_Game_Strings ();
		} else {
			Export_Installer_Strings ();
		}

		//
		//	Restore english
		//
		TranslateDBClass::Enable_Single_Language_Export (false);
		TranslateDBClass::Set_Current_Language (TranslateDBClass::LANGID_ENGLISH);
	}
	
	CDialog::OnOK();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Export_Game_Strings
//
///////////////////////////////////////////////////////////////////////////
void
TranslationExportDialogClass::Export_Game_Strings (void)
{
#if 0
	//
	//	Don't export the installer strings...
	//
	TDBCategoryClass *category = TranslateDBClass::Find_Category ("Installer");
	if (category != NULL) {

		//
		//	Filter out any strings that do not belong to this category
		//
		TranslateDBClass::Set_Export_Filter (TranslateDBClass::FILTER_IF_EQUAL, category->Get_ID ());
	}
#endif

	//
	//	Export this language
	//
	ExporterClass exporter;
	exporter.Export_Database_Mix (::Strip_Filename_From_Path (Filename));

	//
	//	Turn off any filters we may have enabled
	//
	TranslateDBClass::Set_Export_Filter (TranslateDBClass::FILTER_DISABLED, 0);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Export_Installer_Strings
//
///////////////////////////////////////////////////////////////////////////
void
TranslationExportDialogClass::Export_Installer_Strings (void)
{
	//
	//	Find the category we wish to export
	//
	TDBCategoryClass *category = TranslateDBClass::Find_Category ("Installer");
	if (category != NULL) {

		//
		//	Filter out any strings that do not belong to this category
		//
		TranslateDBClass::Set_Export_Filter (TranslateDBClass::FILTER_IF_NOT_EQUAL, category->Get_ID ());
		
		//
		//	Save the database and restore the default filter
		//
		StringsMgrClass::Save_Translation_Database (Filename);
		TranslateDBClass::Set_Export_Filter (TranslateDBClass::FILTER_DISABLED, 0);
	}

	return ;
}

