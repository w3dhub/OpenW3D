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
*     MissionDemo.cpp
*
* DESCRIPTION
*     Mission Demo Specific Scripts
*
* PROGRAMMER
*     Rich Donnelly
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 7 $
*     $Modtime: 11/29/01 11:05a $
*     $Archive: /Commando/Code/Scripts/MissionDemo.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"

// Predefined Constants

#define M02_OBJCONTROLLER 1111112
#define M02_AREACOUNT     26

// Mission Objective Controller and Initializer

DECLARE_SCRIPT(MDD_Objective_Controller, "")
{
	int convoy_trucks;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( convoy_trucks, 1 );
	}

	void Created(GameObject * obj) override
	{
		//DEMO
		Commands->Enable_HUD (false);
		GameObject * camera = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		if (camera)
		{
			Commands->Attach_Script(camera, "Test_Cinematic", "XG_DemoCam.txt");
		}
		int id = Commands->Create_Conversation("Demo_Intro", 0, 0, true);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(STAR, id, true, true, true);
		Commands->Start_Conversation(id, 0);

		//DEMO
		// Set up the expected number of convoy trucks for the objective.

		convoy_trucks = 3;

		// Prevent the Objective Controller from hibernating

		Commands->Enable_Hibernation(obj, false);

		// Turn on the first primary objective (Locate the Missing Scientists)
		// Turn on the tertiary objectives (if any)

		//DEMO Commands->Add_Objective(201, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_M02_OBJECTIVE_01);
		//DEMO Commands->Add_Objective(222, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_M02_OBJECTIVE_22);
		//DEMO Commands->Add_Objective(223, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_M02_OBJECTIVE_23);
	}

	// This custom receives requests to activate and deactivate objectives
	// Type = Objective ID
	// Param 0 = Activate this objective (Pending)
	// Param 1 = Complete this objective (Accomplished)
	// Param 2 = Fail this objective (Failed)
	// Param 3 = Convoy truck special - counting
};


DECLARE_SCRIPT (MDD_Respawn_Controller, "")
{
	bool active_area [M02_AREACOUNT];
	bool area_spawn_heli_drop_01_active [M02_AREACOUNT];
	bool area_spawn_heli_drop_02_active [M02_AREACOUNT];
	bool area_spawn_parachute_active [M02_AREACOUNT];

	char area_unit_count [M02_AREACOUNT];
	char area_unit_max [M02_AREACOUNT];
	int area_officer [M02_AREACOUNT];

	int spawner_rotator;
	int replacement_vehicle;
	int destroyed_sam;
	int destroyed_silo;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( active_area, 1);
		SAVE_VARIABLE( area_spawn_heli_drop_01_active, 2);
		SAVE_VARIABLE( area_spawn_heli_drop_02_active, 3);
		SAVE_VARIABLE( area_spawn_parachute_active, 4);
		SAVE_VARIABLE( area_unit_count, 5);
		SAVE_VARIABLE( area_unit_max, 6);
		SAVE_VARIABLE( area_officer, 7);
		SAVE_VARIABLE( spawner_rotator, 8);
		SAVE_VARIABLE( replacement_vehicle, 9);
		SAVE_VARIABLE( destroyed_sam, 10);
		SAVE_VARIABLE( destroyed_silo, 11);
	}

	void Created (GameObject *obj) override
	{
		int counter;
		counter = 0;
		spawner_rotator = 0;
		replacement_vehicle = -1;
		destroyed_sam = 0;
		destroyed_silo = 0;

		while (counter < ARRAY_ELEMENT_COUNT(active_area))
		{
			active_area [counter] = false;
			area_unit_count [counter] = 0;
			area_officer [counter] = 0;
			area_unit_max [counter] = 4;
			area_spawn_heli_drop_01_active [counter] = false;
			area_spawn_heli_drop_02_active [counter] = false;
			area_spawn_parachute_active [counter] = false;

			// Turning the actives to true in here means they do not occur in the area
			// provided you don't turn them off later on in the mission.

			counter++;
		}

		// Drop the first Humm-Vee into the start area.

		//DEMO Call_GDI_HummVee (0);

		// Wake up the A25 units.

		//DEMO Commands->Send_Custom_Event(obj, Commands->Find_Object(400985), 0, 0, 0.0f);
		//DEMO Turn on A25
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			Commands->Send_Custom_Event(obj, object, 104, 25, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(401741), 0, 0, 0.0f);
		}

		// Call the ORCA strike intro.

		//DEMO GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		/*if (chinook_obj1)
		{
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2A_OrcaStrike.txt");
		}*/

		Commands->Start_Timer(obj, this, 3.0f, 2);

		//DEMO
		Commands->Start_Timer(obj, this, 10.0f, 99);
		Commands->Start_Timer(obj, this, 5.0f, 100);
		Commands->Start_Timer(obj, this, 60.0f, 101);
		//DEMO
	}

	void Call_GDI_PowerUp ()
	{
		Vector3 drop_loc = Vector3(511.3f,680.3f,-21.2f);
		float facing = 146.6f;
		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

		if (chinook_obj1)
		{
			Commands->Set_Facing(chinook_obj1, facing);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2I_GDI_Drop_PowerUp.txt");
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 2)
		{
			// Count through each area, check if it is active.

			int counter;
			counter = 0;
			while (counter < ARRAY_ELEMENT_COUNT(active_area))
			{
				if (active_area[counter])
				{
					// Check if the area still has an officer associated.

					if (area_officer[counter])
					{
						GameObject* officer = Commands->Find_Object(area_officer[counter]);

						if (officer)
						{
							Check_Respawns (counter);
						}
					}
				}
				counter++;
			}
Commands->Debug_Message (">>>>>>>>>>>>>>>> UNIT COUNT = %i, UNIT MAX = %i.\n",area_unit_count[25],area_unit_max[25]);
			int players_difficulty = Commands->Get_Difficulty_Level();
			float timer_length = 25 - (10 * (players_difficulty));
			Commands->Start_Timer(obj, this, timer_length, 2);
		}
		//DEMO
		else if (timer_id == 99)
		{
			GameObject *buggy = Commands->Create_Object ("GDI_Orca", Vector3(506.590f,470.752f,-40.590f));
			if (buggy)
			{
				Commands->Attach_Script (buggy, "MDD_Flying_Vehicle", "0");
			}
		}
		else if (timer_id == 100)
		{
			GameObject *buggy = Commands->Create_Object ("Nod_Apache", Vector3(587.296f,504.567f,-44.034f));
			if (buggy)
			{
				Commands->Attach_Script (buggy, "MDD_Flying_Vehicle", "1");
			}
		}
		else if (timer_id == 101)
		{
			area_unit_count [25] = 0;
			Commands->Start_Timer(obj, this, 60.0f, 101);
		}
		//DEMO
	}

	void Check_Respawns (int area_id)
	{
		// Can this particular area handle respawns?
		// Count through the spawn sequence until all units have been accounted for.

		int counter;
		int stop_point;
		bool stopped_counter;
		int spawner_buffer = 0;

		stopped_counter = false;
		counter = Get_Int_Random(0,2);
		stop_point = counter;

		int tempcount = area_unit_count[area_id];

		while (tempcount < area_unit_max [area_id])
		{
			if (!stopped_counter)
			{
				// Check which respawn type I can use.

				switch (counter)
				{
				case (0):
					{
						if (!area_spawn_heli_drop_01_active[area_id])
						{
							if ((area_unit_max[area_id] - tempcount) > 2)
							{
								area_spawn_heli_drop_01_active[area_id] = true;
								tempcount += 3;
								Use_Heli_Drop_01();
							}
						}
						break;
					}
				case (1):
					{
						if (!area_spawn_heli_drop_02_active[area_id])
						{
							if ((area_unit_max[area_id] - tempcount) > 1)
							{
								area_spawn_heli_drop_02_active[area_id] = true;
								tempcount += 2;
								Use_Heli_Drop_02();
							}
						}
						break;
					}
				case (2):
					{
						if (!area_spawn_parachute_active[area_id])
						{
							if ((area_unit_max[area_id] - tempcount) > 2)
							{
								area_spawn_parachute_active[area_id] = true;
								Use_Parachute_Drop();
								tempcount += 3;
							}
						}
						break;
					}
				}
			}
			else
			{
				spawner_buffer++;
				if (spawner_buffer < 3)
				{
					tempcount ++;
					Use_Spawners ();
				}
				else
				{
					tempcount = area_unit_max[area_id];
				}
			}

			if (!stopped_counter)
			{
				counter++;
				if (counter > 2)
				{
					counter = 0;
				}
				if (counter == stop_point)
				{
					stopped_counter = true;
				}
			}
		}
	}

	// This uses a helicopter to drop three units.

	void Use_Heli_Drop_01 ()
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		parameters = "25,107";
		textfile = "X2I_Drop03_Area25.txt";
		drop_location = Vector3(546.5f,274.5f,-45.0f);
		facing = 30.0f;

		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_location);

		if (chinook_obj1)
		{
			Commands->Set_Facing(chinook_obj1, facing);
			Commands->Attach_Script(chinook_obj1, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(chinook_obj1, cinematic, textfile);
		}
	}

	// This uses a different helicopter route to drop two units.

	void Use_Heli_Drop_02 ()
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		parameters = "25,108";
		textfile = "X2I_Drop02_Area25.txt";
		drop_location = Vector3(558.6f,381.7f,-51.1f);
		facing = -75.6f;

		GameObject * chinook_obj2 = Commands->Create_Object("Invisible_Object", drop_location);

		if (chinook_obj2)
		{
			Commands->Set_Facing(chinook_obj2, facing);
			Commands->Attach_Script(chinook_obj2, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(chinook_obj2, cinematic, textfile);
		}
	}

	// This uses a cargo plane parachute drop to drop three units.

	void Use_Parachute_Drop ()
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		textfile = "X2I_Para03_Area25.txt";
		parameters = "25,109";
		drop_location = Vector3(598.7f,342.2f,-62.7f);
		facing = 173.9f;

		GameObject * plane_obj = Commands->Create_Object("Invisible_Object", drop_location);

		if (plane_obj)
		{
			Commands->Set_Facing(plane_obj, facing);
			Commands->Attach_Script(plane_obj, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(plane_obj, cinematic, textfile);
		}
	}

	// This uses the regular spawners to generate one unit.

	bool Use_Spawners ()
	{
		int spawner_id;
		const char *spawner_params;
		spawner_id = 0;
		spawner_params = "0,0,-1";
		spawner_rotator++;
		if (spawner_rotator > 2)
		{
			spawner_rotator = 0;
		}

		switch (spawner_rotator)
		{
		case (0):
			{
				spawner_id = 401462;
				break;
			}
		case (1):
			{
				spawner_id = 401463;
				break;
			}
		default:
			{
				spawner_id = 401464;
				break;
			}
		}
		spawner_params = "25,0,-1";

		GameObject * spawn1 = Commands->Trigger_Spawner(spawner_id);
		if (spawn1)
		{
			Commands->Attach_Script(spawn1, "MDD_Nod_Soldier", spawner_params);
			return true;
		}
		else
		{
			return false;
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject *sender) override
	{
		if (type == 101)
		{
			// A unit has died, subtract him from his area (param).

			area_unit_count [param]--;
			if (area_unit_count [param] < 0)
			{
				area_unit_count [param] = 0;
			}
		}
		else if (type == 102)
		{
			// A specific area is requesting a forced spawn if possible.

			Check_Respawns (param);
		}
		else if (type == 103)
		{
			// A unit is registering with the area unit count.

			area_unit_count [param]++;
		}
		else if (type == 104)
		{
			// An area is being activated. Turn on the area.
			active_area [param] = true;
		}
		else if (type == 105)
		{
			// An area is being deactivated.

			active_area [param] = false;
		}
		else if (type == 106)
		{
			// An officer is registering with his area.

			if (sender)
			{
				area_officer[param] = Commands->Get_ID(sender);
			}
		}
		else if (type == 107)
		{
			// A Heli_01 spawner type is attempting to reset itself.
			
			area_spawn_heli_drop_01_active[param] = false;
		}
		else if (type == 108)
		{
			// A Heli_02 spawner type is attempting to reset itself.

			area_spawn_heli_drop_02_active[param] = false;
		}
		else if (type == 109)
		{
			// A Parachute spawner type is attempting to reset itself.

			area_spawn_parachute_active[param] = false;
		}
		else if (type == 111)
		{
			// A request to spawn a new vehicle for the commando has been made.

			Replacement_Vehicle (obj, param);
		}
		else if (type == 112)
		{
			// A player controlled vehicle has been destroyed, turn on the ability to make a new one

			replacement_vehicle = param;
		}
		else if (type == 113)
		{
			// A request has been made to call in a GDI power-up drop.

			Call_GDI_PowerUp ();
		}
		else if (type == 114)
		{
			// Reset the unit area count to zero.

			area_unit_count [param] = 0;
		}
		else if (type == 115)
		{
			// Destroyed SAM on the bridge - add to count, determine if both are destroyed.

			destroyed_sam++;
			if (destroyed_sam > 1)
			{
				// Drop a Medium Tank on the bridge.

				replacement_vehicle = 6;
				Replacement_Vehicle (obj, 6);
			}
		}
		else if (type == 116)
		{
			// Destroyed Tiberium Silo - add to count, determine if both are destroyed.

			destroyed_silo++;
			if (destroyed_silo > 1)
			{
				// Complete this mission objective.

				Commands->Send_Custom_Event (obj, obj, 222, 1, 0.0f);
			}
		}
		//DEMO
		else if (type == 180)
		{
			Commands->Start_Timer(obj, this, 20.0f, 99);
		}
		else if (type == 181)
		{
			Commands->Start_Timer(obj, this, 20.0f, 100);
		}
	}

	// When a player controlled vehicle is destroyed, it sends a custom to this controller
	// to turn on the ability to create a new vehicle in a certain area.
	// Another object then sends a custom to check for new vehicle creation. If the flag for
	// creating a new vehicle for this area is active, it is reset and the vehicle is delivered.

	void Replacement_Vehicle (GameObject * /* obj */, int area_id)
	{
		Vector3 drop_loc = Vector3(0.0f,0.0f,0.0f);
		float facing = 0.0f;
		const char* textfile = "X2I_GDI_Drop_HummVee.txt";
		bool nocinematic = false;

		switch (area_id)
		{
		case (0):
			{
				drop_loc = Vector3(161.6f,27.6f,-18.0f);
				facing = 115.4f;
				textfile = "X2I_GDI_Drop_HummVee.txt";
				break;
			}
		case (1):
			{
				drop_loc = Vector3(494.652f,128.786f,-56.159f);
				facing = 90.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Buggy", drop_loc);
				if (buggy)
				{
					Commands->Set_Facing (buggy, facing);
					Commands->Attach_Script (buggy, "M02_Player_Vehicle", "3");
				}
				nocinematic = true;
				break;
			}
		case (2):
			{
				drop_loc = Vector3(471.4f,716.0f,-17.1f);
				facing = -30.0f;
				textfile = "X2I_GDI_Drop_MediumTank.txt";
				break;
			}
		case (4):
			{
				drop_loc = Vector3(429.672f, 894.045f, 4.354f);
				facing = -25.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Light_Tank", drop_loc);
				if (buggy)
				{
					Commands->Set_Facing (buggy, facing);
					Commands->Attach_Script (buggy, "M02_Player_Vehicle", "5");
				}
				nocinematic = true;
				break;
			}
		case (5):
			{
				drop_loc = Vector3(611.1f, 1164.9f, 4.6f);
				facing = -39.7f;
				GameObject *buggy = Commands->Create_Object ("Nod_Recon_Bike", drop_loc);
				if (buggy)
				{
					Commands->Set_Facing (buggy, facing);
					Commands->Attach_Script (buggy, "M02_Player_Vehicle", "6");
				}
				nocinematic = true;
				break;
			}
		case (6):
			{
				drop_loc = Vector3(785.3f,893.9f,21.8f);
				facing = -42.0f;
				textfile = "X2I_GDI_Drop_MediumTank.txt";
				break;
			}
		case (14):
			{
				drop_loc = Vector3(1229.37f, 742.89f, 27.03f);
				facing = 85.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Recon_Bike", drop_loc);
				if (buggy)
				{
					Commands->Set_Facing (buggy, facing);
					Commands->Attach_Script (buggy, "M02_Player_Vehicle", "14");
				}
				nocinematic = true;
				break;
			}
		}

		if (!nocinematic)
		{
			GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

			if (chinook_obj1)
			{
				Commands->Set_Facing(chinook_obj1, facing);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", textfile);
			}
		}
	}
};


DECLARE_SCRIPT (MDD_Nod_Soldier, "Area_Number:int,Area_Officer:int,Pre_Placed:int")
{
	Vector3 my_home_point;
	bool no_return_home;
	int officeranimcounter;
	bool initial_damage;
	float initial_health;
	bool enemy_seen;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( my_home_point, 1);
		SAVE_VARIABLE( no_return_home, 2);
		SAVE_VARIABLE( officeranimcounter, 3);
	}

	void Created (GameObject *obj) override
	{
		officeranimcounter = 0;
		no_return_home = false;
		enemy_seen = false;
		initial_damage = false;
		initial_health = 0.0f;

		// Set the unit's home point, save the value.

		Vector3 this_home_point = Commands->Get_Position(obj);
		Commands->Set_Innate_Soldier_Home_Location(obj, this_home_point, 20.0f);

		// Turn hibernation off for a moment.
		
		Commands->Enable_Hibernation (obj, false);

		// Start the timer to register with the controller.

		Commands->Start_Timer(obj, this, 1.0f, 1);

		//DEMO
		Commands->Start_Timer(obj, this, 10.0f, 8);
		//DEMO

		// If the unit is spawned, do not disturb it.
		// If the unit is preplaced, turn of actions.
		// If the unit is flown in, order it to move to the commando.

		int preplaced = Get_Int_Parameter("Pre_Placed");

		if (!preplaced)
		{
			// Give spawned units a green key for the Hand of Nod.

			Commands->Grant_Key (obj, 1, true);

			//DEMOGameObject *starobj = Commands->Get_A_Star (my_home_point);
			GameObject *starobj = Commands->Find_Object(403063);
			//DEMO

			if (starobj)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 71, 0);
				params.Set_Movement(starobj, RUN, 5.0f, 0);
				Commands->Action_Goto(obj, params);
			}
		}
		else
		{
			if (preplaced > 0)
			{
				// Turn off all actions for preplaced soldiers.

				Commands->Innate_Disable (obj);
			}
		}
	}

	void Enemy_Seen (GameObject * /* obj */, GameObject * /* enemy */) override
	{
		enemy_seen = true;
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		// If an objective shouts, respond to it, letting it know the area is not cleared.

		if (sound.Type == 1000)
		{
			if (sound.Creator)
			{
				Commands->Send_Custom_Event(obj, sound.Creator, 100, 1, 0.0f);
			}
		}

		// If a zone shouts to return to home points, do so.

		else if ((sound.Type > 1049) && (sound.Type < 1100) && (!no_return_home))
		{
			int area_id = (sound.Type - 1050);
			int area_num = Get_Int_Parameter("Area_Number");

			if (area_num == area_id)
			{
				// Tell this unit to return to its home point, conserving unit counts.

				ActionParamsStruct params;
				params.Set_Basic(this, 99, 0);
				params.Set_Movement(my_home_point, RUN, 20.0f, false);
				Commands->Modify_Action (obj, 0, params, true, false);
			}
		}
	}

	void Timer_Expired (GameObject *obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			// Register with the respawn controller - send my area number.

			int officer = Get_Int_Parameter("Area_Officer");
			int preplaced = Get_Int_Parameter("Pre_Placed");
			int param = Get_Int_Parameter("Area_Number");
			GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

			if (object)
			{
				// Register with the respawn controller.

				Commands->Send_Custom_Event(obj, object, 103, param, 0.0f);

				if (officer)
				{
					// Unit is a preplaced officer. Register as an officer and hibernate.
				
					Commands->Set_Innate_Take_Cover_Probability (obj, 100.0f);
					Commands->Send_Custom_Event(obj, object, 106, param, 0.0f);
				}
				else
				{
					if (preplaced > 0)
					{

						// SPECIAL CASE SETUP: Force fire routine for Rocket Soldiers.

						if (preplaced == 4)
						{
							if ((param == 0) || (param == 3) || (param == 6) || (param == 10) || (param == 16) || (param == 21))
							{
								no_return_home = true;
								Commands->Start_Timer(obj, this, 15.0f, 4);
							}
						}

						// Soldiers in start cinematic begin self-destruct.

						if ((preplaced == 2) && (param == 99))
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 99, 0);
							params.Set_Attack(STAR, 150.0f, 0.0f, true);
							Commands->Action_Attack(obj, params);
							Commands->Start_Timer(obj, this, 5.0, 6);
						}
					}
					else
					{
						// Unit is not pre-placed, activate cleanup code and do not hibernate if not in the interior areas.

						Commands->Start_Timer(obj, this, 15.0f, 2);
					}
				}
			}
			//DEMO Commands->Enable_Hibernation (obj, 1);
		}
		else if (timer_id == 2)
		{
			// Cleanup code - find the nearest star, check distance, then check visibility

			Vector3 myloc = Commands->Get_Position(obj);
			//DEMO	GameObject *star_obj = Commands->Get_A_Star(myloc);
			GameObject *star_obj = Commands->Find_Object(403063);
			//DEMO

			if (star_obj)
			{
				Vector3 starloc = Commands->Get_Position(star_obj);
				float distance = Commands->Get_Distance (myloc, starloc);
				
				//DEMOif (distance > 70.0f)
				if (distance > 300.0f)
				{
					bool visibility = Commands->Is_Object_Visible(star_obj, obj);
					if (!visibility)
					{
						Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
					}
					else
					{
						Force_Move (obj);
					}
				}
				else
				{
					Force_Move (obj);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 2);
		}
		else if (timer_id == 3)
		{
			// Force movement code to prevent irregular activity.
		
			Force_Move (obj);
			Commands->Start_Timer(obj, this, 15.0f, 3);
		}
		else if (timer_id == 4)
		{
			// Force fire at star code to prevent irregular activity.

			Vector3 myloc;
			myloc = Commands->Get_Position(obj);
			GameObject* starobj = Commands->Get_A_Star(myloc);

			if (starobj)
			{
				Vector3 starloc;
				float distance;

				starloc = Commands->Get_Position(starobj);
				distance = Commands->Get_Distance(myloc, starloc);

				if (distance < 150.0f)
				{
					ActionParamsStruct params;
					params.Set_Basic(this, 90, 0);
					params.Set_Attack(starobj, 150.0f, 0.0f, true);
					Commands->Action_Attack(obj, params);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 4);
		}
		else if (timer_id == 5)
		{
			// An officer is trying to play an animation.

			Vector3 playerloc = Commands->Get_Position (STAR);
			Vector3 myloc = Commands->Get_Position(obj);

			float distance = Commands->Get_Distance(myloc, playerloc);

			if (distance > 10.0f)
			{
				officeranimcounter++;
				if (officeranimcounter > 2)
				{
					officeranimcounter = 0;
				}

				switch (officeranimcounter)
				{
				case (0):
					{
						GameObject *nearsoldier = Commands->Find_Closest_Soldier (myloc, 1.0f, 50.0f, true);
						if ((nearsoldier) && (nearsoldier != STAR))
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 99, 0);
							Vector3 soldierloc = Commands->Get_Position (nearsoldier);
							params.Set_Animation ("H_A_J18C", false);
							params.Set_Face_Location (soldierloc, 4.0f);
							Commands->Action_Play_Animation (obj, params);
						}
						break;
					}
				case (1):
					{
						if (STAR)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 99, 0);
							params.Set_Animation ("H_A_J23C", false);
							params.Set_Face_Location (playerloc, 4.0f);
							Commands->Action_Play_Animation (obj, params);
						}
						break;
					}
				default:
					{
						if (STAR)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, 99, 0);
							params.Set_Animation ("H_A_J27C", false);
							params.Set_Face_Location (playerloc, 4.0f);
							Commands->Action_Play_Animation (obj, params);
						}
						break;
					}
				}
			}
			Commands->Start_Timer(obj, this, 10.0f, 5);
		}
		else if (timer_id == 6)
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
		else if (timer_id == 7)
		{
			if (!enemy_seen)
			{
				// Time to move the chem-warrior. Pick a new location.
				
				Vector3 newloc;
				int rndnum = (Get_Int_Random(0.0f,1.0f) * 5);
				switch (rndnum)
				{
				case (0):
					{
						newloc = Vector3(1301.55f,617.89f,19.66f);
						break;
					}
				case (1):
					{
						newloc = Vector3(1340.65f,655.04f,19.51f);
						break;
					}
				case (2):
					{
						newloc = Vector3(1310.67f,694.74f,19.71f);
						break;
					}
				default:
					{
						newloc = Vector3(1242.26f,646.04f,19.65f);
						break;
					}
				}
				ActionParamsStruct params;
				params.Set_Basic(this, 31, 0);
				params.Set_Movement(newloc, WALK, 10.0f, false);
				Commands->Action_Goto(obj, params);
			}
			else
			{
				Force_Move (obj);
			}
			Commands->Start_Timer(obj, this, 30.0f, 7);
		}
		//DEMO
		else if (timer_id == 8)
		{
			Vector3 mydemopos = Commands->Get_Position (obj);
			GameObject* starobj = Commands->Find_Object(403063);
			if (starobj)
			{
				Commands->Send_Custom_Event (obj, starobj, 100, 0, 0.0f);
			}
			Commands->Start_Timer(obj, this, 10.0f, 8);
		}
		//DEMO
	}

	void Force_Move (GameObject* obj)
	{
		Vector3 mypos = Commands->Get_Position(obj);
		//DEMO 	GameObject *starobj = Commands->Get_A_Star (mypos);
		GameObject *starobj = Commands->Find_Object(403063);
		//DEMO
		if (starobj)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(starobj, RUN, 10.0f, 0);
			params.Set_Attack(starobj, 150.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
		}
	}

	void Killed (GameObject *obj, GameObject * /* killer */) override
	{
		// Tell the respawn controller I am dead.

		int param;
		param = Get_Int_Parameter("Area_Number");
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			Commands->Send_Custom_Event (obj, object, 101, param, 0.0f);
		}
		if (param == 3)
		{
			int officer = Get_Int_Parameter("Area_Officer");

			if (officer)
			{
				// If a Medium Tank is needed for A03, drop it.
					
				Commands->Send_Custom_Event (obj, object, 111, 2, 0.0f);
			}
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject *sender) override
	{
		if ((type == 0) && (param == 0))
		{
			Commands->Innate_Enable (obj);

			// If the unit is a preplaced minigunner, forced movement code is activated for this unit.
			
			int preplaced = Get_Int_Parameter("Pre_Placed");

			if (preplaced == 2)
			{
				int myid = Commands->Get_ID(obj);
				if ((myid != 400512) || (myid != 400513))
				{
					Commands->Start_Timer(obj, this, 1.0f, 3);
				}
			}
			else if (preplaced == 1)
			{
				// Unit is an officer, start the animation timer.

				Commands->Start_Timer(obj, this, 10.0f, 5);
			}
			else if (preplaced == 6)
			{
				// Unit is a chem-warrior, start low priority wandering.

				Commands->Start_Timer(obj, this, 1.0f, 7);
			}
			else if (preplaced == 5)
			{
				int areanum = Get_Int_Parameter("Area_Number");
				if (areanum == 19)
				{
					// Unit is a technician, who plays an animation in front of a terminal.

					ActionParamsStruct params;
					params.Set_Basic(this, 31, 100);
					params.Set_Animation ("H_A_CON2", true);
					Commands->Action_Play_Animation (obj, params);
				}
			}
		}
		if (type == 99)
		{
			if (param == 214)
			{

				// An Objective or Building is asking to be repaired.

				ActionParamsStruct params;
				params.Set_Basic(this, 99, 214);
				params.Set_Movement(sender, RUN, 10.0f, 0);
				Vector3 senderpos = Commands->Get_Position(sender);
				senderpos.Z -= 1.0f;
				params.Set_Attack(senderpos, 10.0f, 0.0f, false);
				Commands->Action_Attack(obj, params);
			}
			else if (param == 202)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 50, 214);
				params.Set_Movement(Vector3(1113.61f,877.4f,17.15f), RUN, 10.0f, 0);
				params.Set_Attack(Vector3(1113.61f,877.4f,17.15f), 10.0f, 0.0f, false);
				Commands->Action_Attack(obj, params);
			}
			else if (param == 217)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 50, 214);
				params.Set_Movement(Vector3(1062.24f,978.38f,-16.85f), RUN, 10.0f, 0);
				params.Set_Attack(Vector3(1062.24f,978.38f,-16.85f), 10.0f, 0.0f, false);
				Commands->Action_Attack(obj, params);
			}
		}
		if (!initial_damage)
		{
			if (type == CUSTOM_EVENT_FALLING_DAMAGE)
			{
				initial_health = Commands->Get_Health(obj);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject *damager, float /* amount */) override
	{
		if (!initial_damage && damager == NULL)
		{
			initial_damage = true;
			Commands->Set_Health(obj, initial_health);
		}
	}
};


// SOLDIER TYPE PARAMETERS FOR GDI
// 1 = Rocket Soldier
// 2 = Minigunner Officer
// 3 = Minigunner Enlisted
// 4 = Grenadier Enlisted

DECLARE_SCRIPT (MDD_GDI_Soldier, "Area_ID:int, Soldier_Type=0:int")
{
	bool  stop_following;
	//DEMO
	Vector3 startloc;
	//DEMO

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( stop_following, 1);
	}

	void Created(GameObject* obj) override
	{
		//DEMO
		Commands->Enable_Hibernation(obj, false);
		startloc = Commands->Get_Position(obj);
		//DEMO

		stop_following = false;

		// Special case checks for preplaced soldiers - A01 currently, others to come.
		
		int area_id = Get_Int_Parameter("Area_ID");

		if (area_id == 1)
		{
			Commands->Innate_Disable (obj);
		}
		else
		{
			// Default spawned following unit - Give orders to stay near the commando on a timer.

			Commands->Start_Timer(obj, this, 1.0f, 1);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			int type = Get_Int_Parameter("Soldier_Type");
			if (type < 12)
			{
				Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
			}
			if (!stop_following)
			{
				// Follow the commando.

				//Start this check timer again.

				Vector3 mypos = Commands->Get_Position(obj);
				//DEMO GameObject *starobj = Commands->Get_A_Star (mypos);
				GameObject *starobj = Commands->Find_Object(403063);
				//DEMO
				if (starobj)
				{
					ActionParamsStruct params;
					params.Set_Basic(this, 71, 0);
					params.Set_Movement(starobj, 1.0f, 10.0f, 0);
					Commands->Action_Goto(obj, params);
				}
				else
				{
					stop_following = true;
				}

				//DEBUG good time to make a quip or comment.
			}
			Commands->Start_Timer(obj, this, 10.0f, 1);
		}
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		if ((sound.Type > 1099) && (sound.Type < 1199))
		{
			// Make sure this is the sound this unit is supposed to react to.

			int area_id = Get_Int_Parameter("Area_ID");
			if ((sound.Type - 1100) == area_id)
			{
				stop_following = true;
				switch (area_id)
				{
				case (0):
					{
						Vector3 guard_tower_loc = Vector3(128.3f,22.3f,-9.6f);
						ActionParamsStruct params;
						params.Set_Basic(this, 81, 0);
						params.Set_Movement(guard_tower_loc, WALK, 1.0f, 0);
						Commands->Action_Goto(obj, params);
						//DEBUG say something to the commando here indicating a position hold.
						break;
					}
				case (2):
					{
						Vector3 house_loc = Vector3(648.77f,300.74f,-59.99f);
						ActionParamsStruct params;
						params.Set_Basic(this, 81, 0);
						params.Set_Movement(house_loc, WALK, 1.0f, 0);
						Commands->Action_Goto(obj, params);
						//DEBUG say something to the commando here indicating a position hold.
						break;
					}
				}
			}
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if ((type == 0) && (param == 0))
		{
			// A zone is trying to enable actions for this soldier.

			Commands->Innate_Enable (obj);
		}
	}

	//DEMO
	void Destroyed (GameObject* /* obj */) override
	{
		int type = Get_Int_Parameter("Soldier_Type");
		switch (type)
		{
		case (12):
			{
				GameObject * soldier = Commands->Create_Object("GDI_Grenadier_2SF", startloc);
				if (soldier)
				{
					Commands->Attach_Script(soldier, "MDD_GDI_Soldier", "25, 12");
				}
				break;
			}
		case (13):
			{
				GameObject * soldier = Commands->Create_Object("GDI_MiniGunner_2SF", startloc);
				if (soldier)
				{
					Commands->Attach_Script(soldier, "MDD_GDI_Soldier", "25, 13");
				}
				break;
			}
		case (14):
			{
				GameObject * soldier = Commands->Create_Object("GDI_RocketSoldier_2SF", startloc);
				if (soldier)
				{
					Commands->Attach_Script(soldier, "MDD_GDI_Soldier", "25, 14");
				}
				break;
			}
		}
	}
	//DEMO
};


DECLARE_SCRIPT (MDD_Stationary_Vehicle,"Area_ID:int")
{
	void Created (GameObject* obj) override
	{
		//DEMO
		Commands->Set_Shield_Type (obj, "Blamo");
		Commands->Enable_Enemy_Seen (obj, true);
		//DEMO

		int area_id;
		area_id = Get_Int_Parameter("Area_ID");

		// Area One is a conflict area, other stationary vehicles in conflict areas must be paused as well

		//DEMO Commands->Enable_Enemy_Seen (obj, false);

		if (area_id == 99)
		{
			Commands->Start_Timer(obj, this, 6.0f, 1);
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 90, 0);
		params.Set_Attack(enemy, 300.0f, 0.0f, true);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack(obj, params);
		Commands->Start_Timer(obj, this, 5.0f, 2);
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		// If an objective shouts, respond to it, letting it know the area is not cleared.

		if (sound.Type == 1000)
		{
			if (sound.Creator)
			{
				int	player_type = Commands->Get_Player_Type (obj);
				if (player_type == SCRIPT_PLAYERTYPE_NOD)
				{
					Commands->Send_Custom_Event(obj, sound.Creator, 100, 1, 0.0f);
				}
			}
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if ((type == 0) && (param == 0))
		{
			// A zone is trying to enable actions for this vehicle.

			Commands->Enable_Enemy_Seen (obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
		}
		else if (timer_id == 2)
		{
			Commands->Action_Reset (obj, 100);
		}
	}
};


DECLARE_SCRIPT (MDD_Nod_Apache, "Area_ID:int")
{
	int waypath_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( waypath_id, 1);
	}

	void Created (GameObject* obj) override
	{
		int area_id = Get_Int_Parameter("Area_ID");
		float timer_len = 1.0f;

		//First waypath means area for looping Apaches, it is set when timer expires, and then uses real IDs.
		
		switch (area_id)
		{
		case (0):
			{
				waypath_id = 0;
				timer_len = 8.0f;
				break;
			}
		case (15):
			{
				waypath_id = 1;
				timer_len = 15.0f;
				break;
			}
		case (24):
			{
				waypath_id = 24;
				break;
			}
		}

		Commands->Enable_Engine(obj, true);
		Commands->Disable_Physical_Collisions(obj);
		Commands->Start_Timer(obj, this, timer_len, 1);
		Commands->Start_Timer(obj, this, 15.0f, 3);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			// Delayed initial action.

			switch (waypath_id)
			{
			case (0): // Area 0 Apache delay timer expired, start movement
				{
					waypath_id = 400296;
					break;
				}
			case (1): // Area 15 Apache delay timer expired, start movement.
				{
					waypath_id = 401169;
					break;
				}
			case (24):
				{
					waypath_id = 400544;
					break;
				}
			case (400296):
				{
					waypath_id = 400304;
					break;
				}
			case (400304):
				{
					waypath_id = 400310;
					break;
				}
			case (400310):
				{
					waypath_id = 400304;
					break;
				}
			case (400544):
				{
					waypath_id = -1;
					break;
				}
			case (401169):
				{
					waypath_id = 401173;
					break;
				}
			case (401173):
				{
					waypath_id = 401180;
					break;
				}
			case (401180):
				{
					waypath_id = 401173;
					break;
				}
			}
			if (waypath_id != -1)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 90, 0);
				params.Set_Movement(Vector3(0,0,0), 0.2 + (0.1 * DIFFICULTY), 5.0f);
				params.WaypathID = waypath_id;
				params.WaypathSplined = true;
				params.Set_Attack(STAR, 200.0f, 0.0f, true);
				params.AttackCheckBlocked = false;
				params.AttackActive = false;
				Commands->Action_Attack(obj, params);
			}
			Commands->Start_Timer(obj, this, 5.0f, 2);
		}
		else if (timer_id == 2)
		{
			// Time to start firing, and move again in a second.

			Vector3 myloc = Commands->Get_Position(obj);
			//DEMO
			GameObject* target = Commands->Find_Object(403063);
			//DEMO

			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(myloc, 0.0, 5.0f);
			//DEMO params.Set_Attack(STAR, 200.0f, 0.0f, true);
			params.Set_Attack(target, 200.0f, 0.0f, true);
			//DEMO
			params.AttackCheckBlocked = false;
			params.AttackActive = true;
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this, 5.0f, 1);
		}
		else if (timer_id == 3)
		{
			// Cleanup code - find the nearest star, check distance.

			float distance;
			Vector3 myloc;
			Vector3 starloc;
			myloc = Commands->Get_Position(obj);
			GameObject *star_obj = Commands->Get_A_Star(myloc);
			if (star_obj)
			{
				starloc = Commands->Get_Position(star_obj);
				distance = Commands->Get_Distance (myloc, starloc);
				//DEMO if (distance > 300.0f)
				if (distance > 3000.0f)
				//DEMO
				{
					Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 3);
		}
	}

	// If an objective shouts, respond to it, letting it know the area is not cleared.

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		if (sound.Type == 1000)
		{
			if (sound.Creator)
			{
				Commands->Send_Custom_Event(obj, sound.Creator, 100, 1, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MDD_Nod_Stealth, "")
{
	bool nofiring;

	void Created (GameObject* obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		nofiring = true;
		Commands->Enable_Enemy_Seen (obj, true);
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 0);
		params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
		params.WaypathID = 401465;
		Commands->Action_Goto(obj, params);
		Commands->Start_Timer(obj, this, 10.0f, 1);
		Commands->Start_Timer(obj, this, 2.0f, 3);
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		if (!nofiring)
		{
			nofiring = true;
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 0);
			params.Set_Attack(enemy, 200.0f, 0.0f, true);
			params.AttackActive = true;
			Commands->Action_Attack (obj, params);
			Commands->Start_Timer(obj, this, 10.0f, 2);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			nofiring = false;
		}
		else if (timer_id == 2)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 0);
			params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
			params.WaypathID = 401465;
			Commands->Action_Goto(obj, params);
			Commands->Start_Timer(obj, this, 10.0f, 1);
		}
		else if (timer_id == 3)
		{
			Vector3 myloc = Commands->Get_Position (obj);
			Commands->Create_Logical_Sound (obj, 1999, myloc, 10.0f);
			Commands->Start_Timer(obj, this, 2.0f, 3);
		}
	}

	void Destroyed (GameObject* /* obj */) override
	{
		GameObject * nodtank = Commands->Create_Object("Nod_Stealth_Tank", Vector3(598.777f,479.691f,-55.824f));
		if (nodtank)
		{
			Commands->Set_Facing(nodtank, -100.0f);
			Commands->Attach_Script(nodtank, "MDD_Nod_Stealth", "");
		}
	}
};


DECLARE_SCRIPT (MDD_Flying_Vehicle, "Unit_ID:int")
{
	void Created (GameObject* obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		Commands->Enable_Enemy_Seen (obj, true);
		int unit_id = Get_Int_Parameter ("Unit_ID");
		int waypath_id = 0;
		if (unit_id)
		{
			waypath_id = 402786;
		}
		else
		{
			waypath_id = 402768;
		}

		Commands->Enable_Engine(obj, true);
		Commands->Disable_Physical_Collisions(obj);

		ActionParamsStruct params;
		params.Set_Basic(this, 99, 0);
		params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
		params.WaypathID = waypath_id;
		params.WaypathSplined = true;
		Commands->Action_Goto(obj, params);

		Commands->Start_Timer(obj, this, 15.0f, 1);
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 92, 2);
		params.AttackCheckBlocked = false;
		params.Set_Attack(enemy, 300.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			Vector3 myloc = Commands->Get_Position(obj);
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 1);
			params.Set_Movement(myloc, 0.0, 5.0f);
			Commands->Action_Goto(obj, params);
			Commands->Start_Timer(obj, this, 10.0f, 2);
		}
		else if (timer_id == 2)
		{
			int waypath_id = 0;
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 0);
			params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
			int unit_id = Get_Int_Parameter ("Unit_ID");
			if (unit_id)
			{
				waypath_id = 402798;
			}
			else
			{
				waypath_id = 402777;
			}
			params.WaypathID = waypath_id;
			params.WaypathSplined = true;
			Commands->Action_Goto(obj, params);

			Commands->Start_Timer(obj, this, 15.0f, 3);
		}
		else
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
	}

	void Destroyed (GameObject* obj) override
	{
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);
		if (object)
		{
			int unit_id = Get_Int_Parameter ("Unit_ID");
			if (!unit_id)
			{
				Commands->Send_Custom_Event(obj, object, 180, 0, 0.0f);
			}
			else
			{
				Commands->Send_Custom_Event(obj, object, 181, 0, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MDD_Commando, "")
{
	void Created (GameObject* obj) override
	{
		Commands->Control_Enable (obj, false);
		Commands->Set_Shield_Type (obj, "Blamo");
	}
};


DECLARE_SCRIPT (MDD_Havoc_Unit, "")
{
	bool forcemove;

	void Created (GameObject* obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		forcemove = true;
		Commands->Set_Shield_Type (obj, "Blamo");
		Commands->Set_Shield_Strength (obj, 100.0f);
		Commands->Start_Timer(obj, this, 5.0f, 1);

		// move to the center of the field
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 1);
		params.Set_Movement(Vector3(544.119f,339.562f,-60.75f), 1.0, 20.0f);
		Commands->Action_Goto(obj, params);
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if ((type == 100) && (param == 0))
		{
			if (forcemove)
			{
				forcemove = false;
				ActionParamsStruct params;
				params.Set_Basic(this, 70, 1);
				params.Set_Movement(sender, 1.0, 1.0f);
				Commands->Action_Goto(obj, params);
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			forcemove = true;
			Commands->Start_Timer(obj, this, 10.0f, 1);
		}
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		if (sound.Type == 1999)
		{
			if (sound.Creator)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 99, 1);
				params.Set_Movement(Vector3(544.119f,339.562f,-60.75f), 1.0, 20.0f);
				Commands->Action_Goto(obj, params);
			}
		}
	}
};