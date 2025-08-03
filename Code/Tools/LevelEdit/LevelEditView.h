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

// LevelEditView.h : interface of the CLevelEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELEDITVIEW_H__6D711A2B_651E_11D2_9FC8_00104B791122__INCLUDED_)
#define AFX_LEVELEDITVIEW_H__6D711A2B_651E_11D2_9FC8_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "LevelEditDoc.h"

class CLevelEditView : public CView
{
protected: // create from serialization only
	CLevelEditView();
	DECLARE_DYNCREATE(CLevelEditView)

// Attributes
public:
	CLevelEditDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnFinalRelease();
	virtual void OnInitialUpdate();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLevelEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CLevelEditView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		////////////////////////////////////////////////////
		//
		//	Public methods
		//

		//
		//	Render device methods
		//
		void							Set_Render_Device_Info (int device_index, int bits_per_pixel) { m_iDeviceIndex = device_index; m_iBitsPerPixel = bits_per_pixel; }
		bool							Initialize_Render_Device (void);
		bool							Is_Windowed (void) { return bool(m_iWindowed == 1); }
		void							Set_Windowed (bool windowed) { m_iWindowed = (int)windowed; Initialize_Render_Device (); }
	
		//
		//	Painting methods
		//
		void							Repaint_View (void);
		void							Start_Update (bool bstart = true);
		bool							Is_Updating (void) { return (m_TimerID != 0); }
		HHOOK							Get_Hook (void) { return m_hMouseHook; }

		//
		//	Misc accessors
		//
		class CameraMgr *			Get_Camera_Mgr (void) const { return m_pCameraMgr; }
		class MouseMgrClass *	Get_Mouse_Mgr (void) const { return m_pMouseMgr; }
		class SceneClass *		Get_2D_Scene (void) const { return m_p2DScene; }

		//
		//	'New' level methods
		//
		void							Reset_View (void);

		//
		//	Cursor aspect ratio
		//
		float							Get_Cursor_Aspect_RatioX (void) const { return m_CursorXRatio; }
		float							Get_Cursor_Aspect_RatioY (void) const { return m_CursorYRatio; }

		//
		//	Static methods
		//
		static int					_iPaintingLock;
		static bool					_bNeedsRepaint;
		static void					Allow_Repaint (bool active);

	private:
		
		////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////
		bool							m_bActive;
		int							m_iDeviceIndex;
		int							m_iBitsPerPixel;
		bool							m_bRenderDeviceInitialized;
		class CameraMgr *			m_pCameraMgr;
		class MouseMgrClass *	m_pMouseMgr;
		CPoint						m_LastMousePoint;
		UINT							m_TimerID;
		int							m_iWindowed;
		double						m_HorzFOV;
		double						m_VertFOV;
		float							m_CursorXRatio;
		float							m_CursorYRatio;
		HHOOK							m_hMouseHook;

		class SceneClass *		m_p2DScene;
		class CameraClass *		m_p2DCamera;
		class DynamicScreenMeshClass *	m_pCursorMesh;
};

#ifndef _DEBUG  // debug version in LevelEditView.cpp
inline CLevelEditDoc* CLevelEditView::GetDocument()
   { return (CLevelEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELEDITVIEW_H__6D711A2B_651E_11D2_9FC8_00104B791122__INCLUDED_)
