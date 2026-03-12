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
 *                     $Archive:: /Commando/Code/Combat/transitiongameobj.cpp                 $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/17/01 10:28a                                             $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	Includes
*/
#include "transitiongameobj.h"
#include "debug.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "simpledefinitionfactory.h"
#include "combat.h"

/*
** TransitionGameObjDef
*/
DECLARE_FORCE_LINK( Transition )

SimplePersistFactoryClass<TransitionGameObjDef, CHUNKID_GAME_OBJECT_DEF_TRANSITION>	_TransitionGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(TransitionGameObjDef, CLASSID_GAME_OBJECT_DEF_TRANSITION, "Transition") _TransitionGameObjDefDefFactory;

TransitionGameObjDef::~TransitionGameObjDef( void )
{
	Free_Transition_List();
}

uint32	TransitionGameObjDef::Get_Class_ID (void) const
{
	return CLASSID_GAME_OBJECT_DEF_TRANSITION;
}

PersistClass *	TransitionGameObjDef::Create( void ) const
{
	TransitionGameObj * obj = new TransitionGameObj;
	obj->Init( *this );
	return obj;
}

enum	{
	CHUNKID_DEF_PARENT							= 1111991201,
	CHUNKID_DEF_TRANSITION,
};

bool	TransitionGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		BaseGameObjDef::Save( csave );
	csave.End_Chunk();

	/*
	**	Save each of the transition 'definitions' in our list.
	*/
	for( int index = 0; index < Transitions.Count (); index ++ ) {
		TransitionDataClass *transition = Transitions[index];
		if( transition != NULL ) {

			/*
			**	Save this transition 'defintion' to its own chunk.
			*/
			csave.Begin_Chunk( CHUNKID_DEF_TRANSITION );
			transition->Save( csave );
			csave.End_Chunk();
		}
	}

	return true;
}

bool	TransitionGameObjDef::Load( ChunkLoadClass &cload )
{
	Free_Transition_List ();

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				BaseGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_TRANSITION:
			{
				TransitionDataClass *transition = new TransitionDataClass;
				transition->Load( cload );
				Transitions.Add( transition );
			}
			break;

			default:
				Debug_Say(( "Unrecognized TransitionDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

/*
**
*/
void TransitionGameObjDef::Free_Transition_List( void )
{
	/*
	**	Delete each of the transition 'definitions' in our list.
	*/
	for( int index = 0; index < Transitions.Count (); index ++ ) {
		TransitionDataClass *transition = Transitions[index];
		if( transition != NULL ) {
			delete transition;
		}
	}

	Transitions.Delete_All();
	return ;
}

const PersistFactoryClass & TransitionGameObjDef::Get_Factory (void) const
{
	return _TransitionGameObjDefPersistFactory;
}


/*
** TransitionGameObj
*/
SimplePersistFactoryClass<TransitionGameObj, CHUNKID_GAME_OBJECT_TRANSITION>	_TransitionGameObjPersistFactory;

const PersistFactoryClass & TransitionGameObj::Get_Factory (void) const
{
	return _TransitionGameObjPersistFactory;
}

TransitionGameObj::TransitionGameObj()	:
	LadderIndex( -1 )
{
}

TransitionGameObj::~TransitionGameObj()
{
	Destroy_Transitions();
}

/*
**
*/
void TransitionGameObj::Init( void )
{
	Init( Get_Definition() );
}

void	TransitionGameObj::Init( const TransitionGameObjDef & definition )
{
	BaseGameObj::Init( definition );

	Create_Transitions();
}

const TransitionGameObjDef & TransitionGameObj::Get_Definition( void ) const
{
	return (const TransitionGameObjDef &)BaseGameObj::Get_Definition();
}

/*
** TransitionGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	1111991206,
	CHUNKID_VARIABLES,

	MICROCHUNKID_TRANSFORM				= 1,
	MICROCHUNKID_LADDER_INDEX
};

bool	TransitionGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	BaseGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
	WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TRANSFORM,		TM );
	WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LADDER_INDEX,	LadderIndex );
	csave.End_Chunk();

	return true;
}

bool	TransitionGameObj::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				BaseGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
			{
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TRANSFORM,		TM );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LADDER_INDEX,	LadderIndex );
						default:
							Debug_Say(( "Unrecognized TransitionGameObj Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}

				break;
			}

			default:
				Debug_Say(( "Unrecognized Transition chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void	TransitionGameObj::On_Post_Load( void )
{
	BaseGameObj::On_Post_Load();
	Create_Transitions();
	return ;
}

void	TransitionGameObj::Set_Transform(const Matrix3D & tm)
{
	TM = tm;
}

const Matrix3D &	TransitionGameObj::Get_Transform(void) const
{
	return TM;
}

void	TransitionGameObj::Create_Transitions( void )
{
	Destroy_Transitions();

	// Only create the transitions if they haven't already been created
	WWASSERT( TransitionInstances.Count() == 0 );
	if( TransitionInstances.Count() == 0 ) {

		const TRANSITION_DATA_LIST & trans_data_list = Get_Definition().Get_Transition_List();
		Matrix3D tm = Get_Transform();

		for ( int i = 0; i < trans_data_list.Count(); i++ ) {
			// make new instance
			TransitionInstanceClass * trans = new TransitionInstanceClass( *trans_data_list[i] );
			// setup
			trans->Set_Parent_Transform( tm );
			// add to our list
			TransitionInstances.Add( trans );
			// add to master list
			TransitionManager::Add( trans );
		}
	}

	//
	//	Ensure all the transition instances know what ladder they belong to (may
	// not apply for other transitions other than ladder).
	//
	Set_Ladder_Index( LadderIndex );
	return ;
}

void	TransitionGameObj::Destroy_Transitions( void )
{
	while ( TransitionInstances.Count() ) {
  		TransitionManager::Destroy( TransitionInstances[0] );
  		TransitionInstances.Delete( 0 );
	}

	// They get put in a list, so flush the list.
	TransitionManager::Destroy_Pending();
}

void	TransitionGameObj::Update_Transitions( void )
{
	Matrix3D tm = Get_Transform();

	// Update all transitions
	for ( int i = 0; i < TransitionInstances.Count(); i++ ) {
		TransitionInstances[i]->Set_Parent_Transform( tm );
		TransitionInstances[i]->Set_Ladder_Index( LadderIndex );
	}
}

void	TransitionGameObj::Set_Ladder_Index( int ladder_index )
{
	LadderIndex = ladder_index;

	// Update all transitions
	for ( int i = 0; i < TransitionInstances.Count(); i++ ) {
		TransitionInstances[i]->Set_Ladder_Index( LadderIndex );
	}

	return ;
}
