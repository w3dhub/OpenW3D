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

#include "dialogtoolbar.h"
#include "bittype.h"
#include "vector.h"
#include "wwstring.h"

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class DefinitionFactoryClass;
class DefinitionClass;
class PresetClass;
class ParameterClass;


/////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////
typedef struct _DERIVED_PARAM_INFO
{
	PresetClass *									preset;
	DynamicVectorClass<ParameterClass *>	base_param_list;
	DynamicVectorClass<ParameterClass *>	derived_param_list;

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
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
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
	afx_msg void OnPlay();
	afx_msg void OnInfo();
	afx_msg void OnExtra();
	afx_msg void OnUpdateConvert(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInfo(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUpdateVss(CCmdUI* pCmdUI);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnConvert();
	afx_msg void OnUpdateVss();
	afx_msg void OnBatchImportTerrain();
	afx_msg void OnUpdateBatchImportTerrain(CCmdUI* pCmdUI);
	afx_msg void OnBegindragPresetsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBuildEmbedNodeList();
	afx_msg void OnUpdateEmbeddedNodeList(CCmdUI* pCmdUI);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnClearEmbeddedNodeList();
	afx_msg void OnUpdateClearEmbeddedNodeList(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////
	void								Reload_Presets (void);	

	bool								Save_Temp_Presets (void);
	bool								Save_Global_Presets (uint32 class_id);
	
	bool								Save_Presets (LPCTSTR path, uint32 class_id, bool temps_only = false, bool class_id_matters = true);
	static bool						Old_Load_Presets (void);
	static bool						Load_Temp_Presets (void);
	static bool						Load_Databases (void);
	void								Build_DDB_File_List (DynamicVectorClass<CString> &file_list, HTREEITEM parent_item = TVI_ROOT);

	//
	//	File dependency support
	//
	void								Export_File_Dependencies (const char *filename);
	void								Build_File_Dependencies_For_Definition (DynamicVectorClass<StringClass> &file_list, DefinitionClass *definition);

	//
	//	Library support
	//
	void								Save_Preset_Libraries (HTREEITEM parent_item = TVI_ROOT);
	static bool						Load_Preset_Libraries (void);

	//
	//	Preset addition support
	//
	void								Add_Preset (PresetClass *preset);
	void								Sort_Items (uint32 class_id);
	
protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void								Fill_In_Presets (HTREEITEM root_item);
	void								Fill_In_Preset_Children (HTREEITEM root_item, uint32 parent_id);

	bool								Save_Presets (uint32 class_id, bool temps_only = false);
	void								Save_Presets (HANDLE file, uint32 class_id, bool temps_only = false, bool class_id_matters = true);
	static bool						Load_Presets (uint32 class_id);	

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

	PresetClass *					Add_New_Preset (LPCTSTR name, bool is_temp = false, bool sel_new = true);
	bool								Modify_Preset (void);
	PresetClass *					Create_Preset (DefinitionFactoryClass *factory, bool is_temp);
	void								Add_Terrain_Preset (LPCTSTR filename);

	void								Build_Child_List (HTREEITEM parent_item, DynamicVectorClass<HTREEITEM> &list);

	void								Build_Inherited_Param_List (HTREEITEM parent_item, DERIVED_PARAM_LIST &list);
	void								Propagate_Changes (HTREEITEM modified_item, DERIVED_PARAM_LIST &list);
	void								Compare_Derived_Parameters (PresetClass *curr_preset, DefinitionClass *base_def, DefinitionClass *derived_def, DERIVED_PARAM_LIST &list);

	bool								Enable_Button (int cmd_id);
	void								Get_Menu_Text (int menu_id, CString &text);

	//
	//	Embedded node stuff
	//
	void								Update_Embedded_Nodes (PresetClass *preset);
	void								Reset_Embedded_Node_Instances (PresetClass *preset);

	//
	//	Drag/drop stuff
	//
	bool								Is_Drag_OK (HTREEITEM drag_item);
	bool								Is_Drop_OK (HTREEITEM drop_item);
	void								Do_Drop (HTREEITEM drag_item, HTREEITEM drop_item);

	//
	//	Copy/Move stuff
	//
	void								Copy_Preset_Items (HTREEITEM preset_item, HTREEITEM parent_item);
	void								Move_Preset (HTREEITEM preset_item, HTREEITEM parent_item);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	DialogToolbarClass	m_Toolbar;

	bool						m_IsDragging;
	HTREEITEM				m_DragItem;
	CImageList *			m_DragImageList;

	/////////////////////////////////////////////////////////
	//	Friends
	/////////////////////////////////////////////////////////
	friend class SelectionCaptureClass;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESETSFORM_H__89F1AFD8_6F98_11D3_A05A_00104B791122__INCLUDED_)
