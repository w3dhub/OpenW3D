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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/specialeffectsgameobj.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 2:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "specialeffectsgameobj.h"
#include "combatchunkid.h"
#include "WWAudio.h"
#include "hanim.h"
#include "assetmgr.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "chunkio.h"
#include "rendobj.h"
#include "timemgr.h"
#include "debug.h"
#include "phys.h"
#include "combat.h"
#include "wwprofile.h"


DECLARE_FORCE_LINK (SpecialEffects)


///////////////////////////////////////////////////////////////////////////
//	Persist and Editable framework support
///////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<SpecialEffectsGameObj,		CHUNKID_GAME_OBJECT_SPECIAL_EFFECTS>		_SpecialEffectsGameObjPersistFactory;
SimplePersistFactoryClass	<SpecialEffectsGameObjDef, CHUNKID_GAME_OBJECT_DEF_SPECIAL_EFFECTS>	_SpecialEffectsGameObjDefPersistFactory;
DECLARE_DEFINITION_FACTORY	(SpecialEffectsGameObjDef, CLASSID_GAME_OBJECT_DEF_SPECIAL_EFFECTS, "Special Effects") _SpecialEffectsGameObjDefDefFactory;


///////////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_DEF_PARENT				=	0x09010212,
	CHUNKID_DEF_VARIABLES,

	VARID_DEF_ANIMATION_NAME		= 1,
	VARID_DEF_SOUNDID,
};


enum
{
	CHUNKID_PARENT						=	0x09010236,
	CHUNKID_VARIABLES,

	VARID_LIFE_REMAINING				= 1,
	VARID_IS_INITIALIZED
};


///////////////////////////////////////////////////////////////////////////
//
//	SpecialEffectsGameObjDef
//
///////////////////////////////////////////////////////////////////////////
SpecialEffectsGameObjDef::SpecialEffectsGameObjDef (void)	:
	SoundDefID (0)
{
	MODEL_DEF_PARAM(SpecialEffectsGameObjDef, PhysDefID, "TimedDecorationPhysDef");
	EDITABLE_PARAM (SpecialEffectsGameObjDef, ParameterClass::TYPE_STRING,					AnimationName);
	EDITABLE_PARAM (SpecialEffectsGameObjDef, ParameterClass::TYPE_SOUNDDEFINITIONID,	SoundDefID);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~SpecialEffectsGameObjDef
//
///////////////////////////////////////////////////////////////////////////
SpecialEffectsGameObjDef::~SpecialEffectsGameObjDef (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
///////////////////////////////////////////////////////////////////////////
uint32
SpecialEffectsGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_SPECIAL_EFFECTS; 
}


///////////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////////
PersistClass *
SpecialEffectsGameObjDef::Create (void) const 
{
	SpecialEffectsGameObj *obj = new SpecialEffectsGameObj;
	obj->Init (*this);
	return obj;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
SpecialEffectsGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		PhysicalGameObjDef::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
SpecialEffectsGameObjDef::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_DEF_PARENT:
				PhysicalGameObjDef::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;
	  
			default:
				Debug_Say (("Unrecognized SpecialEffectsGameObjDef chunk ID\n"));
				break;

		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObjDef::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK_WWSTRING	(csave, VARID_DEF_ANIMATION_NAME,	AnimationName);
	WRITE_MICRO_CHUNK				(csave, VARID_DEF_SOUNDID,				SoundDefID);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObjDef::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk()) {
		switch (cload.Cur_Micro_Chunk_ID()) {

			READ_MICRO_CHUNK_WWSTRING	(cload, VARID_DEF_ANIMATION_NAME,	AnimationName);
			READ_MICRO_CHUNK				(cload, VARID_DEF_SOUNDID,				SoundDefID);
			
			default:
				Debug_Say (("Unrecognized SpecialEffectsGameObjDef Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}
		cload.Close_Micro_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SpecialEffectsGameObjDef::Get_Factory (void) const 
{ 
	return _SpecialEffectsGameObjDefPersistFactory; 
}



/*
**
**	Start of SpecialEffectsGameObj
**
*/


///////////////////////////////////////////////////////////////////////////
//
//	SpecialEffectsGameObj
//
///////////////////////////////////////////////////////////////////////////
SpecialEffectsGameObj::SpecialEffectsGameObj (void)	:
	LifeRemaining (-1.0F),
	IsInitialized (false)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	~SpecialEffectsGameObj
//
///////////////////////////////////////////////////////////////////////////
SpecialEffectsGameObj::~SpecialEffectsGameObj (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SpecialEffectsGameObj::Get_Factory (void) const 
{
	return _SpecialEffectsGameObjPersistFactory;
}


////////////////////////////////////////////////////////////////
//
//	Init
//
////////////////////////////////////////////////////////////////
void SpecialEffectsGameObj::Init( void )
{
	Init( Get_Definition() );
}

///////////////////////////////////////////////////////////////////////////
//
//	Init
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObj::Init (const SpecialEffectsGameObjDef &definition)
{
	PhysicalGameObj::Init (definition);

	//
	//	Make sure collisions are turned off on this effect
	//
	PhysClass *phys_obj = Peek_Physical_Object();
	if (phys_obj != NULL) {
		phys_obj->Set_Collision_Group( UNCOLLIDEABLE_GROUP );
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
///////////////////////////////////////////////////////////////////////////
const SpecialEffectsGameObjDef &
SpecialEffectsGameObj::Get_Definition (void) const
{
	return (const SpecialEffectsGameObjDef &)BaseGameObj::Get_Definition();
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
bool
SpecialEffectsGameObj::Save (ChunkSaveClass & csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		PhysicalGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
bool
SpecialEffectsGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				PhysicalGameObj::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say (("Unrecognized SpecialEffectsGameObj chunk ID\n"));
				break;

		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//	Do_Effect
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObj::Do_Effect (void)
{
	//
	//	Make sure we have a render object
	//
	RenderObjClass *model = Peek_Model ();
	WWASSERT (model != NULL);

	if (model != NULL) {

		//
		//	Should we play an animation?
		//
		const StringClass &anim_name = Get_Definition ().AnimationName;
		if (anim_name.Get_Length () > 0) {
			
			//
			//	Load the animation
			//
			HAnimClass *animation =  WW3DAssetManager::Get_Instance ()->Get_HAnim (anim_name);
			if (animation != NULL) {
				LifeRemaining = animation->Get_Total_Time ();
				model->Set_Animation (animation, 0, RenderObjClass::ANIM_MODE_ONCE);
				REF_PTR_RELEASE(animation);
			}
		}

		//
		//	Should we play a sound?
		//
		if (Get_Definition ().SoundDefID > 0) {
			WWAudioClass::Get_Instance()->Create_Instant_Sound (Get_Definition ().SoundDefID, Get_Transform ());
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObj::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK	(csave, VARID_LIFE_REMAINING, LifeRemaining);
	WRITE_MICRO_CHUNK	(csave, VARID_IS_INITIALIZED, IsInitialized);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObj::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk()) {
		switch (cload.Cur_Micro_Chunk_ID()) {

			READ_MICRO_CHUNK (cload, VARID_LIFE_REMAINING, LifeRemaining);
			READ_MICRO_CHUNK (cload, VARID_IS_INITIALIZED, IsInitialized);
			
			default:
				Debug_Say (("Unrecognized SpecialEffectsGameObj Variable chunkID %d\n", cload.Cur_Micro_Chunk_ID ()));
				break;
		}
		cload.Close_Micro_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Think
//
///////////////////////////////////////////////////////////////////////////
void
SpecialEffectsGameObj::Think (void)
{
	WWPROFILE( "SpecialEffects Think" );

	//
	//	Kick off the special effect
	//
	if (IsInitialized == false) {
		Do_Effect ();
		IsInitialized = true;
	}

	//
	//	Check to see if we should make ourselves go away
	//
	if (LifeRemaining >= 0) {
		LifeRemaining -= TimeManager::Get_Frame_Seconds ();
		if (LifeRemaining <= 0) {
			Set_Delete_Pending ();
		}
	}
	
	return ;
}

