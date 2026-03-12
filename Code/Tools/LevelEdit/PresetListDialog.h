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

#if !defined(AFX_PRESETLISTDIALOG_H__CFD5DA22_01FA_11D4_A091_00104B791122__INCLUDED_)
#define AFX_PRESETLISTDIALOG_H__CFD5DA22_01FA_11D4_A091_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "vector.h"
#include "bittype.h"
#include "ntree.h"


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class PresetClass;


/////////////////////////////////////////////////////////////////////////////
//
// PresetListDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class PresetListDialogClass : public CDialog
{
// Construction
public:
	PresetListDialogClass (CWnd *pParent = NULL);

// Dialog Data
	//{{AFX_DATA(PresetListDialogClass)
	enum { IDD = IDD_PRESET_LIST };
	CComboBoxEx	m_ComboBox;
	CTreeCtrl	m_PresetTreeCtrl;
	CListCtrl	m_PresetListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PresetListDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PresetListDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnDblclkPresetList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkPresetTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedPresetList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedPresetTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnSelendokTypeCombo();
	afx_msg void OnSelchangeTypeCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////
	void			Set_Preset_List (DynamicVectorClass<int> *list)	{ m_List = list; }
	void			Set_Class_ID (uint32 class_id)						{ m_RootClassID = class_id; }
	void			Set_Selected_Class_ID (uint32 class_id)			{ m_ClassID = class_id; }
	uint32		Get_Selected_Class_ID (void) const					{ return m_ClassID; }
	void			Enable_Type_Selection (bool onoff)					{ m_EnableTypeSel = onoff; }

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	void			Add_Preset (PresetClass *preset);
	void			Sort_Nodes (HTREEITEM root_item, bool recursive);
	void			Update_Button_State (void);

	void			Populate_Preset_Tree (void);
	void			Fill_Tree (NTreeLeafClass<PresetClass *> *leaf, HTREEITEM parent_item);

	void			Add_Factories_To_Combo (NTreeLeafClass<uint32> *leaf, int indent, int &index);
	void			Generate_Type_List (void);


	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DynamicVectorClass<int> *		m_List;
	uint32								m_ClassID;
	uint32								m_RootClassID;
	bool									m_EnableTypeSel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETLISTDIALOG_H__CFD5DA22_01FA_11D4_A091_00104B791122__INCLUDED_)
