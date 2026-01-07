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

// PickerClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "picker.h"
#include "Utils.h"
#include "filemgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////
//	Local constants
///////////////////////////////////////////////////////////
const int BUTTON_WIDTH		= 22;
const int BUTTON_ID			= 101;
const int EDIT_ID				= 102;


///////////////////////////////////////////////////////////
//
//	PickerClass
//
///////////////////////////////////////////////////////////
PickerClass::PickerClass (void)
	:	m_BrowseButton (NULL),
		m_Icon (NULL),
		CStatic ()
{
	//
	//	By default load the folder icon
	//
	m_Icon = (HICON)::LoadImage (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_FOLDER_TINY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	return ;
}


///////////////////////////////////////////////////////////
//
//	~PickerClass
//
///////////////////////////////////////////////////////////
PickerClass::~PickerClass (void)
{
	return ;
}


BEGIN_MESSAGE_MAP(PickerClass, CStatic)
	//{{AFX_MSG_MAP(PickerClass)
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////
//
//	Create_Picker
//
///////////////////////////////////////////////////////////
bool
PickerClass::Create_Picker
(
	DWORD				style,
	const RECT &	rect,
	CWnd *			parent,
	UINT				id
)
{
	//
	//	Create the outer window
	//
	bool success = CreateEx (	WS_EX_CLIENTEDGE,
									  "STATIC",
									  "",
									  (style | SS_WHITERECT | WS_CLIPCHILDREN) & (~WS_BORDER),
									  rect,
									  parent,
									  id) != 0;
	if (success) {
		Initialize_Control ();
	}

	// Return the true/false result code
	return success;
}


///////////////////////////////////////////////////////////
//
//	WindowProc
//
///////////////////////////////////////////////////////////
LRESULT
PickerClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	//
	// Should we just pass this onto the edit control?
	//
	if ((message == WM_GETTEXT) ||
		 (message == WM_GETTEXTLENGTH) ||
		 (message == WM_SETTEXT))
	{
		return SendDlgItemMessage (EDIT_ID, message, wParam, lParam);

	} else if ((message == WM_COMMAND) && (LOWORD (wParam) == EDIT_ID)) {

		//
		//	Translate the message so the dialog thinks it came from an
		// edit control
		//
		LONG id = ::GetWindowLong (m_hWnd, GWL_ID); 
		return ::SendMessage (::GetParent (m_hWnd),
									 message,
									 MAKEWPARAM (id & 0xFFFF, HIWORD (wParam)),
									 (LPARAM)m_hWnd);

	} else if (message == WM_SETFOCUS) {
		::SetFocus (::GetDlgItem (m_hWnd, EDIT_ID));
	}

	return CStatic::WindowProc (message, wParam, lParam);
}


///////////////////////////////////////////////////////////
//
//	Initialize_Control
//
///////////////////////////////////////////////////////////
void
PickerClass::Initialize_Control (void)
{
	//
	// Set the font for this control
	//
	HFONT hfont = (HFONT)GetParent()->SendMessage (WM_GETFONT);
	SendMessage (WM_SETFONT, (WPARAM)hfont);

	//
	//	Determine how tall to make the edit control
	//
	HDC hdc			= ::GetDC (m_hWnd);
	HFONT holdfont	= (HFONT)::SelectObject (hdc, hfont);
	CSize size;
	::GetTextExtentPoint32 (hdc, "XgW", 3, &size);
	::SelectObject (hdc, holdfont);
	::ReleaseDC (m_hWnd, hdc);
	
	//
	// Create the edit control
	//
	CRect rect;
	GetClientRect (&rect);
	m_EditCtrl = ::CreateWindowEx ( 0,
											  "EDIT",
											  "",
											  WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
											  2,
											  (rect.Height () >> 1) - ((size.cy) >> 1),
											  rect.Width () - BUTTON_WIDTH - 3,
											  size.cy,
											  m_hWnd,
											  (HMENU)(DWORD_PTR)EDIT_ID,
											  ::AfxGetInstanceHandle (),
											  NULL);	
		CRect rect2;
		GetWindowRect (&rect2);


	::SendMessage (m_EditCtrl, WM_SETFONT, (WPARAM)hfont, 0L);
	
	//
	// Create the picker button
	//
	m_BrowseButton = ::CreateWindow ("BUTTON",
												 "",
												 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
												 rect.Width () - BUTTON_WIDTH,
												 0,
												 BUTTON_WIDTH,
												 rect.Height (),
												 m_hWnd,
												 (HMENU)(DWORD_PTR)BUTTON_ID,
												 ::AfxGetInstanceHandle (),
												 NULL);
	
	ASSERT (m_BrowseButton != NULL);
	return ;
}


///////////////////////////////////////////////////////////
//
//	Initialize_Control
//
///////////////////////////////////////////////////////////
void
PickerClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
)
{

	//
	// Set the font for this control
	//
	HFONT hfont = (HFONT)GetParent()->SendMessage (WM_GETFONT);
	SendMessage (WM_SETFONT, (WPARAM)hfont);

	//
	//	Determine how tall to make the edit control
	//
	HDC hdc			= ::GetDC (m_hWnd);
	HFONT holdfont	= (HFONT)::SelectObject (hdc, hfont);
	CSize size;
	::GetTextExtentPoint32 (hdc, "XgW", 3, &size);
	::SelectObject (hdc, holdfont);
	::ReleaseDC (m_hWnd, hdc);

	CRect client_rect;
	GetClientRect (&client_rect);
	
	//
	// Resize the edit control
	//
	::SetWindowPos (	m_EditCtrl,
							NULL, 
							2,
							(client_rect.Height () >> 1) - ((size.cy) >> 1),
							client_rect.Width () - BUTTON_WIDTH - 3,
							size.cy,
							SWP_NOZORDER);

	//
	// Reposition the file button
	//
	::SetWindowPos (	m_BrowseButton, 
							NULL,
							client_rect.Width () - BUTTON_WIDTH,
							0,
							BUTTON_WIDTH,
							client_rect.Height (),
							SWP_NOZORDER);

	CStatic::OnSize (nType, cx, cy);	
	return ;	
}


///////////////////////////////////////////////////////////
//
//	OnCommand
//
///////////////////////////////////////////////////////////
BOOL
PickerClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	//
	// Did the user click the 'picker' button?
	//
	if ((LOWORD (wParam) == BUTTON_ID) &&
	    (HIWORD (wParam) == BN_CLICKED))
	{
		On_Pick ();
	}

	// Allow the base class to process this message
	return CStatic::OnCommand (wParam, lParam);
}


///////////////////////////////////////////////////////////
//
//	OnDrawItem
//
///////////////////////////////////////////////////////////
void
PickerClass::OnDrawItem
(
	int nIDCtl,
	LPDRAWITEMSTRUCT pDrawItemStruct
) 
{	
	CRect rect;
	::GetClientRect (m_BrowseButton, &rect);

	//
	// Draw the button frame
	//
	UINT type = (pDrawItemStruct->itemState & ODS_SELECTED) ? (DFCS_SCROLLDOWN | DFCS_PUSHED) : DFCS_SCROLLDOWN;
	::DrawFrameControl (pDrawItemStruct->hDC, &rect, DFC_SCROLL, type);

	//
	//	Calculate the icon's position
	//
	int xpos = (rect.Width () / 2) - 8;
	int ypos = (rect.Height () / 2) - 8;

	if (pDrawItemStruct->itemState & ODS_SELECTED) {
		xpos ++;
		ypos ++;
	}

	//
	// Paint the icon
	//
	::DrawIconEx (pDrawItemStruct->hDC,
					  xpos,
					  ypos,
					  m_Icon,
					  16,
					  16,
					  0,
					  NULL,
					  DI_NORMAL);

	CStatic::OnDrawItem (nIDCtl, pDrawItemStruct);
	return ;
}


///////////////////////////////////////////////////////////
//
//	Set_Read_Only
//
///////////////////////////////////////////////////////////
void
PickerClass::Set_Read_Only (bool readonly)
{
	::SendMessage (m_EditCtrl, EM_SETREADONLY, (WPARAM)readonly, 0L);
	return ;
}


///////////////////////////////////////////////////////////
//
//	OnEraseBkgnd
//
///////////////////////////////////////////////////////////
BOOL
PickerClass::OnEraseBkgnd (CDC *pDC)
{
	CRect rect;
	::GetClientRect (m_hWnd, &rect);

	//
	// Use the 'grayed' brush if this control is read-only
	//
	HBRUSH brush = HBRUSH(COLOR_WINDOW+1);
	if (::GetWindowLong (m_EditCtrl, GWL_STYLE) & ES_READONLY) {
		brush = HBRUSH(COLOR_3DFACE+1);
	}

	//
	//	Erase the background
	//
	::FillRect (*pDC, &rect, brush);
	return true;
}


///////////////////////////////////////////////////////////
//
//	OnPaint
//
///////////////////////////////////////////////////////////
void
PickerClass::OnPaint (void)
{
	//
	//	Force the child windows to be repainted
	//
	::InvalidateRect (m_EditCtrl, NULL, true);
	::InvalidateRect (m_BrowseButton, NULL, true);

	CPaintDC dc (this);
	return ;
}

