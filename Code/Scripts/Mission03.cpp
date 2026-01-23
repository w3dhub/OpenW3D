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
*     Mission3.cpp
*
* DESCRIPTION
*     Mission 3 script
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 72 $
*     $Modtime: 2/11/02 10:38a $
*     $Archive: /Commando/Code/Scripts/Mission03.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include <string.h>
#include <stdio.h>
#include "Mission3.h"

namespace Mission3Var
{
	static constexpr const char* KlaxonNames[2] = {
					"Klaxon Warning",
					"Klaxon Info"
	};
}

DECLARE_SCRIPT(M03_Objective_Controller, "")  //1100004
{
	enum {WEATHER_TIMER};
	
	int sam_count_1, sam_count_2;
	bool gunboat_triggered, rain;
	bool gunboat1, gunboat2, gunboat3;

	int background_song;
	int battle_song;
	int sender1;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(sam_count_1, 1);
		SAVE_VARIABLE(sam_count_2, 2);
		SAVE_VARIABLE(gunboat_triggered, 3);
		SAVE_VARIABLE(rain, 4);
		SAVE_VARIABLE(sender1, 5);
		SAVE_VARIABLE(gunboat1, 6);
		SAVE_VARIABLE(gunboat2, 7);
		SAVE_VARIABLE(gunboat3, 8);
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Num_Tertiary_Objectives ( 4 );

		Commands->Set_Wind(90.0f, 5.0f, 2.0f, 0.0f);	
		Commands->Start_Timer(obj, this, Commands->Get_Random(20.0f, 40.0f), WEATHER_TIMER);

		sam_count_1 = sam_count_2 = 0;
		gunboat_triggered = gunboat1 = gunboat2 = gunboat3 = false;
		rain = true;
	//	Commands->Add_Objective(1006, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, 1008, IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);
	//	Commands->Add_Objective(1007, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_HIDDEN, 1009);
	//	Commands->Add_Objective(1008, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, 1010);
	//	Commands->Add_Objective(1009, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, 1011,IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);
	//	Commands->Add_Objective(1010, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, 1019);
	//	Commands->Add_Objective(1011, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, 1020, IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);
	//	Commands->Add_Objective(1012, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, 1021, IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);

		//Commands->Create_Sound ( "03_AmmoClip", Vector3(0,0,0), obj );
	}
	
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		GameObject *object;

		if (reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if (action_id == 1)
			{
				Commands->Add_Objective(1002, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M03_02, NULL, IDS_Enc_Obj_Secondary_M03_02);
				object = Commands->Find_Object(300056);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1002, object);
					Commands->Set_Objective_HUD_Info( 1002, 94, "POG_M03_2_02.tga", IDS_POG_DESTROY/*, Commands->Get_Position (object)*/);
				}
			}

			if (action_id == 2)
			{
				Commands->Add_Objective(1004, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M03_03, NULL, IDS_Enc_Obj_Secondary_M03_03);
				object = Commands->Find_Object(1205777);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1004, Commands->Find_Object (1205777));
					Commands->Set_Objective_HUD_Info( 1004, 97, "POG_M03_2_02.tga", IDS_POG_DESTROY/*, Commands->Get_Position (object)*/);
					Commands->Send_Custom_Event( obj, obj, REMOVE_SECONDARY_POG, 1004, SECONDARY_POG_DELAY);
				}

				//int id = Commands->Create_Conversation("M03CON064", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100064);
				//Commands->Monitor_Conversation(obj, id);
			}

			if (action_id == 100020)
			{
				//int id = Commands->Create_Conversation("M03CON066", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100066);
				//Commands->Monitor_Conversation(obj, id);

				Commands->Set_Objective_Status(1002, OBJECTIVE_STATUS_ACCOMPLISHED);
			}

			if (action_id == 100028)
			{
				//int id = Commands->Create_Conversation("M03CON066", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100066);
				//Commands->Monitor_Conversation(obj, id);
			}

			if (action_id == 100033 || action_id == 100035 || action_id == 100037)
			{
				//int id = Commands->Create_Conversation("M03CON067", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100067);
				//Commands->Monitor_Conversation(obj, id);
			}
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == GUNBOAT_KILLED)
		{
			if (!gunboat1)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), 301, 2, 0.0f);
			}

			if (!gunboat2)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), 302, 2, 0.0f);
			}

			if (!gunboat3)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), 304, 2, 0.0f);
			}
		}


		if ((type == 302 || type == 304) && Commands->Find_Object(1100003) == NULL)
		{
			return;
		}

		if (type == 301)
		{
			sender1 = Commands->Get_ID ( sender );
		}
		
		if (type >= 300 && type <= 312)
		{
			switch(param)
			{
			case 1: Complete_Mission_Objective(type + 700);
				break;
			case 2: Fail_Mission_Objective(type + 700);
				break;
			case 3: Add_Mission_Objective(type + 700);
				break;
			case 4: Unhide_Mission_Objective(type + 700);
				break;
			}
		}

		if (type == 5000 && param == 5000)
		{
			gunboat_triggered = true;
		}

		if (type == SAKURA_DOGFIGHT)
		{
			//Commands->Stop_Sound ( background_song, true );
			//Commands->Create_Sound ( "Sakura_Dogfight", Vector3(0,0,0), obj );
			Commands->Set_Background_Music ( "Sakura_Dogfight.mp3" );
		}

		if (type == DOGFIGHT_ENDED)
		{
			//Commands->Stop_Sound ( battle_song, true );
			Commands->Set_Background_Music ( "03-ammoclip.mp3" );
		}

		/*if (type == REMOVE_SECONDARY_POG)
		{
			Remove_Pog(param);
		}*/

		if (type == 100063)
		{
			Commands->Send_Custom_Event(Commands->Find_Object (1100004), Commands->Find_Object (1100012), 999, 0, 0.0f);
		}

	}

	void Remove_Pog(int id)
	{
		switch (id)
		{
		// Locate Nod Communications Center
		case 1000: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_1_01.tga", IDS_POG_RESCUE);
			}
			break;
		// Help GDI forces secure beachhead
		case 1001: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_2_01.tga", IDS_POG_RESCUE);
			}
			break;
		// Destroy village SAM sites
		case 1002: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_2_02.tga", IDS_POG_RESCUE);
			}
			break;
		// Destroy SAMs near shore defense cannon
		case 1004: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_2_02.tga", IDS_POG_RENDEZVOUS);
			}
			break;
		// Acquire security card from Nod officer
		case 1007: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_1_02.tga", IDS_POG_RENDEZVOUS);
			}
			break;
		// Access Communications center Mainframe
		case 1008: 
			{
				Commands->Set_Objective_HUD_Info(id, -1, "POG_M03_1_03.tga", IDS_POG_ASSIST);
			}
			break;		
		}
	}
	
	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		/*if (timer_id == WEATHER_TIMER)
		{
			if (!rain)
			{
				return;
			}
			if (Commands->Get_Random(0, 1) < 0.5f)
			{
				Commands->Set_Rain(Commands->Get_Random(1.0f, 2.0f), 0.0f, false);
			}
			else
			{
				Commands->Set_Rain(0.0f, 0.0f, false);
			}
			Commands->Start_Timer(obj, this, Commands->Get_Random(20.0f, 40.0f), WEATHER_TIMER);
		}*/

		if (timer_id == 1002)
		{
			Sam_Sites_1_Dialogue();
		}

		if (timer_id == 1004)
		{
			//Commands->Create_Object("Level_3_Objective_Powerup_Temp", Vector3(48.3f, 79.2f, 21.0f));
			
			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(47.70f, 78.07f, 20.26f));
			Commands->Set_Facing(chinook_obj, 0.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X3I_GDI_Drop_PowerUp.txt");
			
			Commands->Set_Objective_Status(1004, OBJECTIVE_STATUS_ACCOMPLISHED);
			Sam_Sites_2_Complete_Dialog();
		}

		if (timer_id == 2004)
		{
			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(47.70f, 78.07f, 20.26f));
			Commands->Set_Facing(chinook_obj, 0.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X3I_GDI_Drop_PowerUp.txt");
		}

		if (timer_id == 1007)
		{
			Commands->Set_Objective_Status(1007, OBJECTIVE_STATUS_PENDING);
		}

		if (timer_id == 911)
		{
			int id = Commands->Create_Conversation("M03CON024", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100024);
			Commands->Monitor_Conversation(obj, id);
		}

		if (timer_id == 912)
		{
			if (Commands->Find_Object (1100003))
			{
				int id = Commands->Create_Conversation("M03CON032", 99, 2000, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation(STAR, id, true, true, true);
				Commands->Start_Conversation(id, 100032);
				Commands->Monitor_Conversation(obj, id);
			}
		}


	}
	
	void Add_Mission_Objective(int id)
	{
		GameObject *object;
		GameObject *controller = Commands->Find_Object (1100004);

		switch (id)
		{
		case 1000:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M03_01, NULL, IDS_Enc_Obj_Primary_M03_01);
				object = Commands->Find_Object(1100009);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(id, object);
					Commands->Set_Objective_HUD_Info_Position( 1000, 96, "POG_M03_1_01.tga", IDS_POG_LOCATE, Commands->Get_Position (object));
				}

				Commands->Send_Custom_Event( controller, controller, 100063, 0, 0.0f);

				/*int id = Commands->Create_Conversation("M03CON063", 99, 2000, false);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100063);
				Commands->Monitor_Conversation(controller, id);*/
			}
			break;
		case 1001:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M03_01, NULL, IDS_Enc_Obj_Secondary_M03_01);
				object = Commands->Find_Object(1212283);
				if(object)
				{
					Commands->Debug_Message ("object found");
					Commands->Set_Objective_Radar_Blip_Object(id, object);
					Commands->Set_Objective_HUD_Info_Position( 1001, 99, "POG_M03_2_01.tga", IDS_POG_DEFEAT, Commands->Get_Position (object));
					Commands->Send_Custom_Event( controller, controller, REMOVE_SECONDARY_POG, id, SECONDARY_POG_DELAY);
				}

				//int id = Commands->Create_Conversation("M03CON064", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100064);
				//Commands->Monitor_Conversation(controller, id);
			}
			break;
		case 1002: 
			{
				Commands->Add_Objective(1002, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M03_02, NULL, IDS_Enc_Obj_Secondary_M03_02);
				object = Commands->Find_Object(300056);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1002, object);
					Commands->Set_Objective_HUD_Info_Position( 1002, 98, "POG_M03_2_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
					Commands->Send_Custom_Event( controller, controller, REMOVE_SECONDARY_POG, id, SECONDARY_POG_DELAY);
				}

				//int id = Commands->Create_Conversation("M03CON064", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100064);
				//Commands->Monitor_Conversation(controller, id);

				Commands->Start_Timer(controller, this, 15.0f, 911);
			}
			break;
		case 1004: Sam_Sites_2_Dialogue();
			break;
		case 1006:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_01, NULL, IDS_Enc_Obj_Hidden_M03_01);
				Commands->Start_Timer(controller, this, 4.0f, 912);
			}
			break;
		case 1007:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M03_02, NULL, IDS_Enc_Obj_Primary_M03_02);
				object = Commands->Find_Object(1215546);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1007, object);
					Commands->Set_Objective_HUD_Info_Position( 1007, 95, "POG_M03_1_02.tga", IDS_POG_LOCATE, Commands->Get_Position (object));
				}

				/*int id = Commands->Create_Conversation("M03CON063", 99, 2000, false);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100063);
				Commands->Monitor_Conversation(controller, id);*/
			}
			break;
		case 1008:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M03_03, NULL, IDS_Enc_Obj_Primary_M03_03);
				object = Commands->Find_Object(1100009);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1008, object);
					Commands->Set_Objective_HUD_Info_Position( 1008, 94, "POG_M03_1_03.tga", IDS_POG_ACCESS, Commands->Get_Position (object));
				}

				//int id = Commands->Create_Conversation("M03CON063", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100063);
				//Commands->Monitor_Conversation(controller, id);
			}
			break;
		case 1009: Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);
			break;
		case 1010:
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M03_04, NULL, IDS_Enc_Obj_Primary_M03_04);
				object = Commands->Find_Object(1213908);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(1010, object);
					Commands->Set_Objective_HUD_Info_Position( 1010, 93, "POG_M03_1_04.tga", IDS_POG_ESCAPE, Commands->Get_Position (object));
				}

				//int id = Commands->Create_Conversation("M03CON063", 99, 2000, false);
				//Commands->Join_Conversation(NULL, id, true, true, true);
				//Commands->Start_Conversation(id, 100063);
				//Commands->Monitor_Conversation(controller, id);
			}
			break;
		case 1011: Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_03, NULL, IDS_Enc_Obj_Hidden_M03_03);
			break;
		case 1012: Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_04, NULL, IDS_Enc_Obj_Hidden_M03_04);
			break;
		}
	}

	void Sam_Sites_1_Dialogue(void)
	{
		/*GameObject * star = Commands->Get_A_Star(Commands->Get_Position(Owner()));
		int id = Commands->Create_Conversation("SAM_Objective_1", 0, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(star, id, true, true, true);
		Commands->Start_Conversation(id, 1);
		Commands->Monitor_Conversation(Owner(), id);*/
	}

	void Sam_Sites_2_Dialogue(void)
	{
		if (Commands->Find_Object (1100003) && Commands->Find_Object (1100002))
		{
			int id = Commands->Create_Conversation("M03CON026", 0, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 2);
			Commands->Monitor_Conversation(Owner(), id);
		}
	}

	void Sam_Sites_1_Complete_Dialog(void)
	{
		/*GameObject * star = Commands->Get_A_Star(Commands->Get_Position(Owner()));
		int id = Commands->Create_Conversation("SAM_Objective_1_Complete", 0, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(star, id, true, true, true);
		Commands->Start_Conversation(id, 0);*/
	}

	void Sam_Sites_2_Complete_Dialog(void)
	{
		/*GameObject * star = Commands->Get_A_Star(Commands->Get_Position(Owner()));
		int id = Commands->Create_Conversation("SAM_Objective_2_Complete", 0, 2000, false);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(star, id, true, true, true);
		Commands->Start_Conversation(id, 0);*/
	}
	
	void Complete_Mission_Objective(int id)
	{
		GameObject *controller = Commands->Find_Object (1100004);

		if (id == 1000 || id == 1007 || id == 1008 || id == 1010)
		{
			//int id = Commands->Create_Conversation("M03CON065", 99, 2000, false);
			//Commands->Join_Conversation(NULL, id, true, true, true);
			//Commands->Start_Conversation(id, 100065);
			//Commands->Monitor_Conversation(controller, id);
		}

		if (id == 1000)
		{
			GameObject * com_center = Commands->Find_Object (1150002);
			Commands->Send_Custom_Event(controller, com_center, BASE_ENTERED, 0, 0.0f);
		}

		if (id == 1006 || id == 1009 || id == 1011 || id == 1012)
		{
			if (id == 1009)
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_02, NULL, IDS_Enc_Obj_Hidden_M03_02);
				Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_ACCOMPLISHED);

				int convo_id = Commands->Create_Conversation("M03CON033", 99, 2000, false);
				Commands->Join_Conversation(NULL, convo_id, true, true, true);
				Commands->Join_Conversation(STAR, convo_id, true, true, true);
				Commands->Start_Conversation(convo_id, 100033);
				Commands->Monitor_Conversation(controller, convo_id);
			}

			if (id == 1011)
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_03, NULL, IDS_Enc_Obj_Hidden_M03_03);
				Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_ACCOMPLISHED);

				int convo_id = Commands->Create_Conversation("M03CON035", 99, 2000, false);
				Commands->Join_Conversation(NULL, convo_id, true, true, true);
				Commands->Join_Conversation(STAR, convo_id, true, true, true);
				Commands->Start_Conversation(convo_id, 100035);
				Commands->Monitor_Conversation(controller, convo_id);
			}

			if (id == 1012)
			{
				Commands->Add_Objective(id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Hidden_M03_04, NULL, IDS_Enc_Obj_Hidden_M03_04);
				Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_ACCOMPLISHED);

				int convo_id = Commands->Create_Conversation("M03CON037", 99, 2000, false);
				Commands->Join_Conversation(NULL, convo_id, true, true, true);
				Commands->Join_Conversation(STAR, convo_id, true, true, true);
				Commands->Start_Conversation(convo_id, 100037);
				Commands->Monitor_Conversation(controller, convo_id);
			}
		}

		if (id == 1001)
		{
			gunboat1 = true;
		}

		if (id == 1002)
		{
			sam_count_1++;
			if (sam_count_1 >= 2)
			{
				gunboat2 = true;
				/*Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_ACCOMPLISHED);
				GameObject *tailgun_zone = Commands->Find_Object(1141168);
				if (tailgun_zone)
				{
					Commands->Send_Custom_Event(Owner(), tailgun_zone, 200, 200, 0.0f);
				}
				Sam_Sites_1_Complete_Dialog();*/

				int convo_id = Commands->Create_Conversation("M03CON020", 99, 2000, false);
				Commands->Join_Conversation(NULL, convo_id, true, true, true);
				Commands->Join_Conversation(STAR, convo_id, true, true, true);
				Commands->Start_Conversation(convo_id, 100020);
				Commands->Monitor_Conversation(controller, convo_id);

				//Commands->Create_Object("Level_3_Objective_Powerup_Temp", Vector3(73.624f, -78.110f, 0.7f));
				
				GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(73.0f, -80.04f, 0.2f));
				Commands->Set_Facing(chinook_obj, 0.0f);
				Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X3I_GDI_Drop_PowerUp.txt");

				Commands->Send_Custom_Event(controller, Commands->Find_Object (1100001), ENTERED, 0, 0.0f);
			}
		}
		if (id == 1004)
		{
			sam_count_2++;
			if (sam_count_2 >= 2)
			{
				gunboat3 = true;

				Commands->Set_Objective_Status(1004, OBJECTIVE_STATUS_ACCOMPLISHED);

				if (Commands->Find_Object (1100002) && Commands->Find_Object (1100003))
				{
					int convo_id = Commands->Create_Conversation("M03CON027", 99, 2000, false);
					Commands->Join_Conversation(NULL, convo_id, true, true, true);
					Commands->Start_Conversation(convo_id, 100027);
					Commands->Monitor_Conversation(controller, convo_id);

					Commands->Start_Timer(Owner(), this, 4.0f, 1004);
				}

				if (!(Commands->Find_Object (1100002)) && Commands->Find_Object (1100003))
				{
					int convo_id = Commands->Create_Conversation("M03CON028", 99, 2000, true);
					Commands->Join_Conversation(NULL, convo_id, true, true, true);
					Commands->Start_Conversation(convo_id, 100028);
					Commands->Monitor_Conversation(controller, convo_id);
					//Commands->Set_Objective_Status(1004, OBJECTIVE_STATUS_ACCOMPLISHED);

					Commands->Start_Timer(Owner(), this, 4.0f, 2004);
				}
			}
		}
		if (id == 1001)
		{
			//Commands->Create_Object("Level_3_Objective_Powerup_Temp", Vector3(-94.945f, -66.073f, 2.0f));
			
			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(-96.2f, -75.8f, 1.4f));
			Commands->Set_Facing(chinook_obj, 0.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X3I_GDI_Drop_PowerUp.txt");
			
			GameObject * gunboat = Commands->Find_Object(1100003);
			if (gunboat && !gunboat_triggered)
			{
				Commands->Send_Custom_Event(Owner(), gunboat, 2000, 1, 0.0f);
				GameObject *zone = Commands->Find_Object(1100000);
				Commands->Destroy_Object(zone);
			}
		}
		if (id != 1002 && id != 1004 && id != 1009 && id != 1011 && id != 1012)
		{
			Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_ACCOMPLISHED);
		}
	}

	void Fail_Mission_Objective(int id)
	{
		Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_FAILED);
	}

	void Unhide_Mission_Objective(int id)
	{
		if (id == 1007)
		{
			Commands->Start_Timer(Owner(), this, 3.0f, 1007);
		}
		else if (id == 1010)
		{
			rain = false;
			Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_PENDING);
		}
		else
		{
			Commands->Set_Objective_Status(id, OBJECTIVE_STATUS_PENDING);
		}
	}
};

DECLARE_SCRIPT(RMV_Test_Big_Gun_Turning, "")
{
	int count;
	bool gunboat_warned;
	bool cine;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( count, 1 );
		SAVE_VARIABLE( gunboat_warned, 2);
		SAVE_VARIABLE( cine, 3 );
	}

	void Created(GameObject * /*obj*/) override
	{
		cine = false;
		count = 0;
		gunboat_warned = false;
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject *temp;
		//Commands->Create_Sound("00-n036e", Vector3(0,0,0), obj);
		temp = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(temp, "Test_Cinematic", "X3C_Bigguns2.txt");

		Commands->Send_Custom_Event (obj, Commands->Find_Object(1100004), 306, 3, 0.0f);
		Commands->Send_Custom_Event (obj, Commands->Find_Object(1100004), 306, 1, 0.0f);

		Commands->Send_Custom_Event (obj, Commands->Find_Object(1100003), CANNON_KILLED, 0, 0.0f);

		/*int id = Commands->Create_Conversation("IDS_M03_D01", 0, 0, true);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(STAR, id, true, true, true);
		Commands->Start_Conversation(id, 0);*/
	}		

	void Sound_Heard(GameObject * /*obj*/, const CombatSound & sound) override
	{
		if (sound.Type == SOUND_TYPE_DESIGNER04)
		{			
			Commands->Shake_Camera(sound.Position, 40, 0.3f, 1.0f);
			//Commands->Apply_Damage ( Commands->Find_Object(1100003), 20.0f, "EXPLOSIVE", Commands->Find_Object(1100002));
			if (!gunboat_warned)
			{
				float random = Commands->Get_Random(1, 5);
				if (random < 2)
				{
					gunboat_warned = true;
					/*int id = Commands->Create_Conversation("Gunboat_Warn_Big_Gun", 0, 0, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 0);*/
				}
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * /*damager*/, float /*amount*/ ) override
	{
		if (cine)
		{
			if (Commands->Get_Health (obj) < 1)
			{
				Commands->Set_Health (obj, Commands->Get_Health (obj) + 1.0f);
			}


		}
	}

	void Timer_Expired(GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == ORCA_TIMER)
		{
			if (Commands->Find_Object (1100002))
			{
				GameObject *temp;
				//Commands->Create_Sound("00-n036e", Vector3(0,0,0), obj);
				temp = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
				Commands->Attach_Script(temp, "Test_Cinematic", "X3C_Bigguns.txt");

				cine = true;
			}
		}
	}

	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == GUNBOAT_KILLED)
		{
			Commands->Action_Reset (obj, 100);
			Commands->Enable_Enemy_Seen (obj, false);
		}

		if ((type == 1000) && (param == 1000))
		{
			/*Vector3 target, pos;
			pos = Commands->Get_Position(obj);
			target.X = pos.X + 49.31;
			target.Y = pos.Y + 8.25;
			target.Z = pos.Z + 1.0f;
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			if (Commands->Find_Object(1100003) == NULL)
			{
				params.Set_Attack(target, 0.0f, 3.0f, true);
			}
			else
			{
				params.Set_Attack(target, 100.0f, 0.0f, true);
			}*/

			ActionParamsStruct params;
			params.Set_Basic(this, 99, 2);
			params.Set_Attack(Commands->Find_Object (1100003), 1000.0f, 2.0f, true);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack(obj, params);
		}
		if ((type == 2000) && (param == 2000))
		{
			count++;
			if (count == 2 && Commands->Find_Object(1100003) != NULL)
			{
				Commands->Start_Timer(obj, this, 2.0f, ORCA_TIMER);			
			}
		}
		if (type == 2450 && param == 2450)
		{
			GameObject * gunboat = Commands->Find_Object(1100003);
			if (gunboat)
			{
				Commands->Send_Custom_Event(obj, gunboat, 8000, 8000, 0.0f);
			}
			Commands->Destroy_Object(obj);
		}
	}
};

DECLARE_SCRIPT(M03_Gunboat_Controller_RMV, "Receive_Type:int, Receive_Param_For_Village:int, Receive_Param_For_Cannon:int, Beach_Destination:vector3, Village_Start:vector3, Village_Destination:vector3, Cannon_Start:vector3, Cannon_Destination:Vector3, Receive_Param_Destroy:int")
{
	typedef enum {
		STATE_AT_BEACH,
		STATE_AT_VILLAGE,
		STATE_AT_CANNON,
		STATE_IN_TRANSIT,
		STATE_AFTER_CANNON
	} Gunboat_State;

	enum {
		GUNBOAT_ACTION,
		GUNBOAT_TIMER
	};

	Gunboat_State state, last_state;
	Vector3 beach_dest, village_start, village_dest, cannon_start, cannon_dest;
	Vector3 beach_targets[3], village_targets[3], cannon_targets[3];
	float beach_error, village_error, cannon_error;
	float random;
	bool been_shot, warning_1_given, warning_2_given, had_conv, inlet_fire, cannon_killed;
	float last_health;
	int inlet_count, dam_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( state, 1 );
		SAVE_VARIABLE( last_state, 2 );
		SAVE_VARIABLE( beach_dest, 3 );
		SAVE_VARIABLE( village_start, 4 );
		SAVE_VARIABLE( village_dest, 5 );
		SAVE_VARIABLE( cannon_start, 6 );
		SAVE_VARIABLE( cannon_dest, 7 );
		SAVE_VARIABLE( beach_targets, 8 );
		SAVE_VARIABLE( village_targets, 9 );
		SAVE_VARIABLE( cannon_targets, 10 );
		SAVE_VARIABLE( beach_error, 11 );
		SAVE_VARIABLE( village_error, 12 );
		SAVE_VARIABLE( cannon_error, 13 );
		SAVE_VARIABLE( random, 14 );
		SAVE_VARIABLE( been_shot, 15 );
		SAVE_VARIABLE( last_health, 16 );
		SAVE_VARIABLE( warning_1_given, 17);
		SAVE_VARIABLE( warning_2_given, 18);
		SAVE_VARIABLE( had_conv, 19 );
		SAVE_VARIABLE( inlet_count, 20 );
		SAVE_VARIABLE( inlet_fire, 21 );
		SAVE_VARIABLE( dam_count, 22 );
		SAVE_VARIABLE( cannon_killed, 23 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);
		//Commands->Disable_Physical_Collisions(obj);
		//Commands->Disable_All_Collisions(obj);
		Commands->Enable_Engine(obj, true);
		warning_1_given = warning_2_given = been_shot = had_conv = false;
		last_health = Commands->Get_Max_Health(obj);
		beach_dest = Get_Vector3_Parameter("Beach_Destination");
		village_start = Get_Vector3_Parameter("Village_Start");
		village_dest = Get_Vector3_Parameter("Village_Destination");
		cannon_start = Get_Vector3_Parameter("Cannon_Start");
		cannon_dest = Get_Vector3_Parameter("Cannon_Destination");
		beach_error = 3.0f;
		village_error = 3.0f;
		cannon_error = 3.0f;
		beach_targets[0] = Vector3(-116,-44,25);
		beach_targets[1] = Vector3(-96,-36,25);
		beach_targets[2] = Vector3(-74,-35,29);
		village_targets[0] = Vector3(24,-25,16);
		village_targets[1] = Vector3(45,-30,16);
		village_targets[2] = Vector3(80,-12,16);
		cannon_targets[0] = Vector3(91,42,27);
		cannon_targets[1] = Vector3(66,71,17);
		cannon_targets[2] = Vector3(52.558f, 51.739f, 28.880f);
		inlet_count = 0;
		inlet_fire = true;
		state = STATE_AT_BEACH;
		ActionParamsStruct params;
		params.Set_Basic(this, 90, GUNBOAT_ACTION);
		params.Set_Movement(beach_dest, 0.25f, 3);
		random = Commands->Get_Random(0, 3);
		random = WWMath::Clamp(random, 0, 2);
		params.Set_Attack(beach_targets[(int)random], 500.0f, beach_error, true);
		params.AttackCheckBlocked = false;
		params.MovePathfind = false;
		params.AttackFaceTarget = false;
		Commands->Action_Attack(obj, params);
		Commands->Start_Timer(obj, this, 3.5f, GUNBOAT_TIMER);
		cannon_killed = false;

		dam_count = 0;
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == GUNBOAT_TIMER && !cannon_killed)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, GUNBOAT_ACTION);
			params.AttackCheckBlocked = false;
			params.MovePathfind = false;
			params.AttackFaceTarget = false;
			switch (state)
			{
			case STATE_AT_BEACH: random = Commands->Get_Random(0, 3);
				random = WWMath::Clamp(random, 0, 2);
				params.Set_Movement(beach_dest, 0.25f, 3);
				params.Set_Attack(beach_targets[(int)random], 500.0f, beach_error, true);
				Commands->Action_Attack(obj, params);
				break;
			case STATE_AT_VILLAGE: random = Commands->Get_Random(0, 3);
				random = WWMath::Clamp(random, 0, 2);
				params.Set_Movement(village_dest, 0.25f, 3);
				params.Set_Attack(village_targets[(int)random], 500.0f, village_error, true);
				if (!inlet_fire)
				{
					params.AttackActive = false;
				}
				Commands->Action_Attack(obj, params);
				break;
			case STATE_AT_CANNON: random = Commands->Get_Random(0, 3);
				random = WWMath::Clamp(random, 0, 2);
				params.Set_Movement(cannon_dest, 0.25f, 3);
				params.Set_Attack(cannon_targets[(int)random], 500.0f, cannon_error, true);
				Commands->Action_Attack(obj, params);
				break;
			case STATE_AFTER_CANNON:
				{
					Commands->Action_Reset (obj, 100);
				}
			case STATE_IN_TRANSIT: 
				switch (last_state)
				{
				case STATE_AT_BEACH: 
					{
					params.Set_Movement(village_start, 0.5f, 3);
					if (!had_conv)
					{
						had_conv = true;
						
						/*int id = Commands->Create_Conversation("Gunboat_To_Inlet", 0, 0, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Start_Conversation(id, 0);*/
					}
					break;
					}
				case STATE_AT_VILLAGE: params.Set_Movement(cannon_start, 0.5f, 3);
					break;
				}
				Commands->Action_Goto(obj, params);
				break;
			}
			Commands->Start_Timer(obj, this, 1.5f, GUNBOAT_TIMER);
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == CANNON_KILLED)
		{
			cannon_killed = true;
			Commands->Action_Reset (obj, 100);
		}

		if (type == 8000 && param == 8000)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, 40);
			params.Set_Movement(Vector3(-58.0f, 222.0f, -1.120f), 0.5f, 5.0f);
			Commands->Action_Goto(obj, params);
			return;
		}

		if (type == 7000 && param == 7000)
		{
			if (++inlet_count >= 4)
			{
				inlet_fire = false;
			}
		}
		
		ActionParamsStruct params;
		params.Set_Basic(this, 90, GUNBOAT_ACTION);
		if ((type == Get_Int_Parameter("Receive_Type")) && (param == Get_Int_Parameter("Receive_Param_Destroy")))
		{
			Commands->Destroy_Object(obj);
			return;
		}
		if ((type == Get_Int_Parameter("Receive_Type")) && (param == Get_Int_Parameter("Receive_Param_For_Village")))
		{
		/*	// Turn off beach bunkers
			if (Commands->Find_Object(1000001)) {
				Commands->Send_Custom_Event(obj, Commands->Find_Object(1000001), 3000, 3000, 0.0f);
			}
			if (Commands->Find_Object(1000002)) {
				Commands->Send_Custom_Event(obj, Commands->Find_Object(1000002), 3000, 3000, 0.0f);
			}
			if (Commands->Find_Object(1000003)) {
				Commands->Send_Custom_Event(obj, Commands->Find_Object(1000003), 3000, 3000, 0.0f);
			} */

			params.Set_Movement(village_start, 1.0f, 3);
			last_state = STATE_AT_BEACH;
		}
		if ((type == Get_Int_Parameter("Receive_Type")) && (param == Get_Int_Parameter("Receive_Param_For_Cannon")))
		{
			params.Set_Movement(cannon_start, 1.0f, 3);
			last_state = STATE_AT_VILLAGE;
		}
		state = STATE_IN_TRANSIT;
		Commands->Action_Goto(obj, params);
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 40)
		{
			return;
		}
		
		if (reason != ACTION_COMPLETE_NORMAL)
			return;
		if (action_id == GUNBOAT_ACTION)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 90, GUNBOAT_ACTION);
			if (state == STATE_IN_TRANSIT)
			{
				if (last_state == STATE_AT_BEACH)
				{
					params.Set_Movement(village_dest, 0.25f, 3);
					state = STATE_AT_VILLAGE;
				}
				if (last_state == STATE_AT_VILLAGE)
				{
					params.Set_Movement(cannon_dest, 0.25f, 3);
					state = STATE_AT_CANNON;
				}
				Commands->Action_Goto(obj, params);
			}
			else if
				(state == STATE_AT_CANNON)
				Commands->Action_Reset(obj, 90);
		}
	}

	void Check_Health(void)
	{
		if (!warning_2_given && last_health <= 0.25f * Commands->Get_Max_Health(Owner()))
		{
			warning_2_given = true;
			/*int id = Commands->Create_Conversation("Gunboat_Near_Death", 0, 0, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 0);*/
			return;
		}

		if (!warning_1_given && last_health <= 0.5f * Commands->Get_Max_Health(Owner()))
		{
			warning_1_given = true;
			/*int id = Commands->Create_Conversation("Gunboat_Damaged", 0, 0, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 0);*/
		}
	}
	
	void Damaged(GameObject * obj, GameObject * damager, float /*amount*/) override
	{

		if (damager && Commands->Is_A_Star(damager) && !been_shot)
		{
			been_shot = true;
			Commands->Create_2D_Sound("00-N062E");
		}
		if (Commands->Get_Health(obj) <= 0)
		{
			return;
		}
		float damage = last_health - Commands->Get_Health(obj);
		damage *= ((DIFFICULTY + 1));
		Commands->Set_Health(obj, last_health - damage);
		last_health = Commands->Get_Health(obj);
		Check_Health();

		if (damager == Commands->Find_Object (1100002))
		{
			if (++dam_count > 1 && dam_count <= 3)
			{
				if (Commands->Get_Health (Commands->Find_Object (300058)) > 0 || Commands->Get_Health (Commands->Find_Object (300058)) > 0)
				{
					int id = Commands->Create_Conversation("M03CON029", 99, 2000, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Start_Conversation(id, 100029);
					Commands->Monitor_Conversation(obj, id);

					return;
				}
			}

			if (++dam_count > 3 && dam_count <= 5)
			{
				int id = Commands->Create_Conversation("M03CON030", 99, 2000, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100030);
				Commands->Monitor_Conversation(obj, id);
			}
		}
	}

	void Killed(GameObject * obj, GameObject * killer) override
	{
		Commands->Create_Explosion_At_Bone("Ground Explosions Twiddler", obj, "TURRET", NULL);
		
		if (killer == Commands->Find_Object (1100002))
		{
			int id = Commands->Create_Conversation("M03CON031", 0, 0, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100031);
			Commands->Monitor_Conversation(obj, id);

			Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), GUNBOAT_KILLED, 0, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object (1100003), GUNBOAT_KILLED, 0, 0.0f);

			//Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), 304, 2, 0.0f);
		}

		if (killer != Commands->Find_Object (1100002))
		{
			int id = Commands->Create_Conversation("M03CON018", 0, 0, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100018);
			Commands->Monitor_Conversation(obj, id);

			//Commands->Send_Custom_Event(obj, Commands->Find_Object (1100004), 302, 2, 0.0f);
		}
	}
};

/*DECLARE_SCRIPT(RMV_Temp_EVA_Dialogue, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( last, 1 );
	}
	
	void Created(GameObject * obj) override
	{
		last = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		char *list[11];
		
		list[0] = "00-N184E";	//Locate Comm Center
		list[1] = "00-N186E";	//Secure Beachhead
		list[2] = NULL;//"00-N188E";	//Destroy SAM #1
		list[3] = "00-N188E";	//Destroy SAM #2
		list[4] = NULL;//"00-N188E";	//Destroy SAM #3
		list[5] = "00-N188E";	//Destroy SAM #4
		list[6] = "00-N154E";	//Destroy Big Gun
		list[7] = "00-N200E";	//Acquire Keycard
		list[8] = "00-N210E";	//Access Comm Center Terminal
		list[9] = "00-N204E";	//Destroy Power Plant Terminal
		list[10] = "00-N150E";	//Escape via the sub
		
		if (param == 1)
		{
			Commands->Start_Timer(obj, this, 2.0f, 0);
		}
		
		if (type == last) return;
		if ((param == 3) || (param == 4))
		{
			int num = type - 300;
			if (num <= 10)
			{
				if (list[num] != NULL)
				{
				//	Commands->Create_Sound(list[num], Vector3(0,0,0), obj);
				}
			}
		}
		last = type;
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		char *taunts[4];

		taunts[0] = "laugh1";
		taunts[1] = "lefty1";
		taunts[2] = "bombit1";
		taunts[3] = "keepem1";
		
		float random = Commands->Get_Random(0, 11);
		if (random < 8.0f)
		{
			random /= 2.0f;
			random = WWMath::Clamp(random, 0, 3);
			int d_random = (int)random;
			Commands->Create_Sound(taunts[d_random], Vector3(0,0,0), obj);
		}
	}
};*/

DECLARE_SCRIPT(Sakura_Killed, "")
{
	int music_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(music_id, 1);
	}
	
	void Created(GameObject * obj) override
	{
		if (Commands->Find_Object(1144518)) {
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1144518), 622, 622, 0.0f);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /*amount*/ ) override 
	{
		if (damager != STAR)
		{
			if (Commands->Get_Health (obj) < 5)
			{
				Commands->Set_Health (obj, Commands->Get_Health (obj) + 1);
			}
		}
	}
	
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject *temp;
		temp = Commands->Create_Object("Sakura Crash Controller", Vector3(0,0,0));
		if (Commands->Find_Object(1300001)) {
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1300001), 6000, 6600, 0.0f);
		}
		Commands->Grant_Key(STAR, 6, true);
	}
};

DECLARE_SCRIPT(M03_Outro_Cinematic, "")
{
	int star_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( star_id, 1 );
	}
	
	void Entered(GameObject * obj, GameObject * enterer) override
	{
		{
			star_id = Commands->Get_ID(enterer);
			GameObject *controller;
			controller = Commands->Find_Object(1100004);
			if (controller) {
				Commands->Send_Custom_Event(obj, controller, 8000, 8000, 0.0f);
			}
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 8000) && (param >= 3))
		{
			GameObject *temp;
			GameObject *star = Commands->Find_Object(star_id);
			Vector3 pos;
			pos.Set(-324.49f, 71.60f, 4.26f);
			Commands->Set_Position(star, pos);
			temp = Commands->Create_Object("Finale Controller", Vector3(0,0,0));
			Commands->Start_Timer(obj, this, 625.0f/30.0f, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		GameObject *controller;
		controller = Commands->Find_Object(1100004);
		if (controller)
		{
			Commands->Send_Custom_Event(obj, controller, 310, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(RMV_M03_Comm_Center_Terminal, "")
{
	bool has_been_poked;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( has_been_poked, 1 );
	}

	void Created(GameObject * /*obj*/) override
	{
		has_been_poked = false;
	}
	
	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (!has_been_poked && Commands->Is_A_Star(poker))
		{
			has_been_poked = true;

			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 308, 1, 0);

			int id = Commands->Create_Conversation("M03CON008", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100008);
			Commands->Monitor_Conversation(obj, id);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == 100008)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), SAKURA_DOGFIGHT, 0, 0.0f);
			
			/*int id = Commands->Create_Conversation("M03CON010", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100010);
			Commands->Monitor_Conversation(obj, id);*/
			
			//Commands->Grant_Key(STAR, 2, true);
			//Commands->Grant_Key(STAR, 3, true);
			Commands->Grant_Key(STAR, 5, false);

			Vector3 sakura_pos;
			sakura_pos.Set(-160.690f, 76.470f, 16.270f);
			Commands->Create_Object("Boss", sakura_pos);
		}

		if (action_id == 100010)
		{
			//Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 308, 1, 0, 0.0f);
		}
	}

	void Damaged(GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Health(obj, Commands->Get_Max_Health(obj));
	}
};

DECLARE_SCRIPT(RMV_Volcano_And_Lava_Ball_Creator, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}

	void Created(GameObject * obj) override
	{
		last = -1;
		Commands->Enable_Cinematic_Freeze( obj, false );

	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 500) && (param == 500))
		{
			Commands->Reveal_Encyclopedia_Character	( 39 );//sakura
			Commands->Reveal_Encyclopedia_Vehicle ( 31 );//sakura's comanche

			Commands->Enable_Cinematic_Freeze( obj, false );

			int id = Commands->Create_Conversation("M03CON010", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100010);
			Commands->Monitor_Conversation(obj, id);

			Commands->Set_Ash(0.15f, 3.0f, false);

			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), DOGFIGHT_ENDED, 0, 0.0f);
			Commands->Set_Clouds (1.0f, 1.0f, 20);

			/*int id = Commands->Create_Conversation("M03CON010", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100010);
			Commands->Monitor_Conversation(obj, id);*/

			Commands->Start_Timer(obj, this, 55.0, MESSAGE_DELAY);
			Commands->Start_Timer(obj, this, 10.0, ANNOUNCEMENT_DELAY);
			
			Commands->Start_Timer (obj, this, 0.1f, 1001);
			GameObject *temp = Commands->Find_Object(1144977);
			if (temp)
			{
				Commands->Send_Custom_Event(obj, temp, 100, 100, 0.0f);
			}
			Commands->Create_Object("Volcano Controller", Vector3(0,0,0));
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason /*complete_reason*/) override
	{
		if (action_id == 100010)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 310, 3, 0);
		}
	}

	void Timer_Expired (GameObject * obj, int timer_id) override
	{
		if (timer_id == 1001)
		{
			float rnd_val;
			int rnd_int_val;

			rnd_val = Commands->Get_Random(0.0f,1.0f) * 20;
			rnd_int_val = int(rnd_val);
			if (rnd_int_val == last)
			{
				++rnd_int_val %= 20;
			}

			if (rnd_int_val > 19) {
				rnd_int_val = 19;
			}
			switch (rnd_int_val)
			{
			case (0):
				{
					Commands->Create_Object ("LavaBall01", Vector3(0,0,0));
					break;
				}
			case (1):
				{
					Commands->Create_Object ("LavaBall02", Vector3(0,0,0));
					break;
				}
			case (2):
				{
					Commands->Create_Object ("LavaBall03", Vector3(0,0,0));
					break;
				}
			case (3):
				{
					Commands->Create_Object ("LavaBall04", Vector3(0,0,0));
					break;
				}
			case (4):
				{
					Commands->Create_Object ("LavaBall05", Vector3(0,0,0));
					break;
				}
			case (5):
				{
					Commands->Create_Object ("LavaBall06", Vector3(0,0,0));
					break;
				}
			case (6):
				{
					Commands->Create_Object ("LavaBall07", Vector3(0,0,0));
					break;
				}
			case (7):
				{
					Commands->Create_Object ("LavaBall08", Vector3(0,0,0));
					break;
				}
			case (8):
				{
					Commands->Create_Object ("LavaBall09", Vector3(0,0,0));
					break;
				}
			case (9):
				{
					Commands->Create_Object ("LavaBall10", Vector3(0,0,0));
					break;
				}
			case (10):
				{
					Commands->Create_Object ("LavaBall11", Vector3(0,0,0));
					break;
				}
			case (11):
				{
					Commands->Create_Object ("LavaBall12", Vector3(0,0,0));
					break;
				}
			case (12):
				{
					Commands->Create_Object ("LavaBall13", Vector3(0,0,0));
					break;
				}
			case (13):
				{
					Commands->Create_Object ("LavaBall14", Vector3(0,0,0));
					break;
				}
			case (14):
				{
					Commands->Create_Object ("LavaBall15", Vector3(0,0,0));
					break;
				}
			case (15):
				{
					Commands->Create_Object ("LavaBall16", Vector3(0,0,0));
					break;
				}
			case (16):
				{
					Commands->Create_Object ("LavaBall17", Vector3(0,0,0));
					break;
				}
			case (17):
				{
					Commands->Create_Object ("LavaBall18", Vector3(0,0,0));
					break;
				}
			case (18):
				{
					Commands->Create_Object ("LavaBall19", Vector3(0,0,0));
					break;
				}
			case (19):
				{
					Commands->Create_Object ("LavaBall20", Vector3(0,0,0));
					break;
				}
			}
			last = rnd_int_val;
			rnd_val = Commands->Get_Random(3.0f,5.0f) + 2.0f;
			Commands->Start_Timer (obj, this, rnd_val, 1001);
		}
	}
};

DECLARE_SCRIPT(M03_CommCenter_SateliteDish_Controller_JDG, "")
{
	int start_sounds;
	int stop_sounds;
	int play_sounds;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( start_sounds, 1 );
		SAVE_VARIABLE( stop_sounds, 2 );
		SAVE_VARIABLE( play_sounds, 3 );
	}

	void Created( GameObject * obj ) override
	{
		start_sounds		=	100;
		stop_sounds		=	101;
		play_sounds		=	102;
		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_sounds, 0 );
		}
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == stop_sounds)
		{
		}

		if (param == start_sounds)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sounds, 0 );
			}
		}
	
		if (param == play_sounds)
		{
			const char *soundName = "Satelite Dish Moving Twiddler";
			Vector3 soundPosition (-110.26f, 41.30f, 19.37f);

			Commands->Create_Sound ( soundName, soundPosition, obj );
			
			float delayTimer = Commands->Get_Random ( 0, 3 );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sounds, delayTimer );
			}
		}
	}
};

/*DECLARE_SCRIPT(M03_Refinery_Crusher_Controller_JDG, "")
{
	int start_sounds;
	int stop_sounds;
	int play_sounds;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( start_sounds, 1 );
		SAVE_VARIABLE( stop_sounds, 2 );
		SAVE_VARIABLE( play_sounds, 3 );
	}

	void Created( GameObject * obj ) override
	{
		start_sounds		=	100;
		stop_sounds		=	101;
		play_sounds		=	102;
		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_sounds, 0 , 0.0f);
		}
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if (param == stop_sounds)
		{
		}

		if (param == start_sounds)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sounds, 0 , 0.0f);
			}
		}
	
		if (param == play_sounds)
		{
			char *soundName = "Refinery Crusher Twiddler";
			Vector3 soundPosition (-179.60f, -2.03f, 3.42f);

			Commands->Create_Sound ( soundName, soundPosition, obj );
			
			float delayTimer = Commands->Get_Random ( 0, 5 );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sounds, delayTimer , 0.0f);
			}
			
		}
	}
};*/


DECLARE_SCRIPT(M03_Ambient_Birdcall_Controller_JDG, "")
{
	int start_birdcalls;
	int stop_birdcalls;
	int pick_birdcall;
	int play_birdcall;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( start_birdcalls, 1 );
		SAVE_VARIABLE( stop_birdcalls, 2 );
		SAVE_VARIABLE( pick_birdcall, 3 );
		SAVE_VARIABLE( play_birdcall, 4 );
	}

	void Created( GameObject * obj ) override
	{
		start_birdcalls		=	100;
		stop_birdcalls		=	101;
		play_birdcall		=	102;
		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_birdcalls, 0 );
		}
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == stop_birdcalls)
		{
		}

		if (param == start_birdcalls)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_birdcall, 0 );
			}
		}
	
		if (param == play_birdcall)
		{
			const char *soundName = "Birdcall Twiddler";
			Vector3 soundPosition;

			GameObject * star = Commands->Get_A_Star (Vector3(0.0f,0.0f,0.0f));
			if ( star != NULL ) 
			{
				Vector3 playerPosition = Commands->Get_Position ( star );

				float x_factor = Commands->Get_Random(-15,15);
				float y_factor = Commands->Get_Random(-15,15);
				float z_factor = Commands->Get_Random(5,15);

				soundPosition.X = playerPosition.X + x_factor;
				soundPosition.Y = playerPosition.Y + y_factor;
				soundPosition.Z = playerPosition.Z + z_factor;

				Commands->Create_Sound ( soundName, soundPosition, obj );
			}

			float delayTimer = Commands->Get_Random ( 0, 15 );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_birdcall, delayTimer );
			}
		}
	}

	void Entered( GameObject * obj, GameObject * enterer ) override
	{
		if ( Commands->Is_A_Star( enterer ))
		{
			Commands->Destroy_Object ( obj );
		}
	}
};
/*
01-I000E	"Core temperture fluctuating."
01-I002E	"Extended exposure to core environment is hazardous."
01-I004E	"Power output exceeding optimal levels."
01-I006E	"Radiation levels fluctuating."
01-I008E	"Radiation hazard. Proper safety equipment is required."
01-I010E	"If the decontamination shower does not function, contact a technician immediately."
01-I012E	"Tertiary coolant system malfunctioning.  Dispatch technician immediately."
01-I014E	"Comm Center power demands have fallen.  Diverting power to secondary grid."
01-I016E	"Power production levels wavering."
01-I018E	"Critical failure potential increasing. Reallocate available engineers."
01-I020E	"Tertiary grid demands increasing; diverting surplus power."
01-I022E	"Proper Identification must be worn at all times."
01-I024E	"Secondary power grid has been taken offline. Tertiary grid surplus being redirected."
01-I026E	"Power core radiation levels vacillating."
01-I050E	"Do you know someone who would make a positive addition to the Brotherhood? Now they can enlist online at 'WWW dot BrotherhoodRecruitment dot Nod' "
01-I062E	"Immediately report the presence of visceroids to your supervisor."
01-I066E	"Workers found loitering in this area will be terminated."
01-I076E	"Hazmat suits are required for your safety."
01-I078E	"Please do not inhale decontamination agents."

*/


DECLARE_SCRIPT(M03_Announce_PowerPlant_Controller_JDG, "")
{
	int total_number_of_sounds;
	float announce_delay_min;
	float announce_delay_max;
	int start_announcements;
	int stop_announcements;
	int pick_sound;
	int play_sound;
	int play_klaxon;

	int sound;
	int klaxon;

	Vector3 spkr_1_spot;
	Vector3 spkr_2_spot;
	Vector3 spkr_3_spot;
	Vector3 spkr_4_spot;
	Vector3 spkr_5_spot;
	Vector3 spkr_6_spot;
	Vector3 spkr_7_spot;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( total_number_of_sounds, 1 );
		SAVE_VARIABLE( announce_delay_min, 2 );
		SAVE_VARIABLE( announce_delay_max, 3 );
		SAVE_VARIABLE( start_announcements, 4 );
		SAVE_VARIABLE( stop_announcements, 5 );
		SAVE_VARIABLE( pick_sound, 6 );
		SAVE_VARIABLE( play_sound, 7 );
		SAVE_VARIABLE( play_klaxon, 8 );
		SAVE_VARIABLE( sound, 9 );
		SAVE_VARIABLE( klaxon, 10 );
		SAVE_VARIABLE( spkr_1_spot, 11 );
		SAVE_VARIABLE( spkr_2_spot, 12 );
		SAVE_VARIABLE( spkr_3_spot, 13 );
		SAVE_VARIABLE( spkr_4_spot, 14 );
		SAVE_VARIABLE( spkr_5_spot, 15 );
		SAVE_VARIABLE( spkr_6_spot, 16 );
		SAVE_VARIABLE( spkr_7_spot, 17 );
	}


	void Created( GameObject * obj ) override
	{
		total_number_of_sounds	=	15;
		announce_delay_min		=	20.0;
		announce_delay_max		=	30.0;
		start_announcements		=	20;
		stop_announcements		=	21;
		pick_sound				=	22;
		play_sound				=	23;
		play_klaxon				=	24;

		spkr_1_spot.Set(-60.55f,13.55f,-9.69f);
		spkr_2_spot.Set(-65.74f,15.17f,-8.71f);
		spkr_3_spot.Set(-63.75f,22.49f,-7.81f);
		spkr_4_spot.Set(-63.00f,22.05f,-.70f);
		spkr_5_spot.Set(-75.75f,28.92f,-14.77f);
		spkr_6_spot.Set(-64.98f,20.41f,-13.62f);
		spkr_7_spot.Set(-58.05f,7.45f,-14.20f);


		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_announcements, 0 );
		}
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		

		if (param == stop_announcements)
		{
		}

		else if (param == start_announcements)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, 0 );
			}
		}

		else if (param == play_klaxon)
		{
			const auto& klaxonNames = Mission3Var::KlaxonNames;
			const char *klaxonName = klaxonNames[klaxon];

			Commands->Create_Sound ( klaxonName, spkr_1_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_2_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_3_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_4_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_5_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_6_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_7_spot, obj );

			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sound, 1.25 );
			}
			
		}

		else if (param == play_sound)
		{
			static constexpr const char *sounds[19] = {
				"01-i000e",
				"01-i002e",
				"01-i004e",
				"01-i006e",
				"01-i008e",
				"01-i010e",
				"01-i012e",
				"01-i014e",
				"01-i016e",
				"01-i018e",
				"01-i020e",
				"01-i024e",
				"01-i026e",
				"01-i050e",
				"01-i062e",
				"01-i066e",
				"01-i076e",
				"01-i078e",
				"01-i022e"
			};
			const char *soundName;
			soundName = sounds[sound];
			Commands->Create_Sound ( soundName, spkr_1_spot, obj );
			Commands->Create_Sound ( soundName, spkr_2_spot, obj );
			Commands->Create_Sound ( soundName, spkr_3_spot, obj );
			Commands->Create_Sound ( soundName, spkr_4_spot, obj );
			Commands->Create_Sound ( soundName, spkr_5_spot, obj );
			Commands->Create_Sound ( soundName, spkr_6_spot, obj );
			Commands->Create_Sound ( soundName, spkr_7_spot, obj );

		
			float delayTimer = Commands->Get_Random ( announce_delay_min, announce_delay_max );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, delayTimer );
			}
			
		}

		else if (param == pick_sound)
		{

			
			float lineNumber = Commands->Get_Random ( 0.5f, total_number_of_sounds+0.5);

			if ((lineNumber >= 0.5) && (lineNumber < 1.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i000e";
				sound = 0;
				//01-I000E	"Core temperture fluctuating."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 1.5) && (lineNumber < 2.5))
			{
				//soundName = "01-i002e";
				sound = 1;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 2.5) && (lineNumber < 3.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i004e";
				sound = 2;
				//01-I004E	"Power output exceeding optimal levels."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 3.5) && (lineNumber < 4.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i006e";
				sound = 3;
				//01-I006E	"Radiation levels fluctuating."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 4.5) && (lineNumber < 5.5))
			{
				//soundName = "01-i008e";
				sound = 4;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 5.5) && (lineNumber < 6.5))
			{
				//soundName = "01-i010e";
				sound = 5;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 6.5) && (lineNumber < 7.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i012e";
				sound = 6;
				//01-I012E	"Tertiary coolant system malfunctioning.  Dispatch technician immediately."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 7.5) && (lineNumber < 8.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i014e";
				sound = 7;
				//01-I014E	"Comm Center power demands have fallen.  Diverting power to secondary grid."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 8.5) && (lineNumber < 9.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i016e";
				sound = 8;
				//01-I016E	"Power production levels wavering."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 9.5) && (lineNumber < 10.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i018e";
				sound = 9;
				//01-I018E	"Critical failure potential increasing. Reallocate available engineers."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 10.5) && (lineNumber < 11.5))
			{
				//soundName = "01-i020e";
				sound = 10;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 11.5) && (lineNumber < 12.5))
			{
				//soundName = "01-i024e";
				sound = 11;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 12.5) && (lineNumber < 13.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i026e";
				sound = 12;
				//01-I026E	"Power core radiation levels vacillating."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 13.5) && (lineNumber < 14.5))
			{
				//soundName = "01-i050e";
				sound = 13;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
			else if ((lineNumber >= 14.5) && (lineNumber < 15.5))
			{
				//soundName = "01-i062e";
				sound = 14;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
			else if ((lineNumber >= 15.5) && (lineNumber < 16.5))
			{
				//soundName = "01-i066e";
				sound = 15;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
			else if ((lineNumber >= 16.5) && (lineNumber < 17.5))
			{
				//soundName = "01-i076e";
				sound = 16;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
			else if ((lineNumber >= 17.5) && (lineNumber < 18.5))
			{
				//soundName = "01-i078e";
				sound = 17;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else 
			{
				//soundName = "01-i022e";
				sound = 18;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
		}
	}

	void Killed( GameObject * /*obj*/, GameObject * /*killer*/ ) override
	{
		//soundName = "Klaxon Loop";

		//Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );		
	}
	

};

/*
The following is a list of all level 3 refinery related dialogs
Line #		Dialog
01-i010E	"If the decontamination shower does not function, contact a technician immediately."
01-i022E	"Proper Identification must be worn at all times."
01-i032E	"A visceroid has been spotted in Tiberium Field iota. Containment team en route."
01-i034E	"Tiberium value is increasing Worldwide."
01-i052E	"The yellow zone is for harvester unloading only. There is no parking in the yellow zone."
01-i054E	"Harvester approaching. Please clear docking bay at once."  Only plays when harvester is approaching.
01-i056E	"Harvester now unloading"
01-i058E	"Smoking is not permitted within the refinery."
01-i060E	"Leaking barrels are a health hazard. Report any faulty containment vessels to your supervisor immediately."
01-i062E	"Immediately report the presence of visceroids to your supervisor."
01-i064E	"Do not discharge weapons near containment barrels."
01-i066E	"Workers found loitering in this area will be terminated."
01-i068E	"For your safety, avoid all moving parts."
01-i070E	"Supply trucks have the right of way."
01-i072E	"Tiberium field omega has decreased in size. Changing harvester target to facilitate growth."
01-i074E	"Additional tests are required for current Tiberium batch. Highest priority."
01-i076E	"Hazmat suits are required for your safety."
01-i078E	"Please do not inhale decontamination agents."
01-i080E	"Quarternary gas vacuum has malfunctioned. Backup compressors are now on line."
01-i082E	"Blockage in left ventricle of secondary crushing unit."
01-i084E	"Refinery reagents are a biohazard. Report all leaks immediately."
01-i086E	"Hydroxyl levels at supersaturation.  Venting protocols initiated."
01-i088E	"Uranium fuel levels nominal."
01-i090E	"Smelting furnace fully operational."
01-i092E	"SPF 128 required while working under the Ultra Violet lights."
01-i094E	"Polarized safety goggles are mandatory while working under the Ultra Violet lights."
01-i096E	"Distiller fumes are toxic. Avoid inhaling any gasses in this area."
01-i098E	"Tiberium only to be distilled in this area."
*/

DECLARE_SCRIPT(M03_Announce_Refinery_Controller_JDG, "")
{
	int total_number_of_sounds;
	float announce_delay_min;
	float announce_delay_max;
	int start_announcements;
	int stop_announcements;
	int pick_sound;
	int play_sound;
	int play_klaxon;

	int sound;
	int klaxon;

	Vector3 spkr_1_spot;
	Vector3 spkr_2_spot;
	Vector3 spkr_3_spot;
	Vector3 spkr_4_spot;
	Vector3 spkr_5_spot;
	Vector3 spkr_6_spot;
	Vector3 spkr_7_spot;
	Vector3 spkr_8_spot;
	Vector3 spkr_9_spot;
	Vector3 spkr_10_spot;
	Vector3 spkr_11_spot;
	Vector3 spkr_12_spot;
	Vector3 spkr_13_spot;
	Vector3 spkr_14_spot;
	Vector3 spkr_15_spot;
	Vector3 spkr_16_spot;
	Vector3 spkr_17_spot;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( total_number_of_sounds, 1 );
		SAVE_VARIABLE( announce_delay_min, 2 );
		SAVE_VARIABLE( announce_delay_max, 3 );
		SAVE_VARIABLE( start_announcements, 4 );
		SAVE_VARIABLE( stop_announcements, 5 );
		SAVE_VARIABLE( pick_sound, 6 );
		SAVE_VARIABLE( play_sound, 7 );
		SAVE_VARIABLE( play_klaxon, 8 );
		SAVE_VARIABLE( sound, 9 );
		SAVE_VARIABLE( klaxon, 10 );
		SAVE_VARIABLE( spkr_1_spot, 11 );
		SAVE_VARIABLE( spkr_2_spot, 12 );
		SAVE_VARIABLE( spkr_3_spot, 13 );
		SAVE_VARIABLE( spkr_4_spot, 14 );
		SAVE_VARIABLE( spkr_5_spot, 15 );
		SAVE_VARIABLE( spkr_6_spot, 16 );
		SAVE_VARIABLE( spkr_7_spot, 17 );
		SAVE_VARIABLE( spkr_8_spot, 18 );
		SAVE_VARIABLE( spkr_8_spot, 19 );
		SAVE_VARIABLE( spkr_10_spot, 20 );
		SAVE_VARIABLE( spkr_11_spot, 21 );
		SAVE_VARIABLE( spkr_12_spot, 22 );
		SAVE_VARIABLE( spkr_13_spot, 23 );
		SAVE_VARIABLE( spkr_14_spot, 24 );
		SAVE_VARIABLE( spkr_15_spot, 25 );
		SAVE_VARIABLE( spkr_16_spot, 26 );
		SAVE_VARIABLE( spkr_17_spot, 27 );
	}

	void Created( GameObject * obj ) override
	{
		total_number_of_sounds	=	28;
		announce_delay_min		=	20.0;
		announce_delay_max		=	30.0;
		start_announcements		=	20;
		stop_announcements		=	21;
		pick_sound				=	22;
		play_sound				=	23;
		play_klaxon				=	24;

		spkr_1_spot.Set(-156.3f,28.40f,-10.07f);
		spkr_2_spot.Set(-172.19f,28.91f,-10.08f);
		spkr_3_spot.Set(-144.46f,17.18f,-10.0f);
		spkr_4_spot.Set(-136.33f,1.47f,-10.63f);
		spkr_5_spot.Set(-139.38f,-14.86f,-10.79f);
		spkr_6_spot.Set(-146.76f,-27.75f,-10.24f);
		spkr_7_spot.Set(-134.95f,-33.21f,-10.70f);
		spkr_8_spot.Set(-181.62f,16.33f,-10.65f);
		spkr_9_spot.Set(-180.04f,3.15f,-10.16f);
		spkr_10_spot.Set(-178.31f,-13.79f,-10.84f);
		spkr_11_spot.Set(-177.85f,-30.11f,-10.99f);
		spkr_12_spot.Set(-168.09f,-33.71f,-10.01f);
		spkr_13_spot.Set(-156.28f,-39.59f,-10.43f);
		spkr_14_spot.Set(-160.06f,-18.86f,-10.99f);
		spkr_15_spot.Set(-143.87f,-1.17f,-10.99f);
		spkr_16_spot.Set(-159.78f,-1.17f,-10.99f);
		spkr_17_spot.Set(-166.61f,11.61f,-10.99f);

		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_announcements, 0 );
		}
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == stop_announcements)
		{
		}

		else if (param == start_announcements)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, 0 );
			}
		}

		else if (param == play_klaxon)
		{
			const auto& klaxonNames = Mission3Var::KlaxonNames;
			const char *klaxonName = klaxonNames[klaxon];

			Commands->Create_Sound ( klaxonName, spkr_1_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_2_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_3_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_4_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_5_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_6_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_7_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_8_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_9_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_10_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_11_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_12_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_13_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_14_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_15_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_16_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_17_spot, obj );

			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sound, 1.5 );
			}
		}

		else if (param == play_sound)
		{
			static constexpr const char *sounds[28] = {
				"01-i010E",
				"01-i022E",
				"01-i032E",
				"01-i034E",
				"01-i052E",
				"01-i054E",
				"01-i056E",
				"01-i058E",
				"01-i060E",
				"01-i062E",
				"01-i064E",
				"01-i066E",
				"01-i068E",
				"01-i070E",
				"01-i072E",
				"01-i074E",
				"01-i076E",
				"01-i078E",
				"01-i080E",
				"01-i082E",
				"01-i084E",
				"01-i086E",
				"01-i088E",
				"01-i090E",
				"01-i092E",
				"01-i094E",
				"01-i096E",
				"01-i098E"
			};

			const char *soundName = sounds[sound];

			Commands->Create_Sound ( soundName, spkr_1_spot, obj );
			Commands->Create_Sound ( soundName, spkr_2_spot, obj );
			Commands->Create_Sound ( soundName, spkr_3_spot, obj );
			Commands->Create_Sound ( soundName, spkr_4_spot, obj );
			Commands->Create_Sound ( soundName, spkr_5_spot, obj );
			Commands->Create_Sound ( soundName, spkr_6_spot, obj );
			Commands->Create_Sound ( soundName, spkr_7_spot, obj );
			Commands->Create_Sound ( soundName, spkr_8_spot, obj );
			Commands->Create_Sound ( soundName, spkr_9_spot, obj );
			Commands->Create_Sound ( soundName, spkr_10_spot, obj );
			Commands->Create_Sound ( soundName, spkr_11_spot, obj );
			Commands->Create_Sound ( soundName, spkr_12_spot, obj );
			Commands->Create_Sound ( soundName, spkr_13_spot, obj );
			Commands->Create_Sound ( soundName, spkr_14_spot, obj );
			Commands->Create_Sound ( soundName, spkr_15_spot, obj );
			Commands->Create_Sound ( soundName, spkr_16_spot, obj );
			Commands->Create_Sound ( soundName, spkr_17_spot, obj );

		
			float delayTimer = Commands->Get_Random ( announce_delay_min, announce_delay_max );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, delayTimer );
			}
		}

		else if (param == pick_sound)
		{

			float lineNumber = Commands->Get_Random ( 0.5f, total_number_of_sounds+0.5);

			if ((lineNumber >= 0.5) && (lineNumber < 1.5))
			{
				//soundName = "01-i074e";
				sound = 0;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 1.5) && (lineNumber < 2.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i032e";
				sound = 1;
				//01-i032E	"A visceroid has been spotted in Tiberium Field iota. Containment team en route."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 2.5) && (lineNumber < 3.5))
			{
				//soundName = "01-i034e";
				sound = 2;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 3.5) && (lineNumber < 4.5))
			{
				//soundName = "01-i052e";
				sound = 3;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 4.5) && (lineNumber < 5.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i054e";
				sound = 4;
				//01-i054E	"Harvester approaching. Please clear docking bay at once."  Only plays when harvester is approaching.

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 5.5) && (lineNumber < 6.5))
			{
				//soundName = "01-i056e";
				sound = 5;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 6.5) && (lineNumber < 7.5))
			{
				//soundName = "01-i058e";
				sound = 6;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 7.5) && (lineNumber < 8.5))
			{
				//soundName = "01-i060e";
				sound = 7;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 8.5) && (lineNumber < 9.5))
			{
				//soundName = "01-i062e";
				sound = 8;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 9.5) && (lineNumber < 10.5))
			{
				//soundName = "01-i064e";
				sound = 9;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 10.5) && (lineNumber < 11.5))
			{
				//soundName = "01-i066e";
				sound = 10;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 11.5) && (lineNumber < 12.5))
			{
				//soundName = "01-i068e";
				sound = 11;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 12.5) && (lineNumber < 13.5))
			{
				//soundName = "01-i070e";
				sound = 12;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 13.5) && (lineNumber < 14.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i072e";
				sound = 13;
				//01-i072E	"Tiberium field omega has decreased in size. Changing harvester target to facilitate growth."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 14.5) && (lineNumber < 15.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i074e";
				sound = 14;
				//01-i074E	"Additional tests are required for current Tiberium batch. Highest priority."
				
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 15.5) && (lineNumber < 16.5))
			{
				//soundName = "01-i076e";
				sound = 15;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 16.5) && (lineNumber < 17.5))
			{
				//soundName = "01-i078e";
				sound = 16;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 17.5) && (lineNumber < 18.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i080e";
				sound = 17;
				//01-i080E	"Quarternary gas vacuum has malfunctioned. Backup compressors are now on line."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 18.5) && (lineNumber < 19.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i082e";
				sound = 18;
				//01-i082E	"Blockage in left ventricle of secondary crushing unit."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 19.5) && (lineNumber < 20.5))
			{
				//soundName = "01-i084e";
				sound = 19;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				};
			}

			else if ((lineNumber >= 20.5) && (lineNumber < 21.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i086e";
				sound = 20;
				//01-i086E	"Hydroxyl levels at supersaturation.  Venting protocols initiated."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 21.5) && (lineNumber < 22.5))
			{
				//soundName = "01-i088e";
				sound = 21;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 22.5) && (lineNumber < 23.5))
			{
				//soundName = "01-i090e";
				sound = 22;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 23.5) && (lineNumber < 24.5))
			{
				//soundName = "01-i092e";
				sound = 23;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 24.5) && (lineNumber < 25.5))
			{
				//soundName = "01-i094e";
				sound = 24;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 25.5) && (lineNumber < 26.5))
			{
				//soundName = "01-i096e";
				sound = 25;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 26.5) && (lineNumber < 27.5))
			{
				//soundName = "01-i098e";
				sound = 26;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else 
			{
				//soundName = "01-i022e";
				sound = 27;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
		}
	}

	void Killed( GameObject * /*obj*/, GameObject * /*killer*/ ) override
	{
		//soundName = "Klaxon Loop";

		//Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
		
		Commands->Destroy_Object( Commands->Find_Object ( 600067 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600068 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600069 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600070 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600071 ) );
	}
};
/*
01-I022E	"Proper Identification must be worn at all times."
01-I028E	"All coded messages must be sent on designated security channel alpha."
01-I030E	"In accordance with Brotherhood directive AC-MBM, all messages are monitored."
01-I032E	"A visceroid has been spotted in Tiberium Field iota. Containment team en route."
01-I034E	"Tiberium value is increasing Worldwide."
01-I036E	"Project 'Ezekiel's Cape' has passed initial tests.  Prototypes are now in production."
01-I038E	"Colonel Shepard's personal aide has been located in Washington D.C.  Aquisition team en route."
01-I040E	"Anamolous EVA signal detected.  Reconnaissance force is being dispatched."
01-I042E	"World wide public opinion of the Brotherhood is on the rise."
01-I044E	"Refinery technicians have failed to report in.  Investigation team en route."
01-I046E	"Possible EVA intrusion in message squirt Alpha. Switching to Beta channels."
01-I048E	"Incoming transmission for Captain Jones.  Captain Jones please report to a secured terminal."
01-I050E	"Do you know someone who would make a positive addition to the Brotherhood? Now they can enlist online at 'WWW dot BrotherhoodRecruitment dot Nod' "
01-I062E	"Immediately report the presence of visceroids to your supervisor."
01-I066E	"Workers found loitering in this area will be terminated."
*/

DECLARE_SCRIPT(M03_Announce_CommCenter_Controller_JDG, "")
{
	int total_number_of_sounds;
	float announce_delay_min;
	float announce_delay_max;
	int start_announcements;
	int stop_announcements;
	int pick_sound;
	int play_sound;
	int play_klaxon;

	int sound;
	int klaxon;

	Vector3 spkr_1_spot;
	Vector3 spkr_2_spot;
	Vector3 spkr_3_spot;
	Vector3 spkr_4_spot;
	Vector3 spkr_5_spot;
	Vector3 spkr_6_spot;
	Vector3 spkr_7_spot;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( total_number_of_sounds, 1 );
		SAVE_VARIABLE( announce_delay_min, 2 );
		SAVE_VARIABLE( announce_delay_max, 3 );
		SAVE_VARIABLE( start_announcements, 4 );
		SAVE_VARIABLE( stop_announcements, 5 );
		SAVE_VARIABLE( pick_sound, 6 );
		SAVE_VARIABLE( play_sound, 7 );
		SAVE_VARIABLE( play_klaxon, 8 );
		SAVE_VARIABLE( sound, 9 );
		SAVE_VARIABLE( klaxon, 10 );
		SAVE_VARIABLE( spkr_1_spot, 11 );
		SAVE_VARIABLE( spkr_2_spot, 12 );
		SAVE_VARIABLE( spkr_3_spot, 13 );
		SAVE_VARIABLE( spkr_4_spot, 14 );
		SAVE_VARIABLE( spkr_5_spot, 15 );
		SAVE_VARIABLE( spkr_6_spot, 16 );
		SAVE_VARIABLE( spkr_7_spot, 17 );
	}

	void Created( GameObject * obj ) override
	{
		total_number_of_sounds	=	15;
		announce_delay_min		=	20.0;
		announce_delay_max		=	30.0;
		start_announcements		=	20;
		stop_announcements		=	21;
		pick_sound				=	22;
		play_sound				=	23;
		play_klaxon				=	24;

		spkr_1_spot.Set(-104.35f,47.83f,-3.54f);
		spkr_2_spot.Set(116.1f,46.42f,-3.15f);
		spkr_3_spot.Set(-117.04f,41.42f,-5.37f);
		spkr_4_spot.Set(-127.57f,65.42f,-5.75f);
		spkr_5_spot.Set(-121.95f,59.32f,-4.91f);
		spkr_6_spot.Set(-117.48f,45.83f,-3.87f);
		spkr_7_spot.Set(-111.25f,43.69f,-2.27f);


		if (obj) {
			Commands->Send_Custom_Event( obj, obj, 0, start_announcements, 0 );
		}
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == stop_announcements)
		{
		}

		else if (param == start_announcements)
		{
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, 0 );
			}
		}

		else if (param == play_klaxon)
		{
			const auto& klaxonNames = Mission3Var::KlaxonNames;
			const char* klaxonName = klaxonNames[klaxon];

			Commands->Create_Sound ( klaxonName, spkr_1_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_2_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_3_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_4_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_5_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_6_spot, obj );
			Commands->Create_Sound ( klaxonName, spkr_7_spot, obj );

			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, play_sound, 1.25 );
			}
		}

		else if (param == play_sound)
		{
			static constexpr const char *sounds[15] = {
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

			Commands->Create_Sound ( soundName, spkr_1_spot, obj );
			Commands->Create_Sound ( soundName, spkr_2_spot, obj );
			Commands->Create_Sound ( soundName, spkr_3_spot, obj );
			Commands->Create_Sound ( soundName, spkr_4_spot, obj );
			Commands->Create_Sound ( soundName, spkr_5_spot, obj );
			Commands->Create_Sound ( soundName, spkr_6_spot, obj );
			Commands->Create_Sound ( soundName, spkr_7_spot, obj );

		
			float delayTimer = Commands->Get_Random ( announce_delay_min, announce_delay_max );
			if (obj) {
				Commands->Send_Custom_Event( obj, obj, 0, pick_sound, delayTimer );
			}
		}

		else if (param == pick_sound)
		{

			float lineNumber = Commands->Get_Random ( 0.5f, total_number_of_sounds+0.5);

			if ((lineNumber >= 0.5) && (lineNumber < 1.5))
			{
				//soundName = "01-i066e";
				sound = 0;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 1.5) && (lineNumber < 2.5))
			{
				//soundName = "01-i028e";
				sound = 1;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 2.5) && (lineNumber < 3.5))
			{
				//soundName = "01-i030e";
				sound = 2;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 3.5) && (lineNumber < 4.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i032e";
				sound = 3;
				//01-I032E	"A visceroid has been spotted in Tiberium Field iota. Containment team en route."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 4.5) && (lineNumber < 5.5))
			{
				//soundName = "01-i034e";
				sound = 4;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 5.5) && (lineNumber < 6.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i036e";
				sound = 5;
				//01-I036E	"Project 'Ezekiel's Cape' has passed initial tests.  Prototypes are now in production."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 6.5) && (lineNumber < 7.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i038e";
				sound = 6;
				//01-I038E	"Colonel Shepard's personal aide has been located in Washington D.C.  Aquisition team en route."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 7.5) && (lineNumber < 8.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i040e";
				sound = 7;
				//01-I040E	"Anamolous EVA signal detected.  Reconnaissance force is being dispatched."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 8.5) && (lineNumber < 9.5))
			{
				//soundName = "01-i042e";
				sound = 8;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 9.5) && (lineNumber < 10.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i044e";
				sound = 9;
				//01-I044E	"Refinery technicians have failed to report in.  Investigation team en route."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 10.5) && (lineNumber < 11.5))
			{
				//klaxonName = "Klaxon Warning";
				klaxon = 0;
				//soundName = "01-i046e";
				sound = 10;
				//01-I046E	"Possible EVA intrusion in message squirt Alpha. Switching to Beta channels."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 11.5) && (lineNumber < 12.5))
			{
				//klaxonName = "Klaxon Info";
				klaxon = 1;
				//soundName = "01-i048e";
				sound = 11;
				//01-I048E	"Incoming transmission for Captain Jones.  Captain Jones please report to a secured terminal."

				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_klaxon, 0 );
				}
			}

			else if ((lineNumber >= 12.5) && (lineNumber < 13.5))
			{
				//soundName = "01-i050e";
				sound = 12;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else if ((lineNumber >= 13.5) && (lineNumber < 14.5))
			{
				//soundName = "01-i062e";
				sound = 13;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}

			else 
			{
				//soundName = "01-i022e";
				sound = 14;
				if (obj) {
					Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
				}
			}
		}
	}

	void Killed( GameObject * /*obj*/, GameObject * /*killer*/ ) override
	{
		//soundName = "Klaxon Loop";

		//Commands->Send_Custom_Event( obj, obj, 0, play_sound, 0 );
		
		Commands->Destroy_Object( Commands->Find_Object ( 600042 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600056 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600057 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600058 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600059 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600060 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600061 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600062 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600063 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600064 ) );
		Commands->Destroy_Object( Commands->Find_Object ( 600065 ) );
	}
	

};

//Grants initial weapons and key needed for Level 3
DECLARE_SCRIPT(M03_Initial_Powerups, "")
{
	enum 
	{
		POWERUP_TIMER,
	};
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 0.5f, POWERUP_TIMER);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == POWERUP_TIMER)
		{
			GameObject *star = Commands->Get_A_Star(Commands->Get_Position(obj));
			if (star)
			{
				Commands->Give_PowerUp(star, "Shotgun Weapon 1 Clip PU", false);
				Commands->Give_PowerUp(star, "Sniper Weapon 1 Clip PU", false);
				Commands->Give_PowerUp(star, "Remote Mine Weapon 1 Clip PU", false);
				Commands->Grant_Key(star, 5, true);
				//Commands->Grant_Key(star, 1, true);
				char params[20];
				sprintf(params, "%d", Commands->Get_ID(obj));
				Commands->Attach_Script(star, "M03_Commando_Script", params);
			}
			else
			{
				Commands->Start_Timer(obj, this, 1.0f, POWERUP_TIMER);
			}
		}	
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 12176) && (param == 12176))
		{
			Commands->Start_Timer(obj, this, 2.0f, POWERUP_TIMER);
		}
	}
};

DECLARE_SCRIPT(M03_Commando_Script, "Controller_ID:int")
{
	bool has_escort;
	int follower_id;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(has_escort, 1);
		SAVE_VARIABLE(follower_id, 2);
	}
	
	void Created(GameObject * /*obj*/) override
	{
		has_escort = false;
		follower_id = 0;
	}
	
	void Sound_Heard(GameObject * /*obj*/, const CombatSound & sound) override
	{
		if (sound.Type >= 991)
		{
			//Commands->Shake_Camera(sound.Position, 25, 0.2f, 1.5f);
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 3000)
		{
			int *occupied = (int *)param;
			if (occupied != NULL)
			{
				(*occupied) = has_escort ? 1 : 0;
			}
			if (has_escort && (Commands->Get_ID(sender) == follower_id))
			{
				has_escort = false;
				follower_id = 0;
				if (occupied != NULL)
				{
					(*occupied) = -1;
				}
			}
			else if (!has_escort)
			{
				has_escort = true;
				follower_id = Commands->Get_ID(sender);
			}
			
		}

		if (type == 3100 && param == 3100)
		{
			if (Commands->Get_ID(sender) == follower_id)
			{
				has_escort = false;
				follower_id = 0;
			}
		}
	}
	
	void Destroyed(GameObject * obj) override
	{
		GameObject *con = Commands->Find_Object(Get_Int_Parameter("Controller_ID"));
		if (con)
		{
			Commands->Send_Custom_Event(obj, con, 12176, 12176, 0.0f);
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		if (has_escort)
		{
			GameObject *escort = Commands->Find_Object(follower_id);
			if (escort)
			{
				Commands->Send_Custom_Event(obj, escort, 1001, 1001, 0.0f);
			}
		}
		
		// HACK
		GameObject *tailgun_zone = Commands->Find_Object(1141168);
		if (tailgun_zone)
		{
			Commands->Send_Custom_Event(obj, tailgun_zone, 300, 300, 0.0f);
		}
	}
};

//Moves the real Commando into position after intro cinematic
DECLARE_SCRIPT(M03_Move_Commando_To_Start, "")
{
	enum {M03_COMMANDO_MOVE_TIMER};
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, M03_COMMANDO_MOVE_TIMER);
	}
	
	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == M03_COMMANDO_MOVE_TIMER)
		{
			GameObject *star = Commands->Get_A_Star(Commands->Get_Position(obj));
			Vector3 pos = Vector3(-328.09f, 75.65f, 4.25f);
			Commands->Set_Position(star, pos);
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 100) && (param == 100))
		{
			GameObject *star = Commands->Get_A_Star(Commands->Get_Position(obj));
			Vector3 pos = Vector3(-118.12f,-120.12f,-0.72f);
			Commands->Set_Position(star, pos);
		}
	}
};

//Actually creates the GDI soldiers on the Chinook's SPAWNER bone, and attaches them to a slot
DECLARE_SCRIPT(M03_Chinook_Drop_Soldiers_GDI, "Controller_ID:int")
{
	int con_id;
	int count, count2;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( con_id, 1 );
		SAVE_VARIABLE( count, 2);
		SAVE_VARIABLE( count2, 2);
	}
	
	void Created(GameObject * /*obj*/) override
	{
		con_id = Get_Int_Parameter("Controller_ID");
		count = count2 = 0;
	}

	int Find_Location(int type)
	{
		return type - 4001;
	}
	
	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		GameObject *soldier;
		
		soldier = Commands->Find_Object(param);
		if (type > 4000)
		{
			if (soldier)
			{
				char params[80];
				sprintf(params, "%d,2000,%d", Find_Location(type), con_id);
				Commands->Attach_Script(soldier, "M03_Chinook_Spawned_Soldier_GDI", params);
				
				if (Find_Location(type) == 1)
				{
					char params2[80];
					sprintf(params2, "%d", count);
					(++count) %= 3;
					Commands->Attach_Script(soldier, "M03_Inlet_Soldier_GDI", params2);
				}
				if (Find_Location(type) == 0)
				{
					char params3[80];
					sprintf(params3, "%d", count2);
					(++count2) %= 3;
					Commands->Attach_Script(soldier, "M03_Beach_Soldier_GDI", params3);
				}
			}
		}
	}
};

DECLARE_SCRIPT(M03_Inlet_Soldier_GDI, "Number:int")
{
	void Created(GameObject * obj) override
	{
		int num = Get_Int_Parameter("Number");
		switch (num)
		{
		case 0: Commands->Set_Innate_Soldier_Home_Location(obj, Vector3(75.0f,-28.0f,2.001f), 5.0f);
			break;
		case 1: Commands->Set_Innate_Soldier_Home_Location(obj, Vector3(29.0f,-60.0f,2.634f), 5.0f);
			break;
		case 2: Commands->Start_Timer(obj, this, 10.0f, 0);
			break;
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == 50)
		{
			Commands->Start_Timer(obj, this, 10.0f, 0);
		}
	}
	
	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 40, 50);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = 1144691;
		Commands->Action_Goto(obj, params);
	}
};

DECLARE_SCRIPT(M03_Beach_Soldier_GDI, "Number:int")
{
	void Created(GameObject * obj) override
	{
		int num = Get_Int_Parameter("Number");
		switch (num)
		{
		case 0: Commands->Set_Innate_Soldier_Home_Location(obj, Vector3(-112.132f, -54.312f, 7.297f), 2.0f);
			break;
		case 1: Commands->Set_Innate_Soldier_Home_Location(obj, Vector3(-91.731f, -54.618f, 7.129f), 2.0f);
			break;
		case 2: Commands->Start_Timer(obj, this, 10.0f, 0);
			break;
		}
	}

	void Action_Complete(GameObject * /*obj*/, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == 50)
		{
	//		Commands->Start_Timer(obj, this, 10.0f, 0);
		}
	}
	
	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 40, 50);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = 1145037;
		Commands->Action_Goto(obj, params);
	}
};

//Keeps track of current GDI soldiers and brings in area-specific reinforcements when they 
//are gone.  Also tracks "Fortify Beachhead" objective status
DECLARE_SCRIPT(M03_Chinook_Reinforcements, "Beach_Preset:string, Inlet_Preset:string, Base_Preset:string, Trigger_Count:int")
{
	int current[3];
	bool active[3];
	int count;
	int beach_count;
	bool objective_completed;
	int base_count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current, 1 );
		SAVE_VARIABLE( active, 2 );
		SAVE_VARIABLE( count, 3 );
		SAVE_VARIABLE( beach_count, 4 );
		SAVE_VARIABLE( objective_completed, 5);
		SAVE_VARIABLE(base_count, 6);
	}

	void Created(GameObject * /*obj*/) override
	{	count = Get_Int_Parameter("Trigger_Count");
		beach_count = 0;
		base_count = 0;
		objective_completed = false;
		for (int x = 0; x <3; x++)
		{
			current[x] = 0;
			active[x] = true;
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 2000)
		{
			if ((param < 0) || (param > 2))
			{
				return;
			}
			current[param]++;

			if ((param == 0) && (current[0] < 3))
			{
				if (beach_count >= (3 - current[0]))
				{
					GameObject *objective_controller;
					objective_controller = Commands->Find_Object(1100004);
					if (objective_controller && !objective_completed)
					{						
						//Commands->Send_Custom_Event(obj, objective_controller, 301, 1, 0.0f);
						//objective_completed = true;
					}
				}
			}
			
			if ((current[param] >= count) && (active[param]) && Commands->Find_Object(1100003))
			{
				current[param] = 0;
				GameObject *con;
				Commands->Create_Sound("00-N180E", Vector3(0,0,0), obj);
				switch (param)
				{
				case 0:
				{/*const char *beach;
					beach = Get_Parameter("Beach_Preset");
					con = Commands->Create_Object(beach, Vector3(-84.609f, -91.188f, 1.0f));
					Commands->Set_Facing(con, 100.0f);*/

					GameObject *new_obj = Commands->Create_Object("Invisible_Object", Vector3(-84.609f, -91.188f, 1.0f));
					Commands->Set_Facing(new_obj, 190.0f);
					Commands->Attach_Script(new_obj, "Test_Cinematic", "X3I_TroopDrop1.txt");
				}

					break;
				case 1: const char *inlet;
					inlet = Get_Parameter("Inlet_Preset");
					con = Commands->Create_Object(inlet, Vector3(84.398f, -45.703f, 1.0f));
					Commands->Set_Facing(con, 100.0f);
					break;
				case 2: const char *base; 
					base = Get_Parameter("Base_Preset");
					con = Commands->Create_Object(base, Vector3(-94.822f, 15.727f, 10.0f));
					Commands->Set_Facing(con, -155.0f);
					break;
				}
			}
		}

		if (type == 3000)
		{
			if ((param  < 0) || (param > 2))
				return;
			active[param] = false;
		}

		if (type == 4000 && param == 4000)
		{
			base_count++;
			if (base_count >= 3)
			{
				base_count = 0;
				current[2] = 2;
				Commands->Send_Custom_Event(obj, obj, 2000, 2, 0.0f);
			}
		}

		if ((type == 12000) && (param == 12000))
		{
			beach_count++;
			if ((beach_count >= (3 - current[0])) && (!objective_completed))
			{
				GameObject *objective_controller;
				objective_controller = Commands->Find_Object(1100004);
				if (objective_controller && !objective_completed)
				{
					//Commands->Send_Custom_Event(obj, objective_controller, 301, 1, 0.0f);
					//objective_completed = true;
				}
			}
		}
	}
};

//Script attached to the GDI soldiers once they are spawned from the Chinook.  This script
//makes sure they try to attack, listens for beachhead bunker openings, and tells the controller
//when the soldier dies.
DECLARE_SCRIPT(M03_Chinook_Spawned_Soldier_GDI, "Area:int, Send_Type_When_Killed:int, Target_ID:int")
{
	int area, send_type, target_id, null_count;
	bool in_place, en_route, escorting, diverted;

	enum 
	{
		BUNKER_OCCUPY = 50
	};

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( area, 1 );
		SAVE_VARIABLE( send_type, 2 );
		SAVE_VARIABLE( target_id, 3 );
		SAVE_VARIABLE( in_place, 4 );
		SAVE_VARIABLE( en_route, 5 );
		SAVE_VARIABLE( null_count, 6 );
		SAVE_VARIABLE( escorting, 7 );
		SAVE_VARIABLE( diverted, 8);
	}
	
	void Created(GameObject * obj) override
	{
		escorting = false;
		area = Get_Int_Parameter("Area");
		send_type = Get_Int_Parameter("Send_Type_When_Killed");
		target_id = Get_Int_Parameter("Target_ID");
		in_place = false;
		en_route = false;
		null_count = 0;
		Commands->Set_Innate_Aggressiveness(obj, 0.0f);
		Commands->Start_Timer(obj, this, 10.0f, 0);
		Commands->Start_Timer(obj, this, 3.0f, 9);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 3)
		{
			if (escorting && diverted)
			{
				diverted = false;
				ActionParamsStruct params;
				params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN - 1, 5);
				params.Set_Movement(STAR, 1.0f, 3.0f);
				params.MoveFollow = true;
				Commands->Action_Goto(obj, params);
			}
			return;
		}

		if (timer_id == 9)
		{
			float distance = Commands->Get_Random(-12.0f, 12.0f);
			//distance *= 3;
			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * distance;
			float b = sin(DEG_TO_RADF(facing)) * distance;
			Vector3 goto_loc = pos + Vector3(a, b, 0.0f);

			ActionParamsStruct params;
			params.Set_Basic(this, 100, 40);
			params.Set_Movement(goto_loc, RUN, 1.0f);
			Commands->Action_Goto(obj, params);

			return;
		}


		GameObject *target = NULL;
		if (null_count < 3)
		{
			//target = Commands->Find_Closest_Soldier(Commands->Get_Position(obj), 0.0f, 50.0f, true);
		}
		if ((target == NULL) && (null_count < 3)) {
			null_count++;
		}
		if (!in_place) {
			en_route = false;
		}
		if ((null_count < 3) && (target) && (!in_place))
		{
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_IDLE + 1, 0);
			params.Set_Attack(target, 100.0f, 2.0f, true);
			Commands->Action_Attack(obj, params);
		}
		if (!in_place) {
			Commands->Start_Timer(obj, this, 10.0f, 0);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 0)
		{
			en_route = false;
		}
		if (action_id == 5 && reason == ACTION_COMPLETE_LOW_PRIORITY && escorting)
		{
			diverted = true;
			Commands->Start_Timer(obj, this, 5.0f, 3);
		}
		if (reason != ACTION_COMPLETE_NORMAL)
		{
			return;
		}
		else if (action_id != 0)
		{
			GameObject *bunker;
			bunker = Commands->Find_Object(action_id);
			if (bunker)
			{
				Commands->Send_Custom_Event(obj, bunker, 3000, 3000, 0.0f);
			}
			
			GameObject *target = Commands->Find_Object(target_id);
			if (target)
			{
				Commands->Send_Custom_Event(obj, target, 12000, 12000, 0.0f);
			}
			in_place = true;
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject *target = Commands->Find_Object(target_id);
		if (target)
		{
			Commands->Send_Custom_Event(obj, target, send_type, area, 0.0f);
		}
		if (escorting)
		{
			Commands->Send_Custom_Event(obj, STAR, 3100, 3100, 0.0f);
		}
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		int has_escort = 1;
		Commands->Send_Custom_Event(obj, poker, 3000, (uintptr_t)&has_escort, 0.0f);
		if (has_escort == 1)
		{

			/*if (DIFFICULTY == 2)
			{
				return;
			}*/

			// Commando already has someone
			int id = Commands->Create_Conversation("M03CON041", 99, 200, false);
			Commands->Join_Conversation(obj, id, true, true, true);
			Commands->Start_Conversation(id, 100041);
			Commands->Monitor_Conversation(obj, id);
		}
		else if (has_escort == 0)
		{
			// Escort the commando unless we're playing on DIFFICULT

			/*if (DIFFICULTY == 2)
			{
				return;
			}*/

			if (area == 1)
			{
				Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2000.0f);
			}
			
			int id = Commands->Create_Conversation("M03CON040",99, 200, false);
			Commands->Join_Conversation(obj, id, true, true, true);
			Commands->Start_Conversation(id, 100040);
			Commands->Monitor_Conversation(obj, id);

			escorting = true;
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN - 1, 5);
			params.Set_Movement(STAR, 1.0f, 3.0f);
			params.MoveFollow = true;
			Commands->Action_Goto(obj, params);
		}
		else if (has_escort == -1)
		{

			if (DIFFICULTY == 2)
			{
				return;
			}

			// I was following the Commando... no more
			int id = Commands->Create_Conversation("M03CON041",99, 200, false);
			Commands->Join_Conversation(obj, id, true, true, true);
			Commands->Start_Conversation(id, 100041);
			Commands->Monitor_Conversation(obj, id);

			escorting = false;
			Commands->Action_Reset(obj, 100);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 999 && param == 999)
		{
			Vector3 dest = Vector3(0,0,0);
			
			Commands->Action_Reset(obj, 100);

			switch (area)
			{
			case 0: dest.Set(-100, -50, 5.986f);
				break;
			case 1: dest.Set(87, -58, 1.662f);
				break;
			case 2: dest.Set(-125, 13, 9.362f);
				break;
			}
			
			if (escorting)
			{
				escorting = false;
				Commands->Send_Custom_Event(obj, STAR, 3100, 3100, 0.0f);

				int id = Commands->Create_Conversation("M03CON042", 99, 200, false);
				Commands->Join_Conversation(obj, id, true, true, true);
				Commands->Start_Conversation(id, 100042);
				Commands->Monitor_Conversation(obj, id);
			}

			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(dest, RUN, 1.0f);
			Commands->Action_Goto(obj, params);
		}

		if (type == 1001 && param == 1001 && escorting)
		{
			escorting = false;
			Vector3 dest = Vector3(0,0,0);
			
			Commands->Action_Reset(obj, 100);

			switch (area)
			{
			case 0: dest.Set(-100, -50, 5.986f);
				break;
			case 1: dest.Set(87, -58, 1.662f);
				break;
			case 2: dest.Set(-125, 13, 9.362f);
				break;
			}

			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Movement(dest, RUN, 1.0f);
			Commands->Action_Goto(obj, params);
		}
	}
};

//Keeps track of the overall objective status for the mission.  The outro cinematic will not
//trigger until the primary objectives have all been completed
DECLARE_SCRIPT(M03_Objective_Tracker, "")
{
	int number_completed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( number_completed, 1 );
	}

	void Created(GameObject * /*obj*/) override
	{
		number_completed = 0;
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if ((type == 300) || (type == 308) || (type == 309))
		{
			if (param == 1)
				number_completed++;
		}

		if ((type == 8000) && (param == 8000))
		{
			if (sender)
			{
				Commands->Send_Custom_Event(obj, sender, 8000, number_completed, 0.0f);
			}
		}
	}
};

DECLARE_SCRIPT(M03_Intro_Substitute, "")
{
	void Custom( GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if (type == 999)
		{
			Commands->Start_Timer(obj, this, 3.0f, 9998);
		}
	}

	void Created(GameObject * obj) override
	{
		GameObject *target;
		target = Commands->Find_Object(1111000);
		if (target) {
			
			Commands->Send_Custom_Event(obj, target, 100, 100, 0.0f);
			Commands->Send_Custom_Event(obj, target, 200, 200, 420.0f/30.0f);
			Commands->Start_Timer(obj, this, 420.0f/30.0f, 0);
		}
		target = Commands->Find_Object(1100004);
		if (target) {
			Commands->Start_Timer(obj, this, 8.5f, 9997);			
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/ ) override
	{
		GameObject *target;
		target = Commands->Find_Object(1100004);

		/*if (action_id == 100001)
		{
			Commands->Send_Custom_Event(obj, target, 300, 3, 0);

			int id = Commands->Create_Conversation("M03CON063", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100063);
			Commands->Monitor_Conversation(obj, id);
		}*/

		if (action_id == 100001)
		{
			Commands->Send_Custom_Event(obj, target, 300, 3, 0);
		}

		if (action_id == 100001)
		{
			int id = Commands->Create_Conversation("M03CON012", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100012);
			Commands->Monitor_Conversation(obj, id);

			//Commands->Start_Timer(obj, this, 20.0f, 9999);
		}

		if (action_id == 100039)
		{
			int id = Commands->Create_Conversation("M03CON001", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100001);
			Commands->Monitor_Conversation(obj, id);
		}

		if (action_id == 100012)
		{
			//int id = Commands->Create_Conversation("M03CON064", 99, 2000, false);
			//Commands->Join_Conversation(NULL, id, true, true, true);
			//Commands->Start_Conversation(id, 100064);
			//Commands->Monitor_Conversation(obj, id);

			Commands->Send_Custom_Event(obj, target, 301, 3, 0);
		}
	}


	void Timer_Expired(GameObject * obj, int timer_id) override
	{		
		if (timer_id == 9997)
		{
			int id = Commands->Create_Conversation("M03CON039", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100039);
			Commands->Monitor_Conversation(obj, id);
		}
	}
};

DECLARE_SCRIPT(M03_Alternate_Sam_Site, "Chinook_Controller_ID:int")
{
	bool spoke;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(spoke, 1);
	}

	void Created(GameObject * /*obj*/) override
	{
		spoke = false;
		
	}
	
	void Custom(GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/) override
	{
		if (Commands->Find_Object(1100003) == NULL)
		{
			return;
		}
		GameObject *chinook = Commands->Find_Object(param);
		GameObject *target = Commands->Create_Object("Invisible_Object", Vector3(5,5,5));
		if (chinook) {
			char parameters[40];
			int con_id = Get_Int_Parameter("Chinook_Controller_ID");
			sprintf(parameters, "%d,%d", con_id, Commands->Get_ID(target));
			Commands->Attach_Script(chinook, "M03_Destroyed_Chinook", parameters);
		}
		Commands->Attach_To_Object_Bone(target, chinook, "ROTOR00");
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 0);
		params.Set_Attack(target, 0.0f, 0.0f, true);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack(obj, params);
		Commands->Start_Timer(obj, this, 1.0f, Commands->Get_ID(target));
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		GameObject *target = Commands->Find_Object(timer_id);
		Vector3 obj_pos = Commands->Get_Position(obj);
		Vector3 target_pos;
		if (target) {
			target_pos = Commands->Get_Position(target);
		}
		if (target) 
		{
			float range = Commands->Get_Distance(obj_pos, target_pos);
			if (range <= 60.0f)
			{
				/*if (!spoke)
				{
					spoke = true;
					int id = Commands->Create_Conversation("Chinook_Fodder_Dead", 0, 0, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0,0,0)), id, true, true, true);
					Commands->Start_Conversation(id, 0);
				}*/
				ActionParamsStruct params;
				params.Set_Basic(this, 99, 0);
				params.Set_Attack(target, 200.0f, 0.0f, true);
				params.AttackCheckBlocked = false;
				Commands->Action_Attack(obj, params);
			}
			Commands->Start_Timer(obj, this, 1.0f, timer_id);
		}
	} 
};

DECLARE_SCRIPT(M03_Flyover_Controller, "")
{
	int last;
	bool comanches;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
		SAVE_VARIABLE(comanches, 2);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 25.0f, 0);
		last = 21;
		comanches = true;
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 511 && param == 511)
		{
			comanches = false;
		}
		if (type == 512 && param == 512)
		{
			comanches = true;
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		static constexpr const char* flyovers[17] =
		{
			"A-10_1.txt",
			"A-10_2.txt",
			"A-10_3.txt",
			"A-10_4.txt",
			"A-10_5.txt",
			"A-10_6.txt",
			"Orca_1.txt",
			"Orca_2.txt",
			"Orca_3.txt",
			"Orca_4.txt",
			"Orca_5.txt",
			"Orca_1.txt",
			"Orca_2.txt",
			"Orca_3.txt",
			"Orca_4.txt",
			"Orca_5.txt",
			"Orca_6.txt"
		};
		int random = int(Commands->Get_Random(0, 17-WWMATH_EPSILON));
		if (!comanches && random > 5 && random < 11)
			{
				Commands->Start_Timer(obj, this, 25.0f, 0);
				return;
			}
		while (random == last) 
		{
			random = int(Commands->Get_Random(0, 17-WWMATH_EPSILON));
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		Commands->Start_Timer(obj, this, 25.0f, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M03_Destroyed_Chinook, "Controller_ID:int, Simple_ID:int")
{
	void Killed(GameObject * obj, GameObject * killer) override
	{
		/*int id = Commands->Create_Conversation("Chinook_Fodder_Scream", 100);
		Commands->Join_Conversation(NULL, id, true, true, true);
		Commands->Join_Conversation(STAR, id, true, true, true);
		Commands->Start_Conversation(id, 0);*/
		
		GameObject *con = Commands->Find_Object(Get_Int_Parameter("Controller_ID"));
		GameObject *sim = Commands->Find_Object(Get_Int_Parameter("Simple_ID"));
		if (con) {
			Commands->Destroy_Object(con);
		}
		if (sim) {
			Commands->Destroy_Object(sim);
		}
		Commands->Create_Explosion_At_Bone("Air Explosions Twiddler", obj, "SEAT1", killer);
		Commands->Create_Explosion_At_Bone("Air Explosions Twiddler", obj, "ROTOR01", killer);
		/*GameObject *obj_con = Commands->Find_Object(1100004);
		if (obj_con) {
			Commands->Send_Custom_Event(obj, obj_con, 302, 3, 0.0f);
		}*/
	}
};

DECLARE_SCRIPT(M03_Chinook_Fodder_Creator, "")
{
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (Commands->Find_Object(1100003) == NULL)
		{
			return;
		}
		if ((type == 3000) && (param == 3000))
		{
			GameObject *obj_con = Commands->Find_Object(1100004);
			if (obj_con) 
			{
				Commands->Send_Custom_Event(obj, obj_con, 302, 3, 0.0f);
			}
			//Commands->Attach_Script(obj, "Test_Cinematic", "ChinookTest.txt");
			//Commands->Start_Timer(obj, this, 9.5f, 9998);
		}
	}

	void Timer_Expired (GameObject *obj, int timer_id) override
	{
		if (timer_id == 9998)
		{
			//Commands->Create_Sound("00-N180E", Vector3(0,0,0), obj);
			int id = Commands->Create_Conversation("M03CON043", 99, 2000, false);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100043);
			Commands->Monitor_Conversation(obj, id);
						
			//Commands->Attach_Script(obj, "M03_Chinook_Troop_Drop", "ChinookTest.txt, 78.08 -36.88 1.20, 130.00");
		}
	}

	void Action_Complete (GameObject *obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == 100043)
		{
			GameObject *obj_con = Commands->Find_Object(1100004);
			if (obj_con) 
			{
				Commands->Send_Custom_Event(obj, obj_con, 302, 3, 0.0f);
			}
		}
	}
};


DECLARE_SCRIPT(M03_Tailgun, "Controller_ID:int")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject *con = Commands->Find_Object(Get_Int_Parameter("Controller_ID"));
		if (con) {
			Commands->Send_Custom_Event(obj, con, 200, 200, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M03_Tailgun_Fodder_Zone, "Spawner_ID_1:int, Spawner_ID_2:int, Spawner_ID_3:int")
{
	bool active;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active, 1);
	}

	void Created(GameObject * /*obj*/) override
	{
		active = true;

		Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_1"), false);
		Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_2"), false);
		Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_3"), false);
	}
	
	void Entered(GameObject * /*obj*/, GameObject * /*enterer*/) override
	{
		{
			if (!active)
			{
				return;
			}
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_1"), true);
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_2"), true);
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_3"), true);
		}
	}

	void Exited(GameObject * /*obj*/, GameObject * /*exiter*/) override
	{
		{
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_1"), false);
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_2"), false);
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_ID_3"), false);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 5000)
		{
			if (param == 1)
			{
				active = true;
			}
			else if (param == 2)
			{
				active = false;
				Exited(obj, STAR);
			}
		}
		
		if ((type == 200) && (param == 200))
		{
			Commands->Destroy_Object(obj);
		}

		if (type == 300 && param == 300)
		{
			Exited(obj, sender);
		}
	}
};

DECLARE_SCRIPT(M03_Tailgun_Fodder, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Action_Reset(obj, 99);
		GameObject *star = Commands->Get_A_Star(Commands->Get_Position(obj));
		if (star)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 0);
			params.Set_Attack(star, 200.0f, 2.0f - (float)DIFFICULTY, true);
			params.Set_Movement(star, RUN, 5.0f);
			Commands->Action_Attack(obj, params);
		}
	}
};

DECLARE_SCRIPT(M03_Tiberium_Cave_Stay_Put ,"")
{
	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 10.0f);
	}
};

DECLARE_SCRIPT(M03_Big_Gun_Explosion, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Create_3D_Sound_At_Bone("Explosion_Large_07", obj, "O_WREAKAGE");
	}
};

DECLARE_SCRIPT(M03_Inlet_Nod_Reinforcements, "")
{
#define SPOT1 (Vector3(51.0f, -91.0f, 2.0f))
#define SPOT2 (Vector3(97.0f, -46.0f, 2.0f))
#define SPOT3 (Vector3(64.0f, -32.0f, 2.0f))
#define FACE1 75.0f
#define FACE2 170.0f
#define FACE3 70.0f
		
	int count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( count, 1 );
	}
	
	void Created(GameObject * /*obj*/) override
	{
		count = 0;
	}

	void Timer_Expired(GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == 0)
		{
			GameObject * con = Commands->Create_Object("Invisible_Object", SPOT1);
			Commands->Set_Facing(con, FACE1);
			Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
		}

		if (timer_id == 1)
		{
			GameObject * con = Commands->Create_Object("Invisible_Object", SPOT2);
			Commands->Set_Facing(con, FACE2);
			Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
		}

		if (timer_id == 2)
		{
			GameObject * con = Commands->Create_Object("Invisible_Object", SPOT3);
			Commands->Set_Facing(con, FACE3);
			Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
		}
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if ((type == 300) && (param == 300))
		{
			count++;
		}
		
		if (count >= 2)
		{
			Commands->Start_Timer(obj, this, 1.0f, 0);
			Commands->Start_Timer(obj, this, 4.3f, 1);
			Commands->Start_Timer(obj, this, 8.1f, 2);
		}
	}
};

DECLARE_SCRIPT(M03_Base_Patrol, "WaypathID:int")
{
	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;
		//params.Set_Basic(this, INNATE_PRIORITY_BULLET_HEARD - 1, 0);
		params.Set_Basic(this, 49, 0);
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);
		params.Set_Movement(Vector3(0,0,0), WALK, 1.0f);
		params.WaypathID = Get_Int_Parameter("WaypathID");
		Commands->Action_Goto(obj, params);
	}
	
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 0 && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		}
	}

};

DECLARE_SCRIPT(M03_Base_Harvester, "Tiberium_Loc:vector3, Dock_Location:vector3, Dock_Entrance:vector3, Sakura_Dest:vector3")
{
	enum
	{
		HARVESTER_GOTO_TIBERIUM,
		HARVESTER_HARVEST_MOVE,
		HARVESTER_DOCK,
		HARVESTER_BEAT_IT,
		HARVESTER_CHASE_PLAYER,
		DOCK_TIMER
	};

	int harvest_count, anim_count;
	bool docked, playing_anim, power_off;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(harvest_count, 1);
		SAVE_VARIABLE(docked, 2);
		SAVE_VARIABLE(playing_anim, 3);
		SAVE_VARIABLE(anim_count, 4);
		SAVE_VARIABLE(power_off, 5);
	}
	
	void Created(GameObject * obj) override
	{
		harvest_count = 0;
		docked = false;
		playing_anim = false;
		anim_count = 0;
		power_off = false;
		
		ActionParamsStruct params;
		params.Set_Basic(this, 99, HARVESTER_GOTO_TIBERIUM);
		params.Set_Movement(Get_Vector3_Parameter("Tiberium_Loc"), 1.0f, 1.0f);
		Commands->Action_Goto(obj, params);
	}

	void Animation_Complete(GameObject * obj, const char * /*anim*/) override
	{
		if (playing_anim)
		{
			anim_count++;
			if (anim_count >= 3)
			{
				playing_anim = false;
				anim_count = 0;
			}
			else
			{
				Commands->Set_Animation(obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", false, NULL, 0.0f, -1.0f, false);
				return;
			}
		}

		if (harvest_count >= 4)
		{
			harvest_count = 0;
			ActionParamsStruct params;
			params.Set_Basic(this, 99, HARVESTER_DOCK);
			params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
			params.Dock_Vehicle(Get_Vector3_Parameter("Dock_Location"), Get_Vector3_Parameter("Dock_Entrance"));
			Commands->Action_Dock(obj, params);
			
		} 
		else
		{
			harvest_count++;
			Gather_Tiberium();
		}
	}
	
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (reason != ACTION_COMPLETE_NORMAL)
		{
			return;
		}

		if (action_id == HARVESTER_GOTO_TIBERIUM)
		{
			Gather_Tiberium();
		}

		if (action_id == HARVESTER_DOCK)
		{
			Commands->Start_Timer(obj, this, power_off ? 55.0f : 35.0f, DOCK_TIMER);
		}

		if (action_id == HARVESTER_HARVEST_MOVE)
		{
			Commands->Set_Animation(obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", false, NULL, 0.0f, -1.0f, false);
			playing_anim = true;
		}
		if (action_id == HARVESTER_BEAT_IT)
		{
			docked = true;
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 99, HARVESTER_GOTO_TIBERIUM);
		params.Set_Movement(Get_Vector3_Parameter("Tiberium_Loc"), 1.0f, 1.0f);
		Commands->Action_Goto(obj, params);
	}
	
	void Gather_Tiberium(void)
	{
		ActionParamsStruct params;
		params.Set_Basic(this, 99, HARVESTER_HARVEST_MOVE);
		Vector3 loc = Commands->Get_Position(Owner());
		loc.X += Commands->Get_Random(-3.0f, 3.0f);
		loc.Y += Commands->Get_Random(-3.0f, 3.0f);
		params.Set_Movement(loc, 1.0f, 1.0f);
		Commands->Action_Goto(Owner(), params);
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 622 && param == 622)
		{
			Commands->Set_Animation(obj, NULL, false, NULL, 0.0f, -1.0f, false);
			ActionParamsStruct params;
			params.Set_Basic(this, 100, HARVESTER_BEAT_IT);
			params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
			params.Dock_Vehicle(Get_Vector3_Parameter("Dock_Location"), Get_Vector3_Parameter("Dock_Entrance"));
			Commands->Action_Dock(obj, params);
		}
		if (type == 722 && param == 722 && !docked)
		{
			Commands->Set_Animation(obj, "V_NOD_HRVSTR.V_NOD_HRVSTR", true, NULL, 0.0f, -1.0f, false);
			ActionParamsStruct params;
			params.Set_Basic(this, 100, HARVESTER_CHASE_PLAYER);
			params.Set_Movement(STAR, ((float)DIFFICULTY / 10.0f) + 0.8f, 0.0f);
			Commands->Action_Goto(obj, params);
		}
		if (type ==7800 && param == 7800)
		{
			power_off = true;
		}
	}
};

DECLARE_SCRIPT(M03_Wheres_The_Star, "Controller_ID:int, Type:int, Param:int")
{
	void Entered(GameObject * obj, GameObject * /*enterer*/) override
	{
		GameObject * controller = Commands->Find_Object(Get_Int_Parameter("Controller_ID"));
		Commands->Send_Custom_Event(obj, controller, Get_Int_Parameter("Type"), Get_Int_Parameter("Param"), 0.0f);
	}
};

DECLARE_SCRIPT(M03_Reinforce_Area, "")
{
#define INLET_POSITION1 Vector3(99.85f, -49.51f, 2.0f)
#define BASE_POSITION1 Vector3(-138.0f, 50.0f, 9.5f)
#define INLET_FACING1 170.0f
#define BASE_FACING1 -30.0f
#define INLET_POSITION2 Vector3(51.0f, -91.0f, 2.0f)
#define BASE_POSITION2 Vector3(-79.0f, 59.0f, 9.5f)
#define INLET_FACING2 75.0f
#define BASE_FACING2 -120.0f
#define BEACH_POSITION Vector3(-95.656f, -68.236f, 1.433f)
#define BEACH_FACING -180.0f
	
	bool inlet_active, base_active, beach_active;
	bool switcher, forced;
	int count;
	int target_killed[3];
	int custom_count[3];
	int max_reinforcements[3];
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(inlet_active, 1);
		SAVE_VARIABLE(base_active, 2);
		SAVE_VARIABLE(count, 3);
		SAVE_VARIABLE(switcher, 4);
		SAVE_VARIABLE(beach_active, 5);
		SAVE_VARIABLE(target_killed, 6);
		SAVE_VARIABLE(custom_count, 7);
		SAVE_VARIABLE(forced, 8);
		SAVE_VARIABLE(max_reinforcements, 9);
	}
	
	void Created(GameObject * /*obj*/) override
	{
		inlet_active = base_active = switcher = false;
		beach_active = true;
		count = 0;
		target_killed[0] = target_killed[1] = target_killed[2] = 3;
		custom_count[0] = custom_count[1] = 1;
		custom_count[2] = 0;
		max_reinforcements[0] = max_reinforcements[1] = max_reinforcements[2] = 2;
		forced = false;
	}

	/*void Reinforce(void)
	{
		GameObject * officer1 = Commands->Find_Object (300009);
		GameObject * officer2 = Commands->Find_Object (300010);
		
		/*if (inlet_active)
		{
			if (officer1 || officer2)
			{
				Commands->Start_Timer(Commands->Find_Object (1144444), this, 3.0f, INLET_REINFORCE);
			}
		}*/

		/*else if (base_active)
		{
			if (max_reinforcements[2]-- <= 0)
			{
				return;
			}

			GameObject * con = Commands->Create_Object("Invisible_Object", (switcher) ? BASE_POSITION1 : BASE_POSITION2);
			Commands->Set_Facing(con, (switcher) ? BASE_FACING1 : BASE_FACING2);
			Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			if (!forced)
			{
				Commands->Send_Custom_Event(Owner(), Owner(), 7000, 2, 0.0f);
			}
		}*/

		/*else if (beach_active)
		{
			if (max_reinforcements[0]-- <= 0)
			{
				return;
			}

			if (officer3 || officer4 || officer5)
			{
				GameObject * con = Commands->Create_Object("Invisible_Object", BEACH_POSITION);
				Commands->Set_Facing(con, BEACH_FACING);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
				if (!forced)
				{
					Commands->Send_Custom_Event(Owner(), Owner(), 7000, 0, 0.0f);
				}
			}
		}
	}*/

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 5000)
		{
			int *area = (int *)param;
			if (beach_active)
			{
				*area = 0;
			}
			else if (inlet_active)
			{
				*area = 1;
			}
			else if (base_active)
			{
				*area = 2;
			}
			else
			{
				*area = -1;
			}
		}
		if (type == 6300)
		{
			int *target_count = (int *)param;
			if (beach_active)
			{
				*target_count = target_killed[0];
			}
			else if (inlet_active)
			{
				*target_count = target_killed[1];
			}
			else if (base_active)
			{
				*target_count = target_killed[2];
			}
		}
		if (type == 6000 && param == 6000)
		{
			if ((beach_active && target_killed[0] != 1000) || (inlet_active && target_killed[1] != 1000) || (base_active && target_killed[2] != 1000))
			{
				forced = true;
				/*int id = Commands->Create_Conversation("Nod_Reinforcements", 100);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0,0,0)), id, true, true, true);
				Commands->Start_Conversation(id, 0);*/
				Commands->Start_Timer(obj, this, 3.0f, 0);
			}
		}
		if (type == 8000)
		{
			custom_count[param]++;
			if (custom_count[param] >= 2)
			{
				custom_count[param] = 1;
				if (param == 2)
				{
					custom_count[param] = 0;
				}
				target_killed[param]++;
				if (target_killed[param] >= 6)
				{
					target_killed[param] = 1000;
				}
			}
		}
		if (type == 0 && param == 2)
		{
			beach_active = false;
		}
		if (type == 1 && param == 1)
		{
			inlet_active = true;
		}
		if (type == 1 && param == 2)
		{
			inlet_active = false;
		}
		if (type == 2 && param == 1)
		{
			base_active = true;
		}
		if (type == 2 && param == 2)
		{
			base_active = false;
		}
		if (type == 1000 && param == 1000 && (inlet_active || base_active || beach_active))
		{
			count++;
			if ((beach_active && count >= target_killed[0]) || (inlet_active && count >= target_killed[1]) || (base_active && count >= target_killed[2]))
			{
				forced = false;
				/*int id = Commands->Create_Conversation("Nod_Reinforcements", 100);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0,0,0)), id, true, true, true);
				Commands->Start_Conversation(id, 0);*/
				Commands->Start_Timer(obj, this, 3.0f, 0);
				count = 0;
			}
		}

		if (type == UPDATE)
		{
			Commands->Send_Custom_Event(obj, sender, BASE, base_active, 0.0f);
			Commands->Send_Custom_Event(obj, sender, INLET, inlet_active, 0.0f);
			Commands->Send_Custom_Event(obj, sender, BEACH, beach_active, 0.0f);
		}
	}

	void Timer_Expired(GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == 0)
		{
			switcher = !switcher;
			//Reinforce();
		}

		if (timer_id == INLET_REINFORCE)
		{
			if (max_reinforcements[1]-- <= 0)
			{
				return;
			}
			
			GameObject * con = Commands->Create_Object("Invisible_Object", (switcher) ? INLET_POSITION1 : INLET_POSITION2);
			Commands->Set_Facing(con, (switcher) ? INLET_FACING1 : INLET_FACING2);
			Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			if (!forced)
			{
				Commands->Send_Custom_Event(Owner(), Owner(), 7000, 1, 0.0f);
			}
		}
	}
};

DECLARE_SCRIPT(M03_Officer_With_Key_Card, "")
{
	void Killed(GameObject * obj, GameObject * killer) override
	{
		if (!Commands->Has_Key(killer, 1))
		{
			Vector3 pos = Commands->Get_Position(obj) + Vector3(0,0,0.5);
			GameObject * key = Commands->Create_Object("Level_01_Keycard", pos);
			Commands->Attach_Script(key, "M03_Key_Card", "");

			int id = Commands->Create_Conversation("M03CON005", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100005);
			Commands->Monitor_Conversation(obj, id);
		}
	}
};

DECLARE_SCRIPT(M03_Officer_With_Key_Card2, "")
{
	void Killed(GameObject * obj, GameObject * killer) override
	{
		if (!Commands->Has_Key(killer, 2))
		{
			Vector3 pos = Commands->Get_Position(obj) + Vector3(0,0,0.5);
			Commands->Create_Object("Level_02_Keycard", pos);
		}
	}
};

DECLARE_SCRIPT(M03_Key_Card, "")
{
	void Custom(GameObject * obj, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		GameObject * con = Commands->Find_Object(1100004);
		if (con)
		{
			//Commands->Send_Custom_Event(obj, con, 307, 3, 0.0f);
			Commands->Send_Custom_Event(obj, con, 307, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M03_Chinook_ParaDrop, "Preset:string")
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
		GameObject *chinook = Commands->Create_Object("Nod_Transport_Helicopter", loc);
		Commands->Set_Facing(chinook, facing);
		Commands->Set_Animation(chinook, "v_Nod_trnspt.XG_RTN_TrnsptA", true, NULL, 0.0f, -1.0f, false);
		Commands->Attach_To_Object_Bone(chinook, chinook_rail, "BN_Chinook_1");

		dead = false;
		out = 0;
		char params[12];
		sprintf(params, "%d", Commands->Get_ID(obj));
		Commands->Attach_Script(chinook, "M03_Reinforcement_Chinook", params);

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
				Commands->Attach_Script(para1, "M03_No_More_Parachute", "");
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
				Commands->Attach_Script(para2, "M03_No_More_Parachute", "");
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
				Commands->Attach_Script(para3, "M03_No_More_Parachute", "");
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
			Commands->Attach_Script(soldier1, "RMV_Trigger_Killed", "1144444, 1000, 1000");
			Commands->Attach_Script(soldier1, "M03_Killed_Sound", "");
			Commands->Attach_Script(soldier1, "M03_Paratrooper_Run", "");
			Commands->Attach_Script(soldier1, "DLS_Volcano_Stumble", "");
			Commands->Attach_To_Object_Bone( soldier1, box1, "Box01" );
			Commands->Set_Animation(soldier1, "s_a_human.H_A_X5D_ParaT_1", false, NULL, 0.0f, -1.0f, false);
			out++;
			/*if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}*/
		
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
			Commands->Attach_Script(soldier2, "RMV_Trigger_Killed", "1144444, 1000, 1000");
			Commands->Attach_Script(soldier2, "M03_Killed_Sound", "");
			Commands->Attach_Script(soldier2, "M03_Paratrooper_Run", "");
			Commands->Attach_Script(soldier2, "DLS_Volcano_Stumble", "");
			Commands->Set_Animation(soldier2, "s_a_human.H_A_X5D_ParaT_2", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier2, box2, "Box02" );
			out++;
			/*if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}*/
				
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
			Commands->Attach_Script(soldier3, "RMV_Trigger_Killed", "1144444, 1000, 1000");
			Commands->Attach_Script(soldier3, "M03_Killed_Sound", "");
			Commands->Attach_Script(soldier3, "M03_Paratrooper_Run", "");
			Commands->Attach_Script(soldier3, "DLS_Volcano_Stumble", "");
			Commands->Set_Animation(soldier3, "s_a_human.H_A_X5D_ParaT_3", false, NULL, 0.0f, -1.0f, false);
			Commands->Attach_To_Object_Bone( soldier3, box3, "Box03" );
			out++;
			/*if ((out - 1) == DIFFICULTY)
			{
				dead = true;
			}*/
			
			}
			break;
		
		}
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 23000 && param == 23000)
		{
			dead = true;
		}
	}
};

DECLARE_SCRIPT(M03_No_More_Parachute, "")
{
	void Destroyed(GameObject * obj) override
	{
		Commands->Create_3D_Sound_At_Bone("parachute_away", obj, "ROOTTRANSFORM");
	}
};

DECLARE_SCRIPT(M03_Reinforcement_Chinook, "Controller_ID:int")
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
	
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject * con = Commands->Find_Object(Get_Int_Parameter(0));
		Commands->Send_Custom_Event(obj, con, 23000, 23000, 0.0f);
	}

	void Destroyed(GameObject * /*obj*/) override
	{
		Commands->Stop_Sound(sound_id, true);
	}
};

DECLARE_SCRIPT(M03_Staged_Conversation_1, "Soldier_1_ID:int, Soldier_2_ID:int")
{
	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 1000 && param == 1000)
		{
			GameObject * soldier1 = Commands->Find_Object(Get_Int_Parameter(0));
			GameObject * soldier2 = Commands->Find_Object(Get_Int_Parameter(1));
			if (soldier1 && soldier2)
			{
				/*int id = Commands->Create_Conversation("Volcano", INNATE_PRIORITY_ENEMY_SEEN - 1, 20.0f, true);
				Commands->Join_Conversation(soldier1, id, true, true, true);
				Commands->Join_Conversation(soldier2, id, true, true, true);
				Commands->Start_Conversation(id, 0);
				Commands->Monitor_Conversation(obj, id);*/
			}
		}
	}

	void Action_Complete(GameObject * obj, int /*action_id*/, ActionCompleteReason reason) override
	{
		if (reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			GameObject * soldier1 = Commands->Find_Object(Get_Int_Parameter(0));
			GameObject * soldier2 = Commands->Find_Object(Get_Int_Parameter(1));
			if (soldier1)
			{
				Commands->Send_Custom_Event(obj, soldier1, 100, 100, 0.0f);
			}
			if (soldier2)
			{
				Commands->Send_Custom_Event(obj, soldier2, 100, 100, 0.0f);
			}
		}
	}
};

DECLARE_SCRIPT(M03_Staged_Conversation_Soldier, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 0.5f, 0);
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);
		Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, false);
		Commands->Innate_Soldier_Enable_Bullet_Heard(obj, false);
	}

	void Enemy_Seen(GameObject * obj, GameObject * /*enemy*/) override
	{
		Commands->Set_Innate_Is_Stationary(obj, false);
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Bullet_Heard(obj, true);
	}

	void Damaged(GameObject * obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Set_Innate_Is_Stationary(obj, false);
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Bullet_Heard(obj, true);
	}

	void Custom(GameObject * obj, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		Commands->Set_Innate_Is_Stationary(obj, false);
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, true);
		Commands->Innate_Soldier_Enable_Bullet_Heard(obj, true);
	}
};

DECLARE_SCRIPT(M03_Beach_Turret, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		GameObject * gunboat = Commands->Find_Object(1100003);
		ActionParamsStruct params;
		params.Set_Basic(this, 90, 0);
		params.Set_Attack(gunboat, 300.0f, 6.0f - (float)DIFFICULTY, true);
		Commands->Action_Attack(obj, params);
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		Vector3 my_pos = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		GameObject * destroyed_turret = Commands->Create_Object("Nod_Turret_Destroyed", my_pos);
		Commands->Set_Facing(destroyed_turret, facing);
		Commands->Attach_Script(destroyed_turret, "M03_Destroyed_Turret", "");
	}
};

DECLARE_SCRIPT(M03_SAM_Site_Logic, "")
{
	bool target_acquired;
	int current, ignore_ids[10];

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(target_acquired, 1);
		SAVE_VARIABLE(ignore_ids, 2);
		SAVE_VARIABLE(current, 3);
	}
	
	void Created(GameObject * obj) override
	{
				
		Commands->Enable_Enemy_Seen(obj, true);
		target_acquired = false;
		current = 0;
		for (int x = 0; x < 10; x++)
		{
			ignore_ids[x] = 0;
		}
		Commands->Start_Timer(obj, this, Commands->Get_Random(2, 3), 1);
	}
	
	bool Is_Ignore_Target(int id)
	{
		bool retval = false;
		for (int x = 0; x < current; x++)
		{
			if (ignore_ids[x] == id)
			{
				retval = true;
				break;
			}
		}
		return retval;
	}
	
	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		if (target_acquired)
		{
			return;
		}

		if (Commands->Is_A_Star(enemy) || Is_Ignore_Target(Commands->Get_ID(enemy)))
		{
			return;
		}

		Vector3 target_pos = Commands->Get_Position(enemy);
		Vector3 my_pos = Commands->Get_Position(obj);
		if (target_pos.Z - my_pos.Z >= 5.0f)
		{
			target_acquired = true;
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 0);
			params.Set_Attack(enemy, 500.0f, 0.0f, true);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer(obj, this, 2.0f, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 0)
		{
			target_acquired = false;
			Commands->Action_Reset(obj, 95);
		}
		if (timer_id == 1)
		{
			Vector3 my_target = Commands->Get_Position(obj);
			my_target.X += Commands->Get_Random( -10.0f , 10.0f);
			my_target.Y += Commands->Get_Random( -10.0f , 10.0f);
			my_target.Z += Commands->Get_Random( 2.0f, 6.0f);

			ActionParamsStruct params;
			params.Set_Basic( this, 80, 20 );
			params.Set_Attack( my_target, 0, 0, true );
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);
			Commands->Start_Timer(obj, this, Commands->Get_Random(2, 3), 1);
		}
	}

	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		Vector3 my_position = Commands->Get_Position ( obj );
		float my_facing = Commands->Get_Facing ( obj );
		GameObject * destroyedSam = Commands->Create_Object ( "M01_Destroyed_SAM", my_position);
		Commands->Set_Facing ( destroyedSam, my_facing );
		Commands->Attach_Script(destroyedSam, "M01_Destroyed_SAMSITE_JDG", "");
		Commands->Attach_Script(destroyedSam, "M03_Destroyed_SAM_Site", "");
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == M00_CUSTOM_SAM_SITE_IGNORE && current < 10 && current >= 0)
		{
			ignore_ids[current++] = param;
		}
	}
};

DECLARE_SCRIPT(M03_Destroyed_SAM_Site, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
	}

	void Timer_Expired(GameObject * /*obj*/, int /*timer_id*/) override
	{
		Commands->Create_2D_Sound("00-N170E");
	}
};

DECLARE_SCRIPT(M03_Destroyed_Turret, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
		Commands->Start_Timer(obj, this, 4.0f, 1);
	}

	void Timer_Expired(GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == 0)
		{
			Commands->Create_2D_Sound("EVA_Enemy_Structure_Destroyed");
		}
		else if (timer_id == 1)
		{
			/*int id = Commands->Create_Conversation("Turret_Destroyed", 0, 0, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 0);*/
		}
	}
};

DECLARE_SCRIPT(M03_Structure_Powerup_Drop, "Powerup:string")
{
	void Killed(GameObject * obj, GameObject * /*killed*/) override
	{
		Vector3 pos = Commands->Get_Position(obj);
		float facing = Commands->Get_Facing(obj);
		facing = facing + 180.0f;
		pos.X += cos(DEG_TO_RADF(facing)) * 2.5f;
		pos.Y += sin(DEG_TO_RADF(facing)) * 2.5f;
		pos.Z += 0.5f;

		const char *powerup = Get_Parameter("Powerup");
		Commands->Create_Object(powerup, pos);
	}
};

DECLARE_SCRIPT(M03_Beach_Scenario_Controller, "")
{
	int count;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(count, 1);
	}
	
	void Created(GameObject * /*obj*/) override
	{
		count = 0;
	}
	
	void Custom(GameObject * obj, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		count++;
		if (count >= 4)
		{
			count = 0;
			GameObject * obj_con = Commands->Find_Object(1100004);
			if (obj_con && Commands->Find_Object (1100003))
			{
				GameObject * obj_con = Commands->Find_Object(1100004);

				Commands->Send_Custom_Event(obj, obj_con, 301, 3, 0.0f);
				Commands->Send_Custom_Event(obj, obj_con, 301, 1, 0.0f);

				int id = Commands->Create_Conversation("M03CON013", 99, 2000, true);
				Commands->Join_Conversation(NULL, id, true, true, true);
				Commands->Start_Conversation(id, 100013);
				Commands->Monitor_Conversation(obj, id);

				Commands->Grant_Key(STAR, 20, true);
			}
		}
	}
};

/*DECLARE_SCRIPT(M03_Evacuation_Controller, "Preset:string")
{
	bool active;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(active, 1);
	}
	
	void Created(GameObject * obj) override
	{
		active = false;
	}
	
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 100 && param == 100)
		{
			active = true;
			Commands->Start_Timer(obj, this, 1.0f, 0);
		}
		else if (type == 200 && param == 200)
		{
			active = false;
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		int building = Get_Int_Random(0, 2);
		const char * preset = Get_Parameter("Preset");
		GameObject * runner = Commands->Create_Object(preset, Get_Spawn_Loc(building));
		if (runner)
		{
			char params[20];
			sprintf(params, "%d", building);
			Commands->Attach_Script(runner, "M03_Evacuator", params);
		}
		if (active)
		{
			Commands->Start_Timer(obj, this, 20.0f, 0);
		}
	}

	Vector3 Get_Spawn_Loc(int building)
	{
		Vector3 retval(0,0,0);
		switch (building)
		{
		case 0: retval.Set(-157.0f, -28.0f, 1.214f);  //Refinery
			break;
		case 1: retval.Set(-105.0f, 49.0f, -1.948f);  //Comm center
			break;
		case 2: retval.Set(-72.0f, 19.0f, 4.718f);  //Power plant
			break;
		}
		return retval;
	}
};*/

/*DECLARE_SCRIPT(M03_Evacuator, "Building:int")
{
	void Created(GameObject * obj) override
	{
				
		Commands->Grant_Key(obj, 6, true);
		Commands->Grant_Key(obj, 2, true);
		
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 0);
		params.Set_Movement(Vector3(0,0,0), RUN, 1.0f);
		params.WaypathID = Get_Waypath(Get_Int_Parameter("Building"));
		params.WaypathSplined = true;
		Commands->Action_Goto(obj, params);
	}

	int Get_Waypath(int building)
	{
		int waypaths[3][4] = {{1144741, 1144741, 1144767, 1144767}, 
								{1144800, 1144785, 1144816, 1144835}, 
								{1144920, 1144851, 1144898, 1144873}};
		return waypaths[building][Get_Int_Random(0, 3)];
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (reason != ACTION_COMPLETE_NORMAL)
		{
			return;
		}
		if (action_id == 0)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 1);
			params.Set_Movement(Vector3(0,0,0), RUN, 1.0f);
			params.WaypathID = 1144950;
			params.WaypathSplined = true;
			Commands->Action_Goto(obj, params);
		}
		if (action_id == 1)
		{
			Commands->Destroy_Object(obj);
		}
	}
};*/

/*DECLARE_SCRIPT(M03_Dock_Evacuation_Controller, "")
{
	void Custom(GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if (type == 100 && param == 100)
		{
			Commands->Start_Timer(obj, this, 1.0f, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		int building = Get_Int_Random(0, 2);
		GameObject * runner = Commands->Create_Object("M03_Evacuator", Get_Spawn_Loc(building));
		if (runner)
		{
			char params[20];
			sprintf(params, "%d", building);
			Commands->Attach_Script(runner, "M03_Dock_Evacuator", params);
		}
			Commands->Start_Timer(obj, this, 30.0f, 0);
	}

	Vector3 Get_Spawn_Loc(int building)
	{
		Vector3 retval(0,0,0);
		switch (building)
		{
		case 0: retval.Set(-214.0f, 52.0f, 4.785f);  //Q-hut one
			break;
		case 1: retval.Set(-237.0f, 79.0f, 4.758f);  //Q-hut two
			break;
		case 2: retval.Set(-235.0f, 100.0f, 4.758f);  //Q-hut three
			break;
		}
		return retval;
	}
};*/

/*DECLARE_SCRIPT(M03_Dock_Evacuator, "Building:int")
{
	void Created(GameObject * obj) override
	{
				
		Commands->Grant_Key(obj, 6, true);
		Commands->Grant_Key(obj, 2, true);
		
		ActionParamsStruct params;
		params.Set_Basic(this, 99, 0);
		params.Set_Movement(Vector3(0,0,0), RUN, 1.0f);
		params.WaypathID = Get_Waypath(Get_Int_Parameter("Building"));
		params.WaypathSplined = true;
		Commands->Action_Goto(obj, params);
	}

	int Get_Waypath(int building)
	{
		int waypaths[3] = {1144983, 1145002, 1145011};
		return waypaths[building];
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (reason != ACTION_COMPLETE_NORMAL)
		{
			return;
		}
		if (action_id == 0)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 99, 1);
			params.Set_Movement(Vector3(0,0,0), RUN, 1.0f);
			params.WaypathID = 1145021;
			params.WaypathSplined = true;
			Commands->Action_Goto(obj, params);
		}
		if (action_id == 1)
		{
			Commands->Destroy_Object(obj);
		}
	}
};*/

DECLARE_SCRIPT(DLS_Volcano_Active, "Receive_Type=0:int, Receive_Param=0:int, Volcano_Timer_Id=0:int, Volcano_Delay=0.0:float, Explosion_Delay_Min=0.0:float, Explosion_Delay_Max=0.0:float, Rumble_Delay_Min=0.0:float, Rumble_Delay_Max=0.0:float, Debug_Mode=0:int")
{

	int receive_type;
	int receive_param;
	int volcano_timer_id;
	float volcano_delay;
	float explosion_delay_min;
	float explosion_delay_max;
	float rumble_delay_min;
	float rumble_delay_max;
	bool volcano_active;
	bool debug_mode;
	
	float explosion_delay;
	float rumble_delay;
	Vector3 exploc[16];

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(receive_type, 1);
		SAVE_VARIABLE(receive_param, 2);
		SAVE_VARIABLE(volcano_timer_id, 3);
		SAVE_VARIABLE(volcano_delay, 4);
		SAVE_VARIABLE(explosion_delay_min, 5);
		SAVE_VARIABLE(explosion_delay_max, 6);
		SAVE_VARIABLE(rumble_delay_min, 7);
		SAVE_VARIABLE(rumble_delay_max, 8);
		SAVE_VARIABLE(volcano_active, 9);
		SAVE_VARIABLE(debug_mode, 10);
		SAVE_VARIABLE(explosion_delay, 11);
		SAVE_VARIABLE(rumble_delay, 12);
		SAVE_VARIABLE(exploc, 13);
	}

	void Created(GameObject * /*obj*/) override
	{
	//	SCRIPT_DEBUG_MESSAGE (("Volcano Script Created.\n"));
		receive_type = Get_Int_Parameter("Receive_Type");
		receive_param = Get_Int_Parameter("Receive_Param");
		volcano_timer_id = Get_Int_Parameter("Volcano_Timer_Id");
		volcano_delay = Get_Int_Parameter("Volcano_Delay");
		explosion_delay_min = Get_Float_Parameter("Explosion_Delay_Min");
		explosion_delay_max = Get_Float_Parameter("Explosion_Delay_Max");
		rumble_delay_min = Get_Float_Parameter("Rumble_Delay_Min");
		rumble_delay_max = Get_Float_Parameter("Rumble_Delay_Max");
		
		volcano_active = false;

		exploc[0].Set(11.53f,20.95f,50.73f);
		exploc[1].Set(9.8f,11.69f,53.99f);
		exploc[2].Set(4.24f,8.67f,40.85f);
		exploc[3].Set(-15.75f,14.05f,30.02f);
		exploc[4].Set(9.05f,-2.1f,45.83f);
		exploc[5].Set(-57.34f,22.78f,16.88f);
		exploc[6].Set(-151.27f,18.56f,17.59f);
		exploc[7].Set(-98.43f,-7.34f,8.65f);
		exploc[8].Set(-103.61f,17.34f,9.21f);
		exploc[9].Set(-117.75f,0.22f,8.95f);
		exploc[10].Set(-171.99f,71.87f,14.4f);
		exploc[11].Set(-196.42f,87.29f,11.5f);
		exploc[12].Set(-269.45f,40.87f,4.93f);
		exploc[13].Set(-260.96f,132.15f,8.45f);
		exploc[14].Set(-144.31f,44.4f,9.33f);
		exploc[15].Set(-68.42f,22.18f,20.73f);

		

	}

	void Custom(GameObject* obj, int type, intptr_t param, GameObject* /*sender*/) override
	{
		if((type == receive_type) && (param == receive_param))
		{
			Commands->Start_Timer(obj, this, 5.0, CLOUD_DELAY);
			
			volcano_active = true;
			explosion_delay = Commands->Get_Random(explosion_delay_min, explosion_delay_max);
			Commands->Start_Timer(obj, this, explosion_delay, 6701);
			rumble_delay = Commands->Get_Random(rumble_delay_min, rumble_delay_max);
			Commands->Start_Timer(obj, this, rumble_delay, 6702);
			if(volcano_timer_id != 0)
			{
				Commands->Start_Timer(obj, this, volcano_delay, volcano_timer_id);
			}
			
		}
				
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if (timer_id == CLOUD_DELAY)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), DOGFIGHT_ENDED, 0, 0.0f);
			Commands->Set_Clouds (1.0f, 1.0f, 20);

			/*int id = Commands->Create_Conversation("M03CON010", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100010);
			Commands->Monitor_Conversation(obj, id);*/

			Commands->Start_Timer(obj, this, 55.0, MESSAGE_DELAY);
			Commands->Start_Timer(obj, this, 10.0, ANNOUNCEMENT_DELAY);
		}

		if (timer_id == MESSAGE_DELAY)
		{			
			int id = Commands->Create_Conversation("M03CON011", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100011);
			Commands->Monitor_Conversation(obj, id);		
		}

		if (timer_id == ANNOUNCEMENT_DELAY)
		{			
			int id = Commands->Create_Conversation("M03CON048", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100048);
			Commands->Monitor_Conversation(obj, id);

			Commands->Start_Timer(obj, this, 30.0, ANNOUNCEMENT_DELAY);
		}

		if((timer_id == 6701) && (volcano_active == true))
		{
			int i = Get_Int_Random(0,15);
			Commands->Create_Explosion("Ground Explosions - Harmless", exploc[i], obj);
			Commands->Start_Timer(obj, this, explosion_delay, 6701);
		}
		if((timer_id == 6702) && (volcano_active == true))
		{
			Vector3 volcano;
			volcano.X = 0.0;
			volcano.Y = 0.0;
			volcano.Z = 0.0;
			float camera_intensity = Commands->Get_Random(.05f,.1f);
			float camera_duration = Commands->Get_Random(4.0f,8.0f);
			Commands->Shake_Camera(volcano, 5000.0f, camera_intensity, camera_duration);
			Commands->Start_Timer(obj, this, rumble_delay, 6702);
			Commands->Create_Logical_Sound(obj, M03_SOUND_VOLCANO, volcano, 5000.0f);
			Commands->Create_Sound("earthquake_large_01", Vector3(0,0,0), obj);

		}
		if((timer_id == volcano_timer_id) && (volcano_active == true))
		{
			volcano_active = false;
		}
	}

	
	void Entered(GameObject* /*obj*/, GameObject* /*enterer*/) override
	{
			volcano_active = false;
	}
};

DECLARE_SCRIPT(M03_Sakura_Explosion, "")
{
	void Destroyed(GameObject * obj) override
	{
		Commands->Create_Explosion_At_Bone( "Air Explosions Twiddler", obj, "ROTOR00", NULL);
		Commands->Create_Explosion_At_Bone( "Air Explosions Twiddler", obj, "TURRET", NULL);
		Commands->Create_Explosion_At_Bone( "Air Explosions Twiddler", obj, "ROTOR01", NULL);	
	}
};



DECLARE_SCRIPT(DLS_Volcano_Stumble, "Debug_Mode=0:int")
{
	enum {FALL_FORWARD, FALL_BACK, RISE_FORWARD, RISE_BACK, 
			FLEE_VOLCANO, STUMBLE};

	bool animating;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(animating, 1);
	}

	void Created (GameObject *obj) override
	{
		animating = false;

		Commands->Set_Innate_Take_Cover_Probability ( obj, 100.0f );
		Commands->Set_Innate_Aggressiveness ( obj, 100.0f );
	}
	
	/*void Timer_Expired(GameObject* obj, int timer_id)
	{
		if(timer_id == 4455)
		{
	//		Commands->Action_Reset(obj, 97.0f);
			int i = Commands->Get_Random(0,3);

			if(i == 0)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, FALL_FORWARD);
				params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_622A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
			
			if(i == 1)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, STUMBLE);
				params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_611A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
			
			if(i == 2)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, FALL_BACK);
				params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_632A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
		}
	}*/

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		if((sound.Type == M03_SOUND_VOLCANO) && (!animating))
		{
			animating = true;

			Commands->Action_Reset(obj, 97.0f);
			int i = Commands->Get_Random_Int(0,2);

			if(i == 0)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, FALL_FORWARD);
				//params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_622A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
			
			if(i == 1)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, STUMBLE);
				//params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_611A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
			
			if(i == 2)
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 98.0f, FALL_BACK);
				//params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_632A", false);
				Commands->Action_Play_Animation(obj, params);
				//Commands->Start_Timer(obj, this, 6.0f, 4455);
			}
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		switch (action_id)
		{
		// Animation to return units that have fallen to ground back up
		case FALL_FORWARD:
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 99.0f, RISE_FORWARD);
				//params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_H11C", false);
				Commands->Action_Play_Animation(obj, params);
				break;
			}
		case FALL_BACK:
			{
				ActionParamsStruct params;
				params.Set_Basic(this, 99.0f, RISE_BACK);
				//params.Set_Movement(Commands->Get_Position(obj), 1.0f, 2.0f);
				params.Set_Animation("S_A_HUMAN.H_A_H13C", false);
				Commands->Action_Play_Animation(obj, params);
				break;
			}
		// Flee units to dock
		case RISE_BACK:
			{
				animating = false;

				ActionParamsStruct params;
				params.Set_Basic(this, 70.0f, FLEE_VOLCANO);
				params.Set_Movement(Vector3(-288.02f, 78.11f, 9.51f), 1.0f, 15.0f);
				Commands->Action_Goto(obj, params);
				break;
			}
		case RISE_FORWARD:
			{
				animating = false;

				ActionParamsStruct params;
				params.Set_Basic(this, 70.0f, FLEE_VOLCANO);
				params.Set_Movement(Vector3(-288.02f, 78.11f, 9.51f), 1.0f, 15.0f);
				Commands->Action_Goto(obj, params);
				break;
			}
		case STUMBLE:
			{
				animating = false;

				ActionParamsStruct params;
				params.Set_Basic(this, 70.0f, FLEE_VOLCANO);
				params.Set_Movement(Vector3(-288.02f, 78.11f, 9.51f), 1.0f, 15.0f);
				Commands->Action_Goto(obj, params);
				break;
			}
		}

	}
};

DECLARE_SCRIPT(M03_Beach_Radio, "")
{
	int conv_count;
	bool conv_active;
	int conv_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(conv_count, 1);
		SAVE_VARIABLE(conv_active, 2);
		SAVE_VARIABLE(conv_id, 3);
	}

	void Created(GameObject * /*obj*/) override
	{
		conv_count = 0;
		conv_active = false;
		conv_id = 0;
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (conv_count > 2 || conv_active)
		{
			return;
		}
		else
		{
			static constexpr const char* conv[3] =
			{
				"M03CON044",
				"M03CON045",
				"M03CON046"
			};
			

			conv_id = Commands->Create_Conversation(conv[conv_count++], 0, 0, true);
			Commands->Join_Conversation(NULL, conv_id, true, true, true);
			Commands->Join_Conversation(poker, conv_id, true, true, true);
			Commands->Start_Conversation(conv_id, 0);
			Commands->Monitor_Conversation(obj, conv_id);
			conv_active = true;
		}
	}

	void Killed (GameObject * /*obj*/, GameObject * /*killer*/) override
	{
		Commands->Stop_Conversation ( conv_id );
	}

	void Action_Complete(GameObject * /*obj*/, int /*action_id*/, ActionCompleteReason reason) override
	{
		if (reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conv_active = false;
		}
	}
};

DECLARE_SCRIPT(M03_Protect_The_MCT, "Building:int")
{
	bool completed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(completed, 1);
	}

	void Created(GameObject * /*obj*/) override
	{
		completed = false;
	}
	
	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		if (!completed && Commands->Is_A_Star(enemy))
		{
			Vector3 mct_pos;

			/*int id = Commands->Create_Conversation("Protect_The_MCT", 0, 0, true);
			Commands->Join_Conversation(obj, id, true, true, true);
			Commands->Start_Conversation(id, 0);*/
			
			switch (Get_Int_Parameter("Building"))
			{
			case 0: // Refinery
				mct_pos.Set(-154.921f, 15.227f, 2.382f);
				Commands->Set_Innate_Soldier_Home_Location(obj, mct_pos, 10.0f);
				break;
			case 1: // Comm center
				mct_pos.Set(-124.763f, 41.308f, -1.948f);
				Commands->Set_Innate_Soldier_Home_Location(obj, mct_pos, 10.0f);
				break;
			case 2: // Power plant
				mct_pos.Set(-84.583f, 11.975f, -7.074f);
				Commands->Set_Innate_Soldier_Home_Location(obj, mct_pos, 10.0f);
				break;
			}
			completed = true;
		}
	}
};

DECLARE_SCRIPT(M03_Power_Plant, "")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		GameObject * comm_center = Commands->Find_Object(1150002);
		GameObject * refinery = Commands->Find_Object(1150001);
		Commands->Set_Building_Power(comm_center, false);
		Commands->Set_Building_Power(refinery, false);
		GameObject * harvester = Commands->Find_Object(1144518);
		Commands->Send_Custom_Event(obj, harvester, 7800, 7800, 0.0f);

		Commands->Enable_Radar ( true );
		Commands->Send_Custom_Event( obj, Commands->Find_Object (2009818), POWER_KILLED, 0 , 0.0f);
	}
};

DECLARE_SCRIPT(M03_Refinery, "")
{
	void Killed(GameObject * obj, GameObject * /*killer*/) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 311, 3 , 0.0f);
		Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 311, 1 , 0.0f);
	}
};

DECLARE_SCRIPT(M03_Area_Troop_Counter, "")
{
	int area_count[3];
	int killed_count[3];

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(area_count, 1);
		SAVE_VARIABLE(killed_count, 2);
	}

	void Created(GameObject * /*obj*/) override
	{
		area_count[0] = 2;
		area_count[1] = 4;
		area_count[2] = 6;
		killed_count[0] = killed_count[1] = killed_count[2] = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == 1000 && param == 1000)
		{
			int area = -1;
			int target_count = -1;
			Commands->Send_Custom_Event(obj, obj, 5000, (uintptr_t)&area, 0.0f);
			Commands->Send_Custom_Event(obj, obj, 6300, (uintptr_t)&target_count, 0.0f);
			if (area >= 0 && area <= 2)
			{
				area_count[area]--;
				if ((++killed_count[area] % target_count != 0) && area_count[area] <= 0)
				{
					if (area_count[area] < 0)
					{
						area_count[area] = 0;
					}
					Commands->Send_Custom_Event(obj, obj, 6000, 6000, 0.0f);
					area_count[area] += (DIFFICULTY);
				}
			}
		}
		if (type == 7000)
		{
			area_count[param] += (DIFFICULTY + 1);
			killed_count[param] = 0;
		}
	}
};

  ////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////DME////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DECLARE_SCRIPT(M03_Mission_Complete_Zone, "")
{
	bool already_entered;
	
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /*obj*/) override
	{
		already_entered = false;
				
	}
	
	void Entered (GameObject * /*obj*/, GameObject * /*enterer*/) override
	{
		if (!already_entered)
		{
			already_entered = true;
			
			Commands->Mission_Complete ( true );
		}
	}
};

DECLARE_SCRIPT (M03_Zone_Enabled_Spawner, "Spawner_num:int, Control_num:int")
{
	void Entered(GameObject * /*obj*/, GameObject * enterer) override
	{
		int control = Get_Int_Parameter("Control_num");
		GameObject * officer = Commands->Find_Object (control);

		if (enterer == STAR)
		{
			if ((officer) || (Get_Int_Parameter("Control_num") == 0))
			{
				Commands->Enable_Spawner(Get_Int_Parameter("Spawner_num"), true);
			}
		}
	}
};

DECLARE_SCRIPT (M03_Killed_Disabled_Spawner, "Spawner_num:int")
{
	void Killed(GameObject * /*obj*/, GameObject * killer) override
	{
		if (killer == STAR)
		{
			Commands->Enable_Spawner(Get_Int_Parameter("Spawner_num"), false);
		}
	}
};

DECLARE_SCRIPT (M03_Goto_Star, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 3.5f, GOTO_STAR);
	}

	void Timer_Expired (GameObject *obj, int timer_id) override
	{
		if (timer_id == GOTO_STAR)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, 100, GOTO_STAR);
			params.Set_Movement(STAR, RUN, 2.5f);
			params.Set_Attack( STAR, 50, 1, true );
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer(obj, this, 3.0f, GOTO_STAR);
		}
	}
};

DECLARE_SCRIPT (M03_CommCenter_Warning, "")
{
	bool mct_accessed, already_entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( mct_accessed, 1 );
		SAVE_VARIABLE( already_entered, 2 );
	}

	void Created (GameObject * /*obj*/) override
	{
		mct_accessed = false;
		already_entered = false;
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MCT_ACCESSED)
		{
			mct_accessed = true;
		}
	}

	void Entered (GameObject *obj, GameObject * /*enterer*/) override
	{
		if (!mct_accessed && !already_entered)
		{
			already_entered = true;
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208215), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208216), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208217), MCT_ACCESSED, 0 , 0.0f);

			Commands->Set_HUD_Help_Text ( IDS_M03DSGN_DSGN0207I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}
};

DECLARE_SCRIPT (M03_PowerPlant_Warning, "")
{
	bool mct_accessed, already_entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( mct_accessed, 1 );
		SAVE_VARIABLE( already_entered, 2 );
	}

	void Created (GameObject * /*obj*/) override
	{
		mct_accessed = false;
		already_entered = false;
	}

	void Custom(GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MCT_ACCESSED)
		{
			mct_accessed = true;
		}
	}

	void Entered (GameObject *obj, GameObject * /*enterer*/) override
	{
		if (!mct_accessed && !already_entered)
		{
			already_entered = true;
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208218), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208219), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208220), MCT_ACCESSED, 0 , 0.0f);

			Commands->Set_HUD_Help_Text ( IDS_M03DSGN_DSGN0205I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		}
	}
};

DECLARE_SCRIPT (M03_CommCenter_Arrow, "")
{
	bool commcenter_alive, mct_accessed;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( commcenter_alive, 1 );
		SAVE_VARIABLE( mct_accessed, 2 );
	}

	void Created (GameObject * /*obj*/) override
	{
		commcenter_alive = true;
		mct_accessed = false;
	}

	void Timer_Expired (GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == MISSION_FAIL)
		{
			Commands->Mission_Complete(false);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == MCT_ACCESSED)
		{
			mct_accessed = true;

			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208215), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208216), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208217), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208218), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208219), MCT_ACCESSED, 0 , 0.0f);
			Commands->Send_Custom_Event( obj, Commands->Find_Object (1208220), MCT_ACCESSED, 0 , 0.0f);
		}

		if (type == COMM_KILLED)
		{
			if (mct_accessed)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 312, 1 , 0.0f);
			}

			if (!mct_accessed)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 308, 2 , 0.0f);
				Commands->Set_HUD_Help_Text ( IDS_M03DSGN_DSGN0208I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

				Commands->Start_Timer(obj, this, 4.0f, MISSION_FAIL);
			}
		}

		if (type == POWER_KILLED)
		{
			if (mct_accessed)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 309, 1 , 0.0f);
			}

			if (!mct_accessed)
			{
				Commands->Send_Custom_Event( obj, Commands->Find_Object (1100004), 308, 2 , 0.0f);
				Commands->Set_HUD_Help_Text ( IDS_M03DSGN_DSGN0206I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
				
				Commands->Start_Timer(obj, this, 4.0f, MISSION_FAIL);
			}
		}
	}
};

DECLARE_SCRIPT (M03_Mct_Poke, "")
{
	bool already_poked;	
		
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_poked, 1 );		
	}

	void Created (GameObject * /*obj*/) override
	{
		already_poked = false;				
	}

	void Poked(GameObject * obj, GameObject * /*poker*/) override
	{
		if (!already_poked)
		{
			already_poked = true;
			Commands->Send_Custom_Event( obj, Commands->Find_Object (2009818), MCT_ACCESSED, 0 , 0.0f);
		}
	}
};

DECLARE_SCRIPT (M03_Comm_Killed, "")
{
	bool base_entered;
	float max_health;
		
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( base_entered, 1 );
		SAVE_VARIABLE( max_health, 2 );
	}

	void Created (GameObject * obj) override
	{
		base_entered = false;
		max_health = Commands->Get_Health (obj);
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == BASE_ENTERED)
		{
			base_entered = true;
		}
	}
	void Damaged (GameObject *obj, GameObject * /*damager*/, float /*amount*/) override
	{
		if (!base_entered)
		{
			Commands->Set_Health (obj, max_health);
		}
	}
	
	void Killed (GameObject * obj, GameObject * /*killer*/) override
	{
		Commands->Send_Custom_Event( obj, Commands->Find_Object (2009818), COMM_KILLED, 0 , 0.0f);
		Commands->Enable_Radar (true);
	}
};

DECLARE_SCRIPT (M03_Conversation_Zone, "Conv_Num:int")
{
	bool already_entered;
	bool first_conv_played;
		
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( first_conv_played, 2 );
	}

	void Created (GameObject * /*obj*/) override
	{
		already_entered = false;
		first_conv_played = false;
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == ENTERED)
		{
			already_entered = true;
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /*reason*/) override
	{
		if (action_id == 100002)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 300, 1, 0);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 308, 3, 0);
		}

		if (action_id == 100003)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 300, 1, 0);

			Commands->Start_Timer (obj, this, 3.0f, 100000);
		}

		if (action_id == 100004)
		{
			GameObject *objective_controller;
			objective_controller = Commands->Find_Object(1100004);
			if (objective_controller)
			{						
				Commands->Send_Custom_Event(obj, objective_controller, 307, 3, 0.0f);
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if (timer_id == 100000)
		{
			int id = Commands->Create_Conversation("M03CON002", 99, 2000, true);
			Commands->Join_Conversation(NULL, id, true, true, true);
			Commands->Start_Conversation(id, 100002);
			Commands->Monitor_Conversation(obj, id);
		}
	}

	void Entered (GameObject * obj, GameObject * /*enterer*/) override
	{
		if (Get_Int_Parameter("Conv_Num") == 4 && already_entered && !Commands->Has_Key(STAR, 1) && !first_conv_played)
		{
			first_conv_played = true;

			int id = Commands->Create_Conversation("M03CON006", 99, 2000, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Join_Conversation(NULL, id, true, true, true);			
			Commands->Start_Conversation(id, 100006);
			Commands->Monitor_Conversation(obj, id);
		}

		if (!already_entered)
		{
			switch(Get_Int_Parameter("Conv_Num"))
			{
			case 2: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object(1144502), ENTERED, 0, 0);

					Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 300, 1, 0);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 308, 3, 0);

					Commands->Start_Timer (obj, this, 2.0f, 100000);
				}
				break;
			case 3: 
				{
					already_entered = true;

					Commands->Send_Custom_Event(obj, Commands->Find_Object(1100005), ENTERED, 0, 0);

					int id = Commands->Create_Conversation("M03CON003", 99, 2000, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100003);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 4: 
				{
					if (!Commands->Has_Key(STAR, 1))
					{
						already_entered = true;

						Commands->Send_Custom_Event(obj, Commands->Find_Object (2013086), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2013087), ENTERED, 0, 0.0f);

						int id = Commands->Create_Conversation("M03CON004", 99, 2000, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Join_Conversation(STAR, id, true, true, true);
						Commands->Start_Conversation(id, 100004);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 7: 
				{
					if (Commands->Has_Key(STAR, 1))
					{
						already_entered = true;

						Commands->Send_Custom_Event(obj, Commands->Find_Object (2013901), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2013900), ENTERED, 0, 0.0f);
						Commands->Send_Custom_Event(obj, Commands->Find_Object (2013899), ENTERED, 0, 0.0f);

						int id = Commands->Create_Conversation("M03CON007", 99, 2000, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100007);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 9: 
				{
					if (Commands->Has_Key(STAR, 1))
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M03CON009", 99, 2000, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100009);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 16: 
				{
					if (!Commands->Has_Key(STAR, 20))
					{
						already_entered = true;

						int id = Commands->Create_Conversation("M03CON016", 99, 2000, true);
						Commands->Join_Conversation(NULL, id, true, true, true);
						Commands->Start_Conversation(id, 100016);
						Commands->Monitor_Conversation(obj, id);
					}
				}
				break;
			case 22: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M03CON022", 99, 2000, true);
					Commands->Join_Conversation(NULL, id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100022);
					Commands->Monitor_Conversation(obj, id);
				}
				break;
			case 61: 
				{
					already_entered = true;

					int id = Commands->Create_Conversation("M03CON061", 99, 2000, true);
					Commands->Join_Conversation(Commands->Find_Object (2016365), id, true, true, true);
					Commands->Join_Conversation(STAR, id, true, true, true);
					Commands->Start_Conversation(id, 100061);
					Commands->Monitor_Conversation(obj, id);
				}
				break;				
			}
		}
	}
};

DECLARE_SCRIPT(M03_Holograph_EntryZone_JDG, "")
{
	bool entered;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(entered, 1);
	}

	void Created( GameObject * /*obj*/ ) override
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

	void Custom( GameObject * /*obj*/, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == 8000)
		{
			GameObject * holograph = Commands->Find_Object ( 2017192 );
			if (holograph)
			{
				GameObject * kane = Commands->Create_Object_At_Bone ( holograph, "Nod_Kane_HologramHead", "O_ARROW" );
				Commands->Attach_To_Object_Bone( kane, holograph, "O_ARROW" );
				Commands->Disable_All_Collisions ( kane );
				Commands->Set_Facing (kane, Commands->Get_Facing (holograph));
				Commands->Set_Loiters_Allowed( kane, false );
				Commands->Attach_Script(kane, "M03_KaneHead_JDG", "");
				//kane_ID = Commands->Get_ID ( kane );
			}

			//Commands->Destroy_Object ( obj );//one time only zone--cleaning up
		}
	}
};

DECLARE_SCRIPT(M03_KaneHead_JDG, "") //2017221
{
	int kane_conversation02;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(kane_conversation02, 1);
	}

	void Created( GameObject * obj ) override
	{
		if (STAR)
		{
			int id = Commands->Create_Conversation("M03CON061", 99, 30, true);
			Commands->Join_Conversation(obj, id, true, true, true);
			Commands->Join_Conversation(STAR, id, true, true, true);
			Commands->Start_Conversation(id, 100061);
			Commands->Monitor_Conversation(obj, id);
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason ) override
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
	}

	void Custom( GameObject * obj, int /*type*/, intptr_t param, GameObject * /*sender*/ ) override
	{
		if (param == 8000)//conversation is over--go away
		{
			Commands->Debug_Message ( "***************************delete custom received--kane should now vanish\n" );
			Commands->Destroy_Object ( obj );
		}
	}
};

DECLARE_SCRIPT(M03_Killed_Sound, "Officer=0:int, Location=0:int")
{
	void Killed( GameObject * obj, GameObject * /*killer*/ ) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object (2018061), LOCATION, Get_Int_Parameter("Location"), 0.0f );
		Commands->Send_Custom_Event (obj, Commands->Find_Object (2018061), TROOP_KILLED, Get_Int_Parameter("Officer"), 0.0f );
	}
};

DECLARE_SCRIPT(M03_Beach_Reinforce, "")
{
	#define BEACH_POSITION Vector3(-95.656f, -68.236f, 1.433f)
	#define BEACH_FACING -180.0f
	#define INLET_POSITION1 Vector3(99.85f, -49.51f, 2.0f)
	#define INLET_FACING1 170.0f
	#define INLET_POSITION2 Vector3(51.0f, -91.0f, 2.0f)
	#define INLET_FACING2 75.0f
	#define BASE_POSITION1 Vector3(-138.0f, 50.0f, 9.5f)
	#define BASE_FACING1 -30.0f
	#define BASE_POSITION2 Vector3(-79.0f, 59.0f, 9.5f)
	#define BASE_FACING2 -120.0f
	
	int current_loc;
	int counter;
	int switcher;
	int officers_killed_beach;
	int officers_killed_inlet;
	int officers_killed_base;
	int area_count_beach;
	int area_count_inlet;
	int area_count_base;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(counter, 1);
		SAVE_VARIABLE(switcher, 2);
		SAVE_VARIABLE(officers_killed_beach, 3);
		SAVE_VARIABLE(officers_killed_inlet, 4);
		SAVE_VARIABLE(officers_killed_base, 5);
		SAVE_VARIABLE(area_count_beach, 6);
		SAVE_VARIABLE(area_count_inlet, 7);
		SAVE_VARIABLE(area_count_base, 8);
		SAVE_VARIABLE(current_loc, 9);
	}

	void Created (GameObject * /*obj*/) override
	{	
		current_loc = 0;
		counter = 2;
		switcher = 0;
		officers_killed_beach = 0;
		officers_killed_inlet = 0;
		officers_killed_base = 0;
		area_count_beach = (DIFFICULTY + 2);
		area_count_inlet = (DIFFICULTY + 3);
		area_count_base = (DIFFICULTY + 4);
	}

	void Reinforce_Beach (void)
	{
		if (officers_killed_beach < 3 && area_count_beach > 0)
		{
			area_count_beach--;

			if (switcher < 1)
			{
				switcher++;

				GameObject * con = Commands->Create_Object("Invisible_Object", BEACH_POSITION);
				Commands->Set_Facing(con, BEACH_FACING);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			}

			if (switcher == 1)
			{
				switcher = 0;

				Commands->Trigger_Spawner( 2018881 );
				Commands->Trigger_Spawner( 2018880 );
			}

			if (switcher > 1)
			{
				GameObject * con = Commands->Create_Object("Invisible_Object", BEACH_POSITION);
				Commands->Set_Facing(con, BEACH_FACING);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			}
		}
	}

	void Reinforce_Inlet (void)
	{
		if (officers_killed_inlet < 3 && area_count_inlet > 0)
		{
			area_count_inlet--;

			if (switcher >= 1)
			{
				switcher--;

				GameObject * con = Commands->Create_Object("Invisible_Object", INLET_POSITION1);
				Commands->Set_Facing(con, INLET_FACING1);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper2");
			}

			if (switcher < 1)
			{
				switcher = 1;

				GameObject * con = Commands->Create_Object("Invisible_Object", INLET_POSITION2);
				Commands->Set_Facing(con, INLET_FACING2);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper2");
			}
		}
	}

	void Reinforce_Base (void)
	{
		if (officers_killed_base < 3 && area_count_base > 0)
		{
			area_count_base--;

			if (switcher < 1)
			{
				switcher++;

				GameObject * con = Commands->Create_Object("Invisible_Object", BASE_POSITION1);
				Commands->Set_Facing(con, BASE_FACING1);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			}

			if (switcher >= 1)
			{
				switcher = 0;

				GameObject * con = Commands->Create_Object("Invisible_Object", BASE_POSITION2);
				Commands->Set_Facing(con, BASE_FACING2);
				Commands->Attach_Script(con, "M03_Chinook_ParaDrop", "M03_Paratrooper");
			}
		}
	}	

	void Custom (GameObject *obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == LOCATION)
		{
			current_loc = param;
		}

		if (type == BASE)
		{
			if (param == 1)
			{
				Reinforce_Base ();
				//counter = 4;
			}
		}

		if (type == BEACH)
		{
			if (param == 1)
			{
				Reinforce_Beach ();
				//counter = 4;
			}
		}
		
		if (type == INLET)
		{
			if (param == 1)
			{
				Reinforce_Inlet ();
				//counter = 3;
			}
		}

		if (type == TROOP_KILLED)
		{
			if (param == 1)
			{
				if (current_loc == 0)
				{
					officers_killed_beach++;
				}

				if (current_loc == 1)
				{
					officers_killed_inlet++;
				}

				if (current_loc == 2)
				{
					officers_killed_base++;
				}
			}

			if (--counter <= 0)
			{
				counter = (5 - DIFFICULTY);
				Commands->Send_Custom_Event (obj, Commands->Find_Object (1144444), UPDATE, 0, 0.0f);

				//Reinforce_Beach ();
				//counter = 2;
			}
		}

		if (type == PAST_PILLBOX)
		{
			switcher = 10;
		}
	}
};

DECLARE_SCRIPT(M03_Past_Pillbox, "")
{
	void Entered( GameObject * obj, GameObject * /*enterer*/ ) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object (2018061), PAST_PILLBOX, 0, 0.0f);
	}
};

DECLARE_SCRIPT(M03_Engineer_Target, "")
{
	float full_health;
	int engineer;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(engineer, 1);
		SAVE_VARIABLE(full_health, 2);
	}

	void Created (GameObject *obj) override
	{
		full_health = Commands->Get_Health (obj);
	}

	void Custom (GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == ENGINEER)
		{
			engineer = param;
		}
	};

	void Damaged (GameObject *obj, GameObject * /*damager*/, float /*amount*/) override
	{
		Commands->Create_Logical_Sound(obj, HEAL_ME, Commands->Get_Position (obj), 150.0f);
		Commands->Start_Timer (obj, this, 3.0f, HEALTH_CHECK);
	}

	void Destroyed(GameObject * obj) override
	{
		Commands->Create_Logical_Sound(obj, STOP_REPAIR, Commands->Get_Position (obj), 150.0f);
	}


	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if (timer_id == HEALTH_CHECK)
		{
			float current = Commands->Get_Health (obj);

			if (current == full_health)
			{
				Commands->Create_Logical_Sound(obj, STOP_REPAIR, Commands->Get_Position (obj), 150.0f);
			}

			if (current != full_health)
			{
				Commands->Create_Logical_Sound(obj, HEAL_ME, Commands->Get_Position (obj), 150.0f);
				Commands->Start_Timer (obj, this, 3.0f, HEALTH_CHECK);
			}
		}
	}
};

DECLARE_SCRIPT(M03_Engineer_Repair, "Repair_Priority=96:int")
{
	int my_id, target_id;
	bool repairing;
	int repair_priority;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(repairing, 1);
		SAVE_VARIABLE(my_id, 2);
		SAVE_VARIABLE(target_id, 3);
		SAVE_VARIABLE(repair_priority, 4);
	}

	void Created (GameObject * /*obj*/) override
	{
		repairing = false;
		repair_priority = Get_Int_Parameter("Repair_Priority");
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		if (sound.Type == HEAL_ME && !repairing)
		{
			if (Commands->Get_Player_Type (sound.Creator) == Commands->Get_Player_Type (obj))
			{
				my_id = Commands->Get_ID ( obj );
				repairing = true;
				Commands->Send_Custom_Event( obj, sound.Creator, ENGINEER, my_id , 0.0f);

				target_id = Commands->Get_ID ( sound.Creator );

				ActionParamsStruct params;
				
				params.Set_Basic( this, repair_priority, MOVE_TO_HEAL );
				params.Set_Movement( Commands->Get_Position (sound.Creator), RUN, 5.0f );

				Commands->Action_Goto (obj, params);
			}
		}

		if (sound.Type == STOP_REPAIR)
		{
			if (Commands->Get_ID (sound.Creator) == target_id)
			{
				Commands->Action_Reset ( obj, 100 );

				repairing = false;
			}
		}
	}

	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/ ) override
	{
		if (action_id == MOVE_TO_HEAL)
		{
			ActionParamsStruct params;

			params.Set_Basic( this, repair_priority, REPAIRING );
			params.Set_Attack (Commands->Find_Object (target_id), 50.0f, 0.0f, 0);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}
};

DECLARE_SCRIPT(M03_Radar_Scramble, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /*obj*/) override
	{
		already_entered = false;				
	}

	void Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if (type == ENTERED)
		{
			already_entered = true;
		}
	}

	void Entered (GameObject *obj, GameObject * /*enterer*/) override
	{
		if (Commands->Get_Health (Commands->Find_Object (1150002)) > 0)
		{
			if (!already_entered)
			{
				Commands->Send_Custom_Event (obj, Commands->Find_Object (1147513), ENTERED, 0, 0.0f);
				Commands->Send_Custom_Event (obj, Commands->Find_Object (1144628), ENTERED, 0, 0.0f);
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

DECLARE_SCRIPT(M03_Radar_UnScramble, "")
{
	void Entered (GameObject * /*obj*/, GameObject * /*enterer*/) override
	{
		Commands->Enable_Radar ( true );
	}
};

DECLARE_SCRIPT (M10_Elevator_All_Zone, "Controller_num:int")
{
	bool star_in_zone;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( star_in_zone, 1 );
	}

	void Created (GameObject * /*obj*/) override
	{
		star_in_zone = false;
	}


	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (enterer == STAR)
		{
			star_in_zone = true;
			Commands->Send_Custom_Event(obj, Commands->Find_Object(Get_Int_Parameter("Controller_num")), ACTIVATE, 0, 0.0f);
		}		
	}
};

DECLARE_SCRIPT (M10_Elevator_All_Controller, "")
{
	bool star_in_zone;
	bool mobius_in_zone;
	bool transition;

	int block1, block2, block3, block4;
	int block5, block6, block7, block8;

	GameObject *mobius;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( star_in_zone, 1 );
		SAVE_VARIABLE( mobius_in_zone, 2 );
		SAVE_VARIABLE( transition, 3 );
		SAVE_VARIABLE( block1, 4 );
		SAVE_VARIABLE( block2, 5 );
		SAVE_VARIABLE( block3, 6 );
		SAVE_VARIABLE( block4, 7 );
		SAVE_VARIABLE( block5, 8 );
		SAVE_VARIABLE( block6, 9 );
		SAVE_VARIABLE( block7, 10 );
		SAVE_VARIABLE( block8, 11 );
	}

	void Created (GameObject * /*obj*/) override
	{
		star_in_zone = false;
		mobius_in_zone = false;

		block1 = block2 = block3 = block4 = 0;
		block5 = block6 = block7 = block8 = 0;

		Commands->Static_Anim_Phys_Goto_Last_Frame ( 1300881, "cave_lift.cave_lift" );


	}

	void Timer_Expired(GameObject * /*obj*/, int timer_id) override
	{
		if (timer_id == 333)
		{
			Commands->Destroy_Object (Commands->Find_Object (1148346));

			Commands->Destroy_Object (Commands->Find_Object (block1));
			Commands->Destroy_Object (Commands->Find_Object (block2));
			Commands->Destroy_Object (Commands->Find_Object (block3));
			Commands->Destroy_Object (Commands->Find_Object (block4));
			Commands->Destroy_Object (Commands->Find_Object (block5));
			Commands->Destroy_Object (Commands->Find_Object (block6));
			Commands->Destroy_Object (Commands->Find_Object (block7));
			Commands->Destroy_Object (Commands->Find_Object (block8));
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		
		if (type == ACTIVATE)
		{
			Commands->Start_Timer (obj, this, 5.0f, 333);

			GameObject *rub1 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-117.320f, 83.098f, 0.656f));
			block1 = Commands->Get_ID (rub1);
			Commands->Set_Facing (rub1, 90.0);
			Commands->Set_Is_Rendered( rub1, false );
			GameObject *rub2 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-113.706f, 78.884f, 0.656f));
			block2 = Commands->Get_ID (rub2);
			Commands->Set_Is_Rendered( rub2, false );
			GameObject *rub3 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-117.624f, 73.787f, 0.656f));
			block3 = Commands->Get_ID (rub3);
			Commands->Set_Is_Rendered( rub3, false );
			Commands->Set_Facing (rub3, -90.0);
			GameObject *rub4 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-122.458f, 78.303f, 0.656f));
			block4 = Commands->Get_ID (rub4);
			Commands->Set_Is_Rendered( rub4, false );
			GameObject *rub5 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-117.320f, 83.098f, 3.906f));
			block5 = Commands->Get_ID (rub5);
			Commands->Set_Facing (rub5, 90.0);
			Commands->Set_Is_Rendered( rub5, false );
			GameObject *rub6 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-113.706f, 78.884f, 3.906f));
			block6 = Commands->Get_ID (rub6);
			Commands->Set_Is_Rendered( rub6, false );
			GameObject *rub7 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-117.624f, 73.787f, 3.906f));
			block7 = Commands->Get_ID (rub7);
			Commands->Set_Is_Rendered( rub7, false );
			Commands->Set_Facing (rub7, -90.0);
			GameObject *rub8 = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-122.458f, 78.303f, 3.906f));
			block8 = Commands->Get_ID (rub8);
			Commands->Set_Is_Rendered( rub8, false );


			Commands->Static_Anim_Phys_Goto_Frame ( 1300881, 0, "cave_lift.cave_lift" );
		}

		
	}
};

DECLARE_SCRIPT(M03_AggAndCover, "")
{
	void Created (GameObject *obj) override
	{
		Commands->Set_Innate_Aggressiveness ( obj, 50.0f );
		Commands->Set_Innate_Take_Cover_Probability ( obj, 100.0f );
	}
};

DECLARE_SCRIPT(M03_Technician_Work, "")
{
	void Created (GameObject *obj) override
	{
		ActionParamsStruct params;
		params.Set_Basic( this, INNATE_PRIORITY_BULLET_HEARD - 5, 1);
		params.Set_Animation( "H_A_a0f0", true );
		Commands->Action_Play_Animation (obj, params);
	}
};

DECLARE_SCRIPT(M03_Paratrooper_Run, "")
{
	bool initial_damage;
	float initial_health;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( initial_damage, 1 );
		SAVE_VARIABLE( initial_health, 2 );
	}

	void Created( GameObject * /*obj*/ ) override
	{
		initial_damage = false;
		
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (!initial_damage)
		{
			if (type == CUSTOM_EVENT_FALLING_DAMAGE)
			{
				initial_health = Commands->Get_Health(obj);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject *damager, float /*amount*/) override
	{
		if (!initial_damage && damager == NULL)
		{
			initial_damage = true;
			Commands->Set_Health(obj, initial_health);

			float distance = Commands->Get_Random(-12.0f, 12.0f);
			//distance *= 3;
			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * distance;
			float b = sin(DEG_TO_RADF(facing)) * distance;
			Vector3 goto_loc = pos + Vector3(a, b, 0.0f);

			ActionParamsStruct params;
			params.Set_Basic(this, 100, 40);
			params.Set_Movement(goto_loc, RUN, 1.0f);
			Commands->Action_Goto(obj, params);
		}
	}
};

DECLARE_SCRIPT (M03_Damage_Modifier_All, "Damage_multiplier:float")
{
	float current_health, last_health, damage_tally;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( current_health, 1 );
		SAVE_VARIABLE( last_health, 2 );
		SAVE_VARIABLE( damage_tally, 3 );
	}

	void Created (GameObject *obj) override
	{
		last_health = Commands->Get_Health (obj);
		damage_tally = 0;
	}

	void Damaged( GameObject * obj, GameObject * /*damager*/, float /*amount*/ ) override 
	{
		float damage;

		
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
};

DECLARE_SCRIPT(M03_Cine_Explosion, "")
{
	void Custom (GameObject *obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == 600)
		{
			Commands->Create_Explosion( "M03_Cine_Explosion", Commands->Get_Position (obj), obj );
		}
	}
};

DECLARE_SCRIPT(M03_ConYardSeen, "")
{
	bool already_entered;
	
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );		
	}

	void Created (GameObject * /*obj*/) override
	{
		already_entered = false;				
	}

	void Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if (type == 900)
		{
			already_entered = true;
		}
	}

	void Entered (GameObject *obj, GameObject * /*enterer*/) override
	{
		if (!already_entered)
		{
			already_entered = true;
			Commands->Send_Custom_Event(obj, Commands->Find_Object(1100004), 300, 1, 0);
		}
	}
};

DECLARE_SCRIPT(M03_DataDiscMessage, "")
{
	void Custom( GameObject * /*obj*/, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text ( IDS_M00EVAG_DSGN0104I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY  );
		}
	}
};