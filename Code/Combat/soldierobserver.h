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
 *                     $Archive:: /Commando/Code/Combat/soldierobserver.h                     $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 11:03a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 44                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SOLDIEROBSERVER_H
#define	SOLDIEROBSERVER_H

#include "always.h"
#include "persistentgameobjobserver.h"
#include "vector3.h"
#include "gameobjref.h"
#include "cover.h"
#include "wwstring.h"
#include "actionparams.h"

class		SoldierGameObj;
class		PhysicalGameObj;

class	SoldierObserverClass : public PersistentGameObjObserverClass
{
public:
	SoldierObserverClass(void);
	virtual	~SoldierObserverClass(void);

	virtual const char * Get_Name( void ) override						{ return "Innate Soldier"; }

	// PersistClass interface
	virtual const PersistFactoryClass& Get_Factory(void) const override;
	virtual bool Save(ChunkSaveClass &csave) override;
	virtual bool Load(ChunkLoadClass &cload) override;

	// GameObjObserverClass interface
	virtual void Attach(GameObject* obj) override;
	virtual void Detach(GameObject* obj) override;

	virtual	void	Created( GameObject * obj ) override;
	virtual	void	Destroyed( GameObject * obj ) override;
	virtual	void	Killed( GameObject * obj, GameObject * killer ) override;
	virtual	void	Damaged( GameObject * obj, GameObject * damager, float amount ) override;
	virtual	void	Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override;
	virtual	void	Sound_Heard( GameObject * obj, const CombatSound & sound ) override;
	virtual	void	Enemy_Seen( GameObject * obj, GameObject * enemy ) override;
	virtual	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override;
	virtual	void	Timer_Expired( GameObject * obj, int timer_id ) override;
	virtual	void	Animation_Complete( GameObject * /* obj */, const char * /* animation_name */ ) override {}
	virtual	void	Poked( GameObject * obj, GameObject * poker ) override;
	virtual	void	Entered( GameObject * /* obj */, GameObject * /* enterer */ ) override {}
	virtual	void	Exited( GameObject * /* obj */, GameObject * /* exiter */ ) override {}

	// State access
	void		Reset_Conversation_Timer( void );

	void		Set_Home_Location( const Vector3& location, float radius )	{ HomeLocation = location; HomeRadius = radius; }

	void		Set_Aggressiveness( float aggressiveness )		{	Aggressiveness = aggressiveness; }
	void		Set_Take_Cover_Probability( float probability )	{	TakeCoverProbability = probability; }
	void		Set_Is_Stationary( bool stationary )				{	IsStationary = stationary; }

	void		Get_Information( StringClass & string );

	typedef enum {
		SOLDIER_AI_RELAXED_IDLE = 0,	
		SOLDIER_AI_ALERT_IDLE,	
		SOLDIER_AI_FOOTSTEPS_HEARD,
		SOLDIER_AI_BULLET_HEARD,
		SOLDIER_AI_GUNSHOT_HEARD,
		SOLDIER_AI_ENEMY_SEEN,
		NUM_SOLDIER_AI_STATES,
		SOLDIER_AI_CONDITIONAL_IDLE = 100,	
	} SoldierAIStates;

	bool Set_State( SoldierGameObj * soldier, int state, const Vector3& location = Vector3(0,0,0), GameObject* enemy = NULL);

private:
	int		State;					// Current State
	float		StateTimer;				// Time spent in this state

	Vector3	HomeLocation;			// Location to stay centered around
	float		HomeRadius;				// Radius to stay within

	Vector3	AlertPosition;
	float		ActionTimer;	// Used to make us change actions periodically
	GameObjReference EnemyObject;
	bool		CoveredAttack;
	CoverEntryClass * CoverPosition;
	float		ConversationTimer;
	bool		IsAlerted;

	float		Aggressiveness;
	float		TakeCoverProbability;
	bool		IsStationary;

	StringClass	SubStateString;

	int		LastEvent;
	int		LastWeaponIndex;

	/*
	** Innate AI States
	*/
	void State_Changed( SoldierGameObj * soldier );
	void Notify_Neighbors_Sound( SoldierGameObj * soldier, const CombatSound & sound );
	void Notify_Neighbors_Enemy( SoldierGameObj * soldier, GameObject * enemy );

	/*
	** Innate AI State Actions
	*/
	void Think( SoldierGameObj * soldier, bool is_new_state = false );
	void State_Act( SoldierGameObj* soldier, bool is_new_state = false );
	void State_Act_Idle( SoldierGameObj * soldier );
	void State_Act_Footsteps_Heard( SoldierGameObj * soldier );
	void State_Act_Bullet_Heard( SoldierGameObj * soldier, bool is_new_state );
	void State_Act_Gunshot_Heard( SoldierGameObj * soldier );
	void State_Act_Attack( SoldierGameObj * soldier );

	/*
	** Innate AI Actions
	*/
	void Action_Reset( SoldierGameObj * soldier );
	void Action_Face_Location( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, bool crouched = false );
	void Action_Goto_Location( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, float speed = 1.0f, float distance = 1.0f, bool crouched = false );
	void Action_Goto_Location_Facing( SoldierGameObj * soldier, const Vector3 & location, SoldierAIState ai_state, const Vector3 & facing_pos, float speed = 1.0f, float distance = 1.0f, bool crouched = false );
	void Action_Attack_Object( SoldierGameObj * soldier, PhysicalGameObj * enemy, float range, bool kneel, const Vector3 & move_location, float arrived_distance );
	void Action_Dive( SoldierGameObj * soldier, const Vector3 & location );
	void Stay_Within_Home( SoldierGameObj * soldier, Vector3 * location, float * distance );
	bool Take_Cover( SoldierGameObj * soldier, bool force_face = false, const Vector3 & face_pos = Vector3(0,0,0) );
	void Look_Random( SoldierGameObj * soldier, float duration );


	/*
	** Innate AI Cover Positions
	*/
	void	Release_Cover_Position( void );

};

#endif	// SOLDIEROBSERVER_H
