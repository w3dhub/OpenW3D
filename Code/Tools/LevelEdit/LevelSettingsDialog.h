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

#if !defined(AFX_LEVELSETTINGSDIALOG_H__D2D54B36_73E7_4CBB_AC91_8F9FB05FCEBC__INCLUDED_)
#define AFX_LEVELSETTINGSDIALOG_H__D2D54B36_73E7_4CBB_AC91_8F9FB05FCEBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LevelSettingsDialog.h : header file
//

#include "filepicker.h"
#include "stringpicker.h"


/////////////////////////////////////////////////////////////////////////////
//
// LevelSettingsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class LevelSettingsDialogClass : public CDialog
{
// Construction
public:
	LevelSettingsDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(LevelSettingsDialogClass)
	enum { IDD = IDD_LEVEL_SETTINGS };
	CComboBox	RestartScriptCombo;
	CComboBox	RespawnScriptCombo;
	CSpinButtonCtrl	MapScaleYSpin;
	CSpinButtonCtrl	MapScaleXSpin;
	CSpinButtonCtrl	MapOffsetYSpin;
	CSpinButtonCtrl	MapOffsetXSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LevelSettingsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LevelSettingsDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	///////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////
	void			Fill_Script_Combobox (int ctrl_id, const char *default_name);

	///////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////
	FilePickerClass		FilePicker;
	StringPickerClass		StringPicker;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELSETTINGSDIALOG_H__D2D54B36_73E7_4CBB_AC91_8F9FB05FCEBC__INCLUDED_)
