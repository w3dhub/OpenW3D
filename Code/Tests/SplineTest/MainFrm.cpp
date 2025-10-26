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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SplineTest.h"

#include "MainFrm.h"
#include "SplineTestDoc.h"
#include "SplineTestView.h"
#include "curve.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_CURVE_CARDINAL, OnCurveCardinal)
	ON_UPDATE_COMMAND_UI(ID_CURVE_CARDINAL, OnUpdateCurveCardinal)
	ON_COMMAND(ID_CURVE_CATMULL_ROM, OnCurveCatmullRom)
	ON_UPDATE_COMMAND_UI(ID_CURVE_CATMULL_ROM, OnUpdateCurveCatmullRom)
	ON_COMMAND(ID_CURVE_LINEAR, OnCurveLinear)
	ON_UPDATE_COMMAND_UI(ID_CURVE_LINEAR, OnUpdateCurveLinear)
	ON_COMMAND(ID_CURVE_TCB, OnCurveTcb)
	ON_UPDATE_COMMAND_UI(ID_CURVE_TCB, OnUpdateCurveTcb)
	ON_COMMAND(ID_CURVE_RESET, OnCurveReset)
	ON_COMMAND(ID_CURVE_DRAW_TANGENTS, OnCurveDrawTangents)
	ON_UPDATE_COMMAND_UI(ID_CURVE_DRAW_TANGENTS, OnUpdateCurveDrawTangents)
	ON_COMMAND(ID_CURVE_LOOP, OnCurveLoop)
	ON_UPDATE_COMMAND_UI(ID_CURVE_LOOP, OnUpdateCurveLoop)
	ON_COMMAND(IDM_VEHICLE_CURVE, OnVehicleCurve)
	ON_UPDATE_COMMAND_UI(IDM_VEHICLE_CURVE, OnUpdateVehicleCurve)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
#if 0
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
#endif

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
#if 0
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
#endif

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return false;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnCurveCardinal() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc) {
		doc->Set_Curve_Type(CSplineTestDoc::CARDINAL);
	}
}

void CMainFrame::OnUpdateCurveCardinal(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		pCmdUI->SetCheck(doc->Get_Curve_Type() == CSplineTestDoc::CARDINAL);
	}
}

void CMainFrame::OnCurveCatmullRom() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc) {
		doc->Set_Curve_Type(CSplineTestDoc::CATMULL_ROM);
	}
}

void CMainFrame::OnUpdateCurveCatmullRom(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		pCmdUI->SetCheck(doc->Get_Curve_Type() == CSplineTestDoc::CATMULL_ROM);
	}
}

void CMainFrame::OnCurveLinear() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc) {
		doc->Set_Curve_Type(CSplineTestDoc::LINEAR);
	}
}

void CMainFrame::OnUpdateCurveLinear(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		pCmdUI->SetCheck(doc->Get_Curve_Type() == CSplineTestDoc::LINEAR);
	}
}

void CMainFrame::OnCurveTcb() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc) {
		doc->Set_Curve_Type(CSplineTestDoc::TCB);
	}
}

void CMainFrame::OnUpdateCurveTcb(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		pCmdUI->SetCheck(doc->Get_Curve_Type() == CSplineTestDoc::TCB);
	}
}

void CMainFrame::OnCurveReset() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		doc->Reset_Curve();
	}
}

void CMainFrame::OnCurveDrawTangents() 
{
	CSplineTestView * view = (CSplineTestView *)GetActiveView();
	if (view) {
		view->Enable_Draw_Tangents(!view->Is_Draw_Tangents_Enabled());
	}
}

void CMainFrame::OnUpdateCurveDrawTangents(CCmdUI* pCmdUI) 
{
	CSplineTestView * view = (CSplineTestView *)GetActiveView();
	if (view) {
		pCmdUI->SetCheck(view->Is_Draw_Tangents_Enabled());
	}
}

void CMainFrame::OnCurveLoop() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		Curve3DClass * curve = doc->Get_Curve();
		if (curve) {
			curve->Set_Looping(!curve->Is_Looping());
		}
		doc->UpdateAllViews(NULL);
	}
}

void CMainFrame::OnUpdateCurveLoop(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		Curve3DClass * curve = doc->Get_Curve();
		if (curve) {
			pCmdUI->SetCheck(curve->Is_Looping());
		}
	}
}

void CMainFrame::OnVehicleCurve() 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc) {
		doc->Set_Curve_Type(CSplineTestDoc::VEHICLE	);
	}	
}

void CMainFrame::OnUpdateVehicleCurve(CCmdUI* pCmdUI) 
{
	CSplineTestDoc * doc = (CSplineTestDoc *)GetActiveDocument();
	if (doc != NULL) {
		pCmdUI->SetCheck(doc->Get_Curve_Type() == CSplineTestDoc::VEHICLE);
	}
}
