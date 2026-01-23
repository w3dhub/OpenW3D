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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/CameraMgr.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 10:29a                                             $*
 *                                                                                             *
 *                    $Revision:: 46                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "Utils.h"
#include "camera.h"
#include "cameramgr.h"
#include "MouseMgr.h"
#include "leveleditdoc.h"
#include "groupmgr.h"
#include "formtoolbar.h"
#include "camerasettingsform.h"
#include "mainfrm.h"
#include "filemgr.h"
#include "editorassetmgr.h"
#include "_assetmgr.h"
#include "sceneeditor.h"
#include "node.h"
#include "quat.h"
#include "combat.h"
#include "filelocations.h"
#include "combatchunkid.h"
#include "ccamera.h"
#include "input.h"
#include "LevelEditView.h"
#include "nodemgr.h"
#include "actionparams.h"
#include "soldier.h"
#include <algorithm>


////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
////////////////////////////////////////////////////////////////////////////
const float DEF_CAMERA_DIST		= 80.00F;
const float MIN_MOVE_SCALE			= 30.00F;

const float	WALK_THRU_VEL_FWD		= 3.5F;
const float	WALK_THRU_VEL_BKWD	= -WALK_THRU_VEL_FWD;
const float	WALK_THRU_VEL_LEFT	= 0.8F;
const float	WALK_THRU_VEL_RIGHT	= -WALK_THRU_VEL_LEFT;
const float	WALK_THRU_VEL_STRAFE	= 4.5F;


////////////////////////////////////////////////////////////////////////////
//
//	Static member initialization
//
////////////////////////////////////////////////////////////////////////////
CameraMgr::UPDATE_CAMERA_FN CameraMgr::_pfnUpdateMethods[CAMERA_MODE::MODE_COUNT] =
{
	CameraMgr::Update_Camera_MOVE_ZOOM,
	CameraMgr::Update_Camera_MOVE_PLANE,
	CameraMgr::Update_Camera_FLY_THROUGH,
	CameraMgr::Update_Camera_WALK_THROUGH,
	CameraMgr::Update_Camera_ROTATE_FREE,
	CameraMgr::Update_Camera_ROTATE_X,
	CameraMgr::Update_Camera_ROTATE_Y,
	CameraMgr::Update_Camera_ROTATE_Z,
	CameraMgr::Update_Camera_ORBIT,
	CameraMgr::Update_Camera_FLY_TO
};


bool CameraMgr::_pKeyboardState[256] = { 0 };
HHOOK CameraMgr::_hHook = NULL;

////////////////////////////////////////////////////////////////////////////
//
//	CameraMgr
//
////////////////////////////////////////////////////////////////////////////
CameraMgr::CameraMgr (void)
	:	m_pCamera (NULL),
		m_bAutoLevel (false),
		m_CameraMode (MODE_MOVE_PLANE),
		m_SpeedModifier (6.0F),
		m_WalkThruObj (NULL),
		m_WalkThruDef (NULL)
{
	Init_Camera ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	CameraMgr
//
////////////////////////////////////////////////////////////////////////////
CameraMgr::~CameraMgr (void)
{
	MEMBER_RELEASE (m_pCamera);

	if (m_WalkThruObj != NULL) {
		m_WalkThruObj->Set_Delete_Pending ();
		m_WalkThruObj = NULL;
		Input::Shutdown ();
	}

	if (m_WalkThruDef != NULL) {
		delete m_WalkThruDef;
	}


	//MEMBER_RELEASE (m_pWalkThroughPhys);

	// Remove our hook procedure from the chain
	if (_hHook != NULL) {
		::UnhookWindowsHookEx (_hHook);
		_hHook = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	fnCameraKeyboardHook
//
////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK
fnCameraKeyboardHook
(
	int code,
	WPARAM wParam,
	LPARAM lParam
)
{
	// Should we process this message?
	if (code >= 0) {
		if ((wParam >= 0) && (wParam <= 255)) {
			CameraMgr::_pKeyboardState[wParam] = bool((lParam & 0x80000000) != 0x80000000);
		}
	}
	
	// Call the next hook procedure in the chain
	return ::CallNextHookEx (CameraMgr::_hHook, code, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//	Init_Camera
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Init_Camera (void)
{
	if (m_pCamera == NULL) {
		// Create a new camera object
		m_pCamera = new CameraClass;		
	}

	// Were we able to create a new object?
	ASSERT (m_pCamera!= NULL);
	if (m_pCamera) {

		// Create a transformation matrix
		Matrix3D transform_matrix (1);
		transform_matrix.Translate (Vector3 (0.0F, 0.0F, DEF_CAMERA_DIST));
		m_pCamera->Set_Clip_Planes (0.1F, 200.0F);
		//m_pCamera->Set_Environment_Range (9000.0F, 10000.0F);

		// Move the camera back along the z-axis looking at world center
		Set_Transform (transform_matrix);
		m_pCamera->Set_Transform (transform_matrix);
	}

	// Install a windows hook procedure so we know when keys are pressed
	if (_hHook == NULL) {
		::memset (_pKeyboardState, 0, sizeof (_pKeyboardState));
		_hHook = ::SetWindowsHookEx (WH_KEYBOARD,
												fnCameraKeyboardHook,
												::AfxGetInstanceHandle (),
												::GetCurrentThreadId ());
	}

	// Reset the camera mode
	m_CameraMode = MODE_MOVE_PLANE;
	return ;
}

double _XRot = 0;
double _YRot = 0;

////////////////////////////////////////////////////////////////////////////
//
//	Set_Camera_Mode
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Set_Camera_Mode (CAMERA_MODE new_mode)
{
	// Is this really a new mode?
	if (new_mode != m_CameraMode) {

		if (new_mode == MODE_WALK_THROUGH) {			
			::Get_Scene_Editor ()->Set_Selection (NULL);

			GameInFocus = true;
			if (m_WalkThruObj == NULL) {				
				Input::Init ();
				Input::Load_Configuration (DEFAULT_INPUT_FILENAME);
				Input::Set_Mouse_Invert (true);

				//
				//	Create a commando game object and initialize it
				// with the first commando preset we found
				//
				SoldierGameObjDef *definition = (SoldierGameObjDef *)DefinitionMgrClass::Find_Typed_Definition ("Walk-Thru", CLASSID_GAME_OBJECT_DEF_SOLDIER, false);
				m_WalkThruObj = new SoldierGameObj;
				m_WalkThruObj->Init (*definition);

				ActionParamsStruct parameters;
				m_WalkThruObj->Get_Action()->Follow_Input( parameters );
			}

			m_WalkThruObj->Control_Enable (true);
			m_WalkThruObj->Set_Control_Owner (CombatManager::Get_My_Id ());
			CombatManager::Set_I_Am_Client (true);
			CombatManager::Set_The_Star (m_WalkThruObj);

			//
			//	Give the physics object an initial transform
			//
			PhysClass *phys_obj = m_WalkThruObj->Peek_Physical_Object ();
			if (phys_obj != NULL) {
				Matrix3D transform = m_pCamera->Get_Transform ();

				Matrix3D cam_to_world (Vector3 (0, 0, -1), Vector3 (-1, 0, 0), Vector3 (0, 1, 0), Vector3 (0, 0, 0));
				transform = transform * cam_to_world;
				phys_obj->Set_Transform (transform);
			}


		} else if (m_WalkThruObj != NULL) {
						
			m_WalkThruObj->Set_Delete_Pending ();
			m_WalkThruObj = NULL;
			Input::Shutdown ();
		}
		
		m_CameraMode = new_mode;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera
(
	float deltax,
	float deltay
)
{
	// State OK?
	ASSERT (m_pCamera != NULL);
	if (m_pCamera != NULL) {

		// Call into the state table to perform the update
		_pfnUpdateMethods[m_CameraMode] (*m_pCamera, deltax, deltay);

		if (m_bAutoLevel) {
			
			// If ew are auto-leveling, then reset the initial
			// rotation matrix and percent			
			m_AutoLevelInitalMatrix = m_pCamera->Get_Transform ();
			m_AutoLevelPercent = 0.00F;
		}

	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Level_Camera
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Level_Camera (void)
{
	m_bAutoLevel = true;

	// Reset the initial rotation matrix and percent
	m_AutoLevelInitalMatrix = m_pCamera->Get_Transform ();
	m_AutoLevelPercent = 0.00F;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Auto_Level
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Auto_Level (void)
{
	// Is there a mouse button down?
	// We don't want to auto-level if the mouse button is down
	BOOL mouse_down = BOOL(::GetKeyState (VK_LBUTTON) & 0xF000);
	mouse_down |= BOOL(::GetKeyState (VK_RBUTTON) & 0xF000);

	// Should we auto-level the camera?
	if ((m_AutoLevelPercent <= 1) && (mouse_down == false)) {
		
		// Get the camera's current transformation matrix
		Matrix3D matrix = m_pCamera->Get_Transform ();

		Vector3 cam_pos;
		Vector3 cam_target;

		// Get the camera position
		matrix.Get_Translation (&cam_pos);

		// Get a vector 1 meter below the camera position
		cam_target = matrix * Vector3 (0.00F, 0.00F, -1.00F);
		
		// Force the z components of the 2 vectors to be the same.
		cam_target.Z = cam_pos.Z;

		// Create a new transformation matrix which is positioned
		// at the camera and is looking at the vector we created
		Matrix3D new_matrix;
		new_matrix.Look_At (cam_pos, cam_target, 0.0F);

		// Build quaternions for the start and end rotation matricies
		Quaternion initial_quat = ::Build_Quaternion (m_AutoLevelInitalMatrix);
		Quaternion end_quat = ::Build_Quaternion (new_matrix);
		
		// Linerally-interpolate between the 2 rotations (from 0.0 - 1.0)
		Quaternion rotation_now;
		::Slerp (rotation_now, initial_quat, end_quat, m_AutoLevelPercent);

		// Increment the current interpolation percent
		m_AutoLevelPercent += 0.04F;

		// Set the rotation for the camera's transormation matrix
		matrix.Set_Rotation (rotation_now);
	
		// Set the camera's new rotation
		Set_Transform (matrix);

		if (m_AutoLevelPercent >= 1) {
			m_bAutoLevel = false;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Increase_Speed
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Increase_Speed (void)
{
	if (m_SpeedModifier < 1.0F) {
		m_SpeedModifier += 0.1F;
	} else {
		m_SpeedModifier += 0.75F;
	}

	// Normalize the speed
	if (m_SpeedModifier > 25.00F) {
		m_SpeedModifier = 25.00F;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Decrease_Speed
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Decrease_Speed (void)
{
	if (m_SpeedModifier < 2.0F) {
		m_SpeedModifier -= 0.1F;
	} else {
		m_SpeedModifier -= 0.75F;
	}

	// Normalize the speed
	if (m_SpeedModifier < 0.25F) {
		m_SpeedModifier = 0.25F;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	On_Frame
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::On_Frame (void)
{
	float speed_mod = CameraMgr::_pKeyboardState[VK_CONTROL] ? m_SpeedModifier : 1.0F;

	if ((_pKeyboardState[VK_NUMPAD8]) ||
		 (_pKeyboardState[VK_NUMPAD2])) {

		// Get the camera's current position
		Vector3 position = m_pCamera->Get_Position ();
		
		// Get the current direction the camera is looking at
		Matrix3D matrix = m_pCamera->Get_Transform ();
		
		float delta = _pKeyboardState[VK_NUMPAD8] ? -0.35F : 0.35F;
		delta *= speed_mod;

		// Should we lock the z-position?
		if (::GetKeyState (VK_CAPITAL) & 0x0001) {

			// Scale the position 'forward' and 'backward'

			Vector3 y_vector = Vector3::Cross_Product (matrix.Get_X_Vector (), Vector3 (0, 0, 1));
			y_vector.Normalize ();
			position += delta * y_vector;

		} else {

			// Scale the position along the direction vector
			position += (delta * matrix.Get_Z_Vector ());
		}

		// Set the camera's new position
		Set_Position (position);
	}
	
	if ((_pKeyboardState[VK_NUMPAD9]) ||
		 (_pKeyboardState[VK_NUMPAD3])) {

		// Get the camera's current transform
		Matrix3D transform = m_pCamera->Get_Transform ();

		// Rotate by the specified number of radians
		float rotation = _pKeyboardState[VK_NUMPAD9] ? 0.0314F : -0.0314F;
		rotation *= (speed_mod / 2.5F);
		if (rotation > 0.045F) {
			rotation = 0.045F;
		} else if (rotation < -0.045F) {
			rotation = -0.045F;
		}

		transform.Rotate_X (rotation);
		
		// Set the camera's new transform
		Set_Transform (transform);
	} 
	
	if ((_pKeyboardState[VK_NUMPAD4]) ||
		 (_pKeyboardState[VK_NUMPAD6])) {

		if (::GetKeyState (VK_MENU) < 0) {

			// Get the camera's current position
			Vector3 position = m_pCamera->Get_Position ();
			
			float delta = (_pKeyboardState[VK_NUMPAD4]) ? -0.25F : 0.25F;
			delta *= speed_mod;			
			position += (delta * m_pCamera->Get_Transform ().Get_X_Vector ());

			// Set the camera's new position
			Set_Position (position);
			
		} else {				
			
			// Get the pure rotation from the transform
			Matrix3D transform = m_pCamera->Get_Transform ();
			Vector3 translation = transform.Get_Translation ();
			transform.Set_Translation (Vector3 (0, 0, 0));

			// Determine an appropriate rotation delta
			float delta = (_pKeyboardState[VK_NUMPAD4]) ? 0.0314F : -0.0314F;
			delta *= speed_mod;
			if (delta > 0.06F) {
				delta = 0.06F;
			} else if (delta < -0.06F) {
				delta = -0.06F;
			}

			Matrix3D world_rotation(1);
			world_rotation.Rotate_Z (delta);
					
			// Rotate using the world's coord system
			transform = world_rotation * transform;
			transform.Set_Translation (translation);
			
			// Set the camera's new transform
			Set_Transform (transform);		
		}
	}

	if ((_pKeyboardState[VK_NUMPAD7]) ||
		 (_pKeyboardState[VK_NUMPAD1])) {

		// Get the camera's current position
		Vector3 position = m_pCamera->Get_Position ();
		
		float delta = (_pKeyboardState[VK_NUMPAD1]) ? -0.25F : 0.25F;
		delta *= speed_mod;
		position.Z += delta;

		// Set the camera's new position
		Set_Position (position);
	}

	// Are we currently in 'walk-thru' mode?
	if (m_CameraMode == MODE_WALK_THROUGH) {
		if (m_WalkThruObj != NULL) {

			if (::GetAsyncKeyState (VK_ESCAPE) < 0 || ::GetAsyncKeyState (VK_F6) < 0) {
				::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);
			} else {

				//
				//	Check for the VIS-Update keypress
				//
				static DWORD last_vis_gen = 0;
				static DWORD last_static_anim_toggle = 0;
				DWORD current_ticks = ::GetTickCount ();
				if ((::GetAsyncKeyState (VK_RETURN) < 0) && (current_ticks - last_vis_gen) > 1000) {
					Vector3 sample_point	= m_WalkThruObj->Get_Transform ().Get_Translation () + Vector3 (0, 0, 1);
					::Get_Scene_Editor ()->Record_Vis_Info (m_pCamera->Get_Transform (), sample_point);
					last_vis_gen = current_ticks;
				} else if (::GetAsyncKeyState (VK_F3) < 0) {
					bool points_visible = ::Get_Scene_Editor ()->Are_Manual_Vis_Points_Visible ();
					::Get_Scene_Editor ()->Show_Manual_Vis_Points (!points_visible);
				} else if (	(::GetAsyncKeyState (VK_CONTROL) < 0) &&
								(::GetAsyncKeyState (VK_SHIFT) < 0) &&
								(::GetAsyncKeyState ('T') < 0))
				{
					//
					//	Toggle the static anims... (if necessary)
					//
					if ((current_ticks - last_static_anim_toggle) > 250) {
						((CMainFrame *)::AfxGetMainWnd ())->Toggle_Static_Anims ();
						last_static_anim_toggle = current_ticks;
					}
				}

						
				//
				//	Allow the game object and physic object to think
				//			
				m_WalkThruObj->Generate_Control ();
				m_WalkThruObj->Think ();								
				PhysClass *phys_obj = m_WalkThruObj->Peek_Physical_Object ();
				if (phys_obj != NULL) {
					phys_obj->Timestep (TimeManager::Get_Frame_Seconds ());
				}			
				m_WalkThruObj->Post_Think ();

				CCameraClass *game_camera = CombatManager::Get_Camera ();
				if (game_camera != NULL) {

					//
					//	Update the game camera
					//
					Input::Update ();
					CombatManager::Handle_Input ();
					CombatManager::Update_Star ();
					
					Matrix3D tm = m_WalkThruObj->Get_Transform();
					game_camera->Update ();

					//
					//	Copy the game camera's transform
					//
					Matrix3D camera_tm = game_camera->Get_Transform ();
					m_pCamera->Set_Transform (camera_tm);
				}
			}
		}
	}

	if (m_bAutoLevel) {
		Auto_Level ();
	}

	if (m_CameraMode == MODE_FLYTO) {
		Update_Fly_To ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_Animation
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_Animation (void)
{
	//
	//	TODO - move the camera depending on the animation mode
	//
	return ;
}


//**************************************************************************************************************//
//
//	Start of static methods
//
//**************************************************************************************************************//


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_MOVE_ZOOM
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_MOVE_ZOOM
(
	CameraClass &	camera,
	float				/* deltax */,
	float				deltay
)
{
	// Get the camera's current position
	Vector3 position = camera.Get_Position ();
	
	// Get the current direction the camera is looking at
	Matrix3D matrix = camera.Get_Transform ();
	Vector3 orientation = matrix.Get_Z_Vector ();

	float scale_factor = (position.Z > MIN_MOVE_SCALE) ? position.Z : MIN_MOVE_SCALE;

	// Scale the position along the direction vector
	position = position - (deltay * orientation * scale_factor);

	// Set the camera's new position
	::Get_Camera_Mgr ()->Set_Position (position);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_MOVE_PLANE
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_MOVE_PLANE
(
	CameraClass &	camera,
	float				deltax,
	float				deltay
)
{
	// Get the camera's current position
	Vector3 position = camera.Get_Position ();
	
	// Get the vector the camera is 'sliding' up and down on
	Matrix3D matrix = camera.Get_Transform ();
	Vector3 orientation = matrix.Get_Y_Vector ();

	float scale_factor = (position.Z > MIN_MOVE_SCALE) ? position.Z : MIN_MOVE_SCALE;

	// Scale the position along the direction vector
	position = position + (deltay * orientation * scale_factor);

	// Scale the position along the 'left/right' vector
	orientation = matrix.Get_X_Vector ();
	position = position - (deltax * orientation * scale_factor);

	// Set the camera's new position
	::Get_Camera_Mgr ()->Set_Position (position);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_FLY_THROUGH
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_FLY_THROUGH
(
	CameraClass &	/* camera */,
	float				/* deltax */,
	float				/* deltay */
)
{
	//
	//	TODO - update the camera
	//
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_WALK_THROUGH
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_WALK_THROUGH
(
	CameraClass &	/* camera */,
	float				/* deltax */,
	float				deltay
)
{
	//
	//	TODO - update the camera
	//

	//Matrix3D transform = camera.Get_Transform ();
	//transform.Rotate_X (deltay*3.0F);
	_XRot += deltay * 2.0F;
	//_YRot += deltax * 3.0F;
	//camera.Set_Transform (transform);

	// Get the camera's current transformation matrix
	//CamaraCharPhys *pcharacter = ::Get_Camera_Mgr ()->Get_Walkthru_Character ();
	if (0) {//pcharacter != NULL) {
		
		/*Matrix3D transform = pcharacter->Get_Transform ();
		transform.Rotate_Z (deltax * 2.0F);
		pcharacter->Set_Transform (transform);*/
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_ROTATE_FREE
//
void
CameraMgr::Update_Camera_ROTATE_FREE
(
	CameraClass &	camera,
	float				deltax,
	float				deltay
)
{
	// Rotate the camera's transformation matrix based
	// on the deltas
	Matrix3D matrix = camera.Get_Transform ();
	//matrix.Rotate_X (deltay);
	//matrix.Rotate_Y (deltax);

	// Restrict the rotation to the current axis
	switch (::Get_Current_Document ()->Get_Axis_Restriction ()) {
		
		case CLevelEditDoc::RESTRICT_X:
		{
			matrix.Rotate_X (deltay);
		}
		break;

		case CLevelEditDoc::RESTRICT_Y:
		{
			matrix.Rotate_Y (deltax);
		}
		break;

		case CLevelEditDoc::RESTRICT_Z:
		{
			matrix.Rotate_Z (deltay);
		}
		break;

		default:
		{
			matrix.Rotate_X (deltay);
			matrix.Rotate_Y (deltax);
		}
		break;
	}

	// Set the camera's new rotation	
	::Get_Camera_Mgr ()->Set_Transform (matrix);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_ROTATE_X
//
void
CameraMgr::Update_Camera_ROTATE_X
(
	CameraClass &	camera,
	float				/* deltax */,
	float				deltay
)
{
	// Rotate the camera's transformation matrix based
	// on the x delta
	Matrix3D matrix = camera.Get_Transform ();
	matrix.Rotate_X (deltay);

	// Set the camera's new rotation
	::Get_Camera_Mgr ()->Set_Transform (matrix);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_ROTATE_Y
//
void
CameraMgr::Update_Camera_ROTATE_Y
(
	CameraClass &camera,
	float deltax,
	float /* deltay */
)
{
	// Rotate the camera's transformation matrix based
	// on the y delta
	Matrix3D matrix = camera.Get_Transform ();
	matrix.Rotate_Y (deltax);

	// Set the camera's new rotation
	::Get_Camera_Mgr ()->Set_Transform (matrix);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_ROTATE_Z
//
void
CameraMgr::Update_Camera_ROTATE_Z
(
	CameraClass &	camera,
	float				deltax,
	float				deltay
)
{
	// Rotate the camera's transformation matrix based
	// on the x and y deltas, but restrict the rotation
	// to Z
	Matrix3D matrix = camera.Get_Transform ();
	Matrix3D temp_matrix = matrix;
	temp_matrix.Rotate_X (deltay);
	temp_matrix.Rotate_Y (deltax);

	/*Matrix3D new_matrix (1);
	new_matrix.Set_Translation (matrix.Get_Translation ());
	new_matrix.Rotate_X (matrix.Get_X_Rotation ());
	new_matrix.Rotate_Y (matrix.Get_Y_Rotation ());
	new_matrix.Rotate_Z (temp_matrix.Get_Z_Rotation ());*/

	Matrix3D new_matrix = matrix;
	new_matrix.Rotate_Z (deltax+deltay);
	//new_matrix.Rotate_Z (deltay);

	// Set the camera's new rotation
	::Get_Camera_Mgr ()->Set_Transform (matrix);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_FLY_TO
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_FLY_TO
(
	CameraClass &/* camera */,
	float /*deltax*/,
	float /*deltay*/
)
{
	::Get_Camera_Mgr ()->Update_Fly_To ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Camera_ORBIT
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Camera_ORBIT
(
	CameraClass &camera,
	float /*deltax*/,
	float /*deltay*/
)
{
	//
	//	TODO - update the camera
	//

	MouseMgrClass *pmouse_mgr = ::Get_Mouse_Mgr ();

	CPoint point = pmouse_mgr->Get_Mouse_Point ();
	CPoint last_point = pmouse_mgr->Get_Last_Mouse_Point ();

	Vector3 viewpos;
	Vector3 last_viewpos;
	camera.Device_To_View_Space (Vector2 (last_point.x, last_point.y), &last_viewpos);
	camera.Device_To_View_Space (Vector2 (point.x, point.y), &viewpos);

	// Rotate the camara like a trackball
	Quaternion rotation = ::Trackball (last_viewpos.X, last_viewpos.Y, viewpos.X, viewpos.Y, 0.4F);

	// Rotate this item about its z axis
	if (::Get_Selection_Mgr ().Get_Count () > 0) {

		//SphereClass bounding_sphere = ::Get_Selection_Mgr ().Get_Bounding_Sphere ();
		//bounding_sphere.Center;

		Matrix3D matrix = camera.Get_Transform ();
		Matrix3D matrix_inv;
		matrix.Get_Orthogonal_Inverse (matrix_inv);

		Vector3 vector_to_object = matrix_inv * ::Get_Selection_Mgr ().Get_Center ();//bounding_sphere.Center;
		matrix.Translate (vector_to_object);

		Matrix3D::Multiply (matrix, ::Build_Matrix3D (rotation), &matrix);

		matrix.Translate (-vector_to_object);

		::Get_Camera_Mgr ()->Set_Transform (matrix);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Set_Camera_Pos
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Set_Camera_Pos (CAMERA_POS position)
{
	SphereClass sp = ::Get_Selection_Mgr ().Get_Bounding_Sphere ();
	if (Get_Selection_Mgr ().Get_Count () == 0) {
		sp.Center = Vector3 (0, 0, 0);
		sp.Radius = 100.00F;
	}

	float distance = sp.Radius * 3.00F;
	distance = (distance < 1.0F) ? 1.0F : distance;
	distance = (distance > 400.0F) ? 400.0F : distance;

	Matrix3D transform (1);        

	switch (position)
	{
		case CAMERA_FRONT:
		{
			transform.Look_At (sp.Center + Vector3 (distance, 0.00F, 0.00F), sp.Center, 0);
		}
		break;

		case CAMERA_BACK:
		{
			transform.Look_At (sp.Center + Vector3 (-distance, 0.00F, 0.00F), sp.Center, 0);
		}
		break;

		case CAMERA_LEFT:
		{
			transform.Look_At (sp.Center + Vector3 (0.00F, -distance, 0.00F), sp.Center, 0);
		}
		break;

		case CAMERA_RIGHT:
		{
			transform.Look_At (sp.Center + Vector3 (0.00F, distance, 0.00F), sp.Center, 0);
		}
		break;

		case CAMERA_TOP:
		{
			transform.Look_At (sp.Center + Vector3 (0.00F, 0.00F, distance), sp.Center, 3.1415926535F);
		}
		break;

		case CAMERA_BOTTOM:
		{
			transform.Look_At (sp.Center + Vector3 (0.00F, 0.00F, -distance), sp.Center, 3.1415926535F);
		}
		break;
	}

	// Move the camera back to get a good view of the object
	Set_Transform (transform);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Goto_Node
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Goto_Node (NodeClass *node)
{
	RenderObjClass *render_obj = node->Peek_Render_Obj ();
	if (render_obj != NULL) {

		//
		// Get the node's position
		//
		SphereClass sphere = render_obj->Get_Bounding_Sphere ();
		Matrix3D tm = render_obj->Get_Transform ();
		tm.Set_Translation (Vector3 (0, 0, 0));

		//
		//	Calculate an appropriate viewing distance
		//
		float distance = sphere.Radius * 4;
		distance = (distance < 1.0F) ? 1.0F : distance;
		distance = (distance > 100.0F) ? 100.0F : distance;

		Matrix3D transform (1);
		//transform.Look_At (sphere.Center + Vector3 (distance, distance, distance), sphere.Center, 0);
		Vector3 camera_loc = (sphere.Center + Vector3 (-distance, 0, distance));
		transform.Look_At (camera_loc, sphere.Center, 0);
		//tranform.Set_Translation (sphere.Center);
		//render_obj->

		//
		// Move the camera back to get a good view of the object
		//
		Set_Transform (transform);
		
		// Ensure the window is updated
		::Refresh_Main_View ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Goto_Group
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Goto_Group (GroupMgrClass *group)
{
	// State OK?
	if (group != NULL) {

		// Get the node's position
		SphereClass sp = group->Get_Bounding_Sphere ();

		float distance = sp.Radius;
		distance = (distance < 1.0F) ? 1.0F : distance;
		distance = (distance > 400.0F) ? 400.0F : distance;

		Matrix3D transform (1);        
		transform.Look_At (sp.Center + Vector3 (distance, distance, distance), sp.Center, 0);

		// Move the camera back to get a good view of the object
		Set_Transform (transform);
		
		// Ensure the window is updated
		::Refresh_Main_View ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Set_Transform (const Matrix3D &transform)
{
	if (m_pCamera != NULL) {
		
		// Set the camera's new location and orientation
		m_pCamera->Set_Transform (transform);

		// Update the the UI to reflect this new position
		((CMainFrame *)::AfxGetMainWnd ())->Update_Camera_Location (transform.Get_Translation ());		
		Set_Modified (true);

		// If the camera settings dialog is visible, update its contents
		/*FormToolbarClass &toolbar = ((CMainFrame *)::AfxGetMainWnd ())->Get_Camera_Toolbar ();
		if (::IsWindow (toolbar) && toolbar.IsVisible ()) {			
			CameraSettingsFormClass *pform = (CameraSettingsFormClass *)toolbar.Get_Form ();
			if (pform != NULL) {
				pform->Update_Controls ();
			}
		}*/
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Set_Position
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Set_Position (const Vector3 &position)
{
	if (m_pCamera != NULL) {
		
		// Set the camera's new location and orientation
		m_pCamera->Set_Position (position);

		// Update the the UI to reflect this new position
		((CMainFrame *)::AfxGetMainWnd ())->Update_Camera_Location (position);
		Set_Modified (true);

		// If the camera settings dialog is visible, update its contents
		/*FormToolbarClass &toolbar = ((CMainFrame *)::AfxGetMainWnd ())->Get_Camera_Toolbar ();
		if (::IsWindow (toolbar) && toolbar.IsVisible ()) {			
			CameraSettingsFormClass *pform = (CameraSettingsFormClass *)toolbar.Get_Form ();
			if (pform != NULL) {
				pform->Update_Controls ();
			}
		}*/
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Fly_To_Transform
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Fly_To_Transform (const Matrix3D &transform)
{
	m_FlyToStartTransform	= m_pCamera->Get_Transform ();
	m_FlyToEndTransform		= transform;

	Vector3 start_point	= m_FlyToStartTransform.Get_Translation ();
	Vector3 end_point		= m_FlyToEndTransform.Get_Translation ();

	Vector3 delta = end_point - start_point;
	float len = delta.Length () / 2;
	Vector3 middle_point = start_point + Vector3 (delta.X / 2, delta.Y / 2, delta.Z / 2 + len);

	// Remove all keys from the spline
	while (m_FlyToSpline.Key_Count () > 0) {
		m_FlyToSpline.Remove_Key (0);
	}

	Vector3 looking_start = -20.0F * m_FlyToStartTransform.Get_Z_Vector ();
	int index = m_FlyToSpline.Add_Key (start_point, 0);
	m_FlyToSpline.Set_Tangents (index, looking_start, looking_start);

	Vector3 looking_end = -20.0F * m_FlyToEndTransform.Get_Z_Vector ();
	index = m_FlyToSpline.Add_Key (end_point, 1);
	m_FlyToSpline.Set_Tangents (index, looking_end, looking_end);

	m_FlyToStartTime = ::GetTickCount ();
	Set_Camera_Mode (MODE_FLYTO);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Update_Fly_To
//
////////////////////////////////////////////////////////////////////////////
void
CameraMgr::Update_Fly_To (void)
{
	const DWORD FLY_TICKS = 5000;
	float fly_ticks = (float)FLY_TICKS;
	
	if (::GetAsyncKeyState (VK_CONTROL) < 0) {
		fly_ticks = fly_ticks / m_SpeedModifier;
	}

	DWORD current_time = ::GetTickCount ();
	float spline_time = float(current_time - m_FlyToStartTime) / fly_ticks;
	spline_time = std::min (1.0F, spline_time);

	if (m_FlyToStartTime + ((DWORD)fly_ticks) < current_time) {
		m_pCamera->Set_Transform (m_FlyToEndTransform);
		Set_Camera_Mode (MODE_MOVE_PLANE);
	} else {
		Vector3 position (0,0,0);
		Vector3 next_position (0,0,0);
		m_FlyToSpline.Evaluate (spline_time, &position);
		m_FlyToSpline.Evaluate (spline_time + 0.05F, &next_position);

		Matrix3D rot_90 (1);
		rot_90.Rotate_Z ((float)DEG_TO_RAD (90));

		Vector3 x_axis = next_position - position;
		x_axis.Normalize ();

		Vector3 y_axis = rot_90 * x_axis;
		y_axis.Normalize ();

		Vector3 z_axis;
		Vector3::Cross_Product (x_axis, y_axis, &z_axis);
		Matrix3D orientation (x_axis, y_axis, z_axis, position);
		
		Matrix3D world_to_cam (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
		Matrix3D camera_tm = orientation * world_to_cam;

		if (camera_tm.Is_Orthogonal () == false) {
			TRACE ("Matrix not orthogonal!\r\n");
			camera_tm.Re_Orthogonalize();
		}

		Set_Transform (camera_tm);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_Character_TM
//
////////////////////////////////////////////////////////////////////////////
Matrix3D
CameraMgr::Get_Character_TM (void)
{
	Matrix3D tm (1);

	if (m_WalkThruObj != NULL) {
		tm = m_WalkThruObj->Get_Transform ();
	} else {
		tm = m_pCamera->Get_Transform ();
	}

	return tm;
}
