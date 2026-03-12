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

#if !defined(AFX_PRESETDIALOGUETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_PRESETDIALOGUETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PresetDialogueTab.h : header file
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "resource.h"
#include "dockableform.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class PresetClass;
class SoldierGameObjDef;
class DialogueClass;

/////////////////////////////////////////////////////////////////////////////
//
// PresetDialogueTabClass
//
/////////////////////////////////////////////////////////////////////////////
class PresetDialogueTabClass : public DockableFormClass
{

public:
	PresetDialogueTabClass (PresetClass *preset);
	virtual ~PresetDialogueTabClass (void);

// Form Data
public:
	//{{AFX_DATA(PresetDialogueTabClass)
	enum { IDD = IDD_PRESET_DIALOGUE_TAB };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PresetDialogueTabClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(PresetDialogueTabClass)
	afx_msg void OnDblclkDialogueList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////////////
	void						HandleInitDialog (void);
	bool						Apply_Changes (void);

	bool						Is_Read_Only (void) const	{ return m_IsReadOnly; }
	void						Set_Read_Only (bool onoff)	{ m_IsReadOnly = onoff; }

protected:

	/////////////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////////////
	void						Update_Entry (int index);

private:

	/////////////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////////////
	DialogueClass *		m_DialogueList;
	SoldierGameObjDef *	m_Definition;
	bool						m_IsReadOnly;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETDIALOGUETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
