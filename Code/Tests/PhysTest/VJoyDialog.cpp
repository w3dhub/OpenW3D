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

// VJoyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "phystest.h"
#include "VJoyDialog.h"
#include "vector2.h"
#include "vector3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define JOYSTICK_UPDATE_COMMAND	WM_USER+101
#define SLIDER_RESOLUTION			255


/////////////////////////////////////////////////////////////////////////////
// CVJoyDialog dialog


CVJoyDialog::CVJoyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVJoyDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVJoyDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Controller.Reset();
}

CVJoyDialog::~CVJoyDialog(void)
{
}

void CVJoyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVJoyDialog)
	DDX_Control(pDX, IDC_TURNZ_SLIDER, m_TurnZSlider);
	DDX_Control(pDX, IDC_MOVEZ_SLIDER, m_MoveZSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVJoyDialog, CDialog)
	//{{AFX_MSG_MAP(CVJoyDialog)
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/*
** Joystick Window Proc
*/

LRESULT CALLBACK JoystickWndProc(HWND hwnd,unsigned int message,WPARAM wparam,LPARAM lparam)
{
	Vector2 point;
	const int RADIUS = 4;
	
	switch (message)
	{
	
	case WM_LBUTTONDOWN:
		::SetCapture(hwnd);
		break;

	case WM_LBUTTONUP:
		::ReleaseCapture();
		break;

	case WM_MOUSEMOVE:

		if (wparam & MK_LBUTTON) {
			
			float ex,ey,cx,cy;
			short x = LOWORD(lparam);
			short y = HIWORD(lparam);

			RECT rect;
			GetClientRect(hwnd,&rect);
			if (x > rect.right) x = rect.right;
			if (x < rect.left) x = rect.left;
			if (y > rect.bottom) y = rect.bottom;
			if (y < rect.top) y = rect.top;

			ex = (float)(rect.right - rect.left) / 2.0f;
			cx = (float)(rect.right + rect.left) / 2.0f;
			ey = (float)(rect.bottom - rect.top) / 2.0f;
			cy = (float)(rect.bottom + rect.top) / 2.0f;

			point.X = ((float)x - cx) / ex;
			point.Y = ((float)y - cy) / ey;
			
			::SendMessage(GetParent(hwnd),JOYSTICK_UPDATE_COMMAND,GetWindowLong(hwnd,GWL_ID),(int)&point);
			::InvalidateRect(hwnd,NULL,false);
			::UpdateWindow(hwnd);

			SetProp(hwnd,"XCOORD",(HANDLE)x);
			SetProp(hwnd,"YCOORD",(HANDLE)y);
		}	
		break;

	case WM_PAINT:
		{
			RECT rect;
			GetClientRect(hwnd,&rect);
			HDC hdc = GetDC(hwnd);
			FillRect(hdc,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
			FrameRect(hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));
			
			MoveToEx(hdc,rect.right/2,0,NULL);
			LineTo(hdc,rect.right/2,rect.bottom);
			MoveToEx(hdc,0,rect.bottom/2,NULL);
			LineTo(hdc,rect.right,rect.bottom/2);

			int x = (int)GetProp(hwnd,"XCOORD");
			int y = (int)GetProp(hwnd,"YCOORD");
			if (x < RADIUS) x = RADIUS;
			if (x > rect.right-RADIUS) x = rect.right-RADIUS;
			if (y < RADIUS) y = RADIUS;
			if (y > rect.bottom-RADIUS) y = rect.bottom-RADIUS;

			MoveToEx(hdc,rect.bottom/2,rect.right/2,NULL);
			LineTo(hdc,x,y);
			Ellipse(hdc,x-RADIUS,y-RADIUS,x+RADIUS,y+RADIUS);

			ReleaseDC(hwnd,hdc);
		}
		break;
	}
	
	WNDPROC oldwndproc = (WNDPROC)GetProp(hwnd,"OldWndProc");
	//return CallWindowProc(oldwndproc,hwnd,message,wparam,lparam);
	return DefWindowProc (hwnd, message, wparam, lparam);
}

/////////////////////////////////////////////////////////////////////////////
// CVJoyDialog message handlers

BOOL CVJoyDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT rect;
	
	HWND movexy_wnd = ::GetDlgItem(m_hWnd,IDC_MOVEXY_STATIC);	
	void *oldproc = (void *)SetWindowLongPtr(movexy_wnd,GWL_WNDPROC,(LONG_PTR)JoystickWndProc);
	SetProp(movexy_wnd,"OldWndProc",(void*)oldproc);
	
	::GetClientRect(movexy_wnd,&rect);
	SetProp(movexy_wnd,"XCOORD",(HANDLE)(rect.right/2));
	SetProp(movexy_wnd,"YCOORD",(HANDLE)(rect.bottom/2));

	HWND turnxy_wnd = ::GetDlgItem(m_hWnd,IDC_TURNXY_STATIC);
	oldproc = (void*)SetWindowLong(turnxy_wnd,GWLP_WNDPROC,(LONG_PTR)JoystickWndProc);
	SetProp(turnxy_wnd,"OldWndProc",(void*)oldproc);

	::GetClientRect(movexy_wnd,&rect);
	SetProp(turnxy_wnd,"XCOORD",(HANDLE)(rect.right/2));
	SetProp(turnxy_wnd,"YCOORD",(HANDLE)(rect.bottom/2));

	m_MoveZSlider.SetRange(0,SLIDER_RESOLUTION);
	m_TurnZSlider.SetRange(0,SLIDER_RESOLUTION);

	return true;
}


LRESULT CVJoyDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == JOYSTICK_UPDATE_COMMAND) {
		Vector2 * point = (Vector2*)lParam;
		if (wParam == IDC_MOVEXY_STATIC) {
			Controller.Set_Move_Forward(-point->Y);
			Controller.Set_Turn_Left(-point->X);
		} else if (wParam == IDC_TURNXY_STATIC) {
			// Nothing to do here anymore...
		}
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CVJoyDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == GetDlgItem(IDC_MOVEZ_SLIDER)) {
		Controller.Set_Move_Up(1.0f - 2.0f * (float)m_MoveZSlider.GetPos() / (float)SLIDER_RESOLUTION);
	} 
#if 0
	else if (pScrollBar == GetDlgItem(IDC_TURNZ_SLIDER)) {
		Controller.Set_Turn_Left(1.0f - 2.0f * (float)m_TurnZSlider.GetPos() / (float)SLIDER_RESOLUTION);
	}
#endif
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CVJoyDialog::OnDestroy() 
{
	HWND movexy_wnd = ::GetDlgItem(m_hWnd,IDC_MOVEXY_STATIC);	
	RemoveProp(movexy_wnd,"OldWndProc");
	RemoveProp(movexy_wnd,"XCOORD");
	RemoveProp(movexy_wnd,"YCOORD");
	HWND turnxy_wnd = ::GetDlgItem(m_hWnd,IDC_TURNXY_STATIC);
	RemoveProp(turnxy_wnd,"OldWndProc");
	RemoveProp(turnxy_wnd,"XCOORD");
	RemoveProp(turnxy_wnd,"YCOORD");

	CDialog::OnDestroy();
}
