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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LightMap                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tool $*
 *                                                                                             *
 *                      $Author:: Ian_l               $*
 *                                                                                             *
 *                     $Modtime:: 7/09/01 2:42p       $*
 *                                                                                             *
 *                    $Revision:: 15                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef LIGHTMAPVIEW_H
#define LIGHTMAPVIEW_H

#include <afxcview.h>
#include "LightMapDoc.h"
#include "Lightscape.h"

class StringBuilder;

class LightMapView : public CListView
{
	protected:

	private:

		// Static functions.
		static int Compare_Names (const void *index0, const void *index1);

		// Member data.
		unsigned				 *MeshIndexTable;

		// Static data.
		static LightMapDoc *_Document;

// The following is maintained by MFC tools.
protected: // create from serialization only
	LightMapView();
	DECLARE_DYNCREATE(LightMapView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightMapView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate (CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~LightMapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	LightMapDoc* GetDocument();

// Generated message map functions
protected:
	//{{AFX_MSG(LightMapView)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInsertSolve();
	afx_msg void OnUpdateInsertSolve(CCmdUI* pCmdUI);
	afx_msg void OnToolsOptions();
	afx_msg void OnToolsPacking();
	afx_msg void OnUpdateToolsPacking(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in LightMapView.cpp
inline LightMapDoc* LightMapView::GetDocument()  {return (LightMapDoc*)m_pDocument;}
#endif

#endif // LIGHTMAPVIEW_H
