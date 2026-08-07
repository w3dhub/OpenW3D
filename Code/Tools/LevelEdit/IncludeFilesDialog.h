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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/IncludeFilesDialog.h                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if !defined(AFX_INCLUDEFILESDIALOG_H__D3883AD0_DBF1_11D2_A00E_00104B791122__INCLUDED_)
#define AFX_INCLUDEFILESDIALOG_H__D3883AD0_DBF1_11D2_A00E_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FilePicker.H"

/////////////////////////////////////////////////////////////////////////////
//
// IncludeFilesDialogClass
//
class IncludeFilesDialogClass : public CDialog
{
// Construction
public:
	IncludeFilesDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(IncludeFilesDialogClass)
	enum { IDD = IDD_ALWAYS };
	CTreeCtrl	m_IncludesTreeCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(IncludeFilesDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(IncludeFilesDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangedIncludeTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeSpecEdit();
	afx_msg void OnAddRemoveButton();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	protected:

		////////////////////////////////////////////////////////
		//
		//	Protected member data
		//
		void						Update_Add_Remove_Button (void);
		HTREEITEM				Find_Spec (LPCTSTR filespec, HTREEITEM hroot);

	private:

		////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		HTREEITEM				m_hGlobalFolder;
		HTREEITEM				m_hLevelFolder;
		FilePickerClass		m_FilePicker;
		HTREEITEM				m_CurrentItem;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INCLUDEFILESDIALOG_H__D3883AD0_DBF1_11D2_A00E_00104B791122__INCLUDED_)
