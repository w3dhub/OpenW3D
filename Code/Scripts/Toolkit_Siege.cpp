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
*     Toolkit_Siege.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Siege Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 5 $
*     $Modtime: 11/03/00 1:36p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Siege.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

DECLARE_SCRIPT(M00_Siege_Zone_RAD, "Start_Now=1:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Wander_Distance=3.0:float, My_Group_ID=0:int, Attraction_Radius=20.0:float, Debug_Mode=0:int")
{
	int my_actor_id;
	int my_group_id;
	bool script_active;
	bool debug_mode;
	
	void Created (GameObject* obj) override
	{
		float rnd_value;

		rnd_value = Commands->Get_Random(0.1f, 1.0f);
		my_actor_id = 0;
		my_group_id = Get_Int_Parameter("My_Group_ID");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD ACTIVATED.\n"));
			script_active = true;
			Commands->Start_Timer (obj, this, rnd_value, M00_TIMER_SIEGE_ZONE);
		}
		else
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD IS NOT ACTIVE!\n"));
			script_active = false;
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		float		rnd_value;
		GameObject*	actor_obj;
		Vector3		my_loc;

		my_loc = Commands->Get_Position(obj);

		if (timer_id == M00_TIMER_SIEGE_ZONE)
		{
			if (my_actor_id)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has a registered actor.\n"));
				actor_obj = Commands->Find_Object(my_actor_id);

				if (actor_obj)
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has found the registered actor.\n"));
					Commands->Send_Custom_Event (obj, actor_obj, M00_CUSTOM_SIEGE_ZONE_CHECKING_ACTOR, 0, 0.0f);
				}
				else
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD cannot find the actor, clearing zone.\n"));
					my_actor_id = 0;
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has no actor, shouting for one.\n"));
				Commands->Create_Logical_Sound(obj, M00_SOUND_SIEGE_ZONE, my_loc, Get_Float_Parameter("Attraction_Radius"));
			}
			if (script_active)
			{
				rnd_value = Commands->Get_Random(4.0f, 8.0f);
				Commands->Start_Timer (obj, this, rnd_value, M00_TIMER_SIEGE_ZONE);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has received a custom of type %d, param %d.\n", type, param));

		if ((!my_actor_id) && (type == M00_CUSTOM_SIEGE_ACTOR_GROUP_CHECK))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has an actor attempting to register.\n"));
			if ((param == my_group_id) && (sender))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD verified that this actor is with the same group.\n"));
				my_actor_id = Commands->Get_ID (sender);
				Commands->Send_Custom_Event (obj, sender, M00_CUSTOM_SIEGE_ZONE_VERIFICATION, 0, 0.0f);
			}
		}
		if ((my_actor_id) && (param == my_actor_id) && (type == M00_CUSTOM_SIEGE_ACTOR_IS_DEAD))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD was told that its actor is dead.\n"));
			my_actor_id = 0;
		}
		if ((my_actor_id) && (type == M00_CUSTOM_SIEGE_ACTOR_RESPONDING) && (sender))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has an actor responding.\n"));
			if (param == my_actor_id)
			{
				Vector3		my_loc;
				my_loc = Commands->Get_Position(obj);
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD has verified that this is its actor - setting position.\n"));
				Commands->Set_Innate_Soldier_Home_Location(sender, my_loc, Get_Float_Parameter("Wander_Distance"));
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD is registered to the wrong actor - clearing.\n"));
				my_actor_id = 0;
				Commands->Send_Custom_Event (obj, sender, M00_CUSTOM_SIEGE_ACTOR_UNREGISTER, 0, 0.0f);
			}
		}
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD ACTIVATED.\n"));
				script_active = true;
				float	rnd_value;
				rnd_value = Commands->Get_Random(0.1f, 1.0f);
				Commands->Start_Timer (obj, this, rnd_value, M00_TIMER_SIEGE_ZONE);
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD DEACTIVATED.\n"));
				script_active = false;
			}
		}
		if (type == M00_CUSTOM_SIEGE_OBJECT_GROUP_CHANGE)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Zone_RAD is being ordered to change its group to %d.\n", param));
			my_group_id = param;
		}
	}
};

DECLARE_SCRIPT(M00_Siege_Actor_RAD, "Start_Now=1:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, My_Group_ID=0:int, Debug_Mode=0:int")
{
	int		registered_zone;
	int		my_group_id;
	bool	script_active;
	bool	debug_mode;

	void Created (GameObject* /*obj*/) override
	{
		registered_zone = 0;
		my_group_id = Get_Int_Parameter("My_Group_ID");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD ACTIVATED.\n"));
			script_active = true;
		}
		else
		{
			script_active = false;
		}
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		if (script_active)
		{
			if ((sound.Type == M00_SOUND_SIEGE_ZONE) && (!registered_zone))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD has heard a zone shouting for an actor, sound ID = %d.\n", sound.Type));
				Commands->Send_Custom_Event (obj, sound.Creator, M00_CUSTOM_SIEGE_ACTOR_GROUP_CHECK, my_group_id, 0.0f);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		int my_id;

		my_id = Commands->Get_ID(obj);

		if ((!registered_zone) && (type == M00_CUSTOM_SIEGE_ZONE_VERIFICATION))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD is registering with a siege zone.\n"));
			registered_zone = Commands->Get_ID(sender);
		}
		if (type == M00_CUSTOM_SIEGE_ZONE_CHECKING_ACTOR)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD is being checked for proper ID.\n"));
			Commands->Send_Custom_Event (obj, sender, M00_CUSTOM_SIEGE_ACTOR_RESPONDING, my_id, 0.0f);
		}
		if (type == M00_CUSTOM_SIEGE_ACTOR_UNREGISTER)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD is being told to unregister the zone.\n"));
			registered_zone = 0;
		}
		if (type == M00_CUSTOM_SIEGE_OBJECT_GROUP_CHANGE)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD is being told to change its group to %d.\n", param));
			my_group_id = param;
		}
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD ACTIVATED.\n"));
				script_active = true;
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD DEACTIVATED.\n"));
				script_active = false;
			}
		}
	}

	void Destroyed (GameObject* obj) override
	{
		int my_id;
		GameObject* zone_obj;

		my_id = Commands->Get_ID(obj);

		if (registered_zone)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD has been destroyed.\n"));

			zone_obj = Commands->Find_Object(registered_zone);

			if (zone_obj)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Siege_Actor_RAD found its siege zone, unregistering.\n"));
				Commands->Send_Custom_Event (obj, zone_obj, M00_CUSTOM_SIEGE_ACTOR_IS_DEAD, my_id, 0.0f);
			}
		}
	}
};
