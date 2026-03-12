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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/AttenuationSphere.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/04/00 9:58a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "AttenuationSphere.h"
#include "SphereObj.h"
#include "Utils.h"
#include "Node.h"
#include "SceneEditor.h"


///////////////////////////////////////////////////////////////////////////////
//
//	AttenuationSphereClass
//
///////////////////////////////////////////////////////////////////////////////
AttenuationSphereClass::AttenuationSphereClass (void)
	:	m_IsInScene (false),
		m_Color (1, 1, 1),
		m_Radius (1.0F),
		m_Opacity (1.0F)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	~AttenuationSphereClass
//
///////////////////////////////////////////////////////////////////////////////
AttenuationSphereClass::~AttenuationSphereClass (void)
{
	Remove_From_Scene ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Display_Around_Node
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Display_Around_Node (const NodeClass &node)
{
	RenderObjClass *render_obj = node.Peek_Render_Obj ();
	if (render_obj != NULL) {
		Display_Around_Node (*render_obj);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Display_Around_Node
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Display_Around_Node (const RenderObjClass &render_obj)
{
	//
	//	Make sure we have a sphere object
	//
	SphereRenderObjClass *sphere = (SphereRenderObjClass *)Peek_Model ();
	if (sphere == NULL) {
		sphere = new SphereRenderObjClass;
		sphere->Set_Flag (SphereRenderObjClass::USE_ALPHA_VECTOR, false);
		Set_Model (sphere);

		Set_Color (m_Color);
		Set_Radius (m_Radius);
		Set_Opacity (m_Opacity);

		MEMBER_RELEASE (sphere);
	}

	//
	//	Update the sphere's position
	//
	Set_Transform (render_obj.Get_Transform ());

	//
	//	Put the sphere into the scene
	//
	if (m_IsInScene == false) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (this);
		m_IsInScene = true;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Remove_From_Scene (void)
{
	//
	//	Remove the sphere from the scene
	//
	if (m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (this);
		m_IsInScene = false;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Set_Color
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Set_Color (const Vector3 &color)
{
	SphereRenderObjClass *sphere = (SphereRenderObjClass *)Peek_Model ();
	SANITY_CHECK (sphere != NULL) {
		return ;
	}

	m_Color = color;
	sphere->Set_Color (m_Color);
	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Set_Radius
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Set_Radius (float radius)
{
	SphereRenderObjClass *sphere = (SphereRenderObjClass *)Peek_Model ();
	SANITY_CHECK (sphere != NULL) {
		return ;
	}

	m_Radius = radius;
	sphere->Set_Extent (Vector3 (m_Radius, m_Radius, m_Radius));
	sphere->Set_Transform (sphere->Get_Transform ());
	Update_Cull_Box ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Set_Opacity
//
///////////////////////////////////////////////////////////////////////////////
void
AttenuationSphereClass::Set_Opacity (float opacity)
{
	SphereRenderObjClass *sphere = (SphereRenderObjClass *)Peek_Model ();
	SANITY_CHECK (sphere != NULL) {
		return ;
	}

	m_Opacity = opacity;
	sphere->Set_Alpha (m_Opacity);
	return ;
}


