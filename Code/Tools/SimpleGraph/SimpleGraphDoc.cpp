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

// SimpleGraphDoc.cpp : implementation of the CSimpleGraphDoc class
//

#include "stdafx.h"
#include "SimpleGraph.h"

#include "SimpleGraphDoc.h"
#include "SimpleGraphView.h"
#include "lookuptable.h"
#include "rawfile.h"
#include "chunkio.h"
#include "catmullromspline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphDoc

IMPLEMENT_DYNCREATE(CSimpleGraphDoc, CDocument)

BEGIN_MESSAGE_MAP(CSimpleGraphDoc, CDocument)
	//{{AFX_MSG_MAP(CSimpleGraphDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphDoc construction/destruction

CSimpleGraphDoc::CSimpleGraphDoc() :
	m_Spline(NULL)
{
	// TODO: add one-time construction code here
	m_Spline = new CatmullRomSpline1DClass;
}

CSimpleGraphDoc::~CSimpleGraphDoc()
{
	delete m_Spline;
}

BOOL CSimpleGraphDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return false;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	int count = m_Spline->Key_Count ();
	while (count --) {
		m_Spline->Remove_Key (0);
	}

	POSITION pos = GetFirstViewPosition ();
	CSimpleGraphView *view = (CSimpleGraphView *)GetNextView (pos);
	view->Set_Ranges (Vector2 (0, 0), Vector2 (100, 100));
	view->InvalidateRect (NULL, true);
	view->UpdateWindow ();	

	return true;
}



/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphDoc serialization

void CSimpleGraphDoc::Serialize(CArchive& ar)
{
	POSITION pos = GetFirstViewPosition ();
	CSimpleGraphView *view = (CSimpleGraphView *)GetNextView (pos);

	if (ar.IsStoring())
	{
		RawFileClass file;
		file.Attach ((void *)ar.GetFile ()->m_hFile);
		ChunkSaveClass csave (&file);

		Vector2 range_min,range_max;		
		view->Get_Ranges(range_min,range_max);
		_TheLookupTableManager.Save_Table_Desc(csave, m_Spline, range_min, range_max);

		file.Detach ();
	}
	else
	{
		RawFileClass file;
		file.Attach ((void *)ar.GetFile ()->m_hFile);
		ChunkLoadClass cload (&file);

		// Load the curve from the file
		Vector2 range_min,range_max;
		Curve1DClass * curve = NULL;
		_TheLookupTableManager.Load_Table_Desc (cload, &curve, &range_min, &range_max);
		view->Set_Ranges(range_min,range_max);

		// If we successfully loaded a new curve, delete the old one
		// and install this one.
		WWASSERT(m_Spline != NULL);
		delete m_Spline;
		m_Spline = curve;

		file.Detach ();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphDoc diagnostics

#ifdef _DEBUG
void CSimpleGraphDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSimpleGraphDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphDoc commands
