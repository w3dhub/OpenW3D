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

#if !defined(AFX_PRESETSFORM_H__89F1AFD8_6F98_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_PRESETSFORM_H__89F1AFD8_6F98_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PresetsForm.h : header file
//


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "DialogToolbar.h"
#include "bittype.h"
#include "vector.h"


// Forward declarations
class DefinitionFactoryClass;
class DefinitionClass;
class PresetClass;


/////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//
/////////////////////////////////////////////////////////////////////////////
typedef struct _DERIVED_PARAM_INFO
{
	PresetClass *				preset;
	DynamicVectorClass<int>	inherited_param_list;

	// For DyanmicVectorClass
	bool operator== (const _DERIVED_PARAM_INFO &/* src*/) { return false; }
	bool operator!= (const _DERIVED_PARAM_INFO &/* src*/) { return true; }
} DERIVED_PARAM_INFO;

typedef DynamicVectorClass<DERIVED_PARAM_INFO> DERIVED_PARAM_LIST;


/////////////////////////////////////////////////////////////////////////////
//
// PresetsFormClass form view
//
/////////////////////////////////////////////////////////////////////////////
class PresetsFormClass : public CDialog
{
public:
	PresetsFormClass (CWnd *parent_wnd);

// Form Data
public:
	//{{AFX_DATA(PresetsFormClass)
	enum { IDD = IDD_PRESETS_FORM };
	CTreeCtrl	m_TreeCtrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PresetsFormClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~PresetsFormClass();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(PresetsFormClass)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDeleteitemPresetsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnModify();
	afx_msg void OnMake();
	afx_msg void OnAddTemp();
	afx_msg void OnDelete();
	afx_msg void OnSelchangedPresetsTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////
	void								Reload_Presets (void);	
	bool								Is_Modified (void) const	{ return m_IsModified; }

	bool								Save_Presets (void);
	bool								Save_Temp_Presets (void);
	static bool						Load_Presets (void);
	static bool						Load_Temp_Presets (void);
	
protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void								Fill_In_Presets (HTREEITEM root_item);
	void								Fill_In_Preset_Children (HTREEITEM root_item, uint32 parent_id);

	HTREEITEM						Find_Preset (HTREEITEM root_item, uint32 id);
	HTREEITEM						Find_Factory (HTREEITEM root_item, uint32 id);

	void								Set_Item_Data (HTREEITEM item, DefinitionFactoryClass *factory);
	void								Set_Item_Data (HTREEITEM item, PresetClass *preset);
	DefinitionFactoryClass *	Get_Item_Factory (HTREEITEM item);
	DefinitionClass *				Get_Item_Definition (HTREEITEM item);
	PresetClass *					Get_Item_Preset (HTREEITEM item);
	DefinitionFactoryClass *	Get_Selected_Factory (void);

	HTREEITEM						Insert_Preset (HTREEITEM root_item, PresetClass *preset);
	int								Get_Icon (PresetClass *preset);
	void								Reselect_Item (int id, bool is_factory = false);
	void								Sort_Nodes (HTREEITEM root_item, bool recursive = true);

	bool								Add_New_Preset (bool is_temp = false);
	bool								Modify_Preset (void);
	PresetClass *					Create_Preset (DefinitionFactoryClass *factory, bool is_temp);

	void								Build_Child_List (HTREEITEM parent_item, DynamicVectorClass<HTREEITEM> &list);	

	void								Build_Inherited_Param_List (HTREEITEM parent_item, DERIVED_PARAM_LIST &list);
	void								Propagate_Changes (HTREEITEM modified_item, DERIVED_PARAM_LIST &list);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	DialogToolbarClass	m_Toolbar;
	static bool				m_IsModified;

	/////////////////////////////////////////////////////////
	//	Friends
	/////////////////////////////////////////////////////////
	friend class SelectionCaptureClass;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETSFORM_H__89F1AFD8_6F98_11D3_A05A_00104B791122__INCLUDED_)
