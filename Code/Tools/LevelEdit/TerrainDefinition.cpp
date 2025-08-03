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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TerrainDefinition.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/00 2:20p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "TerrainDefinition.h"
#include "simpledefinitionfactory.h"
#include "definitionclassids.h"
#include "definitionmgr.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "terrainnode.h"


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
	VARID_LOD_DIST				= 0x01,
	VARID_MODEL_NAME,
	VARID_LIGHT_FILENAME
};


//////////////////////////////////////////////////////////////////////////////////
//
//	Static factories
//
//////////////////////////////////////////////////////////////////////////////////
DECLARE_DEFINITION_FACTORY(TerrainDefinitionClass, CLASSID_TERRAIN, "Terrain")	_TerrainDefFactory;
SimplePersistFactoryClass<TerrainDefinitionClass, CHUNKID_TERRAIN_DEF>			_TerrainPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//
//	TerrainDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
TerrainDefinitionClass::TerrainDefinitionClass (void)
	:	DefinitionClass ()
		
{
	FILENAME_PARAM (TerrainDefinitionClass, m_ModelName, "Westwood 3D Files", ".w3d");
	FILENAME_PARAM (TerrainDefinitionClass, m_LightFilename, "Westwood Light Database", ".wlt");
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~TerrainDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
TerrainDefinitionClass::~TerrainDefinitionClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
//////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TerrainDefinitionClass::Get_Factory (void) const
{
	return _TerrainPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
TerrainDefinitionClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	retval &= Save_Variables (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
	retval &= DefinitionClass::Save (csave);
	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
TerrainDefinitionClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			case CHUNKID_BASE_CLASS:
				retval &= DefinitionClass::Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
TerrainDefinitionClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	//
	//	Write the list of distances to the chunk
	//
	for (int index = 0; index < m_DistanceList.Count (); index ++) {
		uint32 distance = m_DistanceList[index];

		//
		//	Create a new micro-chunk and write this data out to it.
		//
		csave.Begin_Micro_Chunk (VARID_LOD_DIST);
		retval &= (csave.Write (&distance, sizeof (distance)) == sizeof (distance));
		csave.End_Micro_Chunk ();
	}

	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_MODEL_NAME, m_ModelName)
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_LIGHT_FILENAME, m_LightFilename)
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
TerrainDefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	// Start fresh
	m_DistanceList.Delete_All ();

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			case VARID_LOD_DIST:
			{
				uint32 distance = 0;
				retval &= (cload.Read (&distance, sizeof (distance)) == sizeof (distance));
				m_DistanceList.Add (distance);
			}
			break;

			READ_MICRO_CHUNK_WWSTRING (cload, VARID_MODEL_NAME, m_ModelName)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_LIGHT_FILENAME, m_LightFilename)
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////////////
PersistClass *
TerrainDefinitionClass::Create (void) const
{
	return new TerrainNodeClass ();
}

