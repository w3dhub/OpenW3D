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
 *                     $Archive:: /Commando/Code/Combat/buildingaggregate.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/07/03 1:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BuildingAggregateClass::BuildingAggregateClass -- Constructor                             *
 *   BuildingAggregateClass::BuildingAggregateClass -- Constructor                             *
 *   BuildingAggregateClass::~BuildingAggregateClass -- Destructor                             *
 *   BuildingAggregateClass::Init -- Initialze from a definition                               *
 *   BuildingAggregateClass::Get_Current_State -- returns the current state                    *
 *   BuildingAggregateClass::Set_Current_State -- Sets the current state                       *
 *   BuildingAggregateClass::Is_MCT -- is this aggregate an MCT?                               *
 *   BuildingAggregateClass::Save -- Get the persist factory for this clas                     *
 *   BuildingAggregateClass::Save -- Save the state of this object                             *
 *   BuildingAggregateClass::Load -- Load the state of this object                             *
 *   BuildingAggregateClass::On_Post_Load -- post-load processing                              *
 *   BuildingAggregateDefClass::BuildingAggregateDefClass -- Constructor                       *
 *   BuildingAggregateDefClass::Get_Class_ID -- returns the definition class ID                *
 *   BuildingAggregateDefClass::Create -- creates an instance of a BuildingAggregateClass      *
 *   BuildingAggregateDefClass::Get_Factory -- returns the persist factory                     *
 *   BuildingAggregateDefClass::Save -- Saves the contents of this object                      *
 *   BuildingAggregateDefClass::Save_State_Animation_Data -- Save the data for one of the buil *
 *   BuildingAggregateDefClass::Load -- Load the contents of this object                       *
 *   BuildingAggregateDefClass::Load_State_Animation_Data -- Load anim data for a building sta *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "buildingaggregate.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "hanim.h"
#include "combat.h"
#include "WWAudio.h"
#include "wwdebug.h"
#include "wwhack.h"
#include "wwprofile.h"


DECLARE_FORCE_LINK( buildingaggregate );


/*************************************************************************************************************
**
** BuildingAggregateClass (BAG) Implementation
**
*************************************************************************************************************/

SimplePersistFactoryClass<BuildingAggregateClass, PHYSICS_CHUNKID_BUILDINGAGGREGATE>	_BuildingAggregatePersistFactory;

enum	
{
	BAG_CHUNK_STATICANIMPHYS			=	8281529,
	BAG_CHUNK_VARIABLES,

	BAG_VARIABLE_CURRENTSTATE			=	0,
};


/***********************************************************************************************
 * BuildingAggregateClass::BuildingAggregateClass -- Constructor                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
BuildingAggregateClass::BuildingAggregateClass(void) :
	CurrentState(BuildingStateClass::HEALTH100_POWERON)
{
}


/***********************************************************************************************
 * BuildingAggregateClass::~BuildingAggregateClass -- Destructor                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
BuildingAggregateClass::~BuildingAggregateClass(void)
{
}


/***********************************************************************************************
 * BuildingAggregateClass::Init -- Initialze from a definition                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void BuildingAggregateClass::Init(const BuildingAggregateDefClass & def)
{
	StaticAnimPhysClass::Init(def);
	AnimCollisionManagerClass & anim_mgr = Get_Animation_Manager();
	anim_mgr.Set_Current_Frame(def.Frame0[CurrentState]);
}


/***********************************************************************************************
 * BuildingAggregateClass::Get_Current_State -- returns the current state                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/8/2000   gth : Created.                                                                 *
 *=============================================================================================*/
int BuildingAggregateClass::Get_Current_State(void)
{
	return CurrentState;
}


/***********************************************************************************************
 * BuildingAggregateClass::Set_Current_State -- Sets the current state                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/8/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void BuildingAggregateClass::Set_Current_State(int new_state,bool force_update)
{
	if ((new_state != CurrentState) || (force_update == true)) {
		AnimCollisionManagerClass & anim_mgr = Get_Animation_Manager();
		const BuildingAggregateDefClass * def = Get_BuildingAggregateDef();
		WWASSERT(def != NULL);
		
		switch (def->AnimLogicMode)
		{
			case BuildingAggregateDefClass::ANIM_LOGIC_LOOP:
			{

				/*
				** Calculate the fractional position in our current loop
				*/
				float normalized_frame = 0.0f;
				float cur_loop_len = def->Frame1[CurrentState] - def->Frame0[CurrentState];
				
				if (cur_loop_len > 0.0f) {
					normalized_frame = (anim_mgr.Get_Current_Frame() - def->Frame0[CurrentState]) / 
														(def->Frame1[CurrentState] - def->Frame0[CurrentState]);
				}

				/*
				** Switch the loop start and end
				*/
				anim_mgr.Set_Loop_Start(def->Frame0[new_state]);
				anim_mgr.Set_Loop_End(def->Frame1[new_state]);

				/*
				** Jump to the same fractional position in the new loop
				*/
				anim_mgr.Set_Current_Frame(def->Frame0[new_state] + normalized_frame * (def->Frame1[new_state] - def->Frame0[new_state]));

				/*
				** If animation is disabled, set the animation mode to manual, otherwise set it to loop
				*/
				if (def->AnimationEnabled[new_state] == true) {
					anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_LOOP);
				} else {
					anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_MANUAL);
				}
				break;
			}
			case BuildingAggregateDefClass::ANIM_LOGIC_LINEAR:
			{
				/*
				** The new target frame is determined by the new state
				*/
				anim_mgr.Set_Target_Frame(def->Frame0[new_state]);
				anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);

				/*
				** If we changed power states or animation is disabled, then we need to warp the current frame
				*/
				if (	(BuildingStateClass::Is_Power_On(new_state) != BuildingStateClass::Is_Power_On(CurrentState)) ||
						(def->AnimationEnabled[new_state] == false)) 
				{
					anim_mgr.Set_Current_Frame(def->Frame0[new_state]);
				}
				break;
			}
			case BuildingAggregateDefClass::ANIM_LOGIC_SEQUENCE:
			{
				/*
				** The current and target frames are determined by the new state.  If the new sequence
				** is the same as the previous sequence, don't do anything.
				*/
				if ((def->Frame0[CurrentState] != def->Frame0[new_state]) || (def->Frame1[CurrentState] != def->Frame1[new_state])) {
					anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);
					anim_mgr.Set_Current_Frame(def->Frame0[new_state]);
					anim_mgr.Set_Target_Frame(def->Frame1[new_state]);
				}
				break;
			}
		}

		CurrentState = new_state;
	}
}


/***********************************************************************************************
 * BuildingAggregateClass::Is_MCT -- is this aggregate an MCT?                                 *
 *                                                                                             *
 *    MCT's are weak points for the building.  The building will use an alternate skin type    *
 *    when applying damage that was done to the MCT.                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateClass::Is_MCT(void) 
{
	return Get_BuildingAggregateDef()->IsMCT;
}

/*
** Save and Load
*/


/***********************************************************************************************
 * BuildingAggregateClass::Save -- Get the persist factory for this class                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & BuildingAggregateClass::Get_Factory(void) const 
{
	return _BuildingAggregatePersistFactory;
}


/***********************************************************************************************
 * BuildingAggregateClass::Save -- Save the state of this object                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(BAG_CHUNK_STATICANIMPHYS);
		StaticAnimPhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(BAG_CHUNK_VARIABLES);
		WRITE_MICRO_CHUNK(csave, BAG_VARIABLE_CURRENTSTATE, CurrentState );
	csave.End_Chunk();

	return true;
}


/***********************************************************************************************
 * BuildingAggregateClass::Load -- Load the state of this object                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case BAG_CHUNK_STATICANIMPHYS:
				StaticAnimPhysClass::Load( cload );
				break;
		
			case BAG_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, BAG_VARIABLE_CURRENTSTATE, CurrentState );
						default:
							WWDEBUG_SAY(( "Unrecognized BuildingAggregate Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unrecognized BuildingAggregate Chunk: 0x%x in file %s, line %d\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}


/***********************************************************************************************
 * BuildingAggregateClass::On_Post_Load -- post-load processing                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/8/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void BuildingAggregateClass::On_Post_Load(void)
{
	StaticAnimPhysClass::On_Post_Load();
	Set_Current_State(CurrentState,true);
}


/*
**
*/
void	BuildingAggregateClass::Save_State( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( BAG_CHUNK_STATICANIMPHYS );
		StaticAnimPhysClass::Save_State(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(BAG_CHUNK_VARIABLES);
		WRITE_MICRO_CHUNK(csave, BAG_VARIABLE_CURRENTSTATE, CurrentState );
	csave.End_Chunk();
}

/*
**
*/
void	BuildingAggregateClass::Load_State( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case BAG_CHUNK_STATICANIMPHYS:
				StaticAnimPhysClass::Load_State( cload );
				break;
		
			case BAG_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, BAG_VARIABLE_CURRENTSTATE, CurrentState );
						default:
							WWDEBUG_SAY(( "Unrecognized BuildingAggregate Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(( "Unrecognized BuildingAggregate chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

}



/*************************************************************************************************
**
** BuildingAggregateDefClass Implementation
**
*************************************************************************************************/

/*
** Persist Factory for BuildingAggregateDefClass, this makes them save and load.
*/
SimplePersistFactoryClass<BuildingAggregateDefClass, PHYSICS_CHUNKID_BUILDINGAGGREGATEDEF>	_BuildingAggregateDefPersistFactory;

/*
** Definition Factory for BuildingAggregateDefClass, this makes them show up in the editor
*/
DECLARE_DEFINITION_FACTORY(BuildingAggregateDefClass, CLASSID_BUILDINGAGGREGATEDEF, "BuildingAggregate") _BuildingAggregateDefDefFactory;

/*
** Chunk ID's used by BuildingAggregateDefClass
*/
enum	
{
	BAGDEF_CHUNK_STATICANIMPHYS					= 8281441,
	BAGDEF_CHUNK_VARIABLES,

	BAGDEF_CHUNK_HEALTH100_POWERON_VARIABLES,						
	BAGDEF_CHUNK_HEALTH75_POWERON_VARIABLES,						
	BAGDEF_CHUNK_HEALTH50_POWERON_VARIABLES,						
	BAGDEF_CHUNK_HEALTH25_POWERON_VARIABLES,						
	BAGDEF_CHUNK_DESTROYED_POWERON_VARIABLES,						
	BAGDEF_CHUNK_HEALTH100_POWEROFF_VARIABLES,					
	BAGDEF_CHUNK_HEALTH75_POWEROFF_VARIABLES,						
	BAGDEF_CHUNK_HEALTH50_POWEROFF_VARIABLES,						
	BAGDEF_CHUNK_HEALTH25_POWEROFF_VARIABLES,						
	BAGDEF_CHUNK_DESTROYED_POWEROFF_VARIABLES,					

	BAGDEF_VARIABLE_ANIMLOGICMODE					= 0,
	BAGDEF_VARIABLE_FRAME0,
	BAGDEF_VARIABLE_FRAME1,
	BAGDEF_VARIABLE_ANIMATIONENABLED,
	BAGDEF_VARIABLE_ISMCT,

};





/***********************************************************************************************
 * BuildingAggregateDefClass::BuildingAggregateDefClass -- Constructor                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
BuildingAggregateDefClass::BuildingAggregateDefClass( void ) : 
	StaticAnimPhysDefClass(),
	AnimLogicMode(ANIM_LOGIC_LOOP),
	IsMCT(false)
{
	int i;
	for (i=0; i<BuildingStateClass::STATE_COUNT; i++) {
		Frame0[i] = 0;
		Frame1[i] = 0;
		AnimationEnabled[i] = true;
	}

#ifdef PARAM_EDITING_ON
	PARAM_SEPARATOR(BuildingAggregateDefClass, "Building Behavior Settings");

	EnumParameterClass * anim_logic_param = new EnumParameterClass(&AnimLogicMode);		
	anim_logic_param->Set_Name ("AnimLogicMode");
	anim_logic_param->Add_Value("ANIM_LOGIC_LINEAR",ANIM_LOGIC_LINEAR);
	anim_logic_param->Add_Value("ANIM_LOGIC_LOOP",ANIM_LOGIC_LOOP);
	anim_logic_param->Add_Value("ANIM_LOGIC_SEQUENCE",ANIM_LOGIC_SEQUENCE);
	GENERIC_EDITABLE_PARAM( BuildingAggregateDefClass, anim_logic_param);

	EDITABLE_PARAM(BuildingAggregateDefClass, ParameterClass::TYPE_BOOL, IsMCT);

	for (i=0; i<BuildingStateClass::STATE_COUNT; i++) {
		
		PARAM_SEPARATOR(BuildingAggregateDefClass, BuildingStateClass::Get_State_Name(i))
		NAMED_EDITABLE_PARAM(BuildingAggregateDefClass, ParameterClass::TYPE_INT, Frame0[i], "Frame0");
		NAMED_EDITABLE_PARAM(BuildingAggregateDefClass, ParameterClass::TYPE_INT, Frame1[i], "Frame1");
		NAMED_EDITABLE_PARAM(BuildingAggregateDefClass, ParameterClass::TYPE_BOOL, AnimationEnabled[i], "AnimationEnabled");
		
	}
#endif
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Get_Class_ID -- returns the definition class ID                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
uint32 BuildingAggregateDefClass::Get_Class_ID(void) const	
{ 
	return CLASSID_BUILDINGAGGREGATEDEF; 
}

bool BuildingAggregateDefClass::Is_Type(const char * type_name)
{
	if (openw3d::string_compare(type_name,BuildingAggregateDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return StaticAnimPhysDefClass::Is_Type(type_name);
	}
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Create -- creates an instance of a BuildingAggregateClass        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
PersistClass *	BuildingAggregateDefClass::Create(void) const 
{
	BuildingAggregateClass * obj = new BuildingAggregateClass;
	obj->Init( *this );
	return obj;
}

/*
** Save and Load
*/

/***********************************************************************************************
 * BuildingAggregateDefClass::Get_Factory -- returns the persist factory                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & BuildingAggregateDefClass::Get_Factory(void) const 
{ 
	return _BuildingAggregateDefPersistFactory; 
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Save -- Saves the contents of this object                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateDefClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk( BAGDEF_CHUNK_STATICANIMPHYS );
	StaticAnimPhysDefClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( BAGDEF_CHUNK_VARIABLES );
	WRITE_MICRO_CHUNK( csave, BAGDEF_VARIABLE_ANIMLOGICMODE, AnimLogicMode );
	WRITE_MICRO_CHUNK( csave, BAGDEF_VARIABLE_ISMCT, IsMCT );
	csave.End_Chunk();

	for (int i=0; i<BuildingStateClass::STATE_COUNT; i++) {
		Save_State_Animation_Data(csave,i);
	}

	return true;
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Save_State_Animation_Data -- Save the data for one of the buildi *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateDefClass::Save_State_Animation_Data(ChunkSaveClass & csave,int state_index)
{
	WWASSERT(state_index >= 0);
	WWASSERT(state_index < BuildingStateClass::STATE_COUNT);

	csave.Begin_Chunk(BAGDEF_CHUNK_HEALTH100_POWERON_VARIABLES + state_index);
	WRITE_MICRO_CHUNK(csave,BAGDEF_VARIABLE_FRAME0,Frame0[state_index]);
	WRITE_MICRO_CHUNK(csave,BAGDEF_VARIABLE_FRAME1,Frame1[state_index]);
	WRITE_MICRO_CHUNK(csave,BAGDEF_VARIABLE_ANIMATIONENABLED,AnimationEnabled[state_index]);
	csave.End_Chunk();
	return true;
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Load -- Load the contents of this object                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool	BuildingAggregateDefClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case BAGDEF_CHUNK_STATICANIMPHYS:
				StaticAnimPhysDefClass::Load( cload );
				break;
  
			case BAGDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, BAGDEF_VARIABLE_ANIMLOGICMODE,AnimLogicMode );
						READ_MICRO_CHUNK( cload, BAGDEF_VARIABLE_ISMCT, IsMCT );
						default:
							WWDEBUG_SAY(( "Unrecognized BuildingAggregateDef Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;
			
			case BAGDEF_CHUNK_HEALTH100_POWERON_VARIABLES:
			case BAGDEF_CHUNK_HEALTH75_POWERON_VARIABLES:
			case BAGDEF_CHUNK_HEALTH50_POWERON_VARIABLES:
			case BAGDEF_CHUNK_HEALTH25_POWERON_VARIABLES:
			case BAGDEF_CHUNK_DESTROYED_POWERON_VARIABLES:
			case BAGDEF_CHUNK_HEALTH100_POWEROFF_VARIABLES:
			case BAGDEF_CHUNK_HEALTH75_POWEROFF_VARIABLES:
			case BAGDEF_CHUNK_HEALTH50_POWEROFF_VARIABLES:
			case BAGDEF_CHUNK_HEALTH25_POWEROFF_VARIABLES:
			case BAGDEF_CHUNK_DESTROYED_POWEROFF_VARIABLES:
				Load_State_Animation_Data(cload,cload.Cur_Chunk_ID() - BAGDEF_CHUNK_HEALTH100_POWERON_VARIABLES);
				break;

			default:
				WWDEBUG_SAY(( "Unrecognized BuildingAggregateDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


/***********************************************************************************************
 * BuildingAggregateDefClass::Load_State_Animation_Data -- Load anim data for a building state *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool BuildingAggregateDefClass::Load_State_Animation_Data(ChunkLoadClass & cload,int state_index)
{
	WWASSERT(state_index >= 0);
	WWASSERT(state_index < BuildingStateClass::STATE_COUNT);

	while (cload.Open_Micro_Chunk()) {
		switch(cload.Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK(cload,BAGDEF_VARIABLE_FRAME0,Frame0[state_index]);
			READ_MICRO_CHUNK(cload,BAGDEF_VARIABLE_FRAME1,Frame1[state_index]);
			READ_MICRO_CHUNK(cload,BAGDEF_VARIABLE_ANIMATIONENABLED,AnimationEnabled[state_index]);
			default:
				WWDEBUG_SAY(( "Unrecognized BuildingAggregateDef Variable chunkID\n" ));
				break;
		}
		cload.Close_Micro_Chunk();
	}
	return true;
}

