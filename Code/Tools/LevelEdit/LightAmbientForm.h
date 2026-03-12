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

#if !defined(AFX_LIGHTAMBIENTFORMCLASS_H__11C18BF8_759F_11D2_9FD0_00104B791122__INCLUDED_)
#define AFX_LIGHTAMBIENTFORMCLASS_H__11C18BF8_759F_11D2_9FD0_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightAmbientFormClass.h : header file
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


#include "DockableForm.H"
#include "resource.h"

/////////////////////////////////////////////////////////////////
//
//	LightAmbientFormClass
//
class LightAmbientFormClass : public DockableFormClass
{
	public:
		LightAmbientFormClass (void);
		virtual ~LightAmbientFormClass (void);

// Form Data
public:
	//{{AFX_DATA(LightAmbientFormClass)
	enum { IDD = IDD_LIGHT_AMBIENT_DIALOG };
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightAmbientFormClass)
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
	//{{AFX_MSG(LightAmbientFormClass)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	public:

		///////////////////////////////////////////////////////
		//
		//	Public methods
		//
		void			HandleInitDialog (void);
		void			Update_Settings (void);

	private:

		///////////////////////////////////////////////////////
		//
		//	Private member data
		//
		int	m_InitialRed;
		int	m_InitialGreen;
		int	m_InitialBlue;
		HWND	m_ColorForm;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTAMBIENTFORMCLASS_H__11C18BF8_759F_11D2_9FD0_00104B791122__INCLUDED_)
