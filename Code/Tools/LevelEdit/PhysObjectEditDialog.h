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

#if !defined(AFX_PHYSOBJECTEDITDIALOG_H__4F3E7434_C94F_11D3_A085_00104B791122__INCLUDED_)
#define AFX_PHYSOBJECTEDITDIALOG_H__4F3E7434_C94F_11D3_A085_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "PhysObjEditTab.h"


/////////////////////////////////////////////////////////////////////////////
//
// PhysObjectEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class PhysObjectEditDialogClass : public CDialog
{
// Construction
public:
	PhysObjectEditDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PhysObjectEditDialogClass)
	enum { IDD = IDD_PHYSICS_OBJECT_EDIT_CONTAINER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PhysObjectEditDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PhysObjectEditDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////////
	int				Get_Definition_ID (void) const	{ return m_ObjectEditForm. Get_Definition_ID (); }
	void				Set_Definition_ID (int id)			{ m_ObjectEditForm. Set_Definition_ID (id); }
	void				Set_Filter (LPCTSTR filter)		{ m_ObjectEditForm. Set_Filter (filter); }
	void				Set_Is_Temp (bool is_temp)			{ m_ObjectEditForm. Set_Is_Temp (is_temp); }

	bool				Get_Read_Only (void) const			{ return m_ObjectEditForm. Get_Read_Only (); }
	void				Set_Read_Only (bool read_only)	{ m_ObjectEditForm. Set_Read_Only (read_only); }

	ModelDefParameterClass *	Get_PhysDef_Param (void) const						{ return m_ObjectEditForm. Get_PhysDef_Param (); }
	void							Set_PhysDef_Param (ModelDefParameterClass *param)	{ m_ObjectEditForm. Set_PhysDef_Param (param); }

private:

	///////////////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////////////
	PhysObjEditTabClass		m_ObjectEditForm;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSOBJECTEDITDIALOG_H__4F3E7434_C94F_11D3_A085_00104B791122__INCLUDED_)
