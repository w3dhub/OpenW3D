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
*     Mission01.cpp
*
* DESCRIPTION
*     Mission 1 scripts
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Joe_g $
*     $Revision: 268 $
*     $Modtime: 1/23/02 12:00p $
*     $Archive: /Commando/Code/Scripts/Mission01.cpp $
*
******************************************************************************/

#include "scripts.h"
#include <string.h>
#include "Toolkit.h"
#include "Mission1.h"
#include <cstdint>

namespace M01Vars
{
	static constexpr const char* Powerups[2] =
	{
		"POW_Health_100",
		"POW_Armor_100",
	};
}

DECLARE_SCRIPT(M01_Mission_Controller_JDG, "")//this guys ID number is 100376
{
	typedef enum {
		IDLE,
		ACTIVE,
		UNDER_ATTACK,
		DESTROYED,
	} M01_Building_State;

	typedef enum {
		LANDING_BEACH,
		INSIDE_GDI_CON,
		AT_GDI_BASE,
		INSIDE_GDI_POWERPLANT,
		BY_HAND_OF_NOD,
		INSIDE_HAND_OF_NOD,
		BY_REFINERY,
		INSIDE_REFINERY,
		BY_COMM_CENTER,
		INSIDE_COMM_CENTER,
		BY_TUNNEL,
		INSIDE_TUNNEL,
		BY_WATERFALL,
		INSIDE_WATERFALL,
		AT_SNIPER_SCENARIO,
	} M01_Location;

	bool is_startup;
	bool first_time_in_handofnod;
	bool first_time_in_hon_gruntlevel;
	bool first_time_in_hon_dojolevel;
	bool first_time_poking_gate_lock;
	bool players_first_time_by_commcenter;
	bool first_time_at_barn;
	bool gate_objective_done;
	bool church_guard_is_dead;
	bool loveshack_guard_is_dead;
	bool player_has_an_escort;
	bool clergy_are_boarding;
	bool church_pickup_chinook_is_on_its_way;
	bool player_has_unlocked_pen;
	bool radar_is_scrambled;
	bool unlock_gate_objective_active;
	bool commcenter_sam_objective_active;
	bool prisoners_are_freed;
	bool m01_hon_spawned_guy_01_alive;
	bool m01_hon_spawned_guy_02_alive;
	bool m01_hon_spawned_guy_03_alive;
	bool turret_01_is_dead;
	bool turret_02_is_dead;
	bool medium_tank_spawned;
	bool barn_prisoner_01_is_dead;
	bool barn_prisoner_02_is_dead;
	bool barn_prisoner_03_is_dead;
	bool barn_objective_active;
	bool kane_and_no2_conv01_active;
	bool kane_and_no2_conv02_active;
	bool kane_and_havoc_conv_active;
	bool m01_bridgeWeather_is_Set;
	bool m01_nodBase_weather_is_Set;
	bool missionStart;
	bool barn_objective_failed;
	bool barn_announcement_active;
	bool barn_objective_ended;
	bool turret_objective_over;
	bool final_conv_played;

	int m01_church_interior_nun02_ID;
	int m01_whack_a_mole_guy_ID;
	int m01_hon_spawned_guy_01_ID;
	int m01_hon_spawned_guy_02_ID;
	int m01_hon_spawned_guy_03_ID;
	int m01_medium_tank_ID;
	int m01_hon_spawners_tally;
	int m01_hon_total_spawners_tally;
	int m01_HON_SpawnRate;
	int new_turretbeach_engineers;
	int kane_ID;
	int eva_comm_center_line_01;
	int eva_comm_center_line_02;
	int eva_comm_center_line_03;
	int eva_comm_center_line_04;
	int eva_comm_center_line_05;
	int billy_conv;
	int babushka_conv;
	int start_conv;
	int start_conv2;
	int kane_and_no2_conv01;
	int kane_and_no2_conv02;
	int kane_and_havoc_conv;
	int lockes_hon_mct_conv1;
	int lockes_hon_mct_conv2;
	int barnEvac_conv;
	int barnfailed_conv;
	int civ3_conv;
	int missionIntro;
	int endMission_conv;
	int unlocked_gate_conv;
	int turrets_conv;
	int players_escorts_ID;
	int havocs_targets_ID;
	int m01_hon_chinook_guys_killed;
	int dead_barn_guard_tally;
	int dead_barn_prisoner_tally;

	//new stuff
	bool firstNodCommanderKilled;
	int loveshack_conv;
	bool loveshack_conv_playing;
	int priest_conv;
	bool priest_conv_playing;
	int interior_nun_conv;
	bool interior_nun_conv_playing;
	bool player_has_reached_grunt_level;
	bool civs_scattered;
	bool commcenter_sam_destroyed;

	M01_Building_State hand_of_nod_state, comm_center_state;
	M01_Location players_location;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(is_startup, 1);
		SAVE_VARIABLE(first_time_in_handofnod, 2);
		SAVE_VARIABLE(first_time_in_hon_gruntlevel, 3);
		SAVE_VARIABLE(first_time_in_hon_dojolevel, 4);
		SAVE_VARIABLE(first_time_poking_gate_lock, 5);
		SAVE_VARIABLE(players_first_time_by_commcenter, 6);
		SAVE_VARIABLE(first_time_at_barn, 7);
		SAVE_VARIABLE(gate_objective_done, 8);
		SAVE_VARIABLE(church_guard_is_dead, 9);
		SAVE_VARIABLE(loveshack_guard_is_dead, 10);

		SAVE_VARIABLE(player_has_an_escort, 11);
		SAVE_VARIABLE(clergy_are_boarding, 12);
		SAVE_VARIABLE(church_pickup_chinook_is_on_its_way, 13);
		SAVE_VARIABLE(player_has_unlocked_pen, 14);
		SAVE_VARIABLE(radar_is_scrambled, 15);
		SAVE_VARIABLE(unlock_gate_objective_active, 16);
		SAVE_VARIABLE(commcenter_sam_objective_active, 17);
		SAVE_VARIABLE(prisoners_are_freed, 18);
		SAVE_VARIABLE(m01_hon_spawned_guy_01_alive, 19);
		SAVE_VARIABLE(m01_hon_spawned_guy_02_alive, 20);

		SAVE_VARIABLE(m01_hon_spawned_guy_03_alive, 21);
		SAVE_VARIABLE(turret_01_is_dead, 22);
		SAVE_VARIABLE(turret_02_is_dead, 23);
		SAVE_VARIABLE(medium_tank_spawned, 24);
		SAVE_VARIABLE(barn_prisoner_01_is_dead, 25);
		SAVE_VARIABLE(barn_prisoner_02_is_dead, 26);
		SAVE_VARIABLE(barn_prisoner_03_is_dead, 27);
		SAVE_VARIABLE(barn_objective_active, 28);
		SAVE_VARIABLE(kane_and_no2_conv01_active, 29);
		SAVE_VARIABLE(kane_and_no2_conv02_active, 30);

		SAVE_VARIABLE(kane_and_havoc_conv_active, 31);
		SAVE_VARIABLE(m01_bridgeWeather_is_Set, 32);
		SAVE_VARIABLE(m01_nodBase_weather_is_Set, 33);
		SAVE_VARIABLE(missionStart, 34);
		SAVE_VARIABLE(barn_objective_failed, 35);
		SAVE_VARIABLE(barn_announcement_active, 36);
		SAVE_VARIABLE(barn_objective_ended, 37);
		SAVE_VARIABLE(turret_objective_over, 38);
		SAVE_VARIABLE(final_conv_played, 39);
		SAVE_VARIABLE(m01_church_interior_nun02_ID, 40);

		SAVE_VARIABLE(m01_whack_a_mole_guy_ID, 41);
		SAVE_VARIABLE(m01_hon_spawned_guy_01_ID, 42);
		SAVE_VARIABLE(m01_hon_spawned_guy_02_ID, 43);
		SAVE_VARIABLE(m01_hon_spawned_guy_03_ID, 44);
		SAVE_VARIABLE(m01_medium_tank_ID, 45);
		SAVE_VARIABLE(m01_hon_spawners_tally, 46);
		SAVE_VARIABLE(m01_hon_total_spawners_tally, 47);
		SAVE_VARIABLE(m01_HON_SpawnRate, 48);
		SAVE_VARIABLE(new_turretbeach_engineers, 49);
		SAVE_VARIABLE(kane_ID, 50);

		SAVE_VARIABLE(eva_comm_center_line_01, 51);
		SAVE_VARIABLE(eva_comm_center_line_02, 52);
		SAVE_VARIABLE(eva_comm_center_line_03, 53);
		SAVE_VARIABLE(eva_comm_center_line_04, 54);
		SAVE_VARIABLE(eva_comm_center_line_05, 55);
		SAVE_VARIABLE(billy_conv, 56);
		SAVE_VARIABLE(babushka_conv, 57);
		SAVE_VARIABLE(start_conv, 58);
		SAVE_VARIABLE(start_conv2, 59);
		SAVE_VARIABLE(kane_and_no2_conv01, 60);

		SAVE_VARIABLE(kane_and_no2_conv02, 61);
		SAVE_VARIABLE(kane_and_havoc_conv, 62);
		SAVE_VARIABLE(lockes_hon_mct_conv1, 63);
		SAVE_VARIABLE(lockes_hon_mct_conv2, 64);
		SAVE_VARIABLE(barnEvac_conv, 65);
		SAVE_VARIABLE(barnfailed_conv, 66);
		SAVE_VARIABLE(civ3_conv, 67);
		SAVE_VARIABLE(missionIntro, 68);
		SAVE_VARIABLE(endMission_conv, 69);
		SAVE_VARIABLE(unlocked_gate_conv, 70);

		SAVE_VARIABLE(turrets_conv, 71);
		SAVE_VARIABLE(players_escorts_ID, 72);
		SAVE_VARIABLE(havocs_targets_ID, 73);
		SAVE_VARIABLE(m01_hon_chinook_guys_killed, 74);
		SAVE_VARIABLE(dead_barn_guard_tally, 75);
		SAVE_VARIABLE(dead_barn_prisoner_tally, 76);
		SAVE_VARIABLE(hand_of_nod_state, 77);
		SAVE_VARIABLE(comm_center_state, 78);
		SAVE_VARIABLE(players_location, 79);
		SAVE_VARIABLE(firstNodCommanderKilled, 80);

		SAVE_VARIABLE(loveshack_conv, 81);
		SAVE_VARIABLE(loveshack_conv_playing, 82);
		SAVE_VARIABLE(priest_conv, 83);
		SAVE_VARIABLE(priest_conv_playing, 84);//
		SAVE_VARIABLE(interior_nun_conv, 85);
		SAVE_VARIABLE(interior_nun_conv_playing, 86);
		SAVE_VARIABLE(player_has_reached_grunt_level, 87);
		SAVE_VARIABLE(civs_scattered, 88);
		SAVE_VARIABLE(commcenter_sam_destroyed, 89);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );

		hand_of_nod_state = IDLE;
		comm_center_state = IDLE;
		players_location = LANDING_BEACH;

		is_startup = true;
		first_time_in_handofnod = true;
		first_time_in_hon_gruntlevel = true;
		first_time_in_hon_dojolevel = true;
		first_time_poking_gate_lock = true;
		players_first_time_by_commcenter = true;
		first_time_at_barn = true;
		missionStart = true;

		church_guard_is_dead = false;
		loveshack_guard_is_dead = false;
		player_has_an_escort = false;
		church_pickup_chinook_is_on_its_way = false;
		clergy_are_boarding = false;
		player_has_unlocked_pen = false;
		radar_is_scrambled = false;
		unlock_gate_objective_active = false;
		commcenter_sam_objective_active = false;
		prisoners_are_freed = false;
		m01_hon_spawned_guy_01_alive = false;
		m01_hon_spawned_guy_02_alive = false;
		m01_hon_spawned_guy_03_alive = false;
		turret_01_is_dead = false;
		turret_02_is_dead = false;
		medium_tank_spawned = false;
		barn_prisoner_01_is_dead = false;
		barn_prisoner_02_is_dead = false;
		barn_prisoner_03_is_dead = false;
		barn_objective_active = false;
		kane_and_no2_conv01_active = false;
		kane_and_no2_conv02_active = false;
		kane_and_havoc_conv_active = false;
		m01_bridgeWeather_is_Set = false;
		m01_nodBase_weather_is_Set = false;
		barn_objective_failed = false;
		barn_announcement_active = false;
		barn_objective_ended = false;
		turret_objective_over = false;
		final_conv_played = false;
		gate_objective_done = false;
		firstNodCommanderKilled = false;
		loveshack_conv_playing = false;
		priest_conv_playing = false;
		interior_nun_conv_playing = false;
		player_has_reached_grunt_level = false;
		civs_scattered = false;
		commcenter_sam_destroyed = false;

		m01_hon_chinook_guys_killed = 0;
		m01_hon_spawners_tally = 0;
		m01_hon_total_spawners_tally = 0;
		new_turretbeach_engineers = 0;
		dead_barn_guard_tally = 0;
		dead_barn_prisoner_tally = 0;
		m01_HON_SpawnRate = 14;
		players_escorts_ID = 0;
		havocs_targets_ID = 0;

		//Commands->Set_Clouds (0.25f, 0.25f, 0);
		Commands->Set_Wind (0, 0, 0, 0);
		Commands->Set_Rain (0, 0, true);
		Commands->Set_Lightning (0.25f, 0.75f, 1, 0, 0.10f, 0);

		//putting datalink updates here
		Commands->Reveal_Encyclopedia_Character	( 19 );//gdi minigunner
		Commands->Reveal_Encyclopedia_Character	( 21 );//gdi minigunner officer
		Commands->Reveal_Encyclopedia_Character	( 14 );//gdi grenadier
		Commands->Reveal_Encyclopedia_Character	( 38 );//nod minigunner
		Commands->Reveal_Encyclopedia_Character	( 18 );//Lt Maus -- whoever that is
		Commands->Reveal_Encyclopedia_Character	( 16 );//General Locke
		Commands->Reveal_Encyclopedia_Character	( 39 );//sakura
		Commands->Reveal_Encyclopedia_Character	( 13 );//hotwire
		Commands->Reveal_Encyclopedia_Character	( 15 );//patch
		Commands->Reveal_Encyclopedia_Character	( 22 );//deadeye
		Commands->Reveal_Encyclopedia_Character	( 26 );//gunner
		Commands->Reveal_Encyclopedia_Character	( 24 );//gdi rocket guy
		Commands->Reveal_Encyclopedia_Character	( 25 );//gdi rocket guy officer
		Commands->Reveal_Encyclopedia_Character	( 23 );//gdi POW

		Commands->Reveal_Encyclopedia_Weapon ( 14 );//pistol
		Commands->Reveal_Encyclopedia_Weapon ( 20 );//sniper rifle
		Commands->Reveal_Encyclopedia_Weapon ( 5 );//remote c4

		Commands->Reveal_Encyclopedia_Vehicle ( 18 );//orca
		Commands->Reveal_Encyclopedia_Vehicle ( 19 );//gdi transport helicopter

		Commands->Reveal_Encyclopedia_Building ( 4 );//gdi powerplant
		Commands->Reveal_Encyclopedia_Building ( 2 );//gdi con yard
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == CUSTOM_EVENT_SOUND_ENDED  )
		{
			if (param == eva_comm_center_line_01)
			{
				eva_comm_center_line_02 = Commands->Create_Sound ( "00-N002E", Vector3 (0,0,0), obj );//please stand by
				Commands->Monitor_Sound ( obj, eva_comm_center_line_02 );
			}

			else if (param == eva_comm_center_line_02)
			{
				eva_comm_center_line_03 = Commands->Create_Sound ( "00-N026E", Vector3 (0,0,0), obj );//transmitting
				Commands->Monitor_Sound ( obj, eva_comm_center_line_03 );
			}

			else if (param == eva_comm_center_line_03)
			{
				eva_comm_center_line_04 = Commands->Create_Sound ( "00-N028E", Vector3 (0,0,0), obj );//receiving
				Commands->Monitor_Sound ( obj, eva_comm_center_line_04 );
			}

			else if (param == eva_comm_center_line_04)
			{
				eva_comm_center_line_05 = Commands->Create_Sound ( "00-N030E", Vector3 (0,0,0), obj );//data acquisition complete
				Commands->Monitor_Sound ( obj, eva_comm_center_line_05 );
			}

			else if (param == eva_comm_center_line_05)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_UNLOCK_GATE_OBJECTIVE_JDG, 0 );
				player_has_unlocked_pen = true;
			}
		}

		else if (type == 0)
		{
			switch (param)
			{
				case 0://mission is over--end pass
					{
						Commands->Mission_Complete ( true );
					}
					break;

				case M01_HON_HAS_BEEN_DESTROYED_JDG://Hand of Nod has been destroyed
					{	
						Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_HON_OBJECTIVE_PASS_JDG, 2 );
						float pick_a_sound = Commands->Get_Random ( 0.5f, 2.5f );

						if (pick_a_sound >= 0.5 && pick_a_sound < 1.5)
						{
							Commands->Create_Sound ( "EVA_Enemy_Structure_Destroyed", Vector3 (0,0,0), obj);
						}

						else 
						{
							Commands->Create_Sound ( "EVA_Nod_Structure_Destroyed", Vector3 (0,0,0), obj);
						}

						hand_of_nod_state = DESTROYED;

						Commands->Give_Points( STAR, 5000, false );

						GameObject * honCafeteriaGuy01 = Commands->Find_Object ( M01_HON_CAFETERIA_EATING_GUY_ID );
						if (honCafeteriaGuy01 != NULL)
						{
							Commands->Send_Custom_Event( obj, honCafeteriaGuy01, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honCafeteriaGuy02 = Commands->Find_Object ( M01_HON_CAFETERIA_WALKING_GUY_ID );
						if (honCafeteriaGuy02 != NULL)
						{
							Commands->Send_Custom_Event( obj, honCafeteriaGuy02, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honJudoGuy01 = Commands->Find_Object ( M01_HON_CAFETERIA_JUDO_GUY_ID );
						if (honJudoGuy01 != NULL)
						{
							Commands->Send_Custom_Event( obj, honJudoGuy01, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDormGuy01 = Commands->Find_Object ( M01_HON_DORMROOM_MINIGUNNER_ID );
						if (honDormGuy01 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDormGuy01, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDormGuy02 = Commands->Find_Object ( M01_HON_DORMROOM_ROCKET_GUY_ID );
						if (honDormGuy02 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDormGuy02, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDormGuy03 = Commands->Find_Object ( M01_HON_DORMROOM_CHEM_GUY_ID );
						if (honDormGuy03 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDormGuy03, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDormGuy04 = Commands->Find_Object ( M01_HON_DORMROOM_FLAME_GUY_ID );
						if (honDormGuy04 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDormGuy04, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDormGuy05 = Commands->Find_Object ( M01_HON_DORMROOM_CRAPPER_ID );
						if (honDormGuy05 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDormGuy05, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDojoSensei = Commands->Find_Object ( M01_HON_DOJO_SENSEI_JDG );
						if (honDojoSensei != NULL)
						{
							Commands->Send_Custom_Event( obj, honDojoSensei, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDojoCiv01 = Commands->Find_Object ( M01_HON_DOJO_CIV_01_JDG );
						if (honDojoCiv01 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDojoCiv01, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDojoCiv02 = Commands->Find_Object ( M01_HON_DOJO_CIV_02_JDG );
						if (honDojoCiv02 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDojoCiv02, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						GameObject * honDojoCiv03 = Commands->Find_Object ( M01_HON_DOJO_CIV_03_JDG );
						if (honDojoCiv03 != NULL)
						{
							Commands->Send_Custom_Event( obj, honDojoCiv03, 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, 0 );
						}

						if (players_location != INSIDE_HAND_OF_NOD)
						{
							if (player_has_reached_grunt_level == false)
							{
								GameObject * m01_hon_evacuator_guy01 = Commands->Trigger_Spawner( M01_HON_FRONTDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy01, "M01_HON_FrontDoor_Evacuator_JDG", "");
								GameObject * m01_hon_evacuator_guy02 = Commands->Trigger_Spawner( M01_HON_FRONTDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy02, "M01_HON_FrontDoor_Evacuator_JDG", "");
								GameObject * m01_hon_evacuator_guy03 = Commands->Trigger_Spawner( M01_HON_FRONTDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy03, "M01_HON_FrontDoor_Evacuator_JDG", "");

								GameObject * m01_hon_evacuator_guy04 = Commands->Trigger_Spawner( M01_HON_BACKDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy04, "M01_HON_BackDoor_Evacuator_JDG", "");
								GameObject * m01_hon_evacuator_guy05 = Commands->Trigger_Spawner( M01_HON_BACKDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy05, "M01_HON_BackDoor_Evacuator_JDG", "");
								GameObject * m01_hon_evacuator_guy06 = Commands->Trigger_Spawner( M01_HON_BACKDOOR_DESTROYED_SPAWNER_JDG );
								Commands->Attach_Script(m01_hon_evacuator_guy06, "M01_HON_BackDoor_Evacuator_JDG", "");
							}
						}
					}
					break;

				case M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG://Comm Center has been destroyed
					{	
						if (players_location != INSIDE_COMM_CENTER)
						{
							GameObject * m01_commcenter_evacuator_guy01 = Commands->Trigger_Spawner( M01_COMMCENTER_DESTROYED_SPAWNER_JDG );
							Commands->Attach_Script(m01_commcenter_evacuator_guy01, "M01_CommCenter_Evacuator_JDG", "");
							GameObject * m01_commcenter_evacuator_guy02 = Commands->Trigger_Spawner( M01_COMMCENTER_DESTROYED_SPAWNER_JDG );
							Commands->Attach_Script(m01_commcenter_evacuator_guy02, "M01_CommCenter_Evacuator_JDG", "");
							GameObject * m01_commcenter_evacuator_guy03 = Commands->Trigger_Spawner( M01_COMMCENTER_DESTROYED_SPAWNER_JDG );
							Commands->Attach_Script(m01_commcenter_evacuator_guy03, "M01_CommCenter_Evacuator_JDG", "");
							GameObject * m01_commcenter_evacuator_guy04 = Commands->Trigger_Spawner( M01_COMMCENTER_DESTROYED_SPAWNER_JDG );
							Commands->Attach_Script(m01_commcenter_evacuator_guy04, "M01_CommCenter_Evacuator_JDG", "");
							GameObject * m01_commcenter_evacuator_guy05 = Commands->Trigger_Spawner( M01_COMMCENTER_DESTROYED_SPAWNER_JDG );
							Commands->Attach_Script(m01_commcenter_evacuator_guy05, "M01_CommCenter_Evacuator_JDG", "");
						}

						if (player_has_unlocked_pen == false)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_UNLOCK_GATE_OBJECTIVE_JDG, 0 );
							player_has_unlocked_pen = true;
						}

						if (radar_is_scrambled == true)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_UNSCRAMBLE_THE_RADAR_JDG, 0 );
						}

						float pick_a_sound = Commands->Get_Random ( 0.5f, 2.5f );

						if (pick_a_sound >= 0.5 && pick_a_sound < 1.5)
						{
							Commands->Create_Sound ( "EVA_Enemy_Structure_Destroyed", Vector3 (0,0,0), obj);
						}

						else if (pick_a_sound >= 1.5 && pick_a_sound <= 2.5)
						{
							Commands->Create_Sound ( "EVA_Nod_Structure_Destroyed", Vector3 (0,0,0), obj);
						}

						comm_center_state = DESTROYED;
					}
					break;

				case M01_PLAYER_IS_INSIDE_HON_JDG://Player is inside HON--kill rain sounds
					{	
						players_location = INSIDE_HAND_OF_NOD;

						GameObject * soundController = Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG );
						if (soundController != NULL)
						{
							Commands->Send_Custom_Event( obj, soundController, 0, M01_TURN_OFF_OUTSIDE_AMBIENTS_JDG, 0 );
							Commands->Send_Custom_Event( obj, soundController, 0, M01_PLAYER_IS_INSIDE_HON_JDG, 0 );
						}

						if (first_time_in_handofnod == true)
						{
							first_time_in_handofnod = false;
							Commands->Send_Custom_Event( obj, obj, 0, M01_HON_CUE_WARROOM_LEVEL_ACTORS_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_OUTSIDE_HON_JDG://Player is outside HON--restart rain sounds
					{	
						players_location = BY_HAND_OF_NOD;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_ON_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG://Player is inside Comm Center--kill rain sounds--start internal pagings
					{	
						players_location = INSIDE_COMM_CENTER;

						GameObject * soundController = Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG );
						if (soundController != NULL)
						{
							Commands->Send_Custom_Event( obj, soundController, 0, M01_TURN_OFF_OUTSIDE_AMBIENTS_JDG, 0 );
							Commands->Send_Custom_Event( obj, soundController, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}
						
						GameObject * baseCommander = Commands->Find_Object ( M01_COMMCENTER_BASE_COMMANDER_JDG );
						if (baseCommander != NULL)
						{
							Commands->Send_Custom_Event( obj, baseCommander, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * baseScapeGoat = Commands->Find_Object ( M01_COMMCENTER_BASE_SCAPEGOAT_JDG );
						if (baseScapeGoat != NULL)
						{
							Commands->Send_Custom_Event( obj, baseScapeGoat, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * upstairsTech = Commands->Find_Object ( M01_COMMCENTER_UPSTAIRS_TECH_JDG );
						if (upstairsTech != NULL)
						{
							Commands->Send_Custom_Event( obj, upstairsTech, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * warroomTech01 = Commands->Find_Object ( M01_COMMCENTER_WAROOM_TECH_01_JDG );
						if (warroomTech01 != NULL)
						{
							Commands->Send_Custom_Event( obj, warroomTech01, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * warroomTech02 = Commands->Find_Object ( M01_COMMCENTER_WAROOM_TECH_02_JDG );
						if (warroomTech02 != NULL)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_COMMCENTER_WAROOM_TECH_02_JDG ), 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * mainframeTech = Commands->Find_Object ( M01_COMMCENTER_COMPUTERROOM_TECH_JDG );
						if (mainframeTech != NULL)
						{
							Commands->Send_Custom_Event( obj, mainframeTech, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}

						GameObject * upstairsGuard = Commands->Find_Object ( M01_COMMCENTER_UPSTAIRS_GUARD_JDG );
						if (upstairsGuard != NULL)
						{
							Commands->Send_Custom_Event( obj, upstairsGuard, 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_OUTSIDE_COMM_CENTER_JDG://Player is outside Comm Center--restart rain sounds
					{	
						players_location = BY_COMM_CENTER;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_ON_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_INSIDE_TUNNEL_JDG://Player is inside TUNNEL leading to tailgun run--stop rain sounds
					{	
						players_location = INSIDE_TUNNEL;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_OFF_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_OUTSIDE_TUNNEL_JDG://Player is outside TUNNEL--restart rain sounds
					{	
						players_location = BY_TUNNEL;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_ON_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_INSIDE_WATERFALL_JDG://Player is inside WATERFALL cave--stop rain sounds
					{	
						players_location = INSIDE_WATERFALL;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_OFF_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_OUTSIDE_WATERFALL_JDG://Player is outside WATERFALL cave--restart rain sounds
					{	
						players_location = BY_WATERFALL;
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_TURN_ON_OUTSIDE_AMBIENTS_JDG, 0 );
					}
					break;

				case M01_PLAYER_IS_IN_HON_GRUNT_LEVEL_JDG://Player is entering Grunt level--cue actors
					{
						player_has_reached_grunt_level = true;

						if ((hand_of_nod_state != DESTROYED) && (first_time_in_hon_gruntlevel == true))
						{
							first_time_in_hon_gruntlevel = false;

							GameObject * honCafeteriaGuy01 = Commands->Find_Object ( M01_HON_CAFETERIA_EATING_GUY_ID );
							if (honCafeteriaGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, honCafeteriaGuy01, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honCafeteriaGuy02 = Commands->Find_Object ( M01_HON_CAFETERIA_WALKING_GUY_ID );
							if (honCafeteriaGuy02 != NULL)
							{
								Commands->Send_Custom_Event( obj, honCafeteriaGuy02, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honJudoGuy01 = Commands->Find_Object ( M01_HON_CAFETERIA_JUDO_GUY_ID );
							if (honJudoGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, honJudoGuy01, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honDormGuy01 = Commands->Find_Object ( M01_HON_DORMROOM_MINIGUNNER_ID );
							if (honDormGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, honDormGuy01, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honDormGuy02 = Commands->Find_Object ( M01_HON_DORMROOM_ROCKET_GUY_ID );
							if (honDormGuy02 != NULL)
							{
								Commands->Send_Custom_Event( obj, honDormGuy02, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honDormGuy03 = Commands->Find_Object ( M01_HON_DORMROOM_CHEM_GUY_ID );
							if (honDormGuy03 != NULL)
							{
								Commands->Send_Custom_Event( obj, honDormGuy03, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honDormGuy04 = Commands->Find_Object ( M01_HON_DORMROOM_FLAME_GUY_ID );
							if (honDormGuy04 != NULL)
							{
								Commands->Send_Custom_Event( obj, honDormGuy04, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * honDormGuy05 = Commands->Find_Object ( M01_HON_DORMROOM_CRAPPER_ID );
							if (honDormGuy05 != NULL)
							{
								Commands->Send_Custom_Event( obj, honDormGuy05, 0, M01_START_ACTING_JDG, 0 );
							}
						}
					}
					break;

				case M01_PLAYER_IS_IN_HON_DOJO_JDG://Player is entering DOJO--cue actors
					{	
						if ((hand_of_nod_state != DESTROYED) && (first_time_in_hon_dojolevel == true))
						{
							first_time_in_hon_dojolevel = false;

							if (Commands->Find_Object ( M01_HON_DOJO_SENSEI_JDG ))
							{
								Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_SENSEI_JDG ), 0, M01_START_ACTING_JDG, 0 );
							}
						}
					}
					break;

				case M01_HON_SAMSITE_HAS_BEEN_DESTROYED_JDG://hand of nod sam site has been destroyed--duh!
					{	
						//Commands->Set_Objective_Status( M01_HON_SAM_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );

						GameObject * handOfNod = Commands->Find_Object ( M01_HAND_OF_NOD_JDG );
						float handOfNodHealth = Commands->Get_Health ( handOfNod );
						if (handOfNodHealth > 0)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_REINFORCEMENTS_JDG, 3 );

							float delayTimer_A = Commands->Get_Random ( 0, 2 );
							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_HON_REINFORCEMENTS_A_JDG, delayTimer_A );
							float delayTimer_B = Commands->Get_Random ( 2, 4 );
							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_HON_REINFORCEMENTS_B_JDG, delayTimer_B );
						}
					}
					break;

				case M01_COMMCENTER_SAMSITE_HAS_BEEN_DESTROYED_JDG://Comm Center sam site has been destroyed--duh!
					{	
						commcenter_sam_destroyed = true;
						if (commcenter_sam_objective_active == true)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_PASS_COMM_SAM_OBJECTIVE_JDG, 0 );
						}

						if (player_has_unlocked_pen == true)
						{
							unlocked_gate_conv = Commands->Create_Conversation( "M01_Remove_Unlock_Gate_Objective", 100, 1000, false);
							Commands->Join_Conversation( NULL, unlocked_gate_conv, false, false, true );
							Commands->Start_Conversation( unlocked_gate_conv, unlocked_gate_conv );

							Commands->Monitor_Conversation(  obj, unlocked_gate_conv );
						}

						//Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_REINFORCEMENTS_JDG, 5 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_COMMCENTER_REINFORCEMENTS_JDG, 10 );
					}
					break;

				case M01_ANNOUNCE_AIRSTRIKE_JDG://EVA announces incoming airstrike
					{	
						Commands->Create_Sound ( "00-N036E", Vector3 (0,0,0), obj);
					}
					break;

				case M01_ANNOUNCE_REINFORCEMENTS_JDG://EVA announces incoming reinforcements
					{	
						Commands->Create_Sound ( "00-N180E", Vector3 (0,0,0), obj);
					}
					break;

				case M01_SEND_COMMCENTER_REINFORCEMENTS_JDG://Starting Elie drop off animation at Comm Center
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-296.388f, 497.455f, 29.122f));
						Commands->Set_Facing(chinook_obj1, -130.00f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_GDI_CommCenter_TroopDrop.txt");
					}
					break;

				case M01_SEND_HON_REINFORCEMENTS_A_JDG://Starting Elie drop off animation at Hand of Nod
					{	
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-147.1f, 558.8f, 4.4f));
						Commands->Set_Facing(chinook_obj1, -130.00f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_GDI_HON_TroopDrop01a.txt");
					}
					break;

				case M01_SEND_HON_REINFORCEMENTS_B_JDG://Starting Elie drop off animation at Hand of Nod
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-154.679f, 519.009f, 4.97f));
						Commands->Set_Facing(chinook_obj1, 130.00f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_GDI_HON_TroopDrop01b.txt");
					}
					break;

				case M01_HON_CHINOOK_GUY_HAS_BEEN_KILLED_JDG://a reinforcement guy's been killed--spawn more if needed
					{	
						m01_hon_chinook_guys_killed++;

						if (m01_hon_chinook_guys_killed == 3)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_REINFORCEMENTS_JDG, 3 );

							float pick_a_chinook = Commands->Get_Random ( 0.5f, 2.5f );

							if (pick_a_chinook >= 0.5 && pick_a_chinook < 1.5)
							{
								float delayTimer = Commands->Get_Random ( 0, 2 );
								Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_HON_REINFORCEMENTS_A_JDG, delayTimer );
							}

							else 
							{
								float delayTimer = Commands->Get_Random ( 0, 2 );
								Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_HON_REINFORCEMENTS_B_JDG, delayTimer );
							}
						}
					}
					break;

				case M01_PLAYER_IS_APPROACHING_CHURCH_JDG://cue church actors
					{	
						if (m01_bridgeWeather_is_Set == false)
						{
							m01_bridgeWeather_is_Set = true;
							//Commands->Set_Clouds (0.75f, 0.75f, 30);
							Commands->Set_Wind (60, 3.0f, 0.5f, 15);
							Commands->Set_Rain (0.75f, 30, false);
							Commands->Set_Lightning (0.5f, 0.25f, 0.75f, 10, 0.33f, 5);

							Vector3 spawnLocation (-98.937f, 380.721f, 20.022f);
							GameObject * harvester = Commands->Create_Object ( "Nod_Harvester", spawnLocation );
							Commands->Attach_Script(harvester, "M01_HarvesterScript_New_JDG", "");

							GameObject * chemGuy01 = Commands->Find_Object ( 103270 );

							if (chemGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, chemGuy01, 0, M01_START_ACTING_JDG, 5 );
							}
						} 

						int church_upadate = Commands->Create_Conversation( "M01_Add_Church_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, church_upadate, false, false, true );
						//Commands->Join_Conversation( STAR, church_upadate, false, false );
						Commands->Start_Conversation( church_upadate,  0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_CHURCH_OBJECTIVE_JDG, 8 );

						GameObject * loveshackGuard = Commands->Find_Object ( M01_CHURCH_LOVESHACK_MINIGUNNER_ID );
						if (loveshackGuard != NULL)
						{
							Commands->Send_Custom_Event( obj, loveshackGuard, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID );
						if (loveshackNun != NULL)
						{
							Commands->Send_Custom_Event( obj, loveshackNun, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * exteriorGuard = Commands->Find_Object ( M01_CHURCH_EXTERIOR_MINIGUNNER_ID );
						if (exteriorGuard != NULL)
						{
							Commands->Send_Custom_Event( obj, exteriorGuard, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * balconyGuard = Commands->Find_Object ( M01_CHURCH_BALCONY_MINIGUNNER_ID );
						if (balconyGuard != NULL)
						{
							Commands->Send_Custom_Event( obj, balconyGuard, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * rectoryGuard = Commands->Find_Object ( M01_CHURCH_GUARD_MINIGUNNER_ID );
						if (rectoryGuard != NULL)
						{
							Commands->Send_Custom_Event( obj, rectoryGuard, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * churchNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
						if (churchNun != NULL)
						{
							Commands->Send_Custom_Event( obj, churchNun, 0, M01_START_ACTING_JDG, 0 );
						}

						GameObject * churchPriest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );
						if (churchPriest != NULL)
						{
							Commands->Send_Custom_Event( obj, churchPriest, 0, M01_START_ACTING_JDG, 0 );
						}
					}
					break;

				case M01_ADD_CHURCH_OBJECTIVE_JDG://Loveshack guard is dead--let nun know
					{
						Commands->Add_Objective( M01_CHURCH_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_05, NULL, IDS_Enc_Obj_Secondary_M01_05 );

						GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
						if (pogController != NULL)
						{
							Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_CHURCH_OBJECTIVE_JDG, 0 );
						}

						GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID );
						if (loveshackNun != NULL)
						{
							 Commands->Set_Obj_Radar_Blip_Shape ( loveshackNun, RADAR_BLIP_SHAPE_OBJECTIVE );
							 Commands->Set_Obj_Radar_Blip_Color ( loveshackNun, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
						}

						GameObject * churchNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
						if (churchNun != NULL)
						{
							Commands->Set_Obj_Radar_Blip_Shape ( churchNun, RADAR_BLIP_SHAPE_OBJECTIVE );
							Commands->Set_Obj_Radar_Blip_Color ( churchNun, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
						}

						GameObject * churchPriest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );
						if (churchPriest != NULL)
						{
							Commands->Set_Obj_Radar_Blip_Shape ( churchPriest, RADAR_BLIP_SHAPE_OBJECTIVE );
							Commands->Set_Obj_Radar_Blip_Color ( churchPriest, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
						}
					}
					break;

				case M01_CLEAR_FIRST_NOD_COMMANDER_PASS_JDG://guard tower commander is destroyed--clear objective
					{
						Commands->Set_Objective_Status( M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
						firstNodCommanderKilled = true;
					}
					break;


				case M01_CLEAR_HON_OBJECTIVE_PASS_JDG://HON is destroyed--clear objective
					{
						Commands->Set_Objective_Status( M01_HON_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
					}
					break;

				case M01_CLEAR_CHURCH_OBJECTIVE_PASS_JDG://player has rescued clergy
					{	
						Commands->Set_Objective_Status( M01_CHURCH_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
					}
					break;

				case M01_CLEAR_CHURCH_OBJECTIVE_FAIL_JDG://player has rescued clergy
					{	
						Commands->Set_Objective_Status( M01_CHURCH_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
					}
					break;

				case M01_LOVESHACK_GUARD_IS_DEAD_JDG://Loveshack guard is dead--let nun know
					{	
						loveshack_guard_is_dead = true;

						GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID );
						if (loveshackNun != NULL)
						{
							float delayTimer = Commands->Get_Random ( 0, 2 );
							Commands->Send_Custom_Event( obj, loveshackNun, 0, M01_LOVESHACK_GUARD_IS_DEAD_JDG, delayTimer );
						}

						if ((loveshack_guard_is_dead == true) && (church_guard_is_dead == true) && (church_pickup_chinook_is_on_its_way == false))
						{
							GameObject * churchEvacController = Commands->Find_Object ( 103394 );
							if (churchEvacController != NULL)
							{
								Commands->Send_Custom_Event( obj, churchEvacController, 0, M01_START_ACTING_JDG, 3 );//giving clearance to havoc exit zone for conversation
							}
							church_pickup_chinook_is_on_its_way = true;
						}
					}
					break;

				case M01_CHURCH_GUARD_IS_DEAD_JDG://inner guard is dead--let nuns & prist know
					{	
						church_guard_is_dead = true;

						GameObject * churchNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
						if (churchNun != NULL)
						{
							float delayTimer = Commands->Get_Random ( 0, 2 );
							Commands->Send_Custom_Event( obj, churchNun, 0, M01_CHURCH_GUARD_IS_DEAD_JDG, delayTimer );
						}

						GameObject * churchPriest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );
						if (churchPriest != NULL)
						{
							float delayTimer = Commands->Get_Random ( 0, 2 );
							Commands->Send_Custom_Event( obj, churchPriest, 0, M01_CHURCH_GUARD_IS_DEAD_JDG, delayTimer );
						}

						if ((loveshack_guard_is_dead == true) && (church_guard_is_dead == true) && (church_pickup_chinook_is_on_its_way == false))
						{
							church_pickup_chinook_is_on_its_way = true;
							GameObject * churchEvacController = Commands->Find_Object ( 103394 );
							if (churchEvacController != NULL)
							{
								Commands->Send_Custom_Event( obj, churchEvacController, 0, M01_START_ACTING_JDG, 3 );//giving clearance to havoc exit zone for conversation
							}
						}
					}
					break;

				case M01_CHURCH_CLERGY_HAS_BOARDED_JDG://when all clergy have boarded--tell chinook to take off
					{	
						if (Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID ) || (Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID )) || (Commands->Find_Object ( m01_church_interior_nun02_ID )) || (Commands->Find_Object ( M01_CHURCH_PRIEST_ID )))
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_CHURCH_CLERGY_HAS_BOARDED_JDG, 2 );
						}

						else
						{
							if ((clergy_are_boarding == false))
							{
								clergy_are_boarding = true;

								Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_CHURCH_OBJECTIVE_PASS_JDG, 2 );
							}
						}
					}
					break;

				case M01_CARD_CARRIER_HAS_BEEN_KILLED_JDG://a card carrier has been killed--drop necessary card
					{
						bool playerHasLevel01KeyCard = Commands->Has_Key( STAR, 1 );;
						bool playerHasLevel02KeyCard = Commands->Has_Key( STAR, 2 );;
						bool playerHasLevel03KeyCard = Commands->Has_Key( STAR, 3 );;

						if (playerHasLevel01KeyCard == false)
						{
							Vector3 spawn_location = Commands->Get_Position ( sender );
							spawn_location.Z = spawn_location.Z + 0.75f;

							GameObject * keycard01 = Commands->Create_Object ( "Level_01_Keycard", spawn_location );
							Commands->Attach_Script(keycard01, "M01_KeyCard01_Script_JDG", "");
						}

						else if (playerHasLevel02KeyCard == false)
						{
							Vector3 spawn_location = Commands->Get_Position ( sender );
							spawn_location.Z = spawn_location.Z + 0.75f;

							Commands->Create_Object ( "Level_02_Keycard", spawn_location );
						}

						else if (playerHasLevel03KeyCard == false)
						{
							Vector3 spawn_location = Commands->Get_Position ( sender );
							spawn_location.Z = spawn_location.Z + 0.75f;

							Commands->Create_Object ( "Level_03_Keycard", spawn_location );
						}

						else
						{
							Vector3 spawn_location = Commands->Get_Position ( sender );
							spawn_location.Z = spawn_location.Z + 0.75f;

							Commands->Create_Object ( "POW_Health_100", spawn_location );
						}
					}
					break;

				case M01_GDI_GUY_HAS_BEEN_POKED_JDG://player has poked a GDI soldier--if player is alone assign escort
					{	
						int poked_guys_ID = Commands->Get_ID ( sender );

						if (player_has_an_escort == false)
						{
							Commands->Send_Custom_Event( obj, sender, 0, M01_PLEASE_START_ESCORTING_PLAYER_JDG, 0 );
							players_escorts_ID = poked_guys_ID;
							player_has_an_escort = true;
						}

						else if (player_has_an_escort == true)
						{
							if (poked_guys_ID == players_escorts_ID)//this guy is escorting player--tell him to hold position
							{
								Commands->Send_Custom_Event( obj, sender, 0, M01_PLEASE_STOP_ESCORTING_PLAYER_JDG, 0 );
								player_has_an_escort = false;
							}
						}
					}
					break;

				case M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG://player's escort has been killed--reset escort parameters
					{
						player_has_an_escort = false;
					}
					break;

				case M01_PLAYER_IS_ATTACKING_ME_JDG://if player is being escorted--tell escort who havoc's target is 
					{
						if (player_has_an_escort == true)
						{
							havocs_targets_ID = Commands->Get_ID ( sender );
							float delayTimer = Commands->Get_Random ( 0, 1 );
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( players_escorts_ID ), M01_PLAYER_IS_ATTACKING_ME_JDG, havocs_targets_ID, delayTimer );
						}
					}
					break;

				case M01_PLAYER_IS_LEAVING_CHURCH_01_JDG://if havoc has escort--tell them to go somewhere else
					{
						if (player_has_an_escort == true)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( players_escorts_ID ), 0, M01_PLAYER_IS_LEAVING_CHURCH_01_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_LEAVING_CHURCH_02_JDG://if havoc has escort--tell them to go somewhere else
					{
						if (player_has_an_escort == true)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( players_escorts_ID ), 0, M01_PLAYER_IS_LEAVING_CHURCH_02_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_LEAVING_HON_01_JDG://if havoc has escort--tell them to go somewhere else
					{	
						if (player_has_an_escort == true)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( players_escorts_ID ), 0, M01_PLAYER_IS_LEAVING_HON_01_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_LEAVING_HON_02_JDG://if havoc has escort--tell them to go somewhere else
					{
						if (player_has_an_escort == true)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( players_escorts_ID ), 0, M01_PLAYER_IS_LEAVING_HON_02_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_CROSSING_THE_BRIDGE_JDG://havoc is crossing bridge for first time--cue driveby actors
					{
						if (m01_bridgeWeather_is_Set == false)
						{
							m01_bridgeWeather_is_Set = true;
							//Commands->Set_Clouds (0.75f, 0.75f, 30);
							Commands->Set_Wind (60, 3.0f, 0.5f, 15);
							Commands->Set_Rain (0.75f, 30, false);
							Commands->Set_Lightning (0.5f, 0.25f, 0.75f, 10, 0.33f, 5);

							Vector3 spawnLocation (-98.937f, 380.721f, 20.022f);
							GameObject * harvester = Commands->Create_Object ( "Nod_Harvester", spawnLocation );
							Commands->Attach_Script(harvester, "M01_HarvesterScript_New_JDG", "");

							GameObject * chemGuy01 = Commands->Find_Object ( 103270 );

							if (chemGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, chemGuy01, 0, M01_START_ACTING_JDG, 5 );
							}
						} 

						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_CHURCH_CHINOOK_JDG, 10 );

						GameObject * churchAreaCommander = Commands->Find_Object ( M01_CHURCHAREA_NOD_COMMANDER_JDG );
						if (churchAreaCommander != NULL)
						{
							Commands->Send_Custom_Event( obj, churchAreaCommander, 0, M01_START_ACTING_JDG, 0 );
						}
					}
					break;

				case M01_PLAYER_IS_CROSSING_THE_BRIDGE_VIA_CAVE_JDG://havoc is crossing bridge for first time--cue actors driveby actors
					{
						if (m01_bridgeWeather_is_Set == false)
						{
							m01_bridgeWeather_is_Set = true;
							//Commands->Set_Clouds (0.75f, 0.75f, 30);
							Commands->Set_Wind (60, 3.0f, 0.5f, 15);
							Commands->Set_Rain (0.75f, 30, false);
							Commands->Set_Lightning (0.5f, 0.25f, 0.75f, 10, 0.33f, 5);

							Vector3 spawnLocation (-98.937f, 380.721f, 20.022f);
							GameObject * harvester = Commands->Create_Object ( "Nod_Harvester", spawnLocation );
							Commands->Attach_Script(harvester, "M01_HarvesterScript_New_JDG", "");

							GameObject * chemGuy01 = Commands->Find_Object ( 103270 );

							if (chemGuy01 != NULL)
							{
								Commands->Send_Custom_Event( obj, chemGuy01, 0, M01_START_ACTING_JDG, 5 );
							}
						} 

						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_CHURCH_CHINOOK_JDG, 10 );

						GameObject * churchAreaCommander = Commands->Find_Object ( M01_CHURCHAREA_NOD_COMMANDER_JDG );
						if (churchAreaCommander != NULL)
						{
							Commands->Send_Custom_Event( obj, churchAreaCommander, 0, M01_START_ACTING_JDG, 0 );
						}
					}
					break;

				case M01_SPAWN_TIB_CAVE_HELICOPTER_JDG://player is in tib cave--spawn surprise helicopter
					{	
						GameObject * m01_tiberium_cave_helicopter = Commands->Create_Object ( "NOD_Apache", Vector3(-40.708f, 375.387f, 30.275f) );
						Commands->Set_Facing ( m01_tiberium_cave_helicopter, -30 );
						Commands->Attach_Script(m01_tiberium_cave_helicopter, "M01_Tiberium_Cave_Helicopter_JDG", "");
					}
					break;

				case M01_ANNOUNCE_HON_OBJECTIVE_JDG://player is approaching hand of nod--give objective
					{
						if (m01_nodBase_weather_is_Set == false)
						{
							m01_nodBase_weather_is_Set = true;
							//Commands->Set_Clouds (1.0f, 1.0f, 15);
							Commands->Set_Wind (60, 5.0f, 1.0f, 15);
							Commands->Set_Rain (1.0f, 15, false);
							Commands->Set_Lightning (0.5f, 0, 0.25f, 0, 1, 15);
						} 

						int hand_of_nod_conv = Commands->Create_Conversation( "M01_Add_HON_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, hand_of_nod_conv, false, false, true );
						Commands->Start_Conversation( hand_of_nod_conv,  0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_HON_OBJECTIVE_JDG, 10 );
					}
					break;

				case M01_ADD_HON_OBJECTIVE_JDG://adding objective
					{
						Commands->Add_Objective( M01_HON_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_07, NULL, IDS_Enc_Obj_Secondary_M01_07 );
						Commands->Set_Objective_Radar_Blip( M01_HON_OBJECTIVE_JDG, Vector3 (-179.3f, 541.9f, 3.518f) );

						GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
						if (pogController != NULL)
						{
							Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_HON_OBJECTIVE_JDG, 0 );
						}
					}
					break;

				case M01_KILL_THE_COMM_JDG://apply damage to COMM CENTER
					{
						if (comm_center_state != DESTROYED)
						{
							Commands->Apply_Damage( Commands->Find_Object ( M01_COMM_CENTER_JDG ), 100000000, "BlamoKiller", obj );
						}
					}
					break;

				case M01_KILL_THE_HON_JDG://apply damage to HON
					{
						if (hand_of_nod_state != DESTROYED)
						{
							Commands->Apply_Damage( Commands->Find_Object ( M01_HAND_OF_NOD_JDG	 ), 100000000, "BlamoKiller", obj );
						}
					}
					break;

				case M01_PLAYER_HAS_POKED_COMM_CENTER_PCT_JDG:
					{	
						if (player_has_unlocked_pen == false)//player is poking PCT--play EVA messages
						{
							eva_comm_center_line_01 = Commands->Create_Sound ( "00-N000E", Vector3 (0,0,0), obj );//accessing
							Commands->Monitor_Sound ( obj, eva_comm_center_line_01 );

							//Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_UNLOCK_GATE_OBJECTIVE_JDG, 0 );
							//player_has_unlocked_pen = true;
						}
					}
					break;

				case M01_PLAYER_HAS_POKED_PEN_GATE_JDG:
					{
						if (player_has_unlocked_pen != true)
						{
							if (first_time_poking_gate_lock == true)
							{
								first_time_poking_gate_lock = false;
								int prisoner_conv = Commands->Create_Conversation( "M01_Add_Unlock_Gate_Objective", 100, 1000, false);
								Commands->Join_Conversation( NULL, prisoner_conv, false, false, true );
								Commands->Start_Conversation( prisoner_conv,  prisoner_conv );
							}

							else
							{
								Commands->Create_Sound ( "00-N008E", Vector3 (0,0,0), obj );//access denied
							}
						}

						else if (gate_objective_done == false)
						{
							gate_objective_done = true;
							Commands->Set_Objective_Status( M01_OPEN_THE_GATE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
							Commands->Static_Anim_Phys_Goto_Last_Frame (M01_COMMCENTER_DETENTION_GATE_JDG, NULL );	
							
							//Vector3 gatePosition = Commands->Get_Position ( Commands->Find_Object ( M01_COMMCENTER_DETENTION_GATE_JDG ) );
							//Vector3 gatePosition = Commands->Get_Position ( STAR );
							Vector3 gatePosition (-313.060f, 575.220f, 28.513f);
							Commands->Create_Sound ( "Gate_Down_01", gatePosition, obj );

							if (Commands->Find_Object ( M01_COMMCENTER_SAM_JDG ))
							{
								Commands->Create_Logical_Sound ( obj, M01_DETENTION_GATE_IS_DOWN_JDG, Vector3 (-315.383f, 572.805f, 27.623f), 50.0f );
								Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_COMMCENTER_SAM_OBJECTIVE_JDG, 0 );
							}

							else 
							{
								Commands->Create_Logical_Sound ( obj, M01_DETENTION_GATE_DOWN_SAM_DEAD_JDG, Vector3 (-315.383f, 572.805f, 27.623f), 50.0f );
							}

							if (prisoners_are_freed == false)
							{	
								Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_PRISONERS_PASS_JDG, 0 );
								prisoners_are_freed = true;
							}
							
						}
					}
					break;

				case M01_ANNOUNCE_COMMCENTER_SAM_OBJECTIVE_JDG:
					{
						int comm_sam_conv = Commands->Create_Conversation( "M01_Add_Comm_SAM_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, comm_sam_conv, false, false, true );
						//Commands->Join_Conversation( STAR, comm_sam_conv, false, false, true );
						Commands->Start_Conversation( comm_sam_conv,  0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_COMM_SAM_OBJECTIVE_JDG, 6 );

						commcenter_sam_objective_active = true;
					}
					break;

				case M01_ADD_COMM_SAM_OBJECTIVE_JDG:
					{	
						GameObject * commSamSite = Commands->Find_Object ( M01_COMMCENTER_SAM_JDG );
						if (commSamSite != NULL)
						{
							Commands->Add_Objective( M01_COMM_SAM_OBJECTIVE_JDG, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M01_04, NULL, IDS_Enc_Obj_Primary_M01_04 );
							Commands->Set_Objective_Radar_Blip_Object( M01_COMM_SAM_OBJECTIVE_JDG, commSamSite );

							GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
							if (pogController != NULL)
							{
								Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_COMM_SAM_OBJECTIVE_JDG, 0 );
							}
						}
					}
					break;

				case M01_ANNOUNCE_UNLOCK_GATE_OBJECTIVE_JDG:
					{
						int unlock_gate_conv = Commands->Create_Conversation( "M01_Add_Unlock_Gate_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, unlock_gate_conv, false, false, true );
						Commands->Start_Conversation( unlock_gate_conv,  0 );
					}
					break;

				case M01_ADD_UNLOCK_GATE_OBJECTIVE_JDG:
					{
						Vector3 pogMarker (-296.505f, 521.090f, 27.090f);
						Commands->Add_Objective( M01_UNLOCK_GATE_OBJECTIVE_JDG, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M01_02, NULL, IDS_Enc_Obj_Primary_M01_02 );
						Commands->Set_Objective_HUD_Info_Position( M01_UNLOCK_GATE_OBJECTIVE_JDG, 93, "POG_M01_1_02.tga", IDS_POG_HACK, pogMarker );

						unlock_gate_objective_active = true;

						//Vector3 green (0,1,0);
						Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Primary_M01_02, TEXT_COLOR_OBJECTIVE_PRIMARY );

						GameObject * invisibleCrate = Commands->Find_Object ( 107809 );
						if (invisibleCrate != NULL)
						{
							Commands->Send_Custom_Event( obj, invisibleCrate, 0, M01_COMM_SAM_OBJECTIVE_JDG, 0 );
						}
					}
					break;

				case M01_CLEAR_UNLOCK_GATE_OBJECTIVE_JDG:
					{
						if (commcenter_sam_destroyed == true)
						{
							unlocked_gate_conv = Commands->Create_Conversation( "M01_Remove_Unlock_Gate_Objective", 100, 1000, false);
							Commands->Join_Conversation( NULL, unlocked_gate_conv, false, false, true );
							Commands->Start_Conversation( unlocked_gate_conv, unlocked_gate_conv );

							Commands->Monitor_Conversation(  obj, unlocked_gate_conv );
						}

						else if (commcenter_sam_destroyed == false)//
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_COMMCENTER_SAM_OBJECTIVE_JDG, 0 );
						}
						

						if (unlock_gate_objective_active == true)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_PASS_UNLOCK_GATE_OBJECTIVE_JDG, 6 );
						}
					}
					break;

				case M01_PASS_UNLOCK_GATE_OBJECTIVE_JDG:
					{
						Commands->Set_Objective_Status( M01_UNLOCK_GATE_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
					}
					break;

				case M01_PASS_COMM_SAM_OBJECTIVE_JDG:
					{
						Commands->Set_Objective_Status( M01_COMM_SAM_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
						commcenter_sam_objective_active = false;
						
						if (prisoners_are_freed == true)
						{
							Commands->Create_Logical_Sound ( obj, M01_DETENTION_GATE_DOWN_SAM_DEAD_JDG, Vector3 (-315.383f, 572.805f, 27.623f), 50.0f );
							Commands->Send_Custom_Event( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 60 );
						}
					}
					break;

				case M01_SCRAMBLE_THE_RADAR_JDG:
					{
						if (players_first_time_by_commcenter == true)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103099 ), 0, M01_START_ACTING_JDG, 0 );//telling propaganda to start up
							players_first_time_by_commcenter = false;

							int radar_conv = Commands->Create_Conversation( "M01_Radar_Scrambled_Conversation", 100, 1000, false);
							Commands->Join_Conversation( NULL, radar_conv, false, false, false );
							Commands->Join_Conversation( NULL, radar_conv, false, false, false );
							Commands->Start_Conversation( radar_conv,  radar_conv );
						}

						if ((radar_is_scrambled == false) && (comm_center_state != DESTROYED) )
						{
							radar_is_scrambled = true;
							Commands->Enable_Radar ( false );
						}
					}
					break;

				case M01_UNSCRAMBLE_THE_RADAR_JDG:
					{
						if (radar_is_scrambled == true)
						{
							Commands->Enable_Radar ( true);
							radar_is_scrambled = false;
						}
					}
					break;

				case M01_ANNOUNCE_PRISONER_OBJECTIVE_JDG:
					{
						int prisoner_conv = Commands->Create_Conversation( "M01_Add_Unlock_Gate_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, prisoner_conv, false, false, true );
						Commands->Start_Conversation( prisoner_conv,  prisoner_conv );

						Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_UNLOCK_GATE_OBJECTIVE_JDG, 10 );

						Commands->Set_Objective_Status( M01_PRISONER_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
					}
					break;

				case M01_CLEAR_PRISONERS_PASS_JDG://prisoners have been freed
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 0 );
					}
					break;

				case M01_DO_END_MISSION_CHECK_JDG://see if any primary objectives are active
					{
						if ( (commcenter_sam_objective_active != true) && (prisoners_are_freed == true) )
						{
							//Commands->Set_Clouds (0, 0, 5);
							Commands->Set_Wind (0, 0, 0, 5);
							Commands->Set_Rain (0, 5, false);
							Commands->Set_Lightning (0, 0, 0, 0, 0, 5);

							if (final_conv_played == false)
							{
								final_conv_played = true;

								//GameObject * evacController01 = Commands->Find_Object ( 103380 );
								//GameObject * evacController02 = Commands->Find_Object ( 103381 );

								//if (evacController01 != NULL)
								//{
								//	Commands->Send_Custom_Event( obj, evacController01, 0, M01_START_ACTING_JDG, 0 );
								//}

								//if (evacController02 != NULL)
								//{
								//	Commands->Send_Custom_Event( obj, evacController02, 0, M01_START_ACTING_JDG, 0 );
								//}

								GameObject * detentionPenCiv01 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_01_JDG );
								GameObject * detentionPenCiv02 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_02_JDG );
								GameObject * detentionPenCiv03 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_03_JDG );
								GameObject * detentionPenGDI01 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_01_JDG );
								GameObject * detentionPenGDI02 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_02_JDG );
								GameObject * detentionPenGDI03 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_03_JDG );

								if (detentionPenCiv01 != NULL)
								{
									Commands->Destroy_Object ( detentionPenCiv01 );
								}

								if (detentionPenCiv02 != NULL)
								{
									Commands->Destroy_Object ( detentionPenCiv02 );
								}

								if (detentionPenCiv03 != NULL)
								{
									Commands->Destroy_Object ( detentionPenCiv03 );
								}

								if (detentionPenGDI01 != NULL)
								{
									Commands->Destroy_Object ( detentionPenGDI01 );
								}

								if (detentionPenGDI02 != NULL)
								{
									Commands->Destroy_Object ( detentionPenGDI02 );
								}

								if (detentionPenGDI03 != NULL)
								{
									Commands->Destroy_Object ( detentionPenGDI03 );
								}
								Vector3 newPosition (51.023f, -35.918f, 0.246f);
								Commands->Set_Position ( STAR, newPosition );


								Commands->Set_Is_Rendered( obj, false );

								GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
								Commands->Attach_Script(controller1, "Test_Cinematic", "X1Z_Finale.txt");

								Commands->Send_Custom_Event( obj, obj, 0, M01_END_MISSION_PASS_JDG, 20 );
							}
						}
					}
					break;

				case M01_END_MISSION_PASS_JDG://see if any primary objectives are active
					{
						GameObject * propagandaController = Commands->Find_Object ( 103099 );
						if (propagandaController != NULL)
						{
							Commands->Send_Custom_Event( obj, propagandaController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
						}

						Commands->Mission_Complete ( true );

						
					}
					break;

				case M01_SPAWN_WHACK_A_MOLE_GUY_JDG:
					{
						GameObject * m01_whack_a_mole_guy = Commands->Create_Object ( "Nod_MiniGunner_1Off", Vector3(-198.088f, 525.828f, 10.035f) );
						Commands->Attach_Script(m01_whack_a_mole_guy, "M01_Whack_A_Mole_Minigunner_JDG", "");
						m01_whack_a_mole_guy_ID = Commands->Get_ID ( m01_whack_a_mole_guy );
					}
					break;

				case M01_CHANGE_WHACK_A_MOLE_GUY_JDG:
					{
						GameObject * whackAmole = Commands->Find_Object ( m01_whack_a_mole_guy_ID );
						if (whackAmole != NULL)
						{
							Commands->Send_Custom_Event( obj, whackAmole, 0, M01_CHANGE_WHACK_A_MOLE_GUY_JDG, 0 );
						}
					}
					break;

				case M01_HON_SPAWNS_MINIGUNNER_JDG:
					{	
						if (hand_of_nod_state != DESTROYED)
						{
							m01_HON_SpawnRate = 21;

							if (m01_hon_total_spawners_tally <= 6)
							{
								if ((m01_hon_spawned_guy_01_alive == false) && (players_location != INSIDE_HAND_OF_NOD))
								{
									GameObject * m01_hon_spawned_guy_01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-212.6f, 541.1f, 4));
									Commands->Attach_Script(m01_hon_spawned_guy_01, "M01_HON_Easy_Spawned_Guy_01_JDG", "");
									m01_hon_spawned_guy_01_ID = Commands->Get_ID ( m01_hon_spawned_guy_01 );
									m01_hon_spawned_guy_01_alive = true;
									m01_hon_spawners_tally++;
									m01_hon_total_spawners_tally++;
								}

								else if ((m01_hon_spawned_guy_02_alive == false) && (players_location != INSIDE_HAND_OF_NOD))
								{
									GameObject * m01_hon_spawned_guy_02 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-217.2f, 540.2f, 4) );
									Commands->Attach_Script(m01_hon_spawned_guy_02, "M01_HON_Easy_Spawned_Guy_02_JDG", "");
									m01_hon_spawned_guy_02_ID = Commands->Get_ID ( m01_hon_spawned_guy_02 );
									m01_hon_spawned_guy_02_alive = true;
									m01_hon_spawners_tally++;
									m01_hon_total_spawners_tally++;
								}

								else if ((m01_hon_spawned_guy_03_alive == false) && (players_location != INSIDE_HAND_OF_NOD))
								{
									GameObject * m01_hon_spawned_guy_03 = Commands->Create_Object ( "Nod_FlameThrower_0", Vector3(-217.6f, 533.6f, 4));
									Commands->Attach_Script(m01_hon_spawned_guy_03, "M01_HON_Easy_Spawned_Guy_03_JDG", "");
									m01_hon_spawned_guy_03_ID = Commands->Get_ID ( m01_hon_spawned_guy_03 );
									m01_hon_spawned_guy_03_alive = true;
									m01_hon_spawners_tally++;
									m01_hon_total_spawners_tally++;
								}
							}
							
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNS_MINIGUNNER_JDG, m01_HON_SpawnRate );
						}
					}
					break;

				case M01_HON_SPAWNER_IN_POSITION_JDG:
					{
						if (m01_hon_spawners_tally == 3)
						{
							GameObject * honSpawn01 = Commands->Find_Object ( m01_hon_spawned_guy_01_ID );
							if (honSpawn01 != NULL)
							{
								Commands->Send_Custom_Event( obj, honSpawn01, 0, M01_HUNT_THE_PLAYER_JDG, 0 );
								m01_hon_spawned_guy_01_alive = false;
								m01_hon_spawned_guy_01_ID = 0;
							}

							GameObject * honSpawn02 = Commands->Find_Object ( m01_hon_spawned_guy_02_ID );
							if (honSpawn02 != NULL)
							{
								Commands->Send_Custom_Event( obj, honSpawn02, 0, M01_HUNT_THE_PLAYER_JDG, 0 );
								m01_hon_spawned_guy_02_alive = false;
								m01_hon_spawned_guy_02_ID = 0;
							}

							GameObject * honSpawn03 = Commands->Find_Object ( m01_hon_spawned_guy_03_ID );
							if (honSpawn03 != NULL)
							{
								Commands->Send_Custom_Event( obj, honSpawn03, 0, M01_HUNT_THE_PLAYER_JDG, 0 );
								m01_hon_spawned_guy_03_alive = false;
								m01_hon_spawned_guy_03_ID = 0;
							}

							m01_hon_spawners_tally = 0;
						}
					}
					break;

				case M01_HON_SPAWNER_01_IS_DEAD_JDG:
					{
						m01_hon_spawned_guy_01_alive = false;
						m01_hon_spawners_tally--;
					}
					break;

				case M01_HON_SPAWNER_02_IS_DEAD_JDG:
					{
						m01_hon_spawned_guy_02_alive = false;
						m01_hon_spawners_tally--;
					}
					break;

				case M01_HON_SPAWNER_03_IS_DEAD_JDG:
					{
						m01_hon_spawned_guy_03_alive = false;
						m01_hon_spawners_tally--;
					}
					break;

				case M01_HON_SPAWNER_IS_DEAD_JDG:
					{
						m01_hon_total_spawners_tally--;
					}
					break;

				case M01_CUE_LOVESHACK_NUN_CONVERSATION_JDG:
					{
						GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID );
						if (loveshackNun != NULL && (church_guard_is_dead != true))
						{
							if (loveshack_conv_playing == false)
							{
								loveshack_conv_playing = true;
								loveshack_conv = Commands->Create_Conversation( "M01_Locke_ChurchReminder_Conversation", 45, 1000, true);
								Commands->Join_Conversation( NULL, loveshack_conv, false, false, true );
								//Commands->Join_Conversation( STAR, loveshack_conv, false, false, true );
								Commands->Start_Conversation( loveshack_conv,  loveshack_conv );
								Commands->Monitor_Conversation( obj, loveshack_conv );
							}
						}
					}
					break;

				case M01_CUE_PRIEST_CONVERSATION_JDG:
					{
						GameObject * priest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );
						if (priest != NULL && (loveshack_guard_is_dead != true))
						{
							if (priest_conv_playing == false)
							{
								priest_conv_playing = true;
								priest_conv = Commands->Create_Conversation( "M01_PriestReminder_Conversation", 100, 1000, false);
								Commands->Join_Conversation( priest, priest_conv, false, true, true );
								Commands->Join_Conversation( STAR, priest_conv, false, false, false );
								Commands->Start_Conversation( priest_conv,  priest_conv );
								Commands->Monitor_Conversation( obj, priest_conv );
							}
						}
					}
					break;

				case M01_CUE_INTERIOR_NUN_CONVERSATION_JDG:
					{
						GameObject * interiorNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
						if (interiorNun != NULL && (loveshack_guard_is_dead != true))
						{
							if (interior_nun_conv_playing == false)
							{
								interior_nun_conv_playing = true;
								interior_nun_conv = Commands->Create_Conversation( "M01_Interior_Nun_Conversation", 100, 1000, false);
								Commands->Join_Conversation( interiorNun, interior_nun_conv, false, true, true );
								Commands->Join_Conversation( STAR, interior_nun_conv, false, false, false);
								Commands->Start_Conversation( interior_nun_conv,  interior_nun_conv);
								Commands->Monitor_Conversation( obj, interior_nun_conv );
							}
						}
					}
					break;

				case M01_TURRET_HAS_BEEN_DESTROYED_JDG:
					{	
						GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
						if (turret01 == NULL)
						{
							turret_01_is_dead = true;
						}

						GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
						if (turret02 == NULL)
						{
							turret_02_is_dead = true;
						}

						if ((turret_01_is_dead == true) && (turret_02_is_dead == true) )
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_END_TURRETS_OBJECTIVE_PASS_JDG, 0 );
						}
					}
					break;

				case M01_SPAWN_THE_MEDIUM_TANK_JDG:
					{
						if (medium_tank_spawned == false)
						{
							//Commands->Create_Sound ( "00-N180E", Vector3 (0,0,0), obj);

							medium_tank_spawned = true;
							
							//GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-85.808f, 125.455f, 3.927f));
							//Commands->Set_Facing ( controller1, 180 );
							//Commands->Attach_Script(controller1, "Test_Cinematic", "X1I_GDI_Drop_MediumTank.txt");

							//GameObject *controller2 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
							//Commands->Attach_Script(controller2, "Test_Cinematic", "X1D_Hover_MTank.txt");
						}
					}
					break;

				case M01_ADD_BARN_COMMANDER_OBJECTIVE_JDG:
					{
						GameObject * barnCommander = Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG );
						if (barnCommander != NULL  )
						{
							Commands->Add_Objective( M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_01, NULL, IDS_Enc_Obj_Secondary_M01_01 );
							Commands->Set_Obj_Radar_Blip_Shape ( barnCommander, RADAR_BLIP_SHAPE_OBJECTIVE );	// Set to -1 to reset default
							Commands->Set_Obj_Radar_Blip_Color ( barnCommander, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );

							GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
							if (pogController != NULL)
							{
								Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG, 0 );
							}

							if (firstNodCommanderKilled == true)
							{
								Commands->Send_Custom_Event( obj, obj, 0, M01_CLEAR_FIRST_NOD_COMMANDER_PASS_JDG, 3 );
							}
						}
					}
					break;

				case M01_PLAYER_APPROACHING_BARN_AREA_JDG:
					{
						if (first_time_at_barn == true)
						{
							first_time_at_barn = false;

							GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
							if (turret01 != NULL)
							{
								Commands->Send_Custom_Event( obj, turret01, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
							if (turret02 != NULL)
							{
								Commands->Send_Custom_Event( obj, turret02, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * turretGunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
							if (turretGunboat != NULL)
							{
								Commands->Send_Custom_Event( obj, turretGunboat, 0, M01_START_ACTING_JDG, 0 );
							}

							GameObject * turretEngineer = Commands->Find_Object ( M01_TURRETBEACH_ENGINEER_ID );
							if (turretEngineer != NULL)
							{
								Commands->Send_Custom_Event( obj, turretEngineer, 0, M01_START_ACTING_JDG, 0 );
							}

							Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_TURRETS_OBJECTIVE_JDG, 0 );

							GameObject * barnAreaCommander = Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_02_JDG );
							if (barnAreaCommander != NULL)
							{
								Commands->Send_Custom_Event( obj, barnAreaCommander, 0, M01_START_ACTING_JDG, 5 );
							}

							//Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_BARN_CHINOOK_JDG, 30 );

							//Setting new weather values for barn area
							//Commands->Set_Clouds (0.5f, 0.5f, 120);
							Commands->Set_Wind (60, 1.0f, 0.5f, 120);
							Commands->Set_Rain (0.5f, 120, false);
						}
					}
					break;

				case M01_PLAYER_IS_APPROACHING_TAILGUN_ALLEY_JDG:
					{
						GameObject * tailgunCommander = Commands->Find_Object ( M01_TAILGUNAREA_NOD_COMMANDER_JDG);
						if (tailgunCommander != NULL)
						{
							Commands->Send_Custom_Event( obj, tailgunCommander, 0, M01_START_ACTING_JDG, 8 );
						}

						GameObject * artilleryController = Commands->Find_Object ( M01_GDIBASE_ARTILLERY_CONTROLLER_ID);
						if (artilleryController != NULL)
						{
							Commands->Send_Custom_Event( obj, artilleryController, 0, M01_GOTO_IDLE_JDG, 0 );
						}

						GameObject * lightTank = Commands->Find_Object ( M01_TAILGUNAREA_NOD_LIGHTTANK_JDG);
						if (lightTank != NULL)
						{
							Commands->Send_Custom_Event( obj, lightTank, 0, M01_START_ACTING_JDG, 0 );
						}

						
					}
					break;

				case M01_PLAYER_APPROACHING_GDI_BASE_AREA_JDG:
					{
						GameObject * artilleryController = Commands->Find_Object ( M01_GDIBASE_ARTILLERY_CONTROLLER_ID);
						if (artilleryController != NULL)
						{
							Commands->Send_Custom_Event( obj, artilleryController, 0, M01_START_ACTING_JDG, 0 );
						}
					}
					break;

				case M01_ANNOUNCE_TURRETS_OBJECTIVE_JDG:
					{
						turrets_conv = Commands->Create_Conversation( "M01_Add_Turrets_Objective", 100, 1000, false);
						Commands->Join_Conversation( NULL, turrets_conv, false, false, true );
						Commands->Join_Conversation( NULL, turrets_conv, false, false, true );
						Commands->Start_Conversation( turrets_conv,  turrets_conv );

						Commands->Monitor_Conversation( obj, turrets_conv );
					}
					break;

				case M01_ADD_TURRETS_OBJECTIVE_JDG:
					{
						Commands->Add_Objective( M01_TURRETS_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_02, NULL, IDS_Enc_Obj_Secondary_M01_02 );

						GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
						if (pogController != NULL)
						{
							Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_TURRETS_OBJECTIVE_JDG, 0 );
						}

						GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
						if (turret01 != NULL)
						{
							Commands->Set_Obj_Radar_Blip_Shape ( turret01, RADAR_BLIP_SHAPE_OBJECTIVE );
							Commands->Set_Obj_Radar_Blip_Color ( turret01, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
						}

						GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
						if (turret02 != NULL)
						{
							Commands->Set_Obj_Radar_Blip_Shape ( turret02, RADAR_BLIP_SHAPE_OBJECTIVE );
							Commands->Set_Obj_Radar_Blip_Color ( turret02, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
						}

						int c4_conv = Commands->Create_Conversation( "M01_Locke_Sending_C4_Conversation", 100, 1000, false);
						Commands->Join_Conversation( NULL, c4_conv, false, false, true );
						Commands->Start_Conversation( c4_conv,  c4_conv );

						Vector3 airdropPosition (-82.786f, 113.665f, 2.221f);
						GameObject *controller = Commands->Create_Object("Invisible_Object", airdropPosition);
						Commands->Attach_Script(controller, "Test_Cinematic", "X1D_GTower_FlareDrop.txt");

					}
					break;

				case M01_END_TURRETS_OBJECTIVE_FAIL_JDG:
					{
						if (turret_objective_over == false)
						{
							turret_objective_over = true;
							Commands->Set_Objective_Status( M01_TURRETS_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
						}
					}
					break;

				case M01_END_TURRETS_OBJECTIVE_PASS_JDG:
					{
						if (turret_objective_over == false)
						{
							turret_objective_over = true;
							GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
							if (gunboat != NULL)
							{
								Commands->Send_Custom_Event( obj, gunboat, 0, M01_END_TURRETS_OBJECTIVE_PASS_JDG, 0 );

								int turrets_conv_02 = Commands->Create_Conversation( "M01_Remove_Turrets_Objective", 100, 1000, false);
								Commands->Join_Conversation( NULL, turrets_conv_02, false, false, true );
								Commands->Start_Conversation( turrets_conv_02,  0 );
								Commands->Send_Custom_Event( obj, obj, 0, M01_PASS_TURRETS_OBJECTIVE_JDG, 5 );
							}

							else
							{
								//turrets_hummVee_conv = Commands->Create_Conversation( "M01_Locke_GiveHummVee_Conversation", 100, 1000, false);
								//Commands->Join_Conversation( NULL, turrets_hummVee_conv, false, false, true );
								//Commands->Join_Conversation( STAR, turrets_hummVee_conv, false, false, true );
								//Commands->Start_Conversation( turrets_hummVee_conv,  turrets_hummVee_conv );

								//Commands->Monitor_Conversation(  obj, turrets_hummVee_conv );
								Commands->Send_Custom_Event( obj, obj, 0, M01_PASS_TURRETS_OBJECTIVE_JDG, 5 );
							}
						}
					}
					break;

				case M01_PASS_TURRETS_OBJECTIVE_JDG:
					{
						Commands->Set_Objective_Status( M01_TURRETS_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SPAWN_THE_MEDIUM_TANK_JDG, 0 );
					}
					break;

				/*case M01_START_BARN_OBJECTIVE_JDG:
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_ANNOUNCE_BARN_OBJECTIVE_JDG, 0 );
					}
					break;

				case M01_ANNOUNCE_BARN_OBJECTIVE_JDG:
					{
						if (barn_announcement_active == false)
						{
							int barn_conv = Commands->Create_Conversation( "M01_Add_Barn_Objective", 100, 1000, false);

							Commands->Join_Conversation( NULL, barn_conv, false, false, true );
							Commands->Join_Conversation( STAR, barn_conv, false, false, true );
							Commands->Start_Conversation( barn_conv,  0 );

							Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_BARN_OBJECTIVE_JDG, 6 );

							barn_announcement_active = true;

							GameObject * barnObjectiveZone = Commands->Find_Object ( M01_BARN_OBJECTIVE_ZONE_ID );
							if (barnObjectiveZone != NULL)
							{
								Commands->Destroy_Object ( barnObjectiveZone );
							}
						}
					}
					break;*/

			/*case M01_ADD_BARN_OBJECTIVE_JDG:
					{
						if (barn_objective_active == false)
						{
							barn_objective_active = true;
							Commands->Add_Objective( M01_BARN_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_03, NULL, IDS_Enc_Obj_Secondary_M01_03 );

							GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
							if (pogController != NULL)
							{
								Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_BARN_OBJECTIVE_JDG, 0 );
							}

							GameObject * barnPrisoner01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
							if (barnPrisoner01 != NULL)
							{
								Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner01, RADAR_BLIP_SHAPE_OBJECTIVE );
								Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner01, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
							}

							GameObject * barnPrisoner02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
							if (barnPrisoner02 != NULL)
							{
								Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner02, RADAR_BLIP_SHAPE_OBJECTIVE );
								Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner02, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
							}

							GameObject * barnPrisoner03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );
							if (barnPrisoner03 != NULL)
							{
								Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner03, RADAR_BLIP_SHAPE_OBJECTIVE );
								Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner03, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
							}
						}
					}
					break;*/

				case M01_END_BARN_OBJECTIVE_JDG:
					{
						if (barn_objective_failed != true && civs_scattered == false)
						{
							civs_scattered = true;
							//Commands->Send_Custom_Event( obj, obj, 0, M01_CHANGE_BARN_OBJECTIVE_JDG, 5 );
							
							GameObject * barnPrisoner01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
							if (barnPrisoner01 != NULL)
							{
								//Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner01, RADAR_BLIP_SHAPE_OBJECTIVE );
								//Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner01, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
								Commands->Send_Custom_Event( obj, barnPrisoner01, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}

							GameObject * barnPrisoner02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
							if (barnPrisoner02 != NULL)
							{
								//Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner02, RADAR_BLIP_SHAPE_OBJECTIVE );
								//Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner02, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
								Commands->Send_Custom_Event( obj, barnPrisoner02, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}

							GameObject * barnPrisoner03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );
							if (barnPrisoner03 != NULL)
							{
								//Commands->Set_Obj_Radar_Blip_Shape ( barnPrisoner03, RADAR_BLIP_SHAPE_OBJECTIVE );
								//Commands->Set_Obj_Radar_Blip_Color ( barnPrisoner03, RADAR_BLIP_COLOR_SECONDARY_OBJECTIVE );
								Commands->Send_Custom_Event( obj, barnPrisoner03, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}
						}
					}
					break;

				/*case M01_CHANGE_BARN_OBJECTIVE_JDG:
					{
						if (barn_objective_failed != true)
						{
							int roundupcivs_conv = Commands->Create_Conversation( "M01_Add_Barn_Roundup_Objective", 100, 1000, false);
							Commands->Join_Conversation( NULL, roundupcivs_conv, false, false, true );
							Commands->Start_Conversation( roundupcivs_conv,  roundupcivs_conv );

							Commands->Set_Objective_Status( M01_BARN_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
							Commands->Add_Objective( M01_BARN_ROUNDUP_OBJECTIVE_JGD, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_04, NULL, IDS_Enc_Obj_Secondary_M01_04 );

							GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
							if (pogController != NULL)
							{
								Commands->Send_Custom_Event( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_BARN_ROUNDUP_OBJECTIVE_JGD, 0 );
							}
						}
					}
					break;*/

				case M01_END_BARN_ROUNDUP_OBJECTIVE_JDG:
					{
						if (barn_objective_ended == false)
						{
							barn_objective_ended = true;
							/*if (barn_objective_failed == true)
							{
								Commands->Set_Objective_Status( M01_BARN_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
								Commands->Set_Objective_Status( M01_BARN_ROUNDUP_OBJECTIVE_JGD, OBJECTIVE_STATUS_FAILED );

								barnfailed_conv = Commands->Create_Conversation( "M01_Locke_3CivsDead_Conversation", 100, 1000, false);
								Commands->Join_Conversation( NULL, barnfailed_conv, false, false, true );
								Commands->Join_Conversation( STAR, barnfailed_conv, false, false, true );
								Commands->Start_Conversation( barnfailed_conv,  barnfailed_conv );

								Commands->Monitor_Conversation(  obj, barnfailed_conv );
							}*/

							if (barn_objective_failed != true)
							{
								//Commands->Set_Objective_Status( M01_BARN_ROUNDUP_OBJECTIVE_JGD, OBJECTIVE_STATUS_ACCOMPLISHED );

								//barnfailed_conv = Commands->Create_Conversation( "M01_AirDrop_Conversation", 100, 1000, false);
								//Commands->Join_Conversation( NULL, barnfailed_conv, false, false, true );
								//Commands->Start_Conversation( barnfailed_conv,  barnfailed_conv );

								//Commands->Monitor_Conversation(  obj, barnfailed_conv );
							}
						}
					}
					break;

				case M01_TURRET_BEACH_ENGINEER_IS_DEAD_JDG:
					{
						GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
						GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
						if ( (turret01 != NULL) || (turret02 != NULL) )
						{
							Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );//enemy aircraft detected
							GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-93.689f, 52.850f, -1.95f));
							Commands->Set_Facing(chinook_obj1, 60.0f);
							Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_Nod_TurretBeach_TroopDrop.txt");
						}
					}
					break;

				case M01_CUE_BILLYS_CONVERSATION_JDG:
					{
						GameObject * barnPrisoner02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
						if ( barnPrisoner02 != NULL && barn_prisoner_02_is_dead != true) //this is billy
						{
							billy_conv = Commands->Create_Conversation( "M01_Billys_Conversation", 100, 1000, true);
							Commands->Join_Conversation( barnPrisoner02, billy_conv, true, true, true );
							Commands->Join_Conversation( STAR, billy_conv, false, true, true );
							Commands->Join_Conversation( NULL, billy_conv, false, false, true );
							Commands->Start_Conversation( billy_conv,  billy_conv );
							Commands->Monitor_Conversation(  obj, billy_conv );
						}
					}
					break;

				case M01_CUE_BABUSHKA_CONVERSATION_JDG:
					{
						GameObject * barnPrisoner01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
						GameObject * barnPrisoner03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );
						if ( barnPrisoner01 != NULL && barn_prisoner_01_is_dead != true) //this is babushka
						{
							babushka_conv = Commands->Create_Conversation( "M01_Barn_Babushka_Conversation", 100, 1000, true);
							Commands->Join_Conversation( barnPrisoner01, babushka_conv, true, true, true );
							Commands->Join_Conversation( STAR, babushka_conv, false, false, false );
							Commands->Join_Conversation( NULL, babushka_conv, false, false, false );
							Commands->Start_Conversation( babushka_conv,  babushka_conv );
							Commands->Monitor_Conversation(  obj, babushka_conv );

							Commands->Send_Custom_Event( obj, barnPrisoner01, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 0 );

							if ( barnPrisoner03 != NULL && barn_prisoner_03_is_dead != true) //this is babushka
							{
								Commands->Send_Custom_Event( obj, barnPrisoner03, 0, M01_MODIFY_YOUR_ACTION_11_JDG, 0 );
							}
						}

						else if ( barnPrisoner03 != NULL && barn_prisoner_03_is_dead != true) //this is babushka
						{
							civ3_conv = Commands->Create_Conversation( "M01_Civ03_Conversation", 100, 1000, true);
							Commands->Join_Conversation( barnPrisoner03, civ3_conv, true, true, true );
							Commands->Join_Conversation( STAR, civ3_conv, false, false, false );
							Commands->Join_Conversation( NULL, civ3_conv, false, true, true );
							Commands->Start_Conversation( civ3_conv,  civ3_conv );
							Commands->Monitor_Conversation(  obj, civ3_conv );
						}
						
					}
					break;

				case M01_BARN_GUARD_IS_DEAD_JDG:
					{
						dead_barn_guard_tally++;

						if (dead_barn_guard_tally >= 4)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_END_BARN_OBJECTIVE_JDG, 0 );
						}
					}
					break;	

				case M01_CIVILIAN_KILLED_JDG:
					{
						/*if (sender == Commands->Find_Object ( M01_BARN_PRISONER_01_ID ))//this is babushka
						{
							barn_prisoner_01_is_dead = true;
							dead_barn_prisoner_tally++;
						}

						else if (sender == Commands->Find_Object ( M01_BARN_PRISONER_02_ID ))//this is billy
						{
							barn_prisoner_02_is_dead = true;
							dead_barn_prisoner_tally++;
						}

						else if (sender == Commands->Find_Object ( M01_BARN_PRISONER_03_ID ))//this is pierre
						{
							barn_prisoner_03_is_dead = true;
							dead_barn_prisoner_tally++;
						}

						if (dead_barn_prisoner_tally > 2)
						{
							barn_objective_failed = true;
							Commands->Send_Custom_Event( obj, obj, 0, M01_END_BARN_ROUNDUP_OBJECTIVE_JDG, 0 );
						}*/
					}
					break;

				case M01_KILL_THE_GDI_POWER_PLANT_JDG:
					{
						Commands->Debug_Message ( "***************************Starting the GDI base encounter\n" );

						if (missionStart == true)
						{
							missionStart = false;
							
						}

						//GameObject * crashed_orca = Commands->Find_Object ( 103121 );
						//if (crashed_orca != NULL) 
						//{
						//	Commands->Send_Custom_Event( obj, crashed_orca, 0, M01_START_ACTING_JDG, 0 );
						//}
					}
					break;

				case M01_DO_DAMAGE_CHECK_JDG://a building has been damaged--take appropriate action
					{	
						if (sender == Commands->Find_Object(M01_HAND_OF_NOD_JDG))//hand of nod under attack--refinery okay--send engineer
						{
							GameObject * honEngineer = Commands->Find_Object ( M01_HON_REPAIR_ENGINEER_ID );
							if (honEngineer != NULL)
							{
								Commands->Send_Custom_Event( obj, honEngineer, 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
							}
						}

						else if (sender == Commands->Find_Object(M01_COMM_CENTER_JDG))
						{
							GameObject * commEngineer = Commands->Find_Object ( M01_COMM_CENTER_ENGINEER_ID );
							if (commEngineer != NULL)
							{
								Commands->Send_Custom_Event( obj, commEngineer, 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
							}
						}
					}
					break;

				case M01_CUE_KANE_AND_NUMBER2_CONVERSATION_JDG://player is in comm center--cue kane encounter
					{
						GameObject * holograph = Commands->Find_Object ( 157978 );
						if (holograph != NULL)
						{
							GameObject * kane = Commands->Create_Object_At_Bone ( holograph, "Nod_Kane_HologramHead", "KANEBONE" );
							Commands->Attach_To_Object_Bone( kane, holograph, "KANEBONE" );
							Commands->Set_Loiters_Allowed( kane, false );
							kane_ID = Commands->Get_ID ( kane );

							GameObject * commBaseCommander = Commands->Find_Object ( M01_COMMCENTER_BASE_COMMANDER_JDG );
							if (commBaseCommander != NULL)
							{
								kane_and_no2_conv01_active = true;

								kane_and_no2_conv01 = Commands->Create_Conversation( "M01_Kane_and_Number02_Conversation_01", 100, 1000, true);
								Commands->Join_Conversation( kane, kane_and_no2_conv01, false, true, true );
								Commands->Join_Conversation( commBaseCommander, kane_and_no2_conv01, false, true, true );
								Commands->Start_Conversation( kane_and_no2_conv01,  kane_and_no2_conv01 );
							}
						}
					}
					break;

				case M01_CUE_KANE_AND_HAVOC_CONVERSATION_JDG://player is in comm center war room--cue kane encounter
					{
						if (kane_and_no2_conv01_active == true)
						{
						}

						else if (kane_and_no2_conv02_active == true)
						{
						}

						if (kane_and_havoc_conv_active == false)
						{
							GameObject * kane = Commands->Find_Object ( kane_ID );
							if (kane != NULL)
							{
								kane_and_havoc_conv_active = true;
								kane_and_havoc_conv = Commands->Create_Conversation( "M01_Kane_and_Havoc_Conversation", 100, 1000, false);
								Commands->Join_Conversation( kane, kane_and_havoc_conv, false, true, true );
								Commands->Join_Conversation( STAR, kane_and_havoc_conv, false, false, true );
								Commands->Start_Conversation( kane_and_havoc_conv,  kane_and_havoc_conv );
									
								Commands->Monitor_Conversation(  obj, kane_and_havoc_conv );
							}
							
						}
					}
					break;

				case M01_SEND_BARN_CHINOOK_JDG://havoc is in barn area--send reinforcements
					{
						Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );//enemy aircraft detected
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-87.561f, 114.225f, 1.874f));
						Commands->Set_Facing(chinook_obj1, 0.0f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_EasyNodChinook01.txt");
					}
					break;

				case M01_SEND_GUARD_TOWER_CHINOOK_JDG://havoc is at GDI guard tower
					{
						Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(12.703f, 84.163f, 1.898f));
						Commands->Set_Facing(chinook_obj1, -130.0f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_EasyNodChinook01.txt");	
					}
					break;

				case M01_SEND_TAILGUN_CHINOOK_JDG://havoc is in tailgun area--send reinforcementsM01_SEND_CHURCH_CHINOOK_JDG
					{
						Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );//enemy aircraft detected
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-7.831f, 292.034f, 2.665f));
						Commands->Set_Facing(chinook_obj1, 0.0f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_EasyNodChinook01.txt");
					}
					break;


				case M01_SEND_CHURCH_CHINOOK_JDG://havoc is in church area--send reinforcements
					{
						Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );//enemy aircraft detected
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-91.898f, 424.178f, 20.761f));
						Commands->Set_Facing(chinook_obj1, 0.0f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_EasyNodChinook.txt");
					}
					break;

				case M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG://havoc has found one of the barn civs--send in the chinook
					{
						GameObject * barnEvacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
						if (barnEvacController != NULL)
						{
							Commands->Send_Custom_Event( obj, barnEvacController, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
						}
					}
					break;	
			}
		}	
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == billy_conv) 
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
					}		

					else if (action_id == babushka_conv) 
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
						GameObject * barnPrisoner01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );

						if ( barnPrisoner01 != NULL && barn_prisoner_01_is_dead != true) //this is babushka
						{
							Commands->Send_Custom_Event( obj, barnPrisoner01, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 0 );
						}//int 
					}

					else if (action_id == civ3_conv) 
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
					}

					else if (action_id == turrets_conv) 
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_ADD_TURRETS_OBJECTIVE_JDG, 0 );
					}

					//else if (action_id == turrets_hummVee_conv) //send in the humm vee
					//{
						//GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-96.744f, 138.498f, 1));
						//Commands->Set_Facing(chinook_obj1, 60.0f);
						//Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "XG_M01_HumveeDrop.txt");
					//}

					else if (action_id == barnfailed_conv) 
					{
						//int honAlerted_conv = Commands->Create_Conversation( "M01_HandOfNod_Alerted_Conversation", 100, 1000, false);
						//Commands->Join_Conversation( NULL, honAlerted_conv, false, false, true );
						//Commands->Join_Conversation( STAR, honAlerted_conv, false, false, true );
						//Commands->Start_Conversation( honAlerted_conv,  honAlerted_conv );
					}

					else if (action_id == kane_and_no2_conv01) 
					{
						kane_and_no2_conv01_active = false;

						GameObject * securityCamera = Commands->Find_Object ( M01_COMMCENTER_CEILING_CAMERA_JDG);
						if (securityCamera != NULL)
						{	
							Commands->Send_Custom_Event( obj, securityCamera, 0, M01_START_ATTACKING_01_JDG, 0 );
						}
					}

					else if (action_id == kane_and_no2_conv02) 
					{
						kane_and_no2_conv02_active = false;
					}

					else if (action_id == kane_and_havoc_conv) 
					{
						
						GameObject * kane = Commands->Find_Object ( kane_ID);
						if (kane != NULL)
						{
							Commands->Destroy_Object ( kane );
						}
					}

					else if (action_id == unlocked_gate_conv) //
					{
						Vector3 radarMarker (-310.727f, 571.824f, 28.484f);
						Commands->Add_Objective( M01_OPEN_THE_GATE_JDG, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M01_03, NULL, IDS_Enc_Obj_Primary_M01_03 );
						Commands->Set_Objective_Radar_Blip( M01_OPEN_THE_GATE_JDG, radarMarker );
						Commands->Set_Objective_HUD_Info_Position( M01_OPEN_THE_GATE_JDG, 95, "POG_M01_1_03.tga", IDS_POG_OPEN, radarMarker );

						//Vector3 green (0,1,0);
						Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Primary_M01_03, TEXT_COLOR_OBJECTIVE_PRIMARY );
					}

					else if (action_id == endMission_conv) 
					{
						Commands->Create_2D_Sound ( "00-N048E" );//eva primary accomplished
						Commands->Send_Custom_Event( obj, obj, 0, M01_END_MISSION_PASS_JDG, 3 );
					}

					else if (action_id == loveshack_conv)
					{
						loveshack_conv_playing = false;
					}

					else if (action_id == priest_conv)
					{
						priest_conv_playing = false;
					}

					else if (action_id == interior_nun_conv) 
					{
						interior_nun_conv_playing = false;
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_HON_Medlab_DropOff_Guy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Enable_Hibernation( obj, false );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_TroopDrop") == 0)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(Vector3 (0,0,0), RUN, 1);
			params.WaypathID = 125265;
			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Destroy_Object ( obj );
		}

		else
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(Vector3 (0,0,0), RUN, 1);
			params.WaypathID = 125265;
			Commands->Action_Goto(obj, params);
		}
	}
};

DECLARE_SCRIPT(M01_Hand_of_Nod_Building_Script_JDG, "")
{
	int memorial_sound;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(memorial_sound, 1);
	}

	void Created( GameObject * obj ) override 
	{
		Vector3 memorialSpot (-195.053f, 533.737f, 10.953f);
		memorial_sound = Commands->Create_Sound ( "m01eval_dsgn0254r1ncxk_snd", memorialSpot, obj );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Stop_Sound ( memorial_sound, true );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_HAS_BEEN_DESTROYED_JDG, 0 );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_HON_HAS_BEEN_DESTROYED_JDG, 0 );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103371 ), 0, M01_HON_HAS_BEEN_DESTROYED_JDG, 0 );//this is the final escort zone

		Commands->Create_Object ( "POW_Medal_Armor", Vector3 (-180.312f, 541.400f, 3.518f) );
		Commands->Create_Object ( "POW_Medal_Health", Vector3 (-178.349f, 541.471f, 3.518f) );

		

	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj )
		{
			GameObject * myEngineer = Commands->Find_Object ( M01_HON_REPAIR_ENGINEER_ID );
			if (myEngineer != NULL)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_DO_DAMAGE_CHECK_JDG, 0 );

				if (damager == myEngineer)
				{
					float MyMaxHealth = Commands->Get_Max_Health ( obj );
					float MyMaxShield = Commands->Get_Max_Shield_Strength ( obj );
					Commands->Set_Health ( obj, MyMaxHealth );
					Commands->Set_Shield_Strength ( obj, MyMaxShield );
				}
			}
		}
	}
};


DECLARE_SCRIPT(M01_Flyover_Generic_Script_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Enable_Hibernation( obj, false );
		Commands->Enable_Cinematic_Freeze( obj, false );

		float MyMaxHealth = Commands->Get_Max_Health ( obj );
		float fudgeFactor = Commands->Get_Random ( .05f, 1.0f);
		float myNewHealth = MyMaxHealth * fudgeFactor;

		float MyMaxShield = Commands->Get_Max_Shield_Strength ( obj );
		float fudgeFactor2 = Commands->Get_Random ( .05f, 1.0f);
		float myNewShield = MyMaxShield * fudgeFactor2;

		Commands->Set_Health ( obj, myNewHealth );
		Commands->Set_Shield_Strength ( obj, myNewShield );
	}
};



DECLARE_SCRIPT(M01_Ambient_Sound_Controller_JDG, "")
{
	enum 
	{
		play_destroyed_building_sound = 101,
		pick_refinery_sound,
		play_refinery_klaxon,
		play_refinery_sound,
		pick_commcenter_sound,
		play_commcenter_klaxon,
		play_commcenter_sound,
	};

	typedef enum {
		ACTIVE,
		DESTROYED,
	} M01_Building_State;

	typedef enum {
		INSIDE_GDI_CON,
		INSIDE_GDI_POWERPLANT,
		INSIDE_HAND_OF_NOD,
		INSIDE_REFINERY,
		INSIDE_COMM_CENTER,
		OUTSIDE,
	} M01_Location;

	M01_Building_State hand_of_nod_state, comm_center_state;
	M01_Location players_location;

	int building_sound;
	int destroyed_building_sound;
	int klaxon;
	int sound;
	bool outside_ambients_playing;
	bool building_ambients_playing;
	bool destroyed_building_ambients_playing;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hand_of_nod_state, 1);
		SAVE_VARIABLE(comm_center_state, 2);
		SAVE_VARIABLE(players_location, 3);
		SAVE_VARIABLE(building_sound, 4);
		SAVE_VARIABLE(destroyed_building_sound, 5);
		SAVE_VARIABLE(klaxon, 6);
		SAVE_VARIABLE(sound, 7);
		SAVE_VARIABLE(outside_ambients_playing, 8);
		SAVE_VARIABLE(building_ambients_playing, 9);
		SAVE_VARIABLE(destroyed_building_ambients_playing, 10);
	}

	void Created( GameObject * obj ) override
	{
		Commands->Enable_Hibernation( obj, false );

		hand_of_nod_state = ACTIVE;
		comm_center_state = ACTIVE;

		klaxon = 0;
		sound = 0;

		outside_ambients_playing = false;
		building_ambients_playing = false;
		destroyed_building_ambients_playing = false;
		players_location = OUTSIDE;

		Commands->Send_Custom_Event( obj, obj, 0, M01_TURN_ON_OUTSIDE_AMBIENTS_JDG, 0 );
		
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
	    if ((param == M01_TURN_ON_OUTSIDE_AMBIENTS_JDG) && (outside_ambients_playing == false))
		{
			players_location =	OUTSIDE;
			outside_ambients_playing = true;

			if (building_ambients_playing == true /*&& building_sound != NULL*/)
			{
				Commands->Stop_Sound ( building_sound, true );	
			}

			building_ambients_playing = false;
			destroyed_building_ambients_playing = false;
		}

		else if ((param == M01_TURN_OFF_OUTSIDE_AMBIENTS_JDG) && (outside_ambients_playing == true))
		{
			outside_ambients_playing = false;
		}

		else if ((param == M01_HON_HAS_BEEN_DESTROYED_JDG))//Hand of Nod has been destroyed
		{
			hand_of_nod_state = DESTROYED;
		}

		else if ((param == M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG))//Comm Center has been destroyed
		{
			comm_center_state = DESTROYED;
		}

		else if (param == M01_PLAYER_IS_INSIDE_GDI_CON_JDG) 
		{
			players_location = INSIDE_GDI_CON;
		}

		else if (param == M01_PLAYER_IS_INSIDE_GDI_POWERPLANT_JDG) 
		{
			players_location = INSIDE_GDI_POWERPLANT;
		}

		else if (param == M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG) 
		{
			players_location = INSIDE_COMM_CENTER;

			if ((comm_center_state == ACTIVE) && (building_ambients_playing != true) )
			{
				Commands->Debug_Message ( "**********************turning on active Comm Center ambient noise\n" );
				building_sound = Commands->Create_Sound ("CC_Ambient_M01", Vector3 (0,0,0), obj );
				building_ambients_playing = true;

				float delayTimer = Commands->Get_Random ( 5, 10 );
				Commands->Send_Custom_Event( obj, obj, 0, pick_commcenter_sound, delayTimer );
			}
		}

		else if (param == M01_PLAYER_IS_INSIDE_HON_JDG) 
		{
			players_location = INSIDE_HAND_OF_NOD;

			if ((hand_of_nod_state == ACTIVE) && (building_ambients_playing != true) )
			{
				building_ambients_playing = true;
			}
		}

		else if ((param == play_commcenter_klaxon)  && (players_location == INSIDE_COMM_CENTER) && (comm_center_state != DESTROYED))
		{
			static constexpr const char *klaxonNames[2] = {
				"Klaxon Warning",
				"Klaxon Info"
			};

			const char* klaxonName = klaxonNames[klaxon];

			GameObject * star = Commands->Get_A_Star (Vector3(0.0f,0.0f,0.0f));
			if ( star != NULL ) 
			{
				Vector3 playerPosition = Commands->Get_Position ( star );

				float x_factor = Commands->Get_Random(-5,5);
				float y_factor = Commands->Get_Random(-5,5);

				playerPosition.X +=  x_factor;
				playerPosition.Y +=  y_factor;
				playerPosition.Z += 3;

				Commands->Create_Sound ( klaxonName, playerPosition, obj );

				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 1.5 );
			}
		}

		else if ((param == play_commcenter_sound) && (players_location == INSIDE_COMM_CENTER) && (comm_center_state != DESTROYED))
		{
			static constexpr const char* sounds[15] = {
				"01-I022E",
				"01-I028E",
				"01-I030E",
				"01-I032E",
				"01-I034E",
				"01-I036E",
				"01-I038E",
				"01-I040E",
				"01-I042E",
				"01-I044E",
				"01-I046E",
				"01-I048E",
				"01-I050E",
				"01-I062E",
				"01-I066E"
			};

			const char *soundName = sounds[sound];

			GameObject * star = Commands->Get_A_Star (Vector3(0.0f,0.0f,0.0f));
			if ( star != NULL ) 
			{
				Vector3 playerPosition = Commands->Get_Position ( star );

				float x_factor = Commands->Get_Random(-5,5);
				float y_factor = Commands->Get_Random(-5,5);

				playerPosition.X += x_factor;
				playerPosition.Y += y_factor;
				playerPosition.Z += 3;

				Commands->Create_Sound ( soundName, playerPosition, obj );

				float delayTimer = Commands->Get_Random ( 10, 25 );
				Commands->Send_Custom_Event( obj, obj, 0, pick_commcenter_sound, delayTimer );
			}
		}

		else if ((param == pick_commcenter_sound) && (players_location == INSIDE_COMM_CENTER) && (comm_center_state != DESTROYED))
		{
			float lineNumber = Commands->Get_Random ( 0.5f, 15.5f);

			if ((lineNumber >= 0.5) && (lineNumber < 1.5))
			{
				sound = 0;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 1.5) && (lineNumber < 2.5))
			{
				sound = 1;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 2.5) && (lineNumber < 3.5))
			{
				sound = 2;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 3.5) && (lineNumber < 4.5))
			{
				klaxon = 1;
				sound = 3;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 4.5) && (lineNumber < 5.5))
			{
				sound = 4;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 5.5) && (lineNumber < 6.5))
			{
				klaxon = 1;
				sound = 5;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 6.5) && (lineNumber < 7.5))
			{
				klaxon = 1;
				sound = 6;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 7.5) && (lineNumber < 8.5))
			{
				klaxon = 0;
				sound = 7;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 8.5) && (lineNumber < 9.5))
			{
				sound = 8;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 9.5) && (lineNumber < 10.5))
			{
				klaxon = 0;
				sound = 9;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 10.5) && (lineNumber < 11.5))
			{
				klaxon = 0;
				sound = 10;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 11.5) && (lineNumber < 12.5))
			{
				klaxon = 1;
				sound = 11;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_klaxon, 0 );
			}

			else if ((lineNumber >= 12.5) && (lineNumber < 13.5))
			{
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else if ((lineNumber >= 13.5) && (lineNumber < 14.5))
			{
				sound = 13;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}

			else 
			{
				sound = 14;
				Commands->Send_Custom_Event( obj, obj, 0, play_commcenter_sound, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Entering_Church_Area_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_APPROACHING_CHURCH_JDG, 0 );

			if (Commands->Find_Object ( 100801 ))//these are the three zones leading to the church--clean them up
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100801 ) );
			}

			if (Commands->Find_Object ( 100802 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100802 ) );
			}

			if (Commands->Find_Object ( 100803 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100803 ) );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Hand_Of_Nod_Interior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_INSIDE_HON_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Hand_Of_Nod_Exterior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_OUTSIDE_HON_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Center_Interior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Center_Exterior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_OUTSIDE_COMM_CENTER_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Tunnel_Interior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_INSIDE_TUNNEL_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Tunnel_Exterior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_OUTSIDE_TUNNEL_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Waterfall_Interior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_INSIDE_WATERFALL_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Waterfall_Exterior_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_OUTSIDE_WATERFALL_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Player_is_Leaving_GDI_Base_Zone, "")
{
	bool firstEntry;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(firstEntry, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		firstEntry = true;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (firstEntry == true)
			{
				firstEntry = false;

				GameObject * barnSpawnerController = Commands->Find_Object (M01_GDIBASE_SPAWNER_CONTROLLER_JDG);
				if (barnSpawnerController != NULL)
				{
					Commands->Send_Custom_Event( obj, barnSpawnerController, 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
				}
			}

			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_APPROACHING_BARN_AREA_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Player_is_Entering_GDI_Base_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_APPROACHING_GDI_BASE_AREA_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Lose_Any_Church_Escorts_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_LEAVING_CHURCH_01_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Lose_Any_Church_Escorts_Zone_02, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_LEAVING_CHURCH_02_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Lose_Any_HON_Escorts_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_LEAVING_HON_01_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Lose_Any_HON_Escorts_Zone_02, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_LEAVING_HON_02_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Announce_Prisoner_Objective_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_ANNOUNCE_PRISONER_OBJECTIVE_JDG, 0 );
			Commands->Destroy_Object ( obj );//this is a one-time only zone--clean it up
		}
	}
};

DECLARE_SCRIPT(M01_Scramble_Radar_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SCRAMBLE_THE_RADAR_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_UnScramble_Radar_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_UNSCRAMBLE_THE_RADAR_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Commander_Guy, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG), 0, M01_CARD_CARRIER_HAS_BEEN_KILLED_JDG, 0 );//tell controller you've been killed
	}
};

DECLARE_SCRIPT(M01_COMM_Commander_Guy, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG), 0, M01_CARD_CARRIER_HAS_BEEN_KILLED_JDG, 0 );//tell controller you've been killed
	}
};



DECLARE_SCRIPT(M01_HandOfNod_SAMSite_Script, "")//M01_HON_SAM_SITE_JDG==100031
{
	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == (STAR))
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (M01_MISSION_CONTROLLER_JDG), 0, M01_HON_SAMSITE_HAS_BEEN_DESTROYED_JDG, 5 );	
	}
};



DECLARE_SCRIPT(M01_Destroyed_SAMSITE_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;

		Vector3 my_target = Commands->Get_Position ( obj );

		my_target.Z -= -5;

		params.Set_Basic( this, 100, 20 );
		params.Set_Attack( my_target, 0, 0, true );
		params.AttackCheckBlocked = true;
		Commands->Action_Attack (obj, params);
	}
};

DECLARE_SCRIPT(M01_CommCenter_Evacuator_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		ActionParamsStruct params;
		Vector3 evac_spot;

		float evac_location = Commands->Get_Random ( 0.5, 2.5f );

		if (evac_location >= 0.5 && evac_location < 1.5)
		{
			evac_spot.Set(-286.904f, 557.896f, 27.616f);
		}

		else 
		{
			evac_spot.Set(-318.149f, 530.868f, 27.971f);
		}

		float x_factor = Commands->Get_Random(-5,5);
		float y_factor = Commands->Get_Random(-5,5);

		evac_spot.X += x_factor;
		evac_spot.Y += y_factor;

		params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
		params.Set_Movement( evac_spot, 1.0f, 2 );
		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//Tech is at workstation--face monitor
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_FrontDoor_Evacuator_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;
		Commands->Enable_Hibernation( obj, false );
		params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_PICK_A_NEW_LOCATION_JDG);
		params.Set_Movement( Vector3(-150.3f, 532.0f, 4.2f), 1.0f, 0.25f );
		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		if (action_id == M01_PICK_A_NEW_LOCATION_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{ 
			Vector3 evac_spot;

			float evac_location = Commands->Get_Random ( 0.5, 3.5f );

			if (evac_location >= 0.5 && evac_location < 1.5)
			{
				evac_spot.Set(-156.637f, 553.163f, 4.295f);
			}

			else if (evac_location >= 1.5 && evac_location < 2.5)
			{
				evac_spot.Set(-145.746f, 531.964f, 4.271f);
			}

			else 
			{
				evac_spot.Set(-163.828f, 515.290f, 4.258f);
			}

			float x_factor = Commands->Get_Random(-10,10);
			float y_factor = Commands->Get_Random(-10,10);

			evac_spot.X += x_factor;
			evac_spot.Y += y_factor;

			params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evac_spot, 1.0f, 3 );
			Commands->Action_Goto( obj, params );
		}
			
		else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//Tech is evac'd allow hibernation
		{
			Commands->Enable_Hibernation( obj, true );
		} 
	}
};

DECLARE_SCRIPT(M01_HON_BackDoor_Evacuator_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;
		Commands->Enable_Hibernation( obj, false );
		Vector3 evac_spot;

		evac_spot.Set(-221.947f, 515.124f, 4.012f);

		float x_factor = Commands->Get_Random(-10,10);
		float y_factor = Commands->Get_Random(-10,10);

		evac_spot.X += x_factor;
		evac_spot.Y += y_factor;

		params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
		params.Set_Movement( evac_spot, 1.0f, 3 );
		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//Tech is evac'd allow hibernation
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Dorm_Crapper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, -120 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				//params.Set_Basic(this, 60, 100);
				//params.Set_Animation ("H_A_V22A", true);
				//Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{ 
			GameObject * star = Commands->Get_A_Star (Vector3(0.0f,0.0f,0.0f));
			if ( star != NULL ) 
			{
				int players_ID_number = Commands->Get_ID ( star );

				params.Set_Basic( this, 90, 50 );
				params.Set_Movement( Commands->Find_Object ( players_ID_number ), .8f, 1.5f );
				params.MoveFollow = true;
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG), 0, M01_CARD_CARRIER_HAS_BEEN_KILLED_JDG, 0 );//tell controller you've been killed
	}
};

DECLARE_SCRIPT(M01_HON_Dorm_FlameGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, 90 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 60, 100);
				params.Set_Animation ("H_A_601A", true);
				Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Dorm_ChemGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, -90 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 60, 100);
				params.Set_Animation ("H_A_613A", true);
				Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Dorm_RocketGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, -90 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 60, 100);
				params.Set_Animation ("H_A_601A", true);
				Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Dorm_MiniGunner_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, 180 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 60, 100);
				params.Set_Animation ("H_A_442A", true);
				Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Cafeteria_Eating_Guy_JDG, "")
{
	int enemy_detected;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_detected, 1 );
	}

	
	void Created( GameObject * obj ) override 
	{
		enemy_detected = 0;
		Commands->Innate_Disable(obj);
		Commands->Set_Facing ( obj, -125 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 60, 100);
				params.Set_Animation ("H_A_V11A", true);
				Commands->Action_Play_Animation (obj, params);
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Cafeteria_Walking_Guy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering Grunt level--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( Vector3(0,0,0), WALK, .25f );
				params.WaypathID = 100661;
				params.WaypointStartID = 100662;
				params.WaypointEndID = 100663;
				Commands->Action_Goto( obj, params );
			}
		}

		else if ((param == M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG))//Your building has been destroyed--evacuate
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Enable_Hibernation( obj, false );
				Vector3 evac_spot;

				float evac_location = Commands->Get_Random ( 0.5, 4.5f );

				if (evac_location >= 0.5 && evac_location < 1.5)
				{
					evac_spot.Set(-156.637f, 553.163f, 4.295f);
				}

				else if (evac_location >= 1.5 && evac_location < 2.5)
				{
					evac_spot.Set(-145.746f, 531.964f, 4.271f);
				}

				else if (evac_location >= 2.5 && evac_location < 3.5)
				{
					evac_spot.Set(-163.828f, 515.290f, 4.258f);
				}

				else 
				{
					evac_spot.Set(-221.947f, 515.124f, 4.012f);
				}

				float x_factor = Commands->Get_Random(-10,10);
				float y_factor = Commands->Get_Random(-10,10);

				evac_spot.X += x_factor;
				evac_spot.Y += y_factor;

				params.Set_Basic(this, M01_EVAC_PRIORITY_JDG, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evac_spot, 1.0f, 3 );

				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{ 
			params.Set_Basic(this, 45, M01_FACING_SPECIFIED_DIRECTION_01_JDG);
			params.Set_Face_Location( Vector3(-170.179f, 520.595f, 5.999f), 1 );
			Commands->Action_Face_Location ( obj, params );
		}

		else if (action_id == M01_FACING_SPECIFIED_DIRECTION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{ 
			params.Set_Basic(this, 45, M01_DOING_ANIMATION_01_JDG);
			params.Set_Animation ("H_A_J17C", false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{ 
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement( Vector3(0,0,0), .35f, .25f );
			params.WaypathID = 100674;
			params.WaypointStartID = 100676;
			params.WaypointEndID = 100682;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic(this, 45, M01_DOING_ANIMATION_02_JDG);
			params.Set_Animation ("H_A_V42A", false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Vector3 sitspot(-176.299f, 532.229f, -4.6f);
			Commands->Set_Position (  obj, sitspot );
			Commands->Set_Facing ( obj, 60 );
			params.Set_Basic(this, 45, 100);
			params.Set_Animation ("H_A_V11A", true);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_Hand_Of_Nod_Grunt_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_IN_HON_GRUNT_LEVEL_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Hand_Of_Nod_Dojo_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_IN_HON_DOJO_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Church_LoveShack_MiniGunner_JDG, "")//M01_CHURCH_LOVESHACK_MINIGUNNER_ID		101305
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );

				ActionParamsStruct params;
				params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( STAR, RUN, 5 );
				params.Set_Attack( STAR, 15, 0, true );
				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * conversationZone = Commands->Find_Object ( 103392 );
		if (conversationZone != NULL)
		{
			Commands->Destroy_Object ( conversationZone );
		}

		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_LOVESHACK_GUARD_IS_DEAD_JDG, 0 );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG), 0, M01_CARD_CARRIER_HAS_BEEN_KILLED_JDG, 0 );//tell controller you've been killed
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			Commands->Innate_Enable( obj );
			params.Set_Basic(this, 85, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, 2 );
			params.WaypathID = 100760;
			params.WaypointStartID = 100761;
			params.WaypointEndID = 100769;
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Church_Exterior_MiniGunner_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( Vector3(0,0,0), .2f, .25f );
				params.WaypathID = 100771;
				params.WaypointStartID = 100772;
				params.WaypointEndID = 100776;
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );

				ActionParamsStruct params;
				params.Set_Basic(this, 100, 0);
				params.Set_Movement( STAR, WALK, 5 );
				params.Set_Attack( STAR, 15, 0, true );
				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100778;
			params.WaypointStartID = 100779;
			params.WaypointEndID = 100782;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100778;
			params.WaypointStartID = 100782;
			params.WaypointEndID = 100779;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_03_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_04_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100771;
			params.WaypointStartID = 100776;
			params.WaypointEndID = 100772;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_04_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_04_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_04_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100771;
			params.WaypointStartID = 100772;
			params.WaypointEndID = 100776;
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Church_Balcony_MiniGunner_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( Vector3(0,0,0), WALK, .25f );
				params.WaypathID = 100784;
				params.WaypointStartID = 100785;
				params.WaypointEndID = 100786;
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100788;
			params.WaypointStartID = 100789;
			params.WaypointEndID = 100790;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100792;
			params.WaypointStartID = 100793;
			params.WaypointEndID = 100794;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_03_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_04_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100796;
			params.WaypointStartID = 100797;
			params.WaypointEndID = 100799;
			Commands->Action_Goto( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_04_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is at animation point--choose and do animation
		{ 
			const char* animationName = M01_Choose_Idle_Animation ( );

			params.Set_Basic( this, 45, M01_DOING_ANIMATION_04_JDG );
			params.Set_Animation (animationName, false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if (action_id == M01_DOING_ANIMATION_04_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//minigunner is done with animation--keep moving
		{
			params.Set_Basic(this, 45, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100784;
			params.WaypointStartID = 100785;
			params.WaypointEndID = 100786;
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Announce_Hand_of_Nod_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
			Commands->Attach_Script(controller, "Test_Cinematic", "X1A_Apache_Orca_Chase_01.txt");

			GameObject * buggyController = Commands->Find_Object ( 103367 );
			if (buggyController != NULL)
			{
				Commands->Send_Custom_Event( obj, buggyController, 0, M01_START_ACTING_JDG, 0 );
			}

			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_ANNOUNCE_HON_OBJECTIVE_JDG, 0 );
			
			Commands->Destroy_Object ( obj);//this is a 1-time only zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M01_Announce_Barn_Objective_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_START_BARN_OBJECTIVE_JDG, 0 );
			Commands->Destroy_Object ( obj);//this is a 1-time only zone--cleaning up
		}
	}
};



DECLARE_SCRIPT(M01_Tiberium_Cave_Helicopter_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;

		Commands->Innate_Disable(obj);
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		params.Set_Basic( this, 100, M01_START_ACTING_JDG );
		params.Set_Movement( Vector3(-42.883f, 376.457f, 18.025f), 0.005f, 0.25f );
		params.AttackActive		= false;
		params.AttackCheckBlocked = false;
		params.MovePathfind = false;

		Commands->Action_Attack (obj, params);

		float delayTimer = Commands->Get_Random ( 1, 2 );
		Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_MODIFY_YOUR_ACTION_JDG )
		{
			params.Set_Basic(this, 100, M01_START_ACTING_JDG);
			params.Set_Movement( Vector3(-42.883f, 376.457f, 18.025f), 0.05f, 0.25f );
			params.AttackActive	= true;
			params.AttackCheckBlocked = false;
			params.MovePathfind = false;

			params.Set_Attack (STAR, 100, 0, true);	
			
			Commands->Modify_Action (obj, M01_START_ACTING_JDG, params, true, true);

			float delayTimer = Commands->Get_Random ( 1, 2 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_02_JDG )
		{
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( Vector3(0,0,0), 1.0f, 0.75f );
			params.WaypathID = 100964;
			params.WaypointStartID = 100965;
			params.WaypointEndID = 101366;
			params.WaypathSplined = true;
			params.AttackActive	= true;
			params.AttackCheckBlocked = true;
			params.MovePathfind = false;

			params.Set_Attack (STAR, 100, 0, true);

			Commands->Action_Attack (obj, params);

			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 15 );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_03_JDG )
		{
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Player_Is_Crossing_Bridge_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_CROSSING_THE_BRIDGE_JDG, 0 );

			if ( Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
			}
		
			//this is a one-time only zone--cleaning up
			if (Commands->Find_Object ( 100922 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100922 ));
			}

			if (Commands->Find_Object ( 100934 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100934 ));
			}

			if (Commands->Find_Object ( 100972 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100972 ));
			}
		}
	}
};

DECLARE_SCRIPT(M01_Player_Is_Crossing_Bridge_Via_Cave_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_CROSSING_THE_BRIDGE_VIA_CAVE_JDG, 0 );
			
			if ( Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
			}
			//this is a one-time only zone--cleaning up
			if (Commands->Find_Object ( 100922 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100922 ));
			}

			if (Commands->Find_Object ( 100934 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100934 ));
			}

			if (Commands->Find_Object ( 100972 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100972 ));
			}
		}
	}
};

DECLARE_SCRIPT(M01_Player_Is_Crossing_Bridge_Via_Church_Zone, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_CROSSING_THE_BRIDGE_JDG, 0 );
			
			if ( Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
			}
			//this is a one-time only zone--cleaning up
			if (Commands->Find_Object ( 100922 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100922 ));
			}

			if (Commands->Find_Object ( 100934 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100934 ));
			}

			if (Commands->Find_Object ( 100972 ))//clean up all zones associated with the driveby scenario
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 100972 ));
			}
		}
	}
};

DECLARE_SCRIPT(M01_Flamethrower_Point_Guard_JDG, "")
{
	Vector3 myPosition;
	float myFacing;
	float leftFacing;
	float rightFacing;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(myPosition, 1);
		SAVE_VARIABLE(myFacing, 2);
		SAVE_VARIABLE(leftFacing, 3);
		SAVE_VARIABLE(rightFacing, 4);
	}

	void Created( GameObject * obj ) override
	{
		myPosition = Commands->Get_Position ( obj );
		myFacing = Commands->Get_Facing ( obj );

		leftFacing = myFacing - 45;
		rightFacing = myFacing + 45;

		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 20.0f );
		Commands->Set_Innate_Aggressiveness ( obj, 0.5f );
		Commands->Set_Innate_Take_Cover_Probability ( obj, 0.5f );

		float delayTimer = Commands->Get_Random ( 5, 10 );
		Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_PICK_A_NEW_LOCATION_JDG)
		{
			float atarashiLookSpot = Commands->Get_Random ( leftFacing, rightFacing );
			float delayTimer = Commands->Get_Random ( 5, 10 );

			params.Set_Basic( this, 45, 20 );
			params.Set_Face_Location(  myPosition, DEG_TO_RAD(atarashiLookSpot), delayTimer );
			Commands->Action_Face_Location ( obj, params );
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if ((sound.Type == SOUND_TYPE_BULLET_HIT) || (sound.Type == SOUND_TYPE_GUNSHOT))  
		{
			Commands->Set_Innate_Is_Stationary (  obj, false );
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * /* enemy */ ) override
	{
		Commands->Set_Innate_Is_Stationary (  obj, false );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			if (obj)
			{
				Commands->Set_Innate_Is_Stationary (  obj, false );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_MiniGunner_Point_Guard_JDG, "")
{
	/*Vector3 myPosition;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(myPosition, 1);
	}

	void Created( GameObject * obj ) override 
	{
		myPosition.Set (Commands->Get_Position ( obj ));
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 3 );
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		int current_difficulty = Commands->Get_Difficulty_Level();
		int easy = 0;
		int medium = 1;
		int hard = 2;

		if (current_difficulty == easy)
		{
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, M01_EASY_DIFFICULTY_HOMERANGE_JDG );
			Commands->Set_Innate_Aggressiveness ( obj, M01_EASY_DIFFICULTY_AGGRESSION_JDG );
			Commands->Set_Innate_Take_Cover_Probability ( obj, M01_EASY_DIFFICULTY_TAKECOVER_JDG );
		}

		else if (current_difficulty == medium)
		{
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, M01_MEDIUM_DIFFICULTY_HOMERANGE_JDG );
			Commands->Set_Innate_Aggressiveness ( obj, M01_MEDIUM_DIFFICULTY_AGGRESSION_JDG );
			Commands->Set_Innate_Take_Cover_Probability ( obj, M01_MEDIUM_DIFFICULTY_TAKECOVER_JDG );
		}

		else if (current_difficulty == hard)
		{
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, M01_HARD_DIFFICULTY_HOMERANGE_JDG );
			Commands->Set_Innate_Aggressiveness ( obj, M01_HARD_DIFFICULTY_AGGRESSION_JDG );
			Commands->Set_Innate_Take_Cover_Probability ( obj, M01_HARD_DIFFICULTY_TAKECOVER_JDG );
		}
	}*/
};

DECLARE_SCRIPT(M01_HON_Easy_Spawned_Guy_01_JDG, "")
{
	bool hunting;
	bool fighting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hunting, 1);
		SAVE_VARIABLE(fighting, 2);
	}

	void Created( GameObject * obj ) override 
	{
		hunting = false;
		fighting = false;
		Commands->Enable_Hibernation( obj, false );
		ActionParamsStruct params;
		params.Set_Basic( this, 85, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(-218.772f, 510.932f, 4.017f), RUN, 0.05f );

		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );

				if (fighting == false)
				{
					Commands->Action_Reset ( obj, 100 );
					Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
					fighting = true;
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (hunting != true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_01_IS_DEAD_JDG, 0 );
		}

		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_IS_DEAD_JDG, 0 );
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (enemy == STAR && fighting == false)
		{
			fighting = true;
			Commands->Action_Reset ( obj, 100 );
			Commands->Set_Innate_Aggressiveness ( obj, 0.25f );
			Commands->Set_Innate_Take_Cover_Probability ( obj, 0.75f );
			Commands->Enable_Hibernation( obj, true );
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR && fighting == false)
		{	
			Vector3 playersPosition = Commands->Get_Position ( STAR );

			if (sound.Type == SOUND_TYPE_GUNSHOT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Gunshots_Heard ( obj, playersPosition );
			}

			else if (sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Bullet_Heard ( obj, playersPosition );
			}
		}
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_HUNT_THE_PLAYER_JDG )
		{
			if (obj && STAR)
			{
				hunting = true;
				Vector3 playersPosition = Commands->Get_Position ( STAR );
				params.Set_Basic(this, 45, M01_HUNT_THE_PLAYER_JDG);
				params.Set_Movement( playersPosition, RUN, 5, false );

				Commands->Action_Goto( obj, params );
			}

			Commands->Send_Custom_Event( obj, obj, 0, M01_HUNT_THE_PLAYER_JDG, 30 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_HUNT_THE_PLAYER_JDG && (complete_reason == ACTION_COMPLETE_PATH_BAD_START || complete_reason == ACTION_COMPLETE_PATH_BAD_DEST || complete_reason == ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE))
		{
			Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Easy_Spawned_Guy_02_JDG, "")
{
	bool hunting;
	bool fighting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hunting, 1);
		SAVE_VARIABLE(fighting, 2);
	}

	void Created( GameObject * obj ) override 
	{
		hunting = false;
		fighting = false;
		Commands->Enable_Hibernation( obj, false );
		ActionParamsStruct params;
		params.Set_Basic( this, 85, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(-224.418f, 510.944f, 4.006f), RUN, 0.05f );

		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );

				if (fighting == false)
				{
					Commands->Action_Reset ( obj, 100 );
					Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
					fighting = true;
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (hunting != true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_02_IS_DEAD_JDG, 0 );
		}

		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_IS_DEAD_JDG, 0 );
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{ 
		if (enemy == STAR && fighting == false)
		{
			fighting = true;
			Commands->Action_Reset ( obj, 100 );
			Commands->Set_Innate_Aggressiveness ( obj, 0.25f );
			Commands->Set_Innate_Take_Cover_Probability ( obj, 0.75f );
			Commands->Enable_Hibernation( obj, true );
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR && fighting == false)
		{	
			Vector3 playersPosition = Commands->Get_Position ( STAR );

			if (sound.Type == SOUND_TYPE_GUNSHOT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Gunshots_Heard ( obj, playersPosition );
			}

			else if (sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Bullet_Heard ( obj, playersPosition );
			}
		}
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_HUNT_THE_PLAYER_JDG )
		{
			if (obj && STAR)
			{
				hunting = true;
				Vector3 playersPosition = Commands->Get_Position ( STAR );
				params.Set_Basic(this, 45, M01_HUNT_THE_PLAYER_JDG);
				params.Set_Movement( playersPosition, RUN, 5, false );
				Commands->Action_Goto( obj, params );
			}

			Commands->Send_Custom_Event( obj, obj, 0, M01_HUNT_THE_PLAYER_JDG, 30 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_HUNT_THE_PLAYER_JDG && (complete_reason == ACTION_COMPLETE_PATH_BAD_START || complete_reason == ACTION_COMPLETE_PATH_BAD_DEST || complete_reason == ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE))
		{
			Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Easy_Spawned_Guy_03_JDG, "")
{
	bool hunting;
	bool fighting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hunting, 1);
		SAVE_VARIABLE(fighting, 2);
	}

	void Created( GameObject * obj ) override 
	{
		hunting = false;
		fighting = false;
		Commands->Enable_Hibernation( obj, false );
		ActionParamsStruct params;
		params.Set_Basic( this, 85, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(-221.754f, 514.833f, 4.015f), RUN, 0.05f );

		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR )
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );

				if (fighting == false)
				{
					Commands->Action_Reset ( obj, 100 );
					Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
					fighting = true;
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (hunting != true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_03_IS_DEAD_JDG, 0 );
		}

		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_IS_DEAD_JDG, 0 );
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (enemy == STAR && fighting == false)
		{
			fighting = true;
			Commands->Action_Reset ( obj, 100 );
			Commands->Set_Innate_Aggressiveness ( obj, 0.25f );
			Commands->Set_Innate_Take_Cover_Probability ( obj, 0.75f );
			Commands->Enable_Hibernation( obj, true );
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR && fighting == false)
		{	
			Vector3 playersPosition = Commands->Get_Position ( STAR );

			if (sound.Type == SOUND_TYPE_GUNSHOT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Gunshots_Heard ( obj, playersPosition );
			}

			else if (sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				fighting = true;
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Force_State_Bullet_Heard ( obj, playersPosition );
			}
		}
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_HUNT_THE_PLAYER_JDG )
		{
			if (obj && STAR)
			{
				hunting = true;
				Vector3 playersPosition = Commands->Get_Position ( STAR );
				params.Set_Basic(this, 45, M01_HUNT_THE_PLAYER_JDG);
				params.Set_Movement( playersPosition, RUN, 5, false );

				Commands->Action_Goto( obj, params );
			}

			Commands->Send_Custom_Event( obj, obj, 0, M01_HUNT_THE_PLAYER_JDG, 30 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_HUNT_THE_PLAYER_JDG && (complete_reason == ACTION_COMPLETE_PATH_BAD_START || complete_reason == ACTION_COMPLETE_PATH_BAD_DEST || complete_reason == ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE))
		{
			Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
		}

		else if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNER_IN_POSITION_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Whack_A_Mole_Enter_Zone_JDG, "")//this guys ID is 101209
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SPAWN_WHACK_A_MOLE_GUY_JDG, 0 );
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101207 ), 0, M01_SPAWN_WHACK_A_MOLE_GUY_JDG, 0 );
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Whack_A_Mole_Exit_Zone_JDG, "")//this guys ID is 101207
{
	bool okayToAct;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okayToAct, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okayToAct = false;
	}

	void Custom(GameObject * /* obj */, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		if (param == M01_SPAWN_WHACK_A_MOLE_GUY_JDG )
		{
			okayToAct = true;
		}
	}
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && okayToAct == true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CHANGE_WHACK_A_MOLE_GUY_JDG, 0 );
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Whack_A_Mole_Minigunner_JDG, "")
{
	typedef enum {
		Left_To_Right,
		Right_To_Left,
	} Walking_State;

	bool invincible;
	bool attacked;
	float base_health;
	float speed;
	int points_given;

	Walking_State Whack_a_Mole_State;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(invincible, 1);
		SAVE_VARIABLE(attacked, 2);
		SAVE_VARIABLE(base_health, 3);
		SAVE_VARIABLE(speed, 4);
		SAVE_VARIABLE(points_given, 5);
		SAVE_VARIABLE(Whack_a_Mole_State, 6);
	}

	void Created( GameObject * obj ) override 
	{
		//Commands->Innate_Disable(obj);
		invincible = false;
		attacked = false;
		base_health = Commands->Get_Max_Health ( obj );
		speed = 0.3f;
		points_given = 1;

		ActionParamsStruct params;

		params.Set_Basic( this, 45, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(0,0,0), speed, 0.25f );
		params.WaypathID = 101221;

		Commands->Action_Goto( obj, params );

		Whack_a_Mole_State = Right_To_Left;
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_CHANGE_WHACK_A_MOLE_GUY_JDG )
		{
			if (obj)
			{
				invincible = false;
				Commands->Innate_Enable(obj);
			}
		}

		else if (param == M01_DOING_ANIMATION_01_JDG )
		{
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation( "H_A_J21C", false );

			Commands->Action_Play_Animation ( obj, params );
		}

		else if (param == M01_PICK_A_NEW_LOCATION_JDG )
		{
			Commands->Action_Reset ( obj, 100 );

			if (Whack_a_Mole_State == Right_To_Left)
			{
				params.Set_Basic( this, 45, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( Vector3(0,0,0), speed, 0.25f );
				params.WaypathID = 101210;

				Commands->Action_Goto( obj, params );
				Whack_a_Mole_State = Left_To_Right;
			}

			else if (Whack_a_Mole_State == Left_To_Right)
			{
				params.Set_Basic( this, 45, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(0,0,0), speed, 0.25f );
				params.WaypathID = 101221;

				Commands->Action_Goto( obj, params );
				Whack_a_Mole_State = Right_To_Left;
			}
		}

		else if (param == M01_HUNT_THE_PLAYER_JDG )
		{
			if (obj)
			{
				invincible = false;
				Commands->Innate_Enable(obj);
				Commands->Grant_Key(  obj, 1, true );
				params.Set_Basic(this, 45, M01_HUNT_THE_PLAYER_JDG);
				params.Set_Movement( STAR, WALK, 5 );
				params.Set_Attack( STAR, 15, 0, true );
				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 0 );
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (attacked == false)
			{
				params.Set_Basic( this, 45, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(0,0,0), speed, 0.25f );
				params.WaypathID = 101221;

				Commands->Action_Goto( obj, params );
				Whack_a_Mole_State = Right_To_Left;
			}

			else if (attacked == true)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_HUNT_THE_PLAYER_JDG, 0 );
			}
		}

		else if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, 45, M01_WALKING_WAYPATH_02_JDG );
			params.Set_Movement( Vector3(0,0,0), speed, 0.25f );
			params.WaypathID = 101210;

			Commands->Action_Goto( obj, params );
			Whack_a_Mole_State = Left_To_Right;
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR && invincible == true)
		{
			if (obj)
			{
				attacked = true;
				Commands->Set_Health ( obj, base_health );
				points_given = 2 * points_given;
				speed = speed + .1;

				if (speed > 1)
				{
					speed = 1;
				}

				Commands->Give_Points( damager, points_given, false );

				Commands->Send_Custom_Event( obj, obj, 0, M01_DOING_ANIMATION_01_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Left_Interrogation_Room_Enter_Zone_JDG, "")//this guys ID is 101354
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (Commands->Find_Object ( M01_HON_INTERROGATION_ROOM_GUY_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_INTERROGATION_ROOM_GUY_ID ), 0, M01_PLAYER_ENTERING_LEFT_INTEROG_ROOM_JDG, 0 );
			}

			if (Commands->Find_Object ( 101354 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101354 ) );//cleaning up the 2 interogation room zones
			}

			if (Commands->Find_Object ( 101355 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101355 ) );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Right_Interrogation_Room_Enter_Zone_JDG, "")//this guys ID is 101355
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (Commands->Find_Object ( M01_HON_INTERROGATION_ROOM_GUY_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_INTERROGATION_ROOM_GUY_ID ), 0, M01_PLAYER_ENTERING_RIGHT_INTEROG_ROOM_JDG, 0 );
			}

			if (Commands->Find_Object ( 101354 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101354 ) );//cleaning up the 2 interogation room zones
			}

			if (Commands->Find_Object ( 101355 ))
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101355 ) );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Interrogation_Room_L03_Keycard_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Obj_Radar_Blip_Shape ( obj, RADAR_BLIP_SHAPE_NONE );
	}
};

DECLARE_SCRIPT(M01_Interrogation_Room_Surprise_Guy_JDG, "")//this guys ID is 101357
{
	Vector3 myPostion;
	bool onGuard;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(myPostion, 1);
		SAVE_VARIABLE(onGuard, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		onGuard = true;
	}

	void Enemy_Seen( GameObject * obj, GameObject * /* enemy */ ) override
	{
		ActionParamsStruct params;

		if (onGuard == true)
		{
			if (obj)
			{
				Commands->Innate_Enable( obj );
				Commands->Create_Sound ( "Huh_2", Commands->Get_Position ( obj ), obj );

				float delayTimer = Commands->Get_Random ( 0.5f, 2 );
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );

				onGuard = false;

				if (Commands->Find_Object ( 101354 ))
				{
					Commands->Destroy_Object ( Commands->Find_Object ( 101354 ) );//cleaning up the 2 interogation room zones
				}

				if (Commands->Find_Object ( 101355 ))
				{
					Commands->Destroy_Object ( Commands->Find_Object ( 101355 ) );
				}
			}
		}

		else
		{
		}
	}

	void Custom(GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if (param == M01_PLAYER_ENTERING_LEFT_INTEROG_ROOM_JDG )
		{
			myPostion.Set (-182.199f, 529.167f, -31.130f);

			params.Set_Basic( this, 80, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( myPostion, 0.75f, 0.05f );
			Commands->Action_Goto( obj, params );

			onGuard = false;
		}

		else if (param == M01_PLAYER_ENTERING_RIGHT_INTEROG_ROOM_JDG )
		{
			myPostion.Set (-182.753f, 539.906f, -31.130f);

			params.Set_Basic( this, 80, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( myPostion, 0.75f, 0.05f );
			Commands->Action_Goto( obj, params );

			onGuard = false;
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG )
		{
			if (obj)
			{
				Commands->Grant_Key(  obj, 1, true );
				Commands->Grant_Key(  obj, 2, true );
				Commands->Grant_Key(  obj, 3, true );

				Commands->Innate_Enable( obj );

				params.Set_Basic( this, 80, M01_HUNT_THE_PLAYER_JDG );
				params.Set_Attack( STAR, 20, 0, true );
				params.AttackActive	= true;	
				params.AttackCheckBlocked = true;

				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Create_Sound ( "TargetHasBeenEngaged_2", myPostion, obj );
			params.Set_Basic( this, 80, M01_HUNT_THE_PLAYER_JDG );
			params.Set_Attack( STAR, 20, 0, true );
			params.AttackActive	= true;	
			params.AttackCheckBlocked = false;

			Commands->Action_Attack ( obj, params );

			float delayTimer = Commands->Get_Random ( 1, 3 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
		}
	}
};



DECLARE_SCRIPT(M01_Medium_Tank_Tunnel_Squish_Guy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPositon = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPositon, 10.0f );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_MEDIUM_TANK_IS_HERE_JDG  )
		{
			Commands->Set_Innate_Is_Stationary ( obj, false );
			ActionParamsStruct params;

			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( Vector3(0,0,0), 1.0f, .25f );
			params.WaypathID = 101554;
			params.WaypointStartID = 101561;
			Commands->Action_Goto( obj, params );

			Commands->Create_3D_Sound_At_Bone ( "M01_Nod_HupHup", obj, "ROOTTRANSFORM" );
		}
	}
};

DECLARE_SCRIPT(M01_Billys_Conversation_Zone_JDG, "")//this guys ID is 101661
{
	bool okay_to_play_conversation;
	bool playerEntered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_play_conversation, 1);
		SAVE_VARIABLE(playerEntered, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okay_to_play_conversation = false;
		playerEntered = false;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//billy is in position--give okay for conversation
		{
			okay_to_play_conversation = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_CUE_BILLYS_CONVERSATION_JDG, 0 );
		}

		else if ((param == M01_CUE_BILLYS_CONVERSATION_JDG))//see if billy and havoc are present
		{
			if (playerEntered == true && okay_to_play_conversation == true)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_BILLYS_CONVERSATION_JDG, 0 );
				GameObject * babushkaZone = Commands->Find_Object ( 101662 );
				GameObject * billyZone = Commands->Find_Object ( 101661 );

				if (babushkaZone != NULL)
				{
					Commands->Destroy_Object ( babushkaZone );//this is one-time only zone--cleaning up
				}

				if (billyZone != NULL)
				{
					Commands->Destroy_Object ( billyZone );//this is one-time only zone--cleaning up
				}
			}
		}
	}

	void Exited( GameObject * /* obj */, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			playerEntered = false;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			playerEntered = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_CUE_BILLYS_CONVERSATION_JDG, 0 );
		}
	}
};



DECLARE_SCRIPT(M01_Tank_Entering_Tunnel_Zone_JDG, "")//this guys ID is 
{
	int gdi_medium_tank;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gdi_medium_tank, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		gdi_medium_tank = 0;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == M01_MEDIUM_TANK_IS_HERE_JDG  )
		{
			gdi_medium_tank = param;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		/*GameObject * lightTank = Commands->Find_Object ( M01_TAILGUNAREA_NOD_LIGHTTANK_JDG );
		if (lightTank != NULL && enterer == lightTank)
		{
			Commands->Send_Custom_Event( obj, lightTank, 0, M01_GOTO_IDLE_JDG, 0 );
			Vector3 newTankPosition (14.208f, 225.943f, 4.0f);
			Commands->Set_Position ( lightTank, newTankPosition );	
		}*/

		if (enterer == Commands->Find_Object ( gdi_medium_tank ))
		{
			if ( Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_01_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_01_ID ), 0, M01_MEDIUM_TANK_IS_HERE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_02_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_02_ID ), 0, M01_MEDIUM_TANK_IS_HERE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_03_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TANK_TUNNEL_SQUISH_GUY_03_ID ), 0, M01_MEDIUM_TANK_IS_HERE_JDG, 0 );
			}

			Commands->Destroy_Object ( obj );//one time zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M01_Player_Is_Entering_Tailgun_Alley_JDG, "")//this guys ID is 101692
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			int tankWarning = Commands->Create_Conversation( "M01_Announce_LightTank_Conversation", 100, 1000, false);
			Commands->Join_Conversation( NULL, tankWarning, false, false, true );
			Commands->Join_Conversation( NULL, tankWarning, false, false, true );
			//Commands->Join_Conversation( STAR, tankWarning, false, false, true );
			Commands->Start_Conversation( tankWarning,  tankWarning );

			if ( Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_APPROACHING_TAILGUN_ALLEY_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_01_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_01_ID ), 0, M01_START_ACTING_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_02_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_02_ID ), 0, M01_START_ACTING_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_03_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_03_ID ), 0, M01_START_ACTING_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
			}
		
			if ( Commands->Find_Object ( 101718 ))//this is the backway enter zone
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101718 ) );
			}

			Commands->Destroy_Object ( obj );//one time zone--cleaning up
		}
	}
};



DECLARE_SCRIPT(M01_Player_Is_Entering_Tailgun_Alley_Backway_JDG, "")//this guys ID is 101691
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			if ( Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_APPROACHING_TAILGUN_ALLEY_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_01_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_01_ID ), 0, M01_START_ACTING_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_02_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_02_ID ), 0, M01_START_ACTING_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUN_03_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_03_ID ), 0, M01_START_ACTING_JDG, 0 );
			}	

			if ( Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( 101692 ))//this is the enter zone in the tunnel
			{
				Commands->Destroy_Object ( Commands->Find_Object ( 101692 ) );
			}

			Commands->Destroy_Object ( obj );//one time zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M01_TailGunner_01_JDG, "")//M01_TAILGUNNER_01_ID  101470
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);

		if ( Commands->Find_Object ( M01_TAILGUN_01_ID ))
		{
			ActionParamsStruct params;
			Vector3 tailgunPosition = Commands->Get_Position ( Commands->Find_Object ( M01_TAILGUN_01_ID ) );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( tailgunPosition, .25f, .25f );
			
			Commands->Action_Goto( obj, params );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if ( Commands->Find_Object ( M01_TAILGUN_01_ID ))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_01_ID ), 0, M01_YOUR_OPERATOR_IS_DEAD_JDG, 0 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
			Commands->Action_Enter_Exit (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_TailGunner_02_JDG, "")//M01_TAILGUNNER_02_ID  101471
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);

		if ( Commands->Find_Object ( M01_TAILGUN_02_ID ))
		{
			ActionParamsStruct params;
			Vector3 tailgunPosition = Commands->Get_Position ( Commands->Find_Object ( M01_TAILGUN_02_ID ) );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( tailgunPosition, .25f, .25f );
			
			Commands->Action_Goto( obj, params );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if ( Commands->Find_Object ( M01_TAILGUN_02_ID ))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_02_ID ), 0, M01_YOUR_OPERATOR_IS_DEAD_JDG, 0 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
			Commands->Action_Enter_Exit (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_TailGunner_03_JDG, "")//M01_TAILGUNNER_03_ID  101472
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);

		if ( Commands->Find_Object ( M01_TAILGUN_03_ID ))
		{
			ActionParamsStruct params;
			Vector3 tailgunPosition = Commands->Get_Position ( Commands->Find_Object ( M01_TAILGUN_03_ID ) );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( tailgunPosition, .25f, .35f );
			
			Commands->Action_Goto( obj, params );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if ( Commands->Find_Object ( M01_TAILGUN_03_ID ))
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUN_03_ID ), 0, M01_YOUR_OPERATOR_IS_DEAD_JDG, 0 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
			Commands->Action_Enter_Exit (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_TailGun_01_JDG, "")//M01_TAILGUN_01_ID 100243
{
	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_START_ACTING_JDG )
		{
			params.Set_Basic( this, 100, M01_HUNT_THE_PLAYER_JDG );
			params.Set_Attack( STAR, 50, 0, true );
			params.AttackCheckBlocked = false;

			Commands->Action_Attack (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_TailGun_02_JDG, "")//M01_TAILGUN_02_ID 100244
{
	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_START_ACTING_JDG )
		{
			params.Set_Basic( this, 100, M01_HUNT_THE_PLAYER_JDG );
			params.Set_Attack( STAR, 50, 0, true );
			params.AttackCheckBlocked = false;

			Commands->Action_Attack (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_TailGun_03_JDG, "")//M01_TAILGUN_03_ID 100245
{
	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_START_ACTING_JDG )
		{
			params.Set_Basic( this, 100, M01_HUNT_THE_PLAYER_JDG );
			params.Set_Attack( STAR, 50, 0, true );
			params.AttackCheckBlocked = false;

			Commands->Action_Attack (  obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Tailgun_02_SpawnApache_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
			Commands->Attach_Script(controller, "Test_Cinematic", "X1A_Tailgun_Apache_02.txt");

			Commands->Destroy_Object ( obj );//this is a one-time zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M01_Civ_To_Minigunner_Guy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;

		if (Commands->Find_Object (M01_COMMCENTER_BASE_SCAPEGOAT_JDG))
		{
			Vector3 myGotoSpot = Commands->Get_Position ( Commands->Find_Object (M01_COMMCENTER_BASE_SCAPEGOAT_JDG) );

			params.Set_Basic( this, 50, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( myGotoSpot, RUN, 20 );
			
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Base_Commander_JDG, "")//this guys ID is M01_COMMCENTER_BASE_COMMANDER_JDG 101936
{
	bool enemyEngaged;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(enemyEngaged, 1);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		enemyEngaged = false;
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR && enemyEngaged == false)
		{
			enemyEngaged = true;
			Commands->Innate_Enable(obj);
			Commands->Action_Reset ( obj, 100 );
			Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
			Commands->Enable_Hibernation( obj, true );
			//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_KANE_AND_HAVOC_CONVERSATION_JDG, 0 );
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (enemy == STAR && enemyEngaged == false)
		{
			enemyEngaged = true;
			Commands->Innate_Enable(obj);
			Commands->Action_Reset ( obj, 100 );
			Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
			Commands->Enable_Hibernation( obj, true );
			//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_KANE_AND_HAVOC_CONVERSATION_JDG, 0 );
		}
	}

	void Killed( GameObject * obj, GameObject * killer ) override
	{
		if (killer == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_KANE_AND_HAVOC_CONVERSATION_JDG, 0 );

			Vector3 spawn_location = Commands->Get_Position ( obj );
			spawn_location.Z = spawn_location.Z + 0.75f;

			bool player_has_level02_card = Commands->Has_Key( STAR, 2 );

			if (player_has_level02_card == false)
			{
				Commands->Create_Object ( "Level_02_Keycard", spawn_location );
			}
		}
	}
		
};

/*DECLARE_SCRIPT(M01_Comm_Base_Scapegoat_JDG, "")//this guys ID is M01_COMMCENTER_BASE_SCAPEGOAT_JDG 101938
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
	}
};

DECLARE_SCRIPT(M01_Comm_Ceiling_Camera_JDG, "")//this guys ID is M01_COMMCENTER_CEILING_CAMERA_JDG 101937
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;

		params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
		params.Set_Attack( Commands->Find_Object (M01_COMMCENTER_BASE_SCAPEGOAT_JDG), 0, 0, true );
		Commands->Action_Attack( obj, params );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_START_ATTACKING_01_JDG: //kane has told you to kill the scapegoat
				{
					if (Commands->Find_Object (M01_COMMCENTER_BASE_SCAPEGOAT_JDG))
					{
						params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
						params.Set_Attack( Commands->Find_Object (M01_COMMCENTER_BASE_SCAPEGOAT_JDG), 100, 0, true );
						Commands->Action_Attack( obj, params );
					}
				}
				break;
		}
	}
};*/

DECLARE_SCRIPT(M01_Comm_Stationary_Tech_JDG, "") 
{
	//these guys IDs are  M01_COMMCENTER_UPSTAIRS_TECH_JDG		101168
						//M01_COMMCENTER_WAROOM_TECH_01_JDG		101170
						//M01_COMMCENTER_WAROOM_TECH_02_JDG		101171

	Vector3 myLocation;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(myLocation, 1);
	}

	void Created( GameObject * obj ) override 
	{
		myLocation = Commands->Get_Position ( obj );
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG: 
				{
					Commands->Innate_Enable(obj);
					Commands->Set_Innate_Aggressiveness ( obj, 0.15f );
					Commands->Set_Innate_Take_Cover_Probability ( obj, 1.0f );

					params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement( myLocation, WALK, 0 );
					Commands->Action_Goto( obj, params );
				}
				break;
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic(this, 30, M01_DOING_ANIMATION_01_JDG);
			params.Set_Animation( "H_A_CON2", false );

			Commands->Action_Play_Animation (  obj, params );
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( myLocation, WALK, 0 );
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_ComputerRoom_Tech_JDG, "")//this guys ID is M01_COMMCENTER_COMPUTERROOM_TECH_JDG 101172
{
	Vector3 myLocation;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(myLocation, 1);
	}

	void Created( GameObject * obj ) override 
	{
		myLocation = Commands->Get_Position ( obj );
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG: 
				{
					Commands->Innate_Enable(obj);
					Commands->Set_Innate_Aggressiveness ( obj, 0.15f );
					Commands->Set_Innate_Take_Cover_Probability ( obj, 1.0f );

					params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement( myLocation, WALK, 0 );
					Commands->Action_Goto( obj, params );
				}
				break;
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic(this, 30, M01_DOING_ANIMATION_01_JDG);
			params.Set_Animation( "H_A_CON2", false );

			Commands->Action_Play_Animation (  obj, params );
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( myLocation, WALK, 0 );
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Upstairs_Guard_JDG, "")//this guys ID is M01_COMMCENTER_UPSTAIRS_GUARD_JDG 101946
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_PLAYER_IS_INSIDE_COMM_CENTER_JDG: 
				{
					Commands->Innate_Enable(obj);
					Commands->Set_Innate_Aggressiveness ( obj, 0.75f );
					Commands->Set_Innate_Take_Cover_Probability ( obj, 0.5f );

					params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement( Vector3 (0,0,0), WALK, 0 );
					params.WaypathID = 101940;
					params.WaypointStartID = 101944;
					params.WaypointEndID = 101941;
					Commands->Action_Goto( obj, params );
				}
				break;
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			float delayTimer = Commands->Get_Random ( 1, 10 );
			Commands->Start_Timer(obj, this, delayTimer, M01_WALKING_WAYPATH_01_JDG);
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			float delayTimer = Commands->Get_Random ( 1, 10 );
			Commands->Start_Timer(obj, this, delayTimer, M01_WALKING_WAYPATH_02_JDG);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		switch (timer_id)
		{
			case M01_WALKING_WAYPATH_01_JDG: 
				{
					params.Set_Basic(this, 30, M01_WALKING_WAYPATH_02_JDG);
					params.Set_Movement( Vector3 (0,0,0), WALK, 0 );
					params.WaypathID = 101940;
					params.WaypointStartID = 101944;
					params.WaypointEndID = 101941;
					Commands->Action_Goto( obj, params );
				}
				break;

			case M01_WALKING_WAYPATH_02_JDG: 
				{
					params.Set_Basic(this, 30, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement( Vector3 (0,0,0), WALK, 0 );
					params.WaypathID = 101940;
					params.WaypointStartID = 101941;
					params.WaypointEndID = 101944;
					Commands->Action_Goto( obj, params );
				}
				break;
		}
	}
			
};

DECLARE_SCRIPT(M01_Comm_Base_Commander_Conv_Start_Zone_JDG, "")//this guys ID is 101947
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_KANE_AND_NUMBER2_CONVERSATION_JDG, 0 );
			GameObject * zone01 = Commands->Find_Object ( 101947 );
			if (zone01 != NULL)
			{
				Commands->Destroy_Object ( zone01 );//this is a one-time only zone--cleaning up
			}

			GameObject * zone02 = Commands->Find_Object ( 111809 );
			if (zone02 != NULL)
			{
				Commands->Destroy_Object ( zone02 );//this is a one-time only zone--cleaning up
			}
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Kane_n_Havoc_Conv_Start_Zone_JDG, "")//this guys ID is 101947
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_KANE_AND_HAVOC_CONVERSATION_JDG, 0 );
			Commands->Destroy_Object ( obj );//this is one-time only zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M01_HON_MCT_Placeholder_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_KILL_THE_HON_JDG, 0 );
	}
};

DECLARE_SCRIPT(M01_Comm_MCT_Placeholder_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_KILL_THE_COMM_JDG, 0 );
	}
};

DECLARE_SCRIPT(M01_GDI_Toolshed_PatrolGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;

		params.Set_Basic( this, 30, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(0,0,0), WALK, .05f );
		params.WaypathID = 102153;
		params.WaypointStartID = 102154;
		params.WaypointEndID = 102157;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason complete_reason) override
	{
		ActionParamsStruct params;

		switch (complete_reason)
		{
			case ACTION_COMPLETE_NORMAL: 
				{
					switch (action_id)
					{
						case M01_WALKING_WAYPATH_01_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_01_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_01_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_02_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102159;
								params.WaypointStartID = 102160;
								params.WaypointEndID = 102168;
								Commands->Action_Goto( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_02_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_02_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_02_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_03_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102176;
								params.WaypointStartID = 102177;
								params.WaypointEndID = 102183;
								Commands->Action_Goto( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_03_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_03_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_03_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_04_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102176;
								params.WaypointStartID = 102183;
								params.WaypointEndID = 102177;
								Commands->Action_Goto( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_04_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_04_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_04_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_05_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102159;
								params.WaypointStartID = 102168;
								params.WaypointEndID = 102160;
								Commands->Action_Goto( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_05_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_05_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_05_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_06_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102153;
								params.WaypointStartID = 102157;
								params.WaypointEndID = 102154;
								Commands->Action_Goto( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_06_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 30, M01_DOING_ANIMATION_06_JDG );
								params.Set_Animation (animationName, false);
								Commands->Action_Play_Animation (obj, params);
							}
							break;

						case M01_DOING_ANIMATION_06_JDG: 
							{
								params.Set_Basic( this, 30, M01_WALKING_WAYPATH_01_JDG );
								params.Set_Movement( Vector3(0,0,0), WALK, .05f );
								params.WaypathID = 102153;
								params.WaypointStartID = 102154;
								params.WaypointEndID = 102157;
								Commands->Action_Goto( obj, params );
							}
							break;
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_GDI_Base_Spawner_Controller_JDG, "")//this guys ID is M01_GDIBASE_SPAWNER_CONTROLLER_JDG
{
	int active_unit_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active_unit_count, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		active_unit_count = 0;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_SPAWNER_IS_DEAD_JDG: 
				{
					active_unit_count--;
					Commands->Send_Custom_Event( obj, obj, 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
					Commands->Debug_Message ( "***************************GDI Base spawner has recieved custom that someone is dead\n" );
				}
				break;

			case M01_SPAWNER_SPAWN_PLEASE_JDG: 
				{
					Commands->Enable_Hibernation( obj, false );
					int currentDifficulty = Commands->Get_Difficulty_Level( );
					int easy = 0;
					int medium = 1;
					int hard = 2;

					if (currentDifficulty == easy)
					{
						if (active_unit_count == 0)
						{
							Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_A_JDG );
							active_unit_count++;
							Commands->Debug_Message ( "***************************GDI Base spawner has spawned an easy guy\n" );
						}
					}

					else if (currentDifficulty == medium)
					{
						if (active_unit_count < 2)
						{
							int random = Commands->Get_Random(0.5f, 2.5f);

							if ((random >= 0.5f) && (random < 1.5f))
							{
								Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_A_JDG );
								active_unit_count++;
							}

							else
							{
								Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_B_JDG );
								active_unit_count++;
							}

							Commands->Debug_Message ( "***************************GDI Base spawner has spawned a medium guy\n" );
						}
					}

					else if (currentDifficulty == hard)
					{
						if (active_unit_count < 3)
						{
							int random = Commands->Get_Random(0.5f, 3.5f);

							if ((random >= 0.5f) && (random < 1.5f))
							{
								Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_A_JDG );
								active_unit_count++;
							}

							else if ((random >= 1.5f) && (random < 2.5f))
							{
								Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_B_JDG );
								active_unit_count++;
							}

							else 
							{
								Commands->Trigger_Spawner( M01_GDIBASE_SPAWNER_C_JDG );
								active_unit_count++;
							}

							Commands->Debug_Message ( "***************************GDI Base spawner has spawned a hard guy\n" );
						}
					}

					Commands->Send_Custom_Event( obj, obj, 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 10 );
				}
				break;	

			case M01_QUIT_SPAWNING_PLEASE_JDG: 
				{
					if (Commands->Find_Object (M01_GDIBASE_SPAWNER_A_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_GDIBASE_SPAWNER_A_JDG) );
					}

					if (Commands->Find_Object (M01_GDIBASE_SPAWNER_B_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_GDIBASE_SPAWNER_B_JDG) );
					}

					if (Commands->Find_Object (M01_GDIBASE_SPAWNER_C_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_GDIBASE_SPAWNER_C_JDG) );
					}

					Commands->Destroy_Object ( obj );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_GDI_Base_Spawner_Guy_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ))
		{
			Commands->Debug_Message ( "***************************GDI Base/barn area spawner has been killed--sending custom\n" );
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_GDIBASE_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_IS_DEAD_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Tailgun_Run_Spawner_Controller_JDG, "")//this guys ID is M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG
{
	int active_unit_count;
	bool startup;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active_unit_count, 1);
		SAVE_VARIABLE(startup, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		active_unit_count = 0;
		startup = true;	
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_SPAWNER_IS_DEAD_JDG: 
				{
					active_unit_count--;
					Commands->Send_Custom_Event( obj, obj, 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
				}
				break;

			case M01_SPAWNER_SPAWN_PLEASE_JDG: 
				{
					int currentDifficulty = Commands->Get_Difficulty_Level( );
					int easy = 0;
					int medium = 1;
					int hard = 2;

					if (startup == true)
					{
						startup = false;
						if (currentDifficulty == easy)
						{
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
							active_unit_count++;
						}

						else if (currentDifficulty == medium)
						{
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_B_JDG );
							active_unit_count += 2;
						}

						else if (currentDifficulty == hard)
						{
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_B_JDG );
							Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_C_JDG );
							active_unit_count += 3;
						}
					}

					else 
					{
						if (currentDifficulty == easy)
						{
							if (active_unit_count == 0)
							{
								Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
								active_unit_count++;
							}
						}

						else if (currentDifficulty == medium)
						{
							if (active_unit_count < 2)
							{
								int random = Commands->Get_Random(0.5f, 2.5f);

								if ((random >= 0.5f) && (random < 1.5f))
								{
									Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
									active_unit_count++;
								}

								else
								{
									Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_B_JDG );
									active_unit_count++;
								}
							}
						}

						else if (currentDifficulty == hard)
						{
							if (active_unit_count < 3)
							{
								int random = Commands->Get_Random(0.5f, 3.5f);

								if ((random >= 0.5f) && (random < 1.5f))
								{
									Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_A_JDG );
									active_unit_count++;
								}

								else if ((random >= 1.5f) && (random < 2.5f))
								{
									Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_B_JDG );
									active_unit_count++;
								}

								else 
								{
									Commands->Trigger_Spawner( M01_TAILGUNRUN_SPAWNER_C_JDG );
									active_unit_count++;
								}
							}
						}
					}
				}
				break;	

			case M01_QUIT_SPAWNING_PLEASE_JDG: 
				{
					if (Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_A_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_A_JDG) );
					}

					if (Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_B_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_B_JDG) );
					}

					if (Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_C_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_TAILGUNRUN_SPAWNER_C_JDG) );
					}

					Commands->Destroy_Object ( obj );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_TailgunRun_Spawner_Guy_JDG, "")//Innate_Disable)(GameObject* object);(*Innate_Enable)(GameObject* object);
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ))
		{
			Commands->Debug_Message ( "***************************taigun run area spawner has been killed--sending custom\n" );
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_TAILGUNRUN_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_IS_DEAD_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_Spawner_Controller_JDG, "")//this guys ID is M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG
{
	int active_unit_count;
	bool startup;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active_unit_count, 1);
		SAVE_VARIABLE(startup, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		active_unit_count = 0;
		startup = true;	
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_SPAWNER_IS_DEAD_JDG: 
				{
					active_unit_count--;
					Commands->Send_Custom_Event( obj, obj, 0, M01_SPAWNER_SPAWN_PLEASE_JDG, 0 );
				}
				break;

			case M01_SPAWNER_SPAWN_PLEASE_JDG: 
				{
					int currentDifficulty = Commands->Get_Difficulty_Level( );
					int easy = 0;
					int medium = 1;
					int hard = 2;

					if (startup == true)
					{
						startup = false;
						if (currentDifficulty == easy)
						{
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
							active_unit_count++;
						}

						else if (currentDifficulty == medium)
						{
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_B_JDG );
							active_unit_count += 2;
						}

						else if (currentDifficulty == hard)
						{
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_B_JDG );
							Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_C_JDG );
							active_unit_count += 3;
						}
					}

					else 
					{
						if (currentDifficulty == easy)
						{
							if (active_unit_count == 0)
							{
								Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
								active_unit_count++;
							}
						}

						else if (currentDifficulty == medium)
						{
							if (active_unit_count < 2)
							{
								int random = Commands->Get_Random(0.5f, 2.5f);

								if ((random >= 0.5f) && (random < 1.5f))
								{
									Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
									active_unit_count++;
								}

								else
								{
									Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_B_JDG );
									active_unit_count++;
								}
							}
						}

						else if (currentDifficulty == hard)
						{
							if (active_unit_count < 3)
							{
								int random = Commands->Get_Random(0.5f, 3.5f);

								if ((random >= 0.5f) && (random < 1.5f))
								{
									Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_A_JDG );
									active_unit_count++;
								}

								else if ((random >= 1.5f) && (random < 2.5f))
								{
									Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_B_JDG );
									active_unit_count++;
								}

								else 
								{
									Commands->Trigger_Spawner( M01_CHURCHAREA_SPAWNER_C_JDG );
									active_unit_count++;
								}
							}
						}
					}
				}
				break;	

			case M01_QUIT_SPAWNING_PLEASE_JDG: 
				{
					if (Commands->Find_Object (M01_CHURCHAREA_SPAWNER_A_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_CHURCHAREA_SPAWNER_A_JDG) );
					}

					if (Commands->Find_Object (M01_CHURCHAREA_SPAWNER_B_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_CHURCHAREA_SPAWNER_B_JDG) );
					}

					if (Commands->Find_Object (M01_CHURCHAREA_SPAWNER_C_JDG))
					{
						Commands->Destroy_Object ( Commands->Find_Object (M01_CHURCHAREA_SPAWNER_C_JDG) );
					}

					Commands->Destroy_Object ( obj );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_Spawner_Guy_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ))
		{
			Commands->Debug_Message ( "***************************church area spawner has been killed--sending custom\n" );
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ), 0, M01_SPAWNER_IS_DEAD_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Turn_on_the_Hand_of_Nod_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if ( Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_CHURCHAREA_SPAWNER_CONTROLLER_JDG ), 0, M01_QUIT_SPAWNING_PLEASE_JDG, 0 );
			}

			if ( Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ))
			{	
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_HON_SPAWNS_MINIGUNNER_JDG, 0 );
			}
			
			if (obj)
			{
				Commands->Destroy_Object ( obj);//this is a one-time only zone--cleaning up
			}
		}
	}
};

/*DECLARE_SCRIPT(M01_BarnArea_NOD_Commander_JDG, "")//M01_BARNAREA_NOD_COMMANDER_02_JDG 102476
{
	bool deadYet;
	bool firstTimeDamaged;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(deadYet, 1);
		SAVE_VARIABLE(firstTimeDamaged, 2);
	}

	void Created( GameObject * obj ) override 
	{
		deadYet = false;
		firstTimeDamaged = true;

		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 10 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float amount ) override
	{
		if (obj)
		{
			if (damager == STAR && deadYet == false && firstTimeDamaged == true)
			{	
				Vector3 myPosition = Commands->Get_Position ( obj );
				Vector3 playerPosition = Commands->Get_Position ( STAR );
				float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

				if (playerDistance >= 15)
				{
					firstTimeDamaged = false;
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
					params.Set_Animation( "H_A_J21C", false );
					Commands->Action_Play_Animation (  obj, params );
				}	
			}
		}
	}

	void Killed( GameObject * obj, GameObject * killer ) override
	{
		deadYet = true;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			switch (param)
			{
				case M01_START_ACTING_JDG: //set timer--then call in next reinforcements
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, 0 );
					}
					break;

				case M01_MODIFY_YOUR_ACTION_JDG://here comes player start calling in reinforcements
					{
						if (obj)
						{
							int currentDifficulty = Commands->Get_Difficulty_Level( );
							int medium = 1;
							int hard = 2;
							float delayTimer;

							if (currentDifficulty == hard)
							{	
								delayTimer = 60;
							}

							else if (currentDifficulty == medium)
							{
								delayTimer = 90;
							}

							else
							{
								delayTimer = 120;
							}

							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_BARN_CHINOOK_JDG, delayTimer );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
						}
					}
					break;

				case M01_SEND_BARN_CHINOOK_JDG: //here comes player start calling in reinforcements
					{
						if (obj)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SEND_BARN_CHINOOK_JDG, 0 );
						}
					}
					break;

				case M01_CALL_IN_REINFORCEMENTS_JDG: //here comes player start calling in reinforcements
					{
						if (obj && deadYet == false)
						{
							GameObject * nodCommanderDialogController = Commands->Find_Object ( 103398 );
							if (nodCommanderDialogController != NULL)
							{
								Commands->Send_Custom_Event( obj, nodCommanderDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}
							float delayTimer = Commands->Get_Random ( 15, 30 ); 
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, delayTimer );
						}
					}
					break;
			}
		}
	}		
};*/

DECLARE_SCRIPT(M01_TailgunRun_NOD_Commander_JDG, "")//this guys ID is M01_TAILGUNAREA_NOD_COMMANDER_JDG 102358
{
	bool deadYet;
	bool firstTimeDamaged;
	bool playerSeen;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(deadYet, 1);
		SAVE_VARIABLE(firstTimeDamaged, 2);
		SAVE_VARIABLE(playerSeen, 2);
	}

	void Created( GameObject * obj ) override 
	{
		deadYet = false;
		firstTimeDamaged = true;
		playerSeen = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			if (damager == STAR && deadYet == false && firstTimeDamaged == true)
			{	
				Vector3 myPosition = Commands->Get_Position ( obj );
				Vector3 playerPosition = Commands->Get_Position ( STAR );
				float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

				if (playerDistance >= 35)
				{
					firstTimeDamaged = false;

					const char* animationName = M01_Choose_Duck_Animation();

					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
					params.Set_Animation( animationName, false );
					Commands->Action_Play_Animation (  obj, params );
				}	
			}
		}
	}

	void Enemy_Seen( GameObject * /* obj */, GameObject * enemy ) override
	{
		if (enemy == STAR)
		{
			playerSeen = true;
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR)
		{
			if (/*sound.Type == SOUND_TYPE_GUNSHOT ||*/ sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				if (obj)
				{
					if (deadYet == false && playerSeen == false)
					{	
						Vector3 myPosition = Commands->Get_Position ( obj );
						Vector3 playerPosition = Commands->Get_Position ( STAR );
						float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

						if (playerDistance >= 15)
						{
							//firstTimeDamaged = false;
							const char* animationName = M01_Choose_Duck_Animation();
							ActionParamsStruct params;
							params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
							params.Set_Animation( animationName, false );
							Commands->Action_Play_Animation (  obj, params );
						}	
					}
				}
			}

		}
	}

	void Killed( GameObject * /* obj */, GameObject * /* killer */ ) override
	{
		deadYet = true;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			switch (param)
			{
				case M01_START_ACTING_JDG: //set timer--then call in next reinforcements
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, 0 );
					}
					break;

				case M01_MODIFY_YOUR_ACTION_JDG://here comes player start calling in reinforcements
					{
						if (obj)
						{
							int currentDifficulty = Commands->Get_Difficulty_Level( );
							int medium = 1;
							int hard = 2;
							float delayTimer;

							if (currentDifficulty == hard)
							{	
								delayTimer = 60;
							}

							else if (currentDifficulty == medium)
							{
								delayTimer = 90;
							}

							else
							{
								delayTimer = 120;
							}

							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_TAILGUN_CHINOOK_JDG, delayTimer );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
						}
					}
					break;

				case M01_SEND_TAILGUN_CHINOOK_JDG: //here comes player start calling in reinforcements
					{
						if (obj)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SEND_TAILGUN_CHINOOK_JDG, 0 );
						}
					}
					break;

				case M01_CALL_IN_REINFORCEMENTS_JDG: //here comes player start calling in reinforcements
					{
						if (obj && deadYet == false)
						{
							GameObject * nodCommanderDialogController = Commands->Find_Object ( 103398 );
							if (nodCommanderDialogController != NULL)
							{
								Commands->Send_Custom_Event( obj, nodCommanderDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}
							float delayTimer = Commands->Get_Random ( 15, 30 ); 
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, delayTimer );
						}
					}
					break;
			}
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_NOD_Commander_JDG, "")//M01_CHURCHAREA_NOD_COMMANDER_JDG		102357
{
	bool deadYet;
	bool firstTimeDamaged;
	bool playerSeen;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(deadYet, 1);
		SAVE_VARIABLE(firstTimeDamaged, 2);
		SAVE_VARIABLE(playerSeen, 3);
	}

	void Created( GameObject * obj ) override 
	{
		deadYet = false;
		firstTimeDamaged = true;
		playerSeen = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			if (damager == STAR && deadYet == false && firstTimeDamaged == true && playerSeen == false)
			{	
				Vector3 myPosition = Commands->Get_Position ( obj );
				Vector3 playerPosition = Commands->Get_Position ( STAR );
				float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

				if (playerDistance >= 15)
				{
					firstTimeDamaged = false;
					const char* animationName = M01_Choose_Duck_Animation();
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
					params.Set_Animation( animationName, false );
					Commands->Action_Play_Animation (  obj, params );
				}	
			}
		}
	}

	void Enemy_Seen( GameObject * /* obj */, GameObject * enemy ) override
	{
		if (enemy == STAR)
		{
			playerSeen = true;
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR)
		{
			if (/*sound.Type == SOUND_TYPE_GUNSHOT ||*/ sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				if (obj)
				{
					if (deadYet == false && playerSeen == false)
					{	
						Vector3 myPosition = Commands->Get_Position ( obj );
						Vector3 playerPosition = Commands->Get_Position ( STAR );
						float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

						if (playerDistance >= 15)
						{
							//firstTimeDamaged = false;
							const char* animationName = M01_Choose_Duck_Animation();
							ActionParamsStruct params;
							params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
							params.Set_Animation( animationName, false );
							Commands->Action_Play_Animation (  obj, params );
						}	
					}
				}
			}

		}
	}

	void Killed( GameObject * /* obj */, GameObject * /* killer */ ) override
	{
		deadYet = true;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			switch (param)
			{
				case M01_START_ACTING_JDG: //set timer--then call in next reinforcements
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, 0 );
					}
					break;

				case M01_MODIFY_YOUR_ACTION_JDG://here comes player start calling in reinforcements
					{
						if (obj)
						{
							int currentDifficulty = Commands->Get_Difficulty_Level( );
							int medium = 1;
							int hard = 2;
							float delayTimer;

							if (currentDifficulty == hard)
							{	
								delayTimer = 60;
							}

							else if (currentDifficulty == medium)
							{
								delayTimer = 90;
							}

							else
							{
								delayTimer = 120;
							}

							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_CHURCH_CHINOOK_JDG, delayTimer );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
						}
					}
					break;

				case M01_SEND_CHURCH_CHINOOK_JDG: //here comes player start calling in reinforcements
					{
						if (obj)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SEND_CHURCH_CHINOOK_JDG, 0 );
						}
					}
					break;

				case M01_CALL_IN_REINFORCEMENTS_JDG: //here comes player start calling in reinforcements
					{
						if (obj && deadYet == false)
						{
							GameObject * nodCommanderDialogController = Commands->Find_Object ( 103398 );
							if (nodCommanderDialogController != NULL)
							{
								Commands->Send_Custom_Event( obj, nodCommanderDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}
							float delayTimer = Commands->Get_Random ( 15, 30 ); 
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, delayTimer );
						}
					}
					break;
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDI_Base_Artillery_Controller_JDG, "")//M01_GDIBASE_ARTILLERY_CONTROLLER_ID	102294
{				
	Vector3 locs[35];
	Vector3 sound_location;
	int last;
	int bomb_sound;
	float delayTimerMin;
	float delayTimerMax;
	bool command_clearance;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sound_location, 1);
		SAVE_VARIABLE(last, 2);
		SAVE_VARIABLE(bomb_sound, 3);
		SAVE_VARIABLE(command_clearance, 4);
		SAVE_VARIABLE(delayTimerMin, 5);
		SAVE_VARIABLE(delayTimerMax, 6);
	}

	void Created( GameObject * obj ) override 
	{
		locs[0] = Vector3(96.983f, -75.433f, -7.894f);
		locs[1] = Vector3(64.778f,-70.692f, -7.917f );
		locs[2] = Vector3(39.923f, -55.085f, -6.986f);
		locs[3] = Vector3(64.088f, -51.238f, -1.130f);
		locs[4] = Vector3(90.243f, -42.054f, 7.820f);
		locs[5] = Vector3(101.932f, -22.724f, 3.892f);
		locs[6] = Vector3(110.260f, 6.870f, 11.455f);
		locs[7] = Vector3(104.993f, 34.865f, 7.102f);
		locs[8] = Vector3(88.132f, 31.827f, 9.156f);
		locs[9] = Vector3(82.538f, -2.162f, 7.260f);
		locs[10] = Vector3(73.697f, 10.886f, 10.622f);
		locs[11] = Vector3(61.373f, -9.328f, 9.156f);
		locs[12] = Vector3(45.354f, -11.552f, 7.243f);
		locs[13] = Vector3(53.740f, -1.580f, 13.105f);
		locs[14] = Vector3(53.059f, 17.069f, 13.105f);
		locs[15] = Vector3(24.016f, 8.417f, 7.113f);
		locs[16] = Vector3(22.190f, -4.718f, 10.245f);
		locs[17] = Vector3(1.074f, -48.981f, -1.604f);
		locs[18] = Vector3(-17.956f, -48.856f, 3.441f);
		locs[19] = Vector3(-46.404f, -26.827f, 6.827f);
		locs[20] = Vector3(-40.934f, -20.838f, 2.387f);
		locs[21] = Vector3(9.578f, -17.054f, 7.419f);
		locs[22] = Vector3(-6.474f, -17.783f, 5.052f);
		locs[23] = Vector3(-8.360f, -6.935f, 11.862f);
		locs[24] = Vector3(5.836f, -4.874f, 17.664f);
		locs[25] = Vector3(-40.130f, -1.333f, 6.052f);
		locs[26] = Vector3(-43.953f, 5.812f, 14.117f);
		locs[27] = Vector3(-50.261f, 32.361f, 5.410f);
		locs[28] = Vector3(-50.813f, 48.439f, 11.585f);
		locs[29] = Vector3(-40.847f, 60.449f, 5.454f);
		locs[30] = Vector3(-36.209f, 68.892f, 14.059f);
		locs[31] = Vector3(-14.355f, 40.254f, 3.190f);
		locs[32] = Vector3(9.023f, 39.898f, 4.112f);
		locs[33] = Vector3(-5.525f, 41.241f, 12.043f);
		locs[34] = Vector3(24.496f, 52.051f, 9.968f);

		last = 40;
		delayTimerMin = 20;
		delayTimerMax = 30;
		command_clearance = true;

		Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 2 );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		switch (param)
		{
			case M01_PICK_A_NEW_LOCATION_JDG: 
				{
					if (command_clearance == true)
					{
						int random = int(Commands->Get_Random(0, 35-WWMATH_EPSILON));
						while (random == last) 
						{
							random = int(Commands->Get_Random(0, 35-WWMATH_EPSILON));
						}

						last = random;

						sound_location = locs[random];
						sound_location.Z += 10;

						bomb_sound = Commands->Create_Sound ( "Bomb_Falling_Sound_Twiddler", sound_location, obj );
						Commands->Monitor_Sound ( obj, bomb_sound );
					}
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					command_clearance = false;	

					if (sender == Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG ))//your "commander" is dead--destroy yourself
					{
						Commands->Destroy_Object ( obj );
					}
				}
				break;

			case M01_START_ACTING_JDG:
				{
					command_clearance = true;	
					Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 2 );
				}
				break;
		}

		if (type == CUSTOM_EVENT_SOUND_ENDED  )
		{
			if (param == bomb_sound)
			{
				Vector3 explosion_location = sound_location;
				
				explosion_location.Z -= 10;
				Commands->Create_Explosion("Ground Explosions Twiddler", explosion_location, NULL);

				float delayTimer = Commands->Get_Random ( delayTimerMin, delayTimerMax );
				Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
			}
		}
	}				
};



DECLARE_SCRIPT(M01_Nod_GuardTower_01_Enter_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-148.379f, 342.954f, 9.656f));
			Commands->Attach_Script(sniperTarget01, "M01_GuardTower_Sniper_Target_JDG", "-157.390 363.504 10.695, 148.379 342.954 9.656");

			GameObject * sniperTarget02 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-149.930f, 338.900f, 9.170f));
			Commands->Attach_Script(sniperTarget02, "M01_GuardTower_Sniper_Target_JDG", "-158.760 357.590 10.695, -149.930 338.900 9.170");

			GameObject * sniperTarget03 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-146.812f, 339.745f, 9.345f));
			Commands->Attach_Script(sniperTarget03, "M01_GuardTower_Sniper_Target_JDG", "-162.892 357.941 10.655, -146.812 339.745 9.345");

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Nod_GuardTower_02_Enter_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-153.944f, 602.434f, 4.226f));
			Commands->Attach_Script(sniperTarget01, "M01_GuardTower_Sniper_Target_JDG", "-141.665 573.039 4.409, -153.944 602.434 4.226");

			GameObject * sniperTarget02 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-156.672f, 605.082f, 4.163f));
			Commands->Attach_Script(sniperTarget02, "M01_GuardTower_Sniper_Target_JDG", "-138.653 576.128 4.889, -156.672 605.082 4.163");

			GameObject * sniperTarget03 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-160.792f, 606.559f, 4.075f));
			Commands->Attach_Script(sniperTarget03, "M01_GuardTower_Sniper_Target_JDG", "-143.313 580.514 4.337, -160.792 606.559 4.075");

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Nod_GuardTower_03_Enter_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-254.164f, 564.968f, 30.630f));
			Commands->Attach_Script(sniperTarget01, "M01_GuardTower_Sniper_Target_JDG", "-221.487 556.431 25.927, -254.164 564.968 30.630");

			GameObject * sniperTarget02 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-253.935f, 561.105f, 30.922f));
			Commands->Attach_Script(sniperTarget02, "M01_GuardTower_Sniper_Target_JDG", "-221.169 561.729 25.662, -253.935 561.105 30.922");

			GameObject * sniperTarget03 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-257.185f, 562.242f, 30.540f));
			Commands->Attach_Script(sniperTarget03, "M01_GuardTower_Sniper_Target_JDG", "-225.770 558.924 26.635, -257.185 562.242 30.540");

			GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-200.897f, 533.210f, 19.5f));
			Commands->Attach_Script(controller1, "Test_Cinematic", "X1I_HON_Medlab_Chinook.txt");

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_GuardTower_Sniper_Target_JDG, "wave_location:vector3,  delete_location:vector3")
{
	Vector3 runToPosition;
	Vector3 leavePosition;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(runToPosition, 1);
		SAVE_VARIABLE(leavePosition, 2);
	}

	void Created( GameObject * obj ) override 
	{
		runToPosition = Get_Vector3_Parameter("wave_location");
		leavePosition = Get_Vector3_Parameter("delete_location");

		Commands->Enable_Hibernation( obj, false );

		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( runToPosition, RUN, 1);

		Commands->Action_Goto ( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
		powerupSpawnLocation.Z += 0.75f;

		const auto& powerups = M01Vars::Powerups;
		int random = Commands->Get_Random_Int(0, 2);

		//GameObject *spawn_effect = Commands->Create_Object( "Spawner Created Special Effect", powerupSpawnLocation );

		//if (spawn_effect)
		//{
		//	Commands->Attach_Script( spawn_effect, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
		//}

		Commands->Create_Object ( powerups[random], powerupSpawnLocation );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_FACING_SPECIFIED_DIRECTION_01_JDG)
		{
			const char* animationName = M01_Choose_Search_Animation ( );

			params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation( animationName, false );
			Commands->Action_Play_Animation ( obj, params );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		switch (complete_reason)
		{
			case ACTION_COMPLETE_NORMAL: 
				{
					switch (action_id)
					{
						case M01_WALKING_WAYPATH_01_JDG:
							{
								params.Set_Basic( this, 100, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
								params.Set_Attack( STAR, 0, 0, true );
								Commands->Action_Attack ( obj, params );

								Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 1 );
							}
							break;

						case M01_DOING_ANIMATION_01_JDG:
							{
								params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
								params.Set_Movement( leavePosition, RUN, 5);

								Commands->Action_Goto ( obj, params );
							}
							break;

						case M01_WALKING_WAYPATH_02_JDG:
							{
								Commands->Destroy_Object ( obj );
							}
							break;
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_Gunboat_Spawn_Hovercraft_Zone_JDG, "")//this guys id is 102500
{
	/*bool okay_to_trigger;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_trigger, 1);
	}

	void Created( GameObject * obj ) override 
	{
		okay_to_trigger = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (param == M01_MODIFY_YOUR_ACTION_JDG)
		{
			okay_to_trigger = true;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (okay_to_trigger == true)
		{
			if (enterer == Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID ))
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SPAWN_THE_MEDIUM_TANK_JDG, 0 );
			}
		}
	}*/
};

DECLARE_SCRIPT(M01_BasalBuilding_Population_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		float myHomeRadius = Commands->Get_Random ( 2, 10 );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, myHomeRadius );
	}
};

DECLARE_SCRIPT(M01_HarvesterScript_New_JDG, "")
{
	int animation_count;
	bool damagedYet;
	bool deadYet;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(animation_count, 1);
		SAVE_VARIABLE(damagedYet, 2);
		SAVE_VARIABLE(deadYet, 3);
	}

	void Created( GameObject * obj ) override 
	{
		animation_count = 0;
		damagedYet = false;
		deadYet = false;

		ActionParamsStruct params;

		params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
		Vector3 loc (-103.5f, 375.6f, 19.0f);
		loc.X += Commands->Get_Random(-1.0f, 1.0f);
		loc.Y += Commands->Get_Random(-1.0f, 1.0f);
		params.Set_Movement(loc, 0.25f, 1);
		Commands->Action_Goto(obj, params);
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj && damager == STAR)
		{
			if (damagedYet == false)
			{
				if (deadYet == false)
				{
					Commands->Send_Custom_Event ( obj, obj, 0, M01_START_ACTING_JDG, 0.25 );
				}
				damagedYet = true;
				
			}
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				if (deadYet == false)
				{
					int lockeHarvester = Commands->Create_Conversation( "M01_Add_Harvester_Objective", 100, 1000, true);
					Commands->Join_Conversation( NULL, lockeHarvester, false, false, true );
					Commands->Start_Conversation( lockeHarvester,  lockeHarvester );
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		deadYet = true;
		Commands->Create_Sound ( "EVA_Nod_Unit_Destroyed", Vector3 (0,0,0), obj);
		Vector3 HarvesterDeathSpot (-93.9f, 426.6f, 21.0f);
		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", HarvesterDeathSpot);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason /* complete_reason */) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			animation_count = 0;
			Commands->Set_Animation(obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", false, NULL, 0.0f, -1.0f, false);
		}
	}

	void Animation_Complete(GameObject * obj, const char * /* anim */) override
	{
		if (animation_count <= 2)
		{
			Commands->Set_Animation(obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", false, NULL, 0.0f, -1.0f, false);
			animation_count++;
		} 

		else
		{
			Gather_Tiberium();
		}
	}

	void Gather_Tiberium(void)
	{
		ActionParamsStruct params;

		params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
		Vector3 loc (-103.5f, 375.6f, 19.0f);
		loc.X += Commands->Get_Random(-3.0f, 3.0f);
		loc.Y += Commands->Get_Random(-3.0f, 3.0f);
		params.Set_Movement(loc, 0.25f, 2);
		Commands->Action_Goto(Owner(), params);
	}
};

DECLARE_SCRIPT(M01_TibField_Guard01_New_JDG, "")
{
	void Custom( GameObject * /* obj */, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_START_ACTING_JDG)
		{
			GameObject * chemGuy01 = Commands->Find_Object ( 103270 );
			GameObject * chemGuy02 = Commands->Find_Object ( 103271 );
			GameObject * chemGuy03 = Commands->Find_Object ( 103272 );

			if (chemGuy01 != NULL)
			{
				if (chemGuy02 != NULL)
				{
					if (chemGuy03 != NULL)
					{
						int tibFieldConv = Commands->Create_Conversation( "M01_TiberiumField_Conversation", 80, 25, true);
						Commands->Join_Conversation( chemGuy01, tibFieldConv, true, true, true);
						Commands->Join_Conversation( chemGuy02, tibFieldConv, true, true, true);
						Commands->Join_Conversation( chemGuy03, tibFieldConv, true, true, true);
						Commands->Start_Conversation( tibFieldConv,  tibFieldConv );
					}
				}
			}
		}
	}
	
};

DECLARE_SCRIPT(M01_Barn_Babushkas_Conversation_Zone_JDG, "")//this guys ID is 101662
{
	bool okay_to_play_conversation;
	bool playerEntered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_play_conversation, 1);
		SAVE_VARIABLE(playerEntered, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okay_to_play_conversation = false;
		playerEntered = false;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//babushka is in position--give okay for conversation
		{
			okay_to_play_conversation = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_CUE_BABUSHKA_CONVERSATION_JDG, 0 );
		}

		else if (param == M01_CUE_BABUSHKA_CONVERSATION_JDG)
		{
			if (playerEntered == true && okay_to_play_conversation == true)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_BABUSHKA_CONVERSATION_JDG, 0 );

				GameObject * babushkaZone = Commands->Find_Object ( 101662 );
				GameObject * billyZone = Commands->Find_Object ( 101661 );

				if (babushkaZone != NULL)
				{
					Commands->Destroy_Object ( babushkaZone );//this is one-time only zone--cleaning up
				}

				if (billyZone != NULL)
				{
					Commands->Destroy_Object ( billyZone );//this is one-time only zone--cleaning up
				}
			}
		}
	}

	void Exited( GameObject * /* obj */, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			playerEntered = false;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			playerEntered = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_CUE_BABUSHKA_CONVERSATION_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Barn_Point_Guard_01_JDG, "")//this guys ID is 101439
{
	void Created( GameObject * obj ) override
	{	
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			if (damager == STAR)
			{
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );

				if (Commands->Find_Object ( 101658 ))
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101658 ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );//this is sent to his nearby buddy
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_BARN_GUARD_IS_DEAD_JDG, 0 );
	}
};

DECLARE_SCRIPT(M01_Barn_Point_Guard_02_JDG, "")//this guys ID is 101658
{
	bool retreating;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(retreating, 1);
	}

	void Created( GameObject * obj ) override
	{
		Vector3 myPosition = Commands->Get_Position ( obj );

		retreating = false;

		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 2.0f );
	}

	/*void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		if (param == M01_PLAYER_IS_ATTACKING_ME_JDG)
		{
			Commands->Set_Innate_Is_Stationary (  obj, false );

			params.Set_Basic(this, 60, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Vector3 (-44.8f, 117.8f, 2.6f), .8f, 1 );
			params.Set_Attack( STAR, 15, 0, true );
			Commands->Action_Attack ( obj, params );
		}
	}*/

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			if (damager == STAR)
			{
				if (obj && retreating == false)
				{
					if (Commands->Get_Health (obj) > 0)
					{
						Vector3 myPosition = Commands->Get_Position ( obj );
						Commands->Create_Sound ( "WereOutGunned_2", myPosition, obj );
						Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );

						//ActionParamsStruct params;
						//params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
						//params.Set_Movement( Commands->Find_Object ( M01_BARN_PRISONER_01_ID ), 1, 5 );
						//params.Set_Attack( STAR, 50, 0, true );
						//Commands->Action_Attack ( obj, params );
						retreating = true;
					}
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_BARN_GUARD_IS_DEAD_JDG, 0 );
	}
};



DECLARE_SCRIPT(M01_Barn_Door_Guard_JDG, "")//this guys ID is M01_BARN_DOOR_GUARD_ID	101512
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5.0f );
	}

	//void Damaged( GameObject * obj, GameObject * damager, float amount ) override
	//{
	//	Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_START_BARN_OBJECTIVE_JDG, 0 );
	//}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_BARN_GUARD_IS_DEAD_JDG, 0 );
	}	
};

DECLARE_SCRIPT(M01_Barn_Talk_Guard_01_JDG, "")//this guys ID is M01_BARN_TALK_GUARD_01_ID 103318
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5.0f );
	}

	//void Damaged( GameObject * obj, GameObject * damager, float amount ) override
	//{
	//	Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_START_BARN_OBJECTIVE_JDG, 0 );
	//}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_BARN_GUARD_IS_DEAD_JDG, 0 );
	}
};

DECLARE_SCRIPT(M01_BarnArea_AI_ExitZone_JDG, "")//112015 -- tunnel 112016 -- shack  
{
	int hovercraftGuyId;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hovercraftGuyId, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		hovercraftGuyId = 0;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == M01_ROPE_IS_SENDING_ID_JDG)
		{
			hovercraftGuyId = param;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		GameObject * hovercraftGuy = Commands->Find_Object ( hovercraftGuyId );
		if (hovercraftGuy != NULL && enterer == hovercraftGuy)
		{
			Commands->Send_Custom_Event( obj, hovercraftGuy, 0, M01_HOLD_YOUR_POSITION_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_TurretBeach_GDI_Guy_01_JDG, "")//
{
	int turretbeach_conv;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(turretbeach_conv, 1);
	}

	void Created( GameObject * obj ) override 
	{
		int myId = Commands->Get_ID ( obj );
		GameObject * myExitZone01 = Commands->Find_Object ( 112015 );
		GameObject * myExitZone02 = Commands->Find_Object ( 112016 );

		if (myExitZone01 != NULL)
		{
			Commands->Send_Custom_Event( obj, myExitZone01, M01_ROPE_IS_SENDING_ID_JDG, myId, 0 );
		}

		if (myExitZone02 != NULL)
		{
			Commands->Send_Custom_Event( obj, myExitZone02, M01_ROPE_IS_SENDING_ID_JDG, myId, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_HOLD_YOUR_POSITION_JDG)
			{
				ActionParamsStruct params;
				Commands->Action_Reset ( obj, 100 );
				Vector3 myGuardSpot (-49.146f, 201.741f, 1.590f);
				params.Set_Basic( this, 55, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( myGuardSpot, RUN, 1 );

				Commands->Action_Goto( obj, params );

				Commands->Set_Innate_Soldier_Home_Location ( obj, myGuardSpot, 5 );
			}
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_X1E_Run") == 0)
		{
			ActionParamsStruct params;

			params.Set_Basic(this, 100, M01_HUNT_THE_PLAYER_JDG);
			params.Set_Movement(STAR, RUN, 6);

			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_HUNT_THE_PLAYER_JDG)
		{
			turretbeach_conv = Commands->Create_Conversation( "M01_TurretBeach_TurnOverTank_Conversation", 100, 50, true);
			Commands->Join_Conversation( obj, turretbeach_conv, true, true, true );
			Commands->Start_Conversation( turretbeach_conv,  turretbeach_conv );

			Commands->Monitor_Conversation (obj, turretbeach_conv);
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == turretbeach_conv)
			{
				params.Set_Basic(this, 80, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement(Vector3 (-49.7f, 109.7f, 2.0f), RUN, 0.5f);

				Commands->Action_Goto(obj, params);
			}
		}

		else if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 10 );
		}
	}
};

DECLARE_SCRIPT(M01_TurretBeach_GDI_Guy_02_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_X1E_Run") == 0)
		{
			ActionParamsStruct params;

			GameObject * nearbyTailgun = Commands->Find_Object ( 101448 );
			if (nearbyTailgun != NULL)
			{
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement(nearbyTailgun, RUN, 0.5f);

				Commands->Action_Goto(obj, params);
			}

			else
			{
				params.Set_Basic(this, 80, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement(Vector3 (-65.4f, 204.6f, 3.7f), RUN, 0.5f);

				Commands->Action_Goto(obj, params);
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 10 );
		}
	}
};











DECLARE_SCRIPT(M01_DetentionPen_CivDeathMonitor, "")
{
	bool damagedYet;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(damagedYet, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		damagedYet = false;
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		if (obj && damagedYet == false)
		{
			damagedYet = true;
			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0512I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0513I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Mission_Complete ( false );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103380 ), 0, M01_CIVILIAN_KILLED_JDG, 0 );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_GOING_TO_EVAC_SPOT02_JDG: 
				{
					Vector3 myEvacSpot = Commands->Get_Position ( Commands->Find_Object ( 103380 ) );
					ActionParamsStruct params;
					params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT02_JDG);
					params.Set_Movement(myEvacSpot, RUN, 0.25f);

					Commands->Action_Goto(obj, params);
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_DetentionPen_GDIDeathMonitor, "")
{
	bool damagedYet;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(damagedYet, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		damagedYet = false;
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		if (obj && damagedYet == false)
		{
			damagedYet = true;
			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0512I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0513I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Mission_Complete ( false );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103380 ), 0, M01_CIVILIAN_KILLED_JDG, 0 );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_GOING_TO_EVAC_SPOT02_JDG: 
				{
					Vector3 myEvacSpot = Commands->Get_Position ( Commands->Find_Object ( 103381 ) );
					ActionParamsStruct params;
					params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT02_JDG);
					params.Set_Movement(myEvacSpot, RUN, 0.25f);

					Commands->Action_Goto(obj, params);
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_DetentionPen_Evac_Controller01_JDG, "")//103380
{
	//civGuy01 = 101929
	//civGuy02 = 101930
	//civGuy03 = 101931
	int evac_waypath_id;
	int evac_helicopter_id;
	int deadPrisonerTally;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(evac_waypath_id, 1);
		SAVE_VARIABLE(evac_helicopter_id, 2);
		SAVE_VARIABLE(deadPrisonerTally, 3);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		deadPrisonerTally = 0;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_START_ACTING_JDG)//prisoner have been freed--call in air evac
		{
			GameObject * civGuy01 = Commands->Find_Object ( 101929 );
			GameObject * civGuy02 = Commands->Find_Object ( 101930 );
			GameObject * civGuy03 = Commands->Find_Object ( 101931 );

			if ((civGuy01 != NULL) || (civGuy02 != NULL) || (civGuy03 != NULL) )
			{
				//Vector3 evacPosition = Commands->Get_Position ( obj );
				//GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
				//Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_Detention_EvacAnim.txt");
			}
		}

		else if (param == M01_CIVILIAN_KILLED_JDG)
		{
			deadPrisonerTally++;

			if (deadPrisonerTally == 1)
			{
				Commands->Mission_Complete ( false );
			}
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG)//civs are on board--send aircraft away
		{
			GameObject * evacChopper = Commands->Find_Object ( evac_helicopter_id );
			GameObject * evacWaypath = Commands->Find_Object ( evac_waypath_id );

			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_DetentionCiv_Air_Evac_Waypath_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_TransprtBone.XG_EV1_PathA") == 0)
		{
			Commands->Set_Animation ( obj, "XG_TransprtBone.XG_EV1_Pathloop", true , NULL, 0.0f, -1.0f, false);
		}

		else if (stricmp(anim, "XG_TransprtBone.XG_EV1_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Set_Animation ( obj, "XG_TransprtBone.XG_EV1_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_DetentionCiv_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV1_trnsA") == 0)
		{
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV1_trnsloop", true, NULL, 0.0f, -1.0f, false );

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );

			GameObject * civGuy01 = Commands->Find_Object ( 101929 );
			GameObject * civGuy02 = Commands->Find_Object ( 101930 );
			GameObject * civGuy03 = Commands->Find_Object ( 101931 );

			if (civGuy01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, civGuy01, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (civGuy02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, civGuy02, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (civGuy03 != NULL)
			{
				Commands->Send_Custom_Event ( obj, civGuy03, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV1_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV1_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * civGuy01 = Commands->Find_Object ( 101929 );
					GameObject * civGuy02 = Commands->Find_Object ( 101930 );
					GameObject * civGuy03 = Commands->Find_Object ( 101931 );

					if ((civGuy01 != NULL) || (civGuy02 != NULL) || (civGuy03 != NULL) )
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
					}

					else
					{
						GameObject * civController = Commands->Find_Object ( 103380 );
						if (civController != NULL)
						{
							Commands->Send_Custom_Event ( obj, civController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
						}
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_DetentionPen_Evac_Controller02_JDG, "")//103381
{
	//gdiGuy01 = 101926
	//gdiGuy02 = 101927
	//gdiGuy03 = 101928
	int evac_waypath_id;
	int evac_helicopter_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(evac_waypath_id, 1);
		SAVE_VARIABLE(evac_helicopter_id, 2);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_START_ACTING_JDG)//prisoner have been freed--call in air evac
		{
			GameObject * civGuy01 = Commands->Find_Object ( 101929 );
			GameObject * civGuy02 = Commands->Find_Object ( 101930 );
			GameObject * civGuy03 = Commands->Find_Object ( 101931 );

			if ((civGuy01 != NULL) || (civGuy02 != NULL) || (civGuy03 != NULL) )
			{
				//Vector3 evacPosition = Commands->Get_Position ( obj );
				//GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
				//Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_Detention_EvacAnim.txt");
			}
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG)//civs are on board--send aircraft away
		{
			GameObject * evacChopper = Commands->Find_Object ( evac_helicopter_id );
			GameObject * evacWaypath = Commands->Find_Object ( evac_waypath_id );

			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_DetentionGDI_Air_Evac_Waypath_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_TransprtBone.XG_EV1_PathA") == 0)
		{
			Commands->Set_Animation ( obj, "XG_TransprtBone.XG_EV1_Pathloop", true, NULL, 0.0f, -1.0f, false );
		}

		else if (stricmp(anim, "XG_TransprtBone.XG_EV1_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Set_Animation ( obj, "XG_TransprtBone.XG_EV1_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_DetentionGDI_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV1_trnsA") == 0)
		{
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV1_trnsloop", true, NULL, 0.0f, -1.0f, false );

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );

			GameObject * GDIGuy01 = Commands->Find_Object ( 101926 );
			GameObject * GDIGuy02 = Commands->Find_Object ( 101927 );
			GameObject * GDIGuy03 = Commands->Find_Object ( 101928 );

			if (GDIGuy01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, GDIGuy01, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (GDIGuy02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, GDIGuy02, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (GDIGuy03 != NULL)
			{
				Commands->Send_Custom_Event ( obj, GDIGuy03, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV1_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV1_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * GDIGuy01 = Commands->Find_Object ( 101926 );
					GameObject * GDIGuy02 = Commands->Find_Object ( 101927 );
					GameObject * GDIGuy03 = Commands->Find_Object ( 101928 );

					if ((GDIGuy01 != NULL) || (GDIGuy02 != NULL) || (GDIGuy03 != NULL) )
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
					}

					else
					{
						GameObject * civController = Commands->Find_Object ( 103381 );
						if (civController != NULL)
						{
							Commands->Send_Custom_Event ( obj, civController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
						}
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_Nod_Chinook_Reinforcement_Guy_JDG, "")//
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			Vector3 offset;
			offset.X = Commands->Get_Random(1, 3);
			offset.Y = Commands->Get_Random(1, 3);
			offset.Z = 0;

			ActionParamsStruct params;
			params.Set_Basic(this, 100, 10);
			params.Set_Movement(Commands->Get_Position(obj) + offset, 1.0f, 0.25f);
			Commands->Action_Goto(obj, params);

			Commands->Send_Custom_Event( obj, obj, 0, M01_HUNT_THE_PLAYER_JDG, 1 );
		}
	}	
};

DECLARE_SCRIPT(M01_Hunt_The_Player_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		Commands->Start_Timer ( obj, this, 60, M01_GOTO_IDLE_JDG );
		Commands->Start_Timer ( obj, this, 5, M01_HUNT_THE_PLAYER_JDG );
		ActionParamsStruct params;
		params.Set_Basic( this, 45, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
		params.Set_Movement( STAR, RUN, 1, false );
		Commands->Action_Goto ( obj, params );
	}

	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		if (timer_id == M01_GOTO_IDLE_JDG)
		{
			Commands->Enable_Hibernation( obj, true );
			Vector3 myPosition = Commands->Get_Position ( obj );
			Vector3 playerPosition = Commands->Get_Position ( STAR );
			float distanceFromStar = Commands->Get_Distance ( myPosition, playerPosition );

			if (distanceFromStar >= 80)
			{
				bool visible = Commands->Is_Object_Visible( obj, STAR );

				if (visible != true)
				{
					Commands->Apply_Damage( obj, 100000, "BlamoKiller", NULL );
				}

				else
				{
					Commands->Start_Timer ( obj, this, 20, M01_GOTO_IDLE_JDG );
				}
			}

			else
			{
				Commands->Start_Timer ( obj, this, 20, M01_GOTO_IDLE_JDG );
			}
		}

		else if (timer_id == M01_HUNT_THE_PLAYER_JDG)
		{
			if (obj)
			{
				ActionParamsStruct params;
				params.Set_Basic( this, 45, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
				params.Set_Movement( STAR, RUN, 1, false );
				Commands->Action_Goto ( obj, params );
				Commands->Start_Timer ( obj, this, 5, M01_HUNT_THE_PLAYER_JDG );
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_Escorts_Warroom_MCT_Commander_JDG, "")//103373
{
	void Created( GameObject * obj ) override 
	{ 
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_START_ACTING_JDG)//here come the mct escorts--run into mct room
		{
			Commands->Set_Innate_Is_Stationary ( obj, false );
			Vector3 myHomeSpot(-181.1f, 541.4f, 3.5f);
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(myHomeSpot, RUN, 0.25f);

			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );

			ActionParamsStruct params;
			params.Set_Basic(this, 45, M01_START_ATTACKING_01_JDG);
			params.Set_Attack( STAR, 50, 0, false );

			Commands->Action_Attack (obj, params);
		}
	}
};

DECLARE_SCRIPT(M01_HON_Escorts_Warroom_MCT_Protector01_JDG, "")//103374
{
	void Created( GameObject * obj ) override 
	{ 
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}
};

DECLARE_SCRIPT(M01_HON_Escorts_Warroom_MCT_Protector02_JDG, "")//103375
{
	void Created( GameObject * obj ) override 
	{ 
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}
};



DECLARE_SCRIPT(M01_HON_Engineer02_JDG, "")//101948
{
	bool repairSoundPlaying;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(repairSoundPlaying, 1);
	}

	void Created( GameObject * obj ) override 
	{ 
		repairSoundPlaying = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_PLAYER_IS_ATTACKING_ME_JDG)
		{
			Commands->Enable_Hibernation( obj, false );
			Vector3 myRepairSpot (-178.925f, 542.943f, 4.932f);
			ActionParamsStruct params;
			params.Set_Basic( this, 100, M01_START_ACTING_JDG );
			params.Set_Attack( myRepairSpot, 10, 0, false );
			params.AttackCheckBlocked = false;

			Commands->Action_Attack ( obj, params );

			Commands->Start_Timer ( obj, this, 2, M01_START_ACTING_JDG );
		}
	}

	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		if (timer_id == M01_START_ACTING_JDG)
		{
			Commands->Enable_Hibernation( obj, true );
			
			Commands->Action_Reset ( obj, 100 );

			Vector3 repair_sound_spot (-175.074f, 533.086f, 19.327f);
			

			GameObject * handOfNod = Commands->Find_Object ( M01_HAND_OF_NOD_JDG );
			float hon_max_health = Commands->Get_Max_Health ( handOfNod );
			float hon_max_shield = Commands->Get_Max_Shield_Strength ( handOfNod );

			float hon_current_health = Commands->Get_Health ( handOfNod );

			if (hon_current_health > 0)
			{
				Commands->Set_Health ( handOfNod, hon_max_health );
				Commands->Set_Shield_Strength ( handOfNod, hon_max_shield );

				if (repairSoundPlaying == false)
				{
					Commands->Create_Sound ( "CnC_Healer_Sound", repair_sound_spot, obj );
					
					repairSoundPlaying = true;
					Commands->Start_Timer ( obj, this, 5, M01_GOTO_IDLE_JDG );
				}
			}
		}

		else if (timer_id == M01_GOTO_IDLE_JDG)
		{
			repairSoundPlaying = false;
		}
	}
};





DECLARE_SCRIPT(M01_HON_Chinook_Spawned_Soldier_03_GDI_JDG, "")
{
	int buggy_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(buggy_id, 1);
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			Vector3 myHomeSpot(-209.853f, 505.532f, 4.017f);//going to hand of nod backside
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(myHomeSpot, RUN, 3);
			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			GameObject * buggyController = Commands->Find_Object ( 103367 );
			if (buggyController != NULL)
			{
				Commands->Send_Custom_Event ( obj, buggyController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == M01_MODIFY_YOUR_ACTION_JDG)
		{
			buggy_id = param;

			GameObject * buggy = Commands->Find_Object ( buggy_id );
			if (buggy != NULL)
			{
				ActionParamsStruct params;

				params.Set_Basic( this, 100, M01_START_ATTACKING_01_JDG );
				params.Set_Attack( buggy, 20, 3, true );
				Commands->Action_Attack( obj, params );
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_Chinook_Spawned_Soldier_04_GDI_JDG, "")
{
	int buggy_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(buggy_id, 1);
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			Vector3 myHomeSpot(-204.157f, 508.650f, 4.017f);//going to hand of nod backside
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(myHomeSpot, RUN, 3);
			Commands->Action_Goto(obj, params);
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == M01_MODIFY_YOUR_ACTION_JDG)
		{
			buggy_id = param;

			GameObject * buggy = Commands->Find_Object ( buggy_id );
			if (buggy != NULL)
			{
				ActionParamsStruct params;

				params.Set_Basic( this, 100, M01_START_ATTACKING_01_JDG );
				params.Set_Attack( buggy, 20, 3, true );
				Commands->Action_Attack( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			GameObject * buggyController = Commands->Find_Object ( 103367 );
			if (buggyController != NULL)
			{
				Commands->Send_Custom_Event ( obj, buggyController, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_BuggyNew_Controller_JDG, "")//this guys id is 103367
{
	int buggy_id;
	bool gdi_minigunner_01_inPlace;
	bool gdi_minigunner_02_inPlace;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(buggy_id, 1);
		SAVE_VARIABLE(gdi_minigunner_01_inPlace, 2);
		SAVE_VARIABLE(gdi_minigunner_02_inPlace, 3);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		gdi_minigunner_01_inPlace = false;
		gdi_minigunner_02_inPlace = false;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override
	{
		if (param == M01_START_ACTING_JDG)
		{
			Vector3 spawnLocation (-233.946f, 494.424f, 5.234f);
			GameObject *buggy = Commands->Create_Object("Nod_Buggy", spawnLocation);
			Commands->Set_Facing ( buggy, -135 );
			Commands->Attach_Script(buggy, "M01_BuggyScript_New_JDG", "");
			buggy_id = Commands->Get_ID ( buggy );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG)
		{
			gdi_minigunner_01_inPlace = true;
			int minigunner01_id = Commands->Get_ID ( sender );

			GameObject *buggy = Commands->Find_Object ( buggy_id );
			if (buggy != NULL)
			{
				Commands->Send_Custom_Event ( obj, buggy, M01_MODIFY_YOUR_ACTION_02_JDG, minigunner01_id, 0 );
				Commands->Send_Custom_Event ( obj, sender, M01_MODIFY_YOUR_ACTION_JDG, buggy_id, 0 );
			}
		}

		else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)
		{
			gdi_minigunner_02_inPlace = true;
			int minigunner02_id = Commands->Get_ID ( sender );

			GameObject *buggy = Commands->Find_Object ( buggy_id );
			if (buggy != NULL)
			{
				Commands->Send_Custom_Event ( obj, buggy, M01_MODIFY_YOUR_ACTION_03_JDG, minigunner02_id, 0 );
				Commands->Send_Custom_Event ( obj, sender, M01_MODIFY_YOUR_ACTION_JDG, buggy_id, 0 );
			}
		}
	}
};


DECLARE_SCRIPT(M01_BuggyScript_New_JDG, "")
{
	int minigunner01_id;
	int minigunner02_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(minigunner01_id, 1);
		SAVE_VARIABLE(minigunner02_id, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, true );
		ActionParamsStruct params;

		params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
		params.WaypathID = 103289;
		params.WaypointStartID = 103290;
		params.AttackActive = false;

		Commands->Action_Attack( obj, params );

		Commands->Send_Custom_Event ( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 2 );

		float myMaxHealth = Commands->Get_Max_Health ( obj );
		float myNewHealth = myMaxHealth / 2;
		Commands->Set_Health ( obj, myNewHealth );

		float myMaxShield = Commands->Get_Max_Shield_Strength ( obj );
		float myNewShield = myMaxShield / 2;
		Commands->Set_Shield_Strength ( obj, myNewShield );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Sound ( "EVA_Nod_Unit_Destroyed", Vector3 (0,0,0), obj);
		
		Vector3 buggyDeathSpot (-217.0f, 499.4f, 4.0f);
		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", buggyDeathSpot);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
			params.WaypathID = 103289;
			params.AttackActive = false;

			Commands->Action_Attack( obj, params );
			Commands->Send_Custom_Event ( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 1 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == M01_MODIFY_YOUR_ACTION_02_JDG)
		{
			minigunner01_id = param;
			Commands->Send_Custom_Event ( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 0 );
		}

		else if (type == M01_MODIFY_YOUR_ACTION_03_JDG)
		{
			minigunner02_id = param;
			Commands->Send_Custom_Event ( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 0 );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG)
		{
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
			params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
			params.WaypathID = 103289;
			params.Set_Attack( STAR, 50, 2, true );

			Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_01_JDG, params, false, true );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_04_JDG)
		{
			GameObject * minigunner01 = Commands->Find_Object ( minigunner01_id );
			GameObject * minigunner02 = Commands->Find_Object ( minigunner02_id );

			if (minigunner01 != NULL)
			{
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
				params.WaypathID = 103289;
				params.Set_Attack( minigunner01, 50, 2, true );

				Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_01_JDG, params, false, true );
			}

			else if (minigunner02 != NULL)
			{
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
				params.WaypathID = 103289;
				params.Set_Attack( minigunner02, 50, 2, true );

				Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_01_JDG, params, false, true );
			}

			else
			{
				ActionParamsStruct params;

				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(0,0,0), 0.5f, 5 );
				params.WaypathID = 103289;
				params.Set_Attack( STAR, 50, 2, true );

				Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_01_JDG, params, false, true );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Priest_Conversation_Zone_JDG, "")//this guys ID is 103395  M01_PriestReminder_Conversation
{
	bool okay_to_play_conversation;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_play_conversation, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okay_to_play_conversation = false;
	}

	void Custom( GameObject * /* obj */, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//priest is in position--give okay for conversation
		{
			okay_to_play_conversation = true;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (okay_to_play_conversation == true)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_PRIEST_CONVERSATION_JDG, 0 );
				Commands->Destroy_Object ( obj );//this is one-time only zone--cleaning up
			}
		}
	}
};

DECLARE_SCRIPT(M01_Loveshack_Nun_Conversation_Zone_JDG, "")//this guys ID is 101368
{
	bool okay_to_play_conversation;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_play_conversation, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okay_to_play_conversation = false;
	}

	void Custom( GameObject * /* obj */, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//nun is in position--give okay for conversation
		{
			okay_to_play_conversation = true;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (okay_to_play_conversation == true)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_LOVESHACK_NUN_CONVERSATION_JDG, 0 );
				Commands->Destroy_Object ( obj );//this is one-time only zone--cleaning up
			}
		}
	}
};

DECLARE_SCRIPT(M01_Interior_Nun_Conversation_Zone_JDG, "")//this guys ID is #101370
{
	bool okay_to_play_conversation;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okay_to_play_conversation, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		okay_to_play_conversation = false;
	}

	void Custom( GameObject * /* obj */, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if ((param == M01_START_ACTING_JDG))//nun is in position--give okay for conversation
		{
			okay_to_play_conversation = true;
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			if (okay_to_play_conversation == true)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_INTERIOR_NUN_CONVERSATION_JDG, 0 );
				Commands->Destroy_Object ( obj );//this is one-time only zone--cleaning up
			}
		}
	}
};

DECLARE_SCRIPT(M01_Church_Guard_MiniGunner_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			if (obj)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( STAR, WALK, 5 );
				params.Set_Attack( STAR, 15, 0, true );
				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CHURCH_GUARD_IS_DEAD_JDG, 0 );

		GameObject * havocExitZone = Commands->Find_Object ( 103393 );
		if (havocExitZone != NULL)
		{
			Commands->Send_Custom_Event( obj, havocExitZone, 0, M01_CHURCH_GUARD_IS_DEAD_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Church_Loveshack_InterrogationConv_Zone_JDG, "")//103392 
{
	bool conversation_triggered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(conversation_triggered, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		conversation_triggered = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && conversation_triggered == false)
		{
			conversation_triggered = true;
			GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID );
			GameObject * loveshackGuard = Commands->Find_Object ( M01_CHURCH_LOVESHACK_MINIGUNNER_ID );

			if ((loveshackNun != NULL) && (loveshackGuard != NULL))
			{
				int loveshackConv = Commands->Create_Conversation( "M01_Loveshack_Conversation_JDG", 90, 50, true);
				Commands->Join_Conversation( loveshackGuard, loveshackConv, false, true, true );
				Commands->Join_Conversation( loveshackNun, loveshackConv, false, true, true );
				Commands->Start_Conversation( loveshackConv,  loveshackConv );
			}

			Commands->Destroy_Object ( obj );//cleaning up this zone
		}
	}
};





DECLARE_SCRIPT(M01_GDI_Escort_Conversation_Controller_GDI, "")//103396 
{
	bool first_time_at_escort_limit;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(first_time_at_escort_limit, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		first_time_at_escort_limit = true;
	}

	void Custom( GameObject * /* obj */, int /* type */, intptr_t param, GameObject * sender ) override
	{
		if (param == M01_MODIFY_YOUR_ACTION_JDG)//gdi escort has been poked to follow player
		{
			int random = Commands->Get_Random_Int(0, 3);

			if (random == 0)
			{
				int pokeYesConv01 = Commands->Create_Conversation( "M01_GDI_Escort_Yes01", 90, 50, true);
				Commands->Join_Conversation( STAR, pokeYesConv01, false, false, false );
				Commands->Join_Conversation( sender, pokeYesConv01, false, false, false );
				Commands->Start_Conversation( pokeYesConv01,  pokeYesConv01 );
			}

			else if (random == 1)
			{
				int pokeYesConv01 = Commands->Create_Conversation( "M01_GDI_Escort_Yes02", 90, 50, true);
				Commands->Join_Conversation( STAR, pokeYesConv01, false, false, false );
				Commands->Join_Conversation( sender, pokeYesConv01, false, false, false );
				Commands->Start_Conversation( pokeYesConv01,  pokeYesConv01 );
			}

			else
			{
				int pokeYesConv01 = Commands->Create_Conversation( "M01_GDI_Escort_Yes03", 90, 50, true);
				Commands->Join_Conversation( STAR, pokeYesConv01, false, false, false );
				Commands->Join_Conversation( sender, pokeYesConv01, false, false, false );
				Commands->Start_Conversation( pokeYesConv01,  pokeYesConv01 );
			}
		}

		else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)//gdi escort has been poked to stop following player
		{
		}

		else if (param == M01_MODIFY_YOUR_ACTION_03_JDG)//gdi escort has reached his escort limits
		{
			if (first_time_at_escort_limit == true)
			{
				first_time_at_escort_limit = false;
				int pokeLimitConv01 = Commands->Create_Conversation( "M01_GDI_Escort_MustStayHere01", 90, 50, true);
				Commands->Join_Conversation( sender, pokeLimitConv01, false, false, false );
				Commands->Join_Conversation( STAR, pokeLimitConv01, false, false, false );
				Commands->Start_Conversation( pokeLimitConv01, pokeLimitConv01 );
			}

			else
			{
				int pokeLimitConv01 = Commands->Create_Conversation( "M01_GDI_Escort_MustStayHere02", 90, 50, true);
				Commands->Join_Conversation( sender, pokeLimitConv01, false, false, false );
				Commands->Join_Conversation( STAR, pokeLimitConv01, false, false, false );
				Commands->Start_Conversation( pokeLimitConv01, pokeLimitConv01 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_CHURCH_Chinook_Spawned_Soldier01_GDI, "")
{
	typedef enum {
		ESCORTING,
		NOT_ESCORTING,
		
	} M01_State;

	M01_State thisGuysState;

	int initialEscortConv;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(thisGuysState, 1);
		SAVE_VARIABLE(initialEscortConv, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		thisGuysState = NOT_ESCORTING;
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_GDI_GUY_HAS_BEEN_POKED_JDG, 0 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (thisGuysState == ESCORTING)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement(STAR, RUN, 5);
			Commands->Action_Goto(obj, params);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)
			{
				if (obj && STAR)
				{
					initialEscortConv = Commands->Create_Conversation( "M01_FirstEscort_Conversation", 100, 50, true);
					Commands->Join_Conversation( obj, initialEscortConv, true, true, true );
					Commands->Join_Conversation( STAR, initialEscortConv, true, true, true );
					Commands->Start_Conversation( initialEscortConv,  initialEscortConv );
					Commands->Monitor_Conversation (obj, initialEscortConv);
				}
			}

			if (action_id == M01_GOING_TO_HAVOC_JDG && thisGuysState == ESCORTING)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( STAR, RUN + 0.2f, 2 );
				Commands->Action_Attack (obj, params);
			}
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == initialEscortConv)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 0 );
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		
		if ((param == M01_PICK_A_NEW_LOCATION_JDG) && (thisGuysState != ESCORTING) )
		{
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 6.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-139.048f, 318.861f, 9.066f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-81.160f, 333.117f, 10.131f);
			}

			else if (pick_my_home_spot >= 2.5 && pick_my_home_spot < 3.5)
			{
				homeSpot.Set (-138.786f, 388.250f, 15.093f);
			}

			else if (pick_my_home_spot >= 3.5 && pick_my_home_spot < 4.5)
			{
				homeSpot.Set (-140.785f, 348.389f, 21.931f);
			}

			else if (pick_my_home_spot >= 4.5 && pick_my_home_spot < 5.5)
			{
				homeSpot.Set (-140.792f, 342.980f, 10.387f);
			}

			else 
			{
				homeSpot.Set (-126.169f, 335.223f, 8.997f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 10 );

			params.Set_Basic(this, 50, M01_PICK_A_NEW_LOCATION_JDG);
			params.Set_Movement(homeSpot, RUN, 3);
			Commands->Action_Goto(obj, params);

			float delayTimer = Commands->Get_Random ( 10, 60 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
		}

		else if (param == M01_PLEASE_START_ESCORTING_PLAYER_JDG)
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( STAR, RUN + 0.2f, 2 );
			Commands->Action_Attack (obj, params);

			thisGuysState = ESCORTING;
		}

		else if (param == M01_PLEASE_STOP_ESCORTING_PLAYER_JDG)
		{
			Commands->Action_Reset ( obj, 100 );
			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

			thisGuysState = NOT_ESCORTING;
		}

		else if ((type == M01_PLAYER_IS_ATTACKING_ME_JDG) && (thisGuysState == ESCORTING) )
		{
			GameObject * myTarget = Commands->Find_Object ( param );
			if (myTarget != NULL)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( myTarget, RUN, 5 );
				params.AttackActive	= true;
				params.Set_Attack (myTarget, 50, 0, true);	
				
				Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
			}
		}

		else if ((param == M01_PLAYER_IS_LEAVING_CHURCH_01_JDG) && (thisGuysState == ESCORTING))
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
			}
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
			thisGuysState = NOT_ESCORTING;
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 3.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-160.217f, 446.353f, 34.976f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-173.371f, 437.383f, 29.711f);
			}

			else 
			{
				homeSpot.Set (-138.958f, 435.696f, 27.001f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 3 );

			params.Set_Basic(this, 50, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement(homeSpot, WALK, 3);
			Commands->Action_Goto(obj, params);

			
		}

		else if ((param == M01_PLAYER_IS_LEAVING_CHURCH_02_JDG) && (thisGuysState == ESCORTING))
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
			}
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
			thisGuysState = NOT_ESCORTING;
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 3.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-36.192f, 319.671f, 5.226f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-49.614f, 363.304f, 10.816f);
			}

			else 
			{
				homeSpot.Set (-66.361f, 339.288f, 8.772f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 3 );

			params.Set_Basic(this, 50, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement(homeSpot, WALK, 3);
			Commands->Action_Goto(obj, params);
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		ActionParamsStruct params;

		if (thisGuysState == ESCORTING)
		{
			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( enemy, RUN, 5 );
			params.Set_Attack( enemy, 50, 0, true );
			params.AttackActive	= true;
			params.AttackCheckBlocked = true;
			Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		ActionParamsStruct params;

		if (damager != NULL)
		{
			int attackersType = Commands->Get_Player_Type ( damager );

			if (attackersType != SCRIPT_PLAYERTYPE_NOD)
			{
			}

			else
			{
				if (thisGuysState == ESCORTING && damager != NULL)
				{
					params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
					params.Set_Movement( damager, RUN, 5 );
					params.Set_Attack( damager, 50, 0, true );
					params.AttackActive	= true;
					params.AttackCheckBlocked = true;
					Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
				}
			}
		}	
	}
};

DECLARE_SCRIPT(M01_CHURCH_Chinook_Spawned_Soldier02_GDI, "")
{
	typedef enum {
		ESCORTING,
		NOT_ESCORTING,
		
	} M01_State;

	M01_State thisGuysState;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(thisGuysState, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		thisGuysState = NOT_ESCORTING;
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_GDI_GUY_HAS_BEEN_POKED_JDG, 0 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (thisGuysState == ESCORTING)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 0 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_GOING_TO_HAVOC_JDG && thisGuysState == ESCORTING)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( STAR, RUN + 0.2f, 2 );
				Commands->Action_Attack (obj, params);
			}
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		
		if ((param == M01_PICK_A_NEW_LOCATION_JDG) && (thisGuysState != ESCORTING) )
		{
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 6.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-139.048f, 318.861f, 9.066f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-81.160f, 333.117f, 10.131f);
			}

			else if (pick_my_home_spot >= 2.5 && pick_my_home_spot < 3.5)
			{
				homeSpot.Set (-138.786f, 388.250f, 15.093f);
			}

			else if (pick_my_home_spot >= 3.5 && pick_my_home_spot < 4.5)
			{
				homeSpot.Set (-140.785f, 348.389f, 21.931f);
			}

			else if (pick_my_home_spot >= 4.5 && pick_my_home_spot < 5.5)
			{
				homeSpot.Set (-140.792f, 342.980f, 10.387f);
			}

			else 
			{
				homeSpot.Set (-126.169f, 335.223f, 8.997f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 10 );

			params.Set_Basic(this, 50, M01_PICK_A_NEW_LOCATION_JDG);
			params.Set_Movement(homeSpot, RUN, 3);
			Commands->Action_Goto(obj, params);

			float delayTimer = Commands->Get_Random ( 10, 60 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
		}

		else if (param == M01_PLEASE_START_ESCORTING_PLAYER_JDG)
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( STAR, RUN + 0.2f, 2 );
			Commands->Action_Attack (obj, params);

			thisGuysState = ESCORTING;
		}

		else if (param == M01_PLEASE_STOP_ESCORTING_PLAYER_JDG)
		{
			Commands->Action_Reset ( obj, 100 );
			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

			thisGuysState = NOT_ESCORTING;
		}

		else if ((type == M01_PLAYER_IS_ATTACKING_ME_JDG) && (thisGuysState == ESCORTING) )
		{
			GameObject * myTarget = Commands->Find_Object ( param );
			if (myTarget != NULL)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( myTarget, RUN, 5 );
				params.AttackActive	= true;
				params.Set_Attack (myTarget, 50, 0, true);	
				
				Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
			}
		}

		else if ((param == M01_PLAYER_IS_LEAVING_CHURCH_01_JDG) && (thisGuysState == ESCORTING))
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
			}

			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
			thisGuysState = NOT_ESCORTING;
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 3.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-160.217f, 446.353f, 34.976f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-173.371f, 437.383f, 29.711f);
			}

			else 
			{
				homeSpot.Set (-138.958f, 435.696f, 27.001f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 3 );

			params.Set_Basic(this, 50, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement(homeSpot, WALK, 3);
			Commands->Action_Goto(obj, params);
		}

		else if ((param == M01_PLAYER_IS_LEAVING_CHURCH_02_JDG) && (thisGuysState == ESCORTING))
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
			}

			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
			thisGuysState = NOT_ESCORTING;
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 3.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-36.192f, 319.671f, 5.226f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-49.614f, 363.304f, 10.816f);
			}

			else 
			{
				homeSpot.Set (-66.361f, 339.288f, 8.772f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 3 );

			params.Set_Basic(this, 50, M01_WALKING_WAYPATH_02_JDG);
			params.Set_Movement(homeSpot, WALK, 3);
			Commands->Action_Goto(obj, params);
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		ActionParamsStruct params;

		if (thisGuysState == ESCORTING)
		{
			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( enemy, RUN, 5 );
			params.Set_Attack( enemy, 50, 0, true );
			params.AttackActive	= true;
			params.AttackCheckBlocked = true;
			Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		ActionParamsStruct params;

		if (damager != NULL)
		{
			int attackersType = Commands->Get_Player_Type ( damager );

			if (attackersType != SCRIPT_PLAYERTYPE_NOD)
			{
			}

			else
			{
				if (thisGuysState == ESCORTING && damager != NULL)
				{
					params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
					params.Set_Movement( damager, RUN, 5 );
					params.Set_Attack( damager, 50, 0, true );
					params.AttackActive	= true;
					params.AttackCheckBlocked = true;
					Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
				}
			}
		}	
	}
};

DECLARE_SCRIPT(M01_COMM_Chinook_Spawned_Soldier_GDI, "")
{
	typedef enum {
		ESCORTING,
		NOT_ESCORTING,
		
	} M01_State;

	M01_State thisGuysState;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(thisGuysState, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		thisGuysState = NOT_ESCORTING;
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_GDI_GUY_HAS_BEEN_POKED_JDG, 0 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (thisGuysState == ESCORTING)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYERS_ESCORT_HAS_BEEN_KILLED_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 0 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_GOING_TO_HAVOC_JDG && thisGuysState == ESCORTING)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( STAR, RUN + 0.2f, 2 );
				Commands->Action_Attack (obj, params);
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		
		if ((param == M01_PICK_A_NEW_LOCATION_JDG) && (thisGuysState == NOT_ESCORTING) )
		{
			Vector3 homeSpot;
			Commands->Action_Reset ( obj, 100 );
			float pick_my_home_spot = Commands->Get_Random ( 0.5f, 6.5f );

			if (pick_my_home_spot >= 0.5 && pick_my_home_spot < 1.5)
			{
				homeSpot.Set (-277.309f, 468.428f, 18.553f);
			}

			else if (pick_my_home_spot >= 1.5 && pick_my_home_spot < 2.5)
			{
				homeSpot.Set (-271.237f, 535.962f, 27.935f);
			}

			else if (pick_my_home_spot >= 2.5 && pick_my_home_spot < 3.5)
			{
				homeSpot.Set (-247.278f, 563.557f, 30.610f);
			}

			else if (pick_my_home_spot >= 3.5 && pick_my_home_spot < 4.5)
			{
				homeSpot.Set (-296.349f, 583.760f, 27.739f);
			}

			else if (pick_my_home_spot >= 4.5 && pick_my_home_spot < 5.5)
			{
				homeSpot.Set (-313.597f, 568.022f, 27.740f);
			}

			else 
			{
				homeSpot.Set (-301.160f, 523.405f, 16.583f);
			}

			Commands->Set_Innate_Soldier_Home_Location ( obj, homeSpot, 10 );

			params.Set_Basic(this, 50, 20);
			params.Set_Movement(homeSpot, RUN, 3);
			Commands->Action_Goto(obj, params);

			float delayTimer = Commands->Get_Random ( 10, 60 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, delayTimer );
		}

		else if (param == M01_PLEASE_START_ESCORTING_PLAYER_JDG)
		{
			GameObject * escortDialogController = Commands->Find_Object ( 103396 );
			if (escortDialogController != NULL)
			{
				Commands->Send_Custom_Event( obj, escortDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( STAR, RUN + 0.2f, 2 );
			Commands->Action_Attack (obj, params);

			thisGuysState = ESCORTING;
		}

		else if (param == M01_PLEASE_STOP_ESCORTING_PLAYER_JDG)
		{
			Commands->Action_Reset ( obj, 100 );
			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

			thisGuysState = NOT_ESCORTING;
		}

		else if ((type == M01_PLAYER_IS_ATTACKING_ME_JDG) && (thisGuysState == ESCORTING) )
		{
			GameObject * myTarget = Commands->Find_Object ( param );
			if (myTarget != NULL)
			{
				params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
				params.Set_Movement( myTarget, RUN, 5 );
				params.AttackActive	= true;
				params.Set_Attack (myTarget, 50, 0, true);	
				
				Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );

				Commands->Debug_Message ( "**********************Player's escort has received orders to attack havocs target\n" );
			}
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		ActionParamsStruct params;

		if (thisGuysState == ESCORTING)
		{
			params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
			params.Set_Movement( enemy, RUN, 5 );
			params.Set_Attack( enemy, 50, 0, true );
			params.AttackActive	= true;
			params.AttackCheckBlocked = true;
			Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		ActionParamsStruct params;

		if (damager != NULL)
		{
			int attackersType = Commands->Get_Player_Type ( damager );

			if (attackersType != SCRIPT_PLAYERTYPE_NOD)
			{
			}

			else
			{
				if (thisGuysState == ESCORTING && damager != NULL)
				{
					params.Set_Basic( this, 100, M01_GOING_TO_HAVOC_JDG );
					params.Set_Movement( damager, RUN, 5 );
					params.Set_Attack( damager, 50, 0, true );
					params.AttackActive	= true;
					params.AttackCheckBlocked = true;
					Commands->Modify_Action ( obj, M01_GOING_TO_HAVOC_JDG, params, true, true );
				}
			}
		}	
	}
};














DECLARE_SCRIPT(M01_Barn_Truck_JDG, "")//
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Create_Explosion ( "Explosion_Vehicle", myPosition, NULL );
	}
};

DECLARE_SCRIPT(M01_GDIBase_EvacMonitor_JDG, "")//M01_GDIBASE_EVAC_MONITOR_JDG
{
	int unitsEvacuated;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(unitsEvacuated, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		unitsEvacuated = 0;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_GOING_TO_EVAC_SPOT_JDG)
		{
			unitsEvacuated++;

			if (unitsEvacuated == 4)
			{
				int airdrop_conv = Commands->Create_Conversation( "M01_AirDrop_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, airdrop_conv, false, false, true );
				Commands->Start_Conversation( airdrop_conv,  airdrop_conv );

				Vector3 airDropSpot (-32.247f, 36.075f, 0.547f);
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", airDropSpot);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");

				Commands->Destroy_Object ( obj );
			}
		}
	}
};



DECLARE_SCRIPT(M01_ChurchArea_EvacMonitor_JDG, "")//M01_CHURCHAREA_EVAC_MONITOR_JDG
{
	int unitsEvacuated;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(unitsEvacuated, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		unitsEvacuated = 0;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == M01_GOING_TO_EVAC_SPOT_JDG)
		{
			unitsEvacuated++;

			if (unitsEvacuated == 3)
			{
				int airdrop_conv = Commands->Create_Conversation( "M01_AirDrop_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, airdrop_conv, false, false, true );
				Commands->Start_Conversation( airdrop_conv,  airdrop_conv );

				Vector3 airDropSpot (-158.807f, 351.532f, 10.353f);
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", airDropSpot);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");

				Commands->Destroy_Object ( obj );
			}
		}
	}
};

DECLARE_SCRIPT(M01_MovieProjector_JDG, "")//103942
{//
	bool destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(destroyed, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		destroyed = false;
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR && destroyed == false)
		{
			destroyed = true;
			Commands->Set_Animation ( obj, "DSP_PROJECTOR.DSP_PROJECTOR", false, NULL, 0, 9, false );

			GameObject * propagandaController = Commands->Find_Object ( 103099 );
			if (propagandaController != NULL)
			{
				Commands->Send_Custom_Event( obj, propagandaController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
			}

			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Create_Explosion ( "Explosion_Barrel_Fuel", myPosition, NULL );
		}

		Commands->Set_Health ( obj, 0.25f );
	}
};

DECLARE_SCRIPT(M01_Propaganda_Sounds_Controller_JDG, "")//this guys ID is 103099
{
	int last;
	int displayScreen_ID;
	int monitor_sound01;
	int monitor_sound02;
	int monitor_sound03;
	bool destroyed;
	int propaganda_number;
	bool soundPlaying;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
		SAVE_VARIABLE(displayScreen_ID, 2);
		SAVE_VARIABLE(monitor_sound01, 3);
		SAVE_VARIABLE(destroyed, 4);
		SAVE_VARIABLE(propaganda_number, 5);
		SAVE_VARIABLE(soundPlaying, 6);
		SAVE_VARIABLE(monitor_sound02, 7);
		SAVE_VARIABLE(monitor_sound03, 8);
	}

	void Created(GameObject * /* obj */) override
	{	
		last = 0;
		displayScreen_ID = 0;
		monitor_sound01 = 0;
		monitor_sound02 = 0;
		monitor_sound03 = 0;
		destroyed = false;
		propaganda_number = 0;
		soundPlaying = false;
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 0)
		{
			if (obj && param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				soundPlaying = false;
				GameObject * oldDisplayScreen = Commands->Find_Object ( displayScreen_ID);
				if (oldDisplayScreen != NULL)
				{
					Commands->Destroy_Object ( oldDisplayScreen );
				}
				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 2 );
			}
		
		//if (obj && type == CUSTOM_EVENT_SOUND_ENDED  )
		//{
		//	if (obj && param == monitor_sound01)
			//{
				
			//}
		//}

		
			else if (param == M01_START_ACTING_JDG && destroyed == false)
			{
				static constexpr const char* propaganda_screen[10] =
				{
					"M01_Propaganda_Screen_01",
					"M01_Propaganda_Screen_02",
					"M01_Propaganda_Screen_03",
					"M01_Propaganda_Screen_04",
					"M01_Propaganda_Screen_05",
					"M01_Propaganda_Screen_06",
					"M01_Propaganda_Screen_07",
					"M01_Propaganda_Screen_08",
					"M01_Propaganda_Screen_10",
					"M01_Propaganda_Screen_11",
				};

				static constexpr const char* propaganda_sound[10] =
				{
					"M01NCXS_DSGN0270I1NCXS_SND",
					"M01NCXS_DSGN0271I1NCXS_SND",
					"M01NCXS_DSGN0272I1NCXS_SND",
					"M01NCXS_DSGN0273I1NCXS_SND",
					"M01NCXS_DSGN0279I1NCXS_SND",
					"M01NCXS_DSGN0275I1NCXS_SND",
					"M01NCXS_DSGN0276I1NCXS_SND",
					"M01NCXS_DSGN0277I1NCXS_SND",
					"M01NCXS_DSGN0278I1NCXS_SND",
					"M01NCXS_DSGN0274I1NCXS_SND",
				};

				Vector3 propagandaSpeaker01 (-319.283f, 589.265f, 35.385f);
				Vector3 propagandaSpeaker02 (-277.730f, 581.935f, 35.724f);
				Vector3 propagandaSpeaker03 (-296.269f, 612.473f, 35.468f);

				propaganda_number++;// = Commands->Get_Random_Int(0, 11);

				if (propaganda_number >= 10)
				{
					propaganda_number = 0;
				}

				GameObject * displayScreen = Commands->Create_Object ( propaganda_screen[propaganda_number], Vector3 (0,0,0) );
				displayScreen_ID = Commands->Get_ID ( displayScreen );

				monitor_sound03 = Commands->Create_Sound ( propaganda_sound[propaganda_number], propagandaSpeaker01, obj );
				monitor_sound02 = Commands->Create_Sound ( propaganda_sound[propaganda_number], propagandaSpeaker02, obj );
				monitor_sound01 = Commands->Create_Sound ( propaganda_sound[propaganda_number], propagandaSpeaker03, obj );
				//Commands->Monitor_Sound ( obj, monitor_sound01 );
				soundPlaying = true;

				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 10 );
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)//projector is dead--stop playing propaganda stuff
			{
				destroyed = true;
				GameObject * oldDisplayScreen = Commands->Find_Object ( displayScreen_ID);
				if (oldDisplayScreen != NULL)
				{
					Commands->Destroy_Object ( oldDisplayScreen );
				}

				if (soundPlaying == true)
				{
					Commands->Stop_Sound ( monitor_sound01, true );
					Commands->Stop_Sound ( monitor_sound02, true );
					Commands->Stop_Sound ( monitor_sound03, true );
				}

				Commands->Destroy_Object ( obj );
			}
		}
	}
};

DECLARE_SCRIPT(M01_BackPath_NodGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Innate_Is_Stationary ( obj, true );
		GameObject * gdiBaseCommander = Commands->Find_Object ( 106050 );
		if (gdiBaseCommander != NULL)
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 75, M01_START_ACTING_JDG );
			params.Set_Attack( gdiBaseCommander, 30, 5, true );
			params.AttackCheckBlocked = false;

			Commands->Action_Attack ( obj, params );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Repair_Engineer_JDG, "")//this guys ID is M01_COMM_CENTER_ENGINEER_ID 101949
{
	int healer_sound;
	bool healing;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(healer_sound, 1);
		SAVE_VARIABLE(healing, 2);
	}

	void Created( GameObject * obj ) override 
	{ 
		Commands->Set_Innate_Is_Stationary ( obj, true );
		healing = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == CUSTOM_EVENT_SOUND_ENDED  )
		{
			if (param == healer_sound)
			{
				healing = false;
			}
		}

		else if (type == 0)
		{
			if (param == M01_PLAYER_IS_ATTACKING_ME_JDG && healing == false)
			{
				healing = true;
				Commands->Enable_Hibernation( obj, false );
				Vector3 myRepairSpot (-301.382f, 551.366f, 18.205f);
				ActionParamsStruct params;
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( myRepairSpot, 30, 0, false );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );

				Commands->Start_Timer ( obj, this, 2, M01_START_ACTING_JDG );
			}
		}
	}

	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		if (timer_id == M01_START_ACTING_JDG)
		{
			Commands->Enable_Hibernation( obj, true );
			
			Commands->Action_Reset ( obj, 100 );

			GameObject * commCenter = Commands->Find_Object ( M01_COMM_CENTER_JDG );
			Vector3 repair_sound_spot (-297.633f, 532.144f, 35.458f);
			float commCenter_max_health = Commands->Get_Max_Health ( commCenter );;
			float commCenter_max_shield = Commands->Get_Max_Shield_Strength ( commCenter );

			float commCenter_current_health = Commands->Get_Health ( commCenter );

			if (commCenter_current_health > 0)
			{
				Commands->Set_Health ( commCenter, commCenter_max_health );
				Commands->Set_Shield_Strength ( commCenter, commCenter_max_shield );

				healer_sound = Commands->Create_Sound ( "CnC_Healer_Sound", repair_sound_spot, obj );
				Commands->Monitor_Sound ( obj, healer_sound );
				
			}
		}
	}
};


DECLARE_SCRIPT(M01_Comm_Center_Building_Script_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG, 0 );
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_AMBIENT_SOUND_CONTROLLER_JDG ), 0, M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG, 0 );

		GameObject * mainframe = Commands->Find_Object ( 107809 );
		if (mainframe != NULL)
		{
			Commands->Send_Custom_Event( obj, mainframe, 0, M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG, 0 );
		}

		GameObject * gateTutorialZone = Commands->Find_Object ( 120844 );
		if (gateTutorialZone != NULL)
		{
			Commands->Send_Custom_Event( obj, gateTutorialZone, 0, M01_START_ACTING_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		GameObject * myEngineer = Commands->Find_Object ( M01_COMM_CENTER_ENGINEER_ID );
		if (myEngineer != NULL)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_DO_DAMAGE_CHECK_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_PaintballRoom_ChatterController_JDG, "")//107554
{
	int nodguy01_id;//
	int nodguy02_id;
	int nodguy03_id;
	int nodguy04_id;
	int nodguy05_id;
	int nodguy06_id;

	bool nodguy01_dead;
	bool nodguy02_dead;
	bool nodguy03_dead;
	bool nodguy04_dead;
	bool nodguy05_dead;
	bool nodguy06_dead;

	bool acting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(nodguy01_id, 1);
		SAVE_VARIABLE(nodguy02_id, 2);
		SAVE_VARIABLE(nodguy03_id, 3);
		SAVE_VARIABLE(nodguy04_id, 4);
		SAVE_VARIABLE(nodguy05_id, 5);
		SAVE_VARIABLE(nodguy06_id, 6);
		SAVE_VARIABLE(nodguy01_dead, 7);
		SAVE_VARIABLE(nodguy02_dead, 8);
		SAVE_VARIABLE(nodguy03_dead, 9);
		SAVE_VARIABLE(nodguy04_dead, 10);
		SAVE_VARIABLE(nodguy05_dead, 11);
		SAVE_VARIABLE(nodguy06_dead, 12);
		SAVE_VARIABLE(acting, 13);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		nodguy01_id = 102001;
		nodguy02_id = 101998;
		nodguy03_id = 102000;
		nodguy04_id = 101999;
		nodguy05_id = 102002;
		nodguy06_id = 102003;
		nodguy01_dead = false;
		nodguy02_dead = false;
		nodguy03_dead = false;
		nodguy04_dead = false;
		nodguy05_dead = false;
		nodguy06_dead = false;
		acting = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG && acting == false)
			{
				acting = true;
				if (nodguy01_dead == false)
				{
					float delayTimer = Commands->Get_Random ( 1, 2 );
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
				}

				else if (nodguy01_dead == true)
				{
					if (nodguy02_dead == false)
					{
						float delayTimer = Commands->Get_Random ( 1, 2 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer );
					}
				}

				if (nodguy03_dead == false)
				{
					float delayTimer = Commands->Get_Random ( 1, 2 );
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, delayTimer );
				}

				else if (nodguy03_dead == true)
				{
					if (nodguy04_dead == false)
					{
						float delayTimer = Commands->Get_Random ( 1, 2 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, delayTimer );
					}
				}

				if (nodguy05_dead == false)
				{
					float delayTimer = Commands->Get_Random ( 1, 2 );
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_05_JDG, delayTimer );
				}

				else if (nodguy05_dead == true)
				{
					if (nodguy06_dead == false)
					{
						float delayTimer = Commands->Get_Random ( 1, 2 );
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_06_JDG, delayTimer );
					}
				}
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				GameObject * nodguy01 = Commands->Find_Object ( nodguy01_id );
				GameObject * nodguy02 = Commands->Find_Object ( nodguy02_id );
				GameObject * nodguy03 = Commands->Find_Object ( nodguy03_id );
				GameObject * nodguy04 = Commands->Find_Object ( nodguy04_id );
				GameObject * nodguy05 = Commands->Find_Object ( nodguy05_id );
				GameObject * nodguy06 = Commands->Find_Object ( nodguy06_id );

				if (sender == nodguy01)
				{
					nodguy01_dead = true;
				}

				else if (sender == nodguy02)
				{
					nodguy02_dead = true;
				}

				else if (sender == nodguy03)
				{
					nodguy03_dead = true;
				}

				else if (sender == nodguy04)
				{
					nodguy04_dead = true;
				}

				else if (sender == nodguy05)
				{
					nodguy05_dead = true;
				}

				else if (sender == nodguy06)
				{
					nodguy06_dead = true;
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				GameObject * nodguy01 = Commands->Find_Object ( nodguy01_id );
				if (nodguy01 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_02", 95, 20, true);
					Commands->Join_Conversation( nodguy01, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)
			{
				GameObject * nodguy02 = Commands->Find_Object ( nodguy02_id );
				if (nodguy02 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_02", 95, 20, true);
					Commands->Join_Conversation( nodguy02, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_03_JDG)
			{
				GameObject * nodguy03 = Commands->Find_Object ( nodguy03_id );
				if (nodguy03 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_01", 95, 20, true);
					Commands->Join_Conversation( nodguy03, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_04_JDG)
			{
				GameObject * nodguy04 = Commands->Find_Object ( nodguy04_id );
				if (nodguy04 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_01", 95, 20, true);
					Commands->Join_Conversation( nodguy04, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_05_JDG)
			{
				GameObject * nodguy05 = Commands->Find_Object ( nodguy05_id );
				if (nodguy05 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_03", 95, 20, true);
					Commands->Join_Conversation( nodguy05, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_06_JDG)
			{
				GameObject * nodguy06 = Commands->Find_Object ( nodguy06_id );
				if (nodguy06 != NULL)
				{
					int paintball_alert_conv = Commands->Create_Conversation( "M01_Paintball_Intruder_Conversation_03", 95, 20, true);
					Commands->Join_Conversation( nodguy06, paintball_alert_conv, true, true, true);
					Commands->Start_Conversation( paintball_alert_conv,  paintball_alert_conv );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_Paintball_Team_01_JDG, "")//this is the grunt team
{
	int enemySniperId;
	float myAggressiveness;
	float myCoverProbability;
	float myTimerMax;
	float myTimerMin;
	float myHomeRange;
	Vector3 myPosition;
	bool retreating;
	bool starEncountered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(enemySniperId, 1);
		SAVE_VARIABLE(myAggressiveness, 2);
		SAVE_VARIABLE(myCoverProbability, 3);
		SAVE_VARIABLE(myTimerMax, 4);
		SAVE_VARIABLE(myTimerMin, 5);
		SAVE_VARIABLE(myHomeRange, 6);
		SAVE_VARIABLE(myPosition, 7);
		SAVE_VARIABLE(retreating, 8);
		SAVE_VARIABLE(starEncountered, 9);
	}

	void Created( GameObject * obj ) override 
	{
		enemySniperId = 102001;
		retreating = false;
		starEncountered = false;
		int myIdNumber = Commands->Get_ID ( obj );
		Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_RENEGADE );
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_NOD );

		if (myIdNumber == 101998)//team one sniper
		{
		}

		else
		{
			myPosition = Commands->Get_Position ( obj );
			myTimerMin = 5;
			myTimerMax = 10;

			myAggressiveness = Commands->Get_Random ( 0, 0.5f );
			myCoverProbability = Commands->Get_Random ( 0.5f, 1 );
			myHomeRange = Commands->Get_Random ( 0, 20 );

			Commands->Set_Innate_Aggressiveness ( obj, myAggressiveness );
			Commands->Set_Innate_Take_Cover_Probability ( obj, myCoverProbability );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, myHomeRange );

			float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
			Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * convController = Commands->Find_Object ( 107554 );
		if (convController != NULL)
		{
			Commands->Send_Custom_Event( obj, convController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			starEncountered = true;

			GameObject * convController = Commands->Find_Object ( 107554 );
			if (convController != NULL)
			{
				Commands->Send_Custom_Event( obj, convController, 0, M01_START_ACTING_JDG, 0 );
			}
		}

		else if (damager == Commands->Find_Object ( enemySniperId ))
		{
			retreating = true;

			Commands->Set_Innate_Aggressiveness ( obj, 0.2f );
			Commands->Set_Innate_Take_Cover_Probability ( obj, 1 );

			float delayTimer = Commands->Get_Random ( 0, 2 );
			Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_02_JDG );

			if (starEncountered == false)
			{
				float myMaxHealth = Commands->Get_Max_Health ( obj );
				Commands->Set_Health (  obj, myMaxHealth );
			}
		}

		else 
		{
			if (starEncountered == false)
			{
				float myMaxHealth = Commands->Get_Max_Health ( obj );
				Commands->Set_Health (  obj, myMaxHealth );
			}
		}
	}

	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		switch (timer_id)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					if (retreating == false)
					{
						myAggressiveness = Commands->Get_Random ( 0, 0.5f );
						myCoverProbability = Commands->Get_Random ( 0.5f, 1 );
						myHomeRange = Commands->Get_Random ( 0, 20 );

						Commands->Set_Innate_Aggressiveness ( obj, myAggressiveness );
						Commands->Set_Innate_Take_Cover_Probability ( obj, myCoverProbability );
						Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, myHomeRange );

						float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
						Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
					}
				}
				break;

			case M01_MODIFY_YOUR_ACTION_02_JDG: 
				{
					retreating = false;

					float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
					Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
				}
				break;
		}
		
	}
};

DECLARE_SCRIPT(M01_HON_Paintball_Team_02_JDG, "")//this is the officer team
{
	int enemySniperId;
	float myAggressiveness;
	float myCoverProbability;
	float myTimerMax;
	float myTimerMin;
	float myHomeRange;
	Vector3 myPosition;
	bool retreating;
	bool starEncountered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(enemySniperId, 1);
		SAVE_VARIABLE(myAggressiveness, 2);
		SAVE_VARIABLE(myCoverProbability, 3);
		SAVE_VARIABLE(myTimerMax, 4);
		SAVE_VARIABLE(myTimerMin, 5);
		SAVE_VARIABLE(myHomeRange, 6);
		SAVE_VARIABLE(myPosition, 7);
		SAVE_VARIABLE(retreating, 8);
		SAVE_VARIABLE(starEncountered, 9);
	}

	void Created( GameObject * obj ) override 
	{
		enemySniperId= 101998;
		retreating = false;
		starEncountered = false;
		int myIdNumber = Commands->Get_ID ( obj );

		if (myIdNumber == 102001)//team two sniper
		{
		}

		else
		{
			myPosition = Commands->Get_Position ( obj );
			myTimerMin = 5;
			myTimerMax = 10;

			myAggressiveness = Commands->Get_Random ( 0.75f, 1 );
			myCoverProbability = Commands->Get_Random ( 0, 0.25f );
			myHomeRange = Commands->Get_Random ( 0, 20 );

			Commands->Set_Innate_Aggressiveness ( obj, myAggressiveness );
			Commands->Set_Innate_Take_Cover_Probability ( obj, myCoverProbability );
			Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, myHomeRange );

			float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
			Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
		}
	}

	void Enemy_Seen( GameObject * /* obj */, GameObject * enemy ) override
	{
		if (enemy == STAR)
		{
			starEncountered = true;
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * convController = Commands->Find_Object ( 107554 );
		if (convController != NULL)
		{
			Commands->Send_Custom_Event( obj, convController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			starEncountered = true;

			GameObject * convController = Commands->Find_Object ( 107554 );
			if (convController != NULL)
			{
				Commands->Send_Custom_Event( obj, convController, 0, M01_START_ACTING_JDG, 0 );
			}
		}

		else if (damager == Commands->Find_Object ( enemySniperId ))
		{
			retreating = true;

			Commands->Set_Innate_Aggressiveness ( obj, 0.2f );
			Commands->Set_Innate_Take_Cover_Probability ( obj, 1.0f );

			float delayTimer = Commands->Get_Random ( 1, 3 );
			Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_02_JDG );

			if (starEncountered == false)
			{
				float myMaxHealth = Commands->Get_Max_Health ( obj );
				Commands->Set_Health (  obj, myMaxHealth );
			}
		}

		else
		{
			if (starEncountered == false)
			{
				float myMaxHealth = Commands->Get_Max_Health ( obj );
				Commands->Set_Health (  obj, myMaxHealth );
			}
		}
	}

	void Timer_Expired( GameObject * obj, int timer_id ) override
	{
		switch (timer_id)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					if (retreating == false)
					{
						myAggressiveness = Commands->Get_Random ( 0.75f, 1 );
						myCoverProbability = Commands->Get_Random ( 0, 0.25f );
						myHomeRange = Commands->Get_Random ( 0, 20 );

						Commands->Set_Innate_Aggressiveness ( obj, myAggressiveness );
						Commands->Set_Innate_Take_Cover_Probability ( obj, myCoverProbability );
						Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, myHomeRange );

						float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
						Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
					}
				}
				break;

			case M01_MODIFY_YOUR_ACTION_02_JDG: 
				{
					retreating = false;

					float delayTimer = Commands->Get_Random ( myTimerMin, myTimerMax );
					Commands->Start_Timer ( obj, this, delayTimer, M01_MODIFY_YOUR_ACTION_JDG );
				}
				break;
		}
		
	}
};

DECLARE_SCRIPT(M01_HON_Dojo_Trainer_JDG, "")//this guys ID is M01_HON_DOJO_SENSEI_JDG 102051
{
	int dojo_conv;
	bool first_time;
	bool handOfNodDead;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(dojo_conv, 1);
		SAVE_VARIABLE(first_time, 2);
		SAVE_VARIABLE(handOfNodDead, 3);
	}

	void Created( GameObject * obj ) override 
	{
		handOfNodDead = false;
		first_time = true;
		Commands->Innate_Disable(obj);
		//Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (handOfNodDead == false)
		{
			if (Commands->Find_Object (M01_HON_DOJO_CIV_01_JDG))
			{
				float delayTimer = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_01_JDG ), 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_02_JDG))
			{
				float delayTimer2 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_02_JDG ), 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer2 );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_03_JDG))
			{
				float delayTimer3 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_03_JDG ), 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer3 );
			}
		}

		else 
		{
			if (Commands->Find_Object (M01_HON_DOJO_CIV_01_JDG))
			{
				float delayTimer = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_01_JDG ), 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, delayTimer );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_02_JDG))
			{
				float delayTimer2 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_02_JDG ), 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, delayTimer2 );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_03_JDG))
			{
				float delayTimer3 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_03_JDG ), 0, M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG, delayTimer3 );
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR && first_time == true)
		{
			first_time = false;

			if (Commands->Find_Object (M01_HON_DOJO_CIV_01_JDG))
			{
				float delayTimer = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_01_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, delayTimer );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_02_JDG))
			{
				float delayTimer2 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_02_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, delayTimer2 );
			}

			if (Commands->Find_Object (M01_HON_DOJO_CIV_03_JDG))
			{
				float delayTimer3 = Commands->Get_Random ( 0, 1 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_03_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, delayTimer3 );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_START_ACTING_JDG: //here comes the player--start your routine
				{
					//Commands->Set_Innate_Is_Stationary ( obj, false );
					//Commands->Innate_Enable(obj);
					Commands->Set_Innate_Aggressiveness ( obj, 0.50f );
					Commands->Set_Innate_Take_Cover_Probability ( obj, 0.5f );
					Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, false );
					Commands->Innate_Soldier_Enable_Footsteps_Heard ( obj, false );

					dojo_conv = Commands->Create_Conversation( "M01_Dojo_Sensei_Conversation", 100, 1000, true);
					Commands->Join_Conversation( obj, dojo_conv, false, true, true );
					Commands->Start_Conversation( dojo_conv,  dojo_conv );
							
					Commands->Monitor_Conversation(  obj, dojo_conv);

					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 5 );
				}
				break;

			case M01_MODIFY_YOUR_ACTION_JDG: //turn your sight and hearing back on  M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG
				{
					Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, true );
					Commands->Innate_Soldier_Enable_Footsteps_Heard ( obj, true );
				}
				break;

			case M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG: //turn your sight and hearing back on  M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG
				{
					handOfNodDead = true;
					Commands->Innate_Enable(obj);
					Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, true );
					Commands->Innate_Soldier_Enable_Footsteps_Heard ( obj, true );
				}
				break;
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == dojo_conv)//your initial conv is over--tell civs to do pushups
					{
						if (Commands->Find_Object (M01_HON_DOJO_CIV_01_JDG))
						{
							float delayTimer = Commands->Get_Random ( 0, 1 );
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_01_JDG ), 0, M01_START_ACTING_JDG, delayTimer );
						}

						if (Commands->Find_Object (M01_HON_DOJO_CIV_02_JDG))
						{
							float delayTimer2 = Commands->Get_Random ( 0, 1 );
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_02_JDG ), 0, M01_START_ACTING_JDG, delayTimer2 );
						}

						if (Commands->Find_Object (M01_HON_DOJO_CIV_03_JDG))
						{
							float delayTimer3 = Commands->Get_Random ( 0, 1 );
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_HON_DOJO_CIV_03_JDG ), 0, M01_START_ACTING_JDG, delayTimer3 );
						}
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_HON_Dojo_Civ_01_JDG, "")
{
	/*#define M01_HON_DOJO_CIV_01_JDG					102048
	  #define M01_HON_DOJO_CIV_02_JDG					102049
	  #define M01_HON_DOJO_CIV_03_JDG					102050*/

	int i_am;
	int dojo_civ_01;
	int dojo_civ_02;
	int dojo_civ_03;
	int pushups_done;
	int thanks_conv;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(i_am, 1);
		SAVE_VARIABLE(dojo_civ_01, 2);
		SAVE_VARIABLE(dojo_civ_02, 3);
		SAVE_VARIABLE(dojo_civ_03, 4);
		SAVE_VARIABLE(pushups_done, 5);
		SAVE_VARIABLE(thanks_conv, 6);
	}

	void Created( GameObject * obj ) override 
	{
		dojo_civ_01 = 102048;//babushka
		dojo_civ_02 = 102049;//billy
		dojo_civ_03 = 102050;//farmerjohn
		pushups_done = 0;

		Commands->Innate_Disable(obj);
		int myIdNumber = Commands->Get_ID ( obj );

		if (myIdNumber == dojo_civ_01)
		{
			i_am = dojo_civ_01;
		}

		else if (myIdNumber == dojo_civ_02)
		{
			i_am = dojo_civ_02;
		}

		else if (myIdNumber == dojo_civ_03)
		{
			i_am = dojo_civ_03;
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj);
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_START_ACTING_JDG: //you've been told to do pushups--DO THEM!
				{
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );//this is animation of guy going into pushup position
					params.Set_Animation( "H_A_J13A", false );

					Commands->Action_Play_Animation (  obj, params );
				}
				break;

			case M01_MODIFY_YOUR_ACTION_JDG: //you're done with pushups--report to the armory
				{
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
					params.Set_Movement( Vector3(0,0,0), WALK, 2 );
					params.WaypathID = 102056;
					params.WaypointStartID = 102058;
					params.WaypointEndID = 102062;
					Commands->Action_Goto( obj, params );
				}
				break;

			case M01_PLAYER_IS_ATTACKING_ME_JDG: //your guard is alerted to Havoc--stop whatever you are doing
				{
					Commands->Action_Reset ( obj, 100 );
					Vector3 myPosition = Commands->Get_Position ( obj );
					myPosition.X += Commands->Get_Random ( 0.5f, 1 );
					myPosition.Y += Commands->Get_Random ( 0.5f, 2 );

					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
					params.Set_Movement( myPosition, WALK, 0 );
					params.MoveCrouched = true;
					Commands->Action_Goto( obj, params );
				}
				break;

			case M01_MODIFY_YOUR_ACTION_02_JDG: //your guard is dead--flock Havoc and say thanks
				{
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_03_JDG );
					params.Set_Look( STAR, 5 );
					
					Commands->Action_Face_Location ( obj, params );

					float delayTimer = Commands->Get_Random ( 0, 1 );
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, delayTimer );
				}
				break;

			case M01_MODIFY_YOUR_ACTION_03_JDG: //play thanks conv
				{
					if (obj)
					{
						Commands->Join_Conversation( obj, thanks_conv, false, true, true );
						Commands->Join_Conversation( STAR, thanks_conv, false, false, false );
						Commands->Start_Conversation( thanks_conv,  thanks_conv );
						Commands->Monitor_Conversation( obj, thanks_conv );
					}
				}
				break;

			case M01_MODIFY_YOUR_ACTION_04_JDG: //play thanks conv
				{
					if (i_am == dojo_civ_01)//babushka
					{
						thanks_conv = Commands->Create_Conversation( "M01_Dojo_Babushka_Thanks_Conversation", 100, 1000, true);
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
					}

					else if (i_am == dojo_civ_02)
					{
						thanks_conv = Commands->Create_Conversation( "M01_Dojo_Billy_Thanks_Conversation", 100, 1000, true);
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 2 );
					}

					else 
					{
						thanks_conv = Commands->Create_Conversation( "M01_Dojo_FarmerJohn_Thanks_Conversation", 100, 1000, true);
						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 4 );
					}
					
				}
				break;

			case M01_PICK_A_NEW_LOCATION_JDG: //see how far you are from havoc
				{
					Vector3 myPosition = Commands->Get_Position ( obj );
					Vector3 playerPosition = Commands->Get_Position ( STAR );

					float myDistance = Commands->Get_Distance ( myPosition, playerPosition );

					if (myDistance > 75)
					{
						bool starVisible = Commands->Is_Object_Visible( STAR, obj );
						if (starVisible == true)
						{
							Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 10 );
						}

						else
						{
							Commands->Destroy_Object ( obj );
						}
					}

					else
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 10 );
					}
				}
				break;

			case M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG: //turn your sight and hearing back on  M01_YOUR_BUILDING_HAS_BEEN_DESTROYED_JDG
				{
					Commands->Action_Reset ( obj, 100 );
					Vector3 newGotoSpot(-54.512f, 183.109f, 0.774f);//this is at the landing beach
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
					params.Set_Movement( newGotoSpot, RUN, 5 );
		
					Commands->Action_Goto( obj, params );

					Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 10 );//do distance check from star
				}
				break;
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == thanks_conv)
					{
						Vector3 myPosition = Commands->Get_Position ( obj );
						Commands->Create_Sound ( "M01_GDI_Thanks_Twiddler", myPosition, obj );

						Vector3 newGotoSpot(-54.512f, 183.109f, 0.774f);//this is at the landing beach
						params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
						params.Set_Movement( newGotoSpot, RUN, 5 );
			
						Commands->Action_Goto( obj, params );

						Commands->Send_Custom_Event( obj, obj, 0, M01_PICK_A_NEW_LOCATION_JDG, 10 );//do distance check from star
					}
				}
				break;

			case ACTION_COMPLETE_NORMAL: 
				{
					switch (action_id)
					{
						case M01_DOING_ANIMATION_01_JDG: 
							{
								if (pushups_done < 2)
								{
									params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );//this is pushup animation animation 
									params.Set_Animation( "H_A_J13B", false );

									Commands->Action_Play_Animation (  obj, params );
									pushups_done++;
								}

								else 
								{
									params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );//this is guy standing up 
									params.Set_Animation( "H_A_J13C", false );

									Commands->Action_Play_Animation (  obj, params );
								}
							}
							break;

						case M01_DOING_ANIMATION_02_JDG: 
							{
								const char* animationName = M01_Choose_Idle_Animation ( );

								params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );//this is guy standing up 
								params.Set_Animation( animationName, false );

								Commands->Action_Play_Animation (  obj, params );
							}
							break;

						case M01_DOING_ANIMATION_03_JDG: 
							{
								if (i_am == dojo_civ_01)
								{
									float delayTimer = Commands->Get_Random ( 0, 1 );
									Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
								}

								else if (i_am == dojo_civ_02)
								{
									float delayTimer2 = Commands->Get_Random ( 1, 2 );
									Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer2 );
								}

								else if (i_am == dojo_civ_03)
								{
									float delayTimer3 = Commands->Get_Random ( 2, 3 );
									Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer3 );
								}
							}
							break;

						case M01_WALKING_WAYPATH_01_JDG: 
							{
								params.Set_Basic( this, 100, M01_DOING_ANIMATION_04_JDG );//this is guy placing C$ 
								params.Set_Animation( "H_A_J12C", false );

								Commands->Action_Play_Animation (  obj, params );
							}
							break;

						case M01_DOING_ANIMATION_04_JDG: 
							{
								Vector3 myPosition = Commands->Get_Position ( obj );
								GameObject *newsoldier = Commands->Create_Object("Nod_MiniGunner_0", myPosition);
								Commands->Attach_Script(newsoldier, "M01_Civ_To_Minigunner_Guy_JDG", "");
								Commands->Destroy_Object ( obj );
							}
							break;							
					}
				}
				break;
		}
	}
};



DECLARE_SCRIPT(M01_Nod_GuardTower_Tailgun_JDG, "")//100246
{
	void Created( GameObject * obj ) override 
	{
		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_START_ATTACKING_01_JDG );
		params.Set_Attack( STAR, 30, 1, true);
		Commands->Action_Attack(obj, params);
	}
};

DECLARE_SCRIPT(M01_Comm_Mainframe_PogZone_01_JDG, "")//108024
{//target location ()
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Vector3 radarMarker (-293.163f, 499.787f, 19.959f);
			Commands->Set_Objective_HUD_Info_Position( M01_UNLOCK_GATE_OBJECTIVE_JDG, 93, "POG_M01_1_02.tga", IDS_POG_ACCESS, radarMarker );

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Mainframe_PogZone_02_JDG, "")//108026
{//target location ()
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Vector3 radarMarker (-292.135f, 522.740f, 16.448f);
			Commands->Set_Objective_HUD_Info_Position( M01_UNLOCK_GATE_OBJECTIVE_JDG, 93, "POG_M01_1_02.tga", IDS_POG_ACCESS, radarMarker );

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Mainframe_PogZone_03_JDG, "")//108028
{//target location ()
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Vector3 radarMarker (-314.578f, 517.124f, 16.447f);
			Commands->Set_Objective_HUD_Info_Position( M01_UNLOCK_GATE_OBJECTIVE_JDG, 93, "POG_M01_1_02.tga", IDS_POG_ACCESS, radarMarker );

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Comm_Center_Player_Terminal_Zone, "")//107809  
{
	bool poked;
	bool commCenterDead;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(poked, 1);
		SAVE_VARIABLE(commCenterDead, 2);
	}

	void Created( GameObject * obj ) override 
	{
		commCenterDead = false;
		Commands->Enable_HUD_Pokable_Indicator ( obj, true );
		Commands->Display_Health_Bar( obj, false );
		poked = false;
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR && poked == false)
		{
			if (commCenterDead == false)
			{
				poked = true;
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_HAS_POKED_COMM_CENTER_PCT_JDG, 0 );

				Commands->Set_Obj_Radar_Blip_Shape ( obj, -1 );
				Commands->Set_Obj_Radar_Blip_Color ( obj, -1 );

				GameObject * gateTutorialZone = Commands->Find_Object ( 120844 );
				if (gateTutorialZone != NULL)
				{
					Commands->Send_Custom_Event( obj, gateTutorialZone, 0, M01_START_ACTING_JDG, 0 );
				}

				Commands->Enable_HUD_Pokable_Indicator ( obj, false );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_COMM_SAM_OBJECTIVE_JDG)
			{
				Commands->Set_Obj_Radar_Blip_Shape ( obj, RADAR_BLIP_SHAPE_OBJECTIVE );
				Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_PRIMARY_OBJECTIVE );
			}

			else if (param == M01_COMM_CENTER_HAS_BEEN_DESTROYED_JDG)//comm center has been destroyed
			{
				commCenterDead = true;
				Commands->Enable_HUD_Pokable_Indicator ( obj, false );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Objective_Pog_Controller_JDG, "")//M01_MISSION_POG_CONTROLLER_JDG 105828
{
	int missionIntro;
	int escKeyConv;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(missionIntro, 1);
		SAVE_VARIABLE(escKeyConv, 2);
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == missionIntro) //add first objective pog here
					{
						Vector3 prisonerPenLocation (-312.725f, 575.887f, 27.221f);
						Commands->Add_Objective( M01_PRISONER_OBJECTIVE_JDG, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M01_01, NULL, IDS_Enc_Obj_Primary_M01_01 );
						Commands->Set_Objective_Radar_Blip( M01_PRISONER_OBJECTIVE_JDG, prisonerPenLocation );
						
						Commands->Set_Objective_HUD_Info_Position( M01_PRISONER_OBJECTIVE_JDG, 80, "POG_M01_1_01.tga", IDS_POG_LOCATE, prisonerPenLocation );
						Commands->Set_HUD_Help_Text ( IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

						escKeyConv = Commands->Create_Conversation( "M01_Press_ESC_For_Objectives_Conversation", 100, 1000, false);
						Commands->Join_Conversation( NULL, escKeyConv, false, false, true );
						Commands->Start_Conversation( escKeyConv,  escKeyConv );

						Commands->Monitor_Conversation(  obj, escKeyConv );
					}

					else if (action_id == escKeyConv)
					{
						Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0516I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					}
				}
				break;
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG)
			{
				missionIntro = Commands->Create_Conversation( "M01_Start_Mission_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, missionIntro, false, false, true );
				//Commands->Join_Conversation( STAR, missionIntro, false, false, true );
				Commands->Start_Conversation( missionIntro,  missionIntro );

				Commands->Monitor_Conversation(  obj, missionIntro );//M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG
			}

			if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				escKeyConv = Commands->Create_Conversation( "M01_Press_ESC_For_Objectives_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, escKeyConv, false, false, true );
				Commands->Start_Conversation( escKeyConv,  escKeyConv );

				Commands->Monitor_Conversation(  obj, escKeyConv );
			}
		}

		else if (type == M01_ADD_OBJECTIVE_POG_JDG)
		{
			if (param == M01_GDI_BASE_POW_OBJECTIVE_JDG)
			{
				Vector3 initialCaptainLocation (47.128f, 36.584f, -0.930f);
				Commands->Add_Objective( M01_GDI_BASE_POW_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_06, NULL, IDS_Enc_Obj_Secondary_M01_06 );
				Commands->Set_Objective_Radar_Blip( M01_GDI_BASE_POW_OBJECTIVE_JDG, initialCaptainLocation );

				Commands->Set_Objective_HUD_Info_Position( M01_GDI_BASE_POW_OBJECTIVE_JDG, 85, "POG_M01_2_08.tga", IDS_POG_RESCUE, initialCaptainLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 2 );
			}

			else if (param == M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG)
			{
				GameObject * barnCommander = Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG );
				if (barnCommander != NULL)
				{
					Vector3 commanderPosition = Commands->Get_Position ( barnCommander );
					Commands->Set_Objective_HUD_Info_Position( M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG, 86, "POG_M01_2_01.tga", IDS_POG_ELIMINATE, commanderPosition );

					//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_FIRST_NOD_COMMANDER_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

					//Vector3 blue (0,0,1);
					Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_01, TEXT_COLOR_OBJECTIVE_PRIMARY );
				}
			}

			else if (param == M01_TURRETS_OBJECTIVE_JDG)
			{
				Vector3 turretsLocation;
				GameObject * turret01 = Commands->Find_Object ( 101434 );
				GameObject * turret02 = Commands->Find_Object ( 101435 );
				if (turret01 != NULL)
				{
					turretsLocation.Set  (-83.916f, 91.667f, 2.807f);
				}

				else if (turret02 != NULL)
				{
					turretsLocation.Set (-105.024f, 113.658f, 1.872f);
				}
				
				Commands->Set_Objective_HUD_Info_Position( M01_TURRETS_OBJECTIVE_JDG, 91, "POG_M01_2_02.tga", IDS_POG_DESTROY, turretsLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_TURRETS_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_02, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}

			else if (param == M01_BARN_OBJECTIVE_JDG)
			{
				Vector3 barnLocation (-49.910f, 183.860f, 0.774f);
				Commands->Set_Objective_HUD_Info_Position( M01_BARN_OBJECTIVE_JDG, 92, "POG_M01_2_03.tga", IDS_POG_RESCUE, barnLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_BARN_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_03, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}

			else if (param == M01_BARN_ROUNDUP_OBJECTIVE_JGD)
			{
				Vector3 barnLocation (-54.786f, 213.816f, 6.453f);
				Commands->Set_Objective_HUD_Info_Position( M01_BARN_ROUNDUP_OBJECTIVE_JGD, 93, "POG_M01_2_04.tga", IDS_POG_EVACUATE, barnLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_BARN_ROUNDUP_OBJECTIVE_JGD, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_04, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}

			else if (param == M01_CHURCH_OBJECTIVE_JDG)//
			{
				Vector3 churchLocation = Commands->Get_Position ( Commands->Find_Object ( M01_CHURCH_PRIEST_ID ) );
				Commands->Set_Objective_HUD_Info_Position( M01_CHURCH_OBJECTIVE_JDG, 94, "POG_M01_2_06.tga", IDS_POG_RESCUE, churchLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_CHURCH_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_05, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}

			else if (param == M01_HON_OBJECTIVE_JDG)
			{
				Vector3 mctLocation (-179.3f, 541.9f, 3.518f);
				Commands->Set_Objective_HUD_Info_Position( M01_HON_OBJECTIVE_JDG, 95, "POG_M02_2_01.tga", IDS_POG_DESTROY, mctLocation );

				//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_HON_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

				//Vector3 blue (0,0,1);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_07, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}

			else if (param == M01_COMM_SAM_OBJECTIVE_JDG)
			{
				Vector3 samLocation (-337.169f, 549.232f, 31.949f);
				Commands->Set_Objective_HUD_Info_Position( M01_COMM_SAM_OBJECTIVE_JDG, 96, "POG_M01_1_04.tga", IDS_POG_DESTROY, samLocation );

				//Vector3 green (0,1,0);
				Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Primary_M01_04, TEXT_COLOR_OBJECTIVE_PRIMARY );
			}
		}

		else if (type == M01_CHANGE_OBJECTIVE_POG_JDG)
		{
			GameObject * captDuncan = Commands->Find_Object ( 106050 );
			if (captDuncan != NULL)
			{
				if (param == M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG)
				{
					//Commands->Set_Objective_Status( M01_GDI_BASE_POW_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
					Commands->Add_Objective( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M01_08, NULL, IDS_Enc_Obj_Secondary_M01_08 );
					Vector3 realCaptainLocation (109.959f, 97.340f, 8.923f);
					Commands->Set_Objective_HUD_Info_Position( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, 91, "POG_M01_2_05.tga", IDS_POG_CONTACT, realCaptainLocation );
					Commands->Set_Objective_Radar_Blip( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, realCaptainLocation );
					//Commands->Send_Custom_Event( obj, obj, M01_REMOVE_OBJECTIVE_POG_JDG, M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, SECONDARY_POG_DELAY );

					GameObject * triggerZone03 = Commands->Find_Object ( 106052 );
					if (triggerZone03 != NULL)
					{
						Commands->Send_Custom_Event( obj, triggerZone03, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
					}

					//Vector3 blue (0,0,1);
					Commands->Set_HUD_Help_Text ( IDS_Enc_ObjTitle_Secondary_M01_08, TEXT_COLOR_OBJECTIVE_PRIMARY );
				}
			}
		

			else if (param == M01_TURRETS_OBJECTIVE_JDG)
			{
				Vector3 turretsLocation;
				GameObject * turret02 = Commands->Find_Object ( 101435 );
				if (turret02 != NULL)
				{
					turretsLocation.Set (-105.024f, 113.658f, 1.872f);
				}
				
				Commands->Set_Objective_HUD_Info_Position( M01_TURRETS_OBJECTIVE_JDG, 91, "POG_M01_2_02.tga", IDS_POG_DESTROY, turretsLocation );
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_FirstChinook_Script_JDG, "")//
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * gdiGuy01 = Commands->Find_Object ( 104685 );
		if (gdiGuy01 != NULL)
		{
			Commands->Send_Custom_Event( obj, gdiGuy01, 0, M01_MODIFY_YOUR_ACTION_07_JDG, 0 );
		}
	}
};







DECLARE_SCRIPT(M01_Church_EvacController_JDG, "")//103394 
{
	int evac_waypath_id;
	int evac_helicopter_id;
	int evac_rope_id;
	int protectCivsConv;
	int deadCivs;
	bool transportCalled;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(evac_waypath_id, 1);
		SAVE_VARIABLE(evac_helicopter_id, 2);
		SAVE_VARIABLE(evac_rope_id, 3);
		SAVE_VARIABLE(protectCivsConv, 4);
		SAVE_VARIABLE(deadCivs, 5);
		SAVE_VARIABLE(transportCalled, 6);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		deadCivs = 0;
		transportCalled = false;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override
	{
		if ((param == M01_START_ACTING_JDG))//civs have been freed--play protect civs conversation
		{
			protectCivsConv = Commands->Create_Conversation( "M01_Protect_Clergy_Conversation", 100, 1000, false);
			Commands->Join_Conversation( NULL, protectCivsConv, false, false, true );
			//Commands->Join_Conversation( STAR, protectCivsConv, false, false, true );
			Commands->Start_Conversation( protectCivsConv,  protectCivsConv );
			Commands->Monitor_Conversation (obj, protectCivsConv);
		}

		else if (param == M01_EVERYONES_ON_BOARD_JDG)//civs are on board--send aircraft away
		{
			GameObject * evacChopper = Commands->Find_Object ( evac_helicopter_id );
			GameObject * evacWaypath = Commands->Find_Object ( evac_waypath_id );
			GameObject * evacRope = Commands->Find_Object ( evac_rope_id );

			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (evacRope != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacRope, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}

		else if (param == M01_CIVILIAN_KILLED_JDG)
		{
			deadCivs++;
			if (deadCivs == 3)
			{
				Commands->Send_Custom_Event ( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CLEAR_CHURCH_OBJECTIVE_FAIL_JDG, 0 );
			}
		}

		else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//chopper is sending you his ID
		{
			evac_helicopter_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************chopper id received.....%d\n",evac_helicopter_id );
		}

		else if (param == M01_WAYPATH_IS_SENDING_ID_JDG)//waypath is sending you his ID 
		{
			evac_waypath_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************waypath id received.....%d\n", evac_waypath_id );
		}

		else if (param == M01_ROPE_IS_SENDING_ID_JDG)//rope is sending you his ID 
		{
			evac_rope_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************rope id received.....%d\n", evac_rope_id );
		}

		else if (param == M01_GIVE_ME_A_ROPE_JDG)//someone is waiting--pull him up
		{
			Vector3 evacPosition = Commands->Get_Position ( obj );
			GameObject * troopbone = Commands->Create_Object("Invisible_Object", evacPosition );
			Commands->Set_Model ( troopbone, "XG_EV5_troopBN" );
			
			Commands->Attach_Script(troopbone, "M01_Evac_TroopBone_JDG", "");
			Commands->Innate_Disable(sender);
			Commands->Attach_To_Object_Bone( sender, troopbone, "Troop_L" );
			if(troopbone)
			{
				Commands->Set_Animation ( troopbone, "XG_EV5_troopBN.XG_EV5_troopBN", false, NULL, 0.0f, -1.0f, false);
			}
			else
			{
				Commands->Debug_Message("troopbone not created yet");
			}
			
			Commands->Set_Animation ( sender, "S_A_Human.XG_EV5_troop", false, NULL, 0.0f, -1.0f, false );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == protectCivsConv)
			{
				if (transportCalled == false)
				{
					transportCalled = true;

					Vector3 evacPosition = Commands->Get_Position ( obj );
					GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
					Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_ChurchArea_EvacAnim.txt");
				}

				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-140.9f, 320.2f, 8.5f));
				Commands->Set_Facing(chinook_obj1, -54.0f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_GDI_Church_TroopDrop.txt");

				Commands->Create_Sound ( "00-N180E", Vector3 (0,0,0), obj);
			}
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_Air_Evac_Rope_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Debug_Message ( "**********************script attached to evac rope\n" );
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( 103394 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_ROPE_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************rope has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_Air_Evac_Waypath_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( 103394 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_WAYPATH_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************waypath has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_ChurchArea_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsA", false, NULL, 0.0f, -1.0f, false );
		Commands->Debug_Message ( "**********************script attached to evac chopper\n" );
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );

		GameObject * myController = Commands->Find_Object ( 103394 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsA") == 0)
		{
			Commands->Debug_Message ( "**********************chopper has finished entry anim\n" );
			Vector3 evacLocation = Commands->Get_Position ( obj );
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsL", true, NULL, 0.0f, -1.0f, false );

			GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID ); 
			GameObject * churchNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
			GameObject * priest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );

			if (loveshackNun != NULL)
			{
				Commands->Send_Custom_Event( obj, loveshackNun, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
			}

			if (churchNun != NULL)
			{
				Commands->Send_Custom_Event( obj, churchNun, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
			}

			if (priest != NULL)
			{
				Commands->Send_Custom_Event( obj, priest, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
			}

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************helicopter has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * loveshackNun = Commands->Find_Object ( M01_CHURCH_LOVESHACK_NUN_ID ); 
					GameObject * churchNun = Commands->Find_Object ( M01_CHURCH_INTERIOR_NUN_ID );
					GameObject * priest = Commands->Find_Object ( M01_CHURCH_PRIEST_ID );

					if ((loveshackNun != NULL) || (churchNun != NULL) || (priest != NULL))
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
					}

					else
					{
						Commands->Debug_Message ( "*******************************all barn civs on board--about to send cutom to controller\n" );
						GameObject * controller = Commands->Find_Object ( 103394 );
						if (controller != NULL)
						{
							Commands->Send_Custom_Event ( obj, controller, 0, M01_EVERYONES_ON_BOARD_JDG, 0 );
							Commands->Debug_Message ( "*******************************all barn civs on board--sent cutom to controller\n" );
						}

						
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_Church_Priest_JDG, "")
{
	int lucky_charms;
	int prayerSound;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(lucky_charms, 1);
		SAVE_VARIABLE(prayerSound, 1);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		Vector3 myPosition = Commands->Get_Position ( obj );
		prayerSound = Commands->Create_Sound ( "MX1DSGN_DSGN0020I1DSGN_SND", myPosition, obj );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Stop_Sound ( prayerSound, true );
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj);
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103394 ), 0, M01_CIVILIAN_KILLED_JDG, 0 );
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_PRIEST_CONVERSATION_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			params.Set_Basic( this, 90, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation ("H_A_Host_L2b", true);
			Commands->Action_Play_Animation (obj, params);

			Commands->Start_Sound ( prayerSound );
		}

		else if ((param == M01_CHURCH_GUARD_IS_DEAD_JDG))//Your guard is dead--do something
		{
			Commands->Stop_Sound ( prayerSound, true );
			Commands->Enable_Hibernation( obj, false );
			params.Set_Basic( this, 90, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation ("H_A_Host_L2c", false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if ((param == M01_GOING_TO_EVAC_SPOT_JDG))//your ride is here--go get in
		{
			Vector3 myPosition = Commands->Get_Position ( obj );
			Vector3 starPosition = Commands->Get_Position ( STAR );
			float distance = Commands->Get_Distance ( myPosition, starPosition );

			if (distance <= 15)
			{
				lucky_charms = Commands->Create_Conversation( "M01_Priest_LuckyCharms_Conversation", 100, 1000, false);
				Commands->Join_Conversation( obj, lucky_charms, false, true, true );
				Commands->Join_Conversation( STAR, lucky_charms, false, false, false );
				Commands->Start_Conversation( lucky_charms,  lucky_charms );

				Commands->Monitor_Conversation( obj, lucky_charms );
			}

			else
			{
				Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( 103394 ) );
				params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evacSpot, RUN, 0.5f );
				Commands->Action_Goto( obj, params );
			}
			
			//ActionParamsStruct params;
			//Commands->Action_Reset ( obj, 100 );
			//params.Set_Basic(this, 100, M01_PICK_A_NEW_LOCATION_JDG);
			//params.Set_Movement( STAR, RUN, 8 );
			//Commands->Action_Goto( obj, params );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_DOING_ANIMATION_01_JDG )//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 90, M01_DOING_ANIMATION_02_JDG );
				params.Set_Animation ("H_A_Host_L1b", false);
				Commands->Action_Play_Animation (obj, params);
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG)//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 90, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation ("H_A_Host_L1c", false);
				Commands->Action_Play_Animation (obj, params);
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG)//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 90, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( Vector3 (-135.571f, 340.213f, 9.608f), 0.9f, 0.5f );
				Commands->Action_Goto( obj, params );
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG)//civ is standing up after guard is dead
			{ 
				Commands->Enable_Hibernation( obj, true );

				if (Commands->Find_Object ( 103395 ))//this is your conversation zone
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103395 ), 0, M01_START_ACTING_JDG, 0 );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//Nun is by chinook--play getting on anim
			{ 
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CHURCH_CLERGY_HAS_BOARDED_JDG, 1 );
				Commands->Give_Points( STAR, 2000, false );

				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );

				GameObject * airdropController = Commands->Find_Object ( M01_CHURCHAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( 103394 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_PICK_A_NEW_LOCATION_JDG)//priest is by Havoc--cue lucky charms conversation
			{ 
				//lucky_charms = Commands->Create_Conversation( "M01_Priest_LuckyCharms_Conversation", 100, 1000, false);
				//Commands->Join_Conversation( obj, lucky_charms, true, true, true );
				//Commands->Start_Conversation( lucky_charms,  lucky_charms );

				//Commands->Monitor_Conversation( obj, lucky_charms );
			}

			else if (action_id == M01_DOING_ANIMATION_04_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//priest is by Havoc--cue lucky charms conversation
			{
				Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( 103394 ) );
				params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
				params.Set_Movement( evacSpot, RUN, 0.5f );
				Commands->Action_Goto( obj, params );
			}
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)//priest is by Havoc--cue lucky charms conversation
		{ 
			if (action_id == lucky_charms)
			{
				params.Set_Basic(this, 100, M01_DOING_ANIMATION_04_JDG);
				params.Set_Animation ("H_A_J12C", false);
				Commands->Action_Play_Animation (obj, params);

				Vector3 spawnPosition = Commands->Get_Position ( obj );
				spawnPosition.Z += 0.75f;

				int spawn_count = 1;

				while (spawn_count <= 1)
				{
					spawnPosition.X += Commands->Get_Random( -2.0f , 2.0f);
					spawnPosition.Y += Commands->Get_Random( -2.0f , 2.0f);

					GameObject *luckycharm;
					luckycharm = Commands->Create_Object ( "POW_Data_Disc", spawnPosition );
					Commands->Attach_Script(luckycharm, "M01_Priest_Datadisc_JDG", "");
					spawn_count++;
				}
			}
		}
	}		
};

DECLARE_SCRIPT(M01_Church_Interior_Nun_JDG, "")
{
	bool following_player;
	int prayerSound;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(following_player, 1);
		SAVE_VARIABLE(prayerSound, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		following_player = false;
		Vector3 myPosition = Commands->Get_Position ( obj );
		prayerSound = Commands->Create_Sound ( "MX1DSGN_DSGN0021I1DSGN_SND", myPosition, obj );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Stop_Sound ( prayerSound, true );
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj);
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103394 ), 0, M01_CIVILIAN_KILLED_JDG, 0 );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			params.Set_Basic( this, 90, 0 );
			params.Set_Animation ("S_A_Human.H_A_Host_L2b", true);
			Commands->Action_Play_Animation (obj, params);

			Commands->Start_Sound ( prayerSound );
		}

		else if ((param == M01_CHURCH_GUARD_IS_DEAD_JDG))//Your guard is dead--do something
		{
			Commands->Stop_Sound ( prayerSound, true );
			Commands->Enable_Hibernation( obj, false );
			params.Set_Basic( this, 90, M01_DOING_ANIMATION_01_JDG );
			params.Set_Animation ("S_A_Human.H_A_Host_L2c", false);
			Commands->Action_Play_Animation (obj, params);
		}

		else if ((param == M01_GOING_TO_EVAC_SPOT_JDG))//your ride is here--go get in
		{
			Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( 103394 ) );
			ActionParamsStruct params;
			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evacSpot, RUN, 1 );
				
			Commands->Action_Goto( obj, params );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_DOING_ANIMATION_01_JDG)//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
				params.Set_Animation ("S_A_Human.H_A_Host_L1b", false);
				Commands->Action_Play_Animation (obj, params);
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG)//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation ("S_A_Human.H_A_Host_L1c", false);
				Commands->Action_Play_Animation (obj, params);

				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Create_Sound ( "MX1DSGN_DSGN0022I1DSGN_SND", myPosition, obj );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG)//civ is standing up after guard is dead
			{ 
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( Vector3 (-136.738f, 325.073f, 8.167f), RUN, 0.50f );
				Commands->Action_Goto( obj, params );
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG)//Nun is by chinook--play getting on anim
			{ 
				Commands->Enable_Hibernation( obj, true );

				if (Commands->Find_Object ( 101370 ))//this is your conversation zone
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101370 ), 0, M01_START_ACTING_JDG, 1 );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG )//Nun is by chinook--play getting on anim
			{ 
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CHURCH_CLERGY_HAS_BOARDED_JDG, 1 );
				Commands->Give_Points( STAR, 2000, false );

				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );

				GameObject * airdropController = Commands->Find_Object ( M01_CHURCHAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( 103394 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}
		}
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		ActionParamsStruct params;

		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_INTERIOR_NUN_CONVERSATION_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_Church_LoveShack_Nun_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj);
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( 103394 ), 0, M01_CIVILIAN_KILLED_JDG, 0 );

		GameObject * conversationZone = Commands->Find_Object ( 103392 );
		if (conversationZone != NULL)
		{
			Commands->Destroy_Object ( conversationZone );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ((param == M01_START_ACTING_JDG))//Player is entering church area--cue actors
		{
			params.Set_Basic(this, 90, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Vector3(0,0,0), WALK, .25f );
			params.WaypathID = 100750;
			params.WaypointStartID = 100753;
			params.WaypointEndID = 100758;
			Commands->Action_Goto( obj, params );
		}

		else if ((param == M01_LOVESHACK_GUARD_IS_DEAD_JDG))//Your guard is dead--follow havoc
		{
			Commands->Enable_Hibernation( obj, false );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
			params.Set_Movement( Vector3 (0,0,0), RUN, 1.0f );
			params.WaypathID = 103383;
			params.WaypointStartID = 103384;
			params.WaypointEndID = 103390;
			Commands->Action_Goto( obj, params );

		}

		else if ((param == M01_GOING_TO_EVAC_SPOT_JDG))//your ride is here--go get in
		{
			Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( 103394 ) );
			ActionParamsStruct params;
			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evacSpot, RUN, 1 );
				
			Commands->Action_Goto( obj, params );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_GOING_TO_EVAC_SPOT_JDG )//Nun is by chinook--play getting on anim
			{ 
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CHURCH_CLERGY_HAS_BOARDED_JDG, 1 );
				Commands->Give_Points( STAR, 2000, false );

				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				//GameObject *spawn_effect = Commands->Create_Object( "Spawner Created Special Effect", powerupSpawnLocation );

				//if (spawn_effect)
				//{
				//	Commands->Attach_Script( spawn_effect, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				//}

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );

				GameObject * airdropController = Commands->Find_Object ( M01_CHURCHAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( 103394 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG )//nun is at hide spot--play animation--cue havoc talk zone
			{ 
				Commands->Enable_Hibernation( obj, true );

				if (Commands->Find_Object ( 101368 ))//this is your conversation zone
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101368 ), 0, M01_START_ACTING_JDG, 1 );
				}
			}
		}
	}

	void Poked( GameObject * obj, GameObject * poker ) override
	{
		if (poker == STAR)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_LOVESHACK_NUN_CONVERSATION_JDG, 0 );
		}
	}
};



DECLARE_SCRIPT(M01_BarnArea_Air_Evac_Rope_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Debug_Message ( "**********************script attached to evac rope\n" );
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_ROPE_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************rope has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_BarnArea_Air_Evac_Waypath_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_WAYPATH_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************waypath has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_BarnArea_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsA", false, NULL, 0.0f, -1.0f, false );
		Commands->Debug_Message ( "**********************script attached to evac chopper\n" );
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );

		GameObject * myController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}

		GameObject * barnCiv01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
		GameObject * barnCiv02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
		GameObject * barnCiv03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );

		if (barnCiv01 != NULL)
		{
			Commands->Send_Custom_Event ( obj, barnCiv01, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
		}

		if (barnCiv02 != NULL)
		{
			Commands->Send_Custom_Event ( obj, barnCiv02, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
		}

		if (barnCiv03 != NULL)
		{
			Commands->Send_Custom_Event ( obj, barnCiv03, 0, M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsA") == 0)
		{
			Commands->Debug_Message ( "**********************chopper has finished entry anim\n" );
			Vector3 evacLocation = Commands->Get_Position ( obj );
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsL", true, NULL, 0.0f, -1.0f, false );

			GameObject * barnCiv01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
			GameObject * barnCiv02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
			GameObject * barnCiv03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );

			if (barnCiv01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, barnCiv01, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (barnCiv02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, barnCiv02, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			if (barnCiv03 != NULL)
			{
				Commands->Send_Custom_Event ( obj, barnCiv03, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 2 );
			}

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************helicopter has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * barnCiv01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
					GameObject * barnCiv02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
					GameObject * barnCiv03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );

					if ((barnCiv01 != NULL)||(barnCiv02 != NULL)||(barnCiv03 != NULL))
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 1 );
					}

					else
					{
						Commands->Debug_Message ( "*******************************all barn civs on board--about to send cutom to controller\n" );
						GameObject * controller = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
						if (controller != NULL)
						{
							Commands->Send_Custom_Event ( obj, controller, 0, M01_EVERYONES_ON_BOARD_JDG, 0 );
							Commands->Debug_Message ( "*******************************all barn civs on board--sent cutom to controller\n" );
						}

						GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
						if (missionController != NULL)
						{
							Commands->Send_Custom_Event( obj, missionController, 0, M01_END_BARN_ROUNDUP_OBJECTIVE_JDG, 0 );
						}
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_Barn_Prisoner_01_JDG, "")//this guys ID is M01_BARN_PRISONER_01_ID 101442--this is female
{
	bool talkingToHavoc;
	bool chinookOnWay;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(talkingToHavoc, 1);
		SAVE_VARIABLE(chinookOnWay, 2);
	}

	void Created( GameObject * obj ) override 
	{
		talkingToHavoc = false;
		chinookOnWay = false;
		Commands->Innate_Disable(obj);

		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
		params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2B", true );
		Commands->Action_Play_Animation ( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override //
	{
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj );
		//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CIVILIAN_KILLED_JDG, 0 );

		if (talkingToHavoc == true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CUE_BABUSHKA_CONVERSATION_JDG, 2 );
		}

		GameObject * evacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (evacController != NULL)
		{
			Commands->Send_Custom_Event( obj, evacController, 0, M01_CIVILIAN_KILLED_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_MODIFY_YOUR_ACTION_JDG)//babushka runs to hide spot
		{
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2C", false );
			Commands->Action_Play_Animation ( obj, params );	
		}

		else if (param == M01_MODIFY_YOUR_ACTION_04_JDG)//you are now talking to havoc in your shed
		{
			talkingToHavoc = true;
		}

		else if (param == M01_MODIFY_YOUR_ACTION_05_JDG)//your havoc conversation is now over
		{
			talkingToHavoc = false;
		}

		else if (param == M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG)//your ride is on the way--disable hibernation
		{
			chinookOnWay = true;
			Commands->Enable_Hibernation( obj, false );
		}

		else if (param == M01_GOING_TO_EVAC_SPOT02_JDG)
		{
			Commands->Enable_Hibernation(obj, false );
			Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG ) );
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evacSpot, RUN, 1 );
				
			Commands->Action_Goto( obj, params );
			Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_GDI );

			//Commands->Set_Obj_Radar_Blip_Shape ( obj, RADAR_BLIP_SHAPE_OBJECTIVE );
			Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_NEUTRAL );

			GameObject * babushkazone = Commands->Find_Object ( 101662 );
			if (babushkazone != NULL)//kill babushka talk zone
			{
				Commands->Destroy_Object ( babushkazone );
			}
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_02_JDG)//civ is at hiding spot--crouch
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );

				params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
				params.Set_Attack (STAR, 0, 0, true);
				params.Set_Animation ("S_A_HUMAN.H_A_A0C0", true);
				Commands->Action_Play_Animation (obj, params);

				if (Commands->Find_Object ( 101662 ))//sending custom to babushka conversation zone--okay to play conversation
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101662 ), 0, M01_START_ACTING_JDG, 0 );
				}

				if (chinookOnWay == false)
				{
					Commands->Enable_Hibernation( obj, true );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//civ is at evac chopper--get on
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				//GameObject *spawn_effect = Commands->Create_Object( "Spawner Created Special Effect", powerupSpawnLocation );

				//if (spawn_effect)
				//{
				//	Commands->Attach_Script( spawn_effect, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				//}

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );
				//Commands->Give_Points( STAR, 2000, false );

				GameObject * airdropController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG )//pow guy is standingup
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "S_A_HUMAN.H_A_HOST_L1C", false );
				Commands->Action_Play_Animation ( obj, params );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//play the Havoc conversation
			{
				Commands->Enable_Hibernation( obj, false );
				Commands->Action_Reset ( obj, 100 );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( Vector3(-51.0f, 214.2f, 5.5f), 1.0f, 0.5f );
				Commands->Action_Goto( obj, params );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Barn_Prisoner_02_JDG, "")//this guys ID is M01_BARN_PRISONER_02_ID 101444--this is Billy
{
	bool chinookOnWay;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(chinookOnWay, 1);
	}

	void Created( GameObject * obj ) override 
	{
		chinookOnWay = false;
		Commands->Innate_Disable(obj);

		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
		params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2B", true );
		Commands->Action_Play_Animation ( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj );
		//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CIVILIAN_KILLED_JDG, 0 );

		GameObject * evacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (evacController != NULL)
		{
			Commands->Send_Custom_Event( obj, evacController, 0, M01_CIVILIAN_KILLED_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_MODIFY_YOUR_ACTION_JDG)//goto your hiding spot
		{
			Commands->Enable_Hibernation( obj, false );
			Commands->Action_Reset ( obj, 100 );

			params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2C", false );
			Commands->Action_Play_Animation ( obj, params );

			
		}

		else if (param == M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG)//your ride is on the way--disable hibernation
		{
			chinookOnWay = true;
			Commands->Enable_Hibernation( obj, false );
		}

		else if (param == M01_GOING_TO_EVAC_SPOT02_JDG)
		{
			Commands->Enable_Hibernation(obj, false );
			Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG ) );
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evacSpot, RUN, 1 );
				
			Commands->Action_Goto( obj, params );

			GameObject * billyszone = Commands->Find_Object ( 101661 );
			if (billyszone != NULL)//kill billy's talk zone
			{
				Commands->Destroy_Object ( billyszone );
			}
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)//billys at his hide spot
			{
				if (Commands->Find_Object ( 101661 ))//sending custom to billys conversation zone--okay to play conversation
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101661 ), 0, M01_START_ACTING_JDG, 0 );
				}

				if (chinookOnWay == false)
				{
					Commands->Enable_Hibernation( obj, true );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//civ is at evac chopper--get on
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );
				Commands->Give_Points( STAR, 2000, false );

				GameObject * airdropController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG )//pow guy is standingup
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "S_A_HUMAN.H_A_HOST_L1C", false );
				Commands->Action_Play_Animation ( obj, params );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//play the Havoc conversation
			{
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(-73.572f, 138.013f, 0.329f), RUN, .5f );
				
				Commands->Action_Goto( obj, params );
				Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_GDI );

				//Commands->Set_Obj_Radar_Blip_Shape ( obj, RADAR_BLIP_SHAPE_OBJECTIVE );
				Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_NEUTRAL );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Barn_Prisoner_03_JDG, "")//this guys ID is M01_BARN_PRISONER_03_ID 101443--this is pierre
{
	bool chinookOnWay;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(chinookOnWay, 1);
	}

	void Created( GameObject * obj ) override 
	{
		chinookOnWay = false;
		//Commands->Innate_Disable(obj);
		Commands->Set_Innate_Is_Stationary ( obj, true );

		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
		params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2B", true );
		Commands->Action_Play_Animation ( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Sound ( "EVA_Civilian_Killed", Vector3 (0,0,0), obj );
		//Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_CIVILIAN_KILLED_JDG, 0 );

		GameObject * evacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
		if (evacController != NULL)
		{
			Commands->Send_Custom_Event( obj, evacController, 0, M01_CIVILIAN_KILLED_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_MODIFY_YOUR_ACTION_JDG)//guy runs to back shed
		{
			Commands->Set_Innate_Is_Stationary ( obj, false );
			Commands->Enable_Hibernation( obj, false );
			Commands->Action_Reset ( obj, 100 );

			params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation( "S_A_HUMAN.H_A_HOST_L2C", false );
			Commands->Action_Play_Animation ( obj, params );

			
		}//M01_MODIFY_YOUR_ACTION_11_JDG

		else if (param == M01_MODIFY_YOUR_ACTION_11_JDG)//babushka is talking ot havoc--turn and face him
		{
			Commands->Action_Reset ( obj, 100 );
			//ActionParamsStruct params;
			//params.Set_Basic(this, 100, M01_MODIFY_YOUR_ACTION_JDG);
			//params.Set_Look( STAR, 15 );
			//params.Set_Attack( STAR, 0, 0, true );
			//Commands->Action_Attack ( obj, params );

			//Commands->Action_Face_Location ( obj, params );

			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_MODIFY_YOUR_ACTION_JDG);
			//params.Set_Look( STAR, 15 );
			params.Set_Attack( STAR, 0, 0, true );
			Commands->Action_Attack ( obj, params );
		}

		else if (param == M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG)//your ride is on the way--disable hibernation
		{
			chinookOnWay = true;
			Commands->Enable_Hibernation( obj, false );
			Commands->Set_Innate_Is_Stationary ( obj, false );
		}

		else if (param == M01_GOING_TO_EVAC_SPOT02_JDG)
		{
			Commands->Debug_Message ( "***************************barn civ 3 has received evac chopper call--should now be going to evac spot\n" );
			Commands->Enable_Hibernation(obj, false );
			Commands->Set_Innate_Is_Stationary ( obj, false );

			Vector3 evacSpot = Commands->Get_Position ( Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG ) );
			ActionParamsStruct params;
			params.Set_Basic(this, 100, M01_GOING_TO_EVAC_SPOT_JDG);
			params.Set_Movement( evacSpot, RUN, 1 );
					
			Commands->Action_Goto( obj, params );
			
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)//civ is at hiding spot--crouch
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				ActionParamsStruct params;
				params.Set_Basic(this, 100, M01_MODIFY_YOUR_ACTION_JDG);
				params.Set_Look( STAR, 15 );
				//params.Set_Attack( STAR, 0, 0, true );
				//Commands->Action_Attack ( obj, params );

				Commands->Action_Face_Location ( obj, params );

				if (Commands->Find_Object ( 101662 ))//sending custom to babushka conversation zone--okay to play conversation
				{
					Commands->Send_Custom_Event( obj, Commands->Find_Object ( 101662 ), 0, M01_START_ACTING_JDG, 0 );
				}

				if (chinookOnWay == false)
				{
					Commands->Enable_Hibernation( obj, true );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG )//civ is at evac chopper--get on
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				//GameObject *spawn_effect = Commands->Create_Object( "Spawner Created Special Effect", powerupSpawnLocation );

				//if (spawn_effect)
				//{
				//	Commands->Attach_Script( spawn_effect, "M00_Create_Anim_Effect_DAY", "C4_EX1A.C4_EX1A");
				//}

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );
				//Commands->Give_Points( STAR, 2000, false );

				GameObject * airdropController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( M01_BARNAREA_EVAC_MONITOR_JDG );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG )//pow guy is standingup
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "S_A_HUMAN.H_A_HOST_L1C", false );
				Commands->Action_Play_Animation ( obj, params );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//play the Havoc conversation
			{
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( Vector3(-56.8f, 213.2f, 5.5f), 1.0f, 0.5f );
				Commands->Action_Goto( obj, params );
				Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_GDI );

				//Commands->Set_Obj_Radar_Blip_Shape ( obj, RADAR_BLIP_SHAPE_OBJECTIVE );
				Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_NEUTRAL );
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDIBasePOW_Air_Evac_Rope_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Debug_Message ( "**********************script attached to evac rope\n" );
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_ROPE_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************rope has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};



DECLARE_SCRIPT(M01_GDIBasePOW_Air_Evac_Waypath_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_WAYPATH_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************waypath has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};








DECLARE_SCRIPT(M01_Evac_TroopBone_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_troopBN.XG_EV5_troopBN") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}
};



DECLARE_SCRIPT(M01_GDIBaseCommander_Air_Evac_Waypath_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( 106694 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************waypath has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_GDIBaseCommander_Air_Evac_Rope_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Debug_Message ( "**********************script attached to evac rope\n" );
		Commands->Enable_Hibernation( obj, false );
		GameObject * myController = Commands->Find_Object ( 106694 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************rope has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};









DECLARE_SCRIPT(M01_TriggerZone_GDIBase_BaseCommander_JDG, "")//106267 and 106268
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * gdiBaseCommander = Commands->Find_Object ( 106050 );

			if (gdiBaseCommander != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiBaseCommander, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * zone01 = Commands->Find_Object ( 106267 );//trigger zone 01 for base commander
			if (zone01 != NULL)
			{
				Commands->Destroy_Object ( zone01 );//this is a one-time only zone--cleaning up
			}

			GameObject * zone02 = Commands->Find_Object ( 106268 );//trigger zone 02 for base commander
			if (zone02 != NULL)
			{
				Commands->Destroy_Object ( zone02 );//this is a one-time only zone--cleaning up
			}

			GameObject * zone03 = Commands->Find_Object ( 105615 );//trigger zone 01 for this is the path lets go
			if (zone03 != NULL)
			{
				Commands->Destroy_Object ( zone03 );//this is a one-time only zone--cleaning up
			}

			GameObject * zone04 = Commands->Find_Object ( 113733 );//trigger zone 02 for this is the path lets go
			if (zone04 != NULL)
			{
				Commands->Destroy_Object ( zone04 );//this is a one-time only zone--cleaning up
			}

			GameObject * pow01 = Commands->Find_Object ( 103328 );
			GameObject * pow02 = Commands->Find_Object ( 103327 );

			if (pow01 != NULL && pow02 != NULL)
			{
				Commands->Set_Objective_Status( M01_GDI_BASE_POW_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
			}

			if (pow01 != NULL)
			{
				Commands->Destroy_Object ( pow01 );
			}

			if (pow02 != NULL)
			{
				Commands->Destroy_Object ( pow02 );
			}

			GameObject * pow_guard01 = Commands->Find_Object ( 103334 );
			if (pow_guard01 != NULL)
			{
				Commands->Destroy_Object ( pow_guard01 );
			}

			GameObject * pow_guard02 = Commands->Find_Object ( 103329 );
			if (pow_guard02 != NULL)
			{
				Commands->Destroy_Object ( pow_guard02 );
			}

			GameObject * pow_guard03 = Commands->Find_Object ( 103330 );
			if (pow_guard03 != NULL)
			{
				Commands->Destroy_Object ( pow_guard03 );
			}

			GameObject * pow_guard04 = Commands->Find_Object ( 103332 );
			if (pow_guard04 != NULL)
			{
				Commands->Destroy_Object ( pow_guard04 );
			}

			GameObject * pow_triggerzone = Commands->Find_Object ( 114348 );//
			if (pow_triggerzone != NULL)
			{
				Commands->Destroy_Object ( pow_triggerzone );
			}

			GameObject * gdi_minigunner_escort = Commands->Find_Object ( 104685 );//M01_GDI_BASE_POWS_OVER_JDG
			if (gdi_minigunner_escort != NULL)
			{
				Commands->Send_Custom_Event( obj, gdi_minigunner_escort, 0, M01_GDI_BASE_POWS_OVER_JDG, 0 );
				Commands->Send_Custom_Event( obj, gdi_minigunner_escort, 0, M01_WALKING_WAYPATH_05_JDG, 0 );
			}

			//GameObject * gdi_grenadier_escort = Commands->Find_Object ( 104686 );
			//if (gdi_grenadier_escort != NULL)
			//{
			//	Commands->Send_Custom_Event( obj, gdi_grenadier_escort, 0, M01_GDI_BASE_POWS_OVER_JDG, 0 );
			//}

		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_FirstChinookFlamethrowerGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		GameObject * gdiGuy01 = Commands->Find_Object ( 104685 );
		GameObject * gdiGuy02 = Commands->Find_Object ( 104686 );

		if (gdiGuy01 != NULL)
		{
			Commands->Send_Custom_Event( obj, gdiGuy01, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 0 );
		}

		if (gdiGuy02 != NULL)
		{
			Commands->Send_Custom_Event( obj, gdiGuy02, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 0 );
		}
	}
};



/*DECLARE_SCRIPT(M01_GDIBase_SecondChinookFlamethrowerGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		GameObject * powController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (powController != NULL)
		{
			Commands->Send_Custom_Event( obj, powController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.h_a_troopdrop") == 0)
		{
			Vector3 myHomeSpot (-22.604f, 25.125f, 1.605f);
			ActionParamsStruct params;
			params.Set_Basic(this, 60, M01_START_ATTACKING_03_JDG);
			params.Set_Movement(myHomeSpot, RUN, 5);
			Commands->Action_Attack (obj, params);

			Commands->Set_Innate_Soldier_Home_Location ( obj, myHomeSpot, 10 );
		}
	}

	void Killed( GameObject * obj, GameObject * killer ) override
	{
		GameObject * powController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (powController != NULL)
		{
			Commands->Send_Custom_Event( obj, powController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_SecondChinookMinigunnerGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		GameObject * powController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (powController != NULL)
		{
			Commands->Send_Custom_Event( obj, powController, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.h_a_troopdrop") == 0)
		{
			Vector3 myHomeSpot (-16.144f, 19.274f, 2.312f);
			ActionParamsStruct params;
			params.Set_Basic(this, 60, M01_START_ATTACKING_03_JDG);
			params.Set_Movement(myHomeSpot, RUN, 2);
			Commands->Action_Attack (obj, params);

			Commands->Set_Innate_Soldier_Home_Location ( obj, myHomeSpot, 10 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				params.Set_Basic(this, 100, M01_START_ATTACKING_03_JDG);
				params.Set_Movement(STAR, RUN, 5);
				params.Set_Attack( STAR, 15, 1, true );
				Commands->Action_Attack (obj, params);
			}
		}
	}

	void Killed( GameObject * obj, GameObject * killer ) override
	{
		GameObject * powController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (powController != NULL)
		{
			Commands->Send_Custom_Event( obj, powController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}
};*/
















DECLARE_SCRIPT(M01_TibCave_StartZone_JDG, "")//108922
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * visceroid01 = Commands->Find_Object ( 106908 );
			GameObject * visceroid02 = Commands->Find_Object ( 106909 );
			GameObject * visceroid03 = Commands->Find_Object ( 106907 );
			GameObject * visceroidNodGuy01 = Commands->Find_Object ( 106910 );
			GameObject * visceroidNodGuy02 = Commands->Find_Object ( 106911 );

			if (visceroid01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, visceroid01, 0, M01_START_ACTING_JDG, 0 );
			}

			if (visceroid02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, visceroid02, 0, M01_START_ACTING_JDG, 0 );
			}

			if (visceroid03 != NULL)
			{
				Commands->Send_Custom_Event ( obj, visceroid03, 0, M01_START_ACTING_JDG, 0 );
			}

			if (visceroidNodGuy01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, visceroidNodGuy01, 0, M01_START_ACTING_JDG, 0 );
			}

			if (visceroidNodGuy02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, visceroidNodGuy02, 0, M01_START_ACTING_JDG, 1 );
			}

			Commands->Destroy_Object ( obj );//one time zone--cleaning up
		}
	}
};


DECLARE_SCRIPT(M01_Visceroid01_JDG, "")//106908
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Innate_Enable(obj);
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

				GameObject * nodguy01 = Commands->Find_Object ( 106910 );

				if (nodguy01 != NULL)
				{
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_START_ACTING_JDG );
					params.Set_Attack( nodguy01, 10, 0, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack ( obj, params );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Visceroid02_JDG, "")//106909
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Innate_Enable(obj);
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

				GameObject * nodguy01 = Commands->Find_Object ( 106910 );

				if (nodguy01 != NULL)
				{
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_START_ACTING_JDG );
					params.Set_Attack( nodguy01, 10, 0, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack ( obj, params );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Visceroid03_JDG, "")//106907
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Innate_Enable(obj);
				Vector3 myPosition = Commands->Get_Position ( obj );
				//Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

				ActionParamsStruct params;
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( STAR, 10, 0, true );
				params.Set_Movement( STAR, RUN, 2 );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Visceroid_NodGuy01_JDG, "")//106910
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Innate_Enable(obj);
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

				Commands->Create_Sound ( "MX1DSGN_DSGN0017I1DSGN_SND", myPosition, obj );

				GameObject * visceroid01 = Commands->Find_Object ( 106908 );
				if (visceroid01 != NULL)
				{
					ActionParamsStruct params;
					params.Set_Basic( this, 90, M01_START_ACTING_JDG );
					params.Set_Attack( visceroid01, 25, 0, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack ( obj, params );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Visceroid_NodGuy02_JDG, "")//106911
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Innate_Enable(obj);
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );

				Commands->Create_Sound ( "MX1DSGN_DSGN0018I1DSGN_SND", myPosition, obj );

				GameObject * visceroid01 = Commands->Find_Object ( 106908 );

				if (visceroid01 != NULL)
				{
					ActionParamsStruct params;
					params.Set_Basic( this, 90, M01_START_ACTING_JDG );
					params.Set_Attack( visceroid01, 25, 0, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack ( obj, params );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Tiberium_Cave_Spawn_Helicopter_Zone_JDG, "")//M01_TIB_TUNNEL_HELICOPTER_ZONE_JDG...this guys ID is 100955
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SPAWN_TIB_CAVE_HELICOPTER_JDG, 0 );

			int apacheConv = Commands->Create_Conversation( "M01_Announce_Apache_Conversation", 100, 1000, false);
			Commands->Join_Conversation( NULL, apacheConv, false, false, true );
			Commands->Join_Conversation( STAR, apacheConv, false, false, true );
			Commands->Start_Conversation( apacheConv,  apacheConv );

			GameObject * tunnelGuy = Commands->Find_Object ( 103274 );
			if (tunnelGuy != NULL)
			{
				Commands->Send_Custom_Event( obj, tunnelGuy, 0, M01_START_ACTING_JDG, 0 );
			}

			Commands->Destroy_Object ( obj );
		}	
	
	}	
};


DECLARE_SCRIPT(M01_TiberiumCave_UpThere_NodGuy_JDG, "")//103274
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				ActionParamsStruct params;
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( STAR, 0, 0, true );

				Commands->Action_Attack (obj, params);

				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 3 );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Create_Sound ( "MX1DSGN_DSGN0019I1DSGN_SND", myPosition, obj );

				ActionParamsStruct params;
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( STAR, 50, 0, true );

				Commands->Action_Attack (obj, params);
			}
		}
	}
};

DECLARE_SCRIPT(M01_Detention_GuardTower_Enter_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * detentionCiv01 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_01_JDG );
			if (detentionCiv01 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv01, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * detentionCiv02 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_02_JDG );
			if (detentionCiv02 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv02, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * detentionCiv03 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_03_JDG );
			if (detentionCiv03 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv03, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * detentionGDI01 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_01_JDG );
			if (detentionGDI01 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI01, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * detentionGDI02 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_02_JDG );
			if (detentionGDI02 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI02, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject * detentionGDI03 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_03_JDG );
			if (detentionGDI03 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI03, 0, M01_START_ACTING_JDG, 0 );
			}
		}
	}

	void Exited( GameObject * obj, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			GameObject * detentionCiv01 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_01_JDG );
			if (detentionCiv01 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv01, 0, M01_GOTO_IDLE_JDG, 0 );
			}

			GameObject * detentionCiv02 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_02_JDG );
			if (detentionCiv02 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv02, 0, M01_GOTO_IDLE_JDG, 0 );
			}

			GameObject * detentionCiv03 = Commands->Find_Object ( M01_DETENTION_CIVILIAN_03_JDG );
			if (detentionCiv03 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionCiv03, 0, M01_GOTO_IDLE_JDG, 0 );
			}

			GameObject * detentionGDI01 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_01_JDG );
			if (detentionGDI01 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI01, 0, M01_GOTO_IDLE_JDG, 0 );
			}

			GameObject * detentionGDI02 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_02_JDG );
			if (detentionGDI02 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI02, 0, M01_GOTO_IDLE_JDG, 0 );
			}

			GameObject * detentionGDI03 = Commands->Find_Object ( M01_DETENTION_GDI_GUY_03_JDG );
			if (detentionGDI03 != NULL)
			{
				Commands->Send_Custom_Event( obj, detentionGDI03, 0, M01_GOTO_IDLE_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_PrisonPen_POW_JDG, "")
{
	bool okayToReact;
	bool heardHavocAlert;
	bool gateIsDown;
	bool sam_is_destroyed;

	int pow_guy01_id;//101927

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(okayToReact, 1);
		SAVE_VARIABLE(heardHavocAlert, 2);
		SAVE_VARIABLE(gateIsDown, 3);
		SAVE_VARIABLE(sam_is_destroyed, 4);
		SAVE_VARIABLE(pow_guy01_id, 5);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_GDI );
		pow_guy01_id = 101927;
		okayToReact = false;
		heardHavocAlert = false;
		gateIsDown = false;
		sam_is_destroyed = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );
		Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, false );
		Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
		Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );
	}

	void Killed( GameObject * /* obj */, GameObject * /* killer */ ) override
	{
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_START_ACTING_JDG: 
				{
					Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, true );
					Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, true );
					okayToReact = true;
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
					Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );
					okayToReact = false;
				}
				break;

			case M01_FACING_SPECIFIED_DIRECTION_01_JDG: 
				{
					Commands->Innate_Disable(obj);
					params.Set_Basic( this, 100, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
					params.Set_Attack( STAR, 0, 0, true );
					
					Commands->Action_Attack ( obj, params );

					float delayTimer = Commands->Get_Random ( 0.25f, 2.5f );
					Commands->Send_Custom_Event( obj, obj, 0, M01_DOING_ANIMATION_01_JDG, delayTimer );
				}
				break;

			case M01_DOING_ANIMATION_01_JDG: 
				{
					const char* animationName = M01_Choose_Cheer_Animation ( );

					params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
					params.Set_Animation( animationName, false );

					Commands->Action_Play_Animation ( obj, params );
				}
				break;
		}
	}

	void Poked( GameObject * obj, GameObject * /* poker */ ) override
	{
		if (gateIsDown == true)
		{
			if (sam_is_destroyed == false)
			{
				int apacheConv = Commands->Create_Conversation( "M01_Detention_GDI_Poke_Conversation01", 100, 1000, false);
				Commands->Join_Conversation( obj, apacheConv, false, true, true );
				Commands->Start_Conversation( apacheConv,  apacheConv );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_02_JDG)
		{
			GameObject * gdipow01 = Commands->Find_Object ( pow_guy01_id );//
			if (obj == gdipow01)
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				int apacheConv = Commands->Create_Conversation( "M01_Detention_GDI_Hello_Conversation01", 100, 1000, false);
				Commands->Join_Conversation( obj, apacheConv, false, true, true );
				Commands->Start_Conversation( apacheConv,  apacheConv );
			}
		}

		else if (action_id == M01_DOING_ANIMATION_01_JDG)
		{
			GameObject * gdipow01 = Commands->Find_Object ( pow_guy01_id );
			if (obj == gdipow01)
			{
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( Vector3 (-297.801f, 572.799f, 27.458f), RUN, 1);
				Commands->Action_Goto ( obj, params );
			}
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG)
		{
			const char* animationName = M01_Choose_Cheer_Animation ( );
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
			params.Set_Animation( animationName, false );

			Commands->Action_Play_Animation ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_04_JDG)
		{
			const char* animationName = M01_Choose_Cheer_Animation ( );
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_04_JDG );
			params.Set_Animation( animationName, false );

			Commands->Action_Play_Animation ( obj, params );

			GameObject * gdipow01 = Commands->Find_Object ( pow_guy01_id );

			if (obj == gdipow01)
			{
				int apacheConv = Commands->Create_Conversation( "M01_Detention_GDI_Evac_Conversation01", 100, 1000, false);
				Commands->Join_Conversation( obj, apacheConv, true, true, true );
				Commands->Start_Conversation( apacheConv,  apacheConv );
			}
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );
		}

		else if (action_id == M01_DOING_ANIMATION_04_JDG)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_DO_END_MISSION_CHECK_JDG, 5 );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
			params.Set_Movement( Vector3 (-328.136f, 548.518f, 30.044f), RUN, 3);
			Commands->Action_Goto ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_05_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );
			
			GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
			if (missionController != NULL)
			{
				Commands->Send_Custom_Event( obj, missionController, 0, M01_DO_END_MISSION_CHECK_JDG, 5 );
			}
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		ActionParamsStruct params;

		if (sound.Type == M01_DETENTION_GATE_IS_DOWN_JDG && gateIsDown == false)
		{
			gateIsDown = true;
			Commands->Set_Innate_Is_Stationary ( obj, false );
			Commands->Action_Reset ( obj, 100 );

			GameObject * commCenterSamSite = Commands->Find_Object ( M01_COMMCENTER_SAM_JDG );
			if (commCenterSamSite != NULL)
			{
				Vector3 myGotoSpot;

				int myIdNumber = Commands->Get_ID ( obj );

				if (myIdNumber == M01_DETENTION_GDI_GUY_01_JDG)
				{
					myGotoSpot.Set (-312.413f, 577.030f, 27.567f);
				}

				else if (myIdNumber == M01_DETENTION_GDI_GUY_02_JDG)
				{
					myGotoSpot.Set (-311.138f, 578.398f, 27.687f);
				}

				else
				{
					myGotoSpot.Set (-309.667f, 576.016f, 27.753f);
				}

				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_03_JDG );
				params.Set_Movement( myGotoSpot, RUN, 0.5f);
				Commands->Action_Goto ( obj, params );
			}
		}

		else if (sound.Type == M01_DETENTION_GATE_DOWN_SAM_DEAD_JDG)
		{
			sam_is_destroyed = true;
			Commands->Set_Innate_Is_Stationary ( obj, false );
			Commands->Action_Reset ( obj, 100 );

			Vector3 myGotoSpot;

			int myIdNumber = Commands->Get_ID ( obj );

			if (myIdNumber == M01_DETENTION_GDI_GUY_01_JDG)
			{
				myGotoSpot.Set (-312.413f, 577.030f, 27.567f);
			}

			else if (myIdNumber == M01_DETENTION_GDI_GUY_02_JDG)
			{
				myGotoSpot.Set (-311.138f, 578.398f, 27.687f);
			}

			else
			{
				myGotoSpot.Set (-309.667f, 576.016f, 27.753f);
			}

			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_03_JDG );
			params.Set_Movement( myGotoSpot, RUN, 0.5f);
			Commands->Action_Goto ( obj, params );
			
		}

		else if (sound.Creator == STAR && okayToReact == true && heardHavocAlert == false)
		{
			if (sound.Type == SOUND_TYPE_GUNSHOT)
			{
				heardHavocAlert = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 0 );
			}

			else if (sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				heardHavocAlert = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_PrisonPen_Civilian_JDG, "")
{
	Vector3 wanderSpot[10];

	int gdi_pow_id;
	int my_id;
	int prisoner_2_id;
	int prisoner_3_id;
	int last;

	float loiterTimeMin;
	float loiterTimeMax;

	bool okayToReact;
	bool heardHavocAlert;
	bool gateIsDown;
	bool sam_is_destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gdi_pow_id, 1);
		SAVE_VARIABLE(my_id, 2);
		SAVE_VARIABLE(prisoner_2_id, 3);
		SAVE_VARIABLE(prisoner_3_id, 4);
		SAVE_VARIABLE(last, 5);
		SAVE_VARIABLE(loiterTimeMin, 6);
		SAVE_VARIABLE(loiterTimeMax, 7);
		SAVE_VARIABLE(okayToReact, 8);
		SAVE_VARIABLE(heardHavocAlert, 9);
		SAVE_VARIABLE(gateIsDown, 10);
		SAVE_VARIABLE(sam_is_destroyed, 11);
	}

	void Created( GameObject * obj ) override 
	{
		okayToReact = false;
		heardHavocAlert = false;
		gateIsDown = false;
		sam_is_destroyed = false;
		Commands->Innate_Soldier_Enable_Enemy_Seen ( obj, false );
		Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
		Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );

		wanderSpot[0].Set (-313.881f, 589.854f, 27.457f);
		wanderSpot[1].Set (-303.287f, 587.881f, 27.457f);
		wanderSpot[2].Set (-296.290f, 595.483f, 27.458f);
		wanderSpot[3].Set (-295.055f, 589.244f, 27.458f);
		wanderSpot[4].Set (-309.122f, 578.526f, 27.741f);
		wanderSpot[5].Set (-299.833f, 578.027f, 27.735f);
		wanderSpot[6].Set (-297.657f, 571.844f, 27.458f);
		wanderSpot[7].Set (-289.746f, 578.258f, 27.781f);
		wanderSpot[8].Set (-280.325f, 579.701f, 27.458f);
		wanderSpot[9].Set (-286.094f, 585.007f, 27.720f);

		loiterTimeMin = 5;
		loiterTimeMax = 30;
		last = 20;

		gdi_pow_id = 101927;
		my_id = Commands->Get_ID ( obj );

		if (my_id == 101929)
		{
			prisoner_2_id = 101930;
			prisoner_3_id = 101931;
		}

		else if (my_id == 101930)
		{
			prisoner_2_id = 101929;
			prisoner_3_id = 101931;
		}

		else if (my_id == 101931)
		{
			prisoner_2_id = 101929;
			prisoner_3_id = 101930;
		}

		Commands->Start_Timer ( obj, this, 5, M01_PICK_A_NEW_LOCATION_JDG );
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		ActionParamsStruct params;

		int random = int(Commands->Get_Random(0, 13-WWMATH_EPSILON));
		
		while (random == last) 
		{
			random = int(Commands->Get_Random(0, 13-WWMATH_EPSILON));
		}

		if (random <= 9)
		{
			Vector3 shinGotoSpot = wanderSpot[random];

			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( shinGotoSpot, WALK, 1 );
			Commands->Action_Goto( obj, params );

			loiterTimeMin = 5;
			loiterTimeMax = 30;
		}

		else if (random == 10 && Commands->Find_Object ( prisoner_2_id ))
		{
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Commands->Find_Object ( prisoner_2_id ), WALK, 2 );
			Commands->Action_Goto( obj, params );

			loiterTimeMin = 5;
			loiterTimeMax = 30;
		}

		else if (random == 11 && Commands->Find_Object ( prisoner_3_id ))
		{
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Commands->Find_Object ( prisoner_3_id ), WALK, 2 );
			Commands->Action_Goto( obj, params );

			loiterTimeMin = 5;
			loiterTimeMax = 30;
		}

		else
		{
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
			params.Set_Movement( Commands->Find_Object ( gdi_pow_id ), WALK, 2 );
			Commands->Action_Goto( obj, params );

			loiterTimeMin = 1;
			loiterTimeMax = 3;
		}

		last = random;
	}

	void Poked( GameObject * obj, GameObject * /* poker */ ) override
	{
		if (gateIsDown == true)
		{
			if (sam_is_destroyed == false)
			{
				GameObject * babushka = Commands->Find_Object ( M01_DETENTION_CIVILIAN_01_JDG );
				GameObject * pierre = Commands->Find_Object ( M01_DETENTION_CIVILIAN_02_JDG );
				GameObject * farmerJohn = Commands->Find_Object ( M01_DETENTION_CIVILIAN_03_JDG );

				if (obj == babushka)
				{
					int apacheConv = Commands->Create_Conversation( "M01_Detention_Babushka_Poke_Conversation01", 100, 1000, false);
					Commands->Join_Conversation( obj, apacheConv, false, true, true );
					Commands->Start_Conversation( apacheConv,  apacheConv );
				}

				else if (obj == pierre)
				{
					int apacheConv = Commands->Create_Conversation( "M01_Detention_Pierre_Poke_Conversation01", 100, 1000, false);
					Commands->Join_Conversation( obj, apacheConv, false, true, true );
					Commands->Start_Conversation( apacheConv,  apacheConv );
				}

				else if (obj == farmerJohn)
				{
					int apacheConv = Commands->Create_Conversation( "M01_Detention_FarmerJohn_Poke_Conversation01", 100, 1000, false);
					Commands->Join_Conversation( obj, apacheConv, false, true, true );
					Commands->Start_Conversation( apacheConv,  apacheConv );
				}
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{
			case M01_START_ACTING_JDG: 
				{
					Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, true );
					Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, true );
					okayToReact = true;
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
					Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );
					okayToReact = false;
				}
				break;

			case M01_FACING_SPECIFIED_DIRECTION_01_JDG: 
				{
					Commands->Innate_Disable(obj);
					params.Set_Basic( this, 100, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
					params.Set_Attack( STAR, 0, 0, true );
					
					Commands->Action_Attack ( obj, params );

					float delayTimer = Commands->Get_Random ( 1.25f, 3.5f );
					Commands->Send_Custom_Event( obj, obj, 0, M01_DOING_ANIMATION_01_JDG, delayTimer );
				}
				break;

			case M01_DOING_ANIMATION_01_JDG: 
				{
					const char* animationName = M01_Choose_Cheer_Animation ( );

					params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
					params.Set_Animation( animationName, false );

					Commands->Action_Play_Animation ( obj, params );
				}
				break;
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		ActionParamsStruct params;

		if (sound.Type == M01_DETENTION_GATE_IS_DOWN_JDG && gateIsDown == false)
		{
			gateIsDown = true;
			Commands->Action_Reset ( obj, 100 );

			GameObject * commCenterSamSite = Commands->Find_Object ( M01_COMMCENTER_SAM_JDG );
			if (commCenterSamSite != NULL)
			{
				Vector3 myGotoSpot;

				int myIdNumber = Commands->Get_ID ( obj );

				if (myIdNumber == M01_DETENTION_CIVILIAN_01_JDG)//babushka
				{
					myGotoSpot.Set (-303.867f, 579.217f, 27.604f);
				}

				else if (myIdNumber == M01_DETENTION_CIVILIAN_02_JDG)//pierre
				{
					myGotoSpot.Set (-304.901f, 575.936f, 27.724f);
				}

				else//farmer john
				{
					myGotoSpot.Set (-304.887f, 582.889f, 27.457f);
				}

				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
				params.Set_Movement( myGotoSpot, RUN, 0.5f);
				Commands->Action_Goto ( obj, params );
			}
		}

		else if (sound.Type == M01_DETENTION_GATE_DOWN_SAM_DEAD_JDG )
		{
			sam_is_destroyed = true;
			Commands->Set_Innate_Is_Stationary ( obj, false );
			Commands->Action_Reset ( obj, 100 );

			Vector3 myGotoSpot;

			int myIdNumber = Commands->Get_ID ( obj );

			if (myIdNumber == M01_DETENTION_CIVILIAN_01_JDG)//babushka
			{
				myGotoSpot.Set (-303.867f, 579.217f, 27.604f);
			}

			else if (myIdNumber == M01_DETENTION_CIVILIAN_02_JDG)//pierre
			{
				myGotoSpot.Set (-304.901f, 575.936f, 27.724f);
			}

			else//farmer john
			{
				myGotoSpot.Set (-304.887f, 582.889f, 27.457f);
			}

			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
			params.Set_Movement( myGotoSpot, RUN, 0.5f);
			Commands->Action_Goto ( obj, params );
		}

		else if (sound.Creator == STAR && okayToReact == true && heardHavocAlert == false)
		{
			if (sound.Type == SOUND_TYPE_GUNSHOT)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 0 );
				heardHavocAlert = true;
			}

			else if (sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 0 );
				heardHavocAlert = true;
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			float delayTimer = Commands->Get_Random ( loiterTimeMin, loiterTimeMax );
			Commands->Start_Timer(obj, this, delayTimer, 0);
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG)
		{
			const char* animationName = M01_Choose_Cheer_Animation ( );

			params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
			params.Set_Animation( animationName, false );

			Commands->Action_Play_Animation ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG)//civ is running to gate
		{
			const char* animationName = M01_Choose_Cheer_Animation ( );
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
			params.Set_Animation( animationName, false );

			Commands->Action_Play_Animation ( obj, params );
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );
		}

		else if (action_id == M01_WALKING_WAYPATH_04_JDG)
		{
			const char* animationName = M01_Choose_Cheer_Animation ( );
			params.Set_Basic( this, 100, M01_DOING_ANIMATION_04_JDG );
			params.Set_Animation( animationName, false );

			Commands->Action_Play_Animation ( obj, params );//M01_Detention_Babushka_Evac_Conversation01

			GameObject * babushka = Commands->Find_Object ( M01_DETENTION_CIVILIAN_01_JDG );

			if (obj == babushka)
			{
				int apacheConv = Commands->Create_Conversation( "M01_Detention_Babushka_Evac_Conversation01", 100, 1000, false);
				Commands->Join_Conversation( obj, apacheConv, true, true, true );
				Commands->Join_Conversation( STAR, apacheConv, false, false, false );
				Commands->Start_Conversation( apacheConv,  apacheConv );
			}
		}

		else if (action_id == M01_DOING_ANIMATION_04_JDG)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_DO_END_MISSION_CHECK_JDG, 5 );
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
			params.Set_Movement( Vector3 (-328.136f, 548.518f, 30.044f), RUN, 3);
			Commands->Action_Goto ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_05_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );

			GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
			if (missionController != NULL)
			{
				Commands->Send_Custom_Event( obj, missionController, 0, M01_DO_END_MISSION_CHECK_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_BarnArea_NOD_Commander_Trigger_Zone_JDG, "")//M01_BARNAREA_NOD_COMMANDER_JDG
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR )
		{
			GameObject * firstFieldCommander = Commands->Find_Object (M01_BARNAREA_NOD_COMMANDER_JDG);
			if (firstFieldCommander != NULL)
			{							
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG ), 0, M01_START_ACTING_JDG, 0 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_ADD_BARN_COMMANDER_OBJECTIVE_JDG, 5 );
			}

			GameObject * zone01 = Commands->Find_Object (102361);
			GameObject * zone02 = Commands->Find_Object (103343);

			if (zone01 != NULL)
			{
				Commands->Destroy_Object ( zone01 );//one time zone
			}
			
			if (zone02 != NULL)
			{
				Commands->Destroy_Object ( zone02 );//one time zone
			}
		}
	}
};



DECLARE_SCRIPT(M01_GDI_GuardTower_NOD_Commander_JDG, "")//M01_BARNAREA_NOD_COMMANDER_JDG
{
	int guardTowerIntro;
	bool deadYet;
	bool introConvPlaying;
	bool firstTimeDamaged;
	bool playerSeen;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(guardTowerIntro, 1);
		SAVE_VARIABLE(deadYet, 2);
		SAVE_VARIABLE(introConvPlaying, 3);
		SAVE_VARIABLE(firstTimeDamaged, 4);
		SAVE_VARIABLE(playerSeen, 5);
	}

	void Created( GameObject * obj ) override 
	{
		introConvPlaying = false;
		firstTimeDamaged = true;
		playerSeen = false;
		deadYet = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );	
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			if (damager == STAR && deadYet == false && firstTimeDamaged == true)
			{	
				Vector3 myPosition = Commands->Get_Position ( obj );
				Vector3 playerPosition = Commands->Get_Position ( STAR );
				float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

				if (playerDistance >= 15)
				{
					const char* animationName = M01_Choose_Duck_Animation();
					firstTimeDamaged = false;
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
					params.Set_Animation( animationName, false );
					Commands->Action_Play_Animation (  obj, params );
				}	
			}
		}
	}

	void Enemy_Seen( GameObject * /* obj */, GameObject * enemy ) override
	{
		if (enemy == STAR)
		{
			playerSeen = true;
		}
	}

	void Sound_Heard( GameObject * obj, const CombatSound & sound ) override
	{
		if (sound.Creator == STAR)
		{
			if (/*sound.Type == SOUND_TYPE_GUNSHOT ||*/ sound.Type == SOUND_TYPE_BULLET_HIT)
			{
				if (obj)
				{
					if (deadYet == false && playerSeen == false)
					{	
						Vector3 myPosition = Commands->Get_Position ( obj );
						Vector3 playerPosition = Commands->Get_Position ( STAR );
						float playerDistance = Commands->Get_Distance ( myPosition, playerPosition );

						if (playerDistance >= 15)
						{
							const char* animationName = M01_Choose_Duck_Animation();
							//firstTimeDamaged = false;
							ActionParamsStruct params;
							params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
							params.Set_Animation( animationName, false );
							Commands->Action_Play_Animation (  obj, params );
						}	
					}
				}
			}

		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (introConvPlaying == true)
		{
			Commands->Stop_Conversation( guardTowerIntro );
		}

		GameObject * zone01 = Commands->Find_Object (102361);
		GameObject * zone02 = Commands->Find_Object (103343);
		if (zone01 != NULL)
		{
			Commands->Destroy_Object ( zone01 );//one time zone
		}
		
		if (zone02 != NULL)
		{
			Commands->Destroy_Object ( zone02 );//one time zone
		}

		deadYet = true;

		GameObject * artilleryController = Commands->Find_Object ( M01_GDIBASE_ARTILLERY_CONTROLLER_ID );
		if (artilleryController != NULL)
		{
			Commands->Send_Custom_Event( obj, artilleryController, 0, M01_GOTO_IDLE_JDG, 0 );
		}

		GameObject * objectiveController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
		if (objectiveController != NULL)
		{
			Commands->Send_Custom_Event( obj, objectiveController, 0, M01_CLEAR_FIRST_NOD_COMMANDER_PASS_JDG, 5 );
		}


		if (STAR)
		{
			int guardTowerOutro = Commands->Create_Conversation( "M01_GuardTower_Outro_Conversation", 100, 1000, false);
			Commands->Join_Conversation( NULL, guardTowerOutro, false, false, true );
			//Commands->Join_Conversation( STAR, guardTowerOutro, false, false, true );
			Commands->Start_Conversation( guardTowerOutro,  guardTowerOutro );
		}

		Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
		powerupSpawnLocation.Z += 0.75f;

		const auto& powerups = M01Vars::Powerups;
		int random = Commands->Get_Random_Int(0, 2);

		Commands->Create_Object ( powerups[random], powerupSpawnLocation );
	}

	void Action_Complete( GameObject * /* obj */, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == guardTowerIntro)
			{
				introConvPlaying = false;
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			switch (param)
			{
				case M01_START_ACTING_JDG: //here comes player start calling in reinforcements
					{
						GameObject * zone01 = Commands->Find_Object (102361);
						GameObject * zone02 = Commands->Find_Object (103343);
						if (sender == zone01)
						{
							Commands->Debug_Message ( "**********************Nod gdi tower commander has recieved command to start acting\n" );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 2 );
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, 2 );
						}

						else if (sender == zone02)
						{
							Commands->Debug_Message ( "**********************Nod gdi tower commander has recieved command to start acting\n" );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 20 );
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, 20 );
						}
						

						if (STAR && deadYet == false)
						{
							introConvPlaying = true;
							guardTowerIntro = Commands->Create_Conversation( "M01_GuardTower_Intro_Conversation", 95, 1000, true);
							Commands->Join_Conversation( NULL, guardTowerIntro, false, false, true );
							//Commands->Join_Conversation( STAR, guardTowerIntro, false, false, true );
							Commands->Start_Conversation( guardTowerIntro,  guardTowerIntro );
							Commands->Monitor_Conversation( obj, guardTowerIntro );
						}
					}
					break;

				case M01_MODIFY_YOUR_ACTION_JDG://set timer--then call in next reinforcements
					{
						if (obj)
						{
							int currentDifficulty = Commands->Get_Difficulty_Level( );
							int medium = 1;
							int hard = 2;
							float delayTimer;

							if (currentDifficulty == hard)
							{	
								delayTimer = 60;
							}

							else if (currentDifficulty == medium)
							{
								delayTimer = 90;
							}

							else
							{
								delayTimer = 120;
							}

							Commands->Send_Custom_Event( obj, obj, 0, M01_SEND_GUARD_TOWER_CHINOOK_JDG, delayTimer );
							Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );
						}
					}
					break;

				case M01_SEND_GUARD_TOWER_CHINOOK_JDG: //here comes player start calling in reinforcements
					{
						if (obj)
						{
							Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SEND_GUARD_TOWER_CHINOOK_JDG, 0 );
						}
					}
					break;

				case M01_CALL_IN_REINFORCEMENTS_JDG: //here comes player start calling in reinforcements
					{
						if (obj && deadYet == false)
						{
							GameObject * nodCommanderDialogController = Commands->Find_Object ( 103398 );
							if (nodCommanderDialogController != NULL)
							{
								Commands->Debug_Message ( "**********************Nod gdi tower commander is sending custom to dialog controller\n" );
								Commands->Send_Custom_Event( obj, nodCommanderDialogController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
							}
							float delayTimer = Commands->Get_Random ( 15, 30 ); 
							Commands->Send_Custom_Event( obj, obj, 0, M01_CALL_IN_REINFORCEMENTS_JDG, delayTimer );
						}
					}
					break;
			}
		}
	}		
};

DECLARE_SCRIPT(M01_Nod_Commander_Conversation_Controller_GDI, "")// 103398
{
	bool first_call;
	int initialCommanderConv;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(first_call, 1);
		SAVE_VARIABLE(initialCommanderConv, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		first_call = true;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override
	{
		if (param == M01_MODIFY_YOUR_ACTION_JDG)//nod commander is calling in reinforcements
		{
			GameObject * guardTower01Commander = Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG );
			if (sender == guardTower01Commander && first_call == true)
			{
				first_call = false;
				initialCommanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv04", 100, 300, true);
				Commands->Join_Conversation( sender, initialCommanderConv, false, false, false );
				Commands->Start_Conversation( initialCommanderConv,  initialCommanderConv );
				Commands->Monitor_Conversation( obj, initialCommanderConv );
			}

			else
			{
				int random = Commands->Get_Random_Int(0, 5);

				if (random == 0)
				{
					int commanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv01", 90, 200, true);
					Commands->Join_Conversation( sender, commanderConv, false, false, false );
					Commands->Start_Conversation( commanderConv,  commanderConv );
				}

				else if (random == 1)
				{
					int commanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv02", 90, 200, true);
					Commands->Join_Conversation( sender, commanderConv, false, false, false );
					Commands->Start_Conversation( commanderConv,  commanderConv );
				}

				else if (random == 2)
				{
					int commanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv03", 90, 200, true);
					Commands->Join_Conversation( sender, commanderConv, false, false, false );
					Commands->Start_Conversation( commanderConv,  commanderConv );
				}

				else if (random == 3)
				{
					int commanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv04", 90, 200, true);
					Commands->Join_Conversation( sender, commanderConv, false, false, false );
					Commands->Start_Conversation( commanderConv,  commanderConv );
				}

				else
				{
					int commanderConv = Commands->Create_Conversation( "M01_Nod_Commander_Conv05", 90, 200, true);
					Commands->Join_Conversation( sender, commanderConv, false, false, false );
					Commands->Start_Conversation( commanderConv,  commanderConv );
				}
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		if (action_id == initialCommanderConv)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_SEND_GUARD_TOWER_CHINOOK_JDG, 0 );
		}
	}

};

DECLARE_SCRIPT(M01_GDIBase_LightTank_PastTunnelZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		GameObject * lightTank = Commands->Find_Object ( M01_TAILGUNAREA_NOD_LIGHTTANK_JDG );
		if (enterer == lightTank)
		{
			Commands->Send_Custom_Event( obj, lightTank, 0, M01_GOTO_IDLE_JDG, 0 );	
			Commands->Destroy_Object ( obj );
		}
	}
};











DECLARE_SCRIPT(M01_KeyCard01_Script_JDG, "")
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			int lockeKeycard = Commands->Create_Conversation( "M01_First_Keycard_Conversation", 100, 1000, true);
			Commands->Join_Conversation( NULL, lockeKeycard, false, false, true );
			Commands->Start_Conversation( lockeKeycard,  lockeKeycard );
		}
	}
};



DECLARE_SCRIPT(M01_GDIBase_AI_ExitZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		GameObject * gdiFollower = Commands->Find_Object ( 104686 );
		GameObject * gdiCommander = Commands->Find_Object ( 106050 );
		GameObject * gdiLeader = Commands->Find_Object ( 104685 );
		GameObject * pow01 = Commands->Find_Object ( 103327 );
		GameObject * pow02 = Commands->Find_Object ( 103328 );

		if (enterer == gdiFollower)
		{
			if (gdiFollower != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiFollower, 0, M01_HOLD_YOUR_POSITION_JDG, 0 );
			}
		}

		else if (enterer == gdiCommander)
		{
			if (gdiCommander != NULL)//
			{
				Commands->Send_Custom_Event( obj, gdiCommander, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
				//Commands->Set_Objective_Status( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
			}

			if (gdiFollower != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiFollower, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
			}

			if (gdiLeader != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiLeader, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
			}
		}

		else if (enterer == pow01)
		{
			if (pow01 != NULL)
			{
				Commands->Send_Custom_Event( obj, pow01, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
			}

			if (pow02 != NULL)
			{
				Commands->Send_Custom_Event( obj, pow02, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
			}
		}
	}
};













DECLARE_SCRIPT(M01_GDIBase_LightTank_JDG, "")//M01_TAILGUNAREA_NOD_LIGHTTANK_JDG 102435
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		int tankReward = Commands->Create_Conversation( "M01_TankReward_Conversation", 100, 1000, false);
		Commands->Join_Conversation( NULL, tankReward, false, false, true );
		//Commands->Join_Conversation( STAR, tankReward, false, false, true );
		Commands->Start_Conversation( tankReward,  tankReward );
		
		Vector3 tankDeathSpot (4.2f, 279.8f, 3.2f);
		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", tankDeathSpot);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");

		Vector3 myPosition = Commands->Get_Position ( obj );
		myPosition.Z += 0.25f;
		Commands->Create_Object ( "POW_Medal_Armor", myPosition );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		switch (param)
		{						   
			case M01_START_ACTING_JDG: 
				{
					if (STAR)
					{
						int currentDifficulty = Commands->Get_Difficulty_Level( );
						
						int medium = 1;
						int hard = 2;

						float tankSpeed;
						
						if (currentDifficulty == hard)
						{
							tankSpeed = 0.6f;
						}

						else if (currentDifficulty == medium)
						{
							tankSpeed = 0.4f;
						}

						else
						{
							tankSpeed = 0.2f;
						}

						params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
						params.Set_Movement( Vector3(0,0,0), tankSpeed, 2 );
						params.WaypathID = 102233;
						//params.WaypointStartID = 102235;
						params.AttackCheckBlocked = false;
						params.AttackActive = false;
						Commands->Action_Attack( obj, params );

						Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 3 );
					}

					else
					{
						Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 3 );
					}
				}
				break;

			case M01_MODIFY_YOUR_ACTION_03_JDG: 
				{
					int currentDifficulty = Commands->Get_Difficulty_Level( );
					
					int medium = 1;
					int hard = 2;

					float tankSpeed;
					float tankRange;
					float tankAccuracy;
					
					if (currentDifficulty == hard)
					{
						tankSpeed = 0.5f;
						tankRange = 100;
						tankAccuracy = 0;
					}

					else if (currentDifficulty == medium)
					{
						tankSpeed = 0.3f;
						tankRange = 75;
						tankAccuracy = 1;
					}

					else 
					{
						tankSpeed = 0.1f;
						tankRange = 50;
						tankAccuracy = 3;
					}

					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
					params.Set_Movement( Vector3(0,0,0), tankSpeed, 2 );
					params.WaypathID = 102233;
					//params.WaypointStartID = 102235;
					params.Set_Attack( STAR, tankRange, tankAccuracy, true );
					params.AttackActive = true;

					Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_02_JDG, params, true, true );
				}
				break;
		}
	}
};





DECLARE_SCRIPT(M01_GDIBase_BackPath_NodGuy_JDG, "")//103337
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Set_Innate_Is_Stationary ( obj, false );
				ActionParamsStruct params;

				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement(Vector3 (57.651f, 54.752f, -0.449f), RUN, 1);

				Commands->Action_Goto(obj, params);
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG)
		{
			Commands->Set_Innate_Is_Stationary ( obj, true );
			params.Set_Basic( this, 100, 20 );
			params.Set_Attack( STAR, 50, 3, true );
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);
		}
	}
};















DECLARE_SCRIPT(M01_Sinking_Gunboat_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Animation ( obj, "V_GDI_GBOAT.XG_GBOAT_SINK", false, NULL, 0, 60, false );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "V_GDI_GBOAT.XG_GBOAT_SINK") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}
};


DECLARE_SCRIPT(M01_PlayerEntering_BarnArea_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject *gunboatController = Commands->Find_Object ( M01_TURRETBEACH_CONTROLLER_ID );
			if (gunboatController != NULL)
			{
				Commands->Send_Custom_Event ( obj, gunboatController, 0, M01_START_ACTING_JDG, 0 );
			}

			GameObject *hovercraftController = Commands->Find_Object ( 121630 );
			if (hovercraftController != NULL)
			{
				Commands->Send_Custom_Event ( obj, hovercraftController, 0, M01_START_ACTING_JDG, 10 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_PlayerLeaving_BarnArea_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject *gunboatController = Commands->Find_Object ( M01_TURRETBEACH_CONTROLLER_ID );
			if (gunboatController != NULL)
			{
				Commands->Send_Custom_Event ( obj, gunboatController, 0, M01_GOTO_IDLE_JDG, 0 );
			}
		}
	}
};










DECLARE_SCRIPT(M01_Barn_EntryZone_JDG, "")//113734
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)//
		{
			GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
			if (missionController != NULL)
			{
				Commands->Send_Custom_Event ( obj, missionController, 0, M01_END_BARN_OBJECTIVE_JDG, 0 );
				Commands->Destroy_Object ( obj );//one time zone--cleaning up
			}
		}
	}
};



DECLARE_SCRIPT(M01_GDI_BaseCommander_Backside_EntryZone_JDG, "")//113733
{
	bool entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		entered = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && entered == false)
		{
			entered = true;
			GameObject * gdiBaseLeaderGuy = Commands->Find_Object ( 104685 );//
			if (gdiBaseLeaderGuy != NULL)
			{
				Commands->Send_Custom_Event ( obj, gdiBaseLeaderGuy, 0, M01_MODIFY_YOUR_ACTION_10_JDG, 0 );
				Commands->Destroy_Object ( obj );//one time zone--cleaning up
			}
		}
	}
};
















DECLARE_SCRIPT(M01_Obelisk_UpdateDisc_JDG, "")
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Reveal_Encyclopedia_Building ( 12 );
			Commands->Set_HUD_Help_Text ( IDS_M00EVAG_DSGN0104I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY  );
		}
	}
};

DECLARE_SCRIPT(M01_BarnArea_EvacMonitor_JDG, "")//M01_BARNAREA_EVAC_MONITOR_JDG
{
	int unitsEvacuated;
	int evac_helicopter_id;
	int evac_waypath_id;
	int evac_rope_id;
	bool transportCalled;
	int barnAirdrop_conv;

	bool failed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(unitsEvacuated, 1);
		SAVE_VARIABLE(evac_helicopter_id, 2);
		SAVE_VARIABLE(evac_waypath_id, 3);
		SAVE_VARIABLE(evac_rope_id, 4);
		SAVE_VARIABLE(transportCalled, 5);
		SAVE_VARIABLE(barnAirdrop_conv, 6);
		SAVE_VARIABLE(failed, 7);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		unitsEvacuated = 0;
		transportCalled = false;

		failed = false;
	}

	void Action_Complete( GameObject * /* obj */, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == barnAirdrop_conv)
			{
				int honAlerted_conv = Commands->Create_Conversation( "M01_HandOfNod_Alerted_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, honAlerted_conv, false, false, true );
				//Commands->Join_Conversation( STAR, honAlerted_conv, false, false, true );
				Commands->Start_Conversation( honAlerted_conv,  honAlerted_conv );
			}
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override //
	{
		if (param == M01_CIVILIAN_KILLED_JDG)//
		{
			GameObject * civ01 = Commands->Find_Object ( M01_BARN_PRISONER_01_ID );
			GameObject * civ02 = Commands->Find_Object ( M01_BARN_PRISONER_02_ID );
			GameObject * civ03 = Commands->Find_Object ( M01_BARN_PRISONER_03_ID );

			if (sender == civ01)
			{
				failed = true;
			}

			else if (sender == civ02)
			{
				failed = true;
			}

			else if (sender == civ03)
			{
				failed = true;
			}
		}

		else if (param == M01_GOING_TO_EVAC_SPOT_JDG)
		{
			unitsEvacuated++;

			if (unitsEvacuated == 3 && failed == false)
			{
				barnAirdrop_conv = Commands->Create_Conversation( "M01_AirDrop_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, barnAirdrop_conv, false, false, true );
				Commands->Start_Conversation( barnAirdrop_conv,  barnAirdrop_conv );

				Commands->Monitor_Conversation(  obj, barnAirdrop_conv );

				Vector3 airDropSpot (-0.923f, 143.580f, 0.647f);
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", airDropSpot);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1D_GDIPowerUpDrop.txt");
			}
		}

		else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//chopper is sending you his ID
		{
			evac_helicopter_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************chopper id received.....%d\n",evac_helicopter_id );
		}

		else if (param == M01_WAYPATH_IS_SENDING_ID_JDG)//waypath is sending you his ID 
		{
			evac_waypath_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************waypath id received.....%d\n", evac_waypath_id );
		}

		else if (param == M01_ROPE_IS_SENDING_ID_JDG)//rope is sending you his ID 
		{
			evac_rope_id = Commands->Get_ID ( sender );
			Commands->Debug_Message ( "**********************rope id received.....%d\n", evac_rope_id );
		}

		else if (param == M01_GIVE_ME_A_ROPE_JDG)//someone is waiting--pull him up
		{
			Vector3 evacPosition = Commands->Get_Position ( obj );
			GameObject * troopbone = Commands->Create_Object("Invisible_Object", evacPosition );
			Commands->Set_Model ( troopbone, "XG_EV5_troopBN" );
			
			Commands->Attach_Script(troopbone, "M01_Evac_TroopBone_JDG", "");
			Commands->Innate_Disable(sender);
			Commands->Attach_To_Object_Bone( sender, troopbone, "Troop_L" );
			if(troopbone)
			{
				Commands->Set_Animation ( troopbone, "XG_EV5_troopBN.XG_EV5_troopBN", false, NULL, 0.0f, -1.0f, false);
			}
			else
			{
				Commands->Debug_Message("troopbone not created yet");
			}
			
			Commands->Set_Animation ( sender, "S_A_Human.XG_EV5_troop", false, NULL, 0.0f, -1.0f, false );
		}

		else if (param == M01_SEND_BARN_CIVILIANS_RESCUE_CHINOOK_JDG)//a gdi guy is waiting for evac--call in a helicopter
		{
			if (transportCalled == false)
			{
				transportCalled = true;

				Vector3 evacPosition = Commands->Get_Position ( obj );
				GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
				Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_BarnArea_EvacAnim.txt");
			}
		}

		else if (param == M01_EVERYONES_ON_BOARD_JDG)//chinook is ready to go--give clearance
		{
			Commands->Debug_Message ( "*******************************all barn civs on board--custom received by controller\n" );
			GameObject *evacWaypath = Commands->Find_Object ( evac_waypath_id );
			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			GameObject *evacChopper = Commands->Find_Object ( evac_helicopter_id );
			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			GameObject *evacRope = Commands->Find_Object ( evac_rope_id );
			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event( obj, evacRope, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (failed == true)
			{
				int honAlerted_conv = Commands->Create_Conversation( "M01_HandOfNod_Alerted_Conversation", 100, 1000, false);
				Commands->Join_Conversation( NULL, honAlerted_conv, false, false, true );
				//Commands->Join_Conversation( STAR, honAlerted_conv, false, false, true );
				Commands->Start_Conversation( honAlerted_conv,  honAlerted_conv );
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_RedKey_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			bool playerHasRedKey = Commands->Has_Key( STAR, 3 );
			if (playerHasRedKey == false)
			{
				Vector3 keyCardLocation (-168.195f, 527.489f, -31.130f);
				Commands->Create_Object ( "Level_03_Keycard", keyCardLocation );

				Commands->Destroy_Object ( obj );
			}

		}
	}
};

DECLARE_SCRIPT(M01_Comm_Center_Pen_Gate, "")
{
	bool samDead;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(samDead, 1);
	}

	void Created( GameObject * obj ) override 
	{
		samDead = false;
		Commands->Enable_HUD_Pokable_Indicator ( obj, true );
	}

	void Poked( GameObject * obj, GameObject * poker ) override //
	{
		if (poker == STAR && samDead == true)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_HAS_POKED_PEN_GATE_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_COMMCENTER_SAMSITE_HAS_BEEN_DESTROYED_JDG)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 5 );
			}

			else if (param == M01_START_ACTING_JDG)
			{
				samDead = true;
			}
		}
	}
};


DECLARE_SCRIPT(M01_CommCenter_SAMSite_Script, "")
{
	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == (STAR))
		{
			if (obj)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (M01_MISSION_CONTROLLER_JDG), 0, M01_COMMCENTER_SAMSITE_HAS_BEEN_DESTROYED_JDG, 0 );

		GameObject * detentionGate = Commands->Find_Object ( 101117 );
		if (detentionGate != NULL)
		{
			Commands->Send_Custom_Event( obj, detentionGate, 0, M01_COMMCENTER_SAMSITE_HAS_BEEN_DESTROYED_JDG, 0 );
		}
	}
};



DECLARE_SCRIPT(M01_Commander_Shack_Zone_JDG, "")//115985  
{
	bool commanderAlive;
	bool playerinZone;
	bool commandClearance;
	bool convStarted;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(commanderAlive, 1);
		SAVE_VARIABLE(playerinZone, 2);
		SAVE_VARIABLE(commandClearance, 3);
		SAVE_VARIABLE(convStarted, 4);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		commanderAlive = true;
		playerinZone = false;
		commandClearance = false;
		convStarted = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			playerinZone = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );

			GameObject * convController = Commands->Find_Object ( 125857 );
			if (convController != NULL)
			{
				Commands->Send_Custom_Event( obj, convController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}
	}

	void Exited( GameObject * /* obj */, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			playerinZone = false;
		}
	}	

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				commandClearance = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				 commanderAlive = false;
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG && convStarted == false)
			{
				if (playerinZone == true && commanderAlive == true && commandClearance == true)//M01_MODIFY_YOUR_ACTION_JDG
				{
					GameObject * gdiCommander = Commands->Find_Object ( 106050);
					if (gdiCommander != NULL)
					{
						convStarted = true;
						Commands->Send_Custom_Event( obj, gdiCommander, 0, M01_MODIFY_YOUR_ACTION_JDG, 1 );
					}
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0.25f );
				}
			}
		}
	}
};



DECLARE_SCRIPT(M01_TurretBeach_Engineer_JDG, "")//this guys ID is M01_TURRETBEACH_ENGINEER_ID 101654
{
	bool working_here_boss;
	bool first_time;
	float last_health;
	bool killedYet;
	bool turret01_dead;
	bool turret02_dead;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(working_here_boss, 1);
		SAVE_VARIABLE(first_time, 2);
		SAVE_VARIABLE(killedYet, 3);
		SAVE_VARIABLE(turret01_dead, 4);
		SAVE_VARIABLE(turret02_dead, 5);
	}

	void Created( GameObject * obj ) override 
	{
		turret01_dead = false;
		turret02_dead = false;
		killedYet = false;
		working_here_boss = false;
		first_time = true;
		Commands->Innate_Disable(obj);
		last_health = Commands->Get_Max_Health ( obj );
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (enemy == STAR && working_here_boss == false)
		{
			if (obj)
			{
				Commands->Action_Reset ( obj, 100 );
				Commands->Innate_Enable(obj);
				Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		int damager_id = Commands->Get_ID ( damager );

		if (damager_id != 0)
		{
			Commands->Debug_Message ( "***************************engineers damager is ID %d\n", damager_id );
		}

		else
		{
			Commands->Debug_Message ( "***************************engineers damager is NULL\n" );
		}

		GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );

		if (damager == gunboat)
		{
			Commands->Set_Health ( obj, last_health );
		}

		else if (damager == STAR)
		{
			if (obj)
			{
				last_health = Commands->Get_Health ( obj );

				if (first_time == true)
				{
					working_here_boss = false;
					Commands->Action_Reset ( obj, 100 );
					Commands->Innate_Enable(obj);

					ActionParamsStruct params;
					params.Set_Basic(this, 100, M01_DOING_ANIMATION_03_JDG);
					params.Set_Animation( "H_A_862A", false );
					Commands->Action_Play_Animation ( obj, params );
					Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );

					first_time = false;
				}

				else
				{
					working_here_boss = false;
					Commands->Action_Reset ( obj, 100 );
					Commands->Innate_Enable(obj);
					Commands->Innate_Force_State_Enemy_Seen ( obj, STAR );
				}
			}
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		killedYet = true;
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_TURRET_BEACH_ENGINEER_IS_DEAD_JDG, 5 );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		if (param == M01_START_ACTING_JDG)
		{
			Commands->Debug_Message ( "***************************original engineer custom received--start acting\n" );
			Commands->Enable_Hibernation( obj, false );
		}

		else if (param == M01_GOTO_IDLE_JDG)
		{
			Commands->Debug_Message ( "***************************original engineer custom received--STOP acting\n" );
			Commands->Enable_Hibernation( obj, true );
		}

		else if (param == M01_PLAYER_IS_ATTACKING_ME_JDG)
		{
			if (working_here_boss == false)
			{
				Commands->Debug_Message ( "***************************engineers has received custom to go heal turret\n" );
				Commands->Innate_Disable(obj);
				Commands->Action_Reset ( obj, 100 );

				GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
				GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
				float turret01_health = 0;
				float turret02_health = 0;

				if (turret01 != NULL)
				{
					turret01_health = Commands->Get_Health ( turret01 );
				}

				if (turret02 != NULL)
				{
					turret02_health = Commands->Get_Health ( turret02 );
				}

				if ((turret02_health > turret01_health) && (turret01_health > 0))
				{
					if (turret01 != NULL && turret01_dead == false)
					{
						params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
						params.Set_Movement( Vector3 (-86.5f, 95.5f, 2.3f), 1.0f, 0.5f, false );
						params.MovePathfind = false;
						Commands->Action_Goto ( obj, params );
						working_here_boss = true;
					}
				}

				else if ((turret01_health > turret02_health) && (turret02_health > 0))
				{
					if (turret02 != NULL && turret02_dead == false)
					{
						params.Set_Basic(this, 100, M01_WALKING_WAYPATH_02_JDG);
						params.Set_Movement( Vector3 (-101.5f, 111.7f, 2.3f), 1.0f, 0.5f, false );
						params.MovePathfind = false;
						Commands->Action_Goto ( obj, params );
						working_here_boss = true;
					}
				}

				else
				{
					Commands->Send_Custom_Event( obj, sender, 0, M01_BUSY_TRY_AGAIN_JDG, 1 );
				}
			}

			else if (working_here_boss == true)
			{
				Commands->Send_Custom_Event( obj, sender, 0, M01_BUSY_TRY_AGAIN_JDG, 1 );
			}
		}

		else if (param == M01_START_ATTACKING_01_JDG)
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			if (turret01 != NULL && killedYet == false && turret01_dead == false)
			{
				Vector3 sound_position = Commands->Get_Position ( turret01 );
				Commands->Create_Sound ( "CnC_Healer_Sound", sound_position, obj );
				float turretMaxHealth = Commands->Get_Max_Health ( turret01 );
				Commands->Set_Health ( turret01, turretMaxHealth );
			}

			Commands->Debug_Message ( "***************************turret engineer has fixed turret one--going back to hiding spot\n" );

			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3 (-91.1f, 106.562f, 1.7f), 1.0f, 0.25f, false );
			params.MovePathfind = false;
			Commands->Action_Goto ( obj, params );
		}

		else if (param == M01_START_ATTACKING_02_JDG)
		{
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
			if (turret02 != NULL && killedYet == false && turret02_dead == false)
			{
				Vector3 sound_position = Commands->Get_Position ( turret02 );
				Commands->Create_Sound ( "CnC_Healer_Sound", sound_position, obj );
				float turretMaxHealth = Commands->Get_Max_Health ( turret02 );
				Commands->Set_Health ( turret02, turretMaxHealth );
			}

			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3 (-91.1f, 106.562f, 1.7f), 1.0f, 0.25f, false );
			params.MovePathfind = false;
			Commands->Action_Goto ( obj, params );
		}

		else if (param == M01_TURRET_HAS_BEEN_DESTROYED_JDG)
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

			if (sender == turret01)
			{
				turret01_dead = true;
			}

			if (sender == turret02)
			{
				turret02_dead = true;
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer is at damaged turret 01--play fixing animation
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			if (turret01 != NULL)
			{
				params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
				params.Set_Attack( turret01, 50, 0, false );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );

				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_01_JDG, 1.5f );
			}
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer is at damaged turret 02--play fixing animation
		{
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
			if (turret02 != NULL)
			{
				params.Set_Basic(this, 100, M01_START_ATTACKING_02_JDG);
				params.Set_Attack( turret02, 50, 0, false );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );

				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_02_JDG, 0.5f );
			}
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer has been damaged look around
		{
			const char* animationName = M01_Choose_Duck_Animation();
			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 60, M01_DOING_ANIMATION_04_JDG);
			params.Set_Animation( animationName, false );
			Commands->Action_Play_Animation ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			working_here_boss = false;
			//Commands->Enable_Hibernation( obj, true );
		}
	}
};

DECLARE_SCRIPT(M01_TurretBeach_Chinook_Spawned_Soldier_NOD, "")
{
	bool killedYet;
	bool atTurretsYet;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(killedYet, 1);
		SAVE_VARIABLE(atTurretsYet, 2);
	}

	void Created( GameObject * obj ) override 
	{
		killedYet = false;
		atTurretsYet = false;
		GameObject *gunboatController = Commands->Find_Object ( M01_TURRETBEACH_CONTROLLER_ID );
		if (gunboatController != NULL)
		{
			Commands->Send_Custom_Event ( obj, gunboatController, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}

		GameObject *turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
		if (turret01 != NULL)
		{
			Commands->Send_Custom_Event ( obj, turret01, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}

		GameObject *turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
		if (turret02 != NULL)
		{
			Commands->Send_Custom_Event ( obj, turret02, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)
		{ 
			GameObject *turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			GameObject *turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

			if (turret01 != NULL || turret02 != NULL)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
				params.Set_Movement(Vector3 (0,0,0), RUN, 1);
				params.WaypathID = 109629;
				Commands->Action_Goto(obj, params);	

				Commands->Send_Custom_Event ( obj, obj, 0, M01_WALKING_WAYPATH_03_JDG, 5 );
			}

			else
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 45, M01_WALKING_WAYPATH_03_JDG);
				params.Set_Movement(Vector3 (0,0,0), RUN, 1);
				params.WaypathID = 109629;
				Commands->Action_Goto(obj, params);	

				//Commands->Send_Custom_Event ( obj, obj, 0, M01_WALKING_WAYPATH_03_JDG, 5 );
			}	
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (damager == STAR)
		{
			Commands->Innate_Enable(obj);
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		killedYet = true;
		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_TURRET_BEACH_ENGINEER_IS_DEAD_JDG, 5 );
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (param == M01_WALKING_WAYPATH_03_JDG && atTurretsYet == false)
		{
			GameObject *turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			GameObject *turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

			if (turret01 != NULL || turret02 != NULL)
			{
				//ActionParamsStruct params;
				//params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
				//params.Set_Movement(Vector3 (0,0,0), RUN, 1);
				//params.WaypathID = 109629;
				//Commands->Action_Goto(obj, params);	

				Commands->Send_Custom_Event ( obj, obj, 0, M01_WALKING_WAYPATH_03_JDG, 5 );
			}

			else
			{
				Commands->Action_Reset ( obj, 100 );
				//ActionParamsStruct params;
				//params.Set_Basic(this, 45, M01_WALKING_WAYPATH_03_JDG);
				//params.Set_Movement(Vector3 (0,0,0), RUN, 1);
				//params.WaypathID = 109629;
				//Commands->Action_Goto(obj, params);	

				//Commands->Send_Custom_Event ( obj, obj, 0, M01_WALKING_WAYPATH_03_JDG, 5 );
			}
		}

		else if (param == M01_START_ACTING_JDG)
		{
			Commands->Debug_Message ( "***************************new engineer custom received--start acting\n" );
			Commands->Enable_Hibernation( obj, false );
			Commands->Send_Custom_Event( obj, obj, 0, M01_CHECK_TURRETS_HEALTH_JDG, 1 );
		}

		else if (param == M01_GOTO_IDLE_JDG)
		{
			Commands->Debug_Message ( "***************************new engineer custom received--STOP acting\n" );
			Commands->Enable_Hibernation( obj, true );
		}

		else if (param == M01_CHECK_TURRETS_HEALTH_JDG)
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
			float turret01_health = 0;
			float turret02_health = 0;

			if (turret01 != NULL)
			{
				turret01_health = Commands->Get_Health ( turret01 );
			}

			if (turret02 != NULL)
			{
				turret02_health = Commands->Get_Health ( turret02 );
			}

			if (turret01_health < turret02_health)
			{
				Commands->Action_Reset ( obj, 100 );
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( Vector3 (-86.5f, 95.5f, 2.3f), 1.0f, 0.5f, false );
				//params.MovePathfind = false;
				Commands->Action_Goto ( obj, params );
			}

			if (turret02_health < turret01_health)
			{
				Commands->Action_Reset ( obj, 100 );
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_02_JDG);
				params.Set_Movement( Vector3 (-101.5f, 111.7f, 2.3f), 1.0f, 0.5f, false );
				//params.MovePathfind = false;
				Commands->Action_Goto ( obj, params );
			}
			
			else
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_CHECK_TURRETS_HEALTH_JDG, 1 );
			}
		}

		else if (param == M01_START_ATTACKING_01_JDG)
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			if (turret01 != NULL && killedYet == false)
			{
				Vector3 sound_position = Commands->Get_Position ( turret01 );
				Commands->Create_Sound ( "CnC_Healer_Sound", sound_position, obj );
				float turretMaxHealth = Commands->Get_Max_Health ( turret01 );
				Commands->Set_Health ( turret01, turretMaxHealth );
			}

			Commands->Debug_Message ( "***************************turret engineer has fixed turret one--going back to hiding spot\n" );

			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3 (-91.1f, 106.562f, 1.7f), 1.0f, 0.25f, false );
			params.MovePathfind = false;
			Commands->Action_Goto ( obj, params );
		}

		else if (param == M01_START_ATTACKING_02_JDG)
		{
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
			if (turret02 != NULL && killedYet == false)
			{
				Vector3 sound_position = Commands->Get_Position ( turret02 );
				Commands->Create_Sound ( "CnC_Healer_Sound", sound_position, obj );
				float turretMaxHealth = Commands->Get_Max_Health ( turret02 );
				Commands->Set_Health ( turret02, turretMaxHealth );
			}

			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3 (-91.1f, 106.562f, 1.7f), 1.0f, 0.25f, false );
			params.MovePathfind = false;
			Commands->Action_Goto ( obj, params );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_01_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer is at damaged turret 01--play fixing animation
		{
			GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
			if (turret01 != NULL)
			{
				params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
				params.Set_Attack( turret01, 50, 0, false );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );

				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_01_JDG, 0.5f );
			}
		}

		else if (action_id == M01_WALKING_WAYPATH_02_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer is at damaged turret 02--play fixing animation
		{
			GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
			if (turret02 != NULL)
			{
				params.Set_Basic(this, 100, M01_START_ATTACKING_02_JDG);
				params.Set_Attack( turret02, 50, 0, false );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );

				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_02_JDG, 0.5f );
			}
		}

		else if (action_id == M01_DOING_ANIMATION_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)//engineer has been damaged look around
		{
			const char* animationName = M01_Choose_Duck_Animation();
			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 60, M01_DOING_ANIMATION_04_JDG);
			params.Set_Animation( animationName, false );
			Commands->Action_Play_Animation ( obj, params );
		}

		else if (action_id == M01_WALKING_WAYPATH_03_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			atTurretsYet = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_CHECK_TURRETS_HEALTH_JDG, 1 );
		}

		else if (action_id == M01_WALKING_WAYPATH_05_JDG && complete_reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_CHECK_TURRETS_HEALTH_JDG, 1 );
		}
	}
};












DECLARE_SCRIPT(M01_FP_NodBase_Dogfight_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char *flyovers[28] =
		{
			//these are the level 01 dogfights
			"X1A_Apache_Orca_Chase_01.txt",
			"X1A_Apache_Orca_Chase_02.txt",
			"X1A_Apache_Orca_Chase_03.txt",
			"X1A_Apache_Orca_Chase_04.txt",

			//this is a low level flyover in nod base
			//it goes from the refinery to the waterfall  N->S
			//chinooks look kinda odd on this one
			"X1A_Solo_Orca_01.txt",
			"X1A_Solo_Orca_01.txt",
			"X1A_Solo_Orca_01.txt",
			"X1A_Solo_Orca_01.txt",
			"X1A_Solo_Orca_01.txt",
			"X1A_Solo_Apache_01.txt",
			"X1A_Solo_Apache_01.txt",
			"X1A_Solo_Apache_01.txt",
			"X1A_Solo_Apache_01.txt",
			"X1A_Solo_Apache_01.txt",
			"X1A_Solo_Nod_Chinook_01.txt",
			"X1A_Solo_GDI_Chinook_01.txt",
			
			//this is a low level flyover in nod base
			//it goes from the waterfall to the refinery S->N
			"X1A_Solo_Orca_02.txt",
			"X1A_Solo_Apache_02.txt",
			"X1A_Solo_Apache_02.txt",
			"X1A_Solo_Apache_02.txt",
			"X1A_Solo_Apache_02.txt",
			"X1A_Solo_Apache_02.txt",
			"X1A_Solo_Nod_Chinook_02.txt",
			"X1A_Solo_Nod_Chinook_02.txt",
			"X1A_Solo_Nod_Chinook_02.txt",
			"X1A_Solo_Nod_Chinook_02.txt",
			"X1A_Solo_Nod_Chinook_02.txt",
			"X1A_Solo_GDI_Chinook_02.txt",
			
		};
		int random = Commands->Get_Random_Int(0, 28);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 28);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M01_FP_BaseToBase_NorthSouth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char *flyovers[28] =
		{
			//this is a low level traffic lane from nod comm to gdi ocean-- N->S
			"X1A_Solo_Orca_03.txt",
			"X1A_Solo_Apache_03.txt",
			"X1A_Solo_Apache_03.txt",
			"X1A_Solo_Apache_03.txt",
			"X1A_Solo_Apache_03.txt",
			"X1A_Solo_Apache_03.txt",
			"X1A_Solo_Nod_Chinook_03.txt",
			"X1A_Solo_Nod_Chinook_03.txt",
			"X1A_Solo_Nod_Chinook_03.txt",
			"X1A_Solo_Nod_Chinook_03.txt",
			"X1A_Solo_Nod_Chinook_03.txt",
			"X1A_Solo_GDI_Chinook_03.txt",
			"X1A_Solo_NodCargo_03.txt",
			"X1A_Solo_A10_03.txt",

			//this is a low level traffic lane from nod refinery to gdi ocean-- N->S
			"X1A_Solo_Orca_04.txt",
			"X1A_Solo_Apache_04.txt",
			"X1A_Solo_Apache_04.txt",
			"X1A_Solo_Apache_04.txt",
			"X1A_Solo_Apache_04.txt",
			"X1A_Solo_Apache_04.txt",
			"X1A_Solo_Nod_Chinook_04.txt",
			"X1A_Solo_Nod_Chinook_04.txt",
			"X1A_Solo_Nod_Chinook_04.txt",
			"X1A_Solo_Nod_Chinook_04.txt",
			"X1A_Solo_Nod_Chinook_04.txt",
			"X1A_Solo_GDI_Chinook_04.txt",
			"X1A_Solo_NodCargo_04.txt",
			"X1A_Solo_A10_04.txt",
		};
		int random = Commands->Get_Random_Int(0, 28);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 28);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
		
	}
};

DECLARE_SCRIPT(M01_FP_BaseToBase_SouthNorth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		last = 21;

		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", "X1A_Solo_NodCargo_06.txt");

		Commands->Start_Timer(obj, this, 60.0f, 0);
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[56] =
		{
			//this is a low level traffic lane from gdi ocean to nod comm center-- S->N
			"X1A_Solo_Orca_05.txt",
			"X1A_Solo_Orca_05.txt",
			"X1A_Solo_Orca_05.txt",
			"X1A_Solo_Orca_05.txt",
			"X1A_Solo_Orca_05.txt",
			"X1A_Solo_Apache_05.txt",
			"X1A_Solo_Nod_Chinook_05.txt",
			"X1A_Solo_GDI_Chinook_05.txt",
			"X1A_Solo_GDI_Chinook_05.txt",
			"X1A_Solo_GDI_Chinook_05.txt",
			"X1A_Solo_GDI_Chinook_05.txt",
			"X1A_Solo_GDI_Chinook_05.txt",
			"X1A_Solo_NodCargo_05.txt",
			"X1A_Solo_A10_05.txt",

			//this is a low level traffic lane from gdi ocean to nod refinery-- S->N
			"X1A_Solo_Orca_06.txt",
			"X1A_Solo_Orca_06.txt",
			"X1A_Solo_Orca_06.txt",
			"X1A_Solo_Orca_06.txt",
			"X1A_Solo_Orca_06.txt",
			"X1A_Solo_Apache_06.txt",
			"X1A_Solo_Nod_Chinook_06.txt",
			"X1A_Solo_GDI_Chinook_06.txt",
			"X1A_Solo_GDI_Chinook_06.txt",
			"X1A_Solo_GDI_Chinook_06.txt",
			"X1A_Solo_GDI_Chinook_06.txt",
			"X1A_Solo_GDI_Chinook_06.txt",
			"X1A_Solo_NodCargo_06.txt",
			"X1A_Solo_A10_06.txt",

			//this is a low/med level traffic lane from gdi ocean to nod refinery-- S->N
			"X1A_Solo_Orca_07.txt",
			"X1A_Solo_Orca_07.txt",
			"X1A_Solo_Orca_07.txt",
			"X1A_Solo_Orca_07.txt",
			"X1A_Solo_Orca_07.txt",
			"X1A_Solo_Apache_07.txt",
			"X1A_Solo_Nod_Chinook_07.txt",
			"X1A_Solo_GDI_Chinook_07.txt",
			"X1A_Solo_GDI_Chinook_07.txt",
			"X1A_Solo_GDI_Chinook_07.txt",
			"X1A_Solo_GDI_Chinook_07.txt",
			"X1A_Solo_GDI_Chinook_07.txt",
			"X1A_Solo_NodCargo_07.txt",
			"X1A_Solo_A10_07.txt",
			
			//this is a low level traffic lane from gdi ocean to nod comm center-- S->N
			"X1A_Solo_Orca_08.txt",
			"X1A_Solo_Orca_08.txt",
			"X1A_Solo_Orca_08.txt",
			"X1A_Solo_Orca_08.txt",
			"X1A_Solo_Orca_08.txt",
			"X1A_Solo_Apache_08.txt",
			"X1A_Solo_GDI_Chinook_08.txt",
			"X1A_Solo_GDI_Chinook_08.txt",
			"X1A_Solo_GDI_Chinook_08.txt",
			"X1A_Solo_GDI_Chinook_08.txt",
			"X1A_Solo_GDI_Chinook_08.txt",
			"X1A_Solo_Nod_Chinook_08.txt",
			"X1A_Solo_NodCargo_08.txt",
			"X1A_Solo_A10_08.txt",
			
		};
		int random = Commands->Get_Random_Int(0, 56);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 56);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M01_FP_NodBase_EastWest_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[72] =
		{
			//this is a flyover at nod base it goes from the comm to the refinery-- E->W
			"X1A_Solo_Orca_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Apache_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_Nod_Chinook_09.txt",
			"X1A_Solo_GDI_Chinook_09.txt",
			"X1A_Solo_NodCargo_09.txt",
			"X1A_Solo_A10_09.txt",

			//this is a flyover at nod base it goes from the comm to the refinery-- E->W
			"X1A_Solo_Orca_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Apache_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_Nod_Chinook_10.txt",
			"X1A_Solo_GDI_Chinook_10.txt",
			"X1A_Solo_NodCargo_10.txt",
			"X1A_Solo_A10_10.txt",

			//this is a flyover at nod base it goes from the comm to the refinery-- E->W
			"X1A_Solo_Orca_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Apache_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_Nod_Chinook_11.txt",
			"X1A_Solo_GDI_Chinook_11.txt",
			"X1A_Solo_NodCargo_11.txt",
			"X1A_Solo_A10_11.txt",
		};
		int random = Commands->Get_Random_Int(0, 72);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 72);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M01_FP_NodBase_WestEast_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[48] =
		{
			//this is a flyover at nod base it goes from the refinery to the comm W->E
			"X1A_Solo_Orca_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Apache_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_Nod_Chinook_12.txt",
			"X1A_Solo_GDI_Chinook_12.txt",
			"X1A_Solo_NodCargo_12.txt",
			"X1A_Solo_A10_12.txt",

			//this is a flyover at nod base it goes from the refinery to the comm W->E
			"X1A_Solo_Orca_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Apache_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_Nod_Chinook_13.txt",
			"X1A_Solo_GDI_Chinook_13.txt",
			"X1A_Solo_NodCargo_13.txt",
			"X1A_Solo_A10_13.txt",
		};
		int random = Commands->Get_Random_Int(0, 48);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 48);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
		
	}
};

DECLARE_SCRIPT(M01_FP_NodBase_NorthSouth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[48] =
		{
			//this is a flyover at nod base it goes from the comm to the waterfall N->S
			"X1A_Solo_Orca_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Apache_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_Nod_Chinook_14.txt",
			"X1A_Solo_GDI_Chinook_14.txt",
			"X1A_Solo_NodCargo_14.txt",
			"X1A_Solo_A10_14.txt",

			//this is a flyover at nod base it goes from the comm to the waterfall N->S
			"X1A_Solo_Orca_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Apache_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_Nod_Chinook_15.txt",
			"X1A_Solo_GDI_Chinook_15.txt",
			"X1A_Solo_NodCargo_15.txt",
			"X1A_Solo_A10_15.txt",
		};
		int random = Commands->Get_Random_Int(0, 48);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 48);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M01_FP_NodBase_SouthNorth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[48] =
		{
			//this is a flyover at nod base it goes from guard tower 01 to the comm center S-N
			"X1A_Solo_Orca_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Apache_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_Nod_Chinook_16.txt",
			"X1A_Solo_GDI_Chinook_16.txt",
			"X1A_Solo_NodCargo_16.txt",
			"X1A_Solo_A10_16.txt",

			//this is a flyover at nod base it goes from guard tower 01 to the comm center S-N
			"X1A_Solo_Orca_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Apache_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_Nod_Chinook_17.txt",
			"X1A_Solo_GDI_Chinook_17.txt",
			"X1A_Solo_NodCargo_17.txt",
			"X1A_Solo_A10_17.txt",
		};
		int random = Commands->Get_Random_Int(0, 48);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 48);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;	
	}
};

DECLARE_SCRIPT(M01_FP_GDIOcean_NorthSouth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[28] =
		{
			//this is a GDI ocean traffic lane  N->S
			"X1A_Solo_Orca_18.txt",
			"X1A_Solo_Orca_18.txt",
			"X1A_Solo_Orca_18.txt",
			"X1A_Solo_Orca_18.txt",
			"X1A_Solo_Orca_18.txt",
			"X1A_Solo_Apache_18.txt",
			"X1A_Solo_Nod_Chinook_18.txt",
			"X1A_Solo_GDI_Chinook_18.txt",
			"X1A_Solo_GDI_Chinook_18.txt",
			"X1A_Solo_GDI_Chinook_18.txt",
			"X1A_Solo_GDI_Chinook_18.txt",
			"X1A_Solo_GDI_Chinook_18.txt",
			"X1A_Solo_NodCargo_18.txt",
			"X1A_Solo_A10_18.txt",

			//this is a GDI ocean traffic lane  N->S
			"X1A_Solo_Orca_19.txt",
			"X1A_Solo_Orca_19.txt",
			"X1A_Solo_Orca_19.txt",
			"X1A_Solo_Orca_19.txt",
			"X1A_Solo_Orca_19.txt",
			"X1A_Solo_Apache_19.txt",
			"X1A_Solo_Nod_Chinook_19.txt",
			"X1A_Solo_GDI_Chinook_19.txt",
			"X1A_Solo_GDI_Chinook_19.txt",
			"X1A_Solo_GDI_Chinook_19.txt",
			"X1A_Solo_GDI_Chinook_19.txt",
			"X1A_Solo_GDI_Chinook_19.txt",
			"X1A_Solo_NodCargo_19.txt",
			"X1A_Solo_A10_19.txt",
			
		};
		int random = Commands->Get_Random_Int(0, 28);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 28);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M01_FP_GDIOcean_SouthNorth_Contoller_JDG, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 60.0f, 0);
		last = 21;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char* flyovers[26] =
		{
			//this is a GDI ocean traffic lane  S->N
			"X1A_Solo_Orca_20.txt",
			"X1A_Solo_Orca_20.txt",
			"X1A_Solo_Orca_20.txt",
			"X1A_Solo_Orca_20.txt",
			"X1A_Solo_Orca_20.txt",
			"X1A_Solo_Apache_20.txt",
			"X1A_Solo_Nod_Chinook_20.txt",
			"X1A_Solo_GDI_Chinook_20.txt",
			"X1A_Solo_GDI_Chinook_20.txt",
			"X1A_Solo_GDI_Chinook_20.txt",
			"X1A_Solo_GDI_Chinook_20.txt",
			"X1A_Solo_GDI_Chinook_20.txt",
			"X1A_Solo_A10_20.txt",

			//this is a GDI ocean traffic lane  S->N
			"X1A_Solo_Orca_21.txt",
			"X1A_Solo_Orca_21.txt",
			"X1A_Solo_Orca_21.txt",
			"X1A_Solo_Orca_21.txt",
			"X1A_Solo_Orca_21.txt",
			"X1A_Solo_Apache_21.txt",
			"X1A_Solo_Nod_Chinook_21.txt",
			"X1A_Solo_GDI_Chinook_21.txt",
			"X1A_Solo_GDI_Chinook_21.txt",
			"X1A_Solo_GDI_Chinook_21.txt",
			"X1A_Solo_GDI_Chinook_21.txt",
			"X1A_Solo_GDI_Chinook_21.txt",
			"X1A_Solo_A10_21.txt",
			
		};
		int random = Commands->Get_Random_Int(0, 26);
		
		while (random == last) {
			random = Commands->Get_Random_Int(0, 26);
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		float delayTimer = Commands->Get_Random ( 30, 60 );
		Commands->Start_Timer(obj, this, delayTimer, 0);
		last = random;
		
	}
};

DECLARE_SCRIPT(M01_Deco_LightTanks_JDG, "")//119424 119426 119425
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myTarget (-27.746f, 52.823f, 1.071f);
		ActionParamsStruct params;
		params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
		params.Set_Attack( myTarget, 100, 3, true );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack (obj, params);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ATTACKING_01_JDG)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
				params.Set_Attack( STAR, 100, 8, true );
				params.AttackCheckBlocked = false;
				Commands->Action_Attack (obj, params);
			}

			else if (param == M01_START_ATTACKING_02_JDG)
			{
				if (obj)
				{
					Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Airstrike_Controller_JDG, "")//121041
{
	int decoTank01_id;
	int decoTank02_id;
	int decoTank03_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(decoTank01_id, 1);
		SAVE_VARIABLE(decoTank02_id, 2);
		SAVE_VARIABLE(decoTank03_id, 3);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == 0)//airstrike text file is sendyou the command to play explosions
			{
				GameObject * realtank = Commands->Find_Object ( 120022 );

				if (realtank != NULL)
				{
					Commands->Send_Custom_Event( obj, realtank, 0, M01_START_ATTACKING_01_JDG, 0.5f );
				}

				GameObject * tank01 = Commands->Find_Object ( decoTank01_id );
				GameObject * tank02 = Commands->Find_Object ( decoTank02_id );
				GameObject * tank03 = Commands->Find_Object ( decoTank03_id );

				if (tank01 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank01, 0, M01_START_ATTACKING_02_JDG, 0.5f );
				}

				if (tank02 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank02, 0, M01_START_ATTACKING_02_JDG, 0.75f );
				}

				if (tank03 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank03, 0, M01_START_ATTACKING_02_JDG, 1.0f );
				}
			}

			else if (param == M01_START_ACTING_JDG)
			{
				Vector3 tank01Spot (-12.175f, 68.280f, 3.009f);
				Vector3 tank02Spot (-14.948f, 61.305f, 3.259f);
				Vector3 tank03Spot (-22.011f, 66.635f, 2.959f);
				GameObject * decoTank01 = Commands->Create_Object ( "Nod_Light_Tank_Dec", tank01Spot );
				Commands->Set_Facing ( decoTank01, -145 );
				GameObject * decoTank02 = Commands->Create_Object ( "Nod_Light_Tank_Dec", tank02Spot );
				Commands->Set_Facing ( decoTank02, -150 );
				GameObject * decoTank03 = Commands->Create_Object ( "Nod_Light_Tank_Dec", tank03Spot );
				Commands->Set_Facing ( decoTank03, -115 );
				Commands->Attach_Script(decoTank01, "M01_Deco_LightTanks_JDG", "");
				Commands->Attach_Script(decoTank02, "M01_Deco_LightTanks_JDG", "");
				Commands->Attach_Script(decoTank03, "M01_Deco_LightTanks_JDG", "");

				decoTank01_id = Commands->Get_ID ( decoTank01 );
				decoTank02_id = Commands->Get_ID ( decoTank02 );
				decoTank03_id = Commands->Get_ID ( decoTank03 );
			}

			else if (param == M01_START_ATTACKING_01_JDG)
			{
				GameObject * tank01 = Commands->Find_Object ( decoTank01_id );
				GameObject * tank02 = Commands->Find_Object ( decoTank02_id );
				GameObject * tank03 = Commands->Find_Object ( decoTank03_id );

				if (tank01 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank01, 0, M01_START_ATTACKING_01_JDG, 0.5f );
				}

				if (tank02 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank02, 0, M01_START_ATTACKING_01_JDG, 0.5f );
				}

				if (tank03 != NULL)
				{
					Commands->Send_Custom_Event( obj, tank03, 0, M01_START_ATTACKING_01_JDG, 0.5f );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_01_JDG, 0 );
				
				GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(0,0,0));
				Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X1G_A-10_Strike.txt");
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_RealLightTank_JDG, "")//120022
{//waypath 120218
	void Killed( GameObject * /* obj */, GameObject * /* killer */ ) override
	{
		//int airdrop_conv = Commands->Create_Conversation( "M01_AirDrop_Conversation", 100, 1000, false);
		//Commands->Join_Conversation( NULL, airdrop_conv, false, false, true );
		//Commands->Start_Conversation( airdrop_conv,  airdrop_conv );

		
	}
	
	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
				params.Set_Movement( Vector3(0,0,0), .7f, 10 );
				params.WaypathID = 120218;
				//params.AttackActive = false;
				Commands->Action_Goto( obj, params );

				GameObject * airstrikeController = Commands->Find_Object ( 121041 );
				if (airstrikeController != NULL)
				{
					Commands->Send_Custom_Event( obj, airstrikeController, 0, M01_START_ACTING_JDG, 0 );
					Commands->Send_Custom_Event( obj, airstrikeController, 0, M01_MODIFY_YOUR_ACTION_JDG, 15 );
				}	
			}

			else if (param == M01_START_ATTACKING_01_JDG)
			{
				if (obj)
				{
					Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
				}	
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG )
			{ 
				params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
				params.Set_Attack( STAR, 100, 10, true );
				params.AttackCheckBlocked = false;
				Commands->Action_Attack (obj, params);

				//Commands->Send_Custom_Event( obj, obj, 0, M01_START_ATTACKING_02_JDG, 4 );

				
			}
		}
	}
};

DECLARE_SCRIPT(M01_RealLightTank_TriggerZone_JDG, "")
{
	bool entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		entered = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (entered == false)
		{
			if (enterer == STAR)
			{
				entered = true;
				GameObject * lightTank = Commands->Find_Object ( 120022 );//00-N036E
				if (lightTank != NULL)
				{
					Commands->Send_Custom_Event( obj, lightTank, 0, M01_START_ACTING_JDG, 0 );
					//Commands->Create_2D_Sound ( "00-N036E" );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_First_AutoRifle_JDG, "")
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			int pressEnterConv = Commands->Create_Conversation( "M01_Press_Enter_Conversation", 100, 1000, true);
			Commands->Join_Conversation( NULL, pressEnterConv, false, false, true );
			Commands->Start_Conversation( pressEnterConv,  pressEnterConv );

			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0520I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}
};




DECLARE_SCRIPT(M01_Announce_First_ObjectiveZone_JDG, "")//116380
{
	void Created( GameObject * /* obj */ ) override 
	{
		int pressF1Conv = Commands->Create_Conversation( "M01_Press_F1_Conversation", 100, 1000, true);
		Commands->Join_Conversation( NULL, pressF1Conv, false, false, true );
		Commands->Start_Conversation( pressF1Conv,  pressF1Conv );

		Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0515I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

		GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X1H_Hover_Troop.txt");
	}

	void Entered( GameObject * obj, GameObject * enterer ) override //M01_Press_F1_Conversation
	{
		if (enterer == STAR)
		{
			GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
			if (pogController != NULL)
			{
				Commands->Send_Custom_Event( obj, pogController, 0, M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, 0 );
			}

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_SniperRifle_01_JDG, "")//on powerplant
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			GameObject * c4TextController = Commands->Find_Object ( 117188 );
			if (c4TextController != NULL)
			{
				Commands->Send_Custom_Event( obj, c4TextController, 2, 1, 0 );
			}
			
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3(-53.217f, -68.049f, 13.701f));
			Commands->Attach_Script(sniperTarget01, "M01_SniperRifle_01_Target_JDG", "");
		}
	}
};

DECLARE_SCRIPT(M01_SniperRifle_01_Target_JDG, "")//on lighthouse
{
	void Created( GameObject * obj ) override 
	{
		Commands->Attach_Script(obj, "M00_Soldier_Powerup_Disable", "");
		Commands->Enable_Hibernation( obj, false );
		ActionParamsStruct params;
		params.Set_Basic(this, 90, M01_WALKING_WAYPATH_01_JDG);
		params.Set_Movement( Vector3(0,0,0), WALK, .25f );
		params.WaypathID = 118434;

		Commands->Action_Goto( obj, params );
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG )
			{ 
				Commands->Enable_Hibernation( obj, true );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}
		}
	}
};

DECLARE_SCRIPT(M01_SniperRifle_02_JDG, "")//in guard tower 01
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			GameObject * c4TextController = Commands->Find_Object ( 117188 );
			if (c4TextController != NULL)
			{
				Commands->Send_Custom_Event( obj, c4TextController, 2, 1, 0 );
			}	
		}
	}
};

DECLARE_SCRIPT(M01_SniperRifle_02_AirdropZone_JDG, "")//in guard tower 01
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Create_Sound ( "M01EVAG_DSGN0268I1EVAG_SND", Vector3 (0,0,0), obj );//enemy aircraft detected
			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-12.031f, -42.150f, -1.252f));
			Commands->Set_Facing(chinook_obj3, -45.0f);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X1I_GDIBasePrimaryNodChinook.txt");

			Commands->Destroy_Object (obj );
		}
	}
};

DECLARE_SCRIPT(M01_C4_TutorialScript_JDG, "")//
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			GameObject * c4TextController = Commands->Find_Object ( 117188 );
			if (c4TextController != NULL)
			{
				Commands->Send_Custom_Event( obj, c4TextController, 1, 1, 0 );
			}		
		}
	}
};

DECLARE_SCRIPT(M01_Use_Ladder_Zone_JDG, "")
{
	void Entered( GameObject * /* obj */, GameObject * enterer ) override //M01_Press_F1_Conversation
	{
		if (enterer == STAR)
		{
			int pressF1Conv = Commands->Create_Conversation( "M01_Press_E_For_Ladders_Conversation", 100, 1000, true);
			Commands->Join_Conversation( NULL, pressF1Conv, false, false, true );
			Commands->Start_Conversation( pressF1Conv,  pressF1Conv );

			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0514I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}
};

DECLARE_SCRIPT(M01_AccessDenied_Zone_JDG, "")
{
	void Entered( GameObject * /* obj */, GameObject * enterer ) override //M01_Press_F1_Conversation
	{
		if (enterer == STAR)
		{
			int pressF1Conv = Commands->Create_Conversation( "M01_Lockdown_Conversation", 100, 1000, true);
			Commands->Join_Conversation( NULL, pressF1Conv, false, false, true );
			Commands->Start_Conversation( pressF1Conv,  pressF1Conv );

			//Commands->Set_HUD_Help_Text ( IDS_M00EVAG_DSGN0039I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_FirstChinookMinigunnerGuy_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
	}
};

DECLARE_SCRIPT(M01_QuickSave_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0525I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_GDIBaseCommander_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsA", false, NULL, 0.0f, -1.0f, false );
		Commands->Debug_Message ( "**********************script attached to evac chopper\n" );
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );

		GameObject * myController = Commands->Find_Object ( 106694 );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsA") == 0)
		{
			Commands->Debug_Message ( "**********************chopper has finished entry anim\n" );
			Vector3 evacLocation = Commands->Get_Position ( obj );
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsL", true, NULL, 0.0f, -1.0f, false );

			GameObject * gdiBaseCommander = Commands->Find_Object ( 106050 );
			if (gdiBaseCommander != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiBaseCommander, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
			}

			GameObject * gdiBaseMinigunner = Commands->Find_Object ( 116382 );
			if (gdiBaseMinigunner != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiBaseMinigunner, 0, M01_GOING_TO_EVAC_SPOT_JDG, 3 );
			}

			GameObject * gdiBaseGrenadier = Commands->Find_Object ( 116383 );
			if (gdiBaseGrenadier != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiBaseGrenadier, 0, M01_GOING_TO_EVAC_SPOT_JDG, 6 );
			}

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************helicopter has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * gdiBaseCommander = Commands->Find_Object ( 106050 );
					GameObject * gdiBaseMinigunner = Commands->Find_Object ( 116382 );
					GameObject * gdiBaseGrenadier = Commands->Find_Object ( 116383 );

					if ((gdiBaseCommander != NULL) || (gdiBaseMinigunner != NULL) || (gdiBaseGrenadier != NULL))
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 1 );
					}

					else
					{
						GameObject * controller = Commands->Find_Object ( 106694 );
						if (controller != NULL)
						{
							Commands->Send_Custom_Event ( obj, controller, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 0 );
						}
					}
				}
				break;
		}
	}
};



DECLARE_SCRIPT(M01_GDIBaseCommander_EvacController_JDG, "")//106694
{
	bool evacing;
	int evac_waypath_id;
	int evac_helicopter_id;
	int evac_rope_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(evacing, 1);
		SAVE_VARIABLE(evac_waypath_id, 2);
		SAVE_VARIABLE(evac_helicopter_id, 3);
		SAVE_VARIABLE(evac_rope_id, 4);
	}

	void Created( GameObject * obj ) override 
	{
		evacing = false;
		Commands->Enable_Hibernation( obj, false );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		if (type == 0)//
		{
			if (param == M01_START_ACTING_JDG)//gdi commander is done giving ion beacon--start evac scenario
			{
				GameObject * gdiBaseCommander = Commands->Find_Object ( 106050 );
				GameObject * gdiBaseMinigunner = Commands->Find_Object ( 116382 );
				GameObject * gdiBaseGrenadier = Commands->Find_Object ( 116383 );

				if (gdiBaseCommander != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiBaseCommander, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
				}

				if (gdiBaseMinigunner != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiBaseMinigunner, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
				}

				if (gdiBaseGrenadier != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiBaseGrenadier, 0, M01_GOTO_YOUR_EVAC_SPOT_JDG, 0 );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG && evacing == false)//one of your guys is at his wait spot--call in chopper
			{
				evacing = true;

				Vector3 evacPosition = Commands->Get_Position ( obj );
				GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
				Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_GDIBaseCommander_EvacAnim.txt");
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG && evacing == true)//everyone is on or dead--give take off orders
			{
				GameObject * evacChopper = Commands->Find_Object ( evac_helicopter_id );
				GameObject * evacWaypath = Commands->Find_Object ( evac_waypath_id );
				GameObject * evacRope = Commands->Find_Object ( evac_rope_id );

				if (evacChopper != NULL)
				{
					Commands->Debug_Message ( "**********************sending custom to chopper to leave.....%d\n",evac_helicopter_id );
					Commands->Send_Custom_Event ( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}

				if (evacWaypath != NULL)
				{
					Commands->Debug_Message ( "**********************sending custom to waypath to leave.....%d\n",evac_waypath_id );
					Commands->Send_Custom_Event ( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}

				if (evacRope != NULL)
				{
					Commands->Debug_Message ( "**********************sending custom to rope to leave.....%d\n",evac_rope_id );
					Commands->Send_Custom_Event ( obj, evacRope, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_03_JDG)//chopper is sending you his ID
			{
				evac_helicopter_id = Commands->Get_ID ( sender );
				Commands->Debug_Message ( "**********************chopper id received.....%d\n",evac_helicopter_id );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_04_JDG)//waypath is sending you his ID 
			{
				evac_waypath_id = Commands->Get_ID ( sender );
				Commands->Debug_Message ( "**********************waypath id received.....%d\n", evac_waypath_id );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_05_JDG)//rope is sending you his ID 
			{
				evac_rope_id = Commands->Get_ID ( sender );
				Commands->Debug_Message ( "**********************rope id received.....%d\n", evac_rope_id );
			}

			else if (param == M01_GIVE_ME_A_ROPE_JDG)//someone is waiting--pull him up
			{
				Vector3 evacPosition = Commands->Get_Position ( obj );
				GameObject * troopbone = Commands->Create_Object("Invisible_Object", evacPosition );
				Commands->Set_Model ( troopbone, "XG_EV5_troopBN" );
				
				Commands->Attach_Script(troopbone, "M01_Evac_TroopBone_JDG", "");
				Commands->Innate_Disable(sender);
				Commands->Attach_To_Object_Bone( sender, troopbone, "Troop_L" );
				if(troopbone)
				{
					Commands->Set_Animation ( troopbone, "XG_EV5_troopBN.XG_EV5_troopBN", false, NULL, 0.0f, -1.0f, false);
				}
				else
				{
					Commands->Debug_Message("troopbone not created yet");
				}
				
				Commands->Set_Animation ( sender, "S_A_Human.XG_EV5_troop", false, NULL, 0.0f, -1.0f, false );
			}
		}
	}
};






DECLARE_SCRIPT(M01_Base_Nod_Minigunner_JDG, "")//116384
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}
};

DECLARE_SCRIPT(M01_POW_Nod_Minigunner01_JDG, "")//116388
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Innate_Is_Stationary ( obj, true );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * pow01 = Commands->Find_Object ( 116386 );
		if (pow01 != NULL)
		{
			Commands->Send_Custom_Event( obj, pow01, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}
};





DECLARE_SCRIPT(M01_GDIBase_POWEncounter02_Controller_JDG, "")//M01_GDIBASE_POWSCENE02_MONITOR_JDG 103331
{
	bool transportCalled;
	bool powGuy01isDead;
	bool powGuy02isDead;
	bool objective_failed;
	int evac_waypath_id;
	int evac_helicopter_id;
	int evac_rope_id;


	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(transportCalled, 1);
		SAVE_VARIABLE(powGuy01isDead, 2);
		SAVE_VARIABLE(powGuy02isDead, 3);
		SAVE_VARIABLE(objective_failed, 4);
		SAVE_VARIABLE(evac_waypath_id, 5);
		SAVE_VARIABLE(evac_helicopter_id, 6);
		SAVE_VARIABLE(evac_rope_id, 7);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		powGuy01isDead = false;
		powGuy02isDead = false;
		objective_failed  = false;
		transportCalled = false;
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * sender ) override
	{
		if (param == M01_IVE_BEEN_KILLED_JDG)//one of the gt1 guys has been killed..see who it is
		{
			GameObject * pow01 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY01_JDG );
			GameObject * pow02 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY02_JDG );

			if (sender == pow01)
			{
				powGuy01isDead = true;
			}

			if (sender == pow02)
			{
				powGuy02isDead = true;
			}

			if (powGuy01isDead == true && powGuy02isDead == true)
			{
				Commands->Set_Objective_Status( M01_GDI_BASE_POW_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
				objective_failed = true;
			}
		}

		else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//chopper is sending you his ID
		{
			evac_helicopter_id = Commands->Get_ID ( sender );
			//Commands->Debug_Message ( "**********************chopper id received.....%d\n",evac_helicopter_id );
		}

		else if (param == M01_WAYPATH_IS_SENDING_ID_JDG)//waypath is sending you his ID 
		{
			evac_waypath_id = Commands->Get_ID ( sender );
			//Commands->Debug_Message ( "**********************waypath id received.....%d\n", evac_waypath_id );
		}

		else if (param == M01_ROPE_IS_SENDING_ID_JDG)//rope is sending you his ID 
		{
			evac_rope_id = Commands->Get_ID ( sender );
			//Commands->Debug_Message ( "**********************rope id received.....%d\n", evac_rope_id );
		}

		else if (param == M01_GIVE_ME_A_ROPE_JDG)//someone is waiting--pull him up
		{
			Vector3 evacPosition = Commands->Get_Position ( obj );
			GameObject * troopbone = Commands->Create_Object("Invisible_Object", evacPosition );
			Commands->Set_Model ( troopbone, "XG_EV5_troopBN" );
			
			Commands->Attach_Script(troopbone, "M01_Evac_TroopBone_JDG", "");
			Commands->Innate_Disable(sender);
			Commands->Attach_To_Object_Bone( sender, troopbone, "Troop_L" );
			if(troopbone)
			{
				Commands->Set_Animation ( troopbone, "XG_EV5_troopBN.XG_EV5_troopBN", false, NULL, 0.0f, -1.0f, false);
			}
			else
			{
				Commands->Debug_Message("troopbone not created yet");
			}
			
			Commands->Set_Animation ( sender, "S_A_Human.XG_EV5_troop", false, NULL, 0.0f, -1.0f, false );
		}

		else if (param == M01_MODIFY_YOUR_ACTION_JDG)//a gdi guy is waiting for evac--call in a helicopter
		{
			if (transportCalled == false)
			{
				transportCalled = true;

				Vector3 evacPosition = Commands->Get_Position ( obj );
				GameObject *controller = Commands->Create_Object("Invisible_Object", evacPosition);
				Commands->Attach_Script(controller, "Test_Cinematic", "XG_M01_GDIBasePow_EvacAnim.txt");
			}
		}

		else if (param == M01_MODIFY_YOUR_ACTION_05_JDG)//chinook is ready to go--give clearance
		{
			GameObject *evacWaypath = Commands->Find_Object ( evac_waypath_id );
			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event( obj, evacWaypath, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			GameObject *evacChopper = Commands->Find_Object ( evac_helicopter_id );
			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event( obj, evacChopper, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			GameObject *evacRope = Commands->Find_Object ( evac_rope_id );
			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event( obj, evacRope, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			if (objective_failed != true)
			{
				Commands->Set_Objective_Status( M01_GDI_BASE_POW_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDIBasePOW_Air_Evac_Chopper_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsA", false, NULL, 0.0f, -1.0f, false );
		Commands->Debug_Message ( "**********************script attached to evac chopper\n" );
		Commands->Set_Shield_Type ( obj, "Blamo" );
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, 0, M01_GOTO_IDLE_JDG, 3 );

		GameObject * myController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (myController != NULL)
		{
			Commands->Send_Custom_Event ( obj, myController, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myMaxHealth = Commands->Get_Max_Health ( obj );
		Commands->Set_Health ( obj, myMaxHealth );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsA") == 0)
		{
			Commands->Debug_Message ( "**********************chopper has finished entry anim\n" );
			Vector3 evacLocation = Commands->Get_Position ( obj );
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsL", true, NULL, 0.0f, -1.0f, false );

			GameObject * gdiGuy01 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY01_JDG );
			GameObject * gdiGuy02 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY02_JDG );

			if (gdiGuy01 != NULL)
			{
				Commands->Send_Custom_Event ( obj, gdiGuy01, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 3 );
			}

			if (gdiGuy02 != NULL)
			{
				Commands->Send_Custom_Event ( obj, gdiGuy02, 0, M01_GOING_TO_EVAC_SPOT02_JDG, 0 );
			}

			Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		switch (param)
		{
			case M01_MODIFY_YOUR_ACTION_JDG: 
				{
					Commands->Debug_Message ( "*******************************helicopter has received custom to goto exit anim\n" );
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M01_GOTO_IDLE_JDG: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;

			case M01_DO_END_MISSION_CHECK_JDG: 
				{
					GameObject * gdiGuy01 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY01_JDG );
					GameObject * gdiGuy02 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY02_JDG );

					if ((gdiGuy01 != NULL)||(gdiGuy02 != NULL))
					{
						Commands->Send_Custom_Event ( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 2 );
					}

					else
					{
						GameObject * controller = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
						if (controller != NULL)
						{
							Commands->Send_Custom_Event ( obj, controller, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 0 );
						}
					}
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M01_ConDropZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(54.835f, -3.011f, 13.105f));
			Commands->Set_Facing ( controller1, 45 );
			Commands->Attach_Script(controller1, "Test_Cinematic", "X01_ConYardDrop.txt");

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_ConYard_Dropoff_Dude_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_TroopDrop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_C130_Dropoff_Dude_JDG, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_a_Human.H_A_X5D_ParaT_1") == 0)
		{
			Commands->Destroy_Object ( obj );
		}

		else if (stricmp(anim, "S_a_Human.H_A_X5D_ParaT_2") == 0)
		{
			Commands->Destroy_Object ( obj );
		}

		else if (stricmp(anim, "S_a_Human.H_A_X5D_ParaT_3") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}
};


DECLARE_SCRIPT(M01_Base_GDI_Fodder_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Vector3 myTarget (72.223f, -24.751f, -0.296f);
		ActionParamsStruct params;
		Commands->Set_Innate_Is_Stationary ( obj, true );
		params.Set_Basic( this, 100, M01_START_ACTING_JDG );
		params.Set_Attack( myTarget, 1000, 0, true );
		params.AttackCrouched = true;
		params.AttackCheckBlocked = false;
		Commands->Action_Attack (obj, params);

		float delayTimer = Commands->Get_Random ( 10, 15 );
		Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, delayTimer );//kill yourself
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG)//kill yourself
			{
				Commands->Apply_Damage( obj, 10000, "BlamoKiller", NULL );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Base_StartZone_JDG, "")//119628
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * minigunner = Commands->Find_Object ( 116382 );
			GameObject * grenadier = Commands->Find_Object ( 116383 );

			if (minigunner != NULL)
			{
				Commands->Send_Custom_Event( obj, minigunner, 0, M01_START_ATTACKING_01_JDG, 0 );//tell minigunner to greet havoc
			}

			if (grenadier != NULL)
			{
				Commands->Send_Custom_Event( obj, grenadier, 0, M01_START_ATTACKING_01_JDG, 0 );//tell grenadier to greet havoc
			}

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Nod_Truck_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_NEUTRAL );
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_NOD );
	}
};


DECLARE_SCRIPT(M01_C4_Tutorial_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * c4TextController = Commands->Find_Object ( 117188 );
			if (c4TextController != NULL)
			{
				Commands->Send_Custom_Event( obj, c4TextController, 1, 1, 0 );
			}

			Commands->Destroy_Object ( obj );
		}
	}
};



DECLARE_SCRIPT(M01_Mainframe_Tutorial_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0527I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_GateSwitch_Tutorial_Zone_JDG, "")//120844
{
	bool command_clearance;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(command_clearance, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		command_clearance = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && command_clearance == true)
		{
			Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0528I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				command_clearance = true;
			}
		}
	}

};

DECLARE_SCRIPT(M01_BarnArea_NOD_Commander_Trigger_Zone02_JDG, "")//103343
{
	bool entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		entered = false;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && entered == false)
		{
			entered = true;
			GameObject * firstFieldCommander = Commands->Find_Object (M01_BARNAREA_NOD_COMMANDER_JDG);
			if (firstFieldCommander != NULL)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_BARNAREA_NOD_COMMANDER_JDG ), 0, M01_START_ACTING_JDG, 0 );
				Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_ADD_BARN_COMMANDER_OBJECTIVE_JDG, 5 );
			}
		}
	}
};







DECLARE_SCRIPT(M01_TurretBeach_Turret_01_Script_JDG, "")//M01_TURRETBEACH_TURRET_01_ID 101434
{
	int engineer_id;
	int fodderHovercraft_ID;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(engineer_id, 1);
		SAVE_VARIABLE(fodderHovercraft_ID, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		engineer_id = M01_TURRETBEACH_ENGINEER_ID;
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override //M01_IVE_BEEN_KILLED_JDG
	{
		GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
		if (pogController != NULL)
		{
			Commands->Send_Custom_Event( obj, pogController, M01_CHANGE_OBJECTIVE_POG_JDG, M01_TURRETS_OBJECTIVE_JDG, 0 );
		}

		GameObject * turretEngineer = Commands->Find_Object ( engineer_id );
		if (turretEngineer != NULL)
		{
			Commands->Send_Custom_Event( obj, turretEngineer, 0, M01_TURRET_HAS_BEEN_DESTROYED_JDG, 0 );
		}

		Commands->Send_Custom_Event( obj, Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG ), 0, M01_TURRET_HAS_BEEN_DESTROYED_JDG, 5 );

		Vector3 myPositon = Commands->Get_Position ( obj );
		float myFacing  = Commands->Get_Facing ( obj );

		GameObject * destroyedTurret = Commands->Create_Object ( "Nod_Turret_Destroyed", myPositon);
		Commands->Set_Facing ( destroyedTurret, myFacing );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (obj)
		{
			GameObject * turretEngineer = Commands->Find_Object ( engineer_id );
			if (damager != NULL && damager == turretEngineer)
			{
				float myMaxHealth = Commands->Get_Max_Health ( obj );
				Commands->Set_Health ( obj, myMaxHealth );
			}

			else if (turretEngineer != NULL)
			{
				Commands->Send_Custom_Event( obj, turretEngineer, 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 0 );
			}
		}	
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
				if (gunboat != NULL)
				{
					ActionParamsStruct params;

					params.Set_Basic(this, 100, M01_START_ACTING_JDG);
					params.Set_Attack( gunboat, 3000, 3, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack ( obj, params );
				}
			}

			else if (param == M01_BUSY_TRY_AGAIN_JDG)//engineer is busy--call back in 2 seconds  
			{
				GameObject * turretEngineer = Commands->Find_Object ( engineer_id );
				if (turretEngineer != NULL)
				{
					Commands->Send_Custom_Event( obj, turretEngineer, 0, M01_PLAYER_IS_ATTACKING_ME_JDG, 1 );
				}
			}

			else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//new engineer is actually sending id
			{
				engineer_id = Commands->Get_ID ( sender );
			}
		}

		else if (type == M01_FODDER_HOVERCRAFT_IS_HERE)
		{
			fodderHovercraft_ID = param;
			GameObject * fodderHovercraft = Commands->Find_Object ( fodderHovercraft_ID );
			
			if (fodderHovercraft != NULL)
			{
				Commands->Action_Reset ( obj, 100 );
				ActionParamsStruct params;

				params.Set_Basic(this, 100, M01_START_ATTACKING_01_JDG);
				params.Set_Attack( fodderHovercraft, 3000, 0, true );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_START_ATTACKING_01_JDG)//hovercraft is dead...go back to attacking gunboat
		{ 
			GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
			if (gunboat != NULL)
			{
				ActionParamsStruct params;

				params.Set_Basic(this, 100, M01_START_ACTING_JDG);
				params.Set_Attack( gunboat, 3000, 3, true );
				params.AttackCheckBlocked = false;

				Commands->Action_Attack ( obj, params );
			}
		}
	}
};

DECLARE_SCRIPT(M01_GunboatAction_Controller_JDG, "")//M01_TURRETBEACH_CONTROLLER_ID
{
	bool active;
	int engineer_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active, 1);
		SAVE_VARIABLE(engineer_id, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		active = false;
		engineer_id = M01_TURRETBEACH_ENGINEER_ID;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				if (active == false)
				{
					active = true;

					GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
					GameObject * engineer = Commands->Find_Object ( engineer_id );
					//GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );

					if (gunboat != NULL)
					{
						Commands->Send_Custom_Event ( obj, gunboat, 0, M01_START_ACTING_JDG, 0 );
					}

					if (engineer != NULL)
					{
						Commands->Send_Custom_Event ( obj, engineer, 0, M01_START_ACTING_JDG, 0 );
					}
				}
			}

			else if (param == M01_GOTO_IDLE_JDG)//M01_CHOPPER_IS_SENDING_ID_JDG
			{
				if (active == true)
				{
					active = false;

					GameObject * gunboat = Commands->Find_Object ( M01_TURRETBEACH_GUNBOAT_ID );
					GameObject * engineer = Commands->Find_Object ( engineer_id );

					if (gunboat != NULL)
					{
						Commands->Send_Custom_Event ( obj, gunboat, 0, M01_GOTO_IDLE_JDG, 0 );
					}

					if (engineer != NULL)
					{
						Commands->Send_Custom_Event ( obj, engineer, 0, M01_GOTO_IDLE_JDG, 0 );
					}
				}
			}

			else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//new engineer is actually sending ID
			{
				engineer_id = Commands->Get_ID ( sender );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Initial_Gunboat_Script_JDG, "")//M01_TURRETBEACH_GUNBOAT_ID	101477
{
	bool firstHealthWarningPlayed;
	bool secondHealthWarningPlayed;
	Vector3 myPosition;
	float myFacing;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(firstHealthWarningPlayed, 1);
		SAVE_VARIABLE(secondHealthWarningPlayed, 2);
		SAVE_VARIABLE(myPosition, 3);
		SAVE_VARIABLE(myFacing, 4);
	}

	void Created( GameObject * obj ) override 
	{
		firstHealthWarningPlayed = false;
		secondHealthWarningPlayed = false;
		myPosition = Commands->Get_Position ( obj );
		myFacing = Commands->Get_Facing ( obj );

		

		ActionParamsStruct params;

		params.Set_Basic(this, 90, M01_WALKING_WAYPATH_02_JDG);
		//params.Set_Movement( Vector3(0,0,0), .1f, 3 );
		//params.WaypathID = 101497;
		params.AttackActive		= false;
		params.AttackCheckBlocked = false;
			
		Commands->Action_Attack (obj, params);
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override //
	{	
		GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
		if (missionController != NULL)
		{
			Commands->Send_Custom_Event( obj, missionController, 0, M01_END_TURRETS_OBJECTIVE_FAIL_JDG, 0 );
		}

		GameObject * sinkingGunboat = Commands->Create_Object ( "M01_GDI_Gunboat", myPosition );
		Commands->Set_Facing ( sinkingGunboat, myFacing );
		Commands->Attach_Script(sinkingGunboat, "M01_Sinking_Gunboat_JDG", "");
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		if (obj)
		{
			float myCurrentHealth = Commands->Get_Health (obj);
			float myMaxHealth = Commands->Get_Max_Health (obj);

			float healthPercentage = (myCurrentHealth/myMaxHealth) * 100;

			if (myCurrentHealth <= 10)
			{
				Commands->Set_Health ( obj, 10 );
			}

			if (healthPercentage <= 50)
			{
				if (secondHealthWarningPlayed == false)
				{
					secondHealthWarningPlayed = true;
					int gunboatHealthWarning02 = Commands->Create_Conversation( "M01_Gunboat_HealthWarning_02", 100, 1000, false);
					Commands->Join_Conversation( NULL, gunboatHealthWarning02, false, false, true );
					Commands->Join_Conversation( NULL, gunboatHealthWarning02, false, false, true );
					Commands->Join_Conversation( STAR, gunboatHealthWarning02, false, false, true );
					Commands->Start_Conversation( gunboatHealthWarning02,  gunboatHealthWarning02 );
				}
			}

			else if (healthPercentage <= 75)
			{
				if (firstHealthWarningPlayed == false)
				{
					firstHealthWarningPlayed = true;
					int gunboatHealthWarning01 = Commands->Create_Conversation( "M01_Gunboat_HealthWarning_01", 100, 1000, false);
					Commands->Join_Conversation( NULL, gunboatHealthWarning01, false, false, true );
					Commands->Join_Conversation( NULL, gunboatHealthWarning01, false, false, true );
					Commands->Start_Conversation( gunboatHealthWarning01,  gunboatHealthWarning01 );
				}
			}

			
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;
	
		if (param == M01_MODIFY_YOUR_ACTION_03_JDG)
		{
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_02_JDG);
			//params.Set_Movement( Vector3(0,0,0), .2f, 6 );
			//params.WaypathID = 101497;

			float pickTarget = Commands->Get_Random ( 0.5f, 2.5f );

			if (pickTarget >= 0.5f && pickTarget < 1.5f)
			{
				GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
				GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

				if (turret01 != NULL)
				{
					Vector3 myTarget = Commands->Get_Position ( turret01 );
					myTarget.Z += 2;
					params.Set_Attack( myTarget, 2000, 0, true );
				}

				
				else if (turret02 != NULL)
				{
					Vector3 myTarget = Commands->Get_Position ( turret02 );
					myTarget.Z += 2;
					params.Set_Attack( myTarget, 2000, 0, true );
				}
			}

			else
			{
				GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
				GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

				if (turret02 != NULL)
				{
					Vector3 myTarget = Commands->Get_Position ( turret02 );
					myTarget.Z += 2;
					params.Set_Attack( myTarget, 2000, 0, true );
				}

				else if (turret01 != NULL)
				{
					Vector3 myTarget = Commands->Get_Position ( turret01 );
					myTarget.Z += 2;
					params.Set_Attack( myTarget, 2000, 0, true );
				}
			}

			params.AttackActive		= true;
			params.AttackCheckBlocked = false;
			Commands->Modify_Action (obj, M01_WALKING_WAYPATH_02_JDG, params, true, true);

			float delayTimer = Commands->Get_Random ( 4, 6 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, delayTimer );
		}

		else if (param == M01_START_ACTING_JDG)
		{
			Commands->Debug_Message ( "***************************gunboat custom received--start acting\n" );
			Commands->Enable_Hibernation( obj, false );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_02_JDG);
			//params.Set_Movement( Vector3(0,0,0), .2f, 6 );
			//params.WaypathID = 101497;
			params.AttackActive	= false;

			Commands->Modify_Action (obj, M01_WALKING_WAYPATH_02_JDG, params, true, true);

			float delayTimer = Commands->Get_Random ( 0, 2 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, delayTimer );
		}

		else if (param == M01_GOTO_IDLE_JDG)
		{
			Commands->Debug_Message ( "***************************gunboat custom received--STOP acting\n" );
			Commands->Enable_Hibernation( obj, true );
		}

		else if (param == M01_END_TURRETS_OBJECTIVE_PASS_JDG)
		{
			Commands->Action_Reset ( obj, 100 );
			params.Set_Basic(this, 100, M01_WALKING_WAYPATH_03_JDG);
			params.Set_Movement( Vector3(0,0,0), .7f, 10 );
			params.WaypathID = 103454;
			Commands->Action_Goto( obj, params );

			GameObject * tankZone = Commands->Find_Object ( 102500 );
			if (tankZone != NULL)
			{
				Commands->Send_Custom_Event( obj, tankZone, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /* complete_reason */ ) override
	{
		ActionParamsStruct params;

		if (action_id == M01_WALKING_WAYPATH_03_JDG)//gunboat is at end of final waypath--delete it
		{ 
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_BaseCommander_JDG, "")//106050 Capt Duncan
{
	int gdi04conv01;
	bool gaveIonBeacon;
	int gdi04conv02;
	bool powsRescued;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gdi04conv01, 1);
		SAVE_VARIABLE(gaveIonBeacon, 2);
		SAVE_VARIABLE(gdi04conv02, 3);
		SAVE_VARIABLE(powsRescued, 4);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_GDI );
		powsRescued = false;
		gaveIonBeacon = false;
		Commands->Set_Innate_Is_Stationary ( obj, true );
		Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_NEUTRAL );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (gaveIonBeacon == false)
		{
			Commands->Set_Objective_Status( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
		}

		GameObject * shackzone = Commands->Find_Object ( 115985 );
		if (shackzone != NULL)
		{
			Commands->Send_Custom_Event( obj, shackzone, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		GameObject * pow01 = Commands->Find_Object ( 116386 );
		if (pow01 != NULL)
		{
			Commands->Apply_Damage( pow01, 10000, "Steel", NULL );
		}

		GameObject * pow02 = Commands->Find_Object ( 116385 );
		if (pow02 != NULL)
		{
			Commands->Apply_Damage( pow02, 10000, "Steel", NULL );
		}

		GameObject * baseMinigunner = Commands->Find_Object ( 116382 );
		if (baseMinigunner != NULL)
		{
			Commands->Send_Custom_Event( obj, baseMinigunner, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		GameObject * baseGrenadier = Commands->Find_Object ( 116383 );
		if (baseGrenadier != NULL)
		{
			Commands->Send_Custom_Event( obj, baseGrenadier, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		Vector3 nodGuy01_spot (28.566f, 29.790f, 1.792f);
		Vector3 nodGuy02_spot (65.899f, 46.347f, -0.294f);
		Vector3 nodGuy03_spot (24.776f, 44.863f, 0.708f);

		GameObject * nodGuy01 = Commands->Create_Object ( "Nod_MiniGunner_0", nodGuy01_spot );
		GameObject * nodGuy02 = Commands->Create_Object ( "Nod_MiniGunner_0", nodGuy02_spot );
		GameObject * nodGuy03 = Commands->Create_Object ( "Nod_MiniGunner_0", nodGuy03_spot );

		Commands->Set_Innate_Soldier_Home_Location ( nodGuy01, nodGuy01_spot, 10 );
		Commands->Set_Innate_Soldier_Home_Location ( nodGuy02, nodGuy02_spot, 10 );
		Commands->Set_Innate_Soldier_Home_Location ( nodGuy03, nodGuy03_spot, 10 );
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float myHealth = Commands->Get_Health ( obj );
		if (myHealth <= 10)
		{
			Commands->Set_Health ( obj, 10 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_GDI );
				Commands->Send_Custom_Event( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 1 );//
			}

			else if (param == M01_GDI_BASE_POWS_RESCUED_JDG)
			{
				powsRescued = true;
			}

			else if (param == M01_DO_END_MISSION_CHECK_JDG)
			{
				GameObject * nodGuy01 = Commands->Find_Object ( 101399 );
				GameObject * nodGuy02 = Commands->Find_Object ( 103335 );
				GameObject * nodGuy03 = Commands->Find_Object ( 103340 );
				GameObject * nodGuy04 = Commands->Find_Object ( 102185 );

				if ( (nodGuy01 == NULL) && (nodGuy02 == NULL) && (nodGuy03 == NULL) && (nodGuy04 == NULL) )
				{
					GameObject * letsgozone = Commands->Find_Object ( 105615 );
					if (letsgozone != NULL)
					{
						Commands->Destroy_Object ( letsgozone );
					}

					GameObject * shackzone = Commands->Find_Object ( 115985 );
					if (shackzone != NULL)
					{
						Commands->Send_Custom_Event( obj, shackzone, 0, M01_START_ACTING_JDG, 0 );
					}
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_DO_END_MISSION_CHECK_JDG, 0.25f );
				}
			}

			else if (param == M01_GOTO_YOUR_EVAC_SPOT_JDG)//
			{
				Commands->Enable_Hibernation( obj, false );
				Commands->Action_Reset ( obj, 100 );
				Vector3 myHomeSpot (55.9f, 40.3f, -0.2f);
				params.Set_Basic( this, 100, M01_GOTO_YOUR_EVAC_SPOT_JDG );
				params.Set_Movement(myHomeSpot, RUN, 1);

				Commands->Action_Goto(obj, params);
			}

			else if (param == M01_GOING_TO_EVAC_SPOT_JDG)//your ride is here--go get in
			{
				Commands->Action_Reset ( obj, 100 );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)
				{
					Commands->Set_Innate_Is_Stationary ( obj, false );
					Vector3 myHomeSpot = Commands->Get_Position ( myEvacController );
					params.Set_Basic( this, 100, M01_GOING_TO_EVAC_SPOT_JDG );
					params.Set_Movement(myHomeSpot, RUN, 1);

					Commands->Action_Goto(obj, params);
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)//havoc has saved you--start conversation
			{
				gdi04conv01 = Commands->Create_Conversation( "M01_GDI04_Conversation_01", 100, 1000, false);
				Commands->Join_Conversation( obj, gdi04conv01, false, true, true );
				Commands->Join_Conversation( STAR, gdi04conv01, false, false, false );
				Commands->Start_Conversation( gdi04conv01,  gdi04conv01 );

				Commands->Monitor_Conversation(  obj, gdi04conv01 );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == gdi04conv01)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
				params.Set_Animation ("H_A_J12C", false);
				Commands->Action_Play_Animation (obj, params);//

				Commands->Set_Objective_Status( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_STATUS_ACCOMPLISHED );

				/*GameObject * POW01 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY01_JDG );
				if (POW01 != NULL)
				{
					Commands->Send_Custom_Event( obj, POW01, 0, M01_CAPT_DUNCAN_HAS_BEEN_FOUND_JDG, 0 );
				}

				GameObject * POW02 = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_POWGUY02_JDG );
				if (POW02 != NULL)
				{
					Commands->Send_Custom_Event( obj, POW02, 0, M01_CAPT_DUNCAN_HAS_BEEN_FOUND_JDG, 0 );
				}*/
			}

			if (action_id == gdi04conv02)
			{
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_START_ACTING_JDG, 0 );
				}
			}
		}

		else if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_02_JDG)
			{
				
			}

			else if (action_id == M01_DOING_ANIMATION_01_JDG)
			{
				Vector3 myPostion (114.848f, 95.401f, 8.923f);

				Commands->Create_Object ( "POW_IonCannonBeacon_Player", myPostion );
				gaveIonBeacon = true;

				gdi04conv02 = Commands->Create_Conversation( "M01_GDI04_Conversation_02", 100, 1000, false);
				Commands->Join_Conversation( obj, gdi04conv02, false, true, true );
				Commands->Join_Conversation( STAR, gdi04conv02, false, false, false );
				Commands->Start_Conversation( gdi04conv02,  gdi04conv02 );

				Commands->Monitor_Conversation(  obj, gdi04conv02 );
			}

			else if (action_id == M01_GOTO_YOUR_EVAC_SPOT_JDG)
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to call in a chopper
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//start load animation here
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );
				Commands->Give_Points( STAR, 2000, false );

				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			if (gaveIonBeacon == false)
			{
				Commands->Set_Objective_Status( M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, OBJECTIVE_STATUS_FAILED );
			}

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_Base_GDI_Grenadier_JDG, "")//116383
{
	float currentHealth;
	bool invincible;
	bool ducanRescued;
	bool headingDownBackPath;
	bool still_in_GDI_Base;
	bool damagedByNod;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(currentHealth, 1);
		SAVE_VARIABLE(invincible, 2);
		SAVE_VARIABLE(ducanRescued, 3);
		SAVE_VARIABLE(headingDownBackPath, 4);
		SAVE_VARIABLE(still_in_GDI_Base, 5);
		SAVE_VARIABLE(damagedByNod, 6);
	}

	void Created( GameObject * obj ) override //this guy starts in the guard tower firing over at the conyard
	{
		damagedByNod = false;
		still_in_GDI_Base = true;
		invincible = true;
		ducanRescued = false;
		currentHealth = Commands->Get_Health ( obj );
		Commands->Set_Innate_Is_Stationary ( obj, true );
		Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 10 );//in 2 seconds run to a new position
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		GameObject * nodGuy01 = Commands->Find_Object ( 116388 );
		GameObject * nodGuy02 = Commands->Find_Object ( 116387 );

		if (damager == nodGuy01 || damager == nodGuy02) 
		{
			if (damagedByNod == false)
			{
				damagedByNod = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_04_JDG, 0 );
			}
		}

		if (invincible == true)
		{
			//Commands->Set_Health ( obj, currentHealth );
			float myCurrentHealth = Commands->Get_Health ( obj );
			if (myCurrentHealth <= 10)
			{
				Commands->Set_Health ( obj, 10 );
			}
		}

		else if (obj && damager == obj)
		{
			Commands->Set_Health ( obj, currentHealth );
		}

		else if (obj)
		{
			currentHealth = Commands->Get_Health ( obj );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG)//run over next to ladder
			{
				
				Vector3 newPosition (38.873f, -42.180f, -0.432f);

				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement(newPosition, RUN, 1);
				Commands->Action_Goto ( obj, params );
			}

			else if (param == M01_START_ATTACKING_01_JDG)//player is at top of ladders...check if nod minigunner is still alive
			{
				GameObject * nodMinigunner = Commands->Find_Object ( 116384 );
				if (nodMinigunner != NULL)
				{
					params.Set_Basic( this, 100, M01_START_ATTACKING_01_JDG );
					params.Set_Attack( nodMinigunner, 1000, 0, true );
					params.AttackCrouched = true;
					params.AttackCheckBlocked = false;

					Commands->Action_Attack (obj, params);
				}

				else
				{
					Commands->Action_Reset ( obj, 100 );
				}
			}

			else if (param == M01_START_ACTING_JDG)//
			{
				Commands->Action_Reset ( obj, 100 );

				params.Set_Basic(this, 100, M01_WALKING_WAYPATH_02_JDG);
				params.Set_Attack( STAR, 0, 0, true );

				Commands->Action_Attack (obj, params);

				float delayTimer = Commands->Get_Random ( 1, 2 );
				Commands->Send_Custom_Event ( obj, obj, 0, M01_MODIFY_YOUR_ACTION_02_JDG, delayTimer );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
				params.Set_Animation( "H_A_J19A", false );
				Commands->Action_Play_Animation ( obj, params );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_03_JDG)
			{
				//invincible = false;
				Vector3 newPosition (26.767f, 18.529f, 3.139f);

				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_03_JDG );
				params.Set_Movement(newPosition, RUN, 1);
				Commands->Action_Goto ( obj, params );
			}

			else if (param == M01_GOTO_YOUR_EVAC_SPOT_JDG)
			{
				Commands->Enable_Hibernation( obj, false );
				//evacing = true;
				ducanRescued = true;
				Commands->Action_Reset ( obj, 100 );
				Vector3 myHomeSpot (41.3f, 27.4f, -0.2f);
				params.Set_Basic( this, 100, M01_GOTO_YOUR_EVAC_SPOT_JDG );
				params.Set_Movement(myHomeSpot, RUN, 1);

				Commands->Action_Goto(obj, params);
			}

			else if (param == M01_GOING_TO_EVAC_SPOT_JDG)//your ride is here--go get in
			{
				Commands->Action_Reset ( obj, 100 );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)
				{
					Commands->Set_Innate_Is_Stationary ( obj, false );
					Vector3 myHomeSpot = Commands->Get_Position ( myEvacController );
					params.Set_Basic( this, 100, M01_GOING_TO_EVAC_SPOT_JDG );
					params.Set_Movement(myHomeSpot, RUN, 1);

					Commands->Action_Goto(obj, params);
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_06_JDG && ducanRescued == false)//Head down the path
			{
				still_in_GDI_Base = false;
				Commands->Action_Reset ( obj, 100 );
				headingDownBackPath = true;
				Vector3 newGotoSpot (111.210f, 85.507f, 8.934f);
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
				params.Set_Movement(newGotoSpot, RUN, 1);
				params.AttackActive = false;

				Commands->Action_Attack (obj, params);
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				if (still_in_GDI_Base == true)
				{
					invincible = false;
					Commands->Apply_Damage( obj, 100000, "Steel", NULL );
				}
			}

			else if (param == M01_WALKING_WAYPATH_04_JDG)
			{
				GameObject * nodguy02 = Commands->Find_Object ( 116387 );
				if (nodguy02 != NULL)
				{
					Vector3 myNewSpot (53.363f, 54.018f, -0.439f);
					Commands->Set_Innate_Is_Stationary ( obj, false );
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
					params.Set_Movement(myNewSpot, RUN, 1);
					params.Set_Attack( nodguy02, 20, 0, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack (obj, params);
				}

				else
				{
					Vector3 myNewSpot (53.363f, 54.018f, -0.439f);
					Commands->Set_Innate_Is_Stationary ( obj, false );
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
					params.Set_Movement(myNewSpot, RUN, 1);

					Commands->Action_Goto (obj, params);
				}
			}
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (headingDownBackPath == true)
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
			params.Set_Movement(enemy, WALK, 5);
			params.Set_Attack( enemy, 1000, 0, true );
			params.AttackActive = true;

			Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_06_JDG, params, true, true );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)
			{
				Vector3 myTarget (28.449f, -23.325f, 3.032f);
				
				Commands->Set_Innate_Is_Stationary ( obj, true );
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( myTarget, 1000, 0, true );
				params.AttackCrouched = true;
				params.AttackCheckBlocked = false;

				Commands->Action_Attack (obj, params);
			}

			else if (action_id == M01_DOING_ANIMATION_01_JDG)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
				params.Set_Animation( "H_A_J19S", false );
				Commands->Action_Play_Animation ( obj, params );
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "H_A_J19C", false );
				Commands->Action_Play_Animation ( obj, params );
			}

			else if (action_id == M01_GOTO_YOUR_EVAC_SPOT_JDG)
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to call in a chopper
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//start load animation here
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );

				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_03_JDG)//you're over by the pows now...check if nod guy 02 is still alive
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_04_JDG, 0 );
			}

			else if (action_id == M01_WALKING_WAYPATH_04_JDG)//your target is dead...make sure you go over to path
			{
				Vector3 myNewSpot (53.363f, 54.018f, -0.439f);
				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
				params.Set_Movement(myNewSpot, RUN, 1);

				Commands->Action_Goto (obj, params);
			}

			else if (action_id == M01_WALKING_WAYPATH_05_JDG)//you're by path entrance...goto stationary
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_WALKING_WAYPATH_06_JDG)//you just killed a back path guy...continue your journey
			{
				Vector3 newGotoSpot (111.210f, 85.507f, 8.934f);
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
				params.Set_Movement(newGotoSpot, RUN, 1);
				params.AttackActive = false;

				Commands->Action_Attack (obj, params);
			}
		}
	}
};





DECLARE_SCRIPT(M01_MediumTank_ReminderZone_JDG, "")//122848
{
	bool commandClearance;
	bool player_in_tank;
	int reminderConv;
	int tank_id;
	int reminders;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(commandClearance, 1);
		SAVE_VARIABLE(player_in_tank, 2);
		SAVE_VARIABLE(tank_id, 3);
		SAVE_VARIABLE(reminders, 4);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		commandClearance = false;
		player_in_tank = false;
		reminders = 0;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			if (commandClearance == true)
			{
				if (player_in_tank == true)//
				{
				}

				else
				{
					int tunnelConv = Commands->Create_Conversation( "M01_TunnelTank_Reminder_Conversation", 100, 30, false );
					Commands->Join_Conversation( NULL, tunnelConv, false, true, true );
					Commands->Start_Conversation( tunnelConv,  tunnelConv );

					Commands->Destroy_Object ( obj );
				}
			}
		}
	}

	void Timer_Expired( GameObject * obj, int /* timer_id */ ) override
	{
		if (player_in_tank == false)
		{
			reminders++;

			if (reminders == 1)
			{
				GameObject * mediumTank = Commands->Find_Object ( tank_id );
				if (mediumTank != NULL)
				{
					reminderConv = Commands->Create_Conversation( "M01_GetInTheTank_01_Conversation", 100, 30, false );
					Commands->Join_Conversation( NULL, reminderConv, false, true, true );
					Commands->Start_Conversation( reminderConv,  reminderConv );
					Commands->Monitor_Conversation(  obj, reminderConv );
				}
			}

			else if (reminders == 2)
			{
				GameObject * mediumTank = Commands->Find_Object ( tank_id );
				if (mediumTank != NULL)
				{
					reminderConv = Commands->Create_Conversation( "M01_GetInTheTank_02_Conversation", 100, 30, false );
					Commands->Join_Conversation( NULL, reminderConv, false, true, true );
					Commands->Start_Conversation( reminderConv,  reminderConv );
					Commands->Monitor_Conversation(  obj, reminderConv );
				}
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == reminderConv)
			{
				Commands->Start_Timer ( obj, this, 30, 0 );
				
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)//tank is here
			{
				commandClearance = true;
				Commands->Start_Timer ( obj, this, 10, 0 );
				tank_id = Commands->Get_ID ( obj );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)//player is in the tank
			{
				player_in_tank = true;
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)//player is out of the tank
			{
				player_in_tank = false;
			}
		}
	}
};

DECLARE_SCRIPT(M01_Base_POW01_JDG, "")//116386
{
	int deadGuards;
	bool pokable;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(deadGuards, 1);
		SAVE_VARIABLE(pokable, 2);
	}

	void Created( GameObject * obj ) override 
	{
		deadGuards = 0;
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_GDI );
		Commands->Set_Loiters_Allowed( obj, false );
		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
		params.Set_Animation( "H_A_HOST_L2B", true );
		Commands->Action_Play_Animation ( obj, params );
		pokable = false;
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float currentHealth = Commands->Get_Health ( obj );
		if (currentHealth <= 10)
		{
			Commands->Set_Health ( obj, 10 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * evacController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (evacController != NULL)
		{
			Commands->Send_Custom_Event( obj, evacController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override //M01_POW02_Conversation_01
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				deadGuards++;

				if (deadGuards == 2)
				{
					params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
					params.Set_Animation( "H_A_HOST_L2C", false );
					Commands->Action_Play_Animation ( obj, params );

					GameObject * pow02 = Commands->Find_Object ( 116385 );
					if (pow02 != NULL)
					{
						Commands->Send_Custom_Event( obj, pow02, 0, M01_START_ACTING_JDG, 1 );
					}

					Vector3 airdropPosition (55.062f, 60.164f, -1);
					GameObject *controller = Commands->Create_Object("Invisible_Object", airdropPosition);
					Commands->Set_Facing ( controller, -45 );
					Commands->Attach_Script(controller, "Test_Cinematic", "X1D_GDIPOW_FlareDrop.txt");
				}
			}

			else if (param == M01_GOING_TO_EVAC_SPOT02_JDG)
			{
				Commands->Enable_Hibernation( obj, false );
				Commands->Set_Innate_Is_Stationary ( obj, false );

				Vector3 myEvacSpot = Commands->Get_Position ( Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG ) );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( myEvacSpot, RUN, 1 );

				Commands->Action_Goto( obj, params );
			}

			else if (param == M01_GOTO_INNATE_JDG)
			{
				pokable = false;
				Commands->Enable_HUD_Pokable_Indicator ( obj, false );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_DOING_ANIMATION_02_JDG )//pow guy is standingup
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "H_A_HOST_L1C", false );
				Commands->Action_Play_Animation ( obj, params );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//play the Havoc conversation
			{
				GameObject * convController = Commands->Find_Object ( 121237 );
				if (convController != NULL)
				{
					Commands->Send_Custom_Event( obj, convController, 0, M01_START_ACTING_JDG, 0 );
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG)//gdi guy is at evac chopper--delete him
			{
				GameObject * airdropController = Commands->Find_Object ( M01_GDIBASE_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Base_POW02_JDG, "")//116385
{
	bool pokable;
	int pow02conv01;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(pokable, 1);
		SAVE_VARIABLE(pow02conv01, 2);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_GDI );
		Commands->Set_Loiters_Allowed( obj, false );
		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
		params.Set_Animation( "H_A_HOST_L2B", true );
		Commands->Action_Play_Animation ( obj, params );
		pokable = false;
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		float currentHealth = Commands->Get_Health ( obj );
		if (currentHealth <= 10)
		{
			Commands->Set_Health ( obj, 10 );
		}
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * evacController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
		if (evacController != NULL)
		{
			Commands->Send_Custom_Event( obj, evacController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override //M01_POW02_Conversation_01
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
				params.Set_Animation( "H_A_HOST_L2C", false );
				Commands->Action_Play_Animation ( obj, params );
			}

			else if (param == M01_GOING_TO_EVAC_SPOT02_JDG)
			{
				Commands->Enable_Hibernation( obj, false );
				Commands->Set_Innate_Is_Stationary ( obj, false );

				Vector3 myEvacSpot = Commands->Get_Position ( Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG ) );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( myEvacSpot, RUN, 1 );

				Commands->Action_Goto( obj, params );
			}

			else if (param == M01_GOTO_INNATE_JDG)
			{
				pokable = false;
				Commands->Enable_HUD_Pokable_Indicator ( obj, false );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_DOING_ANIMATION_02_JDG )//pow guy is standingup
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation( "H_A_HOST_L1C", false );
				Commands->Action_Play_Animation ( obj, params );
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//pow guy is standingup  -- do cheer anim
			{
				const char* animationName = M01_Choose_Cheer_Animation ( );

				params.Set_Basic( this, 100, M01_DOING_ANIMATION_04_JDG );
				params.Set_Animation( animationName, false );

				Commands->Action_Play_Animation ( obj, params );
			}

			else if (action_id == M01_DOING_ANIMATION_04_JDG )//pow guy is standingup  -- play havoc anim if pow01 is dead
			{
				GameObject * pow01 = Commands->Find_Object ( 116386 );
				if (pow01 == NULL)
				{
					GameObject * convController = Commands->Find_Object ( 121237 );
					if (convController != NULL)
					{
						Commands->Send_Custom_Event( obj, convController, 0, M01_START_ACTING_JDG, 0 );
					}
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG)//gdi guy is at evac chopper--delete him
			{
				GameObject * airdropController = Commands->Find_Object ( M01_GDIBASE_EVAC_MONITOR_JDG );
				if (airdropController != NULL)
				{
					Commands->Send_Custom_Event( obj, airdropController, 0, M01_GOING_TO_EVAC_SPOT_JDG, 0 );
				}

				GameObject * myEvacController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDIBase_POW_Conversation_Controller_JDG, "")//121237 
{
	bool convPlayed;
	int pow02conv01;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(convPlayed, 1);
		SAVE_VARIABLE(pow02conv01, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		convPlayed = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override //M01_POW02_Conversation_01
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG && convPlayed == false)
			{
				convPlayed = true;
				GameObject * pow01 = Commands->Find_Object ( 116386 );
				GameObject * pow02 = Commands->Find_Object ( 116385 );

				if (sender == pow01 && pow01 != NULL)
				{
					pow02conv01 = Commands->Create_Conversation( "M01_POW02_Conversation_01", 100, 1000, false );
					Commands->Join_Conversation( pow01, pow02conv01, false, true, true );
					Commands->Join_Conversation( STAR, pow02conv01, false, false, false );
					Commands->Start_Conversation( pow02conv01,  pow02conv01 );

					Commands->Monitor_Conversation(  obj, pow02conv01 );
				}

				else if (sender == pow02 && pow02 != NULL)
				{
					pow02conv01 = Commands->Create_Conversation( "M01_POW02_Conversation_01", 100, 1000, false );
					Commands->Join_Conversation( pow02, pow02conv01, false, true, true );
					Commands->Join_Conversation( STAR, pow02conv01, false, false, false );
					Commands->Start_Conversation( pow02conv01,  pow02conv01 );

					Commands->Monitor_Conversation(  obj, pow02conv01 );
				}

				if (pow01 != NULL)//M01_GOTO_INNATE_JDG
				{
					Commands->Send_Custom_Event( obj, pow01, 0, M01_GOTO_INNATE_JDG, 0 );
				}

				if (pow02 != NULL)
				{
					Commands->Send_Custom_Event( obj, pow02, 0, M01_GOTO_INNATE_JDG, 0 );
				}
				
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == pow02conv01)
			{
				GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
				if (pogController != NULL)
				{
					Commands->Send_Custom_Event( obj, pogController, M01_CHANGE_OBJECTIVE_POG_JDG, M01_GDI_BASE_COMMANDER_OBJECTIVE_JDG, 0 );
				}

				GameObject * evacController = Commands->Find_Object ( M01_GDIBASE_POWSCENE02_MONITOR_JDG );
				if (evacController != NULL)
				{
					Commands->Send_Custom_Event( obj, evacController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}

				GameObject * gdiMinigunner = Commands->Find_Object ( 116382 );
				if (gdiMinigunner != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiMinigunner, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 2 );
					//this tells him to start the "this is the path" routine
				}

				GameObject * backPathZone = Commands->Find_Object ( 121433 );
				if (backPathZone != NULL)
				{
					Commands->Send_Custom_Event( obj, backPathZone, 0, M01_START_ACTING_JDG, 0 );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_GDI_BeachGuy01_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Innate_Disable(obj);
		ActionParamsStruct params;
		Commands->Enable_Hibernation( obj, false );
		params.Set_Basic(this, 100, M01_WALKING_WAYPATH_01_JDG);
		params.Set_Movement( Vector3(36.231f, 28.471f, 0.131f), RUN, 2 );
		Commands->Action_Goto( obj, params );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		float currentHealth = Commands->Get_Health ( obj );
		if (currentHealth <= 10 && damager == STAR)
		{
			Commands->Set_Health ( obj, 10 );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)
			{
				Commands->Apply_Damage( obj, 10000, "STEEL", NULL );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Announce_TankAirstrikeZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * missionController = Commands->Find_Object ( M01_MISSION_CONTROLLER_JDG );
			if (missionController != NULL)
			{
				Commands->Send_Custom_Event( obj, missionController, 0, M01_ANNOUNCE_AIRSTRIKE_JDG, 0 );
			}

			Commands->Destroy_Object ( obj );
		}
	}
};












DECLARE_SCRIPT(M01_GDI_GuardTower_02_Enter_Zone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3());
			Commands->Attach_Script(sniperTarget01, "M01_GuardTower_Sniper_Target_JDG", "-90.385 124.308 1.610, -79.477 139.201 0.947");

			GameObject * sniperTarget02 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3());
			Commands->Attach_Script(sniperTarget02, "M01_GuardTower_Sniper_Target_JDG", "-85.499 117.221 1.915, -79.477 139.201 0.947");

			GameObject * sniperTarget03 = Commands->Create_Object ( "Nod_MiniGunner_0", Vector3());
			Commands->Attach_Script(sniperTarget03, "M01_GuardTower_Sniper_Target_JDG", "-93.364 113.202 1.701, -79.477 139.201 0.947");

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_GDI_GuardTower02_SniperRifle_JDG, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) //sniper rifle has been picked up--spawn the actors
		{
			GameObject * c4TextController = Commands->Find_Object ( 117188 );
			if (c4TextController != NULL)
			{
				Commands->Send_Custom_Event( obj, c4TextController, 2, 1, 0 );
			}

			
			Vector3 spawnLocation01 (-79.182f, 141.927f, 1.763f);
			GameObject * sniperTarget01 = Commands->Create_Object ( "Nod_MiniGunner_0", spawnLocation01);
			Commands->Attach_Script(sniperTarget01, "M01_GuardTower02_Sniper_Target01_JDG", "");
		}
	}
};

DECLARE_SCRIPT(M01_GuardTower02_Sniper_Target01_JDG, "")
{
	int guy02_id;
	int sniperConv;
	bool acting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(guy02_id, 1);
		SAVE_VARIABLE(sniperConv, 2);
		SAVE_VARIABLE(acting, 3);
	}

	void Created( GameObject * obj ) override 
	{
		acting = true;
		Commands->Enable_Hibernation( obj, false );
		Vector3 spawnLocation02 (-93.882f, 110.504f, 1.644f);
		GameObject * sniperTarget02 = Commands->Create_Object ( "Nod_MiniGunner_0", spawnLocation02);
		Commands->Attach_Script(sniperTarget02, "M01_GuardTower02_Sniper_Target02_JDG", "");
		Commands->Send_Custom_Event( obj, sniperTarget02, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 1 );
		guy02_id = Commands->Get_ID ( sniperTarget02  );

		Vector3 myGotoSpot (-92.496f, 117.750f, 1.752f);
		ActionParamsStruct params;
		params.Set_Basic( this, 80, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( myGotoSpot, RUN, 1 );

		Commands->Action_Goto( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * reinforceController = Commands->Find_Object ( 123645 );
		if (reinforceController != NULL)
		{
			Commands->Send_Custom_Event( obj, reinforceController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		if (acting == true)
		{
			GameObject * guy02 = Commands->Find_Object ( guy02_id );
			if (guy02 != NULL)
			{
				Commands->Send_Custom_Event( obj, guy02, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
			}
		}

	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_IVE_BEEN_KILLED_JDG)//your buddy's been capped--do look around anim
			{
				Commands->Action_Reset ( obj, 100 );
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
				params.Set_Animation ("H_A_A0A0_L21", false);
				Commands->Action_Play_Animation (obj, params);
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG )//youre now in middle of field--start conversation
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				GameObject * guy02 = Commands->Find_Object ( guy02_id );
				if (guy02 != NULL)
				{
					sniperConv = Commands->Create_Conversation( "M01_SniperTower_Conversation", 100, 80, true);
					Commands->Join_Conversation( obj, sniperConv, false, true, true );
					Commands->Join_Conversation( guy02, sniperConv, false, true, true );
					Commands->Start_Conversation( sniperConv,  sniperConv );

					Commands->Monitor_Conversation(  obj, sniperConv );
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG )//youre now at the tailguns--allow hibernation
			{
				Commands->Enable_Hibernation( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_01_JDG )//youre looked around--now set curent position as home spot
			{
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
			}
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == sniperConv )//conversation is over--go away
			{
				acting = false;
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Vector3 myGotoSpot (46.749f, 197.062f, 6.211f);
				ActionParamsStruct params;
				params.Set_Basic( this, 50, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Movement( myGotoSpot, RUN, 1 );

				Commands->Action_Goto( obj, params );

				GameObject * guy02 = Commands->Find_Object ( guy02_id );
				if (guy02 != NULL)
				{
					Commands->Send_Custom_Event( obj, guy02, 0, M01_START_ACTING_JDG, 0 );
				}
			}
		}
	}
};



DECLARE_SCRIPT(M01_GuardTower02_Sniper_TowerZone_JDG, "")//123645 
{
	bool player_in_zone;
	int targets_killed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(player_in_zone, 1);
		SAVE_VARIABLE(targets_killed, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		player_in_zone = false;
		targets_killed = 0;
	}

	void Entered( GameObject * /* obj */, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			player_in_zone = true;
		}
	}

	void Exited( GameObject * /* obj */, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			player_in_zone = false;
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_IVE_BEEN_KILLED_JDG)//a sniper targets been killed--check player position and spawn new target if needed
			{
				if (player_in_zone == true)
				{
					targets_killed++;

					if (targets_killed == 1)
					{
						Vector3 spawnLocation02 (-51.423f, 126.198f, 2.537f);
						GameObject * new_sniperTarget = Commands->Create_Object ( "Nod_MiniGunner_0", spawnLocation02);
						Commands->Attach_Script(new_sniperTarget, "M01_GuardTower02_NewSniperTarget_JDG", "");

						
					}

					else if (targets_killed == 2)
					{
					}
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_GuardTower02_NewSniperTarget_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		Commands->Enable_Hibernation( obj, false );

		Vector3 runToPosition (-67.347f, 118.048f, 2.641f);
		ActionParamsStruct params;
		params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( runToPosition, RUN, 1);

		Commands->Action_Goto ( obj, params );
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG )//now look over at Havoc
			{
				params.Set_Basic( this, 100, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
				params.Set_Attack( STAR, 0, 0, true );
				Commands->Action_Attack ( obj, params );

				Commands->Send_Custom_Event( obj, obj, 0, M01_FACING_SPECIFIED_DIRECTION_01_JDG, 1 );
			}

			else if (action_id == M01_DOING_ANIMATION_02_JDG )//now do neck chop anim
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_03_JDG );
				params.Set_Animation ("H_A_A0A0_L56", false);
				Commands->Action_Play_Animation (obj, params);
			}

			else if (action_id == M01_DOING_ANIMATION_03_JDG )//now go attack havoc
			{
				params.Set_Basic( this, 50, M01_FACING_SPECIFIED_DIRECTION_01_JDG );
				params.Set_Attack( STAR, 20, 0, true );
				params.Set_Movement( STAR, RUN, 5);
				Commands->Action_Attack ( obj, params );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_FACING_SPECIFIED_DIRECTION_01_JDG)
			{
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_02_JDG );
				params.Set_Animation ("H_A_A0A0_L34", false);
				Commands->Action_Play_Animation (obj, params);
			}
		}
	}
};

DECLARE_SCRIPT(M01_GuardTower02_Sniper_Target02_JDG, "")
{
	int guy01_id;
	bool acting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(guy01_id, 1);
		SAVE_VARIABLE(acting, 2);
	}

	void Created( GameObject * obj ) override 
	{
		acting = true;
		Commands->Enable_Hibernation( obj, false );
		
		Vector3 myGotoSpot (-92.927f, 115.587f, 1.847f);
		ActionParamsStruct params;
		params.Set_Basic( this, 80, M01_WALKING_WAYPATH_01_JDG );
		params.Set_Movement( myGotoSpot, WALK, 1 );

		Commands->Action_Goto( obj, params );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * reinforceController = Commands->Find_Object ( 123645 );
		if (reinforceController != NULL)
		{
			Commands->Send_Custom_Event( obj, reinforceController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		if (acting == true)
		{
			GameObject * guy01 = Commands->Find_Object ( guy01_id );
			if (guy01 != NULL)
			{
				Commands->Send_Custom_Event( obj, guy01, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG )//youre now in middle of field--start conversation
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_DOING_ANIMATION_01_JDG )//youre looked around--now set curent position as home spot
			{
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Vector3 myPosition = Commands->Get_Position ( obj );
				Commands->Set_Innate_Soldier_Home_Location ( obj, myPosition, 5 );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				acting = false;
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Vector3 myGotoSpot (-79.399f, 112.473f, 2.373f);
				
				params.Set_Basic( this, 50, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Movement( myGotoSpot, WALK, 1 );

				Commands->Action_Goto( obj, params );
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)//your buddy's been capped--do look around anim
			{
				Commands->Action_Reset ( obj, 100 );
				params.Set_Basic( this, 100, M01_DOING_ANIMATION_01_JDG );
				params.Set_Animation ("H_A_A0A0_L21", false);
				Commands->Action_Play_Animation (obj, params);
			}

			else if (param == M01_CHOPPER_IS_SENDING_ID_JDG)//guy 1 is actually sending his id
			{
				guy01_id = Commands->Get_ID ( sender );
			}
		}
	}
};

DECLARE_SCRIPT(M01_TurnOff_TankReminder_Zone_JDG, "")  
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * tankReminderZone = Commands->Find_Object ( 122848 );
			if (tankReminderZone != NULL)
			{
				Commands->Destroy_Object ( tankReminderZone );
				Commands->Destroy_Object ( obj );
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_Escorts_Warroom_MCT_ZoneController_JDG, "")//103378  
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * mctCommander = Commands->Find_Object ( 103373 );
			GameObject * mctProtector01 = Commands->Find_Object ( 103374 );
			GameObject * mctProtector02 = Commands->Find_Object ( 103375 );

			if (mctCommander != NULL)
			{
				Commands->Send_Custom_Event ( obj, mctCommander, 0, M01_START_ACTING_JDG, 0 );

				if ((mctProtector01 != NULL) && (mctProtector02 != NULL))
				{
					int protectMct_conv = Commands->Create_Conversation( "M01_Protect_The_MCT", 100, 1000, true);
					Commands->Join_Conversation( mctProtector02, protectMct_conv, false, false, true );
					Commands->Join_Conversation( mctProtector01, protectMct_conv, false, false, true );
					Commands->Start_Conversation( protectMct_conv,  protectMct_conv );
				}
			}

			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M01_HON_Chinook_Spawned_Soldier_01_GDI_JDG, "")
{
	bool player_in_warroom;
	bool at_final_destination;
	bool escorting;
	int dialogs_said;
	int dialogs_said_to_havoc;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(player_in_warroom, 1);
		SAVE_VARIABLE(at_final_destination, 2);
		SAVE_VARIABLE(escorting, 3);
		SAVE_VARIABLE(dialogs_said, 4);
		SAVE_VARIABLE(dialogs_said_to_havoc, 5);
	}

	void Created( GameObject * obj ) override 
	{
		dialogs_said = 0;
		dialogs_said_to_havoc = 0;
		player_in_warroom = false;
		at_final_destination = false;
		escorting = true;
		Commands->Enable_Hibernation( obj, false );
		Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
		Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );
		GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
		if (HON_MonitorZone != NULL)
		{
			Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_ROPE_IS_SENDING_ID_JDG, 0 );
		}

		GameObject * MCT_talkZone = Commands->Find_Object ( 124260 );
		if (MCT_talkZone != NULL)
		{
			Commands->Send_Custom_Event( obj, MCT_talkZone, 0, M01_ROPE_IS_SENDING_ID_JDG, 0 );
		}
	}
	//Vector3 myGotoLocation (-187.848f, 526.480f, 3.518f);
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)	
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_01_JDG, 0 );
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 5 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (escorting == true)
		{
			GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
			if (HON_MonitorZone != NULL)
			{
				Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (obj && type == 0)
		{
			if (param == M01_WALKING_WAYPATH_01_JDG && player_in_warroom == false)
			{
				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					Vector3 myGotoLocation (-187.848f, 526.480f, 3.518f);
					ActionParamsStruct params;
					params.Set_Basic(this, 60, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement(myGotoLocation, RUN, 1);
					Commands->Action_Goto(obj, params);

					Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_01_JDG, 5 );
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
				
			}

			else if (param == M01_START_ACTING_JDG && at_final_destination == false)
			{
				player_in_warroom = true;

				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					Vector3 myGotoLocation (-181.084f, 541.058f, 3.518f);
					ActionParamsStruct params;
					params.Set_Basic(this, 60, M01_WALKING_WAYPATH_02_JDG);
					params.Set_Movement(myGotoLocation, WALK, 1);
					Commands->Action_Goto(obj, params);
					
					Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 5 );
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
			}

			else if (param == M01_GOTO_INNATE_JDG)//escort scenario is over--go to top of HON and goto innate
			{
				escorting = false;
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Commands->Enable_Hibernation( obj, true );
				Vector3 myGotoLocation (-181.163f, 534.529f, 10.034f);
				ActionParamsStruct params;
				params.Set_Basic(this, 60, M01_WALKING_WAYPATH_03_JDG);
				params.Set_Movement(myGotoLocation, RUN, 5);
				Commands->Action_Goto(obj, params);

				Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 5 );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG && escorting == true)//pick a sound to play
			{
				/*Vector3 myPosition = Commands->Get_Position ( obj );
				Vector3 playerPosition = Commands->Get_Position ( STAR );

				float distance = Commands->Get_Distance ( myPosition, playerPosition );

				if (distance <= 8)
				{
					if (dialogs_said_to_havoc == 0)
					{
						Commands->Create_Sound ( "MX1DSGN_DSGN0024I1DSGN_SND", myPosition, obj );//follow us inside
					}

					else if (dialogs_said_to_havoc == 1)
					{
						Commands->Create_Sound ( "MX1DSGN_DSGN0027I1DSGN_SND", myPosition, obj );//this way sir
					}

					dialogs_said_to_havoc++;

					if (dialogs_said_to_havoc >= 2)
					{
						dialogs_said_to_havoc = 1;
					}
				}

				else
				{
					if (dialogs_said == 0)
					{
						Commands->Create_Sound ( "MX1DSGN_DSGN0025I1DSGN_SND", myPosition, obj );//lets do it
					}

					else if (dialogs_said == 1)
					{
						Commands->Create_Sound ( "MX1DSGN_DSGN0026I1DSGN_SND", myPosition, obj );
					}

					else if (dialogs_said == 2)
					{
						Commands->Create_Sound ( "MX1DSGN_DSGN0028I1DSGN_SND", myPosition, obj );
					}

					dialogs_said++;

					if (dialogs_said >= 3)
					{
						dialogs_said = 0;
					}
				}

				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 5 );*/
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_02_JDG)
			{
				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					at_final_destination = true;
					Commands->Set_Innate_Is_Stationary ( obj, true );
					GameObject * MCTspeechZone = Commands->Find_Object ( 124260 );
					if (MCTspeechZone != NULL)
					{
						Commands->Send_Custom_Event( obj, MCTspeechZone, 0, M01_START_ACTING_JDG, 0 );
					}
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_Chinook_Spawned_Soldier_02_GDI_JDG, "")
{
	bool player_in_warroom;
	bool at_final_destination;
	bool escorting;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(player_in_warroom, 1);
		SAVE_VARIABLE(at_final_destination, 2);
		SAVE_VARIABLE(escorting, 3);
	}

	void Created( GameObject * obj ) override 
	{
		player_in_warroom = false;
		at_final_destination = false;
		escorting = true;
		Commands->Enable_Hibernation( obj, false );
		Commands->Innate_Soldier_Enable_Gunshot_Heard ( obj, false );
		Commands->Innate_Soldier_Enable_Bullet_Heard ( obj, false );

		GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
		if (HON_MonitorZone != NULL)
		{
			Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_WAYPATH_IS_SENDING_ID_JDG, 0 );
		}

		GameObject * MCT_talkZone = Commands->Find_Object ( 124260 );
		if (MCT_talkZone != NULL)
		{
			Commands->Send_Custom_Event( obj, MCT_talkZone, 0, M01_WAYPATH_IS_SENDING_ID_JDG, 0 );
		}
	}
	//Vector3 myGotoLocation (-188.670f, 530.083f, 3.518f);
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "s_a_human.h_a_troopdrop") == 0)	
		{
			Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_01_JDG, 0 );
		}
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		if (escorting == true)
		{
			GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
			if (HON_MonitorZone != NULL)
			{
				Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (obj && type == 0)
		{
			if (param == M01_WALKING_WAYPATH_01_JDG && player_in_warroom == false)
			{
				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					Vector3 myGotoLocation (-188.670f, 530.083f, 3.518f);
					ActionParamsStruct params;
					params.Set_Basic(this, 60, M01_WALKING_WAYPATH_01_JDG);
					params.Set_Movement(myGotoLocation, RUN, 1);
					Commands->Action_Goto(obj, params);

					Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_01_JDG, 5 );
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
			}

			else if (param == M01_START_ACTING_JDG && at_final_destination == false)
			{
				player_in_warroom = true;

				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					Vector3 myGotoLocation (-179.440f, 535.697f, 3.518f);
					ActionParamsStruct params;
					params.Set_Basic(this, 60, M01_WALKING_WAYPATH_02_JDG);
					params.Set_Movement(myGotoLocation, WALK, 0);
					Commands->Action_Goto(obj, params);
					
					Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 5 );
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
			}

			else if (param == M01_GOTO_INNATE_JDG)//escort scenario is over--go to top of HON and goto innate
			{
				escorting = false;
				Commands->Set_Innate_Is_Stationary ( obj, false );
				Commands->Enable_Hibernation( obj, true );
				Vector3 myGotoLocation (-178.263f, 528.363f, 10.034f);
				ActionParamsStruct params;
				params.Set_Basic(this, 60, M01_WALKING_WAYPATH_03_JDG);
				params.Set_Movement(myGotoLocation, RUN, 5);
				Commands->Action_Goto(obj, params);

				Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 5 );
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_02_JDG)
			{
				float handOfNodHealth = Commands->Get_Health ( Commands->Find_Object ( M01_HAND_OF_NOD_JDG ) );

				if (handOfNodHealth > 0)
				{
					at_final_destination = true;
					Commands->Set_Innate_Is_Stationary ( obj, true );
					GameObject * MCTspeechZone = Commands->Find_Object ( 124260 );
					if (MCTspeechZone != NULL)
					{
						Commands->Send_Custom_Event( obj, MCTspeechZone, 0, M01_START_ACTING_JDG, 0 );
					}
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_GOTO_INNATE_JDG, 0 );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_GiveMCTSpeech_Zone_JDG, "")//124260
{
	bool commandClearance;
	bool player_in_warroom;
	int gdiGuy01_id;
	int gdiGuy02_id;
	bool convPlaying;
	int hon_gdi_guy_conv1;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(commandClearance, 1);
		SAVE_VARIABLE(player_in_warroom, 2);
		SAVE_VARIABLE(gdiGuy01_id, 3);
		SAVE_VARIABLE(gdiGuy02_id, 4);
		SAVE_VARIABLE(convPlaying, 5);
		SAVE_VARIABLE(hon_gdi_guy_conv1, 6);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		player_in_warroom = false;
		commandClearance = false;
		convPlaying = false;
		gdiGuy01_id = 0;
		gdiGuy02_id = 0;
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == hon_gdi_guy_conv1)
			{
				GameObject * gdiGuy01 = Commands->Find_Object ( gdiGuy01_id );
				GameObject * gdiGuy02 = Commands->Find_Object ( gdiGuy02_id );

				if (gdiGuy01 != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiGuy01, 0, M01_GOTO_INNATE_JDG, 0 );
				}

				if (gdiGuy02 != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiGuy02, 0, M01_GOTO_INNATE_JDG, 0 );
				}

				Commands->Destroy_Object ( obj );
			}
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (obj && type == 0)
		{
			if (param == M01_ROPE_IS_SENDING_ID_JDG)//gdi guy 01 is actually sending his id
			{
				gdiGuy01_id = Commands->Get_ID ( sender );
			}

			else if (param == M01_WAYPATH_IS_SENDING_ID_JDG)//gdi guy 02 is actually sending his id
			{
				gdiGuy02_id = Commands->Get_ID ( sender );
			}

			else if (param == M01_START_ACTING_JDG)
			{
				commandClearance = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				if (commandClearance == true && player_in_warroom == true)
				{
					GameObject * gdiGuy01 = Commands->Find_Object ( gdiGuy01_id );
					GameObject * gdiGuy02 = Commands->Find_Object ( gdiGuy02_id );

					if (gdiGuy01 != NULL && convPlaying == false)
					{
						convPlaying = true;
						hon_gdi_guy_conv1 = Commands->Create_Conversation( "M01_GDI_HON_MCT", 100, 1000, false);
						Commands->Join_Conversation( gdiGuy01, hon_gdi_guy_conv1, false, true, true );
						Commands->Join_Conversation( STAR, hon_gdi_guy_conv1, false, false, false );
						Commands->Start_Conversation( hon_gdi_guy_conv1,  hon_gdi_guy_conv1 );
													
						Commands->Monitor_Conversation(  obj, hon_gdi_guy_conv1 );
					}

					else if (gdiGuy02 != NULL && convPlaying == false)
					{
						convPlaying = true;
						hon_gdi_guy_conv1 = Commands->Create_Conversation( "M01_GDI_HON_MCT", 100, 1000, false);
						Commands->Join_Conversation( gdiGuy02, hon_gdi_guy_conv1, false, true, true );
						Commands->Join_Conversation( STAR, hon_gdi_guy_conv1, false, false, false );
						Commands->Start_Conversation( hon_gdi_guy_conv1,  hon_gdi_guy_conv1 );
													
						Commands->Monitor_Conversation(  obj, hon_gdi_guy_conv1 );
					}
				}
			
			}
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			player_in_warroom = true;
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
		}
	}

	void Exited( GameObject * /* obj */, GameObject * exiter ) override
	{
		if (exiter == STAR)
		{
			player_in_warroom = false;
		}
	}
};


DECLARE_SCRIPT(M01_Havoc_In_WarroomZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
			if (HON_MonitorZone != NULL)
			{
				Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_PLAYER_IS_INSIDE_REFINERY_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Havoc_Out_WarroomZone_JDG, "")
{
	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			GameObject * HON_MonitorZone = Commands->Find_Object ( 124044 );
			if (HON_MonitorZone != NULL)
			{
				Commands->Send_Custom_Event( obj, HON_MonitorZone, 0, M01_PLAYER_IS_OUTSIDE_REFINERY_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_HON_WarroomController_JDG, "")//124044
{
	int gdiGuy01_id;
	int gdiGuy02_id;
	bool player_in_warroom;
	bool scenario_over;
	int dead_soldier_tally;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gdiGuy01_id, 1);
		SAVE_VARIABLE(gdiGuy02_id, 2);
		SAVE_VARIABLE(player_in_warroom, 3);
		SAVE_VARIABLE(scenario_over, 4);
		SAVE_VARIABLE(dead_soldier_tally, 5);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		gdiGuy01_id = 0;
		gdiGuy02_id = 0;
		player_in_warroom = false;
		scenario_over = false;
		dead_soldier_tally = 0;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (obj && type == 0)
		{
			if (param == M01_ROPE_IS_SENDING_ID_JDG)//gdi guy 01 is actually sending his id
			{
				gdiGuy01_id = Commands->Get_ID ( sender );
			}

			else if (param == M01_WAYPATH_IS_SENDING_ID_JDG)//gdi guy 02 is actually sending his id
			{
				gdiGuy02_id = Commands->Get_ID ( sender );
			}

			else if (param == M01_PLAYER_IS_INSIDE_REFINERY_JDG)//player is actually in warroom
			{
				player_in_warroom = true;
			}

			else if (param == M01_PLAYER_IS_OUTSIDE_REFINERY_JDG)//player just left the warroom
			{
				player_in_warroom = false;
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				dead_soldier_tally++;

				if (dead_soldier_tally == 2)
				{
					dead_soldier_tally = 0;

					GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-147.1f, 558.8f, 4.4f));
					Commands->Set_Facing(chinook_obj1, -130.00f);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X1I_GDI_HON_TroopDrop01a.txt");
				}
			}
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		GameObject * gdiGuy01 = Commands->Find_Object ( gdiGuy01_id );
		GameObject * gdiGuy02 = Commands->Find_Object ( gdiGuy02_id );

		if (enterer == gdiGuy01)
		{
			if (player_in_warroom == false)
			{
				Commands->Apply_Damage( gdiGuy01, 10000.0f, "STEEL", NULL );
			}

			else if (gdiGuy01 != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiGuy01, 0, M01_START_ACTING_JDG, 0 );
			}
		}

		else if (enterer == gdiGuy02)
		{
			if (player_in_warroom == false)
			{
				Commands->Apply_Damage( gdiGuy02, 10000.0f, "STEEL", NULL );
			}

			else if (gdiGuy02 != NULL)
			{
				Commands->Send_Custom_Event( obj, gdiGuy02, 0, M01_START_ACTING_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Medium_Tank01_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		GameObject * hovercraftExplosionController = Commands->Find_Object ( 121839 );
		if (hovercraftExplosionController != NULL)
		{
			Commands->Send_Custom_Event( obj, hovercraftExplosionController, 0, M01_MEDIUM_TANK_IS_HERE_JDG, 0 );
		}
	}
};

DECLARE_SCRIPT(M01_CantBring_MediumTank_ThroughHereZone_JDG, "")
{
	bool player_in_tank;
	int tank_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(player_in_tank, 1);
		SAVE_VARIABLE(tank_id, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		player_in_tank = false;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG )
			{
				player_in_tank = true;
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG )
			{
				player_in_tank = false;
			}

			else if (param == M01_MODIFY_YOUR_ACTION_03_JDG )//medium tank is sending his id
			{
				tank_id = Commands->Get_ID ( sender );
			}
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		GameObject * tank = Commands->Find_Object ( tank_id );
		if (enterer == tank)
		{
			if (player_in_tank == true)//M01_Must_Leave_Tank_Conversation
			{
				int hoofit_conv = Commands->Create_Conversation( "M01_Must_Leave_Tank_Conversation", 100, 100, true);
				Commands->Join_Conversation( NULL, hoofit_conv, true, true, true );
				Commands->Start_Conversation( hoofit_conv,  hoofit_conv );

				Commands->Destroy_Object ( obj );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Medium_Tank_JDG, "")
{
	bool firstTimeEntered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(firstTimeEntered, 1);
	}

	void Created( GameObject * obj ) override 
	{
		Commands->Create_Sound ( "m00evag_dsgn0009i1evag_snd", Vector3 (0,0,0), obj);

		firstTimeEntered = true;

		int m01_medium_tank_ID = Commands->Get_ID ( obj );

		GameObject * tunnelZone = Commands->Find_Object ( M01_TANK_TUNNEL_ZONE_ID );
		if (tunnelZone != NULL)
		{
			Commands->Send_Custom_Event( obj, tunnelZone, M01_MEDIUM_TANK_IS_HERE_JDG, m01_medium_tank_ID, 0 );
		}

		GameObject * honSamSite = Commands->Find_Object ( M01_HON_SAM_SITE_JDG );
		if (honSamSite != NULL)
		{
			Commands->Send_Custom_Event( obj, honSamSite, M00_CUSTOM_SAM_SITE_IGNORE, m01_medium_tank_ID, 0 );
		}

		GameObject * fodderHovercraftController = Commands->Find_Object ( 121630 );
		if (fodderHovercraftController != NULL)
		{
			Commands->Destroy_Object ( fodderHovercraftController );
		}

		GameObject * tankReminderZone = Commands->Find_Object ( 122848 );
		if (tankReminderZone != NULL)
		{
			Commands->Send_Custom_Event( obj, tankReminderZone, 0, M01_START_ACTING_JDG, 0 );
		}

		GameObject * tankBarrierZone = Commands->Find_Object ( 125661 );
		if (tankBarrierZone != NULL)
		{
			Commands->Send_Custom_Event( obj, tankBarrierZone, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override //
	{
		if (type == CUSTOM_EVENT_VEHICLE_ENTERED )
		{
			if (firstTimeEntered == true)
			{
				firstTimeEntered = false;
				Vector3 guy01Position (40.286f, 151.346f, 2.107f);
				Vector3 guy02Position (47.476f, 163.355f, 2.458f);
				Vector3 guy03Position (59.446f, 175.107f, -0.453f);
				GameObject * tunnelGuy01 = Commands->Create_Object ( "Nod_Minigunner_0", guy01Position );
				GameObject * tunnelGuy02 = Commands->Create_Object ( "Nod_Minigunner_0", guy02Position );
				GameObject * tunnelGuy03 = Commands->Create_Object ( "Nod_Minigunner_0", guy03Position );

				Commands->Attach_Script(tunnelGuy01, "M01_Medium_Tank_Tunnel_Squish_Guy_JDG", "");
				Commands->Attach_Script(tunnelGuy02, "M01_Medium_Tank_Tunnel_Squish_Guy_JDG", "");
				Commands->Attach_Script(tunnelGuy03, "M01_Medium_Tank_Tunnel_Squish_Guy_JDG", "");

				Commands->Action_Reset ( obj, 100 );
			}

			GameObject * tankReminderZone = Commands->Find_Object ( 122848 );
			if (tankReminderZone != NULL)
			{
				Commands->Send_Custom_Event( obj, tankReminderZone, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}

			GameObject * tankBarrierZone = Commands->Find_Object ( 125661 );
			if (tankBarrierZone != NULL)
			{
				Commands->Send_Custom_Event( obj, tankBarrierZone, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
			}
			
		}

		else if (type == CUSTOM_EVENT_VEHICLE_EXITED )
		{
			GameObject * tankReminderZone = Commands->Find_Object ( 122848 );
			if (tankReminderZone != NULL)
			{
				Commands->Send_Custom_Event( obj, tankReminderZone, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 0 );
			}

			GameObject * tankBarrierZone = Commands->Find_Object ( 125661 );
			if (tankBarrierZone != NULL)
			{
				Commands->Send_Custom_Event( obj, tankBarrierZone, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 0 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_TurretBeach_FodderHovercraft_Controller_JDG, "")//121630  M01_START_ACTING_JDG
{
	int hovercraft_ID;
	bool hovercraftAlive;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(hovercraftAlive, 1);
		SAVE_VARIABLE(hovercraft_ID, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		hovercraftAlive = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG && hovercraftAlive == false)//turret objective has been added--start bringing in hovercraft
			{
				GameObject * oldHovercraft = Commands->Find_Object ( hovercraft_ID );
				if (oldHovercraft == NULL)
				{
					hovercraftAlive = true;
					GameObject * hovercraft = Commands->Create_Object ( "Invisible_Object", Vector3 (0,0,0));
					Commands->Attach_Script(hovercraft, "Test_Cinematic", "X1D_FodderHover_MTank.txt");
				}

				else
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 5 );
				}
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				hovercraftAlive = false;
				Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 15 );
			}

			else if (param == 0)//hovercraft is sending you its ID
			{
				hovercraft_ID = Commands->Get_ID ( sender );
				hovercraftAlive = true;

				GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
				if (turret01 != NULL)
				{
					Commands->Send_Custom_Event( obj, turret01, M01_FODDER_HOVERCRAFT_IS_HERE, hovercraft_ID, 1 );
				}

				GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );
				if (turret02 != NULL)
				{
					Commands->Send_Custom_Event( obj, turret02, M01_FODDER_HOVERCRAFT_IS_HERE, hovercraft_ID, 1 );
				}

				//Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 15 );
			}
		}
	}
};


DECLARE_SCRIPT(M01_Hovercraft_Explosion_Controller_JDG, "")//121839
{
	int tank_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(tank_id, 1);
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * sender ) override
	{
		if (type == 0)
		{
			if (param == M01_MEDIUM_TANK_IS_HERE_JDG)//medium tank is sending its id
			{
				tank_id = Commands->Get_ID ( sender );
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)//hovercraft has been destroyed--delete tank and play explosion
			{
				GameObject * mediumTank = Commands->Find_Object ( tank_id );
				if (mediumTank != NULL)
				{
					//Commands->Destroy_Object ( mediumTank );
					Commands->Apply_Damage( mediumTank, 100000, "Steel", NULL );
				}
			}
		}
	}
};



DECLARE_SCRIPT(M01_FodderHovercraft_Script_JDG, "")
{
	void Created( GameObject * obj ) override 
	{
		GameObject * fodderHovercraftController = Commands->Find_Object ( 121630 );
		if (fodderHovercraftController != NULL)
		{
			Commands->Send_Custom_Event( obj, fodderHovercraftController, 0, 0, 0 );
		}

		//GameObject * hovercraftExplosionController = Commands->Find_Object ( 121839 );
		//if (hovercraftExplosionController != NULL)
		//{
		//	Commands->Send_Custom_Event( obj, hovercraftExplosionController, 0, M01_CHOPPER_IS_SENDING_ID_JDG, 0 );
		//}
		
		float delayTimer = Commands->Get_Random ( 5, 8 );
		Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, delayTimer );
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		//Vector3 myPosition = Commands->Get_Position ( obj );
		//Commands->Create_Explosion ( "Ground Explosions Twiddler", myPosition, NULL );

		GameObject * fodderHovercraftController = Commands->Find_Object ( 121630 );
		if (fodderHovercraftController != NULL)
		{
			Commands->Send_Custom_Event( obj, fodderHovercraftController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}

		//GameObject * fodderHovercraftController = Commands->Find_Object ( 121630 );
		//if (fodderHovercraftController != NULL)
		//{
		//	Commands->Send_Custom_Event( obj, fodderHovercraftController, 0, M01_START_ACTING_JDG, 5 );
		//}

		GameObject * hovercraftExplosionController = Commands->Find_Object ( 121839 );
		if (hovercraftExplosionController != NULL)
		{
			Commands->Send_Custom_Event( obj, hovercraftExplosionController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_START_ACTING_JDG)
			{
				GameObject * turret01 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_01_ID );
				GameObject * turret02 = Commands->Find_Object ( M01_TURRETBEACH_TURRET_02_ID );

				if (turret01 != NULL)
				{
					Commands->Apply_Damage( obj, 100000, "STEEL", NULL );
				}

				else if (turret02 != NULL)
				{
					Commands->Apply_Damage( obj, 100000, "STEEL", NULL );
				}
			}
		}
	}
};

DECLARE_SCRIPT(M01_Beach_Datadisc_JDG, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) //shows the GDI base
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}

			Commands->Clear_Map_Cell ( 10, 15 );
			Commands->Clear_Map_Cell ( 10, 16 );
			Commands->Clear_Map_Cell ( 10, 17 );
			Commands->Clear_Map_Cell ( 10, 18 );
			Commands->Clear_Map_Cell ( 10, 19 );
			Commands->Clear_Map_Cell ( 10, 20 );

			Commands->Clear_Map_Cell ( 11, 15 );
			Commands->Clear_Map_Cell ( 11, 16 );
			Commands->Clear_Map_Cell ( 11, 17 );
			Commands->Clear_Map_Cell ( 11, 18 );
			Commands->Clear_Map_Cell ( 11, 19 );
			Commands->Clear_Map_Cell ( 11, 20 );

			Commands->Clear_Map_Cell ( 12, 15 );
			Commands->Clear_Map_Cell ( 12, 16 );
			Commands->Clear_Map_Cell ( 12, 17 );
			Commands->Clear_Map_Cell ( 12, 18 );
			Commands->Clear_Map_Cell ( 12, 19 );
			Commands->Clear_Map_Cell ( 12, 20 );

			Commands->Clear_Map_Cell ( 13, 15 );
			Commands->Clear_Map_Cell ( 13, 16 );
			Commands->Clear_Map_Cell ( 13, 17 );
			Commands->Clear_Map_Cell ( 13, 18 );
			Commands->Clear_Map_Cell ( 13, 19 );
			Commands->Clear_Map_Cell ( 13, 20 );

			Commands->Clear_Map_Cell ( 14, 15 );
			Commands->Clear_Map_Cell ( 14, 16 );
			Commands->Clear_Map_Cell ( 14, 17 );
			Commands->Clear_Map_Cell ( 14, 18 );
			Commands->Clear_Map_Cell ( 14, 19 );
			Commands->Clear_Map_Cell ( 14, 20 );
		}
	}
};

DECLARE_SCRIPT(M01_TibCave01_Datadisc_JDG, "")//shows the barn area
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}

			Commands->Clear_Map_Cell ( 6, 13 );
			Commands->Clear_Map_Cell ( 6, 14 );

			Commands->Clear_Map_Cell ( 7, 11 );
			Commands->Clear_Map_Cell ( 7, 12 );
			Commands->Clear_Map_Cell ( 7, 13 );
			Commands->Clear_Map_Cell ( 7, 14 );
			Commands->Clear_Map_Cell ( 7, 15 );

			Commands->Clear_Map_Cell ( 8, 11 );
			Commands->Clear_Map_Cell ( 8, 12 );
			Commands->Clear_Map_Cell ( 8, 13 );
			Commands->Clear_Map_Cell ( 8, 14 );
			Commands->Clear_Map_Cell ( 8, 15 );

			Commands->Clear_Map_Cell ( 9, 11 );
			Commands->Clear_Map_Cell ( 9, 12 );
			Commands->Clear_Map_Cell ( 9, 13 );
			Commands->Clear_Map_Cell ( 9, 14 );
			Commands->Clear_Map_Cell ( 9, 15 );

			Commands->Clear_Map_Cell ( 10, 11 );
			Commands->Clear_Map_Cell ( 10, 12 );
			Commands->Clear_Map_Cell ( 10, 13 );
			Commands->Clear_Map_Cell ( 10, 14 );
			Commands->Clear_Map_Cell ( 10, 15 );

			Commands->Clear_Map_Cell ( 11, 12 );
			Commands->Clear_Map_Cell ( 11, 13 );
			Commands->Clear_Map_Cell ( 11, 14 );
			Commands->Clear_Map_Cell ( 11, 15 );
		}
	}
};

DECLARE_SCRIPT(M01_Shed_Datadisc_JDG, "")//shows the light tank area
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}

			Commands->Clear_Map_Cell ( 9, 9 );
			Commands->Clear_Map_Cell ( 9, 10 );

			Commands->Clear_Map_Cell ( 10, 9 );
			Commands->Clear_Map_Cell ( 10, 10 );
			Commands->Clear_Map_Cell ( 10, 11 );

			Commands->Clear_Map_Cell ( 11, 9 );
			Commands->Clear_Map_Cell ( 11, 10 );
			Commands->Clear_Map_Cell ( 11, 11 );
			Commands->Clear_Map_Cell ( 11, 12 );

			Commands->Clear_Map_Cell ( 12, 9 );
			Commands->Clear_Map_Cell ( 12, 10 );
			Commands->Clear_Map_Cell ( 12, 11 );
			Commands->Clear_Map_Cell ( 12, 12 );

			Commands->Clear_Map_Cell ( 13, 9 );
			Commands->Clear_Map_Cell ( 13, 10 );
			Commands->Clear_Map_Cell ( 13, 11 );
			Commands->Clear_Map_Cell ( 13, 12 );

			Commands->Clear_Map_Cell ( 14, 10 );
			Commands->Clear_Map_Cell ( 14, 11 );
			Commands->Clear_Map_Cell ( 14, 12 );
		}
	}
};

DECLARE_SCRIPT(M01_TibCave02_Datadisc_JDG, "")//tib cave no
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) //shows the church and tib field
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}

			Commands->Clear_Map_Cell ( 6, 5 );
			Commands->Clear_Map_Cell ( 6, 6 );
			Commands->Clear_Map_Cell ( 6, 7 );
			Commands->Clear_Map_Cell ( 6, 8 );
			Commands->Clear_Map_Cell ( 6, 9 );

			Commands->Clear_Map_Cell ( 7, 5 );
			Commands->Clear_Map_Cell ( 7, 6 );
			Commands->Clear_Map_Cell ( 7, 7 );
			Commands->Clear_Map_Cell ( 7, 8 );
			Commands->Clear_Map_Cell ( 7, 9 );

			Commands->Clear_Map_Cell ( 8, 5 );
			Commands->Clear_Map_Cell ( 8, 6 );
			Commands->Clear_Map_Cell ( 8, 7 );
			Commands->Clear_Map_Cell ( 8, 8 );
			Commands->Clear_Map_Cell ( 8, 9 );

			Commands->Clear_Map_Cell ( 9, 5 );
			Commands->Clear_Map_Cell ( 9, 6 );
			Commands->Clear_Map_Cell ( 9, 7 );
			Commands->Clear_Map_Cell ( 9, 8 );
			Commands->Clear_Map_Cell ( 9, 9 );

			Commands->Clear_Map_Cell ( 10, 5 );
			Commands->Clear_Map_Cell ( 10, 6 );
			Commands->Clear_Map_Cell ( 10, 7 );
			Commands->Clear_Map_Cell ( 10, 8 );
			Commands->Clear_Map_Cell ( 10, 9 );

			Commands->Clear_Map_Cell ( 11, 6 );
			Commands->Clear_Map_Cell ( 11, 7 );
			Commands->Clear_Map_Cell ( 11, 8 );
			Commands->Clear_Map_Cell ( 11, 9 );
		}
	}
};

DECLARE_SCRIPT(M01_Priest_Datadisc_JDG, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) //shows the hand of nod
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}

			Commands->Clear_Map_Cell ( 5, 1 );
			Commands->Clear_Map_Cell ( 5, 2 );
			Commands->Clear_Map_Cell ( 5, 3 );
			Commands->Clear_Map_Cell ( 5, 4 );
			Commands->Clear_Map_Cell ( 5, 5 );

			Commands->Clear_Map_Cell ( 6, 1 );
			Commands->Clear_Map_Cell ( 6, 2 );
			Commands->Clear_Map_Cell ( 6, 3 );
			Commands->Clear_Map_Cell ( 6, 4 );
			Commands->Clear_Map_Cell ( 6, 5 );

			Commands->Clear_Map_Cell ( 7, 1 );
			Commands->Clear_Map_Cell ( 7, 2 );
			Commands->Clear_Map_Cell ( 7, 3 );
			Commands->Clear_Map_Cell ( 7, 4 );
			Commands->Clear_Map_Cell ( 7, 5 );

			Commands->Clear_Map_Cell ( 8, 1 );
			Commands->Clear_Map_Cell ( 8, 2 );
			Commands->Clear_Map_Cell ( 8, 3 );
			Commands->Clear_Map_Cell ( 8, 4 );
			Commands->Clear_Map_Cell ( 8, 5 );

			Commands->Clear_Map_Cell ( 9, 1 );
			Commands->Clear_Map_Cell ( 9, 2 );
			Commands->Clear_Map_Cell ( 9, 3 );
			Commands->Clear_Map_Cell ( 9, 4 );
			Commands->Clear_Map_Cell ( 9, 5 );
		}
	}
};

DECLARE_SCRIPT(M01_Medlab_Datadisc_JDG, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
			Commands->Reveal_Map ( );

			GameObject * dataDiskController = Commands->Find_Object ( 117188 );
			if (dataDiskController != NULL)
			{
				Commands->Send_Custom_Event( obj, dataDiskController, 0, 10, 2 );
			}
		}
	}
};

DECLARE_SCRIPT(M01_DataDisc_TextController_JDG, "")//117188
{
	bool playingText;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(playingText, 1);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		playingText = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == 0 ) 
		{
			if (param == 10)//now show the text message for ESC key
			{
				playingText = true;
				int pressEnterConv = Commands->Create_Conversation( "M01_Press_ESC_for_Map_Conversation", 100, 1000, true);
				Commands->Join_Conversation( NULL, pressEnterConv, false, false, true );
				Commands->Start_Conversation( pressEnterConv,  pressEnterConv );

				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0517I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

				Commands->Send_Custom_Event( obj, obj, 0, 11, 2 );
			}

			else if (param == 11)
			{
				playingText = false;
			}
		}

		else if (type == 1)
		{
			if (param == 1 && playingText == false)//player has picked up a C4 -- start text routine
			{
				int pressF1Conv = Commands->Create_Conversation( "M01_C4_Tutorial_Conversation", 100, 1000, true);
				Commands->Join_Conversation( NULL, pressF1Conv, false, false, true );
				Commands->Start_Conversation( pressF1Conv,  pressF1Conv );
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0519I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Press '9' to select Remote C4.\n
				Commands->Send_Custom_Event( obj, obj, 1, 2, 2 );
				playingText = true;
			}

			else if (param == 2)
			{
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0523I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Left-click your mouse to place C4.\n
				Commands->Send_Custom_Event( obj, obj, 1, 3, 2 );
			}

			else if (param == 3)
			{
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0524I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Right-click your mouse to detonate.\n
				Commands->Send_Custom_Event( obj, obj, 1, 4, 2 );
			}

			else if (param == 4)
			{
				playingText = false;
			}
		}

		else if (type == 2)//player has picked up sniper rifle
		{
			if (param == 1 && playingText == false)//player has picked up a C4 -- start text routine
			{
				int pressF1Conv = Commands->Create_Conversation( "M01_Sniper_Tutorial_Conversation", 100, 1000, true);
				Commands->Join_Conversation( NULL, pressF1Conv, false, false, true );
				Commands->Start_Conversation( pressF1Conv,  pressF1Conv );
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0518I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Press '3' to select your Sniper Rifle.\n
				Commands->Send_Custom_Event( obj, obj, 2, 2, 2 );
				playingText = true;
			}

			else if (param == 2)
			{
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0521I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Right-click your mouse for sniper mode.\n
				Commands->Send_Custom_Event( obj, obj, 2, 3, 2 );
			}

			else if (param == 3)
			{
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0522I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );//Use the mouse wheel or 'G' and 'T' to zoom.\n
				Commands->Send_Custom_Event( obj, obj, 2, 4, 2 );
			}

			else if (param == 4)
			{
				playingText = false;
			}
		}
	}
};

DECLARE_SCRIPT(M01_Duncan_Assailer_JDG, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		GameObject * convController = Commands->Find_Object ( 125857 );
		if (convController != NULL)
		{
			Commands->Send_Custom_Event( obj, convController, 0, M01_IVE_BEEN_KILLED_JDG, 0 );
		}
	}
};



DECLARE_SCRIPT(M01_Duncan_InHere_ConvController_JDG, "")
{
	int deadNodGuys;
	bool duncanDiscovered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(deadNodGuys, 1);
		SAVE_VARIABLE(duncanDiscovered, 2);
	}

	void Created( GameObject * /* obj */ ) override 
	{
		deadNodGuys = 0;
		duncanDiscovered = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				deadNodGuys++;

				if (deadNodGuys >= 4)
				{
					Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 2 );
				}
			}

			else if (param == M01_START_ACTING_JDG && duncanDiscovered == false)//start playing "in here" conversation
			{
				GameObject * captDuncan = Commands->Find_Object ( 106050 );
				if (captDuncan)
				{
					int inhere_conv = Commands->Create_Conversation( "M01_DuncanInHere_Conversation", 100, 20, false);
					Commands->Join_Conversation( captDuncan, inhere_conv, false, true, true );
					Commands->Join_Conversation( STAR, inhere_conv, false, false, false );
					Commands->Start_Conversation( inhere_conv,  inhere_conv );

					Commands->Send_Custom_Event( obj, obj, 0, M01_START_ACTING_JDG, 10 );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				duncanDiscovered = true;
			}
		}
	}
};

DECLARE_SCRIPT(M01_BackPath_EntranceZone_JDG, "")//121433
{

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR)
		{
			
			Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG)
			{
				GameObject * gdiMinigunner = Commands->Find_Object ( 116382 );
				if (gdiMinigunner != NULL)
				{
					Commands->Send_Custom_Event( obj, gdiMinigunner, 0, M01_MODIFY_YOUR_ACTION_06_JDG, 0 );
					//this tells him to start the "this is the path" routine
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 2 );
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)
			{
				GameObject * grenadier = Commands->Find_Object ( 116383 );
				if (grenadier != NULL)
				{
					Commands->Send_Custom_Event( obj, grenadier, 0, M01_MODIFY_YOUR_ACTION_06_JDG, 0 );//tell grenadier to go down back path too
				}

				Commands->Destroy_Object ( obj );
			}
		}
	}
};

DECLARE_SCRIPT(M01_Base_GDI_Minigunner_JDG, "")//116382
{
	int gdi01conv01;
	int gdi01conv02;
	int gdi01conv03;
	float currentHealth;
	bool invincible;
	bool ducanRescued;
	bool headingDownBackPath;
	bool still_in_GDI_Base;
	bool damagedByNod;

	bool havocDownPath;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(gdi01conv01, 1);
		SAVE_VARIABLE(gdi01conv02, 2);
		SAVE_VARIABLE(currentHealth, 3);
		SAVE_VARIABLE(invincible, 4);
		SAVE_VARIABLE(gdi01conv03, 5);
		SAVE_VARIABLE(ducanRescued, 6);
		SAVE_VARIABLE(headingDownBackPath, 7);
		SAVE_VARIABLE(still_in_GDI_Base, 8);
		SAVE_VARIABLE(damagedByNod, 9);
		SAVE_VARIABLE(havocDownPath, 10);
	}

	void Created( GameObject * obj ) override //this guy starts in the guard tower firing over at the conyard
	{
		havocDownPath = false;
		damagedByNod = false;
		invincible = true;
		ducanRescued = false;
		still_in_GDI_Base = true;
		headingDownBackPath = false;
		currentHealth = Commands->Get_Health ( obj );
		GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller1, "Test_Cinematic", "X1C_Intro.txt");

		Vector3 myTarget (72.223f, -24.751f, -0.296f);
		ActionParamsStruct params;
		Commands->Set_Innate_Is_Stationary ( obj, true );
		params.Set_Basic( this, 100, M01_START_ACTING_JDG );
		params.Set_Attack( myTarget, 1000, 0, true );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack (obj, params);

		Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_JDG, 2 );//in 2 seconds run to a new position
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.XG_EV5_troop") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		GameObject * grenadier = Commands->Find_Object ( 116383 );
		GameObject * nodGuy01 = Commands->Find_Object ( 116388 );
		GameObject * nodGuy02 = Commands->Find_Object ( 116387 );

		if (damager == nodGuy01 || damager == nodGuy02) 
		{
			if (damagedByNod == false)
			{
				damagedByNod = true;
				Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_04_JDG, 0 );
			}
		}

		if (invincible == true)
		{
			float myCurrentHealth = Commands->Get_Health ( obj );
			if (myCurrentHealth <= 10)
			{
				Commands->Set_Health ( obj, 10 );
			}
		}

		else if (obj && damager == grenadier)
		{
			Commands->Set_Health ( obj, currentHealth );
		}

		else if (obj)
		{
			currentHealth = Commands->Get_Health ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == 0)
		{
			if (param == M01_MODIFY_YOUR_ACTION_JDG)//leave the guard tower and take up second position outside guard tower
			{
				Vector3 newPosition (66.771f, -38.435f, -0.058f);
				Vector3 myTarget (72.223f, -24.751f, -0.296f);

				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_01_JDG );
				params.Set_Attack( myTarget, 1000, 0, true );
				params.Set_Movement(newPosition, RUN, 1);
				Commands->Action_Goto ( obj, params );

				GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-74.184f, -28.372f, 3.157f));
				Commands->Set_Facing ( controller1, 90 );
				Commands->Attach_Script(controller1, "Test_Cinematic", "X01D_C130Troopdrop.txt");
			}

			else if (param == M01_MODIFY_YOUR_ACTION_02_JDG)//leave outside guard tower and take up third position beside the ladder
			{
				Vector3 newPosition (40.893f, -39.773f, -0.179f);
				Vector3 myTarget (28.449f, -23.325f, 3.032f);

				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_02_JDG );
				params.Set_Attack( myTarget, 1000, 0, true );
				params.Set_Movement(newPosition, RUN + 0.25f, 1);
				Commands->Action_Goto ( obj, params );
			}

			else if (param == M01_START_ATTACKING_01_JDG)//player is at top of ladders...check if nod minigunner is still alive
			{
				GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(34.165f,41.909f,-10));
				Commands->Attach_Script(controller1, "Test_Cinematic", "XG_Ap_vs_Trn.txt");

				GameObject * nodMinigunner = Commands->Find_Object ( 116384 );
				if (nodMinigunner != NULL)
				{
					Vector3 newPosition (40.893f, -39.773f, -0.179f);
					params.Set_Basic( this, 100, M01_START_ATTACKING_01_JDG );
					params.Set_Movement(newPosition, RUN + 0.25f, 1);
					params.Set_Attack( nodMinigunner, 1000, 0, true );
					params.AttackCrouched = true;
					params.AttackCheckBlocked = false;

					Commands->Action_Attack (obj, params);
				}

				else
				{
					Commands->Action_Reset ( obj, 100 );
					Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );//look at havoc
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_03_JDG)//greet havoc M01_GDI01_Conversation_01
			{
				Vector3 playersPosition = Commands->Get_Position ( STAR );
				ActionParamsStruct params;
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Face_Location( playersPosition, 2 );
				Commands->Action_Face_Location ( obj, params );

				gdi01conv01 = Commands->Create_Conversation( "M01_GDI01_Conversation_01", 100, 1000, false);
				Commands->Join_Conversation( obj, gdi01conv01, true, true, true );
				Commands->Join_Conversation( STAR, gdi01conv01, false, false, false );
				Commands->Start_Conversation( gdi01conv01,  gdi01conv01 );

				Commands->Monitor_Conversation(  obj, gdi01conv01 );

				GameObject * grenadier = Commands->Find_Object ( 116383 );
				if (grenadier != NULL)
				{
					Commands->Send_Custom_Event( obj, grenadier, 0, M01_START_ACTING_JDG, 0 );//tell grenadier to greet havoc
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_04_JDG)//now add the pow objective
			{
				GameObject * pogController = Commands->Find_Object ( M01_MISSION_POG_CONTROLLER_JDG );
				if (pogController != NULL)
				{
					Commands->Send_Custom_Event ( obj, pogController, M01_ADD_OBJECTIVE_POG_JDG, M01_GDI_BASE_POW_OBJECTIVE_JDG, 0 );
				}
			}

			else if (param == M01_GOTO_YOUR_EVAC_SPOT_JDG)
			{
				Commands->Enable_Hibernation( obj, false );
				ducanRescued = true;
				headingDownBackPath = false;
				Commands->Action_Reset ( obj, 100 );
				Vector3 myHomeSpot (56.0f, 28.2f, -0.2f);
				Commands->Set_Innate_Soldier_Home_Location ( obj, myHomeSpot, 10 );
				params.Set_Basic( this, 100, M01_GOTO_YOUR_EVAC_SPOT_JDG );
				params.Set_Movement(myHomeSpot, RUN, 1);

				Commands->Action_Goto(obj, params);
			}

			else if (param == M01_GOING_TO_EVAC_SPOT_JDG)//your ride is here--go get in
			{
				Commands->Action_Reset ( obj, 100 );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)
				{
					Commands->Set_Innate_Is_Stationary ( obj, false );
					Vector3 myHomeSpot = Commands->Get_Position ( myEvacController );
					params.Set_Basic( this, 100, M01_GOING_TO_EVAC_SPOT_JDG );
					params.Set_Movement(myHomeSpot, RUN, 1);

					Commands->Action_Goto(obj, params);
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_05_JDG && headingDownBackPath == false)//POW conversation is over---wave at havoc
			{
				if (havocDownPath == false)
				{
					Vector3 playersPosition = Commands->Get_Position ( STAR );
					ActionParamsStruct params;
					params.Set_Basic( this, 100, M01_START_ACTING_JDG );
					params.Set_Face_Location( playersPosition, 2 );
					Commands->Action_Face_Location ( obj, params );

					gdi01conv03 = Commands->Create_Conversation( "M01_GDI01_Conversation_05", 100, 1000, false);
					Commands->Join_Conversation( obj, gdi01conv03, true, true, true );
					Commands->Join_Conversation( STAR, gdi01conv03, false, false, false );
					Commands->Start_Conversation( gdi01conv03,  gdi01conv03 );

					Commands->Monitor_Conversation(  obj, gdi01conv03 );//M01_MODIFY_YOUR_ACTION_06_JDG
				}

				else
				{
					still_in_GDI_Base = false;
					Commands->Action_Reset ( obj, 100 );
					headingDownBackPath = true;
					Vector3 newGotoSpot (95.229f, 101.136f, 8.934f);
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
					params.Set_Movement(newGotoSpot, RUN, 1);
					params.AttackActive = false;

					Commands->Action_Attack (obj, params);
				}
			}

			else if (param == M01_MODIFY_YOUR_ACTION_06_JDG && ducanRescued == false)//Head down the path
			{
				havocDownPath = true;

				

				//Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_06_JDG, 0.25f );
			}

			else if (param == M01_IVE_BEEN_KILLED_JDG)
			{
				if (still_in_GDI_Base == true)
				{
					invincible = false;
					Commands->Apply_Damage( obj, 100000, "Steel", NULL );
				}
			}

			else if (param == M01_WALKING_WAYPATH_04_JDG)
			{
				GameObject * nodguy01 = Commands->Find_Object ( 116388 );
				if (nodguy01 != NULL)
				{
					Vector3 myNewSpot (58.784f, 55.223f, -0.551f);
					Commands->Set_Innate_Is_Stationary ( obj, false );
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
					params.Set_Movement(myNewSpot, RUN, 1);
					params.Set_Attack( nodguy01, 50, 2, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack (obj, params);
				}

				else
				{
					Vector3 myNewSpot (58.784f, 55.223f, -0.551f);
					Commands->Set_Innate_Is_Stationary ( obj, false );
					params.Set_Basic( this, 100, M01_WALKING_WAYPATH_04_JDG );
					params.Set_Movement(myNewSpot, RUN, 1);
					//params.Set_Attack( nodguy01, 50, 2, true );
					params.AttackCheckBlocked = false;

					Commands->Action_Attack (obj, params);
				}
			}
		}
	}

	void Enemy_Seen( GameObject * obj, GameObject * enemy ) override
	{
		if (headingDownBackPath == true)
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
			params.Set_Movement(enemy, WALK, 5);
			params.Set_Attack( enemy, 1000, 0, true );
			params.AttackActive = true;

			Commands->Modify_Action ( obj, M01_WALKING_WAYPATH_06_JDG, params, true, true );
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		ActionParamsStruct params;

		if (complete_reason == ACTION_COMPLETE_NORMAL)
		{
			if (action_id == M01_WALKING_WAYPATH_01_JDG)
			{
				Vector3 myTarget (72.223f, -24.751f, -0.296f);
				
				Commands->Set_Innate_Is_Stationary ( obj, true );
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( myTarget, 1000, 0, true );
				params.AttackCrouched = true;
				params.AttackCheckBlocked = false;

				Commands->Action_Attack (obj, params);

				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_02_JDG, 2 );	
			}

			else if (action_id == M01_WALKING_WAYPATH_02_JDG)
			{
				Vector3 myTarget (28.449f, -23.325f, 3.032f);
				
				Commands->Set_Innate_Is_Stationary ( obj, true );
				params.Set_Basic( this, 100, M01_START_ACTING_JDG );
				params.Set_Attack( myTarget, 1000, 0, true );
				params.AttackCrouched = true;
				params.AttackCheckBlocked = false;

				Commands->Action_Attack (obj, params);
			}

			else if (action_id == M01_START_ATTACKING_01_JDG)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );
			}

			else if (action_id == M01_GOTO_YOUR_EVAC_SPOT_JDG)
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to call in a chopper
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_MODIFY_YOUR_ACTION_JDG, 0 );
				}
			}

			else if (action_id == M01_GOING_TO_EVAC_SPOT_JDG)//start load animation here
			{
				Vector3 powerupSpawnLocation = Commands->Get_Position (obj );
				powerupSpawnLocation.Z += 0.75f;

				const auto& powerups = M01Vars::Powerups;
				int random = Commands->Get_Random_Int(0, 2);

				Commands->Create_Object ( powerups[random], powerupSpawnLocation );

				GameObject * myEvacController = Commands->Find_Object ( 106694 );
				if (myEvacController != NULL)//tell controller to spawn a rope
				{
					Commands->Send_Custom_Event( obj, myEvacController, 0, M01_GIVE_ME_A_ROPE_JDG, 0 );
				}
			}

			else if (action_id == M01_WALKING_WAYPATH_03_JDG)//you are by pows now...run to path entrance and attack nod guy 01 if still alive
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_WALKING_WAYPATH_04_JDG, 0 );
			}

			else if (action_id == M01_WALKING_WAYPATH_04_JDG)//Youre target is dead....make sure you goto path entrance.
			{
				Vector3 myNewSpot (58.784f, 55.223f, -0.551f);

				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_05_JDG );
				params.Set_Movement(myNewSpot, RUN, 1);
				Commands->Action_Goto (obj, params);
			}

			else if (action_id == M01_WALKING_WAYPATH_05_JDG)//Youre at the back path entrance now....goto stationary
			{
				Commands->Set_Innate_Is_Stationary ( obj, true );
			}

			else if (action_id == M01_WALKING_WAYPATH_06_JDG)//you just killed a back path guy...continue your journey
			{
				Vector3 newGotoSpot (95.229f, 101.136f, 8.934f);
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_06_JDG );
				params.Set_Movement(newGotoSpot, RUN, 1);
				params.AttackActive = false;

				Commands->Action_Attack (obj, params);
			}
		}

		else if (complete_reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == gdi01conv01)
			{
				gdi01conv02 = Commands->Create_Conversation( "M01_GDI01_Conversation_02", 100, 1000, false);
				Commands->Join_Conversation( obj, gdi01conv02, true, true, true );
				Commands->Join_Conversation( STAR, gdi01conv02, false, false, false );
				Commands->Start_Conversation( gdi01conv02,  gdi01conv02 );

				Commands->Monitor_Conversation(  obj, gdi01conv02 );

				
			}

			else if (action_id == gdi01conv02)
			{
				//invincible = false;
				Commands->Set_HUD_Help_Text ( IDS_M01DSGN_DSGN0506I1GCP1_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_04_JDG, 2 );

				Vector3 newPosition (31.697f, 16.155f, 2.200f);

				Commands->Set_Innate_Is_Stationary ( obj, false );
				params.Set_Basic( this, 100, M01_WALKING_WAYPATH_03_JDG );
				params.Set_Movement(newPosition, RUN, 1);
				Commands->Action_Goto ( obj, params );

				GameObject * grenadier = Commands->Find_Object ( 116383 );
				if (grenadier != NULL)
				{
					Commands->Send_Custom_Event( obj, grenadier, 0, M01_MODIFY_YOUR_ACTION_03_JDG, 0 );//tell grenadier to greet havoc	
				}
			}

			else if (action_id == gdi01conv03)
			{
				Commands->Send_Custom_Event( obj, obj, 0, M01_MODIFY_YOUR_ACTION_05_JDG, 5 );
			}
		}
	}
};


