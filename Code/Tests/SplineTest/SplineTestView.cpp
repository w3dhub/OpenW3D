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

// SplineTestView.cpp : implementation of the CSplineTestView class
//

#include "stdafx.h"
#include "SplineTest.h"

#include "SplineTestDoc.h"
#include "SplineTestView.h"
#include "curve.h"
#include "hermitespline.h"
#include "catmullromspline.h"
#include "cardinalspline.h"
#include "tcbspline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const float MIN_X = -10.0f;
const float MIN_Y = -10.0f;
const float MIN_Z = -10.0f;
const float MAX_X = 10.0f;
const float MAX_Y = 10.0f;
const float MAX_Z = 10.0f;



/////////////////////////////////////////////////////////////////////////////
// CSplineTestView

IMPLEMENT_DYNCREATE(CSplineTestView, CView)

BEGIN_MESSAGE_MAP(CSplineTestView, CView)
	//{{AFX_MSG_MAP(CSplineTestView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplineTestView construction/destruction

CSplineTestView::CSplineTestView()
{
	DrawTangents = false;
}

CSplineTestView::~CSplineTestView()
{
}

BOOL CSplineTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSplineTestView drawing

void CSplineTestView::OnDraw(CDC* pDC)
{
	CSplineTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	Curve3DClass * curve = pDoc->Get_Curve();
	if (curve == NULL) return;

	Vector3 pt;
	int x,y;

	// draw coordinate axes
	CPen blackpen;
	blackpen.CreatePen(PS_DASH,1,RGB(0,0,0));
	pDC->SelectObject(&blackpen);

	Map_Point(Vector3(0,10,0),&x,&y);
	pDC->MoveTo(x,y);
	Map_Point(Vector3(0,-10,0),&x,&y);
	pDC->LineTo(x,y);
	Map_Point(Vector3(10,0,0),&x,&y);
	pDC->MoveTo(x,y);
	Map_Point(Vector3(-10,0,0),&x,&y);
	pDC->LineTo(x,y);

	// draw the spline
	CPen greenpen;
	greenpen.CreatePen(PS_SOLID,2,RGB(0,255,0));
	pDC->SelectObject(&greenpen);

	if (curve->Key_Count() >= 2) {
		float t0 = curve->Get_Start_Time();
		float t1 = curve->Get_End_Time();
		float dt = (t1 - t0) / 500.0f;

		curve->Evaluate(t0,&pt);
		Map_Point(pt,&x,&y);
		pDC->MoveTo(x,y);

		for (float t=t0; t<t1; t+=dt) {
			curve->Evaluate(t,&pt);
			Map_Point(pt,&x,&y);
			pDC->LineTo(x,y);
		}
	}

	// draw the control points
	for (int pi=0; pi<curve->Key_Count(); pi++) {
		curve->Get_Key(pi,&pt,NULL);
		Map_Point(pt,&x,&y);
		pDC->FillSolidRect(x-2,y-2,4,4, RGB(255,0,0));
	}

	// draw the tangents
	if ((DrawTangents) && (pDoc->Get_Curve_Type() > CSplineTestDoc::LINEAR)) {

		HermiteSpline3DClass * spline = (HermiteSpline3DClass *)curve;

		CPen blackpen;
		blackpen.CreatePen(PS_SOLID,1,RGB(0,0,0));
		pDC->SelectObject(&blackpen);

		for (int pi=0; pi<spline->Key_Count(); pi++) {

			int cx,cy;
			spline->Get_Key(pi,&pt,NULL);
			Map_Point(pt,&cx,&cy);

			Vector3 in,out;
			spline->Get_Tangents(pi,&in,&out);

			Map_Point(pt-in,&x,&y);
			pDC->MoveTo(cx,cy);
			pDC->LineTo(x,y);

			Map_Point(pt+out,&x,&y);
			pDC->MoveTo(cx,cy);
			pDC->LineTo(x,y);
		}
	}

}

void CSplineTestView::Map_Point(const Vector3 & point,int * set_x,int * set_y)
{
	RECT rect;
	GetClientRect(&rect);

	float nx,ny;

	nx = (point.X - MIN_X) / (MAX_X - MIN_X);
	ny = (point.Y - MIN_Y) / (MAX_Y - MIN_Y);

	*set_x = (float)(rect.right - rect.left) * nx + rect.left;
	*set_y = (float)(rect.top - rect.bottom) * ny + rect.bottom;
}


void CSplineTestView::Un_Map_Point(int x,int y,Vector3 * set_point)
{
	RECT rect;
	GetClientRect(&rect);

	float nx,ny;
	nx = (float)(x - rect.left)   / (float)(rect.right - rect.left);
	ny = (float)(y - rect.bottom) / (float)(rect.top - rect.bottom);

	set_point->X = MIN_X + (MAX_X - MIN_X) * nx;
	set_point->Y = MIN_Y + (MAX_Y - MIN_Y) * ny;
	set_point->Z = 0.0f;

	*set_point = Clamp_Point(*set_point);
}



/////////////////////////////////////////////////////////////////////////////
// CSplineTestView diagnostics

#ifdef _DEBUG
void CSplineTestView::AssertValid() const
{
	CView::AssertValid();
}

void CSplineTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSplineTestDoc* CSplineTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSplineTestDoc)));
	return (CSplineTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSplineTestView message handlers

void CSplineTestView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSplineTestDoc* pDoc = GetDocument();
	if (pDoc) {
		// map the point
		Vector3 pt;
		Un_Map_Point(point.x,point.y,&pt);

		// try to go into drag mode
		bool gotone = pDoc->Grab_Point(pt);
		if (gotone) {
			::SetCapture(m_hWnd);
		}
	}

	CView::OnLButtonDown(nFlags, point);
}

void CSplineTestView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// were we dragging something?  if so release it
	// otherwise add a point to the curve
	CSplineTestDoc* pDoc = GetDocument();
	if (pDoc != NULL) {
		if (pDoc->Is_Dragging()) {
			pDoc->Release_Point();
			::ReleaseCapture();
		} else {
			Vector3 pt;
			Un_Map_Point(point.x,point.y,&pt);
			pDoc->Add_Point(pt);
		}
	}

	CView::OnLButtonUp(nFlags, point);
}

void CSplineTestView::OnMouseMove(UINT nFlags, CPoint point)
{
	CSplineTestDoc* pDoc = GetDocument();

	// are we dragging?
	// if so tell the doc where the point should go
	if (pDoc && pDoc->Is_Dragging()) {
		Vector3 pt;
		Un_Map_Point(point.x,point.y,&pt);
		pDoc->Drag_Point(pt);
	}

	CView::OnMouseMove(nFlags, point);
}

void CSplineTestView::OnRButtonUp(UINT nFlags, CPoint point)
{
	Vector3 pt;
	Un_Map_Point(point.x,point.y,&pt);
	CSplineTestDoc* pDoc = GetDocument();
	if (pDoc) {
		pDoc->Edit_Point(pt);
	}

	CView::OnRButtonUp(nFlags, point);
}


Vector3 CSplineTestView::Clamp_Point(const Vector3 & input)
{
	Vector3 output = input;
	if (output.X < MIN_X) output.X = MIN_X;
	if (output.Y < MIN_Y) output.Y = MIN_Y;
	if (output.Z < MIN_Z) output.Z = MIN_Z;

	if (output.X > MAX_X) output.X = MAX_X;
	if (output.Y > MAX_Y) output.Y = MAX_Y;
	if (output.Z > MAX_Z) output.Z = MAX_Z;

	return output;
}

void CSplineTestView::Enable_Draw_Tangents(bool onoff)
{
	DrawTangents = onoff;
	CSplineTestDoc* pDoc = GetDocument();
	if (pDoc) {
		pDoc->UpdateAllViews(NULL);
	}
}

bool CSplineTestView::Is_Draw_Tangents_Enabled(void)
{
	return DrawTangents;
}
