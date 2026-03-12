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

#if !defined(AFX_PERFORMANCECONFIGDIALOG_H__883CE8FB_E4D2_493A_84F0_C75F2A7826BB__INCLUDED_)
#define AFX_PERFORMANCECONFIGDIALOG_H__883CE8FB_E4D2_493A_84F0_C75F2A7826BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "vector.h"


/////////////////////////////////////////////////////////////////////////////
// PerformanceConfigDialogClass dialog

class PerformanceConfigDialogClass : public CDialog
{
// Construction
public:
	PerformanceConfigDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PerformanceConfigDialogClass)
	enum { IDD = IDD_PERFORMANCE_CONFIG };
	CSliderCtrl	m_GeometrySlider;
	CSliderCtrl	m_CharShadowsSlider;
	CSliderCtrl	m_TextureDetailSlider;
	CSliderCtrl	m_SurfaceEffectsSlider;
	CSliderCtrl	m_PerformanceSlider;
	CSliderCtrl	m_ParticleSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PerformanceConfigDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PerformanceConfigDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnExpertCheck();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnGraphicsAutoSetup();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////
	void			Apply_Changes (void);

private:

	/////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////
	void			Build_Expert_Window_List (void);
	void			Display_Expert_Settings (bool onoff);
	void			Setup_Controls (void);
	void			Update_Expert_Controls (int level);
	void			Load_Values (void);
	void			Determine_Performance_Setting (void);
	void			Get_Settings (DynamicVectorClass<int> &settings);

	/////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////
	DynamicVectorClass<HWND>	ExpertWindowList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFORMANCECONFIGDIALOG_H__883CE8FB_E4D2_493A_84F0_C75F2A7826BB__INCLUDED_)
