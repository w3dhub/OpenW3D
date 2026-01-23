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
#include "MissionX0.h"


// MX0 A02 PROTOTYPING SCRIPT SECTION

DECLARE_SCRIPT (MX0_A02_Controller, "")
{
	int MX0_A02_UNIT_ID [MX0_A02_ACTOR_HEADCOUNT];
	bool pre_ambient_on;
	bool ran_startup;
	bool player_retreat;
	bool sequence_active;
	bool prevent_spawns;
	bool retreat_sequence;
	bool call_heli_destroy_once;
	bool entered_tank;
	int havoc_entered_tank;
	int pre_ambient_count;
	int MX0_A02_HUMM_VEE_ID;
	int MX0_A02_MED_TANK_ID;
	int MX0_A02_GDI_APC_ID;
	int	MX0_A01_NOD_SNIPER_01;
	int MX0_A01_NOD_SNIPER_02;
	int MX0_A02_BLOCKAGE_ID;
	int nod_spawn_sequence;
	int current_nod_target;
	int engineer_01_id;
	int engineer_02_id;
	int congrats_loop;
	int soldier_congrats_loop;
	int death_counter;
	int wrong_way_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (MX0_A02_UNIT_ID, 1);
		SAVE_VARIABLE (pre_ambient_on, 2);
		SAVE_VARIABLE (pre_ambient_count, 3);
		SAVE_VARIABLE (MX0_A02_HUMM_VEE_ID, 4);
		SAVE_VARIABLE (MX0_A02_MED_TANK_ID, 5);
		SAVE_VARIABLE (MX0_A02_GDI_APC_ID, 6);
		SAVE_VARIABLE (MX0_A01_NOD_SNIPER_01, 7);
		SAVE_VARIABLE (MX0_A01_NOD_SNIPER_02, 8);
		SAVE_VARIABLE (player_retreat, 9);
		SAVE_VARIABLE (nod_spawn_sequence, 10);
		SAVE_VARIABLE (sequence_active, 11);
		SAVE_VARIABLE (current_nod_target, 12);
		SAVE_VARIABLE (prevent_spawns, 13);
		SAVE_VARIABLE (MX0_A02_BLOCKAGE_ID, 14);
		SAVE_VARIABLE (retreat_sequence, 15);
		SAVE_VARIABLE (call_heli_destroy_once, 16);
		SAVE_VARIABLE (havoc_entered_tank, 17);
		SAVE_VARIABLE (engineer_01_id, 18);
		SAVE_VARIABLE (engineer_02_id, 19);
		SAVE_VARIABLE (congrats_loop, 20);
		SAVE_VARIABLE (soldier_congrats_loop, 21);
		SAVE_VARIABLE (entered_tank, 22);
		SAVE_VARIABLE (death_counter, 23);
		SAVE_VARIABLE (wrong_way_count, 24);
		SAVE_VARIABLE (ran_startup, 25);
	}

	void Created (GameObject * /*obj*/) override
	{
		ran_startup = false;
		pre_ambient_on = false;
		player_retreat = false;
		sequence_active = true;
		prevent_spawns = false;
		retreat_sequence = false;
		call_heli_destroy_once = false;
		entered_tank = false;
		havoc_entered_tank = 0;
		pre_ambient_count = 0;
		MX0_A02_HUMM_VEE_ID = 0;
		MX0_A02_MED_TANK_ID = 0;
		MX0_A02_GDI_APC_ID = 0;
		MX0_A01_NOD_SNIPER_01 = 0;
		MX0_A01_NOD_SNIPER_02 = 0;
		MX0_A02_BLOCKAGE_ID = 0;
		nod_spawn_sequence = 0;
		current_nod_target = 0;
		engineer_01_id = 0;
		engineer_02_id = 0;
		congrats_loop = 0;
		soldier_congrats_loop = 0;
		death_counter = 0;
		wrong_way_count = 1;
	}

	void Attach_Unit_Script (GameObject * unit, int count)
	{
		if (unit)
		{
			Commands->Attach_Script (unit, "MX0_A02_ACTOR", "0");
			MX0_A02_UNIT_ID [count] = Commands->Get_ID (unit);
			Commands->Send_Custom_Event (unit, unit, MX0_A02_CUSTOM_TYPE_REGISTER_ACTOR_ID, count, 0.1f);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		switch (type)
		{
		case (MX0_A02_CUSTOM_TYPE_MAIN_ENDING):
			{
				// Send shutoff announcements to all my registered actors.
				int count = 1;
				GameObject * actor = obj;
				while (count < MX0_A02_ACTOR_HEADCOUNT)
				{
					actor = Commands->Find_Object (MX0_A02_UNIT_ID [count]);
					if (actor)
					{
						Commands->Send_Custom_Event (obj, actor, MX0_A02_CUSTOM_TYPE_MAIN_ENDING, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					count++;
				}
				//DEBUG send out the required A03 custom.
				Commands->Destroy_Object (obj);
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_01_ID):
			{
				if (sender)
				{
					Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_KILL_SNIPER_01, MX0_A01_NOD_SNIPER_01, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_02_ID):
			{
				if (sender)
				{
					Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_KILL_SNIPER_02, MX0_A01_NOD_SNIPER_02, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GET_SNIPER_01_ID):
			{
				MX0_A01_NOD_SNIPER_01 = param;
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GET_SNIPER_02_ID):
			{
				MX0_A01_NOD_SNIPER_02 = param;
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_MAIN_STARTUP):
			{
				if (!ran_startup)
				{
					ran_startup = true;
					GameObject * spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_01);
					GameObject * unit = Commands->Find_Object (MX0_A02_MOVE_OBJ_01);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_GDI_MiniGunner_0_B", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 1);
						Commands->Attach_Script(unit, "M00_Send_Object_ID", "1400041,11,0.0f");
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_03);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_GDI_MiniGunner_1Off", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 2);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_05);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_GDI_RocketSoldier_1Off", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 3);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_07);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_GDI_MiniGunner_0", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 4);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_09);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_Nod_Minigunner_0", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 5);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_11);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_Nod_Minigunner_0", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 6);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_13);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_Nod_Minigunner_0", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 7);
					}
					spawnloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_15);
					if (spawnloc)
					{
						unit = Commands->Create_Object ("MX0_Nod_Minigunner_0", Commands->Get_Position(spawnloc));
						Attach_Unit_Script (unit, 8);
					}
					pre_ambient_on = true;
					pre_ambient_count = 1;
					Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_PRE_AMBIENT);

					GameObject * humm_vee = Commands->Create_Object ("MX0_GDI_Humm-Vee_destroyed", Vector3(-84.582f,-60.035f,1.050f));
					if (humm_vee)
					{
						Commands->Attach_Script (humm_vee, "MX0_A02_GDI_VEHICLE", "");
						Commands->Set_Facing (humm_vee, 50.0f);
						MX0_A02_HUMM_VEE_ID = Commands->Get_ID (humm_vee);
					}

					GameObject * med_tank = Commands->Create_Object ("MX0_GDI_Medium_Tank_Dec", Vector3(-90.183f,-51.691f,1.216f));
					if (med_tank)
					{
						Commands->Attach_Script (med_tank, "MX0_A02_GDI_VEHICLE", "");
						Commands->Set_Facing (med_tank, -20.0f);
						MX0_A02_MED_TANK_ID = Commands->Get_ID (med_tank);
					}
					GameObject * gdi_apc = Commands->Create_Object ("MX0_GDI_APC", Vector3(-101.576f,-54.945f,1.050f));
					if (gdi_apc)
					{
						Commands->Attach_Script (gdi_apc, "MX0_A02_GDI_APC", "");
						Commands->Set_Facing (gdi_apc, -45.0f);
						MX0_A02_GDI_APC_ID = Commands->Get_ID (gdi_apc);
					}

					GameObject * debris = Commands->Create_Object ("Simple_Level_x0_A02_Blockage", Vector3(-71.233f, -41.517f, 0.181f));
					if (debris)
					{
						MX0_A02_BLOCKAGE_ID = Commands->Get_ID (debris);
						Commands->Set_Facing (debris, -40.000f);
					}

					GameObject * a01_controller = Commands->Find_Object (MX0_A01_CONTROLLER_ID);
					if (a01_controller)
					{
						Commands->Send_Custom_Event (obj, a01_controller, MX0_A02_CUSTOM_TYPE_GET_SNIPER_01_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, a01_controller, MX0_A02_CUSTOM_TYPE_GET_SNIPER_02_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_RANDOM_EXPLOSIONS);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_REQUEST_TARGET):
			{
				if (sender)
				{
					if (param < MX0_A02_ACTOR_NOD_START)
					{
						Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_TYPE_REQUEST_TARGET, MX0_A02_UNIT_ID[Get_Int_Random(MX0_A02_ACTOR_NOD_START, (MX0_A02_ACTOR_HEADCOUNT - 1))], 0.0f);
					}
					else
					{
						Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_TYPE_REQUEST_TARGET, MX0_A02_UNIT_ID[Get_Int_Random(1, (MX0_A02_ACTOR_NOD_START - 1))], 0.0f);
					}
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_STARTUP):
			{
				// Check if the First Nod Sniper is still alive. If so, send unit 03 after him.

				if (!player_retreat)
				{
					player_retreat = true;
					GameObject * sniper = Commands->Find_Object (MX0_A01_NOD_SNIPER_01);
					if (sniper)
					{
						GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
						if (soldier)
						{
							Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_KILL_SNIPER_01, MX0_A01_NOD_SNIPER_01, 0.0f);
						}
					}
					else
					{
						sniper = Commands->Find_Object (MX0_A01_NOD_SNIPER_02);
						if (sniper)
						{
							GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
							if (soldier)
							{
								Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_KILL_SNIPER_02, MX0_A01_NOD_SNIPER_02, 0.0f);
							}
						}
					}

					// Start greeting sequence with Unit 01.

					GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_START_GDI_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}

					// Tell the APC it's okay to start shooting.

					GameObject * apc = Commands->Find_Object (MX0_A02_GDI_APC_ID);
					if (apc)
					{
						Commands->Send_Custom_Event (obj, apc, MX0_A02_CUSTOM_TYPE_STARTUP, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
				}
			}

			// Pre ambient shuts off regardless when sequence is started.

		case (MX0_A02_CUSTOM_TYPE_PRE_AMBIENT_OFF):
			{
				pre_ambient_on = false;
				Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_PLAYER_RETREAT);
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_CONTINUE):
			{
				// Make the Nod soldiers able to be killed.

				int nod_count = MX0_A02_ACTOR_NOD_START;
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				while (nod_count < MX0_A02_ACTOR_HEADCOUNT)
				{
					soldier = Commands->Find_Object (MX0_A02_UNIT_ID[nod_count]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_DAMAGE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					nod_count++;
				}
				sequence_active = false;
				nod_spawn_sequence = 0;


				// Have the Nod Officer say something.
				Commands->Create_2D_Sound ("MX0_NODOFFICER_044");
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_START_GDI_02):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_START_GDI_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_START_GDI_03):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_START_GDI_03, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_START_GDI_04):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_START_GDI_04, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_START_GDI_05):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_START_GDI_05, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_PLAYER_KILLED_NOD):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[Get_Int_Random(1,4)]);
				if (soldier)
				{
					if (congrats_loop > 9)
					{
						congrats_loop = 0;
					}
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_PLAYER_CONGRATS, congrats_loop, 0.0f);
					congrats_loop++;
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_SOLDIER_KILLED_NOD):
			{

				GameObject * soldier = Commands->Find_Object (param);
				if (soldier)
				{
					if (soldier_congrats_loop > 3)
					{
						soldier_congrats_loop = 0;
					}
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SOLDIER_CONGRATS, soldier_congrats_loop, 0.0f);
					soldier_congrats_loop++;
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_NEXT_SEQUENCE):
			{
				current_nod_target = 0;
				sequence_active = false;
				nod_spawn_sequence++;
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_NOD_SOLDIER_KILLED):
			{
				//Remove this soldier's record from the lineup.

				int count = MX0_A02_ACTOR_NOD_START + 1;
				int space_available = 0;

				while (count < MX0_A02_ACTOR_HEADCOUNT)
				{
					GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[count]);
					if ((count == param) || (!soldier))
					{
						MX0_A02_UNIT_ID[count] = 0;
					}
					if (MX0_A02_UNIT_ID[count] == 0)
					{
						death_counter++;
						space_available++;
					}
					count++;
				}

				if ((death_counter > 2) && (death_counter < 99))
				{
					death_counter = 99;
					Commands->Fade_Background_Music( "Level 0 Hero.mp3", 2, 2);
				}
				if (!retreat_sequence)
				{
					if (sequence_active)
					{
						if ((current_nod_target == param) && (current_nod_target))
						{
							current_nod_target = 0;
							sequence_active = false;
							nod_spawn_sequence++;
						}
					}
					if (!prevent_spawns)
					{
						prevent_spawns = true;

						if (!sequence_active)
						{
							//If the amount of unit space is adequate, increment the sequence and spawn the units. Tell them what they are to do.

							if (space_available)
							{
								switch (nod_spawn_sequence)
								{
								case(0): // Nod Guy falls off ledge after getting shot.
									{
										GameObject * soldier = Commands->Create_Object ("MX0_Nod_Minigunner_0", Vector3(-78.442f,-78.044f,6.404f));
										if (soldier)
										{
											space_available--;
											sequence_active = true;
											Commands->Attach_Script (soldier, "MX0_A02_ACTOR", "1");
											MX0_A02_Fill_Empty_Slot (soldier, true);
										}
										break;
									}
								case (1): // Second Nod guy falls off ledge after getting shot.
									{
										GameObject * soldier = Commands->Create_Object ("MX0_Nod_Minigunner_0", Vector3(-102.985f,-73.331f,4.415f));
										if (soldier)
										{
											space_available--;
											sequence_active = true;
											Commands->Attach_Script (soldier, "MX0_A02_ACTOR", "2");
											MX0_A02_Fill_Empty_Slot (soldier, true);
										}
										break;
									}
								case (2):
									{
										GameObject * soldier = Commands->Create_Object ("Nod_RocketSoldier_0", Vector3(-78.442f,-78.044f,6.404f));
										if (soldier)
										{
											space_available--;
											sequence_active = true;
											Commands->Attach_Script (soldier, "MX0_A02_ACTOR", "3");
											MX0_A02_Fill_Empty_Slot (soldier, false);
										}
										break;
									}
								case (3): // Nod Chopper drops 2 minigunners, not tracked.
									{
										GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(-71.497f,-47.387f,9.0f));
										if (chinook_obj1)
										{
											sequence_active = true;
											Commands->Set_Facing(chinook_obj1, -100.0f);
											Commands->Attach_Script(chinook_obj1, "MX0_A02_HELICOPTER", "0");
											Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X0I_Drop02_A02_E01.txt");
											Commands->Start_Timer (obj, this, 20.0f, MX0_A02_TIMER_AFTER_HELI_01);
										}
										GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
										if (soldier)
										{
											Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_HELI_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 10.0f);
										}
										break;
									}
								case (4): // Next chopper arrives with FlameThrower infantry.
									{
										nod_spawn_sequence = 5;
										GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(-83.454f,-70.323f,9.60f));
										if (chinook_obj1)
										{
											sequence_active = true;
											Commands->Set_Facing(chinook_obj1, 90.0f);
											Commands->Attach_Script(chinook_obj1, "MX0_A02_HELICOPTER", "1");
											Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X0I_Drop02_A02_E02.txt");
										}
										GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
										if (soldier)
										{
											Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_HELI_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 10.0f);
										}
										break;
									}
								default:
									{
										break;
									}
								}
							}
						}

						// Fill up any remaining space with generic units.

						if (space_available)
						{
							int count = MX0_A02_ACTOR_NOD_START;
							while (count < MX0_A02_ACTOR_HEADCOUNT)
							{
								if (!MX0_A02_UNIT_ID[count])
								{
									int spawnloc_id = 0;
									switch (count)
									{
									case (6):
										{
											spawnloc_id = MX0_A02_SPAWNER_01;
											break;
										}
									case (7):
										{
											spawnloc_id = MX0_A02_SPAWNER_02;
											break;
										}
									case (8):
										{
											spawnloc_id = MX0_A02_SPAWNER_03;
											break;
										}
									default:
										{
											break;
										}
									}
									GameObject * spawnloc = Commands->Trigger_Spawner (spawnloc_id);
									if (spawnloc)
									{
										Attach_Unit_Script (spawnloc, count);
										Commands->Send_Custom_Event (obj, spawnloc, MX0_A02_CUSTOM_TYPE_DAMAGE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.1f);
										count = MX0_A02_ACTOR_HEADCOUNT;
									}
								}
								count++;
							}
						}
						Commands->Start_Timer (obj, this, 3.0f, MX0_A02_TIMER_PREVENT_SPAWNS);
					}
				}
				else
				{
					GameObject * spawner = Commands->Find_Object (MX0_A02_SPAWNER_01);
					if (spawner)
					{
						Commands->Destroy_Object (spawner);
					}
					spawner = Commands->Find_Object (MX0_A02_SPAWNER_02);
					if (spawner)
					{
						Commands->Destroy_Object (spawner);
					}
					spawner = Commands->Find_Object (MX0_A02_SPAWNER_03);
					if (spawner)
					{
						Commands->Destroy_Object (spawner);
					}
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_NOD_01):
			{
				// Make GDI 01 say something about the soldier spotted.

				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					GameObject * target = Commands->Find_Object (MX0_A02_UNIT_ID[current_nod_target]);
					if (target)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_NOD_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_01):
			{
				// Time for GDI to shoot at the first falling soldier.

				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
				if (soldier)
				{
					GameObject * target = Commands->Find_Object (MX0_A02_UNIT_ID[current_nod_target]);
					if (target)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_01, MX0_A02_UNIT_ID[current_nod_target], 0.0f);
					}
				}

				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_RESPONDS_01):
			{
				// Make 01 say something about the falling soldier.

				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_RESPONDS_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_02):
			{
				// Time for GDI to shoot at the second falling soldier.

				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
				if (soldier)
				{
					GameObject * target = Commands->Find_Object (MX0_A02_UNIT_ID[current_nod_target]);
					if (target)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_02, MX0_A02_UNIT_ID[current_nod_target], 0.0f);
					}
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_APC_BLOWITUP):
			{
				GameObject * apc = Commands->Find_Object (MX0_A02_GDI_APC_ID);
				if (apc)
				{
					Commands->Send_Custom_Event (obj, apc, MX0_A02_CUSTOM_TYPE_DESTROY_APC, MX0_A02_CUSTOM_PARAM_DEFAULT, 2.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GET_APC_ID):
			{
				GameObject * apc = Commands->Find_Object (MX0_A02_GDI_APC_ID);
				if (apc)
				{
					if (sender)
					{
						Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_TYPE_GET_APC_ID, MX0_A02_GDI_APC_ID, 0.0f);
					}
				}
				else
				{
					if (sender)
					{
						Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_TYPE_GET_APC_ID, 0, 0.0f);
					}
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_HELI_DESTROYED):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_HELI_DESTROYED, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME_DEAD):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME_DEAD, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_HELI_DESTROYED_02):
			{
				if (!call_heli_destroy_once)
				{
					player_retreat = false;
					call_heli_destroy_once = true;
					GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_HELI_DESTROYED_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.5f);
					}
					soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_HELI_DESTROYED_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 1.5f);
					}
					Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_NOD_RETREAT);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENDING_02):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_ENDING_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENDING_03):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_ENDING_03, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENDING_04):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_ENDING_04, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENDING_05):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_ENDING_05, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENDING_06):
			{
				Commands->Create_2D_Sound ("MX0_GDIEAGLEBASE_110");
				Commands->Start_Timer (obj, this, 5.0f, MX0_A02_TIMER_EAGLE_BASE);
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENGINEER_MOVETOTANK):
			{
				if (sender)
				{
					engineer_01_id = Commands->Get_ID (sender);
					Commands->Send_Custom_Event (obj, sender, MX0_A02_CUSTOM_TYPE_ENGINEER_MOVETOTANK, MX0_A02_MED_TANK_ID, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENGINEER_DONE):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SAY_TANK, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					Commands->Start_Timer (obj, this, 5.0f, MX0_A02_TIMER_ENTER_TANK);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENGINEER_02_REGISTER):
			{
				engineer_02_id = Commands->Get_ID (sender);
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_ENTERED_TANK):
			{
				entered_tank = true;
				GameObject * controller = Commands->Find_Object (1400041);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MX0_A03_CUSTOM_TYPE_START_ZONE, MX0_A02_CUSTOM_PARAM_DEFAULT, 2.0f);
				}
				GameObject * soldier = Commands->Find_Object (engineer_02_id);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SAY_FIREHOLE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_EXPLODE):
			{
				GameObject * debris = Commands->Find_Object (MX0_A02_BLOCKAGE_ID);
				if (debris)
				{
					Commands->Create_Explosion ("Ground Explosion 01 - Harmless", Vector3(-71.233f, -41.517f, 0.181f), NULL);
					Commands->Set_Animation (debris, "AG_L0_BLOCKAGE1.AG_L0_BLOCKAGE1", false, NULL, 0.0f, -1.0f, false);
					Commands->Start_Timer (obj, this, 2.0f, MX0_A02_TIMER_DESTROY_RUBBLE);
				}
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_FOLLOW_HAVOC, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_WRONG_WAY);
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_SAY_APC_DOWN):
			{
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SAY_APC_DOWN, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MX0_A02_CUSTOM_TYPE_SHOOT_HELI_02):
			{
				int count = 1;
				while (count < 5)
				{
					GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[count]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SHOOT_HELI_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					count++;
				}
			}
		default:
			{
				break;
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		switch (timer_id)
		{
		case (MX0_A02_TIMER_PRE_AMBIENT):
			{
				if (pre_ambient_on)
				{
					GameObject * soldier = NULL;
					switch (pre_ambient_count)
					{
					case (1):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
							break;
						}
					case (2):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
							break;
						}
					case (3):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
							break;
						}
					case (4):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
							break;
						}
					case (5):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
							break;
						}
					case (6):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
							break;
						}
					case (7):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
							break;
						}
					case (8):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
							break;
						}
					case (9):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
							break;
						}
					case (10):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[3]);
							break;
						}
					case (11):
						{
							soldier = Commands->Find_Object (MX0_A02_UNIT_ID[4]);
							break;
						}
					default:
						{
							break;
						}
					}
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_PRE_AMBIENT, pre_ambient_count, 0.0f);
					}
					pre_ambient_count++;
					if (pre_ambient_count > MX0_A02_PRE_AMBIENT_MAX)
					{
						pre_ambient_count = 1;
					}
					Commands->Start_Timer (obj, this, Get_Int_Random (2, 4), MX0_A02_TIMER_PRE_AMBIENT);
				}
				break;
			}
		case (MX0_A02_TIMER_PLAYER_RETREAT):
			{
				if (player_retreat)
				{
					Vector3 player_loc = Commands->Get_Position (STAR);
					Vector3 controller_loc = Commands->Get_Position (obj);
					float distance = Commands->Get_Distance (player_loc, controller_loc);
					if (distance > MX0_A02_PLAYER_RETREAT_DISTANCE)
					{
						GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[Get_Int_Random(1,4)]);
						if (soldier)
						{
							Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_PLAYER_RETREAT, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
					}
					Commands->Start_Timer (obj, this, 3.0f, MX0_A02_TIMER_PLAYER_RETREAT);
				}
				break;
			}
		case (MX0_A02_TIMER_PREVENT_SPAWNS):
			{
				if (!retreat_sequence)
				{
					prevent_spawns = false;
					Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_NOD_SOLDIER_KILLED, 0, 0.1f);
				}
				break;
			}
		case (MX0_A02_TIMER_AFTER_HELI_01):
			{
				Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_NEXT_SEQUENCE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				break;
			}
		case (MX0_A02_TIMER_NOD_RETREAT):
			{
				sequence_active = false;
				prevent_spawns = true;
				retreat_sequence = true;
				player_retreat = false;

				GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(-106.497f,-37.154f,1.093f));
				if (chinook_obj1)
				{
					Commands->Set_Facing(chinook_obj1, -55.0f);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X0I_GDI_Drop02_Engineer.txt");
				}
				GameObject * music_control = Commands->Find_Object (1700006);
				if (music_control)
				{
					Commands->Destroy_Object (music_control);
				}

				int count = MX0_A02_ACTOR_NOD_START;
				while (count < MX0_A02_ACTOR_HEADCOUNT)
				{
					if (MX0_A02_UNIT_ID[count])
					{
						GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[count]);
						if (soldier)
						{
							Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_RETREAT_SEQUENCE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
					}
					count++;
				}
				Commands->Create_2D_Sound ("MX0_NODSOLDIER1_101");
				Commands->Start_Timer (obj, this, 3.0f, MX0_A02_TIMER_RETREAT_DONE);
				break;
			}
		case (MX0_A02_TIMER_RETREAT_DONE):
			{
				// Make soldiers move to default positions.

				int count = 1;
				while (count < MX0_A02_ACTOR_NOD_START)
				{
					GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[count]);
					if (soldier)
					{
						Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_RETREAT_DONE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						if (count == 2)
						{
							Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_ENDING_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
					}
					count++;
				}
				break;
			}
		case (MX0_A02_TIMER_EAGLE_BASE):
			{
				Commands->Create_2D_Sound ("MX0_GDIEAGLEBASE_116");
				break;
			}
		case (MX0_A02_TIMER_ENTER_TANK):
			{
				if ((havoc_entered_tank < 3) && (!entered_tank))
				{
					switch (havoc_entered_tank)
					{
					case (0):
						{
							GameObject * soldier = Commands->Find_Object (engineer_02_id);
							if (soldier)
							{
								Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_TANKSAY_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
							break;
						}
					case (1):
						{
							GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[1]);
							if (soldier)
							{
								Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_TANKSAY_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
							break;
						}
					case (2):
						{
							GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[2]);
							if (soldier)
							{
								Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_TANKSAY_03, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
							break;
						}
					default:
						{
							break;
						}
					}
					havoc_entered_tank++;
					Commands->Start_Timer (obj, this, 5.0f, MX0_A02_TIMER_ENTER_TANK);
				}
				break;
			}
		case (MX0_A02_TIMER_RANDOM_EXPLOSIONS):
			{
				if (!retreat_sequence)
				{
					GameObject * location = Commands->Find_Object (MX0_A02_MOVE_OBJ_SNIPER_01);
					int choice = Get_Int_Random (1, 16);
					switch (choice)
					{
					case (1):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_01);
							break;
						}
					case (2):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_02);
							break;
						}
					case (3):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_03);
							break;
						}
					case (4):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_04);
							break;
						}
					case (5):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_05);
							break;
						}
					case (6):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_06);
							break;
						}
					case (7):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_07);
							break;
						}
					case (8):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_08);
							break;
						}
					case (9):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_09);
							break;
						}
					case (10):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_10);
							break;
						}
					case (11):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_11);
							break;
						}
					case (12):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_12);
							break;
						}
					case (13):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_13);
							break;
						}
					case (14):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_14);
							break;
						}
					case (15):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_15);
							break;
						}
					case (16):
						{
							location = Commands->Find_Object (MX0_A02_MOVE_OBJ_16);
							break;
						}
					default:
						{
							break;
						}
					}
					if (location)
					{
						Vector3 location_pos = Commands->Get_Position (location);
						location_pos.X += float(Get_Int_Random(-2,2));
						location_pos.Y += float(Get_Int_Random(-2,2));
						Commands->Create_Explosion ("Explosion_MX0", location_pos, obj);
						Commands->Start_Timer (obj, this, float(Get_Int_Random(3,6)), MX0_A02_TIMER_RANDOM_EXPLOSIONS);
					}
				}
				break;
			}
		case (MX0_A02_TIMER_DESTROY_RUBBLE):
			{
				GameObject * blockage = Commands->Find_Object (MX0_A02_BLOCKAGE_ID);
				if (blockage)
				{
					Commands->Destroy_Object (blockage);
				}
				break;
			}
		case (MX0_A02_TIMER_WRONG_WAY):
			{
				wrong_way_count++;
				if (wrong_way_count > 4)
				{
					wrong_way_count = 2;
				}
				GameObject * soldier = Commands->Find_Object (MX0_A02_UNIT_ID[wrong_way_count]);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MX0_A02_CUSTOM_TYPE_SAY_WRONG_WAY, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				Commands->Start_Timer (obj, this, 5.0f, MX0_A02_TIMER_WRONG_WAY);
			}
		default:
			{
				break;
			}
		}
	}

	void MX0_A02_Fill_Empty_Slot (GameObject * obj, bool current_target)
	{
		int count = MX0_A02_ACTOR_NOD_START;
		while (count < MX0_A02_ACTOR_HEADCOUNT)
		{
			if (!MX0_A02_UNIT_ID[count])
			{
				MX0_A02_UNIT_ID[count] = Commands->Get_ID (obj);
				if (current_target)
				{
					current_nod_target = count;
				}
				else
				{
					current_nod_target = 0;
				}
				Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_REGISTER_ACTOR_ID, count, 0.1f);
				count = MX0_A02_ACTOR_HEADCOUNT;
			}
			count++;
		}
	}
};


DECLARE_SCRIPT (MX0_A02_ACTOR, "ActorID=0:int")
{
	bool can_damage;
	bool default_state;
	bool active_actor;
	bool death_animation;
	float start_health;
	float last_armor;
	int my_sniper_target;
	int my_register_id;
	int MX0_A02_NOD_SNIPER_01;
	int MX0_A02_NOD_SNIPER_02;
	int medtank_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (can_damage, 1);
		SAVE_VARIABLE (start_health, 2);
		SAVE_VARIABLE (my_sniper_target, 3);
		SAVE_VARIABLE (default_state, 4);
		SAVE_VARIABLE (my_register_id, 5);
		SAVE_VARIABLE (last_armor, 6);
		SAVE_VARIABLE (MX0_A02_NOD_SNIPER_01, 7);
		SAVE_VARIABLE (MX0_A02_NOD_SNIPER_02, 8);
		SAVE_VARIABLE (active_actor, 9);
		SAVE_VARIABLE (death_animation, 10);
		SAVE_VARIABLE (medtank_id, 11);
	}

	// On Created, place unit in Default State
	// Random crouch walk or run between two points and fire at targets
	
	void Created (GameObject * obj) override
	{
		Commands->Attach_Script (obj, "M00_Soldier_Powerup_Disable", "");
		active_actor = true;
		start_health = Commands->Get_Health (obj);
		last_armor = Commands->Get_Shield_Strength (obj);
		Commands->Innate_Disable (obj);
		my_sniper_target = 0;
		my_register_id = 0;
		can_damage = false;
		default_state = false;
		death_animation = false;
		MX0_A02_NOD_SNIPER_01 = 0;
		MX0_A02_NOD_SNIPER_02 = 0;
		medtank_id = 0;

		Commands->Enable_Enemy_Seen (obj, false);

		int actor_id = Get_Int_Parameter("ActorID");
		if (!actor_id)
		{
			Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
		}
		else
		{
			switch (actor_id)
			{
			case (1): // First falling Nod guy moves into position.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NODFALL01_MOVE);
					params.Set_Movement(Vector3(-79.172f,-72.536f,6.0f), RUN, 0.5f, false);
					Commands->Action_Goto (obj, params);
					Commands->Enable_Enemy_Seen (obj, true);
					break;
				}
			case (2): // Second falling Nod guy moves into position.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NODFALL02_MOVE);
					params.Set_Movement(Vector3(-97.415f,-68.822f,3.944f), RUN, 0.5f, false);
					Commands->Action_Goto (obj, params);
					Commands->Enable_Enemy_Seen (obj, true);
					break;
				}
			case (3): // Nod Rocket Soldier shoots APC.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_MOVETO_APC);
					params.Set_Movement(Vector3(-79.172f,-72.536f,6.0f), RUN, 0.5f, false);
					Commands->Action_Goto (obj, params);
					break;
				}
			case (4): // Nod soldiers that get dropped from the helicopter 01.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_HELI_01_MOVE);
					params.Set_Movement(Vector3(0,0,0), RUN, 0.5f, false);
					params.WaypathID = MX0_A02_WAYPATH_01;
					Commands->Action_Goto (obj, params);
					can_damage = true;
					Commands->Start_Timer (obj, this, 20.0f, MX0_A02_TIMER_DESTROY_MINIGUNNER);
					break;
				}
			case (5): // Nod FlameThrower jumps down and fires at targets until dead.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_HELI_02_MOVE);
					params.Set_Attack(STAR, 300.0f, 0.0f, true);
					params.Set_Movement(Vector3(0,0,0), RUN, 0.5f, false);
					params.WaypathID = MX0_A02_WAYPATH_02;
					params.AttackCheckBlocked = false;
					params.AttackForceFire = true;
					Commands->Action_Attack (obj, params);
					can_damage = true;
					Commands->Start_Timer (obj, this, 10.0f, MX0_A02_TIMER_DESTROY_FLAME);
					Commands->Trigger_Weapon (obj, true, Vector3(-90.941f,-60.470f,0.069f), true);
					GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					break;
				}
			case (6): // Nod FlameThrower explodes shortly after creation, before ending rappel.
				{
					can_damage = true;
					Commands->Start_Timer (obj, this, 2.0f, MX0_A02_TIMER_DESTROY_FLAME);
					break;
				}
			case (7): // Engineer 01 moves to repair the tank.
				{
					Commands->Start_Timer (obj, this, 4.0f, MX0_A02_TIMER_WAITFOR_MOVETOTANK);
					break;
				}
			case (8): // Engineer 02 moves to destroy the rubble.
				{
					Commands->Start_Timer (obj, this, 4.0f, MX0_A02_TIMER_WAITFOR_MOVETORUBBLE);
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		if (active_actor)
		{
			if (!can_damage)
			{
				if (default_state)
				{
					last_armor -= 1.0f;
				}
				Commands->Set_Shield_Strength (obj, last_armor);
				Commands->Set_Health (obj, start_health);
			}
			else
			{
				last_armor = Commands->Get_Shield_Strength (obj);
			}
			if (default_state)
			{
				if (damager)
				{
					if (my_register_id < MX0_A02_ACTOR_NOD_START)
					{
						if (damager != STAR)
						{
							int	my_type = Commands->Get_Player_Type (obj);
							int their_type = Commands->Get_Player_Type (damager);
							if (my_type != their_type)
							{
								ActionParamsStruct params;
								params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
								params.Set_Attack(damager, 300.0f, 0.0f, true);
								if (my_register_id >= MX0_A02_ACTOR_NOD_START)
								{
									params.AttackCheckBlocked = false;
								}
								Commands->Action_Attack (obj, params);
							}
						}
					}
					else
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
						params.Set_Attack(damager, 300.0f, 0.0f, true);
						if (my_register_id >= MX0_A02_ACTOR_NOD_START)
						{
							params.AttackCheckBlocked = false;
						}
						Commands->Action_Attack (obj, params);
					}
				}
			}
			else
			{
				if ((!death_animation) && (can_damage))
				{
					death_animation = true;
					// Check here for any special death animations that need to be played.

					int my_id = Get_Int_Parameter ("ActorID");
					switch (my_id)
					{
					case (1):
						{
							//DEBUG insert falling death animation here. Animation must kill unit at completion.
							Commands->Debug_Message ("XXXX INSERT FALLING DEATH ANIMATION HERE.\n");
							//DEBUG also, play a sound effect for the falling scream.
							Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", obj);
							GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
							if (controller)
							{
								Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_RESPONDS_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
							break;
						}
					case (2):
						{
							//DEBUG insert falling death animation here. Animation must kill unit at completion.
							Commands->Debug_Message ("XXXX INSERT FALLING DEATH ANIMATION HERE.\n");
							//DEBUG also, play a sound effect for the falling scream.
							Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", obj);

							// Response from here is action complete send to destroy APC.
							break;
						}
					default:
						{
							break;
						}
					}
				}
			}
		}
	}

	void Killed (GameObject * obj, GameObject * killer) override
	{
		if (active_actor)
		{
			GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
			if (controller)
			{
				if (killer == STAR)
				{
					Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_PLAYER_KILLED_NOD, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
				}
				else
				{
					if (killer)
					{
						int killer_id = Commands->Get_ID (killer);
						Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_SOLDIER_KILLED_NOD, killer_id, 0.0f);
					}
				}
				if (my_register_id >= MX0_A02_ACTOR_NOD_START)
				{
					Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_NOD_SOLDIER_KILLED, my_register_id, 0.1f);
				}
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (active_actor)
		{
			switch (type)
			{
			case (MX0_A02_CUSTOM_TYPE_MAIN_ENDING):
				{
					active_actor = false;
					default_state = false;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_REGISTER_ACTOR_ID):
				{
					my_register_id = param;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_DAMAGE_ON):
				{
					can_damage = true;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_DAMAGE_OFF):
				{
					can_damage = false;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON):
				{
					Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
					default_state = true;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_OFF):
				{
					default_state = false;
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_REQUEST_TARGET):
				{
					if (default_state)
					{
						GameObject * target = Commands->Find_Object (param);
						if (target)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
							params.Set_Attack(target, 300.0f, 0.0f, true);
							params.MoveCrouched = true;
							params.AttackCrouched = true;
							if (my_register_id >= MX0_A02_ACTOR_NOD_START)
							{
								params.AttackCheckBlocked = false;
							}
							Commands->Action_Attack (obj, params);
						}
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_PRE_AMBIENT):
				{
					MX0_A02_Say_Something (obj, param, false);
					break;
				}
			case (MX0_A02_CUSTOM_KILL_SNIPER_01):
				{
					default_state = false;
					GameObject * sniper = Commands->Find_Object (param);
					if (sniper)
					{
						MX0_A02_NOD_SNIPER_01 = Commands->Get_ID (sniper);
						GameObject * location = Commands->Find_Object (MX0_A02_MOVE_OBJ_SNIPER_02);
						if (location)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ROCKET_SNIPER_MOVE);
							params.Set_Movement(Commands->Get_Position (location), RUN, 0.5f, false);
							Commands->Action_Goto (obj, params);
						}
						else
						{
							GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
							if (controller)
							{
								Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_02_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
						}
					}
					else
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_02_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
					}
					break;
				}
			case (MX0_A02_CUSTOM_KILL_SNIPER_02):
				{
					default_state = false;
					GameObject * sniper = Commands->Find_Object (param);
					if (sniper)
					{
						MX0_A02_NOD_SNIPER_02 = Commands->Get_ID (sniper);
						GameObject * location = Commands->Find_Object (MX0_A02_MOVE_OBJ_SNIPER_02);
						if (location)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ROCKET_SNIPER_MOVE_02);
							params.Set_Movement(Commands->Get_Position (location), RUN, 0.5f, false);
							Commands->Action_Goto (obj, params);
						}
						else
						{
							Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
					}
					else
					{
						Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_START_GDI_01):
				{
					Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_SOLDIER_02_START);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_START_GDI_03):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_01_STARTUP_02, true);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_START_GDI_04):
				{
					default_state = false;
					Vector3 my_loc = Commands->Get_Position (obj);
					Vector3 star_loc = Commands->Get_Position (STAR);
					float distance = Commands->Get_Distance (my_loc, star_loc);
					if (distance > MX0_A02_APPROACH_DISTANCE)
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_APPROACH_HAVOC_01);
						params.Set_Movement(STAR, RUN, 3.0f, false);
						Commands->Action_Goto (obj, params);
					}
					else
					{
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_02_BECKON, false);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_START_GDI_05):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_03_STARTUP, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_PLAYER_CONGRATS):
				{
					if (default_state)
					{
						int speech_num = MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_01 + param;
						MX0_A02_Say_Something (obj, speech_num, false);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SOLDIER_CONGRATS):
				{
					if (default_state)
					{
						int speech_num = MX0_A02_SPEECH_GDI_SOLDIER_CONGRATS_01 + param;
						MX0_A02_Say_Something (obj, speech_num, false);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_PLAYER_RETREAT):
				{
					int speech_num = MX0_A02_SPEECH_GDI_PLAYER_RETREAT_01 + my_register_id - 1;
					MX0_A02_Say_Something (obj, speech_num, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_NOD_01):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_NOTICES_NOD_01, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_01):
				{
					GameObject * target = Commands->Find_Object (param);
					if (target)
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_GDI_SHOOTS_NOD_01);
						params.Set_Attack(target, 300.0f, 0.0f, true);
						Commands->Action_Attack (obj, params);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_RESPONDS_01):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_RESPONDS_01, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_02):
				{
					GameObject * target = Commands->Find_Object (param);
					if (target)
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_GDI_SHOOTS_NOD_02);
						params.Set_Attack(target, 300.0f, 0.0f, true);
						Commands->Action_Attack (obj, params);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GET_APC_ID):
				{
					GameObject * apc = Commands->Find_Object (param);
					if (apc)
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_APC_BLOWITUP, MX0_A02_CUSTOM_PARAM_DEFAULT, 1.0f);
						}
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_SHOOT_APC);
						params.Set_Attack(apc, 300.0f, 0.0f, true);
						Commands->Action_Attack (obj, params);
					}
					else
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_DONEWITH_APC);
						params.Set_Movement(Vector3(-78.442f,-78.044f,6.404f), RUN, 0.5f, false);
						Commands->Action_Goto (obj, params);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_HELI_DESTROYED):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_SPOTS_HELI_DESTROY, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_HELI_01):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_SPOTS_HELI_01, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_HELI_02):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_SPOTS_HELI_02, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_NOTICES_FLAME, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME_DEAD):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_NOTICES_FLAME_DEAD, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_HELI_DESTROYED_02):
				{
					if (my_register_id == 1)
					{
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_SPOTS_HELI_DESTROY, false);
					}
					else
					{
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_NOD_HELI_DESTROYED_02, false);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_RETREAT_SEQUENCE):
				{
					GameObject * moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_16);
					switch (my_register_id)
					{
					case (5):
						{
							moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_09);
							break;
						}
					case (6):
						{
							moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_11);
							break;
						}
					case (7):
						{
							moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_13);
							break;
						}
					case (8):
						{
							moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_15);
							break;
						}
					default:
						{
							break;
						}
					}
					if (moveloc)
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_RETREAT_MOVE);
						params.Set_Movement(Commands->Get_Position (moveloc), RUN, 0.5f, false);
						Commands->Action_Goto (obj, params);
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_RETREAT_DONE):
				{
					Vector3 move_location = Vector3(-105.302f,-55.383f,0.0f);
					switch (my_register_id)
					{
					case (1):
						{
							move_location = Vector3(-78.74f,-60.34f,0.0f);
							break;
						}
					case (2):
						{
							move_location = Vector3(-90.786f,-65.492f,0.0f);
							break;
						}
					case (3):
						{
							move_location = Vector3(-94.756f,-59.510f,0.0f);
							break;
						}
					case (4):
						{
							move_location = Vector3(-104.989f,-49.282f,0.0f);
							break;
						}
					default:
						{
							break;
						}
					}
					default_state = false;
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
					params.Set_Movement(move_location, 0.2f, 3.0f, false);
					Commands->Action_Goto (obj, params);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENDING_01):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENDING_01, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENDING_02):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENDING_02, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENDING_03):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENDING_03, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENDING_04):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENDING_04, true);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENDING_05):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENDING_05, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_ENGINEER_MOVETOTANK):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENGINEER_TANK_01, false);
					GameObject * tank = Commands->Find_Object (param);
					if (tank)
					{
						medtank_id = param;
						GameObject * moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_06);
						if (moveloc)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ENGINEER_01_MEDTANK);
							params.Set_Movement(Commands->Get_Position(moveloc), RUN, 3.0f, false);
							Commands->Action_Goto (obj, params);
						}
					}
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SAY_TANK):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_UNSTOPPABLE, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_TANKSAY_01):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_TANKSAY_01, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_TANKSAY_02):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_TANKSAY_02, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_TANKSAY_03):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_TANKSAY_03, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SAY_FIREHOLE):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_TANKSAY_04, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_FOLLOW_HAVOC):
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
					params.Set_Movement(STAR, RUN, 10.0f, false);
					Commands->Action_Goto (obj, params);
					//HANDOFF MY ID TO THE NEW CONTROLLER HERE!
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SAY_APC_DOWN):
				{
					MX0_A02_Say_Something (obj, MX0_A02_SPEECH_APC_DOWN, false);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SHOOT_HELI_02):
				{
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
					params.Set_Attack(Vector3(-85.66f,-73.31f,22.81f), 300.0f, 0.0f, true);
					params.AttackCheckBlocked = false;
					Commands->Action_Attack (obj, params);
					break;
				}
			case (MX0_A02_CUSTOM_TYPE_SAY_WRONG_WAY):
				{
					Vector3 my_loc = Commands->Get_Position (obj);
					Vector3 star_loc = Commands->Get_Position (STAR);
					float distance = Commands->Get_Distance (my_loc, star_loc);
					if (distance < 10.0f)
					{
						switch (my_register_id)
						{
						case (2):
							{
								MX0_A02_Say_Something (obj, MX0_A02_SPEECH_WRONGWAY_01, true);
								break;
							}
						case (3):
							{
								MX0_A02_Say_Something (obj, MX0_A02_SPEECH_WRONGWAY_02, true);
								break;
							}
						case (4):
							{
								MX0_A02_Say_Something (obj, MX0_A02_SPEECH_WRONGWAY_03, true);
								break;
							}
						default:
							{
								break;
							}
						}
					}
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (active_actor)
		{
			if ((complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)	|| (complete_reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED) || (complete_reason == ACTION_COMPLETE_CONVERSATION_UNABLE_TO_INIT))
			{
				switch (action_id)
				{
				case (MX0_A02_SPEECH_GDI_01_STARTUP):
					{
						default_state = true;
						Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_START_GDI_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_02_STARTUP):
					{
						default_state = true;
						Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_START_GDI_04, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_01_STARTUP_02):
					{
						default_state = true;
						Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_START_GDI_05, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_02_BECKON):
					{
						default_state = true;
						Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_START_GDI_03, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_03_STARTUP):
					{
						default_state = true;
						Commands->Start_Timer (obj, this, 0.1f, MX0_A02_TIMER_BASIC_MOVE_02);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_CONTINUE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_NOTICES_NOD_01):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENDING_01):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENDING_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENDING_02):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENDING_03, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENDING_03):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENDING_04, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENDING_04):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENDING_05, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENDING_05):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENDING_06, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_ENGINEER_DONE):
					{
						Commands->Create_2D_Sound ("MX0_GDIEAGLEBASE_117");
						Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0397I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENGINEER_DONE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_TANKSAY_04):
					{
						GameObject * location = Commands->Find_Object (MX0_A02_MOVE_OBJ_SNIPER_02);
						if (location)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ENGINEER_RETREAT);
							params.Set_Movement(Commands->Get_Position (location), RUN, 5.0f, false);
							Commands->Action_Goto (obj, params);
						}
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_EXPLODE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_SPEECH_GDI_SPOTS_HELI_02):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_SHOOT_HELI_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				default:
					{
						break;
					}
				}
			}
			else
			{
				switch (action_id)
				{
				case (MX0_A02_ACTION_ROCKET_SNIPER_MOVE):
					{
						GameObject * sniper = Commands->Find_Object (MX0_A02_NOD_SNIPER_01);
						if (sniper)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ROCKET_SNIPER_SHOOT);
							params.Set_Attack(sniper, 300.0f, 0.0f, true);
							params.AttackCheckBlocked = false;
							Commands->Action_Attack (obj, params);
						}
						else
						{
							GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
							if (controller)
							{
								Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_02_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
							}
						}
						break;
					}
				case (MX0_A02_ACTION_ROCKET_SNIPER_MOVE_02):
					{
						GameObject * sniper = Commands->Find_Object (MX0_A02_NOD_SNIPER_02);
						if (sniper)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ROCKET_SNIPER_SHOOT_02);
							params.Set_Attack(sniper, 300.0f, 0.0f, true);
							params.AttackCheckBlocked = false;
							Commands->Action_Attack (obj, params);
						}
						else
						{
							Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_ROCKET_SNIPER_SHOOT):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HANDOFF_SNIPER_02_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_ROCKET_SNIPER_SHOOT_02):
					{
						Commands->Send_Custom_Event (obj, obj, MX0_A02_CUSTOM_TYPE_DEFAULT_STATE_ON, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						break;
					}
				case (MX0_A02_ACTION_BASIC_MOVE_01):
				case (MX0_A02_ACTION_BASIC_MOVE_02):
					{
						if (default_state)
						{
							GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
							if (controller)
							{
								Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_REQUEST_TARGET, my_register_id, 0.0f);
							}
						}
						break;
					}
				case (MX0_A02_ACTION_APPROACH_HAVOC_01):
					{
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_02_BECKON, true);
						break;
					}
				case (MX0_A02_ACTION_NODFALL01_MOVE):
					{
						can_damage = true;
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_NOD_01, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_NODFALL02_MOVE):
					{
						can_damage = true;
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_SHOOTS_NOD_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_NOD_MOVETO_APC):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GET_APC_ID, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_NOD_SHOOT_APC):
					{
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_SAY_APC_DOWN, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_NOD_DONEWITH_APC);
						params.Set_Movement(Vector3(-78.442f,-78.044f,6.404f), RUN, 0.5f, false);
						Commands->Action_Goto (obj, params);
						break;
					}
				case (MX0_A02_ACTION_NOD_DONEWITH_APC):
					{
						Commands->Destroy_Object (obj);
						GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_NEXT_SEQUENCE, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MX0_A02_ACTION_NOD_HELI_01_MOVE):
					{
						Commands->Enable_Enemy_Seen (obj, true);
						break;
					}
				case (MX0_A02_ACTION_NOD_HELI_02_MOVE):
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
						params.Set_Attack(STAR, 30.0f, 0.0f, true);
						params.Set_Movement (STAR, RUN, 1.0f, false);
						Commands->Action_Attack (obj, params);
						break;
					}
				case (MX0_A02_ACTION_RETREAT_MOVE):
					{
						Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
						break;
					}
				case (MX0_A02_ACTION_ENGINEER_01_MEDTANK):
					{
						GameObject * tank = Commands->Find_Object (medtank_id);
						if (tank)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
							params.Set_Attack(tank, 300.0f, 0.0f, false);
							params.AttackCheckBlocked = false;
							Commands->Action_Attack (obj, params);
							Commands->Start_Timer (obj, this, 5.0f, MX0_A02_TIMER_ENGINEER_FIX_TANK);
						}
						break;
					}
				case (MX0_A02_ACTION_ENGINEER_02_RUBBLE):
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
						params.Set_Attack(Vector3(-75.0f,-43.4f,0.1f), 0.0f, 0.0f, true);
						params.MoveCrouched = true;
						params.AttackCrouched = true;
						Commands->Action_Attack (obj, params);
						break;
					}
				default:
					{
						break;
					}
				}
			}
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
		params.Set_Attack(enemy, 300.0f, 0.0f, true);
		params.MoveCrouched = true;
		params.AttackCrouched = true;
		Commands->Action_Attack (obj, params);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (active_actor)
		{
			switch (timer_id)
			{
			case (MX0_A02_TIMER_BASIC_MOVE_01):
				{
					if (default_state)
					{
						GameObject * moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_16);
						switch (my_register_id)
						{
						case (1):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_02);
								break;
							}
						case (2):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_04);						
								break;
							}
						case (3):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_06);
								break;
							}
						case (4):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_08);
								break;
							}
						case (5):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_10);
								break;
							}
						case (6):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_12);
								break;
							}
						case (7):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_14);
								break;
							}
						case (8):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_16);
								break;
							}
						default:
							{
								break;
							}
						}
						if (moveloc)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_BASIC_MOVE_01);
							params.Set_Movement(Commands->Get_Position (moveloc), RUN, 0.5f, false);
							Commands->Action_Goto (obj, params);
						}
						Commands->Start_Timer (obj, this, (MX0_A02_TIMERLENGTH_BASIC_MOVE + Get_Int_Random(0, MX0_A02_TIMERLENGTH_BASIC_MOVE)), MX0_A02_TIMER_BASIC_MOVE_02);
					}
					break;
				}
			case (MX0_A02_TIMER_BASIC_MOVE_02):
				{
					if (default_state)
					{
						GameObject * moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_16);
						switch (my_register_id)
						{
						case (1):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_01);
								break;
							}
						case (2):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_03);						
								break;
							}
						case (3):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_05);
								break;
							}
						case (4):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_07);
								break;
							}
						case (5):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_09);
								break;
							}
						case (6):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_11);
								break;
							}
						case (7):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_13);
								break;
							}
						case (8):
							{
								moveloc = Commands->Find_Object (MX0_A02_MOVE_OBJ_15);
								break;
							}
						default:
							{
								break;
							}
						}
						if (moveloc)
						{
							ActionParamsStruct params;
							params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_BASIC_MOVE_02);
							params.Set_Movement(Commands->Get_Position (moveloc), RUN, 0.5f, false);
							Commands->Action_Goto (obj, params);
						}
						Commands->Start_Timer (obj, this, (MX0_A02_TIMERLENGTH_BASIC_MOVE + Get_Int_Random(0, MX0_A02_TIMERLENGTH_BASIC_MOVE)), MX0_A02_TIMER_BASIC_MOVE_01);
					}
					break;
				}
			case (MX0_A02_TIMER_SOLDIER_01_START):
				{
					Vector3 my_location = Commands->Get_Position (obj);
					Vector3 star_location = Commands->Get_Position (STAR);
					float distance = Commands->Get_Distance (my_location, star_location);
					if (distance < MX0_A02_APPROACH_DISTANCE)
					{
						default_state = false;
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_01_STARTUP, true);
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_SOLDIER_01_START);
					}
					break;
				}
			case(MX0_A02_TIMER_SOLDIER_02_START):
				{
					Vector3 my_location = Commands->Get_Position (obj);
					Vector3 star_location = Commands->Get_Position (STAR);
					float distance = Commands->Get_Distance (my_location, star_location);
					if (distance < MX0_A02_APPROACH_DISTANCE)
					{
						default_state = false;
						MX0_A02_Say_Something (obj, MX0_A02_SPEECH_GDI_02_STARTUP, true);
					}
					else
					{
						ActionParamsStruct params;
						params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_DEFAULT);
						params.Set_Movement(star_location, RUN, 0.5f, false);
						Commands->Action_Goto (obj, params);
						Commands->Start_Timer (obj, this, 1.0f, MX0_A02_TIMER_SOLDIER_02_START);
					}
					break;
				}
			case (MX0_A02_TIMER_DESTROY_FLAME):
				{
					GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GDI_NOTICES_FLAME_DEAD, MX0_A02_CUSTOM_PARAM_DEFAULT, 1.0f);
					}
					Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", obj);
					break;
				}
			case (MX0_A02_TIMER_DESTROY_MINIGUNNER):
				{
					Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", obj);
					break;
				}
			case (MX0_A02_TIMER_ENGINEER_FIX_TANK):
				{
					GameObject * tank = Commands->Find_Object (medtank_id);
					if (tank)
					{
						Vector3 tank_pos = Commands->Get_Position (tank);
						float tank_facing = Commands->Get_Facing (tank);
						Commands->Destroy_Object (tank);
						GameObject * newtank = Commands->Create_Object ("GDI_Medium_Tank_Player", tank_pos);
						if (newtank)
						{
							Commands->Attach_Script( obj, "M00_Send_Object_ID", "1400041,13,0.0f");
							Commands->Set_Facing (newtank, tank_facing);
							MX0_A02_Say_Something (obj, MX0_A02_SPEECH_ENGINEER_DONE, true);
							Commands->Attach_Script (newtank, "MX0_A02_GDI_MEDTANK", "");
						}
					}
					break;
				}
			case (MX0_A02_TIMER_WAITFOR_MOVETOTANK):
				{
					GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENGINEER_MOVETOTANK, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					break;
				}
			case (MX0_A02_TIMER_WAITFOR_MOVETORUBBLE):
				{
					GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENGINEER_02_REGISTER, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
					}
					ActionParamsStruct params;
					params.Set_Basic(this, MX0_A02_PRIORITY_FORCED_ACTION, MX0_A02_ACTION_ENGINEER_02_RUBBLE);
					params.Set_Movement(Vector3(-76.3f,-45.0f,0.1f), RUN, 1.0f, false);
					Commands->Action_Goto (obj, params);
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void MX0_A02_Say_Something (GameObject * obj, int speech_id, bool face_target)
	{
		if (active_actor)
		{
			int priority = 100;
			const char *conv_name = ("MX0_A02_PREAMB_01"); //DEFAULT
			switch (speech_id)
			{
			case (1):
				{
					conv_name = ("MX0_GDITROOPER2_051");
					break;
				}
			case (2):
				{
					conv_name = ("MX0_GDITROOPER3_053");
					break;
				}
			case (3):
				{
					conv_name = ("MX0_GDITROOPER4_054");
					break;
				}
			case (4):
				{
					conv_name = ("MX0_GDITROOPER2_056");
					break;
				}
			case (5):
				{
					conv_name = ("MX0_GDITROOPER1_057");
					break;
				}
			case (6):
				{
					conv_name = ("MX0_GDITROOPER4_058");
					break;
				}
			case (7):
				{
					conv_name = ("MX0_GDITROOPER4_060");
					break;
				}
			case (8):
				{
					conv_name = ("MX0_GDITROOPER2_062");
					break;
				}
			case (9):
				{
					conv_name = ("MX0_GDITROOPER1_063");
					break;
				}
			case (10):
				{
					conv_name = ("MX0_GDITROOPER3_064");
					break;
				}
			case (11):
				{
					conv_name = ("MX0_GDITROOPER4_065");
					break;
				}
			case (MX0_A02_SPEECH_GDI_02_BECKON):
				{
					conv_name = ("MX0_GDITROOPER2_069"); //DEBUG "Captain Havoc! This way sir!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_01_STARTUP):
				{
					conv_name = ("MX0_GDITROOPER1_066"); //DEBUG "Reinforcements? There's just one guy!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_02_STARTUP):
				{
					conv_name = ("MX0_GDITROOPER2_067"); //DEBUG "Hey, it's Havoc!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_01_STARTUP_02):
				{
					conv_name = ("MX0_GDITROOPER1_068"); //DEBUG "Are we glad to see you!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_03_STARTUP):
				{
					priority = 99;
					conv_name = ("MX0_GDITROOPER3_070"); //DEBUG "Payback time!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SOLDIER_CONGRATS_01):
				{
					conv_name = ("MX0_GDITROOPER1_071"); //DEBUG "Hey, I got one!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SOLDIER_CONGRATS_02):
				{
					conv_name = ("MX0_GDITROOPER4_072"); //DEBUG "Nice shot!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SOLDIER_CONGRATS_03):
				{
					conv_name = ("MX0_GDITROOPER3_073"); //DEBUG "There's one!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SOLDIER_CONGRATS_04):
				{
					conv_name = ("MX0_GDITROOPER2_074"); //DEBUG "Come and get it!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_01):
				{
					conv_name = ("MX0_GDITROOPER1_075"); //DEBUG "Great shot, sir!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_02):
				{
					conv_name = ("MX0_GDITROOPER4_076"); //DEBUG "You got him, Havoc!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_03):
				{
					conv_name = ("MX0_GDITROOPER2_078"); //DEBUG "Outstanding Havoc!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_04):
				{
					conv_name = ("MX0_GDITROOPER3_ALT13"); //DEBUG "Watch it!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_05):
				{
					conv_name = ("MX0_GDITROOPER1_ALT09"); //DEBUG "Yeah!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_06):
				{
					conv_name = ("MX0_GDITROOPER4_ALT07"); //DEBUG "Go!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_07):
				{
					conv_name = ("MX0_GDITROOPER3_ALT13"); //DEBUG "Good work!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_08):
				{
					conv_name = ("MX0_GDITROOPER1_ALT10"); //DEBUG "Alright!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_09):
				{
					conv_name = ("MX0_GDITROOPER2_ALT01"); //DEBUG "Nice shot!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_CONGRATS_10):
				{
					conv_name = ("MX0_GDITROOPER4_ALT06"); //DEBUG "Yeah!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_RETREAT_01):
				{
					conv_name = ("MX0_GDITROOPER1_079"); //DEBUG "Don't leave us sir!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_RETREAT_02):
				{
					conv_name = ("MX0_GDITROOPER2_082"); //DEBUG "Hey! We need your help!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_RETREAT_03):
				{
					conv_name = ("MX0_GDITROOPER3_081"); //DEBUG "Havoc! Come back!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_PLAYER_RETREAT_04):
				{
					conv_name = ("MX0_GDITROOPER4_080"); //DEBUG "Wait! Where are you going Havoc?"
					break;
				}
			case (MX0_A02_SPEECH_GDI_NOTICES_NOD_01):
				{
					conv_name = ("MX0_GDITROOPER1_086"); //DEBUG "Up there on the rocks!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_RESPONDS_01):
				{
					conv_name = ("MX0_GDITROOPER1_088"); //DEBUG "Ha! Look at that!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SPOTS_HELI_DESTROY):
				{
					conv_name = ("MX0_GDITROOPER2_093"); //DEBUG "Chopper down and out!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SPOTS_HELI_01):
				{
					conv_name = ("MX0_GDITROOPER2_092"); //DEBUG "Chopper! They're dropping more troops!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_SPOTS_HELI_02):
				{
					conv_name = ("MX0_GDITROOPER2_095"); //DEBUG "Another chopper - take it out!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_NOTICES_FLAME):
				{
					conv_name = ("MX0_GDITROOPER3_096"); //DEBUG "Flame Troopers! Watch your back!"
					break;
				}
			case (MX0_A02_SPEECH_GDI_NOTICES_FLAME_DEAD):
				{
					conv_name = ("MX0_GDITROOPER4_097"); //DEBUG "Roasted!"
					break;
				}
			case (MX0_A02_SPEECH_NOD_HELI_DESTROYED_01):
				{
					conv_name = ("MX0_GDITROOPER1_099"); //DEBUG "All Right!"
					break;
				}
			case (MX0_A02_SPEECH_NOD_HELI_DESTROYED_02):
				{
					conv_name = ("MX0_GDITROOPER2_098"); //DEBUG "Woo hoo!"
					break;
				}
			case (MX0_A02_SPEECH_ENDING_01):
				{
					conv_name = ("MX0_GDITROOPER2_105"); //DEBUG "We pushed 'em back!"
					break;
				}
			case (MX0_A02_SPEECH_ENDING_02):
				{
					conv_name = ("MX0_GDITROOPER4_106"); //DEBUG "Look! They're on the run!"
					break;
				}
			case (MX0_A02_SPEECH_ENDING_03):
				{
					conv_name = ("MX0_GDITROOPER1_107"); //DEBUG "We did it!"
					break;
				}
			case (MX0_A02_SPEECH_ENDING_04):
				{
					conv_name = ("MX0_GDITROOPER2_108"); //DEBUG "Thanks Havoc. We owe you big time."
					break;
				}
			case (MX0_A02_SPEECH_ENDING_05):
				{
					conv_name = ("MX0_GDITROOPER3_109"); //DEBUG "Recon One to Eagle Base. We've taken casualties, but we're okay."
					break;
				}
			case (MX0_A02_SPEECH_ENGINEER_TANK_01):
				{
					conv_name = ("MX0_ENGINEER1_115"); //DEBUG "Stand by for repairs."
					break;
				}
			case (MX0_A02_SPEECH_ENGINEER_DONE):
				{
					conv_name = ("MX0_ENGINEER1_118"); //DEBUG "Your tank is repaired and ready to roll, Havoc."
					break;
				}
			case (MX0_A02_SPEECH_UNSTOPPABLE):
				{
					conv_name = ("MX0_GDITROOPER1_119"); //DEBUG "You'll be unstoppable in that tank, sir!"
					break;
				}
			case (MX0_A02_SPEECH_TANKSAY_01):
				{
					conv_name = ("MX0_ENGINEER2_120"); //DEBUG "I'll clear the road once you're inside the tank, Havoc."
					break;
				}
			case (MX0_A02_SPEECH_TANKSAY_02):
				{
					conv_name = ("MX0_GDITROOPER1_121"); //DEBUG "Your tank's ready and waiting, sir!"
					break;
				}
			case (MX0_A02_SPEECH_TANKSAY_03):
				{
					conv_name = ("MX0_GDITROOPER2_122"); //DEBUG "The tank's yours, Havoc! Rock and roll!"
					break;
				}
			case (MX0_A02_SPEECH_TANKSAY_04):
				{
					conv_name = ("MX0_ENGINEER2_123"); //DEBUG "C4's set... fire in the hole!"
					break;
				}
			case (MX0_A02_SPEECH_APC_DOWN):
				{
					conv_name = ("MX0_GDITROOPER3_090"); //DEBUG "Argh! We lost our APC!"
					break;
				}
			case (MX0_A02_SPEECH_WRONGWAY_01):
				{
					conv_name = ("MX0_A02_SPEECH_WRONGWAY_01");
					break;
				}
			case (MX0_A02_SPEECH_WRONGWAY_02):
				{
					conv_name = ("MX0_A02_SPEECH_WRONGWAY_02");
					break;
				}
			case (MX0_A02_SPEECH_WRONGWAY_03):
				{
					conv_name = ("MX0_A02_SPEECH_WRONGWAY_03");
					break;
				}
			default:
				{
					break;
				}
			}
			int conversation = Commands->Create_Conversation (conv_name, priority, 300.0f, false);
			if (face_target)
			{
				Commands->Join_Conversation (obj, conversation, false, true, true);
			}
			else
			{
				Commands->Join_Conversation (obj, conversation, false, true, true);
			}
			Commands->Join_Conversation(STAR, conversation, false, false, false);
			Commands->Start_Conversation (conversation, speech_id);
			Commands->Monitor_Conversation (obj, conversation);
		}
	}
};


// This script is for the vehicles to prevent them from taking any further damage.

DECLARE_SCRIPT (MX0_A02_GDI_VEHICLE, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Set_Health (obj, 10.0f);
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health (obj, 10.0f);
	}
};


// This script is for the GDI Medium Tank. It is a trigger for entry that tells the controller.

DECLARE_SCRIPT (MX0_A02_GDI_MEDTANK, "")
{
	bool entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (entered, 1);
	}
	
	void Created (GameObject * /*obj*/) override
	{
		entered = false;
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health (obj, Commands->Get_Max_Health (obj));
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ((type == CUSTOM_EVENT_VEHICLE_ENTERED) && (!entered))
		{
			entered = true;
			Commands->Fade_Background_Music( "Level 0 Tiberium.mp3", 2, 2);
			GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_ENTERED_TANK, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
			}
		}
	}
};


// This script is for the GDI APC, which cannot take damage until it is to be destroyed.

DECLARE_SCRIPT (MX0_A02_GDI_APC, "")
{
	bool can_damage;

	void Created (GameObject * /*obj*/) override
	{
		can_damage = false;
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		if (!can_damage)
		{
			Commands->Set_Health (obj, Commands->Get_Max_Health (obj));
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, MX0_A02_PRIORITY_DEFAULT_SHOOT, MX0_A02_ACTION_DEFAULT);
		params.Set_Attack(enemy, 300.0f, 0.0f, true);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack (obj, params);
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * sender) override
	{
		if (type == MX0_A02_CUSTOM_TYPE_DESTROY_APC)
		{
			can_damage = true;
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", sender);
		}
		else if (type == MX0_A02_CUSTOM_TYPE_STARTUP)
		{
			Commands->Innate_Enable (obj);
			Commands->Enable_Enemy_Seen (obj, true);
		}
	}
};


// This script is for the trigger zone that activates the entire sequence.

DECLARE_SCRIPT (MX0_A02_ZONE_STARTUP, "")
{
	void Entered (GameObject * obj, GameObject * /*enterer*/) override
	{
		GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
		if (controller)
		{
			Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_STARTUP, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
			Commands->Destroy_Object (obj);
		}
	}
};


// This script is for the helicopters to send a custom to the controller when they are destroyed.

DECLARE_SCRIPT (MX0_A02_HELICOPTER, "ActorID=0:int")
{
	void Created (GameObject * obj) override
	{
		int actor_id = Get_Int_Parameter ("ActorID");
		if (actor_id)
		{
			Commands->Start_Timer (obj, this, 23.0f, MX0_A02_TIMER_DESTROY_HELI_02);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		int actor_id = Get_Int_Parameter ("ActorID");
		if (!actor_id)
		{
			GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HELI_DESTROYED, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
			}
		}
		else
		{
			GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_HELI_DESTROYED_02, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == MX0_A02_TIMER_DESTROY_HELI_02)
		{
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
		}
	}
};


// This is a default script for testing. It shuts off a unit.

DECLARE_SCRIPT (MX0_A02_DEFAULT_OFF, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Innate_Disable (obj);
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		if (damager != STAR)
		{
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", obj);
		}
	}
};

DECLARE_SCRIPT (MX0_A02_ZONE_DEFAULT_ON, "")
{
	void Entered (GameObject * obj, GameObject * /*enterer*/) override
	{
		GameObject * controller = Commands->Find_Object (MX0_A02_CONTROLLER_ID);
		if (controller)
		{
			Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_MAIN_STARTUP, MX0_A02_CUSTOM_PARAM_DEFAULT, 0.0f);
			Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GET_SNIPER_01_ID, 1200017, 0.0f);
			Commands->Send_Custom_Event (obj, controller, MX0_A02_CUSTOM_TYPE_GET_SNIPER_02_ID, 1200023, 0.0f);
		}
	}
};

/////////////////////////////////////


/* Evacuation System

  This system spawns soldiers near the base area that run to the dock and disappear behind
  the concrete bunker at the end. They are evacuating the base.

  1. When the commando passes the start zone, three soldiers are spawned with random time
  delays in one of two sets of coordinates. The coordinates used depend on which reinforcement
  swap zone the commando entered last. This helps insure that the commando cannot see the
  spawns as they take place. The soldiers run along one of three waypaths, with a random action
  priority that will make some attack and others not.

  2. If a soldier is killed, he spawns again. If a soldier makes it to the dock, he destroys
  himself and spawns again.

*/

const Vector3 M03_A05_spawn_location [3] =
{
	Vector3(-53.33f,37.58f,8.87f),
	Vector3(-111.40f,32.27f,9.31f),
	Vector3(-166.93f,8.75f,9.43f)
};

DECLARE_SCRIPT (M03_A05_Evac_Zone, "")
{
	bool script_active;
	bool spawn_set;

	void Created (GameObject * /*obj*/) override
	{
		script_active = true;
		spawn_set = false;
	}

	void Entered (GameObject * /*obj*/, GameObject * enterer) override
	{
		int count;

		if ((Commands->Is_A_Star(enterer)) && (script_active))
		{
			script_active = false;

			for (count = 0;count < 3;count++)
			{
				Create_Soldier(M03_A05_spawn_location [count]);
			}
		}
	}

	void Create_Soldier (Vector3 position)
	{
		GameObject* soldier_obj_1;
		soldier_obj_1 = Commands->Create_Object("Nod MiniGunner", position);

		ActionParamsStruct params;
		
		if (soldier_obj_1)
		{
			Commands->Attach_Script(soldier_obj_1, "M00_Trigger_When_Destroyed_RMV", "1,2,1,0,1000004,1000,1,0.0,0.0,0");
			if (Commands->Get_Random(0.0f,1.0f) < 0.75f)
			{
				params.Set_Basic(this, 99, 1);
			}
			else
			{
				params.Set_Basic(this, 85, 1);
			}
			params.Set_Movement(Vector3(-330.0f,75.0f,4.0f), 1.0f, 1.0f);
			Commands->Action_Goto(soldier_obj_1, params);
		}
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		Vector3 spawn_set_location;

		if (type == 1000) // Destroyed Soldier Needs Spawning
		{
			if (spawn_set)
			{
				spawn_set_location = Vector3(-167.38f,0.09f,9.46f);
			}
			else
			{
				spawn_set_location = Vector3(-99.50f,80.53f,9.00f);
			}
			Create_Soldier (spawn_set_location);
		}
		if (type == 1001) // Player entered Zone A
		{
			spawn_set = true;
		}
		if (type == 1002) // Player entered Zone B
		{
			spawn_set = false;
		}
	}
};


DECLARE_SCRIPT (M00_Test_Sound_RAD, "")
{
	void Created (GameObject * /*obj*/) override
	{
		int id = Commands->Create_Conversation("M00_TEST_CONVERSATION", 100, 300, true);
		Commands->Stop_All_Conversations (); /// <--- This cancels the following conversation!
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation (STAR, id, true, false, false);
		Commands->Start_Conversation(id, 0);
	}
};
