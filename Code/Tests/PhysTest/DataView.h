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

#if !defined(AFX_DATAVIEW_H__A9B6C2E1_E61E_11D2_802E_0040056350C8__INCLUDED_)
#define AFX_DATAVIEW_H__A9B6C2E1_E61E_11D2_802E_0040056350C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataView.h : header file
//

#include "AfxCView.H"

class PhysClass;
class ChunkSaveClass;
class ChunkLoadClass;

/////////////////////////////////////////////////////////////////////////////
//
// ItemInfoClass - stores info about all of the items in the CDataView
//
class ItemInfoClass
{
public:
	enum { MODEL = 0, INSTANCE };

	ItemInfoClass(const char * name,int type) : Instance(nullptr) { Name = strdup(name); Type = type; }
	~ItemInfoClass(void) { if (Name) free(Name); }

	char *		Name;
	int			Type;
	PhysClass *	Instance;
};



/////////////////////////////////////////////////////////////////////////////
//
// CDataView view
//
class CDataView : public CTreeView
{
protected:
	CDataView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDataView)

// Attributes
public:

// Operations
public:

	void								Rebuild_Tree(void);
	ItemInfoClass *				Get_Selected_Item(void);
	void								Save(ChunkSaveClass & csave);
	void								Load(ChunkLoadClass & cload);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDataView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	///////////////////////////////////////////////////////
	//
	//	Private member data
	//
	HTREEITEM	ModelsRoot;
	HTREEITEM	InstancesRoot;

	// Generated message map functions
protected:
	//{{AFX_MSG(CDataView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATAVIEW_H__A9B6C2E1_E61E_11D2_802E_0040056350C8__INCLUDED_)
