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
 *                     $Archive:: /Commando/Code/Combat/persistentgameobjobserver.cpp         $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 3:32p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "persistentgameobjobserver.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "debug.h"

/*
**
*/
PersistentGameObjObserverClass::PersistentGameObjObserverClass( void )
{
	Set_ID( GameObjObserverManager::Get_Next_Observer_ID() );
	PersistentGameObjObserverManager::Add( this );
}

PersistentGameObjObserverClass::~PersistentGameObjObserverClass( void )
{
	PersistentGameObjObserverManager::Remove( this );
}

enum	{
	CHUNKID_PARENT							=	 411001149,
	CHUNKID_VARIABLES,

	MICROCHUNKID_OBSERVER_PTR			=	1,
	MICROCHUNKID_OBSERVER_ID,
};

bool	PersistentGameObjObserverClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
	PersistClass::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		void * observer_ptr = (GameObjObserverClass*)this;
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OBSERVER_PTR, observer_ptr );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OBSERVER_ID, ID );
	csave.End_Chunk();

	return true;
}

bool	PersistentGameObjObserverClass::Load( ChunkLoadClass &cload )
{
	void * old_observer_ptr = NULL;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				PersistClass::Load( cload );
				break;
								
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OBSERVER_PTR, old_observer_ptr );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OBSERVER_ID, ID );
						default:
							Debug_Say(( "Unrecognized PersistentGameObjObserverClass Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized PersistentGameObjObserverClass chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	WWASSERT( old_observer_ptr != NULL );
	if ( old_observer_ptr != NULL ) {
		SaveLoadSystemClass::Register_Pointer( old_observer_ptr, (GameObjObserverClass*)this );
	}

	return true;
}

/*
**
*/
DynamicVectorClass<PersistentGameObjObserverClass *>	PersistentGameObjObserverManager::ObserverList;

void	PersistentGameObjObserverManager::Add( PersistentGameObjObserverClass * observer )
{
	ObserverList.Add( observer );
}

void	PersistentGameObjObserverManager::Remove( PersistentGameObjObserverClass * observer )
{
	ObserverList.Delete( observer );
}

/*
**
*/
enum {
	CHUNKID_OBSERVERS			=	1,
};

bool	PersistentGameObjObserverManager::Save( ChunkSaveClass & csave )
{
	// Make sure we flush out all the old observers
	GameObjObserverManager::Delete_Pending();

	csave.Begin_Chunk( CHUNKID_OBSERVERS );
	// Allow each object in the master list to save
	for ( int i = 0; i < ObserverList.Count(); i++ ) {
		csave.Begin_Chunk( ObserverList[i]->Get_Factory().Chunk_ID() );
		ObserverList[i]->Get_Factory().Save( csave, ObserverList[i] );
		csave.End_Chunk();
	}
	csave.End_Chunk();
	return true;
}

bool	PersistentGameObjObserverManager::Load( ChunkLoadClass & cload )
{
	cload.Open_Chunk();
	WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_OBSERVERS );
	while (cload.Open_Chunk()) {
		PersistFactoryClass * factory = SaveLoadSystemClass::Find_Persist_Factory( cload.Cur_Chunk_ID() );
		if ( factory ) {
			factory->Load( cload );
		}
		cload.Close_Chunk();
	}
	cload.Close_Chunk();
	return true;
}


void	PersistentGameObjObserverManager::Reset( void )
{
	GameObjObserverManager::Delete_Pending();

	// Delete each in the list
	while ( ObserverList.Count() ) {
		delete ObserverList[0];
	}

	GameObjObserverManager::Delete_Pending();
}
