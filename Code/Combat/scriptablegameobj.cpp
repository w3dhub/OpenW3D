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
 *                     $Archive:: /Commando/Code/Combat/scriptablegameobj.cpp                 $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/12/01 4:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 37                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scriptablegameobj.h"
#include "damage.h"
#include "scripts.h"
#include "debug.h"
#include "explosion.h"
#include "assets.h"
#include "combatsound.h"
#include "matrix3d.h"
#include "phys.h"
#include "smartgameobj.h"
#include "soldier.h"
#include "animcontrol.h"
#include "chunkio.h"
#include "saveload.h"
#include "combat.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "parameter.h"
#include "radar.h"
#include "playertype.h"
#include "matinfo.h"
#include "gameobjmanager.h"
#include "pscene.h"
#include "soundsceneobj.h"
#include "wwprofile.h"

/*
** ScriptableGameObjDef - Defintion class for a ScriptableGameObj
*/
ScriptableGameObjDef::ScriptableGameObjDef( void )
{
	SCRIPTLIST_PARAM (ScriptableGameObjDef, "Scripts", ScriptNameList, ScriptParameterList);
}

enum	{
	CHUNKID_DEF_PARENT							=	627001056,
	CHUNKID_DEF_VARIABLES,

	XXX_MICROCHUNKID_DEF_TYPE						=	1,
	MICROCHUNKID_DEF_SCRIPT_NAME,
	MICROCHUNKID_DEF_SCRIPT_PARAMETERS,
};

bool	ScriptableGameObjDef::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		BaseGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
		for ( int i = 0; i < ScriptNameList.Count(); i++ ) {
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_SCRIPT_NAME, ScriptNameList[i] );
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_SCRIPT_PARAMETERS, ScriptParameterList[i] );
		}
	csave.End_Chunk();

	return true;
}

bool	ScriptableGameObjDef::Load( ChunkLoadClass &cload )
{
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	StringClass str;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				BaseGameObjDef::Load( cload );
				break;

			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						case MICROCHUNKID_DEF_SCRIPT_NAME:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptNameList.Add( str );
							break;

						case MICROCHUNKID_DEF_SCRIPT_PARAMETERS:
							LOAD_MICRO_CHUNK_WWSTRING( cload, str );
							ScriptParameterList.Add( str );
							break;

						default:
							Debug_Say(( "Unhandled Variable Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	WWASSERT( ScriptNameList.Count() == ScriptParameterList.Count() );
	return true;
}


/*
** Game Object Observer Timer (used in Scripts)
*/
class	GameObjObserverTimerClass {
public:
	GameObjObserverTimerClass( int observer_id = 0, float time = 0, int timer_id = 0 )
		{	ObserverID = observer_id;  RemainingTime = time; TimerID = timer_id; }

	bool	Save( ChunkSaveClass & csave );
	bool	Load( ChunkLoadClass & cload );

	bool	Update( void )		{ RemainingTime -= TimeManager::Get_Frame_Seconds(); return RemainingTime <= 0; }
	bool	Expired( void )	{ return RemainingTime <= 0; }

	int					ObserverID;
	float					RemainingTime;
	int					TimerID;
};

enum	{
	CHUNKID_TIMER_VARIABLES				=	922991755,
	CHUNKID_TIMER_SENDER,

	MICROCHUNKID_REMAINING_TIME		=	1,
	MICROCHUNKID_TIMER_ID,
	MICROCHUNKID_OBSERVER_ID,
	MICROCHUNKID_TYPE,
	MICROCHUNKID_PARAM,
};

bool	GameObjObserverTimerClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_TIMER_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_REMAINING_TIME, RemainingTime );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TIMER_ID, TimerID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OBSERVER_ID, ObserverID );
	csave.End_Chunk();

	return true;
}

bool	GameObjObserverTimerClass::Load( ChunkLoadClass & cload )
{
	cload.Open_Chunk();
	WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_TIMER_VARIABLES );

	while (cload.Open_Micro_Chunk()) {
		switch(cload.Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK( cload, MICROCHUNKID_REMAINING_TIME, RemainingTime );
			READ_MICRO_CHUNK( cload, MICROCHUNKID_TIMER_ID, TimerID );
			READ_MICRO_CHUNK( cload, MICROCHUNKID_OBSERVER_ID, ObserverID );

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Micro_Chunk();
	}
	cload.Close_Chunk();
	return true;
}


/*
** Game Object Custom Timer (used in Scripts)
*/
class	GameObjCustomTimerClass {
public:

	GameObjCustomTimerClass( ScriptableGameObj *sender = NULL, float time = 0, int type = 0, int param = 0 ) :
		RemainingTime( time ), Type( type ), Param( param)		{ if ( sender != NULL ) Sender = sender; }

	bool	Save( ChunkSaveClass & csave );
	bool	Load( ChunkLoadClass & cload );

	bool	Update( void )		{ RemainingTime -= TimeManager::Get_Frame_Seconds(); return RemainingTime <= 0; }
	bool	Expired( void )	{ return RemainingTime <= 0; }

	float					RemainingTime;
	GameObjReference	Sender;
	int					Type;
	int					Param;
};

bool	GameObjCustomTimerClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_TIMER_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_REMAINING_TIME, RemainingTime );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TYPE, Type );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PARAM, Param );
	csave.End_Chunk();

	if ( Sender != NULL ) {
		csave.Begin_Chunk( CHUNKID_TIMER_SENDER );
		Sender.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	GameObjCustomTimerClass::Load( ChunkLoadClass & cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_TIMER_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_REMAINING_TIME, RemainingTime );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TYPE, Type );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PARAM, Param );

						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_TIMER_SENDER:
				Sender.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


/*
** ScriptableGameObj
*/
ScriptableGameObj::ScriptableGameObj( void ) :
	ReferenceableGameObj( this ),
	ObserverCreatedPending( false )
{
}

ScriptableGameObj::~ScriptableGameObj( void )
{
	Remove_All_Observers();

	/*
	** Delete the ObserverTimerList. ST - 6/11/2001 9:20PM
	*/
	while (ObserverTimerList.Count()) {
		delete ObserverTimerList[0];
		ObserverTimerList.Delete(0);
	}

	/*
	** Delete the CustomTimerList. ST - 6/11/2001 9:20PM
	*/
	while (CustomTimerList.Count()) {
		delete CustomTimerList[0];
		CustomTimerList.Delete(0);
	}
}


/*
**
*/
void	ScriptableGameObj::Init( const ScriptableGameObjDef & definition )
{
	BaseGameObj::Init( definition );
	Copy_Settings( definition );
	return ;
}

/*
**
*/
void	ScriptableGameObj::Copy_Settings( const ScriptableGameObjDef & definition )
{
	//
	//	Only assign scripts on the server
	//
	if (CombatManager::I_Am_Server()) {

		//
		//	Attach the scripts
		//
		WWASSERT( definition.ScriptNameList.Count() == definition.ScriptParameterList.Count() );
		for ( int i = 0; i < definition.ScriptNameList.Count(); i++ ) {
			ScriptClass* script = ScriptManager::Create_Script( definition.ScriptNameList[i] );
			if (script) {
				script->Set_Parameters_String( definition.ScriptParameterList[i] );
				// Don't call Add observer, because we don't want Created called yet.
				// Start_Observers should be called later, which will call Created
				Insert_Observer(script);
			}
		}
	}

	return ;
}


/*
**
*/
void	ScriptableGameObj::Re_Init( const ScriptableGameObjDef & definition )
{
	//
	//	Remove all currently running scripts
	//
	Remove_All_Observers();

	//
	//	Copy any internal settings from the definition
	//
	Copy_Settings( definition );

	//
	//	Reset our definition pointer
	//
	BaseGameObj::Init( definition );
	return ;
}


/*
**
*/
void	ScriptableGameObj::Post_Re_Init( void )
{
	//
	//	Start the new scripts executing
	//
	Start_Observers();
	return ;
}


const ScriptableGameObjDef & ScriptableGameObj::Get_Definition( void ) const
{
	return (const ScriptableGameObjDef &)BaseGameObj::Get_Definition();
}

void	ScriptableGameObj::Set_Delete_Pending( void )
{
	if ( !Is_Delete_Pending() ) {
		if ( CombatManager::Are_Observers_Active() ) {
			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Destroyed( this );
			}
		}

		if ( this == COMBAT_STAR ) {
			CombatManager::Star_Killed();
		}

		BaseGameObj::Set_Delete_Pending ();
	}
}


/*
** ScriptableGameObj Save and Load
*/
enum	{
	CHUNKID_PARENT							=	627001122,
	CHUNKID_VARIABLES,
	CHUNKID_REFERENCEABLE,
	CHUNKID_CUSTOM_TIMER,
	CHUNKID_OBSERVER_TIMER,

	MICROCHUNKID_REFERENCEABLE_PTR	= 1,
	MICROCHUNKID_GAME_OBJ_OBSERVER_PTR,
	MICROCHUNKID_OBSERVER_CREATED_PENDING,
};

bool	ScriptableGameObj::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_PARENT );
		BaseGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_REFERENCEABLE );
		ReferenceableGameObj::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		ReferenceableGameObj * referenceable_ptr = (ReferenceableGameObj *)this;
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_REFERENCEABLE_PTR, referenceable_ptr );

		const GameObjObserverList & observer_list = Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			void	* game_obj_observer_ptr = observer_list[ index ];
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_GAME_OBJ_OBSERVER_PTR, game_obj_observer_ptr );
		}

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OBSERVER_CREATED_PENDING, ObserverCreatedPending );
	csave.End_Chunk();

	int i;
	for ( i = 0; i < ObserverTimerList.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_OBSERVER_TIMER );
			ObserverTimerList[i]->Save( csave );
		csave.End_Chunk();
	}

	for ( i = 0; i < CustomTimerList.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_CUSTOM_TIMER );
			CustomTimerList[i]->Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	ScriptableGameObj::Load( ChunkLoadClass &cload )
{
	ReferenceableGameObj * referenceable_ptr = NULL;

	WWASSERT( Observers.Count() == 0 );

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PARENT:
				BaseGameObj::Load( cload );
				break;

			case CHUNKID_REFERENCEABLE:
				ReferenceableGameObj::Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_REFERENCEABLE_PTR, referenceable_ptr );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OBSERVER_CREATED_PENDING, ObserverCreatedPending );

						case MICROCHUNKID_GAME_OBJ_OBSERVER_PTR:
							GameObjObserverClass * ptr;
							cload.Read(&ptr,sizeof(ptr));
							Observers.Add( ptr );
							break;

						default:
							Debug_Say(( "Unhandled Variable Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_OBSERVER_TIMER:
				GameObjObserverTimerClass * otimer;
				otimer = new GameObjObserverTimerClass();
				otimer->Load( cload );
				ObserverTimerList.Add( otimer );
				break;

			case CHUNKID_CUSTOM_TIMER:
				GameObjCustomTimerClass * ctimer;
				ctimer = new GameObjCustomTimerClass();
				ctimer->Load( cload );
				CustomTimerList.Add( ctimer );
				break;

			default:
				Debug_Say(( "Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	// Request a remap on all the observers
	for ( int ob_idx = 0; ob_idx < Observers.Count(); ob_idx++ ) {
		REQUEST_POINTER_REMAP( (void **)&(Observers[ ob_idx ]) );
	}

	WWASSERT(referenceable_ptr != NULL);
	if (referenceable_ptr != NULL) {
		SaveLoadSystemClass::Register_Pointer(referenceable_ptr , (ReferenceableGameObj *)this);
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);

	return true;
}

void ScriptableGameObj::On_Post_Load( void )
{
	BaseGameObj::On_Post_Load();

	// Delete any NULL pointers
	GameObjObserverList & observer_list = (GameObjObserverList &)Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		if ( observer_list[ index ] == NULL ) {
			observer_list.Delete( index );
			index--;
		}
	}

	if ( CombatManager::Is_First_Load() ) {
		ObserverCreatedPending = true;
	}
}

/*
**
*/
void ScriptableGameObj::Start_Observers( void )
{
	// If we just came from the editor, call created on all out observers
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		observer_list[ index ]->Created( this );
	}
}

void ScriptableGameObj::Add_Observer( GameObjObserverClass * observer )
{
	WWASSERT(observer != NULL);

	Insert_Observer( observer );

	// Don't call created if in the editor
	if ( CombatManager::Are_Observers_Active() ) {
		observer->Created( this );
	}
}

void ScriptableGameObj::Insert_Observer( GameObjObserverClass * observer )
{
	WWASSERT(observer != NULL);

	observer->Attach( this );
	Observers.Add( observer );
}

void ScriptableGameObj::Remove_Observer( GameObjObserverClass * observer )
{
	Observers.Delete( observer );
	observer->Detach( this );
	// if observer is a script, if will be deleted soon after this
}

void ScriptableGameObj::Remove_All_Observers(void)
{
	while(Observers.Count() != 0) {
		Remove_Observer(Observers[0]);
	}
}

void	ScriptableGameObj::Start_Observer_Timer( int observer_id, float duration, int timer_id )
{
	ObserverTimerList.Add( new GameObjObserverTimerClass( observer_id, duration, timer_id ) );
}

void	ScriptableGameObj::Start_Custom_Timer( ScriptableGameObj * from, float delay, int type, int param )
{
	CustomTimerList.Add( new GameObjCustomTimerClass( from, delay, type, param ) );
}

void	ScriptableGameObj::Think( void )
{
	if (Is_Always_Dirty()) {
// FIXME Forcing game objects to be network dirty for updates.\n
		Set_Object_Dirty_Bit (NetworkObjectClass::BIT_FREQUENT, true);
	}

	if ( ObserverCreatedPending ) {
		Start_Observers();
		ObserverCreatedPending = false;
	}

	BaseGameObj::Think();
}

void	ScriptableGameObj::Post_Think( void )
{
	BaseGameObj::Post_Think();

	WWPROFILE( "Scriptable PostThink" );

	// Note: The cinematic script comes later in the obj list then the things it creates.
	// This means that objects the script creates when it thinks (via timers) dont think
	// (bump animation forward) until the next frame.  Be wary of changing this order.

	// Check Timers
	int i;
	for ( i = ObserverTimerList.Count() - 1; i >= 0; i-- ) {
		if ( ObserverTimerList[i]->Update() ) {

//			Debug_Say(( "Timer Expired for %d\n", ObserverTimerList[i]->ObserverID ));

			bool found = false;

			WWASSERT( ObserverTimerList[i]->ObserverID != 0 );
			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				if ( observer_list[ index ]->Get_ID() == ObserverTimerList[i]->ObserverID ) {
					observer_list[ index ]->Timer_Expired( this, ObserverTimerList[i]->TimerID );
					found = true;
				}
			}

			if ( !found ) {
				Debug_Say(( "Failed to find observer id %d for timer expired....\n", ObserverTimerList[i]->ObserverID ));

				const GameObjObserverList & observer_list = Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					Debug_Say(( "have %d\n", observer_list[ index ]->Get_ID() ));
				}
			}

			delete ObserverTimerList[i];
			ObserverTimerList.Delete( i );
		}
	}

	for ( i = CustomTimerList.Count() - 1; i >= 0; i-- ) {
		if ( CustomTimerList[i]->Update() ) {

			ScriptableGameObj *sender = CustomTimerList[i]->Sender;

			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Custom( this, CustomTimerList[i]->Type, CustomTimerList[i]->Param, sender );
			}

			delete CustomTimerList[i];
			CustomTimerList.Delete( i );
		}
	}
}

//------------------------------------------------------------------------------------
void	ScriptableGameObj::Get_Information( StringClass & string )
{
	// If we just came from the editor, call created on all out observers
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {
		StringClass temp;
		temp.Format( "%d:%s\n", observer_list[ index ]->Get_ID(), observer_list[ index ]->Get_Name() );
		string += temp;
	}
}

//------------------------------------------------------------------------------------
void	ScriptableGameObj::On_Sound_Ended( SoundSceneObjClass *sound_obj )
{
	if ( sound_obj == NULL ) {
		return ;
	}

	int sound_id = sound_obj->Get_ID();

	//
	// Notify all observers
	//
	const GameObjObserverList & observer_list = Get_Observers();
	for( int index = 0; index < observer_list.Count(); index++ ) {

		//
		//	Send a custom event to this observer notifying it that this sound has ended
		//
		observer_list[ index ]->Custom( this, CUSTOM_EVENT_SOUND_ENDED, sound_id, NULL );
	}

	return ;
}


/*
**
*/
void	ScriptableGameObj::Export_Creation( BitStreamClass &packet )
{
	BaseGameObj::Export_Creation( packet );
	return ;
}


/*
**
*/
void	ScriptableGameObj::Import_Creation( BitStreamClass &packet )
{
	BaseGameObj::Import_Creation( packet );

	//
	//	Ensure we don't have any scripts running
	//
	Remove_All_Observers();
	return ;
}