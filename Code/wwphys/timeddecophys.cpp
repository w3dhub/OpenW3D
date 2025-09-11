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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/timeddecophys.cpp                     $*
 *                                                                                             *
 *                       Author:: Byon Garrabrant                                              *
 *                                                                                             *
 *                     $Modtime:: 9/24/00 1:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "timeddecophys.h"
//#include "rendobj.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "wwprofile.h"
//#include "part_emt.h"

DECLARE_FORCE_LINK(timeddecophys);

/****************************************************************************************************
**
** TimedDecorationPhysClass Implementation
**
****************************************************************************************************/

/*
** Persist factory for TimedDecorationPhysClass
*/
SimplePersistFactoryClass<TimedDecorationPhysClass,PHYSICS_CHUNKID_TIMEDDECORATIONPHYS>	_TimedDecoPhysFactory;


/*
** Chunk-ID's used by TimedDecoPhys
*/
enum 
{
	TIMEDDECOPHYS_CHUNK_DECOPHYS		=	0x005160000,			// decophys class data
	TIMEDDECOPHYS_CHUNK_VARIABLES,

	TIMEDDECOPHYS_VARIABLE_LIFETIME	= 0x00,
};

TimedDecorationPhysClass::TimedDecorationPhysClass(void) :
	DecorationPhysClass(),
	Lifetime(2.0f)
{ 
}

void TimedDecorationPhysClass::Init(const TimedDecorationPhysDefClass & def)
{
	DecorationPhysClass::Init(def);
	Lifetime = def.Lifetime;
}

void	TimedDecorationPhysClass::Set_Lifetime( float time )
{
	Lifetime = time;
}

float	TimedDecorationPhysClass::Get_Lifetime( void )
{
	return Lifetime;
}


void TimedDecorationPhysClass::Timestep(float dt)
{
	
	DecorationPhysClass::Timestep( dt );

	{
		WWPROFILE("TimedDecoPhys::Timestep");
		/*
		** Decrement our life
		*/
		Lifetime -= dt;
		if (Lifetime < 0.0f) {
			ExpirationReactionType result = EXPIRATION_APPROVED;
			if (Observer != NULL) {
				result = Observer->Object_Expired(this);
			}
			if (result == EXPIRATION_APPROVED) {
				PhysicsSceneClass::Get_Instance()->Delayed_Remove_Object(this);
			}
		}
	}
}


const PersistFactoryClass & TimedDecorationPhysClass::Get_Factory (void) const
{
	return _TimedDecoPhysFactory;
}

bool TimedDecorationPhysClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(TIMEDDECOPHYS_CHUNK_DECOPHYS);
	DecorationPhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(TIMEDDECOPHYS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,TIMEDDECOPHYS_VARIABLE_LIFETIME,Lifetime);
	csave.End_Chunk();

	return true;
}

bool TimedDecorationPhysClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case TIMEDDECOPHYS_CHUNK_DECOPHYS:
				DecorationPhysClass::Load(cload);
				break;

			case TIMEDDECOPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,TIMEDDECOPHYS_VARIABLE_LIFETIME,Lifetime);
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}
	return true;
}

/****************************************************************************************************
**
** TimedDecorationPhysDefClass Implementation
**
****************************************************************************************************/

/*
** Persist factory for TimedDecorationPhysDefClass's
*/
SimplePersistFactoryClass<TimedDecorationPhysDefClass,PHYSICS_CHUNKID_TIMEDDECOPHYSDEF>	_TimedDecorationPhysDefFactory;

/*
** Definition factory for TimedDecorationPhysDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(TimedDecorationPhysDefClass, CLASSID_TIMEDDECOPHYSDEF, "TimedDecorationPhys") _TimedDecorationPhysDefDefFactory;

/*
** Chunk ID's used by TimedDecorationPhysDefClass
*/
enum 
{
	TIMEDDECORATIONPHYSDEF_CHUNK_DECORATIONPHYSDEF				= 0x01170003,			// (parent class)
	TIMEDDECORATIONPHYSDEF_CHUNK_VARIABLES,

	TIMEDDECORATIONPHYSDEF_VARIABLE_LIFETIME						= 0x00,
};


TimedDecorationPhysDefClass::TimedDecorationPhysDefClass(void) :
	DecorationPhysDefClass(),
	Lifetime(2.0f)
{
#ifdef PARAM_EDITING_ON
	// make our parameters editable!
	FLOAT_EDITABLE_PARAM(TimedDecorationPhysDefClass, Lifetime, 0.01f, 100.0f);	
#endif
}

uint32 TimedDecorationPhysDefClass::Get_Class_ID (void) const
{
	return CLASSID_TIMEDDECOPHYSDEF; 
}

PersistClass * TimedDecorationPhysDefClass::Create(void) const
{
	TimedDecorationPhysClass * new_obj = NEW_REF(TimedDecorationPhysClass,());
	new_obj->Init(*this);
	return new_obj;
}

const char * TimedDecorationPhysDefClass::Get_Type_Name(void)
{ 
	return "TimedDecorationPhysDef"; 
}

bool TimedDecorationPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::stricmp(type_name,TimedDecorationPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return DecorationPhysDefClass::Is_Type(type_name);
	}
}

const PersistFactoryClass & TimedDecorationPhysDefClass::Get_Factory (void) const
{
	return _TimedDecorationPhysDefFactory;
}

bool TimedDecorationPhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(TIMEDDECORATIONPHYSDEF_CHUNK_DECORATIONPHYSDEF);
	DecorationPhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(TIMEDDECORATIONPHYSDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,TIMEDDECORATIONPHYSDEF_VARIABLE_LIFETIME,Lifetime);
	csave.End_Chunk();
	
	return true;
}

bool TimedDecorationPhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case TIMEDDECORATIONPHYSDEF_CHUNK_DECORATIONPHYSDEF:
				DecorationPhysDefClass::Load(cload);
				break;

			case TIMEDDECORATIONPHYSDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,TIMEDDECORATIONPHYSDEF_VARIABLE_LIFETIME,Lifetime);
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}

