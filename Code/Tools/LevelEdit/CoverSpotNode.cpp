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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/CoverSpotNode.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/13/01 9:44a                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "coverspotnode.h"
#include "coverattackpointnode.h"
#include "sceneeditor.h"
#include "collisiongroups.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "preset.h"
#include "chunkio.h"
#include "nodemgr.h"
#include "coverspotinfopage.h"
#include "nodeinfopage.h"
#include "positionpage.h"
#include "editorpropsheet.h"
#include "cover.h"
#include "modelutils.h"
#include "soldier.h"
#include "presetmgr.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<CoverSpotNodeClass, CHUNKID_NODE_COVER_SPOT> _CoverSpotNodePersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Save/load constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x05260946,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_REQUIRES_CROUCH	= 0x01,
	VARID_ATTACK_POINT,
};


//////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////
static const Vector3	LINE_OFFSET (0, 0, 0.2F);


//////////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////////
PhysClass *	CoverSpotNodeClass::_TheCollisionObj	= NULL;
int			CoverSpotNodeClass::_InstanceCount		= 0;


//////////////////////////////////////////////////////////////////////////////
//
//	CoverSpotNodeClass
//
//////////////////////////////////////////////////////////////////////////////
CoverSpotNodeClass::CoverSpotNodeClass (PresetClass *preset)
	:	m_PhysObj (NULL),
		m_RequiresCrouch (false),
		m_GameCoverSpot (NULL),
		NodeClass (preset)		
{
	_InstanceCount ++;
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	CoverSpotNodeClass
//
//////////////////////////////////////////////////////////////////////////////
CoverSpotNodeClass::CoverSpotNodeClass (const CoverSpotNodeClass &src)
	:	m_PhysObj (NULL),
		m_RequiresCrouch (false),
		m_GameCoverSpot (NULL),
		NodeClass (NULL)
{
	_InstanceCount ++;
	*this = src;	
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	~CoverSpotNodeClass
//
//////////////////////////////////////////////////////////////////////////////
CoverSpotNodeClass::~CoverSpotNodeClass (void)
{	
	Free_Attack_Points ();
	Remove_From_Scene ();
	MEMBER_RELEASE (m_PhysObj);

	//
	//	Free the collision object when all instances have gone away
	//
	_InstanceCount --;
	if (_InstanceCount == 0) {
		MEMBER_RELEASE (_TheCollisionObj);
	}

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
CoverSpotNodeClass::Initialize (void)
{
	MEMBER_RELEASE (m_PhysObj);

	//
	//	Create the camera render object
	//
	RenderObjClass *render_obj = ::Create_Render_Obj ("COVERSPOT");
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
		m_PhysObj->Peek_Model ()->Set_User_Data ((PVOID)&m_HitTestInfo, false);
		m_PhysObj->Set_Transform (m_Transform);
		::Set_Model_Collision_Type (m_PhysObj->Peek_Model (), COLLISION_TYPE_0);
		
		// Release our hold on the render object pointer
		MEMBER_RELEASE (render_obj);
	}

	if (_TheCollisionObj == NULL) {

		//
		//	Create the collision physics object
		//	
		PresetClass *preset = PresetMgrClass::Find_Preset ("Walk-Thru");
		if (preset != NULL && preset->Get_Definition ()  != NULL) {
			
			//
			//	Load the assets for this preset and get its definition
			//
			preset->Load_All_Assets ();
			SoldierGameObjDef *definition	= (SoldierGameObjDef *)preset->Get_Definition ();
			
			//
			//	Create the collision physics object
			//
			SoldierGameObj *game_obj = new SoldierGameObj;
			game_obj->Init (*definition);
			MEMBER_ADD (_TheCollisionObj, game_obj->Peek_Physical_Object ());
			game_obj->Set_Delete_Pending ();
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
CoverSpotNodeClass::Get_Factory (void) const
{	
	return _CoverSpotNodePersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
CoverSpotNodeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		NodeClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_REQUIRES_CROUCH, m_RequiresCrouch);
		
		//
		//	Save the list of cover point attack positions
		//
		for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
			CoverAttackPointNodeClass *attack_point = m_AttackPointNodes[index];
			if (attack_point != NULL) {
				Matrix3D tm = attack_point->Get_Transform ();
				WRITE_MICRO_CHUNK (csave, VARID_ATTACK_POINT, tm);
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
CoverSpotNodeClass::Load (ChunkLoadClass &cload)
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

	SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
CoverSpotNodeClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_REQUIRES_CROUCH, m_RequiresCrouch);

			case VARID_ATTACK_POINT:
			{
				//
				//	Read the attack points transfrom from the chunk
				//
				Matrix3D tm;
				cload.Read (&tm, sizeof (tm));
				m_AttackPointLoadList.Add (tm);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
///////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::On_Post_Load (void)
{
	//
	//	If the spawner isn't valid, then remove it from the system
	//
	if (m_Preset == NULL) {
		::Get_Scene_Editor ()->Delete_Node (this, false);
	} else {

		//
		//	Add each attack point at the given locations
		//
		for (int index = 0; index < m_AttackPointLoadList.Count (); index ++) {
			Add_Attack_Point (m_AttackPointLoadList[index]);
		}

		//
		//	Reset the list
		//
		m_AttackPointLoadList.Delete_All ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const CoverSpotNodeClass &
CoverSpotNodeClass::operator= (const CoverSpotNodeClass &src)
{
	//
	//	Copy the attach point list
	//
	Free_Attack_Points ();
	for (int index = 0; index < src.m_AttackPointNodes.Count (); index ++) {
		CoverAttackPointNodeClass *attack_point = src.m_AttackPointNodes[index];
		if (attack_point != NULL) {
			Add_Attack_Point (attack_point->Get_Transform ());
		}
	}

	m_RequiresCrouch = src.m_RequiresCrouch;
	NodeClass::operator= (src);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Pre_Export
//
//////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Pre_Export (void)
{
	//
	//	Remove ourselves from the 'system' so we don't get accidentally
	// saved during the export. 
	//
	Add_Ref ();
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Remove_Object (m_PhysObj);

		//
		//	Build a cover spot entry that we can pass off the game
		// so it will get exported
		//
		MEMBER_RELEASE (m_GameCoverSpot);
		m_GameCoverSpot = new CoverEntryClass;
		m_GameCoverSpot->Set_Transform (Get_Transform ());
		m_GameCoverSpot->Set_Crouch (m_RequiresCrouch);
		AttackPositionListType *attack_list = m_GameCoverSpot->Get_Attack_Position_List ();			

		//
		//	Remove the unnecessary UI elements from the scene
		//
		for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
			CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
			EditorLineClass *line							= m_AttackPointLines[index];
			::Get_Scene_Editor ()->Remove_Object (line);
			
			attack_list->Add (attack_point->Get_Position ());
		}

		//
		//	Add this cover spot to the game
		//
		CoverManager::Add_Entry (m_GameCoverSpot);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Post_Export
//
//////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Post_Export (void)
{
	//
	//	Put ourselves back into the system
	//
	if (m_PhysObj != NULL && m_IsInScene) {
		::Get_Scene_Editor ()->Add_Dynamic_Object (m_PhysObj);

		//
		//	Pass the Post_Export call onto any attack points as well
		//
		for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
			EditorLineClass *line							= m_AttackPointLines[index];
			::Get_Scene_Editor ()->Add_Dynamic_Object (line);
		}

		//
		//	Remove the cover spot from the game
		//
		CoverManager::Remove_Entry (m_GameCoverSpot);
		MEMBER_RELEASE (m_GameCoverSpot);
	}

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Add_Attack_Point
//
//////////////////////////////////////////////////////////////////////
CoverAttackPointNodeClass *
CoverSpotNodeClass::Add_Attack_Point (const Matrix3D &tm)
{
	//
	//	Create and add the new point
	//
	CoverAttackPointNodeClass *attack_point = new CoverAttackPointNodeClass;
	attack_point->Initialize ();
	attack_point->Set_Transform (tm);
	attack_point->Set_Cover_Spot (this);
	NodeMgrClass::Setup_Node_Identity (*attack_point);
	m_AttackPointNodes.Add (attack_point);

	//
	//	Create and add the line from the coverspot to the attack point
	//
	EditorLineClass *line = new EditorLineClass;	
	line->Reset (m_Transform.Get_Translation () + LINE_OFFSET, tm.Get_Translation () + LINE_OFFSET);
	m_AttackPointLines.Add (line);

	if (m_IsInScene) {
		attack_point->Add_To_Scene ();
		::Get_Scene_Editor ()->Add_Dynamic_Object (line);
	}

	return attack_point;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_Attack_Point
//
//////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Remove_Attack_Point (CoverAttackPointNodeClass *attack_point)
{
	//
	//	Try to find the attack point
	//
	for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
		if (attack_point == m_AttackPointNodes[index]) {
			
			//
			//	Free the attack point
			//
			MEMBER_RELEASE (attack_point);
			m_AttackPointNodes.Delete (index);
			
			//
			//	Remove and free the line to the attack point
			//
			::Get_Scene_Editor ()->Remove_Object (m_AttackPointLines[index]);
			m_AttackPointLines[index]->Release_Ref ();
			m_AttackPointLines.Delete (index);
			break;
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Attack_Points
//
//////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Free_Attack_Points (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Release our hold on each attack point
	//
	for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
		CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
		EditorLineClass *line							= m_AttackPointLines[index];
		
		scene->Remove_Object (line);
		attack_point->Remove_From_Scene ();
		MEMBER_RELEASE (attack_point);
		MEMBER_RELEASE (line);
	}
	
	//
	//	Remove all the attack points from the list
	//
	m_AttackPointNodes.Delete_All ();
	m_AttackPointLines.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Show_Settings_Dialog
//
/////////////////////////////////////////////////////////////////
bool
CoverSpotNodeClass::Show_Settings_Dialog (void)
{	
	NodeInfoPageClass			info_tab (this);
	PositionPageClass			pos_tab (this);
	CoverSpotInfoPageClass	cover_info_tab (this);

	EditorPropSheetClass prop_sheet;
	prop_sheet.Add_Page (&info_tab);
	prop_sheet.Add_Page (&pos_tab);
	prop_sheet.Add_Page (&cover_info_tab);

	// Show the property sheet
	UINT ret_code = prop_sheet.DoModal ();
	
	// Return true if the user clicked OK
	return (ret_code == IDOK);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
//////////////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Add_To_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();

	//
	//	Add all the waypoints to the scene
	//
	for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
		CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
		EditorLineClass *line							= m_AttackPointLines[index];
		attack_point->Add_To_Scene ();
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
CoverSpotNodeClass::Remove_From_Scene (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL && m_IsInScene) {

		//
		//	Remove all the waypoints from the scene
		//
		for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
			CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
			EditorLineClass *line							= m_AttackPointLines[index];
			attack_point->Remove_From_Scene ();
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
CoverSpotNodeClass::Update_Lines (void)
{
	for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
		CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
		EditorLineClass *line							= m_AttackPointLines[index];
		
		line->Reset (Get_Transform ().Get_Translation () + LINE_OFFSET,
							attack_point->Get_Position () + LINE_OFFSET);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Peek_Collision_Obj
//
//////////////////////////////////////////////////////////////////////
PhysClass *
CoverSpotNodeClass::Peek_Collision_Obj (void) const
{
	if (_TheCollisionObj != NULL) {
		_TheCollisionObj->Set_Transform (m_Transform);
	}

	return _TheCollisionObj;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////////////
void
CoverSpotNodeClass::Hide (bool hide)
{
	//
	//	Apply the same operation to all the attack points and lines
	//
	for (int index = 0; index < m_AttackPointNodes.Count (); index ++) {
		CoverAttackPointNodeClass *attack_point	= m_AttackPointNodes[index];
		EditorLineClass *line							= m_AttackPointLines[index];
		attack_point->Hide (hide);
		line->Hide (hide);
	}
	
	NodeClass::Hide (hide);
	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Add_Child_Node
//
//////////////////////////////////////////////////////////////////////////////
NodeClass *
CoverSpotNodeClass::Add_Child_Node (const Matrix3D &tm)
{
	return Add_Attack_Point (tm);
}


//////////////////////////////////////////////////////////////////////////////
//
//	Get_Sub_Node
//
//////////////////////////////////////////////////////////////////////////////
NodeClass *
CoverSpotNodeClass::Get_Sub_Node (int index)
{
	return m_AttackPointNodes[index];
}
