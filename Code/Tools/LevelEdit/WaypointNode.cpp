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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypointNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:07a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "WaypathNode.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "chunkio.h"
#include "nodemgr.h"
#include "segline.h"
#include "WaypointNode.h"
#include "WaypathNode.h"
#include "waypathinfopage.h"
#include "nodeinfopage.h"
#include "positionpage.h"
#include "editorpropsheet.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<WaypointNodeClass, CHUNKID_NODE_WAYPOINT> _WaypointNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x03011130,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_WAYPATH_PTR			= 0x01,
	VARID_OLD_PTR				= 0x02,
	VARID_FLAGS					= 0x03,
};


//////////////////////////////////////////////////////////////////////////////
//
//	WaypointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypointNodeClass::WaypointNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_Waypath (NULL),
		m_Flags (NULL),
		m_ModelType (MODEL_MIDDLE_PT),
		m_Speed (0.6F),
		NodeClass (preset)		
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	WaypointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypointNodeClass::WaypointNodeClass (const WaypointNodeClass &src)
	:	m_PhysObj (NULL),
		m_Waypath (NULL),
		m_Flags (NULL),
		m_ModelType (MODEL_MIDDLE_PT),
		m_Speed (0.6F),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~WaypointNodeClass
//
//////////////////////////////////////////////////////////////////////////////
WaypointNodeClass::~WaypointNodeClass (void)
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
WaypointNodeClass::Initialize (void)
{
	//
	//	Refresh the model
	//
	MEMBER_RELEASE (m_PhysObj);
	Update_Model ();
	return ;
}	


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Model
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Set_Model (MODEL model)
{
	if (m_ModelType != model) {
		m_ModelType = model;
		Update_Model ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Model
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Update_Model (void)
{
	if (m_PhysObj == NULL) {
		m_PhysObj = new DecorationPhysClass;
	}

	StringClass model_name;
	switch (m_ModelType)
	{
		case MODEL_START_PT:
			model_name = "WAY_G";
			break;

		case MODEL_MIDDLE_PT:
			model_name = "WAY_B";
			break;

		case MODEL_END_PT:
			model_name = "WAY_R";
			break;
	}

	//
	//	Create the appropriate model
	//
	RenderObjClass *render_obj = ::Create_Render_Obj (model_name);
	WWASSERT (render_obj != NULL);
	if (render_obj != NULL) {

		//
		//	Pass the model onto the physics object
		//
		m_PhysObj->Set_Model (render_obj);					

		//
		//	Configure the physics object
		//
		m_PhysObj->Set_Transform (Matrix3D(1));
		m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, FALSE);
		m_PhysObj->Peek_Model ()->Set_Collision_Type (COLLISION_TYPE_6);
		m_PhysObj->Set_Transform (m_Transform);

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
WaypointNodeClass::Get_Factory (void) const
{	
	return _WaypointNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
WaypointNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);		
		
		WaypointNodeClass *this_ptr = this;		
		WRITE_MICRO_CHUNK (csave, VARID_OLD_PTR, this_ptr);
		WRITE_MICRO_CHUNK (csave, VARID_WAYPATH_PTR, m_Waypath);
		WRITE_MICRO_CHUNK (csave, VARID_FLAGS, m_Flags);
				
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
WaypointNodeClass::Load (ChunkLoadClass &cload)
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
WaypointNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_FLAGS, m_Flags);

			case VARID_WAYPATH_PTR:
			{
				//
				//	Read the old pointer from the chunk and register it
				// for remapping.
				//				
				cload.Read (&m_Waypath, sizeof (m_Waypath));				
				REQUEST_POINTER_REMAP ((void **)&m_Waypath);
			}
			break;

			case VARID_OLD_PTR:
			{
				//
				//	Read the old pointer from the chunk and submit it
				// to the remapping system.
				//				
				WaypointNodeClass *old_ptr = NULL;
				cload.Read (&old_ptr, sizeof (old_ptr));
				SaveLoadSystemClass::Register_Pointer (old_ptr, this);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Transform
//
///////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::On_Transform (void)
{
	On_Translate ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Translate
//
///////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::On_Translate (void)
{
	if (m_Waypath != NULL) {
		
		//
		//	Find where we are on the waypath
		//
		int index = m_Waypath->Find_Index (this);
		if (index != -1) {
			
			//
			//	Notify our waypath that we have been moved
			//
			const Matrix3D &tm = Get_Transform ();
			m_Waypath->On_Point_Moved (index, tm.Get_Translation ());
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	On_Delete
//
/////////////////////////////////////////////////////////////////
void
WaypointNodeClass::On_Delete (void)
{
	if (m_Waypath != NULL) {
		
		//
		//	Find where we are on the waypath
		//
		int index = m_Waypath->Find_Index (this);
		if (index != -1) {
			
			//
			//	Detach ourselves from the waypath
			//
			m_Waypath->Delete_Point (index);
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const WaypointNodeClass &
WaypointNodeClass::operator= (const WaypointNodeClass &src)
{
	m_Speed		= src.m_Speed;
	m_Flags		= src.m_Flags;
	m_ModelType	= src.m_ModelType;

	NodeClass::operator= (src);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Pre_Export (void)
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
WaypointNodeClass::Post_Export (void)
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


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flags
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Set_Flags (int flags)
{
	m_Flags = flags;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flag
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Set_Flag (int flag, bool onoff)
{
	m_Flags &= ~flag;
	if (onoff) {
		m_Flags |= flag;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Flag
//
//////////////////////////////////////////////////////////////////////////////
bool
WaypointNodeClass::Get_Flag (int flag)
{
	return bool((m_Flags & flag) == flag);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
//////////////////////////////////////////////////////////////////////////////
bool
WaypointNodeClass::Show_Settings_Dialog (void)
{
	NodeInfoPageClass	info_tab (this);
	PositionPageClass	pos_tab (this);
	WaypathInfoPageClass path_tab (this);
	
	//
	//	Add each tab to the property sheet
	//
	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&path_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	if (ret_code == IDOK && m_Waypath != NULL) {
		m_Waypath->Update_Line ();
	}
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Parent_Set_Transform
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Parent_Set_Transform (const Matrix3D &tm)
{
	m_Transform = tm;

	PhysClass *phys_obj = Peek_Physics_Obj ();		
	if (phys_obj != NULL) {
		phys_obj->Set_Transform (tm);		
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Parent_Set_Position
//
//////////////////////////////////////////////////////////////////////////////
void
WaypointNodeClass::Parent_Set_Position (const Vector3 &pos)
{
	m_Transform.Set_Translation (pos);

	PhysClass *phys_obj = Peek_Physics_Obj ();		
	if (phys_obj != NULL) {
		phys_obj->Set_Transform (m_Transform);		
	}

	return ;
}
