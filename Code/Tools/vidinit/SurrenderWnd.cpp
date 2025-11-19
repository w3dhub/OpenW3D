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

// SurrenderWnd.cpp : implementation file
//

#include "stdafx.h"
#include "vidinit.h"
#include "SurrenderWnd.h"
#include <camera.h>
#include <scene.h>
#include <ww3d.h>
#include <assetmgr.h>
#include <rawfile.h>
#include <rendobj.h>
#include <hanim.h>
#include <txt2d.h>
#include "convert.h"
#include "defpal.h"
#include "deffont.h"
#include <bsurface.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static WW3DAssetManager AssetManager;
/////////////////////////////////////////////////////////////////////////////
// CSurrenderWnd

CSurrenderWnd::CSurrenderWnd(CWnd *cwnd, int driver, int width, int height, int bitdepth, bool run_in_window) :
	m_CWnd(cwnd),
	m_Motion(0),
	m_Camera(0),
	m_Scene(0),
	m_RObj(0),
	m_FrameRate(60)
{
	LPCTSTR str = AfxRegisterWndClass(CS_NOCLOSE, NULL, NULL, NULL);
	if (run_in_window) {
	   	CreateEx(0, str, "Card Test Window", WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION|WS_VISIBLE, 0,0,640,480, NULL, NULL, NULL);
	} else {
	   	CreateEx(0, str, "Card Test Window", WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION|WS_VISIBLE, 0,0,640,480, NULL, NULL, NULL);
	}

	WW3D::Set_Window(GetSafeHwnd());
	
	m_SubClass = GetWindowLongPtr(GetSafeHwnd(), GWLP_WNDPROC);

	WW3D::Set_Render_Device(driver, width, height, bitdepth, run_in_window, true);
	AssetManager.Set_WW3D_Load_On_Demand(true);
//	WW3D::Set_Ext_Swap_Interval(1);

	m_Scene		= new SimpleSceneClass();
	m_TextScene = new SimpleSceneClass();

	m_Camera	= new CameraClass();
	m_Camera->Set_Transform(Matrix3D(1));
	m_Camera->Set_Clip_Planes(1.0f, 1000000.0f);
	m_Camera->Set_View_Plane(DEG_TO_RAD(45.0f), DEG_TO_RAD(33.0f));

	m_TextCamera = NEW_REF(CameraClass, ());
	m_TextCamera->Set_View_Plane(Vector2(-1,-1), Vector2(1,1));
	m_TextCamera->Set_Position(Vector3(0,0,1));
	m_TextCamera->Set_Clip_Planes(0.1, 10.0);

	m_RObj		= AssetManager.Create_Render_Obj("vidtest");
	m_RObj->Set_Transform(Matrix3D(1));

	m_Motion = AssetManager.Get_HAnim("vidtest1.vidtest1");
	m_MotionTime = GetTickCount();

//	m_RObj->Set_Animation(m_Motion, 0.0f, RenderObjClass::ANIM_MODE_MANUAL);
	m_RObj->Add(m_Scene);

	m_Text	  = new Text2DObjClass(DefaultFont, "Frame rate = %d", 0, 0, 15, 0, DefaultConvert, false, false, m_FrameRate);
	m_Text->Add(m_TextScene);


}

CSurrenderWnd::~CSurrenderWnd()
{
	if (m_RObj)	{
		m_RObj->Set_Animation();
		m_RObj->Release_Ref();
	}
	if (m_Camera)		m_Camera->Release_Ref();
	if (m_Motion)		m_Motion->Release_Ref();
	if (m_Text)			m_Text->Release_Ref();
	if (m_TextCamera)	m_TextCamera->Release_Ref();
	if (m_Scene)		m_Scene->Release_Ref();
	if (m_TextScene)	m_TextScene->Release_Ref();

	AssetManager.Free_Assets();
}


BEGIN_MESSAGE_MAP(CSurrenderWnd, CWnd)
	//{{AFX_MSG_MAP(CSurrenderWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSurrenderWnd message handlers

void CSurrenderWnd::OnPaint() 
{
//	CPaintDC dc(this); // device context for painting
	
	WW3D::Sync(GetTickCount());

	if (m_Motion && m_RObj) {
		float rate			= m_Motion->Get_Frame_Rate();
		// figure out what the current frame is
		float currentframe	= (GetTickCount() - m_MotionTime)  / (1000 / rate);
		float numframes		= m_Motion->Get_Num_Frames() - 1;
		currentframe = fmod(currentframe, numframes);

		float frame_rate = 1.0f / (WW3D::Get_Frame_Time() * 0.001f);
		m_FrameRate = (0.9f * m_FrameRate) + (0.1f * frame_rate);



		m_Text->Set_Text(DefaultFont, "Frame rate = %d", 0, 0, 15, 0, DefaultConvert, false, false, m_FrameRate);
		m_RObj->Set_Animation(m_Motion, currentframe);

		Matrix3D m = m_RObj->Get_Bone_Transform("camera");
		m.Rotate_Y(DEG_TO_RAD(-90.0f));
		m.Rotate_Z(DEG_TO_RAD(-90.0f));
		m_Camera->Set_Transform(m);
	}

	// Do not call CWnd::OnPaint() for painting messages
	if (!IsIconic()) {
		WW3D::Begin_Render(true, true, Vector3(0.0f,0,0));
		WW3D::Render(m_Scene, m_Camera, false, false, Vector3(0,0,0));
		WW3D::Render(m_TextScene, m_TextCamera, false, false, Vector3(0,0,0));
		WW3D::End_Render();


	}
}

void CSurrenderWnd::RemoveSubclass() 
{
	::SetWindowLongPtr(GetSafeHwnd(), GWLP_WNDPROC, m_SubClass);
}

void CSurrenderWnd::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnClose();
	delete this;
}


void CSurrenderWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (nChar == VK_ESCAPE) {
		m_CWnd->PostMessage(WM_USER + 1, 0xDEADBEEF, 0xDEADBEEF);
	} else {
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}

}
