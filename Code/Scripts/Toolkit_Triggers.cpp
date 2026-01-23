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
*     Toolkit_Triggers.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Triggers Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 5 $
*     $Modtime: 11/29/01 11:05a $
*     $Archive: /Commando/Code/Scripts/Toolkit_Triggers.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

/*
Associated Custom Types:

  Trigger Activation/Deactivation Customs - Send 1 for On, 0 for Off:

  M00_CUSTOM_TRIGGER_ACTIVATE_ALL
  M00_CUSTOM_TRIGGER_ACTIVATE_KILLED
  M00_CUSTOM_TRIGGER_ACTIVATE_DESTROYED
  M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED
  M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_EXITED
  M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED_OR_EXITED
  M00_CUSTOM_TRIGGER_ACTIVATE_ENEMY_SEEN
  M00_CUSTOM_TRIGGER_ACTIVATE_DAMAGED
  M00_CUSTOM_TRIGGER_ACTIVATE_WAYPATH_COMPLETE
  M00_CUSTOM_TRIGGER_ACTIVATE_ANIMATION_COMPLETE
  M00_CUSTOM_TRIGGER_TIMER_EXPIRED
*/

/*
Function - M00_Controller_Trigger_RMV

  This script handles all operations of triggers.

  Parameters:

  GameObject * obj			= The object that is sending the trigger.
  GameObject * target		= The target of the trigger.
  float time_lower_bound	= A time fluctuator to delay activation.
  float time_upper_bound	= A time fluctuator to delay activation.
  int custom_type			= The type of the custom to send.
  int custom_param			= The parameter of the custom to send.
*/

void M00_Controller_Trigger_RMV (GameObject * obj, GameObject * target, float time_lower_bound, float time_upper_bound, int custom_type, int custom_param)
{
	if ((time_lower_bound == 0.0) && (time_upper_bound == 0.0))
	{
		if (target)
			Commands->Send_Custom_Event(obj, target, custom_type, custom_param, 0.0f);
	}
	else if ((time_lower_bound != 0.0) && (time_upper_bound == 0.0))
	{
		if (target)
			Commands->Send_Custom_Event(obj, target, custom_type, custom_param, time_lower_bound);
	}
	else if (time_upper_bound != 0.0)
	{
		float random_time = Commands->Get_Random(time_lower_bound, time_upper_bound);
		if (target)
			Commands->Send_Custom_Event(obj, target, custom_type, custom_param, random_time);
	}
};

/*
Editor Script - M00_Trigger_When_Killed_RMV

  This script triggers when the object it is attached to is killed.
*/

DECLARE_SCRIPT(M00_Trigger_When_Killed_RMV, "Start_Now=1:int, Receive_Type=1:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Debug_Mode=0:int")
{
	bool	script_enabled;
	bool	debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(script_enabled, 1);
		SAVE_VARIABLE(debug_mode, 2);
	}

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV DISABLED.\n"));
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		int target_id = Get_Int_Parameter("Target_ID");
		float time_low = Get_Float_Parameter("Min_Delay");
		float time_up = Get_Float_Parameter("Max_Delay");
		int param = Get_Int_Parameter("Send_Param");
		int type = Get_Int_Parameter("Send_Type");
		GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

		if (script_enabled)
		{
			SCRIPT_DEBUG_MESSAGE (("Unit killed with active M00_Trigger_When_Killed_RMV attached.\n"));
			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV found its target object.\n"));
				M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_When_Killed_RMV could not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV received custom type %d, param %d.\n", type, param));

		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_KILLED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV ENABLED.\n"));
				script_enabled = true;
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Killed_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Destroyed_RMV

  This script triggers when the object it is attached to is destroyed.
*/


DECLARE_SCRIPT(M00_Trigger_When_Destroyed_RMV, "Start_Now=1:int, Receive_Type=2:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Debug_Mode=0:int")
{
	bool	script_enabled;
	bool	debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(script_enabled, 1);
		SAVE_VARIABLE(debug_mode, 2);
	}

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV DISABLED.\n"));
		}
	}

	void Destroyed(GameObject * obj) override
	{
		int target_id = Get_Int_Parameter("Target_ID");
		float time_low = Get_Float_Parameter("Min_Delay");
		float time_up = Get_Float_Parameter("Max_Delay");
		int param = Get_Int_Parameter("Send_Param");
		int type = Get_Int_Parameter("Send_Type");
		GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

		if (script_enabled)
		{
			SCRIPT_DEBUG_MESSAGE (("Unit destroyed with active M00_Trigger_When_Destroyed_RMV attached.\n"));
			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV found its target object.\n"));
				M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_When_Destroyed_RMV could not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_DESTROYED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV ENABLED.\n"));
				script_enabled = true;
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Destroyed_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Created_RMV

  This script triggers when the object it is attached to is created.

  NOTE that this script has no ability to be enabled/disabled.
*/

DECLARE_SCRIPT(M00_Trigger_When_Created_RMV, "Target_ID:int, Send_Type:int, Send_Parameter:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Debug_Mode=0:int")
{
	bool	debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(debug_mode, 1);
	}
	
	void Created(GameObject * obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		SCRIPT_DEBUG_MESSAGE (("Unit created with active M00_Trigger_When_Created_RMV attached.\n"));
		int target_id = Get_Int_Parameter("Target_ID");
		float time_low = Get_Float_Parameter("Min_Delay");
		float time_up = Get_Float_Parameter("Max_Delay");
		int param = Get_Int_Parameter("Send_Param");
		int type = Get_Int_Parameter("Send_Type");

		GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));
		if (target_id > 0)
		{
			target = Commands->Find_Object(target_id);
		}
		if (target)
		{
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Created_RMV found its target object.\n"));
			M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
		}
		else
		{
			SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_When_Created_RMV could not find its target object, id = %d!\n", target_id));
		}
	}
};

/*
Editor Script - M00_Trigger_Zone_Entered_RMV

  This script triggers when the zone it is attached to is entered.

  Parameters:

  Trigger_Count		= How many times this trigger will fire.
  Triggerer         = Who can activate this trigger. 

  Special Information:

  Values for Triggerer: 0 = Anyone, 1 = Commando Only, 2 = Non-Commando only
  Values for Trigger_Count: Enter 0 for infinite triggering.
*/

DECLARE_SCRIPT(M00_Trigger_Zone_Entered_RMV, "Start_Now=1:int, Receive_Type=3:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Triggerer=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	bool	debug_mode;
	int		trigger_count;

	

	typedef enum
	{
		ANYONE,
		COMMANDO,
		NONCOMMANDO
	} EntererType;
	EntererType allowed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(script_enabled, 1);
		SAVE_VARIABLE(debug_mode, 2);
		SAVE_VARIABLE(trigger_count, 3);
		SAVE_VARIABLE(allowed, 4);
	}

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");

		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV DISABLED.\n"));
		}
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Zone entered with active M00_Trigger_Zone_Entered_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV has %d triggers remaining.\n", trigger_count));
			}
			
			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			int who = Get_Int_Parameter("Triggerer");

			switch (who)
			{
			case (0):
				{
					allowed = ANYONE;
					break;
				}
			case (1):
				{
					allowed = COMMANDO;
					break;
				}
			case (2):
				{
					allowed = NONCOMMANDO;
					break;
				}
			}
			
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV found its target object.\n"));
				if ((allowed == COMMANDO) && (enterer == Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if ((allowed == NONCOMMANDO) && (enterer != Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if (allowed == ANYONE)
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_Zone_Entered_RMV did not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_Zone_Exited_RMV

  This script triggers when the zone it is attached to is exited.

  Parameters:

  Trigger_Count		= How many times this trigger will fire.
  Triggerer         = Who can activate this trigger. 

  Special Information:

  Values for Triggerer: 0 = Anyone, 1 = Commando Only, 2 = Non-Commando only
  Values for Trigger_Count: Enter 0 for infinite triggering.
*/

DECLARE_SCRIPT(M00_Trigger_Zone_Exited_RMV, "Start_Now=1:int, Receive_Type=4:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Triggerer=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	bool	debug_mode;
	int		trigger_count;

	

	typedef enum
	{
		ANYONE,
		COMMANDO,
		NONCOMMANDO
	} EntererType;
	EntererType allowed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(script_enabled, 1);
		SAVE_VARIABLE(debug_mode, 2);
		SAVE_VARIABLE(trigger_count, 3);
		SAVE_VARIABLE(allowed, 4);
	}

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");
		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV DISABLED.\n"));
		}
	}

	void Exited(GameObject * obj, GameObject * enterer) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Zone exited with active M00_Trigger_Zone_Exited_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV has %d triggers remaining.\n", trigger_count));
			}
			
			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			int who = Get_Int_Parameter("Triggerer");

			switch (who)
			{
			case (0):
				{
					allowed = ANYONE;
					break;
				}
			case (1):
				{
					allowed = COMMANDO;
					break;
				}
			case (2):
				{
					allowed = NONCOMMANDO;
					break;
				}
			}
			
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV found its target object.\n"));
				if ((allowed == COMMANDO) && (enterer == Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if ((allowed == NONCOMMANDO) && (enterer != Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if (allowed == ANYONE)
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_Zone_Exited_RMV could not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Exited_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_Zone_Entered_Or_Exited_RMV

  This script triggers when the zone it is attached to is entered or exited.

  Parameters:

  Trigger_Count		= How many times this trigger will fire.
  Triggerer         = Who can activate this trigger. 

  Special Information:

  Values for Triggerer: 0 = Anyone, 1 = Commando Only, 2 = Non-Commando only
  Values for Trigger_Count: Enter 0 for infinite triggering.
*/

DECLARE_SCRIPT(M00_Trigger_Zone_Entered_Or_Exited_RMV, "Start_Now=1:int, Receive_Type=5:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Triggerer=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	bool	debug_mode;
	int		trigger_count;

	typedef enum
	{
		ANYONE,
		COMMANDO,
		NONCOMMANDO
	} EntererType;
	EntererType allowed;

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");
		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV DISABLED.\n"));
		}
	}

	void Exited(GameObject * obj, GameObject * enterer) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Zone exited with active M00_Trigger_Zone_Entered_Or_Exited_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV has %d triggers remaining.\n", trigger_count));
			}
			
			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			int who = Get_Int_Parameter("Triggerer");

			switch (who)
			{
			case (0):
				{
					allowed = ANYONE;
					break;
				}
			case (1):
				{
					allowed = COMMANDO;
					break;
				}
			case (2):
				{
					allowed = NONCOMMANDO;
					break;
				}
			}
			
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV found its target object.\n"));
				if ((allowed == COMMANDO) && (enterer == Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if ((allowed == NONCOMMANDO) && (enterer != Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if (allowed == ANYONE)
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_Zone_Entered_Or_Exited_RMV did not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Zone entered with active M00_Trigger_Zone_Entered_Or_Exited_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV has %d triggers remaining.\n", trigger_count));
			}
			
			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			int who = Get_Int_Parameter("Triggerer");

			switch (who)
			{
			case (0):
				{
					allowed = ANYONE;
					break;
				}
			case (1):
				{
					allowed = COMMANDO;
					break;
				}
			case (2):
				{
					allowed = NONCOMMANDO;
					break;
				}
			}
			
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV found its target object.\n"));
				if ((allowed == COMMANDO) && (enterer == Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if ((allowed == NONCOMMANDO) && (enterer != Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f))))
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
				else if (allowed == ANYONE)
				{
					SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV triggered properly.\n"));
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_Zone_Entered_Or_Exited_RMV could not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ZONE_ENTERED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_Zone_Entered_Or_Exited_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Enemy_Seen_RMV

  This script triggers when an enemy is seen.

  Parameters:

  Trigger_Count		= How many times this trigger will fire. Enter 0 for infinite.
*/

DECLARE_SCRIPT(M00_Trigger_When_Enemy_Seen_RMV, "Start_Now=1:int, Receive_Type=6:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Parameter:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	int		trigger_count;
	bool	debug_mode;

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");
		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV DISABLED.\n"));
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject * /*enemy*/) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Enemy seen by unit with active M00_Trigger_When_Enemy_Seen_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV has %d triggers remaining.\n", trigger_count));
			}

			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}

			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV has found its target object.\n"));
				M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_When_Enemy_Seen_RMV could not find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ENEMY_SEEN) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Enemy_Seen_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Damaged_RMV

  This script triggers when the unit is damaged.

  Parameters:

  Trigger_Count	= How many times this will trigger.
*/

DECLARE_SCRIPT(M00_Trigger_When_Damaged_RMV, "Start_Now=1:int, Receive_Type=7:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Debug_Mode=0:int")
{
	bool script_enabled;
	int trigger_count;
	bool		debug_mode;

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");

		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;

			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV ENABLED.\n"));
		}
		else
		{
			script_enabled = false;

			SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV DISABLED.\n"));
		}
	}

	void Damaged(GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE (("Unit damaged with active M00_Trigger_When_Damaged_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV has %d triggers remaining.\n", trigger_count));
			}

			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV found the target object.\n"));
				M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE (("ERROR - M00_Trigger_When_Damaged_RMV could not find the target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV received custom type %d,param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_DAMAGED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE (("M00_Trigger_When_Damaged_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Action_Complete_RMV

  This script triggers when the unit completes any action. It is designed to respond to
  a particular Action ID.

  Parameters:

  Trigger_Count	= How many times this will trigger.
*/

DECLARE_SCRIPT(M00_Trigger_When_Action_Complete_RMV, "Start_Now=1:int, Receive_Type=8:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Action_ID:int, Trigger_Count=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	int		trigger_count;
	bool	debug_mode;

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");

		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV_ENABLED.\n"));
		}
		else
		{
			script_enabled = false;
			SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV DISABLED.\n"));
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (script_enabled)
		{
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE(("Action complete by unit with active M00_Trigger_When_Action_Complete_RMV attached.\n"));
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV has %d triggers remaining.\n", trigger_count));
			}

			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));
			
			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV found its target object.\n"));
				if (action_id == Get_Int_Parameter("Action_ID"))
				{
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Trigger_When_Action_Complete_RMV cannot find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ACTION_COMPLETE) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Action_Complete_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

/*
Editor Script - M00_Trigger_When_Animation_Complete_RMV

  This script triggers when the unit completes an animation.

  Parameters:

  Animation_Name	= The name of the animation that is completing.
  Trigger_Count		= How many times this will trigger.
*/

DECLARE_SCRIPT(M00_Trigger_When_Animation_Complete_RMV, "Start_Now=1:int, Receive_Type=9:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Target_ID:int, Animation_Name:string, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Trigger_Count=0:int, Debug_Mode=0:int")
{
	bool	script_enabled;
	int		trigger_count;
	bool	debug_mode;

	void Created(GameObject * /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		trigger_count = Get_Int_Parameter("Trigger_Count");

		if (Get_Int_Parameter("Start_Now"))
		{
			script_enabled = true;
			SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV ENABLED.\n"));
		}
		else
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV DISABLED.\n"));
			script_enabled = false;
		}
	}

	void Animation_Complete(GameObject * obj, const char * name) override
	{
		if (script_enabled)
		{
			SCRIPT_DEBUG_MESSAGE(("Animation complete by unit with active M00_Trigger_When_Animation_Complete_RMV attached.\n"));
			if (trigger_count == 1)
			{
				script_enabled = false;
			}
			if (trigger_count > 0)
			{
				trigger_count--;
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV has %d triggers remaining.\n", trigger_count));
			}

			int target_id = Get_Int_Parameter("Target_ID");
			float time_low = Get_Float_Parameter("Min_Delay");
			float time_up = Get_Float_Parameter("Max_Delay");
			int param = Get_Int_Parameter("Send_Param");
			int type = Get_Int_Parameter("Send_Type");
			const char *anim = Get_Parameter("Animation_Name");
			GameObject *target = Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f));

			if (target_id > 0)
			{
				target = Commands->Find_Object(target_id);
			}
			if (target)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV found its target object.\n"));
				if (stricmp(anim, name))
				{
					M00_Controller_Trigger_RMV(obj, target, time_low, time_up, type, param);
				}
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Trigger_When_Animation_Complete_RMV cannot find its target object, id = %d!\n", target_id));
			}
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_ACTIVATE_ANIMATION_COMPLETE) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV ENABLED.\n"));
				script_enabled = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_When_Animation_Complete_RMV DISABLED.\n"));
				script_enabled = false;
			}
		}
	}
};

DECLARE_SCRIPT(M00_Trigger_Timer_Expired_RAD, "Start_Now=0:int, Receive_Type=15:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Timer_Min=0.0:float, Timer_Max=1.0:float, Trigger_Count=0:int, Target_ID:int, Send_Type:int, Send_Param:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Debug_Mode=0:int")
{
	bool debug_mode;
	bool script_active;
	int trigger_count;

	void Created(GameObject* obj) override
	{
		script_active = false;

		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			Start_Now(obj);
		}
	}

	void Start_Now(GameObject* obj)
	{
		script_active = true;
		trigger_count = Get_Int_Parameter("Trigger_Count");

		if (trigger_count <= 0)
		{
			trigger_count = -1;
		}
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD ENABLED.\n"));

		Set_Timer(obj);
	}

	void Set_Timer(GameObject* obj)
	{
		float timer_min;
		float timer_max;
		float timer_value;

		timer_min = Get_Float_Parameter("Timer_Min");
		timer_max = Get_Float_Parameter("Timer_Max");

		if (timer_max > timer_min)
		{
			timer_value = Commands->Get_Random(timer_min, timer_max);
		}
		else
		{
			timer_value = timer_min;
		}
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD is starting the timer.\n"));
		Commands->Start_Timer (obj, this, timer_value, M00_TIMER_TRIGGER_EXPIRED);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		SCRIPT_DEBUG_MESSAGE(("A timer has expired on M00_Trigger_Timer_Expired_RAD.\n"));
		if (script_active)
		{
			if (trigger_count > 0)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD has %d triggers remaining.\n", trigger_count));
				trigger_count--;
			}
			if (trigger_count)
			{
				GameObject* target_obj;
				int target_id;
				int min_delay;
				int max_delay;
				int delay_value;

				target_id = Get_Int_Parameter("Target_ID");

				if (timer_id == M00_TIMER_TRIGGER_EXPIRED)
				{
					target_obj = Commands->Find_Object(target_id);
					
					if (target_obj)
					{
						SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD found its target object.\n"));
						min_delay = Get_Float_Parameter("Min_Delay");
						max_delay = Get_Float_Parameter("Max_Delay");

						if (max_delay > min_delay)
						{
							delay_value = Commands->Get_Random(min_delay, max_delay);
						}
						else
						{
							delay_value = min_delay;
						}

						Commands->Send_Custom_Event (obj, target_obj, Get_Int_Parameter("Send_Type"), Get_Int_Parameter("Send_Param"), delay_value);
					}
				}
				else
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD cannot find its target object, id = %d!\n", target_id));
				}
				Set_Timer(obj);
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD received custom type %d, param %d.\n", type, param));
		if ((type == M00_CUSTOM_TRIGGER_ACTIVATE_ALL) || (type == M00_CUSTOM_TRIGGER_TIMER_EXPIRED) || (type == Get_Int_Parameter("Receive_Type")))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				script_active = true;
				trigger_count = Get_Int_Parameter("Trigger_Count");
				Start_Now(obj);
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_Timer_Expired_RAD DISABLED.\n"));
				script_active = false;
			}
		}
	}
};

/*

  Custom Parameter Settings:

  0 = Turn this state sequence off.
  -1 = Advance to the next state.
  1+ = Adjust to this particular numbered state.

*/


DECLARE_SCRIPT(M00_Trigger_State_Sequence_RAD, "Start_Now=0:int, Receive_Type:int, Num_States=2:int, Loop_States=0:int, Send_Type:int, Target_ID:int, Min_Delay=0.0:float, Max_Delay=0.0:float, Debug_Mode=0:int")
{
	int state_num;
	bool debug_mode;

	void Created(GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		state_num = 1;

		if (Get_Int_Parameter("Start_Now"))
		{
			Send_Custom(obj);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_State_Sequence_RAD received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param < 0)
			{
				// Advance to the next state.

				state_num++;
				if (state_num > Get_Int_Parameter("Num_States"))
				{
					if (Get_Int_Parameter("Loop_States"))
					{
						state_num = 1;
					}
					else
					{
						state_num = 0;
					}
				}
				Send_Custom(obj);
			}
			if (param > 0)
			{
				// Adjust to this numbered state.

				state_num = param;

				if (state_num > Get_Int_Parameter("Num_States"))
				{
					if (Get_Int_Parameter("Loop_States"))
					{
						state_num = 1;
					}
					else
					{
						state_num = 0;
					}
				}
				Send_Custom(obj);

			}
			if (!param)
			{
				// Turn this state engine off.

				state_num = 0;
			}
		}
	}

	void Send_Custom(GameObject* obj)
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Trigger_State_Sequence_RAD is sending its state custom.\n"));

		GameObject* target_obj;
		int target_id;
		float min_delay;
		float max_delay;
		float delay_value;

		if (state_num)
		{
			target_id = Get_Int_Parameter("Target_ID");
			target_obj = Commands->Find_Object(target_id);

			if (target_obj)
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Trigger_State_Sequence_RAD found its target object.\n"));

				min_delay = Get_Float_Parameter("Min_Delay");
				max_delay = Get_Float_Parameter("Max_Delay");

				if (max_delay > min_delay)
				{
					delay_value = Commands->Get_Random(min_delay, max_delay);
				}
				else
				{
					delay_value = min_delay;
				}

				Commands->Send_Custom_Event (obj, target_obj, Get_Int_Parameter("Send_Type"), state_num, delay_value);
			}
			else
			{
				SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Trigger_State_Sequence_RAD could not find its target object, id = %d!\n", target_id));
			}
		}
	}
};
