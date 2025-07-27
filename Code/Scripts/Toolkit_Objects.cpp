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
*     Toolkit_Objects.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Object Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dave_s $
*     $Revision: 27 $
*     $Modtime: 12/11/01 11:13a $
*     $Archive: /Commando/Code/Scripts/Toolkit_Objects.cpp $
*
******************************************************************************/

#include "toolkit.h"

/*M00_Object_Create_RMV

  This script creates an object upon activation.

  Parameters:

  Object_To_Create	= The object to create.
  Location			= Where to create the object.
  Facing			= What facing to place the object at.
*/

DECLARE_SCRIPT(M00_Object_Create_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Object_To_Create:string, Location:vector3, Facing=0.00:float, Debug_Mode=0:int")
{
	bool debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(debug_mode, 1);
	}

	void Created(GameObject* obj)
	{
	/*	debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Object_Create_RMV ACTIVATED.\n"));
			Create_Object();
		}  */
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				SCRIPT_DEBUG_MESSAGE(("M00_Object_Create_RMV ACTIVATED.\n"));
				Create_Object();
			}
		}
	}

	void Create_Object (void)
	{
		Vector3 position = Get_Vector3_Parameter("Location");
		GameObject *new_object;
		new_object = Commands->Create_Object(Get_Parameter("Object_To_Create"), position);
		if (new_object)
		{
			Commands->Set_Facing(new_object, Get_Float_Parameter("Facing"));
		}
		else
		{
			SCRIPT_DEBUG_MESSAGE(("ERROR - M00_Object_Create_RMV failed to create a new object!\n"));
		}
	}
};

DECLARE_SCRIPT(M00_Object_Destroy_RMV, "Receive_Type_Activate:int, Debug_Mode=0:int")
{
	int r_type;
	bool debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(r_type, 1);
		SAVE_VARIABLE(debug_mode, 2);
	}
	
	void Created(GameObject * obj)
	{
		r_type = Get_Int_Parameter("Receive_Type_Activate");
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (debug_mode)
			Commands->Debug_Message("M00_Destroy_Object_RMV received custom of type %d and param %d.\n", type, param);
		if (type == r_type)
		{
			if (Commands->Find_Object(param) != NULL)
			{
				Commands->Destroy_Object(Commands->Find_Object(param));
				if (debug_mode)
					Commands->Debug_Message("M00_Destroy_Object_RMV - Object %d destroyed.\n", param);
			} else
			{
				if (debug_mode)
					Commands->Debug_Message("M00_Destroy_Object_RMV - Object %d not found!\n", param);
			}
		}
	}
};

DECLARE_SCRIPT(M00_Object_Create_Attach_Script_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On=1:int, Object_To_Create:string, Location:vector3, Facing=0.00:float, Debug_Mode=0:int, Script_To_Attach:string, Script_Params:string")
{
	bool debug_mode;

	void Created(GameObject* obj)
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;

		if (Get_Int_Parameter("Start_Now"))
		{
			if (debug_mode)
			{
				Commands->Debug_Message("M00_Object_Create_Attach_Script_RMV ACTIVATED.\n");
			}
			Create_Object();
		}
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (type == Get_Int_Parameter("Receive_Type"))
		{
			if (param == Get_Int_Parameter("Receive_Param_On"))
			{
				if (debug_mode)
				{
					Commands->Debug_Message("M00_Object_Create_Attach_Script_RMV ACTIVATED.\n");
				}
				Create_Object();
			}
		}
	}

	void Create_Object (void)
	{
		Vector3 position = Get_Vector3_Parameter("Location");
		GameObject *new_object;
		char *params, fixed_params[255];
		params = (char *)Get_Parameter("Script_Params");
		Fix_Params(params, fixed_params);
		new_object = Commands->Create_Object(Get_Parameter("Object_To_Create"), position);
		if (new_object)
		{
			Commands->Set_Facing(new_object, Get_Float_Parameter("Facing"));
			Commands->Attach_Script(new_object, Get_Parameter("Script_To_Attach"), fixed_params);
		}
		else
		{
			if (debug_mode)
			{
				Commands->Debug_Message("ERROR - M00_Object_Create_Attach_Script_RMV failed to create a new object!\n");
			}
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

DECLARE_SCRIPT(M00_Object_Destroy_Self_RMV, "Start_Now=1:int, Receive_Type=3:int, Receive_Param_On=1:int, Receive_Param_Off=0:int, Receive_Param_Activate:int, Debug_Mode=0:int")
{
	bool active, debug_mode;
	int receive_type, receive_param_on, receive_param_off, receive_param_activate;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active, 1);
		SAVE_VARIABLE(debug_mode, 2);
		SAVE_VARIABLE(receive_type, 3);
		SAVE_VARIABLE(receive_param_on, 4);
		SAVE_VARIABLE(receive_param_off, 5);
		SAVE_VARIABLE(receive_param_activate, 6);
	}
	
	void Created(GameObject * obj)
	{
		active = (Get_Int_Parameter("Start_Now") == 1) ? true : false;
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		receive_type = Get_Int_Parameter("Receive_Type");
		receive_param_on = Get_Int_Parameter("Receive_Param_On");
		receive_param_off = Get_Int_Parameter("Receive_Param_Off");
		receive_param_activate = Get_Int_Parameter("Receive_Param_Activate");
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (debug_mode)
			Commands->Debug_Message("M00_Trigger_Destroy_Self_RMV received custom, type %d and param %d.\n", type, param);
		if (type == receive_type)
		{
			if (param == receive_param_on)
			{
				active = true;
				if (debug_mode)
					Commands->Debug_Message("M00_Trigger_Destroy_Self_RMV on object %d made ACTIVE.\n", Commands->Get_ID(obj));
			}
			if (param == receive_param_off)
			{
				active = false;
				if (debug_mode)
					Commands->Debug_Message("M00_Trigger_Destroy_Self_RMV on object %d made INACTIVE.\n", Commands->Get_ID(obj));
			}
			if ((param == receive_param_activate) && (active))
			{
				if (debug_mode)
					Commands->Debug_Message("M00_Trigger_Destroy_Self_RMV activated on object %d, destroying.\n", Commands->Get_ID(obj));
				Commands->Destroy_Object(obj);
			}
		}
	}
};


/***********************************************************************************************************
*
*	PowerUp Creation
*	
*	- Includes ability to create any preset in the asset database by preset name
*	- Includes ability to check a random to drop item (Drop_Percentage)
*	- Arbitrary Create Position (Create_At_Death_Pos) & (Position)
*	- Provision to Play a "Spawning" Special Effect with Create_Anim_Effect_DAY Script
*
***********************************************************************************************************/

DECLARE_SCRIPT(M00_PowerUp_Create_When_Killed_JDG, "Preset_Name:string,Drop_Percentage=100:float,Create_At_Death_Pos=1:int,Position:vector3,Z_Offset=0.75:float,Spawn_Effect=0:int")
{

	void Killed( GameObject * obj, GameObject * killer ) 
	{

		const char *preset_name;
		bool create_death_pos_flag;
		float random;
		Vector3 create_position;
		float z_offset;
		float drop_percentage;
		bool spawn_effect;
		
		preset_name = Get_Parameter( "Preset_Name" );
		create_death_pos_flag = (Get_Int_Parameter( "Create_At_Death_Pos" ) == 1 ) ? true : false;
		z_offset = Get_Float_Parameter( "Z_Offset" );
		drop_percentage = Get_Float_Parameter( "Drop_Percentage" );
		random = Commands->Get_Random( 0, 1 );
		spawn_effect = (Get_Int_Parameter( "Spawn_Effect" ) == 1 ) ? true : false;

		if ( drop_percentage > random )
		{
			if ( create_death_pos_flag == true )
			{
				create_position = Commands->Get_Position( obj );
			}
			else
			{
				create_position = Get_Vector3_Parameter( "Position" );
			}

			if ( z_offset != 0 )
			{
				create_position.Z = create_position.Z + z_offset;
			}
			if (spawn_effect == true )
			{
				GameObject *spawn_object;
				spawn_object = Commands->Create_Object( "Spawner Created Special Effect", create_position );
				if ( spawn_object )
				{
					Commands->Attach_Script( spawn_object, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}
			}
			Commands->Create_Object( preset_name, create_position );
		}

	}
};


/***********************************************************************************************************
*
*	One Time Special Effect Player
*	Used with M00_PowerUp_Create_When_Killed_JDG
*	
*	- Attach this script to an object that is created through script to play a W3d animation
*	- (Effect_Model:string) is the name of the animation of the object.
*
***********************************************************************************************************/

DECLARE_SCRIPT(M00_Create_Anim_Effect_DAY, "Effect_Model:string")
{

	void Created( GameObject * obj )
	{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 1 );
			params.Set_Animation( Get_Parameter("Effect_Model"), false );
			Commands->Action_Play_Animation( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason)
	{
		if ( obj )
		{
			Commands->Destroy_Object( obj );
		}
	}

};

/***********************************************************************************************************
*
*	One Time Damage Modifier
*		
*	- Attach this script to an object that you do not want to recieve falling damage.
*	  Such as from a paradrop.
*
***********************************************************************************************************/

DECLARE_SCRIPT(M00_No_Falling_Damage_DME, "")
{
	bool ignore_next_dmg;
	float initial_health;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( ignore_next_dmg, 1 );
		SAVE_VARIABLE( initial_health, 2 );
	}

	void Created( GameObject * obj )
	{
		ignore_next_dmg = false;
		
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (type == CUSTOM_EVENT_FALLING_DAMAGE)
		{
			ignore_next_dmg = true;
			initial_health = Commands->Get_Health(obj);
		}
		
	}

	void Damaged( GameObject * obj, GameObject *damager, float amount)
	{
		if (ignore_next_dmg && damager == NULL)
		{			
			Commands->Set_Health(obj, initial_health);
		}
		ignore_next_dmg = false;
	}
};


/***********************************************************************************************************
*
*	Permanent Damage Modifier
*		
*	- Attach this script to an object that you do not want to recieve falling damage.
*
***********************************************************************************************************/

DECLARE_SCRIPT(M00_Permanent_No_Falling_Damage_IML, "")
{
	float initial_health;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( initial_health, 1 );
	}

	void Created( GameObject * obj )
	{
		initial_health = Commands->Get_Health(obj);
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (type == CUSTOM_EVENT_FALLING_DAMAGE)
		{
			initial_health = Commands->Get_Health(obj);
		}
	}

	void Damaged( GameObject * obj, GameObject *damager, float amount)
	{
		if (damager == NULL)
		{
			Commands->Set_Health(obj, initial_health);
		}
	}
};

DECLARE_SCRIPT (M00_Disable_Transition, "")
{
	void Created (GameObject * obj)
	{
		Commands->Enable_Vehicle_Transitions (obj, false);
	}
};

/***********************************************************************************************************
*
*	Death Script for Fire, Gas and Electric troops.
*		
*	- new script for default Fire, Gas and Electric troop deaths.
*	- Takes preset DeathType:string ("Fire", "Gas", "Electric") and plays appropriate animation
*		and weapon FX. (1 = flailing and smoke. 2 = Elec Twiching and smoke? 3 = choking and green smoke. )
*
***********************************************************************************************************/

DECLARE_SCRIPT ( M00_Fire_Gas_Elec_Death_DAK, "DeathType=1:int,HealthThreshhold=0.25:float" )
{

	bool firsttime; // prevents an infinante loop each time obj is damaged by DeathType.

	void Damaged ( GameObject *obj, GameObject *damager, float amount )
	{
		float bleah = Get_Float_Parameter( "HealthThreshhold" );

		// check to see if obj is at 25% or less of its health.
		if ( Commands->Get_Health ( obj ) <= bleah * Commands->Get_Max_Health ( obj ) )
		{
			// plays animation once.
			if ( firsttime == true )
			{
				firsttime = false;

				ActionParamsStruct params;
				params.Set_Basic( this, 99, 1 );

				// set animation based upon DeathType (Gas, Electric or Default to Fire)
				if ( Get_Int_Parameter( "DeathType" ) == 1 )
				{
					params.Set_Animation( "S_A_HUMAN.H_A_FLMA",0 ); // fire death
				}
				else
					if ( Get_Int_Parameter( "DeathType" ) == 2 )
					{
						params.Set_Animation( "S_A_HUMAN.H_A_6X05",0 ); // electric death
					}
					else params.Set_Animation( "S_A_HUMAN.H_A_6X01",0 ); // gas death
				
			Commands->Action_Play_Animation( obj, params );

			// begin DeathType damage 
			Commands->Apply_Damage( obj, 1.0f, Get_Parameter( "DeathType" ), NULL );
			}
		}
	}

	void Killed ( GameObject * obj, GameObject * killer )
	{
		ActionParamsStruct params;
		params.Set_Basic( this, 99, 1 );

		// set animation based upon DeathType (Gas, Electric or Default to Fire)
		if ( strcmp( Get_Parameter( "DeathType" ), "Fire" ) )
		{
			params.Set_Animation( "S_A_HUMAN.H_A_FLMA",0 ); // Fire Death
		}
			else
			if ( strcmp( Get_Parameter( "DeathType" ), "Gas" ) )
			{
				params.Set_Animation( "S_A_HUMAN.H_A_6X01",0 ); // Gas Death
			}
				else params.Set_Animation( "S_A_HUMAN.H_A_6X05",0 ); // Electrocution
			
		Commands->Action_Play_Animation( obj, params );
	}

	void Created ( GameObject *obj )
	{
		firsttime = true;
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason)
	{
		if((action_id == 1) && (reason == ACTION_COMPLETE_NORMAL))
		{
			// animation is complete. kill obj.
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
		}
	}
};

/***********************************************************************************************************
*
*	Vehicle Health Regeneration Script for GDI Mammoth Tanks and Tiberium Harvesters.
*		
*	- continually checks vehicle's health to see if it needs to be regenerated.
*	- regenerates vehicle to 50% max health
*
***********************************************************************************************************/

DECLARE_SCRIPT(M00_Vehicle_Regen_DAK, "" )
{
	void Created ( GameObject *obj )
	{
		Commands->Send_Custom_Event ( obj, obj, 0, 0, 0 );
	}

	void Custom (GameObject* obj, int type, int param, GameObject* sender) 
	{
		if ( type == 0 ) // regenerate health.
		{
			// check to see if health needs to be regenerated.
			if ( Commands->Get_Health ( obj ) < ( Commands->Get_Max_Health ( obj ) ) )
			{
				Commands->Apply_Damage (obj, -2, "RegenHealth", NULL);
			}
			// restart the timer
			Commands->Send_Custom_Event ( obj, obj, 0, 0, 1 );
		}
	}
};

DECLARE_SCRIPT(M00_PCT_Pokable_DAK, "" )
{
	void Created ( GameObject *obj )
	{
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
		Commands->Display_Health_Bar( obj, false );
	}
};

DECLARE_SCRIPT(M00_Send_Object_ID, "Receiver_ID:int, Param=0:int, Delay=1.0f:int")
{
	int r_type;
	bool debug_mode;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(r_type, 1);
		SAVE_VARIABLE(debug_mode, 2);
	}
	
	void Created(GameObject * obj)
	{
		// Send Custom and Parameter to Receiver object
		int receiver_id = Get_Int_Parameter("Receiver_ID");
		int parameter = Get_Int_Parameter("Param");
		float delay = Get_Float_Parameter("Delay");
		GameObject * receiver = Commands->Find_Object(receiver_id);
		if(receiver)
		{
			Commands->Send_Custom_Event ( obj, Commands->Find_Object(receiver_id), M00_SEND_OBJECT_ID, parameter, delay );
		}
		else
		{
			Commands->Debug_Message("M00_Send_Object_ID on object_id %d Warning! Receiver_ID %d does not exist! /n", Commands->Get_ID(obj), receiver_id);
		}
	}
};