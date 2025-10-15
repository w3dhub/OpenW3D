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

// VisErrorReportDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "VisErrorReportDialog.h"
#include "Utils.h"
#include "VisLog.h"
#include "SceneEditor.h"
#include "CameraMgr.h"
#include "phys.h"
#include "rendobj.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////
static const int AXIS_BMP_WIDTH	= 65;
static const int AXIS_BMP_HEIGHT	= 65;


/////////////////////////////////////////////////////////////////////////////
//
//	Local Inlines
//
/////////////////////////////////////////////////////////////////////////////
static inline HBITMAP Load_Bitmap (UINT res_id)
{
	return (HBITMAP)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (res_id), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
}

/////////////////////////////////////////////////////////////////////////////
//
// VisErrorReportDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VisErrorReportDialogClass::VisErrorReportDialogClass (CWnd *pParent)
	:	m_CurrentPoint (0),
		m_GraphArea (0, 0, 0, 0),
		m_AxisArea (0, 0, 0, 0),
		m_CurrentMinPoint (0),
		m_CurrentMaxPoint (0),
		m_TotalPoints (0),
		m_bTrackingSel (false),
		m_TrackPixel (0),
		m_SelStartIndex (0),
		m_SelEndIndex (0),
		m_AxisBMP (NULL),
		m_AxisSelBMP (NULL),
		m_AxisSel (0),
		CDialog(VisErrorReportDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VisErrorReportDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_PrevButton.normal		= Load_Bitmap (IDB_LT_ARROW);
	m_PrevButton.disabled	= Load_Bitmap (IDB_LT_ARROW_D);
	m_NextButton.normal		= Load_Bitmap (IDB_RT_ARROW);
	m_NextButton.disabled	= Load_Bitmap (IDB_RT_ARROW_D);
	m_AxisBMP					= Load_Bitmap (IDB_AXIS_BMP);
	m_AxisSelBMP				= Load_Bitmap (IDB_AXIS_SEL_BMP);

	m_AxisSelAreas[VIS_FORWARD].left		= 1;
	m_AxisSelAreas[VIS_FORWARD].top		= 38;
	m_AxisSelAreas[VIS_FORWARD].right	= m_AxisSelAreas[VIS_FORWARD].left + 20;
	m_AxisSelAreas[VIS_FORWARD].bottom	= m_AxisSelAreas[VIS_FORWARD].top + 16;

	m_AxisSelAreas[VIS_LEFT].left		= 1;
	m_AxisSelAreas[VIS_LEFT].top		= 12;
	m_AxisSelAreas[VIS_LEFT].right	= m_AxisSelAreas[VIS_LEFT].left + 20;
	m_AxisSelAreas[VIS_LEFT].bottom	= m_AxisSelAreas[VIS_LEFT].top + 16;

	m_AxisSelAreas[VIS_BACKWARDS].left		= 42;
	m_AxisSelAreas[VIS_BACKWARDS].top		= 13;
	m_AxisSelAreas[VIS_BACKWARDS].right		= m_AxisSelAreas[VIS_BACKWARDS].left + 20;
	m_AxisSelAreas[VIS_BACKWARDS].bottom	= m_AxisSelAreas[VIS_BACKWARDS].top + 16;
	
	m_AxisSelAreas[VIS_RIGHT].left	= 42;
	m_AxisSelAreas[VIS_RIGHT].top		= 39;
	m_AxisSelAreas[VIS_RIGHT].right	= m_AxisSelAreas[VIS_RIGHT].left + 20;
	m_AxisSelAreas[VIS_RIGHT].bottom	= m_AxisSelAreas[VIS_RIGHT].top + 16;

	m_AxisSelAreas[VIS_UP].left	= 21;
	m_AxisSelAreas[VIS_UP].top		= 1;
	m_AxisSelAreas[VIS_UP].right	= m_AxisSelAreas[VIS_UP].left + 20;
	m_AxisSelAreas[VIS_UP].bottom	= m_AxisSelAreas[VIS_UP].top + 16;

	m_AxisSelAreas[VIS_DOWN].left		= 22;
	m_AxisSelAreas[VIS_DOWN].top		= 47;
	m_AxisSelAreas[VIS_DOWN].right	= m_AxisSelAreas[VIS_DOWN].left + 20;
	m_AxisSelAreas[VIS_DOWN].bottom	= m_AxisSelAreas[VIS_DOWN].top + 16;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~VisErrorReportDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VisErrorReportDialogClass::~VisErrorReportDialogClass (void)
{
	//
	//	Free the BMPs
	//
	if (m_PrevButton.normal)	::DeleteObject (m_PrevButton.normal);
	if (m_PrevButton.disabled) ::DeleteObject (m_PrevButton.disabled);
	if (m_NextButton.normal)	::DeleteObject (m_NextButton.normal);
	if (m_NextButton.disabled) ::DeleteObject (m_NextButton.disabled);

	Free_Point_List ();
	return ;
}

/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VisErrorReportDialogClass)
	DDX_Control(pDX, IDC_ZOOM_SLIDER, m_ZoomSlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(VisErrorReportDialogClass, CDialog)
	//{{AFX_MSG_MAP(VisErrorReportDialogClass)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_PREV_ERROR, OnPrevError)
	ON_BN_CLICKED(IDC_NEXT_ERROR, OnNextError)
	ON_WM_DRAWITEM()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
VisErrorReportDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();	
	
	//
	//	Register this window for timer-based screen refreshing
	//
	::SetProp (m_hWnd, "ALLOW_UPDATE", (HANDLE)1);

	//
	//	Set the dimensions of the graph area
	//
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_HISTOGRAM), &m_GraphArea);
	ScreenToClient (&m_GraphArea);
	m_GraphArea.left ++;
	m_GraphArea.right --;
	m_GraphArea.top ++;
	m_GraphArea.bottom --;
	m_Graph.Set_Dimensions (m_GraphArea.Width (), m_GraphArea.Height ());

	//
	//	Set the dimensions of the 'axis' area.
	//
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_AXIS_AREA), &rect);
	ScreenToClient (&rect);
	rect.left ++;
	rect.right --;
	rect.top ++;
	rect.bottom --;

	m_AxisArea.left	= rect.left + (rect.Width () >> 1) - (AXIS_BMP_WIDTH >> 1);
	m_AxisArea.top		= rect.top + (rect.Height () >> 1) - (AXIS_BMP_HEIGHT >> 1);
	m_AxisArea.right	= m_AxisArea.left + AXIS_BMP_WIDTH;
	m_AxisArea.bottom	= m_AxisArea.top + AXIS_BMP_HEIGHT;
	
	// Update the dialog with data from the scene
	Re_Sync_Data ();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Re_Sync_Data
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Re_Sync_Data (void)
{
	//
	//	Get the error list from the scene
	//
	VisLogClass &vis_log				= ::Get_Scene_Editor ()->Get_Vis_Log ();	
	VIS_SAMPLE_LIST &error_list	= vis_log.Peek_Error_List ();	
	m_TotalPoints						= error_list.Count ();

	//
	//	Setup the 'zoom' slider
	//
	m_ZoomSlider.SetRange (2, m_TotalPoints >> 1);
	m_ZoomSlider.SetPos (m_TotalPoints >> 2);
	
	// Reset the range of values in the y-axis
	int max	= (m_TotalPoints >> 1) - (m_ZoomSlider.GetPos () - 2);
	m_Graph.Set_Y_Axis_Range (0, max);

	//
	//	Add the vis error points to the graph
	//	
	Free_Point_List ();
	int bad_points = 0;
	for (int index = 0; index < error_list.Count (); index ++) {
		VisSampleClass *sample = error_list[index];
		Add_Point (*sample);

		//
		//	Determining which direction resulted in the largest backface fraction
		//
		float fraction = sample->Get_Biggest_Fraction ();
		m_Graph.Add_Data_Point (fraction);

		//
		//	Determine if this point was bad or not
		//
		int dir_index = sample->Get_Biggest_Fraction_Index ();
		if (sample->Sample_Status ((VisDirType)dir_index) != VIS_STATUS_OK) {
			bad_points ++;
		}
	}

	//
	//	Set the dialog text fields
	//
	int points		= ::Get_Scene_Editor ()->Get_Total_Vis_Points ();
	SetDlgItemInt (IDC_VIS_POINTS, points);
	SetDlgItemInt (IDC_BAD_POINTS, bad_points);	
	SetDlgItemInt (IDC_MAX_POINTS, max);

	int bad_percent = (points > 0) ? ((bad_points * 100) / points) : 0;
	CString message;
	message.Format ("%d %%", bad_percent);
	SetDlgItemText (IDC_PERCENT_BAD, message);

	m_CurrentPoint = 0;
	m_CurrentMinPoint = 0;
	m_CurrentMaxPoint = 0;

	On_Graph_Sel_Changed ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Refresh_View
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Refresh_View (void)
{
	if (m_hWnd != NULL) {
		InvalidateRect (&m_GraphArea, FALSE);
		UpdateWindow ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnPaint
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnPaint (void)
{
	CPaintDC dc(this);
	m_Graph.Render (dc, m_GraphArea.left, m_GraphArea.top);
	Paint_Axis_Area (dc);
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// OnVScroll
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnVScroll
(
	UINT			nSBCode,
	UINT			nPos,
	CScrollBar *pScrollBar
) 
{	
	// Reset the range of values in the y-axis
	int max	= m_TotalPoints - (m_ZoomSlider.GetPos () - 2);
	m_Graph.Set_Y_Axis_Range (0, max);
	SetDlgItemInt (IDC_MAX_POINTS, max);

	//
	//	Repaint the graph
	//
	Refresh_View ();	

	CDialog::OnVScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonDown
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnLButtonDown (UINT nFlags, CPoint point) 
{
	if (m_GraphArea.PtInRect (point)) {
		 m_TrackPixel = point.x - m_GraphArea.left;
		 m_Graph.Set_Selection (m_TrackPixel, m_TrackPixel);
		 m_bTrackingSel = true;
		 SetCapture ();
		 Refresh_View ();
		 On_Graph_Sel_Changed ();
	} else if (m_AxisArea.PtInRect (point)) {
		Update_Axis_Area (true);
	}

	CDialog::OnLButtonDown (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonUp
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnLButtonUp (UINT nFlags, CPoint point) 
{
	if (m_bTrackingSel) {
		 m_bTrackingSel = false;
		 ReleaseCapture ();
	}

	CDialog::OnLButtonUp (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnMouseMove
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (m_bTrackingSel) {		 
		 int new_pixel = point.x - m_GraphArea.left;
		 new_pixel = std::max (new_pixel, 0);
		 new_pixel = std::min<int> (new_pixel, m_GraphArea.right-1);
		 m_Graph.Set_Selection (m_TrackPixel, new_pixel);
		 Refresh_View ();
		 On_Graph_Sel_Changed ();
	}

	CDialog::OnMouseMove (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Display_Point
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Display_Point (const Matrix3D &transform, bool fly_to)
{
	if (fly_to) {
		::Get_Camera_Mgr ()->Fly_To_Transform (transform);	
	} else {
		::Get_Camera_Mgr ()->Set_Transform (transform);
	}

	::Refresh_Main_View ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Free_Point_List
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Free_Point_List (void)
{
	//
	//	Delete all the vis samples
	//
	for (int index = 0; index < m_PointList.Count (); index ++) {
		VisSampleClass *vis_sample = m_PointList[index];
		SAFE_DELETE (vis_sample);
	}

	m_PointList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Add_Point
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Add_Point (const VisSampleClass &point)
{
	int start		= 0;
	int end			= m_PointList.Count ();
	end				= std::max (end, 0);
	float fraction = point.Get_Biggest_Fraction ();

	int index = end;
	while (end > start)
	{
		index						= start + ((end - start) >> 1);
		float curr_fraction	= m_PointList[index]->Get_Biggest_Fraction ();

		if (fraction > curr_fraction) {
			if (start == index) { index ++; break; }
			start = index;
		} else if (fraction < curr_fraction) {
			if (end == index) { index --; break; }
			end = index;
		} else {
			break;
		}

	}

	ASSERT (index >=0 );

	//
	//	Simple binary search to find the right place
	// to insert the new data point
	//
	/*while (end > start) {
		
		int index = start + ((end - start) >> 1);
		float curr_fraction = m_PointList[index]->BackfaceFraction;
		if ((index == start) || (fraction == curr_fraction)) {
			break;
		} else if (fraction > curr_fraction) {
			start = index;
		} else if (fraction < curr_fraction) {
			end = index;
		}
	}*/

	//
	//	Insert the new point into the list
	//
	m_PointList.Insert (index, new VisSampleClass (point));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Value_Index
//
/////////////////////////////////////////////////////////////////////////////
int
VisErrorReportDialogClass::Find_Value_Index (float fraction)
{
	/*int retval = -1;

	//
	//	Simple binary search to find the right place
	// to insert the new data point
	//
	int start	= 0;
	int end		= m_PointList.Count () - 1;
	while ((end > start) && (retval == -1)) {
		
		int index = start + ((end - start) >> 1);
		float curr_fraction = m_PointList[index]->BackfaceFraction;
		if (index == start) {
			retval = (fraction > curr_fraction) ? end : start;
			break;
		} else if (fraction > curr_fraction) {
			start = index;
		} else if (fraction < curr_fraction) {
			end = index;
		} else {
			retval = index;
		}
	}

	if (retval == -1) {
		retval = start;
	}*/

	int start		= 0;
	int end			= m_PointList.Count ();
	end				= std::max (end, 0);

	int index = end;
	while (end > start)
	{
		index						= start + ((end - start) >> 1);
		float curr_fraction	= m_PointList[index]->Get_Biggest_Fraction ();

		if (fraction > curr_fraction) {
			if (start == index) { index ++; break; }
			start = index;
		} else if (fraction < curr_fraction) {
			if (end == index) { index --; break; }
			end = index;
		} else {
			
			while ((curr_fraction == fraction) && (index > 0)) {
				curr_fraction = m_PointList[--index]->Get_Biggest_Fraction ();
			}

			index = (curr_fraction == fraction) ? index : index + 1;

			//m_ValueList[index].count ++;
			//found = true;
			break;
		}

	}

	ASSERT (index >=0 );

	return index;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	On_Graph_Sel_Changed
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::On_Graph_Sel_Changed (void)
{
	float range_min = 0;
	float range_max = 0;
	m_Graph.Get_Selection (range_min, range_max);

	if (range_min >= 0 && range_max >= 0) {
		m_SelStartIndex	= Find_Value_Index (range_min);
		m_SelEndIndex		= Find_Value_Index (range_max);
	} else {
		m_SelStartIndex	= 0;
		m_SelEndIndex		= m_PointList.Count () - 1;
		m_SelEndIndex		= std::max (m_SelEndIndex, 0);
	}

	m_CurrentPoint = m_SelStartIndex - 1;

	//
	//	Update the dialog UI
	//
	Update_Current_Point_Text ();
	Update_Nav_Buttons ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Update_Nav_Buttons
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Update_Nav_Buttons (void)
{
	bool enable_prev = m_CurrentPoint > m_SelStartIndex;
	bool enable_next = (m_CurrentPoint < m_SelEndIndex) && (m_PointList.Count () > 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PREV_ERROR), enable_prev);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_NEXT_ERROR), enable_next);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnPrevError
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnPrevError (void)
{
	if (m_CurrentPoint > m_SelEndIndex) {
		m_CurrentPoint = m_SelEndIndex;
	} else {
		m_CurrentPoint --;
		m_CurrentPoint = std::max (m_CurrentPoint, 0);
	}
	
	//
	//	Show the point
	//
	if (m_CurrentPoint < m_PointList.Count ()) {
		VisSampleClass *vis_sample	= m_PointList[m_CurrentPoint];
		int index						= vis_sample->Get_Biggest_Fraction_Index ();
		
		vis_sample->Set_Cur_Direction ((VisDirType)index);
		Display_Point (vis_sample->Get_Camera_Transform ((VisDirType)index), true);
	}

	//
	//	Update the UI
	//
	Update_Current_Point_Text ();
	Update_Nav_Buttons ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	OnNextError
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnNextError (void) 
{
	if (m_CurrentPoint < m_SelStartIndex) {
		m_CurrentPoint = m_SelStartIndex;
	} else {
		m_CurrentPoint ++;
		m_CurrentPoint = std::min (m_CurrentPoint, m_PointList.Count () - 1);
	}

	//
	//	Show the point
	//
	if (m_CurrentPoint < m_PointList.Count ()) {
		VisSampleClass *vis_sample	= m_PointList[m_CurrentPoint];
		int index						= vis_sample->Get_Biggest_Fraction_Index ();

		vis_sample->Set_Cur_Direction ((VisDirType)index);
		Display_Point (vis_sample->Get_Camera_Transform ((VisDirType)index), true);
	}

	//
	//	Update the UI
	//
	Update_Current_Point_Text ();
	Update_Nav_Buttons ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Update_Current_Point_Text
//
//////////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Update_Current_Point_Text (void)
{
	//
	//	Update the point-range static text control
	//
	int current_point = std::max ((m_CurrentPoint - m_SelStartIndex) + 1, 0);
	CString message;
	message.Format ("%d of %d", current_point, m_SelEndIndex - m_SelStartIndex);
	SetDlgItemText (IDC_CURRENT_POINT, message);

	if ((m_CurrentPoint >= 0) &&
		 (m_CurrentPoint < m_PointList.Count ()))
	{
		VisSampleClass *vis_sample	= m_PointList[m_CurrentPoint];
	
		//
		//	Update the backface percent for this sample
		//
		float fraction = vis_sample->Get_Biggest_Fraction ();
		message.Format ("%.2f %%", fraction * 100);
		SetDlgItemText (IDC_CURRENT_PERCENT, message);

		//
		//	Get the status string for the current direction
		//
		int dir_index = vis_sample->Get_Cur_Direction  ();
		switch (vis_sample->Sample_Status ((VisDirType)dir_index)) {
			
			case VIS_STATUS_OK:
				message = "Ok";
				break;

			case VIS_STATUS_NOT_TAKEN:
				message = "Not Taken";
				break;

			case VIS_STATUS_ERROR:
				message = "!!Error!!";
				break;

			case VIS_STATUS_BACKFACE_LEAK:
				message = "Leak";
				break;

			case VIS_STATUS_BACKFACE_OVERFLOW:
				message = "Overflow";
				break;
		}

		SetDlgItemText (IDC_CURRENT_POINT_STATUS, message);
	} else {
		SetDlgItemText (IDC_CURRENT_PERCENT, "");
		SetDlgItemText (IDC_CURRENT_POINT_STATUS, "");
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDrawItem
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnDrawItem
(
	int nIDCtl,
	LPDRAWITEMSTRUCT lpDrawItemStruct
) 
{
	//
	// Determine what state to draw the button in (pushed or normal)
	//
	UINT state = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
	POINT offset = { 0 };
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		state |= DFCS_PUSHED;
		offset.x = 1;
		offset.y = 1;
	}

	// Draw the button's outline
	CRect rect = lpDrawItemStruct->rcItem;
	::DrawFrameControl (lpDrawItemStruct->hDC, rect, DFC_BUTTON, state);

	//
	// Determine which BMP to use
	//
	HBITMAP hbmp = NULL;
	if (nIDCtl == IDC_PREV_ERROR) {
		hbmp = ::IsWindowEnabled (lpDrawItemStruct->hwndItem) ? m_PrevButton.normal : m_PrevButton.disabled;
	} else if (nIDCtl == IDC_NEXT_ERROR) {
		hbmp = ::IsWindowEnabled (lpDrawItemStruct->hwndItem) ? m_NextButton.normal : m_NextButton.disabled;
	}

	HDC mem_dc			= ::CreateCompatibleDC (NULL);
	HBITMAP old_bmp	= (HBITMAP)::SelectObject (mem_dc, hbmp);

	//
	//	Draw the BMP centered in the button
	//
	const int BMP_WIDTH	= 25;
	const int BMP_HEIGHT = 28;
	int x_pos = rect.left + (rect.Width () >> 1) - (BMP_WIDTH >> 1) + offset.x;
	int y_pos = rect.top + (rect.Height () >> 1) - (BMP_HEIGHT >> 1) + offset.y;
	::BitBlt (lpDrawItemStruct->hDC, x_pos, y_pos, BMP_WIDTH, BMP_HEIGHT, mem_dc, 0, 0, SRCCOPY);

	::SelectObject (mem_dc, old_bmp);
	::DeleteDC (mem_dc);
		
	// Draw the focus rectangle if necessary
	if (lpDrawItemStruct->itemState & ODS_FOCUS) {
		::DrawFocusRect (lpDrawItemStruct->hDC, &rect);
	}
	
	// Allow the base class to process this message
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Axis_Area
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::Paint_Axis_Area (HDC hdc)
{
	HDC mem_dc			= ::CreateCompatibleDC (NULL);
	HBITMAP old_bmp	= (HBITMAP)::SelectObject (mem_dc, m_AxisBMP);

	//
	//	Draw the BMP centered in the button
	//
	const int BMP_WIDTH	= 65;
	const int BMP_HEIGHT = 65;
	//int x_pos = m_AxisArea.left + (m_AxisArea.Width () >> 1) - (BMP_WIDTH >> 1);
	//int y_pos = m_AxisArea.top + (m_AxisArea.Height () >> 1) - (BMP_HEIGHT >> 1);
	::BitBlt (hdc, m_AxisArea.left, m_AxisArea.top, BMP_WIDTH, BMP_HEIGHT, mem_dc, 0, 0, SRCCOPY);

	::SelectObject (mem_dc, old_bmp);
	old_bmp	= (HBITMAP)::SelectObject (mem_dc, m_AxisSelBMP);

	//
	//	Paint the selected areas of the axis
	//
	for (int index = 0; index < 6; index ++) {
		if (m_AxisSel & (1 << index)) {
			CRect &rect = m_AxisSelAreas[index];
			int x_pos = m_AxisArea.left + rect.left;
			int y_pos = m_AxisArea.top + rect.top;
			::BitBlt (hdc, x_pos, y_pos, rect.Width (), rect.Height (), mem_dc, rect.left, rect.top, SRCCOPY);
		}
	}


	::SelectObject (mem_dc, old_bmp);
	::DeleteDC (mem_dc);	
	return ;
}

LRESULT VisErrorReportDialogClass::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CDialog::OnNcHitTest(point);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSetCursor
//
/////////////////////////////////////////////////////////////////////////////
BOOL
VisErrorReportDialogClass::OnSetCursor
(
	CWnd *	pwnd,
	UINT		hit_test,
	UINT		message
)
{
	BOOL retval = FALSE;

	//
	//	Determine if the cursor is over a 'hot-spot' in the axis area.
	//
	if (Update_Axis_Area ()) {
		::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_HAND_POINTER)));		
	} else {
		retval = CDialog::OnSetCursor (pwnd, hit_test, message);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Axis_Area
//
/////////////////////////////////////////////////////////////////////////////
bool
VisErrorReportDialogClass::Update_Axis_Area (bool do_view_change)
{
	CPoint point;
	::GetCursorPos (&point);
	ScreenToClient (&point);	

	point.x -= m_AxisArea.left;
	point.y -= m_AxisArea.top;	

	bool found			= false;	
	int old_sel			= m_AxisSel;
	m_AxisSel			= 0;

	//
	//	Only do the UI stuff if we have a current point.
	//
	if ((m_CurrentPoint >= 0) &&
		 (m_CurrentPoint < m_PointList.Count ()))
	{
		VisSampleClass *vis_sample	= m_PointList[m_CurrentPoint];

		if ((point.x >= 0) && (point.y >= 0)) {

			//
			//	Determine which direction we are over (if any)
			//
			for (int index = 0; index < 6 && !found; index ++) {
				if (m_AxisSelAreas[index].PtInRect (point)) {
					m_AxisSel	|= (1 << index);
					found			= true;
					
					//
					//	Update the camera if necessary
					//
					if (do_view_change) {
						vis_sample->Set_Cur_Direction ((VisDirType)index);
						Display_Point (vis_sample->Get_Camera_Transform ((VisDirType)index), false);
					}
				}
			}
		}
		
		//
		//	Hilight the direction that is currently displayed
		//		
		int direction	= vis_sample->Get_Cur_Direction ();
		int bit			= (1 << direction);
		m_AxisSel		|= bit;
	}
	
	if (old_sel != m_AxisSel) {

		//
		//	Repaint the axis area
		//
		HDC hdc = ::GetDC (m_hWnd);
		Paint_Axis_Area (hdc);
		::ReleaseDC (m_hWnd, hdc);
	}

	return found;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
VisErrorReportDialogClass::OnDestroy (void)
{
	CDialog::OnDestroy();
	::RemoveProp (m_hWnd, "ALLOW_UPDATE");
	return ;
}
