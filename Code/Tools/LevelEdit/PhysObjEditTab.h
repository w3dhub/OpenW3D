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

#if !defined(AFX_PHYSOBJEDITDIALOG_H__D527BB16_7C13_11D3_A05F_00104B791122__INCLUDED_)
#define AFX_PHYSOBJEDITDIALOG_H__D527BB16_7C13_11D3_A05F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhysObjEditDialog.h : header file
//

#include "resource.h"
#include "DockableForm.h"


// Forward declaration
class SpecSheetClass;
class ModelDefParameterClass;


/////////////////////////////////////////////////////////////////////////////
//
// PhysObjEditTabClass
//
/////////////////////////////////////////////////////////////////////////////
class PhysObjEditTabClass : public DockableFormClass
{
public:
	PhysObjEditTabClass(void);
	~PhysObjEditTabClass (void);

// Dialog Data
	//{{AFX_DATA(PhysObjEditTabClass)
	enum { IDD = IDD_PHYSICS_OBJECT_EDIT };
	CComboBox	m_ObjTypeCombo;
	CButton	m_SettingsGroup;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PhysObjEditTabClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PhysObjEditTabClass)
	afx_msg void OnDestroy();
	afx_msg void OnSelChangeTypeCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	void				HandleInitDialog (void);
	bool				Apply_Changes (void);

	int				Get_Definition_ID (void) const	{ return m_DefinitionID; }
	void				Set_Definition_ID (int id)			{ m_DefinitionID = id; }
	void				Set_Filter (LPCTSTR filter)		{ m_FilterString = filter; }
	void				Set_Is_Temp (bool is_temp)			{ m_IsTemp = is_temp; }

	bool				Get_Read_Only (void) const			{ return m_ReadOnly; }
	void				Set_Read_Only (bool read_only)	{ m_ReadOnly = read_only; }

	ModelDefParameterClass *	Get_PhysDef_Param (void) const							{ return m_PhysDefParam; }
	void							Set_PhysDef_Param (ModelDefParameterClass *param)	{ m_PhysDefParam = param; }

private:

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	ModelDefParameterClass *	m_PhysDefParam;
	SpecSheetClass	*			m_ParamSheet;
	unsigned int				m_DefinitionID;
	CString						m_FilterString;
	bool							m_IsTemp;
	bool							m_ReadOnly;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSOBJEDITDIALOG_H__D527BB16_7C13_11D3_A05F_00104B791122__INCLUDED_)
