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

#if !defined(AFX_MISSINGTRANSLATIONREPORTDIALOG_H__6A3702A2_485A_4F65_864A_6BD300BF1786__INCLUDED_)
#define AFX_MISSINGTRANSLATIONREPORTDIALOG_H__6A3702A2_485A_4F65_864A_6BD300BF1786__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"


class MissingTranslationReportDialogClass : public CDialog
{
// Construction
public:
	MissingTranslationReportDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(MissingTranslationReportDialogClass)
	enum { IDD = IDD_EXPORT_MISSING_TRANSLATIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MissingTranslationReportDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MissingTranslationReportDialogClass)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////
	void				Set_Filename (const char *filename)	{ Filename = filename; }
	const char *	Get_Filename (void) const				{ return Filename; }

private:

	//////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////
	CString		Filename;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSINGTRANSLATIONREPORTDIALOG_H__6A3702A2_485A_4F65_864A_6BD300BF1786__INCLUDED_)
