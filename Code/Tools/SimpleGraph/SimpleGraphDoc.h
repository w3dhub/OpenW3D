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

// SimpleGraphDoc.h : interface of the CSimpleGraphDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEGRAPHDOC_H__C44E4EBB_F133_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_SIMPLEGRAPHDOC_H__C44E4EBB_F133_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cardinalspline.h"

class CSimpleGraphDoc : public CDocument
{
protected: // create from serialization only
	CSimpleGraphDoc();
	DECLARE_DYNCREATE(CSimpleGraphDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleGraphDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSimpleGraphDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSimpleGraphDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////
	Curve1DClass &				Get_Spline (void) { return *m_Spline; }

protected:

	//////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////

	Curve1DClass *				m_Spline;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEGRAPHDOC_H__C44E4EBB_F133_11D3_A08F_00104B791122__INCLUDED_)
