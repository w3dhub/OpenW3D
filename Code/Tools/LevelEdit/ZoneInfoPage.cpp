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

// ZoneInfoPage.cpp : implementation file
//

#include "StdAfx.h"
#include "leveledit.h"
#include "ZoneInfoPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ZoneInfoPageClass
//
ZoneInfoPageClass::ZoneInfoPageClass ()
	: m_pZone (NULL),
	  DockableFormClass(ZoneInfoPageClass::IDD)
{
	//{{AFX_DATA_INIT(ZoneInfoPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ZoneInfoPageClass
//
ZoneInfoPageClass::ZoneInfoPageClass (ZoneInstanceClass *pzone)
	: m_pZone (pzone),
	DockableFormClass(ZoneInfoPageClass::IDD)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~ZoneInfoPageClass
//
ZoneInfoPageClass::~ZoneInfoPageClass ()
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
ZoneInfoPageClass::DoDataExchange (CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ZoneInfoPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ZoneInfoPageClass, DockableFormClass)
	//{{AFX_MSG_MAP(ZoneInfoPageClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ZoneInfoPageClass diagnostics

#ifdef _DEBUG
void ZoneInfoPageClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void ZoneInfoPageClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
void
ZoneInfoPageClass::HandleInitDialog (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
bool
ZoneInfoPageClass::Apply_Changes (void)
{
	// Return true to allow the dialog to close
	return true;
}
