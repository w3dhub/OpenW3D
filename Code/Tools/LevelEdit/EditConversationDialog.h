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

#if !defined(AFX_EDITCONVERSATIONDIALOG_H__77BB0B01_5D9E_4BFF_9D25_CE964576950D__INCLUDED_)
#define AFX_EDITCONVERSATIONDIALOG_H__77BB0B01_5D9E_4BFF_9D25_CE964576950D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class ConversationClass;
class ConversationRemarkClass;

/////////////////////////////////////////////////////////////////////////////
//
// EditConversationDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class EditConversationDialogClass : public CDialog
{
// Construction
public:
	EditConversationDialogClass (CWnd *pParent = nullptr);
	~EditConversationDialogClass (void);

// Dialog Data
	//{{AFX_DATA(EditConversationDialogClass)
	enum { IDD = IDD_EDIT_CONVERSATION };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditConversationDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditConversationDialogClass)
	afx_msg void OnDblclkRemarkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownRemarkList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnInsert();
	afx_msg void OnItemchangedRemarkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemRemarkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////////////
	void						Set_Conversation (ConversationClass *conversation);
	ConversationClass *	Peek_Conversation (void) { return m_Conversation; }

protected:

	/////////////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////////////
	ConversationRemarkClass *Get_Entry_Data (int index);
	void						Add_Entry (const ConversationRemarkClass &remark, int insert_index = 0xFFFF);
	void						Update_Button_States (void);
	void						Update_Enable_State (int orator_index);
	void						Update_Player_Type_Combos (int orator_index);
	void						Update_Remarks (int orator_index);
	int						Get_Orator_Type (int orator_index);

	int						Find_Combobox_Entry (int orator_type);

	/////////////////////////////////////////////////////////////////////////////
	//	Protected member data
	/////////////////////////////////////////////////////////////////////////////
	ConversationClass *	m_Conversation;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCONVERSATIONDIALOG_H__77BB0B01_5D9E_4BFF_9D25_CE964576950D__INCLUDED_)
