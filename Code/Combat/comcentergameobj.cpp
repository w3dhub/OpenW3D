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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/comcentergameobj.cpp                  $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/16/01 12:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "comcentergameobj.h"
#include "basecontroller.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "definitionmgr.h"
#include "combatchunkid.h"
#include "debug.h"
#include "wwprofile.h"
#include "basecontroller.h"
#include "combat.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (ComCenter)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<ComCenterGameObjDef,	CHUNKID_GAME_OBJECT_DEF_COMCENTER>						_ComCenterGameObjDefPersistFactory;
SimplePersistFactoryClass	<ComCenterGameObj,		CHUNKID_GAME_OBJECT_COMCENTER>							_ComCenterGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (ComCenterGameObjDef,	CLASSID_GAME_OBJECT_DEF_COMCENTER, "Com Center")	_ComCenterGameObjDefDefFactory;


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT						=	0x02211153,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_UNUSED					= 1,
};

enum
{
	CHUNKID_PARENT								=	0x02211154,
	CHUNKID_VARIABLES,

	MICROCHUNKID_UNUSED						= 1,
};


////////////////////////////////////////////////////////////////
//
//	ComCenterGameObjDef
//
////////////////////////////////////////////////////////////////
ComCenterGameObjDef::ComCenterGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ComCenterGameObjDef
//
////////////////////////////////////////////////////////////////
ComCenterGameObjDef::~ComCenterGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
ComCenterGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_COMCENTER;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
ComCenterGameObjDef::Create (void) const
{
	ComCenterGameObj *building = new ComCenterGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
ComCenterGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		BuildingGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
ComCenterGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ())
	{
		switch (cload.Cur_Chunk_ID ())
		{
			case CHUNKID_DEF_PARENT:
				BuildingGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized Com Center Def chunkID\n"));
				break;

		}
		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
ComCenterGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized Com Center Def Variable chunkID\n"));
				break;
		}*/

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
ComCenterGameObjDef::Get_Factory (void) const
{
	return _ComCenterGameObjDefPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	ComCenterGameObj
//
////////////////////////////////////////////////////////////////
ComCenterGameObj::ComCenterGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ComCenterGameObj
//
////////////////////////////////////////////////////////////////
ComCenterGameObj::~ComCenterGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
ComCenterGameObj::Get_Factory (void) const
{
	return _ComCenterGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void ComCenterGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
ComCenterGameObj::Init (const ComCenterGameObjDef &definition)
{
	BuildingGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const ComCenterGameObjDef &
ComCenterGameObj::Get_Definition (void) const
{
	return (const ComCenterGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
ComCenterGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		BuildingGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
ComCenterGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				BuildingGameObj::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized Com Center chunkID\n"));
				break;
		}

		cload.Close_Chunk();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
ComCenterGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized Com Center Variable chunkID\n"));
				break;
		}*/

		cload.Close_Micro_Chunk();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroyed
//
////////////////////////////////////////////////////////////////
void
ComCenterGameObj::On_Destroyed (void)
{
	BuildingGameObj::On_Destroyed ();

	//
	//	Switch off the radar for each player on this team
	//
	if (BaseController != NULL && CombatManager::I_Am_Server ()) {
		BaseController->Check_Radar ();
	}

	return ;
}

