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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/WaypathDefinition.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/01/00 2:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "waypathdefinition.h"
#include "simpledefinitionfactory.h"
#include "definitionclassids.h"
#include "definitionmgr.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "waypathnode.h"


//////////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_BASE_CLASS		= 0x00000200,
};


enum
{
	VARID_PASSABLE_OBJ_ID	= 0x01
};


//////////////////////////////////////////////////////////////////////////////////
//
//	Static factories
//
//////////////////////////////////////////////////////////////////////////////////
DECLARE_DEFINITION_FACTORY(WaypathDefinitionClass, CLASSID_WAYPATH, "Waypath")	_WaypathDefFactory;
SimplePersistFactoryClass<WaypathDefinitionClass, CHUNKID_WAYPATH_DEF>				_WaypathDefPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//
//	WaypathDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
WaypathDefinitionClass::WaypathDefinitionClass (void)
	:	m_PassableObjects (0),
		DefinitionClass ()
		
{
	ENUM_PARAM (WaypathDefinitionClass, m_PassableObjects, ("All", 1, "Humans", 2, "Ground Vehicles", 3, "Flying Vehicles", 4, NULL));
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~WaypathDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
WaypathDefinitionClass::~WaypathDefinitionClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
//////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
WaypathDefinitionClass::Get_Factory (void) const
{
	return _WaypathDefPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
WaypathDefinitionClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		retval &= DefinitionClass::Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_PASSABLE_OBJ_ID, m_PassableObjects);
	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
WaypathDefinitionClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_BASE_CLASS:
				retval &= DefinitionClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
WaypathDefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_PASSABLE_OBJ_ID, m_PassableObjects);
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////////////
PersistClass *
WaypathDefinitionClass::Create (void) const
{
	return new WaypathNodeClass ();
}

