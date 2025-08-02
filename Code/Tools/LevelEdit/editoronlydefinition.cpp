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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editoronlydefinition.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 3:07p                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"

#include "editoronlydefinition.h"
#include "simpledefinitionfactory.h"
#include "definitionclassids.h"
#include "definitionmgr.h"
#include "persistfactory.h"
#include "editorchunkids.h"
#include "editoronlyobjectnode.h"


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
	VARID_MODEL_NAME			= 0x01,
};


//////////////////////////////////////////////////////////////////////////////////
//
//	Static factories
//
//////////////////////////////////////////////////////////////////////////////////
DECLARE_DEFINITION_FACTORY(EditorOnlyDefinitionClass, CLASSID_EDITOR_ONLY_OBJECTS, "Editor Only Objects")	_EditorOnlyObjDefFactory;
SimplePersistFactoryClass<EditorOnlyDefinitionClass, CHUNKID_EDITOR_ONLY_OBJECTS_DEF>			_EditorOnlyObjDefPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//
//	EditorOnlyDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
EditorOnlyDefinitionClass::EditorOnlyDefinitionClass (void)
	:	DefinitionClass ()
		
{
	FILENAME_PARAM (EditorOnlyDefinitionClass, ModelName, "Westwood 3D Files", ".w3d");
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~EditorOnlyDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
EditorOnlyDefinitionClass::~EditorOnlyDefinitionClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
//////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
EditorOnlyDefinitionClass::Get_Factory (void) const
{
	return _EditorOnlyObjDefPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
EditorOnlyDefinitionClass::Save (ChunkSaveClass &csave)
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
EditorOnlyDefinitionClass::Load (ChunkLoadClass &cload)
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
EditorOnlyDefinitionClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_MODEL_NAME, ModelName)
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
EditorOnlyDefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_MODEL_NAME, ModelName)
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
EditorOnlyDefinitionClass::Create (void) const
{
	return new EditorOnlyObjectNodeClass ();
}

