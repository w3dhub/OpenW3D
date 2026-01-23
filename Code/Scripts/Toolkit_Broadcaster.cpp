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
*     Toolkit_Broadcaster.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Broadcaster Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 2 $
*     $Modtime: 9/26/00 11:52a $
*     $Archive: /Commando/Code/Scripts/Toolkit_Broadcaster.cpp $
*
******************************************************************************/

#include "Toolkit.h"

/*
Editor Script - M00_Broadcaster_Register_RAD

  This script registers an object with a terminal.

  Parameters:

  Terminal_ID	= The ID of the terminal you wish to register with.
  Send_Attempts	= The number of attempts to send to the terminal this will make before failing.
  Send_Delay	= The delay between attempts to send.
  Debug_Mode	= Turn this on if debug information is needed.
  
  Custom:

  M00_CUSTOM_BROADCASTER_REGISTRATION
  
  Script activates upon creation.
*/

DECLARE_SCRIPT (M00_Broadcaster_Register_RAD, "Terminal_ID:int, Send_Attempts=3:int, Send_Delay=1:int, Debug_Mode=0:int")
{
	int		send_attempts;
	int		terminal_id;
	int		current_send;
	int		send_delay;
	int		item_id;
	bool	debug_mode;

	void Created (GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		send_attempts = Get_Int_Parameter("Send_Attempts");
		terminal_id = Get_Int_Parameter("Terminal_ID");
		send_delay = Get_Int_Parameter("Send_Delay");
		item_id = Commands->Get_ID(obj);
		current_send = 0;

		SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Register_RAD ACTIVATED.\n"));

		Commands->Start_Timer (obj, this, 0.1f, 0);
		//DEBUG Timer may not work here without proper ID later.
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		GameObject* terminal_obj;

		if (!timer_id)
		{
			current_send++;
			if (current_send > send_attempts)
			{
				DebugPrint("ERROR - M00_Broadcaster_Registry_RAD - Object %d cannot find Terminal %d to register with!\n", item_id, terminal_id);
			}
			else
			{
				terminal_obj = Commands->Find_Object(terminal_id);
				if (terminal_obj)
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Registry_RAD is sending custom type M00_CUSTOM_BROADCASTER_REGISTRATION, param %d.\n", item_id));
					Commands->Send_Custom_Event (obj, terminal_obj, M00_CUSTOM_BROADCASTER_REGISTRATION, item_id, 0.0f);
				}
				else
				{
					Commands->Start_Timer (obj, this, send_delay, 0);
				}
			}
		}
	}

	virtual void Custom (GameObject* obj, int type, intptr_t /*param*/, GameObject* /*sender*/) override
	{
		if (type == M00_CUSTOM_BROADCASTER_REGISTRY_ERROR)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Registry_RAD received custom type M00_CUSTOM_BROADCASTER_REGISTRY_ERROR, param %d.\n", param));
			Commands->Start_Timer (obj, this, send_delay, 0);
		}
		else
		{
			SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Broadcaster_Registry_RAD received custom type %d, param %d - unknown!\n", type, param));
		}
	}

	void Destroyed (GameObject* obj) override
	{
		GameObject* terminal_obj;
		terminal_obj = Commands->Find_Object(terminal_id);
		if (terminal_obj)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Registry_RAD is sending custom type M00_CUSTOM_BROADCASTER_REGISTRATION, param 0.\n"));
			Commands->Send_Custom_Event (obj, terminal_obj, M00_CUSTOM_BROADCASTER_REGISTRATION, 0, 0.0f);
		}
	}
};

/*
Editor Script - M00_Broadcaster_Terminal_RAD

  This script is a terminal, which transfers customs to other objects. It can store
  up to 100 objects before returning an error.

  Values:

  object_specific_record	= Storage of each registry item one at a time.
  object_random_record		= Storage of any registry item as many times as desired.
  object_prompts			= Storage of any sent prompts from objects.
	0 = Object ID that is prompting.
	1 = custom type to send with next regular custom.

  Parameters:

  Random_Percentage	= Number from 1 to 99 to determine chance of sending the custom.
  Random_Param_Min	= Minimum random parameter for random sends.
  Random_Param_Max	= Maximum random parameter for random sends.

  M00_CUSTOM_BROADCASTER_REGISTRATION
  M00_CUSTOM_BROADCASTER_PROMPTER

  Prompt Parameters:

  0	= Object is sending a custom that should be sent to everyone with one parameter.
  1	= Object is sending a custom that should be sent to random objects with one parameter.
  2	= Object is sending a custom that should be sent to everyone with random parameter.
  3	= Object is sending a custom that should be sent to random objects with random parameter.

  Script activates upon receipt of a custom.
*/

DECLARE_SCRIPT (M00_Broadcaster_Terminal_RAD, "Random_Percentage=100.0:float, Random_Param_Min=0:int, Random_Param_Max=0:int, Debug_Mode=0:int")
{
	int		object_specific_record [M00_BROADCASTER_TERMINAL_SIZE_RAD];
	int		object_random_record [M00_BROADCASTER_TERMINAL_SIZE_RAD];
	int		object_prompts [M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD][2];
	bool	ready_for_objects;
	bool	debug_mode;

	void Created (GameObject* /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		int object_count;

		ready_for_objects = false;
		
		for (object_count = 0; object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
		{
			object_specific_record [object_count]	= 0;
			object_specific_record [object_count]	= 0;
			object_random_record [object_count]		= 0;
		}
		for (object_count = 0; object_count < M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD; object_count++)
		{
			object_prompts [object_count][0] = 0;
		}
		
		SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD ACTIVATED.\n"));
		ready_for_objects = true;
	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* sender) override
	{
		int my_id;
		int sender_id;
		int object_count;
		int last_empty;
		int obj_id;
		int prompt_value;
		int random_value;
		int random_value2;
		int parameter_low;
		int parameter_high;

		bool found_object;

		GameObject* target_obj;

		// Check if the sender still exists, just an error catch routine.

		if (sender)
		{
			sender_id = Commands->Get_ID (sender);
		}
		else
		{
			sender_id = 0;
		}

		if (ready_for_objects)
		{
			my_id = Commands->Get_ID (obj);

			if (my_id == sender_id)
			{
				DebugPrint ("ERROR - M00_Broadcaster_Terminal_RAD - Broadcaster sent a custom to itself!\n");
			}
			else
			{
				// Check which type of custom was just sent.

				switch (type)
				{
				case (M00_CUSTOM_BROADCASTER_REGISTRATION):
					{
						// Object is attempting to register or unregister itself.

						if (param)
						{
							SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD received custom type M00_CUSTOM_BROADCASTER_REGISTRATION, param %d.\n", param));

							// Object is attempting to register itself.
							// Check if the object is already in the specific list.

							found_object = false;
							last_empty = -1;

							for (object_count = 0; object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
							{
								if (object_specific_record [object_count] == sender_id)
								{
									// Object already exists in the specific record, skip entry.

									object_count = M00_BROADCASTER_TERMINAL_SIZE_RAD;
									found_object = true;
								}
								else
								{
									if ((!object_specific_record [object_count]) && (last_empty == -1))
									{
										last_empty = object_count;
									}
								}
							}

							if (!found_object)
							{
								// Enter the object into the list in the next available slot.

								if (last_empty >= 0)
								{
									object_specific_record [last_empty] = sender_id;
								}
								else
								{
									obj_id = Commands->Get_ID (obj);
									DebugPrint ("ERROR - M00_Broadcaster_Terminal_RAD - Broadcaster Terminal %d is full!\n", obj_id);
								}
							}

							// Now, insert the object into the random list regardless of number of entries.

							for (object_count = 0; object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
							{
								if (!object_random_record [object_count])
								{
									object_random_record [object_count] = sender_id;
									object_count = M00_BROADCASTER_TERMINAL_SIZE_RAD;
								}
							}
						}
						else
						{
							SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD received custom type M00_CUSTOM_BROADCASTER_REGISTRATION, param 0.\n"));

							// Object is attempting to unregister itself.

							for (object_count = 0; object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
							{
								if (object_specific_record [object_count] == sender_id)
								{
									object_specific_record [object_count] = 0;
								}
								if (object_random_record [object_count] == sender_id)
								{
									object_random_record [object_count] = 0;
								}
							}
						}
						break;
					}
				case (M00_CUSTOM_BROADCASTER_PROMPTER):
					{
						SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD received custom type M00_CUSTOM_BROADCASTER_PROMPTER, param %d.\n", param));

						// Check if the object is already in the prompt list, or if there is a blank slot.

						prompt_value = M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD;
						for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD; object_count++)
						{
							if (object_prompts [object_count][0] == sender_id)
							{
								prompt_value = M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD;
								object_count = M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD;
							}
							if (!object_prompts [object_count][0])
							{
								prompt_value = object_count;
								object_count = M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD;
							}
						}
						if (prompt_value < M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD)
						{
							// Object is attempting to prompt another custom send.

							object_prompts [prompt_value][0] = sender_id;
							object_prompts [prompt_value][1] = param;
						}
						break;
					}
				default:
					{
						SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD received custom type %d, param %d.\n", type, param));

						// Object is sending a custom that should be already prompted.
						// If it is not prompted, default to non-random send.

						prompt_value = 0;
						for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_PROMPT_SIZE_RAD; object_count++)
						{
							if (object_prompts [object_count][0] == sender_id)
							{
								prompt_value = object_prompts [object_count][1];
								object_prompts [object_count][0] = 0;
								object_prompts [object_count][1] = 0;
							}
						}

						switch ( prompt_value )
						{
						case (1): // Object is sending a custom that should be sent to random objects with one parameter.
							{
								SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending a custom to random objects with one parameter.\n"));

								for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
								{
									random_value = Commands->Get_Random (0.0f, 100.0f);
									if (random_value <= Get_Float_Parameter ("Random_Percentage"))
									{
										if (object_random_record [object_count])
										{
											target_obj = Commands->Find_Object (object_random_record [object_count]);
											if (target_obj)
											{
												SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending custom type %d, param %d.\n", type, param));
												Commands->Send_Custom_Event (obj, target_obj, type, param, 0.0f);
											}
											else
											{
												object_random_record [object_count] = 0;
											}
										}
									}
								}
								break;
							}
						case (2): // Object is sending a custom that should be sent to everyone with random parameter.
							{
								SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending a custom to everyone with random parameter.\n"));
								parameter_low = Get_Int_Parameter("Random_Param_Min");
								parameter_high = Get_Int_Parameter("Random_Param_Max");
								for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
								{
									if (object_specific_record [object_count])
									{
										target_obj = Commands->Find_Object (object_specific_record [object_count]);
										if (target_obj)
										{
											random_value = int(Commands->Get_Random (float(parameter_low), float(parameter_high)));
											SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending custom type %d, param %d.\n", type, random_value));
											Commands->Send_Custom_Event (obj, target_obj, type, random_value, 0.0f);
										}
										else
										{
											object_specific_record [object_count] = 0;
										}
									}
								}
								break;
							}
						case (3): // Object is sending a custom that should be sent to random objects with random parameter.
							{
								SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending a custom to random objects with random parameter.\n"));
								parameter_low = Get_Int_Parameter("Random_Param_Min");
								parameter_high = Get_Int_Parameter("Random_Param_Max");

								for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
								{
									if (object_random_record [object_count])
									{
										random_value = Commands->Get_Random (0.0f, 100.0f);
										if (random_value <= Get_Float_Parameter ("Random_Percentage"))
										{
											target_obj = Commands->Find_Object (object_random_record [object_count]);
											if (target_obj)
											{
												random_value2 = int(Commands->Get_Random (float(parameter_low), float(parameter_high)));
												SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending custom type %d, param %d.\n", type, random_value2));
												Commands->Send_Custom_Event (obj, target_obj, type, random_value2, 0.0f);
											}
											else
											{
												object_random_record [object_count] = 0;
											}
										}
									}
								}
								break;
							}
						default: // Object is sending a custom that should be sent to everyone with one parameter.
							{
								SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending a custom to everyone with one parameter.\n"));
								for (object_count = 0;object_count < M00_BROADCASTER_TERMINAL_SIZE_RAD; object_count++)
								{
									if (object_specific_record [object_count])
									{
										target_obj = Commands->Find_Object(object_specific_record [object_count]);
										if (target_obj)
										{
											SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending custom type %d, param %d.\n", type, param));
											Commands->Send_Custom_Event (obj, target_obj, type, param, 0.0f);
										}
										else
										{
											object_specific_record [object_count] = 0;
										}
									}
								}
								break;
							}
						}
						break;
					}
				}
			}
		}
		else
		{
			// Terminal is not ready for customs. Send an error message.

			SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Terminal_RAD is sending custom type M00_CUSTOM_BROADCASTER_REGISTRY_ERROR, param 0.\n"));

			Commands->Send_Custom_Event (obj, sender, M00_CUSTOM_BROADCASTER_REGISTRY_ERROR, 0, 0.0f);
		}
	}
};

/*
Editor Script - M00_Broadcaster_Activator_RAD

  This script is an easy way to activate terminals through the Level Editor.

  Parameters:

  Terminal_ID	= The ID of the terminal you wish to register with.
  Prompt_Value	= The prompt value you wish to send to the terminal before the custom.

  0	= Object is sending a custom that should be sent to everyone with one parameter.
  1	= Object is sending a custom that should be sent to random objects with one parameter.
  2	= Object is sending a custom that should be sent to everyone with random parameter.
  3	= Object is sending a custom that should be sent to random objects with random parameter.
  
  Script activates upon receipt of a custom. Defaults to constant send, 0.
*/

DECLARE_SCRIPT (M00_Broadcaster_Activator_RAD, "Terminal_ID:int, Prompt_Value=0:int, Debug_Mode=0:int")
{
	int terminal_id;
	int prompt_value;
	bool		debug_mode;

	void Created (GameObject* /*obj*/) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		terminal_id = Get_Int_Parameter ("Terminal_ID");
		prompt_value = Get_Int_Parameter ("Prompt_Value");

		SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Activator_RAD ACTIVATED.\n"));
	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Activator_RAD received custom type %d, param %d.\n", type, param));

		GameObject* terminal_obj;

		terminal_obj = Commands->Find_Object(terminal_id);
		if (terminal_obj)
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Activator_RAD is sending custom type M00_CUSTOM_BROADCASTER_PROMPTER, param %d.\n", prompt_value));
			SCRIPT_DEBUG_MESSAGE(("M00_Broadcaster_Activator_RAD is sending custom type %d, param %d.\n", type, param));
			
			Commands->Send_Custom_Event (obj, terminal_obj, M00_CUSTOM_BROADCASTER_PROMPTER, prompt_value, 0.0f);
			Commands->Send_Custom_Event (obj, terminal_obj, type, param, 0.0f);
		}
		else
		{
			DebugPrint ("ERROR - M00_Broadcaster_Activator_RAD - Cannot find Terminal %d!\n", terminal_id);
		}
	}
};
