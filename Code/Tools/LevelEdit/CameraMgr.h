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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/CameraMgr.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 5:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __CAMERAMGR_H
#define __CAMERAMGR_H


#include "refcount.h"
#include "Utils.h"
#include "rendobj.h"
#include "assetmgr.h"
#include "camera.h"
#include "physcontrol.h"
#include "hermitespline.h"


// Forward declarations
class NodeClass;
class SoldierGameObj;
class SoldierGameObjDef;


/*class CamaraCharPhys : public CharPhysClass
{
	public:
		CamaraCharPhys (void);
		virtual ~CamaraCharPhys (void);

	protected:
		RenderObjClass *	m_pRenderObj;
};*/


///////////////////////////////////////////////////////////////
//
//	CameraMgr
//
///////////////////////////////////////////////////////////////
class CameraMgr
{
	public:

		///////////////////////////////////////////////////
		//	Public constructors/destructors
		///////////////////////////////////////////////////
		CameraMgr (void);
		virtual ~CameraMgr (void);


		///////////////////////////////////////////////////
		//	Public data types
		///////////////////////////////////////////////////
		typedef enum
		{
			MODE_MOVE_PLANE		= 0,
			MODE_MOVE_UPDOWN		= 1,
			MODE_FLY_THROUGH		= 2,
			MODE_WALK_THROUGH		= 3,
			MODE_ROTATE_FREE		= 4,
			MODE_ROTATE_X			= 5,
			MODE_ROTATE_Y			= 6,
			MODE_ROTATE_Z			= 7,
			MODE_ORBIT				= 8,
			MODE_FLYTO				= 9,
			MODE_COUNT

		} CAMERA_MODE;

		typedef enum
		{
			CAMERA_FRONT = 1,
			CAMERA_BACK,
			CAMERA_TOP,
			CAMERA_BOTTOM,
			CAMERA_LEFT,
			CAMERA_RIGHT,
			CAMERA_COUNT

		} CAMERA_POS;


		///////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////
		CameraMgr::CAMERA_MODE		Get_Camera_Mode (void) const { return m_CameraMode; }
		void								Set_Camera_Mode (CAMERA_MODE new_mode);
		void								Update_Camera (float deltax, float deltay);
		void								Update_Camera_Animation (void);
		void								On_Frame (void);
		Matrix3D							Get_Character_TM (void);

		//
		// Position methods
		//
		void								Set_Camera_Pos (CAMERA_POS position);
		void								Set_Transform (const Matrix3D &transform);
		void								Set_Position (const Vector3 &position);
		void								Move_Fwd (void) {}
		void								Move_Bkwd (void) {}
		void								Turn_Left (void) {}
		void								Turn_Right (void) {}
		void								Look_Up (void) {}
		void								Look_Dn (void) {}
		void								Look_Center (void) {}
		void								Increase_Speed (void);
		void								Decrease_Speed (void);
		void								Level_Camera (void);
		void								Goto_Node (NodeClass *node);
		void								Goto_Group (GroupMgrClass *pgroup);
		void								Fly_To_Transform (const Matrix3D &transform);

		//
		//	Inline accessors
		//
		CameraClass *					Get_Camera (void) const { return m_pCamera; }
		//CamaraCharPhys *				Get_Walkthru_Character (void) const { return m_pWalkThroughPhys; }
		float								Get_Speed_Modifier (void) const { return m_SpeedModifier; }

	protected:
			
		///////////////////////////////////////////////////
		//	Friends
		///////////////////////////////////////////////////
		friend LRESULT CALLBACK fnCameraKeyboardHook (int code, WPARAM wParam, LPARAM lParam);
		friend class CLevelEditView;

		///////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////
		void								Init_Camera (void);
		void								Auto_Level (void);
		void								Update_Fly_To (void);

		static void						Update_Camera_MOVE_ZOOM		(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_MOVE_PLANE	(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_FLY_THROUGH	(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_WALK_THROUGH	(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_ROTATE_FREE	(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_ROTATE_X		(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_ROTATE_Y		(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_ROTATE_Z		(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_ORBIT			(CameraClass &camera, float deltax, float deltay);
		static void						Update_Camera_FLY_TO			(CameraClass &camera, float deltax, float deltay);

		///////////////////////////////////////////////////
		//	Protected data types
		///////////////////////////////////////////////////
		typedef void					(*UPDATE_CAMERA_FN) (CameraClass &camera, float deltax, float deltay);

		///////////////////////////////////////////////////
		//	Static member data
		///////////////////////////////////////////////////
		static bool						_pKeyboardState[256];
		static UPDATE_CAMERA_FN		_pfnUpdateMethods[CAMERA_MODE::MODE_COUNT];	
		static HHOOK					_hHook;

	private:

		///////////////////////////////////////////////////
		//	Private member data
		///////////////////////////////////////////////////
		
		bool							m_bAutoLevel;
		CameraClass *				m_pCamera;
		CAMERA_MODE					m_CameraMode;
		Matrix3D						m_AutoLevelInitalMatrix;
		Matrix3D						m_AutoLevelEndMatrix;
		double						m_AutoLevelPercent;
		//CamaraCharPhys *			m_pWalkThroughPhys;
		PhysControllerClass		m_VJoystick;
		float							m_SpeedModifier;
		HermiteSpline3DClass		m_FlyToSpline;
		Matrix3D						m_FlyToStartTransform;
		Matrix3D						m_FlyToEndTransform;
		DWORD							m_FlyToStartTime;

		SoldierGameObj *			m_WalkThruObj;
		SoldierGameObjDef *		m_WalkThruDef;
};


#endif //__CAMERAMGR_H
