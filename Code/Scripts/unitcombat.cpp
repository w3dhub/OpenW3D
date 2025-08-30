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
 *                     $Archive:: /Commando/Code/Scripts/unitcombat.cpp                       $* 
 *                                                                                             * 
 *                      $Author:: David_y                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 4/27/00 11:44a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scripts.h"
#include "unitcombat.h"

#define STATE_IDLE				5010
	#define STATE_IDLE_MOVEMENT		5011
	#define STATE_IDLE_ANIMATION	5012
#define STATE_SEARCH			5020
#define STATE_OFFENSIVE			5030
#define STATE_DEFENSIVE			5040
#define STATE_SUPPORT			5050
#define STATE_CRITICAL			5060

#define FUNC_CIRCLE_ATTACK		5110
#define FUNC_CROUCH_ATTACK		5120
#define FUNC_STAND_ATTACK		5130
#define FUNC_DODGE_ENEMY		5140
#define FUNC_SUPPRESSIVE_FIRE	5150
#define FUNC_PANIC				5160
#define FUNC_SEARCH_LOCATION	5170
#define FUNC_SEARCH_CIRCLE		5180
#define FUNC_SEARCH_FACING		5190

DECLARE_SCRIPT ( Unit_Combat,"Scoreboard_ID=0:int,Controller_ID=0:int,Script_Override=0:int,Soldier_Type=0:int" )
{

	int state;
	int function;
	int enemy_id;
	int self_id;
	int scoreboard_id;
	int controller_id;
	int script_override;
	const char * anim_script;
	const char * speech_script;
	Vector3 idleposition;
	Vector3 searchposition;
	bool combatspeech;
	int soldier_type;
	// Sodier type is a Script Param that adjusts the UnitCombat behavior
	//		0 - Default
	//		1 - Crouch Attack with Circle for Motion
	//		2 - Crouch Attack Only
	//		3 - Stand in Place and Fire

/*********************************************************************************************** 
** Script Events
************************************************************************************************/

	void Animation_Complete( GameObject * obj, const char * anim ) override
	{
		if ( script_override > 0 )
		{
			GameObject *controller = Commands->Find_Object(controller_id);
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller, OV_ANIMCOMPLETE, 0);
				return;
			}
		}
		else if ( (state == STATE_IDLE_ANIMATION) )
		{
			state = STATE_IDLE;
		}
		else if ( (state == STATE_SEARCH) )
		{
			if ( function == FUNC_PANIC )
			{
				if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
				{
					Sound_Create_Search( obj );
				}
				if ( soldier_type == 3 )
				{
					Function_Search_Facing( obj , searchposition, 0.0f );
					return;
				}
				Function_Search_Location( obj, searchposition , true );
			}
			else if ( function == FUNC_SEARCH_CIRCLE )
			{
				Function_Search_Circle( obj, searchposition, 7.5f );
			}
		}
	}


	void Created(GameObject * obj) override
	{
		anim_script = NULL;
		self_id = Commands->Get_ID( obj );
		idleposition = Commands->Get_Position( obj );
		state = STATE_IDLE;

		float duration;
		duration = Commands->Get_Random( 5, 15 );

		Commands->Start_Timer ( obj, duration, TIMER_IDLE_ANIM );

		if ( script_override > 0 )
		{
			GameObject *controller = Commands->Find_Object(controller_id);
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller, OV_CREATED, 0);
			}
		}
		else
		{
			Commands->Enable_Enemy_Seen( obj, true );
//			Commands->Enable_Sound_Heard( obj, true );
		}

		if ( Get_Int_Parameter(0) )
		{
			scoreboard_id = Get_Int_Parameter(0);
		}
		else
		{
			scoreboard_id = 0;
		}
		if ( Get_Int_Parameter(1) )
		{
			controller_id = Get_Int_Parameter(1);
		}
		else
		{
			controller_id = 0;
		}
		if ( Get_Int_Parameter(2) )
		{
			script_override = Get_Int_Parameter(2);
		}
		else
		{
			script_override = 0;
		}
		if ( Get_Int_Parameter(3) )
		{
			soldier_type = Get_Int_Parameter(3);
		}
		else
		{
			soldier_type = 0;
		}
		if ( soldier_type > 3 )
		{
			soldier_type;
		}
		else
		{
			soldier_type = 0;
		}
	}

	void Custom ( GameObject * obj, int type, int param, GameObject *sender ) override
	{
		if ( type == OV_OVERRIDE )
		{
			script_override = param;
		}
		if ( type == OV_AWARE )
		{
			if ( param == 1 )
			{
				Commands->Enable_Enemy_Seen( obj, true );
//				Commands->Enable_Sound_Heard( Me(), true );
			}
			if ( param == 0 )
			{
				Commands->Enable_Enemy_Seen( obj, false );
//				Commands->Enable_Sound_Heard( Me(), false );
			}
		}
		if ( script_override > 0 )
		{
			if ( type == OV_VERIFY )
			{
				GameObject *controller = Commands->Find_Object(controller_id);
				if ( controller )
				{
					Commands->Send_Custom_Event( obj, controller, OV_CONFIRM, 0);
				}
			}
			return;
		}
		else if ( type == CUSTOM_ALLY_INFO )
		{
			if ( sender )
			{
				if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
				{
					Sound_Create_Defensive_Attack( obj );
				}
				Commands->Send_Custom_Event( obj, sender, CUSTOM_TARGET_INFO, enemy_id);
			}
		}
		else if ( type == CUSTOM_TARGET_INFO )
		{
			GameObject *obj = Commands->Find_Object(param);

			if ( obj )
			{
				if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
				{
					Sound_Create_Attack( obj );
				}
				searchposition = Commands->Get_Position( obj );

				if ( soldier_type == 3 )
				{
					Function_Search_Facing( obj, searchposition, 0.0f );
					return;
				}

				Function_Search_Location( obj, searchposition, true );
			}
		}
	}

	void Damaged( GameObject * obj, GameObject *damager) override
	{
//  Not using score system
//		Score_Hit_Tally( obj );

		Sound_Create_Damaged ( obj );

		if (Commands->Get_Health(obj) < 10.0f)
		{
			State_Change_Critical( obj );

//  Handled by innate animation system.
//			Function_Play_Knockdown_Anim( obj );

		} 
		else
		{
			Commands->Set_Animation( obj,NULL,0);
			if ( script_override > 0 )
			{
				GameObject *controller = Commands->Find_Object(controller_id);
				if ( controller )
				{
					Commands->Send_Custom_Event( obj, controller, OV_DAMAGED, 0);
					return;
				}
			}
			else if ( state < STATE_OFFENSIVE )
			{
				if (Commands->Is_Object_Visible(obj,damager) == false)
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
					{
						Sound_Create_Search( obj );
					}
					searchposition = Commands->Get_Position( damager );

					if ( soldier_type == 3 )
					{
						Function_Search_Facing( obj, searchposition, 0.0f );
						return;
					}

					Function_Search_Location( obj, searchposition, true );
				}
			}
		}
	}

	void Destroyed(GameObject * obj ) override
	{
		if ( script_override > 0 )
		{
			return;
		}

/*
		float random = Commands->Get_Random( 0.0f, 1.0f );
		if ( random <= 0.25f )
		{
			Create_Crate( obj, Commands->Get_Position( obj ), 1 );
		}
*/
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if ( script_override > 0 )
		{
			GameObject *controller = Commands->Find_Object(controller_id);
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object(controller_id), OV_ENEMYSEEN, 0);
			}
			return;
		}
		else if ( state < STATE_OFFENSIVE )
		{
			if ( Commands->Is_Object_Visible( obj, enemy ) == true )
			{
				enemy_id = Commands->Get_ID(enemy);
				if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
				{
					Sound_Create_Enemy_Seen( obj );
				}
//				Commands->Create_Instant_Logical_Sound( (CombatSoundType)(SOUND_ENEMY_SEEN), 20.0, obj, Commands->Get_Position(obj) );
				Commands->Enable_Enemy_Seen( obj, false );

				if ( soldier_type == 3 )
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
					{
						Sound_Create_Attack( obj );
					}

					Function_Stand_Attack( obj, enemy, 5.0f, 100.0f );
				}
				else if ( Commands->Get_Random(0.0f, 1.0f) < 0.2f )
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
					{
						Sound_Create_Attack( obj );
					}
					Function_Circle_Attack( obj, enemy, 10.0f, 100.0f, 15.0f );
				}
				else
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
					{
						Sound_Create_Defensive_Attack( obj );
					}
					Function_Crouch_Attack( obj, enemy, 10.0f, 100.0f, 4.0f );
				}
			}
		}
	}

	void Movement_Complete( GameObject* obj, MovementCompleteReason reason) override
//	void Goto_Failed( GameObject * me, GotoFailedReason reason )
	{
		if ( script_override > 0 )
		{
			return;
		}
		
		GameObject *target = Commands->Find_Object(enemy_id);

		switch(reason)
		{
		case ( MOVEMENT_COMPLETE_ARRIVED ):
			{
				if ( script_override > 0 )
				{
					GameObject *controller = Commands->Find_Object(controller_id);
					if ( controller )
					{
						Commands->Send_Custom_Event( obj, controller, OV_ARRIVED, 0);
						return;
					}
				}
				else if ( (state == STATE_OFFENSIVE) && (function == FUNC_CIRCLE_ATTACK) )
				{
					GameObject * target  = Commands->Find_Object(enemy_id);
					if ( target )
					{
						if ( Commands->Get_Random(0.0f, 1.0f) < 0.2f )
						{
							if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
							{
								Sound_Create_Attack( obj );
							}
							Function_Circle_Attack( obj, target, 10.0f, 100.0f, 15.0f );
						}
						else
						{
							if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
							{
								Sound_Create_Defensive_Attack( obj );
							}
							Function_Crouch_Attack( obj, target, 10.0f, 100.0f, 4.0f );
						}
					}
					else
					{
						State_Change_Idle( obj, idleposition );
					}
				}
				else if ( state == STATE_IDLE_MOVEMENT )
				{
					state = STATE_IDLE;
				}
				else if ( state == STATE_SEARCH )
				{
					if ( function == FUNC_SEARCH_LOCATION )
					{
						Commands->Start_Timer( obj, 10.0, TIMER_SEARCH_LOCATION );
						Function_Search_Circle( obj, searchposition, 10.0f);
					}
					else if ( function == FUNC_SEARCH_CIRCLE )
					{
						Function_Play_Search_Anim( obj );
					}
				}
			}
		case (MOVEMENT_COMPLETE_BAD_START):
			{
			}
		case (MOVEMENT_COMPLETE_BAD_DEST):
			{
				if ( (state==STATE_OFFENSIVE) && (function == FUNC_CIRCLE_ATTACK) )
				{
					if ( target )
					{
						if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
						{
							Sound_Create_Attack( obj );
						}
						Function_Circle_Attack( obj, target, 10.0f, 100.0f, 15.0f);
					}
					else
					{
						State_Change_Idle( obj, idleposition);
					}
				}
				break;
			}
		case (MOVEMENT_COMPLETE_NO_PROGRESS_MADE):
			{
				if ( (state==STATE_OFFENSIVE) && (function == FUNC_CIRCLE_ATTACK) )
				{
					if ( target )
					{
						if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
						{
							Sound_Create_Defensive_Attack( obj );
						}
						Function_Circle_Attack( obj, target, 10.0f, 100.0f, 15.0f);
					}
					else
					{
						State_Change_Idle( obj, idleposition);
					}
				}
				break;
			}
		}
	}

	void Killed(GameObject * obj, GameObject *killer ) override
	{
		GameObject *scoreboard = Commands->Find_Object(scoreboard_id);
		if ( scoreboard )
		{
			Commands->Send_Custom_Event(obj,scoreboard,TALLY,TALLY_KILL);
		}

		Commands->Action_Movement_Stop( obj );
		Commands->Action_Attack_Stop( obj );
		Commands->Create_Sound("Death01",Commands->Get_Position(obj), obj );
//		Commands->Create_Instant_Logical_Sound( (CombatSoundType)(SOUND_DEATH), 30.0, obj, Commands->Get_Position( obj ) );

		if ( killer == Commands->Get_The_Star() )
		{
			float death_random = Commands->Get_Random(0,100);
			if ( death_random < 25.0 )
			{
				Sound_Create_Commando_Taunt(killer);
			}
		}

		GameObject *controller = Commands->Find_Object(controller_id);
		if (controller)
		{
			Commands->Send_Custom_Event( obj, controller, OV_KILLED, 0);
		}
	}
	
	void Sound_Heard(GameObject * obj, const CombatSound &sound)
	{
		if ( script_override > 0 )
		{
//			Commands->Send_Custom_Event( obj, Commands->Find_Object(controller_id), OV_SOUNDHEARD, 0);
			return;
		}

		if ( Commands->Get_Player_Type(sound.Creator) == Commands->Get_Player_Type( obj) )
		{
			if ( sound.Type == SOUND_DEATH )
			{
				if ( Commands->Get_Random( 0.0f, 100.0f) < 50.0f )
				{
					Sound_Create_Support( obj );
				}
				GameObject *commando = Commands->Get_The_Star();
				if ( commando )
				{
					searchposition = Commands->Get_Position( commando );
				}
				Function_Panic( obj );
			}
		}
		else if ( state < STATE_OFFENSIVE )
		{
			if ( sound.Type == SOUND_TYPE_BULLET_HIT )	// WAS WEAPON
			{
				searchposition = sound.Position;
				if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
				{
					Sound_Create_Panic( obj );
				}
				Function_Panic( obj );
			}
			else if ( sound.Type == SOUND_TYPE_GUNSHOT )
			{
				searchposition = sound.Position;
				if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
				{
					Sound_Create_Panic( obj );
				}
				Function_Panic( obj );
			}
			else if(sound.Type == SOUND_TYPE_FOOTSTEPS)
			{
				searchposition = sound.Position;
				if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
				{
					Sound_Create_Search( obj );
				}
				if ( soldier_type == 3 )
				{
					Function_Search_Facing( obj, searchposition, 0.0f );
					return;
				}
				Function_Search_Location( obj, searchposition, false );
			}
			else if (sound.Type == SOUND_TYPE_VEHICLE)
			{
				searchposition = sound.Position;
				if ( Commands->Get_Random( 0.0f, 100.0f) < 25.0f )
				{
					Sound_Create_Search( obj );
				}
				if ( soldier_type == 3 )
				{
					Function_Search_Facing( obj, searchposition, 0.0f );
					return;
				}
				Function_Search_Location( obj, searchposition, false );
			}
			else if ( sound.Type == SOUND_ENEMY_SEEN )
			{
				if ( sound.Creator )
				{
					Commands->Send_Custom_Event( obj, sound.Creator, CUSTOM_ALLY_INFO, self_id);
				}
			}
		}
	}
	
	void Timer_Expired(GameObject * obj, int timer ) override
	{
		if ( script_override > 0 )
		{
			if ( timer == TIMER_IDLE_ANIM )
			{
				float duration;
				duration = Commands->Get_Random( 15, 25);
				Commands->Start_Timer(obj, duration ,TIMER_IDLE_ANIM);
			}
			return;
		}
		if ( timer == TIMER_CROUCH_ATTACK )
		{
			GameObject *target = Commands->Find_Object(enemy_id);
			if ( target )
			{
				if ( Commands->Get_Random(0.0f, 1.0f) < 0.2f )
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
					{
						Sound_Create_Attack( obj );
					}
					Function_Circle_Attack( obj, target, 10.0f, 100.0f, 15.0f );
				}
				else
				{
					if ( Commands->Get_Random( 0.0f, 100.0f) < 10.0f )
					{
						Sound_Create_Defensive_Attack( obj );
					}
					Function_Crouch_Attack( obj, target, 10.0f, 100.0f, 4.0f );
				}
			}
			else
			{
				State_Change_Idle( obj, idleposition );
			}
		}
		else if ( timer == TIMER_COMBAT_SPEECH )
		{
			combatspeech = false;
		}
		else if ( timer == TIMER_SEARCH_LOCATION )
		{
			State_Change_Idle( obj, idleposition );
		}
		else if ( timer == TIMER_IDLE_ANIM )
		{
			float duration;
			duration = Commands->Get_Random( 15, 25);
			Commands->Start_Timer( obj, duration ,TIMER_IDLE_ANIM);
			if ( state == STATE_IDLE )
			{
				Function_Play_Idle_Anim( obj );
			}
		}
	}
	

/*********************************************************************************************** 
** Save / Load Events
************************************************************************************************/

/*
	typedef enum
	{
		DATA_ID_STATE,
		DATA_ID_FUNCTION,
		DATA_ID_ENEMY_ID,
		DATA_ID_SELF_ID,
		DATA_ID_SCOREBOARD_ID,
		DATA_ID_CONTROLLER_ID,
		DATA_ID_SCRIPT_OVERRIDE,
		DATA_ID_ANIM_SCRIPT,
		DATA_ID_SPEECH_SCRIPT,
		DATA_ID_IDLEPOSITION,
		DATA_ID_SEARCHPOSITION,
		DATA_ID_COMBATSPEECH,
		DATA_ID_SOLDIER_TYPE,
	};

	void	Save_Data( ScriptSaver & saver )
	{
		SAVE_BEGIN

		SAVE_DATA( DATA_ID_STATE, state )
		SAVE_DATA( DATA_ID_FUNCTION, function )
		SAVE_DATA( DATA_ID_ENEMY_ID, enemy_id )
		SAVE_DATA( DATA_ID_SELF_ID, self_id )
		SAVE_DATA( DATA_ID_SCOREBOARD_ID, scoreboard_id )
		SAVE_DATA( DATA_ID_CONTROLLER_ID, controller_id )
		SAVE_DATA( DATA_ID_SCRIPT_OVERRIDE, script_override )
		SAVE_DATA( DATA_ID_ANIM_SCRIPT, anim_script )
		SAVE_DATA( DATA_ID_SPEECH_SCRIPT, speech_script)
		SAVE_DATA( DATA_ID_IDLEPOSITION, idleposition )
		SAVE_DATA( DATA_ID_SEARCHPOSITION, searchposition )
		SAVE_DATA( DATA_ID_COMBATSPEECH, combatspeech )
		SAVE_DATA( DATA_ID_SOLDIER_TYPE, soldier_type )

		SAVE_END
	}

	void	Load_Data( ScriptLoader & loader )
	{
		LOAD_BEGIN

		LOAD_DATA( DATA_ID_STATE, state )
		LOAD_DATA( DATA_ID_FUNCTION, function )
		LOAD_DATA( DATA_ID_ENEMY_ID, enemy_id )
		LOAD_DATA( DATA_ID_SELF_ID, self_id )
		LOAD_DATA( DATA_ID_SCOREBOARD_ID, scoreboard_id )
		LOAD_DATA( DATA_ID_CONTROLLER_ID, controller_id )
		LOAD_DATA( DATA_ID_SCRIPT_OVERRIDE, script_override )
		LOAD_DATA( DATA_ID_ANIM_SCRIPT, anim_script )
		LOAD_DATA( DATA_ID_SPEECH_SCRIPT, speech_script)
		LOAD_DATA( DATA_ID_IDLEPOSITION, idleposition )
		LOAD_DATA( DATA_ID_SEARCHPOSITION, searchposition )
		LOAD_DATA( DATA_ID_COMBATSPEECH, combatspeech )
		LOAD_DATA( DATA_ID_SOLDIER_TYPE, soldier_type )

		LOAD_END
	}
*/

/*********************************************************************************************** 
** Functions - SlaveCombat State Change Functions (Replace Old Queen Calls to Custom)
************************************************************************************************/

/*
	void Create_Crate( GameObject * obj, Vector3 position, int powerup_type )
	{
		// powerup_type refers to a random set of powerups
		//		1 - Health
		//		2 - Armor
		//		3 - Weapon
		//		4 - Ammo
		//
		// Only types 1 & 2 are to be used in a normal soldier drop

		position.Z += .25;
		const char * crate;
		float random;
		random = Commands->Get_Random( 0.0f, 5.0f );
		if ( random <= 0.5f )
		{
			crate = "Health_100";
		}
		else if ( random <= 1.0f )
		{
			crate = "Shield_Kevlar_100";
		}
		else if ( random <= 2.33f )
		{
			crate = "Health_50";
		}
		else if ( random <= 3.66f )
		{
			crate = "Shield_Kevlar_25";
		}
		else
		{
			crate = "Health_25";
		}
		Commands->Create_Object( crate, position);
	}
*/

	void Function_Circle_Attack(GameObject * obj, GameObject * Enemy, float Accuracy, float Range , float Radius)
	{
		state = STATE_OFFENSIVE;
		function = FUNC_CIRCLE_ATTACK;
		Commands->Enable_Enemy_Seen( obj, false);
//		Commands->Enable_Sound_Heard( Me, false);
//		Commands->Create_Instant_Logical_Sound( (CombatSoundType)(SOUND_ENEMY_SEEN), 20.0, Me, Commands->Get_Position( Me ) );
		Commands->Action_Movement_Set_Crouch( obj, false);
		double theta_angle = (360/5);
		float temp = Commands->Get_Random(0.0f,1.0f);
		if (temp > 0.4f)
			{
				theta_angle *= -1;
			}

		Vector3 targetlocation = Commands->Get_Position( obj ) - Commands->Get_Position(Enemy);
		targetlocation.Normalize();
		targetlocation *= Radius;
		targetlocation.Rotate_Z( DEG_TO_RADF(theta_angle) );
		targetlocation += Commands->Get_Position( Enemy );
		targetlocation.Z += 0.5;
		Commands->Action_Movement_Set_Forward_Speed( obj, 0.3f );
		Commands->Action_Movement_Goto_Location( obj, targetlocation, 2.0f);

		if ( soldier_type > 0 )
		{
			return;
		}

		Commands->Action_Attack_Object( obj, Enemy, Accuracy , Range );
	}

	void Function_Crouch_Attack(GameObject * obj, GameObject * Enemy, float Accuracy, float Range, float Duration )
	{
		state = STATE_OFFENSIVE;
		function = FUNC_CROUCH_ATTACK;
		Commands->Enable_Enemy_Seen( obj, false);
//		Commands->Enable_Sound_Heard( Me, false);
//		Commands->Create_Instant_Logical_Sound( (CombatSoundType)(SOUND_ENEMY_SEEN), 20.0, Me, Commands->Get_Position( Me ) );
		Commands->Action_Movement_Set_Crouch( obj, true );
		Commands->Start_Timer( obj, Duration, TIMER_CROUCH_ATTACK );
		Commands->Action_Attack_Object( obj, Enemy, Accuracy, Range );
	}

	void Function_Stand_Attack(GameObject * obj, GameObject * Enemy, float Accuracy, float Range )
	{
		state = STATE_OFFENSIVE;
		function = FUNC_STAND_ATTACK;
		Commands->Enable_Enemy_Seen( obj, false);
//		Commands->Enable_Sound_Heard( Me, false);
//		Commands->Create_Instant_Logical_Sound( (CombatSoundType)(SOUND_ENEMY_SEEN), 20.0, Me, Commands->Get_Position( Me ) );
		Commands->Action_Attack_Object( obj, Enemy, Accuracy, Range );
	}

	void Function_Panic(GameObject * obj)
	{
		if ( function != FUNC_PANIC )
		{
			state = STATE_SEARCH;
			function = FUNC_PANIC;
			Commands->Set_Animation( obj, NULL, 0);
			if ( Commands->Get_Random( 0.0f, 1.0f) < 0.5f )
			{
				Commands->Set_Animation( obj, "human.j21c01",0);
			}
		}
	}

	void Function_Play_Idle_Anim(GameObject * obj)
	{
		Commands->Set_Animation( obj, NULL, 0);
		float animnum = Commands->Get_Random(0,90);
		if (animnum <= 10)
		{anim_script = "human.j03c01";}else
		if (animnum <= 20)
		{anim_script = "human.j04c01";}else
		if (animnum <= 30)
		{anim_script = "human.j13c01";}else
		if (animnum <= 40)
		{anim_script = "human.j14c01";}else
		if (animnum <= 50)
		{anim_script = "human.j15c01";}else
		if (animnum <= 60)
		{anim_script = "human.j17c01";}else
		if (animnum <= 70)
		{anim_script = "human.j24c01";}else
		if (animnum <= 80)
		{anim_script = "human.j22c01";}else
		if (animnum <= 90)
		{anim_script = "human.j20c01";}
		state = STATE_IDLE_ANIMATION;
		Commands->Set_Animation( obj, anim_script, 0);
	}

/*
	void Function_Play_Knockdown_Anim( GameObject * obj )
	{
		Commands->Set_Animation( obj, NULL, 0);
		const char * damaged_bone = Commands->Get_Damage_Bone_Name();
		bool direction = Commands->Get_Damage_Bone_Direction();

		if (direction == false)
		{
			if (!strcmp(damaged_bone,"CHEADD"))
			{
				anim_script = "human.635a01";
			}else
			if (!strcmp(damaged_bone,"CTHORAXD"))
			{
				anim_script ="human.621a";
			}else
			if (!strcmp(damaged_bone,"CLHUMERUSD"))
			{
				anim_script ="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRHUMERUSD"))
			{
				anim_script ="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CLRADIUSD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRRADIUSD"))
			{
				anim_script="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CPELVISD"))
			{
				anim_script="human.612a01";
			}else
			if (!strcmp(damaged_bone,"CLFEMURD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRFEMURD"))
			{
				anim_script="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CLTIBIAD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRTIBIAD"))
			{
				anim_script="human.623a01";
			}
		}
		else if (direction == true)
		{
			if (!strcmp(damaged_bone,"CHEADD"))
			{
				anim_script = "human.635a01";
			}else
			if (!strcmp(damaged_bone,"CTHORAXD"))
			{
				anim_script ="human.622a";
			}else
			if (!strcmp(damaged_bone,"CLHUMERUSD"))
			{
				anim_script ="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRHUMERUSD"))
			{
				anim_script="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CLRADIUSD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRRADIUSD"))
			{
				anim_script="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CLFEMURD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRFEMURD"))
			{
				anim_script="human.623a01";
			}else
			if (!strcmp(damaged_bone,"CLTIBIAD"))
			{
				anim_script="human.624a01";
			}else
			if (!strcmp(damaged_bone,"CRTIBIAD"))
			{
				anim_script="human.623a01";
			}
		}
		else
		{
			anim_script ="human.622a";
		}

		Commands->Set_Animation( obj, anim_script, 0);
	}
*/

	void Function_Play_Search_Anim( GameObject * obj )
	{
		Commands->Set_Animation( obj, NULL, 0);
		float animnum = Commands->Get_Random(0,30);
		if ( animnum <= 10 )
		{anim_script = "human.j09c01";}
		else if ( animnum <= 20 )
		{anim_script = "human.j10c01";}
		else if( animnum <= 30 )
		{anim_script = "human.j11c01";}

		Commands->Set_Animation( obj, anim_script, 0);
	}


	void Function_Search_Circle( GameObject * obj, Vector3 position, float Radius )
	{
		state = STATE_SEARCH;
		function = FUNC_SEARCH_CIRCLE;

		Commands->Enable_Enemy_Seen( obj, true);
//		Commands->Enable_Sound_Heard( Me, true);

		Commands->Action_Movement_Set_Crouch( obj, false);
		Commands->Action_Attack_Stop( obj );
		Commands->Action_Movement_Set_Forward_Speed( obj, 0.4f );

		double theta_angle = (360/5);
		float reverse = Commands->Get_Random(0.0f,1.0f);
		if ( reverse < 0.1f)
			{
				theta_angle *= -1;
			}
		Vector3 targetlocation = Commands->Get_Position( obj ) - position;
		targetlocation.Normalize();
		targetlocation *= Radius;
		targetlocation.Rotate_Z(DEG_TO_RADF(theta_angle));
		targetlocation += position;
		targetlocation.Z += 0.5;
		Commands->Action_Movement_Goto_Location( obj, targetlocation, 2.0f);
	}

	void Function_Search_Facing( GameObject * obj, Vector3 position, float Distance )
	{
		state = STATE_SEARCH;
		function = FUNC_SEARCH_FACING;
		Commands->Enable_Enemy_Seen( obj, true);
//		Commands->Enable_Sound_Heard( Me, true);
		Commands->Action_Attack_Location( obj, searchposition, 0, 0 );
	}

	void Function_Search_Location( GameObject * obj, Vector3 position , bool Crouch )
	{
		state = STATE_SEARCH;
		function = FUNC_SEARCH_LOCATION;

		Commands->Enable_Enemy_Seen( obj, true);
//		Commands->Enable_Sound_Heard( Me, true);

		Commands->Action_Movement_Set_Crouch( obj, Crouch );
		Commands->Action_Movement_Set_Forward_Speed( obj, 0.4f );
		Commands->Action_Attack_Location( obj, searchposition, 0, 0 );
		Commands->Action_Movement_Goto_Location( obj, searchposition, 3.0f );
	}

/*
	void Score_Hit_Tally( GameObject * obj )
	{
		const char * damaged_bone = Commands->Get_Damage_Bone_Name();
		bool direction = Commands->Get_Damage_Bone_Direction();

		GameObject *scoreboard = Commands->Find_Object(scoreboard_id);

		if ( scoreboard )
		{
			if ( direction == false )
			{
				Commands->Send_Custom_Event ( obj, scoreboard, HITDIRECTION, HD_FRONTHIT );
			}
			else if ( direction == true )
			{
				Commands->Send_Custom_Event ( obj, scoreboard, HITDIRECTION, HD_BACKHIT );
			}
			
			if (!strcmp(damaged_bone,"CHEADD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_HEAD );
			}
			else if (!strcmp(damaged_bone,"CTHORAXD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_CHEST );
			}
			else if (!strcmp(damaged_bone,"CLHUMERUSD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_LEFTARM );
			}
			else if (!strcmp(damaged_bone,"CRHUMERUSD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_RIGHTARM );
			}
			else if (!strcmp(damaged_bone,"CLRADIUSD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_LEFTARM );
			}
			else if (!strcmp(damaged_bone,"CRRADIUSD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_RIGHTARM );
			}
			else if (!strcmp(damaged_bone,"CPELVISD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_GROIN );
			}
			else if (!strcmp(damaged_bone,"CLFEMURD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_LEFTLEG );
			}
			else if (!strcmp(damaged_bone,"CRFEMURD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_RIGHTLEG );
			}
			else if (!strcmp(damaged_bone,"CLTIBIAD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_LEFTLEG );
			}
			else if (!strcmp(damaged_bone,"CRTIBIAD"))
			{
				Commands->Send_Custom_Event ( obj, scoreboard, BODYLOCATION, BL_RIGHTLEG );
			}
		}
	}
*/

	void Sound_Create_Attack(GameObject * obj)
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );

			float sound_random = Commands->Get_Random( 0.0f , 80.0f);
			if ( sound_random <= 10)
			{ speech_script = "FireAtWill01"; }
			else if ( sound_random <= 20)
			{ speech_script = "LookAlive01"; }
			else if ( sound_random <= 30)
			{ speech_script = "SearchAndDestroy01"; }
			else if ( sound_random <= 40)
			{ speech_script = "TargetHasBeenEngaged01"; }
			else if ( sound_random <= 50)
			{ speech_script = "FireAtWill02"; }
			else if ( sound_random <= 60)
			{ speech_script = "LookAlive02"; }
			else if ( sound_random <= 70)
			{ speech_script = "SearchAndDestroy02"; }
			else if ( sound_random <= 80)
			{ speech_script = "TargetHasBeenEngaged02"; }

			Commands->Create_Sound( speech_script, Commands->Get_Position( obj ), obj );
		}
	}

	void Sound_Create_Commando_Taunt(GameObject * Commando)
	{
		float sound_random = Commands->Get_Random( 0.0 , 60.0 );

		if ( sound_random <=10.0 )
		{ speech_script = "CVoice03KeepEmComin"; }
		else if ( sound_random <=20.0 )
		{ speech_script = "CVoice04HaHa"; }
		else if ( sound_random <=30.0 )
		{ speech_script = "CVoice05LeftHanded"; }
		else if ( sound_random <=40.0 )
		{ speech_script = "CVoice06NoProblem"; }
		else if ( sound_random <=50.0 )
		{ speech_script = "CVoice10TuffGuy"; }
		else
		{ speech_script = "CVoice14Cmon"; }

		Commands->Create_Sound( speech_script,Commands->Get_Position(Commando), Commando );
	}

	void Sound_Create_Damaged( GameObject * obj )
	{
		float sound_random = Commands->Get_Random( 0.0f, 20.0f );
		if ( sound_random <= 10.0f )
		{
			speech_script = "BodyHit01";
		}
		else if ( sound_random <= 20.0 )
		{
			speech_script = "BodyHit02";
		}

		Commands->Create_Sound( speech_script, Commands->Get_Position( obj ), obj);
	}

	void Sound_Create_Defensive_Attack(GameObject * obj)
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );
			float sound_random = Commands->Get_Random( 0.0f , 40.0f);
			if ( sound_random <=10.0 )
			{ speech_script = "HoldYourPositions01"; }
			else if ( sound_random <=20.0 )
			{ speech_script = "KeepYourHeadDown01"; }
			else if ( sound_random <=30.0 )
			{ speech_script = "HoldYourPositions02"; }
			else if ( sound_random <=40.0 )
			{ speech_script = "KeepYourHeadDown02"; }

			Commands->Create_Sound( speech_script,Commands->Get_Position( obj ), obj );
		}
	}

	void Sound_Create_Enemy_Seen(GameObject * obj)
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );
			float sound_random = Commands->Get_Random( 0.0f , 120.0f);
			if ( sound_random <= 10.0 )
			{ speech_script = "ForKane01"; }
			else if ( sound_random <= 20.0 )
			{ speech_script = "GetHim01"; }
			else if ( sound_random <= 30.0 )
			{ speech_script = "Incoming01"; }
			else if ( sound_random <= 40.0 )
			{ speech_script = "KillHim01"; }
			else if ( sound_random <= 50.0 )
			{ speech_script = "SoundAlarm01"; }
			else if ( sound_random <= 60.0 )
			{ speech_script = "ThereHeIs01"; }
			else if ( sound_random <= 70.0 )
			{ speech_script = "ForKane02"; }
			else if ( sound_random <= 80.0 )
			{ speech_script = "GetHim02"; }
			else if ( sound_random <= 90.0 )
			{ speech_script = "Incoming02"; }
			else if ( sound_random <= 100.0 )
			{ speech_script = "KillHim02"; }
			else if ( sound_random <= 110.0 )
			{ speech_script = "SoundAlarm02"; }
			else if ( sound_random <= 120.0 )
			{ speech_script = "ThereHeIs02"; }

			Commands->Create_Sound( speech_script,Commands->Get_Position( obj ), obj );
		}
	}

	void Sound_Create_Panic(GameObject * obj)
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );
			float sound_random = Commands->Get_Random( 0.0f , 20.0f);
			if ( sound_random <= 10.0 )
			{ speech_script = "UhOh01"; }
			else if ( sound_random <= 20.0 )
			{ speech_script = "WatchOut01"; }
			else if ( sound_random <= 30.0 )
			{ speech_script = "UhOh02"; }
			else if ( sound_random <= 40.0 )
			{ speech_script = "WatchOut02"; }

			Commands->Create_Sound( speech_script,Commands->Get_Position( obj ), obj );
		}
	}

	void Sound_Create_Search(GameObject * obj)
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );
			float taunt_random = Commands->Get_Random( 0.0f , 20.0f);
			if ( taunt_random <= 10.0 )
			{ speech_script = "Q_Huh01"; }
			else if ( taunt_random <= 20.0 )
			{ speech_script = "Q_WhatThat01"; }
			else if ( taunt_random <= 30.0 )
			{ speech_script = "Q_Huh02"; }
			else if ( taunt_random <= 40.0 )
			{ speech_script = "Q_WhatThat02"; }

			Commands->Create_Sound( speech_script,Commands->Get_Position( obj ), obj );
		}
	}

	void Sound_Create_Support(GameObject * obj )
	{
		if (combatspeech == false )
		{
			combatspeech = true;
			Commands->Start_Timer( obj, 4.0f, TIMER_COMBAT_SPEECH );
			float taunt_random = Commands->Get_Random( 0.0f , 20.0f);

			if ( taunt_random <= 10.0 )
			{ speech_script = "GonnaDieHere01"; }
			else if ( taunt_random <= 20.0 )
			{ speech_script = "Medic01"; }
			else if ( taunt_random <= 30.0 )
			{ speech_script = "GonnaDieHere02"; }
			else if ( taunt_random <= 40.0 )
			{ speech_script = "Medic02"; }

			Commands->Create_Sound( speech_script,Commands->Get_Position( obj ), obj );
		}
	}

	void State_Change_Critical( GameObject *obj )
	{
		state = STATE_CRITICAL;
		Commands->Action_Movement_Stop( obj );
		Commands->Action_Attack_Stop( obj );
		Commands->Set_Animation( obj, NULL, 0);
		Commands->Enable_Enemy_Seen( obj, false);
//		Commands->Enable_Sound_Heard( Me, false);
	}

	void State_Change_Idle( GameObject * obj, Vector3 position )
	{
		state = STATE_IDLE_MOVEMENT;
		Commands->Action_Attack_Stop( obj );
		Commands->Action_Movement_Set_Crouch( obj, false);
		Commands->Action_Movement_Set_Forward_Speed( obj, 0.1f);
		Commands->Enable_Enemy_Seen( obj, true);
//		Commands->Enable_Sound_Heard( Me, true);
		Commands->Action_Movement_Goto_Location( obj, position , 1.0 );
	}

};

