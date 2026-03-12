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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/heightfieldpage.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/27/02 12:21p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HEIGHTFIELDPAGE_H
#define __HEIGHTFIELDPAGE_H


#include "nodetypes.h"
#include "resource.h"
#include "bittype.h"
#include "vector.h"


//////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////
class NodeClass;
class DefinitionFactoryClass;


//////////////////////////////////////////////////////////
//
//	HeightfieldPageClass
//
//////////////////////////////////////////////////////////
class HeightfieldPageClass : public CDialog
{
public:
	HeightfieldPageClass (CWnd *parent_wnd);
	virtual ~HeightfieldPageClass (void);

// Form Data
public:
	//{{AFX_DATA(HeightfieldPageClass)
	enum { IDD = IDD_HEIGHTFIELD_FORM };
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HeightfieldPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(HeightfieldPageClass)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnCreateNewButton();
	afx_msg void OnMaterialSettingsButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////

	//
	//	Static methods
	//
	static HeightfieldPageClass *		Get_Instance (void)	{ return _TheInstance; }

	//
	//	Material UI configuration
	//
	void			Update_Material_Button (int index);

protected:

	///////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	static HeightfieldPageClass *	_TheInstance;
	bool									IsInitialized;
};


#endif //__HEIGHTFIELDPAGE_H
