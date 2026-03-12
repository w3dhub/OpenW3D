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

// GraphicView.h : interface of the CGraphicView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYSTESTVIEW_H__616293EC_E4F0_11D2_802E_0040056350C8__INCLUDED_)
#define AFX_PHYSTESTVIEW_H__616293EC_E4F0_11D2_802E_0040056350C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPhysTestDoc;
class RenderObjClass;
class CameraClass;
class SceneClass;
class ChunkSaveClass;
class ChunkLoadClass;

class CGraphicView : public CView
{
protected: // create from serialization only
	CGraphicView();
	DECLARE_DYNCREATE(CGraphicView)

// Attributes
public:
	CPhysTestDoc*					GetDocument();
	void								Save(ChunkSaveClass & csave);
	void								Load(ChunkLoadClass & cload);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphicView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	enum {
		CAMERA_FLY			= 0,
		CAMERA_FOLLOW,
		CAMERA_TETHER,
		CAMERA_RIGID_TETHER,
	};

	virtual ~CGraphicView();
	BOOL			Initialize_WW3D(int device,int bits);
	bool			Is_WW3D_Initialized(void)						{ return Initialized; }

	void			Repaint_View(void);
	void			Set_Active(bool onoff);

	bool			Is_Simulation_Enabled(void)					{ return RunSimulation; }
	void			Enable_Simulation(bool onoff)					{ RunSimulation = onoff; }
	bool			Is_Collision_Box_Display_Enabled(void);
	void			Enable_Collision_Box_Display(bool onoff);

	void			Set_Camera_Mode(int mode)						{ CameraMode = mode; }
	int			Get_Camera_Mode(void)							{ return CameraMode; }

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	bool						Initialized;
	bool						Active;
	bool						RunSimulation;
	bool						DisplayBoxes;
	int						CameraMode;
	CameraClass *			Camera;
	int						TimerID;

	bool						LMouseDown;
	bool						RMouseDown;
	CPoint					LastPoint;

	CameraClass *			PipCamera;
	SceneClass *			PipScene;
	RenderObjClass *		Axes;

	void						Timestep(void);
	void						Update_Pip_Camera(void);

// Generated message map functions
protected:
	//{{AFX_MSG(CGraphicView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PhysTestView.cpp
inline CPhysTestDoc* CGraphicView::GetDocument()
   { return (CPhysTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSTESTVIEW_H__616293EC_E4F0_11D2_802E_0040056350C8__INCLUDED_)
