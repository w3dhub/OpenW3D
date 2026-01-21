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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SpawnerNode.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/13/01 9:44a                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "spawnernode.h"
#include "sceneeditor.h"

#include "filemgr.h"
#include "_assetmgr.h"
#include "editorassetmgr.h"
#include "w3d_file.h"
#include "cameramgr.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "spawn.h"
#include "physicalgameobj.h"
#include "presetmgr.h"
#include "decophys.h"
#include "nodemgr.h"
#include "modelutils.h"
#include "spawnpointnode.h"
#include "nodescriptsproppage.h"
#include "nodeinfopage.h"
#include "editorpropsheet.h"
#include "positionpage.h"
#include "editscript.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<SpawnerNodeClass, CHUNKID_NODE_SPAWNER> _SpawnerNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Save/load constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x10251200,
	CHUNKID_BASE_CLASS,
	CHUNKID_SCRIPT
};

enum
{
	VARID_SPAWN_POINT			= 0x01,
};


//////////////////////////////////////////////////////////////////////////////
//
//	SpawnerNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SpawnerNodeClass::SpawnerNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_SpawnerObj (NULL),
		NodeClass (preset)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	SpawnerNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SpawnerNodeClass::SpawnerNodeClass (const SpawnerNodeClass &src)
	:	m_PhysObj (NULL),
		m_SpawnerObj (NULL),
		NodeClass (NULL)
{
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~SpawnerNodeClass
//
//////////////////////////////////////////////////////////////////////////////
SpawnerNodeClass::~SpawnerNodeClass (void)
{	
	Free_Spawn_Points ();
	Remove_From_Scene ();
	Free_Scripts ();
	MEMBER_RELEASE (m_PhysObj);
	SAFE_DELETE (m_SpawnerObj);
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
SpawnerNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);
	SAFE_DELETE (m_SpawnerObj);

	SpawnerDefClass *definition = static_cast<SpawnerDefClass *> (m_Preset->Get_Definition ());
	if (definition != NULL) {

		//
		//	Create the spawner object object
		//			
		Load_Assets ();
		m_Preset->Load_All_Assets ();
		Create_Spawner_Obj ();
		
		//
		//	Get a model to represet the spawner with
		//
		RenderObjClass *model = Get_Spawned_Model ();
		if (model != NULL) {

			//
			//	Create our own physics object around the visual representation
			// of the game object.
			//
			m_PhysObj = new DecorationPhysClass;					
			m_PhysObj->Set_Model (model);
			model->Set_User_Data ((PVOID)&m_HitTestInfo, false);
			::Set_Model_Collision_Type (model, COLLISION_TYPE_6);
			MEMBER_RELEASE (model);

			//
			//	Make sure the physics object has the correct position
			//
			Set_Transform (m_Transform);			
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SpawnerNodeClass::Get_Factory (void) const
{	
	return _SpawnerNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
SpawnerNodeClass::Save (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	//
	//	Save the list of scripts
	//
	int index;
	for (index = 0; index < m_Scripts.Count (); index ++) {
		EditScriptClass *script = m_Scripts[index];
		
		//
		//	Have this script save itself
		//
		csave.Begin_Chunk (CHUNKID_SCRIPT);
		script->Save (csave);
		csave.End_Chunk ();
	}

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		//
		//	Save the list of spawn points transforms
		//
		for (index = 0; index < m_SpawnPointNodes.Count (); index ++) {
			SpawnPointNodeClass *spawn_point = m_SpawnPointNodes[index];
			if (spawn_point != NULL) {
				Matrix3D tm = spawn_point->Get_Transform ();
				WRITE_MICRO_CHUNK (csave, VARID_SPAWN_POINT, tm);
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
SpawnerNodeClass::Load (ChunkLoadClass &cload)
{
	Free_Spawn_Points ();
	Free_Scripts ();

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				NodeClass::Load (cload);
				break;

			case CHUNKID_SCRIPT:
			{
				EditScriptClass *script = new EditScriptClass;
				if (script->Load (cload)) {
					m_Scripts.Add (script);
				}
			}
			break;
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::On_Post_Load (void)
{
	//
	//	If the spawner isn't valid, then remove it from the system
	//
	if (m_Preset == NULL) {
		::Get_Scene_Editor ()->Delete_Node (this, false);
	} else {

		//
		//	Create the spawner object object
		//			
		Load_Assets ();
		m_Preset->Load_All_Assets ();
		Create_Spawner_Obj ();

		//
		//	Add each spawn point at the given locations
		//
		for (int index = 0; index < m_SpawnPointLoadList.Count (); index ++) {
			Add_Spawn_Point (m_SpawnPointLoadList[index]);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
SpawnerNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			case VARID_SPAWN_POINT:
			{
				//
				//	Read the spawn point's transfrom from the chunk
				//
				Matrix3D tm;
				cload.Read (&tm, sizeof (tm));
				m_SpawnPointLoadList.Add (tm);

				//Add_Spawn_Point (tm);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}



////////////////////////////////////////////////////////////////
//
//	Get_Spawned_Model
//
////////////////////////////////////////////////////////////////
RenderObjClass *
SpawnerNodeClass::Get_Spawned_Model (void)
{
	RenderObjClass *model = NULL;

	if (m_SpawnerObj != NULL) {
		
		//
		//	Spawn an object
		//
		PhysicalGameObj *game_obj = m_SpawnerObj->Create_Spawned_Object ();
		if (game_obj != NULL) {
			
			//
			//	Dig the render object out of the game object
			//
			PhysClass *phys_obj				= game_obj->Peek_Physical_Object ();
			RenderObjClass *render_obj		= phys_obj->Peek_Model ();
			
			//
			//	Make a copy of the render object to return to the caller
			//
			model	= render_obj->Clone ();

			//
			//	We don't need the game object, we just need the visual representation
			// of the gameobject
			//
			game_obj->Set_Delete_Pending ();
		}
	}

	return model;
}


////////////////////////////////////////////////////////////////
//
//	Load_Assets
//
////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Load_Assets (void)
{
	if (m_Preset != NULL) {
		SpawnerDefClass *definition = static_cast<SpawnerDefClass *> (m_Preset->Get_Definition ());
		if (definition != NULL) {

			//
			//	Loop over all the possible spawned objects and make sure to load their assets
			//
			const DynamicVectorClass<int> &list = definition->Get_Spawn_Definition_ID_List ();
			for (int index = 0; index < list.Count (); index ++) {
				PresetClass *preset = PresetMgrClass::Find_Preset (list[index]);
				if (preset != NULL) {
					preset->Load_All_Assets ();
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Spawned_Definition_ID
//
////////////////////////////////////////////////////////////////
uint32
SpawnerNodeClass::Get_Spawned_Definition_ID (void)
{
	uint32 id = 0;

	if (m_Preset != NULL) {
		SpawnerDefClass *definition = static_cast<SpawnerDefClass *> (m_Preset->Get_Definition ());
		if (definition != NULL) {
			const DynamicVectorClass<int> &list = definition->Get_Spawn_Definition_ID_List ();
			if (list.Count () > 0) {
				id = list[0];
			}
		}
	}

	return id;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const SpawnerNodeClass &
SpawnerNodeClass::operator= (const SpawnerNodeClass &src)
{
	NodeClass::operator= (src);

	//
	//	Make sure we have the spawner object created
	//			
	Load_Assets ();
	m_Preset->Load_All_Assets ();
	Create_Spawner_Obj ();

	//
	//	Copy the spawn point list
	//
	Free_Spawn_Points ();
	for (int index = 0; index < src.m_SpawnPointNodes.Count (); index ++) {
		SpawnPointNodeClass *spawn_point = src.m_SpawnPointNodes[index];
		if (spawn_point != NULL) {
			Add_Spawn_Point (spawn_point->Get_Transform ());
		}
	}

	//
	//	Copy the script list from the source object
	//
	Copy_Scripts (src);
	
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		scene->Remove_Object (m_PhysObj);

		//
		//	Get the fresh spawner list
		//
		SpawnPointListType *list = m_SpawnerObj->Get_Spawn_Point_List ();
		list->Delete_All ();

		//
		//	Process each spawn point
		//
		for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
			SpawnPointNodeClass *spawn_point	= m_SpawnPointNodes[index];
			EditorLineClass *line				= m_SpawnPointLines[index];			
			
			//
			//	Remove the editor only objects from the world
			//
			scene->Remove_Object (line);

			//
			//	Add this spawn point to the spawner
			//
			list->Add (spawn_point->Get_Transform ());
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
SpawnerNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		scene->Add_Dynamic_Object (m_PhysObj);

		//
		//	Process each spawn point
		//
		for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
			EditorLineClass *line				= m_SpawnPointLines[index];			
			
			//
			//	Remove the editor only objects from the world
			//
			scene->Add_Dynamic_Object (line);
		}
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Add_To_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	Create_Spawner_Obj ();

	//
	//	Add all the spawn points to the scene
	//
	for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
		SpawnPointNodeClass *spawn_point	= m_SpawnPointNodes[index];
		EditorLineClass *line				= m_SpawnPointLines[index];
		spawn_point->Add_To_Scene ();
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
SpawnerNodeClass::Remove_From_Scene (void)
{
	SAFE_DELETE (m_SpawnerObj);

	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL && m_IsInScene) {

		//
		//	Remove all the spawn points from the scene
		//
		for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
			SpawnPointNodeClass *spawn_point	= m_SpawnPointNodes[index];
			EditorLineClass *line				= m_SpawnPointLines[index];
			spawn_point->Remove_From_Scene ();
			scene->Remove_Object (line);
		}
	}
	
	NodeClass::Remove_From_Scene ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Spawner_Obj
//
//////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Create_Spawner_Obj (void)
{
	if (m_SpawnerObj == NULL) {

		// Recreate the spawner object
		SpawnerDefClass *definition = static_cast<SpawnerDefClass *> (m_Preset->Get_Definition ());
		if (definition != NULL) {
			m_SpawnerObj = (SpawnerClass *)::Instance_Definition (definition);
			if (m_SpawnerObj != NULL) {
				m_SpawnerObj->Set_TM (m_Transform);
				m_SpawnerObj->Set_ID (m_ID);
			}
		}

		Assign_Scripts ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID
//
/////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Set_ID (uint32 id)
{
	NodeClass::Set_ID (id);
	if (m_SpawnerObj != NULL) {
		m_SpawnerObj->Set_ID (m_ID);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Spawn_Point
//
//////////////////////////////////////////////////////////////////////
SpawnPointNodeClass *
SpawnerNodeClass::Add_Spawn_Point (const Matrix3D &tm)
{
	//
	//	Create the new spawn point
	//
	SpawnPointNodeClass *spawn_point = new SpawnPointNodeClass;
	spawn_point->Set_Spawner (this);
	spawn_point->Initialize ();
	spawn_point->Set_Transform (tm);	
	NodeMgrClass::Setup_Node_Identity (*spawn_point);
	
	//
	//	Create the line from the spawner to the spawn point
	//
	EditorLineClass *line = new EditorLineClass;	
	const AABoxClass &box1 = m_PhysObj->Peek_Model ()->Get_Bounding_Box ();
	const AABoxClass &box2 = spawn_point->Peek_Physics_Obj ()->Peek_Model ()->Get_Bounding_Box ();
	Vector3 offset1 (0, 0, box1.Extent.Z);
	Vector3 offset2 (0, 0, box2.Extent.Z);
	line->Set_Color (Vector3 (0.5F, 0, 0.75F));
	line->Reset (m_Transform.Get_Translation () + offset1, tm.Get_Translation () + offset2);
	
	//
	//	Add the spawn point and its line to our data structures
	//
	m_SpawnPointLines.Add (line);
	m_SpawnPointNodes.Add (spawn_point);

	//
	//	Add the spawn point to the scene
	//
	if (m_IsInScene) {
		spawn_point->Add_To_Scene ();
		::Get_Scene_Editor ()->Add_Dynamic_Object (line);
	}

	return spawn_point;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_Spawn_Point
//
//////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Remove_Spawn_Point (SpawnPointNodeClass *spawn_point)
{
	//
	//	Try to find the spawn point
	//
	for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
		if (spawn_point == m_SpawnPointNodes[index]) {
			
			//
			//	Free the spawn point
			//
			MEMBER_RELEASE (spawn_point);
			m_SpawnPointNodes.Delete (index);
			
			//
			//	Remove and free the line to the spawn point
			//
			::Get_Scene_Editor ()->Remove_Object (m_SpawnPointLines[index]);
			m_SpawnPointLines[index]->Release_Ref ();
			m_SpawnPointLines.Delete (index);
			break;
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Spawn_Points
//
//////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Free_Spawn_Points (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Release our hold on each spawn point
	//
	for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
		SpawnPointNodeClass *spawn_point	= m_SpawnPointNodes[index];
		EditorLineClass *line				= m_SpawnPointLines[index];
		
		scene->Remove_Object (line);
		spawn_point->Remove_From_Scene ();
		MEMBER_RELEASE (spawn_point);
		MEMBER_RELEASE (line);
	}
	
	//
	//	Remove all the spawn points from the list
	//
	m_SpawnPointNodes.Delete_All ();
	m_SpawnPointLines.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Update_Lines
//
//////////////////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Update_Lines (void)
{
	for (int index = 0; index < m_SpawnPointNodes.Count (); index ++) {
		SpawnPointNodeClass *spawn_point	= m_SpawnPointNodes[index];
		EditorLineClass *line				= m_SpawnPointLines[index];

		//
		//	Update the line so it goes between the centers of the objects
		//
		const AABoxClass &box1 = m_PhysObj->Peek_Model ()->Get_Bounding_Box ();
		const AABoxClass &box2 = spawn_point->Peek_Physics_Obj ()->Peek_Model ()->Get_Bounding_Box ();
		Vector3 offset1 (0, 0, box1.Extent.Z);
		Vector3 offset2 (0, 0, box2.Extent.Z);
		line->Reset (m_Transform.Get_Translation () + offset1, spawn_point->Get_Position () + offset2);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_Child_Node
//
//////////////////////////////////////////////////////////////////////////////
NodeClass *
SpawnerNodeClass::Add_Child_Node (const Matrix3D &tm)
{
	return Add_Spawn_Point (tm);
}


////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
////////////////////////////////////////////////////////////////
bool
SpawnerNodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass		info_tab (this);
	PositionPageClass		pos_tab (this);
	NodeScriptsPropPage	scripts_tab (&m_Scripts);

	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&scripts_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	if (ret_code == IDOK) {
		
		//
		//	If the scripts changed, then we need to
		// reload the object and assign it the new
		// set of scripts.
		//
		Reload ();
	}
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


/////////////////////////////////////////////////////////////////
//
//	Copy_Scripts
//
/////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Copy_Scripts (const SpawnerNodeClass &src)
{
	Free_Scripts ();

	//
	//	Loop over all the scripts in the src object and copy them.
	//
	for (int index = 0; index < src.m_Scripts.Count (); index ++) {
		EditScriptClass *script = src.m_Scripts[index];
		if (script != NULL) {

			//
			//	Make ourselves a copy of the script
			//
			EditScriptClass *our_copy = new EditScriptClass (*script);
			m_Scripts.Add (our_copy);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Free_Scripts
//
/////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Free_Scripts (void)
{
	for (int index = 0; index < m_Scripts.Count (); index ++) {
		EditScriptClass *script = m_Scripts[index];
		SAFE_DELETE (script);
	}
	
	m_Scripts.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Assign_Scripts
//
/////////////////////////////////////////////////////////////////
void
SpawnerNodeClass::Assign_Scripts (void)
{
	if (m_SpawnerObj != NULL) {
		for (int index = 0; index < m_Scripts.Count (); index ++) {
			EditScriptClass *script	= m_Scripts[index];
			
			m_SpawnerObj->Add_Script (script->Get_Name (), script->Get_Composite_String ());
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Sub_Node
//
//////////////////////////////////////////////////////////////////////////////
NodeClass *
SpawnerNodeClass::Get_Sub_Node (int index)
{
	return m_SpawnPointNodes[index];
}
