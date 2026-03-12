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

#if !defined(AFX_MAINDIALOGBAR_H__89F1AFD5_6F98_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_MAINDIALOGBAR_H__89F1AFD5_6F98_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "editorbuild.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class DockableFormClass;
class PresetsFormClass;
class InstancesPageClass;
class OverlapPageClass;
class ConversationPageClass;
class HeightfieldPageClass;


/////////////////////////////////////////////////////////////////////////////
//
// MainDialogBarClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class MainDialogBarClass : public CDialogBar
{
// Construction
public:
	MainDialogBarClass (void);
	~MainDialogBarClass (void);

// Dialog Data
	//{{AFX_DATA(MainDialogBarClass)
	enum { IDD = IDD_NEW_LIBRARY };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MainDialogBarClass)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MainDialogBarClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeLibraryTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	enum
	{
		TAB_GLOBAL = 0,
		TAB_INSTANCES,
		TAB_CONVERSATION,
		TAB_OVERLAP,

//#ifndef PUBLIC_EDITOR_VER (gth) go ahead and give the heightfield away!
		TAB_HEIGHTFIELD,
//#endif

		TAB_COUNT
	} TAB_INFO;

public:

	///////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////
	BOOL						Create (CWnd *parent_wnd, UINT style, UINT id);
	void						Initialize (void);
	PresetsFormClass *	Get_Presets_Form (void)		{ return (PresetsFormClass *)m_pTabs[TAB_GLOBAL]; }
	InstancesPageClass *	Get_Instances_Form (void)	{ return (InstancesPageClass *)m_pTabs[TAB_INSTANCES]; }
	OverlapPageClass *	Get_Overlap_Form (void)		{ return (OverlapPageClass *)m_pTabs[TAB_OVERLAP]; }

private:

	///////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////
	CDialog *m_pTabs[TAB_COUNT];
	int m_iCurrentTab;
};

inline BOOL
MainDialogBarClass::Create (CWnd *parent_wnd, UINT style, UINT id)
{
	BOOL retval = CDialogBar::Create (parent_wnd, MainDialogBarClass::IDD, style, id);
	Initialize ();
	return retval;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDIALOGBAR_H__89F1AFD5_6F98_11D3_A05A_00104B791122__INCLUDED_)
