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
*     Toolkit_Sounds.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Sounds Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: David_y $
*     $Revision: 13 $
*     $Modtime: 12/05/01 6:04p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Sounds.cpp $
*
******************************************************************************/

#include "Toolkit.h"

/*
Function - M00_Controller_Sound_RAD

  This script plays a 2D or 3D sound once, then returns.

  Parameters:

  GameObject* obj		= The object that is creating the sound.
  Vector3 sound_loc		= Where the sound is to be played.
  char sound_effect[]	= The string name of the sound effect to play.
  bool two_dimensional	= 1 if 2D, 0 if 3D.
*/

void M00_Controller_Sound_RAD (GameObject* obj, const Vector3& sound_loc, const char* sound_effect, bool two_dimensional)
{
	if (two_dimensional)
	{
		Commands->Create_2D_WAV_Sound (sound_effect);
	}
	else
	{
		Commands->Create_Sound (sound_effect, sound_loc, obj);
	}
};

/*
Editor Script - M00_Sound_Play_2D_RAD

  Play a 2D sound one or more times, with a delay factor in between.

  Parameters:

  Sound_Effect		= The 2D sound effect to play.
  Play_Count		= How many times the sound should play.
  Sound_Delay_Min	= The minimum wait before playing the sound.
  Sound_Delay_Max	= The maximum wait before playing the sound.

*/

DECLARE_SCRIPT(M00_Sound_Play_2D_RAD, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Sound_Effect:string, Play_Count=1:int, Sound_Delay_Min=0.1:float, Sound_Delay_Max=0.1:float, Debug_Mode=0:int")
{
	int			play_count;
	int			current_count;
	float		delay_time_min;
	float		delay_time_max;
	float		delay_time;
	bool		debug_mode;

	void Created(GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		play_count = Get_Int_Parameter("Play_Count");
		delay_time_min = Get_Float_Parameter("Sound_Delay_Min");
		delay_time_max = Get_Float_Parameter("Sound_Delay_Max");
		current_count = 0;

		if (delay_time_max < delay_time_min)
		{
			delay_time_max = delay_time_min;
		}
		if (Get_Int_Parameter("Start_Now"))
		{
			delay_time = 0.1f;
			Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_2D_RAD);
			SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD ACTIVATED.\n"));
		}
	}

	void Custom(GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD ACTIVATED.\n"));
				if (play_count > 1)
				{
					delay_time = 0.1f;
					Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_2D_RAD);
				}
				else
				{
					const char *sound_effect;
					sound_effect = Get_Parameter("Sound_Effect");
					SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD Playing sound.\n"));
					M00_Controller_Sound_RAD (obj, Vector3(0.0f, 0.0f, 0.0f), sound_effect, 1);
				}
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				play_count = 0;
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD DEACTIVATED.\n"));
			}
		}
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if (timer_id == M00_TIMER_SOUND_PLAY_2D_RAD)
		{
			current_count++;
			if (current_count <= play_count)
			{
				if (delay_time_min == delay_time_max)
				{
					delay_time = delay_time_min;
				}
				else
				{
					delay_time = Commands->Get_Random(delay_time_min, delay_time_max);
				}
				const char *sound_effect;
				sound_effect = Get_Parameter("Sound_Effect");
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_2D_RAD playing sound.\n"));
				M00_Controller_Sound_RAD (obj, Vector3(0.0f, 0.0f, 0.0f), sound_effect, 1);
				Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_2D_RAD);
			}
			else
			{
				current_count = 0;
			}
		}
	}
};

/*
Editor Script - M00_Sound_Play_3D_On_Object_RAD

  Play a 3D sound one or more times, with a delay factor in between, on an object.

  Parameters:

  Object_ID			= The ID of the object to play the sound on.
  Sound_Effect		= The sound effect to play.
  Play_Count		= The number of times to play this effect.
  Sound_Delay_Min	= The minimum delay until the sound is played.
  Sound_Delay_Max	= The maximum delay until the sound is played.

  Special Information:

  Object_ID of 0	= Play on self.
  Play_Count of 0	= Play an endless loop.
*/

DECLARE_SCRIPT (M00_Sound_Play_3D_On_Object_RAD, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Object_ID:int, Sound_Effect:string, Play_Count=1:int, Sound_Delay_Min=0.1:float, Sound_Delay_Max=0.1:float, Debug_Mode=0:int")
{
	int			play_count;
	int			current_count;
	float		delay_time_min;
	float		delay_time_max;
	float		delay_time;
	Vector3		my_position;
	bool		debug_mode;

	void Created (GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;		
		play_count = Get_Int_Parameter("Play_Count");
		delay_time_min = Get_Float_Parameter("Sound_Delay_Min");
		delay_time_max = Get_Float_Parameter("Sound_Delay_Max");
		current_count = 0;

		if (delay_time_max < delay_time_min)
		{
			delay_time_max = delay_time_min;
		}
		if (Get_Int_Parameter("Start_Now"))
		{
			delay_time = 0.1f;
			Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
			SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD ACTIVATED.\n"));
		}
	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD ACTIVATED.\n"));
				if ((play_count > 1) || (!play_count))
				{
					delay_time = 0.1f;
					Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
				}
				else
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD playing sound.\n"));
					if (Get_Int_Parameter("Object_ID"))
					{
						GameObject* target_object;
						target_object = Commands->Find_Object(Get_Int_Parameter("Object_ID"));
						if (target_object)
						{
							const char* sound_effect;
							sound_effect = Get_Parameter("Sound_Effect");
							my_position = Commands->Get_Position(target_object);
							M00_Controller_Sound_RAD (target_object, my_position, sound_effect, 0);
						}
					}
					else
					{
						const char* sound_effect;
						sound_effect = Get_Parameter("Sound_Effect");
						my_position = Commands->Get_Position(obj);
						M00_Controller_Sound_RAD (obj, my_position, sound_effect, 0);
					}
				}
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD DEACTIVATED.\n"));
				play_count = 0;
			}
		}
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD)
		{
			current_count++;
			if (play_count)
			{
				if (current_count <= play_count)
				{
					Play_The_Sound (obj);
				}
				else
				{
					current_count = 0;
				}
			}
			else
			{
				Play_The_Sound (obj);
			}
		}
	}

	void Play_The_Sound (GameObject* obj)
	{
		if (delay_time_min == delay_time_max)
		{
			delay_time = delay_time_min;
		}
		else
		{
			delay_time = Commands->Get_Random(delay_time_min, delay_time_max);
		}
		SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_On_Object_RAD playing sound.\n"));
		if (!Get_Int_Parameter("Object_ID"))
		{
			const char *sound_effect;
			sound_effect = Get_Parameter("Sound_Effect");
			my_position = Commands->Get_Position(obj);
			M00_Controller_Sound_RAD (obj, my_position, sound_effect, 0);
		}
		else
		{
			GameObject *target_obj;
			target_obj = Commands->Find_Object(Get_Int_Parameter("Object_ID"));
			if (target_obj)
			{
				const char *sound_effect;
				sound_effect = Get_Parameter("Sound_Effect");
				my_position = Commands->Get_Position(target_obj);
				M00_Controller_Sound_RAD (target_obj, my_position, sound_effect, 0);
			}
		}
		Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
	}
};

/*
Editor Script - M00_Sound_Play_3D_At_Location_RMV

  Play a 3D sound one or more times, with a delay factor in between, at this location.

  Parameters:

  Sound_Effect		= The sound effect to be played.
  Play_Count		= The number of times to play the sound.
  Sound_Delay_Min	= The minimum delay before playing the sound.
  Sound_Delay_Max	= The maximum delay before playing the sound.
  Origin			= The Vector3 coordinate to play the sound at.
*/

DECLARE_SCRIPT(M00_Sound_Play_3D_At_Location_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Sound_Effect:string, Play_Count=1:int, Sound_Delay_Min=0.1:float, Sound_Delay_Max=0.1:float, Origin:vector3, Debug_Mode=0:int")
{
	int			play_count;
	int			current_count;
	float		delay_time_min;
	float		delay_time_max;
	float		delay_time;
	Vector3		my_position;
	bool		debug_mode;

	void Created(GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		play_count = Get_Int_Parameter("Play_Count");
		delay_time_min = Get_Float_Parameter("Sound_Delay_Min");
		delay_time_max = Get_Float_Parameter("Sound_Delay_Max");
		my_position = Get_Vector3_Parameter("Origin");
		current_count = 0;

		if (delay_time_max < delay_time_min)
		{
			delay_time_max = delay_time_min;
		}
		if (Get_Int_Parameter("Start_Now"))
		{
			delay_time = 0.1f;
			Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_RAD);
			SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Location_RMV ACTIVATED.\n"));
		}
	}

	void Custom(GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Location_RMV received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Location_RMV ACTIVATED.\n"));
				if (play_count > 1)
				{
					delay_time = 0.1f;
					Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_RAD);
				}
				else
				{
					const char* sound_effect;
					sound_effect = Get_Parameter("Sound_Effect");
					M00_Controller_Sound_RAD (obj, my_position, sound_effect, 0);
				}
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				play_count = 0;
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Location_RMV DEACTIVATED.\n"));
			}
		}
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if (timer_id == M00_TIMER_SOUND_PLAY_3D_RAD)
		{
			current_count++;
			if (current_count <= play_count)
			{
				if (delay_time_min == delay_time_max)
				{
					delay_time = delay_time_min;
				}
				else
				{
					delay_time = Commands->Get_Random(delay_time_min, delay_time_max);
				}
				const char* sound_effect;
				sound_effect = Get_Parameter("Sound_Effect");
				M00_Controller_Sound_RAD (obj, my_position, sound_effect, 0);
				Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_RAD);
			}
			else
			{
				current_count = 0;
			}
		}
	}
};

/*
Editor Script - M00_Sound_Play_3D_At_Bone_RMV

  Play a 3D sound one or more times, with a delay factor in between, on an object.

  Parameters:

  Object_ID			= The ID of the object to play the sound on.
  Bone_Name			= The bone at which to play the sound.
  Sound_Effect		= The sound effect to play.
  Play_Count		= The number of times to play the sound.
  Sound_Delay_Min	= The minimum delay before playing the sound.
  Sound_Delay_Max	= The maximum delay before playing the sound.
*/

DECLARE_SCRIPT(M00_Sound_Play_3D_At_Bone_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Object_ID:int, Bone_Name:string, Sound_Effect:string, Play_Count=1:int, Sound_Delay_Min=0.1:float, Sound_Delay_Max=0.1:float, Debug_Mode=0:int")
{
	int			play_count;
	int			current_count;
	float		delay_time_min;
	float		delay_time_max;
	float		delay_time;
	Vector3		my_position;
	bool		debug_mode;

	void Created(GameObject* obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		play_count = Get_Int_Parameter("Play_Count");
		delay_time_min = Get_Float_Parameter("Sound_Delay_Min");
		delay_time_max = Get_Float_Parameter("Sound_Delay_Max");
		current_count = 0;

		if (delay_time_max < delay_time_min)
		{
			delay_time_max = delay_time_min;
		}
		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Bone_RMV ACTIVATED.\n"));
			delay_time = 0.1f;
			Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
		}
	}

	void Custom(GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Bone_RMV received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Bone_RMV ACTIVATED.\n"));
				if (play_count > 1)
				{
					delay_time = 0.1f;
					Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
				}
				else
				{
					GameObject *target_object;
					target_object = Commands->Find_Object(Get_Int_Parameter("Object_ID"));
					if (target_object)
					{
						const char* sound_effect;
						sound_effect = Get_Parameter("Sound_Effect");
						my_position = Commands->Get_Bone_Position(target_object, Get_Parameter("Bone_Name"));
						M00_Controller_Sound_RAD (target_object, my_position, sound_effect, 0);
					}
				}
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				play_count = 0;
				SCRIPT_DEBUG_MESSAGE(("M00_Sound_Play_3D_At_Bone_RMV DEACTIVATED.\n"));
			}
		}
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if (timer_id == M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD)
		{
			current_count++;
			if (current_count <= play_count)
			{
				if (delay_time_min == delay_time_max)
				{
					delay_time = delay_time_min;
				}
				else
				{
					delay_time = Commands->Get_Random(delay_time_min, delay_time_max);
				}

				GameObject *target_object;
				target_object = Commands->Find_Object(Get_Int_Parameter("Object_ID"));
				if (target_object)
				{
					const char *sound_effect;
					sound_effect = Get_Parameter("Sound_Effect");
					my_position = Commands->Get_Bone_Position(target_object, Get_Parameter("Bone_Name"));
					M00_Controller_Sound_RAD (target_object, my_position, sound_effect, 0);
				}

				Commands->Start_Timer (obj, this, delay_time, M00_TIMER_SOUND_PLAY_3D_ON_OBJECT_RAD);
			}
			else
			{
				current_count = 0;
			}
		}
	}
};

/*
Editor Script - M00_Sound_Building_Ambient_RAD

  This script handles all ambient sound effects for standard buildings. It is placed on an
  object that is located where the Master Control Terminal for the building is. For those
  buildings with no MCT, the object is located at the approximate center of the building
  (this mainly applies to the Guard Tower and Tiberium Silo).

  When the script is created, the building is assumed to start at full power. We can adjust
  this later through other means - this script is only reactive, meaning its output only
  plays sound effects and does not control other objects.

  When a building is active, all sound effects for the building are played normally at 
  appropriate positions. Some may be on a timer basis, which is also handled here.

  When a building is destroyed, the main ambient sounds are deactivated, and the power-down
  sound sequence is played. This is another set of sounds played at preset locations.

  When the power-down sequence completes, the power-down sound effects are deactivated and
  the destroyed building sound effects commence, if the building is destroyed and has not
  merely lost power.

  When the building is repaired, the destroyed building sound effects cease, and a power-up
  sequence is played.

  When the power-up sequence completes, the power-up sound effects are stopped, and the
  standard ambient sounds are reinstated.

  When the building loses power, the power-down sound effects are played, and all power-loss
  sounds are started.

  In essence, there are several points throughout each building where standard sounds will
  be played. Each location plays one sound at a time, and may or may not loop.

  
  Parameters:

  Building_ID =		Which building this script is associated with.
  Building_Type =	A number indicating which type of building this is.
  Building_Facing = Which cardinal direction the building is facing.

  Building Type Numbers:

  01 = NOD Tiberium Refinery
  02 = NOD Communications Center
  03 = NOD Power Plant
  04 = NOD Airfield
  05 = NOD Construction Yard
  06 = NOD Guard Tower
  07 = NOD Hand of Nod
  08 = NOD Landing Pad
  09 = NOD Obelisk of Light
  10 = NOD Repair Facility
  11 = NOD Tiberium Silo

  21 = GDI Tiberium Refinery
  22 = GDI Communications Center
  23 = GDI Power Plant
  24 = GDI Weapons Factory
  25 = GDI Construction Yard
  26 = GDI Guard Tower
  27 = GDI Infantry Barracks
  28 = GDI Landing Pad
  29 = GDI Advanced Guard Tower
  30 = GDI Repair Facility
  31 = GDI Tiberium Silo

  Cardinal Direction Numbers:

  0 = North
  1 = East
  2 = South
  3 = West
*/

DECLARE_SCRIPT (M00_Sound_Building_Ambient_RAD, "Building_ID=0:int, Building_Type=0:int, Building Facing=0:int")
{

};

DECLARE_SCRIPT(RMV_Audio_Timer_Delay, "Target_ID:int, Custom_Type=0:int, Custom_Param=0:int, Delay_Min=0.0:float, Delay_Max=0.0:float, Timer_ID=0:int, Repeat=1:int, Randomize_Each_Time=1:int")
{
	int type, param, id;
	float min, max, delay;
	bool repeat, random;
	
	void Created(GameObject * obj) override
	{
		type = Get_Int_Parameter("Custom_Type");
		param = Get_Int_Parameter("Custom_Param");
		id = Get_Int_Parameter("Timer_ID");
		min = Get_Float_Parameter("Delay_Min");
		max = Get_Float_Parameter("Delay_Max");
		repeat = (Get_Int_Parameter("Repeat") == 1) ? true : false;
		random = (Get_Int_Parameter("Randomize_Each_Time") == 1) ? true : false;
		if ((min == 0) && (max == 0))
		{
			GameObject *target;
			target = Commands->Find_Object(Get_Int_Parameter("Target_ID"));
			Commands->Send_Custom_Event(obj, target, type, param, 0.0f);
		}
		else
			if ((min != 0) && (max == 0))
			{
				delay = min;
				Commands->Start_Timer(obj, this, delay, id);
			}
			else if (max != 0)
			{
				delay = Commands->Get_Random(min, max);
				Commands->Start_Timer(obj, this, delay, id);
			}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if ((timer_id == id) && (repeat))
		{
			if (max !=0)
				if (random)
					delay = Commands->Get_Random(min, max);
			Commands->Start_Timer(obj, this, delay, id);
		}
	}
};

DECLARE_SCRIPT(RMV_Audio_Sound_Player_WAV, "WAV_File:string, Is_3D=1:int, Custom_Type=0:int, Custom_Param=0:int")
{
	bool is3d;
	int c_type, c_param;
	
	void Created(GameObject * /*obj*/) override
	{
		is3d = (Get_Int_Parameter("Is_3D") == 1) ? true : false;
		c_type = Get_Int_Parameter("Custom_Type");
		c_param = Get_Int_Parameter("Custom_Param");
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((c_type == type) && (c_param == param))
		{
			const char *sound;
			sound = Get_Parameter("WAV_File");
			if (is3d)
			{
				Commands->Create_3D_WAV_Sound_At_Bone(sound, obj, "ROOTTRANSFORM");
			}
			else 
			{
				Commands->Create_2D_WAV_Sound(sound);
			}
		}
	}
};

DECLARE_SCRIPT(RMV_Audio_Sound_Player_Preset, "Preset_Name:string, Custom_Type=0:int, Custom_Param=0:int, Sound_Origin:vector3")
{
	Vector3 origin;
	int c_type, c_param;
	
	void Created(GameObject * /*obj*/) override
	{
		origin = Get_Vector3_Parameter("Sound_Origin");
		c_type = Get_Int_Parameter("Custom_Type");
		c_param = Get_Int_Parameter("Custom_Param");
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((c_type == type) && (c_param == param))
		{
			const char *sound;
			sound = Get_Parameter("Preset_Name");
			Commands->Create_Sound(sound, origin, obj);
		}
	}
};

DECLARE_SCRIPT(RMV_Sound_Play_Near_Player, "Receive_Type:int, Receive_Param:int, Max_Offset:vector3, Sound_Preset:string, Frequency_Min:float, Frequency_Max=0.0:float")
{
	Vector3 offset, current;
	bool enabled;
	float freq_min, freq_max, freq;
	enum {SOUND_PLAYER_TIMER};
	
	void Created(GameObject * /*obj*/) override
	{
		enabled = false;
		offset = Get_Vector3_Parameter("Max_Offset");
		freq_min = Get_Float_Parameter("Frequency_Min");
		freq_max = Get_Float_Parameter("Frequency_Max");
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == Get_Int_Parameter("Receive_Type")) && (param == Get_Int_Parameter("Receive_Param")))
			enabled = !enabled;
		Timer_Expired(obj, SOUND_PLAYER_TIMER);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if ((enabled) && (timer_id == SOUND_PLAYER_TIMER))
		{
			const char *preset;
			preset = Get_Parameter("Sound_Preset");
			GameObject *player;
			player = Commands->Get_A_Star(Vector3(0,0,0));
			Vector3 player_pos = Commands->Get_Position(player);
			current.X = (int)Commands->Get_Random(-offset.X, offset.X);
			current.Y = (int)Commands->Get_Random(-offset.Y, offset.Y);
			current.Z = (int)Commands->Get_Random(0.0, offset.Z);
			Vector3 sound_pos = player_pos + current;
			if (freq_max == 0.0)
				freq = freq_min;
			else freq = Commands->Get_Random(freq_min, freq_max);
			Commands->Create_Sound(preset, sound_pos, obj);
			Commands->Start_Timer(obj, this, freq, SOUND_PLAYER_TIMER);
		}
	}
};


/*********************************************************************************************** 
**
** Building State Sound System - Plays sounds at a simple object location.  Requires Building
** controller ID to switch sound from normal to destroyed state.
**
** Requires one script attached to the speaker object and the ID of the building controller.
** On Created, the speaker finds the building controller and attaches a script with the speaker's
** ID as a param.  The script attached to the building sends a custom back to each individual
** speaker on events.
**
** There is a one-to-one ratio of scripts attached to the building controller and speakers
** placed for the building.
**
************************************************************************************************/

DECLARE_SCRIPT (M00_BuildingStateSoundSpeaker, "Sound_Normal:string,Sound_Destroyed:string,BuildingController_ID:int,Is_3D=1:int,Offset:vector3,Offset_Randomness:vector3,Frequency_Min=-1:float,Frequency_Max:float,Is_3D_Destroyed=1:int,Offset_Destroyed:vector3,Offset_Randomness_Destroyed:vector3,Frequency_Min_Destroyed=-1:float,Frequency_Max_Destroyed:float, Explosion_Name:string")
{

	int sound_int;
	bool building_destroyed;
	bool building_explode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( sound_int, 1 );
		SAVE_VARIABLE( building_destroyed, 3);
	}

	void Created( GameObject * obj ) override
	{
		building_destroyed = false;
		building_explode = true; // set play building explosions to true for when building is destroyed.

		GameObject *building = Commands->Find_Object( Get_Int_Parameter("BuildingController_ID") );
		if (building)
		{
			int speaker_id = Commands->Get_ID ( obj );
			char speaker[16];
			sprintf (speaker, "%d", speaker_id);
			Commands->Attach_Script( building, "M00_BuildingStateSoundController", speaker);
		}

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
		if ( building_destroyed == false )
		{
			bool is_3d = (Get_Int_Parameter("Is_3D") == 1) ? true : false;
			Vector3 pos = Commands->Get_Position(obj);
			pos += Get_Vector3_Parameter("Offset");
			Vector3 offset_random = Get_Vector3_Parameter("Offset_Randomness");
			pos.X += Commands->Get_Random(-offset_random.X, offset_random.X);
			pos.Y += Commands->Get_Random(-offset_random.Y, offset_random.Y);
			pos.Z += Commands->Get_Random(-offset_random.Z, offset_random.Z);
			if (is_3d)
			{
				Commands->Debug_Message("Playing 3D Sound\n");
				sound_int = Commands->Create_Sound(Get_Parameter("Sound_Normal"), pos, obj);
			}
			else
			{
				Commands->Debug_Message("Playing 2D Sound\n");
				sound_int = Commands->Create_2D_Sound(Get_Parameter("Sound_Normal"));
			}
		}
		else
		{
			bool is_3d_destroyed = (Get_Int_Parameter("Is_3D_Destroyed") == 1) ? true : false;
			Vector3 pos = Commands->Get_Position(obj);
			pos += Get_Vector3_Parameter("Offset_Destroyed");
			Vector3 offset_random = Get_Vector3_Parameter("Offset_Randomness_Destroyed");
			pos.X += Commands->Get_Random(-offset_random.X, offset_random.X);
			pos.Y += Commands->Get_Random(-offset_random.Y, offset_random.Y);
			pos.Z += Commands->Get_Random(-offset_random.Z, offset_random.Z);
			if (is_3d_destroyed)
			{
				Commands->Debug_Message("Playing 3D Sound\n");
				sound_int = Commands->Create_Sound(Get_Parameter("Sound_Destroyed"), pos, obj);
			}
			else
			{
				Commands->Debug_Message("Playing 2D Sound\n");
				sound_int = Commands->Create_2D_Sound(Get_Parameter("Sound_Destroyed"));
			}
		}

		Commands->Monitor_Sound(obj, sound_int);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (type == CUSTOM_EVENT_SOUND_ENDED)
		{
			if ( building_destroyed == false )
			{
				if (Get_Int_Parameter("Frequency_Min") != -1)
				{
					float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min"), Get_Float_Parameter("Frequency_Max"));
					Commands->Start_Timer(obj, this, time, 0);
				}
			}
			else
			{
				if (Get_Int_Parameter("Frequency_Min_Destroyed") != -1)
				{
					float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min_Destroyed"), Get_Float_Parameter("Frequency_Max_Destroyed"));
					Commands->Start_Timer(obj, this, time, 0);
				}
			}
		}

		if ( type == M00_CUSTOM_SOUND_BUILDING )
		{
			if ( param == 1 )
			{
				Commands->Stop_Sound( sound_int, true );
				Timer_Expired(obj, 0);
			}
		}

		if ( type == M00_CUSTOM_BUILDING_EXPLODE )
		{
			if ( building_explode )
			{
				const char *explosion_def_name;

				// insert EXPLODE sound code here...
				Vector3 pos = Commands->Get_Position(obj);
				pos.X += Commands->Get_Random(2, 5);
				pos.Y += Commands->Get_Random(2, 5);
				pos.Z += Commands->Get_Random(2, 5);

				explosion_def_name = Get_Parameter("Explosion_Name");
				Commands->Create_Explosion( explosion_def_name, pos, NULL );
	
				float time = Commands->Get_Random( 3, 6 );				
				Commands->Send_Custom_Event( obj, obj, M00_CUSTOM_BUILDING_EXPLODE, 1, time );
			}
		}

		if ( type == M00_CUSTOM_BUILDING_EXPLODE_NO )
		{
			building_explode = false;
		}

		if ( type == M00_CUSTOM_BUILDING_EXPLODE_YES )
		{
			building_explode = true;
		}
	}
};

DECLARE_SCRIPT (M00_BuildingStateSoundController, "BuildingSpeaker_ID:int")
{

	int object_from;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( object_from, 1 );
	}

	void Created( GameObject * /*obj*/) override
	{
		object_from = Get_Int_Parameter("BuildingSpeaker_ID");
	}

	void Killed( GameObject * obj, GameObject * /*killer*/ ) override
	{
		GameObject *target = Commands->Find_Object(object_from);

		if (target)
		{
			Commands->Send_Custom_Event( obj, target, M00_CUSTOM_SOUND_BUILDING, 1, 0.0f);		// Play Destroyed Sounds
			Commands->Send_Custom_Event( obj, target, M00_CUSTOM_BUILDING_EXPLODE, 1, 0.0f);	// Play Explosion Sounds
		}
	}
	
	void Custom (GameObject* obj, int type, intptr_t /*param*/, GameObject* /*sender*/) override
	{
		if ( type == M00_CUSTOM_BUILDING_EXPLODE_NO ) // flip play building explosions to false.
		{
			GameObject *target = Commands->Find_Object(object_from);
			if (target)
			Commands->Send_Custom_Event( obj, target, M00_CUSTOM_BUILDING_EXPLODE_NO, 1, 0.0f);
		}

		if ( type == M00_CUSTOM_BUILDING_EXPLODE_YES ) // flip play building explosions to true.
		{
			GameObject *target = Commands->Find_Object(object_from);
			if (target)
			Commands->Send_Custom_Event( obj, target, M00_CUSTOM_BUILDING_EXPLODE_YES, 1, 0.0f);
		}

	}
};
