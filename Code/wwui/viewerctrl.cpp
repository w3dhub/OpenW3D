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
 *                     $Archive:: /Commando/Code/wwui/viewerctrl.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 4:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "viewerctrl.h"
#include "assetmgr.h"
#include "refcount.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "scene.h"
#include "camera.h"
#include "rendobj.h"
#include "hanim.h"
#include "mesh.h"
#include "meshmdl.h"
#include "light.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//
//	ViewerCtrlClass
//
////////////////////////////////////////////////////////////////
ViewerCtrlClass::ViewerCtrlClass (void)	:
	Scene (NULL),
	Camera (NULL),
	Model (NULL),
	Distance (0),
	ZRotation (0),
	MinCameraDist (0),
	RotationRate (DEG_TO_RADF (0.0f)),
	BoundingBox (Vector3 (0, 0, 0), Vector3 (0, 0, 0)),
	IsCameraDirty (true),
	IsBackgroundVisible (true),
	InterfaceMode (Z_ROTATION)
{
	Vector3 ambientcolor (0.2f, 0.2f, 0.2f);
		
	LastMousePosition = DialogMgrClass::Get_Mouse_Pos();

	//
	//	Configure the 2D renderer
	//
	StyleMgrClass::Configure_Renderer (&ControlRenderer);

	//
	//	Create a scene to use to render the model
	//
	Scene = new SimpleSceneClass;
	Scene->Set_Ambient_Light (ambientcolor);

	//
	//	Create a camera to use in the scene
	//	NOTE: Near and far clip planes are set to values which will suit a large range of object sizes.
	//			However, very small objects may pass thru the near clip plane and large objects may pass
	//			thru the far clip plane (and Z-fighting will result if the ratio is too high). If this
	//			proves to problematic replace the constant settings with a function of object size that
	//			can be set on a per object basis.
	//			
	Camera = new CameraClass;
	Camera->Set_Clip_Planes (0.25F, 300.0F);

	// Create and add a light.
	Light = NEW_REF (LightClass, ());
	Light->Set_Intensity (1.0f);
	Light->Set_Ambient (Vector3 (0.0f, 0.0f, 0.0f));
	Light->Set_Diffuse (Vector3 (1.0f, 1.0f, 1.0f));
	Scene->Add_Render_Object (Light);
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ViewerCtrlClass
//
////////////////////////////////////////////////////////////////
ViewerCtrlClass::~ViewerCtrlClass (void)
{
	Free_Model ();
	REF_PTR_RELEASE (Light);
	REF_PTR_RELEASE (Camera);
	REF_PTR_RELEASE (Scene);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderer
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Create_Control_Renderer (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	renderer.Enable_Texturing (false);

	ShaderClass *shader = renderer.Get_Shader ();
	renderer.Set_Z_Value (1.0F);
	shader->Set_Depth_Compare (ShaderClass::PASS_ALWAYS);
	shader->Set_Depth_Mask (ShaderClass::DEPTH_WRITE_ENABLE);
	shader->Set_Color_Mask (ShaderClass::COLOR_WRITE_ENABLE);


	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();
	if (IsEnabled == false) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	}

	//
	//	Draw the window frame
	//
	renderer.Add_Rect (Rect, 1.0F, color, bkcolor);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::On_Set_Cursor (const Vector2 &/* mouse_pos */)
{
	//
	//	Change the mouse cursor
	//
	MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ROTATE);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;

	//
	//	Calculate what the horizontal and vertical field of view
	// should be for this window.
	//
	float hfov = 0;
	float vfov = 0;
	float cx = Rect.Width ();
	float cy = Rect.Height ();
	if (cy > cx) {
		vfov = DEG_TO_RADF (45.0F);
		hfov = (cx / cy) * vfov;
	} else {
		hfov = DEG_TO_RADF (45.0F);
		vfov = (cy / cx) * hfov;
	}

	//
	//	Set the new view plane for the camera
	//
	Camera->Set_View_Plane (hfov, vfov);

	//
	//	Calculate a viewport that we can use to restraint
	// the model to our window.
	//
	const RectClass &screen_rect = Render2DClass::Get_Screen_Resolution ();
	Vector2 upper_left	= Rect.Upper_Left ();
	Vector2 lower_right	= Rect.Lower_Right ();

	upper_left.X	/= screen_rect.Width ();
	lower_right.X	/= screen_rect.Width ();
	upper_left.Y	/= screen_rect.Height ();
	lower_right.Y	/= screen_rect.Height ();

	//
	//	Set the viewport to render only to our window
	//
	Camera->Set_Viewport (upper_left, lower_right);

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Render (void)
{
	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Control_Renderer ();
	}

	//
	//	Render the background
	//
	if (IsBackgroundVisible) {
		ControlRenderer.Render ();
	}

	//
	//	Render the 3D object
	//
	if (Scene != NULL) {
		WW3D::Render (Scene, Camera);
	}

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::On_LButton_Down (const Vector2 &/* mouse_pos */)
{
	// Put the viewer interface into virtual trackball mode.
	Set_Interface_Mode (ViewerCtrlClass::VIRTUAL_TRACKBALL);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Model
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Set_Model (const char *model_name)
{
	//
	//	Load the new model
	//
	RenderObjClass *new_model = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (model_name);
	Set_Model (new_model);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Model
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Set_Model (RenderObjClass *new_model)
{
	//
	//	Start fresh
	//
	Free_Model ();

	if (new_model != NULL) {
		Model = new_model;

		//
		//	Notify any advise sinks
		//
		ADVISE_NOTIFY (On_ViewerCtrl_Model_Loaded (this, Get_ID (), Model));

		//
		//	Force the high LOD
		//
		int count = Model->Get_LOD_Count ();
		Model->Set_LOD_Level (std::max (count-1, 0));
		Model->Set_Transform (Matrix3D (1));

		//
		//	Add the model to the scene
		//
		Scene->Add_Render_Object (Model);

		//
		//	Force a camera update
		//
		IsCameraDirty = true;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Calculate_Camera_Position
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Calculate_Camera_Position (void)
{
	if (Model == NULL) {
		return ;
	}

	Model->Update_Obj_Space_Bounding_Volumes ();
	Model->Update_Sub_Object_Transforms ();

	//
	//	Get the bounds of this object
	//
	bool is_first = true;
	BoundingBox.Center.Set (0, 0, 0);
	BoundingBox.Extent.Set (0, 0, 0);
	Get_Visible_Bounding_Box (&BoundingBox, Model, is_first);

	//
	//	Get information about the camera
	//
	Vector2 vpmin, vpmax;
	Camera->Get_View_Plane (vpmin, vpmax);

	float znear = 0;
	float zfar = 0;
	Camera->Get_Clip_Planes (znear, zfar);

	//
	//	Calculate the closest distance we can get
	// without clipping the rotating object
	//
	float width			= WWMath::Sqrt (BoundingBox.Extent.X * BoundingBox.Extent.X * 4 + BoundingBox.Extent.Y * BoundingBox.Extent.Y * 4);
	float height		= BoundingBox.Extent.Z * 2;

	float near_width	= (vpmax.X - vpmin.X) * znear;
	float far_width	= (vpmax.X - vpmin.X) * zfar;

	float near_height	= (vpmax.Y - vpmin.Y) * znear;
	float far_height	= (vpmax.Y - vpmin.Y) * zfar;

	float percent1		= (width - near_width) / (far_width - near_width);
	float percent2		= (height - near_height) / (far_height - near_height);

	float test_z1		= znear + (zfar - znear) * percent1;
	float test_z2		= znear + (zfar - znear) * percent2;

	Distance				= (std::max (test_z1, test_z2) * 1.2F);
	Distance				= std::max (MinCameraDist, Distance);

	//
	//	Reposition the camera to be looking at this position
	//
	Vector3 center	= BoundingBox.Center;
	Vector3 pos		= center + Vector3 (Distance, 0, 0);
	Matrix3D tm;
	tm.Look_At (pos, center, 0);
	Camera->Set_Transform (tm);

	//
	//	Remember that's we've updated
	//
	IsCameraDirty = false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Animation
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Set_Animation (const char *anim_name)
{
	if (Model == NULL) {
		return ;
	}

	if (anim_name[0] != 0) {

		//
		//	Play the animation on the model
		//
		HAnimClass *anim = WW3DAssetManager::Get_Instance ()->Get_HAnim (anim_name);
		if (anim != NULL) {
			Model->Set_Animation (anim, 0, RenderObjClass::ANIM_MODE_LOOP);
			REF_PTR_RELEASE (anim);
		}

	} else {

		//
		//	Stop the animation
		//
		Model->Set_Animation ();
	}

	//
	//	Force a camera update
	//
	IsCameraDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Model
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Free_Model (void)
{
	if (Model != NULL) {
		Model->Remove ();
		REF_PTR_RELEASE (Model);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Interface_Mode
//
////////////////////////////////////////////////////////////////
void ViewerCtrlClass::Set_Interface_Mode (InterfaceModeEnum mode, float rotationrate)
{
	// If already in the desired mode do not reset the rotation.
	if (mode != InterfaceMode) {
		InterfaceMode = mode;
		ZRotation	  = DEG_TO_RADF (0.0f);
	}
	RotationRate = DEG_TO_RADF (rotationrate);	
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
ViewerCtrlClass::On_Frame_Update (void)
{
	//
	//	Update the camera's data if necessary
	//
	if (IsCameraDirty) {
		Calculate_Camera_Position ();
	}

	switch (InterfaceMode) {

		case Z_ROTATION:
		{
			float delta = ((DialogMgrClass::Get_Frame_Time () / 1000.0F) * RotationRate);

			//
			//	Adjust our rotation
			//
			ZRotation -= delta;
			if (ZRotation < 0) {
				ZRotation += DEG_TO_RADF (360);
			}

			//
			//	Rotate the camera's position about the object
			//
			Matrix3D rotation_tm (1);
			rotation_tm.Rotate_Z (ZRotation);
			Vector3 position = rotation_tm.Rotate_Vector (Vector3 (Distance, 0, 0)) + BoundingBox.Center;

			//
			//	Update the camera's position
			//
			Matrix3D tm;
			tm.Look_At (position, BoundingBox.Center, 0);
			Camera->Set_Transform (tm);
			break;
		}
		
		case VIRTUAL_TRACKBALL:

			if (DialogMgrClass::Is_Button_Down (VK_LBUTTON)) {

				const RectClass &rect = Get_Window_Rect();
				const Vector2	  mouseposition (DialogMgrClass::Get_Mouse_Pos().X, DialogMgrClass::Get_Mouse_Pos().Y);

				if (rect.Contains (mouseposition)) {

					int		  width, height, bits;
					bool		  windowed;
					float		  oow, ooh;	
					Vector2	  viewportmin, viewportmax, viewportextent, viewportcenter;  
					float		  a, b;	
					Vector2	  p0, p1;
					Quaternion rotation;
					Matrix3D	  transform, inversetransform;
					Vector3	  objectcenter;

					WW3D::Get_Render_Target_Resolution (width, height, bits, windowed);
					oow = 1.0f / width;
					ooh = 1.0f / height;

					Camera->Get_Viewport (viewportmin, viewportmax);
					viewportextent = viewportmax - viewportmin;
					viewportcenter = 0.5f * (viewportmin + viewportmax);
					a =  1.0f / viewportextent.X;
					b = -1.0f / viewportextent.Y;
					
					p0.Set (LastMousePosition.X, LastMousePosition.Y);
					p0.Scale (oow, ooh);
					p0 -= viewportcenter;
					p0.Scale (a, b);
					
					p1.Set (DialogMgrClass::Get_Mouse_Pos().X, DialogMgrClass::Get_Mouse_Pos().Y);
					p1.Scale (oow, ooh);
					p1 -= viewportcenter;
					p1.Scale (a, b);
					
					rotation = ::Trackball (p0.X, p0.Y, p1.X, p1.Y, 0.5f);
					
					transform = Camera->Get_Transform();
					transform.Get_Orthogonal_Inverse (inversetransform);
					objectcenter = inversetransform * BoundingBox.Center;
					transform.Translate (objectcenter);
					Matrix3D::Multiply (transform, Build_Matrix3D (rotation), &transform);
					transform.Translate (-objectcenter);

					Camera->Set_Transform (transform);
				}
			}
			break;
	}

	// Place the light source at the camera position.
	Light->Set_Transform (Camera->Get_Transform());
	
	LastMousePosition = DialogMgrClass::Get_Mouse_Pos();
	DialogControlClass::On_Frame_Update ();
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Get_Visible_Bounding_Box
//
///////////////////////////////////////////////////////////////
void
ViewerCtrlClass::Get_Visible_Bounding_Box (AABoxClass *box, RenderObjClass *render_obj, bool &is_first)
{
	if (render_obj == NULL) {
		return ;
	}

	//
	// Recursively walk through the subobjects
	//
	for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
		RenderObjClass *sub_obj = render_obj->Get_Sub_Object (index);

		//
		//	Recurse into this sub-object
		//
		if (sub_obj != NULL && (sub_obj->Is_Hidden () == false)) {
			Get_Visible_Bounding_Box (box, sub_obj, is_first);
		}
		REF_PTR_RELEASE (sub_obj);
	}

	//
	//	Is this a mesh?
	//
	if (render_obj->Class_ID () == RenderObjClass::CLASSID_MESH) {
		MeshClass *mesh = reinterpret_cast<MeshClass *> (render_obj);

		//
		//	Dig out the mesh's model
		//
		MeshModelClass *mesh_model = mesh->Get_Model ();
		if (mesh_model != NULL) {

			//
			//	Get the vertex count
			//
			int vertex_count			= mesh_model->Get_Vertex_Count ();
			Vector3 *vertex_array	= NULL;

			//
			//	Get a pointer to the vertices
			//
			bool is_skin = (mesh_model->Get_Flag (MeshGeometryClass::SKIN) != 0);
			if (is_skin) {
				vertex_array = new Vector3[vertex_count];
				mesh->Get_Deformed_Vertices (vertex_array);
			} else {
				vertex_array = mesh_model->Get_Vertex_Array ();
			}
			
			//
			//	Loop over all the verts inside this model
			//			
			for (int index = 0; index < vertex_count; index ++) {
				
				//
				//	Get the world-space vertex position
				//
				Vector3 vertex_pos = vertex_array[index];
				if (is_skin == false) {
					vertex_pos = mesh->Get_Transform () * vertex_array[index];
				}

				//
				//	Expand our box to account for this vertex
				//
				if (is_first) {
					box->Center = vertex_pos;
					is_first = false;
				} else {
					box->Add_Point (vertex_pos);
				}
			}

			//
			//	Free the vertex array (if necessary)
			//
			if (is_skin) {
				delete [] vertex_array;
				vertex_array = NULL;
			}

			REF_PTR_RELEASE (mesh_model);
		}
	}

	return ;
}
