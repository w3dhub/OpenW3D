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

// SimpleGraphView.cpp : implementation of the CSimpleGraphView class
//

#include "stdafx.h"
#include "SimpleGraph.h"

#include "SimpleGraphDoc.h"
#include "SimpleGraphView.h"
#include "vector3.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphView

IMPLEMENT_DYNCREATE(CSimpleGraphView, CView)

BEGIN_MESSAGE_MAP(CSimpleGraphView, CView)
	//{{AFX_MSG_MAP(CSimpleGraphView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(IDM_DELETE, OnDelete)
	ON_UPDATE_COMMAND_UI(IDM_DELETE, OnUpdateDelete)
	ON_COMMAND(IDM_ZOOM_EXTENTS, OnZoomExtents)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphView construction/destruction

CSimpleGraphView::CSimpleGraphView()
	:	m_Min (0, 0),
		m_Max (100, 100),
		m_DraggingPt (-1),
		m_SelPt (-1),
		m_IsZooming (false)
{
}

CSimpleGraphView::~CSimpleGraphView()
{
}

BOOL CSimpleGraphView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	m_Brush.CreateSolidBrush (RGB (0, 255, 0));
	m_SelBrush.CreateSolidBrush (RGB (255, 0, 0));
	return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDraw
//
/////////////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnDraw (CDC* dc)
{
	HFONT old_font = (HFONT)::SelectObject (*dc, m_Font);

	Render_Axis (dc);
	Render_Graph (dc);
	Render_Points (dc);

	::SelectObject (*dc, old_font);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphView diagnostics

#ifdef _DEBUG
void CSimpleGraphView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleGraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleGraphDoc* CSimpleGraphView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleGraphDoc)));
	return (CSimpleGraphDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimpleGraphView message handlers


/////////////////////////////////////////////////////////////////////////////
//
// Repaint_Graph
//
/////////////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Repaint_Graph (void)
{
	CRect rect;
	Get_Graph_Rect (rect);

	InvalidateRect (rect, true);
	UpdateWindow ();	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Render_Axis
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Render_Axis (CDC *dc)
{	
	CRect view_rect;
	GetClientRect (&view_rect);

	CRect graph_rect;
	Get_Graph_Rect (graph_rect);

	dc->MoveTo (view_rect.left + 70, view_rect.top + 25);
	dc->LineTo (view_rect.left + 70, view_rect.bottom - 50);

	dc->MoveTo (view_rect.left + 70, view_rect.bottom - 45);
	dc->LineTo (view_rect.right - 25, view_rect.bottom - 45);

	int tick_width		= 35;
	int tick_height	= 35;	

	int ticks_x			= (graph_rect.Width () / tick_width);
	int ticks_y			= (graph_rect.Height () / tick_height);
	
	tick_width			= (graph_rect.Width () / ticks_x);	
	tick_height			= (graph_rect.Height () / ticks_y);

	float x_val			= m_Min.X;
	float y_val			= m_Max.Y;
	float x_inc			= (m_Max.X - m_Min.X) / ticks_x;
	float y_inc			= (m_Min.Y - m_Max.Y) / ticks_y;

	//
	//	Draw the x-axis tick marks
	//
	int location = graph_rect.left;
	for (int tick = 0; tick <= ticks_x; tick ++) {
		
		dc->MoveTo (location, view_rect.bottom - 50);
		dc->LineTo (location, view_rect.bottom - 40);

		CRect label_rect;
		label_rect.left	= location - 15;
		label_rect.right	= location + 15;
		label_rect.top		= view_rect.bottom - 38;
		label_rect.bottom	= view_rect.bottom - 20;

		CString label;
		label.Format ("%.2f", x_val);
		dc->DrawText (label, label_rect, DT_CENTER);

		x_val		+= x_inc;
		location += tick_width;
	}

	//
	//	Draw the y-axis tick marks
	//
	location = graph_rect.top;
	for (tick = 0; tick <= ticks_y; tick ++) {
		
		dc->MoveTo (view_rect.left + 65, location);
		dc->LineTo (view_rect.left + 75, location);

		CRect label_rect;
		label_rect.left	= view_rect.left + 38;
		label_rect.right	= view_rect.left + 63;
		label_rect.top		= location - 15;
		label_rect.bottom	= location + 15;

		CString label;
		label.Format ("%.2f", y_val);
		dc->DrawText (label, label_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		y_val		+= y_inc;
		location += tick_height;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Render_Graph
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Render_Graph (CDC *dc)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	if (spline.Key_Count () < 2) {
		return ;
	}

	CRect graph_area;
	Get_Graph_Rect (graph_area);

	float delta_x	= m_Max.X - m_Min.X;
	float delta_y	= m_Max.Y - m_Min.Y;	
	
	float start_time	= 0;
	float end_time		= 0;
	float foo			= 0;
	spline.Get_Key (0, &foo, &start_time);
	spline.Get_Key (spline.Key_Count () - 1, &foo, &end_time);
	
	start_time	= std::max (m_Min.X, start_time);
	end_time		= min (m_Max.X, end_time);		
	
	float start_value	= 0;
	float end_value	= 0;
	CPoint start_pt;
	CPoint end_pt;
	spline.Evaluate (start_time, &start_value);
	spline.Evaluate (end_time, &end_value);	
	Value_To_Point (Vector2 (start_time, start_value), &start_pt);
	Value_To_Point (Vector2 (end_time, end_value), &end_pt);
	
	int count		= ((end_pt.x - start_pt.x) / 4);
	float time_inc = (end_time - start_time) / (float)count;
	
	//
	//	Draw the curve
	//
	dc->MoveTo (start_pt.x, start_pt.y);
	for (float time = start_time; time <= end_time; time += time_inc) {

		float value;
		spline.Evaluate (time, &value);

		float percent_x = (time - m_Min.X) / delta_x;
		float percent_y = (value - m_Min.Y) / delta_y;

		int client_x = graph_area.left + int(graph_area.Width () * percent_x);
		int client_y = graph_area.top + int(graph_area.Height () * (1-percent_y));
		
		if (percent_x <= 0) {
			dc->MoveTo (client_x, client_y);
		}

		if (	percent_x >= 0 && percent_x <= 1.0F &&
				percent_y >= 0 && percent_y <= 1.0F)
		{
			dc->LineTo (client_x, client_y);
		}
	}

	dc->LineTo (end_pt.x, end_pt.y);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Render_Points
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Render_Points (CDC *dc)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	if (spline.Key_Count () < 1) {
		return ;
	}

	//
	//	Draw the points
	//
	CBrush *old_brush = dc->SelectObject (&m_Brush);

	int count = spline.Key_Count ();
	for (int index = 0; index < count; index ++) {

		float time = 0;
		float value;
		spline.Get_Key (index, &value, &time);

		CPoint graph_pt;
		Value_To_Point (Vector2 (time,value), &graph_pt);

		if (m_SelPt == index) {			
			dc->SelectObject (&m_SelBrush);
			dc->Ellipse (graph_pt.x - 4, graph_pt.y - 4, graph_pt.x + 4, graph_pt.y + 4);
			dc->SelectObject (&m_Brush);
		} else {
			dc->Ellipse (graph_pt.x - 4, graph_pt.y - 4, graph_pt.x + 4, graph_pt.y + 4);
		}

		if (m_DraggingPt == index) {

			CRect label_rect;
			label_rect.left	= graph_pt.x - 30;
			label_rect.right	= graph_pt.x + 30;
			label_rect.top		= graph_pt.y - 30;
			label_rect.bottom	= graph_pt.y - 6;
			
			CString label;
			label.Format ("%.2f, %.2f", time, value);
			dc->DrawText (label, label_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		}
	}

	dc->SelectObject (old_brush);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Visible_Points
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Get_Visible_Points (int *left_pt, int *right_pt)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	(*left_pt) = -1;
	(*right_pt) = -1;

	CRect graph_area;
	Get_Graph_Rect (graph_area);

	float delta_x	= m_Max.X - m_Min.X;
	float delta_y	= m_Max.Y - m_Min.Y;

	int count = spline.Key_Count ();
	for (int index = 0; index < count; index ++) {

		float time = 0;
		float value;
		spline.Get_Key (index, &value, &time);
		
		float percent_x = (time - m_Min.X) / delta_x;
		float percent_y = 1.0F - ((value - m_Min.Y) / delta_y);

		//
		//	Is this point in the current view?
		//
		if (	percent_x >= 0 && percent_x <= 1.0F &&
				percent_y >= 0 && percent_y <= 1.0F)
		{			
			if ((*left_pt) == -1) {
				(*left_pt) = index;
			}

			(*right_pt) = index;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Hit_Test
//
///////////////////////////////////////////////////////////////////////
CSimpleGraphView::HITTYPE
CSimpleGraphView::Hit_Test (const CPoint &point, int *hit_pt)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();
	HITTYPE retval = HIT_UNKNOWN;

	CRect graph_area;
	Get_Graph_Rect (graph_area);

	//CPoint graph_pt;
	//graph_pt.x	= point.x - graph_area.left;
	//graph_pt.y	= point.y - graph_area.top;

	if (	(point.x >= 0 && point.x < graph_area.left) ||
			(point.y >= 0 && point.y < graph_area.top))	
	{		
		retval = HIT_AXIS;

	} else if (point.x >= graph_area.left && point.y >= graph_area.top) {

		retval = HIT_GRAPH_AREA;

		float delta_x	= m_Max.X - m_Min.X;
		float delta_y	= m_Max.Y - m_Min.Y;

		int count = spline.Key_Count ();
		for (int index = 0; index < count; index ++) {

			float time = 0;
			float value;
			spline.Get_Key (index, &value, &time);
			
			float percent_x = (time - m_Min.X) / delta_x;
			float percent_y = 1.0F - ((value - m_Min.Y) / delta_y);

			int graph_x = graph_area.left + int(graph_area.Width () * percent_x);
			int graph_y = graph_area.top + int(graph_area.Height () * percent_y);

			//
			//	Are we within 'fudge' of the graph point?
			//
			if (	::abs (graph_x - point.x) < 8 &&
					::abs (graph_y - point.y) < 8)
			{
				(*hit_pt) = index;
				retval = HIT_POINT;
				break;
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	OnLButtonDown
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnLButtonDown (UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_RBUTTON) == 0 && m_DraggingPt == -1) {
		
		int point_index	= 0;
		HITTYPE hit_type	= Hit_Test (point, &point_index);

		if (hit_type == HIT_GRAPH_AREA) {		
			m_DraggingPt = Add_New_Point (point);
		} else if (hit_type == HIT_POINT) {		
			m_DraggingPt = point_index;
		}

		if (m_DraggingPt >= 0) {
			m_SelPt = m_DraggingPt;
			Repaint_Graph ();
			SetCapture ();
		}
	}

	CView::OnLButtonDown (nFlags, point);
	return; 
}


///////////////////////////////////////////////////////////////////////
//
//	OnLButtonDown
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnLButtonUp (UINT nFlags, CPoint point) 
{
	if (m_DraggingPt >= 0) {		
		::ReleaseCapture ();
		m_DraggingPt = -1;
		Repaint_Graph ();
	}

	CView::OnLButtonUp (nFlags, point);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnMouseMove
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (nFlags & MK_LBUTTON && nFlags & MK_RBUTTON) {

		CPoint delta = m_ZoomPt - point;		

		float delta_x = m_Max.X - m_Min.X;
		float delta_y = m_Max.Y - m_Min.Y;

		CRect graph_area;
		Get_Graph_Rect (graph_area);

		float units_x = (delta.x * delta_x) / (float)graph_area.Width ();
		float units_y = -(delta.y * delta_y) / (float)graph_area.Height ();
		
		m_Min.X += units_x;
		m_Min.Y += units_y;
		m_Max.X += units_x;
		m_Max.Y += units_y;

		InvalidateRect (NULL, true);
		UpdateWindow ();

	} else if (m_IsZooming) {
		
		CPoint delta = m_ZoomPt - point;		

		float delta_x = m_Max.X - m_Min.X;
		float delta_y = m_Max.Y - m_Min.Y;

		float factor = ((float)delta.y) / 100.0F;
		/*if (delta.y < 0) {
			factor = 100.0F / ((float)delta.y);
		}*/

		m_Min.X = m_Min.X - (factor * delta_x);
		m_Min.Y = m_Min.Y - (factor * delta_y);

		m_Max.X = m_Max.X + (factor * delta_x);
		m_Max.Y = m_Max.Y + (factor * delta_y);

		InvalidateRect (NULL, true);
		UpdateWindow ();

	} else if (m_DraggingPt >= 0) {

		//
		//	Determine the new value based on the the screen location
		// of the mouse cursor...
		//
		Vector2 new_value;
		Point_To_Value (point, &new_value);

		new_value.X = std::clamp (new_value.X, m_Min.X, m_Max.X);
		new_value.Y = std::clamp (new_value.Y, m_Min.Y, m_Max.Y);

		//
		//	Make sure the new point is sorted correctly
		//
		m_DraggingPt	= Move_Value (m_DraggingPt, new_value);
		m_SelPt			= m_DraggingPt;
	}
	
	m_ZoomPt = point;

	CView::OnMouseMove (nFlags, point);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_New_Point
//
///////////////////////////////////////////////////////////////////////
int
CSimpleGraphView::Add_New_Point (CPoint &point)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	Vector2 new_value;
	Point_To_Value (point, &new_value);


	new_value.X = std::clamp (new_value.X, m_Min.X, m_Max.X);
	new_value.Y = std::clamp (new_value.Y, m_Min.Y, m_Max.Y);

	float time = new_value.X;
	float value = new_value.Y;
	int index = spline.Add_Key (value,time);

	return Move_Value (index, new_value);
}


///////////////////////////////////////////////////////////////////////
//
//	Move_Value
//
///////////////////////////////////////////////////////////////////////
int
CSimpleGraphView::Move_Value (int old_index, const Vector2 &new_value)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	int new_index			= -1;
	int count				= spline.Key_Count ();	
	Vector2 *point_list	= new Vector2[count];
	int curr_index			= 0;

	for (int list_index = 0; list_index < count; list_index ++) {
		
		//
		//	Skip the old value
		//
		if (list_index != old_index) {

			//
			//	Get this key's value
			//
			float time = 0;
			float value;
			spline.Get_Key (list_index, &value, &time);
			
			Vector2 curr_point;
			curr_point.X = time;
			curr_point.Y = value;

			//
			//	Should the moving-value be inserted before the current value?
			//
			if (new_index == -1 && (new_value.X < curr_point.X)) {
				new_index = curr_index++;
				point_list[new_index].Set (new_value.X, new_value.Y);
			}

			//
			//	Add the current value to the list
			//
			point_list[curr_index++].Set (curr_point.X, curr_point.Y);
		}		
	}

	if (old_index != -1 && new_index == -1) {
		new_index = curr_index;
		point_list[new_index].Set (new_value.X, new_value.Y);		
	}

	//
	//	Remove all the keys from the spline
	//
	for (int index = 0; index < count; index ++) {
		spline.Remove_Key (0);
	}

	//
	//	Add the new 'sorted' keys back into the spline
	//
	float time;
	float key;
	float last_time = -9999999999.0F;
	
	for (index = 0; index < count; index ++) {
		
		time = point_list[index].X;
		key = point_list[index].Y;

		if (time == last_time) {
			spline.Add_Key(key,time + WWMATH_EPSILON);
		} else {
			spline.Add_Key(key,time);
		}
		last_time = time;
	}

	Repaint_Graph ();	

	delete [] point_list;
	return new_index;
}


///////////////////////////////////////////////////////////////////////
//
//	Point_To_Value
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Point_To_Value (const CPoint &point, Vector2 *value) 
{
	CRect graph_area;
	Get_Graph_Rect (graph_area);

	CPoint graph_pt;
	graph_pt.x	= point.x - graph_area.left;
	graph_pt.y	= point.y - graph_area.top;

	float percent_x = (float)graph_pt.x / (float)graph_area.Width ();
	float percent_y = (float)graph_pt.y / (float)graph_area.Height ();

	value->X = m_Min.X + ((m_Max.X - m_Min.X) * percent_x);
	value->Y = m_Min.Y + ((m_Max.Y - m_Min.Y) * (1.0F - percent_y));
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Value_To_Point
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Value_To_Point (const Vector2 &value, CPoint *point) 
{
	CRect graph_area;
	Get_Graph_Rect (graph_area);

	float percent_x = (value.X - m_Min.X) / (m_Max.X - m_Min.X);
	float percent_y = (value.Y - m_Min.Y) / (m_Max.Y - m_Min.Y);

	point->x = graph_area.left + int(graph_area.Width () * percent_x);
	point->y = graph_area.top + int(graph_area.Height () * (1.0F - percent_y));	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Graph_Rect
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Get_Graph_Rect (CRect &rect) 
{
	CRect view_rect;
	GetClientRect (&view_rect);
	
	rect.left		= 75;
	rect.right		= view_rect.Width () - 25;
	rect.top			= 25;
	rect.bottom		= view_rect.Height () - 50;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnInitialUpdate
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnInitialUpdate (void)
{
	HDC screen_dc = ::GetDC (NULL);
	m_Font	= CreateFont (-::MulDiv (7, GetDeviceCaps(screen_dc, LOGPIXELSY), 72),
										0,
										0,
										0,
										FW_REGULAR,
										false,
										false,
										false,
										ANSI_CHARSET,
										OUT_DEFAULT_PRECIS,
										CLIP_DEFAULT_PRECIS,
										DEFAULT_QUALITY,
										DEFAULT_PITCH,
										"Small Fonts");
	::ReleaseDC (NULL, screen_dc);

	CView::OnInitialUpdate();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Delete_Point
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::Delete_Point (int index)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();

	if (index >= 0 && index < spline.Key_Count ()) {
		spline.Remove_Key (index);

		if (index == m_SelPt) {
			m_SelPt = -1;
		}

		if (index == m_DraggingPt) {
			m_DraggingPt = -1;
		}

		Repaint_Graph ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnDelete
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnDelete (void)
{
	if (m_SelPt != -1) {
		Delete_Point (m_SelPt);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnUpdateDelete
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnUpdateDelete (CCmdUI *pCmdUI) 
{
	pCmdUI->Enable (m_SelPt != -1);
	return ;	
}


///////////////////////////////////////////////////////////////////////
//
//	OnZoomExtents
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnZoomExtents (void)
{
	Curve1DClass &spline = GetDocument ()->Get_Spline  ();
	int count = spline.Key_Count ();
	
	if (count > 0) {

		m_Min.Set (99999.0F, 99999.0F);
		m_Max.Set (-99999.0F, -99999.0F);

		float	start_time	= spline.Get_Start_Time ();
		float end_time		= spline.Get_End_Time ();		

		float time_inc = (end_time - start_time) / 400.0F;
		float time = start_time;
		for (int index = 0; index < 400; index ++) {

			float value;
			spline.Evaluate (time, &value);

			m_Min.X = min (m_Min.X, time);
			m_Min.Y = min (m_Min.Y, value);

			m_Max.X = std::max (m_Max.X, time);
			m_Max.Y = std::max (m_Max.Y, value);			

			time += time_inc;
		}
		
		float delta_x = m_Max.X - m_Min.X;
		float delta_y = m_Max.Y - m_Min.Y;

		m_Min.X -= (delta_x / 100.0F);
		m_Min.Y -= (delta_y / 100.0F);
		m_Max.X += (delta_x / 100.0F);
		m_Max.Y += (delta_y / 100.0F);
	
		InvalidateRect (NULL, true);
		UpdateWindow ();
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnRButtonDown
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnRButtonDown (UINT nFlags, CPoint point) 
{
	if (m_IsZooming == false) {
		m_ZoomPt = point;
		m_IsZooming = true;
		SetCapture ();
	}

	CView::OnRButtonDown (nFlags, point);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnRButtonUp
//
///////////////////////////////////////////////////////////////////////
void
CSimpleGraphView::OnRButtonUp (UINT nFlags, CPoint point) 
{
	if (m_IsZooming) {
		m_IsZooming = false;
		::ReleaseCapture ();
	}

	CView::OnRButtonUp(nFlags, point);
	return ;
}

