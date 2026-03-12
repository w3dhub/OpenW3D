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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainSectionPersist.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/29/01 12:17p                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TERRAIN_SECTION_PERSIST_H
#define __TERRAIN_SECTION_PERSIST_H


#include "bittype.h"
#include "vector3.h"
#include "wwstring.h"
#include "vector.h"


////////////////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////////////////
class NodeClass;
class ChunkSaveClass;
class ChunkLoadClass;
class TerrainSectionPersistClass;
class TerrainNodeClass;

////////////////////////////////////////////////////////////////////////////
//
//	TerrainSectionPersistListClass
//
////////////////////////////////////////////////////////////////////////////
class TerrainSectionPersistListClass : public DynamicVectorClass<TerrainSectionPersistClass *>
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	virtual ~TerrainSectionPersistListClass (void) { Free_List (); }

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////
	void		Build_List (DynamicVectorClass<NodeClass *> &node_list);
	void		Assign_Section_IDs (TerrainNodeClass *node);
	void		Initialize_Virgin_Sections (void);
	void		Free_List (void);

	void		Save (ChunkSaveClass &csave);
	void		Load (ChunkLoadClass &cload);

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	DynamicVectorClass<NodeClass *>	m_VirginSections;
};


////////////////////////////////////////////////////////////////////////////
//
//	TerrainSectionPersistClass
//
////////////////////////////////////////////////////////////////////////////
class TerrainSectionPersistClass
{

public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	TerrainSectionPersistClass (void);
	virtual ~TerrainSectionPersistClass (void);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// Initialization
	//
	void			Initialize (NodeClass *node);
	void			Apply (NodeClass *node);
	void			Initialize_Virgin_Sections (void)			{ m_TerrainSectionInfo.Initialize_Virgin_Sections (); }

	//
	//	Save/load stuff
	//
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

	//
	//	Accessors
	//
	const Vector3 &		Get_Position (void) const			{ return m_Position; }
	uint32					Get_Def_ID (void) const				{ return m_DefinitionID; }
	int						Get_Instance_ID (void) const		{ return m_InstanceID; }
	uint32					Get_Vis_Obj_ID (void) const		{ return m_VisObjectID; }
	uint32					Get_Vis_Sector_ID (void) const	{ return m_VisSectorID; }
	uint32					Get_Cull_Link (void) const			{ return m_CullLink; }
	const StringClass &	Get_Name (void) const				{ return m_Name; }

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Save_Variables (ChunkSaveClass &csave);
	bool			Load_Variables (ChunkLoadClass &cload);

private:

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	Vector3									m_Position;
	uint32									m_DefinitionID;
	int										m_InstanceID;
	uint32									m_VisObjectID;
	uint32									m_VisSectorID;
	uint32									m_CullLink;
	StringClass								m_Name;
	TerrainSectionPersistListClass	m_TerrainSectionInfo;
};


#endif //__TERRAIN_SECTION_PERSIST_H

