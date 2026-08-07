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

#if !defined(AFX_CONVERSATIONPICKERDIALOG_H__45E7AEA7_6692_43BF_B169_555904929404__INCLUDED_)
#define AFX_CONVERSATIONPICKERDIALOG_H__45E7AEA7_6692_43BF_B169_555904929404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class ConversationClass;


/////////////////////////////////////////////////////////////////////////////
//
// ConversationPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class ConversationPickerDialogClass : public CDialog
{
// Construction
public:
	ConversationPickerDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ConversationPickerDialogClass)
	enum { IDD = IDD_CONVERSATION_PICKER };
	CTreeCtrl	m_TreeCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ConversationPickerDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ConversationPickerDialogClass)
	afx_msg void OnSelchangedConversationTree(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	void						Set_Conversation (ConversationClass *conversation)	{ Conversation = conversation; }
	ConversationClass	*	Get_Conversation (void)										{ return Conversation; }

private:

	//////////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////////
	void						Insert_Entry (ConversationClass *conversation);

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	ConversationClass *	Conversation;
	HTREEITEM				GlobalsRoot;
	HTREEITEM				LevelsRoot;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERSATIONPICKERDIALOG_H__45E7AEA7_6692_43BF_B169_555904929404__INCLUDED_)
