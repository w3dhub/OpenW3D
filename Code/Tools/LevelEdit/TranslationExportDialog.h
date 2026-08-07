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

#if !defined(AFX_TRANSLATIONEXPORTDIALOG_H__C9CE9433_7238_49CF_A222_29D0D36FA529__INCLUDED_)
#define AFX_TRANSLATIONEXPORTDIALOG_H__C9CE9433_7238_49CF_A222_29D0D36FA529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TranslationExportDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TranslationExportDialogClass dialog

class TranslationExportDialogClass : public CDialog
{
// Construction
public:
	TranslationExportDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TranslationExportDialogClass)
	enum { IDD = IDD_EXPORT_LANGUAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TranslationExportDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TranslationExportDialogClass)
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

	void				Set_Is_Installer (bool onoff)			{ IsInstaller = onoff; }
	bool				Is_Installer (void) const				{ return IsInstaller; }

private:

	//////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////
	void				Export_Game_Strings (void);
	void				Export_Installer_Strings (void);

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	CString		Filename;
	bool			IsInstaller;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSLATIONEXPORTDIALOG_H__C9CE9433_7238_49CF_A222_29D0D36FA529__INCLUDED_)
