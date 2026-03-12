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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SpawnerNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/11/01 9:05a                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SPAWNER_NODE_H
#define __SPAWNER_NODE_H

#include "node.h"
#include "icons.h"
#include "spawn.h"
#include "editorline.h"


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;
class PhysClass;
class SpawnPointNodeClass;


////////////////////////////////////////////////////////////////////////////
//
//	SpawnerNodeClass
//
////////////////////////////////////////////////////////////////////////////
class SpawnerNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	SpawnerNodeClass (PresetClass *preset = NULL);
	SpawnerNodeClass (const SpawnerNodeClass &src);
	~SpawnerNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const SpawnerNodeClass &operator= (const SpawnerNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	void											On_Post_Load (void);

	//
	// From NodeClass
	//
	NodeClass *	Clone (void)							{ return new SpawnerNodeClass (*this); }
	void			Initialize (void);
	NODE_TYPE	Get_Type (void) const				{ return NODE_TYPE_SPAWNER; }
	int			Get_Icon_Index (void) const		{ return OBJECT_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const;
	bool			Is_Static (void) const				{ return false; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Set_ID (uint32 id);
	NodeClass *	Add_Child_Node (const Matrix3D &tm);
	bool			Can_Add_Child_Nodes (void)	const	{ return true; }
	bool			Show_Settings_Dialog (void);

	int			Get_Sub_Node_Count (void) const		{ return m_SpawnPointNodes.Count (); }
	NodeClass *	Get_Sub_Node (int index);

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//
	//	From PersistClass
	//
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	// Spawner specific
	//
	uint32						Get_Spawned_Definition_ID (void);
	void							Load_Assets (void);
	RenderObjClass *			Get_Spawned_Model (void);
	SpawnPointNodeClass *	Add_Spawn_Point (const Matrix3D &tm);
	void							Remove_Spawn_Point (SpawnPointNodeClass *spawn_point);
	void							Update_Lines (void);
	void							Copy_Scripts (const SpawnerNodeClass &src);
	SCRIPT_LIST &				Get_Scripts (void)	{ return m_Scripts; }


protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Load_Variables (ChunkLoadClass &cload);
	void			Create_Spawner_Obj (void);
	void			Free_Spawn_Points (void);
	void			Free_Scripts (void);
	void			Assign_Scripts (void);

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	PhysClass *											m_PhysObj;
	SpawnerClass *										m_SpawnerObj;
	DynamicVectorClass<SpawnPointNodeClass *>	m_SpawnPointNodes;
	DynamicVectorClass<EditorLineClass *>		m_SpawnPointLines;
	DynamicVectorClass<Matrix3D>					m_SpawnPointLoadList;
	SCRIPT_LIST											m_Scripts;
};


//////////////////////////////////////////////////////////////////
//	Peek_Physics_Obj
//////////////////////////////////////////////////////////////////
inline PhysClass *
SpawnerNodeClass::Peek_Physics_Obj (void) const
{
	return m_PhysObj;
}


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
SpawnerNodeClass::On_Rotate (void)
{
	Update_Lines ();

	if (m_SpawnerObj != NULL) {
		m_SpawnerObj->Set_TM (m_Transform);
	}
	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
SpawnerNodeClass::On_Translate (void)
{
	Update_Lines ();

	if (m_SpawnerObj != NULL) {
		m_SpawnerObj->Set_TM (m_Transform);
	}
	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
SpawnerNodeClass::On_Transform (void)
{
	Update_Lines ();

	if (m_SpawnerObj != NULL) {
		m_SpawnerObj->Set_TM (m_Transform);
	}
	NodeClass::On_Transform ();
	return ;
}

#endif //__SPAWNER_NODE_H

