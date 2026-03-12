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

#if !defined(AFX_CONVERSATIONPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_)
#define AFX_CONVERSATIONPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_

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
class ConversationClass;


/////////////////////////////////////////////////////////////////////////////
//
// ConversationPageClass
//
/////////////////////////////////////////////////////////////////////////////
class ConversationPageClass : public CDialog
{
public:
	ConversationPageClass (CWnd *parent_wnd);
	virtual ~ConversationPageClass (void);

// Dialog Data
	//{{AFX_DATA(ConversationPageClass)
	enum { IDD = IDD_CONVERSATION_FORM };
	CTreeCtrl	m_TreeCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ConversationPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ConversationPageClass)
	afx_msg void OnDeleteItemConversationTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkConversationTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandedConversationTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnSwap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////
	void					Reload_Data (void);
	void					Reset_Tree (void);

	static ConversationPageClass *	Get_Instance (void)	{ return _TheInstance; }

protected:

	///////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////
	void					Insert_Entry (ConversationClass *conversation, bool sort_items = true);
	void					Edit_Entry (HTREEITEM tree_item);

	void					Reload_Tree (HTREEITEM *item_handle, ConversationClass **conversation_ptr);

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	DialogToolbarClass	m_Toolbar;
	HTREEITEM				m_GlobalsRoot;
	HTREEITEM				m_LevelsRoot;

	static ConversationPageClass *	_TheInstance;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERSATIONPAGE_H__69EA03B9_0F32_11D4_A098_00104B791122__INCLUDED_)
