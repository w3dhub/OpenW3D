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

#if !defined(AFX_FILEPICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_)
#define AFX_FILEPICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "picker.h"

/////////////////////////////////////////////////////////////////////////////
//
// FilePickerClass
//
/////////////////////////////////////////////////////////////////////////////
class FilePickerClass : public PickerClass
{
public:

	//////////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	//////////////////////////////////////////////////////////////////////////////////
	FilePickerClass (void);
	virtual ~FilePickerClass (void);

	//////////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////////

	//
	//	From PickerClass
	//
	void		On_Pick (void);

	//
	//	FilePickerClass specific
	//
	void		Set_Extension_String (LPCTSTR extension)	{ m_ExtensionString = extension; }
	void		Set_Filter_String (LPCTSTR filter)			{ m_FilterString = filter; }
	void		Set_Default_Filename (LPCTSTR filename)	{ m_DefaultFilename = filename; }
	void		Set_Asset_Tree_Only (bool onoff)				{ m_AssetTreeOnly = onoff; }

private:

	//////////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////////
	CString	m_ExtensionString;
	CString	m_FilterString;
	CString	m_DefaultFilename;
	bool		m_AssetTreeOnly;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEPICKERCLASS_H__1A8EC040_8874_11D2_9FE1_00104B791122__INCLUDED_)
