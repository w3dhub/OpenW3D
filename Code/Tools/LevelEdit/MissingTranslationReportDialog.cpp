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

// MissingTranslationReportDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "missingtranslationreportdialog.h"
#include "reportmgr.h"
#include "translatedb.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MissingTranslationReportDialogClass dialog


MissingTranslationReportDialogClass::MissingTranslationReportDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(MissingTranslationReportDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(MissingTranslationReportDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


void
MissingTranslationReportDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MissingTranslationReportDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(MissingTranslationReportDialogClass, CDialog)
	//{{AFX_MSG_MAP(MissingTranslationReportDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
///////////////////////////////////////////////////////////////////////////
BOOL
MissingTranslationReportDialogClass::OnInitDialog (void) 
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
MissingTranslationReportDialogClass::OnOK (void) 
{
	//
	//	Import the data
	//
	int lang_id = SendDlgItemMessage (IDC_LANG_COMBO, CB_GETCURSEL);
	if (lang_id >= 0) {
		ReportMgrClass::Export_Missing_Translation_Report (Filename, lang_id);
	}
	
	CDialog::OnOK();
	return ;
}
