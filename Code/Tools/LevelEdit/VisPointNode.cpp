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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisPointNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/26/01 2:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "vispointnode.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "chunkio.h"
#include "visenum.h"
#include "camera.h"
#include "ccamera.h"
#include "combat.h"
#include "nodemgr.h"
#include "modelutils.h"

//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<VisPointNodeClass, CHUNKID_NODE_VIS_POINT> _VisPointodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x11011130,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_TILE_LOCATION	= 0x01,
	VARID_NEAR_CLIP_DOUBLE,
	VARID_HFOV,
	VARID_VFOV,
	VARID_NEAR_CLIP
};


//////////////////////////////////////////////////////////////////////////////
//
//	VisPointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
VisPointNodeClass::VisPointNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_VisTileLocation (0, 0, 0),
		m_NearClipPlane (0),
		m_HFov (0),
		m_VFov (0),
		NodeClass (preset)		
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	VisPointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
VisPointNodeClass::VisPointNodeClass (const VisPointNodeClass &src)
	:	m_PhysObj (NULL),
		m_VisTileLocation (0, 0, 0),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~VisPointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
VisPointNodeClass::~VisPointNodeClass (void)
{	
	Remove_From_Scene ();
	MEMBER_RELEASE (m_PhysObj);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//	Note:  This may be called more than once.  It is used as an 'initialize'
// and a 're-initialize'.
//
//////////////////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);

	//
	//	Create the camera render object
	//
	RenderObjClass *render_obj = ::Create_Render_Obj ("CAMERA");
	WWASSERT (render_obj != NULL);
	if (render_obj != NULL) {
		
		// Create the new physics object
		m_PhysObj = new DecorationPhysClass;
		
		//
		// Configure the physics object with information about
		// its new render object and collision data.
		//
		m_PhysObj->Set_Model (render_obj);
		m_PhysObj->Set_Transform (Matrix3D(1));
		m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
		m_PhysObj->Set_Transform (m_Transform);
		::Set_Model_Collision_Type (m_PhysObj->Peek_Model (), COLLISION_TYPE_6);
		
		// Release our hold on the render object pointer
		MEMBER_RELEASE (render_obj);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
VisPointNodeClass::Get_Factory (void) const
{	
	return _VisPointodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
VisPointNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_TILE_LOCATION, m_VisTileLocation);
		WRITE_MICRO_CHUNK (csave, VARID_NEAR_CLIP, m_NearClipPlane);
		WRITE_MICRO_CHUNK (csave, VARID_HFOV, m_HFov);
		WRITE_MICRO_CHUNK (csave, VARID_VFOV, m_VFov);
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
VisPointNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
VisPointNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_TILE_LOCATION, m_VisTileLocation);
			READ_MICRO_CHUNK (cload, VARID_NEAR_CLIP, m_NearClipPlane);
			READ_MICRO_CHUNK (cload, VARID_HFOV, m_HFov);
			READ_MICRO_CHUNK (cload, VARID_VFOV, m_VFov);

			case VARID_NEAR_CLIP_DOUBLE:
			{
				double double_value = 0;
				cload.Read (&double_value, sizeof (double_value));
				m_NearClipPlane = double_value;
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Copy the game camera's settings if we hadn't saved the settings
	// the last time.
	//
	if (m_NearClipPlane == 0 || m_HFov == 0 || m_VFov == 0) {
		m_NearClipPlane = 0.5;
		m_HFov = 0.8726F;
		m_VFov = 0.6544F;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
///////////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Set_Transform (const Matrix3D &tm)
{
	NodeClass::Set_Transform (tm);
	m_VisTileLocation = m_Transform.Get_Translation ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const VisPointNodeClass &
VisPointNodeClass::operator= (const VisPointNodeClass &src)
{
	m_VisTileLocation = src.m_VisTileLocation;
	NodeClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////
//
//	Save_Camera_Settings
//
/////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Save_Camera_Settings (const CameraClass &camera)
{
	float far_clip = 0;
	camera.Get_Clip_Planes (m_NearClipPlane, far_clip);
	m_HFov = camera.Get_Horizontal_FOV ();
	m_VFov = camera.Get_Vertical_FOV ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Setup_Camera
//
/////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Setup_Camera (CameraClass &camera)
{
	camera.Set_View_Plane (m_HFov, m_VFov);
	camera.Set_Clip_Planes (m_NearClipPlane, VIS_FAR_CLIP);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
VisPointNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);
	}
	Release_Ref ();
	return ;
}

