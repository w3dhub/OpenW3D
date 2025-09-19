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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/damageablestaticphys.cpp              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 7:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "damageablestaticphys.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "hanim.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "combat.h"
#include "explosion.h"


DECLARE_FORCE_LINK( damageablestaticphys );

/*
** Chunk Id's used by DamageableStaticPhysDefClass
*/
enum	
{
	DAMAGEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF		= 7311734,
	DAMAGEABLESTATICPHYSDEF_CHUNK_VARIABLES,
	DAMAGEABLESTATICPHYSDEF_CHUNK_DEFENSEOBJECTDEF,

	DAMAGEABLESTATICPHYSDEF_VARIABLE_KILLEDEXPLOSION	= 0x00,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_RESETAFTERANIM,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPSTART,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPEND,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHSTART,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHEND,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONSTART,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONEND,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPSTART,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPEND,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHSTART,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHEND,
	DAMAGEABLESTATICPHYSDEF_VARIABLE_PLAYTWITCHESTOCOMPLETION,
};

/*
** Chunk Id's used by DamageableStaticPhysClass
*/
enum 
{
	DAMAGEABLESTATICPHYS_CHUNK_STATICANIMPHYS				= 7311740,
	DAMAGEABLESTATICPHYS_CHUNK_DEFENSEOBJECT,
	DAMAGEABLESTATICPHYS_CHUNK_VARIABLES,

	DAMAGEABLESTATICPHYS_VARIABLE_CURSTATE					= 0x00,
};


/************************************************************************************************
**
** DamageableStaticPhysDefClass Implementation
**
************************************************************************************************/
SimplePersistFactoryClass<DamageableStaticPhysDefClass, PHYSICS_CHUNKID_DAMAGEABLESTATICPHYSDEF>	_DamageableStaticPhysDefPersistFactory;

DECLARE_DEFINITION_FACTORY(DamageableStaticPhysDefClass, CLASSID_DAMAGEABLESTATICPHYSDEF, "DamageableStaticPhys") _DamageableStaticPhysDefDefFactory;


DamageableStaticPhysDefClass::DamageableStaticPhysDefClass(void) : 
	StaticAnimPhysDefClass(),
	KilledExplosion(0),
	LiveLoopStart(0),
	LiveLoopEnd(0),
	LiveTwitchStart(0),
	LiveTwitchEnd(0),
	DeathTransitionStart(0),
	DeathTransitionEnd(0),
	DeadLoopStart(0),
	DeadLoopEnd(0),
	DeadTwitchStart(0),
	DeadTwitchEnd(0),
	PlayTwitchesToCompletion(false)
{
	PARAM_SEPARATOR(DamageableStaticPhysDefClass, "Damage Behavior Controls");

	EDITABLE_PARAM(DamageableStaticPhysDefClass, ParameterClass::TYPE_EXPLOSIONDEFINITIONID,	KilledExplosion);
	
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, LiveLoopStart, 0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, LiveLoopEnd, 0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, LiveTwitchStart, 0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, LiveTwitchEnd,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeathTransitionStart,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeathTransitionEnd,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeadLoopStart,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeadLoopEnd,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeadTwitchStart,0, 999);
	INT_EDITABLE_PARAM(DamageableStaticPhysDefClass, DeadTwitchEnd,0, 999);
	EDITABLE_PARAM(DamageableStaticPhysDefClass,ParameterClass::TYPE_BOOL,PlayTwitchesToCompletion);

	DEFENSEOBJECTDEF_EDITABLE_PARAMS(DamageableStaticPhysDefClass, DefenseObjectDef);
}

uint32 DamageableStaticPhysDefClass::Get_Class_ID(void) const	
{ 
	return CLASSID_DAMAGEABLESTATICPHYSDEF; 
}

bool DamageableStaticPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::string_compare(type_name,DamageableStaticPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return StaticAnimPhysDefClass::Is_Type(type_name);
	}
}

PersistClass *	DamageableStaticPhysDefClass::Create(void) const 
{
	DamageableStaticPhysClass * obj = new DamageableStaticPhysClass;
	obj->Init(*this);
	return obj;
}


bool	DamageableStaticPhysDefClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(DAMAGEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF);
		StaticAnimPhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(DAMAGEABLESTATICPHYSDEF_CHUNK_VARIABLES);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_KILLEDEXPLOSION, KilledExplosion);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPSTART,LiveLoopStart);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPEND,LiveLoopEnd);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHSTART,LiveTwitchStart);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHEND,LiveTwitchEnd);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONSTART,DeathTransitionStart);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONEND,DeathTransitionEnd);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPSTART,DeadLoopStart);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPEND,DeadLoopEnd);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHSTART,DeadTwitchStart);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHEND,DeadTwitchEnd);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYSDEF_VARIABLE_PLAYTWITCHESTOCOMPLETION,PlayTwitchesToCompletion);
	csave.End_Chunk();

	csave.Begin_Chunk(DAMAGEABLESTATICPHYSDEF_CHUNK_DEFENSEOBJECTDEF);
		DefenseObjectDef.Save(csave);
	csave.End_Chunk();

	return true;
}

bool	DamageableStaticPhysDefClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case DAMAGEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF:
				StaticAnimPhysDefClass::Load( cload );
				break;
  
			case DAMAGEABLESTATICPHYSDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_KILLEDEXPLOSION, KilledExplosion);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPSTART,LiveLoopStart);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVELOOPEND,LiveLoopEnd);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHSTART,LiveTwitchStart);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_LIVETWITCHEND,LiveTwitchEnd);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONSTART,DeathTransitionStart);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEATHTRANSITIONEND,DeathTransitionEnd);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPSTART,DeadLoopStart);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADLOOPEND,DeadLoopEnd);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHSTART,DeadTwitchStart);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_DEADTWITCHEND,DeadTwitchEnd);
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYSDEF_VARIABLE_PLAYTWITCHESTOCOMPLETION,PlayTwitchesToCompletion);
						default:
							WWDEBUG_SAY(( "Unrecognized DamageableStaticPhysDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case DAMAGEABLESTATICPHYSDEF_CHUNK_DEFENSEOBJECTDEF:
				DefenseObjectDef.Load(cload);
				break;

			default:
				WWDEBUG_SAY(( "Unrecognized DamageableStaticPhysDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & DamageableStaticPhysDefClass::Get_Factory (void) const 
{ 
	return _DamageableStaticPhysDefPersistFactory; 
}



/************************************************************************************************
**
** DamageableStaticPhysClass Implementation
**
************************************************************************************************/
SimplePersistFactoryClass<DamageableStaticPhysClass, PHYSICS_CHUNKID_DAMAGEABLESTATICPHYS>	_DamageableStaticPhysPersistFactory;

const PersistFactoryClass & DamageableStaticPhysClass::Get_Factory (void) const 
{
	return _DamageableStaticPhysPersistFactory;
}

DamageableStaticPhysClass::DamageableStaticPhysClass(void) :
	CurState(STATE_LIVE_LOOP)
{
}

DamageableStaticPhysClass::~DamageableStaticPhysClass( void )
{
}

void DamageableStaticPhysClass::Init( const DamageableStaticPhysDefClass & def )
{
	// DamageableStaticPhys are not PhysicalGameObjs, so pass in a NULL owner
	DefenseObject.Init(def.DefenseObjectDef, NULL);
	StaticAnimPhysClass::Init(def);
	Start_Loop();
}

void DamageableStaticPhysClass::Timestep(float dt)
{
	StaticAnimPhysClass::Timestep(dt);

	/*
	** If we just reached a target, then transition into the loop for our current health state
	*/
	if (	(Get_Animation_Manager().Get_Animation_Mode() == AnimCollisionManagerClass::ANIMATE_TARGET) && 
			(Get_Animation_Manager().Is_At_Target()) ) 
	{
		Start_Loop();
	}

}


void DamageableStaticPhysClass::Apply_Damage_Static( const OffenseObjectClass & offense )
{
	/*
	** If we're already completely damaged, possibly start a twitch and then return
	*/
	if (DefenseObject.Get_Health() <= 0) {
		Play_Twitch();
		return;
	}

	/*
	** Apply the damage
	*/
	float new_health = DefenseObject.Apply_Damage(offense);

	/*
	** If we just lost all of our health, implement all of the
	** destruction effects. 
	*/
	if (new_health <= 0) {

		/*
		** Play our animation to the last frame
		*/
		Play_Death_Transition();

		/*
		** Set off an explosion if the user defined one
		*/
		const DamageableStaticPhysDefClass * def = Get_DamageableStaticPhysDef();
		if (def->KilledExplosion != 0) {
			ExplosionManager::Create_Explosion_At(def->KilledExplosion,Get_Transform(),NULL);
		}

	} else {
		
		/*
		** We were hit but didn't lose all of our health, so play a twitch
		*/
		Play_Twitch();
	}

}

void DamageableStaticPhysClass::Reset_Health(void)
{
	DefenseObject.Set_Health(DefenseObject.Get_Health_Max());
	Start_Loop();
}


void DamageableStaticPhysClass::Start_Loop(void) 
{
	const DamageableStaticPhysDefClass * def = Get_DamageableStaticPhysDef();
	if (def != NULL) {
	
		int frame0,frame1;

		if (DefenseObject.Get_Health() > 0) {
			frame0 = def->LiveLoopStart;
			frame1 = def->LiveLoopEnd;
			CurState = STATE_LIVE_LOOP;
		} else {
			frame0 = def->DeadLoopStart;
			frame1 = def->DeadLoopEnd;
			CurState = STATE_DEAD_LOOP;
		}
		
		if (frame0 != frame1) {
			Get_Animation_Manager().Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_LOOP);
			Get_Animation_Manager().Set_Current_Frame(frame0);
			Get_Animation_Manager().Set_Loop_Start(frame0);
			Get_Animation_Manager().Set_Loop_End(frame1);
		} else {
			Get_Animation_Manager().Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_MANUAL);
			Get_Animation_Manager().Set_Current_Frame(frame0);
		}
	} else {
		WWDEBUG_SAY(("ERROR: Missing definition for damageable object: %s\n",Model->Get_Name()));
	}
}

void DamageableStaticPhysClass::Play_Twitch(void)
{
	/*
	** Never interrupt death transitions.
	*/
	if (CurState == STATE_DEATH_TRANSITION) {
		return; 
	}

	/*
	** Play the appropriate twitch depending on whether we are alive or dead
	*/
	const DamageableStaticPhysDefClass * def = Get_DamageableStaticPhysDef();
	if (def != NULL) {

		/*
		** Don't interrupt other twitches if the designer doesn't want us to
		*/
		if ((def->PlayTwitchesToCompletion == false) || 
			 ((CurState != STATE_LIVE_TWITCH) && (CurState != STATE_DEAD_TWITCH)))
		{
			int frame0,frame1;

			if (DefenseObject.Get_Health() > 0) {
				frame0 = def->LiveTwitchStart;
				frame1 = def->LiveTwitchEnd;
				CurState = STATE_LIVE_TWITCH;
			} else {
				frame0 = def->DeadTwitchStart;
				frame1 = def->DeadTwitchEnd;
				CurState = STATE_DEAD_TWITCH;
			}
			
			if (frame0 != frame1) {
				Get_Animation_Manager().Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);
				Get_Animation_Manager().Set_Current_Frame(frame0);
				Get_Animation_Manager().Set_Target_Frame(frame1);
			}
		}

	} else {
		WWDEBUG_SAY(("ERROR: Missing definition for damageable object: %s\n",Model->Get_Name()));
	}

}

void DamageableStaticPhysClass::Play_Death_Transition(void)
{
	const DamageableStaticPhysDefClass * def = Get_DamageableStaticPhysDef();
	if (def != NULL) {

		if (def->DeathTransitionStart != def->DeathTransitionEnd) {
			CurState = STATE_DEATH_TRANSITION;
			Get_Animation_Manager().Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);
			Get_Animation_Manager().Set_Current_Frame(def->DeathTransitionStart);
			Get_Animation_Manager().Set_Target_Frame(def->DeathTransitionEnd);
		} else {
			Start_Loop();
		}
	
	} else {
		WWDEBUG_SAY(("ERROR: Missing definition for damageable object: %s\n",Model->Get_Name()));
	}
}

bool DamageableStaticPhysClass::Save( ChunkSaveClass & csave )
{
	/*
	** Save the parent class data
	*/
	csave.Begin_Chunk(DAMAGEABLESTATICPHYS_CHUNK_STATICANIMPHYS);
		StaticAnimPhysClass::Save( csave );
	csave.End_Chunk();

	/*
	** Save the contents of our damage object
	*/
	csave.Begin_Chunk(DAMAGEABLESTATICPHYS_CHUNK_DEFENSEOBJECT);
		DefenseObject.Save(csave);
	csave.End_Chunk();
	
	/*
	** Save our variables
	*/
	csave.Begin_Chunk(DAMAGEABLESTATICPHYS_CHUNK_VARIABLES);
		WRITE_MICRO_CHUNK(csave, DAMAGEABLESTATICPHYS_VARIABLE_CURSTATE, CurState);
	csave.End_Chunk();

	return true;
}


bool DamageableStaticPhysClass::Load(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case DAMAGEABLESTATICPHYS_CHUNK_STATICANIMPHYS:
				StaticAnimPhysClass::Load( cload );
				break;
		
			case DAMAGEABLESTATICPHYS_CHUNK_DEFENSEOBJECT:
				DefenseObject.Load(cload);
				break;

			case DAMAGEABLESTATICPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, DAMAGEABLESTATICPHYS_VARIABLE_CURSTATE, CurState);
						default:
							WWDEBUG_SAY(( "Unrecognized DamageableStaticPhys Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(( "Unrecognized DamageableStaticPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	Start_Loop();
	return true;
}



