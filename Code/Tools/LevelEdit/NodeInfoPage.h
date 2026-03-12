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

#if !defined(AFX_OBJECTINFOPAGECLASS_H__E2623E76_740A_11D2_9FD0_00104B791122__INCLUDED_)
#define AFX_OBJECTINFOPAGECLASS_H__E2623E76_740A_11D2_9FD0_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NodeInfoPageClass.h : header file
//


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.H"
#include "DockableForm.H"

class NodeClass;


//////////////////////////////////////////////////////////
//
//	NodeInfoPageClass
//
//////////////////////////////////////////////////////////
class NodeInfoPageClass : public DockableFormClass
{
	public:
		NodeInfoPageClass (void);
		NodeInfoPageClass (NodeClass *node);
		virtual ~NodeInfoPageClass (void);


// Form Data
public:
	//{{AFX_DATA(NodeInfoPageClass)
	enum { IDD = IDD_OBJECT_INFO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NodeInfoPageClass)
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
	//{{AFX_MSG(NodeInfoPageClass)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////////
		void				HandleInitDialog (void);
		bool				Apply_Changes (void);

		///////////////////////////////////////////////////////
		//	Inline accessors
		///////////////////////////////////////////////////////
		void				Set_Node (NodeClass *node) { m_pNode = node; }
		NodeClass *		Get_Node (void)				{ return m_pNode; }


	private:

		///////////////////////////////////////////////////////
		//	Private member data
		///////////////////////////////////////////////////////
		NodeClass *	m_pNode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NodeInfoPageCLASS_H__E2623E76_740A_11D2_9FD0_00104B791122__INCLUDED_)
