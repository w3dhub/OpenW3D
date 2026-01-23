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
*     Test_RMV_Toolkit.cpp
*
* DESCRIPTION
*     Additional Toolkit Scripts
*
* PROGRAMMER
*     Ryan Vervack
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 154 $
*     $Modtime: 11/03/01 11:18a $
*     $Archive: /Commando/Code/Scripts/Test_RMV_Toolkit.cpp $
*
******************************************************************************/

#include <cstdint>
#include "scripts.h"
#include <string.h>
#include <stdio.h>
#include "Toolkit.h"

enum {
	TECHNICIAN_MOVEMENT,
	TECHNICIAN_ANIMATION
};

DECLARE_SCRIPT(RMV_Trigger_Zone, "TargetID:int, Type:int, Param:int")
{
	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			int type = Get_Int_Parameter("Type");
			int param = Get_Int_Parameter("Param");
			GameObject *target = Commands->Find_Object(Get_Int_Parameter("TargetID"));
			if (target)
				Commands->Send_Custom_Event(obj, target, type, param, 0.0f);
			if (obj) 
				Commands->Destroy_Object(obj);
		}
	}
};

DECLARE_SCRIPT(RMV_Trigger_Zone_2, "TargetID:int, Type:int, Param:int")
{
	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			int type = Get_Int_Parameter("Type");
			int param = Get_Int_Parameter("Param");
			GameObject *target = Commands->Find_Object(Get_Int_Parameter("TargetID"));
			if (target)
				Commands->Send_Custom_Event(obj, target, type, param, 0.0f);
		}
	}
};

DECLARE_SCRIPT(RMV_Trigger_Poked, "Target_ID:int, Type:int, Param:int")
{
	void Poked(GameObject * obj, GameObject * poker) override
	{
		int type = Get_Int_Parameter("Type");
		int param = Get_Int_Parameter("Param");
		GameObject * target;
		if (Get_Int_Parameter("Target_ID") == 0)
		{
			target = poker;
		}
		else
		{
			target = Commands->Find_Object(Get_Int_Parameter("Target_ID"));
		}
		if (target)
		{
			Commands->Send_Custom_Event(obj, target, type, param, 0.0f);
		}
	}
};

DECLARE_SCRIPT(RMV_Engineer_Wander, "Custom_Type:int, Custom_Param_1:int, Custom_Param_2:int, Building_Number:int, Evac_Object=None:string")
{
	int c_type, c_param_1, c_param_2;
	int terminal_id, int_anim;
	bool busy, emergency, always_run, evacuating;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(c_type, 1);
		SAVE_VARIABLE(c_param_1, 2);
		SAVE_VARIABLE(c_param_2, 3);
		SAVE_VARIABLE(terminal_id, 4);
		SAVE_VARIABLE(int_anim, 5);
		SAVE_VARIABLE(busy, 6);
		SAVE_VARIABLE(emergency, 7);
		SAVE_VARIABLE(always_run, 8);
		SAVE_VARIABLE(evacuating, 9);
	}

	void Created(GameObject * /*obj*/) override
	{
		busy = emergency = always_run = evacuating = false;
		c_type = Get_Int_Parameter("Custom_Type");
		c_param_1 = Get_Int_Parameter("Custom_Param_1");
		c_param_2 = Get_Int_Parameter("Custom_Param_2");
	}

	void Sound_Heard(GameObject * obj, const CombatSound & sound) override
	{
		if (evacuating)
		{
			return;
		}
		
		if ((sound.Type == M00_SOUND_ENGINEER_WANDER) && (!busy))
		{
			emergency = false;
			if (always_run)
			{
				emergency = true;
			}
			Commands->Send_Custom_Event(obj, sound.Creator, c_type, c_param_1, 0.0f);
		}
		else if (sound.Type == M00_SOUND_ENGINEER_WANDER_EMERGENCY && !busy)
		{
			emergency = true;
			Commands->Send_Custom_Event(obj, sound.Creator, c_type, c_param_1, 0.0f);
		}
		else if (sound.Type == M00_SOUND_BUILDING_DESTROYED + Get_Int_Parameter("Building_Number"))
		{
			Evacuate();
		}
		else if (sound.Type == M00_SOUND_ALWAYS_RUN)
		{
			always_run = true;
		}
		else if (sound.Type == M00_SOUND_ALWAYS_RUN_OFF)
		{
			always_run = false;
		}
	}

	void Evacuate(void)
	{
		const char* evac = Get_Parameter("Evac_Object");
		Commands->Action_Reset(Owner(), 99);
		evacuating = true;
		
		if (stricmp(evac, "None") == 0)
		{
			return;
		}
		else
		{
			GameObject * my_evac_point = Commands->Find_Random_Simple_Object(evac);
			if (my_evac_point)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 99, 0);
				params.Set_Movement(my_evac_point, RUN, 2.0f);
				Commands->Action_Goto(Owner(), params);
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * sender)
	{
		if (type == c_type)
		{
			busy = true;
			terminal_id = Commands->Get_ID(sender);
			ActionParamsStruct params;
			params.Set_Basic(this, 70, TECHNICIAN_MOVEMENT);
			params.Set_Movement(sender, emergency ? RUN : WALK, 0.75f);
			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == TECHNICIAN_MOVEMENT)
		{
			if (reason != ACTION_COMPLETE_NORMAL)
				return;
			GameObject *terminal;
			terminal = Commands->Find_Object(terminal_id);
			if (terminal)
			{
				float facing = Commands->Get_Facing(terminal);
				Commands->Set_Facing(obj, facing + 180);
			}
			//#CFE_TODO uhh....
			const char *anim;
			anim = "s_a_human.h_a_con2";
			ActionParamsStruct params;
			params.Set_Basic(this, 70, TECHNICIAN_ANIMATION);
			params.Set_Animation(anim, false);
			Commands->Action_Play_Animation(obj, params);
		}
		if (action_id == TECHNICIAN_ANIMATION || reason == ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE)
		{
			GameObject *terminal;
			terminal = Commands->Find_Object(terminal_id);
			Commands->Send_Custom_Event(obj, terminal, c_type, c_param_2, 0.0f);
			busy = emergency = false;
			if (always_run)
			{
				emergency = true;
			}
		}
	}

};

/*DECLARE_SCRIPT(RMV_Engineer_Wander_Terminal, "Animation_Name:string, Custom_Type:int, Custom_Param_1:int, Custom_Param_2:int")
{
	enum {ENGINEER_WANDER_TIMER};
	
	bool i_am_occupied;
	Vector3 mypos;
	int c_type, c_param_1, c_param_2;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(i_am_occupied, 1);
		SAVE_VARIABLE(mypos, 2);
		SAVE_VARIABLE(c_type, 3);
		SAVE_VARIABLE(c_param_1, 4);
		SAVE_VARIABLE(c_param_2, 5);
	}
		
	void Created(GameObject * obj)
	{
	//	Commands->Enable_Hibernation(obj, false);
		i_am_occupied = false;
		mypos = Commands->Get_Position(obj);
		c_type = Get_Int_Parameter("Custom_Type");
		c_param_1 = Get_Int_Parameter("Custom_Param_1");
		c_param_2 = Get_Int_Parameter("Custom_Param_2");
		Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
	}

	void Timer_Expired(GameObject * obj, int timer_id)
	{
		if ((timer_id == ENGINEER_WANDER_TIMER) && (!i_am_occupied))
		{
			Commands->Create_Logical_Sound(obj, M00_SOUND_ENGINEER_WANDER, mypos, 60.0f);
			Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
		}
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if ((type == c_type) && (param == c_param_1) && (!i_am_occupied))
		{
			i_am_occupied = true;
			const char *anim;
			anim = Get_Parameter("Animation_Name");
			Commands->Send_Custom_Event(obj, sender, c_type, (int)anim);
		}
		if ((type == c_type) && (param == c_param_2) && (i_am_occupied))
		{
			i_am_occupied = false;
			Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
		}
	}
};*/

DECLARE_SCRIPT(RMV_Building_Engineer_Controller, "Killed_Broadcast_Radius:float, 25_Start_ID:int, 25_Number:int, 50_Start_ID:int, 50_Number:int, 75_Start_ID:int, 75_Number:int, Building_Number:int")
{
	bool sent_25, sent_50, sent_75;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sent_25, 1);
		SAVE_VARIABLE(sent_50, 2);
		SAVE_VARIABLE(sent_75, 3);
	}
	
	void Created(GameObject * /*obj*/) override
	{
		sent_25 = sent_50 = sent_75 = false;
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{		
		if (type == CUSTOM_EVENT_BUILDING_DAMAGED)
		{
			float percentage = 1 - ((float)param/100.0f);
			if (!sent_25 && percentage >= 0.25)
			{
				Commands->Debug_Message("Building took 25 percent or more damage, notifying engineers.\n");
				
				sent_25 = true;
				for (int x = 0; x < Get_Int_Parameter("25_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("25_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 1000, 1000, 0.0f);
					}
				}
			}
	
			if (!sent_50 && percentage >= 0.50)
			{
				Commands->Debug_Message("Building took 50 percent or more damage, notifying engineers.\n");
				
				Vector3 mypos = Commands->Get_Position(obj);
				Commands->Create_Logical_Sound(obj, M00_SOUND_ALWAYS_RUN, mypos, Get_Float_Parameter("Killed_Broadcast_Radius"));
				
				sent_50 = true;
				for (int x = 0; x < Get_Int_Parameter("50_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("50_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 1000, 1000, 0.0f);
					}
				}
			}
	
			if (!sent_75 && percentage >= 0.75)
			{
				Commands->Debug_Message("Building took 75 percent or more damages, notifying engineers.\n");
				
				sent_75 = true;
				for (int x = 0; x < Get_Int_Parameter("75_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("75_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 1000, 1000, 0.0f);
					}
				}
			}
		}
		
		if (type == CUSTOM_EVENT_BUILDING_REPAIRED)
		{
			float percentage = 1 - ((float)param/100.0f);
			if (percentage <= 0.25)
			{
				Commands->Debug_Message("Building repaired to 75 percent or more health, notifying engineers.\n");
				
				sent_25 = false;
				for (int x = 0; x < Get_Int_Parameter("25_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("25_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 2000, 2000, 0.0f);
					}
				}
			}
	
			if (percentage <= 0.50)
			{
				Commands->Debug_Message("Building repaired to 50 percent or more health, notifying engineers.\n");
				
				Vector3 mypos = Commands->Get_Position(obj);
				Commands->Create_Logical_Sound(obj, M00_SOUND_ALWAYS_RUN_OFF, mypos, Get_Float_Parameter("Killed_Broadcast_Radius"));
				
				sent_50 = false;
				for (int x = 0; x < Get_Int_Parameter("50_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("50_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 2000, 2000, 0.0f);
					}
				}
			}
	
			if (percentage <= 0.75)
			{
				Commands->Debug_Message("Building repaired to 25 percent or more health, notifying engineers.\n");
				
				sent_75 = false;
				for (int x = 0; x < Get_Int_Parameter("75_Number"); x++)
				{
					GameObject * target = Commands->Find_Object(Get_Int_Parameter("75_Start_ID") + x);
					if (target)
					{
						Commands->Send_Custom_Event(obj, target, 2000, 2000, 0.0f);
					}
				}
			}
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		Vector3 mypos = Commands->Get_Position(obj);
		Commands->Create_Logical_Sound(obj, M00_SOUND_BUILDING_DESTROYED + Get_Int_Parameter("Building_Number"), mypos, Get_Float_Parameter("Killed_Broadcast_Radius"));
	}
};

DECLARE_SCRIPT(RMV_Toggled_Engineer_Target, "Emergency=1:int, Animation_Name:string, Custom_Type:int, Custom_Param_1:int, Custom_Param_2:int")
{
	enum {ENGINEER_WANDER_TIMER};
	
	bool i_am_occupied;
	Vector3 mypos;
	int c_type, c_param_1, c_param_2;
	bool active;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(i_am_occupied, 1);
		SAVE_VARIABLE(mypos, 2);
		SAVE_VARIABLE(c_type, 3);
		SAVE_VARIABLE(c_param_1, 4);
		SAVE_VARIABLE(c_param_2, 5);
		SAVE_VARIABLE(active, 6);
	}
		
	void Created(GameObject * obj) override
	{
		active = false;
	//	Commands->Enable_Hibernation(obj, false);
		i_am_occupied = false;
		mypos = Commands->Get_Position(obj);
		c_type = Get_Int_Parameter("Custom_Type");
		c_param_1 = Get_Int_Parameter("Custom_Param_1");
		c_param_2 = Get_Int_Parameter("Custom_Param_2");
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if ((timer_id == ENGINEER_WANDER_TIMER) && (!i_am_occupied) && active)
		{
			Commands->Create_Logical_Sound(obj, (Get_Int_Parameter("Emergency") == 0) ? M00_SOUND_ENGINEER_WANDER : M00_SOUND_ENGINEER_WANDER_EMERGENCY, mypos, 60.0f);
			Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 1000 && param == 1000)
		{
			active = true;
			Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
		}

		if (type == 2000 && param == 2000)
		{
			active = false;
		}

		if ((type == c_type) && (param == c_param_1) && (!i_am_occupied))
		{
			i_am_occupied = true;
			const char *anim;
			anim = Get_Parameter("Animation_Name");
			Commands->Send_Custom_Event(obj, sender, c_type, (uintptr_t)anim, 0.0f);
		}
		if ((type == c_type) && (param == c_param_2) && (i_am_occupied))
		{
			i_am_occupied = false;
			Commands->Start_Timer(obj, this, 2.0f, ENGINEER_WANDER_TIMER);
		}
	}
};

DECLARE_SCRIPT(RMV_MCT_Switcher, "")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject *temp;
		temp = Commands->Create_Object("NOD MCT Off", Commands->Get_Position(obj));
		if (obj)
		{
			Commands->Set_Facing(temp, Commands->Get_Facing(obj));
		}
	}
};

DECLARE_SCRIPT(M00_Play_Sound, "Sound_Preset:string, Is_3D=1:int, Offset:vector3, Offset_Randomness:vector3, Frequency_Min=-1:float, Frequency_Max:float")
{
	void Created(GameObject * obj) override
	{
		if (Get_Int_Parameter("Frequency_Min") == -1)
		{
			Timer_Expired(obj, 0);
		}
		else
		{
			float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min"), Get_Float_Parameter("Frequency_Max"));
			Commands->Start_Timer(obj, this, time, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		const char * sound = Get_Parameter("Sound_Preset");
		bool is_3d = (Get_Int_Parameter("Is_3D") == 1) ? true : false;
		Vector3 pos = Commands->Get_Position(obj);
		pos += Get_Vector3_Parameter("Offset");
		Vector3 offset_random = Get_Vector3_Parameter("Offset_Randomness");
		pos.X += Commands->Get_Random(-offset_random.X, offset_random.X);
		pos.Y += Commands->Get_Random(-offset_random.Y, offset_random.Y);
		pos.Z += Commands->Get_Random(-offset_random.Z, offset_random.Z);
		
		int id;
		
		if (is_3d)
		{
			Commands->Debug_Message("Playing 3D Sound\n");
			id = Commands->Create_Sound(sound, pos, obj);
		}
		else
		{
			Commands->Debug_Message("Playing 2D Sound\n");
			id = Commands->Create_2D_Sound(sound);
		}
		Commands->Monitor_Sound(obj, id);
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_SOUND_ENDED)
		{
			if (Get_Int_Parameter("Frequency_Min") != -1)
			{
				float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min"), Get_Float_Parameter("Frequency_Max"));
				Commands->Start_Timer(obj, this, time, 0);
			}
		}
	}
};

