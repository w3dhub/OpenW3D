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

#if !defined(AFX_SURRENDERWND_H__33722FC2_8D65_466F_9EF6_F5D6E229C20A__INCLUDED_)
#define AFX_SURRENDERWND_H__33722FC2_8D65_466F_9EF6_F5D6E229C20A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SurrenderWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSurrenderWnd window
class CameraClass;
class SimpleSceneClass;
class RenderObjClass;
class HAnimClass;
class Text2DObjClass;
class ConvertClass;

class CSurrenderWnd : public CWnd
{
// Construction
public:
	CSurrenderWnd(CWnd *cwnd, int driver, int width, int height, int bitdepth, bool run_in_window);
	void RemoveSubclass();

// Attributes
public:
	CameraClass			*m_Camera;
	CameraClass			*m_TextCamera;
	SimpleSceneClass	*m_Scene;
	SimpleSceneClass	*m_TextScene;
	RenderObjClass		*m_RObj;
	Text2DObjClass		*m_Text;
	HAnimClass			*m_Motion;
	CWnd				*m_CWnd;
	int					m_MotionTime;
	HWND				m_RenderHWnd;
	int					m_FrameRate;
	LONG_PTR					m_SubClass;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurrenderWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSurrenderWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSurrenderWnd)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURRENDERWND_H__33722FC2_8D65_466F_9EF6_F5D6E229C20A__INCLUDED_)
