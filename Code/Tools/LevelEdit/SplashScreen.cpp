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

// SplashScreen.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "SplashScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////
//
//	SplashScreenClass
//
///////////////////////////////////////////////////////////////////////
SplashScreenClass::SplashScreenClass (void)
	:	m_Size (0, 0),
		m_hFont (NULL),
		m_hBitmap (NULL),
		m_hMemDC (NULL)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~SplashScreenClass
//
///////////////////////////////////////////////////////////////////////
SplashScreenClass::~SplashScreenClass (void)
{
	return ;
}


BEGIN_MESSAGE_MAP(SplashScreenClass, CWnd)
	//{{AFX_MSG_MAP(SplashScreenClass)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
//
//	OnCreate
//
///////////////////////////////////////////////////////////////////////
int
SplashScreenClass::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//
	//	Load the splash background from the resource section
	//
	m_hBitmap = (HBITMAP)::LoadImage (	::AfxGetResourceHandle (),
													MAKEINTRESOURCE (IDB_SPLASH),
													IMAGE_BITMAP,
													0,
													0,
													LR_CREATEDIBSECTION);
	
	//
	//	Get the dimensions of the BMP
	//
	BITMAP bmp_info = { 0 };
	if (::GetObject (m_hBitmap, sizeof (BITMAP), &bmp_info) != 0) {
		
		//
		//	Resize the window so the BMP compeletely fills the window
		//
		m_Size.cx = bmp_info.bmWidth;
		m_Size.cy = bmp_info.bmHeight;		
		SetWindowPos (	NULL,
							(::GetSystemMetrics (SM_CXSCREEN) >> 1) - (m_Size.cx >> 1),
							(::GetSystemMetrics (SM_CYSCREEN) >> 1) - (m_Size.cy >> 1),
							m_Size.cx,
							m_Size.cy,
							SWP_NOZORDER);

		//
		//	Create a memory DC (for drawing) and the font object we will use
		//
		m_hMemDC = ::CreateCompatibleDC (NULL);
		m_hFont = ::CreateFont (-::MulDiv (7, GetDeviceCaps(m_hMemDC, LOGPIXELSY), 72),
										0,
										0,
										0,
										FW_REGULAR,
										FALSE,
										FALSE,
										FALSE,
										ANSI_CHARSET,
										OUT_DEFAULT_PRECIS,
										CLIP_DEFAULT_PRECIS,
										DEFAULT_QUALITY,
										DEFAULT_PITCH,
										"Small Fonts");
	}

	SetForegroundWindow ();
	return 0;
}


///////////////////////////////////////////////////////////////////////
//
//	OnPaint
//
///////////////////////////////////////////////////////////////////////
void
SplashScreenClass::OnPaint (void)
{
	CPaintDC dc (this);

	if (m_hMemDC != NULL && m_hBitmap != NULL) {
		
		//
		//	Paint the BMP into the window
		//
		HBITMAP old_bmp = (HBITMAP)::SelectObject (m_hMemDC, m_hBitmap);
		::BitBlt ((HDC)dc, 0, 0, m_Size.cx, m_Size.cy, m_hMemDC, 0, 0, SRCCOPY);
		::SelectObject (m_hMemDC, old_bmp);

		//
		//	Update the status text in the window
		//
		Paint_Status_Text (dc);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Status_Text
//
///////////////////////////////////////////////////////////////////////
void
SplashScreenClass::Set_Status_Text (LPCTSTR text)
{
	m_StatusText = text;

	//
	//	Repaint the status text area of the window
	//
	HDC hdc = ::GetDC (m_hWnd);
	Paint_Status_Text (hdc);
	::ReleaseDC (m_hWnd, hdc);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Paint_Status_Text
//
///////////////////////////////////////////////////////////////////////
void
SplashScreenClass::Paint_Status_Text (HDC hdc) 
{
	if (m_StatusText.GetLength () > 0) {
		
		//
		//	Select the correct font, pen, and brush into the DC
		//
		HFONT old_font		= (HFONT)::SelectObject (hdc, m_hFont);
		HPEN old_pen		= (HPEN)::SelectObject (hdc, ::GetStockObject (WHITE_PEN));
		HBRUSH old_brush	= (HBRUSH)::SelectObject (hdc, ::GetStockObject (BLACK_BRUSH));
		::SetTextColor (hdc, RGB (255, 255, 255));
		::SetBkColor (hdc, RGB (128, 128, 128));

		//
		//	Draw the status text in its correct position
		//
		RECT rect;
		rect.left	= 15;
		rect.right	= m_Size.cx - 15;
		rect.top		= 224;
		rect.bottom	= 238;
		//::DrawText (hdc, m_StatusText, m_StatusText.GetLength (), &rect, DT_LEFT | DT_BOTTOM);
		::ExtTextOut (hdc, 15, 225, ETO_OPAQUE, &rect, m_StatusText, m_StatusText.GetLength (), NULL);


		// Restore the original settings
		::SelectObject (hdc, old_brush);
		::SelectObject (hdc, old_pen);
		::SelectObject (hdc, old_font);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	OnDestroy
//
///////////////////////////////////////////////////////////////////////
void
SplashScreenClass::OnDestroy (void) 
{
	if (m_hMemDC != NULL) {
		::DeleteDC (m_hMemDC);
		m_hMemDC = NULL;
	}

	if (m_hBitmap != NULL) {
		::DeleteObject (m_hBitmap);
		m_hBitmap = NULL;
	}

	if (m_hFont != NULL) {
		::DeleteObject (m_hFont);
		m_hFont = NULL;
	}

	CWnd::OnDestroy ();

	//
	// Its assumed this window is operating on a separate thread...
	//
	::PostQuitMessage (0);
	return ;	
}


///////////////////////////////////////////////////////////////////////
//
//	WindowProc
//
///////////////////////////////////////////////////////////////////////
LRESULT
SplashScreenClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == WM_USER+102) {
		Set_Status_Text ((LPCTSTR)lParam);
	} else if (message == WM_USER+101) {
		DestroyWindow ();
		return 1L;
	}

	return CWnd::WindowProc (message, wParam, lParam);
}


///////////////////////////////////////////////////////////////////////
//
//	PostNcDestroy
//
///////////////////////////////////////////////////////////////////////
void
SplashScreenClass::PostNcDestroy (void)
{
	CWnd::PostNcDestroy ();

	//	Hehe...  :)
	delete this;
	return ;
}
