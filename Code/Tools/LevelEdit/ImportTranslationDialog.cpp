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

// ImportTranslationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "importtranslationdialog.h"
#include "stringsmgr.h"
#include "translatedb.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////
//
//	ImportTranslationDialogClass
//
///////////////////////////////////////////////////////////////////////////
ImportTranslationDialogClass::ImportTranslationDialogClass (CWnd* pParent /*=NULL*/)
	:	IsForExport (false),
		CDialog(ImportTranslationDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ImportTranslationDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
///////////////////////////////////////////////////////////////////////////
void
ImportTranslationDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ImportTranslationDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ImportTranslationDialogClass, CDialog)
	//{{AFX_MSG_MAP(ImportTranslationDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
///////////////////////////////////////////////////////////////////////////
BOOL
ImportTranslationDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	if (IsForExport) {
		SetDlgItemText (IDC_MESSAGE, "Which language are you exporting?");
		SetWindowText ("Translation Export");
	}

	//
	//	Select english by default
	//
	SendDlgItemMessage (IDC_LANG_COMBO, CB_SETCURSEL, TranslateDBClass::LANGID_ENGLISH);	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
///////////////////////////////////////////////////////////////////////////
void
ImportTranslationDialogClass::OnOK (void)
{
	if (IsForExport) {

		//
		//	Export the data
		//
		int lang_id = SendDlgItemMessage (IDC_LANG_COMBO, CB_GETCURSEL);
		if (lang_id >= 0) {
			StringsMgrClass::Export_For_Translation (Filename, lang_id);
		}
		
	} else {
		
		//
		//	Import the data
		//
		int lang_id = SendDlgItemMessage (IDC_LANG_COMBO, CB_GETCURSEL);
		if (lang_id >= 0) {
			StringsMgrClass::Import_From_Translation (Filename, lang_id);
		}
	}

	CDialog::OnOK ();
	return ;
}
