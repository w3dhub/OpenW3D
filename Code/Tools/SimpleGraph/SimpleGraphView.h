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

// SimpleGraphView.h : interface of the CSimpleGraphView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEGRAPHVIEW_H__C44E4EBD_F133_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_SIMPLEGRAPHVIEW_H__C44E4EBD_F133_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "cardinalspline.h"
#include "vector2.h"

class CSimpleGraphDoc;

class CSimpleGraphView : public CView
{
protected: // create from serialization only
	CSimpleGraphView();
	DECLARE_DYNCREATE(CSimpleGraphView)

// Attributes
public:
	CSimpleGraphDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleGraphView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSimpleGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSimpleGraphView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDelete();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnZoomExtents();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////
	void							Render_Graph (CDC *dc);
	void							Get_Visible_Points (int *left_pt, int *right_pt);

	typedef enum
	{
		HIT_GRAPH_AREA		= 1,
		HIT_POINT,
		HIT_AXIS,
		HIT_UNKNOWN

	} HITTYPE;

	HITTYPE						Hit_Test (const CPoint &point, int *hit_pt);

	void							Get_Graph_Rect (CRect &rect);
	void							Value_To_Point (const Vector2 &value, CPoint *point);
	void							Point_To_Value (const CPoint &point, Vector2 *value);
	int							Move_Value (int old_index, const Vector2 &new_value);
	void							Render_Axis (CDC *dc);
	void							Repaint_Graph (void);
	void							Render_Points (CDC *dc);
	int							Add_New_Point (CPoint &point);
	void							Delete_Point (int index);
	int							Get_Selection (void) const { return m_SelPt; }

	void							Get_Ranges (Vector2 &range_min, Vector2 &range_max)	{ range_min = m_Min; range_max = m_Max; }
	void							Set_Ranges (Vector2 &range_min, Vector2 &range_max)	{ m_Min = range_min; m_Max = range_max; Repaint_Graph (); }

protected:

	//////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////

	Vector2						m_Min;
	Vector2						m_Max;

	int							m_DraggingPt;
	int							m_SelPt;

	CBrush						m_Brush;
	CBrush						m_SelBrush;
	HFONT							m_Font;

	CPoint						m_ZoomPt;
	bool							m_IsZooming;
};

#ifndef _DEBUG  // debug version in SimpleGraphView.cpp
inline CSimpleGraphDoc* CSimpleGraphView::GetDocument()
   { return (CSimpleGraphDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEGRAPHVIEW_H__C44E4EBD_F133_11D3_A08F_00104B791122__INCLUDED_)
