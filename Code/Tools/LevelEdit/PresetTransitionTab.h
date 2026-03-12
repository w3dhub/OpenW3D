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

#if !defined(AFX_PRESETTRANSITIONTAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_PRESETTRANSITIONTAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


#include "resource.h"
#include "dockableform.h"
#include "vector.h"
#include "transition.h"


// Forward declarations
class PresetClass;
class	RenderObjClass;


/////////////////////////////////////////////////////////////////////////////
//
// PresetTransitionTabClass
//
/////////////////////////////////////////////////////////////////////////////
class PresetTransitionTabClass : public DockableFormClass
{

public:
	PresetTransitionTabClass (PresetClass *preset);
	virtual ~PresetTransitionTabClass (void);

// Form Data
public:
	//{{AFX_DATA(PresetTransitionTabClass)
	enum { IDD = IDD_TRANSITION_LIST };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PresetTransitionTabClass)
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
	//{{AFX_MSG(PresetTransitionTabClass)
	afx_msg void OnItemChangedTransitionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddButton();
	afx_msg void OnDeleteButton();
	afx_msg void OnModifyButton();
	afx_msg void OnDeleteitemTransitionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTransitionList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////
	void						HandleInitDialog (void);
	bool						Apply_Changes (void);

	TRANSITION_DATA_LIST *	Get_Transition_List (void) const						{ return m_TransitionList; }
	void						Set_Transition_List (TRANSITION_DATA_LIST *list)	{ m_TransitionList = list; }

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////
	void						Create_Render_Obj (void);
	void						Insert_Transition (TransitionDataClass *transition);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	PresetClass *				m_Preset;
	RenderObjClass	*			m_RenderObj;
	float							m_Height;
	TRANSITION_DATA_LIST *	m_TransitionList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETTRANSITIONTAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
