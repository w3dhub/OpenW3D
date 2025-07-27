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

#if !defined(AFX_VISERRORREPORTDIALOG_H__3CAE29F6_3FAF_11D3_A04C_00104B791122__INCLUDED_)
#define AFX_VISERRORREPORTDIALOG_H__3CAE29F6_3FAF_11D3_A04C_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VisErrorReportDialog.h : header file
//

#include "HistogramCtl.H"
#include "Resource.H"
#include "VisLog.H"


// Forward delcarations
class Matrix3D;

/////////////////////////////////////////////////////////////////////////////
//
// VisErrorReportDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class VisErrorReportDialogClass : public CDialog
{
// Construction
public:
	VisErrorReportDialogClass(CWnd* pParent = NULL);   // standard constructor
	~VisErrorReportDialogClass(void);

// Dialog Data
	//{{AFX_DATA(VisErrorReportDialogClass)
	enum { IDD = IDD_VIS_ERRORS };
	CSliderCtrl	m_ZoomSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VisErrorReportDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VisErrorReportDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPrevError();
	afx_msg void OnNextError();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:
		
		////////////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////////////
		void		Re_Sync_Data (void);
		void		Refresh_View (void);
		void		Display_Point (const Matrix3D &transform, bool fly_to);

		void		Add_Point (const VisSampleClass &point);
		void		Free_Point_List (void);

	protected:

		////////////////////////////////////////////////////////////////////////
		//	Protected methods
		////////////////////////////////////////////////////////////////////////
		void		On_Graph_Sel_Changed (void);
		int		Find_Value_Index (float fraction);
		void		Update_Nav_Buttons (void);
		void		Update_Current_Point_Text (void);
		void		Paint_Axis_Area (HDC hdc);
		bool		Update_Axis_Area (bool do_view_change = false);

	private:

		////////////////////////////////////////////////////////////////////////
		//	Private data types
		////////////////////////////////////////////////////////////////////////
		typedef struct
		{
			HBITMAP	normal;
			HBITMAP	disabled;
		} BUTTON_BMP;

		////////////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////////////		
		BUTTON_BMP				m_PrevButton;
		BUTTON_BMP				m_NextButton;		
		HBITMAP					m_AxisBMP;
		HBITMAP					m_AxisSelBMP;
		VIS_SAMPLE_LIST		m_PointList;
		HistogramCtlClass		m_Graph;
		CRect						m_GraphArea;
		CRect						m_AxisArea;
		int						m_AxisSel;
		CRect						m_AxisSelAreas[6];
		int						m_AxisHilightBit;
		int						m_CurrentPoint;
		int						m_CurrentMinPoint;
		int						m_CurrentMaxPoint;
		int						m_TotalPoints;
		bool						m_bTrackingSel;
		int						m_TrackPixel;

		int						m_SelStartIndex;
		int						m_SelEndIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_VISERRORREPORTDIALOG_H__3CAE29F6_3FAF_11D3_A04C_00104B791122__INCLUDED_)
