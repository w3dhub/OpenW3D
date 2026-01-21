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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/BuildingNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/13/01 9:44a                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "buildingnode.h"
#include "buildingchildnode.h"
#include "sceneeditor.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "preset.h"
#include "editorchunkids.h"
#include "modelutils.h"
#include "vehiclefactorygameobj.h"
#include "refinerygameobj.h"
#include "nodemgr.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<BuildingNodeClass, CHUNKID_NODE_BUILDING> _BuildingNodePersistFactory;


enum
{
	CHUNKID_VARIABLES			= 0x09071125,
	CHUNKID_BASE_CLASS,
};

enum
{
	VARID_VISOBJECTID			= 0x01,
	VARID_VISSECTORID,
	VARID_CHILD_NODE
};


//////////////////////////////////////////////////////////////////////////////
//
//	BuildingNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingNodeClass::BuildingNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		ObjectNodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	BuildingNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingNodeClass::BuildingNodeClass (const BuildingNodeClass &src)
	:	m_PhysObj (NULL),
		ObjectNodeClass (NULL)
{
	(*this) = src;	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~BuildingNodeClass
//
//////////////////////////////////////////////////////////////////////////////
BuildingNodeClass::~BuildingNodeClass (void)
{	
	Free_Child_Nodes ();
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
BuildingNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);

	//
	//	Load the model we want to use to identify the building controller
	//
	RenderObjClass *render_obj = ::Create_Render_Obj ("BUILDINGICON");
	if (render_obj != NULL) {
		
		//
		// Create the new physics object
		//
		m_PhysObj = new DecorationPhysClass;
		m_PhysObj->Set_Model (render_obj);
		m_PhysObj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);				
		m_PhysObj->Set_Transform (m_Transform);
		::Set_Model_Collision_Type (m_PhysObj->Peek_Model (), COLLISION_TYPE_0);
		render_obj->Release_Ref ();
	}

	ObjectNodeClass::Initialize ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
BuildingNodeClass::Get_Factory (void) const
{	
	return _BuildingNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const BuildingNodeClass &
BuildingNodeClass::operator= (const BuildingNodeClass &src)
{
	//
	//	Start fresh
	//
	Free_Child_Nodes ();

	//
	//	Copy the child node list
	//
	for (int index = 0; index < src.m_ChildNodes.Count (); index ++) {
		NodeClass *child_node = src.m_ChildNodes[index];
		if (child_node != NULL) {
			Add_Child_Node (child_node->Get_Transform ());
		}
	}

	NodeClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
BuildingNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		ObjectNodeClass::Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		//
		//	Save the list of child node transforms
		//
		for (int index = 0; index < m_ChildNodes.Count (); index ++) {
			NodeClass *child_node = m_ChildNodes[index];
			if (child_node != NULL) {
				Matrix3D tm = child_node->Get_Transform ();
				WRITE_MICRO_CHUNK (csave, VARID_CHILD_NODE, tm);
			}
		}

	csave.End_Chunk ();

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
BuildingNodeClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				ObjectNodeClass::Load (cload);
				break;
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
bool
BuildingNodeClass::Load_Variables (ChunkLoadClass &cload)
{	
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			case VARID_CHILD_NODE:
			{
				//
				//	Read the child node's  transfrom from the chunk
				//
				Matrix3D tm;
				cload.Read (&tm, sizeof (tm));

				//
				//	Add a new child node at this location
				//
				Add_Child_Node (tm);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Pre_Export (void)
{
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);

		//
		//	Configure the building object (if necessary)
		//
		BuildingGameObj *building = Get_Building ();
		if (building != NULL) {
			
			//
			//	What type of building is this?
			//
			if (building->As_VehicleFactoryGameObj () != NULL) {
				VehicleFactoryGameObj *airstrip = building->As_VehicleFactoryGameObj ();
				
				//
				//	Configure the factory's creation transform
				//
				if (m_ChildNodes.Count () > 0) {
					airstrip->Set_Creation_TM (m_ChildNodes[0]->Get_Transform ());
				}

			} else if (building->As_RefineryGameObj () != NULL) {
				RefineryGameObj *refinery = building->As_RefineryGameObj ();

				//
				//	Configure the refinery's docking transform
				//
				if (m_ChildNodes.Count () > 0) {
					refinery->Set_Dock_TM (m_ChildNodes[0]->Get_Transform ());
				}							
			}
		}


		//
		//	Pass the Pre_Export call onto any child nodes as well
		//
		for (int index = 0; index < m_ChildNodes.Count (); index ++) {
			EditorLineClass *line	= m_ChildLines[index];

			//
			//	Remove the line from the scene
			//	
			::Get_Scene_Editor ()->Remove_Object (line);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Post_Export (void)
{
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);

		//
		//	Pass the Post_Export call onto any child nodes as well
		//
		for (int index = 0; index < m_ChildNodes.Count (); index ++) {
			EditorLineClass *line	= m_ChildLines[index];

			//
			//	Add the line back into the world
			//	
			::Get_Scene_Editor ()->Add_Dynamic_Object (line);
		}
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Child_Node
//
//////////////////////////////////////////////////////////////////////
NodeClass *
BuildingNodeClass::Add_Child_Node (const Matrix3D &tm)
{
	BuildingChildNodeClass *child_node = new BuildingChildNodeClass;
	child_node->Initialize ();
	child_node->Set_Transform (tm);
	child_node->Set_Building (this);
	NodeMgrClass::Setup_Node_Identity (*child_node);
	m_ChildNodes.Add (child_node);

	//
	//	Create and add the line from the building to the child node
	//
	EditorLineClass *line = new EditorLineClass;	
	line->Reset (m_Transform.Get_Translation (), tm.Get_Translation ());
	m_ChildLines.Add (line);

	//
	//	Add the objects to the scene if necessary
	//
	if (m_IsInScene) {
		child_node->Add_To_Scene ();
		::Get_Scene_Editor ()->Add_Dynamic_Object (line);
	}

	return child_node;
}


//////////////////////////////////////////////////////////////////////
//
//	Can_Add_Child_Nodes
//
//////////////////////////////////////////////////////////////////////
bool
BuildingNodeClass::Can_Add_Child_Nodes (void) const
{
	bool retval = false;

	BuildingGameObj *building = Get_Building ();
	if (building != NULL) {
		
		//
		//	Is this one of the types of buildings which need 
		// child nodes?
		//
		if (building->As_VehicleFactoryGameObj () != NULL) {
			
			//
			//	This type of building can only have one child node
			//
			if (m_ChildNodes.Count () == 0) {
				retval = true;
			}

		} else if (building->As_RefineryGameObj () != NULL) {			

			//
			//	This type of building can only have one child node
			//
			if (m_ChildNodes.Count () == 0) {
				retval = true;
			}

		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_Child_Node
//
//////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Remove_Child_Node (NodeClass *child_node)
{
	//
	//	Try to find the child node
	//
	for (int index = 0; index < m_ChildNodes.Count (); index ++) {
		if (child_node == m_ChildNodes[index]) {
			
			//
			//	Free the child node
			//
			MEMBER_RELEASE (child_node);
			m_ChildNodes.Delete (index);
			
			//
			//	Remove and free the line to the child node
			//
			::Get_Scene_Editor ()->Remove_Object (m_ChildLines[index]);
			m_ChildLines[index]->Release_Ref ();
			m_ChildLines.Delete (index);
			break;
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Child_Nodes
//
//////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Free_Child_Nodes (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Release our hold on each child node
	//
	for (int index = 0; index < m_ChildNodes.Count (); index ++) {
		NodeClass *child_node	= m_ChildNodes[index];
		EditorLineClass *line	= m_ChildLines[index];
		
		scene->Remove_Object (line);
		child_node->Remove_From_Scene ();
		MEMBER_RELEASE (child_node);
		MEMBER_RELEASE (line);
	}
	
	//
	//	Remove all the child nodes from the list
	//
	m_ChildNodes.Delete_All ();
	m_ChildLines.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Add_To_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Add all the waypoints to the scene
	//
	for (int index = 0; index < m_ChildNodes.Count (); index ++) {
		NodeClass *child_node	= m_ChildNodes[index];
		EditorLineClass *line	= m_ChildLines[index];
		child_node->Add_To_Scene ();
		scene->Add_Dynamic_Object (line);
	}

	NodeClass::Add_To_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Remove_From_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL && m_IsInScene) {

		//
		//	Remove all the waypoints from the scene
		//
		for (int index = 0; index < m_ChildNodes.Count (); index ++) {
			NodeClass *child_node	= m_ChildNodes[index];
			EditorLineClass *line	= m_ChildLines[index];
			child_node->Remove_From_Scene ();
			scene->Remove_Object (line);
		}
	}
	
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Lines
//
//////////////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Update_Lines (void)
{
	for (int index = 0; index < m_ChildNodes.Count (); index ++) {
		NodeClass *child_node	= m_ChildNodes[index];
		EditorLineClass *line	= m_ChildLines[index];
		
		//
		//	Update the line between the node and its child
		//
		line->Reset (Get_Transform ().Get_Translation (), child_node->Get_Position ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////////////
void
BuildingNodeClass::Hide (bool hide)
{
	//
	//	Apply the same operation to all the child nodes and lines
	//
	for (int index = 0; index < m_ChildNodes.Count (); index ++) {
		NodeClass *child_node	= m_ChildNodes[index];
		EditorLineClass *line	= m_ChildLines[index];
		child_node->Hide (hide);
		line->Hide (hide);
	}
	
	NodeClass::Hide (hide);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Sub_Node
//
//////////////////////////////////////////////////////////////////////////////
NodeClass *
BuildingNodeClass::Get_Sub_Node (int index)
{
	return m_ChildNodes[index];
}
