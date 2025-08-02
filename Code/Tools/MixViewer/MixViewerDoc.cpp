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

// MixViewerDoc.cpp : implementation of the CMixViewerDoc class
//

#include "StdAfx.h"
#include "mixviewer.h"
#include "mixviewerdoc.h"
#include "mixviewerview.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixViewerDoc

IMPLEMENT_DYNCREATE(CMixViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMixViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CMixViewerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixViewerDoc construction/destruction

CMixViewerDoc::CMixViewerDoc()
{
	// TODO: add one-time construction code here

}

CMixViewerDoc::~CMixViewerDoc()
{
}




/////////////////////////////////////////////////////////////////////////////
// CMixViewerDoc serialization

void CMixViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
// CMixViewerDoc diagnostics

#ifdef _DEBUG
void CMixViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMixViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
//
// OnNewDocument
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CMixViewerDoc::OnNewDocument (void)
{
	if (!CDocument::OnNewDocument ()) {
		return FALSE;
	}

	//
	//	Update each view
	//
	POSITION pos = GetFirstViewPosition ();
   while (pos != NULL)
   {
      CMixViewerView *view = (CMixViewerView *)GetNextView (pos);
		view->Reset ();
   }

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOpenDocument
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CMixViewerDoc::OnOpenDocument (LPCTSTR path)
{
	if (!CDocument::OnOpenDocument (path)) {
		return FALSE;
	}

	//
	//	Start fresh
	//
	OnNewDocument ();

	//
	//	Update each view
	//
	POSITION pos = GetFirstViewPosition ();
	while (pos != NULL)
	{
		CMixViewerView *view = (CMixViewerView *)GetNextView (pos);
		view->Reload (path);
	}
		
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reload_Views
//
/////////////////////////////////////////////////////////////////////////////
void
CMixViewerDoc::Reload_Views (void)
{
	//
	//	Update each view
	//
	POSITION pos = GetFirstViewPosition ();
	while (pos != NULL)
	{
		CMixViewerView *view = (CMixViewerView *)GetNextView (pos);
		view->Reload (GetPathName ());
	}
	
	return ;
}
