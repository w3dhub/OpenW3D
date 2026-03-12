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

#if !defined(AFX_POSITIONPAGE_H__BBE6B202_B480_11D2_9FF7_00104B791122__INCLUDED_)
#define AFX_POSITIONPAGE_H__BBE6B202_B480_11D2_9FF7_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class NodeClass;
#include "DockableForm.H"


//////////////////////////////////////////////////////////
//
//	PositionPageClass
//
//////////////////////////////////////////////////////////
class PositionPageClass : public DockableFormClass
{
	public:
		PositionPageClass (void);
		PositionPageClass (NodeClass *node);
		virtual ~PositionPageClass (void);

// Form Data
public:
	//{{AFX_DATA(PositionPageClass)
	enum { IDD = IDD_NODE_POSITION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PositionPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(PositionPageClass)
	afx_msg void OnDeltaPosXPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosXRotSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosYPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosYRotSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosZPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosZRotSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeZRotEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////
	void						HandleInitDialog (void);
	bool						Apply_Changes (void);

protected:

	///////////////////////////////////////////////////////
	//	Inline accessors
	///////////////////////////////////////////////////////

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	NodeClass *		m_pNode;
	bool				m_bInclueRotation;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSITIONPAGE_H__BBE6B202_B480_11D2_9FF7_00104B791122__INCLUDED_)
