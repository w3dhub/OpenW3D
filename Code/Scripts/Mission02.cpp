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
*     Mission02.cpp
*
* DESCRIPTION
*     Mission 02 Specific Scripts
*
* PROGRAMMER
*     Ryan Vervack, Rich Donnelly
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 332 $
*     $Modtime: 1/13/02 5:40p $
*     $Archive: /Commando/Code/Scripts/Mission02.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include "Mission2.h"


// Mission Objective Controller and Initializer

DECLARE_SCRIPT(M02_Objective_Controller, "")
{
	int convoy_trucks;
	int count_dead;
	int count_dead_09;
	bool rocket_soldier_speech;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (convoy_trucks, 1);
		SAVE_VARIABLE (count_dead, 2);
		SAVE_VARIABLE (count_dead_09, 3);
	}

	void Created(GameObject * obj) override
	{
		Commands->Reveal_Encyclopedia_Character (37);
		Commands->Reveal_Encyclopedia_Character (4);
		Commands->Reveal_Encyclopedia_Character (10);
		Commands->Reveal_Encyclopedia_Character (8);
		Commands->Set_Background_Music ("02-Packing Iron.mp3");

		// Set the Dam MCT to animation frame 0

		GameObject * dam = Commands->Find_Object (1111116);
		if (dam)
		{
			Commands->Set_Animation_Frame (dam, "MCT_NOD.MCT_NOD", 0);
		}
		count_dead = 0;
		count_dead_09 = 0;
		rocket_soldier_speech = false;

		// Set up the expected number of convoy trucks for the objective.

		convoy_trucks = 3;

		// Prevent the Objective Controller from hibernating

		Commands->Enable_Hibernation(obj, false);

		// Turn on the first primary objective (Locate the Missing Scientists)
		// Turn on the tertiary objectives (if any)

		Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
		Commands->Add_Objective (201, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M02_01, NULL, IDS_Enc_Obj_Primary_M02_01);
		Commands->Set_Objective_HUD_Info_Position (201, 50, "POG_M08_1_04.tga", IDS_POG_LOCATE, Vector3 (1208.31f,562.49f,55.02f));
		Commands->Add_Objective (223, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M02_01, NULL, IDS_Enc_Obj_Hidden_M02_01);
		Commands->Add_Objective (222, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M02_02, NULL, IDS_Enc_Obj_Hidden_M02_02);
		Commands->Start_Timer (obj, this, 2.0f, 201);
	}

	// This custom receives requests to activate and deactivate objectives
	// Type = Objective ID
	// Param 0 = Activate this objective (Pending)
	// Param 1 = Complete this objective (Accomplished)
	// Param 2 = Fail this objective (Failed)
	// Param 3 = Convoy truck special - counting

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type > 199)
		{
			switch (param)
			{
			case (0): // Activate and set radar marker
				{
					Add_An_Objective (obj, type);
					Commands->Start_Timer (obj, this, 2.0f, type);
					break;
				}
			case (1): // Success
				{
					Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					Commands->Set_Objective_Status (type, OBJECTIVE_STATUS_ACCOMPLISHED);
					Commands->Start_Timer (obj, this, 2.0f, (type + 200));
					break;
				}
			case (2): // Failure
				{
					Commands->Set_Objective_Status (type, OBJECTIVE_STATUS_FAILED);
					break;
				}
			case (3): // Counting Convoy Trucks
				{
					convoy_trucks--;
					if (!convoy_trucks)
					{
						Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1006I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
						Commands->Set_Objective_Status (213, OBJECTIVE_STATUS_ACCOMPLISHED);
						Commands->Stop_All_Conversations ();
						int id = Commands->Create_Conversation ("M02_SECONDARY_09_FINISH", 100, 300, true);
						Commands->Join_Conversation (NULL, id, true, true, true);
						Commands->Join_Conversation (STAR, id, true, false, false);
						Commands->Start_Conversation (id, 0);
					}
					break;
				}
			default:
				{
					if (type == 1000)
					{
						if (param == 1000)
						{
							count_dead++;
							if (count_dead > 1)
							{
								count_dead = 0;
								Vector3 drop_loc = Vector3(-10.6f,-25.1f,-38.2f);
								float facing = 70.5f;
								const char* textfile = "X2I_GDI_Drop02_Rocket.txt";
								GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

								if (chinook_obj1)
								{
									Commands->Set_Facing(chinook_obj1, facing);
									Commands->Attach_Script(chinook_obj1, "Test_Cinematic", textfile);
								}

								if (!rocket_soldier_speech)
								{
									rocket_soldier_speech = true;
									Commands->Stop_All_Conversations ();
									int id = Commands->Create_Conversation("M02_MORE_ROCKET_SOLDIERS", 100, 300, true);
									Commands->Join_Conversation(NULL, id, true, true, true);
									Commands->Join_Conversation (STAR, id, true, false, false);
									Commands->Start_Conversation(id, 0);
								}
							}
						}
						else if (param == 1001)
						{
							count_dead_09++;
							if (count_dead_09 > 2)
							{
								count_dead_09 = 0;
								Vector3 drop_loc = Vector3(819.38f,884.05f,24.50f);
								float facing = 110.0f;
								const char* textfile = "X2I_GDI_Drop03_Minigunner.txt";
								GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

								if (chinook_obj1)
								{
									Commands->Set_Facing(chinook_obj1, facing);
									Commands->Attach_Script(chinook_obj1, "Test_Cinematic", textfile);
								}
							}
						}
						else if (param == 1002)
						{
							Commands->Give_PowerUp (STAR, "Level_01_Keycard", true);
						}
					}
					break;
				}
			}
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		int id = 0;
		Commands->Stop_All_Conversations ();

		switch (timer_id)
		{
		case (10): // Reminder for final objective.
			{
				id = Commands->Create_Conversation("M02_PRIMARY_04_REMIND", 100, 300, true);
				break;
			}

		// Initiation Conversations

		case (201):
			{
				id = Commands->Create_Conversation("M02_PRIMARY_01_START", 100, 300, true);
				break;
			}
		case (204):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_01_START", 100, 300, true);
				break;
			}
		case (205):
			{
				id = Commands->Create_Conversation("M02_PRIMARY_04_START", 100, 300, true);
				Commands->Start_Timer (obj, this, 120.0f, 10);
				break;
			}
		case (206):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_02_START", 100, 300, true);
				break;
			}
		case (208):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_04_START", 100, 300, true);
				break;
			}
		case (210):
			{
				id = Commands->Create_Conversation("MX2DSGN_DSGN0004", 100, 300, true);
				break;
			}
		case (211):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_07_START", 100, 300, true);
				break;
			}
		case (212):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_08_START", 100, 300, true);
				break;
			}
		case (214):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_10_START", 100, 300, true);
				break;
			}
		case (218):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_14_START", 100, 300, true);
				break;
			}

		// Success Conversations

		case (403):
			{
				id = Commands->Create_Conversation("M02_PRIMARY_03_FINISH", 100, 300, true);
				break;
			}
		case (404):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_01_FINISH", 100, 300, true);
				break;
			}
		case (406):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_02_FINISH", 100, 300, true);
				break;
			}
		case (407):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_03_FINISH", 100, 300, true);
				break;
			}
		case (408):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_04_FINISH", 100, 300, true);
				break;
			}
		case (409):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_05_FINISH", 100, 300, true);
				break;
			}
		case (410):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_06_FINISH", 100, 300, true);
				break;
			}
		case (411):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_07_FINISH", 100, 300, true);
				Commands->Start_Timer (obj, this, 5.0f, 11);
				break;
			}
		case (412):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_08_FINISH", 100, 300, true);
				break;
			}
		case (414):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_10_FINISH", 100, 300, true);
				break;
			}
		case (416):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_12_FINISH", 100, 300, true);
				break;
			}
		case (417):
			{
				id = Commands->Create_Conversation("M02_SECONDARY_13_FINISH", 100, 300, true);
				break;
			}
		}
		if (id)
		{
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation (STAR, id, true, false, false);
			Commands->Start_Conversation(id, 0);
		}
	}

	void Add_An_Objective(GameObject * /*obj*/, int id)
	{
		Vector3 objective_pos;
		objective_pos = Objective_Radar_Locations [(id - 202)];

		switch (id)
		{
		case 202:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M02_02, NULL, IDS_Enc_Obj_Primary_M02_02);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 66, "POG_M07_2_05.tga", IDS_POG_DISABLE, objective_pos);
				break;
			}
		case 203:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M02_03, NULL, IDS_Enc_Obj_Primary_M02_03);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 67, "POG_M02_1_03.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 204:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_01, NULL, IDS_Enc_Obj_Secondary_M02_01);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 80, "POG_M02_2_01.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 205:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1004I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M02_04, NULL, IDS_Enc_Obj_Primary_M02_04);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 80, "POG_M02_1_04.tga", IDS_POG_CAPTURE, objective_pos);
				break;
			}
		case 206:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_02, NULL, IDS_Enc_Obj_Secondary_M02_02);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 53, "POG_M02_2_02.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 207:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_03, NULL, IDS_Enc_Obj_Secondary_M02_03);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 54, "POG_M02_2_03.tga", IDS_POG_ELIMINATE, objective_pos);
				break;
			}
		case 208:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_05, NULL, IDS_Enc_Obj_Secondary_M02_05);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 56, "POG_M02_2_06.tga", IDS_POG_ELIMINATE, objective_pos);
				break;
			}
		case 209:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_04, NULL, IDS_Enc_Obj_Secondary_M02_04);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 55, "POG_M02_2_05.tga", IDS_POG_ELIMINATE, objective_pos);
				break;
			}
		case 210:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_06, NULL, IDS_Enc_Obj_Secondary_M02_06);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 57, "POG_M02_2_04.tga", IDS_POG_ELIMINATE, objective_pos);
				break;
			}
		case 211:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_07, NULL, IDS_Enc_Obj_Secondary_M02_07);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 61, "POG_M02_2_07.tga", IDS_POG_ELIMINATE, objective_pos);
				break;
			}
		case 212:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_08, NULL, IDS_Enc_Obj_Secondary_M02_08);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 59, "POG_M02_2_08.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 213:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_09, NULL, IDS_Enc_Obj_Secondary_M02_09);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 60, "POG_M02_2_09.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 214:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_10, NULL, IDS_Enc_Obj_Secondary_M02_10);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 62, "POG_M02_2_10.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 215:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_11, NULL, IDS_Enc_Obj_Secondary_M02_11);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 63, "POG_M02_2_11.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 216:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_12, NULL, IDS_Enc_Obj_Secondary_M02_12);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 64, "POG_M02_2_12.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 217:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_13, NULL, IDS_Enc_Obj_Secondary_M02_13);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 65, "POG_M02_2_13.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 218:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_14, NULL, IDS_Enc_Obj_Secondary_M02_14);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 70, "POG_M02_2_14.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 219:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_15, NULL, IDS_Enc_Obj_Secondary_M02_15);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 71, "POG_M02_2_14.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 220:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_16, NULL, IDS_Enc_Obj_Secondary_M02_16);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 72, "POG_M02_2_14.tga", IDS_POG_DESTROY, objective_pos);
				break;
			}
		case 221:
			{
				Commands->Set_HUD_Help_Text(IDS_M00DSGN_DSGN1005I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M02_17, NULL, IDS_Enc_Obj_Secondary_M02_17);
				Commands->Set_Objective_Radar_Blip(id, objective_pos);
				Commands->Set_Objective_HUD_Info_Position (id, 58, "POG_M02_2_15.tga", IDS_POG_ACQUIRE, objective_pos);
				break;
			}
		}
	}
};


DECLARE_SCRIPT(M02_Objective_Zone, "")
{
	bool enemies_defeated;
	bool was_entered;
	bool gave_secure_warning;
	int mendoza_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (enemies_defeated, 1);
		SAVE_VARIABLE (mendoza_id, 2);
		SAVE_VARIABLE (was_entered, 3);
		SAVE_VARIABLE (gave_secure_warning, 4);
	}

	void Created(GameObject * /*obj*/) override
	{
		gave_secure_warning = false;
		mendoza_id = 0;
		was_entered = false;
		enemies_defeated = false;
	}

	void Entered(GameObject *obj, GameObject * /*enterer*/) override
	{
		// Send customs to activate the appropriate objectives

		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			bool destroy_zone = false;
			int dual_zone = 0;
			int zone_id = Commands->Get_ID(obj);

			switch (zone_id)
			{
			case (301601):
				{
					if (!Commands->Has_Key(STAR, 6))
					{
						Commands->Stop_All_Conversations ();
						int id = Commands->Create_Conversation("M02_BAY_DOOR_WARNING", 100, 300, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation (STAR, id, true, false, false);
						Commands->Start_Conversation(id, 0);
						destroy_zone = true;
					}
					break;
				}
			case (303203):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_PRIMARY_03_GUIDE", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);
					destroy_zone = true;
					break;
				}
			case (304006):
				{
					if (!was_entered)
					{
						was_entered = true;
						int id = Commands->Create_Conversation("MX2DSGN_DSGN0001", 100, 300, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation (STAR, id, true, false, false);
						Commands->Start_Conversation(id, 0);
						Commands->Set_HUD_Help_Text(IDS_M02DSGN_DSGN0186I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
					}
					break;
				}
			case (400184):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 206, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 207, 0, 0.0f);

					// Turn on A00

					Commands->Send_Custom_Event(obj, object, 104, 0, 0.0f);

					// Call GDI Rocket Soldiers

					Call_GDI_Soldiers(0);
					
					// Call the Nod Apache

					Create_Apache(0);

					// Tell A00 units to wake up

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400286), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400287), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400288), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400292), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400293), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400285), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400206), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401004), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401005), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401006), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400185):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 208, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 209, 0, 0.0f);

					// Turn on A01
					
					Commands->Send_Custom_Event(obj, object, 104, 1, 0.0f);

					// Create A01 Nod Buggy for Player

					Commands->Send_Custom_Event(obj, object, 112, 1, 0.0f);
					Commands->Send_Custom_Event(obj, object, 111, 1, 0.0f);

					// Tell the GDI Rocket Soldiers for A00 to go to the tower.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1100, obj_position, 1000.0f);	

					// Tell A23 units to go to home points.

					Commands->Create_Logical_Sound (obj, 1073, obj_position, 1000.0f);

					// Tell A01 units to wake up.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400342), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400343), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400347), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400332), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400340), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400348), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400344), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400345), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400333), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400207), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400371), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401008), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400186):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 210, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 221, 0, 0.0f);

					//Turn off A01, Turn on A02

					Commands->Send_Custom_Event(obj, object, 105, 1, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 2, 0.0f);
					
					// Tell A01 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1051, obj_position, 1000.0f);

					// Turn on A02 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400372), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400373), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400374), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401167), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400208), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401014), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401015), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401016), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401017), 0, 0, 0.0f);

					// Create the A02 buggy

					Create_Buggy (2);

					// Bring in A02 Sakura

					Create_Sakura (2);

					// Start the tank conversation

					destroy_zone = true;
					break;
				}
			case (400187):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 211, 0, 0.0f);

					//Turn off A03, Turn on A04

					Commands->Send_Custom_Event(obj, object, 105, 3, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 4, 0.0f);

					// Tell the GDI Rocket Soldiers for A02 to go to the house.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1102, obj_position, 1000.0f);

					// Tell A03 units to go to home points.

					Commands->Create_Logical_Sound (obj, 1053, obj_position, 1000.0f);

					// Wake up A04 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400210), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400451), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400452), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400453), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400499), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400500), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401025), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401026), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401027), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401028), 0, 0, 0.0f);

					// Create A04 Nod Light Tank for Player

					Commands->Send_Custom_Event(obj, object, 112, 4, 0.0f);
					Commands->Send_Custom_Event(obj, object, 111, 4, 0.0f);

					// Create the A04 Nod Buggy

					Create_Buggy (4);

					// Activate the A04 Medium Tank motion and destruction.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401018), 0, 0, 0.0f);

					// Start the tank speaking timer

					Commands->Start_Timer (obj, this, 0.1f, 10);

					destroy_zone = true;
					dual_zone = 400267;
					break;
				}
			case (400188):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 212, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 213, 0, 0.0f);

					//Turn off A02, Turn on A03

					Commands->Send_Custom_Event(obj, object, 105, 2, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 3, 0.0f);

					// Tell A02 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1052, obj_position, 1000.0f);

					// Create the Light Tank for A03

					Create_Light_Tank (3);

					// Wake up A03 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400410), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400209), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400411), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400412), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400413), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400414), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400415), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400189):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 215, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 216, 0, 0.0f);

					//Turn on A06

					Commands->Send_Custom_Event(obj, object, 104, 6, 0.0f);

					// Wake up A06 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400211), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400976), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400975), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400977), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400978), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400979), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400980), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401036), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401037), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401038), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401045), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400192):
				{
					// Default shut off for A11, Power Plant spawns.

					Commands->Send_Custom_Event(obj, object, 105, 11, 0.0f);
					
					// Tell A11 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1061, obj_position, 1000.0f);

					destroy_zone = true;
					break;
				}
			case (400193):
				{
					GameObject * object = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
					if (object)
					{
						Commands->Control_Enable (STAR, false);
						Commands->Start_Timer (obj, this, 1.0f, 9);
						GameObject * controller = Commands->Find_Object (M02_OBJCONTROLLER);
						if (controller)
						{
							Commands->Send_Custom_Event (obj, controller, 1000, 1002, 25.0f);
						}
						Commands->Attach_Script (object, "Test_Cinematic", "X2K_Midtro.txt");
					}
					break;
				}
			case (400194):
				{
					if (!was_entered)
					{
						was_entered = true;

						// Complete final objective

						Commands->Send_Custom_Event(obj, object, 205, 1, 0.0f);
						Commands->Mission_Complete (true);

						//Turn off A21
						
						Commands->Send_Custom_Event(obj, object, 105, 21, 0.0f);
					}
					break;
				}
			case (400195):
				{
					Check_For_Soldiers (obj, 3);
					break;
				}
			case (400196):
				{
					Check_For_Soldiers (obj, 4);
					break;
				}
			case (400197):
				{
					Check_For_Soldiers (obj, 5);
					break;
				}
			case (400198):
				{
					Check_For_Soldiers (obj, 6);
					break;
				}
			case (400201):
				{
					Check_For_Soldiers (obj, 7);
					break;
				}
			case (400267):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 211, 0, 0.0f);

					// Turn off A03, Turn on A04

					Commands->Send_Custom_Event(obj, object, 105, 3, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 4, 0.0f);

					// Tell A03 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1053, obj_position, 1000.0f);

					// Tell the GDI Rocket Soldiers for A02 to go to the house.

					Commands->Create_Logical_Sound (obj, 1102, obj_position, 1000.0f);	

					// Wake up A04 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400210), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400451), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400452), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400453), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400499), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400500), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401025), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401026), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401027), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401028), 0, 0, 0.0f);

					// Create A04 Nod Light Tank for Player

					Commands->Send_Custom_Event(obj, object, 112, 4, 0.0f);
					Commands->Send_Custom_Event(obj, object, 111, 4, 0.0f);

					// Create the A04 Nod Buggy

					Create_Buggy (4);

					// Activate the A04 Medium Tank motion and destruction.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401018), 0, 0, 0.0f);

					// Start the tank speaking timer

					Commands->Start_Timer (obj, this, 0.1f, 10);

					destroy_zone = true;
					dual_zone = 400187;
					break;
				}
			case (400268):
				{
					// Turn off A04

					Commands->Send_Custom_Event(obj, object, 105, 4, 0.0f);

					// Tell A04 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1054, obj_position, 1000.0f);

					// Turn on A05 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400552), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400553), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400560), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400555), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400558), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400559), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400557), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400554), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400556), 0, 0, 0.0f);

					//Turn off A24

					Commands->Send_Custom_Event(obj, object, 105, 24, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400269):
				{
					// Turn on objectives

					Commands->Send_Custom_Event(obj, object, 202, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 203, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 217, 0, 0.0f);

					//Turn off A06, Turn on A09 (Dam Top)

					Commands->Send_Custom_Event(obj, object, 105, 6, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 9, 0.0f);

					// Turn on A09 units.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400212), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400981), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400982), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400983), 0, 0, 0.0f);

					// Tell A06 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1056, obj_position, 1000.0f);

					// Create the GDI soldier set

					Call_GDI_Soldiers (9);

					// Primary 01 GUIDE Conversation

					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_PRIMARY_01_GUIDE", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);

					destroy_zone = true;
					break;
				}
			case (400270):
				{
					//Turn off A09, Turn on A08 (Dam Interior)

					Commands->Send_Custom_Event(obj, object, 105, 9, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 8, 0.0f);

					// Wake up A08 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400275), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401046), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401047), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401048), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401049), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401050), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401051), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401052), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401053), 0, 0, 0.0f);

					// Tell A09 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1059, obj_position, 1000.0f);

					dual_zone = 400271;
					destroy_zone = true;
					break;
				}
			case (400271):
				{
					//Turn off A09, Turn on A08 (Dam Interior)

					Commands->Send_Custom_Event(obj, object, 105, 9, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 8, 0.0f);

					// Wake up A08 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400275), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401046), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401047), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401048), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401049), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401050), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401051), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401052), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401053), 0, 0, 0.0f);

					// Tell A09 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1059, obj_position, 1000.0f);

					GameObject * deadzone = Commands->Find_Object (303203);
					if (deadzone)
					{
						Commands->Destroy_Object (deadzone);
					}

					dual_zone = 400270;
					destroy_zone = true;
					break;
				}
			case (400272):
				{
					//Turn off A08, Turn on A10

					Commands->Send_Custom_Event(obj, object, 105, 8, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 10, 0.0f);

					// Tell A08 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1058, obj_position, 1000.0f);

					// Wake up A10 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400213), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400984), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401054), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401055), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401056), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401057), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400273):
				{
					//Turn off A10, Turn on A11

					Commands->Send_Custom_Event(obj, object, 105, 10, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 11, 0.0f);

					// Force an A11 spawn

					Commands->Send_Custom_Event(obj, object, 114, 11, 0.0f);
					Commands->Send_Custom_Event(obj, object, 102, 11, 0.0f);

					// Wake up A11 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400276), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400199), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401065), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401058), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401059), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401060), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401061), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401063), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401064), 0, 0, 0.0f);

					// Tell A10 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1060, obj_position, 1000.0f);

					destroy_zone = true;
					break;
				}
			case (400274):
				{
					//Trigger A11 off only when Power Plant is destroyed.

					GameObject* power_plant_obj = Commands->Find_Object(1153846);

					if (power_plant_obj)
					{
						float health_check = Commands->Get_Health(power_plant_obj);
						if (health_check <= 0.0f)
						{
							Commands->Send_Custom_Event(obj, object, 105, 11, 0.0f);

							// Tell A11 units to go to home points.

							Vector3 obj_position = Commands->Get_Position (obj);
							Commands->Create_Logical_Sound (obj, 1061, obj_position, 1000.0f);

							// Force an A10 spawn.

							Commands->Send_Custom_Event(obj, object, 114, 10, 0.0f);
							Commands->Send_Custom_Event(obj, object, 102, 10, 0.0f);
							destroy_zone = true;
						}

						// Shut off the Power Plant announcement

						Commands->Send_Custom_Event(obj, power_plant_obj, 1, 0, 0.0f);
					}
					break;
				}
			case (400316):
				{
					// Make the A00 soldiers return to home points, turn of A00

					Commands->Send_Custom_Event(obj, object, 105, 0, 0.0f);
					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1050, obj_position, 1000.0f);

					// Turn on A23 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400317), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400318), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400319), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (400501):
				{
					//Turn on A24

					// Wake up A24 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400503), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400504), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400505), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400506), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400507), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400508), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400509), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400510), 0, 0, 0.0f);

					Commands->Send_Custom_Event(obj, object, 104, 24, 0.0f);

					// Turn on objective - SAM Site
					
					Commands->Send_Custom_Event(obj, object, 214, 0, 0.0f);

					dual_zone = 400502;
					destroy_zone = true;
					break;
				}
			case (400502):
				{
					//Turn on A24

					Commands->Send_Custom_Event(obj, object, 104, 24, 0.0f);

					// Wake up A24 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400503), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400504), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400505), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400506), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400507), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400508), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400509), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(400510), 0, 0, 0.0f);

					// Turn on objective - SAM Site
					
					Commands->Send_Custom_Event(obj, object, 214, 0, 0.0f);

					dual_zone = 400501;
					destroy_zone = true;
					break;
				}
			case (400551):
				{
					// Create the Apache for this area

					Create_Apache (24);

					destroy_zone = true;
					break;
				}
			case (400989):
				{
					destroy_zone = true;
					break;
				}
			case (400990):
				{
					// Trigger the Failed Drop Cinematic

					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));

					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2E_C130Drop.txt");
					}

					destroy_zone = true;
					break;
				}
			case (400991):
				{
					// Trigger the Orca shootdown cinematic

					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));

					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2F_OrcaSam.txt");
					}

					destroy_zone = true;
					break;
				}
			case (400992):
				{
					// Trigger the Apache Orca crash cinematic

					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));

					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2G_AirCrash.txt");
					}

					destroy_zone = true;
					break;
				}
			case (401001):
				{
					// Trigger the Apache bridge crash cinematic.
					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));

					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2H_ApacheSpray.txt");
					}

					destroy_zone = true;
					break;
				}
			case (401029):
				{
					// Trigger the crash cinematic in A01

					GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));

					if (chinook_obj1)
					{
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2D_MTank_Slip.txt");
					}

					destroy_zone = true;
					break;
				}
			case (401054):
				{
					// Force an A08 spawn

					Commands->Send_Custom_Event(obj, object, 114, 8, 0.0f);
					Commands->Send_Custom_Event(obj, object, 102, 8, 0.0f);
					destroy_zone = true;
					break;
				}
			case (401066):
				{
					// Wake up A13 Obelisk units. No spawns in here.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(400200), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401067), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401068), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401069), 0, 0, 0.0f);

					// Shut off the Obelisk warning.

					GameObject * obelisk = Commands->Find_Object(1153845);
					
					if (obelisk)
					{
						Commands->Send_Custom_Event (obj, obelisk, 1, 0, 0.0f);
					}

					destroy_zone = true;
					break;
				}
			case (401070):
				{
					// Turn on A14, Turn off A11 as default.

					Commands->Send_Custom_Event(obj, object, 105, 11, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 14, 0.0f);

					// Tell A11 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1061, obj_position, 1000.0f);

					// Wake up A14 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401071), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401072), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401073), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401074), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401075), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401076), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401077), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401078), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401192), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401193), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401194), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401195), 0, 0, 0.0f);

					dual_zone = 401196;
					destroy_zone = true;
					break;
				}
			case (401079):
				{
					// Turn off A14, Turn on A15

					Commands->Send_Custom_Event(obj, object, 105, 14, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 15, 0.0f);

					// Tell A14 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1064, obj_position, 1000.0f);

					// Turn on A15 Objectives

					Commands->Send_Custom_Event(obj, object, 218, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 219, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 220, 0, 0.0f);

					// Wake up A15 Units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401089), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401087), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401086), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401088), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401085), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401090), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401091), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401092), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401094), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401095), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401096), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401097), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401189), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401190), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401191), 0, 0, 0.0f);

					dual_zone = 401080;
					destroy_zone = true;
					break;
				}
			case (401080):
				{
					// Turn off A14, Turn on A15

					Commands->Send_Custom_Event(obj, object, 105, 14, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 15, 0.0f);

					// Tell A14 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1064, obj_position, 1000.0f);

					// Turn on A15 Objectives

					Commands->Send_Custom_Event(obj, object, 218, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 219, 0, 0.0f);
					Commands->Send_Custom_Event(obj, object, 220, 0, 0.0f);

					// Wake up A15 Units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401089), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401087), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401086), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401088), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401085), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401090), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401091), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401092), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401094), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401095), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401096), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401097), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401189), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401190), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401191), 0, 0, 0.0f);

					dual_zone = 401079;
					destroy_zone = true;
					break;
				}
			case (401101):
				{
					// Turn on Hand of Nod objective

					Commands->Send_Custom_Event(obj, object, 204, 0, 0.0f);

					// Tell A15 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1065, obj_position, 1000.0f);

					// Turn off A15, Turn on A16 units - First HoN encounter set.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401103), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401104), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401105), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401106), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401107), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401108), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401109), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401110), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401112), 0, 0, 0.0f);

					dual_zone = 401102;
					destroy_zone = true;
					break;
				}
			case (401102):
				{
					// Turn on Hand of Nod objective

					Commands->Send_Custom_Event(obj, object, 204, 0, 0.0f);

					// Tell A15 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1065, obj_position, 1000.0f);

					// Turn off A15, Turn on A16 units - First HoN encounter set.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401103), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401104), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401105), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401106), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401107), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401108), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401109), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401110), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401112), 0, 0, 0.0f);

					dual_zone = 401101;
					destroy_zone = true;
					break;
				}
			case (401113):
				{
					// Turn on A17 Units, Hand of Nod underside.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401115), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401116), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401117), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401118), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401119), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401120), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401121), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401122), 0, 0, 0.0f);

					dual_zone = 401114;
					destroy_zone = true;
					break;
				}
			case (401114):
				{
					// Turn on A17 Units, Hand of Nod underside.

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401115), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401116), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401117), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401118), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401119), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401120), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401121), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401122), 0, 0, 0.0f);

					dual_zone = 401113;
					destroy_zone = true;
					break;
				}
			case (401123):
				{
					// Turn on A18 Units, Hand of Nod Lowest Level

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401124), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401125), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401126), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401127), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401128), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401129), 0, 0, 0.0f);

					destroy_zone = true;
					break;
				}
			case (401130):
				{
					GameObject * controller = Commands->Find_Object (1157690);
					if (controller)
					{
						float health = Commands->Get_Health (controller);
						if (health)
						{
							Commands->Stop_All_Conversations ();
							int id = Commands->Create_Conversation("M02_SECONDARY_01_GUIDE", 100, 300, true);
							Commands->Join_Conversation(NULL, id, true, true, true);
							Commands->Join_Conversation (STAR, id, true, false, false);
							Commands->Start_Conversation(id, 0);
						}
					}

					// Turn on A19 Units, Hand of Nod area

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401132), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401133), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401134), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401135), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401136), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401137), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401138), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401139), 0, 0, 0.0f);

					dual_zone = 401131;
					destroy_zone = true;
					break;
				}
			case (401131):
				{
					GameObject * controller = Commands->Find_Object (1157690);
					if (controller)
					{
						float health = Commands->Get_Health (controller);
						if (health)
						{
							Commands->Stop_All_Conversations ();
							int id = Commands->Create_Conversation("M02_SECONDARY_01_GUIDE", 100, 300, true);
							Commands->Join_Conversation(NULL, id, true, true, true);
							Commands->Join_Conversation (STAR, id, true, false, false);
							Commands->Start_Conversation(id, 0);
						}
					}

					// Turn on A19 Units, Hand of Nod area

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401132), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401133), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401134), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401135), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401136), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401137), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401138), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401139), 0, 0, 0.0f);

					dual_zone = 401130;
					destroy_zone = true;
					break;
				}
			case (401186):
				{
					// Create the A15 Apache

					Create_Apache(15);

					// M02 Secondary 14 GUIDE Conversation

					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_SECONDARY_14_GUIDE", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);

					dual_zone = 401187;
					destroy_zone = true;
					break;
				}
			case (401187):
				{
					// Create the A15 Apache

					Create_Apache(15);

					// M02 Secondary 14 GUIDE Conversation

					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_SECONDARY_14_GUIDE", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);

					dual_zone = 401186;
					destroy_zone = true;
					break;
				}
			case (401196):
				{
					// Turn on A14, Turn off A11 as default.

					Commands->Send_Custom_Event(obj, object, 105, 11, 0.0f);
					Commands->Send_Custom_Event(obj, object, 104, 14, 0.0f);

					// Tell A11 units to go to home points.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1061, obj_position, 1000.0f);

					// Wake up A14 units

					Commands->Send_Custom_Event(obj, Commands->Find_Object(401071), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401072), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401073), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401074), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401075), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401076), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401077), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401078), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401192), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401193), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401194), 0, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(401195), 0, 0, 0.0f);

					dual_zone = 401070;
					destroy_zone = true;
					break;
				}
			case (401982):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_SECONDARY_13_START", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);

					// Fly the Lear Jet over

					GameObject * lear_jet = Commands->Create_Object ("Nod_Jet", Vector3(929.91f,1132.34f,70.5f));
					if (lear_jet)
					{
						Commands->Attach_Script (lear_jet, "M02_Nod_Jet", "");
					}
					destroy_zone = true;
					break;
				}
			case (405116):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_OBELISK_APPROACH", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);
					destroy_zone = true;
					break;
				}
			case (405117):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_PRIMARY_02_REMIND", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);
					destroy_zone = true;
					break;
				}
			case (405118):
				{
					destroy_zone = true;
					break;
				}
			case (405119):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_SECONDARY_03_START", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);
					destroy_zone = true;
					break;
				}
			case (405120):
				{
					// Drop the A02 Mammoth Tank

					Vector3 droploc = Vector3(570.06f,261.47f,-55.0f);
					GameObject * chinook_obj1 = Commands->Create_Object("GDI_Mammoth_Tank_Player", droploc);
					if (chinook_obj1)
					{
						Commands->Set_Facing (chinook_obj1, 45.0f);
						Commands->Attach_Script(chinook_obj1, "M02_Player_Vehicle", "2");
					}
					destroy_zone = true;
					break;
				}
			case (405122):
				{
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_SECONDARY_11_START", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);

					Commands->Start_Timer (obj, this, 6.0f, 12);
					destroy_zone = true;
					break;
				}
			}

			if (dual_zone)
			{
				// Another zone is also associated with this trigger. Destroy it as well.

				GameObject* dual_zone_obj = Commands->Find_Object(dual_zone);

				if (dual_zone_obj)
				{
					Commands->Destroy_Object(dual_zone_obj);
				}
			}

			if (destroy_zone)
			{
				Commands->Destroy_Object(obj);
			}
		}
	}

	void Check_For_Soldiers (GameObject* obj, int timer_id)
	{
		enemies_defeated = true;
		Vector3 obj_position = Commands->Get_Position (obj);
		Commands->Create_Logical_Sound (obj, 1000, obj_position, 70.0f);	
		Commands->Start_Timer(obj, this, 1.0f, timer_id);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 3)
		{
			Send_and_Destroy (obj, 207);
		}
		else if (timer_id == 4)
		{
			Send_and_Destroy (obj, 208);
		}
		else if (timer_id == 5)
		{
			Send_and_Destroy (obj, 209);
		}
		else if (timer_id == 6)
		{
			Send_and_Destroy (obj, 210);
		}
		else if (timer_id == 7)
		{
			Send_and_Destroy (obj, 211);
		}
		else if (timer_id == 9)
		{
			// Teleport the star to the rooftop area.

			Commands->Set_Position (STAR, Vector3(1190.268f,558.538f,32.5f));

			// Create the Nod Jet and fly it out

			GameObject * jet = Commands->Create_Object ("Nod_Jet", Vector3(1209.879f,528.647f,21.313f));
			if (jet)
			{
				Commands->Attach_Script(jet, "M02_Nod_Jet_Waypath", "");
			}

			// Turn on objectives

			GameObject * object = Commands->Find_Object (M02_OBJCONTROLLER);
			if (object)
			{
				Commands->Control_Enable (STAR, true);
				ActionParamsStruct params;
				params.Set_Basic (this, 100, 0);
				Commands->Action_Follow_Input (STAR, params);

				Commands->Send_Custom_Event(obj, object, 201, 1, 0.0f);
				Commands->Send_Custom_Event(obj, object, 205, 0, 0.0f);

				//Turn on A21

				Commands->Send_Custom_Event(obj, object, 104, 21, 0.0f);

				// Wake up A21 units

				Commands->Send_Custom_Event(obj, Commands->Find_Object(400277), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401141), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401142), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401143), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401144), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401145), 0, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(401146), 0, 0, 0.0f);

				// Create Mendoza

				GameObject * mendoza = Commands->Create_Object("Nod_FlameThrower_3Boss", Vector3(1260.007f,535.460f,18.428f));
				if (mendoza)
				{
					Commands->Attach_Script(mendoza, "M02_Mendoza", "");
					mendoza_id = Commands->Get_ID (mendoza);
					GameObject * endzone = Commands->Find_Object (400194);
					if (endzone)
					{
						Commands->Send_Custom_Event(obj, endzone, 999, mendoza_id, 0.0f);
					}
				}
				Commands->Destroy_Object(obj);
			}
		}
		else if (timer_id == 10)
		{
			GameObject * tank = Commands->Find_Object (401028);
			if (tank)
			{
				Vector3 tank_pos = Commands->Get_Position (tank);
				Vector3 my_pos = Commands->Get_Position (STAR);
				float distance = Commands->Get_Distance (tank_pos, my_pos);
				if (distance < 20.0f)
				{
					int id = Commands->Create_Conversation ("MX2DSGN_DSGN0009", 100, 300, true);
					Commands->Join_Conversation (NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation (id, 0);
				}
				else
				{
					Commands->Start_Timer (obj, this, 2.0f, 10);
				}
			}
		}
		else if (timer_id == 11)
		{
			GameObject * tank = Commands->Find_Object (401028);
			if (tank)
			{
				int id = Commands->Create_Conversation ("MX2DSGN_DSGN0010", 100, 300, true);
				Commands->Join_Conversation (NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation (id, 0);
			}
		}
		else if (timer_id == 12)
		{
			GameObject * tank = Commands->Find_Object (401036);
			if (tank)
			{
				int id = Commands->Create_Conversation ("MX2DSGN_DSGN0011", 100, 300, true);
				Commands->Join_Conversation (NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation (id, 0);
			}
		}
	}

	void Send_and_Destroy (GameObject *obj, int type)
	{
		if (enemies_defeated)
		{
			GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

			if (object)
			{
				Commands->Send_Custom_Event(obj, object, type, 1, 0.0f);
				enemies_defeated = false;
				Commands->Destroy_Object(obj);
				
				if (type == 207)
				{
					// Check if the commando needs a new vehicle.

					Commands->Send_Custom_Event(obj, object, 111, 0, 0.0f);

					// Tell the GDI rocket solders to go to the tower.

					Vector3 obj_position = Commands->Get_Position (obj);
					Commands->Create_Logical_Sound (obj, 1100, obj_position, 1000.0f);
				}
				else if (type == 210)
				{
					// Air drop GDI soldiers into A02

					Call_GDI_Soldiers (2);
				}
			}
		}
		else if (!gave_secure_warning)
		{
			gave_secure_warning = true;
			Commands->Stop_All_Conversations ();
			int id = Commands->Create_Conversation("M02_EVAG_SECURE_WARNING", 100, 300, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation (STAR, id, true, false, false);
			Commands->Start_Conversation(id, 0);
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		// Custom received from living units in an area. 

		if ((type == 100) && (param == 1))
		{
			enemies_defeated = false;
		}
		else if (type == 999)
		{
			if (param == 999)
			{
				GameObject * mendoza = Commands->Find_Object (mendoza_id);
				if (mendoza)
				{
					mendoza_id = 0;
					Commands->Destroy_Object (mendoza);
				}
			}
			else
			{
				mendoza_id = param;
			}
		}
	}

	void Call_GDI_Soldiers (int area_id)
	{
		Vector3 drop_loc = Vector3(0.0f,0.0f,0.0f);
		float facing = 0.0f;
		const char* textfile = "X2I_GDI_Drop02_Rocket.txt";

		switch (area_id)
		{
		case (0):
			{
				drop_loc = Vector3(-10.6f,-25.1f,-38.2f);
				facing = 70.5f;
				textfile = "X2I_GDI_Drop02_Rocket.txt";
				break;
			}
		case (2):
			{
				drop_loc = Vector3(664.70f,312.57f,-59.65f);
				facing = -150.0f;
				textfile = "X2I_GDI_Drop02_Rocket_2.txt";
				break;
			}
		case (9): // Dam Top Rush
			{
				drop_loc = Vector3(869.320f,912.316f,30.150f);
				facing = 110.0f;
				textfile = "X2I_GDI_Drop03_Minigunner.txt";
				break;
			}
		}

		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

		if (chinook_obj1)
		{
			Commands->Set_Facing(chinook_obj1, facing);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", textfile);
		}
	}

	void Create_Apache (int area_id)
	{
		GameObject *helipad = Commands->Find_Object(474463);

		if (helipad)
		{
			// This will stop working when the Power Plant goes down, but it won't matter by that time.

			float currenthealth = Commands->Get_Health (helipad);

			if (((Commands->Get_Building_Power (helipad)) && (currenthealth > 0.0f)) || (area_id == 15))
			{
				Vector3 create_pos = Vector3(0.0f, 0.0f, 0.0f);
				float facing = 0.0f;
				const char* apache_params = "0";

				switch(area_id)
				{
				case (0):
					{
						create_pos = Vector3(138.36f, -5.61f, -12.20f);
						facing = -80.0f;
						apache_params = "0";
						break;
					}
				case (15):
					{
						create_pos = Vector3(1300.898f,657.717f,67.364f);
						facing = 0.0f;
						apache_params = "15";
						break;
					}
				case (24):
					{
						create_pos = Vector3(610.03f,1036.35f,-15.676f);
						facing = 0.0f;
						apache_params = "24";
						break;
					}
				}

				GameObject * apache = Commands->Create_Object("Nod_Apache", create_pos);
				if (apache)
				{
					Commands->Set_Facing(apache, facing);
					Commands->Attach_Script(apache, "M02_Nod_Apache", apache_params);
				}
			}
		}
	}

	void Create_Light_Tank (int /*area_id*/)
	{
		Vector3 create_pos = Vector3(465.645f, 715.86f, -14.935f);
		float facing = -10.0f;
		const char* tank_params = "3";
		GameObject * tank = Commands->Create_Object("Nod_Light_Tank", create_pos);

		if (tank)
		{
			Commands->Set_Facing(tank, facing);
			Commands->Attach_Script(tank, "M02_Nod_Vehicle", tank_params);
		}
	}

	void Create_Sakura (int /*area_id*/)
	{
		GameObject *helipad = Commands->Find_Object(474463);

		if (helipad)
		{
			float currenthealth = Commands->Get_Health (helipad);

			if ((Commands->Get_Building_Power (helipad)) && (currenthealth > 0.0f))
			{
				Vector3 create_pos = Vector3(509.362f,473.582f,-40.0f);
				float facing = 0.0f;
				const char* sakura_params = "2";

				GameObject * sakura = Commands->Create_Object("Nod_Apache", create_pos);
				
				if (sakura)
				{
					Commands->Set_Facing(sakura, facing);
					Commands->Attach_Script(sakura, "M02_Nod_Sakura", sakura_params);
				}
			}
		}
	}

	void Create_Buggy (int area_id)
	{
		Vector3 create_pos = Vector3(0.0f, 0.0f, 0.0f);
		float facing = 0.0f;
		const char* buggy_params = "0";

		switch (area_id)
		{
		case (2):
			{
				create_pos = Vector3(674.622f,308.296f,-61.674f);
				facing = -90.0f;
				buggy_params = "2";
				break;
			}
		case (4):
			{
				create_pos = Vector3(366.209f,861.401f,6.488f);
				facing = -110.0f;
				buggy_params = "4";
				break;
			}
		}

		GameObject * buggy = Commands->Create_Object("Nod_Buggy", create_pos);
		if (buggy)
		{
			Commands->Set_Facing(buggy, facing);
			Commands->Attach_Script(buggy, "M02_Nod_Vehicle", buggy_params);
		}
	}
};


DECLARE_SCRIPT (M02_Respawn_Controller, "")
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
			area_unit_max [counter] = 8;
			area_spawn_heli_drop_01_active [counter] = false;
			area_spawn_heli_drop_02_active [counter] = false;
			area_spawn_parachute_active [counter] = false;

			// Turning the actives to true in here means they do not occur in the area
			// provided you don't turn them off later on in the mission.

			switch (counter)
			{
			case (1):
				{
					area_unit_max [counter] = 7;
					break;
				}
			case (2):
				{
					area_unit_max [counter] = 7;
					break;
				}
			case (3):
				{
					area_unit_max [counter] = 5;
					break;
				}
			case (6):
				{
					area_unit_max [counter] = 6;
					break;
				}
			case (8):
				{
					area_spawn_heli_drop_01_active [counter] = true;
					area_spawn_heli_drop_02_active [counter] = true;
					area_spawn_parachute_active [counter] = true;
					area_unit_max [counter] = 3;
					break;
				}
			case (9):
				{
					area_unit_max [counter] = 7;
					break;
				}
			case (10):
				{
					area_spawn_parachute_active [counter] = true;
					break;
				}
			case (11):
				{
					area_spawn_heli_drop_01_active [counter] = true;
					area_spawn_heli_drop_02_active [counter] = true;
					area_spawn_parachute_active [counter] = true;
					area_unit_max [counter] = 3;
					break;
				}
			case (14):
				{
					area_spawn_heli_drop_01_active [counter] = true;
					area_spawn_heli_drop_02_active [counter] = true;
					area_spawn_parachute_active [counter] = true;
					area_unit_max [counter] = 3;
					break;
				}
			case (15):
				{
					area_spawn_heli_drop_01_active [counter] = true;
					area_spawn_heli_drop_02_active [counter] = true;
					area_spawn_parachute_active [counter] = true;
					area_unit_max [counter] = 3;
					break;
				}
			case (21):
				{
					area_unit_max [counter] = 5;
					break;
				}
			case (24):
				{
					area_spawn_heli_drop_01_active [counter] = true;
					area_spawn_heli_drop_02_active [counter] = true;
					area_spawn_parachute_active [counter] = true;
					area_unit_max [counter] = 3;
					break;
				}
			}
			counter++;
		}

		// Create the GDI Transport Helicopter and fly it out of scene.

		GameObject * helicopter = Commands->Create_Object ("GDI_Transport_Helicopter", Vector3(-26.87f,37.42f,20.75f));
		if (helicopter)
		{
			Commands->Set_Facing (helicopter, -155.0f);
			Commands->Attach_Script (helicopter, "M02_GDI_Helicopter", "");
		}

		// Drop the first Humm-Vee into the start area.

		Call_GDI_HummVee (0);

		// Wake up the A25 units.

		Commands->Send_Custom_Event(obj, Commands->Find_Object(400985), 0, 0, 0.0f);

		// Call the ORCA strike intro.

		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		if (chinook_obj1)
		{
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2A_OrcaStrike.txt");
		}

		Commands->Start_Timer(obj, this, 3.0f, 2);
	}

	void Call_GDI_HummVee (int /*area_id*/)
	{
		Vector3 drop_loc = Vector3(-87.2f,-13.1f,-40.1f);
		float facing = 102.9f;
		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

		if (chinook_obj1)
		{
			Commands->Set_Facing(chinook_obj1, facing);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2I_GDI_Drop_HummVee.txt");
		}
	}

	void Call_GDI_PowerUp (int /*area_id*/)
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
			int players_difficulty = Commands->Get_Difficulty_Level();
			float timer_length = 25 - (10 * (players_difficulty));
			Commands->Start_Timer(obj, this, timer_length, 2);
		}
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
								Use_Heli_Drop_01(area_id);
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
								Use_Heli_Drop_02(area_id);
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
								Use_Parachute_Drop(area_id);
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
					Use_Spawners (area_id);
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

	void Use_Heli_Drop_01 (int area_id)
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		textfile = "X2I_Drop03_Area00.txt";
		parameters = "0,107";
		drop_location = Vector3(0.0f,0.0f,0.0f);
		facing = 0.0f;

		switch (area_id)
		{
		case (0): // A00 Guard Tower
			{
				parameters = "0,107";
				textfile = "X2I_Drop03_Area00.txt";
				drop_location = Vector3(119.5f,6.1f,-19.7f);
				facing = 140.0f;
				break;
			}
		case (1):
			{
				parameters = "1,107";
				textfile = "X2I_Drop03_Area01.txt";
				drop_location = Vector3(515.5f,17.1f,-58.9f);
				facing = 51.3f;
				break;
			}
		case (2):
			{
				parameters = "2,107";
				textfile = "X2I_Drop03_Area02.txt";
				drop_location = Vector3(619.9f,310.4f,-59.9f);
				facing = 0.0f;
				break;
			}
		case (3):
			{
				parameters = "3,107";
				textfile = "X2I_Drop03_Area03.txt";
				drop_location = Vector3(530.4f,690.8f,-19.2f);
				facing = -22.8f;
				break;
			}
		case (4):
			{
				parameters = "4,107";
				textfile = "X2I_Drop03_Area04.txt";
				drop_location = Vector3(461.5f,908.0f,5.3f);
				facing = -102.4f;
				break;
			}
		case (6):
			{
				parameters = "6,107";
				textfile = "X2I_Drop03_Area06.txt";
				drop_location = Vector3(772.1f,969.6f,21.4f);
				facing = 163.2f;
				break;
			}
		case (9):
			{
				parameters = "9,107";
				textfile = "X2I_Drop03_Area09.txt";
				drop_location = Vector3(957.638f,879.421f,34.220f);
				facing = 55.0f;
				break;
			}
		case (10):
			{
				parameters = "10,107";
				textfile = "X2I_Drop03_Area10.txt";
				drop_location = Vector3(1044.01f,947.144f,-37.816f);
				facing = -83.5f;
				break;
			}
		case (11):
			{
				parameters = "11,107";
				textfile = "X2I_Drop03_Area11.txt";
				drop_location = Vector3(1072.776f,997.836f,6.492f);
				facing = -35.0f;
				break;
			}
		case (21):
			{
				parameters = "21,107";
				textfile = "X2I_Drop03_Area21.txt";
				drop_location = Vector3(1173.998f,478.803f,22.657f);
				facing = 55.0f;
				break;
			}
		}

		GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_location);

		if (chinook_obj1)
		{
			Commands->Set_Facing(chinook_obj1, facing);
			Commands->Attach_Script(chinook_obj1, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(chinook_obj1, cinematic, textfile);
		}
	}

	// This uses a different helicopter route to drop two units.

	void Use_Heli_Drop_02 (int area_id)
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		textfile = "X2I_Drop02_Area00.txt";
		parameters = "0,108";
		drop_location = Vector3(0.0f,0.0f,0.0f);
		facing = 0.0f;

		switch (area_id)
		{
		case (0):
			{
				parameters = "0,108";
				textfile = "X2I_Drop02_Area00.txt";
				drop_location = Vector3(99.4f,-13.8f,-28.4f);
				facing = 25.1f;
				break;
			}
		case (1):
			{
				parameters = "1,108";
				textfile = "X2I_Drop02_Area01.txt";
				drop_location = Vector3(522.4f,128.1f,-58.7f);
				facing = -133.9f;
				break;
			}
		case (2):
			{
				parameters = "2,108";
				textfile = "X2I_Drop02_Area02.txt";
				drop_location = Vector3(614.8f,278.9f,-52.3f);
				facing = 22.8f;
				break;
			}
		case (3):
			{
				parameters = "3,108";
				textfile = "X2I_Drop02_Area03.txt";
				drop_location = Vector3(506.11f,567.12f,-39.60f);
				facing = 66.9f;
				break;
			}
		case (4):
			{
				parameters = "4,108";
				textfile = "X2I_Drop02_Area04.txt";
				drop_location = Vector3(471.1f,854.9f,3.7f);
				facing = -171.2f;
				break;
			}
		case (6):
			{
				parameters = "6,108";
				textfile = "X2I_Drop02_Area06.txt";
				drop_location = Vector3(786.0f,1032.3f,21.3f);
				facing = 168.7f;
				break;
			}
		case (9):
			{
				parameters = "9,108";
				textfile = "X2I_Drop02_Area09.txt";
				drop_location = Vector3(1047.111f,879.541f,34.220f);
				facing = 140.0f;
				break;
			}
		case (10):
			{
				parameters = "10,108";
				textfile = "X2I_Drop02_Area10.txt";
				drop_location = Vector3(970.504f,941.132f,-37.812f);
				facing = -130.0f;
				break;
			}
		case (11):
			{
				parameters = "11,108";
				textfile = "X2I_Drop02_Area11.txt";
				drop_location = Vector3(1092.028f,975.856f,4.14f);
				facing = 15.0f;
				break;
			}
		case (21):
			{
				parameters = "21,108";
				textfile = "X2I_Drop02_Area21.txt";
				drop_location = Vector3(1173.369f,562.608f,32.394f);
				facing = 0.0f;
				break;
			}
		}

		GameObject * chinook_obj2 = Commands->Create_Object("Invisible_Object", drop_location);

		if (chinook_obj2)
		{
			Commands->Set_Facing(chinook_obj2, facing);
			Commands->Attach_Script(chinook_obj2, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(chinook_obj2, cinematic, textfile);
		}
	}

	// This uses a cargo plane parachute drop to drop three units.

	void Use_Parachute_Drop (int area_id)
	{
		Vector3 drop_location;
		float facing;
		const char* cinematic;
		const char* textfile;
		const char* parameters;

		cinematic = "Test_Cinematic";
		textfile = "X2I_Para03_Area00.txt";
		parameters = "0,109";
		drop_location = Vector3(0.0f,0.0f,0.0f);
		facing = 0.0f;

		switch (area_id)
		{
		case (0):
			{
				textfile = "X2I_Para03_Area00.txt";
				parameters = "0,109";
				drop_location = Vector3(119.5f,38.2f,-15.6f);
				facing = -144.9f;
				break;
			}
		case (1):
			{
				textfile = "X2I_Para03_Area01.txt";
				parameters = "1,109";
				drop_location = Vector3(509.7f,78.3f,-55.2f);
				facing = -73.7f;
				break;
			}
		case (2):
			{
				textfile = "X2I_Para03_Area02.txt";
				parameters = "2,109";
				drop_location = Vector3(598.7f,342.2f,-62.7f);
				facing = -119.1f;
				break;
			}
		case (3):
			{
				textfile = "X2I_Para03_Area03.txt";
				parameters = "3,109";
				drop_location = Vector3(488.2f,608.4f,-41.2f);
				facing = -158.2f;
				break;
			}
		case (4):
			{
				textfile = "X2I_Para03_Area04.txt";
				parameters = "4,109";
				drop_location = Vector3(371.1f,794.0f,3.7f);
				facing = 90.0f;
				break;
			}
		case (6):
			{
				textfile = "X2I_Para03_Area06.txt";
				parameters = "6,109";
				drop_location = Vector3(765.5f,1004.0f,21.4f);
				facing = -14.4f;
				break;
			}
		case (9):
			{
				textfile = "X2I_Para03_Area09.txt";
				parameters = "9,109";
				drop_location = Vector3(1028.1f,876.4f,34.2f);
				facing = 87.2f;
				break;
			}
		case (21):
			{
				textfile = "X2I_Para03_Area21.txt";
				parameters = "21,109";
				drop_location = Vector3(1255.732f,565.134f,17.602f);
				facing = 150.0f;
				break;
			}
		}

		GameObject * plane_obj = Commands->Create_Object("Invisible_Object", drop_location);

		if (plane_obj)
		{
			Commands->Set_Facing(plane_obj, facing);
			Commands->Attach_Script(plane_obj, "M02_Reset_Spawn", parameters);
			Commands->Attach_Script(plane_obj, cinematic, textfile);
		}
	}

	// This uses the regular spawners to generate one unit.

	bool Use_Spawners (int area_id)
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

		switch (area_id)
		{
		case (0):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400233;
						break;
					}
				case (1):
					{
						spawner_id = 400236;
						break;
					}
				default:
					{
						spawner_id = 400237;
						break;
					}
				}
				spawner_params = "0,0,-1";
				break;
			}
		case (1):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400234;
						break;
					}
				case (1):
					{
						spawner_id = 400238;
						break;
					}
				default:
					{
						spawner_id = 400239;
						break;
					}
				}
				spawner_params = "1,0,-1";
				break;
			}
		case (2):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400235;
						break;
					}
				case (1):
					{
						spawner_id = 400240;
						break;
					}
				default:
					{
						spawner_id = 400241;
						break;
					}
				}
				spawner_params = "2,0,-1";
				break;
			}
		case (3):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400242;
						break;
					}
				case (1):
					{
						spawner_id = 400243;
						break;
					}
				default:
					{
						spawner_id = 400244;
						break;
					}
				}
				spawner_params = "3,0,-1";
				break;
			}
		case (4):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400245;
						break;
					}
				case (1):
					{
						spawner_id = 400246;
						break;
					}
				default:
					{
						spawner_id = 400247;
						break;
					}
				}
				spawner_params = "4,0,-1";
				break;
			}
		case (6):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400248;
						break;
					}
				case (1):
					{
						spawner_id = 400249;
						break;
					}
				default:
					{
						spawner_id = 400250;
						break;
					}
				}
				spawner_params = "6,0,-1";
				break;
			}
		case (8):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400261;
						break;
					}
				case (1):
					{
						spawner_id = 400262;
						break;
					}
				default:
					{
						spawner_id = 400263;
						break;
					}
				}
				spawner_params = "8,0,-1";
				break;
			}
		case (9):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400251;
						break;
					}
				case (1):
					{
						spawner_id = 400252;
						break;
					}
				default:
					{
						spawner_id = 400253;
						break;
					}
				}
				spawner_params = "9,0,-1";
				break;
			}
		case (10):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400254;
						break;
					}
				case (1):
					{
						spawner_id = 400255;
						break;
					}
				default:
					{
						spawner_id = 400256;
						break;
					}
				}
				spawner_params = "10,0,-1";
				break;
			}
		case (11):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400257;
						break;
					}
				case (1):
					{
						spawner_id = 400258;
						break;
					}
				default:
					{
						spawner_id = 400259;
						break;
					}
				}
				spawner_params = "11,0,-1";
				break;
			}
		case (14):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 401081;
						break;
					}
				case (1):
					{
						spawner_id = 401082;
						break;
					}
				default:
					{
						spawner_id = 401083;
						break;
					}
				}
				spawner_params = "14,0,-1";
				break;
			}
		case (15):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 401098;
						break;
					}
				case (1):
					{
						spawner_id = 401099;
						break;
					}
				default:
					{
						spawner_id = 401100;
						break;
					}
				}
				spawner_params = "15,0,-1";
				break;
			}
		case (21):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400264;
						break;
					}
				case (1):
					{
						spawner_id = 400265;
						break;
					}
				default:
					{
						spawner_id = 400266;
						break;
					}
				}
				spawner_params = "21,0,-1";
				break;
			}
		case (24):
			{
				switch (spawner_rotator)
				{
				case (0):
					{
						spawner_id = 400540;
						break;
					}
				case (1):
					{
						spawner_id = 400541;
						break;
					}
				default:
					{
						spawner_id = 400542;
						break;
					}
				}
				spawner_params = "24,0,-1";
				break;
			}
		}
		GameObject * spawn1 = Commands->Trigger_Spawner(spawner_id);
		if (spawn1)
		{
			Commands->Attach_Script(spawn1, "M02_Nod_Soldier", spawner_params);
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

			Call_GDI_PowerUp (3);
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
				Commands->Stop_All_Conversations ();
				int id = Commands->Create_Conversation("M02_HIDDEN_02_FINISH", 100, 300, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation(id, 0);
			}
		}
	}

	// When a player controlled vehicle is destroyed, it sends a custom to this controller
	// to turn on the ability to create a new vehicle in a certain area.
	// Another object then sends a custom to check for new vehicle creation. If the flag for
	// creating a new vehicle for this area is active, it is reset and the vehicle is delivered.

	void Replacement_Vehicle (GameObject * /*obj*/, int area_id)
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
				facing = 30.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Buggy_Player", drop_loc);
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
				drop_loc = Vector3(448.32f,717.19f,-12.35f);
				facing = -30.0f;
				textfile = "X2I_GDI_Drop_MediumTank.txt";
				break;
			}
		case (4):
			{
				drop_loc = Vector3(429.672f, 894.045f, 4.354f);
				facing = -25.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Light_Tank_Player", drop_loc);
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
				GameObject *buggy = Commands->Create_Object ("Nod_Recon_Bike_Player", drop_loc);
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
				facing = 138.0f;
				textfile = "X2I_GDI_Drop_MediumTank.txt";
				break;
			}
		case (14):
			{
				drop_loc = Vector3(1229.37f, 742.89f, 27.03f);
				facing = 85.0f;
				GameObject *buggy = Commands->Create_Object ("Nod_Recon_Bike_Player", drop_loc);
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


DECLARE_SCRIPT (M02_Reset_Spawn, "Area_ID:int,Spawn_Type:int")
{
	void Destroyed (GameObject *obj) override
	{
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			int area_id;
			int spawn_type;
			area_id = Get_Int_Parameter("Area_ID");
			spawn_type = Get_Int_Parameter("Spawn_Type");
			Commands->Send_Custom_Event(obj, object, spawn_type, area_id, 0.0f);
		}
	}
};


DECLARE_SCRIPT (M02_Nod_Soldier, "Area_Number:int,Area_Officer:int,Pre_Placed:int")
{
	Vector3 my_home_point;
	bool no_return_home;
	int officeranimcounter;
	bool initial_damage;
	float initial_health;
	bool enemy_seen;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (my_home_point, 1);
		SAVE_VARIABLE (no_return_home, 2);
		SAVE_VARIABLE (officeranimcounter, 3);
		SAVE_VARIABLE (initial_damage, 4);
		SAVE_VARIABLE (initial_health, 5);
		SAVE_VARIABLE (enemy_seen, 6);
	}

	void Created (GameObject *obj) override
	{
		officeranimcounter = 0;
		no_return_home = false;
		enemy_seen = false;
		initial_damage = false;
		initial_health = Commands->Get_Health(obj);

		// Set the unit's home point, save the value.

		Vector3 this_home_point = Commands->Get_Position(obj);
		Commands->Set_Innate_Soldier_Home_Location(obj, this_home_point, 20.0f);

		// Turn hibernation off for a moment.
		
		Commands->Enable_Hibernation (obj, 0);

		// Start the timer to register with the controller.

		Commands->Start_Timer(obj, this, 1.0f, 1);

		// If the unit is spawned, do not disturb it.
		// If the unit is preplaced, turn of actions.
		// If the unit is flown in, order it to move to the commando.

		int preplaced = Get_Int_Parameter("Pre_Placed");

		if (!preplaced)
		{
			// Give spawned units a green key for the Hand of Nod.

			Commands->Grant_Key (obj, 1, true);

			GameObject *starobj = Commands->Get_A_Star (this_home_point);

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

	void Enemy_Seen (GameObject * /*obj*/, GameObject * /*enemy*/) override
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
					Commands->Enable_Hibernation (obj, 1);
				}
				else
				{
					if (preplaced > 0)
					{

						// SPECIAL CASE SETUP: Force fire routine for Rocket Soldiers.

						if (preplaced == 4)
						{
							Commands->Attach_Script (obj, "M00_Soldier_Powerup_Disable", "");

							if ((param == 0) || (param == 3) || (param == 6) || (param == 10) || (param == 16) || (param == 21))
							{
								no_return_home = true;
								Commands->Start_Timer(obj, this, 15.0f, 4);
							}
						}

						// Soldiers in start cinematic begin self-destruct.

						if ((preplaced == 2) && (param == 99))
						{
							Commands->Start_Timer(obj, this, 5.0f, 6);
						}
						Commands->Enable_Hibernation (obj, 1);
					}
					else
					{
						// Unit is not pre-placed, activate cleanup code and do not hibernate if not in the interior areas.

						Commands->Start_Timer(obj, this, 15.0f, 2);
					}
				}
			}
		}
		else if (timer_id == 2)
		{
			// Cleanup code - find the nearest star, check distance, then check visibility

			Vector3 myloc = Commands->Get_Position(obj);
			GameObject *star_obj = Commands->Get_A_Star(myloc);

			if (star_obj)
			{
				Vector3 starloc = Commands->Get_Position(star_obj);
				float distance = Commands->Get_Distance (myloc, starloc);
				
				if (distance > 70.0f)
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
			Commands->Start_Timer(obj, this, 5.0f, 2);
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

			if ((distance > 10.0f) && (!Commands->Is_Performing_Pathfind_Action(obj)))
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
		else if (timer_id == 10)
		{
			Vector3 my_pos = Commands->Get_Position (obj);
			Vector3 star_pos = Commands->Get_Position (STAR);
			float distance = Commands->Get_Distance (my_pos, star_pos);
			if (distance < 20.0f)
			{
				int id = Commands->Create_Conversation ("MX2DSGN_DSGN0014", 100, 300, true);
				Commands->Join_Conversation (NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation (id, 0);
				Commands->Start_Timer (obj, this, 3.0f, 11);
			}
			else
			{
				Commands->Start_Timer (obj, this, 2.0f, 10);
			}
		}
		else if (timer_id == 11)
		{
			int id = Commands->Create_Conversation ("MX2DSGN_DSGN0015", 100, 300, true);
			Commands->Join_Conversation (NULL, id, true, true, true);
			Commands->Join_Conversation (STAR, id, true, false, false);
			Commands->Start_Conversation (id, 0);
		}
	}

	void Force_Move (GameObject* obj)
	{
		if (!Commands->Is_Performing_Pathfind_Action(obj))
		{
			Vector3 mypos = Commands->Get_Position(obj);
			GameObject *starobj = Commands->Get_A_Star (mypos);
			if (starobj)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 90, 0);
				params.Set_Movement(starobj, RUN, 10.0f, 0);
				params.Set_Attack(starobj, 150.0f, 0.0f, true);
				Commands->Action_Attack(obj, params);
			}
		}
	}

	void Killed (GameObject *obj, GameObject * /*killer*/) override
	{
		// Tell the respawn controller I am dead.

		int param;
		param = Get_Int_Parameter("Area_Number");
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			Commands->Send_Custom_Event(obj, object, 101, param, 0.0f);
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
		int specific_id = Commands->Get_ID (obj);
		if (specific_id == 400276)
		{
			Commands->Start_Timer (obj, this, 2.0f, 10);
		}

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
		if ((type == 99) && (!Commands->Is_Performing_Pathfind_Action(obj)))
		{
			if (param == 214)
			{

				// An Objective or Building is asking to be repaired.

				ActionParamsStruct params;
				params.Set_Basic(this, 71, 214);
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
				initial_health = Commands->Get_Max_Health(obj);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject *damager, float /*amount*/) override
	{
		if (!initial_damage && damager == NULL)
		{
			initial_damage = true;
			Commands->Set_Health(obj, initial_health);
		}
	}
};


DECLARE_SCRIPT(M02_Obelisk, "")
{
	bool info_given;

	void Created(GameObject* /*obj*/) override
	{
		info_given = false;
	}

	void Damaged(GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		if (Commands->Get_Building_Power (obj))
		{
			// Check if the engineer is alive

			GameObject * obj_engineer = Commands->Find_Object(400200);

			if (obj_engineer)
			{
				// Tell the Engineer to repair the building.

				Commands->Send_Custom_Event (obj, obj_engineer, 99, 202, 0.0f);

				//Actually repair the building.

				float maxhealth = Commands->Get_Max_Health (obj);
				Commands->Set_Health (obj, maxhealth);

				// Announce the returned health as an engineer repairing internally.
				
				if ((!info_given) && (damager == STAR))
				{
					info_given = true;
					Commands->Stop_All_Conversations ();
					int id = Commands->Create_Conversation("M02_EVA_OBELISK_REPAIR", 100, 300, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation(id, 0);
				}
			}
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		if (Commands->Get_Building_Power (obj))
		{
			GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

			if (object)
			{
				Commands->Send_Custom_Event(obj, object, 202, 1, 0.0f);
			}
		}
		GameObject * zone = Commands->Find_Object (405116);
		if (zone)
		{
			Commands->Destroy_Object (zone);
		}
		zone = Commands->Find_Object (405117);
		if (zone)
		{
			Commands->Destroy_Object (zone);
		}
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == 1)
		{
			info_given = true;
		}
	}
};


DECLARE_SCRIPT (M02_Power_Plant, "")
{
	bool info_given;

	void Created(GameObject* /*obj*/) override
	{
		info_given = false;
	}

	void Damaged(GameObject * obj, GameObject * damager, float /*amount*/) override
	{
		// Check if the engineer is alive

		GameObject * obj_engineer = Commands->Find_Object(400199);

		if (obj_engineer)
		{
			// Tell the Engineer to repair the building.

			Commands->Send_Custom_Event (obj, obj_engineer, 99, 217, 0.0f);

			//Actually repair the building.

			float maxhealth = Commands->Get_Max_Health (obj);
			Commands->Set_Health (obj, maxhealth);

			// Announce the returned health as an engineer repairing internally.

			if ((!info_given) && (damager == STAR))
			{
				info_given = true;
				Commands->Stop_All_Conversations ();
				int id = Commands->Create_Conversation("M02_EVA_POWER_REPAIR", 100, 300, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation(id, 0);
			}
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			Commands->Send_Custom_Event(obj, object, 217, 1, 0.0f);
			Commands->Send_Custom_Event(obj, object, 202, 1, 0.0f);
		}

		GameObject * obelisk = Commands->Find_Object(1153845);
		
		if (obelisk)
		{
			Commands->Set_Building_Power(obelisk, false);
		}

		GameObject * zone = Commands->Find_Object (405116);
		if (zone)
		{
			Commands->Destroy_Object (zone);
		}
		zone = Commands->Find_Object (405117);
		if (zone)
		{
			Commands->Destroy_Object (zone);
		}

		GameObject * hand_of_nod = Commands->Find_Object(1157690);

		if (hand_of_nod)
		{
			Commands->Set_Building_Power(hand_of_nod, false);
		}
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == 1)
		{
			info_given = true;
		}
	}
};


DECLARE_SCRIPT (M02_Dam_MCT, "")
{
	bool destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (destroyed, 1);
	}

	void Created (GameObject * /*obj*/) override
	{
		destroyed = false;
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		float health = Commands->Get_Health(obj);
		if (health < 1.0f)
		{
			Commands->Set_Health (obj, 0.1f);
			if (!destroyed)
			{
				destroyed = true;
				GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);
				if (object)
				{
					Commands->Send_Custom_Event(obj, object, 203, 1, 0.0f);
					Commands->Grant_Key (STAR, 6, true);
				}
				Commands->Set_Animation_Frame (obj, "MCT_NOD.MCT_NOD", 7);
			}
		}
	}
};


DECLARE_SCRIPT (M02_Helipad, "")
{
	void Killed(GameObject *obj, GameObject * /*killer*/) override
	{
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			Commands->Send_Custom_Event(obj, object, 206, 1, 0.0f);
		}
	}
};


DECLARE_SCRIPT (M02_Destroy_Objective, "Objective_ID:int")
{
	void Killed(GameObject *obj, GameObject * /*killer*/) override
	{
		int objective_id;
		objective_id = Get_Int_Parameter ("Objective_ID");
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			if (objective_id != 222)
			{
				Commands->Send_Custom_Event(obj, object, objective_id, 1, 0.0f);
			}

			// Special case items, mission is solved by above line!

			if (objective_id == 212)
			{
				// Drop powerups into A03.

				Commands->Send_Custom_Event(obj, object, 113, 3, 0.0f);
			}
			else if (objective_id == 214)
			{
				GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", Vector3(580.37f,889.69f,-0.57f));

				if (chinook_obj1)
				{
					Commands->Set_Facing(chinook_obj1, -180.0f);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X2I_GDI_Drop02_Rocket_24.txt");
				}
			}
			else if ((objective_id == 215) || (objective_id == 216))
			{
				// Send a custom to the controller to count destroyed SAMs on the bridge.

				Commands->Send_Custom_Event(obj, object, 115, 0, 0.0f);
			}
			else if (objective_id == 204)
			{
				// Hand of Nod destroyed, turn off respawns in the area.

				Commands->Send_Custom_Event(obj, object, 105, 21, 0.0f);
			}
			else if (objective_id == 222)
			{
				// Send a custom to the controller to count destroyed Tiberium Silos.

				Commands->Send_Custom_Event(obj, object, 116, 0, 0.0f);
			}
		}
	}

	void Damaged(GameObject *obj, GameObject * /*damager*/, float /*amount*/) override
	{
		int objective_id;
		objective_id = Get_Int_Parameter ("Objective_ID");
		if (objective_id == 214)
		{
			GameObject* engineer = Commands->Find_Object(400510);
			if (engineer)
			{
				Commands->Send_Custom_Event(obj, engineer, 99, 214, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT (M02_Nod_Convoy_Truck, "")
{
	void Killed(GameObject *obj, GameObject * /*killer*/) override
	{
		GameObject * object = Commands->Find_Object(M02_OBJCONTROLLER);

		if (object)
		{
			// Tell the controller to subtract a truck.

			Commands->Send_Custom_Event(obj, object, 900, 3, 0.0f);

			// Create an explosion here.

			Vector3 myloc;
			myloc = Commands->Get_Position(obj);
		}
	}
};


// SOLDIER TYPE PARAMETERS FOR GDI
// 1 = Rocket Soldier
// 2 = Minigunner Officer
// 3 = Minigunner Enlisted
// 4 = Grenadier Enlisted

DECLARE_SCRIPT (M02_GDI_Soldier, "Area_ID:int, Soldier_Type=0:int")
{
	bool stop_following;
	bool said_message;
	int shout_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (stop_following, 1);
		SAVE_VARIABLE (said_message, 2);
		SAVE_VARIABLE (shout_count, 3);
	}

	void Created(GameObject* obj) override
	{
		Commands->Enable_Hibernation(obj, false);
		stop_following = false;
		said_message = false;
		shout_count = 0;

		// Special case checks for preplaced soldiers.
		
		int area_id = Get_Int_Parameter("Area_ID");

		if (area_id == 1)
		{
			Commands->Innate_Disable (obj);
			int soldier_type = Get_Int_Parameter ("Soldier_Type");
			if (soldier_type == 2)
			{
				Commands->Start_Timer(obj, this, 10.0f, 2);
			}
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
			int area_id = Get_Int_Parameter("Area_ID");
			if (area_id == 9)
			{
				// GDI soldiers in A09 Dam Top rush the Obelisk and die.

				GameObject * obelisk = Commands->Find_Object(1153845);
				
				if (obelisk)
				{
					bool power = Commands->Get_Building_Power (obelisk);
					if (power)
					{
						ActionParamsStruct params;
						params.Set_Basic(this, 71, 0);
						params.Set_Movement(Vector3(1105.78f,897.75f,35.05f), 1.0f, 20.0f, 0);
						Commands->Action_Goto(obj, params);
					}
				}
			}
			else if (!stop_following)
			{
				// Follow the commando. Start this check timer again.

				Vector3 mypos = Commands->Get_Position(obj);
				GameObject *starobj = Commands->Get_A_Star (mypos);
				if (starobj)
				{
					ActionParamsStruct params;
					params.Set_Basic(this, 71, 1);
					params.Set_Movement(starobj, 1.0f, 20.0f, 0);
					Commands->Action_Goto(obj, params);
				}
				else
				{
					stop_following = true;
				}
			}
			Commands->Start_Timer(obj, this, 10.0f, 1);
		}
		else if (timer_id == 2)
		{
			if(!said_message)
			{
				Vector3 mypos = Commands->Get_Position (obj);
				Vector3 starpos = Commands->Get_Position (STAR);
				float distance = Commands->Get_Distance (mypos, starpos);
				if (distance < 30.0f)
				{
					switch (shout_count)
					{
					case (0):
						{
							int id = Commands->Create_Conversation("M02_GOMG_RADIO_01", 99, 20, true);
							Commands->Join_Conversation (obj, id, true, true, true);
							Commands->Start_Conversation(id, 0);
							shout_count++;
							break;
						}
					case (1):
						{
							int id = Commands->Create_Conversation("M02_GOMG_RADIO_02", 99, 20, true);
							Commands->Join_Conversation (obj, id, true, true, true);
							Commands->Start_Conversation(id, 0);
							shout_count++;
							break;
						}
					case (2):
						{
							int id = Commands->Create_Conversation("M02_GOMG_RADIO_03", 99, 20, true);
							Commands->Join_Conversation (obj, id, true, true, true);
							Commands->Start_Conversation(id, 0);
							shout_count++;
							break;
						}
					default:
						{
							break;
						}
					}
				}
				Commands->Start_Timer(obj, this, 10.0f, 2);
			}
		}
	}

	void Poked (GameObject * obj, GameObject * /*poker*/) override
	{
		int soldier_type = Get_Int_Parameter ("Soldier_Type");
		if ((soldier_type == 2) && (!said_message))
		{
			said_message = true;
			int id = Commands->Create_Conversation("M02_GOMG_CONVERSATION", 99, 10, true);
			Commands->Join_Conversation (obj, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, false);
			Commands->Start_Conversation(id, 0);
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
						Commands->Action_Reset (obj, 100);
						break;
					}
				case (2):
					{
						Vector3 house_loc = Vector3(648.77f,300.74f,-59.99f);
						ActionParamsStruct params;
						params.Set_Basic(this, 81, 0);
						params.Set_Movement(house_loc, WALK, 1.0f, 0);
						Commands->Action_Goto(obj, params);
						break;
					}
				}
			}
		}
	}

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 0) && (param == 0))
		{
			// A zone is trying to enable actions for this soldier.

			Commands->Innate_Enable (obj);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		int area_id = Get_Int_Parameter("Area_ID");
		if (area_id == 0)
		{
			GameObject * controller = Commands->Find_Object (M02_OBJCONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, 1000, 1000, 0.0f);
			}
		}
		else if (area_id == 9)
		{
			GameObject * controller = Commands->Find_Object (M02_OBJCONTROLLER);
			if (controller)
			{
				Commands->Send_Custom_Event (obj, controller, 1000, 1001, 0.0f);
			}
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason complete_reason) override
	{
		if ((action_id == 1) && (complete_reason == ACTION_COMPLETE_NORMAL))
		{
			if (!said_message)
			{
				said_message = true;

				int soldier_type = Get_Int_Parameter ("Soldier_Type");

				switch (soldier_type)
				{
				case (1): // Rocket Soldier
					{
						int id = Commands->Create_Conversation("M02_GERS_COVER_01", 99, 50, true);
						Commands->Join_Conversation (obj, id, true, true, false);
						Commands->Join_Conversation(STAR, id, true, true, false);
						Commands->Start_Conversation(id, 0);
						break;
					}
				case (4): // Grenadier
					{
						int id = Commands->Create_Conversation("M02_GEGR_COVER_01", 99, 50, true);
						Commands->Join_Conversation (obj, id, true, true, false);
						Commands->Join_Conversation(STAR, id, true, true, false);
						Commands->Start_Conversation(id, 0);
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


DECLARE_SCRIPT (M02_Stationary_Vehicle,"Area_ID:int")
{
	void Created (GameObject* obj) override
	{
		int area_id;
		area_id = Get_Int_Parameter("Area_ID");

		// Area One is a conflict area, other stationary vehicles in conflict areas must be paused as well

		Commands->Enable_Enemy_Seen (obj, false);

		if (area_id == 99)
		{
			Commands->Innate_Disable (obj);
			Commands->Start_Timer (obj, this, 5.33f, 1);
		}
	}

	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		int area_id = Get_Int_Parameter ("Area_ID");
		int my_id = Commands->Get_ID (obj);
		if (area_id == 2)
		{
			if (my_id == 401016)
			{
				int id = Commands->Create_Conversation ("MX2DSGN_DSGN0008", 100, 300, true);
				Commands->Join_Conversation (NULL, id, true, true, true);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Start_Conversation (id, 0);
			}
			else if (my_id == 401167)
			{
				GameObject * tank = Commands->Find_Object (401016);
				if (tank)
				{
					int id = Commands->Create_Conversation ("MX2DSGN_DSGN0007", 100, 300, true);
					Commands->Join_Conversation (tank, id, true, true, true);
					Commands->Join_Conversation (STAR, id, true, false, false);
					Commands->Start_Conversation (id, 0);
				}
			}
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		int area_id;
		area_id = Get_Int_Parameter("Area_ID");
		if (area_id != 99)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Attack(enemy, 300.0f, 0.0f, true);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this, 5.0f, 2);
		}
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

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /*sender*/) override
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


DECLARE_SCRIPT (M02_Nod_Apache, "Area_ID:int")
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
				params.MovePathfind = false;
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

			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(myloc, 0.0, 5.0f);
			params.MovePathfind = false;
			params.Set_Attack(STAR, 200.0f, 0.0f, true);
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
				if (distance > 300.0f)
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


DECLARE_SCRIPT (M02_Nod_Vehicle, "Area_ID:int")
{
	void Created (GameObject* obj) override
	{
		Commands->Enable_Enemy_Seen (obj, true);
		Commands->Enable_Engine(obj, true);

		// Get my area id and determine what I need to to immediately.

		int area_id = Get_Int_Parameter("Area_ID");

		switch (area_id)
		{
		case (2):
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98, 0);
				params.Set_Movement(Vector3(0,0,0), 1.0f, 5.0f);
				params.WaypathID = 400375;
				params.WaypathSplined = false;
				params.Set_Attack(STAR, 300.0f, 0.0f, true);
				Commands->Action_Attack(obj, params);
				break;
			}
		case (3):
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98, 0);
				params.Set_Movement(Vector3(0,0,0), 1.0f, 5.0f);
				params.WaypathID = 400416;
				params.WaypathSplined = false;
				params.Set_Attack(STAR, 300.0f, 0.0f, true);
				Commands->Action_Attack(obj, params);
				break;
			}
		case (4):
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98, 0);
				params.Set_Movement(Vector3(0,0,0), 1.0f, 5.0f);
				params.WaypathID = 400454;
				params.WaypathSplined = false;
				params.Set_Attack(STAR, 300.0f, 0.0f, true);
				Commands->Action_Attack(obj, params);
				break;
			}
		}
		Commands->Start_Timer(obj, this, 15.0f, 1);
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

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			// Cleanup code - find the nearest star, check distance, then check visibility

			float distance;
			Vector3 myloc;
			Vector3 starloc;
			myloc = Commands->Get_Position(obj);
			GameObject *star_obj = Commands->Get_A_Star(myloc);
			if (star_obj)
			{
				starloc = Commands->Get_Position(star_obj);
				distance = Commands->Get_Distance (myloc, starloc);
				if (distance > 300.0f)
				{
					Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 1);
		}
	}
};


DECLARE_SCRIPT (M02_Player_Vehicle, "Area_ID:int")
{
	bool entered_me;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered_me, 1);
	}

	void Created (GameObject* obj) override
	{
		entered_me = false;
		Commands->Enable_Vehicle_Transitions (obj, true);
		Commands->Set_Player_Type (obj, SCRIPT_PLAYERTYPE_NEUTRAL);
		Commands->Start_Timer(obj, this, 15.0f, 1);
		int unitid = Commands->Get_ID (obj);
		Send_Custom_To_SAM_Sites (obj, M00_CUSTOM_SAM_SITE_IGNORE, unitid);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			// Check if this vehicle needs to be removed.

			Vector3 myloc = Commands->Get_Position(obj);
			GameObject *star_obj = Commands->Get_A_Star(myloc);

			if (star_obj)
			{
				Vector3 starloc = Commands->Get_Position(star_obj);
				float distance = Commands->Get_Distance (myloc, starloc);

				if (distance > 300.0f)
				{
					Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 1);
		}
	}

	void Destroyed (GameObject* obj) override
	{
		int area_id = Get_Int_Parameter ("Area_ID");
		GameObject* controller = Commands->Find_Object(M02_OBJCONTROLLER);
		if (controller)
		{
			Commands->Send_Custom_Event(obj, controller, 112, area_id, 0.0f);
		}
		if ((area_id == 2) && (!entered_me))
		{
			Commands->Send_Custom_Event(obj, controller, 221, 2, 0.0f);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_VEHICLE_ENTERED)
		{
			int area_id = Get_Int_Parameter ("Area_ID");
			if ((area_id == 2) && (!entered_me))
			{
				entered_me = true;
				GameObject* controller = Commands->Find_Object(M02_OBJCONTROLLER);
				if (controller)
				{
					Commands->Send_Custom_Event(obj, controller, 221, 1, 0.0f);
				}
			}
		}
	}
};


DECLARE_SCRIPT (M02_Nod_Sakura, "Area_ID:int")
{
	bool onmywayout;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(onmywayout, 1);
	}

	void Created (GameObject* obj) override
	{
		onmywayout = false;
		int waypath_id = 400397;

		Commands->Enable_Enemy_Seen (obj, true);
		Commands->Enable_Engine(obj, true);
		Commands->Disable_Physical_Collisions(obj);

		ActionParamsStruct params;
		params.Set_Basic(this, 91, 0);
		params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
		params.WaypathID = waypath_id;
		params.WaypathSplined = true;
		params.Set_Attack(STAR, 200.0f, 0.0f, true);
		Commands->Action_Goto(obj, params);

		Commands->Start_Timer(obj, this, 15.0f, 1);
		Commands->Start_Timer(obj, this, 10.0f, 3);
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
			// Cleanup code - find the nearest star, check distance, then check visibility

			float distance;
			Vector3 myloc;
			Vector3 starloc;
			myloc = Commands->Get_Position(obj);
			GameObject *star_obj = Commands->Get_A_Star(myloc);
			if (star_obj)
			{
				starloc = Commands->Get_Position(star_obj);
				distance = Commands->Get_Distance (myloc, starloc);
				if (distance > 300.0f)
				{
					Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
				}
			}
			Commands->Start_Timer(obj, this, 15.0f, 1);
		}
		else if (timer_id == 2)
		{
			Commands->Destroy_Object (obj);
		}
		else if (timer_id == 3)
		{
			Vector3 myloc = Commands->Get_Position(obj);
			ActionParamsStruct params;
			params.Set_Basic(this, 93, 1);
			params.Set_Movement(myloc, 0.0, 5.0f);
			params.Set_Attack(STAR, 200.0f, 0.0f, true);
			params.AttackActive = true;
			Commands->Action_Attack(obj, params);
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


// This script handles approach vehicles.

DECLARE_SCRIPT (M02_Approach_Vehicle, "Area_ID:int")
{
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 0) && (param == 0))
		{
			Commands->Enable_Engine (obj, true);

			// Unit is being activated. Determine waypath and follow it.
			
			int area_id = Get_Int_Parameter("Area_ID");
			ActionParamsStruct params;
			params.Set_Basic(this, 100, 0);
			params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);

			switch (area_id)
			{
			case (1):
				{
					params.WaypathID = 401009;
					break;
				}
			case (4):
				{
					params.WaypathID = 401031;
					break;
				}
			case (6):
				{
					params.WaypathID = 401039;
					break;
				}
			}
			Commands->Action_Goto(obj, params);
			Commands->Start_Timer(obj, this, 6.0f, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
	}
};


// This script is currently only used for an Orca above a SAM Site objective, in A03.

DECLARE_SCRIPT (M02_Destroy_Vehicle, "")
{
	void Created (GameObject* obj) override
	{
		Commands->Enable_Engine(obj, true);
		Commands->Disable_Physical_Collisions(obj);
		Commands->Start_Timer(obj, this, 4.0f, 1);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 1)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 0);
			params.Set_Movement(Vector3(0,0,0), 2.5f, 1.0f);
			params.WaypathID = 400993;
			params.WaypathSplined = true;
			Commands->Action_Goto(obj, params);
			Commands->Start_Timer(obj, this, 20.0f, 2);
		}
		else if (timer_id == 2)
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
	}

	void Damaged (GameObject* obj, GameObject* damager, float /*amount*/) override
	{
		if (damager != STAR)
		{
			Commands->Apply_Damage(obj, 10000.0f, "Blamokiller", NULL);
		}
	}

	void Enemy_Seen (GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 90, 1);
		params.Set_Attack(enemy, 200.0f, 0.0f, true);
		params.AttackActive = true;
		Commands->Action_Attack(obj, params);
	}
};


DECLARE_SCRIPT (M02_Mendoza, "")
{
	bool calling_extraction;
	bool move_toggle;
	float start_health;
	float last_armor;
	int counter;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE (calling_extraction, 1);
		SAVE_VARIABLE (start_health, 2);
		SAVE_VARIABLE (last_armor, 3);
		SAVE_VARIABLE (counter, 4);
		SAVE_VARIABLE (move_toggle, 5);
	}

	void Created (GameObject * obj) override
	{
		start_health = Commands->Get_Health (obj);
		last_armor = Commands->Get_Shield_Strength (obj);
		calling_extraction = false;
		move_toggle = false;
		counter = 0;

		Commands->Innate_Disable (obj);

		ActionParamsStruct params;
		params.Set_Basic (this, 98, 3);
		params.Set_Attack (STAR, 300.0f, 0.0f, true);
		Commands->Action_Attack(obj, params);

		Commands->Start_Timer (obj, this, 2.0f, 1);
	}

	void Damaged (GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health (obj, start_health);
		last_armor -= 2.0f;
		Commands->Set_Shield_Strength (obj, last_armor);

		if (last_armor < 1.0f)
		{
			if (!calling_extraction)
			{
				calling_extraction = true;
				ActionParamsStruct params;
				params.Set_Basic(this, 100, 1);
				params.Set_Movement(Vector3(1260.007f,535.460f,18.428f), RUN, 5.0f);
				Commands->Action_Goto(obj, params);
			}
		}
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 1)
		{
			Commands->Innate_Disable (obj);
			Vector3 my_location = Commands->Get_Position (obj);
			GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", my_location);

			if (chinook_obj1)
			{
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "XG_ROPE_EVAC_F.txt");
				int id = Commands->Create_Conversation ("MX2DSGN_DSGN0025", 100, 300, true);
				Commands->Join_Conversation (obj, id, true, false, false);
				Commands->Join_Conversation (STAR, id, true, false, false);
				Commands->Join_Conversation (NULL, id, true, true, true);
				Commands->Start_Conversation (id, 0);
			}
		}
		else if (action_id == 2)
		{
			ActionParamsStruct params;
			params.Set_Basic (this, 98, 3);
			params.Set_Attack (STAR, 300.0f, 0.0f, true);
			Commands->Action_Attack(obj, params);
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if ((timer_id == 1) && (!calling_extraction))
		{
			int id = Commands->Create_Conversation ("MX2DSGN_DSGN0019", 100, 300, true);

			counter++;
			if (counter > 6)
			{
				counter = 1;
			}

			switch (counter)
			{
			case (1):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0019", 100, 300, true);
					break;
				}
			case (2):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0020", 100, 300, true);
					break;
				}
			case (3):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0021", 100, 300, true);
					break;
				}
			case (4):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0022", 100, 300, true);
					break;
				}
			case (5):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0023", 100, 300, true);
					break;
				}
			case (6):
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0024", 100, 300, true);
					break;
				}
			default:
				{
					id = Commands->Create_Conversation ("MX2DSGN_DSGN0024", 100, 300, true);
					break;
				}
			}
			Commands->Join_Conversation (obj, id, true, false, false);
			Commands->Join_Conversation (STAR, id, true, false, false);
			Commands->Start_Conversation (id, 0);
			Commands->Start_Timer (obj, this, 7.0f, 1);

			if (!move_toggle)
			{
				move_toggle = true;
				Commands->Action_Reset (obj, 100);
				ActionParamsStruct params;
				params.Set_Basic(this, 99, 2);
				switch (Get_Int_Random (0,3))
				{
				case (0):
					{
						params.Set_Movement(Vector3(1248.592f,568.431f,17.396f), RUN, 5.0f);
						break;
					}
				case (1):
					{
						params.Set_Movement(Vector3(1264.680f,564.459f,17.816f), RUN, 5.0f);
						break;
					}
				case (2):
					{
						params.Set_Movement(Vector3(1270.220f,541.406f,17.978f), RUN, 5.0f);
						break;
					}
				case (3):
					{
						params.Set_Movement(Vector3(1251.213f,544.658f,17.854f), RUN, 5.0f);
						break;
					}
				default:
					{
						params.Set_Movement(Vector3(1260.007f,535.460f,18.428f), RUN, 5.0f);
						break;
					}
				}
				Commands->Action_Goto(obj, params);
			}
			else
			{
				move_toggle = false;
			}
		}
	}
};


DECLARE_SCRIPT (M02_Nod_Jet, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Enable_Engine (obj, true);
		Commands->Disable_Physical_Collisions (obj);
		ActionParamsStruct params;
		params.MovePathfind = false;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(1143.09f,702.08f,70.5f), 4.0f, 5.0f);
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 1)
		{
			Commands->Destroy_Object (obj);
		}
	}
};


DECLARE_SCRIPT (M02_Nod_Jet_Waypath, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Enable_Engine (obj, true);
		Commands->Disable_Physical_Collisions (obj);
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(0,0,0), 4.0f, 5.0f);
		params.WaypathID = 403389;
		params.WaypathSplined = true;
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 1)
		{
			Commands->Destroy_Object (obj);
		}
	}
};


DECLARE_SCRIPT (M02_GDI_Helicopter, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Enable_Engine (obj, true);
		Commands->Disable_Physical_Collisions (obj);
		ActionParamsStruct params;
		params.Set_Basic(this, 100, 1);
		params.Set_Movement(Vector3(-246.87f,17.42f,100.75f), 1.0f, 5.0f);
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete (GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 1)
		{
			Commands->Destroy_Object (obj);
		}
	}
};


DECLARE_SCRIPT (M02_Commando_Start, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Give_PowerUp (obj, "POW_MineRemote_Player", false);
		Commands->Give_PowerUp (obj, "POW_Chaingun_Player", false);
		Commands->Give_PowerUp (obj, "POW_SniperRifle_Player", false);
	}
};


DECLARE_SCRIPT (M02_Data_Disk, "Disk_ID:int")
{
	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_POWERUP_GRANTED) 
		{
			int id = Get_Int_Parameter ("Disk_ID");
			Vector3 reveal_loc = Vector3 (0,0,0);
			int radius = 15;
			switch (id)
			{
			case (1): // Disk Reveals the Ski Resort
				{
					reveal_loc = Vector3(425.76f,840.78f,8.19f);
					radius = 15;
					break;
				}
			case (2): // Disk Reveals the Dam Area
				{
					reveal_loc = Vector3(1016.43f,885.96f,35.41f);
					radius = 30;
					break;
				}
			default: // Disk Reveals the Hand of Nod
				{
					reveal_loc = Vector3(1204.46f,565.35f,37.84f);
					radius = 15;
					break;
				}
			}
			Commands->Clear_Map_Region_By_Pos (reveal_loc, radius);
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0103I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}
	}
};


DECLARE_SCRIPT (M02_Encyclopedia_Reveal, "Disk_ID:int")
{
	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_POWERUP_GRANTED) 
		{
			int id = Get_Int_Parameter ("Disk_ID");
			switch (id)
			{
			case (1):
				{
					Commands->Reveal_Encyclopedia_Vehicle (5);
					break;
				}
			default:
				{
					Commands->Reveal_Encyclopedia_Building (15);
					break;
				}
			}
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0104I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);
		}
	}
};
