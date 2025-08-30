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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/viewerctrl.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 4:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VIEWER_CTRL_H
#define __VIEWER_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "render2d.h"
#include "aabox.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class SimpleSceneClass;
class CameraClass;
class RenderObjClass;
class LightClass;


////////////////////////////////////////////////////////////////
//
//	ViewerCtrlClass
//
////////////////////////////////////////////////////////////////
class ViewerCtrlClass : public DialogControlClass
{
public:

	enum InterfaceModeEnum {
		Z_ROTATION,				// Automatic rotation of camera around Z-axis.
		VIRTUAL_TRACKBALL		// Camera rotation with virtual trackball under user control.
	};
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ViewerCtrlClass (void);
	virtual ~ViewerCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	// RTTI.
	ViewerCtrlClass *As_ViewerCtrlClass (void) override		{ return this; }

	//
	//	From DialogControlClass
	//
	void				Render (void) override;

	//
	// Configuation
	//
	void				Set_Background_Visible (bool isvisible)		{IsBackgroundVisible = isvisible;}
	void				Set_Rotation_Rate (float r)						{RotationRate = DEG_TO_RADF (r);}
	void				Set_Interface_Mode (InterfaceModeEnum mode, float rotationrate = 0.0f);
	void				Set_Camera_Min_Dist (float dist)				{ MinCameraDist = dist; }

	//
	//	Selection management
	//
	SimpleSceneClass *Peek_Scene()									{ return (Scene); }
	CameraClass		  *Peek_Camera()									{ return (Camera); }	
	void					Set_Model (const char *model_name);
	void					Set_Model (RenderObjClass *new_model);
	RenderObjClass	  *Peek_Model()									{ return (Model); }
	void					Set_Animation (const char *anim_name);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Frame_Update (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderer (void);
	void				Free_Model (void);
	void				Get_Visible_Bounding_Box (AABoxClass *box, RenderObjClass *render_obj, bool &is_first);
	void				Calculate_Camera_Position (void);


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	SimpleSceneClass *	Scene;
	CameraClass	*			Camera;
	LightClass *			Light;					// Light source.
	RenderObjClass *		Model;
	Render2DClass			ControlRenderer;
	AABoxClass				BoundingBox;
	float						Distance;
	float						ZRotation;
	float						RotationRate;
	float						MinCameraDist;
	bool						IsCameraDirty;
	bool						IsBackgroundVisible;
	InterfaceModeEnum		InterfaceMode;
	Vector3					LastMousePosition;
};


#endif //__VIEWER_CTRL_H
