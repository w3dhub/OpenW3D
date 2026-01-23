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

#include "scripts.h"
#include "Toolkit.h"
#include "Mission7.h"
#include "Mission5.h"
#include <stdio.h>
#include <time.h>
#include <cstdint>

DECLARE_SCRIPT (DLS_SpawnTest, "")
{
	bool already_entered;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		Commands->Enable_Spawner(100002, false);
		already_entered = false;
	}

	void Entered (GameObject * /*obj*/, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
		//	Commands->Trigger_Spawner(100002);
			Commands->Enable_Spawner(100002, true);
		}
	}
};

DECLARE_SCRIPT (DLS_Spawner, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Spawner(Commands->Get_ID(obj), false);
	}

	
};

DECLARE_SCRIPT (DLS_Two_Way, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD + 5, 10 );
		params.Set_Movement( Commands->Find_Object(100001), WALK, 1.5f );
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if((action_id == 10) && (reason == ACTION_COMPLETE_NORMAL))
		{
			int id = 100000;
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD + 5, 11 );
			params.Set_Movement( Commands->Find_Object(id), WALK, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if((action_id == 11) && (reason == ACTION_COMPLETE_NORMAL))
		{
			int id = 100001;
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD + 5, 10 );
			params.Set_Movement( Commands->Find_Object(id), WALK, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	
};

DECLARE_SCRIPT(DLS_Invulnerable_Test, "")
{
		
	bool debug_mode;
	int poke_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Health( obj, 2);
		Commands->Apply_Damage( obj, 50000.0f, "STEEL", NULL);
		
	}

	
};

DECLARE_SCRIPT(DLS_InnateIsStationary_Test, "")
{
		
	bool debug_mode;
	int poke_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		
	}
	
	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		ActionParamsStruct params;

		params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 100755);
		params.Set_Movement (STAR, RUN, 2.0f);
		params.MoveFollow = true;
		Commands->Action_Goto (obj, params);
		
	}

	
};

DECLARE_SCRIPT(DLS_Camera_Test, "Pan_Loc1_ID=0:int, Pan_Loc2_ID=0:int, Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	int pan_loc1_id;
	int pan_loc2_id;
	int enemy_id;
	int seen_cnt;
	int time;
	
	
	enum {CAMERA_LOC1 = 15, CAMERA_LOC2 = 16, STAR_VISIBLE = 17};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Enable_Enemy_Seen( obj, true );
		
		pan_loc1_id = Get_Int_Parameter("Pan_Loc1_ID");
		pan_loc2_id = Get_Int_Parameter("Pan_Loc2_ID");
		
		enemy_id = 0;
		seen_cnt = 0;
		time = 0;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, CAMERA_LOC1 );
		params.Set_Attack (Commands->Find_Object(pan_loc1_id), 0.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );

	

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if((action_id == CAMERA_LOC1) && (reason == ACTION_COMPLETE_NORMAL))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, CAMERA_LOC2 );
			params.Set_Attack (Commands->Find_Object(pan_loc2_id), 0.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
		if((action_id == CAMERA_LOC1) && (reason == ACTION_COMPLETE_LOW_PRIORITY))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, CAMERA_LOC2 );
			params.Set_Attack (Commands->Find_Object(pan_loc2_id), 0.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer (obj, this, 2.0f, STAR_VISIBLE);
		}
		if((action_id == CAMERA_LOC2) && (reason == ACTION_COMPLETE_NORMAL))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, CAMERA_LOC1 );
			params.Set_Attack (Commands->Find_Object(pan_loc2_id), 0.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == STAR_VISIBLE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, CAMERA_LOC1 );
			params.Set_Attack (Commands->Find_Object(pan_loc2_id), 0.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		enemy_id = Commands->Get_ID(enemy);

		SCRIPT_DEBUG_MESSAGE (("Enemy Seen: %d Seen Time: %d.\n", Commands->Get_ID(enemy), time));
			
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
		params.Set_Attack (enemy, 0.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );

		if(seen_cnt < 5)
		{
			seen_cnt++;
			Commands->Create_Sound ("Klaxon Warning", Commands->Get_Position (obj), obj);
			Commands->Start_Timer (obj, this, 2.0f, STAR_VISIBLE);
		}
		else
		{
			Commands->Create_Sound ("Klaxon", Commands->Get_Position (obj), obj);
		}

		
		
	}

	
};




DECLARE_SCRIPT(DLS_Gun_Test, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	enum {GUN_LOC1, GUN_LOC2};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( debug_mode, 2 );
	}

	

	void Action_Complete(GameObject * /*obj*/, int action_id, ActionCompleteReason /*reason*/) override
	{
		ActionParamsStruct params;

		if(action_id == 666)
		{
			SCRIPT_DEBUG_MESSAGE (("Action ID: %d ActionCompleteReason: %d.\n", action_id, reason));
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/)override
	{
		ActionParamsStruct params;

		if (type == 666 && param == 666)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (STAR, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}
};

DECLARE_SCRIPT(DLS_Test_NULL, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created(GameObject * /*obj*/) override
	{
		
	}

	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		const char *conv_name = ("IDS_M06_D05");
		int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
		Commands->Join_Conversation(NULL, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 1);
		Commands->Monitor_Conversation (obj, conv_id);	
	}

	
	
	
};

DECLARE_SCRIPT(DLS_Rappelling_Activate, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	bool already_entered;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		SAVE_VARIABLE( already_entered, 2 );
	}

	
	void Damaged (GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/) override
	{
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(controller, 0.000f);
		Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops3.txt");
	}

	
};

DECLARE_SCRIPT(DLS_Tank_Path_Test, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	enum {STAR_VISIBLE};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		
		Commands->Start_Timer (obj, this, 2.0f, STAR_VISIBLE);

		
	}

	void Damaged( GameObject * obj, GameObject * /*damager*/, float /*amount*/ ) override
	{
		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
		params.Set_Attack (STAR, 200.0f, 5.0f, 0);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack (obj, params);
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == STAR_VISIBLE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (STAR, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack (obj, params);
			
		}
		
	}

	

};

DECLARE_SCRIPT(DLS_Vehicle_Follow, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	enum {STAR_VISIBLE};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		
	}

	void Created (GameObject * obj) override
	{
			
		Commands->Start_Timer (obj, this, 2.0f, STAR_VISIBLE);

	}

	

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == STAR_VISIBLE)
		{
			Commands->Apply_Damage( obj, 100000, "STEEL", NULL );
			
		}
		
	}

	

};

DECLARE_SCRIPT(DLS_Filing_Cabinet, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	bool destroyed_state;
	
	enum {FRAME0, FRAME1, FRAME2, FRAME3, FRAME5, FRAME6, FRAME7, FRAME8, FRAME9};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		destroyed_state = false;

		Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 0);
	}
	
	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		if(!destroyed_state)
		{
			if(Commands->Get_Health(obj) < (Commands->Get_Max_Health(obj) * 3.0f))
			{
				Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 4);
				Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME5);
				destroyed_state = true;
			}
			else
			{
				Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 1);
				Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME2);
			}
		}
	}
	
	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == FRAME0)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 0);
			
		}
		if(timer_id == FRAME2)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 2);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME3);
			
		}
		if(timer_id == FRAME3)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 3);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME0);
			
		}
		if(timer_id == FRAME5)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 5);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME6);
			
		}
		if(timer_id == FRAME6)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 6);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME7);
			
		}
		if(timer_id == FRAME7)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 7);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME8);
			
		}
		if(timer_id == FRAME8)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 8);
			Commands->Start_Timer (obj, this, (1.0f / 30.0f), FRAME9);
			
		}
		if(timer_id == FRAME9)
		{
			Commands->Set_Animation_Frame(obj, "dsp_lockers.dsp_lockers", 9);
						
		}
		
		
		
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		Commands->Create_Explosion("Small Crate Explosion", Commands->Get_Position(obj), obj);
	}
	

	

};

DECLARE_SCRIPT(DLS_Waypath_Test, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 16 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
		params.WaypathID = 100001;
		params.WaypointStartID = 100002;
		params.WaypointEndID = 100009;
		params.Set_Attack (Commands->Find_Object(100021), 250.0f, 10.0f, 1);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if((action_id == 15) && (reason == ACTION_COMPLETE_NORMAL))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 16 );
			params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
			params.WaypathID = 100001;
			params.WaypointStartID = 100002;
			params.WaypointEndID = 100009;
			params.Set_Attack (Commands->Find_Object(100021), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
		if((action_id == 16) && (reason == ACTION_COMPLETE_NORMAL))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
			params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
			params.WaypathID = 100011;
			params.WaypointStartID = 100012;
			params.WaypointEndID = 100019;
			params.Set_Attack (Commands->Find_Object(100021), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}

	

	

};



DECLARE_SCRIPT(M06_Camera_Behavior, "Angle:float")
{
	int switcher, enemy_id;
	bool enemy_seen, timer_expired, alert;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(switcher, 1);
		SAVE_VARIABLE(enemy_seen, 2);
		SAVE_VARIABLE(enemy_id, 3);
		SAVE_VARIABLE(timer_expired, 4);
		SAVE_VARIABLE(alert, 5);
	}

	Vector3 Get_Target(void)
	{
		float facing = Commands->Get_Facing(Owner());
		Vector3 target = Commands->Get_Position(Owner());
		target.X += cos(DEG_TO_RADF(facing)) * 10.0f;
		target.Y += sin(DEG_TO_RADF(facing)) * 10.0f;
		target.Z -= 0.9f;
		float angle = Get_Float_Parameter(0);
		angle = WWMath::Clamp(angle, 0.0f, 360.0f);
		switch (switcher)
		{
		case 0: angle /= 2.0f;
			break;
		case 1: angle /= 4.0f;
			break;
		case 2: angle = 0.0f;
			break;
		case 3: angle /= -4.0f;
			break;
		case 4: angle /= -2.0f;
			break;
		case 5: angle /= -4.0f;
			break;
		case 6: angle = 0.0f;
			break;
		case 7: angle /= 4.0f;
			break;
		}
		target -= Commands->Get_Position(Owner());
		target.Rotate_Z(DEG_TO_RADF(angle));
		target += Commands->Get_Position(Owner());
		
		return target;
	}

	Vector3 Get_First_Target(void)
	{
		float angle = Get_Float_Parameter(0);
		float facing = Commands->Get_Facing(Owner());
		angle = angle / 2;
		Vector3 target = Commands->Get_Position(Owner());
		target.X += cos(DEG_TO_RADF(facing)) * 10.0f;
		target.Y += sin(DEG_TO_RADF(facing)) * 10.0f;
		target.Z -= 1.0f;
		target -= Commands->Get_Position(Owner());
		target.Rotate_Z(DEG_TO_RADF(angle));
		target += Commands->Get_Position(Owner());

		return target;
	}
	
	void Created(GameObject * obj) override
	{
		switcher = enemy_id = 0;
		enemy_seen = timer_expired = alert = false;

		Commands->Enable_Enemy_Seen(obj, true);

		ActionParamsStruct params;
		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN - 5, 0);
		params.Set_Attack(Get_First_Target(), 0.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);
		Commands->Start_Timer(obj, this, Get_Int_Parameter(0) / 20.0f, 0);
	}

	void Resume(void)
	{
		Commands->Debug_Message("**** Enemy lost... resuming.\n");
		
		Commands->Action_Reset(Owner(), INNATE_PRIORITY_ENEMY_SEEN - 5);
		enemy_id = 0;
		enemy_seen = timer_expired = alert = false;
	}

	void Alarm(void)
	{
		// React with lots of violence
		Commands->Debug_Message("**** Alarm activated!\n");

		enemy_id = 0;
		enemy_seen = timer_expired = alert = false;
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			timer_expired = true;
			Commands->Start_Timer(obj, this, 2.0f, 2);
		}
		else if (timer_id == 2)
		{
			if (!alert)
			{
				Resume();
			}
			else
			{
				Alarm();
			}
		}
		else
		{
			++switcher %= 8;
	
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN - 5, 0);
			params.Set_Attack(Get_Target(), 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this,  Get_Float_Parameter(0) / 40.0f, 0);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		if (!enemy_seen)
		{
			Commands->Debug_Message("**** Enemy acquired.\n");
			
			enemy_id = Commands->Get_ID(enemy);
			enemy_seen = true;
		
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN, 0);
			params.Set_Attack(enemy, 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this, 2.0f, 1);
		}
		if (enemy_seen && timer_expired && (enemy_id == Commands->Get_ID(enemy)))
		{
			Commands->Debug_Message("**** Enemy still seen.\n");

			alert = true;
		}
	}
};

DECLARE_SCRIPT(DLS_Test_Pickup, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	bool already_entered;
	
	enum {TROOP_PICKUP1, TROOP_PICKUP2, M06_STOP_PICKUP};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( debug_mode, 1 );
		
	}

		
	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 0.0f, TROOP_PICKUP1);
		already_entered = false;
	}

	
	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TROOP_PICKUP1 && (!already_entered))
		{
			float random = Commands->Get_Random(10.0f, 35.0f);

			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(1.548f, -3.497f, 3.595f));
			Commands->Set_Facing(controller, 45.000f);
			Commands->Attach_Script(controller, "Test_Cinematic", "X6I_TroopPickup.txt");

			Commands->Start_Timer (obj, this, random, TROOP_PICKUP2);
			Commands->Start_Timer (obj, this, random, TROOP_PICKUP1);
			
		}
		if(timer_id == TROOP_PICKUP2 && (!already_entered))
		{
			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-58.759f, 29.089f, -6.875f));
			Commands->Set_Facing(controller, -90.000f);
			Commands->Attach_Script(controller, "Test_Cinematic", "X6I_TroopPickup.txt");

		}
		
	}

	void Entered (GameObject * /*obj*/, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		ActionParamsStruct params;

		if (type == M06_STOP_PICKUP)
		{
			already_entered = true;

			Commands->Send_Custom_Event (obj, Commands->Find_Object(100018), M06_STOP_PICKUP, 1, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100018), M06_STOP_PICKUP, 1, 0.0f);
		}
	}

	

};

DECLARE_SCRIPT (DLS_Flame_Tank_Test, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
		params.Set_Movement( Commands->Get_Position(obj), WALK, 1.5f );
		params.Set_Attack (Commands->Find_Object(100001), 250.0f, 0.0f, true);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
	}

	
};

DECLARE_SCRIPT (DLS_Innate_Force_State, "")
{
	enum {BULLET_HEARD, FOOTSTEPS_HEARD, GUNSHOTS_HEARD, ENEMY_SEEN};

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 8.0f, FOOTSTEPS_HEARD);
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		Vector3 pos = Commands->Get_Position(obj);

		if(timer_id == FOOTSTEPS_HEARD)
		{
			Commands->Action_Reset(obj, 100.0f);
			Commands->Innate_Force_State_Footsteps_Heard(obj, pos);
			Commands->Start_Timer (obj, this, 5.0f, BULLET_HEARD);
			Commands->Debug_Message("**** Innate_Force_State_Footsteps_Heard.\n");
		}
		if(timer_id == BULLET_HEARD)
		{
		
			Commands->Innate_Force_State_Bullet_Heard(obj, pos);
			Commands->Start_Timer (obj, this, 5.0f, GUNSHOTS_HEARD);
			Commands->Debug_Message("**** Innate_Force_State_Bullet_Heard.\n");
		}
		
		if(timer_id == GUNSHOTS_HEARD)
		{
			Commands->Innate_Force_State_Gunshots_Heard(obj, pos);
			Commands->Start_Timer (obj, this, 5.0f, ENEMY_SEEN);
			Commands->Debug_Message("**** Innate_Force_State_Gunshots_Heard.\n");
		}		

		if(timer_id == ENEMY_SEEN)
		{
			Commands->Innate_Force_State_Enemy_Seen(obj, STAR);
			Commands->Start_Timer (obj, this, 5.0f, FOOTSTEPS_HEARD);
			Commands->Debug_Message("**** Innate_Force_State_Enemy_Seen.\n");
		}
	}

	
};

DECLARE_SCRIPT (DLS_Cargo_Plane_Test, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(obj));
		Commands->Set_Facing(chinook_obj, 0.0f);
//		Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X5D_C130Troopdrop.txt");
		Commands->Attach_Script(chinook_obj, "M00_C130_ParaDrop", "Nod_MiniGunner_0");
	}

	
};

DECLARE_SCRIPT(M00_C130_ParaDrop, "Preset:string")
{
	int chinook_id;
	bool dead;
	int out;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(chinook_id, 1);
		SAVE_VARIABLE(dead, 2);
		SAVE_VARIABLE(out, 3);
	}
	
	void Created(GameObject * obj) override
	{
		Vector3 loc = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		
		GameObject *chinook_rail = Commands->Create_Object("Generic_Cinematic", loc);
		Commands->Set_Model(chinook_rail, "X5D_Chinookfly");
		Commands->Set_Facing(chinook_rail, facing);
		Commands->Set_Animation(chinook_rail, "X5D_Chinookfly.X5D_Chinookfly", false, NULL, 0.0f, -1.0f, false);
		GameObject *chinook = Commands->Create_Object("Nod_Cargo_Plane", loc);
		Commands->Set_Facing(chinook, facing);
		Commands->Set_Animation(chinook, "vf_nod c-130.vf_nod c130", true, NULL, 0.0f, -1.0f, false);
		Commands->Attach_To_Object_Bone(chinook, chinook_rail, "BN_Chinook_1");

		dead = false;
		out = 0;
		char params[12];
		sprintf(params, "%d", Commands->Get_ID(obj));
		Commands->Attach_Script(chinook, "M00_Reinforcement_C130", params);

				chinook_id = Commands->Get_ID(chinook);
		
		// Destroy Chinook
		Commands->Start_Timer(obj, this, 280.0f/30.0f, 0);
		// Parachutes
		Commands->Start_Timer(obj, this, 169.0f/30.0f, 1);
		Commands->Start_Timer(obj, this, 179.0f/30.0f, 2);
		Commands->Start_Timer(obj, this, 198.0f/30.0f, 3);
		// Soldiers
		Commands->Start_Timer(obj, this, 145.0f/30.0f, 4);
		Commands->Start_Timer(obj, this, 155.0f/30.0f, 5);
		Commands->Start_Timer(obj, this, 165.0f/30.0f, 6);

		
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		Vector3 loc = Commands->Get_Position(obj);
		const char * preset = Get_Parameter("Preset");
		float facing = Commands->Get_Facing(obj);

		switch (timer_id)
		{
		case 0: 
			GameObject *chinook;
			chinook = Commands->Find_Object(chinook_id);
			Commands->Destroy_Object(chinook);
			break;
		case 1: 
			if (out >= 1)
			{
				GameObject *para1;
				para1 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para1, facing);
				Commands->Set_Model(para1, "X5D_Parachute");
				Commands->Set_Animation(para1, "X5D_Parachute.X5D_ParaC_1", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para1, "ROOTTRANSFORM");
				Commands->Attach_Script(para1, "M03_No_More_Parachute", "");
			}
			break;
		case 2: 
			if (out >= 2)
			{
				GameObject *para2;
				para2 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para2, facing);
				Commands->Set_Model(para2, "X5D_Parachute");
				Commands->Set_Animation(para2, "X5D_Parachute.X5D_ParaC_2", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para2, "ROOTTRANSFORM");
				Commands->Attach_Script(para2, "M03_No_More_Parachute", "");
			}
			break;
		case 3: 
			if (out == 3)
			{
				GameObject *para3;
				para3 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para3, facing);
				Commands->Set_Model(para3, "X5D_Parachute");
				Commands->Set_Animation(para3, "X5D_Parachute.X5D_ParaC_3", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para3, "ROOTTRANSFORM");
				Commands->Attach_Script(para3, "M03_No_More_Parachute", "");
			}
			break;
		case 4: 
			if (!dead)
			{
			
			GameObject *box1 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box1, "X5D_Box01");
			Commands->Set_Facing(box1, facing);
			Commands->Set_Animation(box1, "X5D_Box01.X5D_Box01", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier1;
			soldier1 = Commands->Create_Object_At_Bone(box1, preset, "Box01");
			Commands->Set_Facing(soldier1, facing);
			Commands->Attach_To_Object_Bone( soldier1, box1, "Box01" );
			Commands->Set_Animation(soldier1, "s_a_human.H_A_X5D_ParaT_1", false, NULL, 0.0f, -1.0f, false);
			out++;
			if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}
		
			}
			break;
		case 5: 
			if (!dead)
			{

			GameObject *box2 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box2, "X5D_Box02");
			Commands->Set_Facing(box2, facing);
			Commands->Set_Animation(box2, "X5D_Box02.X5D_Box02", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier2;
			soldier2 = Commands->Create_Object_At_Bone(box2, preset, "Box02");
			Commands->Set_Facing(soldier2, facing);
			Commands->Set_Animation(soldier2, "s_a_human.H_A_X5D_ParaT_2", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier2, box2, "Box02" );
			out++;
			if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}
				
			}
			break;
		case 6: 
			if (!dead)
			{

			GameObject *box3 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box3, "X5D_Box03");
			Commands->Set_Facing(box3, facing);
			Commands->Set_Animation(box3, "X5D_Box03.X5D_Box03", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier3;
			soldier3 = Commands->Create_Object_At_Bone(box3, preset, "Box03");
			Commands->Set_Facing(soldier3, facing);
			Commands->Set_Animation(soldier3, "s_a_human.H_A_X5D_ParaT_3", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier3, box3, "Box03" );
			out++;
			if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}
			
			}
			break;
		
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 23000 && param == 23000)
		{
			dead = true;
		}
	}
};

DECLARE_SCRIPT(M00_Reinforcement_C130, "Controller_ID:int")
{
	int sound_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sound_id, 1);
	}
	
	void Created(GameObject * obj) override
	{
		sound_id = Commands->Create_3D_Sound_At_Bone("C130_Idle_01", obj, "BODYMAIN");
	}
	
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject * con = Commands->Find_Object(Get_Int_Parameter(0));
		Commands->Send_Custom_Event(obj, con, 23000, 23000, 0.0f);
		Commands->Stop_Sound(sound_id, true);
	}
};

DECLARE_SCRIPT(DLS_Test_Conversation, "Debug_Mode=0:int")
{
	bool debug_mode;
	
	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
	}
	
	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
	
		const char *conv_name = ("IDS_M00_Test");
		int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
		Commands->Join_Conversation(Commands->Get_A_Star(Commands->Get_Position(obj)), conv_id, false, true, true);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 300001);
		Commands->Monitor_Conversation (obj, conv_id);
		
	}

	void Action_Complete(GameObject * /*obj*/, int /*action_id*/, ActionCompleteReason /*reason*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("Action_Complete action_id %d reason %d.\n", action_id, reason));	
		
	}
};

DECLARE_SCRIPT(DLS_Test_Zone_Timer, "Debug_Mode=0:int")
{
		
	bool debug_mode;
	
	
		
	void Created (GameObject * obj) override
	{
		SCRIPT_DEBUG_MESSAGE (("DLS_Test_Zone_Timer Created.\n"));

		Commands->Enable_Hibernation(obj, false);
		Commands->Start_Timer(obj, this, 1.0f, 666);
	}

	
	void Timer_Expired(GameObject * obj, int /*timer_id*/ ) override
	{
		SCRIPT_DEBUG_MESSAGE (("Timer_Expired on DLS_Test_Zone_Timer timer_id: %d.\n", timer_id));

		Commands->Start_Timer(obj, this, 1.0f, 666);
		
	}
};

DECLARE_SCRIPT(DLS_Drop_Unit, "Box_ID:int")
{
		
	

	void Damaged (GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/) override
	{
		int box_id = Get_Int_Parameter("Box_ID");
		switch(box_id)
		{
		case 1:
			{
				GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
				Commands->Set_Facing(controller, 0.000f);
				Commands->Attach_Script(controller, "Test_Cinematic", "No_Definition.txt");
			}
			break;
		case 2:
			{
				GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
				Commands->Set_Facing(controller, 0.000f);
				Commands->Attach_Script(controller, "Test_Cinematic", "No_Definition2.txt");
			}
			break;
		}

	}

};

DECLARE_SCRIPT(DLS_CNC_Sound, "Box_ID:int")
{
		
	

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Create_Sound ( "CnC_Healer_Sound", Commands->Get_Position(obj), obj );

	}

};

DECLARE_SCRIPT(DLS_Spawn_Engineer, "")
{
		
	

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health(obj, Commands->Get_Max_Health(obj));
		Commands->Enable_Spawner(100001, true);

	}

};

DECLARE_SCRIPT(DLS_Spawn_Engineer2, "")
{
		
	

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health(obj, Commands->Get_Max_Health(obj));
		Commands->Enable_Spawner(100001, false);

	}

};

DECLARE_SCRIPT(DLS_Test_Homepoint, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2.0f);
	}
};
	
DECLARE_SCRIPT (DLS_Test_Apache, "")
{
	

	void Created (GameObject *obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, 100, 10 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 2.0f );		
		params.WaypathID = 2000222;
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );
	}
};	

DECLARE_SCRIPT (DLS_Test_Tank, "")
{
	

	void Created (GameObject *obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, 100, 10 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 2.0f );		
		params.WaypathID = 100002;
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );
	}
};	

DECLARE_SCRIPT (DLS_Goto_Unit, "")
{
	enum{GO_STAR};

	void Created (GameObject *obj) override
	{
		ActionParamsStruct params;

		Commands->Start_Timer (obj, this, 5.0f, GO_STAR);
	
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, 100, 10 );
			params.Set_Movement( STAR, 1.0f, 2.0f );		
			Commands->Action_Goto( obj, params );
		}
		
	}
};

DECLARE_SCRIPT (DLS_Playertype, "")
{

	void Created (GameObject * /*obj*/) override
	{
		
	}

	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		Commands->Give_PowerUp(obj, "MG Weapon 1 Clip PowerUp", false);
		Commands->Give_PowerUp(obj, "MiniGun 2 Clips PU", false);
		
	}
};

DECLARE_SCRIPT (DLS_No_Innate, "")
{

	void Created (GameObject *obj) override
	{
		Commands->Innate_Disable(obj);
	}

	
};
		

DECLARE_SCRIPT(DLS_ActionComplete_Test, "")
{


	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 16 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
		params.WaypathID = 100001;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * /*obj*/, int action_id, ActionCompleteReason reason) override
	{
		Commands->Debug_Message("Action_Complete %d %d. \n", action_id, reason);
	}
};

DECLARE_SCRIPT(DLS_Artillery_Test, "")
{
	int artillery_loc[2];

	enum{ARTILLERY_DROP};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject *game_obj) override
	{
		artillery_loc[0] = 100600;
		artillery_loc[1] = 100601;

		Commands->Start_Timer(game_obj, this, 10.0F, ARTILLERY_DROP);
	}


	void Timer_Expired (GameObject * /*game_obj*/, int timer_id) override
	{
		if (timer_id == ARTILLERY_DROP) 
		{
			
		}

		return ;
	}
};

DECLARE_SCRIPT(DLS_Cinematic_Test, "")
{
	void Created (GameObject * /*obj*/) override
	{
	/*	Commands->Enable_Hibernation(obj, false);
		Commands->Set_Player_Type(obj, PLAYERTYPE_NEUTRAL );
		Commands->Create_Object ( "XG_TransprtBone", Vector3(0.0f, 0.0f, 0.0f));*/
	}


	void Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override
	{
	/*	GameObject * chinook_obj0 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj0, 0.0f);
		Commands->Attach_Script(chinook_obj0, "Test_Cinematic", "X7A_TrnsptCt_00.txt");

		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj1, 0.0f);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7A_TrnsptCt_01.txt");*/

		GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj2, 0.0f);
		Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "XG_EV5.txt");

	/*	GameObject * nukebits = Commands->Create_Object("M07_Nukebits", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing( nukebits, -10.000f );*/


	//	GameObject * napc = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
	//	Commands->Set_Model(napc, "e_chopperdust1");

	/*	Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), M07_EVAC_INN, 1, 0.0f);

		Commands->Enable_Spawner(100002, true);
		Commands->Enable_Spawner(100004, true);
		Commands->Enable_Spawner(100005, true);
		Commands->Enable_Spawner(100006, true);
		Commands->Enable_Spawner(100007, true);*/
	}
};

DECLARE_SCRIPT(DLS_SSM_Test, "")
{
	int ssm_missile_id;

	void Created (GameObject * obj) override
	{
		GameObject * ssm_missile = Commands->Create_Object_At_Bone(obj, "Invisible_Object", "V_LAUNCHER");
		Commands->Set_Model(ssm_missile, "V_AG_NOD_SSM_MS");
		Commands->Attach_To_Object_Bone(ssm_missile, obj, "V_LAUNCHER");
		ssm_missile_id = Commands->Get_ID(ssm_missile);
	}

	void Damaged( GameObject * obj, GameObject * /*damager*/, float /*amount*/ ) override
	{
		Commands->Set_Animation(obj, "V_NOD_SSM.V_NOD_SSM", 0, NULL, 0.0f, -1.0f, false);
		Commands->Set_Animation(Commands->Find_Object(ssm_missile_id), "v_nod_ssm_Missl.v_nod_ssm_Missl", 0, NULL, 0.0f, -1.0f, false);
	}
};





DECLARE_SCRIPT(DLS_Cinematic_Test2, "")
{
	void Created (GameObject * /*obj*/) override
	{
	/*	Commands->Enable_Hibernation(obj, false);
		Commands->Set_Player_Type(obj, PLAYERTYPE_NEUTRAL );
		Commands->Create_Object ( "XG_TransprtBone", Vector3(0.0f, 0.0f, 0.0f));*/
	}


	void Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override
	{
	/*	GameObject * chinook_obj0 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj0, 0.0f);
		Commands->Attach_Script(chinook_obj0, "Test_Cinematic", "X7A_TrnsptCt_00.txt");

		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj1, 0.0f);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7A_TrnsptCt_01.txt");*/

		GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj2, 0.0f);
		Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X6C_MIDTRO.txt");

	/*	Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), M07_EVAC_INN, 1, 0.0f);

		Commands->Enable_Spawner(100002, true);
		Commands->Enable_Spawner(100004, true);
		Commands->Enable_Spawner(100005, true);
		Commands->Enable_Spawner(100006, true);
		Commands->Enable_Spawner(100007, true);*/
	}
};

DECLARE_SCRIPT(DLS_Where_Am_I, "")
{
	enum{WHERE_AM_I};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
//		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created (GameObject * obj) override
	{
		Vector3 pos = Commands->Get_Position(obj);
		const char * preset_name = Commands->Get_Preset_Name( obj );
		Commands->Debug_Message("%s at: X:%f Y:%f Z:%f \n", preset_name, pos.X, pos.Y, pos.Z);

		Commands->Start_Timer (obj, this, 3.0f, WHERE_AM_I);
		
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == WHERE_AM_I)
		{
			Vector3 pos = Commands->Get_Position(obj);
			const char * preset_name = Commands->Get_Preset_Name( obj );
			Commands->Debug_Message("%s at: X:%f Y:%f Z:%f \n", preset_name, pos.X, pos.Y, pos.Z);

			Commands->Start_Timer (obj, this, 3.0f, WHERE_AM_I);
			
		}
		
	}
	
};

DECLARE_SCRIPT(DLS_Test_Flyovers, "")
{
	enum{WHERE_AM_I};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
//		SAVE_VARIABLE( debug_mode, 2 );
	}

	void Created (GameObject * obj) override
	{
		
		Commands->Start_Timer (obj, this, 17.0f, WHERE_AM_I);
		
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == WHERE_AM_I)
		{
			

			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(chinook_obj1, 0.0f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7A_Apache_00.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(chinook_obj2, 0.0f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X7A_CPlane_00.txt");

			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(chinook_obj3, 0.0f);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X7A_TrnsptCt_00.txt");

			Commands->Start_Timer (obj, this, 17.0f, WHERE_AM_I);
			
		}
		
	}
	
};

DECLARE_SCRIPT(DLS_Test_Hand_Over_Head, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poked, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Innate_Disable(obj);

		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
		params.Set_Animation("S_A_HUMAN.H_A_Host_L2b", true);
		Commands->Action_Play_Animation(obj, params);
	}


};

DECLARE_SCRIPT(DLS_Created_Too_Early, "")
{
	void Created (GameObject * /*obj*/) override
	{
	//	Commands->Enable_Hibernation(obj, false);
	//	Commands->Set_Player_Type(obj, PLAYERTYPE_NEUTRAL );
	//	Commands->Create_Object ( "XG_TransprtBone", Vector3(0.0f, 0.0f, 0.0f));
	}


	void Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override
	{
		GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj3, 0.0f);
		Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "Created_Too_Early.txt");
	}
};

DECLARE_SCRIPT(DLS_Blink, "")
{
	void Created (GameObject * /*obj*/) override
	{
	//	Commands->Enable_Hibernation(obj, false);
	//	Commands->Set_Player_Type(obj, PLAYERTYPE_NEUTRAL );
	//	Commands->Create_Object ( "XG_TransprtBone", Vector3(0.0f, 0.0f, 0.0f));
	}


	void Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override
	{
		GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj3, 0.0f);
		Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "Blink.txt");
	}
};

DECLARE_SCRIPT(DLS_Math, "")
{
	void Created (GameObject * obj) override
	{
		Vector3 pos = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		float a = cos(DEG_TO_RADF(facing));
		float b = sin(DEG_TO_RADF(facing));
		Commands->Debug_Message("", a);
		Commands->Debug_Message("", b);
	}
};

DECLARE_SCRIPT(DLS_Test_Evac, "")  // Deadeye2
{
	enum{WAYPATH};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Animation ( obj, "S_A_Human.XG_EV5_troop", true, NULL, 0.0f, -1.0f, false );
		Commands->Enable_Hibernation(obj, false);
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100008;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == WAYPATH)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}
		
	}

};

DECLARE_SCRIPT(DLS_Innate_Disable, "")  // Deadeye2
{
	void Created(GameObject * obj) override
	{
		Commands->Innate_Disable(obj);
	}

};



/**********************************************************************************************
/*
/*
/*
/*		MX0
/*
/*
/*
/**********************************************************************************************/



// MX0 Custom Enumerations

enum
{
	MX0_STAR_AREA =	445000,				// 0
	MX0_VEHICLE_MOVE,					// 1
	MX0_SPECIFIC_ACTION,				// 2
	MX0_OBELISK_HUMVEE,					// 3
	MX0_OBELISK_DRIVER,					// 4
	MX0_OBELISK_MEDIUM_TANK,			// 5
	MX0_SOLDIER_MOVE,					// 6
	MX0_FIRE_SAM,						// 7	Hardcoded into X0E_Obelisk.txt 
	MX0_A10_STRIKE,						// 8
	MX0_MISSION_SUCCESS,				// 9	Hardcoded into X0Z_Finale.txt
	MX0_DISCOVERS_NOD_BASE,				// 10
	MX0_HUMVEE_DRIVER_COWER,			// 11
	MX0_ROCKETTROOPER_SEES_OBELISK,		// 12
	MX0_ROCKETTROOPER_HOT_OBELISK,		// 13
	MX0_ROCKETTROOPER_OBELISK_DOWN,		// 14
	MX0_ROCKETTROOPER_TAKE_SAMS,		// 15
	MX0_ROCKETTROOPER_SAMS_HISTORY,		// 16
	MX0_TROOPER_UP_ON_WALL,				// 17
	MX0_DESTROY_OBELISK,				// 18	Hardcoded into X0E_Obelisk.txt 
	MX0_GDI_REINFORCEMENT_KILLED,		// 19
	MX0_NEED_YOU_HERE,					// 20
	MX0_ROCKETTROOPER_CANT_TURN_BACK,	// 21
	
};

#define	WRONG_WAY_CONV_TABLE_SIZE  ( sizeof(Wrong_Way_Conv_Table) / sizeof (Wrong_Way_Conv_Table[0]) )
const char *	Wrong_Way_Conv_Table[] = 
{
	"MX0_A04_CON015",	// You�re going the wrong way, Havoc!
	"MX0_A04_CON016",	// Stay with the Mission, Havoc!
	"MX0_A04_CON017",	// This area is already secure.
	"MX0_A04_CON018",	// Havoc, Did I say retreat? Now Turn Around!
};

DECLARE_SCRIPT (MX0_Area4_Controller_DLS, "")
{
	int star_area;
	bool area4_activated;
	int humvee_id;
	int medium_tank_blasted_id;
	int medium_tank_escort_id;
	int light_tank_a_id;
	int light_tank_b_id;
	int nod_base_rocketsoldier_a_id;
	int nod_base_rocketsoldier_b_id;
	int player_tank_id;
	int gdi_trooper1_id;
	int gdi_trooper2_id;
	int gdi_trooper3_id;
	int obelisk_id;
	int obelisk_weapon_id;
	int obelisk_orca_id;
	int mobile_artillery_id;
	bool orca_strike;
	int basewall_id;
	bool say_take_out_sams;
	int mx0_gdi_reinforcement_killed;
	int gdi_reinforcement1_id;
	int gdi_reinforcement2_id;
	int conv_num;
	int say_need_here;
		
	enum
	{
		AREA4_ACTIVATED = 0,
		HUMMVEE_DESTRUCTION,
		MEDIUM_TANK_DESTRUCTION,
		OBELISK_DESTRUCTION,
		SAMS_DESTRUCTION,
		A10_STRIKE,
		ION_CANNON_STRIKE,
		ORCA_STRIKE1,
		ORCA_STRIKE2,
		FINALE,
		CONVERSATION_HAVOC_TAKE_OUT_SAMS,
		A10_HIT,
		DESTROY_SAM1,
		DESTROY_SAM2,
		FLASH_TO_WHITE,

	};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{		
		SAVE_VARIABLE( star_area, 1 );
		SAVE_VARIABLE( area4_activated, 2 );
		SAVE_VARIABLE( humvee_id, 3 );
		SAVE_VARIABLE( medium_tank_blasted_id, 4 );
		SAVE_VARIABLE( medium_tank_escort_id, 5 );
		SAVE_VARIABLE( light_tank_a_id, 6 );
		SAVE_VARIABLE( light_tank_b_id, 7 );
		SAVE_VARIABLE( nod_base_rocketsoldier_a_id, 8 );
		SAVE_VARIABLE( nod_base_rocketsoldier_b_id, 9 );
		SAVE_VARIABLE( player_tank_id, 10 );
		SAVE_VARIABLE( gdi_trooper1_id, 11 );
		SAVE_VARIABLE( gdi_trooper2_id, 12 );
		SAVE_VARIABLE( gdi_trooper3_id, 13 );
		SAVE_VARIABLE( obelisk_id, 14 );
		SAVE_VARIABLE( obelisk_weapon_id, 15 );
		SAVE_VARIABLE( obelisk_orca_id, 16 );
		SAVE_VARIABLE( mobile_artillery_id, 17 );
		SAVE_VARIABLE( orca_strike, 18 );
		SAVE_VARIABLE( basewall_id, 19 );
		SAVE_VARIABLE( say_take_out_sams, 20 );
		SAVE_VARIABLE( mx0_gdi_reinforcement_killed, 21 );
		SAVE_VARIABLE( gdi_reinforcement1_id, 22 );
		SAVE_VARIABLE( gdi_reinforcement2_id, 23 );
		SAVE_VARIABLE( conv_num, 24 );
		SAVE_VARIABLE( say_need_here, 25 );
	}

	void Created( GameObject * /*obj*/ ) override
	{
		star_area = 0;
		area4_activated = false;
		// Scale up the sight range of all units in game
		Commands->Scale_AI_Awareness(3.0f, 1.0f);
		say_take_out_sams = false;
		orca_strike = false;
		// Reinforcement counter
		mx0_gdi_reinforcement_killed = 0;
		// Wrong Way conversation
		conv_num = 0;
		// Rocket Trooper telling Havoc need up here
		say_need_here = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if(type == MX0_DESTROY_OBELISK)
		{
			// Remove old Obelisk
			Commands->Destroy_Object(Commands->Find_Object(obelisk_id));
		}
		// What zone is the star in?
		if(type == MX0_STAR_AREA)
		{
			star_area = param;
			if(star_area == 1 && !area4_activated)
			{
				area4_activated = true;
				Commands->Start_Timer (obj, this, 3.0f, AREA4_ACTIVATED);

				// Start area4 actors
				// Additional GDI troop drop
		//		GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(Commands->Find_Object(1500051)));
		//		Commands->Set_Facing(chinook_obj2, 0.0f);
		//		Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_GDI_TroopDrop_Area4.txt");
				// Humvee
				GameObject * humvee = Commands->Find_Object(humvee_id);
				Commands->Attach_Script(humvee, "MX0_Vehicle_DLS", "1500018, 1500019, 0, 0, 0.5f");
				Commands->Attach_Script(humvee, "MX0_GDI_Killed_DLS", "1");
				
				// Medium Tank Blasted  *** Ajdusted 12/12/01 No longer gets destroyed
				GameObject * medium_tank_blasted = Commands->Find_Object(medium_tank_blasted_id);
				Commands->Attach_Script(medium_tank_blasted, "MX0_Vehicle_DLS", "1500022, 1500023, 0, 0, 1.0f");
				Commands->Attach_Script(medium_tank_blasted, "MX0_GDI_Killed_DLS", "2");
				Commands->Attach_Script(medium_tank_blasted, "M00_Damage_Modifier_DME", ".10f,1,1,1,1");

				// Medium Tank Escort
				GameObject * medium_tank_escort = Commands->Find_Object(medium_tank_escort_id);
				Commands->Attach_Script(medium_tank_escort, "MX0_Vehicle_DLS", "1500027, 1500028, 1500029, 0, 1.0f");
				

				// GDI Trooper 1
				GameObject * gdi_trooper1 = Commands->Find_Object(gdi_trooper1_id);
				Commands->Attach_Script(gdi_trooper1, "MX0_GDI_Soldier_DLS", "1500049, 1500050, 0, 0, 0.8f");
				// GDI RocketTrooper
				GameObject * gdi_trooper2 = Commands->Create_Object("GDI_RocketSoldier_0", Vector3(51.644f, 27.306f, 4.850f));
				Commands->Attach_Script(gdi_trooper2, "M00_Send_Object_ID", "1500017, 12, 0.0f");
				Commands->Attach_Script(gdi_trooper2, "MX0_GDI_Soldier_DLS", "1500053, 1500070, 0, 0, 0.8f");
				// Humvee announces discovery of Nod Base
				Commands->Send_Custom_Event( obj, Commands->Find_Object(humvee_id), MX0_SPECIFIC_ACTION, MX0_DISCOVERS_NOD_BASE, 0.0f);
			}
		}
		// Actor IDs
		if(type == M00_SEND_OBJECT_ID)
		{

			if(param == 1)
			{
				humvee_id = Commands->Get_ID(sender);				
			}
			if(param == 3)
			{
				medium_tank_blasted_id = Commands->Get_ID(sender);				
			}
			if(param == 4)
			{
				medium_tank_escort_id = Commands->Get_ID(sender);				
			}
			if(param == 5)
			{
				light_tank_a_id = Commands->Get_ID(sender);				
			}
			if(param == 6)
			{
				light_tank_b_id = Commands->Get_ID(sender);				
			}
			if(param == 8)
			{
				nod_base_rocketsoldier_a_id = Commands->Get_ID(sender);				
			}
			if(param == 9)
			{
				nod_base_rocketsoldier_b_id = Commands->Get_ID(sender);				
			}
			if(param == 10)
			{
				player_tank_id = Commands->Get_ID(sender);				
			}
			if(param == 11)
			{
				gdi_trooper1_id = Commands->Get_ID(sender);				
			}
			if(param == 12)
			{
				gdi_trooper2_id = Commands->Get_ID(sender);				
			}
			if(param == 13)
			{
				gdi_trooper3_id = Commands->Get_ID(sender);				
			}
			if(param == 14)
			{
				obelisk_id = Commands->Get_ID(sender);				
			}
			if(param == 15)
			{
				obelisk_weapon_id = Commands->Get_ID(sender);				
			}
			if(param == 16)
			{
				obelisk_orca_id = Commands->Get_ID(sender);				
			}
			if(param == 17)
			{
				mobile_artillery_id = Commands->Get_ID(sender);				
			}
			if(param == 18)
			{
				basewall_id = Commands->Get_ID(sender);				
			}
			if(param == 19)
			{
				gdi_reinforcement1_id = Commands->Get_ID(sender);				
			}
			if(param == 20)
			{
				gdi_reinforcement2_id = Commands->Get_ID(sender);				
			}
			
		}
		// Commands for SAMs to fire on incoming Orca
		if (type == MX0_FIRE_SAM)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object(1500015), MX0_FIRE_SAM, obelisk_orca_id, 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object(1500016), MX0_FIRE_SAM, obelisk_orca_id, 0.0f);
		}
		// Commands to destroy Mobile Artillery in front of base
		if (type == MX0_A10_STRIKE)
		{
			Commands->Apply_Damage( Commands->Find_Object(mobile_artillery_id), 50000.0f, "STEEL", NULL);
		}
		// Finale concludes, mission success
		if(type == MX0_MISSION_SUCCESS)
		{
			Commands->Mission_Complete(true);
		}
		if(type == MX0_GDI_REINFORCEMENT_KILLED)
		{
			mx0_gdi_reinforcement_killed++;
			if(mx0_gdi_reinforcement_killed == 2)
			{
				mx0_gdi_reinforcement_killed = 0;
				// Additional GDI troop drop
				GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(Commands->Find_Object(1500102)));
				Commands->Set_Facing(chinook_obj2, Commands->Get_Facing(Commands->Find_Object(1500102)));
				Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_GDI_Reinforce_Area4.txt");
			}
		}

	}
	// Simple Function to move reinforcement troops
	void Relocate_Soldiers (GameObject * obj)
	{
		// Initial Chinook Drops
		Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SOLDIER_MOVE, 1, 0.0f);
		Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper3_id), MX0_SOLDIER_MOVE, 1, 0.0f);
		// GDI Reinforcements
		int rand = Commands->Get_Random_Int(0, 3);
		Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_reinforcement1_id), MX0_SOLDIER_MOVE, rand, 0.0f);
		Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_reinforcement2_id), MX0_SOLDIER_MOVE, rand, 0.0f);
	}
	void Wrong_Way(GameObject * obj)
	{
		
		if(conv_num == WRONG_WAY_CONV_TABLE_SIZE)
		{
			conv_num = 0;
		}
		
		const char *conv_name = Wrong_Way_Conv_Table[conv_num];
		int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 10);
		conv_num++;
	}
	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == AREA4_ACTIVATED)
		{			
			if(star_area > 0)  // Area 2 or Area 3
			{
				// Start moving convoy forward with all GDI troops
				// Humvee
				Commands->Send_Custom_Event( obj, Commands->Find_Object(humvee_id), MX0_VEHICLE_MOVE, 1, 0.0f);
				// Medium Tank Blasted
				Commands->Send_Custom_Event( obj, Commands->Find_Object(medium_tank_blasted_id), MX0_VEHICLE_MOVE, 1, 0.0f);
				// Medium Tank Escort
				Commands->Send_Custom_Event( obj, Commands->Find_Object(medium_tank_escort_id), MX0_VEHICLE_MOVE, 1, 0.0f);
				// GDI Trooper 1
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper1_id), MX0_SOLDIER_MOVE, 1, 0.0f);
				// Relocate GDI Trooper2 & GDI Trooper3
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SOLDIER_MOVE, 1, 0.0f);
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper3_id), MX0_SOLDIER_MOVE, 1, 0.0f);
					
				// Light Tank A
				GameObject * light_tank_a = Commands->Find_Object(light_tank_a_id);
				Commands->Attach_Script(light_tank_a, "MX0_Vehicle_DLS", "1500033, 1500034, 0, 0, 0.4f");

				// Light Tank B
				GameObject * light_tank_b = Commands->Find_Object(light_tank_b_id);
				Commands->Attach_Script(light_tank_b, "MX0_Vehicle_DLS", "0, 0, 0, 0, 0.0f");

				// Mobile Artillery
				GameObject * mobile_artillery = Commands->Find_Object(mobile_artillery_id);
				Commands->Attach_Script(mobile_artillery, "MX0_Vehicle_DLS", "1500084, 1500085, 0, 0, 1.0f");

				// Move Nod tanks into position on far side of river
				Commands->Start_Timer (obj, this, 2.0f, HUMMVEE_DESTRUCTION);

				// Humvee is now vulnerable
				Commands->Send_Custom_Event( obj, Commands->Find_Object(humvee_id), M00_ENABLE_DAMAGE_MOD, 0, 0.0f);

				// RocketTrooper sees Obelisk
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper1_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_SEES_OBELISK, 4.0f);

				
			}
			else
			{
				if(!say_need_here)
				{
					say_need_here = true;
					// We need you up here, Sir.
					Commands->Send_Custom_Event( obj, Commands->Find_Object(humvee_id), MX0_SPECIFIC_ACTION, MX0_NEED_YOU_HERE, 0.0f);
				}
				// "We need you Commando"
				Commands->Start_Timer (obj, this, 2.0f, AREA4_ACTIVATED);
			}
		}
		if(timer_id == HUMMVEE_DESTRUCTION)
		{
			if(star_area > 1)  // Area 2 or Area 3
			{
				
				// Light Tank A
				Commands->Send_Custom_Event( obj, Commands->Find_Object(light_tank_a_id), MX0_VEHICLE_MOVE, 1, 0.0f);
				
				// Obelisk blasts Hummvee
				Commands->Send_Custom_Event( obj, Commands->Find_Object(1500020), MX0_OBELISK_HUMVEE, 1, 0.0f);

				// *** Adjusted 12/12/01 Medium Tank no longer destroyed, run through function anyways
				// Obelisk blasts GDI Medium Tank
				Commands->Start_Timer (obj, this, 0.0f, MEDIUM_TANK_DESTRUCTION);

				// RocketTrooper - The Obelisk is hot!  Look out!
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper1_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_HOT_OBELISK, 4.0f);

				// Relocate Troops
				Relocate_Soldiers (obj);
			}
			else
			{
				// We need your help - over here, Sir!
				Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_CANT_TURN_BACK, 0.0f);
				// "We need you Commando"
				Commands->Start_Timer (obj, this, 6.0f, HUMMVEE_DESTRUCTION);
			}
	
		}
		if(timer_id == MEDIUM_TANK_DESTRUCTION)
		{
			switch(star_area)
			{
			case 0:
			case 1:
				{
					Wrong_Way(obj);
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, MEDIUM_TANK_DESTRUCTION);
				}
				break;
			case 2:
				{
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, MEDIUM_TANK_DESTRUCTION);
				}
				break;
			case 3:
				{
					// Activate Wall Nod Rocket Soldiers
					Commands->Enable_Spawner(1500042, true);
					Commands->Enable_Spawner(1500044, true);
					// Activate Wall Gun Emplacement
					Commands->Send_Custom_Event( obj, Commands->Find_Object(1500039), MX0_SPECIFIC_ACTION, 1, 0.0f);

					// Obelisk blasts GDI Medium Tank  **** Ajdusted 12/12/01 No longer destroy tank
			//		Commands->Send_Custom_Event( obj, Commands->Find_Object(1500020), MX0_OBELISK_MEDIUM_TANK, 2, 0.0f);
					
					// Relocate Medium Tank Escort
					Commands->Send_Custom_Event( obj, Commands->Find_Object(medium_tank_escort_id), MX0_VEHICLE_MOVE, 2, 0.0f);
					// Mobile Artillery
					Commands->Send_Custom_Event( obj, Commands->Find_Object(mobile_artillery_id), MX0_VEHICLE_MOVE, 1, 0.0f);
					
					// Orca is shot down with SAMs, slams into Obelisk
					Commands->Start_Timer (obj, this, 8.0f, OBELISK_DESTRUCTION);

					// Begin reinforcing GDI via Chinook
					// Additional GDI troop drop
					GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(Commands->Find_Object(1500102)));
					Commands->Set_Facing(chinook_obj2, Commands->Get_Facing(Commands->Find_Object(1500102)));
					Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "MX0_GDI_Reinforce_Area4.txt");

					// Relocate Troops
					Relocate_Soldiers (obj);
				}
				break;
			}
			
		}
		if(timer_id == OBELISK_DESTRUCTION)
		{
			switch(star_area)
			{
			case 0:
			case 1:
				{
					Wrong_Way(obj);
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, OBELISK_DESTRUCTION);
				}
				break;
			case 2:
				{
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, OBELISK_DESTRUCTION);
				}
				break;
			case 3:
				{
					// Orca Mix
					Commands->Create_2D_Sound("MX0_A4_Orca_Mix");
					// Cinematic of Orca
					// Shot down by SAMs, slams into Obelisk
					GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
					Commands->Set_Facing(controller, 0.000f);
					Commands->Attach_Script(controller, "Test_Cinematic", "X0E_Obelisk.txt");
					
					// Remove Obelisk weapon
					Commands->Destroy_Object(Commands->Find_Object(obelisk_weapon_id));
					
					// Orcas sweeping in to take out Obelisk
					Commands->Start_Timer (obj, this, 5.0f, ORCA_STRIKE1);
					Commands->Start_Timer (obj, this, 10.0f, ORCA_STRIKE2);
					orca_strike = true;
					
					// Commando, take out those SAMs
					Commands->Start_Timer (obj, this, 8.0f, CONVERSATION_HAVOC_TAKE_OUT_SAMS);
					// Nod soldiers being shot off balcony
					Commands->Start_Timer (obj, this, 2.0f, SAMS_DESTRUCTION);
					
					// Relocate Troops
					Relocate_Soldiers (obj);
				}
				break;
			}
			
		}
		// Commando, take out those SAMs
		if(timer_id == CONVERSATION_HAVOC_TAKE_OUT_SAMS)
		{
			// Havoc, you�ve got  to clear out those SAM sites!
			const char *conv_name = ("MX0_A04_CON005");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);
			// RocketTrooper - It�s down! The Obelisk is down!
			Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_OBELISK_DOWN, 3.0f);
		}
		if(timer_id == SAMS_DESTRUCTION)
		{
			switch(star_area)
			{
			case 0:
			case 1:
				{
					Wrong_Way(obj);
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, SAMS_DESTRUCTION);
				}
				break;
			case 2:
				{
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, SAMS_DESTRUCTION);
				}
				break;
			case 3:
				{
					// If the SAMs still exist get Havoc to take them out
					if(Commands->Find_Object(1500015) || Commands->Find_Object(1500016))
					{
						// Orcas sweeping in to take out Obelisk
						Commands->Start_Timer (obj, this, 5.0f, ORCA_STRIKE1);
						Commands->Start_Timer (obj, this, 10.0f, ORCA_STRIKE2);
						orca_strike = false;
						
						// Commando, take out those SAMs ** Only say it once
						if(!say_take_out_sams)
						{
							say_take_out_sams = true;
							// RocketTrooper - Go! Go!  Knock out those SAMs!
							Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_TAKE_SAMS, 9.0f);
							// Send custom to disable damage modifier on SAMs
							Commands->Send_Custom_Event( obj, Commands->Find_Object(1500015), M00_ENABLE_DAMAGE_MOD, 0, 0.0f);
							Commands->Send_Custom_Event( obj, Commands->Find_Object(1500016), M00_ENABLE_DAMAGE_MOD, 0, 0.0f);
							// Start timer to take out SAMs regardless of player intervention
							Commands->Start_Timer (obj, this, 20.0f, DESTROY_SAM1);
							Commands->Start_Timer (obj, this, 24.0f, DESTROY_SAM2);
						}

						// Nod soldiers being shot off balcony
						Commands->Start_Timer (obj, this, 5.0f, SAMS_DESTRUCTION);
					}
					// If SAMs no longer exist, trigger A10 strike
					else
					{
						//Nice job Havoc
						// RocketTrooper - The SAMs are history!
						Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper2_id), MX0_SPECIFIC_ACTION, MX0_ROCKETTROOPER_SAMS_HISTORY, 0.0f);
						// Trooper1 - There, on the wall!
						Commands->Send_Custom_Event( obj, Commands->Find_Object(gdi_trooper1_id), MX0_SPECIFIC_ACTION, MX0_TROOPER_UP_ON_WALL, 2.0f);
						// Nod RocketSoldier on base wall fire at Havoc's tank
						Commands->Send_Custom_Event (obj, Commands->Find_Object(nod_base_rocketsoldier_a_id), MX0_SPECIFIC_ACTION, player_tank_id, 0.0f);
						Commands->Send_Custom_Event (obj, Commands->Find_Object(nod_base_rocketsoldier_b_id), MX0_SPECIFIC_ACTION, player_tank_id, 0.0f);
						
						// A10 strike and destruction
						Commands->Start_Timer (obj, this, 5.0f, A10_STRIKE);
					}
					
				}
				break;
			}
			
		}
		if(timer_id == DESTROY_SAM1)
		{
			Commands->Apply_Damage(Commands->Find_Object(1500015), 50000.0f, "STEEL", NULL);
		}
		if(timer_id == DESTROY_SAM2)
		{
			Commands->Apply_Damage(Commands->Find_Object(1500016), 50000.0f, "STEEL", NULL);
		}

		if(timer_id == A10_STRIKE)
		{
			switch(star_area)
			{
			case 0:
			case 1:
				{
					Wrong_Way(obj);
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, A10_STRIKE);
				}
				break;
			case 2:
				{
					// Shouts from units calling Havoc back
					Commands->Start_Timer (obj, this, 5.0f, A10_STRIKE);
				}
				break;
			case 3:
				{
					// Start A10 - Outro muzak
					Commands->Fade_Background_Music( "Renegade_A10_Outro.mp3", 1, 1);
					// A10 - This is Eagle Claw 1 �Starting  attack run
					const char *conv_name = ("MX0_A04_CON010");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(NULL, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					// A10 - I�m hit! I�m hit!
					Commands->Start_Timer (obj, this, 5.0f, A10_HIT);
					// A10 cinematic
					GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
					Commands->Set_Facing(controller, 0.000f);
					Commands->Attach_Script(controller, "Test_Cinematic", "X0D_A10_Crash.txt");
					
					// Apaches rise up from behind wall, Ion cannon strike
					Commands->Start_Timer (obj, this, 6.0f, ION_CANNON_STRIKE);

					// Relocate Troops
					Relocate_Soldiers (obj);
				}
				break;
			}
			
		}
		// A10 - I�m hit! I�m hit!
		if(timer_id == A10_HIT)
		{
			const char *conv_name = ("MX0_A04_CON011");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);

		}
		if(timer_id == ION_CANNON_STRIKE)
		{
			// We have a lock on that base
			// This is Eagle Base.  I�m not risking any more pilots.
			const char *conv_name = ("MX0_A04_CON012");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);
			// Ion Cannon strike
			GameObject * ion_cannon_strike = Commands->Create_Object("Nod_RocketSoldier_1Off", Commands->Get_Position(Commands->Find_Object(1500087)));
			Commands->Attach_Script(ion_cannon_strike, "MX0_Plant_Ion_Beacon_DLS", "");
			Commands->Start_Timer (obj, this, 22.0f, FLASH_TO_WHITE);
			Commands->Start_Timer (obj, this, 25.0f, FINALE);

			// Relocate Troops
			Relocate_Soldiers (obj);
			
		}
		if(timer_id == FLASH_TO_WHITE)
		{
			Commands->Set_Screen_Fade_Color(1.0f, 1.0f, 1.0f, 0.2f);
			Commands->Set_Screen_Fade_Opacity(1.0f, 0.2f);
		
		}
		if(timer_id == FINALE)
		{
			Commands->Destroy_Object(Commands->Find_Object(basewall_id));
			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(controller, 0.000f);
			Commands->Attach_Script(controller, "Test_Cinematic", "X0Z_Finale.txt");
		}
		if(timer_id == ORCA_STRIKE1 && orca_strike)
		{
			GameObject *orca_controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(orca_controller, 0.000f);
			Commands->Attach_Script(orca_controller, "Test_Cinematic", "X0C_Flyovers_01.txt");
		}
		if(timer_id == ORCA_STRIKE2 && orca_strike)
		{
			GameObject *orca_controller = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(orca_controller, 0.000f);
			Commands->Attach_Script(orca_controller, "Test_Cinematic", "X0C_Flyovers_02.txt");
		}
		
	}

};

DECLARE_SCRIPT (MX0_Area4_Zone_DLS, "Area=0:int")
{
	bool first_time;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( first_time, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		first_time = true;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer) && first_time )
		{
			int area = Get_Int_Parameter("Area");
			// Send custom to area controller to determine STAR location
			Commands->Send_Custom_Event( obj, Commands->Find_Object(1500017), MX0_STAR_AREA, area, 0.0f);

			first_time = false;
		}
	}
};

DECLARE_SCRIPT (MX0_Vehicle_DLS, "Attack_Loc0=0:int, Attack_Loc1=0:int, Attack_Loc2=0:int, Attack_Loc3=0:int, Speed=1.0:float")
{
	int attack_loc [4];
	int loc;
	float speed;

	enum{CON001, CON002};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attack_loc, 1 );
		SAVE_VARIABLE( loc, 2 );
		SAVE_VARIABLE( speed, 3 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		Commands->Enable_Engine(obj, true);

		attack_loc [0] = Get_Int_Parameter("Attack_Loc0");
		attack_loc [1] = Get_Int_Parameter("Attack_Loc1");
		attack_loc [2] = Get_Int_Parameter("Attack_Loc2");
		attack_loc [3] = Get_Int_Parameter("Attack_Loc3");

		loc = 0;
		speed = Get_Float_Parameter("Speed");

		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
		params.Set_Attack(Commands->Find_Object(1500024), 0.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		Commands->Debug_Message("ID %d sees Enemy ID %d \n", Commands->Get_ID(obj), Commands->Get_ID(enemy));
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
		params.Set_Attack(enemy, 200.0f, 5.0f, true);
	//	params.AttackCheckBlocked = false;
	//	params.AttackForceFire = true;
					
		Commands->Action_Attack(obj, params);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /*sender*/ ) override
	{
		ActionParamsStruct params;

		if(type == MX0_VEHICLE_MOVE)
		{
			loc = param;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
			params.Set_Attack(Commands->Find_Object(1500024), 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
			Commands->Debug_Message("Attack_Loc [%d] = %d \n", loc, attack_loc[loc]);					
			
		}
		if(type == MX0_SPECIFIC_ACTION)
		{
			// Humvee announces discovery of Nod Base
			if(param == MX0_DISCOVERS_NOD_BASE)  
			{
				// Eagle Base� We found it!
				const char *conv_name = ("MX0_A04_CON001");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, CON001);
				Commands->Monitor_Conversation (obj, conv_id);		
			}
			// We need you up here, Sir.
			if(param == MX0_NEED_YOU_HERE)  
			{
				// We need you up here, Sir.
				const char *conv_name = ("MX0_A04_CON013");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 10);
					
			}
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if((action_id == CON001) && (reason == ACTION_COMPLETE_CONVERSATION_ENDED))
		{
			// Confirmed.  Excellent work!
			const char *conv_name = ("MX0_A04_CON002");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, CON002);
		//	Commands->Monitor_Conversation (obj, conv_id);	
		}
		if((action_id == CON002) && (reason == ACTION_COMPLETE_CONVERSATION_ENDED))
		{
			// They have an Obelisk!
			const char *conv_name = ("MX0_A04_CON003");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, CON002);
			
			
		}
		if(action_id == 10)
		{
			Commands->Debug_Message("Action_Complete");
		}
		
	}
};

DECLARE_SCRIPT (MX0_Obelisk_Weapon_DLS, "Max_Range=75.0f:float")
{
	bool able_to_fire;
	int current_target;
	int powerup_effect_id;
	int humvee_id;
	int humvee_driver_id;
	int medium_tank_blasted_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (able_to_fire, 1);
		SAVE_VARIABLE (current_target, 2);
		SAVE_VARIABLE (powerup_effect_id, 3);
		SAVE_VARIABLE (humvee_id, 4);
		SAVE_VARIABLE (humvee_driver_id, 5);
		SAVE_VARIABLE (medium_tank_blasted_id, 6);
	}

	void Created (GameObject * obj) override
	{
		powerup_effect_id = 0;
		current_target = 0;
		able_to_fire = true;
		Commands->Set_Player_Type (obj, SCRIPT_PLAYERTYPE_NOD);
		Commands->Set_Is_Rendered (obj, false);
	//	Commands->Enable_Enemy_Seen (obj, true);
		Commands->Enable_Hibernation (obj, false);
	//	Commands->Innate_Enable (obj);
		Vector3 my_position = Commands->Get_Position (obj);
		GameObject * effect = Commands->Create_Object ("Obelisk Effect", my_position);
		if (effect)
		{
			powerup_effect_id = Commands->Get_ID (effect);
			Commands->Set_Animation_Frame (effect, "OBL_POWERUP.OBL_POWERUP", 0);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if(type == MX0_OBELISK_HUMVEE)
		{
			Commands->Send_Custom_Event (obj, obj, 2, humvee_id, 0.0f);
		}
		if(type == MX0_OBELISK_DRIVER)
		{
			Commands->Send_Custom_Event (obj, obj, 2, humvee_driver_id, 1.0f);
		}
		if(type == MX0_OBELISK_MEDIUM_TANK)
		{
			Commands->Send_Custom_Event (obj, obj, 2, medium_tank_blasted_id, 0.0f);
		}

		// Actor IDs
		if(type == M00_SEND_OBJECT_ID)
		{
			if(param == 1)
			{
				humvee_id = Commands->Get_ID(sender);				
			}
			if(param == 2)
			{
				humvee_driver_id = Commands->Get_ID(sender);				
			}
			if(param == 3)
			{
				medium_tank_blasted_id = Commands->Get_ID(sender);				
			}
		}
		if (type == 1)
		{
			if (param == 0)
			{
				Commands->Enable_Enemy_Seen (obj, false);
				able_to_fire = false;
				ActionParamsStruct params;
				params.Set_Basic (this, 100, 0);
				params.Set_Attack (obj, 0.0f, 0.0f, true);
				Commands->Action_Attack (obj, params);
				Commands->Action_Reset (obj, 100.0f);
				Destroy_Obelisk_Effect ();
			}
			else
			{
				Commands->Enable_Enemy_Seen (obj, true);
				able_to_fire = true;
			}
		}
		else if (type == 2)
		{
			if (able_to_fire)
			{
				GameObject * target_obj = Commands->Find_Object (param);
				if (target_obj)
				{
					Vector3 enemy_position = Commands->Get_Position (target_obj);
					Vector3 my_position = Commands->Get_Position (obj);
					float distance = Commands->Get_Distance (my_position, enemy_position);
					enemy_position.Z = 0.0f;
					my_position.Z = 0.0f;
					float difference = Commands->Get_Distance (my_position, enemy_position);
					float max_range = Get_Float_Parameter("Max_Range");
					if ((difference > 15.0f) && (distance < max_range))
					{
						current_target = param;
						able_to_fire = false;
						Commands->Start_Timer (obj, this, 2.5f, 1);
						GameObject * effect = Commands->Find_Object (powerup_effect_id);
						if (effect)
						{
							Commands->Set_Animation_Frame (effect, "OBL_POWERUP.OBL_POWERUP", 1);
						}
						my_position.Z -= 20.0f;
						Commands->Create_Sound ("Obelisk_Warm_Up", my_position, obj);
					}
					else
					{
						Destroy_Obelisk_Effect ();
						Commands->Action_Reset (obj, 100.0f);
					}
				}
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			GameObject * target_obj = Commands->Find_Object (current_target);
			if (target_obj)
			{
				Vector3 my_position = Commands->Get_Position (obj);
				Vector3 enemy_position = Commands->Get_Position (target_obj);
				float distance = Commands->Get_Distance (my_position, enemy_position);
				enemy_position.Z = 0.0f;
				my_position.Z = 0.0f;
				float difference = Commands->Get_Distance (my_position, enemy_position);
				float max_range = Get_Float_Parameter("Max_Range");
				if ((difference > 15.0f) && (distance < max_range))
				{
					ActionParamsStruct params;
					params.Set_Basic (this, 100, 0);
					params.Set_Attack (target_obj, max_range, 0.0f, true);
					params.AttackCheckBlocked = false;
					Commands->Action_Attack (obj, params);
					current_target = 0;
					Commands->Start_Timer (obj, this, 2.0f, 2);
				}
				else
				{
					Destroy_Obelisk_Effect ();
					Commands->Action_Reset (obj, 100.0f);
					able_to_fire = true;
				}
			}
			else
			{
				Destroy_Obelisk_Effect ();
				Commands->Action_Reset (obj, 100.0f);
				able_to_fire = true;
			}
		}
		else if (timer_id == 2)
		{
			Destroy_Obelisk_Effect ();
			able_to_fire = true;
		}
	}

	void Action_Complete(GameObject * /*obj*/, int action_id, ActionCompleteReason /*reason*/) override
	{
		if(action_id == 0)
		{
			Commands->Debug_Message("Action_Complete Obelisk Firing");
		}
	}

	void Destroy_Obelisk_Effect (void)
	{
		if (powerup_effect_id)
		{
			GameObject * effect = Commands->Find_Object (powerup_effect_id);
			if (effect)
			{
				Commands->Set_Animation_Frame (effect, "OBL_POWERUP.OBL_POWERUP", 0);
			}
		}
	}

	void Destroyed(GameObject * /*obj*/ ) override
	{
		if (powerup_effect_id)
		{
			GameObject * effect = Commands->Find_Object (powerup_effect_id);
			if (effect)
			{
				Commands->Set_Animation_Frame (effect, "OBL_POWERUP.OBL_POWERUP", 0);
			}
		}
	}
};

DECLARE_SCRIPT (MX0_GDI_Killed_DLS, "Unit_ID=0:int")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( attack_loc, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		// If damaged by the Obelisk Weapon
		if((damager == Commands->Find_Object(1500020)) && (Commands->Find_Object(1500020)))
		{
			Commands->Apply_Damage( obj, 50000.0f, "STEEL", NULL);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		switch(unit_id)
		{
		case 1:
			{
				GameObject * humvee_driver = Commands->Create_Object("GDI_Minigunner_1Off", Commands->Get_Position(obj));
				Commands->Set_Facing(humvee_driver, 0.000f);
				Commands->Attach_Script(humvee_driver, "M00_Send_Object_ID", "1500020, 2, 0.0f");
				Commands->Attach_Script(humvee_driver, "MX0_GDI_Soldier_DLS", "0,0,0,0,0.0f");
				// Custom for Humvee driver to be covwering and fleeing
				Commands->Send_Custom_Event (obj, humvee_driver, MX0_SPECIFIC_ACTION, MX0_HUMVEE_DRIVER_COWER, 0.0f);
				// Obelisk blasts Hummvee
				Commands->Send_Custom_Event( obj, Commands->Find_Object(1500020), MX0_OBELISK_DRIVER, 1, 1.0f);
			}
			break;
		// When Medium Tank is destroyed replace with destroyed model
		case 2:
			{
				GameObject * destroyed_medium_tank = Commands->Create_Object("MX0_GDI_Medium_Tank_Destroyed", Commands->Get_Position(obj));
				Commands->Set_Facing(destroyed_medium_tank, Commands->Get_Facing(obj));
			}
			break;
		// GDI Reinforcement, send custom to controller on killed
		case 3:
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object(1500017), MX0_GDI_REINFORCEMENT_KILLED, 1, 1.0f);
			}
			break;
		}
	}
};

DECLARE_SCRIPT (MX0_GDI_Soldier_DLS, "Attack_Loc0=0:int, Attack_Loc1=0:int, Attack_Loc2=0:int, Attack_Loc3=0:int, Speed=1.0:float")
{
	int attack_loc [4];
	int loc;
	float speed;
	int mx0_soldier_move_attempts;

	enum{CON001};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attack_loc, 1 );
		SAVE_VARIABLE( loc, 2 );
		SAVE_VARIABLE( speed, 3 );
		SAVE_VARIABLE( mx0_soldier_move_attempts, 4 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		Commands->Enable_Engine(obj, true);

		attack_loc [0] = Get_Int_Parameter("Attack_Loc0");
		attack_loc [1] = Get_Int_Parameter("Attack_Loc1");
		attack_loc [2] = Get_Int_Parameter("Attack_Loc2");
		attack_loc [3] = Get_Int_Parameter("Attack_Loc3");

		loc = 0;
		speed = Get_Float_Parameter("Speed");

		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
		params.Set_Attack(Commands->Find_Object(1500024), 0.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		Commands->Debug_Message("ID %d sees Enemy ID %d \n", Commands->Get_ID(obj), Commands->Get_ID(enemy));
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
		params.Set_Attack(enemy, 200.0f, 5.0f, true);
	//	params.AttackCheckBlocked = false;
	//	params.AttackForceFire = true;
					
		Commands->Action_Attack(obj, params);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /*sender*/ ) override
	{
		ActionParamsStruct params;

		if(type == MX0_SOLDIER_MOVE)
		{
			loc = param;
			bool move_crouched = Commands->Get_Random_Int(0, 1) ? false : true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, MX0_SOLDIER_MOVE );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
			params.Set_Attack(Commands->Find_Object(1500024), 0.0f, 0.0f, true);
			params.MoveCrouched = move_crouched;
			Commands->Action_Attack(obj, params);
			Commands->Debug_Message("Attack_Loc [%d] = %d \n", loc, attack_loc[loc]);					
			
		}
		if(type == MX0_SPECIFIC_ACTION)
		{
			// Soldier that pops out of destroyed Humvee
			if(param == MX0_HUMVEE_DRIVER_COWER)  // Cower animation
			{
				Commands->Innate_Disable(obj);

				ActionParamsStruct params;

				params.Set_Basic( this, 100, 1 );
				params.Set_Animation ("H_A_A0A0_L51", true);
				Commands->Action_Play_Animation (obj, params);
				
				// Death #1 - Bullet death scream
				const char *conv_name = ("MX0_A04_CON019");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 10);	
			}
			// Rocket Trooper sights obelisk
			if(param == MX0_ROCKETTROOPER_SEES_OBELISK) 
			{
				// They have an Obelisk!
				const char *conv_name = ("MX0_A04_CON003");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_SEES_OBELISK);	
				
				// Start Area4 muzak
				Commands->Fade_Background_Music( "Level 0 Nod base.mp3", 2, 2);
			
			}
			// Rocket Trooper announces obelisk is hot
			if(param == MX0_ROCKETTROOPER_HOT_OBELISK) 
			{
				// The Obelisk is hot!  Look out!
				const char *conv_name = ("MX0_A04_CON004");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_HOT_OBELISK);	
			}
			// Rocket Trooper - It�s down! The Obelisk is down!
			if(param == MX0_ROCKETTROOPER_OBELISK_DOWN) 
			{
				// It�s down! The Obelisk is down!
				const char *conv_name = ("MX0_A04_CON006");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_OBELISK_DOWN);	
			}
			// Rocket Trooper - Go! Go!  Knock out those SAMs!
			if(param == MX0_ROCKETTROOPER_TAKE_SAMS) 
			{
				// Go! Go!  Knock out those SAMs!
				const char *conv_name = ("MX0_A04_CON007");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_TAKE_SAMS);
			}
			// Rocket Trooper - The SAMs are history!
			if(param == MX0_ROCKETTROOPER_SAMS_HISTORY) 
			{
				// The SAMs are history!
				const char *conv_name = ("MX0_A04_CON008");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_SAMS_HISTORY);
			}
			// Trooper1 - There, on the wall!
			if(param == MX0_TROOPER_UP_ON_WALL) 
			{
				// There, on the wall!
				const char *conv_name = ("MX0_A04_CON009");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_TROOPER_UP_ON_WALL);
			}
			// If Havoc turns back and exits Area4
			if(param == MX0_ROCKETTROOPER_CANT_TURN_BACK) 
			{
				// We need your help - over here, Sir!
				const char *conv_name = ("MX0_A04_CON014");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, MX0_ROCKETTROOPER_CANT_TURN_BACK);
			}
					
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == MX0_SOLDIER_MOVE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, MX0_SOLDIER_MOVE );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), speed, 5.0f );
			params.Set_Attack(Commands->Find_Object(1500024), 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		// If action fails, reattempt
		if((action_id == MX0_SOLDIER_MOVE) && (reason != ACTION_COMPLETE_NORMAL) && (mx0_soldier_move_attempts < 4))
		{
			mx0_soldier_move_attempts++;
			Commands->Start_Timer (obj, this, 4.0f, MX0_SOLDIER_MOVE);

			Commands->Debug_Message("Attempt %d for MX0_SOLDIER_MOVE on loc %d");
		}
		if(action_id == 10)
		{
			Commands->Debug_Message("Action_Complete");
		}
	}
};

DECLARE_SCRIPT (MX0_Gun_Emplacement_DLS, "Left_Point=0:int, Right_Point=0:int")
{
	bool left;

	enum{STRAFE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		left = true;
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		ActionParamsStruct params;

		if(type == MX0_SPECIFIC_ACTION)
		{
			Timer_Expired(obj, STRAFE);
			
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == STRAFE)
		{
			if(left)
			{
				left = false;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
				params.Set_Attack (Commands->Find_Object(Get_Int_Parameter("Left_Point")), 0.0f, 0.0f, 1);
				params.AttackCheckBlocked = false;
				params.AttackForceFire = true;
				Commands->Action_Attack( obj, params );
			}
			else
			{
				left = true;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
				params.Set_Attack (Commands->Find_Object(Get_Int_Parameter("Right_Point")), 0.0f, 0.0f, 1);
				params.AttackCheckBlocked = false;
				params.AttackForceFire = true;
				Commands->Action_Attack( obj, params );
			}
			Commands->Start_Timer (obj, this, 4.0f, STRAFE);
		}		
	}
};

DECLARE_SCRIPT (MX0_Nod_RocketSoldier_DLS, "Stationary_Point=0:int")
{
	enum{GO_POSITION};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_POSITION );
		params.Set_Movement( Commands->Find_Object(Get_Int_Parameter("Stationary_Point")), RUN, 0.0f );
		Commands->Action_Goto(obj, params);


	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		ActionParamsStruct params;

		if(type == MX0_SPECIFIC_ACTION)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
			params.Set_Attack (Commands->Find_Object(param), 150.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			params.AttackForceFire = true;
			Commands->Action_Attack( obj, params );			
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if((action_id == GO_POSITION) && (reason == ACTION_COMPLETE_NORMAL))
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Attack (enemy, 150.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		params.AttackForceFire = true;
		Commands->Action_Attack( obj, params );
	}
};

DECLARE_SCRIPT (MX0_SAM_DLS, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);

	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		ActionParamsStruct params;

		if (type == MX0_FIRE_SAM)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(1500081), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}
};

DECLARE_SCRIPT (MX0_Nod_Bunker_DLS, "")
{
	enum{GO_POSITION};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);

	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Attack (enemy, 0.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		params.AttackCrouched = true;
		params.AttackForceFire = true;
		Commands->Action_Attack( obj, params );
	}
};

DECLARE_SCRIPT (MX0_Plant_Ion_Beacon_DLS, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Give_PowerUp(obj, "POW_IonCannonBeacon_Ai", false );
		Commands->Select_Weapon(obj, "Weapon_IonCannonBeacon_Ai" );

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Attack (Commands->Get_Position(obj), 5.0f, 0.0f, 1);
		params.AttackForceFire = true;
		Commands->Action_Attack( obj, params );
	}
};

DECLARE_SCRIPT (DLS_Star_No_Fall, "")
{
	enum{ATTACH_SCRIPT};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 1.0f, ATTACH_SCRIPT);
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if(timer_id == ATTACH_SCRIPT)
		{
			if(STAR)
			{
				Commands->Attach_Script(STAR, "M00_No_Falling_Damage_DME", "");
			}
			else
			{
				Commands->Start_Timer (obj, this, 1.0f, ATTACH_SCRIPT);
			}
		}
		
	}
};

DECLARE_SCRIPT (MX0_Explosive_Barrels_DLS, "Logical_Sound=0:int, Radius:float")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( left, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		
	}

	void Custom (GameObject * /*obj*/, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if(sound.Type == Get_Int_Parameter("Logical_Sound"))
		{
			Commands->Apply_Damage( obj, 50000.0f, "STEEL", NULL);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		float radius = Get_Float_Parameter("Radius");
		Commands->Create_Explosion("Air Explosions Twiddler", Commands->Get_Position(obj), obj);
		Commands->Create_Logical_Sound(obj, Get_Int_Parameter("Logical_Sound"), Commands->Get_Position(obj), radius);
	}
};



