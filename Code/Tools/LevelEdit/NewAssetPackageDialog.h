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

#if !defined(AFX_NEWASSETPACKAGEDIALOG_H__C37F6692_87EF_4A1C_AAFE_AE04BA2B2C24__INCLUDED_)
#define AFX_NEWASSETPACKAGEDIALOG_H__C37F6692_87EF_4A1C_AAFE_AE04BA2B2C24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// NewAssetPackageDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class NewAssetPackageDialogClass : public CDialog
{
// Construction
public:
	NewAssetPackageDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NewAssetPackageDialogClass)
	enum { IDD = IDD_NEW_PACKAGE_NAME };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewAssetPackageDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NewAssetPackageDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	void					Set_Package_Name (const char *name)	{ PackageName = name; }
	const CString &	Get_Package_Name (void)	const			{ return PackageName; }

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	bool					Is_Duplicate_Name (const CString &name);
	bool					Is_Valid_Name (const CString &name);

private:

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	CString			PackageName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWASSETPACKAGEDIALOG_H__C37F6692_87EF_4A1C_AAFE_AE04BA2B2C24__INCLUDED_)
