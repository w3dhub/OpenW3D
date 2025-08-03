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

// ParameterSheet.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "ParameterSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ParameterSheetClass

IMPLEMENT_DYNCREATE(ParameterSheetClass, CScrollView)

ParameterSheetClass::ParameterSheetClass()
{
}

ParameterSheetClass::~ParameterSheetClass()
{
}


BEGIN_MESSAGE_MAP(ParameterSheetClass, CScrollView)
	//{{AFX_MSG_MAP(ParameterSheetClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ParameterSheetClass drawing

void ParameterSheetClass::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void ParameterSheetClass::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ParameterSheetClass diagnostics

#ifdef _DEBUG
void ParameterSheetClass::AssertValid() const
{
	CScrollView::AssertValid();
}

void ParameterSheetClass::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// ParameterSheetClass message handlers
