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
*     Mission10.cpp
*
* DESCRIPTION
*     Mission 10 scripts
*
* PROGRAMMER
*     Ryan Vervack, Dan Etter
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 69 $
*     $Modtime: 2/11/02 9:41a $
*     $Archive: /Commando/Code/Scripts/Mission10.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include "Mission10.h"

DECLARE_SCRIPT(M10_Objective_Controller, "")
{
	int silo_count, turret_count;
	int e_sam_count, w_sam_count;
	int primary_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(silo_count, 1);
		SAVE_VARIABLE(turret_count, 2);
		SAVE_VARIABLE(e_sam_count, 3);
		SAVE_VARIABLE(w_sam_count, 4);
		SAVE_VARIABLE(primary_count, 5);
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Num_Tertiary_Objectives ( 2 );

		Commands->Start_Timer (obj, this, 0.5f, HAVOCS_SCRIPT);

		silo_count = turret_count = 0;
		e_sam_count = w_sam_count = 0;
		primary_count = 0;

		Commands->Add_Objective(1020, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M10_01, NULL, IDS_Enc_Obj_Hidden_M10_01);
		Commands->Add_Objective(1021, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M10_02, NULL, IDS_Enc_Obj_Hidden_M10_02);
	}

	void Remove_Pog(int id)
	{
		switch (id)
		{
		// Destroy the Power Plant
		case 1001: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_05.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Construction Yard
		case 1002: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_02.tga", IDS_POG_DESTROY);
			}
			break;
		// Place the Ion Cannon Beacon at the Temple of Nod Entrance
		case 1003: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_07.tga", IDS_POG_SABOTAGE);
			}
			break;
		// Destroy the Communications Center
		case 1004: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_04.tga", IDS_POG_DESTROY);
			}
			break;
		// Lower the Southeastern Base Gate
		case 1005: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_01.tga", IDS_POG_OPEN);
			}
			break;
		// Lower the Northeastern Base Gate
		case 1006: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_01.tga", IDS_POG_OPEN);
			}
			break;
		// Lower the Northwestern Base Gate
		case 1007: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_1_01.tga", IDS_POG_OPEN);
			}
			break;
		// Destroy the Hand of Nod
		case 1008: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_01.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Tiberium Refinery
		case 1009: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_02.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Nod Airstrip
		case 1010: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_06.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Tiberium Silos
		case 1011: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_02.tga", IDS_POG_DESTROY);
			}
			break;
		// Acquire the Level 1 Key From an Officer
		case 1012: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_1_02.tga", IDS_POG_ACQUIRE);
			}
			break;
		// Destroy the Northern Nod Helipad
		case 1013: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_02.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Western Nod Helipad
		case 1014: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_07.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Southern SAM Site
		case 1015: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_03.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Northern SAM Site
		case 1016: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_11.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Western SAM Site
		case 1017: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_06.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Eastern SAM Site
		case 1018: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_05.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy the Nod Turrets
		case 1019: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M10_2_12.tga", IDS_POG_DESTROY);
			}
			break;
		}
	}
	
	void Add_An_Objective(int id)
	{
		GameObject *object;
		GameObject *object2;
		GameObject *obj = Commands->Find_Object (1100154);

		switch (id)
		{
		case 1001: 
			{
				Commands->Add_Objective(1001, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_05, NULL, IDS_Enc_Obj_Primary_M10_05);
				object = Commands->Find_Object(2007444);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1001, object);
					Commands->Set_Objective_HUD_Info_Position( 1001, 75, "POG_M10_1_05.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		case 1002:
			{
				Commands->Add_Objective(1002, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_02, NULL, IDS_Enc_Obj_Primary_M10_02);
				object = Commands->Find_Object(2007445);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1002, object);
					Commands->Set_Objective_HUD_Info_Position( 1002, 78, "POG_M10_1_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		case 1003:
			{
				Commands->Add_Objective(1003, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_07, NULL, IDS_Enc_Obj_Primary_M10_07);
				object = Commands->Find_Object(2007446);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1003, object);
					Commands->Set_Objective_HUD_Info_Position( 1003, 73, "POG_M10_1_07.tga", IDS_POG_SABOTAGE, Commands->Get_Position (object));
				}
			}
			break;
		case 1004:
			{
				Commands->Add_Objective(1004, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_04, NULL, IDS_Enc_Obj_Primary_M10_04);
				object = Commands->Find_Object(2007447);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1004, object);
					Commands->Set_Objective_HUD_Info_Position( 1004, 76, "POG_M10_1_04.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		case 1005:
			{
				Commands->Add_Objective(1005, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_01, NULL, IDS_Enc_Obj_Primary_M10_01);
				object = Commands->Find_Object(1100166);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1005, object);
					Commands->Set_Objective_HUD_Info_Position( 1005, 80, "POG_M10_1_01.tga", IDS_POG_OPEN, Commands->Get_Position (object));
				}
			}
			break;
		case 1006:
			{
				Commands->Add_Objective(1006, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_06, NULL, IDS_Enc_Obj_Primary_M10_06);
				object = Commands->Find_Object(2017706);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1006, object);
					Commands->Set_Objective_HUD_Info_Position( 1006, 74, "POG_M10_1_01.tga", IDS_POG_OPEN, Commands->Get_Position (object));
				}
			}
			break;
		case 1007:
			{
				Commands->Add_Objective(1007, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_03, NULL, IDS_Enc_Obj_Primary_M10_03);
				object = Commands->Find_Object(1100169);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1007, object);
					Commands->Set_Objective_HUD_Info_Position( 1007, 77, "POG_M10_1_01.tga", IDS_POG_OPEN, Commands->Get_Position (object));
				}
			}
			break;
		case 1008:
			{
				Commands->Add_Objective(1008, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_01, NULL, IDS_Enc_Obj_Secondary_M10_01);
				object = Commands->Find_Object(2007448);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1008, object);
					Commands->Set_Objective_HUD_Info_Position( 1008, 51, "POG_M10_2_01.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1008, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1009:
			{
				Commands->Add_Objective(1009, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_04, NULL, IDS_Enc_Obj_Secondary_M10_04);
				object = Commands->Find_Object(2007449);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1009, object);
					Commands->Set_Objective_HUD_Info_Position( 1009, 50, "POG_M10_2_03.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1009, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1010:
			{
				Commands->Add_Objective(1010, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_06, NULL, IDS_Enc_Obj_Secondary_M10_06);
				object = Commands->Find_Object(2007450);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1010, object);
					Commands->Set_Objective_HUD_Info_Position( 1010, 59, "POG_M10_2_05.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1010, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1011:
			{
				Commands->Add_Objective(1011, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_08, NULL, IDS_Enc_Obj_Secondary_M10_08);
				object = Commands->Find_Object(2005260);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1011, object);
					Commands->Set_Objective_HUD_Info_Position( 1011, 58, "POG_M10_2_07.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1011, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1012:
			{
				Commands->Add_Objective(1012, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M10_08, NULL, IDS_Enc_Obj_Primary_M10_08);
				
				object = Commands->Find_Object(2000890);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1012, object);
					Commands->Set_Objective_HUD_Info_Position( 1012, 79, "POG_M10_1_08.tga", IDS_POG_ACQUIRE, Commands->Get_Position (object));
				}
				
			}
			break;
		case 1013:
			{
				Commands->Add_Objective(1013, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_10, NULL, IDS_Enc_Obj_Secondary_M10_10);
				object = Commands->Find_Object(1154083);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1013, object);
					Commands->Set_Objective_HUD_Info_Position( 1013, 57, "POG_M10_2_06.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1013, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1014:
			{
				Commands->Add_Objective(1014, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_07, NULL, IDS_Enc_Obj_Secondary_M10_07);
				object = Commands->Find_Object(1154084);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1014, object);
					Commands->Set_Objective_HUD_Info_Position( 1014, 56, "POG_M10_2_06.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1014, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1015:
			{
				Commands->Add_Objective(1015, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_03, NULL, IDS_Enc_Obj_Secondary_M10_03);
				object = Commands->Find_Object(1100007);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1015, object);
					Commands->Set_Objective_HUD_Info_Position( 1015, 55, "POG_M10_2_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1015, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1016:
			{
				Commands->Add_Objective(1016, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_11, NULL, IDS_Enc_Obj_Secondary_M10_11);
				object = Commands->Find_Object(1100010);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1016, object);
					Commands->Set_Objective_HUD_Info_Position( 1016, 54, "POG_M10_2_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1016, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1017:
			{
				Commands->Add_Objective(1017, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_09, NULL, IDS_Enc_Obj_Secondary_M10_09);
				object = Commands->Find_Object(1100012);
				object2 = Commands->Find_Object(1100011);
				if(object && object2)
				{
					Commands->Set_Objective_Radar_Blip_Object(1017, object);
					//Commands->Set_Objective_Radar_Blip_Object(1017, object2);
					Commands->Set_Objective_HUD_Info_Position( 1017, 53, "POG_M10_2_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1017, SECONDARY_POG_DELAY );
				}
			}
			break;
		case 1018:
			{
				Commands->Add_Objective(1018, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_05, NULL, IDS_Enc_Obj_Secondary_M10_05);
				object = Commands->Find_Object(1100008);
				object2 = Commands->Find_Object(1100009);
				if(object || object2)
				{
					Commands->Set_Objective_Radar_Blip_Object(1018, Commands->Find_Object (1209367));
					//Commands->Set_Objective_Radar_Blip_Object(1018, object2);
					Commands->Set_Objective_HUD_Info_Position( 1018, 52, "POG_M10_2_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		case 1019:
			{
				Commands->Add_Objective(1019, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_12, NULL, IDS_Enc_Obj_Secondary_M10_12);
				object = Commands->Find_Object(1100013);
				object2 = Commands->Find_Object(1100014);
				if(object || object2)
				{
					Commands->Set_Objective_Radar_Blip_Object(1019, object);
					//Commands->Set_Objective_Radar_Blip_Object(1019, object2);
					Commands->Set_Objective_HUD_Info_Position( 1019, 51, "POG_M10_2_11.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		}
	}

	void Action_Complete (GameObject * /* obj */, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == 100045)
		{
			Commands->Set_Objective_Status(1017, OBJECTIVE_STATUS_ACCOMPLISHED);
		}

		if (action_id == 100052)
		{
			Commands->Add_Objective(1019, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_12, NULL, IDS_Enc_Obj_Secondary_M10_12);
			Commands->Set_Objective_Status(1019, OBJECTIVE_STATUS_ACCOMPLISHED);
		}		
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		/*if (type == REMOVE_SECONDARY_POG)
		{
			Remove_Pog(param);
		}*/

		if (type >= 1000 && type <= 1025)
		{

			switch (param)
			{
				case 1: if (type == 1011)
						  {						  
								  Commands->Set_Objective_Status(1011, OBJECTIVE_STATUS_ACCOMPLISHED);
						  }
					else if (type == 1019)
						  {
							  if (++turret_count == 2)
							  {
								  Commands->Add_Objective(1019, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_12, NULL, IDS_Enc_Obj_Secondary_M10_12);
								  Commands->Set_Objective_Status(1019, OBJECTIVE_STATUS_ACCOMPLISHED);

								  int id = Commands->Create_Conversation("M10CON052", 99, 2000, false);
								  Commands->Join_Conversation(NULL, id, true, true, true);
								  Commands->Join_Conversation(STAR, id, true, true, true);
								  Commands->Start_Conversation(id, 100052);
								  Commands->Monitor_Conversation(obj, id);
							  }
						  }
					else if (type == 1015)
					{
						int id = Commands->Create_Conversation("M10CON028", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100028);
						Commands->Monitor_Conversation(obj, id);
					}

					else if (type == 1018)
					{
						if (++e_sam_count >= 1)
						{
							if (e_sam_count == 1)
							{
								int id = Commands->Create_Conversation("M10CON035", 99, 2000, false);
								Commands->Join_Conversation(NULL, id, true, true, true);
								Commands->Join_Conversation(STAR, id, true, true, true);
								Commands->Start_Conversation(id, 100035);
								Commands->Monitor_Conversation(obj, id);
							}

							if (e_sam_count == 2)
							{
								Commands->Add_Objective(1018, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M10_05, NULL, IDS_Enc_Obj_Secondary_M10_05);
								Commands->Set_Objective_Status(1018, OBJECTIVE_STATUS_ACCOMPLISHED);

								int id = Commands->Create_Conversation("M10CON034", 99, 2000, false);
								Commands->Join_Conversation(STAR, id, true, true, true);
								Commands->Start_Conversation(id, 100034);
								Commands->Monitor_Conversation(obj, id);
							}
						}
					}
					else if (type == 1017)
					{
						if (++w_sam_count == 2)
						{
							int id = Commands->Create_Conversation("M10CON045", 99, 2000, false);
							Commands->Join_Conversation(NULL, id, true, true, true);
							Commands->Join_Conversation(STAR, id, true, true, true);
							Commands->Start_Conversation(id, 100045);
							Commands->Monitor_Conversation(obj, id);

							Vector3 drop_loc = Commands->Get_Position (Commands->Find_Object (2005997));
							float facing = Commands->Get_Facing (Commands->Find_Object (2005997));
							GameObject * chinook_obj1 = Commands->Create_Object("Invisible_Object", drop_loc);

							if (chinook_obj1)
							{
								Commands->Set_Facing(chinook_obj1, facing);
								Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M10_GDI_Drop_HummVee.txt");
							}
						}
					}
					else
					{
						Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_ACCOMPLISHED);
						if (type < 1008 || type == 1012)
						{
							if (++primary_count >= 7)
							{
								if (primary_count == 7)
								{
									int id = Commands->Create_Conversation("M10CON019", 99, 2000, false);
									Commands->Join_Conversation(STAR, id, true, true, true);
									Commands->Join_Conversation(NULL, id, true, true, true);
									Commands->Start_Conversation(id, 100019);
									Commands->Monitor_Conversation(obj, id);

									GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(146.59f, 67.52f, -5.25f));
									Commands->Set_Facing(chinook_obj, 0.0f);
									Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X10I_GDI_Drop_PowerUp.txt");
								}

								if (type == 1003)
								{
									Commands->Start_Timer(obj, this, 5.0f, MISSION_COMPLETE);
								}
							}
							
						}
					}
					break;
				case 2: Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_FAILED);
						break;
				case 3: Add_An_Objective(type);
					break;
				case 4: Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_PENDING);
					break;
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == HAVOCS_SCRIPT)
		{
			GameObject * obj_con = Commands->Find_Object(1100154);
			if (obj_con)
			{
				Commands->Start_Timer(obj, this, 3.0f, 1);
				Commands->Send_Custom_Event(obj, obj_con, 1003, 3, 0.0f);
				/*Commands->Send_Custom_Event(obj, obj_con, 1009, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 1008, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 1015, 3, 0.0f);				
				Commands->Send_Custom_Event(obj, obj_con, 1012, 3, 0.0f);*/
			}

			Commands->Attach_Script(STAR, "M10_Havoc_Script", "");
			
		}

		if(timer_id == 1)
		{
			int id = Commands->Create_Conversation("M10CON064", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100064);
			Commands->Monitor_Conversation(obj, id);
		}


		if(timer_id == MISSION_COMPLETE)
		{
			Commands->Mission_Complete(true);
		}
	}
};

DECLARE_SCRIPT (M10_Ion_Cannon, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			GameObject * obj_con = Commands->Find_Object(1100154);
			Commands->Send_Custom_Event(obj, obj_con, 1003, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT (M10_Havoc_Script, "")
{
	void Created (GameObject * /* obj */) override
	{
		/*Commands->Give_PowerUp(obj, "POW_LaserChaingun_Player");
		Commands->Give_PowerUp(obj, "POW_LaserRifle_Player");
		Commands->Give_PowerUp(obj, "POW_MineRemote_Player");
		Commands->Give_PowerUp(obj, "POW_SniperRifle_Player");
		Commands->Give_PowerUp(obj, "POW_TiberiumAutoRifle_Player");
		Commands->Give_PowerUp(obj, "POW_GrenadeLauncher_Player");
		Commands->Give_PowerUp(obj, "POW_FlameThrower_Player");
		Commands->Give_PowerUp(obj, "POW_ChemSprayer_Player");
		Commands->Give_PowerUp(obj, "POW_LaserChaingun_Player");
		Commands->Give_PowerUp(obj, "POW_LaserRifle_Player");
		Commands->Give_PowerUp(obj, "POW_MineRemote_Player");
		Commands->Give_PowerUp(obj, "POW_RocketLauncher_Player");
		Commands->Give_PowerUp(obj, "POW_RocketLauncher_Player");
		Commands->Give_PowerUp(obj, "POW_TiberiumAutoRifle_Player");
		Commands->Give_PowerUp(obj, "POW_PersonalIonCannon_Player");
		Commands->Give_PowerUp(obj, "POW_FlameThrower_Player");
		Commands->Give_PowerUp(obj, "POW_ChemSprayer_Player");*/

	}
	
	/*void Destroyed(GameObject * obj) override 
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(2000071), 666, 666, 0.0f);	
	}*/
};

DECLARE_SCRIPT(M10_Turret_Tank, "CheckBlocked=1:int")
{
	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen(obj, true);
	}

	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		bool blocked;

		if (Get_Int_Parameter ("CheckBlocked") != 0)
		{
			blocked = true;
		}

		else
		{
			blocked = false;
		}

		ActionParamsStruct params;
		params.Set_Basic(this, 90, 0);
		params.Set_Attack(enemy, 100.0f, 4.0f, true);
		params.AttackCheckBlocked = blocked;
		Commands->Action_Attack(obj, params);

		Commands->Start_Timer(obj, this, 10.0f, 0);
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		Commands->Action_Reset(obj, 99);
	}
};

DECLARE_SCRIPT(M10_Power_Plant, "")
{
	bool con_yard_destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(con_yard_destroyed, 1);
	}

	void Created(GameObject * /* obj */) override
	{
		con_yard_destroyed = false;
		Commands->Static_Anim_Phys_Goto_Frame ( 1285077, 0, "L10_LASERFENCE1.L10_LASERFENCE1" );
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 1000 && param == 1000)
		{
			con_yard_destroyed = true;
		}
	}

	void Power_Off(void)
	{
		//Turns power off on all buildings (besides Obelisk, which is done elsewhere)
		GameObject * building;
		
		building = Commands->Find_Object(1153933); //Con Yard
		Commands->Set_Building_Power(building, false);
		building = Commands->Find_Object(1153932); //Comm Center
		Commands->Set_Building_Power(building, false);
		building = Commands->Find_Object(1153934); //Airstrip
		Commands->Set_Building_Power(building, false);
		building = Commands->Find_Object(1153940); //Refinery
		Commands->Set_Building_Power(building, false);
		building = Commands->Find_Object(1153939); //Hand of Nod
		Commands->Set_Building_Power(building, false);
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100014)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1001, 3, 0.0f);
			Commands->Send_Custom_Event(obj, obj_con, 1001, 1, 0.0f);
		}
	}
	
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Enable_Radar ( true );

		Commands->Destroy_Object (Commands->Find_Object (1203589));

		int id = Commands->Create_Conversation("M10CON014", 99, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Start_Conversation(id, 100014);
		Commands->Monitor_Conversation(obj, id);

		GameObject * obelisk = Commands->Find_Object(1153938);
		if (obelisk)
		{
			Commands->Send_Custom_Event(obj, obelisk, 1000, 1000, 0.0f);
		}

		Commands->Static_Anim_Phys_Goto_Frame ( 1285077, 1, "L10_LASERFENCE1.L10_LASERFENCE1" );
		Power_Off();
	}
};

DECLARE_SCRIPT(M10_Con_Yard, "")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100005)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1002, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Destroy_Object (Commands->Find_Object (2014793));
		Commands->Destroy_Object (Commands->Find_Object (2014792));

		int id = Commands->Create_Conversation("M10CON005", 99, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Start_Conversation(id, 100005);
		Commands->Monitor_Conversation(obj, id);

		GameObject * power_plant = Commands->Find_Object(1153931);
		if (power_plant)
		{
			Commands->Send_Custom_Event(obj, power_plant, 3000, 3000, 0.0f);
		}
		GameObject * turret1 = Commands->Find_Object(1100013);
		if (turret1)
		{
			Commands->Send_Custom_Event(obj, turret1, 3000, 3000, 0.0f);
		}
		GameObject * comm_center = Commands->Find_Object(1153932);
		if (comm_center)
		{
			Commands->Send_Custom_Event(obj, comm_center, 3000, 3000, 0.0f);
		}
		GameObject * heli_pad = Commands->Find_Object(1154083);
		if (heli_pad)
		{
			Commands->Send_Custom_Event(obj, heli_pad, 3000, 3000, 0.0f);
		}
		GameObject * sam_site = Commands->Find_Object(1100010);
		if (sam_site)
		{
			Commands->Send_Custom_Event(obj, sam_site, 3000, 3000, 0.0f);
		}
		GameObject * turret2 = Commands->Find_Object(1100014);
		if (turret2)
		{
			Commands->Send_Custom_Event(obj, turret2, 3000, 3000, 0.0f);
		}

		GameObject * hon = Commands->Find_Object(1153939);
		if (hon)
		{
			Commands->Send_Custom_Event(obj, hon, 3000, 3000, 0.0f);
		}
		GameObject * sam1 = Commands->Find_Object(1100007);
		if (sam1)
		{
			Commands->Send_Custom_Event(obj, sam1, 3000, 3000, 0.0f);
		}
		GameObject * turret3 = Commands->Find_Object(2000728);
		if (turret3)
		{
			Commands->Send_Custom_Event(obj, turret3, 3000, 3000, 0.0f);
		}
		GameObject * turret4 = Commands->Find_Object(2000729);
		if (turret4)
		{
			Commands->Send_Custom_Event(obj, turret4, 3000, 3000, 0.0f);
		}
		GameObject * refinery = Commands->Find_Object(1153940);
		if (refinery)
		{
			Commands->Send_Custom_Event(obj, refinery, 3000, 3000, 0.0f);
		}
		GameObject * turret5 = Commands->Find_Object(1205749);
		if (turret5)
		{
			Commands->Send_Custom_Event(obj, turret5, 3000, 3000, 0.0f);
		}
		GameObject * turret6 = Commands->Find_Object(1205748);
		if (turret6)
		{
			Commands->Send_Custom_Event(obj, turret6, 3000, 3000, 0.0f);
		}
		GameObject * obelisk = Commands->Find_Object(1153938);
		if (obelisk)
		{
			Commands->Send_Custom_Event(obj, obelisk, 3000, 3000, 0.0f);
		}
		GameObject * sam2 = Commands->Find_Object(1100008);
		if (sam2)
		{
			Commands->Send_Custom_Event(obj, sam2, 3000, 3000, 0.0f);
		}
		GameObject * sam3 = Commands->Find_Object(1100009);
		if (sam3)
		{
			Commands->Send_Custom_Event(obj, sam3, 3000, 3000, 0.0f);
		}
		GameObject * airstrip = Commands->Find_Object(1154061);
		if (airstrip)
		{
			Commands->Send_Custom_Event(obj, airstrip, 3000, 3000, 0.0f);
		}
		GameObject * helipad1 = Commands->Find_Object(1154084);
		if (helipad1)
		{
			Commands->Send_Custom_Event(obj, helipad1, 3000, 3000, 0.0f);
		}
		GameObject * sam4 = Commands->Find_Object(1100012);
		if (sam4)
		{
			Commands->Send_Custom_Event(obj, sam4, 3000, 3000, 0.0f);
		}
		GameObject * sam5 = Commands->Find_Object(1100011);
		if (sam5)
		{
			Commands->Send_Custom_Event(obj, sam5, 3000, 3000, 0.0f);
		}
		GameObject * silo1 = Commands->Find_Object(1153935);
		if (silo1)
		{
			Commands->Send_Custom_Event(obj, silo1, 3000, 3000, 0.0f);
		}
		GameObject * silo2 = Commands->Find_Object(1154051);
		if (silo2)
		{
			Commands->Send_Custom_Event(obj, silo2, 3000, 3000, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M10_Comm_Center, "")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100011)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1004, 3, 0.0f);
			Commands->Send_Custom_Event(obj, obj_con, 1004, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Destroy_Object (Commands->Find_Object (2014802));
		Commands->Enable_Radar (true);

		int id = Commands->Create_Conversation("M10CON011", 99, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Start_Conversation(id, 100011);
		Commands->Monitor_Conversation(obj, id);

		//Commands->Give_PowerUp(STAR, "Ion Cannon Powerup");
	}
};

DECLARE_SCRIPT(M10_Hand_Of_Nod, "")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100021)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1008, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		int id = Commands->Create_Conversation("M10CON021", 99, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(STAR, id, true, true, true);
		Commands->Start_Conversation(id, 100021);
		Commands->Monitor_Conversation(obj, id);
	}
};

DECLARE_SCRIPT(M10_Refinery, "")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100031)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1009, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		int id = Commands->Create_Conversation("M10CON031", 99, 2000, false);
		Commands->Join_Conversation(STAR, id, true, true, true);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Start_Conversation(id, 100031);
		Commands->Monitor_Conversation(obj, id);
	}
};

DECLARE_SCRIPT(M10_Airstrip, "")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100037)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1010, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100161), NO_DROP, 0, 0.0f);

		int id = Commands->Create_Conversation("M10CON037", 99, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Start_Conversation(id, 100037);
		Commands->Monitor_Conversation(obj, id);
	}
};

DECLARE_SCRIPT(M10_Silo, "")
{
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * silo_con = Commands->Find_Object(2005260);
		if (silo_con)
		{
			Commands->Send_Custom_Event(obj, silo_con, KILLED, 0, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M10_Silo_Controller, "")
{
	int count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(count, 1);		
	}

	void Created (GameObject * /* obj */) override
	{
		count = 0;
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == 100043)
		{
			GameObject * obj_con = Commands->Find_Object(1100154);

			//Commands->Set_Objective_Status(1011, OBJECTIVE_STATUS_ACCOMPLISHED);
			Commands->Send_Custom_Event(obj, obj_con, 1011, 3, 0.0f);
			Commands->Send_Custom_Event(obj, obj_con, 1011, 1, 0.0f);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == KILLED)
		{
			count++;

			if (count >= 2)
			{
				int id = Commands->Create_Conversation("M10CON043", 99, 2000, false);
				Commands->Join_Conversation(STAR, id, true, true, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100043);
				Commands->Monitor_Conversation(obj, id);
			}
		}
	}
};



DECLARE_SCRIPT(M10_Obelisk, "")
{
	bool objective_completed, inside_obelisk, conyard_destroyed;
	float full_health, curr_health;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(objective_completed, 1);
		SAVE_VARIABLE(inside_obelisk, 2);
		SAVE_VARIABLE(conyard_destroyed, 3);
	}

	void Created(GameObject * obj) override
	{
		objective_completed = inside_obelisk = conyard_destroyed = false;
		full_health = Commands->Get_Health(obj);
	}
	
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		if (!objective_completed)
		{
			GameObject * obj_con = Commands->Find_Object(1100154);
			if (obj_con)
			{
				Commands->Send_Custom_Event(obj, obj_con, 1020, 1, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object (2010415), KILLED, 0, 0.0f);
				Commands->Send_Custom_Event(obj, Commands->Find_Object (6000728), KILLED, 0, 0.0f);
			}
		}
	}

	/*void Damaged(GameObject * obj, GameObject * damager, float amount) override
	{
		if (inside_obelisk || conyard_destroyed)
		{
			return;
		}
		else
		{
			curr_health = Commands->Get_Health (obj);
			float counter = 1.0f;
			Commands->Set_Health(obj, (curr_health + 1));

			for (float x = curr_health; x <= full_health; x++)
			{
				Commands->Start_Timer(obj, this, counter, REBUILD);
				counter++;
			
				//Commands->Set_Health(obj, full_health);
			}
		}
	}*/

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if (timer_id == REBUILD)
		{
			curr_health++;
			Commands->Set_Health(obj, curr_health);
		}
	}


	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 1000 && param == 1000)
		{
			Commands->Set_Building_Power(obj, false);
			objective_completed = false;
		}
		if (type == 2000 && param == 2000)
		{
			inside_obelisk = true;
		}
		if (type == 2000 && param == 2001)
		{
			inside_obelisk = false;
			full_health = Commands->Get_Health(obj);
		}
		if (type == 3000 && param == 3000)
		{
			conyard_destroyed = true;
		}
	}
};

DECLARE_SCRIPT(M10_Turret, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen(obj, true);
	}

	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 90, 0);
		params.Set_Attack(enemy, 100.0f, 4.0f, true);
		Commands->Action_Attack(obj, params);

		Commands->Start_Timer(obj, this, 10.0f, 0);
	}
	
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Vector3 my_pos = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		GameObject * destroyed_turret = Commands->Create_Object("Nod_Turret_Destroyed", my_pos);
		Commands->Set_Facing(destroyed_turret, facing);
		Commands->Attach_Script(destroyed_turret, "M10_Destroyed_Turret", "");
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		Commands->Action_Reset(obj, 99);
	}
};

DECLARE_SCRIPT(M10_Destroyed_Turret, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
	}

	void Timer_Expired(GameObject * /* obj */, int /* timer_id */) override
	{
			Commands->Create_2D_Sound("EVA_Enemy_Structure_Destroyed");
	}
};

DECLARE_SCRIPT(M10_Destroy_Self, "")
{
	void Custom(GameObject * obj, int /* type */, intptr_t /* param */, GameObject * /* sender */) override
	{
		Commands->Destroy_Object(obj);
	}
};

DECLARE_SCRIPT(M10_Apache_Controller, "")
{
	bool active;
	int area;
	bool destroyed[3];
	int apache_id[3];
	int attacking_apache;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active, 1);
		SAVE_VARIABLE(area, 2);
		SAVE_VARIABLE(destroyed, 3);
		SAVE_VARIABLE(apache_id, 4);
		SAVE_VARIABLE(attacking_apache, 5);
	}
	
	void Created(GameObject * /* obj */) override
	{
		active = false;
		attacking_apache = 0;
		area = -1;
		destroyed[0] = destroyed[1] = destroyed[2] = false;
		//apache_id[0] = Commands->Get_ID(Commands->Create_Object("Nod_Apache_No_Idle", Vector3(-12.129f, -149.546f, 2.949f)));
		//GameObject * apache = Commands->Find_Object(apache_id[0]);
		//Commands->Attach_Script(apache, "M10_Apache", "0");
		apache_id[1] = Commands->Get_ID(Commands->Create_Object("Nod_Apache_No_Idle", Vector3(-171.980f, 51.905f, 8.959f)));
		GameObject * apache = Commands->Find_Object(apache_id[1]);
		Commands->Attach_Script(apache, "M10_Apache", "1");
		apache_id[2] = Commands->Get_ID(Commands->Create_Object("Nod_Apache_No_Idle", Vector3(8.548f, 226.140f, 1.697f)));
		apache = Commands->Find_Object(apache_id[2]);
		Commands->Attach_Script(apache, "M10_Apache", "2");
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (param < -1 || param > 2)
		{
			return;
		}
		
		if (type == 1000)
		{
			destroyed[param] = true;
			if (Commands->Find_Object(apache_id[param]))
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(apache_id[param]), 300, 300, 0.0f);
			}
		}
		if (type == 2000)
		{
			apache_id[param] = 0;
			if (!destroyed[param])
			{
				Commands->Start_Timer(obj, this, 40.0f, param);
			}
			else
			{
			}
		}
		if (type == 3000)
		{
			if (param == area)
			{
				return;
			}
			if (area > -1)
			{
				Return_To_Helipad(area);
			}
			area = param;
			if (area > -1)
			{
				Attack_Player(area);
			}
		}
		if (type == 4000)
		{
			if (area == param)
			{
				Commands->Send_Custom_Event(obj, sender, 100, 100, 0.0f);
				attacking_apache = area;
			}
		}
		if (type == 5000)
		{
			Reload_At_Helipad(area);
			Commands->Start_Timer(obj, this, 25.0f, 10 + area);
		}
	}

	void Return_To_Helipad(int current_area)
	{
		GameObject * apache = Commands->Find_Object(apache_id[current_area]);
		if (apache)
		{
			Commands->Send_Custom_Event(Owner(), apache, 200, 200, 0.0f);
		}
	}

	void Reload_At_Helipad(int current_area)
	{
		GameObject * apache = Commands->Find_Object(apache_id[current_area]);
		if (apache)
		{
			Commands->Send_Custom_Event(Owner(), apache, 500, 500, 0.0f);
		}
	}

	void Attack_Player(int current_area)
	{
		GameObject * apache = Commands->Find_Object(apache_id[current_area]);
		if (apache)
		{
			Commands->Send_Custom_Event(Owner(), apache, 100, 100, 0.0f);
			attacking_apache = current_area;
		}
	}

	void Replace_Apache(int current_area)
	{
		Vector3 start_loc;
		start_loc.Set(8.107f, 70.031f, 44.679f);

		GameObject * apache = Commands->Create_Object("Nod_Apache_No_Idle", start_loc);
		Commands->Enable_Engine(apache, true);
		char param[12];
		sprintf(param, "%d", current_area);
		Commands->Attach_Script(apache, "M10_Apache", param);
		Commands->Send_Custom_Event(Owner(), apache, 400, 400, 0.0f);
		apache_id[current_area] = Commands->Get_ID(apache);
	}
	
	void Timer_Expired(GameObject * /* obj */, int timer_id) override
	{
		if (timer_id >= 10)
		{
			int current_area = timer_id - 10;
			if (current_area == area)
			{
				GameObject * apache = Commands->Find_Object(apache_id[area]);
				if (apache)
				{
					Commands->Send_Custom_Event(Owner(), apache, 100, 100, 0.0f);
					attacking_apache = area;
				}
			}
			return;
		}

		if (!destroyed[timer_id])
		{
			Replace_Apache(timer_id);
		}
		else
		{
		}
	}
};

DECLARE_SCRIPT(M10_Apache, "Area:int")
{
	bool on_pad, pad_destroyed;
	int my_area;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(on_pad, 1);
		SAVE_VARIABLE(my_area, 2);
		SAVE_VARIABLE(pad_destroyed, 3);
	}
	
	void Created(GameObject * obj) override
	{
		//Commands->Enable_Vehicle_Transitions ( obj, false );
		Commands->Enable_Hibernation(obj, false);
		
		on_pad = true;
		pad_destroyed = false;

		my_area = Get_Int_Parameter("Area");
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 100 && param == 100)
		{
			on_pad = false;

			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			Vector3 pos = Commands->Get_Position(obj);
			pos.Z += 10.0f;
			params.Set_Movement(pos, 1.0f, 1.0f);
			params.MovePathfind = false;
			Commands->Action_Goto(obj, params);
		}
		if (type == 200 && param == 200)
		{
			int pad_obj[3] =
			{
				1110050,
				1110051,
				1110052
			};
			
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 2);
			int area = Get_Int_Parameter("Area");
			params.Set_Movement(Commands->Find_Object(pad_obj[area]), 1.0f, 0.1f);
			params.MovePathfind = false;
			Commands->Action_Goto(obj, params);

		}
		if (type == 300 && param == 300)
		{
			pad_destroyed = true;
			if (on_pad)
			{
				Commands->Apply_Damage(obj, 10000.0f, "EXPLOSIVE", NULL);
			}
			else 
			{
				int pad_obj[3] =
				{
					1110050,
					1110051,
					1110052
				};
				
				ActionParamsStruct params;
				params.Set_Basic(this, 90, 2);
				int area = Get_Int_Parameter("Area");
				params.Set_Movement(Commands->Find_Object(pad_obj[area]), 1.0f, 0.1f);
				params.MovePathfind = false;
				Commands->Action_Goto(obj, params);
			}
		}
		if (type == 400 && param == 400)
		{
			int waypath[3] = 
			{
				1110082,
				1110040,
				1110045
			};
			
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 2);
			int area = Get_Int_Parameter("Area");
			params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
			params.MovePathfind = false;
			params.WaypathID = waypath[area];
			Commands->Action_Goto(obj, params);

		}
		if (type == 500 && param == 500)
		{
			int pad_obj[3] =
			{
				1110050,
				1110051,
				1110052
			};
			
			ActionParamsStruct params;
			params.Set_Basic(this, 91, 3);
			int area = Get_Int_Parameter("Area");
			params.Set_Movement(Commands->Find_Object(pad_obj[area]), 1.0f, 0.1f);
			params.MovePathfind = false;
			Commands->Action_Goto(obj, params);

		}
	}
	
	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (!pad_destroyed)
		{
			if (timer_id == 0)
			{
				Vector3 pos = Commands->Get_Position(STAR);
				float facing = Commands->Get_Facing(STAR);
				pos.X -= cos(DEG_TO_RADF(facing)) * 6.0f;
				pos.Y -= sin(DEG_TO_RADF(facing)) * 6.0f;

				pos.Z = WWMath::Max(pos.Z + 12.0f, Commands->Get_Safe_Flight_Height(pos.X, pos.Y) + 6.0f);
				
				ActionParamsStruct params;
				params.Set_Basic(this, 90, 1);
				params.Set_Movement(pos, 1.0f, 5.0f);
				params.MovePathfind = false;
				Commands->Action_Goto(obj, params);
			}
			if (timer_id == 1)
			{
				if (!pad_destroyed && on_pad)
				{
					if (Commands->Get_Health(obj) < Commands->Get_Max_Health(obj))
					{
						Commands->Set_Health(obj, Commands->Get_Health(obj) + 5.0f);
					}
					Commands->Start_Timer(obj, this, 3.0f, 1);
				}
			}
			if (timer_id == 2)
			{
				Commands->Action_Reset(obj, 90);
				Commands->Start_Timer(obj, this, Commands->Get_Random(1.0f, 3.0f), 0);
			}
			if (timer_id == 3)
			{
				GameObject * controller = Commands->Find_Object(1110009);
				if (controller)
				{
					Commands->Send_Custom_Event(obj, controller, 5000, Get_Int_Parameter("Area"), 0.0f);
				}
			}
			if (timer_id == 4)
			{
				Commands->Enable_Engine(obj, false);
			}
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (!pad_destroyed)
		{
			if (reason != ACTION_COMPLETE_NORMAL)
			{
				return;
			}
			if (action_id == 0)
			{
				Commands->Start_Timer(obj, this, 2.0f, 0);
				Commands->Start_Timer(obj, this, 60.0f, 3);
			}
			if (action_id == 1)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 90, 2);
				params.Set_Attack(STAR, 150.0f, 2.0f, true);
				params.MovePathfind = false;
				Commands->Action_Attack(obj, params);
		
				Commands->Start_Timer(obj, this, Commands->Get_Random(4.0f, 6.0f), 2);
			}
			if (action_id == 2)
			{
				Commands->Start_Timer(obj, this, 1.0f, 4);

				on_pad = true;
				Commands->Start_Timer(obj, this, 5.0f, 1);

				GameObject * controller = Commands->Find_Object(1110009);
				if (controller)
				{
					Commands->Send_Custom_Event(obj, controller, 4000, Get_Int_Parameter("Area"), 0.0f);
				}
			}
			if (action_id == 3)
			{
				Commands->Start_Timer(obj, this, 1.0f, 4);

				on_pad = true;
				Commands->Start_Timer(obj, this, 5.0f, 1);
			}
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * controller = Commands->Find_Object(1110009);
		if (controller)
		{
			Commands->Send_Custom_Event(obj, controller, 2000, Get_Int_Parameter("Area"), 0.0f);
		}
	}
};

DECLARE_SCRIPT(M10_Reinforcement_Controller, "")
{
	int area_count[3];			//Number of units killed in area since last reinforcement
	int target_count[3];			//Number of units neccessary to be killed before reinforcements arrive
	int max_reinforcements[3]; //Max number of times an area can be reinforced
	int current_area;				//Current area the Commando is in

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(area_count, 1);
		SAVE_VARIABLE(current_area, 2);
		SAVE_VARIABLE(target_count, 3);
		SAVE_VARIABLE(max_reinforcements, 4);
	}

	void Created(GameObject * /* obj */) override
	{
		area_count[0] = area_count[1] = area_count[2] = 0;
		target_count[0] = target_count[1] = target_count[2] = 4 - DIFFICULTY;
		max_reinforcements[0] = max_reinforcements[1] = max_reinforcements[2] = 3;
		current_area = -1;
	}
	
	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 3000)
		{
			current_area = param;
			
			if (current_area > 2)
			{
				current_area = 2;
			}
			if (current_area < -1)
			{
				current_area = -1;
			}
		}
		if (type == 10000 && param == 10000 && current_area != -1)
		{
			if (++area_count[current_area] >= target_count[current_area])
			{
				area_count[current_area] = 0;
				Reinforce_Area();
			}
		}
		if (type == 20000 && param >= 0 && param <= 2 && param > -1)
		{
			target_count[param]++;
		}
	}

	void Reinforce_Area(void)
	{
		if (current_area != -1 && max_reinforcements[current_area]-- <= 0)
		{
			max_reinforcements[current_area] = 0;
			return;
		}
		
		typedef struct
		{
			Vector3 Pos;
			float Facing;
		} Reinforce_Info;
		
		Reinforce_Info reinforce[3][4];

		reinforce[0][0].Pos.Set(110.498f, -161.705f, 3.060f);
		reinforce[0][0].Facing = 140.0f;
		reinforce[0][1].Pos.Set(31.971f, -136.521f, 6.669f);
		reinforce[0][1].Facing = 165.0f;
		reinforce[0][2].Pos.Set(-11.678f, -140.165f, 5.467f);
		reinforce[0][2].Facing = -165.0f;
		reinforce[0][3].Pos.Set(11.674f, -195.396f, 0.926f);
		reinforce[0][3].Facing = -175.0f;
		reinforce[1][0].Pos.Set(-98.021f, -67.086f, 7.0f);
		reinforce[1][0].Facing = 130.0f;
		reinforce[1][1].Pos.Set(-96.862f, 19.633f, 7.0f);
		reinforce[1][1].Facing = -105.0f;
		reinforce[1][2].Pos.Set(-161.272f, -26.111f, 8.513f);
		reinforce[1][2].Facing = 15.0f;
		reinforce[1][3].Pos.Set(-184.338f, 60.513f, 8.852f);
		reinforce[1][3].Facing = -145.0f;
		reinforce[2][0].Pos.Set(16.075f, 172.231f, 2.541f);
		reinforce[2][0].Facing = 175.0f;
		reinforce[2][1].Pos.Set(56.842f, 182.570f, 1.399f);
		reinforce[2][1].Facing = 30.0f;
		reinforce[2][2].Pos.Set(24.164f, 230.841f, 0.704f);
		reinforce[2][2].Facing = 15.0f;
		reinforce[2][3].Pos.Set(23.470f, 158.990f, 6.251f);
		reinforce[2][3].Facing = 0.0f;

		if (Commands->Get_Random_Int(0, 2) == 1)
		{
			int num = Commands->Get_Random_Int(0, 4);
			Reinforce_By_Rope(reinforce[current_area][num].Pos, reinforce[current_area][num].Facing);
		}
		else
		{
			int num = Commands->Get_Random_Int(0, 4);
			Reinforce_By_Parachute(reinforce[current_area][num].Pos, reinforce[current_area][num].Facing);
		}
	}

	void Reinforce_By_Rope(const Vector3 & pos, float facing)
	{
		GameObject *new_obj = Commands->Create_Object("Invisible_Object", pos);
		Commands->Set_Facing(new_obj, facing);
		Commands->Attach_Script(new_obj, "Test_Cinematic", "X10I_TroopDrop.txt");
	}

	void Reinforce_By_Parachute(const Vector3 & pos, float facing)
	{
		GameObject * new_obj = Commands->Create_Object("Invisible_Object", pos);
		Commands->Set_Facing(new_obj, facing);
		Commands->Attach_Script(new_obj, "Test_Cinematic", "X10D_CHTroopdrop1.txt");
	}
};

DECLARE_SCRIPT(M10_Chinook_ParaDrop, "Preset:string")
{
	int chinook_id;
	bool dead;
	int out;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(chinook_id, 1);
		SAVE_VARIABLE(dead, 2);
		SAVE_VARIABLE(out, 3);
	}
	
	void Created(GameObject * obj) override
	{
		Vector3 loc = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		
		GameObject *chinook_rail = Commands->Create_Object("Generic_Cinematic", loc);
		Commands->Set_Model(chinook_rail, "X5D_Chinookfly");
		Commands->Set_Facing(chinook_rail, facing);
		Commands->Set_Animation(chinook_rail, "X5D_Chinookfly.X5D_Chinookfly", false, NULL, 0.0f, -1.0f, false);
		GameObject *chinook = Commands->Create_Object("Nod_Chinook", loc);
		Commands->Set_Facing(chinook, facing);
		Commands->Set_Animation(chinook, "v_nod_chinook.vf_nod_chinook", true, NULL, 0.0f, -1.0f, false);
		Commands->Attach_To_Object_Bone(chinook, chinook_rail, "BN_Chinook_1");

		dead = false;
		out = 0;
		char params[12];
		sprintf(params, "%d", Commands->Get_ID(obj));
		Commands->Attach_Script(chinook, "M10_Reinforcement_Chinook", params);

		chinook_id = Commands->Get_ID(chinook);
		
		// Destroy Chinook
		Commands->Start_Timer(obj, this, 280.0f/30.0f, 0);
		// Parachutes
		Commands->Start_Timer(obj, this, 169.0f/30.0f, 1);
		Commands->Start_Timer(obj, this, 179.0f/30.0f, 2);
		Commands->Start_Timer(obj, this, 198.0f/30.0f, 3);
		// Soldiers
		Commands->Start_Timer(obj, this, 145.0f/30.0f, 4);
		Commands->Start_Timer(obj, this, 155.0f/30.0f, 5);
		Commands->Start_Timer(obj, this, 165.0f/30.0f, 6);

		
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		Vector3 loc = Commands->Get_Position(obj);
		const char * preset = Get_Parameter("Preset");
		float facing = Commands->Get_Facing(obj);

		switch (timer_id)
		{
		case 0: 
			GameObject *chinook;
			chinook = Commands->Find_Object(chinook_id);
			Commands->Destroy_Object(chinook);
			break;
		case 1: 
			if (out >= 1)
			{
				GameObject *para1;
				para1 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para1, facing);
				Commands->Set_Model(para1, "X5D_Parachute");
				Commands->Set_Animation(para1, "X5D_Parachute.X5D_ParaC_1", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para1, "ROOTTRANSFORM");
				Commands->Attach_Script(para1, "M10_No_More_Parachute", "");
			}
			break;
		case 2: 
			if (out >= 2)
			{
				GameObject *para2;
				para2 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para2, facing);
				Commands->Set_Model(para2, "X5D_Parachute");
				Commands->Set_Animation(para2, "X5D_Parachute.X5D_ParaC_2", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para2, "ROOTTRANSFORM");
				Commands->Attach_Script(para2, "M10_No_More_Parachute", "");
			}
			break;
		case 3: 
			if (out == 3)
			{
				GameObject *para3;
				para3 = Commands->Create_Object("Generic_Cinematic", loc);
				Commands->Set_Facing(para3, facing);
				Commands->Set_Model(para3, "X5D_Parachute");
				Commands->Set_Animation(para3, "X5D_Parachute.X5D_ParaC_3", false, NULL, 0.0f, -1.0f, false);
				Commands->Create_3D_Sound_At_Bone("parachute_open", para3, "ROOTTRANSFORM");
				Commands->Attach_Script(para3, "M10_No_More_Parachute", "");
			}
			break;
		case 4: 
			if (!dead)
			{
			
			GameObject *box1 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box1, "X5D_Box01");
			Commands->Set_Facing(box1, facing);
			Commands->Set_Animation(box1, "X5D_Box01.X5D_Box01", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier1;
			soldier1 = Commands->Create_Object_At_Bone(box1, preset, "Box01");
			Commands->Set_Facing(soldier1, facing);
			Commands->Attach_Script(soldier1, "RMV_Trigger_Killed", "1110009, 10000, 10000");
			Commands->Attach_Script(soldier1, "M00_No_Falling_Damage_DME", "");
			Commands->Attach_To_Object_Bone( soldier1, box1, "Box01" );
			Commands->Set_Animation(soldier1, "s_a_human.H_A_X5D_ParaT_1", false, NULL, 0.0f, -1.0f, false);
			out++;
		
			}
			break;
		case 5: 
			if (!dead)
			{

			GameObject *box2 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box2, "X5D_Box02");
			Commands->Set_Facing(box2, facing);
			Commands->Set_Animation(box2, "X5D_Box02.X5D_Box02", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier2;
			soldier2 = Commands->Create_Object_At_Bone(box2, preset, "Box02");
			Commands->Set_Facing(soldier2, facing);
			Commands->Attach_Script(soldier2, "RMV_Trigger_Killed", "1110009, 10000, 10000");
			Commands->Attach_Script(soldier2, "M00_No_Falling_Damage_DME", "");
			Commands->Set_Animation(soldier2, "s_a_human.H_A_X5D_ParaT_2", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier2, box2, "Box02" );
			out++;
				
			}
			break;
		case 6: 
			if (!dead)
			{

			GameObject *box3 = Commands->Create_Object("Generic_Cinematic", loc);
			Commands->Set_Model(box3, "X5D_Box03");
			Commands->Set_Facing(box3, facing);
			Commands->Set_Animation(box3, "X5D_Box03.X5D_Box03", false, NULL, 0.0f, -1.0f, false);

			GameObject *soldier3;
			soldier3 = Commands->Create_Object_At_Bone(box3, preset, "Box03");
			Commands->Set_Facing(soldier3, facing);
			Commands->Attach_Script(soldier3, "RMV_Trigger_Killed", "1110009, 10000, 10000");
			Commands->Attach_Script(soldier3, "M00_No_Falling_Damage_DME", "");
			Commands->Set_Animation(soldier3, "s_a_human.H_A_X5D_ParaT_3", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier3, box3, "Box03" );
			out++;
			
			}
			break;
		
		}
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 23000 && param == 23000)
		{
			dead = true;
		}
	}
};

DECLARE_SCRIPT(M10_No_More_Parachute, "")
{
	void Destroyed(GameObject * obj) override
	{
		Commands->Create_3D_Sound_At_Bone("parachute_away", obj, "ROOTTRANSFORM");
	}
};

DECLARE_SCRIPT(M10_Reinforcement_Chinook, "Controller_ID:int")
{
	int sound_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sound_id, 1);
	}
	
	void Created(GameObject * obj) override
	{
		sound_id = Commands->Create_3D_Sound_At_Bone("Chinook_Idle_01", obj, "V_FUSELAGE");
	}
	
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * con = Commands->Find_Object(Get_Int_Parameter(0));
		Commands->Send_Custom_Event(obj, con, 23000, 23000, 0.0f);
	}

	void Destroyed(GameObject * /* obj */) override
	{
		Commands->Stop_Sound(sound_id, true);
	}
};

DECLARE_SCRIPT(M10_Cargo_Plane_Dropoff, "")
{
	bool already_entered;
	bool tank_alive;
	bool airstrip_alive;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( tank_alive, 2 );
		SAVE_VARIABLE( airstrip_alive, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		tank_alive = false;
		airstrip_alive = true;				
	}

	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if (type == TANK_KILLED)
		{
			tank_alive = false;
		}

		if (type == NO_DROP)
		{
			airstrip_alive = false;
		}
	}

	void Entered(GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered && Commands->Get_Health (Commands->Find_Object (1154061)) > 0.0f)
		{
			already_entered = true;
			tank_alive = true;
		
			Vector3 pos = Vector3(-128.772f, -3.245f, 8.151f);
			GameObject * new_object = Commands->Create_Object("Invisible_Object", pos);
			Commands->Set_Facing(new_object, 90.0f);
			Commands->Attach_Script(new_object, "Test_Cinematic", "CnC_C130Drop.txt");

			GameObject * stealth_tank = Commands->Create_Object("Nod_Stealth_Tank", Vector3(0,0,0));
			if (stealth_tank)
			{
				Commands->Send_Custom_Event(obj, new_object, M00_CUSTOM_CINEMATIC_SET_SLOT + 3, Commands->Get_ID(stealth_tank), 0.0f);
				Commands->Attach_Script(stealth_tank, "M10_Stealth_Attack_02", "");
			}

			Commands->Start_Timer (obj, this, 10.0f, CARGO_DROP);
		}		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if (timer_id == CARGO_DROP)
		{
			if (!tank_alive && airstrip_alive)
			{
				tank_alive = true;

				Vector3 pos = Vector3(-128.772f, -3.245f, 8.151f);
				GameObject * new_object = Commands->Create_Object("Invisible_Object", pos);
				Commands->Set_Facing(new_object, 90.0f);
				Commands->Attach_Script(new_object, "Test_Cinematic", "CnC_C130Drop.txt");

				GameObject * stealth_tank = Commands->Create_Object("Nod_Stealth_Tank", Vector3(0,0,0));
				if (stealth_tank)
				{
					Commands->Send_Custom_Event(obj, new_object, M00_CUSTOM_CINEMATIC_SET_SLOT + 3, Commands->Get_ID(stealth_tank), 0.0f);
					Commands->Attach_Script(stealth_tank, "M10_Stealth_Attack_02", "");
				}

				Commands->Start_Timer (obj, this, 10.0f, CARGO_DROP);
			}
		}
	}
};

DECLARE_SCRIPT(M10_Light_Tank, "")
{
	bool attacking;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(attacking, 1);
	}
	
	void Created(GameObject * obj) override
	{
		attacking = false;
		
		Commands->Start_Timer(obj, this, (470.0f/30.0f + 2.0f), 0);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 0)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(STAR, 0.2f, 20.0f);
			params.MoveFollow = true;
			params.Set_Attack(STAR, 100.0f, 4.0f, true);
			params.AttackActive = attacking;
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this, 3.0f, 1);
		}
		if (timer_id == 1)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(STAR, 0.2f, 20.0f);
			params.MoveFollow = true;
			params.Set_Attack(STAR, 100.0f, 4.0f, true);
			attacking = !attacking;
			params.AttackActive = attacking;
			Commands->Modify_Action(obj, 0, params, true, true);
			Commands->Start_Timer(obj, this, (attacking) ? 3.0f : 5.0f, 1);
		}
	}
};

DECLARE_SCRIPT(M10_Ion_Cannon_Detector, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Set_Is_Rendered(obj, false);
		Commands->Enable_Hibernation(obj, false);
		Commands->Disable_All_Collisions(obj);
	}
	
	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		Commands->Set_Health(obj, Commands->Get_Max_Health(obj));
	}
	
	void Sound_Heard(GameObject * obj, const CombatSound & sound) override
	{
		if (sound.Type != SOUND_TYPE_DESIGNER07)
		{
			return;
		}
		
		GameObject * obj_con = Commands->Find_Object(1100154);
		Vector3 my_pos = Commands->Get_Position(obj);
		if (Commands->Get_Distance(my_pos, sound.Position) <= 6.0f)
		{
			if (obj_con)
			{
				Commands->Send_Custom_Event(obj, obj_con, 1007, 1, 0.0f);
			}
		}
		else
		{
			if (obj_con)
			{
				Commands->Send_Custom_Event(obj, obj_con, 1007, 2, 0.0f);
			}
		}
	}
};

    ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////DME/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

DECLARE_SCRIPT (M10_Vehicle_Attack_02, "")	
{
	bool attacking, charge, charging;

	Vector3 current_loc;
	Vector3 enemy_loc;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{		
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( charge, 2 );
		SAVE_VARIABLE( charging, 3 );
		SAVE_VARIABLE( current_loc, 4 );
		SAVE_VARIABLE( enemy_loc, 5 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_NOD );
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;	
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		current_loc = Commands->Get_Position ( obj );
		enemy_loc = Commands->Get_Position ( enemy );
		
		if (!attacking)
		{
			if ((Commands->Get_Distance(current_loc, enemy_loc)) < 100.0f)
			{
				attacking = true;
				
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
				params.Set_Attack(enemy, 100.0f, 7.5f, true);
				params.AttackCheckBlocked = true;
				params.AttackActive = true;
				Commands->Action_Attack (obj, params);				
				
				Commands->Start_Timer(obj, this, 5.0f, ATTACK_OVER);
								
			}
		}

	}

	void Timer_Expired (GameObject* /* obj */, int timer_id) override
	{
		ActionParamsStruct params;

		if (timer_id == ATTACK_OVER)
		{
			attacking = false;
			
		}

	}
	
};

DECLARE_SCRIPT (M10_Stealth, "")	
{
	void Created (GameObject *obj) override
	{
		Commands->Enable_Stealth (obj, true);
	}
};

DECLARE_SCRIPT(M10_Home_Point, "Radius=5.0:float")
{
	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), Get_Float_Parameter("Radius"));
	}
};

DECLARE_SCRIPT(M10_HON_Spawn_Zones, "Zone_Number:int")
{
	void Entered(GameObject * /* obj */, GameObject * /* enterer */) override
	{
		switch (Get_Int_Parameter("Zone_Number"))
		{
		case 1:
			{
				Commands->Enable_Spawner (1100124, true);
				Commands->Enable_Spawner (1100123, true);
			}
			break;
		case 2:
			{
				Commands->Enable_Spawner (1100125, true);
				Commands->Enable_Spawner (1100127, true);
			}
			break;
		case 3:
			{
				Commands->Enable_Spawner (1100128, true);
				Commands->Enable_Spawner (1100130, true);
				Commands->Enable_Spawner (1100129, true);
			}
			break;
		case 4:
			{
				Commands->Enable_Spawner (1100142, true);
				Commands->Enable_Spawner (1100143, true);
				Commands->Enable_Spawner (1100145, true);
				Commands->Enable_Spawner (1100147, true);
				Commands->Enable_Spawner (1100153, true);
			}
			break;
		case 5:
			{
				Commands->Enable_Spawner (1100148, true);
				Commands->Enable_Spawner (1100150, true);
				Commands->Enable_Spawner (1100151, true);
				Commands->Enable_Spawner (2000748, true);
				Commands->Enable_Spawner (2000749, true);
				Commands->Enable_Spawner (2000750, true);
				Commands->Enable_Spawner (2000751, true);
			}
			break;
		}
	}
};

DECLARE_SCRIPT(M10_Refinery_Spawn_Zones, "Zone_Number:int")
{
	void Entered(GameObject * /* obj */, GameObject * /* enterer */) override
	{
		switch (Get_Int_Parameter("Zone_Number"))
		{
		case 1:
			{
				Commands->Enable_Spawner (1100202, true);
				Commands->Enable_Spawner (1100209, true);
			}
			break;
		case 2:
			{
				Commands->Enable_Spawner (1100203, true);
				Commands->Enable_Spawner (1100204, true);
				Commands->Enable_Spawner (2000890, true);
			}
			break;
		case 3:
			{
				Commands->Enable_Spawner (1100212, true);
				Commands->Enable_Spawner (1100210, true);
			}
			break;
		case 4:
			{
				Commands->Enable_Spawner (2000767, true);
				Commands->Enable_Spawner (2000758, true);
				Commands->Enable_Spawner (1100213, true);
				Commands->Enable_Spawner (2000768, true);
			}
			break;
		case 5:
			{
				Commands->Enable_Spawner (1100215, true);
				Commands->Enable_Spawner (1100216, true);
				Commands->Enable_Spawner (2000769, true);
			}
			break;
		case 6:
			{
				Commands->Enable_Spawner (2000770, true);
				Commands->Enable_Spawner (1100220, true);
			}
			break;
		}
	}
};

DECLARE_SCRIPT(M10_PowerPlant_Spawn_Zones, "Zone_Number:int")
{
	void Entered(GameObject * /* obj */, GameObject * /* enterer */) override
	{
		switch (Get_Int_Parameter("Zone_Number"))
		{
		case 1:
			{
				Commands->Enable_Spawner (1100244, true);
				Commands->Enable_Spawner (1100245, true);
			}
			break;
		case 2:
			{
				Commands->Enable_Spawner (1100252, true);
				Commands->Enable_Spawner (1100255, true);
			}
			break;
		case 3:
			{
				Commands->Enable_Spawner (1100257, true);
				Commands->Enable_Spawner (1100259, true);
			}
			break;
		case 4:
			{
				Commands->Enable_Spawner (2000801, true);
				Commands->Enable_Spawner (2000802, true);
				Commands->Enable_Spawner (2000803, true);
				Commands->Enable_Spawner (2000804, true);
			}
			break;
		}
	}
};

DECLARE_SCRIPT(M10_ComCenter_Spawn_Zones, "Zone_Number:int")
{
	void Entered(GameObject * /* obj */, GameObject * /* enterer */) override
	{
		switch (Get_Int_Parameter("Zone_Number"))
		{
		case 1:
			{
				Commands->Enable_Spawner (1100232, true);
				Commands->Enable_Spawner (2000805, true);
			}
			break;
		case 2:
			{
				Commands->Enable_Spawner (2000806, true);
				//Commands->Enable_Spawner (1100238, true);
				Commands->Enable_Spawner (1100243, true);
				Commands->Enable_Spawner (2000812, true);
			}
			break;
		case 3:
			{
				Commands->Enable_Spawner (2000807, true);
				Commands->Enable_Spawner (1100233, true);
				//Commands->Enable_Spawner (1100234, true);
				//Commands->Enable_Spawner (2000811, true);
				Commands->Enable_Spawner (2000809, true);
				Commands->Enable_Spawner (2000810, true);
				//Commands->Enable_Spawner (2000808, true);
			}
			break;
		case 4:
			{
				Commands->Enable_Spawner (2000813, true);
				//Commands->Enable_Spawner (1100241, true);
				Commands->Enable_Spawner (1100240, true);
				//Commands->Enable_Spawner (2000814, true);
				Commands->Enable_Spawner (2000815, true);
			}
			break;
		case 5:
			{
				Commands->Enable_Spawner (2000816, true);
				Commands->Enable_Spawner (2000817, true);
			}
			break;
		case 6:
			{
				Commands->Enable_Spawner (2000819, true);
				Commands->Enable_Spawner (2000818, true);
			}
			break;
		}
	}
};

DECLARE_SCRIPT (M10_Gate_Test, "")
{
	void Poked(GameObject * /* obj */, GameObject * /* poker */) override
	{
		Commands->Static_Anim_Phys_Goto_Last_Frame (2050007, NULL );
	}
};

DECLARE_SCRIPT (M10_Mammoth_Attack, "")	
{
	bool attacking;
	int target [4];
	bool valid [4];
	int choice;

	
	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{		
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( valid, 2 );
		SAVE_VARIABLE( choice, 3 );
	}

	void Created(GameObject * /* obj */) override
	{
		attacking = false;
		
		target [0] = 2000788;
		target [1] = 2000795;
		target [2] = 2000796;
		target [3] = 2000797;

		valid [0] = true;
		valid [1] = true;
		valid [2] = true;
		valid [3] = true;

		choice = Commands->Get_Random_Int(0,4);
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == MAMMOTH && param == ATTACK)
		{
			Commands->Start_Timer(obj, this, 0.1f, ATTACK_OVER);
		}

		if (type == TARGET)
		{
			switch (param)
			{
			case 0:
				{
					valid [param] = false;
				}
				break;
			case 1:
				{
					valid [param] = false;
				}
				break;
			case 2:
				{
					valid [param] = false;
				}
				break;
			}
		}
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{


		if (timer_id == ATTACK_OVER)
		{

			if (!attacking)
			{
					attacking = true;
					
					while (!(valid [choice]))
					{
						choice = Commands->Get_Random_Int(0,4);
					}

					GameObject *mammoth_target = Commands->Find_Object(target [choice]);

					ActionParamsStruct params;
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
					params.Set_Attack(mammoth_target, 1500.0f, 1.5f, true);
					params.AttackCheckBlocked = false;
					params.AttackActive = true;
					Commands->Action_Attack (obj, params);				
					
					Commands->Start_Timer(obj, this, 5.0f, ATTACK_OVER);
			}
			attacking = false;			
		}
	}	
};

DECLARE_SCRIPT (M10_Mammoth_Target_Destruction, "Target_Number:int")
{
	void Destroyed(GameObject * obj) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(2000787), TARGET, Get_Int_Parameter("Target_Number"), 0.0f);
	}
};


DECLARE_SCRIPT (M10_Mammoth_Activate_Zone, "")
{
	bool already_entered;
	
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
				
	}
	
	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			Commands->Send_Custom_Event (obj, Commands->Find_Object(2000787), MAMMOTH, ATTACK, 0.0f);
		}
	}
};

DECLARE_SCRIPT (M10_Stationary, "")
{
	void Created(GameObject *obj) override
	{
		Commands->Set_Innate_Is_Stationary ( obj, true );

	}
};

DECLARE_SCRIPT(M10_TestStealth, "")
{
	bool stealthed;
	float delayTimer;

	REGISTER_VARIABLES()
	{		
		SAVE_VARIABLE( stealthed, 1 );
		SAVE_VARIABLE( delayTimer, 2 );
	}

	void Created( GameObject * obj ) override
	{
		Commands->Innate_Disable(obj);
		delayTimer = Commands->Get_Random ( 10, 20 );
		Commands->Start_Timer(obj, this, delayTimer, 5);
		stealthed = false;
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		if (stealthed == true)
		{
			stealthed = false;
			Commands->Enable_Stealth(obj, false);
			Commands->Debug_Message ( "***************************going out of stealth\n" );
		}

		else
		{
			stealthed = true;
			Commands->Enable_Stealth(obj, true);
			Commands->Debug_Message ( "***************************going to stealth\n" );
		}


		Commands->Start_Timer(obj, this, delayTimer, 5);
	}
};

DECLARE_SCRIPT (M10_GDI_Reinforcement_Waypath, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Aggressiveness ( obj, 10.0f );
		Commands->Set_Innate_Take_Cover_Probability ( obj, 100.0f );

		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
		params.Set_Movement( Vector3(0,0,0), RUN, 0 );
		params.WaypathID = 2000852;			
		Commands->Action_Goto (obj, params);
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (2000861), KILLED, 1, 0.0f);
	}		
};

DECLARE_SCRIPT (M10_GDI_Reinforcement, "")
{
	bool already_entered;
	int kill_tally;
	
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( kill_tally, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		kill_tally = 0;
				
	}

	void Custom(GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == REINFORCE)
		{
			Commands->Enable_Spawner (2000850, true);
			Commands->Enable_Spawner (2000849, true);
			Commands->Enable_Spawner (2000851, true);


			GameObject *drop_obj = Commands->Find_Object (2000861);
			GameObject *drop_obj2 = Commands->Find_Object (1203588);

			float facing = Commands->Get_Facing (drop_obj);
			Vector3 drop_loc = Commands->Get_Position (drop_obj);

			float facing2 = Commands->Get_Facing (drop_obj2);
			Vector3 drop_loc2 = Commands->Get_Position (drop_obj2);

			GameObject *new_obj = Commands->Create_Object("Invisible_Object", drop_loc);
			Commands->Set_Facing(new_obj, facing);
			Commands->Attach_Script(new_obj, "Test_Cinematic", "M10_X3I_GDI_TroopDrop1.txt");

			GameObject *new_obj2 = Commands->Create_Object("Invisible_Object", drop_loc2);
			Commands->Set_Facing(new_obj2, facing2);
			Commands->Attach_Script(new_obj2, "Test_Cinematic", "M10_X3I_GDI_TroopDrop1.txt");
		}

		if (type == KILLED)
		{
			kill_tally++;

			if (kill_tally >= 3)
			{
				kill_tally = 0;

				GameObject *drop_obj = Commands->Find_Object (2000861);

				float facing = Commands->Get_Facing (drop_obj);
				Vector3 drop_loc = Commands->Get_Position (drop_obj);

				GameObject *new_obj = Commands->Create_Object("Invisible_Object", drop_loc);
				Commands->Set_Facing(new_obj, facing);
				Commands->Attach_Script(new_obj, "Test_Cinematic", "M10_X3I_GDI_TroopDrop1.txt");
			}
		}
	}
};

DECLARE_SCRIPT (M10_SAM_Reinforce, "")
{
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (2000861), REINFORCE, 1, 0.0f);
	}
};

DECLARE_SCRIPT (M10_Refinery_Key_Grant, "")
{
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Vector3 create_position = Commands->Get_Position( obj );
		create_position.Z += 1.0f;
		GameObject * key1;
		key1 = Commands->Create_Object( "Level_01_Keycard", create_position );
		Commands->Attach_Script(key1, "M10_Refinery_Keycard", "");
		
		Commands->Set_Objective_Radar_Blip_Object(1012, key1);
	}
};

DECLARE_SCRIPT (M10_Gate_Check, "Objective:int, Gate1:int, Gate2:int")
{
	bool already_poked, first, second;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_poked, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_poked = false;
		first = false;
		second = false;
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100002)
		{
			first = true;
			Commands->Send_Custom_Event(obj, obj_con, 1005, 1, 0.0f);
		}

		if (action_id == 100008)
		{
			second = true;
			Commands->Send_Custom_Event(obj, obj_con, 1007, 1, 0.0f);
		}

		if (action_id == 100017)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1006, 1, 0.0f);
		}

	}
	
	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (!already_poked)
		{			
			if (Commands->Has_Key(STAR, 1))
			{
				already_poked = true;

				int objective = Get_Int_Parameter("Objective");
				int gate1 = Get_Int_Parameter("Gate1");
				int gate2 = Get_Int_Parameter("Gate2");

					//Commands->Send_Custom_Event(obj, Commands->Find_Object (1100154), objective, 1, 0.0f);
				Commands->Static_Anim_Phys_Goto_Last_Frame (gate1, NULL );
				Commands->Static_Anim_Phys_Goto_Last_Frame (gate2, NULL );

				if (objective == 1005)
				{
					int id = Commands->Create_Conversation("M10CON002", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100002);
					Commands->Monitor_Conversation(obj, id);
				}

				if (objective == 1007 && first)
				{
					int id = Commands->Create_Conversation("M10CON008", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100008);
					Commands->Monitor_Conversation(obj, id);
				}

				if (objective == 1007 && !first)
				{
					second = true;
					Commands->Send_Custom_Event(obj, obj_con, 1007, 1, 0.0f);
				}

				if (objective == 1006 && first && second)
				{
					int id = Commands->Create_Conversation("M10CON017", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100017);
					Commands->Monitor_Conversation(obj, id);
				}

				if (objective == 1006 && !first && second)
				{
					Commands->Send_Custom_Event(obj, obj_con, 1006, 1, 0.0f);
				}

				if (objective == 1006 && first && !second)
				{
					Commands->Send_Custom_Event(obj, obj_con, 1006, 1, 0.0f);
				}

				if (objective == 1006 && !first && !second)
				{
					Commands->Send_Custom_Event(obj, obj_con, 1006, 1, 0.0f);
				}
			}
		}
	}
};

DECLARE_SCRIPT (M10_ConYard_Key_Grant, "")
{
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Vector3 create_position = Commands->Get_Position( obj );
		create_position.Z += 1.0f;
		Commands->Create_Object( "Level_03_Keycard", create_position );
			
	}
};

DECLARE_SCRIPT (M10_Spacecraft_Check, "")
{
	void Poked(GameObject * /* obj */, GameObject * /* poker */) override
	{
		if (Commands->Has_Key(STAR, 3))
		{
			Commands->Static_Anim_Phys_Goto_Last_Frame (2058171, NULL );
		}
	}
};

DECLARE_SCRIPT (M10_Mammoth_Grant_Controller, "") //2001634
{
	bool hon_alive;


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( hon_alive, 1 );
		
	}

	void Created (GameObject * /* obj */) override
	{
		hon_alive = true;
				
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		GameObject * mammoth = Commands->Find_Object (2000787);

		if (type == KILLED)
		{
			if (param == HON)
			{
				hon_alive = false;
				
				if (!hon_alive && mammoth)
				{
					Commands->Send_Custom_Event(obj, obj, GRANT, 0, 0.0f);
				}
			}			
		}

		if (type == GRANT)
		{
			GameObject *mammoth = Commands->Find_Object (2000787);
			float curr_health = Commands->Get_Health(mammoth);
			Vector3 mammoth_loc = Commands->Get_Position (mammoth);
			float mammoth_face = Commands->Get_Facing (mammoth);

			GameObject *mammoth_new;
			
			mammoth_new = Commands->Create_Object ("GDI_Mammoth_Tank_Player", mammoth_loc);
			Commands->Attach_Script(mammoth_new, "M10_Occupied", "");
			Commands->Set_Health(mammoth_new, curr_health);
			Commands->Set_Facing ( mammoth_new, mammoth_face );

			Commands->Destroy_Object (mammoth);

			GameObject *GDI_soldier;
			Vector3 GI_loc = Commands->Get_Position (Commands->Find_Object (2001634));
			GDI_soldier = Commands->Create_Object ("GDI_MP", GI_loc);

			int id = Commands->Create_Conversation("M10CON071", 99, 2000, false);
			Commands->Join_Conversation(GDI_soldier, id, true, true, true);
			Commands->Start_Conversation(id, 100071);
			Commands->Monitor_Conversation(obj, id);		
		}
	}
};

DECLARE_SCRIPT (M10_Hon_Killed, "")
{
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Enable_Spawner (2000849, false);
		Commands->Enable_Spawner (2000850, false);
		Commands->Enable_Spawner (2000851, false);

		Commands->Send_Custom_Event(obj, Commands->Find_Object (2001634), KILLED, HON, 0.0f);
	}
};

DECLARE_SCRIPT (M10_Helipad_Killed, "")
{
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (2001634), KILLED, HELIPAD, 0.0f);
	}
};

DECLARE_SCRIPT (M10_Stealth_Drop, "")
{
	void Created (GameObject * obj) override
	{
		Commands->Enable_Stealth (obj, false);
	}

	void Destroyed(GameObject * obj) override 
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (1110056), KILLED, 0 , 0.0f);
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == 2500)
		{
			Commands->Enable_Stealth (obj, true);

			Commands->Start_Timer(obj, this, 2.5f, 2501);
		}

		if (type == 3500)
		{
			Commands->Enable_Stealth (obj, true);			
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{		
		if(timer_id == 2501)
		{
			Commands->Destroy_Object (obj);
		}
	}
};

DECLARE_SCRIPT(DME_Cinematic_Zone, "")
{
	bool already_entered;
	int drop_area [4];
	bool stealthtank_alive;
	int reinforce_chance;
	int stealth_counter;
		
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( stealthtank_alive, 2 );
		SAVE_VARIABLE( reinforce_chance, 3 );
		SAVE_VARIABLE( stealth_counter, 4 );
		SAVE_VARIABLE( drop_area [0], 5 );
		SAVE_VARIABLE( drop_area [1], 6 );
		SAVE_VARIABLE( drop_area [2], 7 );
		SAVE_VARIABLE( drop_area [3], 8 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		stealthtank_alive = false;
		reinforce_chance = 0;
		stealth_counter = 0;

		drop_area [0] = 2002356; //Top of Hill
		drop_area [1] = 2002357; //Hand of Nod
		drop_area [2] = 2003814; //Bottom of Hill
		drop_area [3] = 2003815; //Above Tank				
	}
	
	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if(!already_entered)
		{
			already_entered = true;
			stealth_counter++;

			float stealth_face = Commands->Get_Facing (Commands->Find_Object (drop_area [2]));
			Vector3 drop_loc = Commands->Get_Position (Commands->Find_Object (drop_area [2]));

			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", drop_loc);
			Commands->Set_Facing(chinook_obj3, stealth_face);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "M10_XG_VehicleDrop2.txt");

			stealthtank_alive = true;
			Commands->Start_Timer(obj, this, 10.0f, FAKE_TIMER);
		}

	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		reinforce_chance = Commands->Get_Random_Int (1, 6);

		if (timer_id == FAKE_TIMER && stealth_counter < 4)
		{
			if (!stealthtank_alive && reinforce_chance >= 4)
			{
				stealth_counter++;
				stealthtank_alive = true;

				int random_loc = Commands->Get_Random_Int (0, 4);

				float stealth_face = Commands->Get_Facing (Commands->Find_Object (drop_area [random_loc]));
				Vector3 drop_loc = Commands->Get_Position (Commands->Find_Object (drop_area [random_loc]));

				GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", drop_loc);
				Commands->Set_Facing(chinook_obj3, stealth_face);
				Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "M10_XG_VehicleDrop2.txt");
			}

			Commands->Start_Timer(obj, this, 10.0f, FAKE_TIMER);
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == KILLED)
		{
			stealthtank_alive = false;
		}
	}

};

DECLARE_SCRIPT (M10_Stealth_Attack_01, "")	
{
	bool attacking, charge, charging, same;
	int attack_loc [13];
	float loc_dist;
	int loc;

	Vector3 current_loc;
	Vector3 enemy_loc;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( charge, 2 );
		SAVE_VARIABLE( charging, 3 );
		SAVE_VARIABLE( current_loc, 4 );
		SAVE_VARIABLE( enemy_loc, 5 );
		SAVE_VARIABLE( loc_dist, 6 );
		SAVE_VARIABLE( loc, 7 );
		SAVE_VARIABLE( same, 8 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		charge = false;
		charging = false;
		same = false;

		attack_loc [0] = 2003080;
		attack_loc [1] = 2003081;
		attack_loc [2] = 2003082;
		attack_loc [3] = 2003083;
		attack_loc [4] = 2003084;
		attack_loc [5] = 2003085;
		attack_loc [6] = 2003086;
		attack_loc [7] = 2003087;
		attack_loc [8] = 2003088;
		attack_loc [9] = 2003089;
		attack_loc [10] = 2003090;
		attack_loc [11] = 2003091;
		attack_loc [12] = 2003092;

		loc_dist = 1000.0f;
		loc = 100;

		Commands->Start_Timer(obj, this, 1.0f, GOTO_LOC);			
		

	}

	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 10 && reason ==ACTION_COMPLETE_NORMAL)
		{
			Commands->Debug_Message ("Up Yours!!");
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN +5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
		params.Set_Attack(enemy, 100.0f, 5.0f, true);
		params.AttackCheckBlocked = false;
		//params.AttackActive = true;
		
		Commands->Modify_Action(obj, 10, params, true, true);				
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (1110056), KILLED, 0 , 0.0f);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == GOTO_LOC)
		{
			current_loc = Commands->Get_Position ( obj );
			Vector3 star_loc = Commands->Get_Position (STAR);

			for (int x = 0; x <= 12; x++)
			{
				float dist = Commands->Get_Distance(star_loc, Commands->Get_Position (Commands->Find_Object (attack_loc [x])));
				
				if (dist < loc_dist)
				{
					loc_dist = dist;
					
					loc = x;
				}
			}

			loc_dist = 1000.0f;
			if(loc >= 13)
			{
				Commands->Debug_Message("*** Warning! Loc = %d", loc);
			}

			if (!same)
			{
				Commands->Action_Reset(obj, 99);
			
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN +5, 10 );
				params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
				//params.Set_Attack(STAR, 100.0f, 5.0f, true);
				//params.AttackCheckBlocked = false;
				//params.AttackActive = true;
				//Commands->Modify_Action(obj, 10, params, true, true);

				Commands->Action_Attack(obj, params);
			}

			Commands->Start_Timer(obj, this, 10.0f, GOTO_LOC);			
		}
	}
};

DECLARE_SCRIPT (M10_Stealth_Attack_02, "")	
{
	bool attacking, charge, charging, same;
	int attack_loc [13];
	float loc_dist;
	int loc;

	Vector3 current_loc;
	Vector3 enemy_loc;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( charge, 2 );
		SAVE_VARIABLE( charging, 3 );
		SAVE_VARIABLE( current_loc, 4 );
		SAVE_VARIABLE( enemy_loc, 5 );
		SAVE_VARIABLE( loc_dist, 6 );
		SAVE_VARIABLE( loc, 7 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		charge = false;
		charging = false;
		same = false;

		Commands->Enable_Stealth (obj, true);

		attack_loc [0] = 2005985;
		attack_loc [1] = 2005986;
		attack_loc [2] = 2005987;
		attack_loc [3] = 2005988;
		attack_loc [4] = 2005989;
		attack_loc [5] = 2005990;
		attack_loc [6] = 2005991;
		attack_loc [7] = 2005992;
		attack_loc [8] = 2005993;
		attack_loc [9] = 2005994;
		attack_loc [10] = 2005995;
		attack_loc [11] = 2005996;
		attack_loc [12] = 2005996;

		loc_dist = 1000.0f;
		loc = 100;

		Commands->Start_Timer(obj, this, 0.25f, GOTO_LOC);			
		

		/*ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 0 );
		params.WaypathID = 2000095;
		
		Commands->Action_Attack (obj, params);*/
	}

	void Killed(GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (1100161), TANK_KILLED, 0, 0.0f);
	}
	
	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 10 && reason ==ACTION_COMPLETE_NORMAL)
		{
			Commands->Debug_Message ("Up Yours!!");
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN +5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
		params.Set_Attack(enemy, 100.0f, 5.0f, true);
		params.AttackCheckBlocked = false;
		//params.AttackActive = true;
		
		Commands->Modify_Action(obj, 10, params, true, true);				
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == GOTO_LOC)
		{
			current_loc = Commands->Get_Position ( obj );
			Vector3 star_loc = Commands->Get_Position (STAR);

			for (int x = 0; x <= 12; x++)
			{
				float dist = Commands->Get_Distance(star_loc, Commands->Get_Position (Commands->Find_Object (attack_loc [x])));
				
				if (dist < loc_dist)
				{
					loc_dist = dist;
					
					loc = x;
				}
			}

			loc_dist = 1000.0f;

			if (!same)
			{
				Commands->Action_Reset(obj, 99);
			
				ActionParamsStruct params;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN +5, 10 );
				params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
				//params.Set_Attack(STAR, 100.0f, 5.0f, true);
				//params.AttackCheckBlocked = false;
				//params.AttackActive = true;
				//Commands->Modify_Action(obj, 10, params, true, true);

				Commands->Action_Attack(obj, params);
			}

			Commands->Start_Timer(obj, this, 10.0f, GOTO_LOC);			
		}
	}
};

DECLARE_SCRIPT(M10_Nod_Obelisk, "")
{
	bool attacking;
	
	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Disable_All_Collisions(obj);
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
		Commands->Set_Is_Rendered(obj, false);
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, 99, 1 );
			params.Set_Attack (enemy, 100.0f, 0.5f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M10_PLAYERTYPE_CHANGE_OBELISK)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );			
		}

		if (type == KILLED)
		{
			Commands->Destroy_Object (obj);
		}		
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, 99, 1 );
			params.Set_Attack (damager, 100.0f, 0.5f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}

	}

	void Timer_Expired (GameObject* /* obj */, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;
		}
	}	
};

DECLARE_SCRIPT(M10_Obelisk_MCT, "")
{
	bool alive;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( alive, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		//Commands->Set_Is_Rendered(obj, false);
		alive = true;
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		if (alive)
		{
			Commands->Send_Custom_Event( obj, Commands->Find_Object (2010415), M10_PLAYERTYPE_CHANGE_OBELISK, 0, 0.0f);
		}
	}

	void Custom(GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == KILLED)
		{
			alive = false;
		}		
	}
};

DECLARE_SCRIPT(M10_Refinery_Keycard, "")
{
	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			GameObject * obj_con = Commands->Find_Object(1100154);
			if (obj_con)
			{
				Commands->Send_Custom_Event(obj, obj_con, 1012, 1, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT(M10_Playertype_Nod, "")
{
	void Created (GameObject *obj) override
	{
		Commands->Set_Player_Type ( obj, SCRIPT_PLAYERTYPE_NOD );
	}
};

DECLARE_SCRIPT (M10_Flyover_Controller, "")
{
	int last;
	int last2;
	int last3;
	int count;
	bool already_entered;


	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( last, 1 );
		SAVE_VARIABLE( last2, 2 );
		SAVE_VARIABLE( last3, 3 );
		SAVE_VARIABLE( count, 4 );
		SAVE_VARIABLE( already_entered, 5 );
	}

	void Created (GameObject * /* obj */) override
	{
		last, last2, last3 = 100;
		count = 2;
		already_entered = false;
	}

	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			already_entered = true;

			Commands->Send_Custom_Event ( obj, obj, FLYOVER, 0, 0 );
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		
		if (type == FLYOVER)
		{
			if (++count >= 3)
			{
				count = 0;

				static constexpr const char* flyovers[10] =
				{
					"X10A_Apache_00.txt",
					"X10A_Apache_01.txt",
					"X10A_Apache_02.txt",
					"X10A_Apache_03.txt",
					"X10A_Apache_04.txt",
					
					"X10A_Trnspt_00.txt",
					"X10A_Trnspt_01.txt",
					"X10A_Trnspt_02.txt",
					"X10A_Trnspt_03.txt",
					"X10A_Trnspt_04.txt",
			
				};
				
				int random = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));			
				while (random == last) {
					random = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));
				}

				GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
				Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
				last = random;
		
				int random2 = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));	
				while (random2 == last2 || random2 == random) {
					random2 = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));
				}

				GameObject *controller2 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
				Commands->Attach_Script(controller2, "Test_Cinematic", flyovers[random2]);
				last2 = random2;
	
				int random3 = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));	
				while (random3 == last3 || random3 == random || random3 == random2) {
					random3 = int(Commands->Get_Random(0, 8-WWMATH_EPSILON));
				}					

				GameObject *controller3 = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
				Commands->Attach_Script(controller3, "Test_Cinematic", flyovers[random3]);
				last3 = random3;
			}
		}	
	}
};

DECLARE_SCRIPT (M10_Conversation_Zone, "Conv_Num:int")
{
	bool already_entered;
		
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;				
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == ENTERED)
		{
			already_entered = true;
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (timer_id == KEY_OBJ)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1012, 3, 0.0f);
		}
	};

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100001)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1005, 3, 0.0f);
			Commands->Start_Timer (obj, this, 2.5f, KEY_OBJ);		
		}
		
		if (action_id == 100004)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1002, 3, 0.0f);
		}

		if (action_id == 100007)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1007, 3, 0.0f);
		}

		if (action_id == 100010)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1004, 3, 0.0f);
		}

		if (action_id == 100015)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1001, 3, 0.0f);
		}

		if (action_id == 100018)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1006, 3, 0.0f);
		}

		if (action_id == 100020)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1008, 3, 0.0f);
		}

		if (action_id == 100027)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1015, 3, 0.0f);
		}

		if (action_id == 100030)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1009, 3, 0.0f);
		}

		if (action_id == 100033)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1018, 3, 0.0f);
		}

		if (action_id == 100036)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1010, 3, 0.0f);
		}

		if (action_id == 100039)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1014, 3, 0.0f);
		}

		if (action_id == 100042)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1011, 3, 0.0f);
		}

		if (action_id == 100044)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1017, 3, 0.0f);
		}

		if (action_id == 100051)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1019, 3, 0.0f);
		}
	}


	void Entered (GameObject * obj, GameObject * /* enterer */) override
	{
		if (!already_entered)
		{
			switch(Get_Int_Parameter("Conv_Num"))
			{
			case 1: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON001", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100001);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 3: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON003", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100003);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 4: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON004", 100, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100004);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 6: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014793), ENTERED, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014792), ENTERED, 0, 0.0f);

					int id = Commands->Create_Conversation("M10CON006", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100006);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 7: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object (2017710), ENTERED, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014796), ENTERED, 0, 0.0f);

					int id = Commands->Create_Conversation("M10CON007", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100007);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 10: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON010", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100010);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 13: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON013", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100013);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 15: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014798), ENTERED, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014799), ENTERED, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014800), ENTERED, 0, 0.0f);

					int id = Commands->Create_Conversation("M10CON015", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100015);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 16: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON009", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100016);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 18: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON018", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);					
					Commands->Start_Conversation(id, 100018);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 20: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON020", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);					
					Commands->Start_Conversation(id, 100020);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 22: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1153939)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON022", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);					
						Commands->Start_Conversation(id, 100022);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 23: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1153939)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON023", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100023);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 27: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M10CON027", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);					
					Commands->Start_Conversation(id, 100027);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 29: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1100007)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON029", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);					
						Commands->Start_Conversation(id, 100029);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 30: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object (2015525), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2015524), ENTERED, 0, 0.0f);

					int id = Commands->Create_Conversation("M10CON030", 99, 2000, false);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);					
					Commands->Start_Conversation(id, 100030);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 32: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1153940)) > 0.0f)
					{
						already_entered = true;

						Commands->Send_Custom_Event(obj, Commands->Find_Object (2016248), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2016249), ENTERED, 0, 0.0f);

						int id = Commands->Create_Conversation("M10CON032", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);					
						Commands->Start_Conversation(id, 100032);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 33: 
				{
					if (Commands->Find_Object (1100008) && Commands->Find_Object (1100009))
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON033", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);					
						Commands->Start_Conversation(id, 100033);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 36: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1153934)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON036", 99, 2000, false);
						Commands->Join_Conversation(STAR, id, true, true, true);		
						Commands->Start_Conversation(id, 100036);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 38: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1153934)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON038", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);		
						Commands->Start_Conversation(id, 100038);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 39: 
				{
					if (Commands->Get_Health (Commands->Find_Object (2063322)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON039", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);		
						Commands->Start_Conversation(id, 100039);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 42: 
				{
					if (Commands->Get_Health (Commands->Find_Object (2063323)) > 0.0f || Commands->Get_Health (Commands->Find_Object (1153935)) > 0.0f)
					{
						already_entered = true;

						Commands->Send_Custom_Event(obj, Commands->Find_Object (2016251), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2016976), ENTERED, 0, 0.0f);

						int id = Commands->Create_Conversation("M10CON042", 99, 2000, false);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100042);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 44: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1100012)) > 0.0f || Commands->Get_Health (Commands->Find_Object (1100011)) > 0.0f)
					{
						already_entered = true;

						Commands->Send_Custom_Event(obj, Commands->Find_Object (2017698), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2017699), ENTERED, 0, 0.0f);

						int id = Commands->Create_Conversation("M10CON044", 99, 2000, false);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100044);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 47: 
				{
					if (Commands->Get_Health (Commands->Find_Object (2063324)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON047", 99, 2000, false);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100047);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 49: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1100010)) > 0.0f)
					{
						already_entered = true;

						/*int id = Commands->Create_Conversation("M10CON049", 99, 2000, false);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100049);
						Commands->Monitor_Conversation(obj, id);*/
					}
				}
				break;
			case 51: 
				{
					if (Commands->Get_Health (Commands->Find_Object (1100014)) > 0.0f || Commands->Get_Health (Commands->Find_Object (1100013)) > 0.0f)
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M10CON051", 99, 2000, false);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100051);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 54: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object (2014795), ENTERED, 0, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object (2017711), ENTERED, 0, 0.0f);

					int id = Commands->Create_Conversation("M10CON054", 99, 2000, false);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100054);
					Commands->Monitor_Conversation(obj, id);					
				}
				break;
			}
		}
	}
};

DECLARE_SCRIPT(M10_Sam_Killed, "SamNumber:int")
{
	/*void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100028)
		{
			Commands->Set_Objective_Status(1015, OBJECTIVE_STATUS_ACCOMPLISHED);
		}

		if (action_id == 100050)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1016, 1, 0.0f);
		}
	}*/

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		switch(Get_Int_Parameter("SamNumber"))
		{
		case 1: 
			{
				Commands->Send_Custom_Event(obj, obj_con, 1015, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 1015, 1, 0.0f);

				Commands->Set_Objective_Status(1015, OBJECTIVE_STATUS_ACCOMPLISHED);
			}
			break;
		case 2:
			{
				Commands->Send_Custom_Event(obj, obj_con, 1017, 1, 0.0f);
			}
			break;
		case 3:
			{
				Commands->Send_Custom_Event(obj, obj_con, 1018, 1, 0.0f);
			}
			break;
		case 4:
			{
				Commands->Send_Custom_Event(obj, obj_con, 1016, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 1016, 1, 0.0f);

				Commands->Set_Objective_Status(1016, OBJECTIVE_STATUS_ACCOMPLISHED);
			}
			break;
		}
	}	
};

DECLARE_SCRIPT(M10_Helipad_Destroyed, "PadNumber:int")
{
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		if (action_id == 100040)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1014, 3, 0.0f);
			Commands->Send_Custom_Event(obj, obj_con, 1014, 1, 0.0f);
		}

		if (action_id == 100048)
		{
			Commands->Send_Custom_Event(obj, obj_con, 1013, 3, 0.0f);
			Commands->Send_Custom_Event(obj, obj_con, 1013, 1, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		switch(Get_Int_Parameter("PadNumber"))
		{
		case 1: 
			{
				int id = Commands->Create_Conversation("M10CON040", 99, 2000, false);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation(STAR, id, true, true, true);
				Commands->Start_Conversation(id, 100040);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 2:
			{
				GameObject * obj_con = Commands->Find_Object(1100154);

				Commands->Send_Custom_Event(obj, obj_con, 1013, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 1013, 1, 0.0f);

				/*int id = Commands->Create_Conversation("M10CON048", 99, 2000, false);
				Commands->Join_Conversation(STAR, id, true, true, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100048);
				Commands->Monitor_Conversation(obj, id);*/
			}
			break;
		}
	}	
};

DECLARE_SCRIPT(M10_Turret_Killed, "")
{
	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * obj_con = Commands->Find_Object(1100154);

		Commands->Send_Custom_Event(obj, obj_con, 1019, 3, 0.0f);
		Commands->Send_Custom_Event(obj, obj_con, 1019, 1, 0.0f);
	}	
};

DECLARE_SCRIPT(M10_Mrls_Grant, "")
{
	bool already_poked;
	int drop_loc1;
	int drop_loc2;
	bool occupied1;
	bool occupied2;
	bool grant;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_poked, 1 );
		SAVE_VARIABLE( drop_loc1, 2 );
		SAVE_VARIABLE( drop_loc2, 3 );
		SAVE_VARIABLE( occupied1, 4 );
		SAVE_VARIABLE( occupied2, 5 );
		SAVE_VARIABLE( grant, 6 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_poked = false;
		drop_loc1 = 1112072;
		drop_loc2 = 1112073;
		occupied1 = false;
		occupied1 = false;
		grant = false;
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		if (!already_poked)
		{			
			if (Commands->Has_Key(STAR, 1))
			{
				already_poked = true;

				Commands->Create_Logical_Sound (obj, CLEAR1, Commands->Get_Position (Commands->Find_Object (drop_loc1)), 10.0f);
				Commands->Create_Logical_Sound (obj, CLEAR2, Commands->Get_Position (Commands->Find_Object (drop_loc2)), 10.0f);
				
				Commands->Start_Timer ( obj, this, 2.0f, GRANT_MRLS );
			}
		}
	}	

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == MAMMY)
		{
			if (param == 1)
			{
				grant = false;
			}

			if (param == 0)
			{
				grant = true;
			}
		}

		if (type == OCCUPIED)
		{
			if (param == 1)
			{
				occupied1 = true;
			}

			if (param == 2)
			{
				occupied2 = true;
			}
		}
	}

	void Timer_Expired(GameObject * /* obj */, int timer_id) override
	{
		if (timer_id == GRANT_MRLS)
		{
			if (!occupied1 && grant)
			{
				GameObject * mrls = Commands->Create_Object ( "GDI_MRLS_Player", Commands->Get_Position (Commands->Find_Object (drop_loc1)));
				Commands->Attach_Script(mrls, "M10_Mrls_Waypath", "");
			}

			else if (!occupied2 && grant)
			{
				GameObject * mrls2 = Commands->Create_Object ( "GDI_MRLS_Player", Commands->Get_Position (Commands->Find_Object (drop_loc2)));
				Commands->Attach_Script(mrls2, "M10_Mrls_Waypath", "");
			}
		}
	}
};

DECLARE_SCRIPT(M10_Mrls_Waypath, "")
{
	void Created (GameObject *obj) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100007), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100008), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100009), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100010), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100011), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100012), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);

		ActionParamsStruct params;
		params.Set_Basic( this, 99, 10 );
		params.Set_Movement( Vector3(0,0,0), RUN, 4.0f );
		params.WaypathID = 1112795;			
		Commands->Action_Goto (obj, params);
	}
};

DECLARE_SCRIPT(M10_Humm_SAMIgnore, "")
{
	void Created (GameObject *obj) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100007), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100008), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100009), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100010), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100011), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100012), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
	}
};

DECLARE_SCRIPT(M10_Occupied, "")
{
	void Created (GameObject *obj) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100007), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100008), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100009), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100010), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100011), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100012), M00_CUSTOM_SAM_SITE_IGNORE, Commands->Get_ID (obj), 0.0f);
	}

	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object (1100166), MAMMY, 0, 0.0f);
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		if (sound.Type == CLEAR1)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object (1100166), OCCUPIED, 1, 0.0f);
		}

		if (sound.Type == CLEAR2)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object (1100166), OCCUPIED, 2, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M10_Con_Yard_Repair, "RepairSpeed=1:float")
{
	bool objective_completed, inside_obelisk, conyard_destroyed;
	float full_health, curr_health, counter;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(objective_completed, 1);
		SAVE_VARIABLE(conyard_destroyed, 2);
		SAVE_VARIABLE(curr_health, 3);
		SAVE_VARIABLE(counter, 4);
		SAVE_VARIABLE(full_health, 5);

	}

	void Created(GameObject * obj) override
	{
		objective_completed = conyard_destroyed = false;
		full_health = curr_health = Commands->Get_Health(obj);
	}
	
	
	void Damaged(GameObject * obj, GameObject * damager, float /* amount */) override
	{
		if (conyard_destroyed)
		{
			return;
		}
		else
		{
			if (damager == STAR)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object (1206469), DAMAGED, 0, 0.0f);
			}

			curr_health = Commands->Get_Health (obj);
			counter = 1.0f;
			//Commands->Set_Health(obj, (curr_health + Get_Float_Parameter("RepairSpeed")));

			for (float x = curr_health; x <= full_health; x++)
			{
				Commands->Start_Timer(obj, this, counter, REBUILD);
				counter++;
			
				//Commands->Set_Health(obj, full_health);
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if (timer_id == REBUILD && Commands->Get_Health(obj) > 0)
		{
			curr_health += Get_Float_Parameter("RepairSpeed");
			Commands->Set_Health(obj, curr_health);
		}
	}


	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 3000 && param == 3000)
		{
			conyard_destroyed = true;
		}
	}
};

DECLARE_SCRIPT (M10_Ssm_Trigger, "")
{
	bool already_entered;
		
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;				
	}

	void Entered( GameObject * obj, GameObject * /* enterer */ ) override
	{
		if (!already_entered)
		{
			already_entered = true;

			Commands->Send_Custom_Event (obj, Commands->Find_Object (2009688), M00_LAUNCH_SSM, 0, 0.0f);
		}
	}
};

DECLARE_SCRIPT (M10_Pokeable_Item_OnePoke, "")
{
	bool poked;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );		
	}

	void Created (GameObject * obj) override
	{
		poked = false;
		Commands->Enable_HUD_Pokable_Indicator ( obj, true );			
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		if (!poked)
		{
			poked = true;

			Commands->Enable_HUD_Pokable_Indicator ( obj, false );
		}
	}
};

DECLARE_SCRIPT(M10_Radar_Scramble, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;				
	}

	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if (type == ENTERED)
		{
			already_entered = true;
		}
	}

	void Entered (GameObject *obj, GameObject * /* enterer */) override
	{
		if (Commands->Get_Health (Commands->Find_Object (1153932)) > 0 && Commands->Get_Health (Commands->Find_Object (1153931)) > 0)
		{
			if (!already_entered)
			{
				Commands->Send_Custom_Event (obj, Commands->Find_Object (1110022), ENTERED, 0, 0.0f);
				Commands->Send_Custom_Event (obj, Commands->Find_Object (1100160), ENTERED, 0, 0.0f);
				already_entered = true;

				int id = Commands->Create_Conversation("M03CON068", 99, 2000, false);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100068);
				Commands->Monitor_Conversation(obj, id);
			}
			Commands->Enable_Radar ( false );
		}
	}
};

DECLARE_SCRIPT(M10_Radar_UnScramble, "")
{
	void Entered (GameObject * /* obj */, GameObject * /* enterer */) override
	{
		Commands->Enable_Radar ( true );
	}
};

DECLARE_SCRIPT (M10_HON_KillPrevention, "")
{
	void Killed(GameObject * obj, GameObject * killer) override
	{
		if (killer != STAR)
		{
			Commands->Set_Health (obj, 1.0f);
		}
	}
};

DECLARE_SCRIPT (M10_SoldierPoke, "")
{
	bool already_picked [6];
	int count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_picked [0], 1 );
		SAVE_VARIABLE( already_picked [1], 2 );
		SAVE_VARIABLE( already_picked [2], 3 );
		SAVE_VARIABLE( already_picked [3], 4 );
		SAVE_VARIABLE( already_picked [4], 5 );
		SAVE_VARIABLE( already_picked [5], 6 );
	}

	void Created (GameObject * /* obj */) override
	{
		count = 0;
		for (int x = 0; x <= 5; x++)
		{
			already_picked [x] = false;
		}
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		if (count >= 5)
		{
			for (int x = 0; x <= 5; x++)
			{
				already_picked [x] = false;
			}
		}

		int random = Commands->Get_Random_Int (0, 6);
		while (!already_picked)
		{
			random = Commands->Get_Random_Int (0, 6);
		}

		already_picked [random] = true;
		count++;

		switch (random)
		{
		// Destroy the Power Plant
		case 0: 
			{
				int id = Commands->Create_Conversation("M10CON065", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100065);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 1:
			{
				int id = Commands->Create_Conversation("M10CON066", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100066);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 2:
			{
				int id = Commands->Create_Conversation("M10CON067", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100067);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 3:
			{
				int id = Commands->Create_Conversation("M10CON068", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100068);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 4:
			{
				int id = Commands->Create_Conversation("M10CON069", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100069);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		case 5:
			{
				int id = Commands->Create_Conversation("M10CON070", 99, 2000, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100070);
				Commands->Monitor_Conversation(obj, id);
			}
			break;
		}
	}
};

DECLARE_SCRIPT(M10_NBase_Attacked, "") //1206469
{
	bool message_fired;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( message_fired, 1 );
	}
	
	void Created (GameObject * /* obj */) override
	{
		message_fired = false;
	}

	void Timer_Expired (GameObject * /* obj */, int timer_id) override
	{
		if (timer_id == RESET)
		{
			message_fired = false;
		}
	}

	void Custom (GameObject *obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == DAMAGED && !message_fired)
		{
			message_fired = true;

			int id = Commands->Create_Conversation("M10CON072", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100072);
			Commands->Monitor_Conversation(obj, id);

			Commands->Start_Timer(obj, this, 60.0f, RESET);
		}
	}
};

DECLARE_SCRIPT (M10_NBase_Damage_Modifier, "Damage_multiplier:float")
{
	float current_health, last_health, damage_tally;
	bool conyard_destroyed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current_health, 1 );
		SAVE_VARIABLE( last_health, 2 );
		SAVE_VARIABLE( damage_tally, 3 );
		SAVE_VARIABLE( conyard_destroyed, 4 );
	}

	void Created (GameObject *obj) override
	{
		last_health = Commands->Get_Health (obj);
		damage_tally = 0;

		conyard_destroyed = false;
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == 3000 && param == 3000)
		{
			conyard_destroyed = true;
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override
	{
		if (!conyard_destroyed)
		{
			float damage;

			if (damager == STAR)
			{
				last_health = Commands->Get_Health (obj);
			}

			if (damager != STAR)
			{
				current_health = Commands->Get_Health (obj);
				if (current_health == 0) 
				{
					damage = ((last_health - current_health) + damage_tally);
					damage_tally = 0;
				}

				else
				{
					damage = (last_health - current_health);
					damage_tally = 0;
				}
				float mod_damage = (damage * (Get_Float_Parameter("Damage_multiplier")));
				damage_tally += mod_damage;
				
				Commands->Set_Health (obj, (last_health - mod_damage));
				last_health = Commands->Get_Health (obj);
				current_health = Commands->Get_Health (obj);
			}
		}
	}
};

DECLARE_SCRIPT(M10_Lv2_KeyCarrier, "")
{
	void Killed( GameObject * obj, GameObject * /* killer */ ) override
	{
		Vector3 spawnLocation = Commands->Get_Position ( obj );
		spawnLocation.Z += 0.75f;
		Commands->Create_Object ( "Level_02_Keycard", spawnLocation );
	}
};

DECLARE_SCRIPT(M10_Holograph_EntryZone_DME, "")
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
			float delayTimer = Commands->Get_Random ( 0.25f, 1.0f );
			Commands->Send_Custom_Event( obj, obj, 0, 8000, delayTimer );
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == 8000)
		{
			GameObject * holograph = Commands->Find_Object ( 1208599 );
			if (holograph)
			{
				GameObject * kane = Commands->Create_Object_At_Bone ( holograph, "Nod_Kane_HologramHead", "O_ARROW" );
				Commands->Attach_To_Object_Bone( kane, holograph, "O_ARROW" );
				Commands->Disable_All_Collisions ( kane );
				Commands->Set_Facing (kane, Commands->Get_Facing (holograph));
				Commands->Set_Loiters_Allowed( kane, false );
				Commands->Attach_Script(kane, "M10_KaneHead_DME", "");

				Commands->Send_Custom_Event( obj, Commands->Find_Object (1209308), KANE_CONV, Commands->Get_ID (obj), 0.0f );
				//kane_ID = Commands->Get_ID ( kane );
			}

			//Commands->Destroy_Object ( obj );//one time only zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M10_Holograph_EntryZone2_DME, "")
{
	bool entered;
	int kane_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered, 1);
		SAVE_VARIABLE(kane_id, 2);
	}

	void Created( GameObject * /* obj */ ) override
	{
		entered = false;
		kane_id = 0;
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if (enterer == STAR && entered == false)
		{
			entered = true;
			Commands->Send_Custom_Event( obj, obj, 0, 8500, 0 );
		}
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == KANE_CONV)
		{
			kane_id = param;
		}

		if (param == 8500)
		{
			GameObject * kane = Commands->Find_Object ( kane_id );
			if (kane)
			{
				Commands->Attach_Script(kane, "M10_KaneHead2_DME", "");
			}

			//Commands->Destroy_Object ( obj );//one time only zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M10_KaneHead_DME, "") //2017221
{
	int kane_conversation02;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(kane_conversation02, 1);
	}

	void Created( GameObject * obj ) override
	{
		GameObject *officer = Commands->Find_Object (1208600);

		if (officer)
		{
			int id = Commands->Create_Conversation("M10CON056", 99, 100, true);
			Commands->Join_Conversation_Facing ( obj, id, Commands->Get_ID (officer) );
			Commands->Join_Conversation_Facing ( officer, id, Commands->Get_ID (obj) );
			Commands->Start_Conversation(id, 100056);
			Commands->Monitor_Conversation(obj, id);
		}
	}

	/*void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == 100061) 
					{
						Commands->Debug_Message ( "***************************kane conversation 02 is over--sending delete custom\n" );
						Commands->Send_Custom_Event ( obj, obj, 0, 8000, 2 );
					}
				}
				break;
		}
	}*/

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == 8000)//conversation is over--go away
		{
			Commands->Debug_Message ( "***************************delete custom received--kane should now vanish\n" );
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M10_KaneHead2_DME, "") //2017221
{
	void Created( GameObject * obj ) override
	{
		if (STAR)
		{
			int id = Commands->Create_Conversation("M10CON057", 99, 100, true);
			Commands->Join_Conversation_Facing ( obj, id, Commands->Get_ID (STAR) );
			Commands->Join_Conversation_Facing ( STAR, id, Commands->Get_ID (obj) );
			Commands->Start_Conversation(id, 100057);
			Commands->Monitor_Conversation(obj, id);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
	{
		switch (complete_reason)
		{
			case ACTION_COMPLETE_CONVERSATION_ENDED: 
				{
					if (action_id == 100057) 
					{
						Commands->Debug_Message ( "***************************kane conversation 02 is over--sending delete custom\n" );
						Commands->Send_Custom_Event ( obj, obj, 0, 8000, 2 );
					}
				}
				break;
		}
	}

	void Custom( GameObject * obj, int /* type */, intptr_t param, GameObject * /* sender */ ) override
	{
		if (param == 8000)//conversation is over--go away
		{
			Commands->Debug_Message ( "***************************delete custom received--kane should now vanish\n" );
			Commands->Destroy_Object ( obj );
		}
	}
};
