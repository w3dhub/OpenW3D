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

// ZoneEditDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "LevelEditView.h"
#include "ZoneEditDialog.h"
#include "ww3d.h"
#include "scene.h"
#include "camera.h"
#include "Utils.h"
#include "mmsystem.h"
#include "matrix3d.h"
#include "rendobj.h"
#include "sphere.h"
#include "box3d.h"
#include "cameramgr.h"
#include "filemgr.h"
#include "filelocations.h"
#include "quat.h"
#include "vector3.h"
#include "obbox.h"
#include "staticanimphys.h"
#include "hanim.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// Local constants
//
/////////////////////////////////////////////////////////////////////////////
const int TOOLBAR_HEIGHT		= 36;
const int TOOLBAR_V_SPACING	= 5;
const int TOOLBAR_V_BORDER		= TOOLBAR_V_SPACING * 2;
const int TOOLBAR_H_SPACING	= 5;
const int TOOLBAR_H_BORDER		= TOOLBAR_H_SPACING * 2;


/////////////////////////////////////////////////////////////////////////////
//
// Local prototypes
//
/////////////////////////////////////////////////////////////////////////////
static void Trackball_Camera (HWND hwnd, CameraClass &camera, const Vector3 &center, POINT point, POINT last_point);


/////////////////////////////////////////////////////////////////////////////
//
// ZoneEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ZoneEditDialogClass::ZoneEditDialogClass(CWnd* pParent /*=NULL*/)
	:	m_LookAtDist (0),
		m_Camera (NULL),
		m_Scene (NULL),
		m_RenderObj (NULL),
		m_PhysObj (NULL),
		m_Zone (NULL),
		m_TimerID (0),
		m_IsEditingZone (true),
		m_IsSizingZone (false),
		m_SwapChain (NULL),
		m_Initialized (false),
		CDialog(ZoneEditDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ZoneEditDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ZoneEditDialogClass)
	DDX_Control(pDX, IDC_EDIT_ZONE, m_MoveZoneCheck);
	DDX_Control(pDX, IDC_SIZE_ZONE, m_SizeZoneCheck);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ZoneEditDialogClass, CDialog)
	//{{AFX_MSG_MAP(ZoneEditDialogClass)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_TOP, OnTop)
	ON_BN_CLICKED(IDC_FRONT, OnFront)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_EDIT_ZONE, OnEditZone)
	ON_BN_CLICKED(IDC_SIZE_ZONE, OnSizeZone)
	ON_BN_CLICKED(IDC_LAST_FRAME, OnLastFrame)
	ON_BN_CLICKED(IDC_FIRST_FRAME, OnFirstFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ZoneEditDialogClass::OnInitDialog (void)
{
	CWaitCursor wait_cursor;

	CLevelEditView::Allow_Repaint (false);
	CDialog::OnInitDialog ();

	//
	//	Subclass the 3D window for mouse-tracking
	//
	SetWindowLongPtr (::GetDlgItem (m_hWnd, IDC_3D_WINDOW), GWLP_WNDPROC, (LONG_PTR)fn3DWindow);
	::SetProp (::GetDlgItem (m_hWnd, IDC_3D_WINDOW), "ZONE_DIALOG", (HANDLE)this);

	//
	//	Get the dimensions of the client area of the window we'll be rendering in
	//
	CRect rect;
	::GetClientRect (::GetDlgItem (m_hWnd, IDC_3D_WINDOW), &rect);
	int cx = rect.Width ();
	int cy = rect.Height ();

	//
	//	Create a swap chain so we can render to this window as well
	//
	m_SwapChain = DX8Wrapper::Create_Additional_Swap_Chain (::GetDlgItem (m_hWnd, IDC_3D_WINDOW));

	//
	//	Create the scene and camera we will use
	//
	m_Scene	= new SimpleSceneClass;
	m_Camera	= new CameraClass;	
	m_Scene->Set_Ambient_Light (Vector3 (1, 1, 1));
	m_Camera->Set_Clip_Planes (0.1F, 400.0F);
	
	//
	//	Configure the camera
	//
	double hfov = 0;
	double vfov = 0;
	if (cy > cx) {
		vfov = (float)DEG_TO_RAD(45.0f);
		hfov = (double)cx / (double)cy * vfov;
	} else {
		hfov = (float)DEG_TO_RAD(45.0f);
		vfov = (double)cy / (double)cx * hfov;
	}
	m_Camera->Set_View_Plane (hfov, vfov);

	//
	//	Setup the scene
	//
	Load_Object ();
	Insert_Zone ();

	//
	//	Select the default UI
	//
	SendDlgItemMessage (IDC_TOP, BM_SETCHECK, (WPARAM)true);
	SendDlgItemMessage (IDC_EDIT_ZONE, BM_SETCHECK, (WPARAM)true);	
	OnTop ();
	OnEditZone ();
	Update_Status ();

	//
	//	Enable/disable the animation controls
	//
	bool has_animation = false;
	if (m_PhysObj != NULL && m_PhysObj->As_StaticAnimPhysClass () != NULL) {
		has_animation = true;
		OnFirstFrame ();
	}
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FIRST_FRAME), has_animation);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LAST_FRAME), has_animation);

	//
	// Kick off a timer that we can use to update
	// the display (kinda like a game loop iterator)
	//
	m_TimerID = ::timeSetEvent (	50,
											50,
											fnUpdateTimer,
											(DWORD_PTR)m_hWnd,
											TIME_PERIODIC);	
		
	m_Initialized = true;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnOK (void)
{
	//
	//	Create a matrix we can use to transform the zone into 'relative' space.
	//
	Matrix3D obj_tm = m_RenderObj->Get_Transform ();
	Matrix3D obj_inv;
	obj_tm.Get_Orthogonal_Inverse (obj_inv);

	//
	//	Transform the zone from world space coords to relative coords
	//
	OBBoxClass zone_box (m_Zone->Get_Transform ().Get_Translation (), m_Zone->Get_Dimensions () * 0.5F);
	OBBoxClass::Transform (obj_inv, zone_box, &m_ZoneBox);

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnDestroy (void)
{
	// Stop the timer
	if (m_TimerID != 0) {		
		::timeKillEvent (m_TimerID);
		m_TimerID = 0;
	}

	//
	//	Free our swap chain
	//
	if (m_SwapChain != NULL) {
		m_SwapChain->Release ();
		m_SwapChain = NULL;
	}

	CLevelEditView::Allow_Repaint (true);

	//
	//	Destroy the scene/camera/render-obj
	//
	m_Scene->Remove_Render_Object (m_RenderObj);
	m_Scene->Remove_Render_Object (m_Zone);
	MEMBER_RELEASE (m_Scene);
	MEMBER_RELEASE (m_Camera);
	MEMBER_RELEASE (m_RenderObj);
	MEMBER_RELEASE (m_Zone);
	MEMBER_RELEASE (m_PhysObj);

	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Render_View
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Render_View (void)
{
	static bool render_lock = false;

	if (!render_lock) {
		render_lock = true;
		
		//
		//	Move the camera if the user is pressing
		// the camera-control keys.
		//
		Handle_Keypress ();

		//
		//	Configure the render target
		//
		DX8Wrapper::Set_Render_Target (m_SwapChain);

		//
		//	Render the scene
		//
		WW3D::Begin_Render (true, true, Vector3 (0.4F, 0.4F, 0.4F));
		WW3D::Render (m_Scene, m_Camera, false, false);
		WW3D::End_Render ();

		//
		//	Blit the frame to the client area of the window
		//
		m_SwapChain->Present (NULL, NULL, NULL, NULL, 0);
		
		//
		//	Restore the render target
		//
		DX8Wrapper::Set_Render_Target ((LPDIRECT3DSURFACE9)NULL);

		//
		//	Cleanup
		//
		RemoveProp (m_hWnd, "WaitingToProcess");
		render_lock = false;		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fnUpdateTimer
//
/////////////////////////////////////////////////////////////////////////////
void CALLBACK
ZoneEditDialogClass::fnUpdateTimer
(
	UINT	/* uID */,
	UINT	/* uMsg */,
	DWORD_PTR	user_data,
	DWORD_PTR	/* dw1 */,
	DWORD_PTR	/* dw2 */
)
{
	HWND hwnd = (HWND)user_data;
	if (hwnd != NULL) {

        if ((GetProp (hwnd, "WaitingToProcess") == NULL)) {
            SetProp (hwnd, "WaitingToProcess", (HANDLE)1);

            // Send the message to the view so it will be in the
				// same thread (We don't seem to be thread-safe)
				::PostMessage (hwnd, WM_USER+101, 0, 0L);
        }
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
ZoneEditDialogClass::WindowProc
(
	UINT		message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == (WM_USER + 101)) {
		Render_View ();
	} else if (message == WM_KEYDOWN || message == WM_KEYUP) {
		
		//
		//	Eat the keyboard messages we 'special case'.
		//
		if (wParam == VK_NUMPAD8 || wParam == VK_NUMPAD2 ||
			 wParam == VK_NUMPAD4 || wParam == VK_NUMPAD6 ||
			 wParam == VK_NUMPAD7 || wParam == VK_NUMPAD1)
		{
			return 0;
		}
	}

	return CDialog::WindowProc (message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//  OnTop
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnTop (void)
{
	Vector3 pos = m_RenderObj->Get_Position ();

	Matrix3D transform (1);
	transform.Look_At (pos + Vector3 (0, 0, m_LookAtDist), pos, 3.1415926535F);
	m_Camera->Set_Transform (transform);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnFront
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnFront (void)
{
	Vector3 pos = m_RenderObj->Get_Position ();

	Matrix3D transform (1);
	transform.Look_At (pos + Vector3 (m_LookAtDist, 0, 0), pos, 0);
	m_Camera->Set_Transform (transform);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLeft
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnLeft (void)
{
	Vector3 pos = m_RenderObj->Get_Position ();

	Matrix3D transform (1);
	transform.Look_At (pos + Vector3 (0, -m_LookAtDist, 0), pos, 0);
	m_Camera->Set_Transform (transform);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnRight
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnRight (void)
{
	Vector3 pos = m_RenderObj->Get_Position ();

	Matrix3D transform (1);
	transform.Look_At (pos + Vector3 (0, m_LookAtDist, 0), pos, 0);
	m_Camera->Set_Transform (transform);
	return ;	
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Object
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Load_Object (void)
{
	//
	//	Determine how big the object is
	//
	SphereClass sphere	= m_RenderObj->Get_Bounding_Sphere ();
	m_LookAtDist			= sphere.Radius * 1.5F;
	m_LookAtDist			= std::max (m_LookAtDist, 4.0F);

	//
	//	Add the render object to the world at 0, 0, 0
	//
	m_Scene->Add_Render_Object (m_RenderObj);
	m_RenderObj->Set_Transform (Matrix3D(Vector3(0, 0, 0)));

	//
	//	Add a grid render object to the world to act as a floor
	//
	RenderObjClass *floor = ::Create_Render_Obj ("GRID");
	if (floor != NULL) {
		floor->Set_Transform (Matrix3D(1));
		m_Scene->Add_Render_Object (floor);
		floor->Release_Ref ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Insert_Zone
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Insert_Zone (void)
{
	//
	//	Create the zone and add it to the scene
	//
	m_Zone = new Box3DClass (Vector3 (1, 1, 1));
	m_Zone->Set_Color (Vector3 (0, 0.7F, 0));	

	//
	//	Transform the zone from relative to world space coords
	//
	Matrix3D obj_tm = m_RenderObj->Get_Transform ();
	OBBoxClass zone_box;
	OBBoxClass::Transform (obj_tm, m_ZoneBox, &zone_box);

	m_Zone->Set_Position (zone_box.Center);

	if (zone_box.Extent.X > 0 || zone_box.Extent.Y > 0 || zone_box.Extent.Z > 0) {
		m_Zone->Set_Dimensions (zone_box.Extent * 2.0F);
	}

	m_Scene->Add_Render_Object (m_Zone);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Phys_Obj
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Set_Phys_Obj (PhysClass *phys_obj)
{	
	MEMBER_ADD (m_PhysObj, phys_obj);

	//
	//	Extract the model from the physics object
	//
	MEMBER_RELEASE (m_RenderObj);
	if (m_PhysObj != NULL) {
		m_RenderObj = m_PhysObj->Peek_Model ();
		m_RenderObj->Add_Ref ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Keypress
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Handle_Keypress (void)
{
	//
	//	Determine which speed modifier to use
	//
	float speed_mod = 1.0F;
	if (::GetAsyncKeyState (VK_CONTROL) < 0) {
		speed_mod = 3.0F;
	}

	//
	//	Move the camera forward-backward (if necessary)
	//
	if ((::GetAsyncKeyState (VK_NUMPAD8) < 0) ||
		 (::GetAsyncKeyState (VK_NUMPAD2) < 0)) {

		// Get the camera's current position
		Vector3 position	= m_Camera->Get_Position ();
		float orig_z		= position.Z;
		
		//
		//	Determine how far to move the camera
		//
		float delta	= (::GetAsyncKeyState (VK_NUMPAD8) < 0) ? -0.35F : 0.35F;
		delta			*= speed_mod;

		//
		// Scale the position along the direction vector
		//
		Matrix3D transform = m_Camera->Get_Transform ();
		position += (delta * transform.Get_Z_Vector ());

		//
		// Should we lock the z-position?
		//
		if (::GetAsyncKeyState (VK_CAPITAL) < 0) {
			position.Z = orig_z;
		}

		// Set the camera's new position
		m_Camera->Set_Position (position);
	}


	//
	//	Pan the camera left-right (if neccessary)
	//
	if ((::GetAsyncKeyState (VK_NUMPAD4) < 0) ||
		 (::GetAsyncKeyState (VK_NUMPAD6) < 0)) {

		// Get the camera's current position
		Vector3 position	= m_Camera->Get_Position ();
		
		//
		//	Determine how far to move the camera
		//
		float delta	= (::GetAsyncKeyState (VK_NUMPAD4) < 0) ? -0.35F : 0.35F;
		delta			*= speed_mod;

		//
		// Scale the position along the x-vector
		//
		Matrix3D transform = m_Camera->Get_Transform ();
		position += (delta * transform.Get_X_Vector ());

		// Set the camera's new position
		m_Camera->Set_Position (position);
	}

	//
	//	Elevate the camera up-down (if neccessary)
	//
	if ((::GetAsyncKeyState (VK_NUMPAD7) < 0) ||
		 (::GetAsyncKeyState (VK_NUMPAD1) < 0)) {

		// Get the camera's current position
		Vector3 position	= m_Camera->Get_Position ();
		
		//
		//	Determine how far to move the camera
		//
		float delta	= (::GetAsyncKeyState (VK_NUMPAD1) < 0) ? -0.35F : 0.35F;
		delta			*= speed_mod;

		//
		// Scale the position along the x-vector
		//
		Matrix3D transform = m_Camera->Get_Transform ();
		position += (delta * transform.Get_Y_Vector ());

		// Set the camera's new position
		m_Camera->Set_Position (position);
	}
	
	if ((::GetAsyncKeyState (VK_UP) < 0) ||
		 (::GetAsyncKeyState (VK_DOWN) < 0) ||
		 (::GetAsyncKeyState (VK_LEFT) < 0) ||
		 (::GetAsyncKeyState (VK_RIGHT) < 0))
	{
		float amount = 0.025F;
		if (::GetAsyncKeyState (VK_CONTROL) < 0) {
			amount *= 4.0F;
		}

		Vector3 translation(0,0,0);

		if (::GetAsyncKeyState (VK_UP) < 0) {
			if (::GetAsyncKeyState (VK_SHIFT) < 0) {
				translation.Z += amount;
			} else {
				translation.X += amount;
			}			
		}

		if (::GetAsyncKeyState (VK_DOWN) < 0) {
			if (::GetAsyncKeyState (VK_SHIFT) < 0) {
				translation.Z -= amount;
			} else {
				translation.X -= amount;
			}			
		}

		if (::GetAsyncKeyState (VK_RIGHT) < 0) {
			translation.Y += amount;
		}

		if (::GetAsyncKeyState (VK_LEFT) < 0) {
			translation.Y -= amount;
		}		

		//
		//	Move the zone
		//
		if (m_IsEditingZone) {
			Matrix3D tm = m_Zone->Get_Transform ();
			tm.Translate (translation);
			m_Zone->Set_Transform (tm);
		}

		//
		//	Size the Zone
		//
		if (m_IsSizingZone) {
			Vector3 size = m_Zone->Get_Dimensions ();
			size += translation;
			m_Zone->Set_Dimensions (size);
		}

		Update_Status ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Status
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Update_Status (void)
{
	//
	//	Update the zone's status
	//
	Vector3 zone_pos	= m_Zone->Get_Transform ().Get_Translation ();
	Vector3 zone_size	= m_Zone->Get_Dimensions ();
	CString status;
	status.Format ("Zone Pos (%.2f, %.2f, %.2f)\nZone Size (%.2f, %.2f, %.2f)",
						zone_pos.X, zone_pos.Y, zone_pos.Z,
						zone_size.X, zone_size.Y, zone_size.Z);
	SetDlgItemText (IDC_ZONE_STATUS, status);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnEditZone
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnEditZone (void)
{
	m_IsEditingZone = (SendDlgItemMessage (IDC_EDIT_ZONE, BM_GETCHECK) == 1);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fn3DWindow
//
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
ZoneEditDialogClass::fn3DWindow
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	if (message == WM_LBUTTONDOWN) {
		ZoneEditDialogClass *dialog = NULL;
		dialog = (ZoneEditDialogClass *)::GetProp (hwnd, "ZONE_DIALOG");
		dialog->Handle_LBUTTON_DOWN (wparam, lparam);
	} else if (message == WM_LBUTTONUP) {
		ZoneEditDialogClass *dialog = NULL;
		dialog = (ZoneEditDialogClass *)::GetProp (hwnd, "ZONE_DIALOG");
		dialog->Handle_LBUTTON_UP (wparam, lparam);
	} else if (message == WM_MOUSEMOVE) {
		ZoneEditDialogClass *dialog = NULL;
		dialog = (ZoneEditDialogClass *)::GetProp (hwnd, "ZONE_DIALOG");
		dialog->Handle_MOUSEMOVE (wparam, lparam);		
	}

	return ::DefWindowProc (hwnd, message, wparam, lparam);
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_LBUTTON_DOWN
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Handle_LBUTTON_DOWN (WPARAM /* wparam */, LPARAM lparam)
{
	if (m_Initialized == false) {
		return ;
	}

	::SetCapture (::GetDlgItem (m_hWnd, IDC_3D_WINDOW));
	m_LastPoint.x = LOWORD (lparam);
	m_LastPoint.y = HIWORD (lparam);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_LBUTTON_UP
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Handle_LBUTTON_UP (WPARAM /* wparam */, LPARAM /* lparam */)
{
	::ReleaseCapture ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_MOUSEMOVE
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::Handle_MOUSEMOVE (WPARAM wparam, LPARAM lparam)
{
	if (m_Initialized == false) {
		return ;
	}

	POINT point = { LOWORD (lparam), HIWORD (lparam) };
	WWASSERT (m_Camera != NULL);

	if ((wparam & MK_LBUTTON) && (wparam & MK_RBUTTON)) {
		float delta_x = float(m_LastPoint.x - point.x) / 32;
		float delta_y = float(point.y - m_LastPoint.y) / 32;

		//
		// Scale the position along the direction vector
		//
		Vector3 position	= m_Camera->Get_Position ();
		Matrix3D transform = m_Camera->Get_Transform ();
		position += (delta_x * transform.Get_X_Vector ());
		position += (delta_y * transform.Get_Y_Vector ());

		// Set the camera's new position
		m_Camera->Set_Position (position);

	} else if (wparam & MK_LBUTTON) {

		//
		//	Orbit the camera around the object
		//
		if (::GetCapture () == ::GetDlgItem (m_hWnd, IDC_3D_WINDOW)) {
			::Trackball_Camera (	::GetDlgItem (m_hWnd, IDC_3D_WINDOW),
										*m_Camera,
										Vector3 (0, 0, 0),
										point,
										m_LastPoint	);
		}

	} else if (wparam & MK_RBUTTON) {				
		float delta = float(m_LastPoint.y - point.y) / 12;

		//
		// Scale the position along the direction vector
		//
		Vector3 position	= m_Camera->Get_Position ();
		Matrix3D transform = m_Camera->Get_Transform ();
		position += (delta * transform.Get_Z_Vector ());

		// Set the camera's new position
		m_Camera->Set_Position (position);
	}

	m_LastPoint = point;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Trackball_Camera
//
/////////////////////////////////////////////////////////////////////////////
void
Trackball_Camera
(
	HWND					hwnd,
	CameraClass &		camera,
	const Vector3 &	center,
	POINT					point,
	POINT					last_point
)
{
	RECT rect;
	::GetClientRect (hwnd, &rect);

	//
	//	Convert the points to normalized units
	//
	float mid_point_x		= float(rect.right >> 1);
	float mid_point_y		= float(rect.bottom >> 1);

	float last_point_x	= ((float)last_point.x - mid_point_x) / mid_point_x;
	float last_point_y	= (mid_point_y - (float)last_point.y) / mid_point_y;

	float point_x			= ((float)point.x - mid_point_x) / mid_point_x;
	float point_y			= (mid_point_y - (float)point.y) / mid_point_y;

	//
	// Calculate the orbit
	//
	Quaternion rotation = ::Trackball (last_point_x, last_point_y, point_x, point_y, 0.8F);

	//
	// Get the transformation matrix for the camera and its inverse
	//
	Matrix3D transform = camera.Get_Transform ();
	Matrix3D inv_tm;
	transform.Get_Orthogonal_Inverse (inv_tm);

	//
	//	Translate to the object's center, perform the rotation, then translate back out.
	//
	Vector3 to_object = inv_tm * center;
	transform.Translate (to_object);
	Matrix3D::Multiply (transform, Build_Matrix3D (rotation), &transform);
	transform.Translate (-to_object);

	// Pass the new trnasform onto the camera
	camera.Set_Transform (transform);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  PreTranslateMessage
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ZoneEditDialogClass::PreTranslateMessage (MSG *pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) {
		
		if (pMsg->wParam == VK_NUMPAD8 || pMsg->wParam == VK_NUMPAD2 ||
			 pMsg->wParam == VK_NUMPAD4 || pMsg->wParam == VK_NUMPAD6 ||
			 pMsg->wParam == VK_NUMPAD7 || pMsg->wParam == VK_NUMPAD1 ||
			 pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ||
			 pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT)
		{
			return 1;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSizeZone
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnSizeZone (void)
{
	m_IsSizingZone = (m_SizeZoneCheck.GetCheck () == 1);
	
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_EDIT_ZONE), !m_IsSizingZone);

	if (m_IsSizingZone) {
		m_IsEditingZone	= false;
	} else {
		OnEditZone ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLastFrame
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnLastFrame (void) 
{
	SendDlgItemMessage (IDC_FIRST_FRAME, BM_SETCHECK, (WPARAM)false);
	SendDlgItemMessage (IDC_LAST_FRAME, BM_SETCHECK, (WPARAM)true);

	if (m_PhysObj == NULL) {
		return ;
	}

	//
	//	Make sure this object can be animated...
	//
	StaticAnimPhysClass *anim_obj = m_PhysObj->As_StaticAnimPhysClass ();
	if (anim_obj != NULL) {
		AnimCollisionManagerClass &anim_mgr = anim_obj->Get_Animation_Manager ();

		//
		//	Pop the animation to its last frame
		//
		HAnimClass *animation = anim_mgr.Peek_Animation ();
		if (animation != NULL) {
			m_RenderObj->Set_Animation (animation, animation->Get_Num_Frames () - 1);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnFirstFrame
//
/////////////////////////////////////////////////////////////////////////////
void
ZoneEditDialogClass::OnFirstFrame (void)
{
	SendDlgItemMessage (IDC_FIRST_FRAME, BM_SETCHECK, (WPARAM)true);
	SendDlgItemMessage (IDC_LAST_FRAME, BM_SETCHECK, (WPARAM)false);

	if (m_PhysObj == NULL) {
		return ;
	}

	//
	//	Make sure this object can be animated...
	//
	StaticAnimPhysClass *anim_obj = m_PhysObj->As_StaticAnimPhysClass ();
	if (anim_obj != NULL) {
		AnimCollisionManagerClass &anim_mgr = anim_obj->Get_Animation_Manager ();

		//
		//	Pop the animation to its first frame
		//
		HAnimClass *animation = anim_mgr.Peek_Animation ();
		if (animation != NULL) {
			m_RenderObj->Set_Animation (animation, 0);
		}
	}

	return ;
}
