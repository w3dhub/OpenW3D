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
*     Test_RMV.cpp
*
* DESCRIPTION
*     Test code location
*
* PROGRAMMER
*     Ryan Vervack
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 154 $
*     $Modtime: 11/29/01 11:05a $
*     $Archive: /Commando/Code/Scripts/Test_RMV.cpp $
*
******************************************************************************/

#include <cstdint>
#include "scripts.h"
#include "Toolkit.h"
#include "wwmath.h"
#include <string.h>
#include <stdio.h>


#include "Mission3.h"

DECLARE_SCRIPT(RMV_Test_Script, "")
{
	void Sound_Heard(GameObject * /*obj*/, const CombatSound & /*sound*/) override
	{
		Commands->Debug_Message("Sound heard.\n");
	}
};

DECLARE_SCRIPT(M00_C130_Dropoff_RMV, "ObjToCreate=:string")
{
	enum {M00_TIMER_DROP_OBJECT_RMV};
	
	void Created( GameObject * obj ) override
	{
		Commands->Set_Animation ( obj, "V_NOD_C-130E3.M_cargo-drop", 0 , NULL, 0.0f, -1.0f, false);
		int drop_frame;
		drop_frame = 460;
		float drop_time;
		drop_time= ( float )drop_frame / 30.0;
		Commands->Start_Timer( obj, this, drop_time, M00_TIMER_DROP_OBJECT_RMV );
	}

	void Killed(GameObject * obj, GameObject * killer) override
	{
		Commands->Create_Explosion_At_Bone("C-130 Explosion 01", obj, "BODYMAIN", killer);
	}

	void Timer_Expired ( GameObject* obj, int Timer_ID ) override
	{
		if ( Timer_ID == M00_TIMER_DROP_OBJECT_RMV )
		{
			GameObject *object;
			object = Commands->Create_Object_At_Bone( obj, Get_Parameter( "ObjToCreate" ), "CARGO" );
			if (object)
			{
				//This is where you can manipulate the object, attach scripts, etc.
			}
		}
	}

	void Animation_Complete(GameObject * obj, const char * /*anim*/) override
	{
		Commands->Destroy_Object(obj);
	}
};

DECLARE_SCRIPT(M00_Commando_Death_Taunt, "")
{
	void Killed(GameObject * /*obj*/, GameObject * killer) override
	{
		if (Commands->Is_A_Star(killer))
		{
			float random = Commands->Get_Random(1, 20);
			switch (int(random))
			{
			case 1: Commands->Create_Sound("tuffguy1", Vector3(0,0,0), killer);
				break;
			case 5: Commands->Create_Sound("laugh1", Vector3(0,0,0), killer);
				break;
			case 10: Commands->Create_Sound("keepem1", Vector3(0,0,0), killer);
				break;
			case 15: Commands->Create_Sound("lefty1", Vector3(0,0,0), killer);
				break;
			}
		}
	}
};

DECLARE_SCRIPT(M00_Damaged_Warning, "")
{
	bool just_sent;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( just_sent, 1 );
	}
	
	void Created(GameObject * /*obj*/) override
	{
		just_sent = false;
	}
	
	void Damaged(GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		if ((Commands->Get_Health(obj) <= 25.0f) && (!just_sent))
		{
			Commands->Create_Sound("00-N112E", Vector3(0,0,0), obj);
		/*	Commands->Set_Display_Color(255,0,0);
			Commands->Display_Text(IDS_M00_HEALTH_WARNING);
			Commands->Set_Display_Color(); */
			just_sent = true;
			Commands->Start_Timer(obj, this, 30.0f, 0);
		}
	}

	void Timer_Expired(GameObject * /*obj*/, int /*timer_id*/) override
	{
		just_sent = false;
	}
};

DECLARE_SCRIPT(M00_Put_Script_On_Commando, "")
{
	enum {SCRIPT_ON_COMMANDO_TIMER};
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 0.2f, SCRIPT_ON_COMMANDO_TIMER);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == SCRIPT_ON_COMMANDO_TIMER)
		{
			GameObject *star;
			star = Commands->Get_A_Star(Commands->Get_Position(obj));
			if (star)
			{
				Commands->Attach_Script(star, "M00_Damaged_Warning", "");
			}
			else
			{
				Commands->Start_Timer(obj, this, 1.0f, SCRIPT_ON_COMMANDO_TIMER);
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 12176) && (param == 12176))
		{
			Commands->Start_Timer(obj, this, 2.0f, SCRIPT_ON_COMMANDO_TIMER);
		}
	}
};

DECLARE_SCRIPT(RMV_Hostage_Rescue_Point, "")
{
	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if (!Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, enterer, 999, 999, 0.0f);
		}
	}
};

DECLARE_SCRIPT(RMV_Trigger_Killed, "ID:int, Type:int, Param:int")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
	//	if (Commands->Is_A_Star(killer))
		{
			GameObject * target = Commands->Find_Object(Get_Int_Parameter("ID"));
			int type = Get_Int_Parameter("Type");
			int param = Get_Int_Parameter("Param");
			Commands->Send_Custom_Event(obj, target, type, param, 0.0f);
		}
	}
};

DECLARE_SCRIPT(RMV_Home_Point, "Radius:float")
{
	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), Get_Float_Parameter("Radius"));
	}
};

DECLARE_SCRIPT(RMV_Test_Facing, "")
{
	void Damaged(GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		Vector3 my_pos = Commands->Get_Position(obj);
		Vector3 target_pos = Commands->Get_Position(damager);

		float angle = WWMath::Atan2((my_pos.X - target_pos.X), (my_pos.Y - target_pos.Y));
		Commands->Set_Facing(obj, -90.0f - RAD_TO_DEGF(angle));
	}
};

DECLARE_SCRIPT(RMV_Camera_Behavior, "Angle:float, Alarm_ID=0:int, Is_Gun=0:int, Delay=0.0:float")
{
	int switcher, enemy_id, sound_id;
	bool enemy_seen, timer_expired, alert, is_gun, attacking;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(switcher, 1);
		SAVE_VARIABLE(enemy_seen, 2);
		SAVE_VARIABLE(enemy_id, 3);
		SAVE_VARIABLE(timer_expired, 4);
		SAVE_VARIABLE(alert, 5);
		SAVE_VARIABLE(sound_id, 6);
		SAVE_VARIABLE(is_gun, 7);
		SAVE_VARIABLE(attacking, 8);
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
		switcher = enemy_id = sound_id = 0;
		is_gun = (Get_Int_Parameter("Is_Gun") == 1) ? true : false;
		enemy_seen = timer_expired = alert = attacking = false;

		Commands->Enable_Enemy_Seen(obj, true);

		ActionParamsStruct params;
		params.Set_Basic(this, 90, 0);
		params.Set_Attack(Get_First_Target(), 0.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);
		Commands->Start_Timer(obj, this, Get_Int_Parameter(0) / 60.0f, 0);
	}

	void Resume(void)
	{
		
		enemy_id = 0;
		enemy_seen = timer_expired = alert = attacking = false;

		Commands->Action_Reset(Owner(), 100);
		switcher = 0;
	}

	void Alarm(void)
	{
		Commands->Stop_Sound(sound_id, true);
		
		if (Get_Int_Parameter("Alarm_ID") != 0)
		{
			GameObject * alarm = Commands->Find_Object(Get_Int_Parameter("Alarm_ID"));
			if (alarm)
			{
				Commands->Send_Custom_Event(Owner(), alarm, M00_CUSTOM_CAMERA_ALARM, enemy_id, 0.0f);
			}
		}
		
		enemy_id = 0;
		enemy_seen = timer_expired = alert = false;

		switcher = 0;
		Commands->Action_Reset(Owner(), 100);
	}

	void Fire(void)
	{
		Commands->Action_Reset(Owner(), 100);

		Commands->Stop_Sound(sound_id, true);

		attacking = true;
		
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 3);
		params.Set_Attack(Commands->Find_Object(enemy_id), 200.0f, 0.0f, true);
		Commands->Action_Attack(Owner(), params);
		
		Commands->Start_Timer(Owner(), this, 3.0f, 10);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 10)
		{
			GameObject * enemy = Commands->Find_Object(enemy_id);
			if (!Commands->Is_Object_Visible(obj, enemy))
			{
				enemy_id = 0;
				enemy_seen = timer_expired = alert = attacking = false;
				switcher = 0;
				Commands->Action_Reset(obj, 100);
			}
			else
			{
				Commands->Start_Timer(obj, this, 3.0f, 10);
			}
		}
		else if (timer_id == 1)
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
				is_gun ? Fire() : Alarm();
			}
		}
		else
		{
			++switcher %= 8;
	
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Attack(Get_Target(), 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this,  Get_Float_Parameter(0) / 120.0f, 0);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (reason != ACTION_COMPLETE_NORMAL)
		{
			return;
		}
		
		if (action_id == 3)
		{
			enemy_id = 0;
			enemy_seen = timer_expired = alert = attacking = false;
			switcher = 0;
			Commands->Action_Reset(obj, 100);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		if (!enemy_seen && !attacking)
		{
			Vector3 pos = Commands->Get_Position(obj);
			sound_id = Commands->Create_Sound("Beep", pos, obj);

			ActionParamsStruct params;
			params.Set_Basic(this, 99, 1);
			params.Set_Attack(enemy, 0.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
			
			enemy_id = Commands->Get_ID(enemy);
			enemy_seen = true;
			Commands->Start_Timer(obj, this, Get_Float_Parameter("Delay"), 1);
		}
		if (enemy_seen && timer_expired && (enemy_id == Commands->Get_ID(enemy)))
		{
			alert = true;
		}
	}

	void Killed(GameObject * /*obj*/, GameObject * /*killer*/) override
	{
		Commands->Stop_Sound(sound_id, true);
	}
};

DECLARE_SCRIPT(RMV_Cinematic_Position, "Bone:string")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		const char * bone = Get_Parameter("Bone");
		if (bone)
		{
			Vector3 pos = Commands->Get_Bone_Position(obj, bone);
			Commands->Debug_Message("Position of %s bone is (%3.2f, %3.2f, %3.2f).\n", bone, pos.X, pos.Y, pos.Z);
			Commands->Start_Timer(obj, this, 1.0f, 0);
		}
	}
};

DECLARE_SCRIPT(M00_Destroyed_Turret, "")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		Vector3 my_pos = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		GameObject * destroyed_turret = Commands->Create_Object("Nod_Turret_Destroyed", my_pos);
		Commands->Set_Facing(destroyed_turret, facing);
	}
};

DECLARE_SCRIPT(RMV_Engine_Sound, "Preset:string, Bone:string")
{
	int sound_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sound_id, 1);
	}
	
	void Created(GameObject * obj) override
	{
		const char * sound = Get_Parameter("Preset");
		const char * bone = Get_Parameter("Bone");
		sound_id = Commands->Create_3D_Sound_At_Bone(sound, obj, bone);
	}

	void Destroyed(GameObject * /*obj*/) override
	{
		Commands->Stop_Sound(sound_id, true);
	}
};

DECLARE_SCRIPT(RMV_Transport_Evac, "Number:int, Nod=0:int")
{
	int number;
	bool loading_complete;
	int traj_id, trans_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(number, 1);
		SAVE_VARIABLE(loading_complete, 2);
		SAVE_VARIABLE(traj_id, 3);
		SAVE_VARIABLE(trans_id, 4);
	}
	
	void Created(GameObject * obj) override
	{
		number = Get_Int_Parameter("Number");
		int nod = Get_Int_Parameter("Nod");
		loading_complete = false;

		GameObject * traj = Commands->Create_Object("Invisible_Object", Commands->Get_Position(obj));
		Commands->Set_Model(traj, "XG_TransprtBone");
		Commands->Set_Facing(traj, Commands->Get_Facing(obj));
		GameObject * transport = Commands->Create_Object("Invisible_Object", Commands->Get_Position(obj));
		Commands->Set_Model(transport, (nod == 1) ? "v_NOD_trnspt" : "v_GDI_trnspt");
		Commands->Set_Facing(transport, Commands->Get_Facing(obj));
		char traj_anim[40];
		sprintf(traj_anim, "XG_TransprtBone.XG_EV%d_PathA", number);
		char trans_anim[40];
		sprintf(trans_anim, "v_%s_trnspt.XG_EV%d_trnsA", (nod == 1) ? "NOD" : "GDI", number);
		Commands->Set_Animation(traj, traj_anim, false, NULL, 0.0f, -1.0f, false);
		Commands->Set_Animation(transport, trans_anim, false, NULL, 0.0f, -1.0f, false);
		Commands->Attach_To_Object_Bone(transport, traj, "BN_Trajectory");
		Commands->Start_Timer(obj, this, 280.0f / 30.0f, 0);

		traj_id = Commands->Get_ID(traj);
		trans_id = Commands->Get_ID(transport);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		GameObject * traj = Commands->Find_Object(traj_id);
		GameObject * transport = Commands->Find_Object(trans_id);

		int nod = Get_Int_Parameter("Nod");
		
		if (timer_id == 0)
		{
			char traj_anim[40];
			sprintf(traj_anim, "XG_TransprtBone.XG_EV%d_Pathloop", number);
			char trans_anim[40];
			sprintf(trans_anim, "v_%s_trnspt.XG_EV%d_trnsloop",(nod == 1) ? "NOD" : "GDI", number);
			Commands->Set_Animation(traj, traj_anim, false, NULL, 0.0f, -1.0f, false);
			Commands->Set_Animation(transport, trans_anim, false, NULL, 0.0f, -1.0f, false);
			Commands->Start_Timer(obj, this, 94.0f / 30.0f, 1);
		}
		else if (timer_id == 1)
		{
			if (loading_complete)
			{
				char traj_anim[40];
				sprintf(traj_anim, "XG_TransprtBone.XG_EV%d_PathZ", number);
				char trans_anim[40];
				sprintf(trans_anim, "v_%s_trnspt.XG_EV%d_trnsz", (nod == 1) ? "NOD" : "GDI", number);
				Commands->Set_Animation(traj, traj_anim, false, NULL, 0.0f, -1.0f, false);
				Commands->Set_Animation(transport, trans_anim, false, NULL, 0.0f, -1.0f, false);
				Commands->Start_Timer(obj, this, 231.0f / 30.0f, 2);
			}
			else
			{
				char traj_anim[40];
				sprintf(traj_anim, "XG_TransprtBone.XG_EV%d_Pathloop", number);
				char trans_anim[40];
				sprintf(trans_anim, "v_%s_trnspt.XG_EV%d_trnsloop", (nod == 1) ? "NOD" : "GDI", number);
				Commands->Set_Animation(traj, traj_anim, false, NULL, 0.0f, -1.0f, false);
				Commands->Set_Animation(transport, trans_anim, false, NULL, 0.0f, -1.0f, false);
				Commands->Start_Timer(obj, this, 94.0f / 30.0f, 1);
			}
		}
		else if (timer_id == 2)
		{
			Commands->Destroy_Object(traj);
			Commands->Destroy_Object(transport);
			Commands->Destroy_Object(obj);
		}
	}

	void Custom(GameObject * /*obj*/, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		loading_complete = true;
	}
};

DECLARE_SCRIPT(Poke_And_Play_Cinematic, "Text_File:string, Location=0 0 0:vector3")
{
	void Poked(GameObject * /*obj*/, GameObject * /*poker*/) override
	{
		Vector3 pos = Get_Vector3_Parameter("Location");
		const char * file = Get_Parameter("Text_File");
		GameObject * arrow = Commands->Create_Object("Invisible_Object", pos);
		Commands->Attach_Script(arrow, "Test_Cinematic", file);
	}
};

DECLARE_SCRIPT(M00_Ion_Cannon_Sound, "Number=0:int")
{
	void Created(GameObject * obj) override
	{
		int num = Get_Int_Parameter(0);
		Vector3 pos = Commands->Get_Position(obj);
		Commands->Create_Sound((num == 0) ? "Ion_Cannon_Buildup" : "Ion_Cannon_Fire", pos, obj);
	}
};

DECLARE_SCRIPT(RMV_Trigger_Poked_2, "Target:int, Type:int, Param:int")
{
	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		GameObject * target = Commands->Find_Object(Get_Int_Parameter("Target"));
		if (target)
		{
			Commands->Send_Custom_Event(obj, target, Get_Int_Parameter("Type"), Get_Int_Parameter("Param"), 0.0f);
		}
		Commands->Destroy_Object(obj);
	}
};

DECLARE_SCRIPT(RMV_Test_Damage, "")
{
	float total_damage;
	float max_shield;
	int count;

	void Created(GameObject * obj) override
	{
		total_damage = 0.0f;
		count = 0;
		max_shield = Commands->Get_Shield_Strength(obj);
	}
	
	void Damaged(GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		count++;
		float damage = Commands->Get_Max_Health(obj) - Commands->Get_Health(obj);
		Commands->Set_Health(obj, Commands->Get_Max_Health(obj));
		total_damage += damage;
		bool maxed;
		if (damage >= Commands->Get_Max_Health(obj))
		{
			maxed = true;
		}
		else
		{
			maxed = false;
		}
		Commands->Debug_Message("Object took %3.2f points of damage%s.\n", damage, maxed ? " or more" : "");
	}

	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		Commands->Debug_Message("Cumulative damage was %3.2f from %d sources.\n", total_damage, count);
		total_damage = 0.0f;
		count = 0;
		Commands->Set_Shield_Strength(obj, max_shield);
	}
};

DECLARE_SCRIPT(RMV_Test_Stealth, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Enable_Stealth(obj, true);
	}
};
