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
*     Mission00.cpp
*
* DESCRIPTION
*     Tutorial Mission Specific Scripts
*
* PROGRAMMER
*     Rich Donnelly
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 174 $
*     $Modtime: 1/24/02 2:44p $
*     $Archive: /Commando/Code/Scripts/Mission00.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Mission00.h"
#include <cstdint>


// MTU_Tutorial_Controller - This script handles all mission objectives and state switches for Tutorial.

DECLARE_SCRIPT (MTU_Tutorial_Controller, "")
{
	bool sydney_restart;
	bool mobius_restart;
	bool petrova_restart;
	bool finale_active;
	int sydney_health_pow;
	int sydney_armor_pow;
	int range_target_one;
	int range_target_two;
	int range_target_three;
	int range_target_four;
	int range_powerup;
	int hotwire_vehicle;
	int hotwire_state;
	int checkpoint_count;
	int squish_target_01;
	int squish_target_02;
	int squish_target_03;
	int squish_target_04;
	int squish_target_05;
	int squish_target_06;
	int lieutenant_id;
	int officer_count;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (sydney_health_pow, 1);
		SAVE_VARIABLE (sydney_armor_pow, 2);
		SAVE_VARIABLE (range_target_one, 3);
		SAVE_VARIABLE (range_target_two, 4);
		SAVE_VARIABLE (range_target_three, 5);
		SAVE_VARIABLE (range_target_four, 6);
		SAVE_VARIABLE (range_powerup, 7);
		SAVE_VARIABLE (sydney_restart, 8);
		SAVE_VARIABLE (hotwire_vehicle, 9);
		SAVE_VARIABLE (hotwire_state, 10);
		SAVE_VARIABLE (checkpoint_count, 11);
		SAVE_VARIABLE (squish_target_01, 12);
		SAVE_VARIABLE (squish_target_02, 13);
		SAVE_VARIABLE (squish_target_03, 14);
		SAVE_VARIABLE (squish_target_04, 15);
		SAVE_VARIABLE (squish_target_05, 16);
		SAVE_VARIABLE (squish_target_06, 17);
		SAVE_VARIABLE (mobius_restart, 18);
		SAVE_VARIABLE (petrova_restart, 19);
		SAVE_VARIABLE (finale_active, 20);
		SAVE_VARIABLE (lieutenant_id, 21);
		SAVE_VARIABLE (officer_count, 22);
	}

	void Created (GameObject *obj) override
	{
		sydney_restart = true;
		mobius_restart = true;
		petrova_restart = true;
		finale_active = false;
		sydney_health_pow = 0;
		sydney_armor_pow = 0;
		range_target_one = 0;
		range_target_two = 0;
		range_target_three = 0;
		range_target_four = 0;
		range_powerup = 0;
		hotwire_vehicle = 0;
		hotwire_state = -1;
		checkpoint_count = 0;
		squish_target_01 = 0;
		squish_target_02 = 0;
		squish_target_03 = 0;
		squish_target_04 = 0;
		squish_target_05 = 0;
		squish_target_06 = 0;
		lieutenant_id = 0;
		officer_count = 0;

		Commands->Reveal_Map ();

		// Create radar markers for each building MCT.

		Commands->Add_Radar_Marker (MTU_RADAR_MCT_01, Vector3(29.63f,11.98f,-8.04f), RADAR_BLIP_SHAPE_STATIONARY, RADAR_BLIP_COLOR_GDI);
		Commands->Add_Radar_Marker (MTU_RADAR_MCT_02, Vector3(10.15f,-19.68f,-8.36f), RADAR_BLIP_SHAPE_STATIONARY, RADAR_BLIP_COLOR_GDI);
		Commands->Add_Radar_Marker (MTU_RADAR_MCT_03, Vector3(-43.23f,25.66f,-6.32f), RADAR_BLIP_SHAPE_STATIONARY, RADAR_BLIP_COLOR_GDI);
		Commands->Add_Radar_Marker (MTU_RADAR_MCT_04, Vector3(-25.33f,56.15f,-7.56f), RADAR_BLIP_SHAPE_STATIONARY, RADAR_BLIP_COLOR_GDI);
		Commands->Add_Radar_Marker (MTU_RADAR_MCT_05, Vector3(-7.89f,15.57f,-8.75f), RADAR_BLIP_SHAPE_STATIONARY, RADAR_BLIP_COLOR_GDI);

		// Add all objectives as hidden to start.

		Commands->Add_Objective (MTU_OBJECTIVE_01, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_01, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_01);
		Commands->Add_Objective (MTU_OBJECTIVE_02, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_02, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_02);
		Commands->Add_Objective (MTU_OBJECTIVE_03, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_03, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_03);
		Commands->Add_Objective (MTU_OBJECTIVE_04, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_04, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_04);
		Commands->Add_Objective (MTU_OBJECTIVE_05, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_05, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_05);
		Commands->Add_Objective (MTU_OBJECTIVE_06, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Primary_M00_06, "IDS_M00EVAG_DSGN0056I1EVAG_SND", IDS_Enc_Obj_Primary_M00_06);

		// Start a slight delay timer, then tell Logan Sheppard to begin.

		Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_MISSION_START);
		Commands->Start_Timer (obj, this, (10 + Get_Int_Random (0, 20)), MTU_TIMER_FLYOVERS);

		// Set the gates to be closed at first.
		
		GameObject * course_gate = Commands->Find_Object (MTU_GATE_01);
		if (course_gate)
		{
			Commands->Set_Animation_Frame (course_gate, "CHT_JAIL.CHT_JAIL", 0);
		}
		course_gate = Commands->Find_Object (MTU_GATE_02);
		if (course_gate)
		{
			Commands->Set_Animation_Frame (course_gate, "CHT_JAIL.CHT_JAIL", 0);
		}
		course_gate = Commands->Find_Object (MTU_GATE_03);
		if (course_gate)
		{
			Commands->Set_Animation_Frame (course_gate, "CHT_JAIL.CHT_JAIL", 0);
		}
		course_gate = Commands->Find_Object (MTU_GATE_04);
		if (course_gate)
		{
			Commands->Set_Animation_Frame (course_gate, "CHT_JAIL.CHT_JAIL", 0);
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		Vector3 powerup_loc = Vector3(-36.504f,75.985f,1.006f);

		switch (timer_id)
		{
		case (MTU_TIMER_MISSION_START):
			{
				// Tell Logan Sheppard to begin the tutorial.

				GameObject* logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_INTRO, 0.0f);
				}

				// Turn off the AGT

				GameObject * tower = Commands->Find_Object (MTU_TOWER);
				if (tower)
				{
					Commands->Send_Custom_Event (obj, tower, 0, 0, 0.0f);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_SNIPER_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_SniperRifle_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "1");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_AUTORIFLE_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_AutoRifle_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "2");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_GRENADE_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_GrenadeLauncher_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "3");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_CHAINGUN_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_Chaingun_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "4");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_FLAMETHROWER_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_Flamethrower_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "5");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_ROCKET_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_RocketLauncher_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "6");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_GUNNER_C4_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_MineRemote_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "7");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case(MTU_TIMER_GUNNER_ION_AMMO):
			{
				GameObject * powerup = Commands->Create_Object ("POW_IonCannonBeacon_Player", powerup_loc);
				if (powerup)
				{
					Commands->Attach_Script (powerup, "MTU_Range_Powerup", "8");
					range_powerup = Commands->Get_ID (powerup);
				}
				break;
			}
		case (MTU_TIMER_ENDGAME):
			{
				Commands->Mission_Complete (true);
				break;
			}
		case (MTU_TIMER_FLYOVERS):
			{
				const char * vehicle = "";

				if (Get_Int_Random (0, 2))
				{
					vehicle = "GDI_Orca";
				}
				else
				{
					vehicle = "GDI_Transport_Helicopter";
				}
				if (Get_Int_Random (0, 2))
				{
					GameObject * flyover = Commands->Create_Object (vehicle, Vector3 (51.02f,128.63f,41.45f));
					if (flyover)
					{
						Commands->Attach_Script (flyover, "MTU_Flyover", "0");
					}
				}
				else
				{
					GameObject * flyover = Commands->Create_Object (vehicle, Vector3 (131.78f,-14.58f,51.21f));
					if (flyover)
					{
						Commands->Attach_Script (flyover, "MTU_Flyover", "1");
					}
				}
				Commands->Start_Timer (obj, this, (10 + Get_Int_Random (0, 20)), MTU_TIMER_FLYOVERS);
				break;
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		Vector3 powerup_loc = Vector3(-36.504f,75.985f,1.006f);

		switch (type)
		{
		case (MTU_TYPE_SYDNEY_RESET):
			{
				sydney_restart = true;
				break;
			}
		case (MTU_TYPE_SYDNEY_CHECK_RESET):
			{
				if (sydney_restart)
				{
					GameObject * sydney_zone = Commands->Find_Object (MTU_ZONE_START_SYDNEY);
					if (sydney_zone)
					{
						sydney_restart = false;
						Commands->Send_Custom_Event (obj, sydney_zone, MTU_TYPE_SYDNEY_IS_RESET, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_TYPE_HEALTH_POWERUP_ADD):
			{
				sydney_health_pow = param;
				break;
			}
		case (MTU_TYPE_HEALTH_POWERUP_SUBTRACT):
			{
				sydney_health_pow = 0;
				break;
			}
		case (MTU_TYPE_ALL_POWERUPS_RESET):
			{
				GameObject * health_pow = Commands->Find_Object (sydney_health_pow);
				if (health_pow)
				{
					Commands->Destroy_Object (health_pow);
				}
				sydney_health_pow = 0;
				GameObject * armor_pow = Commands->Find_Object (sydney_armor_pow);
				if (armor_pow)
				{
					Commands->Destroy_Object (armor_pow);
				}
				sydney_armor_pow = 0;
				break;
			}
		case (MTU_TYPE_ARMOR_POWERUP_ADD):
			{
				sydney_armor_pow = param;
				break;
			}
		case (MTU_TYPE_ARMOR_POWERUP_SUBTRACT):
			{
				sydney_armor_pow = 0;
				break;
			}
		case (MTU_TYPE_RANGE_CLEANUP_TARGETS):
			{
				GameObject * range_target_one_obj = Commands->Find_Object (range_target_one);
				if (range_target_one_obj)
				{
					range_target_one = 0;
					Commands->Destroy_Object (range_target_one_obj);
				}
				GameObject * range_target_two_obj = Commands->Find_Object (range_target_two);
				if (range_target_two_obj)
				{
					range_target_two = 0;
					Commands->Destroy_Object (range_target_two_obj);
				}
				GameObject * range_target_three_obj = Commands->Find_Object (range_target_three);
				if (range_target_three_obj)
				{
					range_target_three = 0;
					Commands->Destroy_Object (range_target_three_obj);
				}
				GameObject * range_target_four_obj = Commands->Find_Object (range_target_four);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					Commands->Destroy_Object (range_target_four_obj);
				}
				GameObject * range_powerup_obj = Commands->Find_Object (range_powerup);
				if (range_powerup_obj)
				{
					range_powerup = 0;
					Commands->Destroy_Object (range_powerup_obj);
				}
				break;
			}
		case (MTU_TYPE_RANGE_CHECK_TARGETS):
			{
				bool found_targets = false;
				GameObject * range_target_one_obj = Commands->Find_Object (range_target_one);
				if (range_target_one_obj)
				{
					found_targets = true;
				}
				GameObject * range_target_two_obj = Commands->Find_Object (range_target_two);
				if (range_target_two_obj)
				{
					found_targets = true;
				}
				GameObject * range_target_three_obj = Commands->Find_Object (range_target_three);
				if (range_target_three_obj)
				{
					found_targets = true;
				}
				GameObject * gunner = Commands->Find_Object (MTU_GUNNER);
				if (gunner)
				{
					if (found_targets)
					{
						Commands->Send_Custom_Event (obj, gunner, MTU_TYPE_GUNNER, MTU_PARAM_SPEECH_MORE_TARGETS, 0.0f);
					}
					else
					{
						GameObject * range_powerup_obj = Commands->Find_Object (range_powerup);
						if (range_powerup_obj)
						{
							range_powerup = 0;
							Commands->Destroy_Object (range_powerup_obj);
						}
						Commands->Send_Custom_Event (obj, gunner, MTU_TYPE_GUNNER, MTU_PARAM_TARGETS_ELIMINATED, 0.0f);
					}
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_HANDGUN):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-32.264f,50.158f,1.015f));
				if (range_target_one_obj)
				{
					Commands->Set_Facing (range_target_one_obj, 90.0f);
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-33.849f,53.635f,1.371f));
				if (range_target_two_obj)
				{
					Commands->Set_Facing (range_target_two_obj, 90.0f);
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-35.616f,53.662f,1.394f));
				if (range_target_three_obj)
				{
					Commands->Set_Facing (range_target_three_obj, 90.0f);
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				break;
			}
		case (MTU_TYPE_RANGE_TARGET_DESTROYED):
			{
				switch (param)
				{
				case (1):
					{
						range_target_one = 0;
						break;
					}
				case (2):
					{
						range_target_two = 0;
						break;
					}
				case (3):
					{
						range_target_three = 0;
						break;
					}
				case (4):
					{
						range_target_four = 0;
						break;
					}
				default:
					{
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_SNIPER_RIFLE):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-39.228f,47.670f,3.232f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target_Path_Mid", "");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-36.870f,50.308f,1.014f));
				if (range_target_two_obj)
				{
					Commands->Set_Facing (range_target_two_obj, 90.0f);
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-32.215f,50.497f,1.014f));
				if (range_target_three_obj)
				{
					Commands->Set_Facing (range_target_three_obj, 90.0f);
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_SniperRifle_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "1");
				}
				break;
			}
		case (MTU_TYPE_RANGE_POWERUP_RETRIEVED):
			{
				switch (param)
				{
				case (1):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_SNIPER_AMMO);
						break;
					}
				case (2):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_AUTORIFLE_AMMO);
						break;
					}
				case (3):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_GRENADE_AMMO);
						break;
					}
				case (4):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_CHAINGUN_AMMO);
						break;
					}
				case (5):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_FLAMETHROWER_AMMO);
						break;
					}
				case (6):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_ROCKET_AMMO);
						break;
					}
				case (7):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_C4_AMMO);
						break;
					}
				case (8):
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_GUNNER_ION_AMMO);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_AUTORIFLE):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-39.228f,47.670f,3.232f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target_Path_Mid", "");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-39.557f,49.958f,3.232f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target_Path_Right", "");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-29.893f,50.071f,3.232f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target_Path_Left", "");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_AutoRifle_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "2");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_GRENADE):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Buggy", Vector3(-34.853f,54.844f,2.641f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Set_Facing (range_target_one_obj, 130.0f);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-30.224f,47.9f,3.232f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Set_Facing (range_target_two_obj, 90.0f);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target_Miss_Commando", "");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-36.812f,47.595f,3.232f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Set_Facing (range_target_three_obj, 90.0f);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_GrenadeLauncher_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "3");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_CHAINGUN):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-32.160f,50.072f,1.015f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target_Miss_Commando", "");
					Commands->Set_Facing (range_target_one_obj, 90.0f);
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-34.704f,47.525f,3.232f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target_Miss_Commando", "");
					Commands->Set_Facing (range_target_two_obj, 90.0f);
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-37.425f,49.562f,1.015f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target_Miss_Commando", "");
					Commands->Set_Facing (range_target_three_obj, 90.0f);
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_Chaingun_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "4");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_FLAMETHROWER):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-29.966f,50.615f,3.232f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target_Path_Left", "");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-39.083f,47.287f,3.232f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target_Path_Mid", "");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-39.557f,50.218f,3.232f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target_Path_Right", "");
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_Flamethrower_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "5");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_ROCKET):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Buggy", Vector3(-34.853f,54.844f,2.641f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Set_Facing (range_target_one_obj, 135.0f);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-33.967f,50.601f,1.014f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Set_Facing (range_target_two_obj, 90.0f);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_RocketLauncher_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "6");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_C4):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Buggy", Vector3(-34.853f,54.844f,2.641f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Set_Facing (range_target_one_obj, 90.0f);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-31.702f,49.550f,1.016f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Set_Facing (range_target_two_obj, -90.0f);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-37.801f,49.708f,1.015f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Set_Facing (range_target_three_obj, -90.0f);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_MineRemote_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "7");
				}
				break;
			}
		case (MTU_TYPE_RANGE_SETUP_ION):
			{
				GameObject * range_target_one_obj = Commands->Create_Object ("Nod_Light_Tank", Vector3(-34.556f,52.912f,1.841f));
				if (range_target_one_obj)
				{
					range_target_one = Commands->Get_ID (range_target_one_obj);
					Commands->Set_Facing (range_target_one_obj, 90.0f);
					Commands->Attach_Script (range_target_one_obj, "MTU_Range_Target", "1");
				}
				GameObject * range_target_two_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-31.562f,50.246f,1.015f));
				if (range_target_two_obj)
				{
					range_target_two = Commands->Get_ID (range_target_two_obj);
					Commands->Set_Facing (range_target_two_obj, 90.0f);
					Commands->Attach_Script (range_target_two_obj, "MTU_Range_Target", "2");
				}
				GameObject * range_target_three_obj = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-37.501f,50.184f,1.014f));
				if (range_target_three_obj)
				{
					range_target_three = Commands->Get_ID (range_target_three_obj);
					Commands->Set_Facing (range_target_three_obj, 90.0f);
					Commands->Attach_Script (range_target_three_obj, "MTU_Range_Target", "3");
				}
				GameObject * range_target_four_obj = Commands->Create_Object ("POW_IonCannonBeacon_Player", powerup_loc);
				if (range_target_four_obj)
				{
					range_target_four = 0;
					range_powerup = Commands->Get_ID (range_target_four_obj);
					Commands->Attach_Script (range_target_four_obj, "MTU_Range_Powerup", "8");
				}
				break;
			}
		case (MTU_TYPE_WEAP_FACTORY_CLEANUP):
			{
				// This merely cleans up the weapons factory mess, it does not reset it!

				if (hotwire_vehicle)
				{
					GameObject * vehicle_obj = Commands->Find_Object (hotwire_vehicle);
					if (vehicle_obj)
					{
						GameObject * hotwire_obj = Commands->Find_Object (MTU_HOTWIRE);
						if (hotwire_obj)
						{
							Commands->Apply_Damage (vehicle_obj, 10000.0f, "Blamokiller", hotwire_obj);
							hotwire_vehicle = 0;
						}
					}
				}
				if (squish_target_01)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_01);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_01 = 0;
				}
				if (squish_target_02)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_02);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_02 = 0;
				}
				if (squish_target_03)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_03);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_03 = 0;
				}
				if (squish_target_04)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_04);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_04 = 0;
				}
				if (squish_target_05)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_05);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_05 = 0;
				}
				if (squish_target_06)
				{
					GameObject * squish_obj = Commands->Find_Object (squish_target_06);
					if (squish_obj)
					{
						Commands->Destroy_Object (squish_obj);
					}
					squish_target_06 = 0;
				}

				break;
			}
		case (MTU_TYPE_WEAP_ATTEMPT_RESET):
			{
				// This restarts the weapons factory only if it is ready to roll from beginning!

				if (hotwire_state == -1)
				{
					hotwire_state = 0;
					Commands->Send_Custom_Event (obj, obj, MTU_TYPE_WEAP_FACTORY_CLEANUP, MTU_PARAM_DEFAULT, 0.0f);
					Commands->Send_Custom_Event (obj, obj, MTU_TYPE_RESET_CHECKPOINT_COUNT, MTU_PARAM_DEFAULT, 0.0f);
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_WEAPONS, 0.0f);
					}
				}
				break;
			}
		case (MTU_TYPE_WEAP_FORCE_RESET):
			{
				// This is called by all other buildings to force weapon factory cleanup!
				hotwire_state = -1;
				Commands->Send_Custom_Event (obj, obj, MTU_TYPE_WEAP_FACTORY_CLEANUP, MTU_PARAM_DEFAULT, 0.0f);
				Commands->Send_Custom_Event (obj, obj, MTU_TYPE_RESET_CHECKPOINT_COUNT, MTU_PARAM_DEFAULT, 0.0f);
				break;
			}
		case (MTU_TYPE_HOTWIRE_START):
			{
				bool advance_state = false;

				if (hotwire_state > -1)
				{
					switch (hotwire_state)
					{
					case (0):
						{
							GameObject * hotwire = Commands->Find_Object (MTU_HOTWIRE);
							if (hotwire)
							{
								Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_INTRO, 0.0f);
							}
							advance_state = true;
							break;
						}
					case(1):
						{
							GameObject * hotwire = Commands->Find_Object (MTU_HOTWIRE);
							if (hotwire)
							{
								if (checkpoint_count > 3)
								{
									Commands->Send_Custom_Event (obj, obj, MTU_TYPE_RESET_CHECKPOINT_COUNT, MTU_PARAM_DEFAULT, 0.0f);
									Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_MEDTANK, 0.0f);
									Commands->Send_Custom_Event (obj, obj, MTU_TYPE_WEAP_FACTORY_CLEANUP, MTU_PARAM_DEFAULT, 0.0f);
									Commands->Send_Custom_Event (obj, obj, MTU_TYPE_WEAP_CREATE_MEDTANK, MTU_PARAM_DEFAULT, 0.0f);
									GameObject * action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_01);
									if (action_zone)
									{
										Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
									}
									action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_02);
									if (action_zone)
									{
										Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
									}
									action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_03);
									if (action_zone)
									{
										Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
									}
									action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_04);
									if (action_zone)
									{
										Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
									}
									advance_state = true;
								}
								else
								{
									Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_UNFINISHED, 0.0f);
								}
							}
							break;
						}
					}
					if (advance_state)
					{
						hotwire_state++;
					}
				}
				break;
			}
		case (MTU_TYPE_WEAP_CREATE_HUMMVEE):
			{
				GameObject * vehicle = Commands->Create_Object ("GDI_Humm-vee_Player", Vector3 (-10.5f,-34.2f,10.192f));
				if (vehicle)
				{
					Commands->Set_Facing (vehicle, -90.0f);
					Commands->Attach_Script (vehicle, "MTU_GDI_Vehicle", "1");
					hotwire_vehicle = Commands->Get_ID (vehicle);
				}
				break;
			}
		case (MTU_TYPE_WEAP_CREATE_MEDTANK):
			{
				GameObject * vehicle = Commands->Create_Object ("GDI_Medium_Tank_Player", Vector3 (-10.5f,-34.2f,12.192f));
				if (vehicle)
				{
					Commands->Set_Facing (vehicle, -90.0f);
					Commands->Attach_Script (vehicle, "MTU_GDI_Vehicle", "2");
					hotwire_vehicle = Commands->Get_ID (vehicle);
				}
				break;
			}
		case (MTU_TYPE_RESET_CHECKPOINT_COUNT):
			{
				checkpoint_count = 0;
				break;
			}
		case (MTU_TYPE_ADD_CHECKPOINT):
			{
				checkpoint_count++;
				break;
			}
		case (MTU_TYPE_WEAP_CREATE_SQUISHIES):
			{
				GameObject * squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-27.336f,-11.719f,1.101f));
				if (squish_target)
				{
					squish_target_01 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, -45.0f);
					Commands->Innate_Disable (squish_target);
				}
				squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(-28.596f,21.970f,1.365f));
				if (squish_target)
				{
					squish_target_02 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, 0.0f);
					Commands->Innate_Disable (squish_target);
				}
				squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(10.713f,35.423f,0.691f));
				if (squish_target)
				{
					squish_target_03 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, -90.0f);
					Commands->Innate_Disable (squish_target);
				}
				squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(50.947f,25.583f,0.899f));
				if (squish_target)
				{
					squish_target_04 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, -90.0f);
					Commands->Innate_Disable (squish_target);
				}
				squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(41.174f,-12.436f,0.947f));
				if (squish_target)
				{
					squish_target_05 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, 135.0f);
					Commands->Innate_Disable (squish_target);
				}
				squish_target = Commands->Create_Object ("Nod_Minigunner_0", Vector3(7.102f,-49.075f,1.026f));
				if (squish_target)
				{
					squish_target_06 = Commands->Get_ID (squish_target);
					Commands->Set_Facing (squish_target, 105.0f);
					Commands->Innate_Disable (squish_target);
				}
				break;
			}
		case (MTU_TYPE_MOBIUS_RESET):
			{
				mobius_restart = true;
				break;
			}
		case (MTU_TYPE_MOBIUS_CHECK_RESET):
			{
				if (mobius_restart)
				{
					mobius_restart = false;
					GameObject * mobius = Commands->Find_Object (MTU_MOBIUS);
					if (mobius)
					{
						Commands->Send_Custom_Event (obj, mobius, MTU_TYPE_MOBIUS, MTU_PARAM_SPEECH_MOBIUS_REFINERY, 0.0f);
					}
				}
				break;
			}
		case (MTU_TYPE_RESET_PETROVA):
			{
				petrova_restart = true;
				break;
			}
		case (MTU_TYPE_PETROVA_CHECK_RESET):
			{
				if (petrova_restart)
				{
					petrova_restart = false;
					GameObject * petrova = Commands->Find_Object (MTU_PETROVA);
					if (petrova)
					{
						Commands->Send_Custom_Event (obj, petrova, MTU_TYPE_PETROVA, MTU_PARAM_SPEECH_PETROVA_POWER, 0.0f);
					}
				}
				break;
			}
		case (MTU_TYPE_ACTIVATE_FINALE):
			{
				Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
				GameObject * instructor = Commands->Find_Object (MTU_SYDNEY);
				if (instructor)
				{
					Commands->Destroy_Object (instructor);
				}
				instructor = Commands->Find_Object (MTU_GUNNER);
				if (instructor)
				{
					Commands->Destroy_Object (instructor);
				}
				instructor = Commands->Find_Object (MTU_HOTWIRE);
				if (instructor)
				{
					Commands->Destroy_Object (instructor);
				}
				instructor = Commands->Find_Object (MTU_MOBIUS);
				if (instructor)
				{
					Commands->Destroy_Object (instructor);
				}
				instructor = Commands->Find_Object (MTU_PETROVA);
				if (instructor)
				{
					Commands->Destroy_Object (instructor);
				}
				instructor = Commands->Find_Object (MTU_CONTROLLER);
				if (instructor)
				{
					Commands->Send_Custom_Event (obj, instructor, MTU_TYPE_WEAP_FORCE_RESET, MTU_PARAM_DEFAULT, 0.0f);
				}
				instructor = Commands->Create_Object ("GDI_Female_Lieutenant", Vector3(-43.724f,37.967f,2.0f));
				if (instructor)
				{
					Commands->Attach_Script (instructor, "MTU_Tutorial_Instructor", "");
					lieutenant_id = Commands->Get_ID (instructor);
				}
				finale_active = true;
				break;
			}
		case (MTU_TYPE_LIEUTENANT_START):
			{
				GameObject * lieutenant = Commands->Find_Object (lieutenant_id);
				if (lieutenant)
				{
					Commands->Send_Custom_Event (obj, lieutenant, MTU_TYPE_LIEUTENANT, MTU_PARAM_ACTION_GOTO_HAVOC, 0.0f);
				}
				break;
			}
		case (MTU_TYPE_MOCK_INVASION):
			{
				//set objectives for the two officers. Upon completion, end the mission.
				GameObject * soldier = Commands->Find_Object (MTU_GATE_GUARD);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MTU_TYPE_GDI_SOLDIER_PATROL, MTU_PARAM_DEFAULT, 0.0f);
				}
				soldier = Commands->Find_Object (MTU_GDI_01);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MTU_TYPE_GDI_SOLDIER_PATROL, MTU_PARAM_DEFAULT, 0.0f);
				}
				soldier = Commands->Find_Object (MTU_GDI_02);
				if (soldier)
				{
					Commands->Send_Custom_Event (obj, soldier, MTU_TYPE_GDI_SOLDIER_PATROL, MTU_PARAM_DEFAULT, 0.0f);
				}
				GameObject * officer = Commands->Create_Object ("Nod_Minigunner_1Off", Vector3 (51.793f,23.554f,1.135f));
				if (officer)
				{
					Commands->Set_Obj_Radar_Blip_Shape (officer, RADAR_BLIP_SHAPE_OBJECTIVE);
					Commands->Set_Obj_Radar_Blip_Color (officer, RADAR_BLIP_COLOR_PRIMARY_OBJECTIVE);
					Commands->Attach_Script (officer, "MTU_Nod_Soldier", "2");
				}
				officer = Commands->Create_Object ("Nod_Minigunner_1Off", Vector3 (52.793f,24.554f,2.135f));
				if (officer)
				{
					Commands->Set_Obj_Radar_Blip_Shape (officer, RADAR_BLIP_SHAPE_OBJECTIVE);
					Commands->Set_Obj_Radar_Blip_Color (officer, RADAR_BLIP_COLOR_PRIMARY_OBJECTIVE);
					Commands->Attach_Script (officer, "MTU_Nod_Soldier", "2");
				}
				GameObject * hummvee = Commands->Create_Object ("GDI_Humm-vee_Player", Vector3(-10.5f,-34.2f,10.192f));
				if (hummvee)
				{
					Commands->Attach_Script (hummvee, "MTU_GDI_Vehicle", "4");
				}
				GameObject * spawn = Commands->Trigger_Spawner(MTU_SPAWN_01);
				if (spawn)
				{
					Commands->Attach_Script(spawn, "MTU_Nod_Soldier", "0");
				}
				spawn = Commands->Trigger_Spawner(MTU_SPAWN_02);
				if (spawn)
				{
					Commands->Attach_Script(spawn, "MTU_Nod_Soldier", "0");
				}
				spawn = Commands->Trigger_Spawner(MTU_SPAWN_03);
				if (spawn)
				{
					Commands->Attach_Script(spawn, "MTU_Nod_Soldier", "0");
				}
				break;
			}
		case (MTU_TYPE_TRIGGER_SPAWNER):
			{
				int spawner_id = MTU_SPAWN_01 + Get_Int_Random (0, 2);
				GameObject * spawn = Commands->Trigger_Spawner (spawner_id);
				if (spawn)
				{
					Commands->Attach_Script(spawn, "MTU_Nod_Soldier", "0");
				}
				break;
			}
		case (MTU_TYPE_COUNT_OFFICERS):
			{
				officer_count++;
				if (officer_count > 1)
				{
					Commands->Set_Objective_Status (MTU_OBJECTIVE_06, OBJECTIVE_STATUS_ACCOMPLISHED);
					Commands->Set_HUD_Help_Text(IDS_MENU_TEXT795, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Start_Timer (obj, this, 3.0f, MTU_TIMER_ENDGAME);
				}
				break;
			}
		case (MTU_TYPE_REMOVE_OBJECTIVES):
			{
				Commands->Clear_Radar_Marker (MTU_RADAR_GUNNER);
				break;
			}
		}
	}
};


// MTU_Tutorial_Instructor - This script is used for all instructors throughout the tutorial mission.

DECLARE_SCRIPT (MTU_Tutorial_Instructor, "")
{
	bool logan_heard_commando;
	bool logan_told_about_hearing;
	bool logan_gave_sneak_speech;
	bool intro_barracks;
	bool intro_weapons;
	bool intro_refinery;
	bool intro_power;
	bool check_wait_for_havoc;
	bool check_wait_refinery;
	bool check_logan_wait_power;
	bool check_wait_agt;
	bool check_wait_infantry;
	bool logan_wait_havoc;
	bool logan_wait_refinery;
	bool logan_wait_power;
	bool logan_wait_agt;
	bool logan_wait_infantry;
	int gunner_range_state;
	int engineer_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (logan_heard_commando, 1);
		SAVE_VARIABLE (logan_told_about_hearing, 2);
		SAVE_VARIABLE (logan_gave_sneak_speech, 3);
		SAVE_VARIABLE (gunner_range_state, 4);
		SAVE_VARIABLE (intro_barracks, 5);
		SAVE_VARIABLE (intro_weapons, 6);
		SAVE_VARIABLE (intro_refinery, 7);
		SAVE_VARIABLE (intro_power, 8);
		SAVE_VARIABLE (engineer_id, 9);
		SAVE_VARIABLE (check_wait_for_havoc, 10);
		SAVE_VARIABLE (check_wait_refinery, 11);
		SAVE_VARIABLE (check_logan_wait_power, 12);
		SAVE_VARIABLE (check_wait_agt, 13);
		SAVE_VARIABLE (check_wait_infantry, 14);
		SAVE_VARIABLE (logan_wait_havoc, 15);
		SAVE_VARIABLE (logan_wait_refinery, 16);
		SAVE_VARIABLE (logan_wait_power, 17);
		SAVE_VARIABLE (logan_wait_agt, 18);
		SAVE_VARIABLE (logan_wait_infantry, 19);
	}

	void Created (GameObject * obj) override
	{
		engineer_id = 0;
		gunner_range_state = MTU_RANGE_STATE_DEFAULT;
		Commands->Set_Loiters_Allowed (obj, false);
		Commands->Innate_Disable (obj);
		Commands->Set_Shield_Type (obj, "Blamo");
		Commands->Enable_Hibernation (obj, false);
		logan_heard_commando = false;
		logan_told_about_hearing = false;
		logan_gave_sneak_speech = false;
		intro_barracks = false;
		intro_weapons = false;
		intro_refinery = false;
		intro_power = false;
		check_wait_for_havoc = false;
		check_wait_refinery = false;
		check_logan_wait_power = false;
		check_wait_agt = false;
		check_wait_infantry = false;
		logan_wait_havoc = false;
		logan_wait_refinery = false;
		logan_wait_power = false;
		logan_wait_agt = false;
		logan_wait_infantry = false;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		switch (type)
		{
		case (MTU_TYPE_LOGAN):
			{
				switch (param)
				{
				case (MTU_PARAM_SPEECH_INTRO): // Logan is told to start the tutorial.
					{
						Say_Something (obj, MTU_SPEECH_LOGAN_START);
						break;
					}
				case (MTU_PARAM_SPEECH_CROUCH): // Logan is told to start sneaking speech.
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_LOGAN_CROUCH);
						break;
					}
				case (MTU_PARAM_SPEECH_JUMP): // Logan is told to give results of sneaking and start jump speech.
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_JUMP_TRAINING, MTU_ACTION_LOGAN_JUMP_TEST);
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						/*if (logan_heard_commando)
						{
							Say_Something (obj, MTU_SPEECH_LOGAN_SNEAK_LOSE);
							logan_told_about_hearing = true;
						}
						else
						{
							Say_Something (obj, MTU_SPEECH_LOGAN_SNEAK_WIN);
							logan_told_about_hearing = true;
						}*/
						break;
					}
				case (MTU_PARAM_SPEECH_EVA): // Logan is told to start the eva training speech.
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_LOGAN_EVA);
						break;
					}
				case (MTU_PARAM_SPEECH_COURSE_DONE): // Logan is told to move to the commando and finish the obstacle course.
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Move_Somewhere (obj, MTU_MOVE_LOGAN_COURSE_EXTERIOR, MTU_ACTION_LOGAN_COURSE_DONE);
						break;
					}
				case (MTU_PARAM_ACTION_KEYCARD_TRAIN): // The commando has arrived at the AGT, lock control, give keycard speech.
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_LOGAN_KEYCARDS);
						break;
					}
				case (MTU_PARAM_ACTION_GOTO_AGT_RESET):
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_AGT, MTU_ACTION_DEFAULT);
						break;
					}
				case (MTU_PARAM_ACTION_GOTO_INFANTRY):
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_INFANTRY, MTU_ACTION_DEFAULT);
						break;
					}
				case (MTU_PARAM_ACTION_GOTO_WEAPONS):
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_WEAPONS, MTU_ACTION_DEFAULT);
						break;
					}
				case (MTU_PARAM_ACTION_PREPARE_REFINERY):
					{
						logan_wait_havoc = true;
						logan_wait_refinery = false;
						logan_wait_power = false;
						logan_wait_agt = false;
						logan_wait_infantry = false;
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_FOR_HAVOC);
						break;
					}
				case (MTU_PARAM_ACTION_GOTO_REFINERY):
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_REFINERY, MTU_ACTION_DEFAULT);
						break;
					}
				case (MTU_PARAM_SPEECH_INTRO_REFINERY):
					{
						if (!intro_refinery)
						{
							intro_refinery = true;
							Say_Something (obj, MTU_SPEECH_INTRODUCE_REFINERY);
						}
						break;
					}
				case (MTU_PARAM_ACTION_PREPARE_POWER):
					{
						logan_wait_havoc = false;
						logan_wait_refinery = true;
						logan_wait_power = false;
						logan_wait_agt = false;
						logan_wait_infantry = false;
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_REFINERY);
						break;
					}
				case (MTU_PARAM_ACTION_GOTO_POWER):
					{
						Move_Somewhere (obj, MTU_MOVE_LOGAN_POWER, MTU_ACTION_DEFAULT);
						break;
					}
				case (MTU_PARAM_SPEECH_INTRO_POWER):
					{
						if (!intro_power)
						{
							intro_power = true;
							Say_Something (obj, MTU_SPEECH_INTRODUCE_POWER);
						}
						break;
					}
				case (MTU_PARAM_ACTION_PREPARE_FINALE):
					{
						logan_wait_havoc = false;
						logan_wait_refinery = false;
						logan_wait_power = true;
						logan_wait_agt = false;
						logan_wait_infantry = false;
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_POWER);
						break;
					}
				case (MTU_PARAM_SPEECH_INTRO_INFANTRY):
					{
						if (!intro_barracks)
						{
							intro_barracks = true;
							Say_Something (obj, MTU_SPEECH_INTRODUCE_BARRACKS);
						}
						break;
					}
				case (MTU_PARAM_ACTION_PREPARE_INFANTRY):
					{
						logan_wait_havoc = false;
						logan_wait_refinery = false;
						logan_wait_power = false;
						logan_wait_agt = true;
						logan_wait_infantry = false;
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_AGT);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_INTRO):
					{
						if (!intro_weapons)
						{
							intro_weapons = true;
							Say_Something (obj, MTU_SPEECH_INTRODUCE_WEAP_FACTORY);
						}
						break;
					}
				case (MTU_PARAM_ACTION_PREPARE_WEAPONS):
					{
						logan_wait_havoc = false;
						logan_wait_refinery = false;
						logan_wait_power = false;
						logan_wait_agt = false;
						logan_wait_infantry = true;
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_INFANTRY);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_SYDNEY):
			{
				switch (param)
				{
				case (MTU_PARAM_SPEECH_SYDNEY_START):
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_SYDNEY_START);
						break;
					}
				case (MTU_PARAM_SPEECH_HEALTH):
					{
						Commands->Action_Reset (obj, 100);
						Say_Something (obj, MTU_SPEECH_SYDNEY_HEALTH);
						break;
					}
				case (MTU_PARAM_SPEECH_PICKUP):
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_SYDNEY_ARMOR);
						break;
					}
				case (MTU_PARAM_SPEECH_SHOOT_AGAIN):
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Say_Something (obj, MTU_SPEECH_SYDNEY_SHOOT_AGAIN);
						break;
					}
				case (MTU_PARAM_SPEECH_LAST_TIME):
					{
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
						Commands->Action_Reset (obj, 100);
						Say_Something (obj, MTU_SPEECH_SYDNEY_LAST_TIME);
						break;
					}
				case (MTU_PARAM_SPEECH_RADAR):
					{
						Say_Something (obj, MTU_SPEECH_SYDNEY_RADAR);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_GUNNER):
			{
				switch (param)
				{
				case (MTU_PARAM_GUNNER_RESET):
					{
						gunner_range_state = MTU_RANGE_STATE_DEFAULT;
						break;
					}
				case (MTU_PARAM_CHECK_TARGETS):
					{
						GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_CHECK_TARGETS, MTU_PARAM_DEFAULT, 0.0f);
						}
						break;
					}
				case (MTU_PARAM_TARGETS_ELIMINATED):
					{
						gunner_range_state++;
						switch (gunner_range_state)
						{
						case (1):
							{
								Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
								Say_Something (obj, MTU_SPEECH_GUNNER_START);
								break;
							}
						case (2):
							{
								Commands->Set_HUD_Help_Text(IDS_M01DSGN_DSGN0518I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
								Say_Something (obj, MTU_SPEECH_GUNNER_SNIPER_RIFLE);
								Commands->Give_PowerUp (STAR, "POW_SniperRifle_Player", true);
								Commands->Create_Object ("POW_SniperRifle_Player", Vector3(-17.298f,73.106f,1.042f));
								GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
								if (controller)
								{
									Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_SNIPER_RIFLE, MTU_PARAM_DEFAULT, 0.0f);
								}
								break;
							}
						case (3):
							{
								Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0392I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
								Say_Something (obj, MTU_SPEECH_GUNNER_AUTORIFLE);
								Commands->Give_PowerUp (STAR, "POW_AutoRifle_Player", true);
								Commands->Create_Object ("POW_AutoRifle_Player", Vector3(-17.298f,73.106f,1.042f));
								GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
								if (controller)
								{
									Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_AUTORIFLE, MTU_PARAM_DEFAULT, 0.0f);
								}
								break;
							}
						case (4):
							{
								Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0393I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
								Say_Something (obj, MTU_SPEECH_GUNNER_ROCKET);
								Commands->Give_PowerUp (STAR, "POW_RocketLauncher_Player", true);
								Commands->Create_Object ("POW_RocketLauncher_Player", Vector3(-17.298f,73.106f,1.042f));
								GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
								if (controller)
								{
									Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_ROCKET, MTU_PARAM_DEFAULT, 0.0f);
								}
								break;
							}
						case (5):
							{
								Commands->Set_HUD_Help_Text(IDS_M01DSGN_DSGN0519I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
								Say_Something (obj, MTU_SPEECH_GUNNER_C4);
								Commands->Give_PowerUp (STAR, "POW_MineRemote_Player", true);
								Commands->Create_Object ("POW_MineRemote_Player", Vector3(-17.298f,73.106f,1.042f));
								GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
								if (controller)
								{
									Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_C4, MTU_PARAM_DEFAULT, 0.0f);
								}
								break;
							}
						case (6):
							{
								Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0394I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
								Say_Something (obj, MTU_SPEECH_GUNNER_ION);
								Commands->Give_PowerUp (STAR, "POW_IonCannonBeacon_Player", true);
								Commands->Create_Object ("POW_IonCannonBeacon_Player", Vector3(-17.298f,73.106f,1.042f));
								GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
								if (controller)
								{
									Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_ION, MTU_PARAM_DEFAULT, 0.0f);
								}
								break;
							}
						case (7):
							{
								Say_Something (obj, MTU_SPEECH_GUNNER_ENDING);
								break;
							}
						default:
							{
								break;
							}
						}
						break;
					}
				case (MTU_PARAM_SPEECH_MORE_TARGETS):
					{
						Say_Something (obj, MTU_SPEECH_GUNNER_MORE_TARGETS);
						break;
					}
				case (MTU_PARAM_SPEECH_RETICULE):
					{
						Say_Something (obj, MTU_SPEECH_GUNNER_RETICULE);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_HOTWIRE):
			{
				switch (param)
				{
				case (MTU_PARAM_SPEECH_WEAP_INTRO):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_INTRO);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_MOVEOUT):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_MOVEOUT);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_ACTION):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_ACTION);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_MEDTANK):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_MEDTANK);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_UNFINISHED):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_UNFINISHED);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_SQUISH):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_SQUISH);
						break;
					}
				case (MTU_PARAM_SPEECH_WEAP_BIKE):
					{
						Say_Something (obj, MTU_SPEECH_HOTWIRE_BIKE);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_MOBIUS):
			{
				if (param == MTU_PARAM_SPEECH_MOBIUS_REFINERY)
				{
					Say_Something (obj, MTU_SPEECH_MOBIUS_REFINERY);
				}
				break;
			}
		case (MTU_TYPE_PETROVA):
			{
				switch (param)
				{
				case (MTU_PARAM_SPEECH_PETROVA_POWER):
					{
						Say_Something (obj, MTU_SPEECH_PETROVA_POWER);
						break;
					}
				}
				break;
			}
		case (MTU_TYPE_LIEUTENANT):
			{
				switch (param)
				{
				case (MTU_PARAM_ACTION_GOTO_HAVOC):
					{
						Move_Somewhere (obj, MTU_MOVE_LIEUTENANT_HAVOC, MTU_ACTION_MOVE_LIEUTENANT_HAVOC);
						break;
					}
				}
				break;
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		switch (timer_id)
		{
		case (MTU_TIMER_LOGAN_WAIT_FOR_HAVOC):
			{
				if (logan_wait_havoc)
				{
					if (Get_Obj_Distance (obj, STAR) < 4.0f)
					{
						if (!check_wait_for_havoc)
						{
							Say_Something (obj, MTU_SPEECH_LOGAN_WHATSNEXT);
							check_wait_for_havoc = true;
						}
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_FOR_HAVOC);
					}
				}
				break;
			}
		case (MTU_TIMER_LOGAN_WAIT_REFINERY):
			{
				if (logan_wait_refinery)
				{
					if (Get_Obj_Distance (obj, STAR) < 4.0f)
					{
						if (!check_wait_refinery)
						{
							Say_Something (obj, MTU_SPEECH_LOGAN_PREPARE_POWER);
							check_wait_refinery = true;
						}
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_REFINERY);
					}
				}
				break;
			}
		case (MTU_TIMER_LOGAN_WAIT_POWER):
			{
				if (logan_wait_power)
				{
					if (Get_Obj_Distance (obj, STAR) < 2.0f)
					{
						if (!check_logan_wait_power)
						{
							Commands->Set_Position (STAR, Vector3(-35.771f,35.033f,2.0f));
							Commands->Set_Position (obj, Vector3(-33.699f,35.179f,2.0f));
							check_logan_wait_power = true;
							Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_POWER_02);
						}
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_POWER);
					}
				}
				break;
			}
		case(MTU_TIMER_LOGAN_WAIT_POWER_02):
			{
				Say_Something (obj, MTU_SPEECH_LOGAN_PREPARE_FINALE);
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ACTIVATE_FINALE, MTU_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MTU_TIMER_LOGAN_WAIT_AGT):
			{
				if (logan_wait_agt)
				{
					if (Get_Obj_Distance (obj, STAR) < 4.0f)
					{
						if (!check_wait_agt)
						{
							Say_Something (obj, MTU_SPEECH_LOGAN_PREPARE_INFANTRY);
							check_wait_agt = true;
						}
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_AGT);
					}
				}
				break;
			}
		case (MTU_TIMER_LOGAN_WAIT_INFANTRY):
			{
				if (logan_wait_infantry)
				{
					if (Get_Obj_Distance (obj, STAR) < 4.0f)
					{
						if (!check_wait_infantry)
						{
							Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1003I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
							Move_Somewhere (obj, MTU_MOVE_LOGAN_WEAPONS, MTU_ACTION_DEFAULT);
							check_wait_infantry = true;
						}
					}
					else
					{
						Commands->Start_Timer (obj, this, 1.0f, MTU_TIMER_LOGAN_WAIT_INFANTRY);
					}
				}
				break;
			}
		case (MTU_TIMER_LIEUTENANT_WAIT):
			{
				Say_Something (obj, MTU_SPEECH_LIEUTENANT_LETIN);
				break;
			}
		case (MTU_TIMER_ANOTHER_APACHE):
			{
				GameObject * apache = Commands->Create_Object ("NOD_Apache", Vector3(-52.440f,33.644f,-7.825f));
				if (apache)
				{
					Commands->Attach_Script (apache, "MTU_Nod_Apache", "0");
				}
				break;
			}
		case (MTU_TIMER_MCT_ATTACK):
			{
				break;
			}
		}
	}

	// This handles conversations with instructors and Havoc.

	void Say_Something (GameObject * obj, int speech_id)
	{
		const char *conv_name = ("IDS_MTU_CONVERSATION_01"); //DEFAULT
		Vector3 target_loc = Commands->Get_Position (obj);

		switch (speech_id)
		{

		// Logan Sheppard

		case (MTU_SPEECH_LOGAN_START):
			{
				conv_name = ("MTU_LOGAN_START");
				break;
			}
		case (MTU_SPEECH_LOGAN_CROUCH):
			{
				conv_name = ("MTU_LOGAN_CROUCH");
				break;
			}
		case (MTU_SPEECH_LOGAN_CROUCH_TEST):
			{
				conv_name = ("MTU_LOGAN_CROUCH_TEST");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_HEARD):
			{
				conv_name = ("MTU_LOGAN_HEARD");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_SNEAK_LOSE):
			{
				conv_name = ("MTU_LOGAN_SNEAK_LOSE");
				break;
			}
		case (MTU_SPEECH_LOGAN_SNEAK_WIN):
			{
				conv_name = ("MTU_LOGAN_SNEAK_WIN");
				break;
			}
		case (MTU_SPEECH_LOGAN_JUMP_TEST):
			{
				conv_name = ("MTU_LOGAN_JUMP_TEST");
				break;
			}
		case (MTU_SPEECH_LOGAN_EVA):
			{
				conv_name = ("MTU_LOGAN_EVA");
				break;
			}
		case (MTU_SPEECH_LOGAN_POKE):
			{
				conv_name = ("MTU_LOGAN_POKE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_COURSE_DONE):
			{
				conv_name = ("MTU_LOGAN_COURSE_DONE");
				break;
			}
		case (MTU_SPEECH_LOGAN_KEYCARDS):
			{
				conv_name = ("MTU_LOGAN_KEYCARDS");
				target_loc = Vector3(-12.776f,23.761f,-0.5f);
				break;
			}
		case (MTU_SPEECH_LOGAN_GO_INSIDE):
			{
				conv_name = ("MTU_LOGAN_GO_INSIDE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_INTRODUCE_BARRACKS):
			{
				conv_name = ("MTU_LOGAN_INTRODUCE_BARRACKS");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_INTRODUCE_WEAP_FACTORY):
			{
				conv_name = ("MTU_LOGAN_INTRODUCE_WEAP");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_WHATSNEXT):
			{
				conv_name = ("MTU_LOGAN_WHATSNEXT");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_INTRODUCE_REFINERY):
			{
				conv_name = ("MTU_LOGAN_INTRODUCE_REFINERY");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_PREPARE_POWER):
			{
				conv_name = ("MTU_LOGAN_PREPARE_POWER");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_INTRODUCE_POWER):
			{
				conv_name = ("MTU_LOGAN_INTRODUCE_POWER");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LOGAN_PREPARE_FINALE):
			{
				conv_name = ("MTU_LOGAN_OUTRO");
				break;
			}
		case (MTU_SPEECH_LOGAN_PREPARE_INFANTRY):
			{
				conv_name = ("MTU_LOGAN_PREPARE_INFANTRY");
				target_loc.X = 0;
				break;
			}

		// Sydney Mobius

		case (MTU_SPEECH_SYDNEY_START):
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Set_Objective_Status (MTU_OBJECTIVE_01, OBJECTIVE_STATUS_ACCOMPLISHED);
				conv_name = ("MTU_SYDNEY_START");
				break;
			}
		case (MTU_SPEECH_SYDNEY_HEALTH):
			{
				conv_name = ("MTU_SYDNEY_HEALTH");
				break;
			}
		case (MTU_SPEECH_SYDNEY_PICKUP):
			{
				conv_name = ("MTU_SYDNEY_PICKUP");
				break;
			}
		case (MTU_SPEECH_SYDNEY_ARMOR):
			{
				float health = Commands->Get_Max_Health (STAR);
				Commands->Set_Health (STAR, health);
				conv_name = ("MTU_SYDNEY_ARMOR");
				break;
			}
		case (MTU_SPEECH_SYDNEY_SHOOT_AGAIN):
			{
				float shield = Commands->Get_Max_Shield_Strength (STAR);
				Commands->Set_Shield_Strength (STAR, shield);
				conv_name = ("MTU_SYDNEY_SHOOT_AGAIN");
				break;
			}
		case (MTU_SPEECH_SYDNEY_LAST_TIME):
			{
				conv_name = ("MTU_SYDNEY_LAST_TIME");
				break;
			}
		case (MTU_SPEECH_SYDNEY_RADAR):
			{
				conv_name = ("MTU_SYDNEY_RADAR");
				target_loc.X = 0;
				break;
			}

		// Gunner

		case (MTU_SPEECH_GUNNER_START):
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Set_Objective_Status (MTU_OBJECTIVE_02, OBJECTIVE_STATUS_ACCOMPLISHED);
				conv_name = ("MTU_GUNNER_START");
				break;
			}
		case (MTU_SPEECH_GUNNER_RETICULE):
			{
				conv_name = ("MTU_GUNNER_RETICULE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_SNIPER_RIFLE):
			{
				conv_name = ("MTU_GUNNER_SNIPER_RIFLE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_AUTORIFLE):
			{
				conv_name = ("MTU_GUNNER_AUTORIFLE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_GRENADE):
			{
				conv_name = ("MTU_GUNNER_GRENADE");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_CHAINGUN):
			{
				conv_name = ("MTU_GUNNER_CHAINGUN");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_FLAMETHROWER):
			{
				conv_name = ("MTU_GUNNER_FLAMETHROWER");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_ROCKET):
			{
				conv_name = ("MTU_GUNNER_ROCKET");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_C4):
			{
				conv_name = ("MTU_GUNNER_C4");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_ION):
			{
				Commands->Add_Radar_Marker (MTU_RADAR_GUNNER, Vector3 (-34.91f,61.62f,2.60f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				conv_name = ("MTU_GUNNER_ION");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_MORE_TARGETS):
			{
				conv_name = ("MTU_GUNNER_MORE_TARGETS");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_GUNNER_ENDING):
			{
				Commands->Clear_Radar_Marker (MTU_RADAR_GUNNER);
				conv_name = ("MTU_GUNNER_ENDING");
				target_loc.X = 0;
				break;
			}

		//Hotwire

		case (MTU_SPEECH_HOTWIRE_INTRO):
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Set_Objective_Status (MTU_OBJECTIVE_03, OBJECTIVE_STATUS_ACCOMPLISHED);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_01, Vector3 (-32.12f,-48.05f,1.37f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_02, Vector3 (-26.36f,34.21f,2.0f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_03, Vector3 (47.16f,39.18f,1.51f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_04, Vector3 (30.77f,-26.61f,2.75f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				conv_name = ("MTU_HOTWIRE_INTRO");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_GO_OUT):
			{
				conv_name = ("MTU_HOTWIRE_GO_OUT");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_MOVEOUT):
			{
				conv_name = ("MTU_HOTWIRE_MOVEOUT");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_ACTION):
			{
				conv_name = ("MTU_HOTWIRE_ACTION");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_MEDTANK):
			{
				Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0395I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_01, Vector3 (-32.12f,-48.05f,1.37f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_02, Vector3 (-26.36f,34.21f,2.0f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_03, Vector3 (47.16f,39.18f,1.51f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_04, Vector3 (30.77f,-26.61f,2.75f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				conv_name = ("MTU_HOTWIRE_MEDTANK");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_UNFINISHED):
			{
				conv_name = ("MTU_HOTWIRE_UNFINISHED");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_SQUISH):
			{
				conv_name = ("MTU_HOTWIRE_SQUISH");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_HOTWIRE_BIKE):
			{
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_01, Vector3 (-32.12f,-48.05f,1.37f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_02, Vector3 (-26.36f,34.21f,2.0f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_03, Vector3 (47.16f,39.18f,1.51f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				Commands->Add_Radar_Marker (MTU_RADAR_COURSE_04, Vector3 (30.77f,-26.61f,2.75f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
				conv_name = ("MTU_HOTWIRE_BIKE");
				target_loc.X = 0;
				break;
			}

		// Mobius

		case (MTU_SPEECH_MOBIUS_REFINERY):
			{
				target_loc.X = 0;
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Set_Objective_Status (MTU_OBJECTIVE_04, OBJECTIVE_STATUS_ACCOMPLISHED);
				conv_name = ("MTU_MOBIUS_REFINERY");
				break;
			}

		// Petrova

		case (MTU_SPEECH_PETROVA_POWER):
			{
				target_loc.X = 0;
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Set_Objective_Status (MTU_OBJECTIVE_05, OBJECTIVE_STATUS_ACCOMPLISHED);
				conv_name = ("MTU_PETROVA_POWER");
				break;
			}
		case (MTU_SPEECH_PETROVA_POWER_END):
			{
				target_loc.X = 0;
				conv_name = ("MTU_PETROVA_POWER_END");
				break;
			}

		// Lieutenant Maus

		case (MTU_SPEECH_LIEUTENANT_START):
			{
				conv_name = ("MTU_LIEUTENANT_START_ASSAULT");
				break;
			}
		case (MTU_SPEECH_LIEUTENANT_LETIN):
			{
				conv_name = ("MTU_LIEUTENANT_LETIN");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LIEUTENANT_MCT):
			{
				conv_name = ("MTU_LIEUTENANT_MCT");
				target_loc.X = 0;
				break;
			}
		case (MTU_SPEECH_LIEUTENANT_AFTER):
			{
				conv_name = ("MTU_LIEUTENANT_AFTER");
				target_loc.X = 0;
				break;
			}
		}
		Commands->Stop_All_Conversations ();

		Vector3 my_pos = Commands->Get_Position(obj);
		Vector3 target_pos = Commands->Get_Position(STAR);
		float angle = WWMath::Atan2((my_pos.X - target_pos.X), (my_pos.Y - target_pos.Y));
		Commands->Set_Facing(obj, -90.0f - RAD_TO_DEGF(angle));

		if (target_loc.X != 0)
		{
			target_loc.Z += 1.5f;
			Commands->Force_Camera_Look (target_loc);
		}

		int conversation = Commands->Create_Conversation (conv_name, 100.0f, 300.0f, false);
		Commands->Join_Conversation(obj, conversation, false, true, true);
		Commands->Join_Conversation(STAR, conversation, false, false, false);
		Commands->Start_Conversation (conversation, speech_id);
		Commands->Monitor_Conversation (obj, conversation);
	}

	void Move_Somewhere (GameObject * obj, int destination_id, int action_id)
	{
		Vector3 destination = Vector3 (0,0,0);
		bool running = false;
		
		ActionParamsStruct params;
		params.Set_Basic(this, 100, action_id);
		params.WaypathID = 0;
		
		switch (destination_id)
		{
		case (MTU_MOVE_LOGAN_JUMP_TRAINING):
			{
				params.WaypathID = MTU_WAYPATH_LOGAN_JUMP_TRAINING;
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_EVA_TRAINING):
			{
				params.WaypathID = MTU_WAYPATH_LOGAN_EVA_TRAINING;
				break;
			}
		case (MTU_MOVE_LOGAN_COURSE_EXTERIOR):
			{
				params.WaypathID = MTU_WAYPATH_LOGAN_COURSE_EXTERIOR;
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_AGT):
			{
				destination = Vector3 (-13.094f,32.111f,1.023f);
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_INFANTRY):
			{
				destination = Vector3 (-18.648f,42.75f,1.0f);
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_WEAPONS):
			{
				destination = Vector3 (7.191f,-37.515f,0.0f);
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_REFINERY):
			{
				destination = Vector3 (22.689f,30.0f,0.0f);
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_POWER):
			{
				destination = Vector3 (-39.967f,36.628f,2.0f);
				running = true;
				break;
			}
		case (MTU_MOVE_LOGAN_EXIT):
			{
				destination = Vector3 (-51.787f,36.32f,2.0f);
				running = true;
				break;
			}
		case (MTU_MOVE_LIEUTENANT_HAVOC):
			{
				destination = Vector3 (-32.740f,36.773f,2.0f);
				running = true;
				break;
			}
		}
		if (params.WaypathID)
		{
			if (running)
			{
				params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
			}
			else
			{
				params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
			}
		}
		else
		{
			if (running)
			{
				params.Set_Movement(destination, 1.0f, 1.0f);
			}
			else
			{
				params.Set_Movement(destination, WALK, 1.0f);
			}
		}
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if ((complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)	|| (complete_reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED) || (complete_reason == ACTION_COMPLETE_CONVERSATION_UNABLE_TO_INIT))
		{
			switch (action_id)
			{

			// Logan Sheppard

			case (MTU_SPEECH_LOGAN_START): // Logan finished intro speech, open the first gate.
				{
					Commands->Set_HUD_Help_Text(IDS_M01DSGN_DSGN0515I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject * course_gate = Commands->Find_Object (MTU_GATE_01);
					if (course_gate)
					{
						Commands->Set_Animation (course_gate, "CHT_JAIL.CHT_JAIL", false, NULL, 0.0f, 2.0f, false);
					}
					course_gate = Commands->Find_Object (MTU_GATE_02);
					if (course_gate)
					{
						Commands->Set_Animation (course_gate, "CHT_JAIL.CHT_JAIL", false, NULL, 0.0f, 2.0f, false);
					}
					break;
				}
			case (MTU_SPEECH_LOGAN_CROUCH): // Logan finished introducing crouch and sneak. Turn on commando and start test.
				{
					logan_gave_sneak_speech = true;
					logan_heard_commando = false;
					Say_Something (obj, MTU_SPEECH_LOGAN_CROUCH_TEST);
					/*GameObject * course_gate = Commands->Find_Object (MTU_GATE_02);
					if (course_gate)
					{
						Commands->Set_Animation (course_gate, "CHT_JAIL.CHT_JAIL", false, NULL, 0.0f, 2.0f);
					}*/
					break;
				}
			case (MTU_SPEECH_LOGAN_CROUCH_TEST):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0382I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					break;
				}
			case (MTU_SPEECH_LOGAN_SNEAK_LOSE):
			case (MTU_SPEECH_LOGAN_SNEAK_WIN): // Logan gave results of sneak test, move to jump test location.
				{
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_DISABLE, 0.0f);
					Move_Somewhere (obj, MTU_MOVE_LOGAN_JUMP_TRAINING, MTU_ACTION_LOGAN_JUMP_TEST);
					break;
				}
			case (MTU_SPEECH_LOGAN_JUMP_TEST): // Logan finished giving jump test introduction, enable commando and move to next spot.
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0383I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					Move_Somewhere (obj, MTU_MOVE_LOGAN_EVA_TRAINING, MTU_ACTION_LOGAN_EVA_TRAIN);
					GameObject * course_gate = Commands->Find_Object (MTU_GATE_03);
					if (course_gate)
					{
						Commands->Set_Animation (course_gate, "CHT_JAIL.CHT_JAIL", false, NULL, 0.0f, 2.0f, false);
					}
					break;
				}
			case (MTU_SPEECH_LOGAN_EVA): // Logan finished introducing EVA, turn on first objective radar location.
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Set_Objective_Status (MTU_OBJECTIVE_01, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_Radar_Blip (MTU_OBJECTIVE_01, Vector3(-12.39f,20.78f,1.81f));
					Commands->Set_Objective_HUD_Info_Position (MTU_OBJECTIVE_01, 99, "POG_M00_1_01.tga", IDS_POG_LOCATE, Vector3 (-12.39f,20.78f,1.81f));
					Say_Something (obj, MTU_SPEECH_LOGAN_POKE);
					break;
				}
			case (MTU_SPEECH_LOGAN_POKE): // Logan finished explaining poke command, enable commando control.
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0385I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Select_Weapon (STAR, "Weapon_Pistol_Player");
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					break;
				}
			case (MTU_SPEECH_LOGAN_COURSE_DONE): // Logan finished telling the commando about the course. Move to the AGT.
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1003I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					Move_Somewhere (obj, MTU_MOVE_LOGAN_AGT, MTU_ACTION_LOGAN_GOTO_AGT);
					break;
				}
			case (MTU_SPEECH_LOGAN_KEYCARDS): // Logan finished introducing keycards, give the commando a green one and continue.
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0386I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Create_Object ("Level_01_Keycard", Vector3(-12.637f,24.426f,0.0f));
					Say_Something (obj, MTU_SPEECH_LOGAN_GO_INSIDE);
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					break;
				}
			case (MTU_SPEECH_LOGAN_WHATSNEXT): // Logan gave the Whats Next dialog.
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1003I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Set_Objective_Status (MTU_OBJECTIVE_04, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_Radar_Blip (MTU_OBJECTIVE_04, Vector3(24.40f,11.86f,2.60f));
					Commands->Set_Objective_HUD_Info_Position (MTU_OBJECTIVE_04, 99, "POG_M00_1_04.tga", IDS_POG_LOCATE, Vector3 (24.40f,11.86f,2.60f));
					Move_Somewhere (obj, MTU_MOVE_LOGAN_REFINERY, MTU_ACTION_DEFAULT);
					break;
				}
			case (MTU_SPEECH_LOGAN_PREPARE_POWER): // Logan is moving toward the power plant
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1003I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Set_Objective_Status (MTU_OBJECTIVE_05, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_Radar_Blip (MTU_OBJECTIVE_05, Vector3(-42.46f,19.45f,3.79f));
					Commands->Set_Objective_HUD_Info_Position (MTU_OBJECTIVE_05, 99, "POG_M00_1_05.tga", IDS_POG_LOCATE, Vector3 (-42.46f,19.45f,3.79f));
					Move_Somewhere (obj, MTU_MOVE_LOGAN_POWER, MTU_ACTION_DEFAULT);
					break;
				}
			case (MTU_SPEECH_LOGAN_PREPARE_INFANTRY):
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1003I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Move_Somewhere (obj, MTU_MOVE_LOGAN_INFANTRY, MTU_ACTION_DEFAULT);
					break;
				}
			case (MTU_SPEECH_LOGAN_PREPARE_FINALE):
				{
					Move_Somewhere (obj, MTU_MOVE_LOGAN_EXIT, MTU_ACTION_MOVE_LOGAN_EXIT);
					break;
				}

			// Sydney Mobius

			case (MTU_SPEECH_SYDNEY_START): // Sydney finished introducing HUD, shoot the commando.
				{
					ActionParamsStruct params;
					params.Set_Basic(this, 100, MTU_ACTION_DEFAULT);
					params.Set_Attack(STAR, 150.0f, 0.0f, true);
					Commands->Action_Attack(obj, params);
					break;
				}
			case (MTU_SPEECH_SYDNEY_HEALTH): // Sydney finished explaining about health loss. Create a health powerup.
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0387I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject* health_pow = Commands->Create_Object ("POW_Health_100", Vector3 (-11.975f,23.808f,-9.498f));
					if (health_pow)
					{
						int pow_id = Commands->Get_ID(health_pow);
						Commands->Attach_Script (health_pow, "MTU_PowerUp_Health", "");
						GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
						if (controller)
						{
							Commands->Send_Custom_Event(obj, controller, MTU_TYPE_HEALTH_POWERUP_ADD, pow_id, 0.0f);
						}
						Say_Something (obj, MTU_SPEECH_SYDNEY_PICKUP);
					}
					break;
				}
			case (MTU_SPEECH_SYDNEY_ARMOR):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0388I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject* armor_pow = Commands->Create_Object ("POW_Armor_100", Vector3 (-11.853f,25.720f,-9.498f));
					if (armor_pow)
					{
						int pow_id = Commands->Get_ID(armor_pow);
						Commands->Attach_Script (armor_pow, "MTU_PowerUp_Armor", "");
						GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
						if (controller)
						{
							Commands->Send_Custom_Event(obj, controller, MTU_TYPE_ARMOR_POWERUP_ADD, pow_id, 0.0f);
						}
						Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					}
					break;
				}
			case (MTU_SPEECH_SYDNEY_PICKUP):
				{
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					break;
				}
			case (MTU_SPEECH_SYDNEY_SHOOT_AGAIN):
				{
					ActionParamsStruct params;
					params.Set_Basic(this, 100, MTU_ACTION_DEFAULT);
					params.Set_Attack(STAR, 150.0f, 0.0f, true);
					Commands->Action_Attack(obj, params);
					break;
				}
			case (MTU_SPEECH_SYDNEY_LAST_TIME):
				{
					// Create a Nod Apache and fly it over the building.

					GameObject * apache = Commands->Create_Object ("NOD_Apache", Vector3(-52.440f,33.644f,-7.825f));
					if (apache)
					{
						Commands->Start_Timer (obj, this, 2.0f, MTU_TIMER_ANOTHER_APACHE);
						Commands->Attach_Script (apache, "MTU_Nod_Apache", "1");
					}
					break;
				}
			case (MTU_SPEECH_SYDNEY_RADAR):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0389I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					Commands->Set_Objective_Status (MTU_OBJECTIVE_02, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_Radar_Blip (MTU_OBJECTIVE_02, Vector3(-16.216f,72.693f,1.042f));
					Commands->Set_Objective_HUD_Info_Position (MTU_OBJECTIVE_02, 99, "POG_M00_1_02.tga", IDS_POG_LOCATE, Vector3 (-16.216f,72.693f,1.042f));
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_PREPARE_INFANTRY, 0.0f);
					}
					break;
				}

			// Gunner

			case (MTU_SPEECH_GUNNER_START):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0390I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Send_Custom_Event (obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_SETUP_HANDGUN, MTU_PARAM_DEFAULT, 0.0f);
					}
					break;
				}
			case (MTU_SPEECH_GUNNER_ENDING):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0389I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_PREPARE_WEAPONS, 0.0f);
					}
					Commands->Set_Objective_Status (MTU_OBJECTIVE_03, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_Radar_Blip (MTU_OBJECTIVE_03, Vector3(-0.687f,-18.630f,0.100f));
					Commands->Set_Objective_HUD_Info_Position (MTU_OBJECTIVE_03, 99, "POG_M00_1_03.tga", IDS_POG_LOCATE, Vector3 (-0.687f,-18.630f,0.100f));
					break;
				}

			// Hotwire

			case (MTU_SPEECH_HOTWIRE_INTRO):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0395I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_CREATE_HUMMVEE, MTU_PARAM_DEFAULT, 0.0f);
					}
					Say_Something (obj, MTU_SPEECH_HOTWIRE_GO_OUT);
					break;
				}
			case (MTU_SPEECH_HOTWIRE_GO_OUT):
				{
					GameObject * action_zone = Commands->Find_Object (MTU_ZONE_VEHICLE_APPROACHED);
					if (action_zone)
					{
						Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
					}
					action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_01);
					if (action_zone)
					{
						Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
					}
					action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_02);
					if (action_zone)
					{
						Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
					}
					action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_03);
					if (action_zone)
					{
						Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
					}
					action_zone = Commands->Find_Object (MTU_ZONE_CHECKPOINT_04);
					if (action_zone)
					{
						Commands->Send_Custom_Event (obj, action_zone, MTU_TYPE_RESET_TRIGGER_ONCE, MTU_PARAM_DEFAULT, 0.0f);
					}
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RESET_CHECKPOINT_COUNT, MTU_PARAM_DEFAULT, 0.0f);
					}
					break;
				}

			// Igantio Mobius

			case (MTU_SPEECH_MOBIUS_REFINERY):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0389I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					break;
				}

			// Petrova

			case (MTU_SPEECH_PETROVA_POWER):
				{
					GameObject * powerplant = Commands->Find_Object (MTU_POWERPLANT);
					if (powerplant)
					{
						Commands->Send_Custom_Event (obj, powerplant, MTU_TYPE_BUILDING_POWER_OFF, MTU_PARAM_DEFAULT, 0.0f);
					}
					Say_Something (obj, MTU_SPEECH_PETROVA_POWER_END);
					break;
				}
			case (MTU_SPEECH_PETROVA_POWER_END):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0389I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					GameObject * powerplant = Commands->Find_Object (MTU_POWERPLANT);
					if (powerplant)
					{
						Commands->Send_Custom_Event (obj, powerplant, MTU_TYPE_BUILDING_POWER_ON, MTU_PARAM_DEFAULT, 0.0f);
					}
					break;
				}

			// Lieutenant Maus

			case (MTU_SPEECH_LIEUTENANT_START):
				{
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR_FACING, 1, 0.0f);
					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(-16.836f,36.188f,1.341f));
					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X0I_Drop02.txt");
					}
					Commands->Start_Timer (obj, this, 18.0f, MTU_TIMER_LIEUTENANT_WAIT);
					break;
				}
			case (MTU_SPEECH_LIEUTENANT_LETIN):
				{
					Commands->Set_Position (STAR, Vector3(-16.382f,55.559f,-8.956f));
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR_FACING, 2, 0.0f);
					GameObject * engineer = Commands->Create_Object ("GDI_Engineer_0", Vector3(-24.003f,56.286f,-8.596f));
					if (engineer)
					{
						Commands->Innate_Disable (engineer);
						engineer_id = Commands->Get_ID (engineer);
						Commands->Set_Facing (engineer, 180.0f);
					}
					Commands->Start_Timer (obj, this, 5.0f, MTU_TIMER_MCT_ATTACK);
					Say_Something (obj, MTU_SPEECH_LIEUTENANT_MCT);
					break;
				}
			case (MTU_SPEECH_LIEUTENANT_MCT):
				{
					Commands->Set_Position (STAR, Vector3(-16.487f,30.453f,1.125f));
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR_FACING, 3, 0.0f);
					GameObject * barracks = Commands->Find_Object (MTU_BARRACKS);
					if (barracks)
					{
						Commands->Send_Custom_Event(obj, barracks, MTU_TYPE_BUILDING_DESTROY, MTU_PARAM_DEFAULT, 0.0f);
					}
					Commands->Send_Custom_Event(obj, STAR, MTU_TYPE_STAR, MTU_PARAM_CONTROL_ENABLE, 0.0f);
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_MOCK_INVASION, MTU_PARAM_DEFAULT, 0.0f);
					}
					Say_Something (obj, MTU_SPEECH_LIEUTENANT_AFTER);
					break;
				}
			case (MTU_SPEECH_LIEUTENANT_AFTER):
				{
					Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0398I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Set_Objective_Status (MTU_OBJECTIVE_06, OBJECTIVE_STATUS_PENDING);
					Commands->Set_Objective_HUD_Info (MTU_OBJECTIVE_06, 99, "POG_M00_1_06.tga", IDS_POG_ELIMINATE);
					ActionParamsStruct params;
					params.Set_Basic(this, 100, MTU_ACTION_LIEUTENANT_LEAVE);
					params.Set_Movement(Vector3(-63.745f,-38.599f,0.495f), WALK, 1.0f);
					Commands->Action_Goto(obj, params);
					break;
				}
			}
		}
		else
		{
			switch (action_id)
			{
			case (MTU_ACTION_LOGAN_JUMP_TEST): // Logan finished moving to jump test location, give speech.
				{
					Commands->Set_Position (obj, Vector3(-53.697f,-5.334f,2.0f));
					Say_Something (obj, MTU_SPEECH_LOGAN_JUMP_TEST);
					break;
				}
			case (MTU_ACTION_LOGAN_COURSE_DONE): // Logan reached the commando outside the course. Wrap it up and move on.
				{
					Commands->Set_Position (obj, Vector3(-35.01f,-15.658f,0.705f));
					Say_Something (obj, MTU_SPEECH_LOGAN_COURSE_DONE);
					break;
				}
			case (MTU_ACTION_MOVE_LOGAN_EXIT):
				{
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_LIEUTENANT_START, MTU_PARAM_DEFAULT, 0.0f);
					}
					Commands->Destroy_Object (obj);
					break;
				}
			case (MTU_ACTION_MOVE_LIEUTENANT_HAVOC):
				{
					Say_Something (obj, MTU_SPEECH_LIEUTENANT_START);
					break;
				}
			case (MTU_ACTION_LIEUTENANT_LEAVE):
				{
					GameObject * tower = Commands->Find_Object (MTU_TOWER);
					if (tower)
					{
						Commands->Send_Custom_Event (obj, tower, 1, 0, 0.0f);
					}
					Commands->Destroy_Object (obj);
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void Sound_Heard (GameObject * obj, const CombatSound & sound) override
	{
		int my_id = Commands->Get_ID (obj);

		if ((sound.Creator == STAR) && (my_id == MTU_LOGAN))
		{
			if ((!logan_told_about_hearing) && (logan_gave_sneak_speech))
			{
				//logan_told_about_hearing = true;
				//logan_heard_commando = true;
				Say_Something (obj, MTU_SPEECH_LOGAN_HEARD);
			}
		}
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		float maxhealth = Commands->Get_Max_Health (obj);
		Commands->Set_Health (obj, maxhealth);
	}
};


// MTU_Trigger_Zone - This script is used for all trigger zones throughout the tutorial mission.

DECLARE_SCRIPT (MTU_Trigger_Zone, "")
{
	bool trigger_once;
	bool gave_elevator_help;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (trigger_once, 1);
		SAVE_VARIABLE (gave_elevator_help, 2);
	}

	void Created (GameObject * /*obj*/) override
	{
		gave_elevator_help = false;
		trigger_once = true;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		int zone_id = Commands->Get_ID (obj);
		bool destroy_zone = false;
		int dual_zone = 0;

		switch (zone_id)
		{
		case (MTU_ZONE_JUMP_HUD_INFO):
			{
				Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0384I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				destroy_zone = true;
				break;
			}
		/*case (MTU_ZONE_TRIGGER_SNEAK_TRAINING):
			{
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_CROUCH);
				}
				destroy_zone = true;
				break;
			}*/
		case (MTU_ZONE_TRIGGER_JUMP_TRAINING):
			{
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_JUMP, 0.0f);
				}
				destroy_zone = true;
				break;
			}
		case (MTU_ZONE_TRIGGER_EVA_TRAINING):
			{
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_EVA, 0.0f);
				}
				destroy_zone = true;
				break;
			}
		case (MTU_ZONE_TRIGGER_MOVE_TO_AGT):
			{
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_COURSE_DONE, 0.0f);
				}
				destroy_zone = true;
				break;
			}
		case (MTU_ZONE_TRIGGER_KEYCARD_TRAINING):
			{
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_KEYCARD_TRAIN, 0.0f);
				}
				destroy_zone = true;
				break;
			}
		case (MTU_ZONE_RESET_SYDNEY):
			{
				if (!gave_elevator_help)
				{
					gave_elevator_help = true;
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1013I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				}
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ALL_POWERUPS_RESET, MTU_PARAM_DEFAULT, 0.0f);
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_SYDNEY_RESET, MTU_PARAM_DEFAULT, 0.0f);
				}
				GameObject * logan = Commands->Find_Object(MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_AGT_RESET, 0.0f);
				}
				Commands->Send_Custom_Event (obj, STAR, MTU_TYPE_STAR, MTU_PARAM_SYDNEY_SHOT_RESET, 0.0f);
				break;
			}
		case (MTU_ZONE_START_SYDNEY):
			{
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_SYDNEY_CHECK_RESET, MTU_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MTU_ZONE_INTRODUCE_BARRACKS):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * logan = Commands->Find_Object(MTU_LOGAN);
					if (logan)
					{
						if (Get_Obj_Distance (logan, STAR) < 10.0f)
						{
							Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_INTRO_INFANTRY, 0.0f);
						}
						else
						{
							Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_INFANTRY, 0.0f);
						}
					}
					destroy_zone = true;
				}
				break;
			}
		case (MTU_ZONE_RESET_GUNNER):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * gunner = Commands->Find_Object (MTU_GUNNER);
					if (gunner)
					{
						Commands->Send_Custom_Event (obj, gunner, MTU_TYPE_GUNNER, MTU_PARAM_GUNNER_RESET, 0.0f);
					}
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_CLEANUP_TARGETS, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_FORCE_RESET, MTU_PARAM_DEFAULT, 0.0f);
					}
					GameObject * logan = Commands->Find_Object(MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_INFANTRY, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_RESET_RANGE):
			{
				GameObject * gunner = Commands->Find_Object (MTU_GUNNER);
				if (gunner)
				{
					Commands->Send_Custom_Event (obj, gunner, MTU_TYPE_GUNNER, MTU_PARAM_SPEECH_RETICULE, 0.0f);
				}
				destroy_zone = true;
				break;
			}
		case (MTU_ZONE_GUNNER_NEXT_WEAPON):
			{
				GameObject * gunner = Commands->Find_Object (MTU_GUNNER);
				if (gunner)
				{
					Commands->Send_Custom_Event (obj, gunner, MTU_TYPE_GUNNER, MTU_PARAM_CHECK_TARGETS, 0.0f);
				}
				break;
			}
		case (MTU_ZONE_INTRODUCE_WEAPONS_FACTORY):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						if (Get_Obj_Distance (logan, STAR) < 4.0f)
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_WEAP_INTRO, 0.0f);
						}
						else
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_WEAPONS, 0.0f);
						}
					}
					destroy_zone = true;
				}
				break;
			}
		case (MTU_ZONE_RESET_HOTWIRE_01):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_ATTEMPT_RESET, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_RESET_HOTWIRE_02):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_ATTEMPT_RESET, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_START_HOTWIRE):
			{
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_HOTWIRE_START, MTU_PARAM_DEFAULT, 0.0f);
				}
				break;
			}
		case (MTU_ZONE_VEHICLE_APPROACHED):
			{
				if (!trigger_once)
				{
					Commands->Set_HUD_Help_Text(IDS_M02DSGN_DSGN0186I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					trigger_once = true;
					GameObject * hotwire = Commands->Find_Object (MTU_HOTWIRE);
					if (hotwire)
					{
						Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_ACTION, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_CHECKPOINT_01):
			{
				if (!trigger_once)
				{
					Commands->Clear_Radar_Marker (MTU_RADAR_COURSE_02);
					trigger_once = true;
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ADD_CHECKPOINT, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_CHECKPOINT_02):
			{
				if (!trigger_once)
				{
					Commands->Clear_Radar_Marker (MTU_RADAR_COURSE_03);
					trigger_once = true;
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ADD_CHECKPOINT, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_CHECKPOINT_03):
			{
				if (!trigger_once)
				{
					Commands->Clear_Radar_Marker (MTU_RADAR_COURSE_04);
					trigger_once = true;
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ADD_CHECKPOINT, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_CHECKPOINT_04):
			{
				if (!trigger_once)
				{
					Commands->Clear_Radar_Marker (MTU_RADAR_COURSE_01);
					trigger_once = true;
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_ADD_CHECKPOINT, MTU_PARAM_DEFAULT, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_INTRODUCE_REFINERY):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						if (Get_Obj_Distance (logan, STAR) < 4.0f)
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_INTRO_REFINERY, 0.0f);
						}
						else
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_REFINERY, 0.0f);
						}
					}
					destroy_zone = true;
				}
				break;
			}
		case (MTU_ZONE_RESET_IGNATIO):
			{
				if (!gave_elevator_help)
				{
					gave_elevator_help = true;
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1013I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				}
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_FORCE_RESET, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_MOBIUS_RESET, MTU_PARAM_DEFAULT, 0.0f);
					}
					GameObject * logan = Commands->Find_Object(MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_REFINERY, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_START_IGNATIO):
			{
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_MOBIUS_CHECK_RESET, MTU_PARAM_DEFAULT, 0.0f);
				}
				GameObject * logan = Commands->Find_Object (MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_PREPARE_POWER, 0.0f);
				}
				break;
			}
		case (MTU_ZONE_INTRODUCE_POWER_PLANT):
			{
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * logan = Commands->Find_Object (MTU_LOGAN);
					if (logan)
					{
						if (Get_Obj_Distance (logan, STAR) < 4.0f)
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_SPEECH_INTRO_POWER, 0.0f);
						}
						else
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_POWER, 0.0f);
						}
					}
					destroy_zone = true;
				}
				break;
			}
		case (MTU_ZONE_RESET_PETROVA):
			{
				if (!gave_elevator_help)
				{
					gave_elevator_help = true;
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1013I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				}
				bool green_key = Commands->Has_Key (enterer, 1);
				if (green_key)
				{
					GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
					if (controller)
					{
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_REMOVE_OBJECTIVES, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_FORCE_RESET, MTU_PARAM_DEFAULT, 0.0f);
						Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RESET_PETROVA, MTU_PARAM_DEFAULT, 0.0f);
					}
					GameObject * logan = Commands->Find_Object(MTU_LOGAN);
					if (logan)
					{
						Commands->Send_Custom_Event(obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_GOTO_POWER, 0.0f);
					}
				}
				break;
			}
		case (MTU_ZONE_START_PETROVA):
			{
				GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event (obj, controller, MTU_TYPE_PETROVA_CHECK_RESET, MTU_PARAM_DEFAULT, 0.0f);
				}
				GameObject * logan = Commands->Find_Object (MTU_LOGAN);
				if (logan)
				{
					Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_PREPARE_FINALE, 0.0f);
				}
				break;
			}
		default:
			{
				break;
			}
		}

		if (destroy_zone)
		{
			Commands->Destroy_Object (obj);
		}

		if (dual_zone)
		{
			GameObject * zone = Commands->Find_Object (dual_zone);
			if (zone)
			{
				Commands->Destroy_Object (zone);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MTU_TYPE_SYDNEY_IS_RESET)
		{
			float commando_max_health = Commands->Get_Max_Health (STAR);
			Commands->Set_Health (STAR, commando_max_health);
			Commands->Set_Shield_Strength (STAR, 0.0f);

			GameObject * sydney = Commands->Find_Object (MTU_SYDNEY);
			if (sydney)
			{
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_SYDNEY_START, 0.0f);
			}
		}
		if (type == MTU_TYPE_RESET_TRIGGER_ONCE)
		{
			trigger_once = false;
		}
	}
};


// MTU_GDI_Solder - This script is used for all GDI soldiers in the tutorial mission.

DECLARE_SCRIPT (MTU_GDI_Soldier, "")
{
	bool gate_guard_opened;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gate_guard_opened, 1);
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Hibernation (obj, false);
		Commands->Set_Loiters_Allowed (obj, false);
		Commands->Enable_Enemy_Seen (obj, false);
		gate_guard_opened = false;
		Commands->Innate_Disable (obj);
		int my_id = Commands->Get_ID (obj);
		if (my_id != MTU_GATE_GUARD)
		{
			Commands->Start_Timer (obj, this, 5.0f, MTU_TIMER_GDI_CONVERSATION);
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == MTU_TIMER_GDI_CONVERSATION)
		{
			Vector3 my_location = Commands->Get_Position (obj);
			Vector3 star_location = Commands->Get_Position (STAR);
			float distance = Commands->Get_Distance (my_location, star_location);
			if (distance < 20.0f)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, MTU_ACTION_DEFAULT);
				params.WaypathID = 400083;
				params.Set_Movement(obj, WALK, 1.0f);
				Commands->Action_Goto(obj, params);
			}
			else
			{
				Commands->Start_Timer (obj, this, 5.0f, MTU_TIMER_GDI_CONVERSATION);
			}
		}
		else if (timer_id == MTU_TIMER_POKE_DELAY)
		{
			Commands->Select_Weapon (STAR, NULL);
			gate_guard_opened = true;
			int conversation = Commands->Create_Conversation ("MTU_GDI_POKE", 100.0f, 300.0f, true);
			Commands->Join_Conversation(obj, conversation, false, true, true);
			Commands->Join_Conversation(STAR, conversation, false, true, true);
			Commands->Start_Conversation (conversation, MTU_SPEECH_GATEGUARD_POKE);
			Commands->Monitor_Conversation (obj, conversation);
		}
	}

	void Poked (GameObject * obj, GameObject * /*poker*/) override
	{
		int my_id = Commands->Get_ID (obj);
		if ((my_id == MTU_GATE_GUARD) && (!gate_guard_opened))
		{
			Commands->Start_Timer (obj, this, 1.5f, MTU_TIMER_POKE_DELAY);
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == MTU_SPEECH_GATEGUARD_POKE)
			{
				GameObject * course_gate = Commands->Find_Object (MTU_GATE_04);
				if (course_gate)
				{
					Commands->Set_Animation_Frame (course_gate, "CHT_JAIL.CHT_JAIL", 1);
					ActionParamsStruct params;
					params.Set_Basic(this, 100, MTU_ACTION_GATEGUARD_OUTSIDE);
					params.Set_Movement(Vector3(-20.148f,-19.637f,0.0f), WALK, 1.0f);
					Commands->Action_Goto(obj, params);
				}
			}
		}
		else
		{
			if (action_id == MTU_ACTION_GATEGUARD_OUTSIDE)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, MTU_ACTION_GATEGUARD_PATROL);
				params.WaypathID = 400083;
				params.Set_Movement(obj, WALK, 1.0f);
				Commands->Action_Goto(obj, params);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MTU_TYPE_GDI_SOLDIER_PATROL)
		{
			Commands->Action_Reset (obj, 100);
			Commands->Enable_Enemy_Seen (obj, true);
			Commands->Innate_Enable (obj);
		}
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		int my_id = Commands->Get_ID (obj);
		if (my_id == MTU_GATE_GUARD)
		{
			float health = Commands->Get_Max_Health (obj);
			Commands->Set_Health (obj, health);
		}
	}
};


// MTU_Commando - This script is attached to the commando by the Level Editor function.

DECLARE_SCRIPT (MTU_Commando, "")
{
	bool sydney_shot;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sydney_shot, 1);
	}

	void Created (GameObject * obj) override
	{
		Commands->Reveal_Encyclopedia_Weapon (14);
		Commands->Select_Weapon (obj, NULL);
		sydney_shot = false;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == MTU_TYPE_STAR)
		{
			switch (param)
			{
			case (MTU_PARAM_CONTROL_DISABLE): // The commando is having his control disabled.
				{
					Commands->Control_Enable (obj, false);
					break;
				}
			case (MTU_PARAM_CONTROL_ENABLE): // The commando is having is control returned.
				{
					Commands->Control_Enable (obj, true);
					ActionParamsStruct params;
					params.Set_Basic (this, 100, 0);
					Commands->Action_Follow_Input (obj, params);
					break;
				}
			case (MTU_PARAM_SYDNEY_SHOT_RESET):
				{
					sydney_shot = false;
					break;
				}
			}
		}
		else if (type == MTU_TYPE_STAR_FACING)
		{
			switch (param)
			{
			case (1):
				{
					Commands->Force_Camera_Look (Vector3(-15.329f,37.597f,3.074f));
					break;
				}
			case (2):
				{
					Commands->Start_Timer (obj, this, 0.1f, MTU_TIMER_COMMANDO_CAMERA_01);
					break;
				}
			case (3):
				{
					Commands->Start_Timer (obj, this, 0.1f, MTU_TIMER_COMMANDO_CAMERA_02);
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void Timer_Expired (GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == MTU_TIMER_COMMANDO_CAMERA_01)
		{
			Commands->Force_Camera_Look (Vector3(-25.079f,56.451f,-7.670f));
		}
		else if (timer_id == MTU_TIMER_COMMANDO_CAMERA_02)
		{
			Commands->Force_Camera_Look (Vector3(-16.330f,48.791f,5.461f));
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		float commandohealth = Commands->Get_Health (STAR);
		float commandomax = (Commands->Get_Max_Health (STAR) / 4.0f);
		if (commandohealth < commandomax)
		{
			Commands->Set_Health (STAR, commandomax);
		}

		GameObject * sydney = Commands->Find_Object (MTU_SYDNEY);
		if (damager == sydney)
		{
			if (!sydney_shot)
			{
				Commands->Set_Health (obj, 25.0f);
				sydney_shot = true;
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_HEALTH, 0.0f);
			}
			else
			{
				Commands->Set_Health (obj, 50.0f);
				Commands->Set_Shield_Strength (obj, 50.0f);
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_LAST_TIME, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MTU_Commando_Startup, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Attach_Script (obj, "MTU_Commando", "");
	}
};


DECLARE_SCRIPT (MTU_PowerUp_Health, "")
{
	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_POWERUP_GRANTED)
		{
			GameObject * sydney = Commands->Find_Object (MTU_SYDNEY);
			if (sydney)
			{
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_PICKUP, 0.0f);
			}
			GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event(obj, controller, MTU_TYPE_HEALTH_POWERUP_SUBTRACT, 0, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MTU_PowerUp_Armor, "")
{
	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_POWERUP_GRANTED)
		{
			GameObject * sydney = Commands->Find_Object (MTU_SYDNEY);
			if (sydney)
			{
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_SHOOT_AGAIN, 0.0f);
			}
			GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event(obj, controller, MTU_TYPE_ARMOR_POWERUP_SUBTRACT, 0, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MTU_Nod_Apache, "Apache_ID:int")
{
	void Created (GameObject * obj) override
	{
		Commands->Disable_Physical_Collisions (obj);
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 0);
		params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
		params.WaypathID = MTU_WAYPATH_APACHE_FLYOVER;
		params.WaypathSplined = true;
		Commands->Action_Goto(obj, params);
		int apache_id = Get_Int_Parameter ("Apache_ID");
		if (apache_id)
		{
			Commands->Start_Timer(obj, this, 4.0f, MTU_TIMER_SYDNEY_CONTINUE);
		}
		Commands->Start_Timer(obj, this, 10.0f, MTU_TIMER_APACHE_DESTROY);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == MTU_TIMER_APACHE_DESTROY)
		{
			Commands->Destroy_Object (obj);
		}
		else if (timer_id == MTU_TIMER_SYDNEY_CONTINUE)
		{
			GameObject * sydney = Commands->Find_Object (MTU_SYDNEY);
			if (sydney)
			{
				Commands->Send_Custom_Event(obj, sydney, MTU_TYPE_SYDNEY, MTU_PARAM_SPEECH_RADAR, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MTU_Range_Target, "Target_ID:int")
{
	void Created (GameObject * obj) override
	{
		Commands->Attach_Script (obj, "M00_Soldier_Powerup_Disable", "");
		Commands->Innate_Disable (obj);
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		Vector3 myloc = Commands->Get_Position (obj);
		GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
		if (controller)
		{
			int param = Get_Int_Parameter ("Target_ID");
			Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_TARGET_DESTROYED, param, 0.0f);
		}
	}
};


DECLARE_SCRIPT (MTU_Range_Target_Path_Mid, "")
{
	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = 400105;
		Commands->Action_Goto(obj, params);
	}
};


DECLARE_SCRIPT (MTU_Range_Target_Path_Right, "")
{
	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = 400115;
		Commands->Action_Goto(obj, params);
	}
};


DECLARE_SCRIPT (MTU_Range_Target_Path_Left, "")
{
	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = 400129;
		Commands->Action_Goto(obj, params);
	}
};


DECLARE_SCRIPT (MTU_Range_Target_Miss_Commando, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen (obj, true);
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		if (enemy == STAR)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, 0);
			params.Set_Attack(enemy, 150.0f, 5.0f, true);
			Commands->Action_Attack(obj, params);
		}
	}
};


DECLARE_SCRIPT (MTU_Range_Powerup, "Powerup_ID:int")
{
	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_POWERUP_GRANTED)
		{
			GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
			if (controller)
			{
				int int_param = Get_Int_Parameter ("Powerup_ID");
				Commands->Send_Custom_Event (obj, controller, MTU_TYPE_RANGE_POWERUP_RETRIEVED, int_param, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (MTU_GDI_Vehicle, "Vehicle_ID:int")
{
	bool entered_me;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered_me, 1);
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type (obj, SCRIPT_PLAYERTYPE_NEUTRAL);
		entered_me = false;
	}

	void Killed (GameObject * obj, GameObject * killer) override
	{
		GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
		GameObject * hotwire_obj = Commands->Find_Object (MTU_HOTWIRE);
		if ((controller) && (killer != hotwire_obj))
		{
			int my_id = Get_Int_Parameter ("Vehicle_ID");
			if (my_id == 1)
			{
				Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_CREATE_HUMMVEE, MTU_PARAM_DEFAULT, 0.0f);
			}
			else
			{
				Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_CREATE_MEDTANK, MTU_PARAM_DEFAULT, 0.0f);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_VEHICLE_ENTERED)
		{
			if (!entered_me)
			{
				entered_me = true;
				int my_id = Get_Int_Parameter ("Vehicle_ID");
				switch (my_id)
				{
				case (1):
					{
						Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0396I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
						GameObject * hotwire = Commands->Find_Object (MTU_HOTWIRE);
						if (hotwire)
						{
							Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_MOVEOUT, 0.0f);
						}
						break;
					}
				case (2):
					{
						Commands->Set_HUD_Help_Text(IDS_MTUDSGN_DSGN0389I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
						GameObject * hotwire = Commands->Find_Object (MTU_HOTWIRE);
						if (hotwire)
						{
							Commands->Send_Custom_Event (obj, hotwire, MTU_TYPE_HOTWIRE, MTU_PARAM_SPEECH_WEAP_SQUISH, 0.0f);
						}
						GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, MTU_TYPE_WEAP_CREATE_SQUISHIES, MTU_PARAM_DEFAULT, 0.0f);
						}

						// Setup Logan to intro the next part of the course - Tiberium Refinery.
						
						GameObject * logan = Commands->Find_Object (MTU_LOGAN);
						if (logan)
						{
							Commands->Send_Custom_Event (obj, logan, MTU_TYPE_LOGAN, MTU_PARAM_ACTION_PREPARE_REFINERY, 0.0f);
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
	}
};


DECLARE_SCRIPT (MTU_Building_Controller, "Building_ID:int")
{
	bool can_be_damaged;

	void Created (GameObject * /*obj*/) override
	{
		can_be_damaged = false;
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		int id = Commands->Get_ID (obj);
		if ((id == MTU_BARRACKS) || (id == MTU_POWERPLANT))
		{
			if (!can_be_damaged)
			{
				float max_health = Commands->Get_Max_Health (obj);
				Commands->Set_Health (obj, max_health);
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MTU_TYPE_BUILDING_DAMAGEABLE)
		{
			can_be_damaged = true;
		}
		else if (type == MTU_TYPE_BUILDING_POWER_OFF)
		{
			Commands->Set_Building_Power (obj, false);
		}
		else if (type == MTU_TYPE_BUILDING_POWER_ON)
		{
			Commands->Set_Building_Power (obj, true);
		}
		else if (type == MTU_TYPE_BUILDING_DESTROY)
		{
			can_be_damaged = true;
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
		}
	}
};


DECLARE_SCRIPT (MTU_Nod_Soldier, "Soldier_ID:int")
{
	void Created (GameObject * obj) override
	{
		Commands->Grant_Key (obj, 1, true);
		int soldier_id = Get_Int_Parameter ("Soldier_ID");
		if (soldier_id == 1)
		{
			Commands->Innate_Disable (obj);
			ActionParamsStruct params;
			params.Set_Basic(this, 100, MTU_ACTION_DEFAULT);
			params.Set_Movement(Vector3(-12.07f,47.084f,1.042f), RUN, 2.0f);
			Commands->Action_Goto(obj, params);
			Commands->Start_Timer (obj, this, 6.0f, MTU_TIMER_NOD_SOLDIER_REMOVAL);
		}
		else if (!soldier_id)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 30, MTU_ACTION_DEFAULT);
			params.Set_Movement(STAR, RUN, 2.0f);
			Commands->Action_Goto(obj, params);
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		int soldier_id = Get_Int_Parameter ("Soldier_ID");
		if ((damager != STAR) && (soldier_id == 2))
		{
			Commands->Set_Health (obj, 10.0f);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		int soldier_id = Get_Int_Parameter ("Soldier_ID");
		if (!soldier_id)
		{
			GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, MTU_TYPE_TRIGGER_SPAWNER, MTU_PARAM_DEFAULT, 0.0f);
			}
		}
		else if (soldier_id == 2)
		{
			GameObject * controller = Commands->Find_Object (MTU_CONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, MTU_TYPE_COUNT_OFFICERS, MTU_PARAM_DEFAULT, 0.0f);
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == MTU_TIMER_NOD_SOLDIER_REMOVAL)
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
	}
};


DECLARE_SCRIPT (MTU_Flyover, "Vehicle_ID:int")
{
	void Created (GameObject * obj) override
	{
		int vehicle_id = Get_Int_Parameter ("Vehicle_ID");
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 0);
		params.WaypathID = 0;
		if (vehicle_id)
		{
			params.Set_Movement(Vector3 (-185.13f,3.50f,51.21f), RUN, 5.0f);
		}
		else
		{
			params.Set_Movement(Vector3 (-37.96f,-169.71f,41.45f), RUN, 5.0f);
		}
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete (GameObject * obj, int /*action_id*/, ActionCompleteReason /*complete_reason*/) override
	{
		Commands->Destroy_Object (obj);
	}
};


// This is a controller script for the multiplayer skirmish tutorial.

// On created, force all the bots to appear.
// If a bot sends a custom saying it has just died, recreate the bot by triggering the spawner.


DECLARE_SCRIPT (MSK_Controller, "")
{
	void Created (GameObject * obj) override
	{
		int conversation = Commands->Create_Conversation ("MSK_MP_STARTUP_DESC", 100.0f, 300.0f, false);
		Commands->Join_Conversation(obj, conversation, false, false, false);
		Commands->Start_Conversation (conversation, 1);

		Commands->Set_HUD_Help_Text(IDS_MP_STARTUP_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);

		Commands->Add_Radar_Marker (300204, Vector3(-148.55f, -53.50f, -2.44f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300205, Vector3(-130.60f, -23.70f, -1.33f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300206, Vector3(-172.46f, -18.81f, -2.22f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300207, Vector3(-142.41f, -63.27f, -1.28f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300208, Vector3(-171.69f, -42.64f, -1.67f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300209, Vector3(-138.67f, -80.16f, -1.29f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300216, Vector3(-138.72f, -38.65f, -2.11f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300219, Vector3(-147.80f, -36.11f, -1.95f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300220, Vector3(-42.57f, 53.47f, 1.32f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300221, Vector3(-63.18f, 61.87f, -0.89f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300222, Vector3(-43.25f, 130.42f, -1.98f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300223, Vector3(-45.95f, 45.55f, 0.59f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300224, Vector3(-21.88f, 86.67f, -2.14f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300225, Vector3(-26.22f, 43.53f, -2.12f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300226, Vector3(-46.19f, 77.10f, 1.31f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);
		Commands->Add_Radar_Marker (300230, Vector3(-45.71f, 62.55f, -2.55f), RADAR_BLIP_SHAPE_OBJECTIVE, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE);

		Commands->Trigger_Spawner(MSK_GDI_DEF_01);
		Commands->Trigger_Spawner(MSK_GDI_DEF_02);
		Commands->Trigger_Spawner(MSK_GDI_DEF_03);
		Commands->Trigger_Spawner(MSK_GDI_DEF_04);
		Commands->Trigger_Spawner(MSK_NOD_DEF_01);
		Commands->Trigger_Spawner(MSK_NOD_DEF_02);
		Commands->Trigger_Spawner(MSK_NOD_DEF_03);
		Commands->Trigger_Spawner(MSK_NOD_DEF_04);

		Commands->Trigger_Spawner(MSK_GDI_ATK_01);
		Commands->Trigger_Spawner(MSK_GDI_ATK_02);
		Commands->Trigger_Spawner(MSK_GDI_ATK_03);
		Commands->Trigger_Spawner(MSK_NOD_ATK_01);
		Commands->Trigger_Spawner(MSK_NOD_ATK_02);
		Commands->Trigger_Spawner(MSK_NOD_ATK_03);
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == MSK_SOLDIER_DEAD)
		{
			Commands->Trigger_Spawner(param);
		}
	}
};


// This is a script for the innate soldiers in the multiplayer skirmish tutorial.


DECLARE_SCRIPT (MSK_Soldier, "Spawner_ID:int")
{
	int spawner_id;

	void Created (GameObject * obj) override
	{
		spawner_id = Get_Int_Parameter ("Spawner_ID");
		Commands->Start_Timer (obj, this, 2.0f, MSK_TIMER_02);
		Commands->Enable_Innate_Conversations (obj, false);
		Commands->Enable_Hibernation (obj, false);
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == MSK_TIMER_02)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 35, 2);
			switch (spawner_id)
			{
			case (MSK_GDI_DEF_01):
				{
					params.Set_Movement(Vector3(-182.345f, -27.856f, -3.270f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_DEF_02):
				{
					params.Set_Movement(Vector3(-143.940f, -40.531f, -3.277f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_DEF_03):
				{
					params.Set_Movement(Vector3(-169.060f, -16.929f, -3.320f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_DEF_04):
				{
					params.Set_Movement(Vector3(-131.300f, -24.240f, -3.353f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_ATK_01):
				{
					params.Set_Movement(Vector3(-121.312f, -67.623f, -2.948f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_ATK_02):
				{
					params.Set_Movement(Vector3(-126.934f, -72.430f, -2.966f), RUN, 5.0f);
					break;
				}
			case (MSK_GDI_ATK_03):
				{
					params.Set_Movement(Vector3(-112.535f, -45.235f, -3.317f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_DEF_01):
				{
					params.Set_Movement(Vector3(-44.969f, 62.217f, -3.368f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_DEF_02):
				{
					params.Set_Movement(Vector3(-0.394f, 69.853f, -3.570f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_DEF_03):
				{
					params.Set_Movement(Vector3(-95.266f, 62.548f, -3.498f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_DEF_04):
				{
					params.Set_Movement(Vector3(-58.992f, 131.231f, -4.208f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_ATK_01):
				{
					params.Set_Movement(Vector3(-29.350f, 46.417f, -3.428f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_ATK_02):
				{
					params.Set_Movement(Vector3(-8.963f, 43.646f, -3.478f), RUN, 5.0f);
					break;
				}
			case (MSK_NOD_ATK_03):
				{
					params.Set_Movement(Vector3(-29.370f, 43.768f, -3.473f), RUN, 5.0f);
					break;
				}
			default:
				{
					break;
				}
			}
			Commands->Action_Goto(obj, params);
		}
		else if (timer_id == MSK_TIMER)
		{
			ActionParamsStruct params;
			if (Get_Int_Random (0,7))
			{
				params.Set_Basic(this, 75, 1);
			}
			else
			{
				params.Set_Basic(this, 40, 1);
			}
			params.Set_Movement(Vector3(0,0,0), RUN, 5.0f);
			params.WaypathID = 0;

			switch (spawner_id)
			{
			case (MSK_GDI_DEF_01):
				{
					params.WaypathID = MSK_GDI_DEF_PATH_01;
					break;
				}
			case (MSK_GDI_DEF_02):
				{
					params.WaypathID = MSK_GDI_DEF_PATH_02;
					break;
				}
			case (MSK_GDI_DEF_03):
				{
					params.WaypathID = MSK_GDI_DEF_PATH_03;
					break;
				}
			case (MSK_GDI_DEF_04):
				{
					params.WaypathID = MSK_GDI_DEF_PATH_04;
					break;
				}
			case (MSK_GDI_ATK_01):
				{
					params.WaypathID = MSK_GDI_ATK_PATH_01;
					break;
				}
			case (MSK_GDI_ATK_02):
				{
					params.WaypathID = MSK_GDI_ATK_PATH_02;
					break;
				}
			case (MSK_GDI_ATK_03):
				{
					params.WaypathID = MSK_GDI_ATK_PATH_03;
					break;
				}
			case (MSK_NOD_DEF_01):
				{
					params.WaypathID = MSK_NOD_DEF_PATH_01;
					break;
				}
			case (MSK_NOD_DEF_02):
				{
					params.WaypathID = MSK_NOD_DEF_PATH_02;
					break;
				}
			case (MSK_NOD_DEF_03):
				{
					params.WaypathID = MSK_NOD_DEF_PATH_03;
					break;
				}
			case (MSK_NOD_DEF_04):
				{
					params.WaypathID = MSK_NOD_DEF_PATH_04;
					break;
				}
			case (MSK_NOD_ATK_01):
				{
					params.WaypathID = MSK_NOD_ATK_PATH_01;
					break;
				}
			case (MSK_NOD_ATK_02):
				{
					params.WaypathID = MSK_NOD_ATK_PATH_02;
					break;
				}
			case (MSK_NOD_ATK_03):
				{
					params.WaypathID = MSK_NOD_ATK_PATH_03;
					break;
				}
			default:
				{
					break;
				}
			}
			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if ((action_id == 2) && (complete_reason == ACTION_COMPLETE_NORMAL))
		{
			Commands->Start_Timer (obj, this, 0.1f, MSK_TIMER);
		}
		else
		{
			Commands->Start_Timer (obj, this, 0.1f, MSK_TIMER_02);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject * controller = Commands->Find_Object (MSK_CONTROLLER);
		if (controller)
		{
			Commands->Send_Custom_Event (obj, controller, MSK_SOLDIER_DEAD, spawner_id, 20.0f);
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		int	player_type = Commands->Get_Player_Type (obj);
		int	damager_type = Commands->Get_Player_Type (damager);

		if (player_type != damager_type)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 76, 1);
			params.Set_Attack (damager, 300.0f, 0.0f, true);
			Commands->Action_Attack (obj, params);
		}
	}
};


DECLARE_SCRIPT (MSK_Info_Zone, "")
{
	bool was_entered;
	bool first_message;

	void Created (GameObject * /*obj*/) override
	{
		was_entered = false;
		first_message = false;
	}

	void Entered (GameObject * obj, GameObject * /*enterer*/) override
	{
		if (!was_entered)
		{
			const char *conv_name = (""); //DEFAULT
			int zone_id = Commands->Get_ID (obj);
			Commands->Clear_Radar_Marker (zone_id);
			switch (zone_id)
			{
			case (300204):
				{
					Commands->Force_Camera_Look (Vector3(-151.38f, -53.24f, -1.89f));
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_GDI");
					was_entered = true;
					break;
				}
			case (300205):
				{
					Commands->Force_Camera_Look (Vector3(-131.16f, -15.65f, 1.42f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_PowerPlant_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_POWER_DESC");
					was_entered = true;
					break;
				}
			case (300206):
				{
					Commands->Force_Camera_Look (Vector3(-178.36f, -4.89f, 4.02f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_TiberiumRefinery_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_REFINERY_DESC");
					was_entered = true;
					break;
				}
			case (300207):
				{
					Commands->Force_Camera_Look (Vector3(-142.59f, -59.16f, 0.86f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_InfantryBarracks_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_BARRACKS_DESC");
					was_entered = true;
					break;
				}
			case (300208):
				{
					Commands->Force_Camera_Look (Vector3(-175.71f, -45.22f, 1.45f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_WeaponsFactory_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_WEAPONS_DESC");
					was_entered = true;
					break;
				}
			case (300209):
				{
					Commands->Force_Camera_Look (Vector3(-139.77f, -83.85f, 4.71f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_AdvancedGuardTower_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_AGT_DESC");
					was_entered = true;
					break;
				}
			case (300210):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300211):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300212):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300213):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300214):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300215):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_DEFENSES_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300216):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_NUKE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_NUKE_NOD");
					break;
				}
			case (300217):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_VEHICLE_DESC");
					break;
				}
			case (300218):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_HARVESTER_DESC");
					break;
				}
			case (300219):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-149.33f, -38.15f, -1.99f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_MCT_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_MCT_DESC");
					break;
				}
			case (300220):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-40.03f, 53.96f, 2.23f));
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_NOD");
					break;
				}
			case (300221):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-79.35f, 62.74f, 5.56f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_PowerPlant_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_POWER_DESC");
					break;
				}
			case (300222):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-49.14f, 144.49f, 4.40f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_TiberiumRefinery_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_REFINERY_DESC");
					break;
				}
			case (300223):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-45.95f, 51.97f, 9.28f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_Nod_HandOfNod_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_HAND_DESC");
					break;
				}
			case (300224):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-9.24f, 78.92f, 3.32f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_Nod_Airstrip_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_AIRSTRIP_DESC");
					break;
				}
			case (300225):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-18.51f, 43.76f, 11.13f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_Nod_Obelisk_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_OBELISK_DESC");
					break;
				}
			case (300226):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_ION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_ION_GDI");
					break;
				}
			case (300227):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_VEHICLE_DESC");
					break;
				}
			case (300228):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_HARVESTER_DESC");
					break;
				}
			case (300229):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_CAUTION_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_TIBERIUM_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300230):
				{
					was_entered = true;
					Commands->Force_Camera_Look (Vector3(-45.90f, 59.84f, -2.18f));
					Commands->Set_HUD_Help_Text(IDS_Enc_Struct_GDI_MCT_Name, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_MCT_DESC");
					break;
				}
			case (300231):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_SNIPER_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_SNIPER_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300232):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_SNIPER_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_SNIPER_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300233):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_SNIPER_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_SNIPER_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300234):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_TUNNEL_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_TUNNEL_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300235):
				{
					if (!first_message)
					{
						Commands->Set_HUD_Help_Text(IDS_MP_TUNNEL_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
						conv_name = ("MSK_MP_TUNNEL_DESC");
						first_message = true;
					}
					else
					{
						switch (Get_Int_Random(0,3))
						{
						case (0):
							{
								conv_name = ("MSK_MP_STRATEGY_01");
								break;
							}
						case (1):
							{
								conv_name = ("MSK_MP_STRATEGY_02");
								break;
							}
						case (2):
							{
								conv_name = ("MSK_MP_STRATEGY_03");
								break;
							}
						default:
							{
								conv_name = ("MSK_MP_STRATEGY_04");
								break;
							}
						}
					}
					break;
				}
			case (300236):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_NOD");
					break;
				}
			case (300237):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_NOD");
					break;
				}
			case (300238):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_NOD");
					break;
				}
			case (300239):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_NOD");
					break;
				}
			case (300240):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_GDI");
					break;
				}
			case (300241):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_GDI");
					break;
				}
			case (300242):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_GDI");
					break;
				}
			case (300243):
				{
					was_entered = true;
					Commands->Set_HUD_Help_Text(IDS_MP_PURCHASE_HUD, TEXT_COLOR_OBJECTIVE_PRIMARY);
					conv_name = ("MSK_MP_PURCHASE_GDI");
					break;
				}
			default:
				{
					break;
				}
			}
			int conversation = Commands->Create_Conversation (conv_name, 100.0f, 300.0f, false);
			Commands->Join_Conversation(obj, conversation, false, false, false);
			Commands->Start_Conversation (conversation, 1);
		}
	}
};
