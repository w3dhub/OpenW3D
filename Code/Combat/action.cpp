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
 *                     $Archive:: /Commando/Code/Combat/action.cpp                            $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/19/02 11:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 266                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "action.h"
#include "input.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "Path.h"
#include "pathsolve.h"
#include "pathmgr.h"
#include "vehicledriver.h"
#include "pilot.h"
#include "soldier.h"
#include "vehicle.h"
#include "weapons.h"
#include "movephys.h"
#include "Pathfind.h"
#include "PathfindPortal.h"
#include "debug.h"
#include "animcontrol.h"
#include "conversationmgr.h"
#include "activeconversation.h"
#include "orator.h"
#include "soldierobserver.h"
#include "physcon.h"
#include "phys3.h"
#include "humanphys.h"
#include "pscene.h"
#include "staticphys.h"
#include "elevator.h"
#include "doors.h"
#include "pathaction.h"
#include "crandom.h"
#include "playertype.h"
#include "wwprofile.h"

#include "combat.h"
#include "waypath.h"
#include "waypoint.h"
#include "gameobjmanager.h"

#include "colmathaabox.h"
#include "dinput.h"
#include <algorithm>

int _ActionActCalls = 0;
int _ActionCodeChanges = 0;

/*
**
*/
static	bool	Display_Findpaths = false;
void	Toggle_Display_Findpaths( void ) { Display_Findpaths = !Display_Findpaths; }

/*
** ActionCodeClass
*/
class	ActionCodeClass : public PersistClass {

public:
	typedef enum {
		ACTION_DONE,
		ACTION_IN_PROGRESS,
	} ActResult;

	ActionCodeClass( void ) : Action( NULL )	  		{}
	virtual ~ActionCodeClass( void )				  		{}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001350,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			PersistClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					PersistClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	void	Init( ActionClass * action )
	{
		WWASSERT( action );
		WWASSERT( Action == NULL );
		Action = action;

		//
		//	Check to see if we need to change the soldier's AI state
		//
		if ( Action->Get_Parameters().AIState != NO_AI_STATE_CHANGE ) {
			SmartGameObj *	obj = Action->Get_Action_Obj();
			if ( obj != NULL ) {

				//
				//	Check to ensure this is really a soldier
				//
				SoldierGameObj * soldier = obj->As_SoldierGameObj();
				if ( soldier != NULL ) {
					soldier->Set_AI_State( Action->Get_Parameters().AIState );
				}
			}
		}
	}

	virtual	void	Modify_Parameters( const SafeActionParamsStruct & /* parameters */, bool /* modify_move */, bool /* modify_attack */ ) {}

	virtual	void	Set_Action( ActionClass * action )	{ WWASSERT( action ); WWASSERT( Action == NULL ); Action = action; }
	virtual	void	Shutdown( void )						{ Action = NULL; }
	virtual	ActResult	Act( void )			  			{ return ACTION_IN_PROGRESS; }
	virtual	bool	Is_Animating( void )					{ return false; }
	virtual	bool	Is_Busy( void )						{ return false; }

	virtual	void	Begin_Hibernation( void )	{}
	virtual	void	End_Hibernation( void )		{}

protected:
	ActionClass					*	Action;

};

/*
**
*/
SimpleDynVecClass<ActionCodeClass*>	ActionDeleteList;

void	Register_Action_Code_Deletion( ActionCodeClass * action )
{
	ActionDeleteList.Add( action );
}

void	Delete_Action_Code( void )
{
	for ( int i = 0; i < ActionDeleteList.Count(); i++ ) {
		delete ActionDeleteList[i];
	}
	// Clear vector but don't allow shrink.
	ActionDeleteList.Delete_All(false);
}


/*
** Follow Input Action Code Class
*/
class	FollowInputActionCodeClass;
SimplePersistFactoryClass<FollowInputActionCodeClass, CHUNKID_ACTION_CODE_FOLLOW_INPUT>	_FollowInputActionCodeClassFactory;

class	FollowInputActionCodeClass : public ActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _FollowInputActionCodeClassFactory; }

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001349,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			ActionCodeClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					ActionCodeClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	ActResult	Act( void ) override
	{
		float amount;
		SmartGameObj *	obj = Action->Get_Action_Obj();

		// If we're in a vehicle, use the input function VEHICLE_TURN_LEFT...
		if ((obj->As_SoldierGameObj() != NULL) && (obj->As_SoldierGameObj()->Get_Vehicle() != NULL)) {
			amount = Input::Get_Amount( INPUT_FUNCTION_VEHICLE_TURN_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_VEHICLE_TURN_RIGHT );
		} else {
			amount = Input::Get_Amount( INPUT_FUNCTION_TURN_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_TURN_RIGHT );
		}
		obj->Set_Analog_Control( ControlClass::ANALOG_TURN_LEFT, amount );

		float forward_amount = Input::Get_Amount( INPUT_FUNCTION_MOVE_FORWARD ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_BACKWARD );
		float left_amount = Input::Get_Amount( INPUT_FUNCTION_MOVE_LEFT ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_RIGHT );
		float up_amount = Input::Get_Amount( INPUT_FUNCTION_MOVE_UP ) - Input::Get_Amount( INPUT_FUNCTION_MOVE_DOWN );

		if ( obj->As_SoldierGameObj() && obj->As_SoldierGameObj()->Get_Vehicle() ) {
			VehicleGameObj * vehicle = obj->As_SoldierGameObj()->Get_Vehicle();
			// Convert turn to straffe
			if ( vehicle->Is_Aircraft() && (Input::Is_Button_Down (DIK_LCONTROL) || Input::Is_Button_Down (DIK_LMENU) ) ) {
				left_amount = amount;
				obj->Set_Analog_Control( ControlClass::ANALOG_TURN_LEFT, 0 );
			}
		}

		Vector3 move( forward_amount, left_amount, up_amount );
		float length = move.Length();
		if ( length > 1 ) {
			forward_amount /= length;
			left_amount /= length;
		}

#if 0
		if ( Input::Get_State( INPUT_FUNCTION_WALK_MODE ) ) {
			forward_amount /= 4;
			left_amount /= 4;
		}
#else
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WALK,					Input::Get_State( INPUT_FUNCTION_WALK_MODE ) );
#endif
		obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_FORWARD, forward_amount );
		obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_LEFT, left_amount );
		obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_UP, up_amount );

		obj->Set_Boolean_Control( ControlClass::BOOLEAN_JUMP,					Input::Get_State( INPUT_FUNCTION_JUMP ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_VEHICLE_TOGGLE_GUNNER, Input::Get_State( INPUT_FUNCTION_VEHICLE_TOGGLE_GUNNER ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_NEXT,		Input::Get_State( INPUT_FUNCTION_NEXT_WEAPON ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_PREV,		Input::Get_State( INPUT_FUNCTION_PREV_WEAPON ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY,	Input::Get_State( INPUT_FUNCTION_FIRE_WEAPON_PRIMARY ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY,	Input::Get_State( INPUT_FUNCTION_FIRE_WEAPON_SECONDARY ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_USE,			Input::Get_State( INPUT_FUNCTION_USE_WEAPON ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_RELOAD,		Input::Get_State( INPUT_FUNCTION_RELOAD_WEAPON ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_DIVE_FORWARD,		Input::Get_State( INPUT_FUNCTION_DIVE_FORWARD ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_DIVE_BACKWARD,		Input::Get_State( INPUT_FUNCTION_DIVE_BACKWARD ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_DIVE_LEFT,			Input::Get_State( INPUT_FUNCTION_DIVE_LEFT ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_DIVE_RIGHT,			Input::Get_State( INPUT_FUNCTION_DIVE_RIGHT ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH,				Input::Get_State( INPUT_FUNCTION_CROUCH ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_ACTION,				Input::Get_State( INPUT_FUNCTION_ACTION ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_DROP_FLAG,			Input::Get_State( INPUT_FUNCTION_DROP_FLAG ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_NO_WEAPON,	Input::Get_State( INPUT_FUNCTION_SELECT_NO_WEAPON ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_0,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_0 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_1,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_1 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_2,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_2 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_3,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_3 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_4,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_4 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_5,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_5 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_6,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_6 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_7,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_7 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_8,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_8 ) );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_SELECT_WEAPON_9,	Input::Get_State( INPUT_FUNCTION_SELECT_WEAPON_9 ) );
		return ACTION_IN_PROGRESS;
	}
};


/*
** Stand Action Code Class
*/
class	StandActionCodeClass;
SimplePersistFactoryClass<StandActionCodeClass, CHUNKID_ACTION_CODE_STAND>	_StandActionCodeClassFactory;

class	StandActionCodeClass : public ActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _StandActionCodeClassFactory; }

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001348,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			ActionCodeClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					ActionCodeClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	ActResult	Act( void ) override
	{
		return ACTION_IN_PROGRESS;
	}
};


/*
** Play Animation Action Code Class
*/
class	PlayAnimationActionCodeClass;
SimplePersistFactoryClass<PlayAnimationActionCodeClass, CHUNKID_ACTION_CODE_PLAY_ANIMATION>	_PlayAnimationActionCodeClassFactory;

class	PlayAnimationActionCodeClass : public ActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _PlayAnimationActionCodeClassFactory; }

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001347,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			ActionCodeClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					ActionCodeClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	bool	Is_Animating( void ) override					{ return true; }

	virtual	void	Init( ActionClass * action ) override
	{
		ActionCodeClass::Init( action );

		SmartGameObj *	obj = Action->Get_Action_Obj();

		WWASSERT( obj->Get_Anim_Control() != NULL );
		obj->Set_Animation( action->Get_Parameters().SafeAnimationName, action->Get_Parameters().AnimationLooping );
	}

	virtual	void	Shutdown( void ) override
	{
		SmartGameObj *	obj = Action->Get_Action_Obj();
		WWASSERT( obj->Get_Anim_Control() != NULL );
//		WWASSERT( obj->Get_Anim_Control()->Get_Animation_Name()[0] != 0 );
		obj->Set_Animation( NULL, false );
		ActionCodeClass::Shutdown();
	}

	virtual	ActResult	Act( void ) override
	{
		SmartGameObj *	obj = Action->Get_Action_Obj();
		WWASSERT( obj->Get_Anim_Control() != NULL );

		if ( obj->Get_Anim_Control()->Get_Animation_Name()[0] == 0 ) {
			Debug_Say(( "Not playing an anim when we should be playing %s\n", Action->Get_Parameters().SafeAnimationName ));
			Action->Done( ACTION_COMPLETE_NORMAL );
			return ACTION_DONE;
		}

//		WWASSERT( obj->Get_Anim_Control()->Get_Animation_Name()[0] != 0 );

		if ( obj->Get_Anim_Control()->Is_Complete() ) {
			Action->Done( ACTION_COMPLETE_NORMAL );
			return ACTION_DONE;
		}

		return ACTION_IN_PROGRESS;
	}
};


/*
** Base Action Code Class
**
** This action will move and/or shoot context Object to the context Locations
** From the outside, you can set the moveto location, and you can set the
** shootAt location.  The last one you set sets the moving flag, which
** is ignored if object can move and shoot independently.
** Need to know:
		Can object move and fire independantly( if turreted || moves laterally )
		Is Object Turreted ( gun can face a different direction than object, just tanks )
		Can object move laterally (tanks and cars cannot, humans can)
		Can object turn without moving ( cars cannot )
*/
class	BaseActionCodeClass : public ActionCodeClass {
public:

	BaseActionCodeClass( void )
	{
		IsMoving = false;
		IsAttacking = false;
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001345,
		CHUNKID_VARIABLES,
		CHUNKID_VEHICLE_DRIVER,
		CHUNKID_PILOT,

		MICROCHUNKID_IS_ATTACKING					=	1,
		MICROCHUNKID_IS_MOVING						=	2,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			ActionCodeClass::Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_ATTACKING,				IsAttacking				);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_MOVING,					IsMoving					);
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VEHICLE_DRIVER );
			VehicleDriver.Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_PILOT );
			Pilot.Save( csave );
		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					ActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_ATTACKING,				IsAttacking				);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_MOVING,					IsMoving					);

							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				case CHUNKID_VEHICLE_DRIVER:
					VehicleDriver.Load( cload );
					break;

				case CHUNKID_PILOT:
					Pilot.Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}
		return true;
	}


		// Return a pointer to the object that will be doing the movement
	SmartGameObj *Peek_Transport_Object( SmartGameObj *game_obj )
	{
		SmartGameObj *transport = game_obj;

		if (	game_obj->As_SoldierGameObj() != NULL  &&
				game_obj->As_SoldierGameObj()->Get_Profile_Vehicle() != NULL ) {
			transport = game_obj->As_SoldierGameObj()->Get_Profile_Vehicle();
		}

		return transport;
	}

	void Clamped_Units( Vector3& value, float rate )
	{
		// convert to "units", and clamp
		rate *= TimeManager::Get_Frame_Seconds();
		if ( rate > 0.0001f ) {
			value /= rate;
		}
		if ( (rate <= 0.0001f) || value.Length() > 1.0 ) {
			value.Normalize();
		}
	}

	// Move Human object to given relative position
	void	Human_Move_To_Relative( Vector3 & orig_rel_pos )
	{
//Debug_Say(( "Mov To Rel %f %f %f\n", rel_pos.X, rel_pos.Y, rel_pos.Z ));
		SmartGameObj *	obj = Action->Get_Action_Obj();
		float speed = Action->Get_Parameters().MoveSpeed;

		Vector3 move = orig_rel_pos;

		//
		// Zero out the relative distance in the z axis (if necessary)
		//
		SoldierGameObj * soldier = obj->As_SoldierGameObj();
		if ( soldier == NULL || soldier->Is_On_Ladder() == false) {
			move.Z = 0;
		}

		Clamped_Units( move, obj->Get_Max_Speed() * speed );

		if ( move.Length2() > 1 ) {
			move.Normalize();
		}

   	obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_FORWARD, move[0] * speed );
		obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_LEFT, move[1] * speed );

		// Special case for soldier moving
		if ( soldier && soldier->Is_On_Ladder() ) {
			obj->Set_Analog_Control( ControlClass::ANALOG_MOVE_FORWARD, (move[2] >= 0) ? 1 : -1 );		// Could we make this move up?
		}
	}

	// Move object to given relative position, return if complete
	bool	Move_To_Absolute( const Vector3 & abs_pos, bool ignore_arrived_dist = false )
	{
		SmartGameObj *	obj = Action->Get_Action_Obj();

		//	Should the vehicle AI code take over?
		SmartGameObj *transport_obj = Peek_Transport_Object( obj );
		VehicleGameObj *vehicle = transport_obj->As_VehicleGameObj ();
		if ( vehicle != NULL ) {

			bool retval = false;
			if (vehicle->Is_Aircraft ()) {
				Pilot.Set_Speed_Factor( Action->Get_Parameters().MoveSpeed );

				if ( Action->Get_Parameters().AttackObject != NULL ) {
					Vector3 target_pos;
					Action->Get_Parameters().AttackObject->Get_Position( &target_pos );
					Pilot.Set_Target( &target_pos );
				}

				Pilot.Set_Speed_Factor( Action->Get_Parameters().MoveSpeed );
				Pilot.Set_Arrived_Dist( Action->Get_Parameters().MoveArrivedDistance );

				retval = Pilot.Think();
			} else {
				VehicleDriver.Set_Next_Position( abs_pos );
				VehicleDriver.Set_Speed_Factor( Action->Get_Parameters().MoveSpeed );
				VehicleDriver.Set_Arrived_Dist( Action->Get_Parameters().MoveArrivedDistance );
				VehicleDriver.Force_Backup( Action->Get_Parameters().MoveBackup );
				retval = VehicleDriver.Drive();
			}

			IsMoving = true;
			return retval;
		}

		// If we want to be facing a particular position, set targeting to it.
		if ( Action->Get_Parameters().ForceFacing ) {
			obj->Set_Targeting( Action->Get_Parameters().FaceLocation, false ); // Don't tilt
		}

		Vector3	curr_pos;
		obj->Get_Position( &curr_pos );
		curr_pos -= abs_pos;

		// I'm having soldiers get stuck because they are directly below
		// the goal point.  Lets try ignoring Z
		curr_pos.Z = 0;

		float range = curr_pos.Length();
		SoldierGameObj *soldier	= obj->As_SoldierGameObj();

		bool done_facing = true;	// Assume we are done facing

		//
		// If we are not attacking, and not force facing, and are a soldier and
		// are not on a ladder and not very close
		//
		if (	!IsAttacking && !Action->Get_Parameters().ForceFacing &&
				!Action->Get_Parameters().IgnoreFacing && soldier &&
				!soldier->Is_On_Ladder() && (range > 0.1f ) )
		{
			// Then try to face the point we are moving to
			Vector3 face_pt = abs_pos + Vector3( 0,0, obj->Get_Bullseye_Offset_Z() );
			done_facing = obj->Set_Targeting( face_pt, false );
		}

		//
		//	Determine how close we want to get to the end-point before
		// we stop moving
		//
		float arrived_dist = Action->Get_Parameters().MoveArrivedDistance;
		if ( ignore_arrived_dist ) {
			arrived_dist = 0.05F;
		}

		//
		// If we are close enough
		//
		if ( (Get_Distance_From_Goal() + range) <=  arrived_dist) {
			IsMoving = false;
			return done_facing;	// complete;
		}

		IsMoving = true;

		// Find the relitive location
		Vector3 rel_pos;
		Matrix3D::Inverse_Transform_Vector( obj->Get_Transform(), abs_pos, &rel_pos );

		// we done facing, or facing close enough, move
		float goal_bearing = WWMath::Atan2( rel_pos.Y, rel_pos.X );
		if ( done_facing || WWMath::Fabs(goal_bearing) < DEG_TO_RAD( 40 ) ) {
			Human_Move_To_Relative( rel_pos );
		}

		return false;
	}

	virtual float	Get_Distance_From_Goal (void)
	{
		SmartGameObj *	obj = Action->Get_Action_Obj();

		//
		//	Get the object's current position
		//
		Vector3	curr_pos;
		obj->Get_Position( &curr_pos );

		//
		//	Find the difference between the current position and
		// the destination
		//
		curr_pos	-= Action->Get_Parameters().MoveLocation;
		curr_pos.Z = 0;

		//
		//	Return the length
		//
		return curr_pos.Length ();
	}

protected:
	bool						IsAttacking;
	bool						IsMoving;
	VehicleDriverClass	VehicleDriver;
	PilotClass				Pilot;
};


/*
** Goto Action Code Class
*/
class	GotoActionCodeClass;
SimplePersistFactoryClass<GotoActionCodeClass, CHUNKID_ACTION_CODE_GOTO>	_GotoActionCodeClassFactory;

class	GotoActionCodeClass : public BaseActionCodeClass {
public:

	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _GotoActionCodeClassFactory; }

	GotoActionCodeClass( void ) :
		UseMoveObject( false ),
		FirstCall( true ),
		PathSolver( NULL ),
		Path( NULL )
	{
	}

	~GotoActionCodeClass( void )
	{
		return ;
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001308,
		CHUNKID_VARIABLES,
		CHUNKID_PATH,
		CHUNKID_PATH_SOLVER,
		CHUNKID_PATH_ACTION,

		MICROCHUNKID_FIRST_CALL						=	1,
		MICROCHUNKID_USE_MOVE_OBJECT				=	2,
		MICROCHUNKID_TARGET_PREV_POS				=	3,
		MICROCHUNKID_PATH_SOLVE_PTR				=	4,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			BaseActionCodeClass::Save( csave );
		csave.End_Chunk();

		if ( Path != NULL ) {
			csave.Begin_Chunk( CHUNKID_PATH );
				Path->Save( csave );
			csave.End_Chunk();
		}

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FIRST_CALL,			FirstCall		);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_USE_MOVE_OBJECT,	UseMoveObject	);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TARGET_PREV_POS,	TargetPrevPos	);
			WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_PATH_SOLVE_PTR,	PathSolver		);
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_PATH_ACTION );
			PathAction.Save( csave );
		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					BaseActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_FIRST_CALL,			FirstCall		);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_USE_MOVE_OBJECT,	UseMoveObject	);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_TARGET_PREV_POS,	TargetPrevPos	);

							case MICROCHUNKID_PATH_SOLVE_PTR:
								LOAD_MICRO_CHUNK( cload, PathSolver );
								if (PathSolver != NULL) {
									REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&PathSolver );
								}
								break;

							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				case CHUNKID_PATH:
				{
					Path = new PathClass;
					Path->Load( cload );
				}
				break;

				case CHUNKID_PATH_ACTION:
					PathAction.Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual void	Begin_Hibernation( void ) override
	{
		BaseActionCodeClass::Begin_Hibernation();
		FirstCall = true;

		//
		//	Return the path we got from the manager
		//
		if ( PathSolver != NULL ) {
			PathMgrClass::Return_Path_Object( PathSolver );
			PathSolver = NULL;
		}

		return ;
	}


	virtual	void	Init( ActionClass * action ) override
	{
		BaseActionCodeClass::Init( action );
		FirstCall = true;

		SoldierGameObj *soldier_obj = Action->Get_Action_Obj()->As_SoldierGameObj();
		if ( soldier_obj && Action->Get_Parameters().LookDuration != 0 ) {
			soldier_obj->Look_At( Action->Get_Parameters().LookLocation, Action->Get_Parameters().LookDuration );
		}

		UseMoveObject = (Action->Get_Parameters().MoveObjectRef.Get_Ptr() != NULL);
	}

	virtual void	Shutdown( void ) override
	{
		//
		//	Return the path we got from the manager
		//
		if (PathSolver != NULL) {
			PathMgrClass::Return_Path_Object (PathSolver);
			PathSolver = NULL;
		}

		Release_Path();
		BaseActionCodeClass::Shutdown();
		return ;
	}

	virtual	void	Init( WaypathClass *waypath )
	{
		//
		//	Return the path we got from the manager
		//
		if (PathSolver != NULL) {
			PathMgrClass::Return_Path_Object (PathSolver);
			PathSolver = NULL;
		}

		Release_Path();

		Prepare_Path ();
		Path->Set_Traversal_Type( PathClass::LINEAR );
		Path->Initialize( waypath, -1, -1 );
		Initialize_Vehicle_AI();

		return ;
	}

	virtual	bool	Is_Busy( void ) override
	{
		return ( PathAction.Get_State () != PathActionClass::STATE_FINISHED );
	}


	virtual	ActResult	Arrived()
	{
		if ( Action->Get_Parameters().MoveFollow ) {
			return ACTION_IN_PROGRESS;				// Do nothing special when we arrive
		}

		if ( Action->Get_Action_Obj() && Action->Get_Parameters().ShutdownEngineOnArrival ) {
			SmartGameObj *transport_obj = Peek_Transport_Object( Action->Get_Action_Obj() );
			if ( transport_obj && transport_obj->As_VehicleGameObj() ) {
				transport_obj->As_VehicleGameObj()->Enable_Engine (false);
			}
		}

		Action->Done( ACTION_COMPLETE_NORMAL );
		return ACTION_DONE;			// We are done
	}

	virtual	ActResult	Beeline( void )
	{
		//
		//	Make sure we have a valid path object to follow
		//
		if ( Path == NULL ) {
			Prepare_Path();
			Vector3 curr_pos;
			Action->Get_Action_Obj()->Get_Position( &curr_pos );
			Path->Initialize( curr_pos, Action->Get_Parameters().MoveLocation );
			Initialize_Vehicle_AI();
		}

		ActResult result = ACTION_IN_PROGRESS;
		if ( Path != NULL ) {
			result = Traverse_Path ();
		}

		// Tell the unit to beeline w/o using pathfind
		/*ActResult result = ACTION_IN_PROGRESS;
		if ( Move_To_Absolute( Action->Get_Parameters().MoveLocation ) ) {
			result = Arrived();
		}*/

		return result;
	}

	void	Prepare_Path( void )
	{
		//
		//	Allocate a path object (if necessary)
		//
		if ( Path == NULL ) {
			Path = new PathClass;
		}

		//
		//	Setup the path traverser
		//
		PathObjectClass path_obj;
		Configure_Path_Object( path_obj );

		//
		//	Initialize the path
		//
		Path->Set_Path_Object( path_obj );
		return ;
	}

	void	Initialize_Vehicle_AI( void )
	{
		//
		//	Get the transport object
		//
		SmartGameObj *transport_obj = Peek_Transport_Object( Action->Get_Action_Obj() );

		//
		//	Initialize the different driving/flying AI's
		//
		VehicleDriver.Initialize( transport_obj, Path );
		VehicleDriver.Force_Backup( Action->Get_Parameters().MoveBackup );
		Pilot.Initialize( transport_obj );
		Pilot.Set_Mode( PilotClass::MODE_FLY_TO_POINT );
		Pilot.Set_Current_Path ( Path );
		Pilot.Set_Arrived_Dist( Action->Get_Parameters().MoveArrivedDistance );
		Pilot.Set_Is_Exact_Z_Important( bool(Action->Get_Parameters().WaypathID != 0) );

		//
		//	Check to make sure we aren't trying to drive a "simple" vehicle around
		//
		if (	transport_obj != NULL && transport_obj->As_VehicleGameObj () != NULL &&
				transport_obj->Peek_Physical_Object () != NULL &&
				transport_obj->Peek_Physical_Object ()->As_MoveablePhysClass () == NULL )
		{
			//
			//	Warn the user that they are trying to drive a non-driveable object
			//
			Debug_Say (("Warning!  Attempting to drive a non-moveable vehicle.  ID = %d.\n", transport_obj->Get_ID ()));
		}

		return ;
	}

	void	Initialize_Waypath( void )
	{
		Prepare_Path ();

		//
		//	Lookup the waypath object
		//
		WaypathClass *waypath = PathfindClass::Get_Instance()->Find_Waypath( Action->Get_Parameters().WaypathID );
		if ( waypath == NULL ) {
			Debug_Say (("Warning!  Waypath %d does not exist.\n", Action->Get_Parameters().WaypathID));
		}
		WWASSERT( waypath != NULL );

		//
		//	Initialize the path
		//
		Path->Set_Traversal_Type( Action->Get_Parameters().WaypathSplined ? PathClass::SPLINE : PathClass::LINEAR );
		Path->Initialize( waypath, Action->Get_Parameters().WaypointStartID, Action->Get_Parameters().WaypointEndID );
		return ;
	}

	void	Configure_Path_Object( PathObjectClass &path_obj )
	{
		//
		//	Determine which game object will actually be doing the moving along the path
		//
		SmartGameObj *transport_obj = Peek_Transport_Object( Action->Get_Action_Obj() );
		WWASSERT( transport_obj != NULL );

		//
		//	Determine the turn radius
		//
		float turn_radius					= 0;
		VehicleGameObj *vehicle_obj	= transport_obj->As_VehicleGameObj();

		if ( vehicle_obj != NULL ) {
			turn_radius = vehicle_obj->Get_Turn_Radius();
		}

		//
		//	Dig out the physics object that will be moving along the path
		//
		MoveablePhysClass *phys_obj = transport_obj->Peek_Physical_Object()->As_MoveablePhysClass();
		if (phys_obj != NULL) {

			//
			//	Configure the path-traverser object
			//
			path_obj.Initialize( *phys_obj );
			path_obj.Set_Max_Speed( transport_obj->Get_Max_Speed () );
			path_obj.Set_Turn_Radius( turn_radius );
			path_obj.Set_Flag( PathObjectClass::IS_VEHICLE, bool(vehicle_obj != NULL) );
		}

		//
		//	Pass the key information onto the path object
		//
		SoldierGameObj *soldier = Action->Get_Action_Obj()->As_SoldierGameObj();
		if ( soldier != NULL ) {
			path_obj.Set_Key_Ring( soldier->Get_Key_Ring () );
		}

		return ;
	}

	void	Initialize_Path_Solver( void )
	{
		//
		//	Get a new path object from the manager (if necessary)
		//
		if (PathSolver == NULL) {
			PathSolver = PathMgrClass::Request_Path_Object();
		}

		//
		//	Get the destination position
		//
		Vector3 dest_pos	= Action->Get_Parameters().MoveLocation;
		TargetPrevPos		= Action->Get_Parameters().MoveLocation;

		//
		//	Setup the path traverser
		//
		PathObjectClass path_obj;
		Configure_Path_Object( path_obj );

		//
		//	Initialize the path itself
		//
		Vector3 cur_pos;
		Peek_Transport_Object (Action->Get_Action_Obj())->Get_Position( &cur_pos );

		PathSolver->Set_Path_Object( path_obj );
		PathSolver->Reset( cur_pos, dest_pos, 2.0F );
		return ;
	}

	void	Initialize_Path( void )
	{
		Release_Path();

		//
		//	Determine if we should use a waypath instead of trying to solve
		// the path dynamically
		//
		if ( Action->Get_Parameters().WaypathID != 0 ) {
			Initialize_Waypath();
			Initialize_Vehicle_AI();
		} else {
			Initialize_Path_Solver();
		}

		return ;
	}

	void	Release_Path( void )
	{
		Pilot.Reset();
		VehicleDriver.Reset();
		REF_PTR_RELEASE( Path );
		// Reset the PathAction class too - otherwise it keeps the pointer we game it to Path. ST - 9/6/2001 10:36AM
		PathAction.Reset();
		return ;
	}

bool
Clip_Point (Vector3 *point, const AABoxClass &box)
{
	Vector3 temp_point = *point;

	//
	//	Clip the temporary point
	//
	temp_point.X = std::clamp (temp_point.X, box.Center.X - box.Extent.X, box.Center.X + box.Extent.X);
	temp_point.Y = std::clamp (temp_point.Y, box.Center.Y - box.Extent.Y, box.Center.Y + box.Extent.Y);
	temp_point.Z = std::clamp (temp_point.Z, box.Center.Z - box.Extent.Z, box.Center.Z + box.Extent.Z);
	
	//
	//	Did the clip change the point?
	//
	bool retval = (point->X != temp_point.X);
	retval		|= (point->Y != temp_point.Y);
	retval		|= (point->Z != temp_point.Z);

	//
	//	Pass the new point back to the caller
	//
	(*point) = temp_point;

	return retval;
}

	virtual	ActResult	Traverse_Path ( void )
	{
		ActResult act_result = ACTION_IN_PROGRESS;

		//
		// Let the pathfind system know which directions we can (and are) moving
		//
		Path->Set_Movement_Directions( PathClass::MOVE_X | PathClass::MOVE_Y );

		//
		//	Is the soldier only moving in Z?
		//
		SmartGameObj *obj			= Action->Get_Action_Obj();
		SoldierGameObj *soldier	= obj->As_SoldierGameObj();
		if( soldier != NULL ) {
			if ( soldier->Is_On_Ladder() ) {
				Path->Set_Movement_Directions( PathClass::MOVE_Z );
			}
		}

		//
		//	Get the object's current position
		//
		Vector3 curr_pos;
		obj->Get_Position( &curr_pos );

		//
		//	Determine where the object should go next
		//
		Vector3 next_pos;
		if( Path->Evaluate_Next_Point( curr_pos, next_pos ) ) {

			/*return act_result;
			}*/

			//
			//	Allow debug display of the path
			//
			Path->Display_Path( Display_Findpaths );

			//
			//	What should we be doing?
			//
			switch( Path->Get_State() )
			{
				case PathClass::STATE_PATH_COMPLETE:
				case PathClass::STATE_TRAVERSING_PATH:

					//
					//	Don't allow any movement if something is in our way
					//					
					if ( soldier == NULL || soldier->Is_Soldier_Blocked( curr_pos ) == false ) {
						if ( Move_To_Absolute( next_pos ) ) {
							act_result = Arrived();
						}
					}
					break;

				case PathClass::STATE_ACTION_REQUIRED:
					act_result = Perform_Action();
					break;

				/*case PathClass::STATE_PATH_COMPLETE:
					act_result = Arrived();
					break;					*/
			}

		} else {

			//
			//	Error, free the path object
			//
			Release_Path();
			act_result = ACTION_DONE;
			Action->Done( ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE );
		}

		return act_result;
	}

	virtual	ActResult	Perform_Action ( void )
	{
		SmartGameObj *obj = Action->Get_Action_Obj();

		//
		//	Determine what action we need to perform
		//
		PathClass::ACTION_ID action_id = Path->Get_Action_Type();
		switch( action_id )
		{
			case PathClass::ACTION_LADDER:
			{
				//
				//	Initialize the action object that will be responsible for
				// controlling this action
				//
				PathAction.Initialize( PathActionClass::TYPE_LADDER, Path, obj, NULL );
				PathAction.Set_Ladder_Index( Path->Get_Action_Mechanism() );
			}
			break;

			case PathClass::ACTION_MECHANISM:
			{
				//
				//	Lookup the physics object we will be interacting with
				//
				uint32 mechanism_id	= Path->Get_Action_Mechanism();
				StaticPhysClass *mechanism = PhysicsSceneClass::Get_Instance ()->Find_Static_Object( mechanism_id );
				if (mechanism != NULL) {

					PathActionClass::TYPE type = PathActionClass::TYPE_UNKNOWN;

					//
					//	Determine what type of mechanism this is
					//
					if( mechanism->As_ElevatorPhysClass () != NULL ) {
						type = PathActionClass::TYPE_ELEVATOR;
					} else if( mechanism->As_DoorPhysClass () != NULL ) {
						type = PathActionClass::TYPE_DOOR;
					}

					//
					//	Initialize the action object that will be responsible for
					// controlling this action
					//
					PathAction.Initialize( type, Path, obj, mechanism );
				}
			}
			break;

			case PathClass::ACTION_LEAP:
			case PathClass::ACTION_JUMP:
			{
				//
				//	Get the point where we will be jumping to
				//
				Vector3 dest_pos;
				Path->Get_Action_Destination( dest_pos );

				//
				//	Instruct the physics object to jump to this point
				//
				HumanPhysClass *human_phys_obj = obj->Peek_Physical_Object()->As_HumanPhysClass ();
				if( human_phys_obj != NULL ) {
					human_phys_obj->Jump_To_Point( dest_pos );
				}

				//
				//	Initialize the action object that will be responsible for
				// controlling this action
				//
				PathAction.Initialize( PathActionClass::TYPE_JUMPING, Path, obj, NULL );
			}
			break;
		}

		return ACTION_IN_PROGRESS;
	}

	virtual	ActResult	Process_Action ( void )
	{
		//
		//	Continue to let the action take place
		//
		PathAction.Process ();

		//
		//	Does this action require us to move somewhere?
		//
		if( PathAction.Get_State () == PathActionClass::STATE_MOVING ) {
			Move_To_Absolute( PathAction.Get_Destination (), true );
		}

		return ACTION_IN_PROGRESS;
	}

	virtual	ActResult	Solve_Path ( void )
	{
		ActResult act_result = ACTION_IN_PROGRESS;

		//
		//	Evaluate the path
		//
		PathSolveClass::STATE_DESC result = PathSolver->Get_State();

		//
		//	Has the path finished solving?
		//
		bool is_finished = true;
		switch( result ) {

			case PathSolveClass::THINKING:
				is_finished	= false;
				break;

			case PathSolveClass::SOLVED_PATH:

				//
				//	Allocate and initialize the path object
				//
				Prepare_Path();
				Initialize_Vehicle_AI();

				//
				//	Configure the path object with information we learned
				// during the path solve
				//
				Path->Set_Traversal_Type( PathClass::SPLINE );
				Path->Initialize( *PathSolver );
				break;

			case PathSolveClass::ERROR_INVALID_START_POS:
			case PathSolveClass::ERROR_INVALID_DEST_POS:
			case PathSolveClass::ERROR_NO_PATH:
				break;
		}

		//
		//	If we are finished solving the path, then release our
		// hold on the path solver.
		//
		if ( is_finished ) {
			PathMgrClass::Return_Path_Object( PathSolver );
			PathSolver = NULL;
		}

		return act_result;
	}


	virtual	ActResult	Pathfind( void )
	{
		ActResult act_result	= ACTION_IN_PROGRESS;

		if( PathAction.Get_State () != PathActionClass::STATE_FINISHED ) {

			//
			//	Keep performing the action until we've completed it
			//
			act_result = Process_Action();
		} else {

			if( PathSolver != NULL ) {

				//
				//	Try to find a path to the destination
				//
				act_result = Solve_Path();
			}

			if( PathSolver == NULL) {
				if( Path != NULL ) {

					//
					//	Move along the path we found
					//
					act_result = Traverse_Path();
				} else if ( PathSolver == NULL ){

					//
					//	Unable to solve path...
					//
					//	If this level doesn't have any pathfind data, then allow the unit
					// to beeline -- otherwise, kill the action...
					//
					if (PathfindClass::Get_Instance ()->Does_Pathfind_Data_Exist ()) {
						
						//
						//	Kill the action
						//
						Action->Done( ACTION_COMPLETE_PATH_BAD_DEST );
						act_result = ACTION_DONE;

					} else {
						act_result = Beeline ();
					}
				}
			}
		}

		return act_result;
	}

	virtual float	Get_Distance_From_Goal (void) override
	{
		float dist_to_goal = 0;

		//
		//	Get the distance remaining from the path object (if possible)
		//
		if ( Path != NULL ) {
			dist_to_goal = Path->Get_Remaining_Path_Length();
		} else {
			dist_to_goal = BaseActionCodeClass::Get_Distance_From_Goal();
		}

		//
		//	Return the length
		//
		return dist_to_goal;
	}

	void	Update_Move_Location( void )
	{
		ScriptableGameObj *target	= Action->Get_Parameters().MoveObjectRef.Get_Ptr();

		if ( target == NULL ) {		// If the target is gone
//			Debug_Say(( "No Movement Target\n" ));
//			Move_Target_Gone();
//			Action->Reset( Action->Get_Parameters().Priority );		// Stop current Command	(this may get this deleted!)
			// Do we need to notify observers?
//			return ACTION_DONE;
			return;
		}

//		Action->Get_Parameters().MoveLocation = target->Get_Transform() * Action->Get_Parameters().MoveObjectOffset;
		target->Get_Position( &(Action->Get_Parameters().MoveLocation) );

		if ( FirstCall == false ) {
			SmartGameObj *	obj = Action->Get_Action_Obj();
			SoldierGameObj * soldier = NULL;
			if ( target->As_PhysicalGameObj() != NULL ) {
				soldier = target->As_PhysicalGameObj()->As_SoldierGameObj();
			}

			//	Get the current positions of the action object and the target object
			Vector3 curr_pos;
			Vector3 target_pos;
			target->Get_Position( &target_pos );
			obj->Get_Position( &curr_pos );

			//	Determine how far the target has moved since we last
			// did a pathfind.
			float target_move_dist	= (target_pos - TargetPrevPos).Length ();
			float to_target_dist		= (curr_pos - target_pos).Length ();
			if (	to_target_dist > 0 &&
					(soldier == NULL || (soldier->Is_Airborne () == false && PathAction.Get_State () == PathActionClass::STATE_FINISHED)))
			{
				//	Get the ratio of the distance the target has moved relative
				// to the distance we are from the target.
				float ratio = target_move_dist / to_target_dist;
				if ((target_move_dist > 2.0F && ratio > 0.1F)) {
					Initialize_Path();
				}
			}
		}
	}

	virtual	ActResult	Act() override
	{
		WWPROFILE( "Goto Act" );
		SoldierGameObj * soldier = Action->Get_Action_Obj()->As_SoldierGameObj();
		ScriptableGameObj * look_obj = Action->Get_Parameters().LookObjectRef.Get_Ptr();
		if ( soldier != NULL && look_obj != NULL ) {
			Vector3 pos;
			look_obj->Get_Position( &pos );
			pos.Z += 1;	// Offset bullseye
			soldier->Update_Look_At( pos );
		}

		if ( UseMoveObject ) {
			Update_Move_Location();
		}

		if ( FirstCall ) {
			Initialize_Path();
			FirstCall = false;
		}

#if 0
		//	Make the soldier crouch (if necessary)
		if ( soldier && (soldier->Is_Crouched() != Action->Get_Parameters().MoveCrouched) ) {
			soldier->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH );
		}
#else
		//	Make the soldier crouch (if necessary)
		if ( soldier ) {
			soldier->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH, Action->Get_Parameters().MoveCrouched );
		}
#endif

		ActResult result = ACTION_IN_PROGRESS;

		//	Determine whether we should use pathfinding or
		// simply beeline to the destination
		if ( Action->Get_Parameters().MovePathfind ) {
			result = Pathfind();
		} else {
			result = Beeline();
		}

		return result;
	}

private:

	PathClass *			Path;
	PathSolveClass	*	PathSolver;
	PathActionClass	PathAction;

	bool					FirstCall;
	bool					UseMoveObject;
	Vector3				TargetPrevPos;
};


/*
** EnterExit Action Code Class
*/
class	EnterExitActionCodeClass;
SimplePersistFactoryClass<EnterExitActionCodeClass, CHUNKID_ACTION_CODE_ENTER_EXIT>	_EnterExitActionCodeClassFactory;

class	EnterExitActionCodeClass : public BaseActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _EnterExitActionCodeClassFactory; }

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001312,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			BaseActionCodeClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					BaseActionCodeClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	ActResult	Act( void ) override
	{
		SoldierGameObj	* obj = Action->Get_Action_Obj()->As_SoldierGameObj();
		WWASSERT( obj );
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_ACTION );
		Action->Done( ACTION_COMPLETE_NORMAL );
		return ACTION_DONE;
	}
};


/*
** Dive Action Code Class
*/
class	DiveActionCodeClass;
SimplePersistFactoryClass<DiveActionCodeClass, CHUNKID_ACTION_CODE_DIVE>	_DiveActionCodeClassFactory;

class	DiveActionCodeClass : public BaseActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _DiveActionCodeClassFactory; }

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001310,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			BaseActionCodeClass::Save( csave );
		csave.End_Chunk();
		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					BaseActionCodeClass::Load( cload );
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;
			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	ActResult	Act( void ) override
	{
		SoldierGameObj	* obj = Action->Get_Action_Obj()->As_SoldierGameObj();
		WWASSERT( obj );

		Vector3 rel_pos;
		Matrix3D::Inverse_Transform_Vector( obj->Get_Transform(), Action->Get_Parameters().MoveLocation, &rel_pos );

		if ( WWMath::Fabs( rel_pos.X ) > WWMath::Fabs( rel_pos.Y ) ) {
			obj->Set_Boolean_Control( rel_pos.X > 0 ? ControlClass::BOOLEAN_DIVE_FORWARD : ControlClass::BOOLEAN_DIVE_BACKWARD );
		} else {
			obj->Set_Boolean_Control( rel_pos.Y > 0 ? ControlClass::BOOLEAN_DIVE_LEFT : ControlClass::BOOLEAN_DIVE_RIGHT );
		}

#if 0
		if ( !obj->Is_Crouched() ) {
			obj->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH );
		}
#else
		obj->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH );
#endif

		Action->Done( ACTION_COMPLETE_NORMAL );
		return ACTION_DONE;
	}
};


/*
** Attack - Location
*/
class	AttackActionCodeClass;
SimplePersistFactoryClass<AttackActionCodeClass, CHUNKID_ACTION_CODE_ATTACK>	_AttackActionCodeClassFactory;

class	AttackActionCodeClass : public GotoActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _AttackActionCodeClassFactory; }

	AttackActionCodeClass( void ) :
		UseAttackObject( false ),
		WanderPos(0,0,0),
		WanderTimer(0),
		HasArrived( false ),
		HoldPrimaryTriggerTimer( 0 )
	{
		ErrorScale = FreeRandom.Get_Float( 0.75f, 1.25f );
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001308,
		CHUNKID_VARIABLES,

		MICROCHUNKID_USE_ATTACK_OBJECT			=	1,
		MICROCHUNKID_IS_BLOCKED						=	2,
		MICROCHUNKID_CHECK_BLOCKED_TIMER			=	3,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			GotoActionCodeClass::Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_USE_ATTACK_OBJECT,		UseAttackObject		);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_BLOCKED,					IsBlocked				);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CHECK_BLOCKED_TIMER,		CheckBlockedTimer		);
		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					GotoActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_USE_ATTACK_OBJECT,		UseAttackObject		);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_BLOCKED,				IsBlocked				);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_CHECK_BLOCKED_TIMER,	CheckBlockedTimer		);
							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	void	Init( ActionClass * action ) override
	{
		GotoActionCodeClass::Init( action );
		UseAttackObject = (Action->Get_Parameters().AttackObjectRef.Get_Ptr() != NULL);
		IsBlocked = action->Get_Parameters().AttackCheckBlocked;
		CheckBlockedTimer = 0;
		WanderPos = Vector3(0,0,0);
	}

	virtual	ActResult	Arrived() override
	{
		if ( !HasArrived ) {
//			Debug_Say(( "Attack Arrived %d\n", HasArrived ));
			HasArrived = true;
			SoldierGameObj	* obj = Action->Get_Action_Obj()->As_SoldierGameObj();
			if ( obj != NULL ) {
				int observer_id = Action->Get_Parameters ().ObserverID;
				if ( observer_id != 0 ) {
					const GameObjObserverList & observer_list = obj->Get_Observers();
					for( int index = 0; index < observer_list.Count(); index++ ) {
						if ( observer_list[ index ]->Get_ID() == observer_id ) {
							observer_list[ index ]->Custom( obj, CUSTOM_EVENT_ATTACK_ARRIVED, Action->Get_Parameters().ActionID, NULL );
						}
					}
				}
			}
		}

		return ACTION_IN_PROGRESS;				// Do nothing special when we arrive
	}

	// Attack the given absolute position, return nothing
	void	Attack_Absolute( Vector3 & abs_pos )
	{
		WWPROFILE( "Attack Absolute" );
		IsAttacking = true;

		float range = Action->Get_Parameters().AttackRange;

		SmartGameObj *	obj = Action->Get_Action_Obj();

		Vector3 my_pos;
		obj->Get_Position( &my_pos );

		// move the abs_pos around a little for AttackError
		Vector3 attack_pos = abs_pos;
		Matrix3D error;
		error.Obj_Look_At( my_pos, abs_pos, 0 );
		float dist = (my_pos - abs_pos).Length();
		float move = TimeManager::Get_Total_Seconds() * ErrorScale;

		float error_angle = WWMath::Fabs(Action->Get_Parameters().AttackError) + 
									WWMath::Fabs(obj->Get_Weapon_Error());

#if 0		
		// Modify error angle based on difficulty
		switch ( CombatManager::Get_Difficulty_Level() ) {
			case 0:	
				error_angle += 20;
				break;
			case 1:	
				// no change
				break;
			case 2:	
				error_angle -= 20;
				break;
		};

		if ( error_angle < 0 ) {
			error_angle = 0;
		}

#endif

		// If AttackErrorOverride, only use AttackError
		if ( Action->Get_Parameters().AttackErrorOverride ) {
			error_angle = WWMath::Fabs(Action->Get_Parameters().AttackError);
		}

		error.Rotate_Z( DEG_TO_RADF( WWMath::Sin( move * 1.7f ) * error_angle ) );
		error.Rotate_Y( DEG_TO_RADF( WWMath::Cos( move ) * error_angle ) * 0.5f );
		attack_pos = error * Vector3( dist,0,0 );

		if ( obj->Set_Targeting( attack_pos ) == false && !Action->Get_Parameters().AttackForceFire ) {
			// we are not facing it yet, don't fire
			return;
		}

		WeaponClass * weapon = obj->Get_Weapon();
		WWASSERT( weapon );

		if ( Action->Get_Parameters().AttackCheckBlocked ) {
			WWPROFILE( "Check Blocked" );
			CheckBlockedTimer -= TimeManager::Get_Frame_Seconds();
			if ( CheckBlockedTimer <= 0 ) {
				CheckBlockedTimer = FreeRandom.Get_Float( 2, 3 );		// Re check every 2 - 3 sec

				// cast a ray from muzzle to target
				PhysicalGameObj * hit_obj = weapon->Cast_Weapon( abs_pos );
				if ( hit_obj != NULL ) {
					if ( UseAttackObject ) {
						PhysicalGameObj * target = (PhysicalGameObj *)Action->Get_Parameters().AttackObjectRef.Get_Ptr();
						IsBlocked = ( hit_obj != target );
						if ( IsBlocked && target->As_SoldierGameObj() && target->As_SoldierGameObj()->Get_Vehicle() == hit_obj ) {
							IsBlocked = false;
						}
					} else {
						IsBlocked = !hit_obj->Is_Enemy( obj );
					}
				} else {
					IsBlocked = true;
				}
			}
		}

		if ( IsBlocked && !Action->Get_Parameters().AttackForceFire ) {
			return;
		}

		// Update Weapons Dynamic Error
		if ( weapon ) {
			if ( Action->Get_Parameters().AttackCheckBlocked != false ) {
				range = MIN( range, weapon->Get_Range() );	// dont fire if not in range
			}
		}

		my_pos -= abs_pos;
		my_pos.Z = 0;
		if ( my_pos.Length() < range || Action->Get_Parameters().AttackForceFire ) {
			if ( Action->Get_Parameters().AttackPrimaryFire ) {
				obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY );

				// Keep the trigger down (helps obelisk)
				float rate = 0;
				if ( obj->Get_Weapon() ) {
					rate = obj->Get_Weapon()->Get_Primary_Fire_Rate();
				}
				if ( rate != 0 ) {
					HoldPrimaryTriggerTimer = (1 / rate) * 0.75f;
				}
			} else {
				obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_FIRE_SECONDARY );
			}
		}
	}

	virtual	void	Modify_Parameters( const SafeActionParamsStruct & parameters, bool modify_move, bool modify_attack ) override
	{
		//
		//	Copy the parameters that are safe to change in the middle of an action
		//
		if ( modify_attack ) {
			Action->Get_Parameters().AttackLocation		= parameters.AttackLocation;
			Action->Get_Parameters().AttackObjectRef		= parameters.AttackObjectRef;
			Action->Get_Parameters().AttackCrouched		= parameters.AttackCrouched;
			Action->Get_Parameters().AttackRange			= parameters.AttackRange;
			Action->Get_Parameters().AttackCheckBlocked	= parameters.AttackCheckBlocked;
			Action->Get_Parameters().AttackError			= parameters.AttackError;
			Action->Get_Parameters().AttackErrorOverride	= parameters.AttackErrorOverride;
			Action->Get_Parameters().AttackPrimaryFire	= parameters.AttackPrimaryFire;
			Action->Get_Parameters().AttackActive			= parameters.AttackActive;
			Action->Get_Parameters().AttackWanderAllowed	= parameters.AttackWanderAllowed;
			Action->Get_Parameters().AttackFaceTarget		= parameters.AttackFaceTarget;
			Action->Get_Parameters().AttackForceFire		= parameters.AttackForceFire;
		}

		if ( modify_move ) {
			Action->Get_Parameters().MoveObject				= parameters.MoveObject;
			Action->Get_Parameters().MoveObjectOffset		= parameters.MoveObjectOffset;
			Action->Get_Parameters().MoveSpeed				= parameters.MoveSpeed;
			Action->Get_Parameters().MoveArrivedDistance	= parameters.MoveArrivedDistance;
			Action->Get_Parameters().MoveBackup				= parameters.MoveBackup;
			Action->Get_Parameters().MoveFollow				= parameters.MoveFollow;
			Action->Get_Parameters().MoveCrouched			= parameters.MoveCrouched;
			Action->Get_Parameters().ShutdownEngineOnArrival = parameters.ShutdownEngineOnArrival;
			Action->Get_Parameters().MovePathfind			= parameters.MovePathfind;
		}


		//
		//	Reset some of the internal flags
		//
		UseAttackObject	= (parameters.AttackObjectRef.Get_Ptr() != NULL);
		IsBlocked			= false;
		CheckBlockedTimer	= 0;
		return ;
	}

	virtual	ActResult	Act( void ) override
	{
		WWPROFILE( "Attack Act" );
		//
		//	Do the movement (if necessary)
		//
		if ( Action->Get_Parameters().MoveArrivedDistance < DONT_MOVE_ARRIVED_DIST ) {
			GotoActionCodeClass::Act();
		}

		ActResult result = ACTION_IN_PROGRESS;

		//
		//	Only "do" the attack if the attack state is currently active
		//
		if ( Action != NULL && Action->Get_Parameters().AttackActive ) {
			Vector3 set_target;
			result = Process_Attack(&set_target);

			if ( Action != NULL && Action->Get_Parameters().AttackFaceTarget) {
				Pilot.Set_Target (&set_target);
			}

		} else {
			Pilot.Set_Target (NULL);
		}
		
		return result;
	}

	ActResult Process_Attack( Vector3 * set_target )
	{
		SmartGameObj *	obj = Action->Get_Action_Obj();

		if ( HoldPrimaryTriggerTimer > 0 ) {
			HoldPrimaryTriggerTimer -= TimeManager::Get_Frame_Seconds();
			obj->Set_Boolean_Control( ControlClass::BOOLEAN_WEAPON_FIRE_PRIMARY );
		}

		Vector3	target_pos = Action->Get_Parameters().AttackLocation;

		WeaponClass * weapon = obj->Get_Weapon();
		if ( weapon == NULL ) {
			Action->Done( ACTION_COMPLETE_NORMAL );		// Stop current Command	(this may get this deleted!)
			return ACTION_DONE;
		}

		if ( UseAttackObject ) {
			WWPROFILE( "Attack Object" );

			// See if our target still exists
			PhysicalGameObj * target = (PhysicalGameObj *)Action->Get_Parameters().AttackObjectRef.Get_Ptr();
			if ( target ) {
				if ( target->Get_Defense_Object()->Get_Health() <= 0 ) {
//					Debug_Say(( "Attack target dead\n" ));
					target = NULL;
				}
			}
			if ( target == NULL ) {		// If the target is gone or dead

				static int count = 0;
				if (count++ < 20) {
//					Debug_Say(( "No Attack Target\n" ));
				}

				Action->Done( ACTION_COMPLETE_NORMAL );		// Stop current Command	(this may get this deleted!)
				return ACTION_DONE;
			}

			// Goto our target's location
			target_pos = target->Get_Bullseye_Position();

			if ( target->As_SoldierGameObj() != NULL && target->As_SoldierGameObj()->Peek_Human_Phys() != NULL ) {
				Vector3 vel;
				target->As_SoldierGameObj()->Peek_Human_Phys()->Get_Velocity( &vel );
				vel *= TimeManager::Get_Frame_Seconds();
				target_pos += vel;
			}
		}

		// If not moving, use attack crouch
		if ( !IsMoving ) {
			SoldierGameObj	* soldier = Action->Get_Action_Obj()->As_SoldierGameObj();
			if ( soldier != NULL ) {
#if 0
				if ( soldier->Is_Crouched() != Action->Get_Parameters().AttackCrouched ) {
					soldier->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH );
				}
#else
				soldier->Set_Boolean_Control( ControlClass::BOOLEAN_CROUCH, Action->Get_Parameters().AttackCrouched );
#endif
			}
		}


		Attack_Absolute( target_pos );

#if 0
		// Elie Wander!
		// Always wander if attacking!
		WanderTimer -= TimeManager::Get_Frame_Seconds();
		SoldierGameObj	* soldier = Action->Get_Action_Obj()->As_SoldierGameObj();

		if ( !IsMoving && WanderTimer <= 0 && soldier != NULL && Action->Get_Parameters().AttackWanderAllowed ) {
			WWPROFILE( "Wander" );

			Vector3 rel_pos;
			Matrix3D::Inverse_Transform_Vector( obj->Get_Transform(), WanderPos, &rel_pos );
//			Debug_Say(( "Wander Pos %f %f %f\n", WanderPos.X, WanderPos.Y, WanderPos.Z ));
//			Debug_Say(( "Rel Pos %f %f %f\n", rel_pos.X, rel_pos.Y, rel_pos.Z ));

			// If he is near the wander position, or very far from it, pick a new wander pos
			float dist = rel_pos.Length();
			if ( dist < 0.5 || dist > 8 ) {
				WanderTimer = FreeRandom.Get_Float( 0.4f, 1.2f );	// random pause
				// If near the move loc, wander away, else wander back
				Vector3	dif = WanderPos - Action->Get_Parameters().MoveLocation;
				WanderPos = Action->Get_Parameters().MoveLocation;
				if ( dif.Length() < 2 ) {
#if 1
					PathfindClass *pathfind = PathfindClass::Get_Instance();		//	Lookup a safe random position to walk to
					pathfind->Find_Random_Spot( Action->Get_Parameters().MoveLocation, 5, &WanderPos );
#else
					float angle = FreeRandom.Get_Float( 0, DEG_TO_RADF( 360.0f ) );
					Vector3 move( ::sinf(angle), ::cosf(angle), 0 );
//					Debug_Say(( " Move to %f %f %f\n", move.X, move.Y, move.Z ));
					WanderPos += move * FreeRandom.Get_Float( 3, 5 );
#endif

					soldier->Look_At( WanderPos + Vector3( 0,0,1 ), FreeRandom.Get_Float( 0.2f, 0.6f ) );

				}
			}

			Human_Move_To_Relative( rel_pos );
		}
#endif
		*set_target = target_pos;
		return ACTION_IN_PROGRESS;
	}

	bool	UseAttackObject;
	bool	IsBlocked;
	float	CheckBlockedTimer;
	bool	HasArrived;
	float	ErrorScale;

	float	HoldPrimaryTriggerTimer;

	Vector3	WanderPos;
	float		WanderTimer;
};


/*
** Face - Location
*/
class	FaceLocationActionCodeClass;
SimplePersistFactoryClass<FaceLocationActionCodeClass, CHUNKID_ACTION_CODE_FACE_LOCATION>	_FaceLocationActionCodeClassFactory;

class	FaceLocationActionCodeClass : public BaseActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _FaceLocationActionCodeClassFactory; }

	FaceLocationActionCodeClass( void )	:
		EndTime( 0 )
	{
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001305,
		CHUNKID_VARIABLES,

		MICROCHUNKID_END_TIME						=	1,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			BaseActionCodeClass::Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_END_TIME,					EndTime					);
		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					BaseActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_END_TIME,					EndTime					);
							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}
		return true;
	}

	virtual	void	Init( ActionClass * action ) override
	{
		BaseActionCodeClass::Init( action );

		SoldierGameObj *soldier_obj = Action->Get_Action_Obj()->As_SoldierGameObj();
		if ( soldier_obj && Action->Get_Parameters().LookDuration != 0 ) {
			soldier_obj->Look_At( Action->Get_Parameters().LookLocation, Action->Get_Parameters().LookDuration );
		}

		EndTime = TimeManager::Get_Seconds() +
			MAX(action->Get_Parameters().FaceDuration, Action->Get_Parameters().LookDuration);
	}

	virtual	ActResult	Act( void ) override
	{
		BaseActionCodeClass::Act();

		// Look at the moving object
		SoldierGameObj * soldier = Action->Get_Action_Obj()->As_SoldierGameObj();
		ScriptableGameObj * look_obj = Action->Get_Parameters().LookObjectRef.Get_Ptr();
		if ( soldier != NULL && look_obj != NULL ) {
			Vector3 pos;
			look_obj->Get_Position( &pos );
			pos.Z += 1;	// Offset bullseye
			soldier->Update_Look_At( pos );
		}

		if ( Action->Get_Parameters().FaceDuration != 0 ) {
			//
			//	Make the object target (face for humans) the given location
			//
			SmartGameObj *	obj		= Action->Get_Action_Obj();
			Vector3 face_location	= Action->Get_Parameters().FaceLocation;

			obj->Set_Targeting( face_location, false );
		}

		//
		//	Have we timed out yet?
		//
		ActResult result = ACTION_IN_PROGRESS;
		if ( EndTime < TimeManager::Get_Seconds() ) {

			//
			//	Stop this action.  Note:  The following call may cause the
			// object to be deleted...
			//
			Action->Done( ACTION_COMPLETE_NORMAL );
			result = ACTION_DONE;
		}

		return result;
	}

	float	EndTime;
};



/*
** Have a conversation
*/
class	ConversationActionCodeClass;
SimplePersistFactoryClass<ConversationActionCodeClass, CHUNKID_ACTION_CODE_CONVERSATION>	_ConversationActionCodeClassFactory;

class	ConversationActionCodeClass : public GotoActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _ConversationActionCodeClassFactory; }

	enum
	{
		STATE_WALKING_TO_START_POS		= 0,
		STATE_HAVING_CONVERSATION,
		STATE_FINISHED
	};

	ConversationActionCodeClass( void )	:
		Conversation( NULL ),
		State( STATE_WALKING_TO_START_POS ),
		OriginalPos( 0, 0, 0 )
	{
	}

	~ConversationActionCodeClass( void )
	{
		return ;
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001258,
		CHUNKID_VARIABLES,

		MICROCHUNKID_STATE							=	1,
		MICROCHUNKID_ORIGINAL_POS					=	2,
		MICROCHUNKID_CONVERSATION_PTR				=	3,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			GotoActionCodeClass::Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,						State				);
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ORIGINAL_POS,				OriginalPos		);
			
			//
			//	Only save the conversation ptr if it exists in the manager
			//
			if (Conversation != NULL) {
				ConversationClass *conv = ConversationMgrClass::Find_Conversation (Conversation->Get_ID ());
				if (conv != NULL) {

					//
					//	Save the conversation pointer
					//
					WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_CONVERSATION_PTR, Conversation );
					REF_PTR_RELEASE( conv );
				}
			}

		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					GotoActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,						State				);
							READ_MICRO_CHUNK( cload, MICROCHUNKID_ORIGINAL_POS,			OriginalPos		);

							case MICROCHUNKID_CONVERSATION_PTR:
								LOAD_MICRO_CHUNK( cload, Conversation );

								//
								//	Fixup the pointer
								//
								if (Conversation != NULL) {
									REQUEST_REF_COUNTED_POINTER_REMAP( (RefCountClass **)&Conversation );
								}
								break;

							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}

		return true;
	}

	virtual void	Begin_Hibernation( void ) override
	{
		//
		//	Stop this action.  Note:  The following call may cause the
		// object to be deleted...
		//
		if ( Action != NULL ) {
			Action->Done( ACTION_COMPLETE_NORMAL );
		}

		return ;
	}

	virtual	void	Init( ActionClass * action ) override
	{
		State = STATE_WALKING_TO_START_POS;
		REF_PTR_RELEASE( Conversation );
		OriginalPos.Set( 0, 0, 0 );

		SmartGameObj *obj = action->Get_Action_Obj();
		if ( obj != NULL ) {
			SoldierGameObj *soldier = obj->As_SoldierGameObj ();
			if( soldier != NULL ) {


				//
				//	Should we join an already active conversation or should
				// we start our own conversation?
				//
				if ( action->Get_Parameters().ActiveConversationID != 0 ) {
					Conversation = ConversationMgrClass::Find_Active_Conversation( action->Get_Parameters().ActiveConversationID );
				} else {

					//
					//	Start a new conversation
					//
					Conversation = ConversationMgrClass::Start_Conversation( soldier, action->Get_Parameters().SafeConversationName );
				}

				//
				//	By default our new position is our current position
				//
				soldier->Get_Position( &OriginalPos );

				if ( Conversation != NULL ) {
					OratorClass *orator_info = Conversation->Get_Orator_Information( soldier );

					//
					//	Determine if the soldier needs to move to a new location
					//
					if (orator_info->Get_Flag (OratorClass::FLAG_DONT_MOVE) == false) {

						//
						//	Get the soldier's expected position
						//
						Vector3 new_pos = OriginalPos;
						Conversation->Get_Orator_Location( soldier, &new_pos );

						//
						//	Set our movement location
						//
						action->Get_Parameters().MoveLocation			= new_pos;
						action->Get_Parameters().MoveSpeed				= 0.15F;
						action->Get_Parameters().MoveArrivedDistance	= 0.25F;

					} else {
						State = STATE_HAVING_CONVERSATION;
					}
				}
			}
		}

		GotoActionCodeClass::Init( action );
		return ;
	}

	virtual	void	Shutdown( void ) override
	{
		//
		//	Release our hold on the conversation (but don't terminate it)
		//
		if ( Conversation != NULL ) {
			//Conversation->Stop_Conversation ();
			REF_PTR_RELEASE( Conversation );
		}

		GotoActionCodeClass::Shutdown();
		return ;
	}

	virtual	ActResult	Act( void ) override
	{
		SmartGameObj *obj = Action->Get_Action_Obj();
		if ( obj != NULL ) {
			SoldierGameObj *soldier = obj->As_SoldierGameObj ();
			if( soldier != NULL ) {

				if( State == STATE_WALKING_TO_START_POS ) {
					GotoActionCodeClass::Act();
				} else if ( State == STATE_HAVING_CONVERSATION ) {

					//
					//	Check to see if the conversation has finished yet
					//
					if ( Conversation == NULL || Conversation->Is_Finished () ) {
						State ++;

						//
						//	Tell this soldier to return to his original position
						//
						ActionParamsStruct parameters;
						parameters.Priority					= Action->Get_Parameters().Priority;
						parameters.ObserverID				= Action->Get_Parameters().ObserverID;
						parameters.MoveLocation				= OriginalPos;
						parameters.MoveSpeed					= 0.15F;
						parameters.MoveArrivedDistance	= 1.0F;
						soldier->Get_Action()->Goto (parameters);
					}
				}
			}
		}

		//
		//	Have we timed out yet?
		//
		ActResult result = ACTION_IN_PROGRESS;
		if ( Conversation == NULL || Conversation->Is_Finished () ) {

			//
			//	Stop this action.  Note:  The following call may cause the
			// object to be deleted...
			//
			if ( Action != NULL ) {
				Action->Done( ACTION_COMPLETE_NORMAL );
			}
			result = ACTION_DONE;
		}

		return result;
	}

	virtual	ActResult	Arrived (void) override
	{
		if ( Conversation != NULL && State == STATE_WALKING_TO_START_POS ) {

			//
			//	Let the conversation object know this soldier has arrived
			//
			SmartGameObj *obj = Action->Get_Action_Obj();
			if ( obj != NULL ) {
				SoldierGameObj *soldier = obj->As_SoldierGameObj();
				if( soldier != NULL ) {

					OratorClass *orator_info = Conversation->Get_Orator_Information( soldier );
					if ( orator_info != NULL ) {
						orator_info->Set_Flag( OratorClass::FLAG_TEMP_DONT_FACE, false );
					}

					Conversation->Set_Orator_Arrived( soldier, true );
				}
			}


			State ++;
		}

		return ACTION_IN_PROGRESS;
	}

private:
	ActiveConversationClass *	Conversation;
	int								State;
	Vector3							OriginalPos;
};






/*
** Dock vehicle (backup) into a bay
*/
class	DockActionCodeClass;
SimplePersistFactoryClass<DockActionCodeClass, CHUNKID_ACTION_CODE_DOCK>	_DockActionCodeClassFactory;

class	DockActionCodeClass : public GotoActionCodeClass {
public:
	virtual const PersistFactoryClass &	Get_Factory( void ) const override	{ return _DockActionCodeClassFactory; }

	enum
	{
		STATE_DRIVING_TO_ENTRANCE		= 0,
		STATE_DRIVING_TO_SIDE,
		STATE_BACKING_INTO_BAY
	};

	DockActionCodeClass( void )	:
		State( STATE_DRIVING_TO_ENTRANCE )
	{
	}

	~DockActionCodeClass( void )
	{
		return ;
	}

	/*
	** Save / Load
	*/
	enum	{
		CHUNKID_PARENT									=	1031001258,
		CHUNKID_VARIABLES,

		MICROCHUNKID_STATE							=	1,
	};

	virtual	bool	Save( ChunkSaveClass & csave ) override
	{
		csave.Begin_Chunk( CHUNKID_PARENT );
			GotoActionCodeClass::Save( csave );
		csave.End_Chunk();

		csave.Begin_Chunk( CHUNKID_VARIABLES );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STATE,						State				);
		csave.End_Chunk();

		return true;
	}

	virtual	bool	Load( ChunkLoadClass & cload ) override
	{
		while (cload.Open_Chunk()) {
			switch(cload.Cur_Chunk_ID()) {

				case CHUNKID_PARENT:
					GotoActionCodeClass::Load( cload );
					break;

				case CHUNKID_VARIABLES:
					while (cload.Open_Micro_Chunk()) {
						switch(cload.Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK( cload, MICROCHUNKID_STATE,						State				);

							default:
								Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
								break;
						}
						cload.Close_Micro_Chunk();
					}
					break;

				default:
					Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
					break;

			}
			cload.Close_Chunk();
		}

		return true;
	}


	virtual	void	Init( ActionClass * action ) override
	{
		State = STATE_DRIVING_TO_ENTRANCE;

		//
		//	Skip the "driving to side" state if we're a tracked vehicle which
		// can turn in place.
		//
		SmartGameObj *obj			= action->Get_Action_Obj();
		VehicleGameObj *vehicle	= obj->As_VehicleGameObj();
		if ( vehicle != NULL && vehicle->Get_Turn_Radius() == 0 ) {
			State = STATE_DRIVING_TO_SIDE;
		}

		//
		//	Setup the action structure so the vehicle will drive
		// to the entrance of the docking bay
		//
		action->Get_Parameters().MoveLocation	= action->Get_Parameters().DockEntrance;
		action->Get_Parameters().MovePathfind	= true;
		action->Get_Parameters().MoveBackup		= false;
		OriginalMoveSpeed = action->Get_Parameters().MoveSpeed;

		//
		//	Ensure that the arrived distance is valid
		//
		if (action->Get_Parameters().MoveArrivedDistance == DONT_MOVE_ARRIVED_DIST) {
			action->Get_Parameters().MoveArrivedDistance = 1.0F;
		}

		GotoActionCodeClass::Init( action );
		return ;
	}

	virtual	ActResult	Arrived (void) override
	{
		SafeActionParamsStruct &params	= Action->Get_Parameters();
		ActResult result						= ACTION_IN_PROGRESS;

		//
		//	Determine which action we just finished
		//
		if ( State == STATE_DRIVING_TO_ENTRANCE ) {

			//
			//	Terminate the action, we have arrived successfully
			//
			SmartGameObj *obj			= Action->Get_Action_Obj();
			VehicleGameObj *vehicle	= obj->As_VehicleGameObj();

			//
			//	Get the vector which points the direction the vehicle is
			// currently facing
			//
			const Matrix3D &tm	= obj->Get_Transform();
			Vector3 facing_vector;
			tm.Get_X_Vector( &facing_vector );

			//
			//	Build a vector which points the way into the docking bay
			//
			Vector3 entrance_vector = params.DockEntrance - params.DockLocation;
			entrance_vector.Normalize();

			//
			//	Lookup the vehicle's turn radius
			//
			float turn_radius = 2.0F;
			if ( vehicle != NULL ) {
				turn_radius = vehicle->Get_Turn_Radius();
			}			

			if (turn_radius == 0) {
								
				//
				//	Instruct the object to drive to the side of the docking bay
				//
				ActionClass *action = Action;
				action->Get_Parameters().MoveLocation	= params.DockEntrance;
				action->Get_Parameters().MovePathfind	= false;
				action->Get_Parameters().MoveBackup		= true;
				action->Get_Parameters().MoveSpeed		= OriginalMoveSpeed * 0.25F;
				GotoActionCodeClass::Init( action );

			} else {
				
				WaypathClass *test_waypath = new WaypathClass;
				Vector3 side_pos = params.DockLocation + (params.DockEntrance - params.DockLocation) * 0.8F;

				//
				// Determine which side we should drive to in order to be oriented
				// correctly for backing up
				//
				float dot_product = Vector3::Dot_Product (facing_vector, entrance_vector);
				if (dot_product > 0) {

					Vector3 vector1 = entrance_vector;
					Vector3 vector2 = Vector3 (0, 0, 1.0F);
					Vector3 side_vector = Vector3::Cross_Product (vector1, vector2);

					test_waypath->Add_Point (side_pos + (side_vector * (turn_radius * 0.5F)));

					side_pos += (side_vector * (turn_radius * 1.4F));
					test_waypath->Add_Point (side_pos);

				} else {
					Vector3 vector1 = entrance_vector;
					Vector3 vector2 = Vector3 (0, 0, 1.0F);
					Vector3 side_vector = Vector3::Cross_Product (vector1, vector2);

					test_waypath->Add_Point (side_pos - (side_vector * (turn_radius * 0.5F)));

					side_pos -= (side_vector * (turn_radius * 1.4F));
					test_waypath->Add_Point (side_pos);
				}

				//
				//	Instruct the object to drive to the side of the docking bay
				//
				ActionClass *action = Action;
				action->Get_Parameters().MoveLocation	= side_pos;
				action->Get_Parameters().MovePathfind	= true;
				action->Get_Parameters().MoveBackup		= false;
				action->Get_Parameters().MoveSpeed		= OriginalMoveSpeed * 0.25F;
				GotoActionCodeClass::Init( test_waypath );

				//
				// Free our temporary waypath
				//
				REF_PTR_RELEASE( test_waypath );
			}

			//
			//	Move onto the next state
			//
			State ++;
		} else if ( State == STATE_DRIVING_TO_SIDE ) {

			WaypathClass *test_waypath = new WaypathClass;

			SmartGameObj *obj			= Action->Get_Action_Obj();
			VehicleGameObj *vehicle	= obj->As_VehicleGameObj();
			float turn_radius = 2.0F;
			if ( vehicle != NULL ) {
				turn_radius = vehicle->Get_Turn_Radius();
			}

			Vector3 entrance_vector = params.DockEntrance - params.DockLocation;
			entrance_vector.Normalize();
			Vector3 vector1 = entrance_vector;
			Vector3 vector2 = Vector3 (0, 0, 1.0F);
			Vector3 side_vector = Vector3::Cross_Product (vector1, vector2);

			test_waypath->Add_Point (params.DockLocation + (params.DockEntrance - params.DockLocation) * 0.7F - (side_vector * turn_radius * 0.6F));
			test_waypath->Add_Point (params.DockLocation + (params.DockEntrance - params.DockLocation) * 0.3F);
			test_waypath->Add_Point (params.DockLocation);

			//
			//	Instruct the object to back into the docking bay
			//
			ActionClass *action = Action;
			action->Get_Parameters().MoveLocation	= params.DockLocation;
			action->Get_Parameters().MovePathfind	= true;
			action->Get_Parameters().MoveBackup		= true;
			action->Get_Parameters().MoveSpeed		= OriginalMoveSpeed * 0.1F;
			GotoActionCodeClass::Init( test_waypath );

			//
			//	Move onto the next state
			//
			State ++;

			//
			// Free our temporary waypath
			//
			REF_PTR_RELEASE( test_waypath );

			//
			//	Let the observer know that the vehicle will be
			// backing into the bay
			//
			int observer_id = Action->Get_Parameters ().ObserverID;
			if ( observer_id != 0 ) {
				const GameObjObserverList & observer_list = obj->Get_Observers();
				for( int index = 0; index < observer_list.Count(); index++ ) {
					if ( observer_list[ index ]->Get_ID() == observer_id ) {
						observer_list[ index ]->Custom( obj, CUSTOM_EVENT_DOCK_BACKING_IN, 0, NULL );
					}
				}
			}

		} else if ( State == STATE_BACKING_INTO_BAY ) {

			//
			//	Terminate the action, we have arrived successfully
			//
			State ++;
			Action->Done( ACTION_COMPLETE_NORMAL );
			result = ACTION_DONE;
		}

		return result;
	}

private:
	int	State;
	float	OriginalMoveSpeed;
};









/*
**
*/
SafeActionParamsStruct & SafeActionParamsStruct::operator = (const ActionParamsStruct & src)
{
	Priority					= src.Priority;
	ActionID					= src.ActionID;
	ObserverID				= src.ObserverID;
	LookLocation			= src.LookLocation;
	LookObjectRef			= src.LookObject;
	LookDuration			= src.LookDuration;
	MoveLocation			= src.MoveLocation;
	MoveObjectRef			= src.MoveObject;
	MoveObjectOffset		= src.MoveObjectOffset;
	MoveSpeed				= src.MoveSpeed;
	MoveArrivedDistance	= src.MoveArrivedDistance;
	MoveBackup				= src.MoveBackup;
	MoveFollow				= src.MoveFollow;
	MoveCrouched			= src.MoveCrouched;
	ShutdownEngineOnArrival = src.ShutdownEngineOnArrival;
	MovePathfind			= src.MovePathfind;
	AttackRange				= src.AttackRange;
	AttackError				= src.AttackError;
	AttackErrorOverride	= src.AttackErrorOverride;
	AttackPrimaryFire		= src.AttackPrimaryFire;
	AttackCrouched			= src.AttackCrouched;
	AttackObjectRef		= src.AttackObject;
	AttackLocation			= src.AttackLocation;
	AttackCheckBlocked	= src.AttackCheckBlocked;
	AttackActive			= src.AttackActive;
	AttackWanderAllowed	= src.AttackWanderAllowed;
	AttackFaceTarget		= src.AttackFaceTarget;
	AttackForceFire		= src.AttackForceFire;
	WaypathID				= src.WaypathID;
	WaypointStartID		= src.WaypointStartID;
	WaypointEndID			= src.WaypointEndID;
	WaypathSplined			= src.WaypathSplined;
	AnimationLooping		= src.AnimationLooping;
	ForceFacing				= src.ForceFacing;
	IgnoreFacing			= src.IgnoreFacing;
	FaceLocation			= src.FaceLocation;
	FaceDuration			= src.FaceDuration;
	SafeConversationName	= src.ConversationName;
	ActiveConversationID	= src.ActiveConversationID;
	SafeAnimationName		= src.AnimationName;
	AIState					= src.AIState;
	DockLocation			= src.DockLocation;
	DockEntrance			= src.DockEntrance;

	return *this;
}

enum	{
	CHUNKID_VARIABLES								=	712001855,
	CHUNKID_MOVE_OBJECT,
	CHUNKID_ATTACK_OBJECT,
	CHUNKID_LOOK_OBJECT,

	MICROCHUNKID_PRIORITY						=	1,
	MICROCHUNKID_ACTION_ID,
	MICROCHUNKID_OBSERVER_ID,
	MICROCHUNKID_MOVE_LOCATION,
	MICROCHUNKID_MOVE_OBJECT_OFFSET,
	MICROCHUNKID_MOVE_SPEED,
	MICROCHUNKID_MOVE_ARRIVED_DISTANCE,
	MICROCHUNKID_MOVE_BACKUP,
	MICROCHUNKID_MOVE_FOLLOW,
	MICROCHUNKID_MOVE_CROUCHED,
	MICROCHUNKID_MOVE_PATHFIND,
	MICROCHUNKID_ATTACK_RANGE,
	MICROCHUNKID_ATTACK_ERROR,
	MICROCHUNKID_ATTACK_PRIMARY_FIRE,
	MICROCHUNKID_ATTACK_CROUCHED,
	MICROCHUNKID_ATTACK_LOCATION,
	MICROCHUNKID_WAYPATH_ID,
	MICROCHUNKID_WAYPOINT_START_ID,
	MICROCHUNKID_WAYPOINT_END_ID,
	MICROCHUNKID_WAYPATH_SPLINED,
	MICROCHUNKID_ANIMATION_NAME,
	MICROCHUNKID_ANIMATION_LOOPING,
	MICROCHUNKID_FACE_LOCATION,
	MICROCHUNKID_FACE_DURATION,
	MICROCHUNKID_CONVERSATION_NAME,
	MICROCHUNKID_ACTIVE_CONVERSATION_ID,
	MICROCHUNKID_LOOK_LOCATION,
	MICROCHUNKID_LOOK_DURATION,
	MICROCHUNKID_DISABLE_HIBERNATION,
	MICROCHUNKID_ATTACK_CHECK_BLOCKED,
	MICROCHUNKID_ATTACK_ACTIVE,
	MICROCHUNKID_ATTACK_WANDER_ALLOWED,
	MICROCHUNKID_FORCE_FACING,
	MICROCHUNKID_ATTACK_ERROR_OVERRIDE,
	MICROCHUNKID_ATTACK_FACE_TARGET,
	MICROCHUNKID_IGNORE_FACING,
	MICROCHUNKID_ATTACK_FORCE_FIRE,
	MICROCHUNKID_SHUTDOWN_ENGINE_ON_ARRIVAL,
};

bool	SafeActionParamsStruct::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_PRIORITY,					Priority					);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ACTION_ID,					ActionID					);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_OBSERVER_ID,				ObserverID				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOOK_LOCATION,				LookLocation			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LOOK_DURATION,				LookDuration			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_LOCATION,				MoveLocation			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_OBJECT_OFFSET,		MoveObjectOffset		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_SPEED,					MoveSpeed				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_ARRIVED_DISTANCE,	MoveArrivedDistance	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_BACKUP,				MoveBackup				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_FOLLOW,				MoveFollow				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_CROUCHED,				MoveCrouched			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SHUTDOWN_ENGINE_ON_ARRIVAL, ShutdownEngineOnArrival	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_MOVE_PATHFIND,				MovePathfind			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_RANGE,				AttackRange				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_ERROR,				AttackError				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_ERROR_OVERRIDE,	AttackErrorOverride	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_PRIMARY_FIRE,		AttackPrimaryFire		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_CROUCHED,			AttackCrouched			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_LOCATION,			AttackLocation			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_ACTIVE,				AttackActive			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_CHECK_BLOCKED,	AttackCheckBlocked	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_WANDER_ALLOWED,	AttackWanderAllowed	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_FACE_TARGET,		AttackFaceTarget		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ATTACK_FORCE_FIRE,		AttackForceFire		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WAYPATH_ID,					WaypathID				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WAYPOINT_START_ID,		WaypointStartID		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WAYPOINT_END_ID,			WaypointEndID			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WAYPATH_SPLINED,			WaypathSplined			);
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_ANIMATION_NAME,SafeAnimationName		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ANIMATION_LOOPING,		AnimationLooping		);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FORCE_FACING,				ForceFacing				);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IGNORE_FACING,				IgnoreFacing			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FACE_LOCATION,				FaceLocation			);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FACE_DURATION,				FaceDuration			);
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_CONVERSATION_NAME,	SafeConversationName	);
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ACTIVE_CONVERSATION_ID,	ActiveConversationID	);
	csave.End_Chunk();

	if ( MoveObjectRef != NULL ) {
		csave.Begin_Chunk( CHUNKID_MOVE_OBJECT );
		MoveObjectRef.Save( csave );
		csave.End_Chunk();
	}

	if ( AttackObjectRef != NULL ) {
		csave.Begin_Chunk( CHUNKID_ATTACK_OBJECT );
		AttackObjectRef.Save( csave );
		csave.End_Chunk();
	}

	if ( LookObjectRef != NULL ) {
		csave.Begin_Chunk( CHUNKID_LOOK_OBJECT );
		LookObjectRef.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	SafeActionParamsStruct::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_PRIORITY,					Priority					);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ACTION_ID,				ActionID					);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_OBSERVER_ID,				ObserverID				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOOK_LOCATION,			LookLocation			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LOOK_DURATION,			LookDuration			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_LOCATION,			MoveLocation			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_OBJECT_OFFSET,	MoveObjectOffset		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_SPEED,				MoveSpeed				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_ARRIVED_DISTANCE,MoveArrivedDistance	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_BACKUP,				MoveBackup				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_FOLLOW,				MoveFollow				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_CROUCHED,			MoveCrouched			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SHUTDOWN_ENGINE_ON_ARRIVAL, ShutdownEngineOnArrival	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_MOVE_PATHFIND,			MovePathfind			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_RANGE,			AttackRange				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_ERROR,			AttackError				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_ERROR_OVERRIDE,AttackErrorOverride	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_PRIMARY_FIRE,	AttackPrimaryFire		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_CROUCHED,		AttackCrouched			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_LOCATION,		AttackLocation			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_ACTIVE,			AttackActive			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_CHECK_BLOCKED,	AttackCheckBlocked	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_WANDER_ALLOWED,	AttackWanderAllowed	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_FACE_TARGET,	AttackFaceTarget		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ATTACK_FORCE_FIRE,		AttackForceFire		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WAYPATH_ID,				WaypathID				);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WAYPOINT_START_ID,		WaypointStartID		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WAYPOINT_END_ID,		WaypointEndID			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WAYPATH_SPLINED,		WaypathSplined			);
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_ANIMATION_NAME,SafeAnimationName	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ANIMATION_LOOPING,		AnimationLooping		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FORCE_FACING,				ForceFacing			);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IGNORE_FACING,				IgnoreFacing		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FACE_LOCATION,				FaceLocation		);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FACE_DURATION,				FaceDuration		);
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_CONVERSATION_NAME,	SafeConversationName	);
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ACTIVE_CONVERSATION_ID,	ActiveConversationID	);

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_MOVE_OBJECT:
				MoveObjectRef.Load( cload );
				break;

			case CHUNKID_ATTACK_OBJECT:
				AttackObjectRef.Load( cload );
				break;

			case CHUNKID_LOOK_OBJECT:
				LookObjectRef.Load( cload );
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
**
*/
ActionClass::ActionClass( SmartGameObj *obj ) :
	ActionCode( NULL ),
	ActCount( 0 ),
	IsPaused( false )
{
	WWASSERT( obj != NULL );
	ActionObj = obj;
}

ActionClass::~ActionClass( void )
{
	Set_Action_Code( NULL );
	Delete_Action_Code();
}

void	ActionClass::Set_Action_Code( ActionCodeClass * code )
{
	if ( ActionCode != NULL ) {
		ActionCode->Shutdown();
		Register_Action_Code_Deletion( ActionCode );
	}
	ActionCode = code;
	if ( ActionCode ) {
		_ActionCodeChanges++;
		ActionCode->Init( this );
	}
}

void	ActionClass::Done( int reason )
{
	Parameters.Priority = 0;		// reset
	if ( ActionCode != NULL ) {
		Set_Action_Code( NULL );
		Notify_Completed( Parameters.ObserverID, Parameters.ActionID, reason );	// Notify the observer
	}
}

bool	ActionClass::Is_Acting( void )
{
	return ( ActionCode != NULL );
}

bool	ActionClass::Is_Animating( void )
{
	if ( ActionCode != NULL ) {
		return ActionCode->Is_Animating();
	}
	return false;
}

bool	ActionClass::Modify( const ActionParamsStruct & parameters, bool modify_move, bool modify_attack )
{
	bool retval = false;

	if ( ActionCode != NULL ) {
		SafeActionParamsStruct safe_params;
		safe_params = parameters;
		ActionCode->Modify_Parameters( safe_params, modify_move, modify_attack );
		retval = true;
	}

	return retval;
}


bool	ActionClass::Reset( float priority )
{
	if ( priority >= Parameters.Priority ) {
		Done( ACTION_COMPLETE_LOW_PRIORITY );
		return true;
	}
	return false;
}

void	ActionClass::Notify_Completed( int observer_id, int action_id, int reason )
{
	if ( observer_id != 0 ) {
		// Find the observer and notify
		SmartGameObj *	obj = Get_Action_Obj();
		const GameObjObserverList & observer_list = obj->Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			if ( observer_list[ index ]->Get_ID() == observer_id ) {
				observer_list[ index ]->Action_Complete( obj, action_id, (ActionCompleteReason)reason );
			}
		}
	}
}

bool	ActionClass::Request_Action( ActionCodeClass * action, const ActionParamsStruct & parameters )
{
	if ( parameters.Priority >= Parameters.Priority ) {
		bool old_action_completed = false;
		int old_observer_id = 0;
		int old_action_id = 0;

		if ( ActionCode != NULL ) {
			old_action_completed = true;
			old_observer_id = Parameters.ObserverID;
			old_action_id = Parameters.ActionID;
		}
		Parameters = parameters;

		Set_Action_Code( action );
		if ( old_action_completed ) {
			Notify_Completed( old_observer_id, old_action_id, ACTION_COMPLETE_LOW_PRIORITY );
		}
		return true;
	} else {
		Notify_Completed( parameters.ObserverID, parameters.ActionID, ACTION_COMPLETE_LOW_PRIORITY );
		if ( action != NULL ) {
			delete action;
		}
		return false;
	}
}

bool	ActionClass::Follow_Input( const ActionParamsStruct & parameters )
{
	return Request_Action( new FollowInputActionCodeClass(), parameters );
}

bool	ActionClass::Stand( const ActionParamsStruct & parameters )
{
	return Request_Action( new StandActionCodeClass(), parameters );
}

bool	ActionClass::Play_Animation( const ActionParamsStruct & parameters )
{
#if 0		// This is difficult, because huamns play default anims
	// Try to confirm we are not playing a non-action anim
	if ( !Is_Animating() && ActionObj->Get_Anim_Control()->Get_Animation_Name()[0] != 0 ) {
		Debug_Say(( "Can't Action_Play_Animation when playing a non-action anim\n" ));
		WWASSERT( 0 );
	}
#endif

	return Request_Action( new PlayAnimationActionCodeClass(), parameters );
}

bool	ActionClass::Goto( const ActionParamsStruct & parameters )
{
	return Request_Action( new GotoActionCodeClass(), parameters );
}

bool	ActionClass::Attack( const ActionParamsStruct & parameters )
{
	return Request_Action( new AttackActionCodeClass(), parameters );
}

bool	ActionClass::Face_Location( const ActionParamsStruct & parameters )
{
	return Request_Action( new FaceLocationActionCodeClass(), parameters );
}

bool	ActionClass::Have_Conversation( const ActionParamsStruct & parameters )
{
	return Request_Action( new ConversationActionCodeClass(), parameters );
}

bool	ActionClass::Dock_Vehicle( const ActionParamsStruct & parameters )
{
	return Request_Action( new DockActionCodeClass(), parameters );
}

bool	ActionClass::Enter_Exit( const ActionParamsStruct & parameters )
{
	return Request_Action( new EnterExitActionCodeClass(), parameters );
}

bool	ActionClass::Dive( const ActionParamsStruct & parameters )
{
	return Request_Action( new DiveActionCodeClass(), parameters );
}

void	ActionClass::Act( void )
{
	if ( IsPaused == false ) {
		ActCount++;

		ActionObj->Clear_Control();

		if ( ActionCode != NULL ) {
			WWPROFILE( "Action Code" );
			_ActionActCalls++;
			ActionCode->Act();
		}

		Delete_Action_Code();
	}
}

bool	ActionClass::Is_Active( void )
{
	return ( ActionCode != NULL );
}

bool	ActionClass::Is_Busy( void )
{
	bool retval = false;
	if ( ActionCode != NULL ) {
		retval = ActionCode->Is_Busy();
	}

	return retval;
}


/*
** ActionClass Save and Load
*/
enum	{
	CHUNKID_ACTION_CODE						=	712001900,
	CHUNKID_PARAMETERS,
};

bool	ActionClass::Save( ChunkSaveClass & csave )
{
	if ( ActionCode != NULL ) {
		csave.Begin_Chunk( CHUNKID_ACTION_CODE );
			csave.Begin_Chunk( ActionCode->Get_Factory().Chunk_ID() );
				ActionCode->Get_Factory().Save( csave, ActionCode );
			csave.End_Chunk();
		csave.End_Chunk();
	}

	csave.Begin_Chunk( CHUNKID_PARAMETERS );
		Parameters.Save( csave );
	csave.End_Chunk();

// Don't need to save ActionObj because it is set on ActionClass Constructor
// Don't need to save ActCount because it is diagnostic

	return true;
}

bool	ActionClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_ACTION_CODE:
				while (cload.Open_Chunk()) {
					PersistFactoryClass * factory = SaveLoadSystemClass::Find_Persist_Factory( cload.Cur_Chunk_ID() );
					if ( factory ) {
						ActionCode = (ActionCodeClass *)factory->Load( cload );
						WWASSERT( ActionCode != NULL );
						ActionCode->Set_Action( this );
					}
					cload.Close_Chunk();
				}
				break;

			case CHUNKID_PARAMETERS:
				Parameters.Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized Action chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}


void	ActionClass::Begin_Hibernation( void )
{
	//
	//	Let the action code know we are entering hibernation
	//
	if ( ActionCode != NULL ) {
		ActionCode->Begin_Hibernation();
	}

	return ;
}

void	ActionClass::End_Hibernation( void )
{
	//
	//	Let the action code know we are coming out of hibernation
	//
	if ( ActionCode != NULL ) {
		ActionCode->End_Hibernation();
	}

	return ;
}
