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

// SplineTestView.h : interface of the CSplineTestView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLINETESTVIEW_H__2AE09FDD_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
#define AFX_SPLINETESTVIEW_H__2AE09FDD_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Vector3;

class CSplineTestView : public CView
{
protected: // create from serialization only
	CSplineTestView();
	DECLARE_DYNCREATE(CSplineTestView)

// Attributes
public:
	CSplineTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplineTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSplineTestView();
	void			Map_Point(const Vector3 & point,int * set_x,int * set_y);
	void			Un_Map_Point(int x,int y,Vector3 * set_point);
	Vector3		Clamp_Point(const Vector3 & input);
	void			Enable_Draw_Tangents(bool onoff);
	bool			Is_Draw_Tangents_Enabled(void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	bool			DrawTangents;

// Generated message map functions
protected:
	//{{AFX_MSG(CSplineTestView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SplineTestView.cpp
inline CSplineTestDoc* CSplineTestView::GetDocument()
   { return (CSplineTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLINETESTVIEW_H__2AE09FDD_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
