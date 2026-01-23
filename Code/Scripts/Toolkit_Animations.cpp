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
*     Toolkit_Animations.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Animations Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 4 $
*     $Modtime: 10/06/00 4:18p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Animations.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

/*
Function - M00_Controller_Animation_RMV

  This function handles any animation calls.

  Parameters:

  GameObject * obj				= The object the animation is being called upon.
  GameObjObserverClass * script	= this
  const char * animation_name	= The name of the animation.
  bool loop						= Whether the animation should loop or not.
  int priority					= The Action Priority of this animation.
  int action_id					= The Action ID.
  int drop_frame				= The frame at which an item should be dropped.
  const char * drop_object		= The object to be dropped.
  const char * drop_bone		= The bone from which the object is dropped.
*/

void M00_Controller_Animation_RMV (GameObject * obj, GameObjObserverClass * script, const char * animation_name, bool loop, int priority, int action_id, int drop_frame, const char * drop_object, const char * drop_bone)
{
	ActionParamsStruct params;
	params.Set_Basic(script, priority, action_id);
	params.Set_Animation(animation_name, loop);
	Commands->Action_Play_Animation(obj, params);
	if (drop_frame != 0)
	{
		if (drop_object != NULL)
		{
			if (drop_bone != NULL)
			{
				Commands->Send_Custom_Event(obj, obj, M00_CUSTOM_ANIMATION_DROP_OBJECT, 0, drop_frame/30);
			}
		}
	}
}

/*
Editor Script - M00_Animation_Play_On_Activation_RMV

  This script plays an animation when the object it is attached to receives a custom.
  
  Parameters:

  Animation	= The animation to play.
  Loop		= Whether the animation should loop or not. 1 for looping. 0 for not.
*/

DECLARE_SCRIPT(M00_Animation_Play_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Action_Priority:int, Action_ID:int, Animation:string, Loop=0:int, Debug_Mode=0:int")
{
	int priority, action_id;
	bool debug_mode;

	void Created(GameObject * obj) override
	{
		priority = Get_Int_Parameter("Action_Priority");
		action_id = Get_Int_Parameter("Action_ID");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_RMV ACTIVATED.\n"));
			Do_Animation(obj);
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_RMV received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_RMV ACTIVATED.\n"));
				Do_Animation(obj);
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				Commands->Action_Reset(obj, (priority + 1));
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_RMV DEACTIVATED.\n"));
			}
		}
	}

	void Do_Animation(GameObject* obj)
	{
		const char *anim = Get_Parameter("Animation");
		int loop_int = Get_Int_Parameter("Loop");
		bool loop = (loop_int == 1) ? true : false;
		M00_Controller_Animation_RMV (obj, this, anim, loop, priority, action_id, 0, 0, 0);
	}
};

/*
Editor Script - M00_Animation_Play_Drop_Object_RMV

  This script plays an animation when the object it is attached to receives a custom.
  
  Parameters:

  Animation		= The animation to play.
  Drop_Frame	= The frame at which an object should drop.
  Drop_Object	= The object to drop.
  Drop_Bone		= The bone from which to drop the object.
*/

DECLARE_SCRIPT(M00_Animation_Play_Drop_Object_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=1:int, Action_Priority:int, Action_ID:int, Animation:string, Drop_Frame:int, Drop_Object:string, Drop_Bone:string, Debug_Mode=0:int")
{
	int		priority, action_id;
	bool	debug_mode;
	
	void Created(GameObject * obj) override
	{
		priority = Get_Int_Parameter("Action_Priority");
		action_id = Get_Int_Parameter("Action_ID");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_RMV ACTIVATED.\n"));
			const char *anim = Get_Parameter("Animation");
			int frame = Get_Int_Parameter("Drop_Frame");
			const char *object = Get_Parameter("Drop_Object");
			const char *bone = Get_Parameter("Drop_Bone");

			M00_Controller_Animation_RMV (obj, this, anim, 0, priority, action_id, frame, object, bone);
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		const char *anim = Get_Parameter("Animation");
		int frame = Get_Int_Parameter("Drop_Frame");
		const char *object = Get_Parameter("Drop_Object");
		const char *bone = Get_Parameter("Drop_Bone");
		
		SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_RMV received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_RMV ACTIVATED.\n"));
				M00_Controller_Animation_RMV (obj, this, anim, 0, priority, action_id, frame, object, bone);
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				Commands->Action_Reset(obj, (priority + 1));
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_RMV DEACTIVATED.\n"));
			}
		}
		if ((type == M00_CUSTOM_ANIMATION_DROP_OBJECT) && (param == 0))
		{
			Commands->Create_Object_At_Bone(obj, object, bone);
			SCRIPT_DEBUG_MESSAGE(("Object %s created by M00_Animation_Play_Drop_Object_RMV.\n", object));
		}
	}
};

/*
Editor Script - M00_Animation_Drop_Object_Attach_Script_RMV

  This script plays an animation when the object it is attached to receives a custom, drops
  a new object at a specified frame of the animation, and then attaches a script to the
  new object.
  
  Parameters:

  Animation		= The animation to play.
  Drop_Frame	= The frame at which to drop the object.
  Drop_Object	= The object to drop.
  Drop_Bone		= The bone from which the object is dropped.
  Script_Name	= The name of the script to attach.
  Script_Params	= The parameters for the attached script.
*/

DECLARE_SCRIPT(M00_Animation_Play_Drop_Object_Attach_Script_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Action_Priority:int, Action_ID:int, Animation:string, Drop_Frame:int, Drop_Object:string, Drop_Bone:string, Script_Name:string, Script_Params:string, Debug_Mode=0:int")
{
	int priority, action_id;
	bool		debug_mode;
	
	void Created(GameObject * obj) override
	{
		priority = Get_Int_Parameter("Action_Priority");
		action_id = Get_Int_Parameter("Action_ID");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_Attach_Script_RMV ACTIVATED.\n"));

			const char *anim = Get_Parameter("Animation");
			int frame = Get_Int_Parameter("Drop_Frame");
			const char *object = Get_Parameter("Drop_Object");
			const char *bone = Get_Parameter("Drop_Bone");

			M00_Controller_Animation_RMV (obj, this, anim, 0, priority, action_id, frame, object, bone);
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		const char *anim = Get_Parameter("Animation");
		int frame = Get_Int_Parameter("Drop_Frame");
		const char *object = Get_Parameter("Drop_Object");
		const char *bone = Get_Parameter("Drop_Bone");

		SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_Attach_Script_RMV received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_Attach_Script_RMV ACTIVATED.\n"));
				M00_Controller_Animation_RMV (obj, this, anim, 0, priority, action_id, frame, object, bone);
			}
			if (param == Get_Int_Parameter("Receive_Param_Off"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Animation_Play_Drop_Object_Attach_Script_RMV DEACTIVATED.\n"));
				Commands->Action_Reset(obj, (priority + 1));
			}
		}
		if ((type == M00_CUSTOM_ANIMATION_DROP_OBJECT) && (param == 0))
		{
			GameObject *new_object = Commands->Create_Object_At_Bone(obj, object, bone);
			SCRIPT_DEBUG_MESSAGE(("Object %s created by M00_Animation_Play_Drop_Object_Attach_Script_RMV.\n", object));

			const char *script = Get_Parameter("Script_Name");
			const char *params = Get_Parameter("Script_Params");
			char fixed_params[255];
			Fix_Params(params, fixed_params);

			Commands->Attach_Script(new_object, script, fixed_params);
			SCRIPT_DEBUG_MESSAGE(("Script %s attached to new object by M00_Animation_Play_Drop_Object_Attach_Script_RMV.", script));
		}
	}

	void Fix_Params(const char *input, char * output)
	{
		// copy string, converting | to ,
		while ( *input != 0 ) {
			if ( *input == '|' ) {
				*output++ = ',';
				input++;
			} else {
				*output++ = *input++;
			}
			
		}
		*output = 0;  // null terminate
	}
};
