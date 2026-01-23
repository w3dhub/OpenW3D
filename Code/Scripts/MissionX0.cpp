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

/******************************************************************************
*
* FILE
*     MissionX0.cpp
*
* DESCRIPTION
*     Mission X0 scripts
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Darren_k $
*     $Revision: 25 $
*     $Modtime: 2/08/02 9:15a $
*     $Archive: /Commando/Code/Scripts/MissionX0.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "MissionX0.h"


DECLARE_SCRIPT(MX0_MissionStart_DME, "") //1200001
{
	int engineer1, engineer2, eng_num, count, sniper_1, sniper_2, curr_conv7;
	bool SniperNotify;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( engineer1, 1 );
		SAVE_VARIABLE( engineer2, 2 );
		SAVE_VARIABLE( eng_num, 3 );
		SAVE_VARIABLE( count, 4 );
		SAVE_VARIABLE( sniper_1, 5 );
		SAVE_VARIABLE( sniper_2, 6 );
		SAVE_VARIABLE( curr_conv7, 7 );
		SAVE_VARIABLE( SniperNotify, 8 );
	}

	void Created (GameObject * /* obj */) override
	{
		Commands->Select_Weapon (STAR, NULL );
		engineer1 = engineer2 = eng_num = count = sniper_1 = sniper_2 = curr_conv7 = 0;
		SniperNotify = false;

		Vector3 drop_loc = Commands->Get_Position ( Commands->Find_Object(1200001));
		//float chin_face = Commands->Get_Facing ( Commands->Find_Object(1200001));

		Commands->Fade_Background_Music ( "renegade_intro_no_vox.mp3", 0, 1 );
		GameObject * intro = Commands->Create_Object ( "Invisible_Object", Vector3 (0, 0, 0));
		//Commands->Set_Facing(intro, chin_face);
		Commands->Attach_Script(intro, "Test_Cinematic", "X00_Intro.txt");
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == 100001)
		{
			Commands->Start_Timer(obj, this, 2.0f, INITIAL_CONV);
		}

		if (action_id == 100002)
		{
			Commands->Start_Timer(obj, this, 5.0f, PROMPT_LOOP);
		}

		if (action_id == 100004)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200015), SEND_EM, 0, 0.0f );
		}

	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == STAY_HERE)
		{
			int id = Commands->Create_Conversation("MX0_ENGINEER1_048", 97, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer1), id, false, true, true );			
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100048);
			Commands->Monitor_Conversation(obj, id);
		}

		if (timer_id == CROUCH_WANDER && Commands->Find_Object (sniper_1))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer1), CROUCH_WANDER, sniper_1, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer2), CROUCH_WANDER, sniper_1, 0.0f );

			Commands->Start_Timer(obj, this, 3.0f, CROUCH_WANDER);
		}

		if (timer_id == SNIPER_EXCHANGE)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (MX0_A02_CONTROLLER_ID), MX0_A02_CUSTOM_TYPE_MAIN_STARTUP, 0, 0.0f );
		}

		if (timer_id == SNIPER_CREATE)
		{
			Vector3 sniper1_loc = Commands->Get_Position (Commands->Find_Object (1200043));
			Vector3 sniper2_loc = Commands->Get_Position (Commands->Find_Object (1200044));

			GameObject *sniper1 = Commands->Create_Object ("NOD_Minigunner_2SF", sniper1_loc);
			Commands->Attach_Script(sniper1, "M00_Soldier_Powerup_Disable", "");
			Commands->Attach_Script(sniper1, "M00_Send_Object_ID", "1200001, 1, 0.0f");
			Commands->Attach_Script(sniper1, "MX0_SniperAction", "1200045");
			Commands->Attach_Script(sniper1, "MX0_KillNotify", "");

			sniper_1 = Commands->Get_ID (sniper1);

			GameObject *sniper2 = Commands->Create_Object ("NOD_Minigunner_2SF", sniper2_loc);
			Commands->Attach_Script(sniper2, "M00_Soldier_Powerup_Disable", "");
			Commands->Attach_Script(sniper2, "M00_Send_Object_ID", "1200001, 2, 0.0f");
			Commands->Attach_Script(sniper2, "MX0_SniperAction", "1200045");
			sniper_2 = Commands->Get_ID (sniper2);
		}

		if (timer_id == SNIPER_REMINDER && Commands->Find_Object (sniper_1))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100007, 0, 0.0f );
		}

		if (timer_id == INITIAL_CONV)
		{
			Commands->Send_Custom_Event( obj, obj, 100002, 0, 0.0f );
		}

		if (timer_id == PROMPT_LOOP && count <= 1)
		{
			Commands->Send_Custom_Event( obj, obj, 100003, 0, 0.0f );
			Commands->Start_Timer(obj, this, 5.0f, PROMPT_LOOP);
		}

		if (timer_id == MOUSE_HELP)
		{
			Commands->Set_HUD_Help_Text(MX0_HELPTEXT_01, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}

		if (timer_id == WSAD_HELP)
		{
			Commands->Set_HUD_Help_Text(MX0_HELPTEXT_02, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}

		if (timer_id == F1_HELP)
		{
			Commands->Set_HUD_Help_Text(MX0_HELPTEXT_03, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}

		if (timer_id == EQUIP)
		{
			Commands->Set_Background_Music ( "Level0_pt1_music.mp3" );
			Commands->Give_PowerUp (STAR, "POW_AutoRifle_Player", false);
			Commands->Give_PowerUp (STAR, "POW_AutoRifle_Player", false);
			Commands->Select_Weapon (STAR, "Weapon_AutoRifle_Player" );

			Commands->Attach_Script(STAR, "Havoc_Script", "");
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject *sender) override
	{
		if (type == SNIPER1KILLED)
		{
			if (!SniperNotify)
			{
				int id = Commands->Create_Conversation("MX0CON018", 97, 2000, false);
				Commands->Join_Conversation (Commands->Find_Object (engineer1), id, false, true, true );			
				Commands->Join_Conversation (STAR, id, true, true, true );
				Commands->Start_Conversation(id, 100018);
				Commands->Monitor_Conversation(obj, id);

				Commands->Start_Timer(obj, this, 3.0f, STAY_HERE);
			}

		}

		if (type == CROUCH_WANDER)
		{
			Commands->Start_Timer(obj, this, 2.0f, CROUCH_WANDER);
		}

		if (type == START_SNIPER)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (sniper_1), START_SNIPER, 0, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (sniper_2), START_SNIPER, 0, 0.0f );
		}

		if (type == MX0_A02_CUSTOM_TYPE_GET_SNIPER_01_ID)
		{
			Commands->Send_Custom_Event( obj, sender, type, sniper_1, 0.0f );
		}

		if (type == MX0_A02_CUSTOM_TYPE_GET_SNIPER_02_ID)
		{
			Commands->Send_Custom_Event( obj, sender, type, sniper_2, 0.0f );
		}

		if(type == M00_SEND_OBJECT_ID)
		{
			if(param == 1)
			{
				sniper_1 = Commands->Get_ID(sender);
			}

			if(param == 2)
			{
				sniper_2 = Commands->Get_ID(sender);
				Commands->Start_Timer(obj, this, 2.0f, SNIPER_EXCHANGE);
			}
		}

		if (type == 99)
		{
			count = 0;

//			Vector3 drop_loc = Commands->Get_Position ( Commands->Find_Object(1200001));
//			float chin_face = Commands->Get_Facing ( Commands->Find_Object(1200001));

//			GameObject * chinook = Commands->Create_Object ( "Invisible_Object", drop_loc);
//			Commands->Set_Facing(chinook, chin_face);
//			Commands->Attach_Script(chinook, "Test_Cinematic", "MX0_TroopDrop_2X.txt");


			GameObject * block1 = Commands->Create_Object ( "Large_Blocker", Vector3 (-148.071f,-31.267f,-0.306f));
			Commands->Set_Facing(block1, -30.000f);
			Commands->Set_Is_Rendered( block1, false );
			GameObject * block2 = Commands->Create_Object ( "Large_Blocker", Vector3 (-148.234f,-31.542f,1.357f));
			Commands->Set_Facing(block2, -30.000f);
			Commands->Set_Is_Rendered( block2, false );

			Commands->Start_Timer(obj, this, 0.5f, EQUIP);
			Commands->Start_Timer(obj, this, 3.0f, MOUSE_HELP);
			Commands->Start_Timer(obj, this, 6.0f, WSAD_HELP);
			Commands->Start_Timer(obj, this, 9.0f, F1_HELP);

			Commands->Start_Timer(obj, this, 1.0f, SNIPER_CREATE);

		}

		if (type == ENGINEER_RETURN)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer1), RUN_AWAY, 0, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer2), RUN_AWAY, 0, 0.0f );

		}

		if (type == ENGINEER_CROUCH)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer1), ENGINEER_CROUCH, 0, 0.0f );
		}

		if (type == RUNNING_CONV)
		{
			/*int id = Commands->Create_Conversation("MX0CON005", 95, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer2), id, true, true, true );			
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer2), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100005);
			Commands->Monitor_Conversation(obj, id);*/
		}

		if (type == KILL_SNIPER)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer1), KILL, 0, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer2), KILL, 0, 0.0f );
		}

		if (type == COUNTER)
		{
			count = param;
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer1), COUNT_UP, count, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer2), COUNT_UP, count, 0.0f );
		}

		if (type == SET_ENGINEER)
		{
			if (eng_num == 0)
			{
				engineer1 = param;
				eng_num++;
			}
			else
			{
				engineer2 = param;
			}
		}

		if (type == CHECK_ENGINEER)
		{
			if (param == 1)
			{
				Commands->Send_Custom_Event( obj, sender, RETURN_ENGINEER1, engineer1, 0.0f );
			}

			if (param == 2)
			{
				Commands->Send_Custom_Event( obj, sender, RETURN_ENGINEER2, engineer2, 0.0f );
			}
		}

		if (type == 100001)
		{
			int id = Commands->Create_Conversation("MX0CON001", 99, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );
			Commands->Start_Conversation(id, 100001);
			Commands->Monitor_Conversation(obj, id);
		}

		if (type == 100002)
		{
			int id = Commands->Create_Conversation ("MX0CON002", 97, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer1), id, false, true, true );
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer1), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );			
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer2), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100002);
			Commands->Monitor_Conversation(obj, id);
		}

		if (type == 100003)
		{
			static constexpr const char *conv3[4] = 
				{
					"MX0CON003",
					"MX0CON003alt1",
					"MX0CON003alt2",
					"MX0CON003alt3",
				};
			
			int rand_conv3 = Commands->Get_Random_Int (0, 4);

			int id = Commands->Create_Conversation(conv3[rand_conv3], 97, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );			
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer2), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100003);
			Commands->Monitor_Conversation(obj, id);
		}

		if (type == 100004)
		{
			int id = Commands->Create_Conversation("MX0CON004", 95, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer1), id, true, true, true );			
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer1), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100004);
			Commands->Monitor_Conversation(obj, id);
		
			Commands->Innate_Disable (Commands->Find_Object (engineer1));
			Commands->Innate_Disable (Commands->Find_Object (engineer2));
		}

		if (type == 100005)
		{
			int id = Commands->Create_Conversation("MX0CON005", 97, 2000, false);
			Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );			
			Commands->Join_Conversation_Facing (Commands->Find_Object (engineer2), id, Commands->Get_ID (STAR));
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100005);
			Commands->Monitor_Conversation(obj, id);
		}

		if (type == 100006)
		{
			if (Commands->Find_Object (sniper_1))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (engineer2), SNIPER_ANIM, 0, 0.0f );
			
				Commands->Start_Timer(obj, this, 5.0f, SNIPER_REMINDER);

				int id = Commands->Create_Conversation("MX0CON006", 97, 2000, false);
				Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );			
				Commands->Join_Conversation_Facing (Commands->Find_Object (engineer2), id, Commands->Get_ID (STAR));
				Commands->Join_Conversation (STAR, id, true, true, true );
				Commands->Start_Conversation(id, 100006);
				Commands->Monitor_Conversation(obj, id);

				SniperNotify = true;
			}
		}

		if (type == 100007)
		{
			if (Commands->Find_Object (sniper_1))
			{
				if (curr_conv7 == 0 || curr_conv7 == 2 || curr_conv7 == 4)
				{

					static constexpr const char *conv7[6] = 
						{
							"MX0CON012",
							"MX0CON013",
							"MX0CON014",
							"MX0CON015",
							"MX0CON016",
							"MX0CON017",
						};

					int id = Commands->Create_Conversation(conv7[curr_conv7], 97, 2000, false);
					Commands->Join_Conversation (Commands->Find_Object (engineer1), id, false, true, true );			
					Commands->Join_Conversation (STAR, id, true, true, true );
					Commands->Start_Conversation(id, 100007);
					Commands->Monitor_Conversation(obj, id);
				}

				if (curr_conv7 == 1 || curr_conv7 == 3 || curr_conv7 == 5)
				{

					static constexpr const char *conv7[6] =
						{
							"MX0CON012",
							"MX0CON013",
							"MX0CON014",
							"MX0CON015",
							"MX0CON016",
							"MX0CON017",
						};

					int id = Commands->Create_Conversation(conv7[curr_conv7], 97, 2000, false);
					Commands->Join_Conversation (Commands->Find_Object (engineer2), id, false, true, true );			
					Commands->Join_Conversation (STAR, id, true, true, true );
					Commands->Start_Conversation(id, 100007);
					Commands->Monitor_Conversation(obj, id);
				}

				if (curr_conv7 < 5)
				{
					curr_conv7++;
				}

				if (curr_conv7 >= 5)
				{
					curr_conv7 = 0;
				}

				Commands->Start_Timer(obj, this, 5.0f, SNIPER_REMINDER);
			}
		}
	}
};

DECLARE_SCRIPT (Havoc_Script, "")
{
	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Mission_Complete (false);
	}
};

DECLARE_SCRIPT (MX0_Engineer1, "Damage_multiplier:float")
{
	float current_health, last_health;
	int count, point_id;
	bool doing_anim;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current_health, 1 );
		SAVE_VARIABLE( last_health, 2 );
		SAVE_VARIABLE( count, 3 );
		SAVE_VARIABLE( point_id, 4 );
		SAVE_VARIABLE( doing_anim, 5 );
	}

	void Created (GameObject *obj) override
	{
		doing_anim = false;
		count = 0;
		point_id = 0;
		last_health = Commands->Get_Health (obj);

		Commands->Start_Timer(obj, this, 4.0f, STATIONARY);
		Commands->Start_Timer(obj, this, 4.0f, POINT_ANIM);

		Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), SET_ENGINEER, Commands->Get_ID (obj), 0.0f );
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == RESEND_GOTO)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (point_id)), RUN, 1.0f );

			Commands->Action_Goto (obj, params);
		}

		if (timer_id == STATIONARY)
		{
			Commands->Set_Innate_Is_Stationary (obj, true);
		}

		if (timer_id == POINT_ANIM)
		{
			/*Commands->Set_Facing ( obj, Commands->Get_Facing (Commands->Find_Object (1200006)));
			ActionParamsStruct params;

			params.Set_Basic( this, 98, DOING_ANIMATION );
			params.Set_Animation ("H_A_A0A0_L39", false);
			Commands->Action_Play_Animation (obj, params);*/

			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (1200029)), RUN, 0.8f );

			Commands->Action_Goto (obj, params);
		}

		
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == DOING_ANIMATION)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (1200029)), RUN, 0.8f );

			Commands->Action_Goto (obj, params);		
		}

		if (action_id == HIT_ANIMATION)
		{
			doing_anim = false;			
		}

		if((action_id == ENGINEER_GOTO) && (action_id != Commands->Get_Action_ID(obj)) && (!Commands->Is_Performing_Pathfind_Action(obj)) && (reason == ACTION_COMPLETE_LOW_PRIORITY))
		{
			Commands->Start_Timer (obj, this, 5.0f, RESEND_GOTO);
		}

		if (action_id == ENGINEER_GOTO && count == 3 && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100004, 0, 0.0f );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		float damage;

		current_health = Commands->Get_Health (obj);
		if (current_health == 0) 
		{
			damage = ((last_health - current_health));
		}

		else
		{
			damage = (last_health - current_health);
		}
		float mod_damage = (damage * (Get_Float_Parameter("Damage_multiplier")));
		
		Commands->Set_Health (obj, (last_health - mod_damage));
		last_health = Commands->Get_Health (obj);
		current_health = Commands->Get_Health (obj);

		if (damager == STAR && !doing_anim)
		{
			static constexpr const char* convs[4] =
				{
					"MX0CON008",
					"MX0CON009",
					"MX0CON010",
					"MX0CON011",
				};
			
			int rand_conv = Commands->Get_Random_Int (0, 4);

			int id = Commands->Create_Conversation(convs [rand_conv], 97, 2000, false);
			Commands->Join_Conversation (obj, id, false, true, true );			
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100008);
			Commands->Monitor_Conversation(obj, id);

			doing_anim = true;
			ActionParamsStruct params;
			params.Set_Basic( this, 100, HIT_ANIMATION );
			params.Set_Animation( "H_A_J21C", false );
			Commands->Action_Play_Animation (  obj, params );
			//Vector3 myPosition = Commands->Get_Position ( obj );
			//Commands->Create_Sound ( "M04 PanicGuy 01 Twiddler", myPosition, obj );
		}	
		
	}

	void Animation_Complete(GameObject * /* obj */, const char *anim) override
	{
		if (stricmp(anim, "H_A_J21C") == 0)
		{
			doing_anim = false;			
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == CROUCH_WANDER)
		{
			GameObject * sniper1 = Commands->Find_Object (param);
			
			Commands->Unlock_Soldier_Facing (obj);

			if (sniper1)
			{
				Commands->Lock_Soldier_Facing (obj, sniper1, true );

				int crouch_goto [3];
				crouch_goto [0] = 1200010;
				crouch_goto [1] = 1200022;
				crouch_goto [2] = 1200039;

				int rand_goto = Commands->Get_Random_Int (0, 3);

				ActionParamsStruct params;

				params.Set_Basic( this, 97, CROUCH_GOTO );
				params.Set_Movement( Commands->Find_Object (crouch_goto [rand_goto]), WALK, 0.2f, true );
				params.AttackCrouched = true;
				params.MoveCrouched = true;
				Commands->Action_Goto ( obj, params );
			}
		}

		if (type == RUN_AWAY)
		{
			Commands->Action_Reset ( obj, 100 );

			ActionParamsStruct params;

			params.Set_Basic( this, 97, RUN_AWAY );
			params.Set_Movement( Commands->Find_Object (1200006), RUN, 1.0f, false );
			Commands->Action_Goto ( obj, params );
		}

		if (type == ENGINEER_CROUCH)
		{
			Commands->Lock_Soldier_Facing (obj, STAR, true );

			ActionParamsStruct params;

			params.Set_Basic( this, 97, DOING_ANIMATION );
			params.Set_Animation ("h_a_b0c0", true);
			Commands->Action_Play_Animation (obj, params);
		}

		if (type == KILL)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 95, 1 );
			params.Set_Movement( Commands->Find_Object(1200021), 1.2f, 1.0f, true );
			params.Set_Attack (Commands->Find_Object(1200017), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );
		}

		if (type == COUNT_UP)
		{
			count = param;
		}

		if (type == LEAD)
		{
			point_id = param;

			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (point_id)), RUN, 0.8f );

			Commands->Action_Goto (obj, params);
		}
	}
};

DECLARE_SCRIPT (MX0_Engineer2, "Damage_multiplier:float")
{
	float current_health, last_health;
	int count, point_id, sniper_1;
	bool doing_anim;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current_health, 1 );
		SAVE_VARIABLE( last_health, 2 );
		SAVE_VARIABLE( count, 3 );
		SAVE_VARIABLE( point_id, 4 );
		SAVE_VARIABLE( doing_anim, 5 );
		SAVE_VARIABLE( sniper_1, 6 );
	}

	void Created (GameObject *obj) override
	{
		sniper_1 = 0;
		count = 0;
		point_id = 0;
		last_health = Commands->Get_Health (obj);

		doing_anim = false;

		Commands->Start_Timer(obj, this, 4.0f, STATIONARY);
		Commands->Start_Timer(obj, this, 3.5f, WAVE_ANIM);

		Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), SET_ENGINEER, Commands->Get_ID (obj), 0.0f );
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == DOING_ANIMATION)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (1200026)), RUN, 0.8f );

			Commands->Action_Goto (obj, params);
			
			Commands->Send_Custom_Event( obj, obj, 100005, 0, 0.6f );
		}

		if (action_id == HIT_ANIMATION)
		{
			doing_anim = false;			
		}

		if((action_id == ENGINEER_GOTO) && (action_id != Commands->Get_Action_ID(obj)) && (!Commands->Is_Performing_Pathfind_Action(obj)) && (reason == ACTION_COMPLETE_LOW_PRIORITY || reason == ACTION_COMPLETE_PATH_BAD_DEST))
		{
			Commands->Start_Timer (obj, this, 5.0f, RESEND_GOTO);
		}

		if (action_id == ENGINEER_GOTO && count == 4 && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), ENGINEER_CROUCH, 0, 0.0f );

			Commands->Lock_Soldier_Facing (obj, STAR, true );

			/*Vector3 direction = Commands->Get_Position (STAR) - Commands->Get_Position (obj);
			float angle = RAD_TO_DEGF(WWMath::Atan2(direction.Y,direction.X));
			Commands->Set_Facing ( obj, angle );*/
			ActionParamsStruct params;

			params.Set_Basic( this, 97, DOING_ANIMATION );
			params.Set_Animation ("h_a_b0c0", true);
			Commands->Action_Play_Animation (obj, params);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == CROUCH_GOTO)
		{
			GameObject * sniper1 = Commands->Find_Object (sniper_1);
			
			Commands->Unlock_Soldier_Facing (obj);
			
			if (sniper1)
			{
				Commands->Lock_Soldier_Facing (obj, sniper1, true );

				int crouch_goto [3];
				crouch_goto [0] = 1200011;
				crouch_goto [1] = 1200021;
				crouch_goto [2] = 1200040;

				int rand_goto = Commands->Get_Random_Int (0, 3);

				ActionParamsStruct params;

				params.Set_Basic( this, 97, CROUCH_GOTO );
				params.Set_Movement( Commands->Find_Object (crouch_goto [rand_goto]), WALK, 0.2f, true );
				params.AttackCrouched = true;
				params.MoveCrouched = true;
				Commands->Action_Goto ( obj, params );
			}
		}

		if (timer_id == RESEND_GOTO)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (point_id)), RUN, 1.0f );

			Commands->Action_Goto (obj, params);
		}

		if (timer_id == STATIONARY)
		{
			Commands->Set_Innate_Is_Stationary (obj, true);
		}

		/*if (timer_id == 990 && Commands->Find_Object (1200017))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100007, 0, 0.0f );
		}

		if (timer_id == 989 && Commands->Find_Object (1200017) && count == 5)
		{
			//Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100006, 0, 0.0f );
		}*/

		if (timer_id == WAVE_ANIM)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 98, DOING_ANIMATION );
			params.Set_Animation ("H_A_A0A0_L32", false);
			Commands->Action_Play_Animation (obj, params);
		}		
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		float damage;

		current_health = Commands->Get_Health (obj);
		if (current_health == 0) 
		{
			damage = ((last_health - current_health));
		}

		else
		{
			damage = (last_health - current_health);
		}
		float mod_damage = (damage * (Get_Float_Parameter("Damage_multiplier")));
		
		Commands->Set_Health (obj, (last_health - mod_damage));
		last_health = Commands->Get_Health (obj);
		current_health = Commands->Get_Health (obj);

		if (damager == STAR && !doing_anim)
		{
			static constexpr const char* convs[4] =
				{
					"MX0CON008",
					"MX0CON009",
					"MX0CON010",
					"MX0CON011",
				};
			
			int rand_conv = Commands->Get_Random_Int (0, 4);

			int id = Commands->Create_Conversation(convs [rand_conv], 97, 2000, false);
			Commands->Join_Conversation (obj, id, false, true, true );			
			Commands->Join_Conversation (STAR, id, true, true, true );
			Commands->Start_Conversation(id, 100008);
			Commands->Monitor_Conversation(obj, id);

			doing_anim = true;
			ActionParamsStruct params;
			params.Set_Basic( this, 100, HIT_ANIMATION );
			params.Set_Animation( "H_A_J21C", false );
			Commands->Action_Play_Animation (  obj, params );
			//Vector3 myPosition = Commands->Get_Position ( obj );
			//Commands->Create_Sound ( "M04 PanicGuy 01 Twiddler", myPosition, obj );
		}
		
	}

	void Animation_Complete(GameObject * /* obj */, const char *anim) override
	{
		if (stricmp(anim, "H_A_J21C") == 0)
		{
			doing_anim = false;			
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == CROUCH_WANDER)
		{
			sniper_1 = param;
			Commands->Start_Timer (obj, this, 5.0f, CROUCH_GOTO);
		}

		if (type == RUN_AWAY)
		{
			Commands->Action_Reset ( obj, 100 );

			ActionParamsStruct params;

			params.Set_Basic( this, 97, RUN_AWAY );
			params.Set_Movement( Commands->Find_Object (1200007), RUN, 1.0f, false );
			Commands->Action_Goto ( obj, params );
		}

		if (type == SNIPER_ANIM)
		{
			Vector3 direction = Commands->Get_Position (STAR) - Commands->Get_Position (obj);
			float angle = RAD_TO_DEGF(WWMath::Atan2(direction.Y,direction.X));
			Commands->Set_Facing ( obj, angle );

			Commands->Action_Reset(obj, 100);
			Commands->Set_Animation (obj, "H_A_A0A0_L53", false, NULL, 0.0F, -1.0F, true );

			/*ActionParamsStruct params;
			params.Set_Basic( this, 98, DOING_ANIMATION );
			params.Set_Animation ("H_A_A0A0_L53", false);
			Commands->Action_Play_Animation (obj, params);*/
		}

		if (type == KILL)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 95, 1 );
			params.Set_Movement( Commands->Find_Object(1200022), 1.2f, 1.0f, true );
			params.Set_Attack (Commands->Find_Object(1200017), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );
		}

		if (type == COUNT_UP)
		{
			count = param;
		}

		if (type == LEAD)
		{
			point_id = param;

			ActionParamsStruct params;

			params.Set_Basic( this, 96, ENGINEER_GOTO );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (point_id)), RUN, 0.8f );

			Commands->Action_Goto (obj, params);
		}
	}
};

DECLARE_SCRIPT(MX0_Engineer_Return, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			already_entered = true;
			
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), ENGINEER_RETURN, 1, 0.0f );
		}
	}
};


DECLARE_SCRIPT(MX0_Engineer_Goto, "GotoDest1:int, GotoDest2:int, Count:int")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == RETURN_ENGINEER1)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (param), LEAD, Get_Int_Parameter("GotoDest1"), 0.0f );
		}

		if (type == RETURN_ENGINEER2)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (param), LEAD, Get_Int_Parameter("GotoDest2"), 0.0f );
		}
	}
	
	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			already_entered = true;

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), COUNTER, Get_Int_Parameter("Count"), 0.0f );

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), CHECK_ENGINEER, 1, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), CHECK_ENGINEER, 2, 0.0f );

			/*GameObject * engineer = Commands->Find_Object (Get_Int_Parameter("GotoObject"));

			Commands->Send_Custom_Event( obj, engineer, LEAD, Get_Int_Parameter("GotoDest"), 0.0f );*/
		}
	}
};

DECLARE_SCRIPT(MX0_Engineer_Goto2, "GotoDest1:int, GotoDest2:int, Count:int")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == RETURN_ENGINEER1)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (param), LEAD, Get_Int_Parameter("GotoDest1"), 0.0f );
		}

		if (type == RETURN_ENGINEER2)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (param), LEAD, Get_Int_Parameter("GotoDest2"), 0.0f );
		}

		if (type == SEND_EM)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), RUNNING_CONV, 1, 0.0f );

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), COUNTER, Get_Int_Parameter("Count"), 0.0f );

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), CHECK_ENGINEER, 1, 0.0f );
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), CHECK_ENGINEER, 2, 0.0f );
		}

	}

	void Timer_Expired (GameObject* /* obj */, int timer_id) override
	{
		if (timer_id == FIRE_HELP)
		{
			Commands->Set_HUD_Help_Text(MX0_HELPTEXT_04, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}
	}
	
	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			if (Get_Int_Parameter("Count") != 5)
			{
				already_entered = true;

				//Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100004, 1, 0.0f );
			}

			if (Get_Int_Parameter("Count") == 5)
			{
				already_entered = true;

				Commands->Start_Timer(obj, this, 3.0f, FIRE_HELP);
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), CROUCH_WANDER, 0, 0.0f );

				Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), 100006, 1, 0.0f );
			}
		}
	}
};

DECLARE_SCRIPT(MX0_Kill_Sniper, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			already_entered = true;

			if (Commands->Find_Object (1200017))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), KILL_SNIPER, 0, 0.0f );
			}
		}
	}
};


DECLARE_SCRIPT(MX0_NOD_TroopDrop, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Timer_Expired (GameObject* /* obj */, int timer_id) override
	{
		if (timer_id == TROOP_DROP)
		{
			Vector3 drop_loc = Commands->Get_Position ( Commands->Find_Object(1200018));
			float chin_face = Commands->Get_Facing ( Commands->Find_Object(1200018));

			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", drop_loc);
			Commands->Set_Facing(chinook_obj, chin_face);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "MX0_C130Troopdrop.txt");

			drop_loc = Commands->Get_Position ( Commands->Find_Object(1200020));
			chin_face = Commands->Get_Facing ( Commands->Find_Object(1200020));

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", drop_loc);
			Commands->Set_Facing(chinook_obj2, chin_face);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_C130Troopdrop2.txt");
		}
	}

	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			already_entered = true;

			Vector3 drop_loc = Commands->Get_Position ( Commands->Find_Object(1200018));
			float chin_face = Commands->Get_Facing ( Commands->Find_Object(1200018));

			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", drop_loc);
			Commands->Set_Facing(chinook_obj, chin_face);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "MX0_C130Troopdrop.txt");

			drop_loc = Commands->Get_Position ( Commands->Find_Object(1200020));
			chin_face = Commands->Get_Facing ( Commands->Find_Object(1200020));

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", drop_loc);
			Commands->Set_Facing(chinook_obj2, chin_face);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_C130Troopdrop2.txt");

			Commands->Start_Timer(obj, this, 3.0f, TROOP_DROP);

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), START_SNIPER, 0, 0.0f );
		}
	}
};

DECLARE_SCRIPT (MX0_AmbientBattle, "")
{
	int battle_sound;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( battle_sound, 1 );
	}

	void Created (GameObject *obj) override
	{
		Timer_Expired (obj, AMB_SOUND);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == AMB_SOUND)
		{
			battle_sound = Commands->Create_Sound ( "MX0_AmbientBattle_DME", Commands->Get_Position(obj), obj );
			Commands->Monitor_Sound (obj, battle_sound);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == battle_sound)
		{
			Commands->Start_Timer(obj, this, 3.0f, AMB_SOUND);
		}
	}
};

DECLARE_SCRIPT (MX0_SniperAction, "FaceObj:int")
{
	float current_health, last_health;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current_health, 1 );
		SAVE_VARIABLE( last_health, 2 );
	}

	void Created (GameObject *obj) override
	{
		last_health = Commands->Get_Health (obj);

		Commands->Innate_Disable (obj);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == START_SNIPER)
		{
			Commands->Start_Timer(obj, this, 5.0f, SNIPE);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);

		if ( damager == STAR )
		{
			Commands->Apply_Damage( obj, 10000.0f, "Blamokiller", STAR );
			
			bool reveal = Commands->Reveal_Encyclopedia_Character( 41 );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}

		}
		else
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == SNIPE)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 99, 1 );
			params.Set_Movement( obj, 0.0, 5.0, true );
			params.Set_Attack (Commands->Find_Object(Get_Int_Parameter("FaceObj")), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}
};

DECLARE_SCRIPT (MX0_KillNotify, "")
{
	bool hit;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( hit, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		hit = false;
	}

	void Damaged (GameObject * /* obj */, GameObject *damager, float /* amount */) override
	{
		if (damager == STAR)
		{
			hit = true;
		}
	}


	void Custom (GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == START_SNIPER)
		{
			Commands->Start_Timer(obj, this, 3.0f, SNIPE_CONV);
			Commands->Start_Timer(obj, this, 7.0f, SNIPE_CONV2);
		}
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == SNIPE_CONV)
		{
			int id = Commands->Create_Conversation("MX0CON020", 95, 2000, false);
			Commands->Join_Conversation (obj, id, true, true, true );			
			Commands->Start_Conversation(id, 100020);
			Commands->Monitor_Conversation(obj, id);			
		}

		if (timer_id == SNIPE_CONV2)
		{
			int id = Commands->Create_Conversation("MX0CON021", 99, 2000, false);
			Commands->Join_Conversation (obj, id, true, true, true );			
			Commands->Start_Conversation(id, 100021);
			Commands->Monitor_Conversation(obj, id);

			Commands->Start_Timer(obj, this, 5.0f, SNIPE_CONV_DONE);
		}

		if (timer_id == SNIPE_CONV_DONE)
		{
			if (hit)
			{
				static constexpr const char* snipe_conv[6] =
					{
						"Mx0_NODSNIPER_Alt01",
						"Mx0_NODSNIPER_Alt02",
						"Mx0_NODSNIPER_Alt03",
						"Mx0_NODSNIPER_Alt04",
						"Mx0_NODSNIPER_Alt05",
						"Mx0_NODSNIPER_Alt06",
					};

				int rand_conv_snipe = Commands->Get_Random_Int (0, 6);

				int id = Commands->Create_Conversation(snipe_conv[rand_conv_snipe], 95, 2000, false);
				Commands->Join_Conversation (obj, id, true, true, true );			
				Commands->Start_Conversation(id, 100021);
				Commands->Monitor_Conversation(obj, id);

				Commands->Start_Timer(obj, this, 5.0f, SNIPE_CONV_DONE);
			}
		}
	}

	void Killed (GameObject *obj, GameObject *killer) override
	{
		if (killer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1200001), SNIPER1KILLED, 0, 0.0f );
		}
	}
};


DECLARE_SCRIPT(DAK_MX0_Sec_3_Humvee, "" )
{
	void Entered ( GameObject * /* obj */, GameObject * /* enterer */ ) override
	{
		ActionParamsStruct params;
		params.Set_Basic( this, 100, 100 ); //MX0_HUMVEE_WAYPATH_01_DAK
		params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
		params.WaypathID = 1400037;
		params.WaypointStartID = 1400038;

		GameObject *humvee = Commands->Find_Object( 1400035 ); // gets pointer to section 3 Humvee
		Commands->Action_Goto( humvee, params );
		Commands->Debug_Message( "***** DAK_MX0_Sec_3_Humvee:Entered code exectued.\n" );
	}
};

DECLARE_SCRIPT(MX0_A03_NOD_PLACED_MINIGUNNER, "" )
{
	void Created( GameObject *obj ) override
	{
		Vector3 home_pos = Commands->Get_Position( obj );
		Commands->Set_Innate_Soldier_Home_Location( obj, home_pos, 5);
		// disable innate
		Commands->Innate_Disable( obj );
		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if (type == MX0_A03_CUSTOM_NOD_PLACED_MINIGUNNER_INNATE_ENABLE )
		{
			Commands->Innate_Enable ( obj );
			Commands->Set_Innate_Is_Stationary(obj, false);
		}
	}
};

DECLARE_SCRIPT(MX0_GDI_ORCA, "" )
{
	int Trooper_One_Id;
	bool first_time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(first_time, 1);
	}

	void Created (GameObject * /* obj */) override
	{
		first_time = true;
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if ( action_id == 1 )
			{
				// start orca Cinematic.
				GameObject * orca = Commands->Create_Object ( "Invisible_Object", Vector3 (0, 0, 0));
				Commands->Attach_Script ( orca, "Test_Cinematic", "X0F_Harvester.txt" );
				// send custom to self to fire off orca "spotted harvester" conversation
				Commands->Send_Custom_Event( obj, obj, 1, 0, 1.5 );
				// send custom to harvester to damage self ( 1st time )
				GameObject *harvester = Commands->Find_Object( MX0_A03_NOD_HARVESTER_ID );
				Commands->Send_Custom_Event( obj, harvester, MX0_A03_CUSTOM_NOD_HARVESTER_DMG_SELF, 4, (1 + 198/30) );
				// send custom to harvester to damage self ( 2nd time )
				Commands->Send_Custom_Event( obj, harvester, MX0_A03_CUSTOM_NOD_HARVESTER_DMG_SELF, 3, (1 + 260/30) );
				// send custom to harvester to damage self ( 3rd time )
				Commands->Send_Custom_Event( obj, harvester, MX0_A03_CUSTOM_NOD_HARVESTER_DMG_SELF, 3, (1 + 272/30) );
				// send custom to self to fire off orca "finished and departing" conversation
				Commands->Send_Custom_Event( obj, obj, 2, 0, 9.5 );
			}
		}
	}

	void Entered ( GameObject *obj, GameObject * /* enterer */ ) override
	{
		if (first_time)
		{
			// start section 3 music.
			Commands->Fade_Background_Music ( "Level 0 Tiberium.mp3", 2, 2 );

			// "Eagle Base, we must be close. There's a NOD Harvester here..."
			const char *conv_name = ("MX0_A03_02");
			int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
			Commands->Join_Conversation(NULL, conv_id, true, true, true);
			Commands->Start_Conversation (conv_id, 1);
			Commands->Monitor_Conversation (obj, conv_id);

			// bring in GDI Medium Tank
			GameObject *Tank_Drop = Commands->Find_Object ( MX0_A03_GDI_TANK_DROP_ID );
			if ( Tank_Drop )
			{
				// Commands->Debug_Message( "***** DAK ***** sending custom, dropping Medium tank\n" );
				Commands->Attach_Script ( Tank_Drop, "Test_Cinematic", "XG_A03_Tank_Drop.txt" );
			}

			first_time = false;
		}
	}

	void Custom( GameObject *obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == 1 )
		{
			// "Orca6: "Orca 6 to Eagle Base. I have visual on the harvester. Starting my run, now."
			const char *conv_name = ("MX0_A03_03");
			int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
			Commands->Join_Conversation(NULL, conv_id, true, true, true);
			Commands->Start_Conversation (conv_id, 1);
		}

		if ( type == 2 )
		{
			// get Trooper 1
			GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller , M00_SEND_OBJECT_ID, 12, 0 );
			}
			GameObject *Trooper_One = Commands->Find_Object( Trooper_One_Id );

			// "Trooper1: Woah. "That Rocked!" Orca6: "This is Orca 6 -- bingo Fuel. Returning to Base..."
			int conv_id = Commands->Create_Conversation ( "MX0_A03_04" , 0, 0, true);
			Commands->Join_Conversation(Trooper_One, conv_id, false, false, true);
			Commands->Join_Conversation(NULL, conv_id, true, true, true);
			Commands->Start_Conversation (conv_id, 2);

			// start Trooper1 attacking Buggie.
			if (Trooper_One)
			{
				Commands->Send_Custom_Event( obj, Trooper_One, 5, 0, 0 );
			}

			// send custom to trooper1 to fire off "Blast that harvester for us sir!" after 10 seconds.
			if (Trooper_One)
			{
				Commands->Send_Custom_Event( obj, Trooper_One, 3, 0, 10 );
			}
		}

		if ( type == M00_SEND_OBJECT_ID )
		{
			Trooper_One_Id  = param;
		}
	}
};

DECLARE_SCRIPT(MX0_NOD_INFANTRY, "troop_num:int")
{
	int troop_num;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(troop_num, 1);
	}

	void Created ( GameObject *obj ) override
	{
		troop_num = Get_Int_Parameter( "troop_num" );

		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_NOD_LEDGE_CREATED, 0, 0);
		}

		if (troop_num == 1 )
		{
			Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,15,0"); // sending id to A03 Controller
		}
		else
		{
			Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,17,0"); // sending id to A03 Controller
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_TroopDrop") == 0)
		{
			Commands->Send_Custom_Event( obj, obj, 1, 0, 0.33f );
		}
	}

	void Action_Complete( GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if ( action_id == 0 )
		{
			Commands->Set_Innate_Is_Stationary( obj, true );
		}
	}

	void Custom ( GameObject *obj, int type, intptr_t /* param */, GameObject *target ) override
	{
		if (type == 0) // GDI troop is attacking you! attack back!
		{
			if ( target )
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.Set_Attack(target, 100.0f, 0.75f, true);
				Commands->Action_Attack( obj, params );
			}
		}
		if (type == 1) // you've landed! move to your position.
		{
			
			Commands->Action_Reset( obj, 100 );
			if (troop_num == 1 ) // move to left pos.
			{	
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 0);
				// Commands->Debug_Message( "***** DAK ***** Setting NOD Ledge Troop 1 to move to left position.\n" );
				params.Set_Movement( Commands->Find_Object( MX0_A03_NOD_LEDGE_LEFT_LOC_ID ), RUN, 0.25f );
				Commands->Action_Goto (obj, params);

			}
			else // move to right pos.
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 0);
				// Commands->Debug_Message( "***** DAK ***** Setting NOD Ledge Troop 2 to move to right position.\n" );
				params.Set_Movement( Commands->Find_Object( MX0_A03_NOD_LEDGE_RIGHT_LOC_ID ), RUN, 0.25f );
				Commands->Action_Goto (obj, params);
			}
		}
	}

	void Killed ( GameObject *obj, GameObject * /* killer */ ) override
	{
		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_NOD_LEDGE_KILLED, troop_num, 2);
		}
	}
};

DECLARE_SCRIPT(MX0_A03_GDI_INFANTRY, "troop_num:int" )
{
	int troop_num;
	int Current_Target;

		REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(troop_num, 1);
		SAVE_VARIABLE(Current_Target, 2);
	}

	void Created ( GameObject *obj ) override
	{

		troop_num = Get_Int_Parameter( "troop_num" );
		
		// disable inate
		// Commands->Innate_Disable (obj);
		Commands->Attach_Script( obj, "M00_Damage_Modifier_DME", "0.15,0,1,1,1"); // Setting Damage Modifier.

		if ( troop_num == 1 )
		{
			// first Drop Troop stuff. GDI Minigunner Officer
	
			// send id to controller.
			Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,3,0"); // sending first Drop Troop id to A03 Controller

			// move to your Position
			Commands->Action_Reset( obj, 100 );
			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 0 );
			params.Set_Movement( Commands->Find_Object( MX0_A03_DROP_TROOP_ONE_LOC_ID ), RUN, 0.25f );
			Commands->Action_Goto (obj, params);
		}

		if ( troop_num == 2 )
		{
			// second Drop Troop stuff. GDI Minigunner
			
			// send id to controller.
			Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,5,0"); // sending second Drop Troop id to A03 Controller.

			// move to your Position
			Commands->Action_Reset( obj, 100 );
			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 0 );
			params.Set_Movement( Commands->Find_Object( MX0_A03_DROP_TROOP_TWO_LOC_ID ), RUN, 0.25f );
			Commands->Action_Goto (obj, params);
		}

		if ( troop_num == 3 )
		{
			// third Drop Troop stuff. GDI Rocket Soldier
		
			// send id to controller.
			Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,7,0"); // sending third Drop Troop id to A03 Controller.

			// set target
			Current_Target = MX0_A03_NOD_HARVESTER_ID;

			// move to your Position
			Commands->Action_Reset( obj, 100 );
			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 1 );
			params.Set_Movement( Commands->Find_Object( MX0_A03_DROP_TROOP_THREE_LOC_ID ), RUN, 0.25f );
			Commands->Action_Goto (obj, params);
		}
	}
	void Action_Complete( GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */) override
	{
		if (action_id == 0 ) // first movement finished. begin attacking.
		{
			Commands->Set_Innate_Is_Stationary( obj, true );
		}
		
		if (action_id == 1) // Rocket Launcher is finished moving. begin attacking
		{
			Commands->Set_Innate_Is_Stationary( obj, true );
			Commands->Send_Custom_Event( obj, obj, 0, 0, 0 );
		}

		if (action_id == 2) // Rocket Launcher finished attacking harvester. attack buggie.
		{
			Current_Target = MX0_A03_NOD_BUGGIE_ID;
			Commands->Send_Custom_Event( obj, obj, 0, 0, 0 );
		}
	}

	void Custom ( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == 0 ) // Rocket troop attack
		{			
			GameObject *target = Commands->Find_Object( Current_Target );

			if ( target ) 
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 2);
				params.Set_Attack(target, 60.0f, 0.25f, true);
				Commands->Action_Attack(obj, params);
			}
		}
	}
};

DECLARE_SCRIPT(MX0_A03_GDI_TROOP_DROP_ZONE_DAK, "" )
{
	bool First_Time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(First_Time, 1);
	}

	void Created ( GameObject * /* obj */) override
	{
		First_Time = true;
	}

	void Entered ( GameObject * /* obj */, GameObject * /* enterer */ ) override
	{
		if ( First_Time )
		{
			GameObject *Troop_Drop =  Commands->Find_Object ( MX0_A03_GDI_TROOP_DROP_ID );
			if ( Troop_Drop )
			{
				// Commands->Debug_Message( "***** sending custom, dropping 3 infantry\n" );
				Commands->Attach_Script ( Troop_Drop, "Test_Cinematic", "MX0_A03_GDI_TroopDrop.txt" );
			}
			First_Time = false;
		}
	}
};

DECLARE_SCRIPT(MX0_A03_GDI_TANK_DROP_ZONE_DAK, "" )
{
	bool First_Time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(First_Time, 1);
	}

	void Created ( GameObject * /* obj */) override
	{
		First_Time = true;
	}

	void Entered ( GameObject * /* obj */, GameObject * /* enterer */ ) override
	{
		if ( First_Time )
		{
			// Fly in Med Tank and attack turrets
			GameObject *Tank_Drop = Commands->Find_Object ( MX0_A03_GDI_TANK_DROP_ID );
			if ( Tank_Drop )
			{
				// Commands->Debug_Message( "***** sending custom, dropping Medium tank\n" );
				Commands->Attach_Script ( Tank_Drop, "Test_Cinematic", "XG_A03_Tank_Drop.txt" );
			}
			First_Time = false;
		}
	}
};

DECLARE_SCRIPT (MX0_A03_CONTROLLER_DAK, "" )
{
	int Humvee_Id;
	int Trooper_One_Id;
	int Havoc_Tank_Id;
	int GDI_Drop_Tank_Id;
	int GDI_Drop_Troop_One_Id;
	int GDI_Drop_Troop_Two_Id;
	int GDI_Drop_Troop_Three_Id;
	int num_NOD_Turret;
	int num_Ledge_Troops;
	int Nod_Ledge_One_Id;
	int Nod_Ledge_Two_Id;
	int target;
	bool harvester_Dead;
	bool buggie_Dead;
	bool zone_not_finished;
	int num_drops;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Humvee_Id, 1);
		SAVE_VARIABLE(GDI_Drop_Troop_One_Id, 2);
		SAVE_VARIABLE(GDI_Drop_Troop_Two_Id, 3);
		SAVE_VARIABLE(GDI_Drop_Troop_Three_Id, 4);
		SAVE_VARIABLE(GDI_Drop_Tank_Id, 5);
		SAVE_VARIABLE(Trooper_One_Id, 6);
		SAVE_VARIABLE(Havoc_Tank_Id, 7);
		SAVE_VARIABLE(num_NOD_Turret, 8);
		SAVE_VARIABLE(num_Ledge_Troops, 9);
		SAVE_VARIABLE(Nod_Ledge_One_Id, 10);
		SAVE_VARIABLE(Nod_Ledge_Two_Id, 11);
		SAVE_VARIABLE(target, 12);
		SAVE_VARIABLE(harvester_Dead, 14);
		SAVE_VARIABLE(num_drops, 15);
		SAVE_VARIABLE(buggie_Dead, 16);
		SAVE_VARIABLE(zone_not_finished, 17);
	}

	void Created( GameObject * /* obj */ ) override
	{
		num_drops = 1;
		harvester_Dead = false;
		buggie_Dead = false;
		num_NOD_Turret = 0;
		num_Ledge_Troops = 0;
		zone_not_finished = true;
	}

	void Custom ( GameObject *obj, int type, intptr_t param, GameObject *sender ) override
	{
		if ( type == 0 )
		{
			// check to see if both buggie and harvester are dead.
			if ( buggie_Dead && harvester_Dead )
			{
				if ( zone_not_finished )
				{
					Commands->Send_Custom_Event( obj, obj, MX0_A03_CUSTOM_TYPE_END_ZONE, 0, 0);
					GameObject *end_zone = Commands->Find_Object (MX0_A03_END_ZONE_ID );
					Commands->Send_Custom_Event( obj, end_zone, MX0_A03_CUSTOM_LOCK_END_ZONE, 0, 0 );
				}
			}
		}

		if ( type == MX0_A03_CUSTOM_NOD_TURRET_CREATED )
		{
			num_NOD_Turret = num_NOD_Turret + 1;
		}

		if ( type == MX0_A03_CUSTOM_NOD_TURRET_DESTROYED )
		{
			num_NOD_Turret = num_NOD_Turret - 1;

			if ( num_NOD_Turret == 0 )
			{

			}
		}

		if ( type == MX0_A03_CUSTOM_NOD_LEDGE_CREATED )
		{
			num_Ledge_Troops = num_Ledge_Troops + 1;
		}

		if ( type == MX0_A03_CUSTOM_NOD_LEDGE_KILLED )
		{
			num_Ledge_Troops = num_Ledge_Troops - 1;

			if ( num_Ledge_Troops == 0 && num_drops < 5 && !harvester_Dead ) // all ledge troops have been killed. bring in some more
			{
				GameObject * minigunners = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position( Commands->Find_Object ( MX0_A03_NOD_LEDGE_DROP_LOC_ID )));
				Commands->Attach_Script ( minigunners, "Test_Cinematic", "MX0_A03_NOD_LedgeDrop.txt" );
				num_drops++;
			}
		}

		if ( type == MX0_A03_CUSTOM_TYPE_END_ZONE )
		{
			zone_not_finished = false;

			// Additional GDI troop drop
			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(Commands->Find_Object(1500051)));
			Commands->Set_Facing(chinook_obj2, 0.0f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_GDI_TroopDrop_Area4.txt");

			// Move Drop Tank up to section 4
			GameObject *tank = Commands->Find_Object( GDI_Drop_Tank_Id );
			if ( tank )
			{
				Commands->Action_Reset( tank, 100 );

				ActionParamsStruct params;
				params.Set_Basic ( this, 91, 1 );
				params.Set_Movement( Vector3(0,0,0), 1.0f, 1 );
				params.WaypathID = MX0_A03_WAYPATH_TANK_ID;
				Commands->Action_Goto (tank, params);
			}

			// Move Humvee up to Section 4
			GameObject *humvee = Commands->Find_Object( Humvee_Id );
			if (humvee)
			{
				Commands->Send_Custom_Event( obj, humvee, 3, 0, 1.25f);
			}

			// Move Trooper 1 up to Seciton 4
			GameObject *trooper_one = Commands->Find_Object( Trooper_One_Id );
			if ( trooper_one )
			{
				Commands->Send_Custom_Event( obj, trooper_one, 2, 0, 0);
				//Commands->Debug_Message( "***** DAK ***** custom sent to trooper 1.\n" );
			}
		}


		if ( type == MX0_A03_CUSTOM_TYPE_START_ZONE )
		{
			Commands->Scale_AI_Awareness( 2, 1 );

			// create humvee
			GameObject *humvee =  Commands->Find_Object ( MX0_A03_HUMVEE_DROP_ID );
			if ( humvee )
			{
				Commands->Debug_Message( "***** sending custom, dropping humvee, attaching script\n" );
				Commands->Attach_Script ( humvee, "Test_Cinematic", "XG_A03_HumveeDrop_B.txt" );
			}
			// begin moving NOD Buggie
			GameObject *buggie = Commands->Find_Object ( MX0_A03_NOD_BUGGIE_ID );
			if ( buggie )
			{
				Commands->Send_Custom_Event( obj, buggie, 1, 0, 0 );
			}

			// attach my Trooper1 script to Trooper1 passed from rich.
			GameObject *trooper_one = Commands->Find_Object( Trooper_One_Id );
			if (trooper_one)
			{
				Commands->Attach_Script( trooper_one, "MX0_A03_GDI_TROOPER_ONE", "" );
				// move trooper1 up to first A03 position.
				Commands->Send_Custom_Event( obj, trooper_one, 1, 0, 2);
			}

			// enable placed minigunners

			GameObject *minigunner_one = Commands->Find_Object ( MX0_A03_NOD_PLACED_MINIGUNNER_ONE_ID);
			if (minigunner_one)
			{
				Commands->Send_Custom_Event( obj, minigunner_one, MX0_A03_CUSTOM_NOD_PLACED_MINIGUNNER_INNATE_ENABLE, 0, 0);
			}

			GameObject *minigunner_two = Commands->Find_Object ( MX0_A03_NOD_PLACED_MINIGUNNER_TWO_ID);
			if (minigunner_two)
			{
				Commands->Send_Custom_Event( obj, minigunner_two, MX0_A03_CUSTOM_NOD_PLACED_MINIGUNNER_INNATE_ENABLE, 0, 0);
			}
		}
		if ( type == MX0_A03_CUSTOM_BUGGIE_DEAD )
		{
			buggie_Dead = true;

/*			if ( zone_not_finished )
			{
				GameObject *Trooper_One = Commands->Find_Object( Trooper_One_Id );

				int rand = Commands->Get_Random_Int( 1, 2 );
				if ( rand == 1 )
				{
					// Trooper1: "Standing in Tiberium...Not Smart.
					int conv_id = Commands->Create_Conversation ( "MX0_A03_06" , 0, 0, true);
					Commands->Join_Conversation( Trooper_One, conv_id, false, false, true);
					Commands->Start_Conversation (conv_id, 0);
				}
				else
				{
					// Trooper1: "What was he thinking? Standing in a Tiberium field!"
					int conv_id = Commands->Create_Conversation ( "MX0_A03_07" , 0, 0, true);
					Commands->Join_Conversation( Trooper_One, conv_id, false, false, true);
					Commands->Start_Conversation (conv_id, 0);
				}
			}
*/
			// send custom to controller to see if END zone.
			Commands->Send_Custom_Event( obj, obj, 0, 0, 0 );
		}

		if ( type == MX0_A03_CUSTOM_HARVESTER_DEAD )
		{
			harvester_Dead = true;
			GameObject *Trooper_One = Commands->Find_Object( Trooper_One_Id );

			if ( zone_not_finished )
			{
				if (Trooper_One)
				{
					GameObject *Trooper_One = Commands->Find_Object( Trooper_One_Id );
					// Trooper1: "Nice! That'll cost 'em!"
					int conv_id = Commands->Create_Conversation ( "MX0_A03_08" , 0, 0, true);
					Commands->Join_Conversation( Trooper_One, conv_id, false, false, true);
					Commands->Start_Conversation (conv_id, 0);
				}
			}

			// set trooper 1 to not play "Destroy Harvester havoc..." line
			if (Trooper_One)
			{
				Commands->Send_Custom_Event( obj, Trooper_One, 4, 0, 0 );
			}

			// send custom to controller to see if END zone.
			Commands->Send_Custom_Event( obj, obj, 0, 0, 0 );
		}

		if ( type == M00_SEND_OBJECT_ID )
		{
			if ( param == 1 )
			{
				Humvee_Id = Commands->Get_ID( sender ); // get humvee Id.
			}
			if ( param == 2 ) // send humvee Id.
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, Humvee_Id, 0 );
			}
			if ( param == 3 )
			{
				GDI_Drop_Troop_One_Id = Commands->Get_ID( sender ); // get GDI_Drop_Troop_One Id.
			}
			if ( param == 4 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, GDI_Drop_Troop_One_Id, 0 );
			}
			if ( param == 5 )
			{
				GDI_Drop_Troop_Two_Id = Commands->Get_ID( sender ); // get GDI_Drop_Troop_Two Id.
			}
			if ( param == 6 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, GDI_Drop_Troop_Two_Id, 0 );
			}
			if ( param == 7 )
			{
				GDI_Drop_Troop_Three_Id = Commands->Get_ID( sender ); // get GDI_Drop_Troop_Three Id.
			}
			if ( param == 8 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, GDI_Drop_Troop_Three_Id, 0 );
			}
			if ( param == 9 )
			{
				GDI_Drop_Tank_Id = Commands->Get_ID( sender ); // get GDI_Drop_Tank_Id Id.
			}
			if ( param == 10 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, GDI_Drop_Tank_Id, 0 );
			}
			if ( param == 11 )
			{
				Trooper_One_Id = Commands->Get_ID( sender );
				Commands->Debug_Message( "***** DAK ***** A03: recieved trooper1 ID.\n" );
			}
			if ( param == 12 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, Trooper_One_Id, 0 );
			}
			if ( param == 13 )
			{
				Havoc_Tank_Id = Commands->Get_ID( sender );
				Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500017,10,1.0f"); // sending Havoc_Tank_Id to shuman.
			}
			if ( param == 14 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, Havoc_Tank_Id, 0 );
			}
			if ( param == 15 )
			{
				Nod_Ledge_One_Id = Commands->Get_ID( sender );
			}
			if ( param == 16 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, Nod_Ledge_One_Id, 0 );
			}
			if ( param == 17 )
			{
				Nod_Ledge_Two_Id = Commands->Get_ID( sender );
			}
			if ( param == 18 )
			{
				Commands->Send_Custom_Event( obj, sender, M00_SEND_OBJECT_ID, Nod_Ledge_Two_Id, 0 );
			}
		}
	}
};

DECLARE_SCRIPT ( MX0_A03_HUMVEE, "" ) // moves humvee
{
	int Target_Id[3];
	int target;
	int Current_Target;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Target_Id, 1);
		SAVE_VARIABLE(target, 2);
		SAVE_VARIABLE(Current_Target, 3);
	}

	void Created( GameObject *obj ) override
	{
		target = 1;
		// set targets
		Target_Id[1] = MX0_A03_NOD_BUGGIE_ID;
		Target_Id[2] = MX0_A03_NOD_HARVESTER_ID;


		// send id to controller.
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,1,0"); // sending humvee id to A03 Controller.

		// send id to shuman fool.
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500017,1,1.0f"); // sending humvee id to shuman
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500020,1,1.0f"); 

		const char *conv_name = ("MX0_A03_01");
		int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
		Commands->Join_Conversation(NULL, conv_id, true, true, true);
		Commands->Start_Conversation (conv_id, 1);

		// set first movement.
		ActionParamsStruct params;
		params.Set_Basic ( this, INNATE_PRIORITY_ENEMY_SEEN, 0 );
		params.Set_Movement( Vector3(0,0,0), 0.5f, 1 );
		params.WaypathID = MX0_A03_WAYPATH_HUMVEE_ID;
		Commands->Action_Goto (obj, params);
	}

	void Action_Complete( GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */) override
	{
		if ( action_id == 0 ) // humvee at end of first waypath. begin attacking buggie.
		{
			// Commands->Debug_Message( "***** DAK ***** finding Object Buggie.\n" );
			Current_Target = Target_Id[target];
			GameObject *current_target = Commands->Find_Object( Current_Target );

			if ( current_target ) 
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.Set_Attack(current_target, 60.0f, 0.25f, true);
				Commands->Action_Attack(obj, params);
			}
		}
		if ( action_id == 1 ) // humvee done with target. attack next target
		{
			target = target + 1;
			Current_Target = Target_Id[target];
			GameObject *current_target = Commands->Find_Object( Current_Target );

			if ( current_target )
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.Set_Attack(current_target, 60.0f, 0.25f, true);
				Commands->Action_Attack(obj, params);
			}
			else // no targets. attack whatever you can see.
				Commands->Innate_Enable( obj );
		}
	}

	void Custom ( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == 1 ) // halt fire for 1 - 3 seconds.
		{
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 1);
			params.AttackActive = false;
			Commands->Modify_Action( obj, 1, params, false, true );
			
			float delay = Commands->Get_Random( 1, 3 );
			Commands->Send_Custom_Event( obj, obj, 2, 0, delay );
		}

		if ( type == 2 ) // resume fire on buggy. halt after 3 - 6 seconds.
		{
			GameObject *current_target = Commands->Find_Object( Current_Target );

			if ( current_target )
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.AttackActive = true;
				Commands->Modify_Action( obj, 1, params, false, true );
				
				// send custom to self to halt fire after 3 - 6 seconds
				float delay = Commands->Get_Random( 3, 6 );
				Commands->Send_Custom_Event( obj, obj, 1, 0, delay );
			}
		}
		if ( type == 3 )
		{
			Commands->Debug_Message( "***** DAK ***** Humvee: Recieved custom to move to section 4.\n" );
			// move Humvee to section 4
			Commands->Action_Reset( obj, 100 );

			ActionParamsStruct params;
			params.Set_Basic ( this, INNATE_PRIORITY_ENEMY_SEEN, 2 );
			params.Set_Movement( Vector3(0,0,0), 0.5f, 1 );
			params.WaypathID = MX0_A03_WAYPATH_HUMVEE_TO_A04_ID;

			Commands->Action_Goto (obj, params);
		}
	}
};

DECLARE_SCRIPT ( MX0_A03_TANK, "" ) // moves tank
{
	int Target_Id[5];
	int target;
	int Current_Target;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Target_Id, 1);
		SAVE_VARIABLE(target, 2);
		SAVE_VARIABLE(Current_Target, 3);
	}

	void Created( GameObject *obj ) override
	{
		target = 1;

		// send id to controller.
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,9,0"); // sending humvee id to A03 Controller.

		// send Id to shuman
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500017,3,1.0f"); // sending tank id to A03 Controller.
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500020,3,1.0f"); // sending tank id to A03 Controller.

		// set targets
		Target_Id[1] = MX0_A03_NOD_TURRET_ONE;
		Target_Id[2] = MX0_A03_NOD_TURRET_TWO;
		Target_Id[4] = MX0_A03_NOD_HARVESTER_ID;
		Target_Id[3] = MX0_A03_NOD_BUGGIE_ID;

		// destroy NOD Turrets.
		Commands->Send_Custom_Event ( obj, obj, 0, 0, 1); // attack Turret_One
	}
	void Action_Complete ( GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if ( action_id == 0 ) // done attacking target. attack next target.
		{
			target = target + 1;
			Commands->Send_Custom_Event ( obj, obj, 0, 0, 1); // attack next target.
		}
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == 0 ) // attack Target_Id[target]
		{
			Current_Target = Target_Id[target];
			GameObject *current_target = Commands->Find_Object ( Current_Target );
			if ( current_target )
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 0);
				params.Set_Attack( current_target, 60.0f, 0.25f, true);
				Commands->Action_Attack(obj, params);
			}
		}
	}
};

DECLARE_SCRIPT ( MX0_A03_NOD_BUGGIE, "" )
{
	void Created( GameObject * /* obj */ ) override
	{
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == 1 )
		{
			// move buggie
			ActionParamsStruct params;
			params.Set_Basic ( this, 0, 0 );
			params.Set_Movement( Vector3(0,0,0), 0.25f, 1 );
			params.WaypathID = MX0_A03_WAYPATH_NOD_BUGGIE_ID;
			Commands->Action_Goto (obj, params);
		}
	}
	
	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if ( action_id == 0 )
		{
			// send buggy on waypath again.
			ActionParamsStruct params;
			params.Set_Basic ( this, 0, 0 );
			params.Set_Movement( Vector3(0,0,0), 0.33f, 1 );
			params.WaypathID = MX0_A03_WAYPATH_NOD_BUGGIE_ID;
			Commands->Action_Goto (obj, params);
		}
	}

	void Killed ( GameObject *obj, GameObject * /* killer */ ) override
	{
		// spawn a NOD minigunner and attack script to him for when he dies from tib.
		Vector3 position = Commands->Get_Position ( obj );
		GameObject *nod_trooper = Commands->Create_Object("Nod_Minigunner_0", position);
		Commands->Attach_Script( nod_trooper, "MX0_A03_NOD_TROOPER_TIB_DEATH", "" );
	}
};

DECLARE_SCRIPT ( MX0_A03_NOD_TROOPER_TIB_DEATH, "" )
{
	int Trooper_One_Id;

	void Killed ( GameObject *obj, GameObject * /* killer */ ) override
	{
		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_BUGGIE_DEAD, 0, 2);
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == M00_SEND_OBJECT_ID )
		{
			Trooper_One_Id  = param;
		}
	}

};

DECLARE_SCRIPT ( MX0_A03_NOD_HARVESTER, "" )
{
	bool Is_Waypath_Complete;
	int Trooper_One_Id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Is_Waypath_Complete, 1);
		SAVE_VARIABLE(Trooper_One_Id, 2);
	}

	void Created ( GameObject *obj ) override
	{
		// Commands->Attach_Script( obj, "M00_Damage_Modifier_DME", "0.25,1,1,0,0" );
		Commands->Set_Health(obj, 1200);
		Is_Waypath_Complete = false;

		// begin moving harvester
		ActionParamsStruct params;
		params.Set_Basic ( this, 0, 0 );
		params.Set_Movement( Vector3(0,0,0), 0.25f, 1 );
		params.WaypathID = MX0_A03_WAYPATH_NOD_HARVESTER_ID;
		Commands->Action_Goto (obj, params);
	}

	void Action_Complete( GameObject * obj, int /* action_id */, ActionCompleteReason /* complete_reason */ ) override
	{
		if ( obj )
		{
			Commands->Set_Animation( obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", false, NULL, 0.0f, -1.0f, false);
		}
	}

	void Animation_Complete(GameObject* obj, const char* /* name */) override
	{
		if ( Is_Waypath_Complete )
		{
			Is_Waypath_Complete = false;

			// send harvester the other direction
			ActionParamsStruct params;
			params.Set_Basic ( this, 0, 0 );
			params.Set_Movement( Vector3(0,0,0), 0.25f, 1 );
			params.WaypathID = MX0_A03_WAYPATH_NOD_HARVESTER_ID;
			Commands->Action_Goto (obj, params);
			
		}
		else
		{
			Is_Waypath_Complete = true;

			// send harvester the other direction
			ActionParamsStruct params;
			params.Set_Basic ( this, 0, 0 );
			params.Set_Movement( Vector3(0,0,0), 0.25f, 1 );
			params.WaypathID = MX0_A03_WAYPATH_NOD_HARVESTER_ID;
			params.WaypointStartID = 1400095;
			params.WaypointEndID = 1400094;
			Commands->Action_Goto (obj, params);
		}
	}

	void Damaged ( GameObject * /* obj */, GameObject * /* damager */, float /* ammount */ ) override
	{
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == MX0_A03_CUSTOM_NOD_HARVESTER_DMG_SELF )
		{
			// turn off damage modifier script.
			// Commands->Send_Custom_Event( obj, obj, M00_ENABLE_DAMAGE_MOD, 0, 0 );
			// apply damage to yourself based on param.
			// Commands->Debug_Message( "***** DAK ***** Orcastrike: Damaging harvester.\n" );
			// float health = Commands->Get_Health( obj );
			// float damage = health / param;
			Commands->Apply_Damage( obj, 200, "EXPLOSIVE", NULL );
		}
	}
	void Killed( GameObject *obj, GameObject * /* killer */ ) override
	{
		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller , MX0_A03_CUSTOM_HARVESTER_DEAD, 0, 0 );
		}
	}
};

DECLARE_SCRIPT( MX0_A03_GDI_TROOPER_ONE, "" )
{
	bool Harvester_Not_Destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Harvester_Not_Destroyed, 1);
	}

	void Created( GameObject *obj ) override
	{
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1500017,3,1.0f"); // sending Trooper_One_Id to shuman.
		Commands->Innate_Disable( obj );
		Harvester_Not_Destroyed = true;
	}

	void Action_Complete( GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == 1) // trooper1 is at end of waypath B. open fire!
		{
			Commands->Set_Innate_Is_Stationary( obj, true );
			Commands->Innate_Enable( obj );
		}
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{		
		if (type == 0) // move along MX0_A03_WAYPATH_GDI_TROOPER_ONE_B_ID 
		{
			Commands->Action_Reset( obj, 100 );
			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 1 );
			params.Set_Movement( Vector3(0,0,0), RUN, 1 );
			params.WaypathID = MX0_A03_WAYPATH_GDI_TROOPER_ONE_B_ID;
			Commands->Action_Goto (obj, params);
		}
		if (type == 1)
		{
			// Commands->Debug_Message( "***** DAK ***** Trooper One: Custom Recieved. Moving along MX0_A03_WAYPATH_GDI_TROOPER_ONE_A_ID\n" );
			Commands->Action_Reset( obj, 100 );

			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 0 );
			params.Set_Movement( Vector3(0,0,0), RUN, 1 );
			params.WaypathID = MX0_A03_WAYPATH_GDI_TROOPER_ONE_A_ID;
			Commands->Action_Goto (obj, params);
		}
		if (type == 2)
		{
			Commands->Action_Reset( obj, 100 );
			ActionParamsStruct params;
			params.Set_Basic ( this, 91, 2 );
			params.Set_Movement( Vector3(0,0,0), RUN, 1 );
			params.WaypathID = MX0_A03_WAYPATH_GDI_TROOPER_ONE_C_ID;
			Commands->Action_Goto (obj, params);
		}

		if (type == 3)
		{
			if ( Harvester_Not_Destroyed )
			{
				// Trooper1: Blast that Harvester for us, sir!
				int conv_id = Commands->Create_Conversation ( "MX0_A03_05" , 0, 0, true);
				Commands->Join_Conversation( obj, conv_id, false, false, true);
				Commands->Start_Conversation (conv_id, 0);

				// send custom to self to repeat "Blast that Harvester..." line.
				Commands->Send_Custom_Event( obj, obj, 3, 0, 7 );
			}
		}

		if ( type == 4 )
		{
			Harvester_Not_Destroyed = false;
		}

		if ( type == 5 )
		{
			// start attacking Buggie.
			Commands->Action_Reset( obj, 100 );

			GameObject *buggie = Commands->Find_Object( MX0_A03_NOD_BUGGIE_ID );

			if ( buggie ) 
			{
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 3);
				params.Set_Attack(buggie, 60.0f, 0.25f, true);
				Commands->Action_Attack(obj, params);
			}
		}

	}
};

DECLARE_SCRIPT( MX0_A03_HAVOC_TANK, "" )
{
	bool first_time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(first_time, 1);
	}

	void Custom( GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if (type == CUSTOM_EVENT_VEHICLE_ENTERED )
		{
			if (first_time) // first time player enters tank. send trooper 1 ahead.
			{
				GameObject *controller =  Commands->Find_Object ( MX0_A03_CONTROLLER_ID );
				if ( controller )
				{
					// Commands->Debug_Message( "***** sending custom MX0_A03_CUSTOM_TYPE_START_ZONE\n" );
					Commands->Send_Custom_Event ( obj, controller, MX0_A03_CUSTOM_TYPE_START_ZONE, 0, 0 );
				}
				first_time = false;
			}
		}
	}
};

DECLARE_SCRIPT( MX0_A03_FIRST_PLAYER_ZONE, "" )
{
	int Trooper_One_Id;
	bool first_time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(Trooper_One_Id, 1);
		SAVE_VARIABLE(first_time, 1);
	}

	void Created( GameObject * /* obj */ ) override
	{
		first_time = true;
	}

	void Entered ( GameObject *obj, GameObject * /* enterer */ ) override
	{
		if (first_time)
		{
			// reset trooper one and send him to MX0_A03_WAYPATH_GDI_TROOPER_ONE_B_ID
			GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller , M00_SEND_OBJECT_ID, 12, 0 );
			}
			GameObject *trooper_one = Commands->Find_Object( Trooper_One_Id );
			if (trooper_one)
			{
				Commands->Action_Reset( trooper_one, 100 );
				// send custom to move trooper one along MX0_A03_WAYPATH_GDI_TROOPER_ONE_B_ID
				Commands->Send_Custom_Event( obj, trooper_one, 0, 0, 0.0f);
			}

			// drop off first NOD minigunners.
			GameObject * minigunners = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position( Commands->Find_Object ( MX0_A03_NOD_LEDGE_DROP_LOC_ID )));
			Commands->Attach_Script ( minigunners, "Test_Cinematic", "MX0_A03_NOD_LedgeDrop.txt" );

			first_time = false;
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == M00_SEND_OBJECT_ID )
		{
			Trooper_One_Id  = param;
		}
	}
};

DECLARE_SCRIPT( MX0_A03_TROOPER_ONE_TEST, "" )
{
	void Created (GameObject *obj ) override
	{
		Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,11,0.0f"); // sending Trooper_One_Id to A03_Controller
	}
};

DECLARE_SCRIPT ( MX0_A03_NOD_TURRET, "" )
{
	void Created ( GameObject *obj ) override
	{

		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_NOD_TURRET_CREATED, 0, 0);
		}
	}

	void Killed ( GameObject *obj, GameObject * /* killer */ ) override
	{
		GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
		if ( controller )
		{
			Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_NOD_TURRET_DESTROYED, 0, 0);
		}
	}
};

DECLARE_SCRIPT ( MX0_A03_END_ZONE, "" )
{
	int Trooper_One_Id;
	bool first_time;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(first_time, 1);
	}

	void Created ( GameObject * /* obj */ ) override
	{
		first_time = true;
	}
	void Entered( GameObject *obj, GameObject * /* enterer */ ) override
	{
		if ( first_time )
		{
			// get Trooper 1
			GameObject *controller = Commands->Find_Object( MX0_A03_CONTROLLER_ID );
			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller , M00_SEND_OBJECT_ID, 12, 0 );
			}

			GameObject *Trooper_One = Commands->Find_Object( Trooper_One_Id );

			// set trooper 1 to not play "Destroy Harvester havoc..." line
			if (Trooper_One)
			{
				Commands->Send_Custom_Event( obj, Trooper_One, 4, 0, 0 );
			}

			if ( controller )
			{
				Commands->Send_Custom_Event( obj, controller, MX0_A03_CUSTOM_TYPE_END_ZONE, 1, 0);
			}
			first_time = false;
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == MX0_A03_CUSTOM_LOCK_END_ZONE )
		{
			first_time = false;
		}

		if ( type == M00_SEND_OBJECT_ID )
		{
			Trooper_One_Id  = param;
		}
	}
};
/*
DECLARE_SCRIPT (MX0_Triggered_Conv, "")
{
	void Created (GameObject *obj) override
	{
		char *ENGINEER1_Hit[5] = 
		{
			"Mx0_ENGINEER1_Hit1",
			"Mx0_ENGINEER1_Hit2",
			"Mx0_ENGINEER1_Hit3",
			"Mx0_ENGINEER1_Hit4",
			"Mx0_ENGINEER1_Hit5",
		};

		char *ENGINEER1_Breath[3] = 
		{
			"Mx0_ENGINEER1_Breath1",
			"Mx0_ENGINEER1_Breath2",
			"Mx0_ENGINEER1_Breath3",
		};

		char *ENGINEER1_FFire[9] = 
		{
			"Mx0_ENGINEER1_FFire1",
			"Mx0_ENGINEER1_FFire2",
			"Mx0_ENGINEER1_FFire3",
			"Mx0_ENGINEER1_FFire4",
			"Mx0_ENGINEER1_FFire5",
			"Mx0_ENGINEER1_FFire6",
			"Mx0_ENGINEER1_FFire7",
			"Mx0_ENGINEER1_FFire8",
			"Mx0_ENGINEER1_FFire9",
		};

		char *ENGINEER2_Hit[12] = 
		{
			"Mx0_ENGINEER2_Hit1",
			"Mx0_ENGINEER2_Hit2",
			"Mx0_ENGINEER2_Hit3",
			"Mx0_ENGINEER2_Hit4",
			"Mx0_ENGINEER2_Hit5",
			"Mx0_ENGINEER2_Hit6",
			"Mx0_ENGINEER2_Hit7",
			"Mx0_ENGINEER2_Hit8",
			"Mx0_ENGINEER2_Hit9",
			"Mx0_ENGINEER2_Hit10",
			"Mx0_ENGINEER2_Hit11",
			"Mx0_ENGINEER2_Hit12",
		};

		char *ENGINEER2_Breath[4] = 
		{
			"Mx0_ENGINEER2_Breath1",
			"Mx0_ENGINEER2_Breath2",
			"Mx0_ENGINEER2_Breath3",
			"Mx0_ENGINEER2_Breath4",
		};

		char *ENGINEER2_FFire[7] = 
		{
			"Mx0_ENGINEER2_FFire1",
			"Mx0_ENGINEER2_FFire2",
			"Mx0_ENGINEER2_FFire3",
			"Mx0_ENGINEER2_FFire4",
			"Mx0_ENGINEER2_FFire5",
			"Mx0_ENGINEER2_FFire6",
			"Mx0_ENGINEER2_FFire7",
		};

		char *GDITROOPER1_Hit[4] = 
		{
			"Mx0_GDITROOPER1_Hit1",
			"Mx0_GDITROOPER1_Hit2",
			"Mx0_GDITROOPER1_Hit3",
			"Mx0_GDITROOPER1_Hit4",
		};

		char *GDITROOPER1_Die[6] = 
		{
			"Mx0_GDITROOPER1_Die1",
			"Mx0_GDITROOPER1_Die2",
			"Mx0_GDITROOPER1_Die3",
			"Mx0_GDITROOPER1_Die4",
			"Mx0_GDITROOPER1_Die5",
			"Mx0_GDITROOPER1_Die6",
		};

		char *GDITROOPER1_Breath[3] = 
		{
			"Mx0_GDITROOPER1_Breath1",
			"Mx0_GDITROOPER1_Breath2",
			"Mx0_GDITROOPER1_Breath3",
		};

		char *GDITROOPER1_FFire[6] = 
		{
			"Mx0_GDITROOPER1_FFire1",
			"Mx0_GDITROOPER1_FFire2",
			"Mx0_GDITROOPER1_FFire3",
			"Mx0_GDITROOPER1_FFire4",
			"Mx0_GDITROOPER1_FFire5",
			"Mx0_GDITROOPER1_FFire6",
		};

		char *GDITROOPER1_DieBurn[3] = 
		{
			"Mx0_GDITROOPER1_DieBurn1",
			"Mx0_GDITROOPER1_DieBurn2",
			"Mx0_GDITROOPER1_DieBurn3",
		};

		  char *GDITROOPER1_DieExplo[2] = 
		{
			"Mx0_GDITROOPER1_DieExplo1",
			"Mx0_GDITROOPER1_DieExplo2",
		};

		char *GDITROOPER2_Hit[9] = 
		{
			"Mx0_GDITROOPER2_Hit1",
			"Mx0_GDITROOPER2_Hit2",
			"Mx0_GDITROOPER2_Hit3",
			"Mx0_GDITROOPER2_Hit4",
			"Mx0_GDITROOPER2_Hit5",
			"Mx0_GDITROOPER2_Hit6",
			"Mx0_GDITROOPER2_Hit7",
			"Mx0_GDITROOPER2_Hit8",
			"Mx0_GDITROOPER2_Hit9",
		};

		char *GDITROOPER2_Die[2] = 
		{
			"Mx0_GDITROOPER2_Die1",
			"Mx0_GDITROOPER2_Die2",
		};

		char *GDITROOPER2_DieFire[2] = 
		{
			"Mx0_GDITROOPER2_DieFire1",
			"Mx0_GDITROOPER2_DieFire2",
		};

		char *GDITROOPER2_Breath[3] = 
		{
			"Mx0_GDITROOPER2_Breath1",
			"Mx0_GDITROOPER2_Breath2",
			"Mx0_GDITROOPER2_Breath3",
		};

		char *GDITROOPER2_FFire[6] = 
		{
			"Mx0_GDITROOPER2_FFire1",
			"Mx0_GDITROOPER2_FFire2",
			"Mx0_GDITROOPER2_FFire3",
			"Mx0_GDITROOPER2_FFire4",
			"Mx0_GDITROOPER2_FFire5",
			"Mx0_GDITROOPER2_FFire6",
		};

		char *GDITROOPER3_Hit[7] = 
		{
			"Mx0_GDITROOPER3_Hit2",
			"Mx0_GDITROOPER3_Hit3",
			"Mx0_GDITROOPER3_Hit4",
			"Mx0_GDITROOPER3_Hit5",
			"Mx0_GDITROOPER3_Hit6",
			"Mx0_GDITROOPER3_Hit7",
			"Mx0_GDITROOPER3_Hit8",
		};

		char *GDITROOPER3_Die[2] = 
		{
			"Mx0_GDITROOPER3_Die1",
			"Mx0_GDITROOPER3_Die2",
		};

		char *GDITROOPER3_Breath[3] = 
		{
			"Mx0_GDITROOPER3_Breath1",
			"Mx0_GDITROOPER3_Breath2",
			"Mx0_GDITROOPER3_Breath3",
		};

		char *GDITROOPER3_FFire[7] = 
		{
			"Mx0_GDITROOPER3_FFire1",
			"Mx0_GDITROOPER3_FFire2",
			"Mx0_GDITROOPER3_FFire3",
			"Mx0_GDITROOPER3_FFire4",
			"Mx0_GDITROOPER3_FFire5",
			"Mx0_GDITROOPER3_FFire6",
			"Mx0_GDITROOPER3_FFire7",
		};

		char *GDITROOPER4_Hit[3] = 
		{
			"Mx0_GDITROOPER4_Hit2",
			"Mx0_GDITROOPER4_Hit3",
			"Mx0_GDITROOPER4_Hit4",
		};

		char *GDITROOPER4_Breath[6] = 
		{
			"Mx0_GDITROOPER4_Breath1",
			"Mx0_GDITROOPER4_Breath2",
			"Mx0_GDITROOPER4_Breath3",
			"Mx0_GDITROOPER4_Breath4",
			"Mx0_GDITROOPER4_Breath5",
			"Mx0_GDITROOPER4_Breath6",
		};

		char *GDITROOPER4_FFire[10] = 
		{
			"Mx0_GDITROOPER4_FFire1",
			"Mx0_GDITROOPER4_FFire2",
			"Mx0_GDITROOPER4_FFire3",
			"Mx0_GDITROOPER4_FFire4",
			"Mx0_GDITROOPER4_FFire5",
			"Mx0_GDITROOPER4_FFire6",
			"Mx0_GDITROOPER4_FFire7",
			"Mx0_GDITROOPER4_FFire8",
			"Mx0_GDITROOPER4_FFire9",
			"Mx0_GDITROOPER4_FFire10",
		};

		char *HUM_VEESCOUT_Die[2] = 
		{
			"Mx0_HUM-VEESCOUT_Die1",
			"Mx0_HUM-VEESCOUT_Die2",
		};

		char *HUM_VEESCOUT_Breath[1] = 
		{
			"Mx0_HUM-VEESCOUT_Breath1",
		};

		char *HUM_VEESCOUT_FFire[6] = 
		{
			"Mx0_HUM-VEESCOUT_FFire1",
			"Mx0_HUM-VEESCOUT_FFire2",
			"Mx0_HUM-VEESCOUT_FFire3",
			"Mx0_HUM-VEESCOUT_FFire4",
			"Mx0_HUM-VEESCOUT_FFire5",
			"Mx0_HUM-VEESCOUT_FFire6",
		};

		char *NODOFFICER_Hit[11] = 
		{
			"Mx0_NODOFFICER_Hit2",
			"Mx0_NODOFFICER_Hit3",
			"Mx0_NODOFFICER_Hit4",
			"Mx0_NODOFFICER_Hit5",
			"Mx0_NODOFFICER_Hit6",
			"Mx0_NODOFFICER_Hit7",
			"Mx0_NODOFFICER_Hit8",
			"Mx0_NODOFFICER_Hit9",
			"Mx0_NODOFFICER_Hit10",
			"Mx0_NODOFFICER_Hit11",
			"Mx0_NODOFFICER_Hit12",
		};

		char *NODOFFICER_Die[3] = 
		{
			"Mx0_NODOFFICER_Die1",
			"Mx0_NODOFFICER_Die2",
			"Mx0_NODOFFICER_Die3",
		};

		char *NODOFFICER_DeathFire[2] = 
		{
			"Mx0_NODOFFICER_DeathFire1",
			"Mx0_NODOFFICER_DeathFire2",
		};

		char *NODSNIPER_Hit[4] = 
		{
			"Mx0_NODSNIPER_Hit2",
			"Mx0_NODSNIPER_Hit3",
			"Mx0_NODSNIPER_Hit4",
			"Mx0_NODSNIPER_Hit5",
		};

		char *NODSNIPER_Die[4] = 
		{
			"Mx0_NODSNIPER_Die1",
			"Mx0_NODSNIPER_Die2",
			"Mx0_NODSNIPER_Die3",
			"Mx0_NODSNIPER_Die4",
		};

		char *NODSNIPER_Alt[9] = 
		{
			"Mx0_NODSNIPER_Alt01",
			"Mx0_NODSNIPER_Alt02",
			"Mx0_NODSNIPER_Alt03",
			"Mx0_NODSNIPER_Alt04",
			"Mx0_NODSNIPER_Alt05",
			"Mx0_NODSNIPER_Alt06",
			"Mx0_NODSNIPER_Alt07",
			"Mx0_NODSNIPER_Alt08",
			"Mx0_NODSNIPER_Alt09",
		};

		char *NODSOLDIER1_Hit[4] = 
		{
			"Mx0_NODSOLDIER1_Hit1",
			"Mx0_NODSOLDIER1_Hit2",
			"Mx0_NODSOLDIER1_Hit3",
			"Mx0_NODSOLDIER1_Hit4",
		};

		char *NODSOLDIER1_Die[12] = 
		{
			"Mx0_NODSOLDIER1_Die1",
			"Mx0_NODSOLDIER1_Die2",
			"Mx0_NODSOLDIER1_Die3",
			"Mx0_NODSOLDIER1_Die4",
			"Mx0_NODSOLDIER1_Die5",
			"Mx0_NODSOLDIER1_Die6",
			"Mx0_NODSOLDIER1_Die7",
			"Mx0_NODSOLDIER1_Die8",
			"Mx0_NODSOLDIER1_Die9",
			"Mx0_NODSOLDIER1_Die10",
			"Mx0_NODSOLDIER1_Die11",
			"Mx0_NODSOLDIER1_Die12",
		};

		char *NODSOLDIER2_Hit[5] = 
		{
			"Mx0_NODSOLDIER2_Hit1",
			"Mx0_NODSOLDIER2_Hit2",
			"Mx0_NODSOLDIER2_Hit3",
			"Mx0_NODSOLDIER2_Hit4",
			"Mx0_NODSOLDIER2_Hit5",
		};

		char *NODSOLDIER2_Die[3] = 
		{
			"Mx0_NODSOLDIER2_Die1",
			"Mx0_NODSOLDIER2_Die2",
			"Mx0_NODSOLDIER2_Die3",
		};

		char *ROCKETTROOPER_Hit[3] = 
		{
			"Mx0_ROCKETTROOPER_Hit2",
			"Mx0_ROCKETTROOPER_Hit3",
			"Mx0_ROCKETTROOPER_Hit4",
		};

		char *ROCKETTROOPER_Breath[3] = 
		{
			"Mx0_ROCKETTROOPER_Breath1",
			"Mx0_ROCKETTROOPER_Breath2",
			"Mx0_ROCKETTROOPER_Breath3",
		};

		char *ROCKETTROOPER_FFire[5] = 
		{
			"Mx0_ROCKETTROOPER_FFire1",
			"Mx0_ROCKETTROOPER_FFire2",
			"Mx0_ROCKETTROOPER_FFire3",
			"Mx0_ROCKETTROOPER_FFire5",
			"Mx0_ROCKETTROOPER_FFire6",
		};

		char *GDITROOPER1_Alt[22] = 
		{
			"Mx0_GDITROOPER1_Alt01",
			"Mx0_GDITROOPER1_Alt02",
			"Mx0_GDITROOPER1_Alt03",
			"Mx0_GDITROOPER1_Alt04",
			"Mx0_GDITROOPER1_Alt05",
			"Mx0_GDITROOPER1_Alt06",
			"Mx0_GDITROOPER1_Alt07",
			"Mx0_GDITROOPER1_Alt08",
			"Mx0_GDITROOPER1_Alt09",
			"Mx0_GDITROOPER1_Alt10",
			"Mx0_GDITROOPER1_Alt11",
			"Mx0_GDITROOPER1_Alt12",
			"Mx0_GDITROOPER1_Alt13",
			"Mx0_GDITROOPER1_Alt14",
			"Mx0_GDITROOPER1_Alt15",
			"Mx0_GDITROOPER1_Alt17",
			"Mx0_GDITROOPER1_Alt18",
			"Mx0_GDITROOPER1_Alt19",
			"Mx0_GDITROOPER1_Alt20",
			"Mx0_GDITROOPER1_Alt21",
			"Mx0_GDITROOPER1_Alt22",
			"Mx0_GDITROOPER1_Alt23",
		};

		char *GDITROOPER2_Alt[20] = 
		{
			"Mx0_GDITROOPER2_Alt01",
			"Mx0_GDITROOPER2_Alt02",
			"Mx0_GDITROOPER2_Alt03",
			"Mx0_GDITROOPER2_Alt05",
			"Mx0_GDITROOPER2_Alt06",
			"Mx0_GDITROOPER2_Alt07",
			"Mx0_GDITROOPER2_Alt08",
			"Mx0_GDITROOPER2_Alt09",
			"Mx0_GDITROOPER2_Alt10",
			"Mx0_GDITROOPER2_Alt11",
			"Mx0_GDITROOPER2_Alt12",
			"Mx0_GDITROOPER2_Alt13",
			"Mx0_GDITROOPER2_Alt14",
			"Mx0_GDITROOPER2_Alt15",
			"Mx0_GDITROOPER2_Alt16",
			"Mx0_GDITROOPER2_Alt17",
			"Mx0_GDITROOPER2_Alt18",
			"Mx0_GDITROOPER2_Alt19",
			"Mx0_GDITROOPER2_Alt20",
			"Mx0_GDITROOPER2_Alt21",
		};

		char *GDITROOPER3_Alt[21] = 
		{
			"Mx0_GDITROOPER3_Alt02",
			"Mx0_GDITROOPER3_Alt03",
			"Mx0_GDITROOPER3_Alt04",
			"Mx0_GDITROOPER3_Alt05",
			"Mx0_GDITROOPER3_Alt06",
			"Mx0_GDITROOPER3_Alt07",
			"Mx0_GDITROOPER3_Alt08",
			"Mx0_GDITROOPER3_Alt09",
			"Mx0_GDITROOPER3_Alt11",
			"Mx0_GDITROOPER3_Alt12",
			"Mx0_GDITROOPER3_Alt13",
			"Mx0_GDITROOPER3_Alt14",
			"Mx0_GDITROOPER3_Alt15",
			"Mx0_GDITROOPER3_Alt16",
			"Mx0_GDITROOPER3_Alt17",
			"Mx0_GDITROOPER3_Alt18",
			"Mx0_GDITROOPER3_Alt22",
			"Mx0_GDITROOPER3_Alt23",
			"Mx0_GDITROOPER3_Alt24",
			"Mx0_GDITROOPER3_Alt25",
			"Mx0_GDITROOPER3_Alt26",	
		};

		char *GDITROOPER4_Alt[18] = 
		{
			"Mx0_GDITROOPER4_Alt01",
			"Mx0_GDITROOPER4_Alt02",
			"Mx0_GDITROOPER4_Alt03",
			"Mx0_GDITROOPER4_Alt04",
			"Mx0_GDITROOPER4_Alt05",
			"Mx0_GDITROOPER4_Alt06",
			"Mx0_GDITROOPER4_Alt07",
			"Mx0_GDITROOPER4_Alt08",
			"Mx0_GDITROOPER4_Alt09",
			"Mx0_GDITROOPER4_Alt10",
			"Mx0_GDITROOPER4_Alt11",
			"Mx0_GDITROOPER4_Alt12",
			"Mx0_GDITROOPER4_Alt13",
			"Mx0_GDITROOPER4_Alt14",
			"Mx0_GDITROOPER4_Alt15",
			"Mx0_GDITROOPER4_Alt16",
			"Mx0_GDITROOPER4_Alt17",
			"Mx0_GDITROOPER4_Alt18",
		};

		char *ENGINEER1_Alt[7] = 
		{
			"Mx0_ENGINEER1_Alt01",
			"Mx0_ENGINEER1_Alt02",
			"Mx0_ENGINEER1_Alt03",
			"Mx0_ENGINEER1_Alt04",
			"Mx0_ENGINEER1_Alt05",
			"Mx0_ENGINEER1_Alt08",
			"Mx0_ENGINEER1_Alt09",
		};

		char *ENGINEER2_Alt[12] = 
		{
			"Mx0_ENGINEER2_Alt01",
			"Mx0_ENGINEER2_Alt02",
			"Mx0_ENGINEER2_Alt03",
			"Mx0_ENGINEER2_Alt04",
			"Mx0_ENGINEER2_Alt05",
			"Mx0_ENGINEER2_Alt06",
			"Mx0_ENGINEER2_Alt08",
			"Mx0_ENGINEER2_Alt11",
			"Mx0_ENGINEER2_Alt16",
			"Mx0_ENGINEER2_Alt17",
			"Mx0_ENGINEER2_Alt18",
			"Mx0_ENGINEER2_Alt19",
		};

		char *ROCKETTROOPER_Alt[29] = 
		{
			"Mx0_ROCKETTROOPER_Alt01",
			"Mx0_ROCKETTROOPER_Alt02",
			"Mx0_ROCKETTROOPER_Alt03",
			"Mx0_ROCKETTROOPER_Alt04",
			"Mx0_ROCKETTROOPER_Alt05",
			"Mx0_ROCKETTROOPER_Alt06",
			"Mx0_ROCKETTROOPER_Alt07",
			"Mx0_ROCKETTROOPER_Alt08",
			"Mx0_ROCKETTROOPER_Alt09",
			"Mx0_ROCKETTROOPER_Alt10",
			"Mx0_ROCKETTROOPER_Alt12",
			"Mx0_ROCKETTROOPER_Alt13",
			"Mx0_ROCKETTROOPER_Alt14",
			"Mx0_ROCKETTROOPER_Alt15",
			"Mx0_ROCKETTROOPER_Alt16",
			"Mx0_ROCKETTROOPER_Alt17",
			"Mx0_ROCKETTROOPER_Alt18",
			"Mx0_ROCKETTROOPER_Alt19",
			"Mx0_ROCKETTROOPER_Alt20",
			"Mx0_ROCKETTROOPER_Alt21",
			"Mx0_ROCKETTROOPER_Alt22",
			"Mx0_ROCKETTROOPER_Alt23",
			"Mx0_ROCKETTROOPER_Alt24",
			"Mx0_ROCKETTROOPER_Alt25",
			"Mx0_ROCKETTROOPER_Alt26",
			"Mx0_ROCKETTROOPER_Alt27",
			"Mx0_ROCKETTROOPER_Alt28",
			"Mx0_ROCKETTROOPER_Alt29",
			"Mx0_ROCKETTROOPER_Alt30",
		};

		char *HUM_VEESCOUT_Alt[17] = 
		{
			"Mx0_HUM-VEESCOUT_Alt01",
			"Mx0_HUM-VEESCOUT_Alt02",
			"Mx0_HUM-VEESCOUT_Alt03",
			"Mx0_HUM-VEESCOUT_Alt04",
			"Mx0_HUM-VEESCOUT_Alt05",
			"Mx0_HUM-VEESCOUT_Alt06",
			"Mx0_HUM-VEESCOUT_Alt07",
			"Mx0_HUM-VEESCOUT_Alt08",
			"Mx0_HUM-VEESCOUT_Alt10",
			"Mx0_HUM-VEESCOUT_Alt11",
			"Mx0_HUM-VEESCOUT_Alt12",
			"Mx0_HUM-VEESCOUT_Alt13",
			"Mx0_HUM-VEESCOUT_Alt14",
			"Mx0_HUM-VEESCOUT_Alt15",
			"Mx0_HUM-VEESCOUT_Alt16",
			"Mx0_HUM-VEESCOUT_Alt17",
			"Mx0_HUM-VEESCOUT_Alt18",
		};

		char *NODOFFICER_Alt[23] = 
		{
			"Mx0_NODOFFICER_Alt01",
			"Mx0_NODOFFICER_Alt02",
			"Mx0_NODOFFICER_Alt03",
			"Mx0_NODOFFICER_Alt04",
			"Mx0_NODOFFICER_Alt05",
			"Mx0_NODOFFICER_Alt06",
			"Mx0_NODOFFICER_Alt07",
			"Mx0_NODOFFICER_Alt08",
			"Mx0_NODOFFICER_Alt09",
			"Mx0_NODOFFICER_Alt10",
			"Mx0_NODOFFICER_Alt11",
			"Mx0_NODOFFICER_Alt13",
			"Mx0_NODOFFICER_Alt14",
			"Mx0_NODOFFICER_Alt15",
			"Mx0_NODOFFICER_Alt16",
			"Mx0_NODOFFICER_Alt17",
			"Mx0_NODOFFICER_Alt18",
			"Mx0_NODOFFICER_Alt19",
			"Mx0_NODOFFICER_Alt22",
			"Mx0_NODOFFICER_Alt23",
			"Mx0_NODOFFICER_Alt24",
			"Mx0_NODOFFICER_Alt25",
			"Mx0_NODOFFICER_Alt26",
		};

		char *NODSOLDIER1_Alt[23] = 
		{
			"Mx0_NODSOLDIER1_Alt01",
			"Mx0_NODSOLDIER1_Alt02",
			"Mx0_NODSOLDIER1_Alt03",
			"Mx0_NODSOLDIER1_Alt04",
			"Mx0_NODSOLDIER1_Alt05",
			"Mx0_NODSOLDIER1_Alt06",
			"Mx0_NODSOLDIER1_Alt07",
			"Mx0_NODSOLDIER1_Alt08",
			"Mx0_NODSOLDIER1_Alt09",
			"Mx0_NODSOLDIER1_Alt10",
			"Mx0_NODSOLDIER1_Alt11",
			"Mx0_NODSOLDIER1_Alt12",
			"Mx0_NODSOLDIER1_Alt13",
			"Mx0_NODSOLDIER1_Alt14",
			"Mx0_NODSOLDIER1_Alt15",
			"Mx0_NODSOLDIER1_Alt16",
			"Mx0_NODSOLDIER1_Alt17",
			"Mx0_NODSOLDIER1_Alt18",
			"Mx0_NODSOLDIER1_Alt19",
			"Mx0_NODSOLDIER1_Alt20",
			"Mx0_NODSOLDIER1_Alt21",
			"Mx0_NODSOLDIER1_Alt22",
			"Mx0_NODSOLDIER1_Alt23",
		};

		char *NODSOLDIER2_Alt[17] = 
		{
			"Mx0_NODSOLDIER2_Alt01",
			"Mx0_NODSOLDIER2_Alt02",
			"Mx0_NODSOLDIER2_Alt03",
			"Mx0_NODSOLDIER2_Alt04",
			"Mx0_NODSOLDIER2_Alt05",
			"Mx0_NODSOLDIER2_Alt06",
			"Mx0_NODSOLDIER2_Alt09",
			"Mx0_NODSOLDIER2_Alt10",
			"Mx0_NODSOLDIER2_Alt11",
			"Mx0_NODSOLDIER2_Alt12",
			"Mx0_NODSOLDIER2_Alt14",
			"Mx0_NODSOLDIER2_Alt17",
			"Mx0_NODSOLDIER2_Alt18",
			"Mx0_NODSOLDIER2_Alt19",
			"Mx0_NODSOLDIER2_Alt20",
			"Mx0_NODSOLDIER2_Alt21",
			"Mx0_NODSOLDIER2_Alt22",
		};

		char *ORCA6_Alt[1] = 
		{
			"Mx0_ORCA6_Alt01",
		};

		char *NODTROOPER1_Hit[6] = 
		{
			"Mx0_NODTROOPER1_Hit01",
			"Mx0_NODTROOPER1_Hit02",
			"Mx0_NODTROOPER1_Hit03",
			"Mx0_NODTROOPER1_Hit04",
			"Mx0_NODTROOPER1_Hit05",
			"Mx0_NODTROOPER1_Hit06",
		};

		char *NODTROOPER1_Fall[1] = 
		{
			"Mx0_NODTROOPER1_Fall01",
		};

		char *NODTROOPER1_Breath[1] = 
		{
			"Mx0_NODTROOPER1_Breath3",
		};

		char *NODTROOPER1_Death[2] = 
		{
			"Mx0_NODTROOPER1_Death1",
			"Mx0_NODTROOPER1_Death2",
		};

		char *NODTROOPER1_DeathFire[2] = 
		{
			"Mx0_NODTROOPER1_DeathFire1",
			"Mx0_NODTROOPER1_DeathFire2",
		};

		char *NODFALLINGGUY3_Fall[1] = 
		{
			"Mx0_NODFALLINGGUY3_Fall1",
		};

		char *NODFALLINGGUY2_Die[1] = 
		{
			"Mx0_NODFALLINGGUY2_Die1",
		};
	}
};
*/
