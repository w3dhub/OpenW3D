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

#if !defined(AFX_GENERATINGPATHFINDDIALOG_H__4F3E7431_C94F_11D3_A085_00104B791122__INCLUDED_)
#define AFX_GENERATINGPATHFINDDIALOG_H__4F3E7431_C94F_11D3_A085_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//
// GeneratingPathfindDialogClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class GeneratingPathfindDialogClass : public CDialog
{
// Construction
public:
	GeneratingPathfindDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GeneratingPathfindDialogClass)
	enum { IDD = IDD_GENERATING_SECTORS };
	CProgressCtrl	m_ProgressBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GeneratingPathfindDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GeneratingPathfindDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////
	//	Public data types
	/////////////////////////////////////////////////////////////
	typedef enum
	{
		STATE_FLOODFILL	= 1,
		STATE_COMPRESS
	} STATE;

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	void			Set_State (STATE state);
	void			Set_Status (LPCTSTR text, float percent);

	bool			Was_Cancelled (void) const	{ return m_Cancelled; }

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	bool			m_Cancelled;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATINGPATHFINDDIALOG_H__4F3E7431_C94F_11D3_A085_00104B791122__INCLUDED_)
