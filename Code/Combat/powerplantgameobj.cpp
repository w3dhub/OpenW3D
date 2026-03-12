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
 *                     $Archive:: /Commando/Code/Combat/powerplantgameobj.cpp                 $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 9:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "powerplantgameobj.h"
#include "basecontroller.h"
#include "wwhack.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "definitionmgr.h"
#include "combatchunkid.h"
#include "debug.h"
#include "wwprofile.h"
#include "basecontroller.h"


////////////////////////////////////////////////////////////////
//	Hacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (PowerPlant)

////////////////////////////////////////////////////////////////
//	Editable and persist factories
////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<PowerPlantGameObjDef,	CHUNKID_GAME_OBJECT_DEF_POWERPLANT>						_PowerPlantGameObjDefPersistFactory;
SimplePersistFactoryClass	<PowerPlantGameObj,		CHUNKID_GAME_OBJECT_POWERPLANT>							_PowerPlantGameObjPersistFactory;
DECLARE_DEFINITION_FACTORY (PowerPlantGameObjDef,	CLASSID_GAME_OBJECT_DEF_POWERPLANT, "Powerplant")	_PowerPlantGameObjDefDefFactory;


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
//	PowerPlantGameObjDef
//
////////////////////////////////////////////////////////////////
PowerPlantGameObjDef::PowerPlantGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~PowerPlantGameObjDef
//
////////////////////////////////////////////////////////////////
PowerPlantGameObjDef::~PowerPlantGameObjDef (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
////////////////////////////////////////////////////////////////
uint32
PowerPlantGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_POWERPLANT;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
PersistClass *
PowerPlantGameObjDef::Create (void) const
{
	PowerPlantGameObj *building = new PowerPlantGameObj;
	building->Init (*this);

	return building;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
bool
PowerPlantGameObjDef::Save (ChunkSaveClass &csave)
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
PowerPlantGameObjDef::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized Powerplant Def chunkID\n"));
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
PowerPlantGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized Powerplant Def Variable chunkID\n"));
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
PowerPlantGameObjDef::Get_Factory (void) const
{
	return _PowerPlantGameObjDefPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	PowerPlantGameObj
//
////////////////////////////////////////////////////////////////
PowerPlantGameObj::PowerPlantGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~PowerPlantGameObj
//
////////////////////////////////////////////////////////////////
PowerPlantGameObj::~PowerPlantGameObj (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////
const PersistFactoryClass &
PowerPlantGameObj::Get_Factory (void) const
{
	return _PowerPlantGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void PowerPlantGameObj::Init( void )
{
	Init( Get_Definition() );
}

////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void
PowerPlantGameObj::Init (const PowerPlantGameObjDef &definition)
{
	BuildingGameObj::Init (definition);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
////////////////////////////////////////////////////////////////
const PowerPlantGameObjDef &
PowerPlantGameObj::Get_Definition (void) const
{
	return (const PowerPlantGameObjDef &)BaseGameObj::Get_Definition ();
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
PowerPlantGameObj::Save (ChunkSaveClass &csave)
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
PowerPlantGameObj::Load (ChunkLoadClass &cload)
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
				Debug_Say (("Unrecognized Powerplant chunkID\n"));
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
PowerPlantGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {

		/*switch (cload.Cur_Micro_Chunk_ID ())
		{
			default:
				Debug_Say (("Unrecognized Powerplant Variable chunkID\n"));
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
PowerPlantGameObj::On_Destroyed (void)
{
	BuildingGameObj::On_Destroyed ();

	//
	//	Tell the base that it better check the power state
	// of the base.
	//
	if (BaseController != NULL) {
		BaseController->Check_Base_Power ();
	}

	return ;
}
