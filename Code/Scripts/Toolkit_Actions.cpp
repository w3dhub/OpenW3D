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
*     Toolkit_Actions.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Actions Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 10 $
*     $Modtime: 11/03/00 2:25p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Actions.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

/*Editor Script - M00_Action

  Animations are not handled with this script - they are handled separately.

  Parameters:

  _Move_Target_ID			= The Target ID to move towards.
  _Move_Following			= Whether this unit should follow the Target ID or not.
  _Move_Destination			= The Vector3 location of the movement destination.

  _Move_Waypath_ID			= The ID of a waypath to follow.
  _Move_Waypath_Start_ID	= The starting point to use on the waypath.
  _Move_Waypath_End_ID		= The ending point to use on the waypath.
  _Move_Waypath_Splined		= Whether to use splined movement or not on waypaths.
  _Move_Waypath_Looping		= If the unit should circle on the waypath.

  _Move_Patrol_Radius		= A radius to use if patrolling an area.
  _Move_Patrol_Loiter_Time	= How long to wait at each point until patrolling again.

  _Move_Arrive_Distance		= The distance to close to with the destination.
  _Move_Speed				= The speed at which the unit moves.
  _Move_Crouch				= Whether to move crouched or not.
  _Move_Backwards			= Whether to move backwards or not.
  _Move_Pathfind			= Whether to use pathfinding data for movement or not.
  
  _Attack_Target_ID			= The attack target's ID.
  _Attack_Location			= The location of the attack (if no target).

  _Attack_Range				= The maximum effective range of the attack.
  _Attack_Deviation			= The deviation of the attack.
  _Attack_Primary			= Whether to use the primary weapon or not.
  _Attack_Crouched			= Whether to crouch when firing or not.
*/

DECLARE_SCRIPT(M00_Action, "Start_Now=0:int, Receive_Type=14:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Action_Priority=99:int, Action_ID=0:int, _Move_Target_ID=0:int, _Move_Following=0:int, _Move_Destination:vector3, _Move_Waypath_ID=0:int, _Move_Waypath_Start_ID=0:int, _Move_Waypath_End_ID:int, _Move_Waypath_Splined=0:int, _Move_Arrive_Distance=1.0:float, _Move_Speed=1.0:float, _Move_Crouch=0:int, _Move_Backwards=0:int, _Move_Pathfind=1:int, _Attack_Target_ID=0:int, _Attack_Location:vector3, _Attack_Range=100.0:float, _Attack_Deviation=0.0:float, _Attack_Primary=1:int, _Attack_Crouched=0:int, Debug_Mode=0:int")
{
	bool		debug_mode;
	bool		script_active;
	bool		attacking;
	bool		primary_weapon;

	Vector3		current_position;
	Vector3		current_fire_position;
	Vector3		current_attack_location;
	Vector3		empty_vector;

	void Created(GameObject* obj) override
	{
		ActionParamsStruct params;
		script_active = false;
		attacking = false;
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		
		GameObject*	move_target;
		GameObject* attack_target;
			
		move_target = Commands->Find_Object(Get_Int_Parameter("_Move_Target_ID"));
		attack_target = Commands->Find_Object(Get_Int_Parameter("_Attack_Target_ID"));
		primary_weapon = (Get_Int_Parameter("_Attack_Primary") == 1) ? true : false;
		current_attack_location = Get_Vector3_Parameter("_Attack_Location");
		empty_vector = Vector3(0.0f,0.0f,0.0f);

		params.Set_Basic(this, Get_Int_Parameter("Action_Priority"), Get_Int_Parameter("Action_ID"));
		params.Set_Movement(Get_Vector3_Parameter("_Move_Destination"), Get_Float_Parameter("_Move_Speed"), Get_Float_Parameter("_Move_Arrive_Distance"));
		params.Set_Attack(Get_Vector3_Parameter("_Attack_Location"), Get_Float_Parameter("_Attack_Range"), Get_Float_Parameter("_Attack_Deviation"), primary_weapon);
		
		params.MoveBackup = (Get_Int_Parameter("_Move_Backwards") == 1) ? true : false;
		params.MoveFollow = (Get_Int_Parameter("_Move_Following") == 1) ? true : false;
		params.MoveCrouched = (Get_Int_Parameter("_Move_Crouch") == 1) ? true : false;
		params.MovePathfind = (Get_Int_Parameter("_Move_Pathfind") == 1) ? true : false;
		
		params.AttackCrouched = (Get_Int_Parameter("_Attack_Crouched") == 1) ? true : false;
		params.WaypathID = Get_Int_Parameter("_Move_Waypath_ID");
		params.WaypointStartID = Get_Int_Parameter("_Move_Waypath_Start_ID");
		params.WaypointEndID = Get_Int_Parameter("_Move_Waypath_End_ID");
		params.WaypathSplined = (Get_Int_Parameter("_Move_Waypath_Splined") == 1) ? true : false;
		
		if (attack_target)
		{
			// Attacking a target, override the attack location values

			params.Set_Attack(attack_target, Get_Float_Parameter("_Attack_Range"), Get_Float_Parameter("_Attack_Deviation"), primary_weapon);
			attacking = true;
		}
		if (current_attack_location != empty_vector)
		{
			// Attacking a location, be sure to turn on attacking bool

			attacking = true;
		}
		if (move_target)
		{
			// Moving toward a target, override other movement values

			params.Set_Movement(move_target, Get_Float_Parameter("_Move_Speed"), Get_Float_Parameter("_Move_Arrive_Distance"));
			params.WaypathID = 0;
			params.WaypointStartID = 0;
			params.WaypointEndID = 0;
			params.WaypathSplined = false;
		}
		if (params.MoveLocation != empty_vector)
		{
			// Moving to a location, override other movement values

			params.WaypathID = 0;
			params.WaypointStartID = 0;
			params.WaypointEndID = 0;
			params.WaypathSplined = false;
		}
		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Action ACTIVATED.\n"));
			script_active = true;
			if (script_active)
			{
				// Check if we are firing and moving, or just moving.
	
				if (attacking)
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Action is moving and attacking.\n"));
	
					Commands->Action_Attack(obj, params);
				}
				else
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Action is moving only - no attack.\n"));
	
					Commands->Action_Goto(obj, params);
				}
			}	
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Action received custom type %d, param %d.\n", type, param));
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				GameObject*	move_target;
				GameObject* attack_target;

				move_target = Commands->Find_Object(Get_Int_Parameter("_Move_Target_ID"));
				attack_target = Commands->Find_Object(Get_Int_Parameter("_Attack_Target_ID"));
				
				ActionParamsStruct params;
				params.Set_Basic(this, Get_Int_Parameter("Action_Priority"), Get_Int_Parameter("Action_ID"));
				params.Set_Movement(Get_Vector3_Parameter("_Move_Destination"), Get_Float_Parameter("_Move_Speed"), Get_Float_Parameter("_Move_Arrive_Distance"));
				params.Set_Attack(Get_Vector3_Parameter("_Attack_Location"), Get_Float_Parameter("_Attack_Range"), Get_Float_Parameter("_Attack_Deviation"), primary_weapon);
			
				params.MoveBackup = (Get_Int_Parameter("_Move_Backwards") == 1) ? true : false;
				params.MoveFollow = (Get_Int_Parameter("_Move_Following") == 1) ? true : false;
				params.MoveCrouched = (Get_Int_Parameter("_Move_Crouch") == 1) ? true : false;
				params.MovePathfind = (Get_Int_Parameter("_Move_Pathfind") == 1) ? true : false;
			
				params.AttackCrouched = (Get_Int_Parameter("_Attack_Crouched") == 1) ? true : false;
				params.WaypathID = Get_Int_Parameter("_Move_Waypath_ID");
				params.WaypointStartID = Get_Int_Parameter("_Move_Waypath_Start_ID");
				params.WaypointEndID = Get_Int_Parameter("_Move_Waypath_End_ID");
				params.WaypathSplined = (Get_Int_Parameter("_Move_Waypath_Splined") == 1) ? true : false;
				
				SCRIPT_DEBUG_MESSAGE(("M00_Action ACTIVATED.\n"));

				if (attack_target)
				{
					// Attacking a target, override the attack location values
	
					params.Set_Attack(attack_target, Get_Float_Parameter("_Attack_Range"), Get_Float_Parameter("_Attack_Deviation"), primary_weapon);
					attacking = true;
				}
				if (current_attack_location != empty_vector)
				{
					// Attacking a location, be sure to turn on attacking bool
		
					attacking = true;
				}
				if (move_target)
				{
					// Moving toward a target, override other movement values
	
					params.Set_Movement(move_target, Get_Float_Parameter("_Move_Speed"), Get_Float_Parameter("_Move_Arrive_Distance"));
					params.WaypathID = 0;
					params.WaypointStartID = 0;
					params.WaypointEndID = 0;
					params.WaypathSplined = false;
				}
				if (params.MoveLocation != empty_vector)
				{
					// Moving to a location, override other movement values
		
					params.WaypathID = 0;
					params.WaypointStartID = 0;
					params.WaypointEndID = 0;
					params.WaypathSplined = false;
				}
	
				if (script_active)
				{
				// Check if we are firing and moving, or just moving.
				if (attacking)
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Action is moving and attacking.\n"));

				Commands->Action_Attack(obj, params);
				}
				else
				{
					SCRIPT_DEBUG_MESSAGE(("M00_Action is moving only - no attack.\n"));
	
					Commands->Action_Goto(obj, params);
				}
			}
		}
		if (param == Get_Int_Parameter("Receive_Param_Off"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Action DEACTIVATED.\n"));
			script_active = false;
			Commands->Action_Reset(obj, Get_Int_Parameter("Action_Priority")+1);
		}
	}
}

	void Action_Complete(GameObject * /*obj*/, int action_id, ActionCompleteReason reason) override
	{
		if (script_active)
		{
			if (action_id == Get_Int_Parameter("Action_ID"))
			{
				switch (reason)
					{
					case (ACTION_COMPLETE_LOW_PRIORITY):
						{
							SCRIPT_DEBUG_MESSAGE(("M00_Action returned a Low Priority Warning.\n"));
							break;
						}
					case (ACTION_COMPLETE_PATH_BAD_START):
						{
							SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Action returned a Path Bad Start Error!\n"));
							break;
						}
					case (ACTION_COMPLETE_PATH_BAD_DEST):
						{
							SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Action returned a Path Bad Destination Error!\n"));
							break;
						}
					case (ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE):
						{
							SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Action returned a No Progress Made Error!\n"));
							break;
						}
					case (ACTION_COMPLETE_ATTACK_OUT_OF_RANGE):
						{
							SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Action returned an Attack Out Of Range Error!\n"));
							break;
						}
					case (MOVEMENT_COMPLETE_ARRIVED):
						{
							SCRIPT_DEBUG_MESSAGE(("M00_Action returned a Movement Complete Arrived.\n"));
							break;
						}
					default:
						{
							SCRIPT_DEBUG_MESSAGE(("M00_Action returned with default success.\n"));
							break;
						}
					}
	
			}
		}
	}
};

DECLARE_SCRIPT(M00_Action_Set_Home_Location, "Start_Now=1:int, Receive_Type:int, Receive_Param_On:int, Home_Location:vector3, Wander_Distance=99999.0:float")
{
	void Created(GameObject* obj) override
	{
		if (Get_Int_Parameter("Start_Now"))
		{
			Set_Innate_Position (obj);
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				Set_Innate_Position (obj);
			}
		}
	}

	void Set_Innate_Position (GameObject* obj)
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Get_Vector3_Parameter("Home_Location"), Get_Float_Parameter("Wander_Distance"));
	}
};

DECLARE_SCRIPT (M00_Action_Innate_Follow_Waypath, "Waypath_ID:int")
{

	void Created (GameObject* obj) override
	{
		Perform_Action (obj);
	}

	void Action_Complete(GameObject * /*obj*/, int /*action_id*/, ActionCompleteReason /*complete_reason*/) override
	{
		//Commands->Start_Timer (obj, this, 3.0f, 32);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == 32)
		{
			Perform_Action (obj);
		}
	}

	void Perform_Action (GameObject* /*obj*/)
	{
		/*
		ActionParamsStruct params;

		params.Set_Basic(this, 29, 11);
		params.Set_Movement(Vector3(0.0f,0.0f,0.0f), 0.5f, 1.0f);
		params.WaypathID = Get_Int_Parameter("Waypath_ID");
		Commands->Action_Goto (obj, params);
		*/
	}
};

DECLARE_SCRIPT (M00_Action_Innate_Follow_Player, "")
{
	void Created (GameObject* obj) override
	{
		Commands->Start_Timer (obj, this, 1.0f, 33);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == 33)
		{
			Vector3 my_loc = Commands->Get_Position(obj);
			GameObject * nearest = Commands->Get_A_Star (my_loc);
			if (nearest)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 39, 11);
				params.Set_Movement(nearest, 1.0f, 3.0f);
				Commands->Action_Goto (nearest, params);
				Commands->Start_Timer (obj, this, 10.0f, 33);
			}
		}
	}
};
