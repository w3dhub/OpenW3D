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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/UpdatePresetDialog.h                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if !defined(AFX_UPDATEPRESETDIALOG_H__E2337FFD_C5CF_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_UPDATEPRESETDIALOG_H__E2337FFD_C5CF_11D2_9FFA_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "listtypes.h"

//
// Forward delcarations
//
class PresetClass;
class SpecSheetClass;


/////////////////////////////////////////////////////////////////////////////
//
// UpdatePresetDialogClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class UpdatePresetDialogClass : public CDialog
{
// Construction
public:
	UpdatePresetDialogClass (PresetClass *preset, CWnd* pParent = nullptr);

// Dialog Data
	//{{AFX_DATA(UpdatePresetDialogClass)
	enum { IDD = IDD_UPDATE_VSS_ASSET };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(UpdatePresetDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(UpdatePresetDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnChangeFilenameEdit();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected  methods
	//////////////////////////////////////////////////////////////////////
	void					Update_INI (LPCTSTR rel_folder);

private:

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	PresetClass *		m_Preset;
	CString				m_ModelFileName;
	CString				m_ModelLocalPath;

	STRING_LIST			m_LocalFileList;

	SpecSheetClass *	m_ParamSheet;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPRESETDIALOG_H__E2337FFD_C5CF_11D2_9FFA_00104B791122__INCLUDED_)
