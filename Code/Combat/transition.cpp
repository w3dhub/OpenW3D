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
 *                     $Archive:: /Commando/Code/Combat/transition.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/09/02 3:53p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 75                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	Includes
*/
#include "transition.h"
#include "soldier.h"
#include "debug.h"
#include "assets.h"
#include "combat.h"
#include "ccamera.h"
#include "gameobjmanager.h"		// hack???
#include "scriptman.h"
#include "hanim.h"
#include "slist.h"
#include "vehicle.h"
#include "phys3.h"
#include "wwprofile.h"
#include "diaglog.h"
#include <string.h>
#include <stdlib.h>
#include "pathaction.h"


/*
**
*/
const char * TransitionTypeNames[] = {
		"LADDER_EXIT_TOP",
		"LADDER_EXIT_BOTTOM",
		"LADDER_ENTER_TOP",
		"LADDER_ENTER_BOTTOM",
		"LEGACY_VEHICLE_ENTER_0",
		"LEGACY_VEHICLE_ENTER_1",
		"LEGACY_VEHICLE_EXIT_0",
		"LEGACY_VEHICLE_EXIT_1",
		"VEHICLE_ENTER",
		"VEHICLE_EXIT",
};


/*
**
*/
bool	Is_Point_Inside( const Vector3 & pos, const Vector3 & min, const Vector3 & max )
{
	return ( ( pos.X >= min.X ) && ( pos.X <= max.X ) && 
				( pos.Y >= min.Y ) && ( pos.Y <= max.Y ) && 
				( pos.Z >= min.Z ) && ( pos.Z <= max.Z ) );
}

/*
** Save and Load for TransitionDataClass
*/
namespace TRANSITION_DATA_CLASS_SAVELOAD
{
	enum	{
		CHUNKID_VARIABLES						=  0x11051106,

		MICROCHUNKID_TYPE						=	1,
		MICROCHUNKID_ZONE,
		MICROCHUNKID_ANIMATION_NAME,
		MICROCHUNKID_ENDING_TM,
		MICROCHUNKID_LADDER_INDEX
	};
}

/*
**
*/
TransitionDataClass::TransitionDataClass( )
	:	Type( LADDER_EXIT_TOP )
{

}

/*
**
*/
const char * TransitionDataClass::Get_Type_Name( StyleType type )
{
	WWASSERT( ((int)type) < Get_Num_Types() );
	return TransitionTypeNames[((int)type)];
}

/*
**
*/
bool TransitionDataClass::Save( ChunkSaveClass & csave )
{
	using namespace TRANSITION_DATA_CLASS_SAVELOAD;
	csave.Begin_Chunk( CHUNKID_VARIABLES );

		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TYPE, Type );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ZONE, Zone );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENDING_TM, EndingTM );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_ANIMATION_NAME, AnimationName );
		
	csave.End_Chunk();
	return true;
}

/*
**
*/
bool TransitionDataClass::Load( ChunkLoadClass & cload )
{
	using namespace TRANSITION_DATA_CLASS_SAVELOAD;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
			{
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {

						READ_MICRO_CHUNK( cload, MICROCHUNKID_TYPE, Type );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ZONE, Zone );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENDING_TM, EndingTM );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_ANIMATION_NAME, AnimationName );

						default:
							Debug_Say(( "Unrecognized Transition Variable chunkID\n" ));
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

	switch ( Type ) { 
		case LEGACY_VEHICLE_ENTER_0:
		case LEGACY_VEHICLE_ENTER_1:
			Type = VEHICLE_ENTER;
			break;
		case LEGACY_VEHICLE_EXIT_0:
		case LEGACY_VEHICLE_EXIT_1:
			Type = VEHICLE_EXIT;
			break;
	}

	return true;
}

/*
** TransitionCompletionDataStruct
*/
namespace TRANSITION_COMPLETION_SAVE_LOAD
{

enum	{
	CHUNKID_VARIABLES								= 1105991816,
	CHUNKID_VEHICLE,

	MICROCHUNKID_TYPE								=	1,
};

}

bool	TransitionCompletionDataStruct::Save( ChunkSaveClass & csave )
{
	using namespace TRANSITION_COMPLETION_SAVE_LOAD;

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TYPE, Type );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VEHICLE );
		Vehicle.Save( csave );
	csave.End_Chunk();

	return true;
}

bool	TransitionCompletionDataStruct::Load( ChunkLoadClass & cload )
{
	using namespace TRANSITION_COMPLETION_SAVE_LOAD;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TYPE, Type );

						default:
							Debug_Say(( "Unrecognized TransitionCompletion Variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_VEHICLE:
				Vehicle.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized TransitionCompletion chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


/*
** TransitionInstanceClass
*/
TransitionInstanceClass::TransitionInstanceClass( const TransitionDataClass & data ) :
	Data( data ),
	EndingTM( 1 ),
	LadderIndex( -1 ),
	Zone( Vector3( 0,0,0 ), Vector3( 0,0,0 ) )
{
}

TransitionInstanceClass::~TransitionInstanceClass( void )
{
}

bool	TransitionInstanceClass::Check( SoldierGameObj *obj, bool action_trigger )
{
	Vector3	pos;
	obj->Get_Position( &pos );

	// make sure we are in the zone
	if ( CollisionMath::Overlap_Test( Zone, pos ) == CollisionMath::OUTSIDE ) {
		return false;		// not inside
	}

	// ignore triggers when transitioning
	if ( obj->Is_State_Locked() ) {
		return false;		
	}

	// Bail if not manually triggered and not ladder exit and
	if ( !action_trigger && 
		  ( Get_Type() != TransitionDataClass::LADDER_EXIT_TOP ) && 
		  ( Get_Type() != TransitionDataClass::LADDER_EXIT_BOTTOM ) ) {
#if 0
		// no auto transitions or not human controlled
		if ( !CombatManager::Are_Transitions_Automatic() ||
			  !obj->Is_Human_Controlled() ) 
#endif
		{
			return false;
		}
	}

	bool condition = action_trigger;

	if ( action_trigger == false ) {
		const float	MIN_VELOCITY = 0.1f;
		// Check for auto trigger
		Vector3 vel;
		obj->Get_Velocity( vel );
		if ( vel.Length() > MIN_VELOCITY ) {

			Vector3 end_direction;

			// make sure we meet the conditions
			switch ( Get_Type() ) {

				case TransitionDataClass::LADDER_ENTER_BOTTOM:
					end_direction = EndingTM.Get_X_Vector();
					end_direction.Normalize();
					break;

				case TransitionDataClass::LADDER_ENTER_TOP:
					end_direction = -EndingTM.Get_X_Vector();
					end_direction.Normalize();
					break;

				case TransitionDataClass::LADDER_EXIT_BOTTOM:
					end_direction = Vector3( 0, 0, -1 );
					break;

				case TransitionDataClass::LADDER_EXIT_TOP:
					end_direction = Vector3( 0, 0, 1 );
					break;

				case TransitionDataClass::VEHICLE_ENTER:
				case TransitionDataClass::VEHICLE_EXIT:
					vel.Z = 0; // don't consider vertical
					end_direction = EndingTM.Get_Translation() - Zone.Center; 
					end_direction.Normalize();
					break;

				default:
					Debug_Say(( "Unrecognized Transition Type\n" ));
					condition = false;
					break;
			}


			// If still has length
			if ( vel.Length() > MIN_VELOCITY ) {
				vel.Normalize();
				float	dotp = Vector3::Dot_Product( vel, end_direction );
				if ( dotp > 0.5f ) {
					condition = true;
				}
			}
		}
	}

	// make sure we meet the conditions
	switch ( Get_Type() ) {

		case TransitionDataClass::LADDER_EXIT_TOP:
		case TransitionDataClass::LADDER_EXIT_BOTTOM:
			if ( !obj->Is_On_Ladder() ) {
				condition = false;
			}
			break;

		case TransitionDataClass::LADDER_ENTER_TOP:
		case TransitionDataClass::LADDER_ENTER_BOTTOM:
			if ( !obj->Is_Upright() ) {
				condition = false;
			}
			break;

		case TransitionDataClass::VEHICLE_ENTER:
			if ( !obj->Is_Upright()  && !obj->Is_Wounded() ) {
				// Added wounded because we couldn't enter vehicles in a tib field
				condition = false;
			}

			VehicleGameObj * p_vehicle;
			p_vehicle = (VehicleGameObj *) Vehicle.Get_Ptr();
			WWASSERT(p_vehicle != NULL);

			if (!obj->Is_Permitted_To_Enter_Vehicle() ||
				 !p_vehicle->Is_Entry_Permitted(obj)) {
				condition = false;
			}

			// When entering a vehicle, ensure that there are no walls between us
			// Do this by casting a ray from our position to the vehicle position; only
			// checking against static geometry.
			{
				Vector3 p0,p1;
				p_vehicle->Get_Position(&p0);
				obj->Get_Position(&p1);

				CastResultStruct result;
				LineSegClass ray(p0,p1);
				int colgroup = obj->Peek_Physical_Object()->Get_Collision_Group();
				PhysRayCollisionTestClass raytest(ray,&result,colgroup,COLLISION_TYPE_PHYSICAL);
				raytest.CheckDynamicObjs = false;

				PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest);

				if (result.Fraction < 1.0f) {
					condition = false;
				}
			}

			break;

		case TransitionDataClass::VEHICLE_EXIT:
			if ( !obj->Is_In_Vehicle() ) {
				condition = false;
			}
			break;


		default:
			Debug_Say(( "Unrecognized Transition Type\n" ));
			condition = false;
			break;
	}

	if ( !condition ) {
		return false;
	}

	//
	//	Check to ensure the object can teleport to the ending location
	//
	Matrix3D dummy_tm;
	MoveablePhysClass *move_phys = obj->Peek_Physical_Object()->As_MoveablePhysClass();
	
	switch ( Get_Type() ) {

		case TransitionDataClass::LADDER_ENTER_TOP:
		case TransitionDataClass::LADDER_ENTER_BOTTOM:
		{			
			//
			//	Its not legal to get on a ladder when someone else is on the ladder.
			// Try to detect this case.
			//
			if( obj == COMBAT_STAR && LadderIndex >= 0 ) {
				ScriptableGameObj	*occupant = PathActionClass::Get_Ladder_Occupant( LadderIndex );
				if( occupant != NULL && occupant != obj ) {
					condition = false;
				}
			}
			break;
		}

		case TransitionDataClass::LADDER_EXIT_TOP:
		case TransitionDataClass::LADDER_EXIT_BOTTOM:
			
			//
			// When exiting a Ladder, we just have to check for a dynamic object blocking our
			// exit point.  (the third parameter to Can_Teleport_And_Stand is true...)
			//
			/*if (	move_phys != NULL &&	
				   move_phys->Can_Teleport(EndingTM, true) == false)
			{
				condition = false;
			}*/
		break;

		case TransitionDataClass::VEHICLE_EXIT:
			
			//
			// When exiting a vehicle, we perform two steps:
			// - ignore our vehicle and sweep the character to the exit transform
			// - un-ignore the vehicle and do a teleport test.
			//
			if (move_phys != NULL) {
				
				bool can_move_to;
				
				VehicleGameObj * vgobj = Get_Vehicle();
				PhysClass * pobj = vgobj->Peek_Physical_Object();
				
				pobj->Inc_Ignore_Counter();
				can_move_to = move_phys->Can_Move_To(EndingTM);
				pobj->Dec_Ignore_Counter();

				if (	(can_move_to == false) || 
						(move_phys->Can_Teleport_And_Stand(EndingTM,&dummy_tm) == false) ) 
				{
					condition = false;
				}
			}
	
		break;

	}

	if ( !condition ) {
		return false;
	}

	Start( obj );

	return true;
}

void	TransitionInstanceClass::Start( SoldierGameObj *obj )
{
//	Debug_Say(( "Starting Animation\n" ));

	// if this is the camera's target...
	if ( COMBAT_STAR == obj ) {
		// Get duration
#if 0
		HAnimClass * animation = WW3DAssetManager::Get_Instance()->Get_HAnim( Data.Get_Animation_Name() );
		if ( animation ) {
			float anim_duration = animation->Get_Num_Frames() / animation->Get_Frame_Rate();
			animation->Release_Ref();
//			COMBAT_CAMERA->Set_Lerp_Time( anim_duration * 1.3f );
			COMBAT_CAMERA->Set_Lerp_Time( anim_duration );
		}
#else
		COMBAT_CAMERA->Set_Lerp_Time( 1.0f );

#endif
	}

	// set the object ending position & orientation
#ifdef WWDEBUG
	Vector3 pos = EndingTM.Get_Translation();
	if (!pos.Is_Valid()) {
		WWDEBUG_SAY(("Transition EndingTM has invalid position: %f, %f, %f\r\n",pos.X,pos.Y,pos.Z));
	}
#endif

	obj->Set_Transform( EndingTM );

	if ( obj->Peek_Physical_Object() != NULL && obj->Peek_Physical_Object()->As_Phys3Class() != NULL ) {
		obj->Peek_Physical_Object()->As_Phys3Class()->Set_Velocity( Vector3(0,0,0) );
	}

	// make the completion data
	TransitionCompletionDataStruct * completion_data = new TransitionCompletionDataStruct;
	completion_data->Type = Data.Get_Type();
	completion_data->Vehicle = Vehicle;

	//
	//	Mark ladders as being used/free as necessary
	//
	switch ( Get_Type() ) {

		case TransitionDataClass::LADDER_ENTER_TOP:
		case TransitionDataClass::LADDER_ENTER_BOTTOM:
		{			
			//
			//	Mark this ladder as being "in use" so no one else tries to get on it
			//
			if( obj == COMBAT_STAR && LadderIndex >= 0 ) {
				PathActionClass::Set_Ladder_Occupant( LadderIndex, obj );
			}
			break;
		}

		case TransitionDataClass::LADDER_EXIT_TOP:
		case TransitionDataClass::LADDER_EXIT_BOTTOM:
		{
			//
			//	Free this ladder so other's can use it
			//
			if( obj == COMBAT_STAR && LadderIndex >= 0 ) {
				PathActionClass::Set_Ladder_Occupant( LadderIndex, NULL );
			}

			break;
		}
	}

#if 0
	obj->Start_Transition_Animation( Data.Get_Animation_Name(), completion_data );

	VehicleGameObj * vehicle = Get_Vehicle();

	// If vehicle entry/exit, allow it to animate as well
	switch ( Get_Type() ) {

		case TransitionDataClass::VEHICLE_ENTER:
		{
			if ( vehicle ) {
				vehicle->Passenger_Entering();
			}
			break;
		}

		case TransitionDataClass::VEHICLE_EXIT:
		{
			WWASSERT( vehicle );
//			obj->Set_Vehicle_State( SoldierGameObj::EXITING_VEHICLE, vehicle );
			vehicle->Passenger_Exiting();
			break;
		}

		default:
			break;
	}
#else
	End( obj, completion_data );
#endif

}

void	TransitionInstanceClass::End( SoldierGameObj *obj, 
				TransitionCompletionDataStruct * completion_data )
{
	VehicleGameObj * vehicle = (VehicleGameObj *)completion_data->Vehicle.Get_Ptr();

	switch ( completion_data->Type ) {

		case TransitionDataClass::LADDER_EXIT_TOP:
		case TransitionDataClass::LADDER_EXIT_BOTTOM:
			obj->Exit_Ladder();

			if ( obj == COMBAT_STAR ) {
				Vector3 pos;
				obj->Get_Position( &pos );
				DIAG_LOG(( "LAEX", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
			}
			break;

		case TransitionDataClass::LADDER_ENTER_TOP:
		case TransitionDataClass::LADDER_ENTER_BOTTOM:
			obj->Enter_Ladder( completion_data->Type == TransitionDataClass::LADDER_ENTER_TOP );

			if ( obj == COMBAT_STAR ) {
				Vector3 pos;
				obj->Get_Position( &pos );
				DIAG_LOG(( "LAEN", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
			}
			break;

		case TransitionDataClass::VEHICLE_ENTER:
		{
			if ( vehicle != NULL ) {
				vehicle->Add_Occupant( obj );
			}
			break;
		}

		case TransitionDataClass::VEHICLE_EXIT:
		{
			if ( vehicle != NULL ) {
				vehicle->Remove_Occupant( obj );
			} else {
				obj->Exit_Vehicle();
			}
			break;
		}

		default:
			Debug_Say(( "Unrecognized Transition Type\n" ));
			break;
	}

	// Delete the data (soldier better not use it)
	delete completion_data;
}

void	TransitionInstanceClass::Set_Parent_Transform( const Matrix3D & tm )
{
	Matrix3D::Multiply( tm, Data.Get_Ending_TM(), &EndingTM );
	OBBoxClass::Transform( tm, Data.Get_Zone(), &Zone );
}


/*
** TransitionManager
*/
SList<TransitionInstanceClass>		Transitions;
SList<TransitionInstanceClass>		TransitionsDestroy;

void	TransitionManager::Add( TransitionInstanceClass * transition )
{
	Transitions.Add_Tail( transition );
}

void	TransitionManager::Destroy( TransitionInstanceClass * transition )
{
	TransitionsDestroy.Add_Tail( transition );
}

void	TransitionManager::Reset( void )
{
	Destroy_Pending();

	while ( Transitions.Get_Count() ) {
		delete Transitions.Remove_Head();
	}
}

void	TransitionManager::Destroy_Pending( void )
{
	while ( TransitionsDestroy.Get_Count() ) {
		TransitionInstanceClass * trans = TransitionsDestroy.Remove_Head(); 
		Transitions.Remove( trans );
		delete trans;
	}
}

bool	TransitionManager::Check( SoldierGameObj *obj, bool action_trigger )
{
	WWPROFILE( "Transition Check" );
	SLNode<TransitionInstanceClass> *ti_node;
	for (	ti_node = Transitions.Head(); ti_node; ti_node = ti_node->Next()) {
		if ( ti_node->Data()->Check( obj, action_trigger ) ) {
			Destroy_Pending();
			return true;
		}
	}
	return false;
}


void	TransitionManager::Build_Ladder_List (DynamicVectorClass<TransitionInstanceClass *> &list)
{
	SLNode<TransitionInstanceClass> *ti_node = NULL;
	for (	ti_node = Transitions.Head(); ti_node; ti_node = ti_node->Next()) {
		
		//
		//	Is this a ladder transition?
		//
		TransitionDataClass::StyleType type = ti_node->Data()->Get_Type ();	
		if (	type == TransitionDataClass::LADDER_EXIT_TOP || type == TransitionDataClass::LADDER_EXIT_BOTTOM ||
				type == TransitionDataClass::LADDER_ENTER_TOP || type == TransitionDataClass::LADDER_ENTER_BOTTOM)
		{
			list.Add( ti_node->Data () );
		}
	}

	return ;
}
