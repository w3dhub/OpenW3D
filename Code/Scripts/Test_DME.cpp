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
*
* DESCRIPTION
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 39 $
*     $Modtime: 1/07/02 2:51p $
*     $Archive: /Commando/Code/Scripts/Test_DME.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"

#define DME_OCCUPIED	13000

DECLARE_SCRIPT (DME_Test_Powerup, "")
{	

	int reward_type;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(reward_type, 1);
	}


	void Killed( GameObject * obj, GameObject * /*killer*/ ) override
	{
		
		Commands->Trigger_Spawner ( 100022 );
		reward_type = Get_Int_Random(0, 7);
		Vector3 enemy_loc = Commands->Get_Position ( obj );
		enemy_loc.Z += 1.0f;
		
		switch (reward_type)
		{
		case 0:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Armor 025 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 1:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Armor 050 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 2:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Armor 100 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 3:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Health 025 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 4:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Health 050 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 5:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Health 100 PowerUp", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 6:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "Shotgun Weapon PowerUps", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		case 7:
			{
				GameObject *spawn_objectarmor;
				spawn_objectarmor = Commands->Create_Object( "Spawner Created Special Effect", enemy_loc );

				if (spawn_objectarmor)
				{
					Commands->Attach_Script( spawn_objectarmor, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				}

				GameObject *armor;
				armor = Commands->Create_Object ( "MiniGun Weapon PowerUps", enemy_loc );
				Commands->Attach_Script (armor, "DME_Destroy_Item", "15.0f");
				break;
			}

		default:
			{
				Commands->Debug_Message ("********NO POWER-UP CREATED********");
				break;
			}
		}


	}	
	
};

DECLARE_SCRIPT (DME_Destroy_Item, "timer_length: float")
{
	void Created( GameObject * obj ) override
	{
		float timer_length = Get_Float_Parameter("timer_length");
		Commands->Start_Timer(obj, this, timer_length, 10);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id ==10)
		{
			Commands->Destroy_Object (obj);
		}

	}
};

DECLARE_SCRIPT (DME_Waypath_test, "")
{
	void Created ( GameObject * obj ) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, 100, 10 );
		params.Set_Movement( Vector3(0,0,0), 1.5f, 0 );		
		params.WaypathID = 100036;
		params.WaypointStartID = 100037;
		params.WaypointEndID = 100039;		
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );
	}

};

DECLARE_SCRIPT (DME_Test_Ejected_Soldier, "")
{
	void Killed ( GameObject * obj, GameObject * /*killer*/ ) override
	{
		Vector3 spawn_loc = Commands->Get_Position ( obj );
		spawn_loc.Z -= 30.0f;

		GameObject *spawn_objectsoldier;
		spawn_objectsoldier = Commands->Create_Object( "Spawner Created Special Effect", spawn_loc );

		GameObject *soldier;
		soldier = Commands->Create_Object ( "Nod_Technician_0", spawn_loc );
		Commands->Attach_Script (soldier, "DME_Test_Paradrop", "");
				
	}
};


DECLARE_SCRIPT (DME_Test_Paradrop, "")
{
	void Created ( GameObject * obj ) override
	{		
		/*Vector3 loc = Commands->Get_Position ( obj );
		GameObject *para1;
		para1 = Commands->Create_Object_At_Bone (obj, "Generic_Cinematic", "C HEAD");
		Commands->Attach_To_Object_Bone( obj, para1, "C HEAD" );
		Commands->Set_Model(para1, "X5D_Parachute");
		Commands->Set_Animation(para1, "X5D_Parachute.X5D_ParaC_1", false);
		Commands->Create_3D_Sound_At_Bone("parachute_open", para1, "ROOTTRANSFORM");*/

		Commands->Start_Timer (obj, this, .8385f, 10);
		Vector3 loc = Commands->Get_Position ( obj );
		float facing = Commands->Get_Facing(obj);
		
		GameObject *box1 = Commands->Create_Object("Generic_Cinematic", loc);
		Commands->Set_Model(box1, "X5D_Box01");
		Commands->Set_Facing(box1, facing);
		Commands->Set_Animation(box1, "X5D_Box01.X5D_Box01", false, NULL, 0.0f, -1.0f, false);

		GameObject *soldier1;
		soldier1 = Commands->Create_Object_At_Bone(box1, "Nod_Technician_0", "Box01");
		Commands->Set_Facing(soldier1, facing);
		Commands->Attach_Script(soldier1, "RMV_Trigger_Killed", "1144444, 1000, 1000");
		Commands->Attach_To_Object_Bone( soldier1, box1, "Box01" );
		Commands->Set_Animation(soldier1, "s_a_human.H_A_X5D_ParaT_1", false, NULL, 0.0f, -1.0f, false);
		
		

	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == 10)
		{
			GameObject *para1;
			Vector3 loc = Commands->Get_Position ( obj );
			loc.Z += 1.5;
			float facing = Commands->Get_Facing(obj);
			para1 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Facing(para1, facing);
			Commands->Set_Model(para1, "X5D_Parachute");
			Commands->Set_Animation(para1, "X5D_Parachute.X5D_ParaC_1", false, NULL, 0.0f, -1.0f, false);
			Commands->Create_3D_Sound_At_Bone("parachute_open", para1, "ROOTTRANSFORM");
			Commands->Attach_Script(para1, "M03_No_More_Parachute", "");
		}
	
	}

};


DECLARE_SCRIPT (DME_Test_Worker_Wander, "Work_Area=3:int")
{
	int loc_id [20], count, choice, last_loc, poke_choice;
	int area_range, area_start, area_end;
	char area_anim [9];
	bool work_done[20], star_seen, poked, reward_given, anim_playing;
	float facing;
	Vector3 location;

	GameObject *destination_object;

	
	
	void init ()	//initializes count and work_done values.
	{
		int x;

		count = 0;
		for (x = area_start; x <= area_end; x++)
		{
			work_done [x] = 1;
		}
		
	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* sender) override	//recieves custom from dave's arrow as to if the location is occupied or not.
	{																				//100 not occupied, 200 is occupied.
		if (type == DME_OCCUPIED && param == 100)
		{
			Commands->Send_Custom_Event(obj, sender, 70, 70, 0.0f);				//if not occupied, sends custom to set status to occupied.
		}

		else if (type == 200 && param == 200)
		{
			Commands->Debug_Message("Custom received on worker type: %d param: %d sender_id %d. \n", type, param, Commands->Get_ID(sender));
			Worker_Loop (obj);												//if occupied, tells worker to choose a now location.
		}
	}


	void Worker_Loop ( GameObject * obj )		//this is the main function that checks if the location is occupied
	{											//and if it is not, tells the servant to go to that location.
		if (count == area_range)
		{
			init ();
		}

		
		GameObject *work_loc;
				
		choice = Get_Int_Random (area_start, area_end);
		while (work_done [choice] == 0 || choice == last_loc) 
		{
			choice = Get_Int_Random (area_start, area_end);
		} 
		
		work_done [choice] = 0;
		last_loc = choice;
		count++;

		destination_object = Commands->Find_Object (loc_id [choice]);
		Commands->Send_Custom_Event(obj, destination_object, 50, 50, 0.5f);

		work_loc = Commands->Find_Object (loc_id [choice]);
		location = Commands->Get_Position ( work_loc );
		facing = Commands->Get_Facing (work_loc);
				
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 1, 90 );
		params.Set_Movement( location, WALK, 0.0f );		
		Commands->Action_Goto( obj, params );
		
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/ ) override	//this is called when the servant reaches the destination location.
	{																										//the purpose of this function is to have the servant perform their destination animation.
		
		Vector3 current_loc = Commands->Get_Position ( obj );
		float distance = Commands->Get_Distance ( current_loc, location );
		
		if (action_id == 90)
		{
			if (star_seen == false && distance <= 3.0f)
			{
				anim_playing = true;
				
				Commands->Start_Timer (obj, this, 2.0f, 500);
				Commands->Set_Facing(obj, facing);
				
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1);
				params.Set_Animation( area_anim, true );
				Commands->Action_Play_Animation (obj, params);
			}
		}
			
	}

	void Created ( GameObject * obj ) override		//this is the created function for the servant, it sets up most of the variables.
	{
		int work_area = Get_Int_Parameter("Work_Area");
		Commands->Set_Player_Type( obj, 0 );
		Commands->Innate_Disable( obj );
		poke_choice = 3;	//Get_Int_Random (1, 3);
		star_seen = false;
		reward_given = false;
		poked = false;
		anim_playing = false;



		loc_id [0] = 100001;				//these are the dave's arrow id's for all 3 work areas.
		loc_id [1] = 100002;
		loc_id [2] = 100003;
		loc_id [3] = 2000217;
		loc_id [4] = 2000218;
		loc_id [5] = 2000219;
		loc_id [6] = 2000220;
		loc_id [7] = 2000221;
		loc_id [8] = 2000222;
		loc_id [9] = 2000223;
		loc_id [10] = 2000224;
		loc_id [11] = 2000225;
		loc_id [12] = 2000226;
		loc_id [13] = 2000227;
		loc_id [14] = 2000228;
		loc_id [15] = 2000229;
		loc_id [16] = 2000230;
		loc_id [17] = 2000231;
		loc_id [18] = 2000232;
		loc_id [19] = 2000233;

		//Area 1 Variables.
		if (work_area == 1)
		{
			area_range = 3;
			area_start = 0;
			area_end = 2;
			sprintf(area_anim, "%s", "H_A_a0f0");
		}
		
		//Area 2 Variables.
		else if (work_area == 2)
		{
			area_range = 4;
			area_start = 12;
			area_end = 15;
			sprintf(area_anim, "%s", "H_A_a0f0");
		}
		
		//Area 3 Variables.
		else if (work_area == 3)
		{
			area_range = 4;
			area_start = 16;
			area_end = 19;
			sprintf(area_anim, "%s", "H_A_a0f0");
		}

		init ();

		Worker_Loop (obj);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == 500)			//this timer resets the servant from his animation loop and calls Worker_Loop to send him to a new location.
		{
			anim_playing = false;
			
			Commands->Action_Reset(obj, 99);
			Commands->Set_Animation( obj, NULL, false , NULL, 0.0f, -1.0f, false);
			
			destination_object = Commands->Find_Object (loc_id [choice]);
			Commands->Send_Custom_Event(obj, destination_object, 90, 90, 0.0f);

			Worker_Loop (obj);
						
		}

		
		else if (timer_id == 510)		//this timer resets the servant after seeing the star.  He completes his animation and returns to work.
		{
			star_seen = false;

			Commands->Action_Reset(obj, 99);
			Commands->Set_Animation( obj, "H_A_HOST_L1B", false, NULL, 0.0f, -1.0f, false );
			Commands->Set_Animation_Frame(obj, "H_A_HOST_L1B", 16);
			
			Worker_Loop (obj);
			
		}

		
		else if (timer_id == 520)			//this timer is a general reset after being poked.
		{
			poked = false;
			Commands->Set_Player_Type( obj, 0 );

			Worker_Loop ( obj );
		}

		else if (timer_id == 530)			//this timer resets the servant after giving the star assistance.
		{
			poked = false;
			Worker_Loop ( obj );
		}
		
	}

	void Enemy_Seen (GameObject* obj, GameObject* /*enemy*/) override	//this function sets the servant in a "don't hurt me!" stance (based on a timer)
	{
		if (star_seen == false)
		{
			if (anim_playing == false)
			{
				destination_object = Commands->Find_Object (loc_id [choice]);
				Commands->Send_Custom_Event(obj, destination_object, 90, 90, 0.0f);

				star_seen = true;
				Commands->Start_Timer (obj, this, 9.0f, 510);
				
				const char *conv_name = ("IDS_M06_DME");
				int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
				Commands->Join_Conversation(obj, conv_id, false, false, true);
				Commands->Start_Conversation (conv_id, 1);
				
				
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.Set_Animation( "H_A_HOST_L1B", true );
				Commands->Action_Play_Animation (obj, params);
			}

		}
		
	}

	void Poked(GameObject * obj, GameObject * /*poker*/) override  //function runs from a pre-defined random poke_choice.
	{
		if (poked == !true)
		{
			Commands->Set_Player_Type( obj, -2 );

			destination_object = Commands->Find_Object (loc_id [choice]);
			Commands->Send_Custom_Event(obj, destination_object, 90, 90, 0.0f);

			poked = true;
			
			switch (poke_choice)
			{
			case 1:		//call for guards
				{
					Commands->Start_Timer (obj, this, 5.0f, 520);
					
					const char *conv_name = ("IDS_M06_DME02");
					int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, false, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Create_Logical_Sound(obj, SOUND_TYPE_GUNSHOT, Commands->Get_Position(obj), 75.0f);

					

				}
				break;

			case 2:		//sound alarm
				{
					Commands->Start_Timer (obj, this, 5.0f, 520);
					
					const char *conv_name = ("IDS_M06_D05");
					int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, false, true);
					Commands->Start_Conversation (conv_id, 1);

				
					
				}
				break;

			case 3:		//give assistance
				{
					if (reward_given == false)
					{
						Commands->Action_Reset(obj, 99);
						Commands->Set_Animation( obj, "H_A_HOST_L1B", false , NULL, 0.0f, -1.0f, false);
						Commands->Set_Animation_Frame(obj, "H_A_HOST_L1B", 16);
						
						reward_given = true;									//records that the reward has been given.
						int reward_type = Get_Int_Random(0, 5);
						Commands->Start_Timer (obj, this, 5.0f, 530);

						const char *conv_name = ("IDS_M06_DME3");				//starts conversation.
						int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
						Commands->Join_Conversation(obj, conv_id, false, false, true);
						Commands->Start_Conversation (conv_id, 1);

						
						Vector3 pos = Commands->Get_Position(obj);				//specifies drop location and plays droping anim.
						float obj_facing = Commands->Get_Facing(obj);
						Commands->Set_Animation( obj, "H_A_J12C", false, NULL, 0.0f, -1.0f, false );
						float a = cos(DEG_TO_RADF(obj_facing)) * 1.5;
						float b = sin(DEG_TO_RADF(obj_facing)) * 1.5;
						Vector3 powerup_loc = pos + Vector3(a, b, 0.5f);
						
						switch (reward_type)									//random reward type.
						{
						case 0:					
							{
								Commands->Create_Object("Armor 025 PowerUp", powerup_loc);
								
							}
							break;
						
						case 1:					
							{
								Commands->Create_Object("Armor 050 PowerUp", powerup_loc);
							}
							break;

						case 2:					
							{
								Commands->Create_Object("Armor 100 PowerUp", powerup_loc);
							}
							break;

						case 3:					
							{
								Commands->Create_Object("Health 025 PowerUp", powerup_loc);
							}
							break;

						case 4:					
							{
								Commands->Create_Object("Health 025 PowerUp", powerup_loc);
							}
							break;

						case 5:					
							{
								Commands->Create_Object("Health 025 PowerUp", powerup_loc);
							}
							break;
						}
					}

					else
					{
						Commands->Start_Timer (obj, this, 3.0f, 530);
					}
									
				}
				break;

			default:
				{
					Commands->Debug_Message ("********poke_choice out of range");
					break;
				}
				
			}
		}
	}
	
};

DECLARE_SCRIPT (DME_Test_Work_Area, "")			//this script needs to be placed on all daves arrows.
{												//the script takes and returns customs to tell servants if a location is occupied.
	bool occupied;								//type & param info:
												//50 - is destination occupied?
	void Created( GameObject * /*obj*/ ) override	//  100 - no
	{											//  200 - yes
		occupied = false;						//70 - change occupied status to true
	}											//90 - change occupied status to false
	
	void Custom (GameObject* obj, int type, intptr_t param, GameObject* sender) override
	{
		if (type == 50 && param == 50)
		{
			if (!occupied)
			{
				Commands->Debug_Message("Not occupied\n");
				Commands->Send_Custom_Event(obj, sender, DME_OCCUPIED, 100, 0.0f);
			}

			else
			{
				Commands->Debug_Message("Occupied\n");
				Commands->Send_Custom_Event(obj, sender, 200, 200, 0.0f);
			}
		}

		else if (type == 70 && param == 70)
		{
			occupied = true;
		}

		else if (type == 90 && param == 90)
		{
			occupied = false;
		}
	}
};




DECLARE_SCRIPT (M05_Tank_Drop_01_DME, "")	
{
	bool entered;

	void Created ( GameObject * /*obj*/) override
	{
		entered = false;
	}
	
	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * sender) override
	{
		if (entered == false)
		{
			if (type == CUSTOM_EVENT_VEHICLE_ENTERED && Commands->Is_A_Star(sender))
			{
				entered = true;
				
				Vector3 drop_loc;
				drop_loc.X = -476.07f;
				drop_loc.Y = -130.76f;
				drop_loc.Z = 13.020f;
				float facing = 337.603f;

				GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", drop_loc);
				Commands->Set_Facing(chinook_obj, facing);
				Commands->Attach_Script(chinook_obj, "Test_Cinematic", "M05_XG_VehicleDrop1.txt");
			}
		}
	}
};

DECLARE_SCRIPT (M05_Tank_Attack_DME, "")	
{
	bool attacking, first;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		first = true;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		Vector3 current_loc;
		Vector3 enemy_loc;
		current_loc = Commands->Get_Position ( obj );
		enemy_loc = Commands->Get_Position ( enemy );
		
		if ((Commands->Get_Distance(current_loc, enemy_loc)) < 65)
		{
			if(!attacking && first)
			{
				attacking = true;
				first = false;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
				params.Set_Attack (enemy, 65.0f, 3.0f, 1);
				params.AttackCheckBlocked = true;
				Commands->Action_Attack( obj, params );
				
				Commands->Start_Timer(obj, this, 5.0f, ATTACK_OVER);
			}
		}

		else if (!attacking && !first)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 3.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 5.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired (GameObject* /*obj*/, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;
		}
	}
	
};


DECLARE_SCRIPT (M05_Tech_Wander_DME, "Work_Area=1:int")
{
	int loc_id [7], count, choice, last_loc, poke_choice;
	int area_range, area_start, area_end;
	char area_anim [9];
	bool work_done[20], star_seen, poked, reward_given, anim_playing;
	float facing;
	Vector3 location;

	GameObject *destination_object;

	
	
	void init ()	//initializes count and work_done values.
	{
		int x;

		count = 0;
		for (x = area_start; x <= area_end; x++)
		{
			work_done [x] = 1;
		}
		
	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* sender) override	//recieves custom from dave's arrow as to if the location is occupied or not.
	{																				//100 not occupied, 200 is occupied.
		if (type == DME_OCCUPIED && param == 100)
		{
			Commands->Send_Custom_Event(obj, sender, 70, 70, 0.0f);				//if not occupied, sends custom to set status to occupied.
		}

		else if (type == 200 && param == 200)
		{
			Commands->Debug_Message("Custom received on worker type: %d param: %d sender_id %d. \n", type, param, Commands->Get_ID(sender));
			Worker_Loop (obj);												//if occupied, tells worker to choose a now location.
		}
	}


	void Worker_Loop ( GameObject * obj )		//this is the main function that checks if the location is occupied
	{											//and if it is not, tells the servant to go to that location.
		if (count == area_range)
		{
			init ();
		}

		
		GameObject *work_loc;
				
		choice = Get_Int_Random (area_start, area_end);
		while (work_done [choice] == 0 || choice == last_loc) 
		{
			choice = Get_Int_Random (area_start, area_end);
		} 
		
		work_done [choice] = 0;
		last_loc = choice;
		count++;

		destination_object = Commands->Find_Object (loc_id [choice]);
		Commands->Send_Custom_Event(obj, destination_object, 50, 50, 0.5f);

		work_loc = Commands->Find_Object (loc_id [choice]);
		location = Commands->Get_Position ( work_loc );
		facing = Commands->Get_Facing (work_loc);
				
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 1, 90 );
		params.Set_Movement( location, WALK, 0.0f );		
		Commands->Action_Goto( obj, params );
		
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/ ) override	//this is called when the servant reaches the destination location.
	{																										//the purpose of this function is to have the servant perform their destination animation.
		
		Vector3 current_loc = Commands->Get_Position ( obj );
		float distance = Commands->Get_Distance ( current_loc, location );
		
		if (action_id == 90)
		{
			if (star_seen == false && distance <= 3.0f)
			{
				anim_playing = true;
				
				Commands->Start_Timer (obj, this, 2.0f, 500);
				Commands->Set_Facing(obj, facing);
				
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1);
				params.Set_Animation( area_anim, true );
				Commands->Action_Play_Animation (obj, params);
			}
		}
			
	}

	void Created ( GameObject * obj ) override		//this is the created function for the servant, it sets up most of the variables.
	{
		int work_area = Get_Int_Parameter("Work_Area");
		Commands->Set_Player_Type( obj, 0 );
		Commands->Innate_Disable( obj );
		poke_choice = 3;	//Get_Int_Random (1, 3);
		star_seen = false;
		reward_given = false;
		anim_playing = false;



		loc_id [0] = 2000025;				//these are the dave's arrow id's for all 3 work areas.
		loc_id [1] = 2000026;
		loc_id [2] = 2000027;
		loc_id [3] = 2000028;
		loc_id [4] = 2000031;
		loc_id [5] = 2000032;
		loc_id [6] = 2000030;

		//Area 1 Variables.
		if (work_area == 1)
		{
			area_range = 7;
			area_start = 0;
			area_end = 6;
			sprintf(area_anim, "%s", "H_A_a0f0");
		}
		
		init ();

		Worker_Loop (obj);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == 500)			//this timer resets the servant from his animation loop and calls Worker_Loop to send him to a new location.
		{
			anim_playing = false;
			
			Commands->Action_Reset(obj, 99);
			Commands->Set_Animation( obj, NULL, false, NULL, 0.0f, -1.0f, false );
			
			destination_object = Commands->Find_Object (loc_id [choice]);
			Commands->Send_Custom_Event(obj, destination_object, 90, 90, 0.0f);

			Worker_Loop (obj);
						
		}

		
		else if (timer_id == 510)		//this timer resets the servant after seeing the star.  He completes his animation and returns to work.
		{
			star_seen = false;

			Commands->Action_Reset(obj, 99);
			Commands->Set_Animation( obj, "H_A_HOST_L1B", false , NULL, 0.0f, -1.0f, false);
			Commands->Set_Animation_Frame(obj, "H_A_HOST_L1B", 16);
			
			Worker_Loop (obj);
			
		}		
	}

	void Enemy_Seen (GameObject* obj, GameObject* /*enemy*/) override	//this function sets the servant in a "don't hurt me!" stance (based on a timer)
	{
		if (star_seen == false)
		{
			if (anim_playing == false)
			{
				destination_object = Commands->Find_Object (loc_id [choice]);
				Commands->Send_Custom_Event(obj, destination_object, 90, 90, 0.0f);

				star_seen = true;
				Commands->Start_Timer (obj, this, 9.0f, 510);
				
				const char *conv_name = ("IDS_M06_DME");
				int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
				Commands->Join_Conversation(obj, conv_id, false, false, true);
				Commands->Start_Conversation (conv_id, 1);
				
				
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1);
				params.Set_Animation( "H_A_HOST_L1B", true );
				Commands->Action_Play_Animation (obj, params);
			}
		}		
	}	
};

DECLARE_SCRIPT (test_Ssm_Trigger, "")
{
	void Damaged( GameObject * obj, GameObject * damager, float /*amount*/ ) override
	{
		if (damager == STAR)
		{
			Commands->Send_Custom_Event (obj, obj, M00_LAUNCH_SSM, 0, 0.0f);
		}
	}
};

DECLARE_SCRIPT(DME_Cinematic_Test, "")
{
	void Created (GameObject * /*obj*/) override
	{
	/*	Commands->Enable_Hibernation(obj, false);
		Commands->Set_Player_Type(obj, PLAYERTYPE_NEUTRAL );
		Commands->Create_Object ( "XG_TransprtBone", Vector3(0.0f, 0.0f, 0.0f));*/
	}


	void Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override
	{
	/*	GameObject * chinook_obj0 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj0, 0.0f);
		Commands->Attach_Script(chinook_obj0, "Test_Cinematic", "X7A_TrnsptCt_00.txt");

		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj1, 0.0f);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7A_TrnsptCt_01.txt");*/

		GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
		Commands->Set_Facing(chinook_obj2, 0.0f);
		Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X10D_CHTroopdrop1.txt");

	//	GameObject * napc = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
	//	Commands->Set_Model(napc, "e_chopperdust1");

	/*	Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), M07_EVAC_INN, 1, 0.0f);

		Commands->Enable_Spawner(100002, true);
		Commands->Enable_Spawner(100004, true);
		Commands->Enable_Spawner(100005, true);
		Commands->Enable_Spawner(100006, true);
		Commands->Enable_Spawner(100007, true);*/
	}
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                             MISSION X0                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////



