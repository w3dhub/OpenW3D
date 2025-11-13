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
 *                     $Archive:: /Commando/Code/Combat/actionparams.h                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/19/02 11:31a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 30                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ACTIONPARAMS_H
#define	ACTIONPARAMS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

#ifndef	GAMEOBJOBSERVER_H
	#include "gameobjobserver.h"
#endif

/*
**
*/
class		ScriptableGameObj;
typedef	ScriptableGameObj	 GameObject;

/*
**
*/
typedef enum {
	NO_AI_STATE_CHANGE			= -1,
	AI_STATE_IDLE					= 0,
	AI_STATE_SECONDARY_IDLE,
	AI_STATE_SEARCH,
	AI_STATE_COMBAT
} SoldierAIState;

/*
**
*/
const float	DONT_MOVE_ARRIVED_DIST	= 1000.0F;

/*
**
*/
class	ActionParamsStruct {

public:
	ActionParamsStruct( void );

	void	Set_Basic( GameObjObserverClass * script, float priority, int action_id, SoldierAIState ai_state = NO_AI_STATE_CHANGE ) { ObserverID = script->Get_ID(); Priority = priority; ActionID = action_id; AIState = ai_state; }
   void	Set_Basic( int observer_id, float priority, int action_id, SoldierAIState ai_state = NO_AI_STATE_CHANGE ) { ObserverID = observer_id; Priority = priority; ActionID = action_id; AIState = ai_state; }

	void	Set_Look( const Vector3 & location, float duration ) { LookLocation = location; LookDuration = duration; }	
	void	Set_Look( GameObject * object, float duration ) { LookObject = object; LookDuration = duration; }
	void	Set_Look( const Vector3 & obj_pos, float angle,  float duration );

	void	Set_Movement( const Vector3 & location, float speed, float arrived_distance, bool crouched = false ) { MoveLocation = location; MoveSpeed = speed; MoveArrivedDistance = arrived_distance; MoveCrouched = crouched; }
	void	Set_Movement( GameObject *	object, float speed, float arrived_distance, bool crouched = false ) { MoveObject = object; MoveSpeed = speed; MoveArrivedDistance = arrived_distance; MoveCrouched = crouched; }

	void	Set_Attack( const Vector3 & location, float range, float error, bool primary_fire ) { AttackLocation = location; AttackRange = range; AttackError = error; AttackPrimaryFire = primary_fire; }
	void	Set_Attack( GameObject * object, float range, float error, bool primary_fire ) { AttackObject = object; AttackRange = range; AttackError = error; AttackPrimaryFire = primary_fire; }

	void	Set_Animation( const char * name, bool looping ) { AnimationName = name; AnimationLooping = looping; }

	void	Set_Face_Location( const Vector3 &location, float duration )	{ FaceLocation = location; FaceDuration = duration; }
	void	Set_Face_Location(  const Vector3 & obj_pos, float angle, float duration );

	void	Join_Conversation( int active_conversation_id )	{ ActiveConversationID = active_conversation_id; }
	void	Start_Conversation( const char * name )			{ ConversationName = name; }

	void	Dock_Vehicle( const Vector3 &dock_location, const Vector3 &dock_entrance ) { DockLocation = dock_location; DockEntrance = dock_entrance; }

//protected:
	// Note:  all of these must be saved in SafeActionParamsStruct::Save
	int					Priority;
	int					ActionID;
	int					ObserverID;

	Vector3				LookLocation;
	GameObject *		LookObject;
	float					LookDuration;

	Vector3				MoveLocation;
	GameObject *		MoveObject;
	Vector3				MoveObjectOffset;
	float					MoveSpeed;
	float					MoveArrivedDistance;
	bool					MoveBackup;
	bool					MoveFollow;
	bool					MoveCrouched;
	bool					MovePathfind;
	bool					ShutdownEngineOnArrival;

	float					AttackRange;
	float					AttackError;
	bool					AttackErrorOverride;
	GameObject *		AttackObject;
	bool					AttackPrimaryFire;
	bool					AttackCrouched;
	Vector3				AttackLocation;
	bool					AttackCheckBlocked;
	bool					AttackActive;
	bool					AttackWanderAllowed;
	bool					AttackFaceTarget;			// (gth) control over whether VTOL vehicles face their target when attacking
	bool					AttackForceFire;

	bool					ForceFacing;
	Vector3				FaceLocation;
	float					FaceDuration;
	bool					IgnoreFacing;

	int					WaypathID;
	int					WaypointStartID;
	int					WaypointEndID;
	bool					WaypathSplined;

	const char * 		AnimationName;
	bool					AnimationLooping;

	int					ActiveConversationID;
	const char *		ConversationName;	
	SoldierAIState		AIState;

	Vector3				DockLocation;
	Vector3				DockEntrance;
};


inline ActionParamsStruct::ActionParamsStruct( void ) :
	Priority( 0 ),
	ActionID( 0 ),
	ObserverID( 0 ),
	LookLocation( 0,0,0 ),
	LookObject( NULL ),
	LookDuration( 0 ),
	MoveLocation( 0,0,0 ),
	MoveObject( NULL ),
	MoveObjectOffset( 0,0,0 ),
	MoveSpeed( 1 ),
	MoveArrivedDistance( DONT_MOVE_ARRIVED_DIST ),
	MoveBackup( false ),
	MoveFollow( false ),
	MoveCrouched( false ),
	MovePathfind( true ),
	ShutdownEngineOnArrival( false ),
	AttackRange( 20 ),
	AttackError( 0 ),
	AttackErrorOverride( false ),
	AttackObject( NULL ),
	AttackPrimaryFire( true ),
	AttackCrouched( false ),
	AttackLocation( 0,0,0 ),
	AttackCheckBlocked( true ),
	AttackActive( true ),
	AttackWanderAllowed( false ),
	AttackFaceTarget( true ),
	AttackForceFire( false ),
	ForceFacing( false ),
	IgnoreFacing( false ),
	FaceLocation( 0, 0, 0 ),
	FaceDuration( 2.0F ),
	WaypathID( 0 ),
	WaypointStartID( 0 ),
	WaypointEndID( 0 ),
	WaypathSplined( false ),
	AnimationName( NULL ),
	AnimationLooping( false ),
	ActiveConversationID( 0 ),
	ConversationName( NULL ),
	AIState( NO_AI_STATE_CHANGE ),
	DockLocation( 0,0,0 ),
	DockEntrance( 0,0,0 )
{
}

void	inline ActionParamsStruct::Set_Face_Location( const Vector3 &obj_pos, float angle, float duration )
{
	FaceLocation	= obj_pos;
	FaceLocation.X	+= ::cos( angle );
	FaceLocation.Y	+= ::sin( angle );
	FaceDuration	= duration;
	return ;
}


void	inline ActionParamsStruct::Set_Look( const Vector3 &obj_pos, float angle,  float duration )
{
	LookLocation	= obj_pos;
	LookLocation.X	+= ::cos( angle );
	LookLocation.Y	+= ::sin( angle );
	LookDuration	= duration;
	return ;
}

#endif	// ACTIONPARAMS_H

