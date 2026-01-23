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

// SpecSheet.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "specsheet.h"
#include "definition.h"
#include "parameterctrls.h"
#include "wwstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////////
//	Local prototypes
//////////////////////////////////////////////////////////////////////////////
int __cdecl fnCtrlCompareCallback (void const *ptr1, void const *ptr2);
int Rate_Param_Type (ParameterClass::Type type);


/////////////////////////////////////////////////////////////////////////////
//
// SpecSheetClass
//
/////////////////////////////////////////////////////////////////////////////
SpecSheetClass::SpecSheetClass (DefinitionClass *definition)
	:	m_IsScrolling (false),
		m_ScrollPos (0),
		m_MaxScrollPos (0),
		m_IsTemp (false),
		m_IsReadOnly (false),
		m_ShowFileOnly (false),
		m_WereFilesChanged (false),
		m_AssetTreeOnly (true),		
		m_Definition (definition)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~SpecSheetClass
//
/////////////////////////////////////////////////////////////////////////////
SpecSheetClass::~SpecSheetClass (void)
{
	int index;
	for (index = 0; index < m_CtrlList.Count (); index ++) {
		ParameterCtrlClass *ctrl = m_CtrlList[index];		
		delete ctrl;
	}

	//
	//	Unlock all the parameters in this definition
	//
	int count = m_Definition->Get_Parameter_Count ();
	for (index = 0; index < count; index ++) {
		m_Definition->Unlock_Parameter (index);
	}

	m_CtrlList.Delete_All ();
	return ;
}


BEGIN_MESSAGE_MAP(SpecSheetClass, CWnd)
	//{{AFX_MSG_MAP(SpecSheetClass)
	ON_WM_CREATE()
	ON_WM_NCHITTEST()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// Is_Filtered
//
/////////////////////////////////////////////////////////////////////////////
bool
SpecSheetClass::Is_Filtered (ParameterClass *parameter)
{
	bool retval = false;

	if (parameter != NULL) {
		if (m_ShowFileOnly) {
			
			int type = parameter->Get_Type ();
			if (	type != ParameterClass::TYPE_FILENAME &&
					type != ParameterClass::TYPE_SOUND_FILENAME )
			{
				retval = true;
			}
		}
	}

	return retval;
}

/////////////////////////////////////////////////////////////////////////////
//
// Add_Parameter
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::Add_Parameter (ParameterClass *parameter)
{
	//
	//	What type of parameter is this?
	//
	ParameterCtrlClass *parameter_ctrl = NULL;
	switch (parameter->Get_Type ()) {
		
		case ParameterClass::TYPE_INT:
			parameter_ctrl = new IntParameterCtrlClass ((IntParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_FLOAT:
			parameter_ctrl = new FloatParameterCtrlClass ((FloatParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_FILENAME:
			parameter_ctrl = new FileParameterCtrlClass ((FilenameParameterClass *)parameter);
			((FileParameterCtrlClass *)parameter_ctrl)->Get_File_Picker ().Set_Asset_Tree_Only (m_AssetTreeOnly);
			break;

		case ParameterClass::TYPE_SOUND_FILENAME:
			parameter_ctrl = new SoundFileParameterCtrlClass ((SoundFilenameParameterClass *)parameter);
			((FileParameterCtrlClass *)parameter_ctrl)->Get_File_Picker ().Set_Asset_Tree_Only (m_AssetTreeOnly);
			break;

		case ParameterClass::TYPE_STRING:
			parameter_ctrl = new StringParameterCtrlClass ((StringParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_BOOL:
			parameter_ctrl = new BoolParameterCtrlClass ((BoolParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_VECTOR3:
			parameter_ctrl = new Vector3ParameterCtrlClass ((Vector3ParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_VECTOR2:
			parameter_ctrl = new Vector2ParameterCtrlClass ((Vector2ParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_RECT:
			parameter_ctrl = new RectParameterCtrlClass ((RectParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_COLOR:
			parameter_ctrl = new ColorParameterCtrlClass ((ColorParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_ENUM:
			parameter_ctrl = new EnumParameterCtrlClass ((EnumParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_PHYSDEFINITIONID:
			parameter_ctrl = new PhysDefParameterCtrlClass ((PhysDefParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_GENERICDEFINITIONID:
			parameter_ctrl = new GenericDefParameterCtrlClass ((GenericDefParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_GAMEOBJDEFINITIONID:
			parameter_ctrl = new GameObjDefParameterCtrlClass ((GameObjDefParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_WEAPONOBJDEFINITIONID:
			parameter_ctrl = new WeaponObjDefParameterCtrlClass ((WeaponObjDefParameterClass *)parameter);
			break;			

		case ParameterClass::TYPE_AMMOOBJDEFINITIONID:
			parameter_ctrl = new AmmoObjDefParameterCtrlClass ((AmmoObjDefParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_EXPLOSIONDEFINITIONID:
			parameter_ctrl = new ExplosionObjDefParameterCtrlClass ((ExplosionObjDefParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_SOUNDDEFINITIONID:
			parameter_ctrl = new SoundDefParameterCtrlClass ((SoundDefParameterClass *)parameter);
			break;			

		case ParameterClass::TYPE_SCRIPT:
			parameter_ctrl = new ScriptParameterCtrlClass ((ScriptParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_ANGLE:
			parameter_ctrl = new AngleParameterCtrlClass ((AngleParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_DEFINITIONIDLIST:
			parameter_ctrl = new DefIDListParameterCtrlClass ((DefIDListParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_FILENAMELIST:
			parameter_ctrl = new FilenameListParameterCtrlClass ((FilenameListParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_SEPARATOR:
			parameter_ctrl = new SeparatorParameterCtrlClass ((SeparatorParameterClass *)parameter);
			break;

		case ParameterClass::TYPE_STRINGSDB_ID:
			parameter_ctrl = new StringsDBEntryParameterCtrlClass ((StringsDBEntryParameterClass *)parameter);
			break;
	}

	if (parameter_ctrl != NULL) {
		parameter_ctrl->Set_Is_Temp (m_IsTemp);
		parameter_ctrl->Set_Spec_Sheet (this);
		m_CtrlList.Add (parameter_ctrl);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCreate
//
/////////////////////////////////////////////////////////////////////////////
int
SpecSheetClass::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//
	// Set the font for this control
	//
	HFONT hfont = (HFONT)GetParent()->SendMessage (WM_GETFONT);
	SendMessage (WM_SETFONT, (WPARAM)hfont);

	CRect client_rect;
	GetClientRect (&client_rect);

	//
	//	Create the scrollbar
	//
	HWND scrollbar = ::CreateWindow (	"SCROLLBAR",
													"",
													WS_CHILD | WS_VISIBLE | SBS_VERT,
													client_rect.right - ::GetSystemMetrics (SM_CXVSCROLL),
													0,
													::GetSystemMetrics (SM_CXVSCROLL),
													client_rect.Height (),
													m_hWnd,
													(HMENU)878,
													::AfxGetInstanceHandle (),
													NULL);

	//
	//	Configure the scrollbar
	//
	::SendMessage (scrollbar, SBM_SETRANGE, (WPARAM)0, (WPARAM)100);
	::SendMessage (scrollbar, WM_SETFONT, (WPARAM)hfont, 0L);

	//
	//	Create a control object for every parameter
	//
	int count = m_Definition->Get_Parameter_Count ();
	int index;
	for (index = 0; index < count; index ++) {
		ParameterClass *parameter = m_Definition->Lock_Parameter (index);

		//
		//	Add this control to the dialog if its not filtered...
		//
		if (Is_Filtered (parameter) == false) {
			Add_Parameter (parameter);
		}
	}

	int start_id = 500;

	CRect rect;
	GetClientRect (rect);
	rect.top		+= 10;
	rect.bottom = rect.top;

	//
	//	Ask each control to create its UI
	//
	for (index = 0; index < m_CtrlList.Count (); index ++) {
		ParameterCtrlClass *parameter_ctrl = m_CtrlList[index];
		
		parameter_ctrl->Set_Read_Only (m_IsReadOnly);

		start_id		= parameter_ctrl->Create (m_hWnd, start_id, &rect);
		rect.top		= rect.bottom + 10;
		rect.bottom	= rect.top;		
	}

	m_MaxScrollPos = rect.bottom;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Current_Filename_Value
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::Get_Current_Filename_Value (int index, CString &value)
{
	//
	//	Is the index valid?
	//
	ASSERT (index >= 0 && index < Get_Parameter_Count ());
	if (index >= 0 && index < Get_Parameter_Count ()) {
		
		//
		//	Return the parameter that is associated with this ctrl
		//
		ParameterCtrlClass *ctrl = m_CtrlList[index];
		if (	ctrl->Get_Type () == ParameterClass::TYPE_FILENAME || 
				ctrl->Get_Type () == ParameterClass::TYPE_SOUND_FILENAME )
		{
			((FileParameterCtrlClass *)ctrl)->Get_Current_Value (value);
		}		
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Parameter
//
/////////////////////////////////////////////////////////////////////////////
ParameterClass *
SpecSheetClass::Get_Parameter (int index)
{
	ParameterClass *parameter = NULL;

	//
	//	Is the index valid?
	//
	ASSERT (index >= 0 && index < Get_Parameter_Count ());
	if (index >= 0 && index < Get_Parameter_Count ()) {
		
		//
		//	Return the parameter that is associated with this ctrl
		//
		ParameterCtrlClass *ctrl = m_CtrlList[index];
		parameter = ctrl->Get_Parameter ();
	}

	return parameter;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNcHitTest
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
SpecSheetClass::OnNcHitTest (CPoint /* point */) 
{
	return HTCLIENT;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnVScroll
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnVScroll
(
	UINT			nSBCode,
	UINT			nPos,
	CScrollBar *pScrollBar
)
{
	if (pScrollBar != NULL && GetWindowLong (*pScrollBar, GWL_ID) == 878) {
		int new_pos = m_ScrollPos;

		CRect client_rect;
		GetClientRect (&client_rect);
		int height = client_rect.Height ();

		switch (nSBCode)
		{
			case SB_BOTTOM:
				new_pos = m_MaxScrollPos;
				break;

			case SB_LINEDOWN:			
				if (new_pos < (m_MaxScrollPos - 10)) {
					new_pos += 10;
				} else {
					new_pos = m_MaxScrollPos;
				}
				break;

			case SB_LINEUP:
				if (new_pos > 10) {
					new_pos -= 10;
				} else {
					new_pos = 0;
				}
				break;

			case SB_PAGEDOWN:
				if (new_pos < (m_MaxScrollPos - (height / 2))) {
					new_pos += (height / 2);
				} else {
					new_pos = m_MaxScrollPos;
				}
				break;

			case SB_PAGEUP:
				if (new_pos > (height / 2)) {
					new_pos -= (height / 2);
				} else {
					new_pos = 0;
				}
				break;

			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:
				new_pos = (m_MaxScrollPos * nPos) / 100;
				break;
		}

		//
		//	Scroll to the new position
		//
		Set_Scroll_Pos (new_pos);
	}

	CWnd::OnVScroll (nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Scroll_Pos
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::Set_Scroll_Pos (int new_pos)
{
	//
	//	Scroll the controls
	//
	Scroll_Controls (m_ScrollPos - new_pos);		
	m_ScrollPos = new_pos;
	

	//
	//	Update the scroll bar
	//
	int sb_pos = ((m_ScrollPos * 100) / m_MaxScrollPos);
	SendDlgItemMessage (878, SBM_SETPOS, (WPARAM)sb_pos, (LPARAM)true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSetCursor
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SpecSheetClass::OnSetCursor
(
	CWnd *	pWnd,
	UINT		nHitTest,
	UINT		message
)
{
	BOOL retval = false;

	if (nHitTest == HTCLIENT) {
		::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_OBJ_MOVE)));
		//retval = true;
	} else {
		retval = CWnd::OnSetCursor (pWnd, nHitTest, message);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNcMouseMove
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnNcMouseMove
(
	UINT		nHitTest,
	CPoint	point
) 
{	
	CWnd::OnNcMouseMove (nHitTest, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNcLButtonDown
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnNcLButtonDown
(
	UINT		nHitTest,
	CPoint	point
)
{
	/*if (nHitTest == HTVSCROLL) {
		m_IsScrolling = true;
		m_LastPoint = point;
		SetCapture ();
	}*/

	CWnd::OnNcLButtonDown (nHitTest, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNcLButtonDown
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnNcLButtonUp
(
	UINT		nHitTest,
	CPoint	point
)
{	
	CWnd::OnNcLButtonUp (nHitTest, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonDown
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnLButtonDown (UINT nFlags, CPoint point)
{
	//if (nHitTest == HTVSCROLL) {
		m_IsScrolling = true;
		m_LastPoint = point;
		SetCapture ();
	//}

	CWnd::OnLButtonDown(nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonUp
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnLButtonUp (UINT nFlags, CPoint point) 
{
	m_IsScrolling = false;
	ReleaseCapture ();
	
	CWnd::OnLButtonUp (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnMouseMove
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (m_IsScrolling) {

		CPoint delta		= m_LastPoint - point;
		int scroll_amt		= delta.y;		
		int new_pos			= m_ScrollPos + scroll_amt;			
		
		//
		//	Clip the scroll position to the min and max
		//
		if (new_pos < 0) {
			new_pos = 0;
		} else if (new_pos > m_MaxScrollPos) {
			new_pos = m_MaxScrollPos;
		}
		
		//
		//	Scroll to the new position
		//
		Set_Scroll_Pos (new_pos);

		//
		//	Remember the last point
		//
		m_LastPoint = point;
	}
	
	CWnd::OnMouseMove (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Scroll_Controls
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::Scroll_Controls (int amount)
{
	DynamicVectorClass<HWND> child_wnd_list;

	//
	//	Build a list of all the child windows we want to scroll
	//
	HWND child_wnd;
	for (	child_wnd = ::GetWindow (m_hWnd, GW_CHILD);
			child_wnd != NULL;
			child_wnd = ::GetWindow (child_wnd, GW_HWNDNEXT))
	{
		//
		//	If this isn't the scroll-bar, then add it to the list
		//
		if (::GetWindowLong (child_wnd, GWL_ID) != 878) {
			child_wnd_list.Add (child_wnd);
		}
	}

	//
	//	Build a defer struct so that we can move all the windows in one
	// operation
	//
	int count = child_wnd_list.Count ();
	HDWP defer_struct = ::BeginDeferWindowPos (count);

	for (int index = 0; index < count; index ++) {		
		child_wnd = child_wnd_list[index];
		
		//
		//	Get the current position of the child window
		//
		CRect rect;
		::GetWindowRect (child_wnd, &rect);
		ScreenToClient (&rect);

		//
		//	Scroll the window
		//
		::DeferWindowPos (defer_struct, child_wnd, NULL, rect.left, rect.top + amount, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
	}

	//
	//	Perform the move operations in batch
	//
	::EndDeferWindowPos (defer_struct);

	CRect client_rect;
	GetClientRect (&client_rect);
	client_rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	
	HDC dc = ::GetDC (m_hWnd);
	::ScrollDC (dc, 0, amount, &client_rect, &client_rect, NULL, NULL);
	::ReleaseDC (m_hWnd, dc);

	
	if (amount > 0) {		
		client_rect.bottom = client_rect.top + amount;
	} else {
		client_rect.top = client_rect.bottom + amount;
	}

	InvalidateRect (client_rect, true);
	UpdateWindow ();
	
	/*CRect client_rect;
	GetClientRect (&client_rect);
	client_rect.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	InvalidateRect (client_rect, true);
	UpdateWindow ();*/
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnSize
(
	UINT	nType,
	int	cx,
	int	cy
)
{
	
	CRect client_rect;
	GetClientRect (&client_rect);

	//
	//	Resize the scrollbar
	//
	HWND scrollbar = ::GetDlgItem (m_hWnd, 878);
	::SetWindowPos (	scrollbar,
							NULL,
							client_rect.right - ::GetSystemMetrics (SM_CXVSCROLL),
							0,
							::GetSystemMetrics (SM_CXVSCROLL),
							client_rect.Height (),
							SWP_NOZORDER);

	//
	//	Resize the controls
	//
	client_rect.right -= (::GetSystemMetrics (SM_CXVSCROLL) + 10);
	for (int index = 0; index < m_CtrlList.Count (); index ++) {
		ParameterCtrlClass *ctrl = m_CtrlList[index];
		ctrl->Resize (client_rect);
	}


	CWnd::OnSize (nType, cx, cy);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// PreCreateWindow
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SpecSheetClass::PreCreateWindow (CREATESTRUCT &cs) 
{
	cs.dwExStyle |= WS_EX_CONTROLPARENT;
	return CWnd::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::Apply (void) 
{
	m_WereFilesChanged = false;

	for (int index = 0; index < m_CtrlList.Count (); index ++) {
		ParameterCtrlClass *ctrl = m_CtrlList[index];

		//
		//	Determine if any file parameters were changed...
		//
		int type = ctrl->Get_Type ();
		if (	type == ParameterClass::TYPE_FILENAME ||
				type == ParameterClass::TYPE_SOUND_FILENAME )
		{
			m_WereFilesChanged |= ctrl->Is_Modified ();
		}

		//
		//	Get the control to read its data from the dialog controls
		//
		ctrl->Read_Data (NULL);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Rate_Param_Type
//
/////////////////////////////////////////////////////////////////////////////
int
Rate_Param_Type (ParameterClass::Type type)
{
	int retval = 100;

	switch (type)
	{
		case ParameterClass::TYPE_FILENAME:
		case ParameterClass::TYPE_SOUND_FILENAME:
			retval = 0;
			break;		

		case ParameterClass::TYPE_PHYSDEFINITIONID:
			retval = 1;
			break;

		case ParameterClass::TYPE_GAMEOBJDEFINITIONID:
		case ParameterClass::TYPE_WEAPONOBJDEFINITIONID:
		case ParameterClass::TYPE_AMMOOBJDEFINITIONID:
		case ParameterClass::TYPE_SOUNDDEFINITIONID:
			retval = 2;
			break;

		case ParameterClass::TYPE_SCRIPT:
			retval = 3;
			break;

		case ParameterClass::TYPE_STRING:
			retval = 4;
			break;

		case ParameterClass::TYPE_ENUM:
			retval = 5;
			break;
		
		case ParameterClass::TYPE_COLOR:
			retval = 8;
			break;

		case ParameterClass::TYPE_VECTOR3:
			retval = 9;
			break;

		case ParameterClass::TYPE_INT:
			retval = 10;
			break;

		case ParameterClass::TYPE_ANGLE:
			retval = 11;
			break;

		case ParameterClass::TYPE_FLOAT:
			retval = 12;
			break;

		case ParameterClass::TYPE_BOOL:
			retval = 20;
			break;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnCtrlCompareCallback
//
/////////////////////////////////////////////////////////////////////////////
int __cdecl
fnCtrlCompareCallback (void const *ptr1, void const *ptr2)
{
	ParameterClass::Type type1 = (*((ParameterCtrlClass **)ptr1))->Get_Type ();
	ParameterClass::Type type2 = (*((ParameterCtrlClass **)ptr2))->Get_Type ();

	//
	//	Determine a sort rating based on the param type
	//
	int rating1 = ::Rate_Param_Type (type1);
	int rating2 = ::Rate_Param_Type (type2);

	int retval = rating1 - rating2;

	//
	//	If the type is the same, then sort alphanumerically
	//
	if (retval == 0) {
		ParameterClass *param1 = (*((ParameterCtrlClass **)ptr1))->Get_Parameter ();
		ParameterClass *param2 = (*((ParameterCtrlClass **)ptr2))->Get_Parameter ();

		//
		//	Compare the param names
		//
		CString name1 = param1->Get_Name ();
		CString name2 = param2->Get_Name ();
		retval = name1.CompareNoCase (name2);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SpecSheetClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
) 
{
	bool processed = false;
	for (int index = 0; (index < m_CtrlList.Count ()) && !processed; index ++) {
		ParameterCtrlClass *ctrl = m_CtrlList[index];		
		processed = ctrl->On_Command (*this, wParam, lParam);
	}

	return CWnd::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
SpecSheetClass::OnDestroy (void)
{
	for (int index = 0; index < m_CtrlList.Count (); index ++) {
		ParameterCtrlClass *ctrl = m_CtrlList[index];		
		ctrl->On_Destroy ();
	}
	
	CWnd::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
SpecSheetClass::WindowProc (UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT result = 0L;

	if (message == WM_DRAWITEM) {
		
		//
		//	Determine which parameter ctrl wants to process this message...
		//
		for (int index = 0; (index < m_CtrlList.Count ()) && (result == 0L); index ++) {
			ParameterCtrlClass *ctrl = m_CtrlList[index];		
			if (ctrl->On_DrawItem (*this, wParam, lParam)) {
				result = 1L;
			}
		}
	}

	if (result == 0L) {
		result = CWnd::WindowProc(message, wParam, lParam);
	}

	return result;
}
