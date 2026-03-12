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

#if !defined(AFX_OVERLAPPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_)
#define AFX_OVERLAPPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "dialogtoolbar.h"
#include "nodetypes.h"

/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class NodeClass;


/////////////////////////////////////////////////////////////////////////////
//
// OverlapPageClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class OverlapPageClass : public CDialog
{
public:
	OverlapPageClass (CWnd *parent_wnd);
	virtual ~OverlapPageClass (void);

// Dialog Data
	//{{AFX_DATA(OverlapPageClass)
	enum { IDD = IDD_OVERLAP_FORM };
	CTreeCtrl	m_TreeCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OverlapPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OverlapPageClass)
	afx_msg void OnDeleteitemOverlapTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnRefresh();
	afx_msg void OnDblclkOverlapTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandedOverlapTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////
	void					Build_Tree (void);
	void					Reset_Tree (void);

protected:

	///////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////
	void					Detect_Overlaps (HTREEITEM root_item, NODE_TYPE node_type);
	HTREEITEM			Insert_Node (HTREEITEM parent_item, NodeClass *node);

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	DialogToolbarClass	m_Toolbar;
	HTREEITEM				m_SoundsRoot;
	HTREEITEM				m_LightsRoot;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OVERLAPPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_)
