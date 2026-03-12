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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__2AE09FD9_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
#define AFX_MAINFRM_H__2AE09FD9_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
//	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCurveCardinal();
	afx_msg void OnUpdateCurveCardinal(CCmdUI* pCmdUI);
	afx_msg void OnCurveCatmullRom();
	afx_msg void OnUpdateCurveCatmullRom(CCmdUI* pCmdUI);
	afx_msg void OnCurveLinear();
	afx_msg void OnUpdateCurveLinear(CCmdUI* pCmdUI);
	afx_msg void OnCurveTcb();
	afx_msg void OnUpdateCurveTcb(CCmdUI* pCmdUI);
	afx_msg void OnCurveReset();
	afx_msg void OnCurveDrawTangents();
	afx_msg void OnUpdateCurveDrawTangents(CCmdUI* pCmdUI);
	afx_msg void OnCurveLoop();
	afx_msg void OnUpdateCurveLoop(CCmdUI* pCmdUI);
	afx_msg void OnVehicleCurve();
	afx_msg void OnUpdateVehicleCurve(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__2AE09FD9_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
