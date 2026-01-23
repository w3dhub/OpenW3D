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

// TransitionEditDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "LevelEditView.h"
#include "TransitionEditDialog.h"
#include "ww3d.h"
#include "scene.h"
#include "camera.h"
#include "Utils.h"
#include "mmsystem.h"
#include "matrix3d.h"
#include "rendobj.h"
#include "transition.h"
#include "sphere.h"
#include "box3d.h"
#include "cameramgr.h"
#include "filemgr.h"
#include "filelocations.h"
#include "hanim.h"
#include "quat.h"
#include "vector3.h"
#include "obbox.h"
#include "combatchunkid.h"
#include "soldier.h"
#include "editorbuild.h"
#include "mixfiledatabase.h"
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
// TransitionEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
TransitionEditDialogClass::TransitionEditDialogClass(CWnd* pParent /*=NULL*/)
	:	m_LookAtDist (0),
		m_Camera (NULL),
		m_Scene (NULL),
		m_RenderObj (NULL),
		m_Transition (NULL),
		m_Zone (NULL),
		m_CharacterObj (NULL),
		m_Animation (NULL),
		m_SwapChain (NULL),
		m_TimerID (0),
		m_CurrFrame (0),
		m_LastAnimUpdate (0),
		m_IsEditingZone (true),
		m_IsEditingChar (true),
		m_IsAnimating (false),
		m_IsSizingZone (false),
		m_IsRotatingChar (false),
		CDialog(TransitionEditDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(TransitionEditDialogClass)
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
TransitionEditDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TransitionEditDialogClass)
	DDX_Control(pDX, IDC_EDIT_ZONE, m_MoveZoneCheck);
	DDX_Control(pDX, IDC_EDIT_CHAR, m_MoveCharCheck);
	DDX_Control(pDX, IDC_SIZE_ZONE, m_SizeZoneCheck);
	DDX_Control(pDX, IDC_ROTATE_CHAR, m_RotateCharCheck);
	DDX_Control(pDX, IDC_ANIMATION_LIST, m_AnimationList);
	DDX_Control(pDX, IDC_TRIGGER_LIST, m_TriggerList);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TransitionEditDialogClass, CDialog)
	//{{AFX_MSG_MAP(TransitionEditDialogClass)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_TOP, OnTop)
	ON_BN_CLICKED(IDC_FRONT, OnFront)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_EDIT_CHAR, OnEditChar)
	ON_BN_CLICKED(IDC_EDIT_ZONE, OnEditZone)
	ON_CBN_SELCHANGE(IDC_ANIMATION_LIST, OnSelChangeAnimationList)
	ON_COMMAND(IDM_ANI_BACK_FRAME, OnAniBackFrame)
	ON_COMMAND(IDM_ANI_FIRST_FRAME, OnAniFirstFrame)
	ON_COMMAND(IDM_ANI_FWD_FRAME, OnAniFwdFrame)
	ON_COMMAND(IDM_ANI_LAST_FRAME, OnAniLastFrame)
	ON_COMMAND(IDM_ANI_PAUSE, OnAniPause)
	ON_COMMAND(IDM_ANI_PLAY, OnAniPlay)
	ON_COMMAND(IDM_ANI_STOP, OnAniStop)
	ON_BN_CLICKED(IDC_SIZE_ZONE, OnSizeZone)
	ON_BN_CLICKED(IDC_ROTATE_CHAR, OnRotateChar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
TransitionEditDialogClass::OnInitDialog (void)
{
	CWaitCursor wait_cursor;

	CLevelEditView::Allow_Repaint (false);
	CDialog::OnInitDialog ();

	//
	//	Subclass the 3D window for mouse-tracking
	//
	SetWindowLongPtr (::GetDlgItem (m_hWnd, IDC_3D_WINDOW), GWLP_WNDPROC, (LONG_PTR)fn3DWindow);
	::SetProp (::GetDlgItem (m_hWnd, IDC_3D_WINDOW), "TRANSITION_DIALOG", (HANDLE)this);

	//
	//	Setup the toolbar
	//
	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_BORDER | WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 10, 10), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_ANI_TOOLBAR);
	m_Toolbar.SetBarStyle (m_Toolbar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY);

	// Position the toolbar
	CRect rect;
	CSize size;
	m_Toolbar.GetToolBarCtrl ().GetMaxSize (&size);
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_TOOLBAR), &rect);
	ScreenToClient (&rect);
	m_Toolbar.SetWindowPos (NULL,
									rect.left + (rect.Width () >> 1) - (size.cx >> 1),
									rect.top + (rect.Height () >> 1) - (size.cy >> 1),
									rect.Width (),
									rect.Height (),
									SWP_SHOWWINDOW);

	//
	//	Get the dimensions of the client area of the window we'll be rendering in
	//
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
	Fill_Trigger_List ();
	Fill_Animation_List ();
	Load_Animation ();

	//
	//	Select the default UI
	//
	SendDlgItemMessage (IDC_TOP, BM_SETCHECK, (WPARAM)true);
	SendDlgItemMessage (IDC_EDIT_ZONE, BM_SETCHECK, (WPARAM)true);
	SendDlgItemMessage (IDC_EDIT_CHAR, BM_SETCHECK, (WPARAM)true);
	OnTop ();
	OnEditZone ();
	OnEditChar ();
	Update_Status ();

	// Kick off a timer that we can use to update
	// the display (kinda like a game loop iterator)
	m_TimerID = ::timeSetEvent (	50,
											50,
											fnUpdateTimer,
											(DWORD_PTR)m_hWnd,
											TIME_PERIODIC);	
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnOK (void)
{
	//
	//	Create a matrix we can use to transform the zone and
	// character into 'relative' space.
	//
	Matrix3D obj_tm = m_RenderObj->Get_Transform ();
	Matrix3D obj_inv;
	obj_tm.Get_Orthogonal_Inverse (obj_inv);

	//
	//	Pass the new animation name onto the transition object
	//
	CString name;
	m_AnimationList.GetWindowText (name);
	CString animation_name = CString ("S_A_HUMAN.") + name;
	m_Transition->Set_Animation_Name ((LPCTSTR)animation_name);

	//
	//	Pass the trigger type onto the transition
	//
	m_Transition->Set_Type ((TransitionDataClass::StyleType)m_TriggerList.GetCurSel ());

	//
	//	Pass the character's ending position onto the transition
	//
	Matrix3D ending_tm = m_CharacterObj->Get_Transform () * obj_inv;
	m_Transition->Set_Ending_TM (ending_tm);

	//
	//	Transform the zone from world space coords to relative coords
	//
	OBBoxClass zone_box (m_Zone->Get_Transform ().Get_Translation (), m_Zone->Get_Dimensions () * 0.5F);
	OBBoxClass rel_zone; 
	OBBoxClass::Transform (obj_inv, zone_box, &rel_zone);
	m_Transition->Set_Zone (rel_zone);

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnDestroy (void)
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
	m_Scene->Remove_Render_Object (m_CharacterObj);	
	MEMBER_RELEASE (m_Scene);
	MEMBER_RELEASE (m_Camera);
	MEMBER_RELEASE (m_RenderObj);
	MEMBER_RELEASE (m_CharacterObj);	
	MEMBER_RELEASE (m_Zone);
	MEMBER_RELEASE (m_Animation);	

	//
	//	Free all the data we associated with the animation list
	//
	int count = m_AnimationList.GetCount ();
	for (int index = 0; index < count; index ++) {
		LPTSTR data = (LPTSTR)m_AnimationList.GetItemData (index);
		if (data != NULL) {
			::free (data);
		}
	}
	m_AnimationList.ResetContent ();

	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Render_View
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Render_View (void)
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
		//	Pick the next animation frame
		//
		Update_Animation ();

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
TransitionEditDialogClass::fnUpdateTimer
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
TransitionEditDialogClass::WindowProc
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
TransitionEditDialogClass::OnTop (void)
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
TransitionEditDialogClass::OnFront (void)
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
TransitionEditDialogClass::OnLeft (void)
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
TransitionEditDialogClass::OnRight (void)
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
TransitionEditDialogClass::Load_Object (void)
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
	m_RenderObj->Set_Transform (Matrix3D(Vector3(0, 0, m_Height)));

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
TransitionEditDialogClass::Insert_Zone (void)
{
	//
	//	Create the zone and add it to the scene
	//
	m_Zone = new Box3DClass (Vector3 (1, 1, 1));
	m_Zone->Set_Color (Vector3 (0, 0, 0.7F));	

	//
	//	Transform the zone from relative to world space coords
	//
	Matrix3D obj_tm = m_RenderObj->Get_Transform ();
	OBBoxClass rel_zone = m_Transition->Get_Zone ();
	OBBoxClass zone_box;
	OBBoxClass::Transform (obj_tm, rel_zone, &zone_box);

	m_Zone->Set_Position (zone_box.Center);
	m_Zone->Set_Dimensions (zone_box.Extent * 2.0F);

	m_Scene->Add_Render_Object (m_Zone);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Render_Obj
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Set_Render_Obj (RenderObjClass *render_obj)
{
	MEMBER_ADD (m_RenderObj, render_obj);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Transition
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Set_Transition (TransitionDataClass *transition)
{
	m_Transition = transition;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Keypress
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Handle_Keypress (void)
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

		//
		//	Move the character
		//
		if (m_IsEditingChar) {
			Matrix3D tm = m_CharacterObj->Get_Transform ();
			tm.Set_Translation (tm.Get_Translation () + translation);
			m_CharacterObj->Set_Transform (tm);
		}

		//
		//	Rotate the character
		//
		if (	m_IsRotatingChar &&
				((::GetAsyncKeyState (VK_RIGHT) < 0) ||
				 (::GetAsyncKeyState (VK_LEFT) < 0))	)
		{
			Matrix3D tm = m_CharacterObj->Get_Transform ();
			float deg = (::GetAsyncKeyState (VK_RIGHT) < 0) ? 5.0F : -5.0F;
			tm.Rotate_Z (DEG_TO_RAD (deg));
			m_CharacterObj->Set_Transform (tm);
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
TransitionEditDialogClass::Update_Status (void)
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

	//
	//	Update the character's status
	//
	Vector3 char_pos	= m_CharacterObj->Get_Transform ().Get_Translation ();
	float char_rot		= m_CharacterObj->Get_Transform ().Get_Z_Rotation ();
	status.Format ("Char Pos (%.2f, %.2f, %.2f)\nChar Rot (%.2f)",
						char_pos.X, char_pos.Y, char_pos.Z,
						RAD_TO_DEG (char_rot));

	SetDlgItemText (IDC_CHAR_STATUS, status);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Fill_Trigger_List
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Fill_Trigger_List (void)
{
	int type_to_sel = (int)m_Transition->Get_Type ();

	int count = TransitionDataClass::Get_Num_Types ();
	for (int index = 0; index < count; index ++) {
		LPCTSTR name = TransitionDataClass::Get_Type_Name ((TransitionDataClass::StyleType)index);
		int item_index = m_TriggerList.AddString (name);
		if (index == type_to_sel) {
			m_TriggerList.SetCurSel (item_index);
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Fill_Animation_List
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Fill_Animation_List (void)
{
	int index = 0;

	//
	// Generate a list of animation files
	//
	DynamicVectorClass<StringClass> file_list;
	file_list.Set_Growth_Step (500);

	#ifdef PUBLIC_EDITOR_VER
	
		MixFileDatabaseClass::Get_Instance ()->Find_Files (file_list, "h_a_*.w3d");

	#else

		//
		// Generate a list of animation files
		//
		DynamicVectorClass<CString> temp_file_list;
		CString path = ::Get_File_Mgr()->Make_Full_Path (CHAR_ANIMS_PATH);
		::Get_File_Mgr ()->Find_Files (path, "*.w3d", temp_file_list, true);
		for (index = 0; index < temp_file_list.Count (); index ++) {
			file_list.Add ((const char *)temp_file_list[index]);
		}

	#endif //PUBLIC_EDITOR_VER
		
	//
	// Populate the combobox
	//
	bool selected = false;
	for (index = 0; index < file_list.Count (); index ++) {
		StringClass &filename	= file_list[index];
		CString animation_name	= ::Asset_Name_From_Filename (filename);
		
		//
		// Add this entry to the animation combobox
		//
		int cb_index = m_AnimationList.AddString (animation_name);
		if (cb_index != CB_ERR) {
			m_AnimationList.SetItemData (cb_index, (DWORD_PTR)::strdup ((LPCTSTR)filename));

			//
			//	Should we select this animation by default?
			//
			CString name = m_Transition->Get_Animation_Name ();
			CString real_name = name;
			LPCTSTR suffix = ::strstr (name, ".");
			if (suffix != NULL) {
				real_name = suffix + 1;
			}
			if (::lstrcmpi (animation_name, real_name) == 0) {
				m_AnimationList.SetCurSel (cb_index);
				selected = true;
			}
		}
	}

	if (!selected) {
		m_AnimationList.SetCurSel (0);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnEditChar
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnEditChar (void)
{
	m_IsEditingChar = (SendDlgItemMessage (IDC_EDIT_CHAR, BM_GETCHECK) == 1);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnEditZone
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnEditZone (void)
{
	m_IsEditingZone = (SendDlgItemMessage (IDC_EDIT_ZONE, BM_GETCHECK) == 1);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Animation
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Load_Animation (void)
{
	//
	//	Determine which animation file to load
	//
	int index			= m_AnimationList.GetCurSel ();
	if (index != CB_ERR) {
		LPCTSTR filename	= (LPCTSTR)m_AnimationList.GetItemData (index);

		//
		//	Load the animation from the file
		//
		if (filename != NULL) {
			Load_Animation (filename);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Animation
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Load_Animation (LPCTSTR filename)
{
	//
	//	Create the character if necessary
	//
	Load_Character ();
	MEMBER_RELEASE (m_Animation);

	//
	//	Load the new animation
	//
	CString path			= ::Strip_Filename_From_Path (filename);
	CString asset_name	= CString ("S_A_HUMAN.") + ::Asset_Name_From_Filename (filename);
	::Set_Current_Directory (path);
	m_Animation = WW3DAssetManager::Get_Instance ()->Get_HAnim (asset_name);
	ASSERT (m_Animation != NULL);

	//
	//	Pass the new animation onto the character
	//
	m_CurrFrame = 0;
	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Character
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Load_Character (void)
{
	if (m_CharacterObj == NULL) {

		//
		//	Create the character's render object
		//
		SoldierGameObjDef *definition = (SoldierGameObjDef *)DefinitionMgrClass::Find_Typed_Definition ("Commando", CLASSID_GAME_OBJECT_DEF_SOLDIER, false);
		if (definition != NULL) {
			SoldierGameObj *game_obj = new SoldierGameObj;
			game_obj->Init (*definition);
			REF_PTR_SET (m_CharacterObj, game_obj->Peek_Model ());
			game_obj->Set_Delete_Pending ();
		}

		//
		//	Transform the character from relative to world space coords
		//
		Matrix3D obj_tm	= m_RenderObj->Get_Transform ();
		Matrix3D char_tm	= m_Transition->Get_Ending_TM ();
		char_tm				= char_tm * obj_tm;		
		m_CharacterObj->Set_Transform (char_tm);

		//
		//	Add the character to the world
		//
		m_Scene->Add_Render_Object (m_CharacterObj);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSelChangeAnimationList
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnSelChangeAnimationList (void)
{
	Load_Animation ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  fn3DWindow
//
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
TransitionEditDialogClass::fn3DWindow
(
	HWND		hwnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	if (message == WM_LBUTTONDOWN) {
		TransitionEditDialogClass *dialog = NULL;
		dialog = (TransitionEditDialogClass *)::GetProp (hwnd, "TRANSITION_DIALOG");
		dialog->Handle_LBUTTON_DOWN (wparam, lparam);
	} else if (message == WM_LBUTTONUP) {
		TransitionEditDialogClass *dialog = NULL;
		dialog = (TransitionEditDialogClass *)::GetProp (hwnd, "TRANSITION_DIALOG");
		dialog->Handle_LBUTTON_UP (wparam, lparam);
	} else if (message == WM_MOUSEMOVE) {
		TransitionEditDialogClass *dialog = NULL;
		dialog = (TransitionEditDialogClass *)::GetProp (hwnd, "TRANSITION_DIALOG");
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
TransitionEditDialogClass::Handle_LBUTTON_DOWN (WPARAM /* wparam */, LPARAM lparam)
{
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
TransitionEditDialogClass::Handle_LBUTTON_UP (WPARAM /* wparam */, LPARAM /* lparam */)
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
TransitionEditDialogClass::Handle_MOUSEMOVE (WPARAM wparam, LPARAM lparam)
{
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

	} else  if (wparam & MK_LBUTTON) {

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
TransitionEditDialogClass::PreTranslateMessage (MSG *pMsg)
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
//  OnAniBackFrame
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniBackFrame (void)
{
	m_CurrFrame -= 1.0F;
	if (m_CurrFrame < 0) {
		m_CurrFrame = float(m_Animation->Get_Num_Frames () - 1);
	}

	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniFirstFrame
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniFirstFrame (void)
{
	m_CurrFrame = 0;
	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniFwdFrame
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniFwdFrame (void)
{
	m_CurrFrame += 1;
	if (m_CurrFrame >= float(m_Animation->Get_Num_Frames () - 1)) {
		m_CurrFrame = 0;
	}

	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniLastFrame
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniLastFrame (void)
{
	m_CurrFrame = float(m_Animation->Get_Num_Frames () - 1);
	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniPause
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniPause (void)
{
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniPlay
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniPlay (void)
{
	m_IsAnimating		= true;
	m_LastAnimUpdate	= ::GetTickCount ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnAniStop
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnAniStop (void)
{
	m_CurrFrame = 0;
	m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
	m_IsAnimating = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Animation
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::Update_Animation (void)
{
	if (m_IsAnimating) {
		DWORD curr_ticks = ::GetTickCount ();		
		
		float seconds		= float((curr_ticks - m_LastAnimUpdate)/1000.0F);
		float frame_inc	= seconds * m_Animation->Get_Frame_Rate ();
		m_CurrFrame			+= frame_inc;		

		if (m_CurrFrame > float(m_Animation->Get_Num_Frames ())) {
			m_CurrFrame -= float(m_Animation->Get_Num_Frames ());
		}

		m_CharacterObj->Set_Animation (m_Animation, m_CurrFrame);
		m_LastAnimUpdate = curr_ticks;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSizeZone
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnSizeZone (void)
{
	m_IsSizingZone = (m_SizeZoneCheck.GetCheck () == 1);
	
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_EDIT_ZONE), !m_IsSizingZone);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_EDIT_CHAR), !m_IsSizingZone);

	if (m_IsSizingZone) {
		m_IsEditingZone	= false;
		m_IsEditingChar	= false;
		m_IsRotatingChar	= false;
		m_RotateCharCheck.SetCheck (false);
	} else {
		OnEditChar ();
		OnEditZone ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnRotateChar
//
/////////////////////////////////////////////////////////////////////////////
void
TransitionEditDialogClass::OnRotateChar (void)
{
	m_IsRotatingChar = (m_RotateCharCheck.GetCheck () == 1);
	
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_EDIT_ZONE), !m_IsRotatingChar);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_EDIT_CHAR), !m_IsRotatingChar);

	if (m_IsRotatingChar) {
		m_IsEditingZone	= false;
		m_IsEditingChar	= false;
		m_IsSizingZone		= false;
		m_SizeZoneCheck.SetCheck (false);
	} else {
		OnEditChar ();
		OnEditZone ();
	}

	return ;
}
