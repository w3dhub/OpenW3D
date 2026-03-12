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

#if !defined(AFX_PRESETZONETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_PRESETZONETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "resource.h"
#include "dockableform.h"
#include "vector.h"
#include "zone.h"
#include "listtypes.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class PresetClass;
class	PhysClass;


/////////////////////////////////////////////////////////////////////////////
//
// PresetZoneTabClass
//
/////////////////////////////////////////////////////////////////////////////
class PresetZoneTabClass : public DockableFormClass
{

public:
	PresetZoneTabClass (PresetClass *preset);
	virtual ~PresetZoneTabClass (void);

// Form Data
public:
	//{{AFX_DATA(PresetZoneTabClass)
	enum { IDD = IDD_ZONE_LIST };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PresetZoneTabClass)
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
	//{{AFX_MSG(PresetZoneTabClass)
	afx_msg void OnDblclkZoneList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////
	void						HandleInitDialog (void);
	bool						Apply_Changes (void);

	ZONE_PARAM_LIST *		Get_Zone_Param_List (void) const					{ return m_ZoneList; }
	void						Set_Zone_Param_List (ZONE_PARAM_LIST *list)	{ m_ZoneList = list; }

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////
	void						Create_Phys_Obj (void);
	void						Modify_Selected_Zone (void);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	PresetClass *			m_Preset;
	PhysClass	*			m_PhysObj;
	ZONE_PARAM_LIST *		m_ZoneList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETZONETAB_H__9CC8BAE8_7052_11D3_A05A_00104B791122__INCLUDED_)
