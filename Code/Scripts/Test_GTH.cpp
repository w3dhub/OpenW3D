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

#include <cstdint>
#include "scripts.h"
#include <string.h>
#include "Toolkit.h"

#define GTH_DEBUG	0

#if (GTH_DEBUG)
#define GTH_DEBUG_INT( x , format )			Commands->Display_Int( x, format )
#define GTH_DEBUG_FLOAT( x , format )		Commands->Display_Float( x, format )
#else
#define GTH_DEBUG_INT( x , format )			
#define GTH_DEBUG_FLOAT( x , format )		
#endif


/*
** GTH_Drop_Object_On_Death  (verified)
** This script will create an object at the position of the object when it dies.
** 
** Params:
** Drop_Object - name of the preset to create an instance of
** Drop_Height - float meters to add to the Z coord of the original object when creating the drop obj
** Probability - int between 1 and 100, chance that the object will be created
*/
DECLARE_SCRIPT(GTH_Drop_Object_On_Death, "Drop_Object=:string,Drop_Height=0.25:float,Probability=100:int")
{
	void Killed( GameObject * obj, GameObject * /*killer*/ ) override
	{
		const char * obj_name = Get_Parameter("Drop_Object");
		
		bool doit = false;
		float probability = Get_Int_Parameter("Probability");
		
		if (probability >= 100) {
			doit = true;
		} else {
			int random = Commands->Get_Random_Int(0,100);
			doit = random < probability;
		}

		if ((obj_name != NULL) && (doit)) {
			Vector3 spawn_location = Commands->Get_Position ( obj );
			spawn_location.Z = spawn_location.Z + Get_Float_Parameter("Drop_Height");
			Commands->Create_Object ( obj_name, spawn_location );
		}
	}
};

/*
** GTH_Drop_Object_On_Death_Zone (verified)
** This script is just like the other drop object on death except that it must also
** be activated by a custom message from another script.  Use the GTH_Zone_Send_Custom 
** to enable and disable this script.  
**
** Params:
** Custom_Message - message id that turns this script on or off, use message ID's greater than 10000!
** Drop_Object - name of the preset to create an instance of
** Drop_Height - float meters to add to the Z coord of the original object when creating the drop obj
** Probability - int between 1 and 100, chance that the object will be created
*/ 
DECLARE_SCRIPT(GTH_Drop_Object_On_Death_Zone, "Custom_Message=20000:int,Drop_Object=:string,Drop_Height=0.25:float,Probability=100:int")
{
	bool enabled;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(enabled, 1);
	}

	void Created( GameObject * /*obj*/ ) override
	{
		enabled = false;
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == Get_Int_Parameter("Custom_Message")) {
			enabled = (param==1);
			GTH_DEBUG_INT(enabled,"GTH_Drop_Object_On_Death_Zone custom message: enable = %d\n");
		}
	}

	void Killed( GameObject * obj, GameObject * /*killer*/ ) override
	{
		GTH_DEBUG_INT(0,"GTH_Drop_Object_On_Death_Zone Killed callback\n");
		if (!enabled) {
			GTH_DEBUG_INT(0,"not in zone\n");
			return;
		}
		
		const char * obj_name = Get_Parameter("Drop_Object");
		
		bool doit = false;
		float probability = Get_Int_Parameter("Probability");
		
		if (probability >= 100) {
			doit = true;
		} else {
			int random = Commands->Get_Random_Int(0,100);
			doit = random < probability;
		}

		if ((obj_name != NULL) && (doit)) {

			Vector3 spawn_location = Commands->Get_Position ( obj );
			spawn_location.Z = spawn_location.Z + Get_Float_Parameter("Drop_Height");

			GTH_DEBUG_INT(0,"Creating object ");
			GTH_DEBUG_INT(0,obj_name);
			GTH_DEBUG_FLOAT(spawn_location.X," x=%f, ");
			GTH_DEBUG_FLOAT(spawn_location.Y," y=%f, ");
			GTH_DEBUG_FLOAT(spawn_location.Z," z=%f, ");
			
			Commands->Create_Object ( obj_name, spawn_location );
		}
	}
};

/*
** GTH_Zone_Send_Custom (verified)
** This script lets you send a custom message to an object on enter and exit of a zone.  To talk 
** to the "drop in death zone" script, send the same custom message with 1 for Enter_Param and
** 0 for Exit_Param...
** 
** Params:
** Enter_Message = message id to send when an object enters this zone
** Enter_Param = message parameter to send when an object enters
** Exit_Message = message id to send when an object exits
** Exit_Param = message id to send when and object exits
*/
DECLARE_SCRIPT(GTH_Zone_Send_Custom, "Enter_Message=20000:int,Enter_Param=1:int,Exit_Message=20000:int,Exit_Param=0:int")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		int message = Get_Int_Parameter("Enter_Message");
		GTH_DEBUG_INT(message,"GTH_Zone_Send_Custom sending enter message %d\n");
		if (message != 0) {
			Commands->Send_Custom_Event(obj,enterer,message,Get_Int_Parameter("Enter_Param"), 0.0f);
		}
	}

	void Exited( GameObject * obj, GameObject * exiter ) override
	{
		int message = Get_Int_Parameter("Exit_Message");
		GTH_DEBUG_INT(message,"GTH_Zone_Send_Custom sending exit message %d\n");
		if (message != 0) {
			Commands->Send_Custom_Event(obj,exiter,message,Get_Int_Parameter("Exit_Param"), 0.0f);
		}
	}
};

/*
** GTH_Create_Object_On_Enter (verified)
** This script will create an object when a script zone is entered by a game object.  Use it
** to fire off cinematics for example... 
** 
** Params:
** Create_Object - name of the preset to create an instance of
** Position - world space position to create the object at
** Min_Delay - amount of time to wait before re-enabling the script once it has fired
** Max_Creations - maximum number of times the script should create an object
** Probability - integer between 1 and 100, chance on any given "Enter" that the object will be created
** Player_Type - type of player that can trigger integer, 0 = Nod, 1 = GDI, 2 = any
*/
DECLARE_SCRIPT(GTH_Create_Object_On_Enter, "Create_Object=:string,Position:vector3,Min_Delay=10:int,Max_Creations=1:int,Probability=100:int,Player_Type=2:int")
{
	bool script_enabled;
	int trigger_count;
	
	enum { TIMER_ID_REENABLE = 0 };

public:
	GTH_Create_Object_On_Enter(void) :
		script_enabled(true),
		trigger_count(0)
	{
	}

protected:
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(script_enabled, 1);
		SAVE_VARIABLE(trigger_count, 2);
	}
	
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (script_enabled == false) {
			return;
		}

		// Check player type, if it doesn't match, just return
		int our_player_type = Get_Int_Parameter("Player_Type");
		if (our_player_type != 2) {
			int enter_player_type = Commands->Get_Player_Type(enterer);
			if (enter_player_type != our_player_type) {
				return;
			}
		}

		// Check probability, if the check fails, just return
		float probability = Get_Int_Parameter("Probability");
		if (probability < 100) {
			int random = Commands->Get_Random_Int(0,100);
			if (random > probability) {
				return;
			}
		}
		
		GTH_DEBUG_INT( 0, "Enter callback in GTH_Create_Object_On_Enter" );

		// Inc our trigger count, if we're allowed to fire again, set a timer to re-enable the script
		trigger_count++;
		script_enabled = false;
		if (trigger_count < Get_Int_Parameter("Max_Creations")) {
			Commands->Start_Timer(obj, this, Get_Float_Parameter("Min_Delay"), TIMER_ID_REENABLE);
		}

		// Ok, create the object
		const char * obj_name = Get_Parameter("Create_Object");
		Vector3 obj_pos = Get_Vector3_Parameter("Position");
		
		if (obj_name != NULL) {

			GTH_DEBUG_INT( 0, obj_name );
			GTH_DEBUG_INT( obj_pos.X, " x: %f, " );
			GTH_DEBUG_INT( obj_pos.Y, "y: %f, " );
			GTH_DEBUG_INT( obj_pos.Z, "z: %f\n" );

			Commands->Create_Object ( obj_name, obj_pos );
		}
	}

	void Timer_Expired( GameObject * /*obj*/, int timer_id ) override
	{
		if (timer_id == TIMER_ID_REENABLE) {
			script_enabled = true;
		}
	}

};

#if 0
/*
** GTH_Speed_Controlled_Anim
** NOTE: THIS PROBABLY WON'T WORK IN A NET GAME ANYWAY....  DELETE ME
** Stop_Speed - float, maximum speed where the object is considered "stopped"
** Stop_Anim - name of the anim to play when the unit is "stopped"
** Walk_Speed - float, max speed where the object is considered "walking"
** Walk_Anim - name of the anim to play when the unit is "walking"
** Run_Anim - name of anim to play when the unit is moving faster than "walking"
** Update_Delay - delay between re-evaluating the state of the object (0.1 = 10 times a second)
*/
DECLARE_SCRIPT(GTH_Speed_Controlled_Anim,"Stop_Speed=0.1:float,StopAnim=none:string,Walk_Speed=5.0:float,Walk_Anim=none:string,Run_Anim=none:string,Update_Rate=0.1:float")
{
	enum { STOPPED = 0, WALKING, RUNNING };
	enum { TIMER_ID_TICK = 0xbeef };

	int cur_state;
	Vector3 last_pos;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(cur_state, 1);
		SAVE_VARIABLE(last_pos, 2);
	}

	void Created( GameObject * obj ) override
	{
		// initialize the state
		cur_state = -1;
		last_pos = Commands->Get_Position( obj );
		update_state(obj,eval_speed(obj));
		
		// start up our update timer
		Commands->Start_Timer(obj, this, Get_Float_Parameter("Update_Rate"), TIMER_ID_TICK);
	}

		
	void	( * Set_Animation )( GameObject * obj, const char * anim_name, bool looping, const char * sub_obj_name = NULL, float start_frame = 0.0F, float end_frame = -1.0F, bool is_blended = false );


	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		if (timer_id == TIMER_ID_TICK) {
			update_state(obj,eval_speed(obj));
			Commands->Start_Timer(obj, this, get_update_rate(), TIMER_ID_TICK);
		}
	}

	float get_update_rate( void ) 
	{
		float update_rate = Get_Float_Parameter("Update_Rate");
		if (update_rate <= 0.01f) {
			update_rate = 0.01f;
		}
		return update_rate;
	}

	int	eval_speed( GameObject * obj )
	{
		// This function evaluates the object's current speed and returns the state
		Vector3 cur_pos = Commands->Get_Position(obj);
		Vector3 vel = (cur_pos - last_pos) / get_update_rate();
		last_pos = cur_pos;

		float speed = vel.Length();
		if (speed <= Get_Float_Parameter("Stop_Speed")) {
			return STOPPED;
		}

		if (speed <= Get_Float_Parameter("Walk_Speed")) {
			return WALKING;
		}

		return RUNNING;
	}
	
	void update_state( GameObject * obj, int new_state )
	{
		// This function plugs in the animation depending on the current state
		if (new_state != cur_state) {
			cur_state = new_state;
			const char * anim = NULL;
			switch( cur_state )
			{
			case STOPPED:
				{
					anim = Get_Parameter("Stop_Anim");
				}
				break;
			case WALKING:
				{ 
					anim = Get_Parameter("Walk_Anim");
				}
				break;
			case RUNNING:
				{
					anim = Get_Parameter("Run_Anim");
				}
				break;
			};

			if (anim != NULL) {
				Commands->Set_Animation( obj, anim, true );
			}
		}
	}
};
#endif

/*
** GTH_On_Enter_Mission_Complete (verified)
** When you enter a zone with this script on it, the mission is complete. NOTE, this only
** works in single player levels
** 
** Parameters:
** Success - 0 = mission failed, 1 = missuion succeeded
** Player_Type - type of player allowed to trigger, 0=nod, 1=gdi, 2=any
*/
DECLARE_SCRIPT(GTH_On_Enter_Mission_Complete, "Success=1:int, Player_Type=2:int" )
{
	void Entered( GameObject * /*obj*/, GameObject * enterer ) override
	{
		GTH_DEBUG_INT(0,"GTH_On_Enter_Mission_Complete::Entered\n");

		// check the player type
		int our_player_type = Get_Int_Parameter("Player_Type");
		GTH_DEBUG_INT(our_player_type,"Script desired player type: %d\n");

		if (our_player_type != 2) {
		
			int enter_player_type = Commands->Get_Player_Type(enterer);
			GTH_DEBUG_INT(enter_player_type,"Enterer player type: %d\n");
			if (enter_player_type != our_player_type) {
				return;
			}
		}
		
		// if we get here, complete the mission
		int success = Get_Int_Parameter("Success");
		GTH_DEBUG_INT(success,"Calling Mission_Complete(%d)\n");
		
		if (success == 0) {
			Commands->Mission_Complete(false);
		} else {
			Commands->Mission_Complete(true);
		}
	}	
};


/*
** GTH_On_Death_Mission_Complete (verified)
** When you kill something this script on it, the mission is complete.  NOTE this 
** only works in single-player levels.
** 
** Parameters:
** Success - 0 = mission failed, 1 = missuion succeeded
** Player_Type - type of player allowed to trigger, 0=nod, 1=gdi, 2=any
*/
DECLARE_SCRIPT(GTH_On_Killed_Mission_Complete, "Success=1:int, Player_Type=2:int" )
{
	void Killed( GameObject * /*obj*/, GameObject * killer ) override
	{
		GTH_DEBUG_INT(0,"GTH_On_Killed_Mission_Complete::Killed\n");
	
		// check the player type
		int our_player_type = Get_Int_Parameter("Player_Type");
		GTH_DEBUG_INT(our_player_type,"Script desired player type: %d\n");

		if (our_player_type != 2) {
			int enter_player_type = Commands->Get_Player_Type(killer);
			GTH_DEBUG_INT(enter_player_type,"Enterer player type: %d\n");

			if (enter_player_type != our_player_type) {
				return;
			}
		}
		
		// if we get here, complete the mission
		int success = Get_Int_Parameter("Success");
		GTH_DEBUG_INT(success,"Calling Mission_Complete(%d)\n");

		if (success == 0) {
			Commands->Mission_Complete(false);
		} else {
			Commands->Mission_Complete(true);
		}
	}	
};


/*
** GTH_Create_Objective
** Adds an objective to the mission when the specified action (create, enter, poke, or kill) 
** happens to the object with this script on it.
** 
** params:
** Creation_Type - 0=Create, 1=Entered, 2=Poked, 3=Killed
** Objective_ID - id of the objective, match this with the "GTH_Objective_Complete" script
** Objective_Type - 0=PRIMARY, 1=SECONDARY
** Short_Desc_ID - string id for short description
** Long_Desc_ID - string id for long description
** Priority - priority of this objective
** Position - 3d position of the objective 
** Pog_Texture - tga file for the objective pog
** Pog_Text_ID - string id for the pog text (usually something like IDS_POG_DESTROY)
** 
*/
DECLARE_SCRIPT(GTH_Create_Objective,"Creation_Type=0:int,Objective_ID=0:int,Objective_Type=0:int,Short_Desc_ID=0:int,Long_Desc_ID=0:int,Priority=90:float,Position:vector3,Pog_Texture:string")
{
	bool	already_triggered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(already_triggered, 1);
	}

	void	Created( GameObject * /*obj*/ ) override							{ if (Get_Int_Parameter("Creation_Type") == 0) create_objective(); }
	void	Entered( GameObject * /*obj*/, GameObject * /*enterer*/ ) override	{ if (Get_Int_Parameter("Creation_Type") == 1) create_objective(); }
	void	Poked( GameObject * /*obj*/, GameObject * /*poker*/ ) override		{ if (Get_Int_Parameter("Creation_Type") == 2) create_objective(); }
	void	Killed( GameObject * /*obj*/, GameObject * /*killer*/ ) override		{ if (Get_Int_Parameter("Creation_Type") == 3) create_objective(); }

	void create_objective(void) 
	{
		if (already_triggered) {
			return;
		}

		// create the objective
		int id = Get_Int_Parameter("Objective_ID");
		int type;

		if (Get_Int_Parameter("Objective_Type") == 0) {
			type = OBJECTIVE_TYPE_PRIMARY;
		} else {
			type = OBJECTIVE_TYPE_SECONDARY;
		}
		int short_desc_id = Get_Int_Parameter("Short_Desc_ID");
		int long_desc_id = Get_Int_Parameter("Long_Desc_ID");

		Commands->Add_Objective(id,type,OBJECTIVE_STATUS_PENDING,short_desc_id, NULL, long_desc_id );

		// set up the radar blip
		Vector3 objective_pos = Get_Vector3_Parameter("Position");
		Commands->Set_Objective_Radar_Blip(id, objective_pos);

		// set up the hud stuff
		const char * pog = Get_Parameter("Pog_Texture");
		float priority = Get_Float_Parameter("Priority");
		int pog_text_id = Get_Int_Parameter("Pog_Text_ID");
		
		Commands->Set_Objective_HUD_Info_Position(id, priority, pog, pog_text_id, objective_pos);
	}
};


/*
** GTH_Objective_Complete
** Ends an objective with either success or failure.  All of the following things
** cause the objective to complete: "Entered", "Killed", or "Poked"
**
** param
** Objective_ID - id of the objective
** Success - 0 or 1, success or failure
** Player_Type - player type allowed to trigger this.  0=nod, 1=gdi, 2=any
*/
DECLARE_SCRIPT(GTH_Objective_Complete_Enter_Kill_Poke, "Objective_ID=0:int, Success=1:int, Player_Type=2:int" )
{
	bool	already_triggered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(already_triggered, 1);
	}
	
	void	Created( GameObject * /*obj*/ ) override							{ already_triggered = false; }
	void	Poked( GameObject * /*obj*/, GameObject * poker ) override		{ trigger(poker); }
	void	Killed( GameObject * /*obj*/, GameObject * killer ) override		{ trigger(killer); }
	void	Entered( GameObject * /*obj*/, GameObject * enterer ) override	{ trigger(enterer); }

	
	void trigger(GameObject * obj)
	{
		// check if we've already triggered
		if (already_triggered) {
			return;
		}

		// check the player type
		int our_player_type = Get_Int_Parameter("Player_Type");
		if (our_player_type != 2) {
			int other_player_type = Commands->Get_Player_Type(obj);
			if (other_player_type != our_player_type) {
				return;
			}
		}
		
		// if we get here, complete the mission
		int success = Get_Int_Parameter("Success");
		int id = Get_Int_Parameter("Objective_ID");

		if (success == 0) {
			Commands->Set_Objective_Status(id, 0);
		} else {
			Commands->Set_Objective_Status(id, 1);
		}

		already_triggered = true;
	}	
};


/*
** GTH_User_Controllable_Base_Defense (verified)
** Just like M00_Base_Defense except that if a player enters, he can control the object
**
** params:
** MinAttackDistance - min range for auto attack
** MaxAttackDistance - max range for auto attack
** AttackTimer - amount of time to continue tracking after last "enemy seen"
*/
DECLARE_SCRIPT (GTH_User_Controllable_Base_Defense, "MinAttackDistance=0:int, MaxAttackDistance=300:int, AttackTimer=10:int")
{
	int token_01_id;
	int token_02_id;
	int token_03_id;
	int player_type;
	bool occupied;


	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (token_01_id, 1);
		SAVE_VARIABLE (token_02_id, 2);
		SAVE_VARIABLE (token_03_id, 3);
		SAVE_VARIABLE (player_type, 4);
		SAVE_VARIABLE (occupied, 5);
	}

	void Created (GameObject * obj) override
	{
		occupied = false;
		
		// find out what my team preset is.
		player_type = Commands->Get_Player_Type( obj );
		Commands->Debug_Message( "***** Player Type Saved *****\n" );

		Commands->Enable_Hibernation (obj, false);
		Commands->Innate_Enable (obj);
		Commands->Enable_Enemy_Seen (obj, true);

		Vector3 my_position = Commands->Get_Position (obj);
		Vector3 token_01_pos = my_position;
		Vector3 token_02_pos = my_position;
		Vector3 token_03_pos = my_position;

		token_01_pos.X -= 10.0f;
		token_01_pos.Y -= 10.0f;
		token_01_pos.Z += 2.0f;

		token_02_pos.X += 10.0f;
		token_02_pos.Z += 2.0f;

		token_03_pos.X += 10.0f;
		token_03_pos.Y -= 10.0f;
		token_03_pos.Z += 2.0f;

		GameObject * token_01 = Commands->Create_Object ("Invisible_Object", token_01_pos);
		if (token_01)
		{
			token_01_id = Commands->Get_ID (token_01);
		}
		token_01 = Commands->Create_Object ("Invisible_Object", token_02_pos);
		if (token_01)
		{
			token_02_id = Commands->Get_ID (token_01);
		}
		token_01 = Commands->Create_Object ("Invisible_Object", token_03_pos);
		if (token_01)
		{
			token_03_id = Commands->Get_ID (token_01);
		}
		Commands->Start_Timer (obj, this, 10.0f, 1);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			int rnd_num = Get_Int_Random(0,2);

			if (occupied == false) {
	
				GTH_DEBUG_INT(rnd_num,"GTH_User_Controllable_Base_Defense aiming at target %d\n");
				switch (rnd_num)
				{
				case (0):
					{
						GameObject * token_01 = Commands->Find_Object (token_01_id);
						if (token_01)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 70, 1);
							params.Set_Attack(token_01, 0.0f, 0.0f, true);
							Commands->Action_Attack(obj, params);
						}
						break;
					}
				case (1):
					{
						GameObject * token_02 = Commands->Find_Object (token_02_id);
						if (token_02)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 70, 1);
							params.Set_Attack(token_02, 0.0f, 0.0f, true);
							Commands->Action_Attack(obj, params);
						}
						break;
					}
				default:
					{
						GameObject * token_03 = Commands->Find_Object (token_03_id);
						if (token_03)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 70, 1);
							params.Set_Attack(token_03, 0.0f, 0.0f, true);
							Commands->Action_Attack(obj, params);
						}
					}
				}
			}
			Commands->Start_Timer (obj, this, 10.0f, 1);
		}
		else if (timer_id == 2)
		{
			Commands->Action_Reset (obj, 100.0f);
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		GTH_DEBUG_INT(occupied,"GTH_User_Controllable_Base_Defense::Enemy_Seen, occupied = %d\n");
		if (occupied == false) {
			Vector3 my_loc = Commands->Get_Position (obj);
			Vector3 enemy_loc = Commands->Get_Position (enemy);
			float distance = Commands->Get_Distance (my_loc, enemy_loc);
			if (distance > Get_Int_Parameter("MinAttackDistance") )
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, 2);
				params.Set_Attack(enemy, Get_Int_Parameter("MaxAttackDistance"), 0.0f, true);
				params.AttackCheckBlocked = false;
				Commands->Action_Attack(obj, params);
				Commands->Start_Timer (obj, this, Get_Int_Parameter( "AttackTimer"), 2);
			}
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 2)
		{
			Commands->Action_Reset (obj, 100.0f);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		ActionParamsStruct params;
			
		switch (type)
		{
		case CUSTOM_EVENT_VEHICLE_ENTERED:
			occupied = true;

			params.Set_Basic(this, 100, 3);
			Commands->Action_Follow_Input( obj, params );
			
			break;

		case CUSTOM_EVENT_VEHICLE_EXITED:
			occupied = false;
			// set team back to my preset.
			Commands->Set_Player_Type( obj, player_type );
			Commands->Action_Reset (obj, 100.0f);

			break;
		}

		GTH_DEBUG_INT(occupied,"GTH_User_Controllable_Base_Defense occupied = %d\n");
			
	}
};


/*
** GTH_Credit_Trickle
** This script will give an amount money to its team at a regular interval.  You can use it to
** create silos that give money as long as they're alive.
** NOTE: this won't work on buildings, only things like turrets, characters, or vehicles so make your
** "silos" as a weaponless vehcile set up like the nod-turret for example.
** 
** Params:
** Credits - number of credits to give
** Delay - time between credit grants
*/
DECLARE_SCRIPT(GTH_Credit_Trickle, "Credits=1:int,Delay=2.0:float")
{
	enum { TRICKLE_TIMER = 667 };

	void Created (GameObject * obj) override
	{
		// start the trickle timer
		Commands->Start_Timer (obj, this, Get_Float_Parameter("Delay"), TRICKLE_TIMER);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == TRICKLE_TIMER)
		{
			Commands->Give_Money( obj, Get_Int_Parameter("Credits"), true );
			Commands->Start_Timer (obj, this, Get_Int_Parameter("Delay"), TRICKLE_TIMER);
		}
	}
};


/*
** GTH_Enable_Spawner_On_Enter
** This script will enable or disable a spawner when its zone is entered
**
** Params:
** SpawnerID - id of the spawner
** Player_Type - type of player that can trigger integer, 0 = Nod, 1 = GDI, 2 = any
** Enable - enable or disable the spawner (1=enable, 0=disable)
*/
DECLARE_SCRIPT(GTH_Enable_Spawner_On_Enter, "SpawnerID=0:int,Player_Type=2:int,Enable=1:int")
{
	void Entered( GameObject * /*obj*/, GameObject * enterer ) override
	{
		// Check player type, if it doesn't match, just return
		int our_player_type = Get_Int_Parameter("Player_Type");
		if (our_player_type != 2) {
			int enter_player_type = Commands->Get_Player_Type(enterer);
			if (enter_player_type != our_player_type) {
				return;
			}
		}

		// Now enable the spawner
		bool enable = (Get_Int_Parameter("Enable") != 0);
		Commands->Enable_Spawner( Get_Int_Parameter("SpawnerID"),enable);
	}
};






/*
** GTH_CTF_Object
** This script will make the object it is attached to behave kind of like a CTF "flag" by
** attaching to the opposing player who pokes it.  If its position gets within a 
** certain distance of the "enemy home" an internal counter is incremented.  Once the counter
** reaches a desired number, an object in the level is destroyed.  This object should be the
** only building owned by the flag's team so that they immediately lose.  
**
** You should use this script on an object that has "projectile" collision only.  Make a model
** of your "flag", give it projectile collision and make a preset for it similar to the "Marker_Flag"
** in LevelEdit (you can find "Marker_Flag" under Object->Simple).
**
** CTF Rules as implemented:
** - this script controls how many flag captures cause a win
** - enemy entering the pickup radius of the flag attaches it to him
** - enemy teammates can take the flag from each other 
** - friend poking the flag sends it back to its initial position (only if its not carried by an enemy)
** - enemy getting the flag to his home position increments an internal counter
** - when win count is reached, all of the "Win_Object_To_Kill" objects are destroyed (make sure
**   this destroys all of the enemy team's buildings
**  
** Params:
** Update_Delay - how many times per second to update (this will *always* be laggy though...)
** Enemy_Player_Type - type of player that wants to grab this flag (0=Nod,1=GDI)
** Enemy_Home_Position - when flag gets here, capture count increments!
** Pickup_Radius - how close to flag we need to pickup flag
** Home_Radius - how close to enemy home position we need to get to count
** Capture_Respawn_Timer - When flag is dropped respawn back at home position after x seconds.  
**       x < 0 means flag MUST be poked for it to be returned.
** Captures_Needed_To_Win - after this many captures, we destroy the token "building" for the win
** Flag_Stolen_Event_ID - custom event id to send when the flag is stolen
** Flag_Stolen_Object_ID - Object that receives the event
** Flag_Lost_Event_ID - custom event id to send when enemy team gets flag back to Enemy_Home_Position :-(
** Flag_Lost_Object_ID - Object that receives the event
** Flag_Saved_Event_ID - custom event id to send when flag carrier is killed
** Flag_Saved_Object_ID - Object that receives the event
** Flag_Returned_Event_ID - custom event id to send when flag has been returned to home position.
** Flag_Returned_Object_ID - Object that receives the event
** Captures_Exceeded_Event_ID - custom event id to send when flag has been captured "Captures_Needed_To_Win" times.
** Captures_Exceeded_Object_ID - Object that receives the event
** Win_Object_To_Kill0 - object that we destroy when the capture count is reached
** Win_Object_To_Kill1 - object that we destroy when the capture count is reached
** Win_Object_To_Kill2 - object that we destroy when the capture count is reached
** Win_Object_To_Kill3 - object that we destroy when the capture count is reached
** Win_Object_To_Kill4 - object that we destroy when the capture count is reached
** 
*/
DECLARE_SCRIPT(GTH_CTF_Object2, "Update_Delay=0.05:float,Enemy_Player_Type=0:int,Enemy_Home_Position:vector3,"
                                "Pickup_Radius=5.0:float,Home_Radius=5.0:float,Capture_Respawn_Timer=30.0:float,"
                                "Captures_Needed_To_Win=5:int,"
										  "Flag_Stolen_Event_ID=25000:int,Flag_Stolen_Object_ID=0:int,Flag_Lost_Event_ID=25001:int,"
										  "Flag_Lost_Object_ID=0:int,Flag_Saved_Event_ID=25002:int,Flag_Saved_Object_ID=0:int,"
										  "Flag_Returned_Event_ID=25003:int,Flag_Returned_Object_ID=0:int,"
										  "Captures_Exceeded_Event_ID=25004:int,Captures_Exceeded_Object_ID=0:int,"
                                "Win_Object_To_Kill0=0:int,Win_Object_To_Kill1=0:int,Win_Object_To_Kill2=0:int,"
                                "Win_Object_To_Kill3=0:int,Win_Object_To_Kill4=0:int")
{
	enum { CTF_UPDATE_TIMER = 0x10211131 };

	int capture_count;
	int captured_by_id;
	Vector3 home_position;
	float capture_timer;


	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (capture_count, 1);
		SAVE_VARIABLE (captured_by_id, 2);
		SAVE_VARIABLE (home_position, 3);
		SAVE_VARIABLE (capture_timer, 4);
	}

	void Created (GameObject * obj) override
	{
		// Initialize everything
		capture_count = 0;
		captured_by_id = 0;
		capture_timer = 0.0;
		home_position = Commands->Get_Position(obj);
		
		GTH_DEBUG_FLOAT(home_position.X,"Flag x: %f");
		GTH_DEBUG_FLOAT(home_position.Y,"Flag y: %f");
		GTH_DEBUG_FLOAT(home_position.Z,"Flag z: %f\n");

		// Start the update timer!
		Commands->Start_Timer (obj, this, Get_Float_Parameter("Update_Delay"), CTF_UPDATE_TIMER);
	}

	void Damaged( GameObject * obj, GameObject * damager, float /*amount*/ ) override
	{
		Grab_Flag(obj,damager);
	}

//	void Poked( GameObject * obj, GameObject * poker )	
//	{
//		Grab_Flag(obj,poker);
//	}

	void Grab_Flag( GameObject * obj, GameObject * grabber )
	{ 
		// If we're already captured, bail
		if (captured_by_id != 0) {
			return;
		}
		
		// Check distance, if the damager is too far away, bail
		Vector3 my_pos = Commands->Get_Position(obj);
		Vector3 delta = my_pos - Commands->Get_Position(grabber);
		if (delta.Length() > Get_Float_Parameter("Pickup_Radius")) {
			GTH_DEBUG_FLOAT(delta.Length(),"Flag too far away, dist = %f\n");
			return;
		}
		
		// Check player type, if it doesn't match, warp back to our home position
		int capture_player_type = Get_Int_Parameter("Enemy_Player_Type");
		int poke_player_type = Commands->Get_Player_Type(grabber);

		// Someone poked the flag...see if it needs to respawn back at home.
		if (capture_player_type != 2) {
			if (poke_player_type != capture_player_type) {
				// if we're not being carried and the flag isn't on a timer, warp back
				if (captured_by_id == 0 && Get_Float_Parameter("Capture_Respawn_Timer") < 0) {
					Commands->Set_Position(obj,home_position);
			
					GTH_DEBUG_INT(poke_player_type,"Flag recovered back by team: %d\n");
					// Play flag returned sound
					int cust_id = Get_Int_Parameter("Flag_Returned_Event_ID");
					GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Flag_Returned_Object_ID"));
					if (event_obj == NULL) event_obj = obj;
					Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);
				}
				return;
			}
		}

		// The player type matches, Attach to this dude! 
		captured_by_id = Commands->Get_ID(grabber);

		// attach to bone 0 (root) 
		Commands->Attach_To_Object_Bone(obj,grabber,"ROOTTRANSFORM");
		GTH_DEBUG_INT(poke_player_type,"Flag captured by team: %d\n");

		// Play picked up sound
		int cust_id = Get_Int_Parameter("Flag_Stolen_Event_ID");
		GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Flag_Stolen_Object_ID"));
		if (event_obj == NULL) event_obj = obj;
		Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == CTF_UPDATE_TIMER)
		{
			// Search for a player around the flag and let them auto-grab it.
			// BMH 12/7/02 - Get_A_Star only returns the closest to the position.  So if someone just sat
			// on top of the flag that's the only star we'd ever check.  So check from a random position
			// within the pickup radius
			Vector3 my_pos = Commands->Get_Position(obj);
			Vector3 find_pos = my_pos;
			float pick_rad = Get_Float_Parameter("Pickup_Radius");
			float rval = Commands->Get_Random(-pick_rad, pick_rad);
			find_pos.X += rval;
			rval = Commands->Get_Random(-pick_rad, pick_rad);
			find_pos.Y += rval;
			rval = Commands->Get_Random(-pick_rad, pick_rad);
			find_pos.Z += rval;
			GameObject *star_obj = Commands->Get_A_Star(find_pos);
			Grab_Flag(obj,star_obj);

			// check if we've been captured enough to end the game
			if (capture_count >= Get_Int_Parameter("Captures_Needed_To_Win")) {
				
				// Play Win Sound!
				int cust_id = Get_Int_Parameter("Captures_Exceeded_Event_ID");
				GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Captures_Exceeded_Object_ID"));
				if (event_obj == NULL) event_obj = obj;
				Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);

				// win!  
				GameObject * win_obj = Commands->Find_Object(Get_Int_Parameter("Win_Object_To_Kill0"));
				if (win_obj != NULL) {
					Commands->Destroy_Object( win_obj );
				}
				win_obj = Commands->Find_Object(Get_Int_Parameter("Win_Object_To_Kill1"));
				if (win_obj != NULL) {
					Commands->Destroy_Object( win_obj );
				}
				win_obj = Commands->Find_Object(Get_Int_Parameter("Win_Object_To_Kill2"));
				if (win_obj != NULL) {
					Commands->Destroy_Object( win_obj );
				}
				win_obj = Commands->Find_Object(Get_Int_Parameter("Win_Object_To_Kill3"));
				if (win_obj != NULL) {
					Commands->Destroy_Object( win_obj );
				}
				win_obj = Commands->Find_Object(Get_Int_Parameter("Win_Object_To_Kill4"));
				if (win_obj != NULL) {
					Commands->Destroy_Object( win_obj );
				}
		
				GTH_DEBUG_INT(0,"Capture count exceeded\n");
				
				return;
			}

			// check if we're in our enemies home position
			Vector3 delta = my_pos - Get_Vector3_Parameter("Enemy_Home_Position");

			float dist = delta.Length();
			if (dist < Get_Float_Parameter("Home_Radius")) {
				
				// Increment the capture count, detatch from our carrier and warp home
				capture_count++;
				captured_by_id = 0;
				Commands->Attach_To_Object_Bone(obj,NULL,NULL);
				Commands->Set_Position(obj,home_position);
				GTH_DEBUG_INT(capture_count,"Flag stolen, counter now %d, returning to home!\n");

				// Play flag capture sound
				int cust_id = Get_Int_Parameter("Flag_Lost_Event_ID");
				GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Flag_Lost_Object_ID"));
				if (event_obj == NULL) event_obj = obj;
				Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);
			}
			
			// check if we have a capturer and need to update
			if (captured_by_id != 0) {
				GameObject * capturer = Commands->Find_Object(captured_by_id);
				
				if (capturer != NULL) {
				
					// Don't do this cause we're using the attach to bone method!
					//Vector3 cap_position = Commands->Get_Position(capturer);
					//Commands->Set_Position(obj,cap_position);
				
				} else {
				
					captured_by_id = 0;
					// start the flag respawn timer.
					capture_timer = (((float)Commands->Get_Sync_Time()) / 1000.0);
					Commands->Attach_To_Object_Bone(obj,NULL,NULL);
					GTH_DEBUG_INT(0,"Capturer killed!\n");

					// Play flag saved sound
					int cust_id = Get_Int_Parameter("Flag_Saved_Event_ID");
					GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Flag_Saved_Object_ID"));
					if (event_obj == NULL) event_obj = obj;
					Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);
				}
			}

			// Check if the flag has been sitting idle for too long and return it if need be.
			float ctimer = Get_Float_Parameter("Capture_Respawn_Timer");
			if (captured_by_id == 0 && ctimer > 0) {

				// Make sure the flag isn't already at the base
				delta = my_pos - home_position;
				dist = delta.Length();
				if (dist > Get_Float_Parameter("Pickup_Radius")) {

					// has the Respawn_Timer expired
					if (((((float)Commands->Get_Sync_Time()) / 1000.0) - capture_timer) > ctimer) {

						// Return the flag
						Commands->Set_Position(obj,home_position);
						GTH_DEBUG_INT(0,"Flag timeout has been hit, respawning flag back at home.\n");

						// Play flag returned sound
						int cust_id = Get_Int_Parameter("Flag_Returned_Event_ID");
						GameObject * event_obj = Commands->Find_Object(Get_Int_Parameter("Flag_Returned_Object_ID"));
						if (event_obj == NULL) event_obj = obj;
						Commands->Send_Custom_Event(obj,event_obj,cust_id, 1, 0.0f);
					}
				}
			}

			// register for another update!
			Commands->Start_Timer (obj, this, Get_Float_Parameter("Update_Delay"), CTF_UPDATE_TIMER);
		}
	}
};

