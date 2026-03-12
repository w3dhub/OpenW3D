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
 *                     $Archive:: /Commando/Code/Combat/simplegameobj.cpp                     $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/10/01 1:43p                                              $*
 *                                                                                             *
 *                    $Revision:: 29                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "simplegameobj.h"
#include "combat.h"
#include "animcontrol.h"
#include "pscene.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "debug.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "phys.h"
#include "apppackettypes.h"


/*
** SimpleGameObjDef
*/
DECLARE_FORCE_LINK( Simple )

SimplePersistFactoryClass<SimpleGameObjDef, CHUNKID_GAME_OBJECT_DEF_SIMPLE>	_SimpleGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(SimpleGameObjDef, CLASSID_GAME_OBJECT_DEF_SIMPLE, "Simple") _SimpleGameObjDefDefFactory;

SimpleGameObjDef::SimpleGameObjDef( void ) :
	IsEditorObject( false ),
	IsHiddenObject( false ),
	PlayerTerminalType( PlayerTerminalClass::TYPE_NONE )
{
	MODEL_DEF_PARAM( SimpleGameObjDef, PhysDefID, "PhysDef" );
	EDITABLE_PARAM( SimpleGameObjDef, ParameterClass::TYPE_BOOL, IsEditorObject );
	EDITABLE_PARAM( SimpleGameObjDef, ParameterClass::TYPE_BOOL, IsHiddenObject );

#ifdef	PARAM_EDITING_ON

	//
	//	Configure the orator types parameter
	//
	EnumParameterClass *pt_type_param = new EnumParameterClass( (int *)&PlayerTerminalType );
	pt_type_param->Set_Name( "Player Terminal Type" );
	pt_type_param->Add_Value( "<None>", PlayerTerminalClass::TYPE_NONE );
	pt_type_param->Add_Value( "GDI",		PlayerTerminalClass::TYPE_GDI );
	pt_type_param->Add_Value( "NOD",		PlayerTerminalClass::TYPE_NOD );
	pt_type_param->Add_Value( "Mutant", PlayerTerminalClass::TYPE_MUTANT );
	GENERIC_EDITABLE_PARAM( SimpleGameObjDef, pt_type_param );
#endif

	return ;
}

uint32	SimpleGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_SIMPLE;
}

PersistClass *	SimpleGameObjDef::Create( void ) const
{
	SimpleGameObj * obj = new SimpleGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							=	930991656,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_IS_EDITOR_OBJECT		=	1,
	MICROCHUNKID_DEF_IS_HIDDEN_OBJECT,
	MICROCHUNKID_DEF_PLAYER_TERM_TYPE
};

bool	SimpleGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		PhysicalGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_EDITOR_OBJECT, IsEditorObject );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_IS_HIDDEN_OBJECT, IsHiddenObject );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_PLAYER_TERM_TYPE, PlayerTerminalType );
	csave.End_Chunk();

	return true;
}

bool	SimpleGameObjDef::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				PhysicalGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_EDITOR_OBJECT, IsEditorObject );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_IS_HIDDEN_OBJECT, IsHiddenObject );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_PLAYER_TERM_TYPE, PlayerTerminalType );

						default:
							Debug_Say(( "Unrecognized SimpleDef Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized SimpleDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & SimpleGameObjDef::Get_Factory (void) const
{
	return _SimpleGameObjDefPersistFactory;
}


/*
** SimpleGameObj
*/
SimplePersistFactoryClass<SimpleGameObj, CHUNKID_GAME_OBJECT_SIMPLE>	_SimpleGameObjPersistFactory;

const PersistFactoryClass & SimpleGameObj::Get_Factory (void) const
{
	return _SimpleGameObjPersistFactory;
}

/*
**
*/
SimpleGameObj::SimpleGameObj()
{
	Set_App_Packet_Type(APPPACKETTYPE_SIMPLE);

	//WWDEBUG_SAY(("SimpleGameObj::SimpleGameObj\n"));

	/*
	if (Get_Definition().Get_Name() != NULL) {
		WWDEBUG_SAY(("  for %s\n", Get_Definition().Get_Name()));
	}
	/**/
}

SimpleGameObj::~SimpleGameObj()
{
}

/*
**
*/
void SimpleGameObj::Init( void )
{
	Init( Get_Definition() );

	/*
	if (Get_Definition().Get_Name() != NULL) {
		WWDEBUG_SAY(("SimpleGameObj::Init for %s\n", Get_Definition().Get_Name()));
	}
	/**/
}

void	SimpleGameObj::Init( const SimpleGameObjDef & definition )
{
	PhysicalGameObj::Init( definition );
}

const SimpleGameObjDef & SimpleGameObj::Get_Definition( void ) const
{
	return (const SimpleGameObjDef &)BaseGameObj::Get_Definition();
}


/*
** SimpleGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	927991712,
	CHUNKID_VARIABLES,
	XXXCHUNKID_ANIM_CONTROL,

	XXXMICROCHUNKID_PHYSOBJ					=	1,
};

bool	SimpleGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		PhysicalGameObj::Save( csave );
	csave.End_Chunk();

	return true;
}

bool	SimpleGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				PhysicalGameObj::Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized SimpleGameObj chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

void	SimpleGameObj::On_Post_Load( void )
{
	PhysicalGameObj::On_Post_Load();

	// NOTE: the On_Post_Load function is only run when loading a level in the game engine
	// so we can put game-specific behavior into this function without messing up the level
	// editor.
	if ( Get_Definition().IsEditorObject ) {
		// Switch to a NULL model
		Peek_Physical_Object()->Set_Model_By_Name( "NULL" );
		// Add clear anim_control
		if ( Get_Anim_Control() != NULL ) {
			Get_Anim_Control()->Set_Model( Peek_Model() );
		}
	}

	if ( Is_Hidden_Object() ) {
		RenderObjClass * model = Peek_Physical_Object()->Peek_Model();
		if (model) {
			model->Set_Hidden(true);
		}
	}

	/*
	if (Get_Definition().Get_Name() != NULL) {
		WWDEBUG_SAY(("SimpleGameObj::On_Post_Load for %s\n", Get_Definition().Get_Name()));
	}
	*/
}



