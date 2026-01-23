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
*     Mission05.cpp
*
* DESCRIPTION
*     Mission 5 script
*
* PROGRAMMER
*     Rich Donnelly
*
* VERSION INFO
*     $Author: Darren_k $
*     $Revision: 266 $
*     $Modtime: 1/12/02 1:52p $
*     $Archive: /Commando/Code/Scripts/Mission05.cpp $
*
******************************************************************************/

#include "Mission5.h"
#include "Toolkit.h"
#include "scripts.h"


// M05C

DECLARE_SCRIPT(M05_Objective_Controller, "")  // 100001
{

	bool m05_512_failed;

	enum {HAVOCS_SCRIPT, HOTWIRE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m05_512_failed, 1);
	}

	void Created(GameObject * obj) override
	{
		// Background Music
		// Commands->Set_Background_Music ("05-moveit.mp3");
		Commands->Set_Background_Music ("05-moveit.mp3");

		Commands->Enable_Hibernation(obj, false);
		Commands->Start_Timer (obj, this, 1.0f, HAVOCS_SCRIPT);
		Commands->Start_Timer (obj, this, 1.0f, HOTWIRE);
		m05_512_failed = false;

		// Capture Nod Vehicles
		Add_An_Objective(514);
		Add_An_Objective(515);
		Add_An_Objective(516);
		Add_An_Objective(517);

		

		// Drop Mendoza
		GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-555.290f, -89.967f, 31.411f));
		Commands->Set_Facing(chinook_obj1, 100.000f);
		Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5I_TroopDrop7.txt");
	}

	void Add_An_Objective(int id)
	{
		GameObject *object;
		
		switch (id)
		{
		// Contact Gunner
		case 501: 
			{
				Commands->Add_Objective(501, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M05_02, NULL, IDS_Enc_Obj_Primary_M05_02);
				object = Commands->Find_Object(100003);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0175I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(501, object);
					Commands->Set_Objective_HUD_Info_Position(501, 90.0f, "POG_M05_1_04.tga", IDS_POG_CONTACT, Commands->Get_Position (object));
				}
			}
			break;
		// Contact Deadeye
		case 502: 
			{
				Commands->Add_Objective(502, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M05_03, NULL, IDS_Enc_Obj_Primary_M05_03);
				object = Commands->Find_Object(100004);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0177I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(502, object);
					Commands->Set_Objective_HUD_Info_Position(502, 90.0f, "POG_M05_1_05.tga", IDS_POG_CONTACT, Commands->Get_Position (object));
				}
			}
			break;
		// Rescue Hotwire
		case 503: 
			{
				Commands->Add_Objective(503, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M05_01, NULL, IDS_Enc_Obj_Primary_M05_01);
				object = Commands->Find_Object(100002);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0173I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(503, object);
					Commands->Set_Objective_HUD_Info_Position(503, 90.0f, "POG_M05_1_03.tga", IDS_POG_RESCUE, Commands->Get_Position (object));
				}
			}
			break;
		// Contact Patch
		case 504: 
			{
				Commands->Add_Objective(504, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M05_04, NULL, IDS_Enc_Obj_Primary_M05_04);
				object = Commands->Find_Object(100006);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(504, object);
					Commands->Set_Objective_HUD_Info_Position(504, 90.0f, "POG_M05_1_02.tga", IDS_POG_CONTACT, Commands->Get_Position (object));
				}
			}
			break;
		// Eliminate Black Hand
		case 506: 
			{
				Commands->Add_Objective(506, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M05_05, NULL, IDS_Enc_Obj_Primary_M05_05);
				object = Commands->Find_Object(100287);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(506, object);
					Commands->Set_Objective_HUD_Info_Position(506, 90.0f, "POG_M05_1_01.tga", IDS_POG_ELIMINATE, Commands->Get_Position (object));
				}
			}
			break;
		// Secure Town Square
		case 507: 
			{
				Commands->Add_Objective(507, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M05_01, NULL, IDS_Enc_Obj_Secondary_M05_01);
				object = Commands->Find_Object(100112);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(507, object);
					Commands->Set_Objective_HUD_Info_Position(507, 90.0f, "POG_M05_2_01.tga", IDS_POG_SECURE, Commands->Get_Position (object));
				}
			}
			break;
		// Escort Resistance
		case 508: 
			{
				Commands->Add_Objective(508, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M05_02, NULL, IDS_Enc_Obj_Secondary_M05_02);
				object = Commands->Find_Object(101261);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(508, object);
					Commands->Set_Objective_HUD_Info_Position(508, 90.0f, "POG_M05_2_07.tga", IDS_POG_ESCORT, Commands->Get_Position (object));
				}
			}
			break;
		// Contact Escapee
		case 509: 
			{
				Commands->Add_Objective(509, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M05_03, NULL, IDS_Enc_Obj_Secondary_M05_03);
				object = Commands->Find_Object(100038);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(509, object);
					Commands->Set_Objective_HUD_Info_Position(509, 90.0f, "POG_M05_2_03.tga", IDS_POG_CONTACT, Commands->Get_Position (object));
				}
			}
			break;
		// Contact Babushka
		case 510: 
			{
				Commands->Add_Objective(510, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M05_05, NULL, IDS_Enc_Obj_Secondary_M05_05);
				object = Commands->Find_Object(100050);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(510, object);
					Commands->Set_Objective_HUD_Info_Position(510, 90.0f, "POG_M05_2_06.tga", IDS_POG_CONTACT, Commands->Get_Position (object));
				}
			}
			break;
		// Eliminate Engineers
		case 512: 
			{
				Commands->Add_Objective(512, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M05_04, NULL, IDS_Enc_Obj_Secondary_M05_04);
				object = Commands->Find_Object(100632);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(512, object);
					Commands->Set_Objective_HUD_Info_Position(512, 90.0f, "POG_M05_2_04.tga", IDS_POG_ELIMINATE, Commands->Get_Position (object));
				}
			}						
			break;
		
		}
		
	}

	void Remove_Pog(int id)
	{
		switch (id)
		{
		// Rescue Dead-6 Special Forces Unit - Gunner
		case 501: 
			{
				Commands->Set_Objective_HUD_Info(501, -1, "POG_M05_1_04.tga", IDS_POG_RESCUE);
			}
			break;
		// Rescue Dead-6 Special Forces Unit - Deadeye
		case 502: 
			{
				Commands->Set_Objective_HUD_Info(502, -1, "POG_M05_1_05.tga", IDS_POG_RESCUE);
			}
			break;
		// Rescue Dead-6 Special Forces Unit - Hotwire
		case 503: 
			{
				Commands->Set_Objective_HUD_Info(503, -1, "POG_M05_1_03.tga", IDS_POG_RESCUE);
			}
			break;
		// Rendezvous with Special Forces Unit - Patch
		case 504: 
			{
				Commands->Set_Objective_HUD_Info(501, -1, "POG_M05_1_02.tga", IDS_POG_RENDEZVOUS);
			}
			break;
		// Repel Black Hand assault
		case 506: 
			{
				Commands->Set_Objective_HUD_Info(501, -1, "POG_M05_1_02.tga", IDS_POG_RENDEZVOUS);
			}
			break;
		// Assist village resistance in retaking Town Square
		case 507: 
			{
				Commands->Set_Objective_HUD_Info(507, -1, "POG_M05_2_01.tga", IDS_POG_ASSIST);
			}
			break;
		// Escort village resistance to crashed Nod Transport Helicopter
		case 508: 
			{
				Commands->Set_Objective_HUD_Info(508, -1, "POG_M05_2_07.tga", IDS_POG_ESCORT);
			}
			break;
		// Contact Chateau escapee and gather intelligence
		case 509: 
			{
				Commands->Set_Objective_HUD_Info(509, -1, "POG_M05_2_03.tga", IDS_POG_CONTACT);
			}
			break;
		// Free Babushka, village resistance leader
		case 510: 
			{
				Commands->Set_Objective_HUD_Info(510, -1, "POG_M05_2_06.tga", IDS_POG_FREE);
			}
			break;
		// Eliminate Nod Engineers in Village Park
		case 512: 
			{
				Commands->Set_Objective_HUD_Info(512, -1, "POG_M05_2_04.tga", IDS_POG_ELIMINATE);
			}						
			break;
		// Commandeer Nod Light Tank
		case 514: 
			{
				
			}
			break;
		
		}
	}
	
	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		switch (param)
		{
			case 1: 
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_ACCOMPLISHED);
					Remove_Pog(type);
					if(type == 506)
					{
						Commands->Mission_Complete ( true );
					}
				break;
			case 2: 
					if(type == 512 && !m05_512_failed)
					{
						m05_512_failed = true;
					}
					else if(type == 512 && m05_512_failed)
					{
						return;
					}
					if(type == 503)
					{
						Commands->Mission_Complete ( false );
					}
					if(type == 501)
					{
						Commands->Mission_Complete ( false );
					}
					if(type == 502)
					{
						Commands->Mission_Complete ( false );
					}
					Commands->Clear_Radar_Marker (type);
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_FAILED);
					Remove_Pog(type);
					
				break;
			case 3: Add_An_Objective(type);
				break;
			case 4: Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_PENDING);
				break;
		}

		if(type == M05_RELOCATE)
		{
			if(param == 0)
			{
				//Hotwire Midtro
				Commands->Set_Position (STAR, Commands->Get_Position(Commands->Find_Object(108474)));
			}
			if(param == 1)
			{
				// Gunner Midtro
				Commands->Set_Position (STAR, Commands->Get_Position(Commands->Find_Object(108475)));
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		
		if(timer_id == HAVOCS_SCRIPT)
		{
			Commands->Attach_Script(STAR, "M05_Havoc_DLS", "");
			
		}
		if(timer_id == HOTWIRE)
		{
			// Hotwire here, I can't rendezvous at your position, I've got tons of armor pounding my location in the southwest quarter.\n
			const char *conv_name = ("M05_CON001");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300503);
			Commands->Monitor_Conversation (obj, conv_id);	
		}
		
	}

	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == 300503)
		{
			Add_An_Objective(503);
		}
		
		
	}

};

//Grants initial weapons
DECLARE_SCRIPT(M05_Havoc_DLS, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( just_sent, 1 );
	}

	void Created(GameObject * obj) override
	{
	
		Commands->Give_PowerUp(obj, "POW_Chaingun_Player", false);
		Commands->Give_PowerUp(obj, "POW_SniperRifle_Player", false);
				
	}

};

DECLARE_SCRIPT(M05_Activate_Objective_501, "")
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

			// This is Gunner, I'm pinned down under heavy fire on the south side of the village, requesting backup!\n
			const char *conv_name = ("M05_CON003");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300501);
			Commands->Monitor_Conversation (obj, conv_id);	
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300501 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 501, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_Activate_Objective_502, "")
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

			// This is Deadeye - I'm held up in the Fancy Inn on the east side with some local resistance fighters. Need further assistance.\n
			const char *conv_name = ("M05_CON004");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f)), conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300502);
			Commands->Monitor_Conversation (obj, conv_id);	
		
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300502 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 502, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_Activate_Objective_504, "")
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

			// Havoc, this is Patch.  I'm at the Cathedral. Nod's gathering for major bloodletting. Get up here.\n
			const char *conv_name = ("M05_CON005");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300504);
			Commands->Monitor_Conversation (obj, conv_id);	
		
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300504 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 504, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_Activate_Objective_507, "")
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

			// Attention GDI forces, this is Resistance Radio. Nod is assaulting the Town Square, on the south side of village. Requesting assistance.\n
			const char *conv_name = ("M05_CON006");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300507);
			Commands->Monitor_Conversation (obj, conv_id);	
			
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300507 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 507, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_Activate_Objective_508, "")
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

			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
			Commands->Attach_Script(controller, "Test_Cinematic", "X5B_CH_DOWN.txt");

			// This is Nod supply helo Delta-12, possible rooftop sighting of terrorists in quadrant 4.  Are those rocket launchers? Evasive- <explosion and static>\n
			const char *conv_name = ("M05_CON007");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300501);
			Commands->Monitor_Conversation (obj, conv_id);
		}
	}

	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300501 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
//			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 508, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M05_Activate_Nod_Apc, "")
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

			// Activate corridor APC onto waypath
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100144), M05_CUSTOM_ACTIVATE, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M05_Activate_Objective_509, "")
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

			// GDI forces, this is Resistance Radio. Communications indicate an escapee from the Chateau is in our custody on the northern side of the village. He has valuable information, make contact if needed.\n
			const char *conv_name = ("M05_CON038");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300509);
			Commands->Monitor_Conversation (obj, conv_id);


			// Buggy and Tank
			Commands->Enable_Spawner(100164, true);

			// Paradrop cache
			GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-495.497f, 40.504f, 22.714f));
			Commands->Set_Facing(controller1, -5.000f);
			Commands->Attach_Script(controller1, "Test_Cinematic", "X5D_CHTroopdrop3.txt");

			// Paradrop cache
			GameObject *controller2 = Commands->Create_Object("Invisible_Object", Vector3(-401.111f, 43.222f, 20.018f));
			Commands->Set_Facing(controller2, -180.000f);
			Commands->Attach_Script(controller2, "Test_Cinematic", "X5D_CHTroopdrop4.txt");
			
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300509 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 509, 3, 0.0f);
		}
		
		
	}

};

DECLARE_SCRIPT(M05_Activate_Objective_510, "")
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

			// How's this work? Okay... Attention, this is Resistance Radio. Rumours are true, our leader Babushka is kidnapped. Is anybody around to help? Hello? Is this on?
			const char *conv_name = ("M05_CON008");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f)), conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300510);
			Commands->Monitor_Conversation (obj, conv_id);	
		
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300510 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 510, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_DEAD6_Engineer, "") // first hotwire
{
	int poke_id;
	bool conversation;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( conversation, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		poke_id = 1;
		conversation = false;
		Commands->Set_Innate_Is_Stationary(obj, true);

		Commands->Enable_HUD_Pokable_Indicator( obj, true );
		Commands->Set_Player_Type( obj, SCRIPT_PLAYERTYPE_NEUTRAL );
		Commands->Set_Obj_Radar_Blip_Color ( obj, RADAR_BLIP_COLOR_GDI );
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0174I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 503, 2, 1.0f);
		
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch(poke_id)
			{
			case 1:
				{
					// Work your way to the Cathedral and secure comm link to Locke.\n
					const char *conv_name = ("M05_CON009");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(Commands->Get_A_Star(Vector3(0.0f,0.0f,0.0f)), conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300551);
					Commands->Monitor_Conversation (obj, conv_id);	

					poke_id = 2;
				}
				break;
			}

		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_DESTROY_OBJECT)
		{
			Commands->Destroy_Object(obj);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == 300551)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 503, 1, 0.0f);
			Commands->Destroy_Object(obj);
		}
		
		
	}
	
};

DECLARE_SCRIPT(M05_DEAD6_Engineer2, "")
{
	
	enum{GO_STAR, ATTACK_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2.0f);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0174I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 503, 2, 1.0f);
		Commands->Mission_Complete ( false );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_PROTECT_HAVOC)
		{
			Commands->Start_Timer (obj, this, 5.0f, GO_STAR);
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 40.0f);
		}

	}

	

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		float distance = Commands->Get_Distance (Commands->Get_Position(obj), Commands->Get_Position(STAR));
		if(timer_id == GO_STAR && distance > 30.0f)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), GO_STAR);
			params.Set_Movement (STAR, RUN, 30.0f);
			Commands->Action_Goto (obj, params);
		}
		else
		{
			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_STAR && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), ATTACK_STAR);
			params.Set_Movement (STAR, RUN, 3.0f);
			Commands->Action_Goto (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}
	
};

DECLARE_SCRIPT(M05_DEAD6_Rocket_Soldier2, "")
{
	enum{GO_STAR, ATTACK_STAR};

	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2.0f);
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		if(Commands->Get_Health(obj) < 10.0f)
		{
			Commands->Set_Health(obj, 10.0f);
		}
		if(Commands->Get_Health(obj) < (.3 * Commands->Get_Max_Health(obj)))
		{
			Commands->Apply_Damage( obj, -10000.0f, "STEEL", NULL );
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0176I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 503, 2, 1.0f);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_PROTECT_HAVOC)
		{
			Commands->Start_Timer (obj, this, 5.0f, GO_STAR);
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 40.0f);
		}

	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		float distance = Commands->Get_Distance (Commands->Get_Position(obj), Commands->Get_Position(STAR));
		if(timer_id == GO_STAR && distance > 30.0f)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), GO_STAR);
			params.Set_Movement (STAR, RUN, 30.0f);
			Commands->Action_Goto (obj, params);
		}
		else
		{
			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_STAR && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), ATTACK_STAR);
			params.Set_Movement (STAR, RUN, 3.0f);
			Commands->Action_Goto (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}
};

DECLARE_SCRIPT(M05_DEAD6_Rocket_Soldier, "") // first gunner
{
	int poke_id;
	bool conversation;
	bool immortal;
	int healed;

	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( conversation, 2 );
		SAVE_VARIABLE( immortal, 3 );
		SAVE_VARIABLE( healed, 4 );
	}

	void Created (GameObject * obj) override
	{
		poke_id = 1;
		conversation = false;
		immortal = true;
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Enable_HUD_Pokable_Indicator( obj, true );	
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Debug_Message("Gunner in Town Square was Killed. \n");
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0176I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 501, 2, 1.0f);
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch (poke_id)
			{
			case 1: 
				{
					// I'm going to take this town if I have to vaporize every Nod reject in existence. I assume you're assisting?\n
					const char *conv_name = ("M05_CON010");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300001);
					Commands->Monitor_Conversation (obj, conv_id);
					
					poke_id = 2;
					immortal = false;
				}
				break;
			case 2: 
				{
					// We won't make it out of here if we don't clear the area.\n
					const char *conv_name = ("M05_CON011");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300002);
					Commands->Monitor_Conversation (obj, conv_id);	
				}
				break;
			case 3: 
				{
					Vector3 havoc_loc = Commands->Get_Position (STAR);
					havoc_loc.Z += 7.0f;
					Commands->Set_Position (STAR, havoc_loc );

					GameObject * cinematic_obj = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
					Commands->Attach_Script(cinematic_obj, "Test_Cinematic", "X5M_MIDTRO_A.txt");

					Commands->Destroy_Object(obj);

					// Gunner, make your way to the Cathedral, we'll rendezvous there.\n
				//	const char *conv_name = ("M05_CON012");
				//	int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				//	Commands->Join_Conversation(Commands->Get_A_Star(Commands->Get_Position(obj)), conv_id, false, true, true);
				//	Commands->Join_Conversation(obj, conv_id, false, true, true);
				//	Commands->Start_Conversation (conv_id, 300003);
				//	Commands->Monitor_Conversation (obj, conv_id);	
				}
				break;
			
			}

		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 500 && param == 500)
		{
			// Village Resistance has seized town square
			poke_id = 3;
		}
		if(type == 500 && param == 501)
		{
			// Nod has seized town square
			poke_id = 4;
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == 300003 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 501, 1, 0.0f);
			Commands->Destroy_Object(obj);
		}
		
	}

	
};

DECLARE_SCRIPT(M05_DEAD6_MiniGunner, "") // deadeye
{
	int poke_id;
	bool immortal;
	bool conversation;
	bool hold;
	bool objective_513;
	int move_loc[6];
	int current_move_loc;

	enum {DESTROY_WINDOW, GO_WINDOW, GO_STAR, DEADEYE_MOVE_LOC, MOVE_LOC_LOW_PRIORITY};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( immortal, 2 );
		SAVE_VARIABLE( conversation, 3 );
		SAVE_VARIABLE( hold, 4 );
		SAVE_VARIABLE( objective_513, 5 );
		SAVE_VARIABLE( current_move_loc, 6 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
			
		objective_513 = false;
		poke_id = 1;
		hold = false;
		immortal = true;
		conversation = false;
		current_move_loc = 0;

		move_loc[0] = 101332;
		move_loc[1] = 101333;
		move_loc[2] = 101334;
		move_loc[3] = 100260;
		move_loc[4] = 100286;
		move_loc[5] = 100287;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		
		if(type == M05_STAR_INN)
		{
			Commands->Start_Timer(obj, this, 2.0f, GO_WINDOW);
			
		}
		if(type == M05_MOVE_DEADEYE)
		{
			current_move_loc = param;

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), DEADEYE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(move_loc[current_move_loc]), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
			
		}
		if(action_id == 300004 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Set_Innate_Is_Stationary(obj, false);
			// Rescued Deadeye objective
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 502, 1, 0.0f);
			// Deadeye_Freed
			Commands->Send_Custom_Event (obj, Commands->Find_Object(112559), M05_DEADEYE_FREED, 1, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100049), M05_DEADEYE_FREED, 1, 0.0f);
			// Vaporize Fancy Inn objective
	//		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 513, 3, 2.0f);
			// Inn can now be nuked
			objective_513 = true;

			// Goto Cathedral
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), DEADEYE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(100287), 0.6f, 3.0f);
			Commands->Action_Goto (obj, params);
			
		}
		if(action_id == DEADEYE_MOVE_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		}
		if(action_id == DEADEYE_MOVE_LOC && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 4.0f, MOVE_LOC_LOW_PRIORITY);
		}

	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_WINDOW)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
			params.WaypathID = 100040;
			params.Set_Attack (Commands->Find_Object(100044), 250.0f, 0.0f, 1);
			params.AttackWanderAllowed = false ;
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer(obj, this, 3.0f, DESTROY_WINDOW);

		}
		if(timer_id == MOVE_LOC_LOW_PRIORITY)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), DEADEYE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(100287), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}
		if(timer_id == DESTROY_WINDOW)
		{
			Commands->Action_Reset ( obj, INNATE_PRIORITY_ENEMY_SEEN + 5 );
			immortal = false;
		}
		
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		if(immortal)
		{
			if(Commands->Get_Health(obj) < 10.0f)
			{
				Commands->Set_Health(obj, 10.0f);
			}
			if(Commands->Get_Health(obj) < (.3 * Commands->Get_Max_Health(obj)))
			{
				Commands->Apply_Damage( obj, -10000.0f, "STEEL", NULL );
			}
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0178I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 502, 2, 1.0f);
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch(poke_id)
			{
			case 1:
				{
					// Nod is about to roll over this place, time to move out Deadeye.\n
					const char *conv_name = ("M05_CON013");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300004);
					Commands->Monitor_Conversation (obj, conv_id);
					Commands->Enable_HUD_Pokable_Indicator( obj, false );

					poke_id = 3;
		
				}
			break;
			case 2:
				{
					// Alright, let's keep moving.
					
				}
			break;
			case 3:
				{
					// Hold here.  I'll be back.
					
				}
			break;
			}
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		ActionParamsStruct params;
		if ( sound.Type == SOUND_TYPE_DESIGNER07 )
		{
			if(objective_513)
			{
				GameObject * inn = Commands->Find_Object(100244);
				float nuke_radius = Commands->Get_Distance(sound.Position, Commands->Get_Position( inn ));
				if(nuke_radius < 30.0f)
				{
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 513, 1, 0.0f);
				}
			}
		}
	}
};

DECLARE_SCRIPT(M05_DEAD6_Grenadier, "")
{
	int poke_id;
	bool conversation;

	enum{GO_STAR, ATTACK_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( conversation, 2 );
	}

	void Created (GameObject * obj) override
	{
		poke_id = 1;
		conversation = false;
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
	}

	

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0092I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 504, 2, 0.0f);
		Commands->Mission_Complete ( false );
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch (poke_id)
			{
			case 1: 
				{
					// Havoc! Black Hand squads are arriving from the Chateau! Support fire needed!\n
					const char *conv_name = ("M05_CON014");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300004);
					Commands->Monitor_Conversation (obj, conv_id);
					
					poke_id = 2;
					// Send custom to cathedral controller to initiate attack
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_INITIATE_CATHEDRAL, 1, 0.0f);
					// Change soundtrack to attack
					Commands->Set_Background_Music ("In the Line of Fire.mp3");
					Commands->Enable_HUD_Pokable_Indicator( obj, false );
				}
				break;
			}
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == 300004 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 504, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 506, 3, 3.0f);

			// Activate DEAD6 to protect Havoc
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100047), M05_PROTECT_HAVOC, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100006), M05_PROTECT_HAVOC, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100048), M05_PROTECT_HAVOC, 1, 0.0f);
		}

		if(action_id == GO_STAR && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), ATTACK_STAR);
			params.Set_Movement (STAR, RUN, 3.0f);
			Commands->Action_Goto (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_PROTECT_HAVOC)
		{
			Commands->Start_Timer (obj, this, 5.0f, GO_STAR);
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 40.0f);
		}

	}

	
};


DECLARE_SCRIPT(M05_Mendoza, "")
{
	
	bool exiting;
	float health;
	
	enum {PLACE_C4, BYE_MENDOZA};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( exiting, 1 );
		SAVE_VARIABLE( health, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Start_Timer (obj, this, 3.0f, PLACE_C4);
		exiting = false;
		health = Commands->Get_Max_Health(obj);

	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == PLACE_C4)
		{
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15);
			params.Set_Animation("S_A_HUMAN.H_A_J12C", false);
			Commands->Action_Play_Animation(obj, params);
		}
		if(timer_id == BYE_MENDOZA)
		{
			Commands->Destroy_Object(obj);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 15 && reason == ACTION_COMPLETE_NORMAL)
		{
			// You hear me in there GDI? Comfy? This is gonna be your tomb, get used to it!\n
			const char *conv_name = ("M05_CON015");
			int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Join_Conversation(Commands->Find_Object(100652), conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 16);
			Commands->Monitor_Conversation (obj, conv_id);			

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100010), M05_BLOW_BUILDING, 1, 0.0f);
		}

		if(action_id == 16 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Start_Timer (obj, this, 6.0f, BYE_MENDOZA);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
			params.WaypathID = 100011;
			Commands->Action_Goto( obj, params );

			
		}
		
		
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(health > (.5 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.25f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
		if((Commands->Get_Health(obj)) < (Commands->Get_Max_Health(obj) * .23))
		{
			Commands->Set_Health( obj, (Commands->Get_Max_Health(obj) * .23) );
			
			if(!exiting)
			{
				// Is that all you got?
				const char *conv_name = ("M05_CON016");
				int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 1);
				Commands->Monitor_Conversation (obj, conv_id);		

				exiting = true;
			}
		}
	}
	

};

DECLARE_SCRIPT(M05_Building_Collapse, "")
{
		
	bool already_entered;
	int m06_roof_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		GameObject * m06_roof = Commands->Create_Object("M05_Roof", Vector3(0,0,0));
		m06_roof_id = Commands->Get_ID(m06_roof);
	}

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
			
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * sender) override
	{
		
		ActionParamsStruct params;
		Commands->Debug_Message("Custom received from object %d.  Type and param of %d and %d.\n", Commands->Get_ID(sender), type, param);

		if(type == 50 && param == 50)
		{
		//	GameObject * debris = Commands->Create_Object("M05_X5A_AG_DLoiter", Vector3(0,0,0));
			

			GameObject * debris = Commands->Create_Object("Invisible_Object", Vector3(0.0f, 0.0f, 0.0f) );
			Commands->Set_Model ( debris, "x5a_debris" );
			Commands->Attach_Script(debris, "M05_Building_Debris", "");
		}
		if(type == M05_BLOW_BUILDING)
		{
			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
			Commands->Attach_Script(controller, "Test_Cinematic", "X5A_BCollapse.txt");
			Commands->Destroy_Object(Commands->Find_Object(m06_roof_id));
		}

		
	}

};

DECLARE_SCRIPT(M05_Triangle_Tank, "")
{

	int fire_loc[3];	
	bool attacking;

	enum {TANK_TIMER, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		attacking = false;

		ActionParamsStruct params;

		fire_loc[0] = 100020;
		fire_loc[1] = 100021;
		fire_loc[2] = 100022;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
		params.Set_Movement( Vector3(0,0,0), WALK, 2.0f );		
		params.WaypathID = 100015;
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );

		// Fire upon building
		Commands->Start_Timer (obj, this, 6.0f, TANK_TIMER);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TANK_TIMER)
		{
			ActionParamsStruct params;
			attacking = false;
			int random = Commands->Get_Random_Int(0,ARRAY_ELEMENT_COUNT(fire_loc)); 
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			// Fire upon building
			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}

		ActionParamsStruct params;
		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (damager, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);

		}
	}

};

DECLARE_SCRIPT(M05_TownSquare_Tank, "")
{

	int fire_loc[3];

	enum {TANK_TIMER, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( fire_loc, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		fire_loc[0] = 100024;
		fire_loc[1] = 100025;
		fire_loc[2] = 100026;

		// Fire upon building
		Commands->Start_Timer (obj, this, 3.0f, TANK_TIMER);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TANK_TIMER)
		{
			ActionParamsStruct params;
			
			int random = Commands->Get_Random_Int(0, ARRAY_ELEMENT_COUNT(fire_loc));
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
		params.Set_Attack (damager, 200.0f, 5.0f, 1);
		params.AttackCheckBlocked = true;
		Commands->Action_Attack (obj, params);

		Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100112), M05_TOWNSQUARE_REINFORCE, 3, 0.0f);
	}

};

DECLARE_SCRIPT(M05_Bridge_Tank, "")
{
	bool attacking;
	int fire_loc[3];

	enum {TANK_TIMER, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( fire_loc, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		fire_loc[0] = 100029;
		fire_loc[1] = 100030;
		fire_loc[2] = 100031;

		// Fire upon building
		Commands->Start_Timer (obj, this, 3.0f, TANK_TIMER);
		attacking = false;
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TANK_TIMER)
		{
			ActionParamsStruct params;
			attacking = false;
			int random = Commands->Get_Random_Int(0, ARRAY_ELEMENT_COUNT(fire_loc));
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}

		ActionParamsStruct params;
		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (damager, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

};

DECLARE_SCRIPT(M05_Enable_Overlook, "")
{
	
	bool already_entered;
	
	enum {DROP_SNIPER};

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

			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(-298.246f, -48.874f, 14.633f));
			Commands->Set_Facing(chinook_obj, 90.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X5I_TroopDrop1.txt");

			// Activate Bridge Gun emplacement
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100127), M05_CUSTOM_ACTIVATE, 1, 0.0f);

			Commands->Start_Timer (obj, this, 2.0f, DROP_SNIPER);
			
		}
	}

	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		// Drop sniper on balcony
		if(timer_id == DROP_SNIPER)
		{
			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(-335.883f, -82.737f, 24.550f));
			Commands->Set_Facing(chinook_obj, 80.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X5I_TroopDrop2.txt");
		}
		
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}
		
	}
};

DECLARE_SCRIPT(M05_Cache_Escort, "")
{
	int poke_id;
	bool arrived;
	bool hold;
	bool conversation;

	enum {GO_CHINOOK};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( arrived, 2 );
		SAVE_VARIABLE( hold, 3 );
		SAVE_VARIABLE( conversation, 4 );
	}

	void Created (GameObject * obj) override
	{
		poke_id = 1;
		arrived = false;
		hold = false;
		conversation = false;
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100162), M05_CACHE_CIV_KILLED, 1, 0.0f);
			
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if (Commands->Is_A_Star(poker) && !conversation)
		{
			conversation = true;

			switch(poke_id)
			{
			case 1:
				{
					// Send custom to the remainder of the civ resist to turn off poke, and have them follow
					Commands->Send_Custom_Event (obj, Commands->Find_Object(100033), 100, 100, 0.0f);
					Commands->Send_Custom_Event (obj, Commands->Find_Object(100034), 100, 100, 0.0f);

					// Paratroop in 3 Chemwarriors to hunt escort group
					GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-529.402f, 23.791f, 23.472f));
					Commands->Set_Facing(chinook_obj1, -95.000f);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_C130TroopDrop2.txt");
				
					// GDI, you've come to help? There's a crashed chopper with supplies all over nearby. Come with us, we'll show you.\n
					const char *conv_name = ("M05_CON017");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300508);
					Commands->Monitor_Conversation (obj, conv_id);
					
					// start objective 508
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 508, 3, 0.0f);

					

					poke_id = 3;
				
				}
			break;
			case 2:
				{
					// \\Let's keep moving.\n
					const char *conv_name = ("M05_CON018");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);

					params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_CHINOOK);
					params.Set_Movement (Commands->Find_Object(104374), RUN, 3.0f);
					Commands->Action_Goto (obj, params);

					Commands->Set_Innate_Is_Stationary(obj, false);
					hold = false;
					poke_id = 3;
				}
			break;
			case 3:
				{
					// \\Hold here. I'll be back.\n
					const char *conv_name = ("M05_CON019");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);
					
					Vector3 my_loc = Commands->Get_Position(obj);
					params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 1);
					params.Set_Movement (my_loc, RUN, 3.0f);
					Commands->Action_Goto (obj, params);

					Commands->Set_Innate_Is_Stationary(obj, true);

					hold = true;
					poke_id = 2;
				}
			break;
			case 4:
				{
					// We shall never surrender!\n
					const char *conv_name = ("M05_CON021");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);


				}
			break;
			}
			
			
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			Commands->Enable_HUD_Pokable_Indicator( obj, false );
			poke_id = 3;

			// Enable Mendoza above cache
			Commands->Enable_Spawner(100076, true);

			Commands->Action_Reset ( obj, INNATE_PRIORITY_ENEMY_SEEN - 5 );
			Commands->Set_Innate_Is_Stationary(obj, false);

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_CHINOOK);
			params.Set_Movement (Commands->Find_Object(104374), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
			
			
		}
		if(type == 100 && param == 101)
		{
			arrived = true;

			Commands->Action_Reset(obj, true);
	//		Commands->Set_Innate_Is_Stationary(obj, true);

			if(obj == Commands->Find_Object(100033))
			{
				if(!Commands->Find_Object(100034))
				{
					// Thank you GDI, these weapons will help.\n
					const char *conv_name = ("M05_CON020");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);
				}
			}
			else
			{
				// Thank you GDI, these weapons will help.\n
				const char *conv_name = ("M05_CON020");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Join_Conversation(STAR, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 1);
				Commands->Monitor_Conversation (obj, conv_id);
			}
			
			// Establish homepoints around cache
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);

			poke_id = 4;

			
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}
		if(action_id == GO_CHINOOK && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		}
		if((action_id == GO_CHINOOK) && (action_id != Commands->Get_Action_ID(obj)) && (!Commands->Is_Performing_Pathfind_Action(obj)) && (reason == ACTION_COMPLETE_LOW_PRIORITY))
		{
			Commands->Start_Timer(obj, this, 4.0f, GO_CHINOOK);
		}
	}

	

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_CHINOOK)
		{
			if(!hold)
			{
				params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_CHINOOK);
				params.Set_Movement (Commands->Find_Object(104374), RUN, 3.0f);
				Commands->Action_Goto (obj, params);
			}
			
		}
	}
};

DECLARE_SCRIPT(M05_Cache_Assault, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		if(Commands->Find_Object(100133))
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 10);
			params.Set_Movement (Commands->Find_Object(100133), RUN, 3.0f);
			params.MoveFollow = true;
			Commands->Action_Goto (obj, params);
		}
		else if(Commands->Find_Object(100134))
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 10);
			params.Set_Movement (Commands->Find_Object(100134), RUN, 3.0f);
			params.MoveFollow = true;
			Commands->Action_Goto (obj, params);
		}
	}

	

};

DECLARE_SCRIPT(M05_Chinook_Supply_Cache, "")
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
		if (((enterer == Commands->Find_Object(100033)) || (enterer == Commands->Find_Object(100034))) && (!already_entered))
		{
			already_entered = true;
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 508, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100162), M05_CIV_ARRIVED, 1, 0.0f);

			// Send custom to the two civ resist escort to inform them of arrival
			if(Commands->Find_Object(100033))
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100033), 100, 101, 0.0f);
			}
			if(Commands->Find_Object(100034))
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100034), 100, 101, 0.0f);
			}
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == 100 && param == 100)
		{
			already_entered = true;
		}

	}

	
};

DECLARE_SCRIPT(M05_Chateau_Escapee, "")
{
	enum{DESTROY_SELF};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Innate_Disable(obj);
		Commands->Set_Animation ( obj, "S_A_HUMAN.H_A_601A", true, NULL, 0.0f, -1.0f, false );
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if (type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Set_Animation ( obj, "S_A_HUMAN.H_A_7002", true, NULL, 0.0f, -1.0f, false );

			Vector3 myPosition = Commands->Get_Position ( obj );
			Commands->Create_Object ( "Visceroid", myPosition );

			Commands->Start_Timer (obj, this, 0.5f, DESTROY_SELF);	
		}

	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == DESTROY_SELF)
		{
			Commands->Apply_Damage( obj, 10000, "STEEL", NULL );
		}

	}

};

DECLARE_SCRIPT(M05_Escapee_Brother, "")
{
	int poke_id;
	bool conversation;
	bool complete;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( conversation, 2 );
		SAVE_VARIABLE( complete, 3 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2.0f);
		Commands->Set_Innate_Is_Stationary(obj, true);
		poke_id = 1;
		conversation = false;
		complete = false;
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
		Commands->Give_PowerUp(obj, "POW_Shotgun_AI", false);

	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch (poke_id)
			{
			case 1: 
				{
					// Got one back from the dead huh? What you got for me?\n
					const char *conv_name = ("M05_CON022");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300509);
					Commands->Monitor_Conversation (obj, conv_id);

					poke_id = 2;
					Commands->Enable_HUD_Pokable_Indicator( obj, false );
				}
				break;
			}
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == 300509 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			
			Commands->Select_Weapon(obj, "Weapon_Shotgun_Ai" );

			complete = true;
			Commands->Set_Innate_Is_Stationary(obj, false);

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * 1.5;
			float b = sin(DEG_TO_RADF(facing)) * 1.5;
			Vector3 powerup_loc = pos + Vector3(a, b, 0.5f);
			Commands->Create_Object("POW_PersonalIonCannon_Player", powerup_loc);

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 509, 1, 0.0f);

			GameObject * window_shooter = Commands->Create_Object ( "Nod_Gun_Emplacement_Dec", Vector3(-346.865f, 40.418f, 22.279f));
			Commands->Set_Facing(window_shooter, -145.000f);
			Commands->Attach_Script(window_shooter, "M05_Escapee_Windows", "");

			// Spawn Escapee Invaders
			Commands->Enable_Spawner(100749, true);
			Commands->Enable_Spawner(100750, true);
			Commands->Enable_Spawner(100751, true);

			// Turn escapee into Visceroid
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100037), M05_CUSTOM_ACTIVATE, 1, 0.0f);
			
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(!complete)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 509, 2, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M05_Escapee_Invaders, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 10);
		params.Set_Movement (STAR, RUN, 3.0f);
		Commands->Action_Goto (obj, params);
	}
	
};

DECLARE_SCRIPT(M05_Escapee_Windows, "")
{
	
	enum{DESTROY_WINDOWS, DESTROY_SELF};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Set_Is_Rendered(obj, false);

		GameObject * window1 = Commands->Find_Object(100754);

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, DESTROY_WINDOWS );
		params.Set_Attack (window1, 250.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
		Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(window1), NULL);

		Commands->Start_Timer (obj, this, 1.0f, DESTROY_WINDOWS);
		Commands->Start_Timer (obj, this, 3.0f, DESTROY_SELF);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == DESTROY_WINDOWS)
		{
			GameObject * window2 = Commands->Find_Object(100755);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, DESTROY_WINDOWS );
			params.Set_Attack (window2, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Modify_Action (obj, DESTROY_WINDOWS, params, true, true);

			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(window2), NULL);
		}

		if(timer_id == DESTROY_SELF)
		{
			Commands->Destroy_Object(obj);
		}
		
	}
	
};

DECLARE_SCRIPT(M05_Activate_Deadeye, "")
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

			// Send custom to Deadeye to activate him
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100004), M05_STAR_INN, 1, 0.0f);
			
		}
	}

};

DECLARE_SCRIPT(M05_Heal_Dead6, "")
{
		
	enum{HEAL_DEAD6};

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
			Commands->Start_Timer (obj, this, 0.0f, HEAL_DEAD6);
			
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == HEAL_DEAD6)
		{
			// Gunner
			Commands->Apply_Damage(Commands->Find_Object(100048), -10000.0f, "STEEL", NULL );
			// Deadeye
			Commands->Apply_Damage(Commands->Find_Object(100004), -10000.0f, "STEEL", NULL );
			// Hotwire
			Commands->Apply_Damage(Commands->Find_Object(100047), -10000.0f, "STEEL", NULL );
			// Patch
			Commands->Apply_Damage(Commands->Find_Object(100006), -10000.0f, "STEEL", NULL );

			Commands->Start_Timer (obj, this, 12.0f, HEAL_DEAD6);
		}
	}

};

DECLARE_SCRIPT(M05_Activate_Babushka_Encounter, "")
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
			// Custom sent to Nod_Minigunner_2SF holding Babushka to start interrogation
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100052), M05_CUSTOM_ACTIVATE, 1, 0.0f);
		}
	}

};

DECLARE_SCRIPT(M05_Babushka, "")
{
	int poke_id;
	bool conversation;
	bool saved;
	
	enum{ARM_SHOTGUN};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( conversation, 2 );
		SAVE_VARIABLE( saved, 3 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		conversation = false;
		saved = false;
		poke_id = 1;
		Commands->Enable_HUD_Pokable_Indicator( obj, true );

		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );
		Commands->Set_Innate_Is_Stationary(obj, true);

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1000);
		params.Set_Animation( "H_A_Host_L1b", true );
		Commands->Action_Play_Animation (obj, params);
	}

	
	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ARM_SHOTGUN)
		{
	//		Commands->Select_Weapon(obj, "Weapon_Chaingun_AI" );
		}
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if (Commands->Is_A_Star(poker) && (!conversation))
		{
			conversation = true;

			switch (poke_id)
			{
			case 1: 
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1000);
					params.Set_Animation( "H_A_Host_L1c", false );
					Commands->Action_Play_Animation (obj, params);

					Commands->Action_Reset(obj, 100);

			//		Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
					Commands->Start_Timer (obj, this, 1.0f, ARM_SHOTGUN);
					Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
					// You have my thanks, brave warrior. Our village is in debt to you.\n
					const char *conv_name = ("M05_CON023");
					int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN - 5, 200.0f, false);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300509);
					Commands->Monitor_Conversation (obj, conv_id);

					poke_id = 2;
					Commands->Enable_HUD_Pokable_Indicator( obj, false );
				}
				break;
			}
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == 300509 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			saved = true;
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 510, 1, 0.0f);

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 5);
			params.Set_Movement (Vector3 (-75.498f, 191.288f, 30.246f), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}
		
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		if(Commands->Get_Health(obj) < 10.0f)
		{
		//	Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
			Commands->Start_Timer (obj, this, 1.0f, ARM_SHOTGUN);
		//	Commands->Set_Health(obj, 10.0f);
		}
	/*	if(Commands->Get_Health(obj) < (.3 * Commands->Get_Max_Health(obj)))
		{
			Commands->Apply_Damage( obj, -10000.0f, "STEEL", NULL );
		}*/

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(!saved)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 510, 2, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M05_Babushka_Guard, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
	}
	
	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if(Commands->Is_A_Star(enemy))
		{
			Commands->Set_Innate_Is_Stationary(obj, false);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_CUSTOM_ACTIVATE)
		{
			if(Commands->Find_Object(100051) && Commands->Find_Object(100050))
			{
				// We know that you are their leader.\n
				const char *conv_name = ("M05_CON024");
				int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Join_Conversation(Commands->Find_Object(100051), conv_id, false, true, true);
				Commands->Join_Conversation(Commands->Find_Object(100050), conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 300000);
				Commands->Monitor_Conversation (obj, conv_id);
			}
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		
		if(action_id == 300000)
		{
			// Send custom to Babushka to make her playertype GDI
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100050), M05_CUSTOM_ACTIVATE, 1, 0.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100050), M05_CUSTOM_ACTIVATE, 1, 0.0f);
	}

};

DECLARE_SCRIPT (M05_Park_Activate, "")
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

			// Attention GDI forces, this is Resistance Radio. Nod is completing construction of an Obelisk in the Park, on the northwestern side. If anyone is near the area, eliminate all Nod Engineers to stop construction.\n
			const char *conv_name = ("M05_CON037");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300512);
			Commands->Monitor_Conversation (obj, conv_id);

		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		
		if(action_id == 300512 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 512, 3, 0.0f);
		}
		
	}

		
};

DECLARE_SCRIPT (M05_Park_Controller, "")
{
	int m05_park_engineer_killed;
	int m05_park_unit_killed;
	int m05_park_vehicle_killed;
	int artillery_sound1;
	int artillery_sound2;
	int artillery_sound3;
	Vector3 artillery_loc1;
	Vector3 artillery_loc2;
	Vector3 artillery_loc3;
	int loop_count;
	int loop_amnt;

	enum{START_ARTILLERY, PARK_ROCKET_SOLDIERS};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m05_park_engineer_killed, 1 );
		SAVE_VARIABLE( m05_park_unit_killed, 2 );
		SAVE_VARIABLE( m05_park_vehicle_killed, 3 );
		SAVE_VARIABLE( artillery_sound1, 4 );
		SAVE_VARIABLE( artillery_sound2, 5 );
		SAVE_VARIABLE( artillery_sound3, 6 );
		SAVE_VARIABLE( loop_count, 7 );
		SAVE_VARIABLE( loop_amnt, 8 );
	}

	void Created (GameObject * /* obj */) override
	{
		m05_park_engineer_killed = 0;
		m05_park_unit_killed = 0;
		m05_park_vehicle_killed = 0;

		artillery_sound1 = 0;
		artillery_sound2 = 0;
		artillery_sound3 = 0;

		loop_count = 0;
		loop_amnt = 0;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == M05_PARK_ENGINEER_KILLED)
		{
			m05_park_engineer_killed++;
			if (m05_park_engineer_killed == 1)
			{
				Commands->Enable_Spawner(100074, true);
			}
			if (m05_park_engineer_killed == 2)
			{
				Commands->Enable_Spawner(100075, true);
			}
			if (m05_park_engineer_killed == 6)
			{
				// Accomplished mission objective by killing all of the engineers
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 512, 1, 0.0f);
				// Grant armor increase medal
				Commands->Create_Object ( "POW_Medal_Armor", Vector3(-405.399f, 64.234f, 28.250f));
			}
		}
		if(type == M05_PARK_UNIT_KILLED)
		{
			m05_park_unit_killed++;
			if (m05_park_unit_killed%2 == 0 && m05_park_unit_killed < 8)
			{
				// Respawn Park Rocket Soldiers
				Commands->Start_Timer (obj, this, 10.0f, PARK_ROCKET_SOLDIERS);
				
			}
			else
			{
				// Start Artillery
				Commands->Start_Timer (obj, this, 1.0f, START_ARTILLERY);
			}
			
		}
		if(type == M05_PARK_VEHICLE_KILLED)
		{
			m05_park_vehicle_killed++;
			if (m05_park_vehicle_killed == 1)
			{
				// Spawn Apache
				Commands->Enable_Spawner(100721, true);
			}
			else
			{
				// Start Artillery
				Commands->Start_Timer (obj, this, 1.0f, START_ARTILLERY);
			}
			
		}
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound1)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc1, NULL);
		}
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound2)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc2, NULL);
		}
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound3)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc3, NULL);
		}
		
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if(timer_id == PARK_ROCKET_SOLDIERS)
		{
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-446.924f, 103.269f, 17.665f));
			Commands->Set_Facing(chinook_obj1, 10.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_C130TroopDrop7.txt");
		}
		if(timer_id == START_ARTILLERY && loop_count < loop_amnt)
		{
			loop_count++;

			int artillery_id1 = Get_Int_Parameter("Artillery_ID1");
			artillery_loc1 = Commands->Get_Position(Commands->Find_Object(artillery_id1));

			int artillery_id2 = Get_Int_Parameter("Artillery_ID2");
			artillery_loc2 = Commands->Get_Position(Commands->Find_Object(artillery_id2));

			int artillery_id3 = Get_Int_Parameter("Artillery_ID3");
			artillery_loc3 = Commands->Get_Position(Commands->Find_Object(artillery_id3));
			
			static constexpr const char *bomb[3] =
			{
				"Falling_Bomb_01",
				"Falling_Bomb_02",
				"Falling_Bomb_03",
			};

			int random = Commands->Get_Random_Int(0, 2);

			artillery_sound1 = Commands->Create_Sound ( bomb[random], artillery_loc1, obj );
			Commands->Monitor_Sound (obj, artillery_sound1);
			
			random = Commands->Get_Random_Int(0, 2);

			artillery_sound2 = Commands->Create_Sound ( bomb[random], artillery_loc2, obj );
			Commands->Monitor_Sound (obj, artillery_sound2);
			
			random = Commands->Get_Random_Int(0, 2);

			artillery_sound3 = Commands->Create_Sound ( bomb[random], artillery_loc3, obj );
			Commands->Monitor_Sound (obj, artillery_sound3);

			float time = Commands->Get_Random(0.5f, 3.0f);
			Commands->Start_Timer (obj, this, time, START_ARTILLERY);
			
		}
	}

};

DECLARE_SCRIPT(M05_Park_Apache, "")
{
	bool attacking;
	bool alerted;
	
	enum{WAYPATH, NEW_CLOSEST, ATTACK_OVER};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( alerted, 2 );
	}

	void Created(GameObject * obj) override
	{
		attacking = false;
		alerted = false;
		
		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100722;
		Commands->Action_Attack( obj, params );
		Commands->Enable_Enemy_Seen( obj, true);

		Commands->Innate_Force_State_Enemy_Seen(obj, STAR);
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 100722;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

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

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		ActionParamsStruct params;
		if ( sound.Type == M05_PARK_ALERT )
		{
			if(!alerted)
			{
				alerted = true;

				Commands->Innate_Force_State_Enemy_Seen(obj, STAR);

			}
		}
	}

};

DECLARE_SCRIPT(M05_ParkSniper, "")
{

	bool already_seen;
	bool alerted;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_seen, 1 );
		SAVE_VARIABLE( alerted, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		already_seen = false;
		alerted = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if(Commands->Is_A_Star(enemy))
		{
			Commands->Create_Logical_Sound(obj, M05_PARK_ALERT, Commands->Get_Position(obj), 5000.0f);
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		ActionParamsStruct params;
		if ( sound.Type == M05_PARK_ALERT )
		{
			if(!alerted)
			{
				alerted = true;

				Commands->Innate_Force_State_Enemy_Seen(obj, STAR);

			}
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /* amount */) override
	{
		if(Commands->Is_A_Star(damager))
		{
			Commands->Create_Logical_Sound(obj, M05_PARK_ALERT, Commands->Get_Position(obj), 5000.0f);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100632), M05_PARK_UNIT_KILLED, 1, 0.0f);
	}

	
};

DECLARE_SCRIPT(M05_Park_Buggy, "")
{
	bool attacking;
	bool alerted;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( alerted, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		alerted = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		ActionParamsStruct params;
		if ( sound.Type == M05_PARK_ALERT )
		{
			if(!alerted)
			{
				alerted = true;

				Commands->Innate_Force_State_Enemy_Seen(obj, STAR);

			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (damager, 250.0f, 0.0f, 1);
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

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100632), M05_PARK_VEHICLE_KILLED, 1, 0.0f);
	}
	
};

DECLARE_SCRIPT(M05_ParkEngineer, "")
{

	int truck_id[4];
	int truck_cnt;
	bool alerted;
	
	enum {ESCAPE_ROUTE, ESCAPED};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( truck_cnt, 1 );
		SAVE_VARIABLE( alerted, 2 );
	}

	void Created (GameObject * obj) override
	{
		alerted = false;

		ActionParamsStruct params;
		truck_id[0] = 100060;
		truck_id[1] = 100061;
		truck_id[2] = 100062;
		truck_id[3] = 100063;
	
		truck_cnt = 0;
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
		params.Set_Movement( Commands->Find_Object(truck_id[truck_cnt]), WALK, 1.5f );
		Commands->Action_Goto( obj, params );
	
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		
		// Technicians working on the obelisk
		if(action_id == 10 && reason == ACTION_COMPLETE_NORMAL)
		{
			Vector3 hedge_pos = Commands->Get_Position(Commands->Find_Object(truck_id[truck_cnt]));
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 11);
			params.Set_Face_Location( hedge_pos, 1.0f);
			Commands->Action_Face_Location ( obj, params );
		}
		if(action_id == 11 && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer(obj, this, 4.0f, 12);
			truck_cnt++;
			if(truck_cnt == 3)
			{
				truck_cnt = 0;
			}
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 12 );
			params.Set_Animation ("H_A_a0f0", false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == ESCAPE_ROUTE && reason == ACTION_COMPLETE_NORMAL)
		{
			int waypath_id[3];
			waypath_id[0] = 100681;
			waypath_id[1] = 100692;
			waypath_id[2] = 100703;

			int random = Commands->Get_Random_Int(0, 2);
				
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ESCAPED );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = waypath_id[random];
			Commands->Action_Goto( obj, params );	
		}

		if(action_id == ESCAPED && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Destroy_Object(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 512, 2, 0.0f);
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == 12)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD + 5, 10 );
			params.Set_Movement( Commands->Find_Object(truck_id[truck_cnt]), WALK, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		ActionParamsStruct params;
		if ( sound.Type == M05_PARK_ALERT )
		{
			if(!alerted)
			{
				int engineer_conv = Commands->Get_Random_Int(0, 1);
				if(engineer_conv == 0)
				{
					// Let's get out of here!\n
					const char *conv_name = ("M05_CON025");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);
				}
				else
				{
					// Make a run for the chopper!\n
					const char *conv_name = ("M05_CON026");
					int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
					Commands->Join_Conversation(obj, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 1);
					Commands->Monitor_Conversation (obj, conv_id);
				}
	
				alerted = true;

				int waypath_start_loc[3];
				waypath_start_loc[0] = 101300;
				waypath_start_loc[1] = 101301;
				waypath_start_loc[2] = 101302;

				int random = Commands->Get_Random_Int(0, 2);
				
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ESCAPE_ROUTE );
				params.Set_Movement( Commands->Find_Object(waypath_start_loc[random]), RUN, 1.5f );
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if(Commands->Is_A_Star(enemy))
		{
			Commands->Create_Logical_Sound(obj, M05_PARK_ALERT, Commands->Get_Position(obj), 5000.0f);
		}
	}

	void Damaged (GameObject * obj, GameObject * damager, float /* amount */) override
	{
		if(Commands->Is_A_Star(damager))
		{
			Commands->Create_Logical_Sound(obj, M05_PARK_ALERT, Commands->Get_Position(obj), 5000.0f);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100632), M05_PARK_ENGINEER_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M05_FleeEngineer, "")
{

	bool already_seen;
	
	enum {ESCAPE_ROUTE, ESCAPED};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
		SAVE_VARIABLE( already_seen, 1 );
	}

	void Created (GameObject * obj) override
	{
		int engineer_conv = Commands->Get_Random_Int(0, 1);
		if(engineer_conv == 0)
		{
			// Let's get out of here!\n
			const char *conv_name = ("M05_CON025");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);
			Commands->Monitor_Conversation (obj, conv_id);
		}
		else
		{
			// Make a run for the chopper!\n
			const char *conv_name = ("M05_CON026");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);
			Commands->Monitor_Conversation (obj, conv_id);
		}

		ActionParamsStruct params;
		
		int waypath_start_loc[3];
		waypath_start_loc[0] = 101300;
		waypath_start_loc[1] = 101301;
		waypath_start_loc[2] = 101302;

		int random = Commands->Get_Random_Int(0, 2);
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ESCAPE_ROUTE );
		params.Set_Movement( Commands->Find_Object(waypath_start_loc[random]), RUN, 1.5f );
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
				
		if(action_id == ESCAPE_ROUTE && reason == ACTION_COMPLETE_NORMAL)
		{
			ActionParamsStruct params;

			int waypath_id[3];
			waypath_id[0] = 100681;
			waypath_id[1] = 100692;
			waypath_id[2] = 100703;

			int random = Commands->Get_Random_Int(0, 2);
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ESCAPED );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = waypath_id[random];
			Commands->Action_Goto( obj, params );	
		}
		if(action_id == ESCAPED && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Destroy_Object(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 512, 2, 0.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100632), M05_PARK_ENGINEER_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Mendoza3, "")
{

	bool exiting;
	bool saw_enemy;
	float health;
	
	enum {BYE_MENDOZA3};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( exiting, 1 );
		SAVE_VARIABLE( saw_enemy, 2 );
		SAVE_VARIABLE( health, 3 );
		
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		saw_enemy = false;
		exiting = false;
		health = Commands->Get_Max_Health(obj);

		Commands->Give_PowerUp(obj, "POW_LaserRifle_AI", false);
		Commands->Select_Weapon(obj, "Weapon_LaserRifle_AI" );
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == BYE_MENDOZA3)
		{
			Commands->Destroy_Object(obj);
		}
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
		if((Commands->Get_Health(obj)) < (Commands->Get_Max_Health(obj) * .13))
		{
			Commands->Set_Health( obj, (Commands->Get_Max_Health(obj) * .13) );
			
			if(!exiting)
			{
				// All right, I'm bored. Enough of this.\n
				const char *conv_name = ("M05_CON027");
				int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 1);
				Commands->Monitor_Conversation (obj, conv_id);		

				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
				params.WaypathID = 100077;
				Commands->Action_Goto( obj, params );
				
				Commands->Start_Timer (obj, this, 3.0f, BYE_MENDOZA3);
				exiting = true;
			}
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject * /* enemy */ ) override
	{
		if(!saw_enemy)
		{
			// Like sheep to the slaughter!\n
			const char *conv_name = ("M05_CON028");
			int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1);
			Commands->Monitor_Conversation (obj, conv_id);	
			saw_enemy = true;
		}
		
	}
	

};

DECLARE_SCRIPT(M05_Activate_Nuke_Encounter, "")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			// Entrapement Technician
			Commands->Enable_Spawner(100083, true);

			// Sniper Alley Snipers
		/*	Commands->Enable_Spawner(100218, true);
			Commands->Enable_Spawner(100219, true);
			Commands->Enable_Spawner(100220, true);
			Commands->Enable_Spawner(100221, true);
			Commands->Enable_Spawner(100222, true);
			Commands->Enable_Spawner(100223, true);
			Commands->Enable_Spawner(100224, true);
			Commands->Enable_Spawner(100225, true);*/
			
			// Alley Sprint
			Commands->Enable_Spawner(100227, true);
			Commands->Enable_Spawner(100228, true);
		}
	}

};

DECLARE_SCRIPT(M05_Entrapment_Technician, "")
{
	bool poked;
	bool called_mendoza;
	
	enum {HELP_ME, SOMEONE, MENDOZA_ITS_HIM};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );
		SAVE_VARIABLE( called_mendoza, 2 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
		params.Set_Animation("S_A_HUMAN.H_A_601A", true);
		Commands->Action_Play_Animation(obj, params);

		// Help me!  Oh please, help me!\n
		const char *conv_name = ("M05_CON029");
		int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, HELP_ME);
		Commands->Monitor_Conversation (obj, conv_id);	

		Commands->Start_Timer (obj, this, 5.0f, SOMEONE);

		poked = false;
		called_mendoza = false;
		Commands->Enable_HUD_Pokable_Indicator( obj, true );


	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if((timer_id == HELP_ME) && (!poked))
		{
			// Someone, anyone!\n
			const char *conv_name = ("M05_CON030");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, SOMEONE);
			Commands->Monitor_Conversation (obj, conv_id);

			Commands->Start_Timer (obj, this, 5.0f, SOMEONE);
		}
		if((timer_id == SOMEONE) && (!poked))
		{
			// Help me!  Oh please, help me!
			const char *conv_name = ("M05_CON029");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, HELP_ME);
			Commands->Monitor_Conversation (obj, conv_id);	

			Commands->Start_Timer (obj, this, 5.0f, HELP_ME);
		}
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if ((Commands->Is_A_Star(poker)) && (!poked))
		{
			poked = true;
			Commands->Enable_HUD_Pokable_Indicator( obj, false );

			// Mendoza, it's Havoc!\n
			const char *conv_name = ("M05_CON031");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, MENDOZA_ITS_HIM);
			Commands->Monitor_Conversation (obj, conv_id);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == MENDOZA_ITS_HIM && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Enable_Spawner(100084, true);

			// Transition from sleeping to neutral standing
			params.Set_Basic(this, 100, 10);
			params.Set_Animation("S_A_HUMAN.H_A_j13c", false);
			Commands->Action_Play_Animation(obj, params);

			called_mendoza = true;
		}
		
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		if(!called_mendoza)
		{
			Commands->Enable_Spawner(100084, true);
		}
	}

};

DECLARE_SCRIPT(M05_Mendoza4, "")
{

	bool exiting;
	bool saw_enemy;
	float health;
	
	enum {BYE_MENDOZA4, FLEE_NUKE, BOTH_TEAMS};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( exiting, 1 );
		SAVE_VARIABLE( saw_enemy, 2 );
		SAVE_VARIABLE( health, 3 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		saw_enemy = false;
		exiting = false;
		health = Commands->Get_Max_Health(obj);

		// Both teams drop! Now! Now! Now!\n
		const char *conv_name = ("M05_CON032");
		int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, BOTH_TEAMS);
		Commands->Monitor_Conversation (obj, conv_id);
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
		params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
		params.WaypathID = 100085;
		params.Set_Attack (STAR, 250.0f, 0.0f, 1);
		params.AttackWanderAllowed = false ;
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );

		Commands->Start_Timer (obj, this, 7.0f, FLEE_NUKE);
		// Send custom to nuke to start countdown
	//	Commands->Send_Custom_Event(obj, Commands->Find_Object(100096), 100, 100, 2.0f);

		Commands->Give_PowerUp(obj, "POW_LaserRifle_AI", false);
		Commands->Select_Weapon(obj, "Weapon_LaserRifle_AI" );
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if((timer_id == FLEE_NUKE) && (!exiting))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
			params.WaypathID = 100089;
			Commands->Action_Goto( obj, params );
				
			Commands->Start_Timer (obj, this, 5.0f, BYE_MENDOZA4);
			exiting = true;
		}
		if(timer_id == BYE_MENDOZA4)
		{
			Commands->Destroy_Object(obj);
		}
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
		if((Commands->Get_Health(obj)) < (Commands->Get_Max_Health(obj) * .13))
		{
			Commands->Set_Health( obj, (Commands->Get_Max_Health(obj) * .13) );
			
			if(!exiting)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
				params.WaypathID = 100089;
				Commands->Action_Goto( obj, params );
				
				Commands->Start_Timer (obj, this, 5.0f, BYE_MENDOZA4);
				exiting = true;
			}
		}
	}

	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == BOTH_TEAMS && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-230.672f, -23.591f, 12.130f));
			Commands->Set_Facing(chinook_obj1, -165.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_C130TroopDrop8.txt");
		}
		
	}

};



DECLARE_SCRIPT(M05_Civ_Warn, "")
{

	enum {GO_CACHE, GO_STAR};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 5.5f, GO_STAR);
		Commands->Enable_Hibernation( obj, false );
		Commands->Give_PowerUp(obj, "POW_Shotgun_AI", false);
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 100097)
		{
			// Don't shoot, I am a friend! There are enemies shooting from the balcony. Can you help?\n
			const char *conv_name = ("M05_CON002");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1000971);
			Commands->Monitor_Conversation (obj, conv_id);	
		}
		if(action_id == 1000971 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Start_Timer (obj, this, 0.5f, GO_CACHE);
			
			Commands->Select_Weapon(obj, "Weapon_Shotgun_Ai" );
		}
		
	}
	
	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_STAR)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 100097);
			params.Set_Movement (STAR, RUN, 2.0f);
			Commands->Action_Goto (obj, params);
		}
		if(timer_id == GO_CACHE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 1);
			params.Set_Movement (Vector3 (-482.504f, -94.474f, 13.726f), RUN, 5.0f);
			Commands->Action_Goto (obj, params);

			Commands->Start_Timer (obj, this, 8.0f, GO_CACHE);
		}
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Enable_Spawner(100104, true);
	}

	
};

DECLARE_SCRIPT(M05_Civ_Lead, "")
{
	
	int poke_id;
	bool hold;
	bool conversation;
	bool first_poke;
	bool at_resistance;

	enum {GO_CIV_CACHE_TIMER, GO_CIV_CACHE};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{		
		SAVE_VARIABLE( poke_id, 1 );
		SAVE_VARIABLE( hold, 2 );
		SAVE_VARIABLE( conversation, 3 );
		SAVE_VARIABLE( first_poke, 4 );
		SAVE_VARIABLE( at_resistance, 5 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		at_resistance = false;
		poke_id = 1;
		hold = false;
		conversation = false;
		first_poke = true;
		Commands->Innate_Disable(obj);
		Commands->Enable_HUD_Pokable_Indicator( obj, true );		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Innate_Enable(obj);

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 100755);
			params.Set_Movement (STAR, RUN, 2.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

//		if(action_id == GO_CIV_CACHE && reason == ACTION_COMPLETE_NORMAL)
//		{
//		}

//		if(action_id == 100755)
//		{
			// GDI - I'll take you to the resistance. Follow quickly!\n
//			const char *conv_name = ("M05_CON033");
//			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
//			Commands->Join_Conversation(obj, conv_id, false, true, true);
//			Commands->Start_Conversation (conv_id, 1007551);
//			Commands->Monitor_Conversation (obj, conv_id);	
//		}

		if(action_id == 1007551)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_CIV_CACHE);
			params.Set_Movement (Vector3 (-513.603f, 8.030f, 28.080f), RUN, 5.0f);
			Commands->Action_Goto (obj, params);
			Commands->Start_Timer (obj, this, 6.0f, GO_CIV_CACHE_TIMER);
		}

		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}

		if(action_id == GO_CIV_CACHE )
		{
			// at resistance. no longer pokable
			at_resistance = true;
		}
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if(Commands->Is_A_Star(poker) && first_poke)
		{
			first_poke = false;

			// GDI - I'll take you to the resistance. Follow quickly!\n
			const char *conv_name = ("M05_CON033");
			int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1007551);
			Commands->Monitor_Conversation (obj, conv_id);
			Commands->Enable_HUD_Pokable_Indicator( obj, false );
		}

		else if ( !at_resistance )	// stop / go conversation.
		{
			if(Commands->Is_A_Star(poker) && !conversation)
			{
				conversation = true;
				ActionParamsStruct params;
				switch(poke_id)
				{
				case 1:
					{
						// \\Hold here. I'll be back.\n
						const char *conv_name = ("M05_CON019");
						int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
						Commands->Join_Conversation(STAR, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 1);
						Commands->Monitor_Conversation (obj, conv_id);
						
						Vector3 my_loc = Commands->Get_Position(obj);
						params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 6);
						params.Set_Movement (my_loc, RUN, 3.0f);
						Commands->Action_Goto (obj, params);

						Commands->Set_Innate_Is_Stationary(obj, true);


						hold = true;
						poke_id = 2;
					}
					break;
				case 2:
					{
						// \\Let's keep moving.\n
						const char *conv_name = ("M05_CON018");
						int conv_id = Commands->Create_Conversation(conv_name, 0, 0, true);
						Commands->Join_Conversation(STAR, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 1);
						Commands->Monitor_Conversation (obj, conv_id);

						Commands->Set_Innate_Is_Stationary(obj, false);
						hold = false;
						Commands->Start_Timer (obj, this, 1.0f, GO_CIV_CACHE_TIMER);
						
						poke_id = 1;
					}
					break;
				}
			}
		}
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_CIV_CACHE_TIMER)
		{
			if(!hold)
			{
				params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_CIV_CACHE);
				params.Set_Movement (Vector3 (-513.603f, 8.030f, 28.080f), RUN, 5.0f);
				Commands->Action_Goto (obj, params);
			}

			Commands->Start_Timer (obj, this, 6.0f, GO_CIV_CACHE_TIMER);
		}
	}
};


DECLARE_SCRIPT(M05_Triangle_Unit, "Unit_ID=0:int")
{
	int unit_id;
	
	enum{GO_TRIANGLE};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 3.0f, GO_TRIANGLE);

		unit_id = Get_Int_Parameter("Unit_ID");
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == CUSTOM_EVENT_FALLING_DAMAGE)
		{
			float health = Commands->Get_Health(obj);
			health = health + 1;
		}

	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		ActionParamsStruct params;

		float health = Commands->Get_Health(obj);
		health = health + 1;

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_TRIANGLE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer (obj, this, 6.0f, GO_TRIANGLE);
		}
		if(action_id == GO_TRIANGLE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer (obj, this, 20.0f, GO_TRIANGLE);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_TRIANGLE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_TRIANGLE);
			params.Set_Movement (Commands->Find_Object(100105), RUN, 5.0f);
			Commands->Action_Goto (obj, params);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100105), M05_TRIANGLE_REINFORCE, unit_id, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Triangle_Controller, "")  // 100105
{
	int reinforce_cnt;
	int unit_id1;
	int unit_id2;
	int unit_id3;
	int unit_id4;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( reinforce_cnt, 1 );
		SAVE_VARIABLE( unit_id1, 2 );
		SAVE_VARIABLE( unit_id2, 3 );
		SAVE_VARIABLE( unit_id3, 4 );
		SAVE_VARIABLE( unit_id4, 5 );
	}

	void Created (GameObject * /* obj */) override
	{
		reinforce_cnt = 3;
		unit_id1 = 0;
		unit_id2 = 0;
		unit_id3 = 0;
		unit_id4 = 0;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_TRIANGLE_REINFORCE)
		{
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1 < reinforce_cnt)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-521.328f, -68.988f, 17.960f));
						Commands->Set_Facing(controller, 105.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops01.txt");
					}
					else
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-521.328f, -68.988f, 17.960f));
						Commands->Set_Facing(controller, 105.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops11.txt");
					}
				}
				break;
			case 2:
				{
					unit_id2++;
					if(unit_id2 < reinforce_cnt)
					{
			//			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-485.633f, -94.664f, 18.238f));
			//			Commands->Set_Facing(controller, 175.000f);
			//			Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops02.txt");
					}
					else
					{
			//			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-485.633f, -94.664f, 18.238f));
			//			Commands->Set_Facing(controller, 175.000f);
			//			Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops21.txt");
					}
				}
				break;
			case 3:
				{
					unit_id3++;
					if(unit_id3 < reinforce_cnt)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-518.077f, -96.536f, 17.114f));
						Commands->Set_Facing(controller, -20.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops03.txt");
					}
					else
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-518.077f, -96.536f, 17.114f));
						Commands->Set_Facing(controller, -20.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops13.txt");
					}
				}
				break;
			case 4:
				{
					unit_id4++;
					if(unit_id4%2 == 0 &&  unit_id4 < reinforce_cnt * 2)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-493.674f, -63.951f, 14.002f));
						Commands->Set_Facing(controller, 10.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5D_CHTroopdrop9.txt");
					}
					
				}
				break;
			}
		}

	}
};

DECLARE_SCRIPT(M05_TownSquare_Sniper, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Set_Innate_Is_Stationary(obj, true);
			
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
		params.Set_Attack (Commands->Find_Object(100114), 250.0f, 0.0f, 1);
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Enable_Spawner(100115, true);
	}

};

DECLARE_SCRIPT(M05_TownSquare_Controller, "")  // 100112
{
	int unit_id1;
	int unit_id2;
	int unit_id3;
	int unit_id4;
	bool townsquare_retaken;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id1, 1 );
		SAVE_VARIABLE( unit_id2, 2 );
		SAVE_VARIABLE( unit_id3, 3 );
		SAVE_VARIABLE( unit_id4, 4 );
		SAVE_VARIABLE( townsquare_retaken, 5 );
	}

	void Created (GameObject * /* obj */) override
	{
		unit_id1 = 0;
		unit_id2 = 0;
		unit_id3 = 0;
		unit_id4 = 0;
		townsquare_retaken = false;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_TOWNSQUARE_REINFORCE)
		{
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if((unit_id1%2 == 0) && (unit_id1 < 5))
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-344.528f, -144.159f, 5.168f));
						Commands->Set_Facing(chinook_obj1, 90.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop1.txt");
					}
					else
					{
						Commands->Enable_Spawner(100117, true);
					}
				}
				break;
			case 2:
				{
					unit_id2++;
					if((unit_id2%2 == 0) && (unit_id2 < 5))
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-320.817f, -128.650f, 5.289f));
						Commands->Set_Facing(chinook_obj1, -90.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop2.txt");
					}
					else
					{
						Commands->Enable_Spawner(100118, true);
					}
				}
				break;
			case 3:
				{
					unit_id3++;	
				}
				break;
			case 4:
				{
					unit_id4++;
				}
				break;
			
			}
			// Check that all Nod soldiers, tank, cannon emplacement killed
			if((unit_id1 == 6) && (unit_id2 == 6) && (unit_id3 == 1) && (unit_id4 == 1) && (!townsquare_retaken))
			{
				Commands->Debug_Message("Resistance have taken town square, Gunner will now leave");
				// Resistance have taken town square
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 507, 1, 0.0f);
				// Gunner will now leave
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100003), 500, 500, 0.0f);
				townsquare_retaken = true;
			}
		}

	}
};

DECLARE_SCRIPT(M05_TownSquare_Unit, "Unit_ID=0:int")
{
	enum{GO_TOWNSQUARE};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 3.0f, GO_TOWNSQUARE);
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_TOWNSQUARE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer (obj, this, 6.0f, GO_TOWNSQUARE);
		}
		if(action_id == GO_TOWNSQUARE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 2.0f);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_TOWNSQUARE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_TOWNSQUARE );
			params.Set_Movement( Commands->Find_Object(100112), RUN, 5.0f );
			Commands->Action_Goto( obj, params );	
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100112), M05_TOWNSQUARE_REINFORCE, unit_id, 0.0f);
	}
};

DECLARE_SCRIPT(M05_TownSquare_FlameTank, "Unit_ID=0:int")
{
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100112), M05_TOWNSQUARE_REINFORCE, unit_id, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Nod_Gun_Emplacement, "")
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
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (damager, 250.0f, 0.0f, 1);
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

DECLARE_SCRIPT(M05_Nod_Bridge_Gun_Emplacement, "")
{
	bool attacking;

	enum{ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		
		attacking = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 12.0f, ATTACK_OVER);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Enable_Enemy_Seen( obj, true);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			Commands->Enable_Enemy_Seen( obj, true);
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (damager, 250.0f, 0.0f, 1);
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

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-292.298f, -82.194f, 10.478f));
		Commands->Set_Facing(controller1, -150.000f);
		Commands->Attach_Script(controller1, "Test_Cinematic", "X5D_CHTroopdrop3.txt");
	}
	
};

DECLARE_SCRIPT(M05_Overlook_Captives, "")
{
	
	enum{GO_POWERUPS, ARM_CHAINGUN};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );

		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_FREE_OVERLOOK_CAPTIVES)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Set_Innate_Is_Stationary(obj, false);

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), GO_POWERUPS);
			params.Set_Movement (Commands->Find_Object(100131), RUN, 3.0f);
			Commands->Action_Goto (obj, params);

		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		
		if(action_id == GO_POWERUPS && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
			Commands->Start_Timer(obj, this, 1.0f, ARM_CHAINGUN);
			
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ARM_CHAINGUN)
		{
			Commands->Select_Weapon(obj, "Weapon_Chaingun_AI" );

		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100136), M05_OVERLOOK_REINFORCE, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Free_Overlook_Captives, "")
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

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100129), M05_FREE_OVERLOOK_CAPTIVES, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100130), M05_FREE_OVERLOOK_CAPTIVES, 1, 0.0f);
		}
	}

};

DECLARE_SCRIPT(M05_Overlook_Controller, "")  // 100136
{
	int unit_id1;
	int unit_id2;
	int reinforce_cnt;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id1, 1 );
		SAVE_VARIABLE( unit_id2, 2 );
		SAVE_VARIABLE( reinforce_cnt, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		unit_id1 = 0;
		unit_id2 = 0;
		reinforce_cnt = 5;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_OVERLOOK_REINFORCE)
		{
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1 == 1)
					{
						Commands->Enable_Spawner(100142, true);
					}
					if(unit_id1 == 2)
					{
						Commands->Enable_Spawner(100143, true);
					}
					
				}
				break;
			case 2:
				{
					unit_id2++;
					if(unit_id2%2 == 0 && unit_id2 < reinforce_cnt)
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-312.794f, -48.780f, 14.414f));
						Commands->Set_Facing(chinook_obj1, 15.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_C130TroopDrop1.txt");
					}
					else
					{
						Commands->Enable_Spawner(100140, true);
						Commands->Enable_Spawner(100141, true);
					}
				}
				break;
			
			}
		}

	}
};

DECLARE_SCRIPT(M05_Overlook_Nod, "")  
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( unit_id1, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 1);
		params.Set_Movement (Commands->Find_Object(100136), RUN, 3.0f);
		Commands->Action_Goto (obj, params);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100136), M05_OVERLOOK_REINFORCE, 2, 0.0f);
	}
	
};

DECLARE_SCRIPT(M05_Overlook_Civ, "")  
{
	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 1);
		params.Set_Movement (Commands->Find_Object(100136), RUN, 3.0f);
		Commands->Action_Goto (obj, params);
	}
	
};

DECLARE_SCRIPT(M05_Nod_APC, "")  
{
	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_CUSTOM_ACTIVATE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100145;
			params.WaypathSplined = true;
			Commands->Action_Goto( obj, params );
		}

	}
};

DECLARE_SCRIPT(M05_Cache_Controller, "")  // 100162
{
	int reinforce_cnt;
	int unit_id1;
	int unit_id2;
	int unit_id3;
	bool civ_arrived;
	int civ_killed;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( reinforce_cnt, 1 );
		SAVE_VARIABLE( unit_id1, 2 );
		SAVE_VARIABLE( unit_id2, 3 );
		SAVE_VARIABLE( unit_id3, 4 );
		SAVE_VARIABLE( civ_arrived, 5 );
		SAVE_VARIABLE( civ_killed, 6 );
	}

	void Created (GameObject * /* obj */) override
	{
		reinforce_cnt = 3;
		unit_id1 = 0;
		unit_id2 = 0;
		unit_id3 = 0;
		civ_arrived = false;
		civ_killed = 0;

	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_CACHE_REINFORCE)
		{
			Vector3 star_pos = Commands->Get_Position(STAR);
			GameObject * drop_loc1 = Commands->Find_Object(100162);
			GameObject * drop_loc2 = Commands->Find_Object(100163);
			Vector3 drop_loc1_pos = Commands->Get_Position(drop_loc1);
			Vector3 drop_loc2_pos = Commands->Get_Position(drop_loc2);

			float dist_drop_loc1 = Commands->Get_Distance(star_pos, drop_loc1_pos);
			float dist_drop_loc2 = Commands->Get_Distance(star_pos, drop_loc2_pos);
			float least_dist = WWMath::Min(dist_drop_loc1, dist_drop_loc2);

			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1%2 == 0 && unit_id1 < 8)
					{
						if(dist_drop_loc1 == least_dist)
						{
							GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-495.497f, 40.504f, 22.714f));
							Commands->Set_Facing(controller, -5.000f);
							Commands->Attach_Script(controller, "Test_Cinematic", "X5D_CHTroopdrop4.txt");
						}
						else
						{
							GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-401.111f, 43.222f, 20.018f));
							Commands->Set_Facing(controller, -180.000f);
							Commands->Attach_Script(controller, "Test_Cinematic", "X5D_CHTroopdrop4.txt");
						}
					}
					else
					{
						
					}
				}
				break;
			
			}
		}

		if(type == M05_CACHE_CIV_KILLED)
		{
			if(!civ_arrived)
			{
				civ_killed++;
				if(civ_killed == 2)
				{
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 508, 2, 0.0f);
				}

			}
		}

		if(type == M05_CIV_ARRIVED)
		{
			civ_arrived = true;
		}

	}
};

DECLARE_SCRIPT(M05_Cache_Unit, "")
{
	enum{ATTACK_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Start_Timer(obj, this, 3.0f, ATTACK_STAR);

		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100162), M05_CACHE_REINFORCE, 1, 0.0f);
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_STAR)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 10);
			params.Set_Movement (STAR, RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}
	}

};

DECLARE_SCRIPT(M05_Free_Dump_Captives, "")
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

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100167), M05_FREE_DUMP_CAPTIVES, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100168), M05_FREE_DUMP_CAPTIVES, 1, 0.0f);

			// Shutdown Alley Sprint
			Commands->Enable_Spawner(100760, true);
			Commands->Enable_Spawner(100766, true);
		}
	}

};

DECLARE_SCRIPT(M05_Dump_Captives, "")
{
	
	enum{GO_POWERUPS, ARM_SHOTGUN};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );

		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_FREE_DUMP_CAPTIVES)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Set_Innate_Is_Stationary(obj, false);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_POWERUPS );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
			params.WaypathID = 100756;
			Commands->Action_Goto( obj, params );

		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		
		if(action_id == GO_POWERUPS && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);

			Commands->Start_Timer (obj, this, 1.0f, ARM_SHOTGUN);
			
		}
	}

	Vector3 Get_Circle_Position( GameObject *obj, GameObject *target , float radius , float reverseprob)
	{
		double theta_angle = (360/5);
		float reverse = Commands->Get_Random(0.0f,1.0f);
		if ( reverse < reverseprob)
			{
				theta_angle *= -1;
			}
		Vector3 position = Commands->Get_Position(obj);
		Vector3 targetlocation = Commands->Get_Position( target ) - position;
		targetlocation.Normalize();
		targetlocation *= radius;
		targetlocation.Rotate_Z(DEG_TO_RADF(theta_angle));
		targetlocation += position;
		return targetlocation;
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ARM_SHOTGUN)
		{
			Commands->Select_Weapon(obj, "Weapon_Chaingun_AI" );

			Vector3 curr_loc = Commands->Get_Position(obj);
			Commands->Debug_Message("Current Location: %f, %f, %f", curr_loc[0], curr_loc[1], curr_loc[2]);
			Vector3 go_loc = Get_Circle_Position(obj, obj, 5.0f, 0.0f);
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_POWERUPS );
			params.Set_Movement( go_loc, RUN, 0.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100173), M05_DUMP_REINFORCE, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Dump_Unit, "Unit_ID=0:int")
{
	int unit_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		unit_id = Get_Int_Parameter("Unit_ID");
		if(unit_id == 3)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100173), M05_DUMP_REINFORCE, unit_id, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Dump_Controller, "")  // 100173
{
	int reinforce_cnt;
	int unit_id1;
	int unit_id2;
	int unit_id3;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( reinforce_cnt, 1 );
		SAVE_VARIABLE( unit_id1, 2 );
		SAVE_VARIABLE( unit_id2, 3 );
		SAVE_VARIABLE( unit_id3, 4 );
	}

	void Created (GameObject * /* obj */) override
	{
		reinforce_cnt = 3;
		unit_id1 = 0;
		unit_id2 = 0;
		unit_id3 = 0;

	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_DUMP_REINFORCE)
		{
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1%2 == 0)
					{
						// Additional civilian resistance
						Commands->Enable_Spawner(100771, true);
						Commands->Enable_Spawner(100772, true);
					}
					
				}
				break;
			case 2:
				{
					unit_id2++;
					if(unit_id2%2 == 0 && unit_id2 < 7)
					{
						GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(-275.241f, 12.450f, 16.044f));
						Commands->Set_Facing(chinook_obj, -125.000f);
						Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X5I_TroopDrop3.txt");
					}
					else
					{
						
					}
				}
				break;
			case 3:
				{
					unit_id3++;
					if(unit_id3 < 3)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-283.920f, 21.640f, 20.110f));
						Commands->Set_Facing(controller, 75.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_WINTROOPS04.txt");
					}
					else
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-283.920f, 21.640f, 20.110f));
						Commands->Set_Facing(controller, 75.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_WINTROOPS05.txt");
					}
				}
				break;
			
			}
		}

	}
};

DECLARE_SCRIPT(M05_SniperAlley_Sniper, "Sniper_ID=0:int")
{
	int sniper_id;
	
	enum {SNIPER_LOC};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( sniper_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		sniper_id = Get_Int_Parameter("Sniper_ID");

		switch(sniper_id)
		{
		case 1:
			{
				

				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100184;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 2:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100189;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 3:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100193;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 4:
			{
				Commands->Set_Innate_Is_Stationary(obj, true);
			}
			break;
		case 5:
			{
				Commands->Set_Innate_Is_Stationary(obj, true);
			}
			break;
		case 6:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100201;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 7:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100205;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 8:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100214;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 9:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100761;
				Commands->Action_Goto( obj, params );
			}
			break;
		case 10:
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, SNIPER_LOC );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
				params.WaypathID = 100767;
				Commands->Action_Goto( obj, params );
			}
			break;
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == SNIPER_LOC)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
			Commands->Innate_Force_State_Enemy_Seen(obj, STAR);
		}
		
		
	}
};

DECLARE_SCRIPT(M05_Alley_Sprint, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( sniper_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
		params.WaypathID = 100235;
		Commands->Action_Goto( obj, params );
		
	}

	
};

DECLARE_SCRIPT(M05_Deactivate_Alley_Sprint, "")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			// Shutdown Alley Sprint
			Commands->Enable_Spawner(100227, false);
			Commands->Enable_Spawner(100228, false);
		
		}
	}

};

DECLARE_SCRIPT(M05_Inn_Tank, "")
{

	int fire_loc[3];
	bool attacking;

	enum {TANK_TIMER, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( fire_loc, 2 );
		SAVE_VARIABLE( attacking, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		fire_loc[0] = 100249;
		fire_loc[1] = 100250;
		fire_loc[2] = 100251;

		attacking = false;

		// Fire upon building
		Commands->Start_Timer (obj, this, 0.0f, TANK_TIMER);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TANK_TIMER)
		{
			ActionParamsStruct params;
			attacking = false;
			int random = Commands->Get_Random_Int(0,ARRAY_ELEMENT_COUNT(fire_loc)); 
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 250.0f, 10.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			// Fire upon building
			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			return;
		}
		ActionParamsStruct params;
		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (damager, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer (obj, this, 15.0f, TANK_TIMER);
		}
	}

};

DECLARE_SCRIPT(M05_Inn_Controller, "")  // 100244
{
	int unit_id1;
	int unit_id2;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id1, 1 );
		SAVE_VARIABLE( unit_id2, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		unit_id1 = 0;
		unit_id2 = 0;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M05_INN_REINFORCE)
		{
		
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1%2 == 0)
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-50.117f, -62.048f, 4.001f));
						Commands->Set_Facing(chinook_obj1, 95.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop5.txt");	
					}
					else
					{
						
					}
				}
				break;
			case 2:
				{
					unit_id2++;
					if(unit_id2%2 == 0)
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-98.226f, -75.228f, 4.042f));
						Commands->Set_Facing(chinook_obj1, -80.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop6.txt");	
					}
					else
					{
						
					}
				}
				break;
			
			}
		}

	}
};

DECLARE_SCRIPT (M05_Apc_Drop_DME, "")
{
	int path_sec;
	int daves_1, daves_2, daves_3;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( path_sec, 1 );
		SAVE_VARIABLE( daves_1, 2 );
		SAVE_VARIABLE( daves_2, 3 );
		SAVE_VARIABLE( daves_3, 4 );
	}

	void Created (GameObject */* obj */) override
	{
		path_sec = 0;
		daves_1 = 100258;
		daves_2 = 100259;
		daves_3 = 100260;

	}

	void Custom (GameObject* obj, int type, intptr_t param, GameObject* sender) override
	{
		if (sender == STAR && type == 100 && param == 100)
		{
			GameObject *point_1 = Commands->Find_Object (daves_1);
			GameObject *point_2 = Commands->Find_Object (daves_2);
			GameObject *point_3 = Commands->Find_Object (daves_3);
			Vector3 loc_1 = Commands->Get_Position (point_1);
			Vector3 loc_2 = Commands->Get_Position (point_2);
			Vector3 loc_3 = Commands->Get_Position (point_3);

			switch (path_sec)
			{
			case 0:
				{
					
					path_sec++;

					ActionParamsStruct params;

					params.Set_Basic( this, 99, 90 );
					params.Set_Movement( loc_1, 1.5f, 2.0f );		
					Commands->Action_Goto( obj, params );

										
				}
				break;

			case 1:
				{
					path_sec++;

					ActionParamsStruct params;

					params.Set_Basic( this, 99, 90 );
					params.Set_Movement( loc_2, 1.5f, 2.0f );		
					Commands->Action_Goto( obj, params );
				}
				break;

			case 2:
				{
					path_sec++;

					ActionParamsStruct params;

					params.Set_Basic( this, 99, 90 );
					params.Set_Movement( loc_3, 1.5f, 2.0f );		
					Commands->Action_Goto( obj, params );
				}
				break;

			}
		}


	}
};

DECLARE_SCRIPT (M05_Apc_Drop_Zone_DME, "")
{
	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		if (enterer == STAR)
		{
			GameObject *apc1 = Commands->Find_Object (100256);
			Commands->Send_Custom_Event ( STAR, apc1, 100, 100, 0.0f );
			
			
		}
	}
};

DECLARE_SCRIPT(M05_Activate_Surprise_Tank, "")
{
		
	bool already_entered;
	
	enum{EXPLODE_BLOCK};

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

			Commands->Enable_Spawner(100271, true);

			Commands->Start_Timer (obj, this, 1.0f, EXPLODE_BLOCK);
			
			
		}
	}
	
	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if(timer_id == EXPLODE_BLOCK)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(105382)), NULL);
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(101865)), NULL);
			

			Commands->Destroy_Object(Commands->Find_Object(105382));
			Commands->Destroy_Object(Commands->Find_Object(101865));
			Commands->Destroy_Object(obj);
			

		}
		
	}

};

DECLARE_SCRIPT(M05_Surprise_Tank, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Enable_Enemy_Seen( obj, true);
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
		params.WaypathID = 101236;
		params.WaypathSplined = true;
		Commands->Action_Attack( obj, params );
		
	}

};

DECLARE_SCRIPT(M05_Execution_Civilian, "")
{
	bool poked;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
		params.Set_Animation("S_A_HUMAN.H_A_Host_L2b", true);
		Commands->Action_Play_Animation(obj, params);

		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		poked = false;
		Commands->Innate_Disable(obj);
	}


	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if ((Commands->Is_A_Star(poker)) && (!poked))
		{
			poked = true;
			Commands->Innate_Enable(obj);
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );

			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
			params.Set_Animation("S_A_HUMAN.H_A_Host_L2c", false);
			Commands->Action_Play_Animation(obj, params);

			// You are so brave, GDI! How can I ever repay you for your help?
			const char *conv_name = ("M05_CON034");
			int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300509);

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * 1.5;
			float b = sin(DEG_TO_RADF(facing)) * 1.5;
			Vector3 powerup_loc = pos + Vector3(a, b, 0.5f);
			Commands->Create_Object("tw_POW00_Health", powerup_loc);

		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		}

	}

};

DECLARE_SCRIPT(M05_Execution_Nod, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poked, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;
		Commands->Innate_Disable(obj);
		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_CUSTOM_ACTIVATE)
		{
			Commands->Innate_Enable(obj);
/*			if(Commands->Find_Object(101239))
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 10 );
				params.Set_Attack (Commands->Find_Object(101239), 250.0f, 0.0f, 1);
				Commands->Action_Attack (obj, params);
			}
*/
		}
	}
};

DECLARE_SCRIPT(M05_Activate_Entrapment_Civ, "")
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
			// Send custom to Civs to change playertype
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100272), 100, 100, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100273), 100, 100, 0.0f);

			// Mendoza
			Commands->Enable_Spawner(100275, true);

			// Drop balcony soldiers
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-158.668f, 53.528f, 28.338f));
			Commands->Set_Facing(chinook_obj1, -30.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_C130TroopDrop3.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-126.819f, 76.467f, 34.683f));
			Commands->Set_Facing(chinook_obj2, -30.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X5D_C130TroopDrop4.txt");

	//		GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-121.787f, 43.174f, 21.511f));
	//		Commands->Set_Facing(chinook_obj3, -30.000f);
	//		Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X5D_C130TroopDrop5.txt");

		}
	}

};

DECLARE_SCRIPT(M05_Entrapment_Mendoza, "")
{
	bool exiting;
	float health;

	enum{BYE_MENDOZA};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 1 );
		SAVE_VARIABLE( exiting, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		exiting = false;
		health = Commands->Get_Max_Health(obj);

		// You're gonna bleed, GDI!\n
		const char *conv_name = ("M05_CON035");
		int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 1);
		Commands->Monitor_Conversation (obj, conv_id);	

		Commands->Set_Innate_Is_Stationary(obj, true);

	}

	
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 15 && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
			Commands->Set_Facing(obj, -180.0f);
		}
		if(action_id == BYE_MENDOZA && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Destroy_Object(obj);
		}
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
		if((Commands->Get_Health(obj)) < (Commands->Get_Max_Health(obj) * .23))
		{
			Commands->Set_Health( obj, (Commands->Get_Max_Health(obj) * .23) );
			
			if(!exiting)
			{
				// Hah! Is that all you got?\n
				const char *conv_name = ("M05_CON016");
				int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 200.0f, false);
				Commands->Join_Conversation(obj, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 1);
				Commands->Monitor_Conversation (obj, conv_id);		

				exiting = true;

				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 15 );
				params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
				params.WaypathID = 100282;
				Commands->Action_Goto( obj, params );
			}
		}
	}
	

};

DECLARE_SCRIPT(M05_Cathedral_Controller, "")  // 100287
{
		
	int soldier_id1;
	int soldier_id2;
	int soldier_id3;
	int m05_cathedral_vehicle_killed;
	int blackhand_cnt;

	enum{SECOND_APACHE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( soldier_id1, 1 );
		SAVE_VARIABLE( soldier_id2, 2 );
		SAVE_VARIABLE( soldier_id3, 3 );
		SAVE_VARIABLE( m05_cathedral_vehicle_killed, 4 );
		SAVE_VARIABLE( blackhand_cnt, 5 );
	}

	void Created (GameObject * /* obj */) override
	{
		soldier_id1 = 0;
		soldier_id2 = 0;
		soldier_id3 = 0;
		m05_cathedral_vehicle_killed = 0;
		blackhand_cnt = 0;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M05_INITIATE_CATHEDRAL)
		{
			GameObject * apache1 = Commands->Create_Object ( "Nod_Apache", Vector3(-182.814f, 226.598f, 65.317f));
			Commands->Attach_Script(apache1, "M05_Cathedral_Apache", "1");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_VEHICLE_CREATED, 1, 0.0f);

			Commands->Start_Timer(obj, this, 3.0f, SECOND_APACHE);

			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-237.069f, 182.809f, 30.381f));
			Commands->Set_Facing(chinook_obj1, 125.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M05_XG_VehicleDrop4.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-120.930f, 108.820f, 29.828f));
			Commands->Set_Facing(chinook_obj2, -45.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "M05_XG_VehicleDrop5.txt");

			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-252.027f, 118.122f, 29.883f));
			Commands->Set_Facing(chinook_obj3, -150.000f);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "M05_XG_VehicleDrop6.txt");
		}

		if(type == M05_CATHEDRAL_VEHICLE_CREATED)
		{
			m05_cathedral_vehicle_killed++;
		}

		if(type == M05_CATHEDRAL_VEHICLE_KILLED)
		{
			m05_cathedral_vehicle_killed--;
					
		}

		if(type == M05_CATHEDRAL_REINFORCE && blackhand_cnt < 9)
		{
			blackhand_cnt++;

			switch(param)
			{
			case 7:
				{
					GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-167.978f, 178.023f, 29.226f));
					Commands->Set_Facing(chinook_obj1, 0.000f);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop7.txt");
					
				}
				break;
			case 8:
				{
					GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-179.221f, 110.382f, 28.744f));
					Commands->Set_Facing(chinook_obj2, 0.000f);
					Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X5D_CHTroopdrop8.txt");
					
				}
				break;
			case 10:
				{
					GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-137.472f, 141.174f, 29.144f));
					Commands->Set_Facing(chinook_obj3, 0.000f);
					Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X5D_CHTroopdrop10.txt");
				
				}
				break;
			}
						
		}

		if(m05_cathedral_vehicle_killed == 0 && blackhand_cnt > 7)
		{
			Commands->Create_Logical_Sound(obj, M05_CATHEDRAL_FREE, Commands->Get_Position(obj), 5000.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 506, 1, 0.0f);
		}
		
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if(timer_id == SECOND_APACHE)
		{
			GameObject * apache2 = Commands->Create_Object ( "Nod_Apache", Vector3(-116.199f, 300.494f, 54.718f));
			Commands->Attach_Script(apache2, "M05_Cathedral_Apache", "2");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_VEHICLE_CREATED, 1, 0.0f);

			// Blackhand infantry
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-167.978f, 178.023f, 29.226f));
			Commands->Set_Facing(chinook_obj1, 0.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X5D_CHTroopdrop7.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-179.221f, 110.382f, 28.744f));
			Commands->Set_Facing(chinook_obj2, 0.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X5D_CHTroopdrop8.txt");

			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-137.472f, 141.174f, 29.144f));
			Commands->Set_Facing(chinook_obj3, 0.000f);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X5D_CHTroopdrop10.txt");
		}
	}
};

DECLARE_SCRIPT(M05_Cathedral_Apache, "Apache_ID=0:int")  // 100287
{
	int apache_id;
	int fire_loc[10];
	bool attacking;

	enum{ASSAULT_WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( apache_id, 1 );
//		SAVE_VARIABLE( fire_loc, 2 );
		SAVE_VARIABLE( attacking, 3 );
	}

	void Created (GameObject * obj) override
	{
		fire_loc[0] = 100326;
		fire_loc[1] = 100327;
		fire_loc[2] = 100328;
		fire_loc[3] = 100329;
		fire_loc[4] = 100330;
		fire_loc[5] = 100331;
		fire_loc[6] = 100332;
		fire_loc[7] = 100333;
		fire_loc[8] = 100334;
		fire_loc[9] = 100335;

		ActionParamsStruct params;

		Commands->Enable_Enemy_Seen( obj, true);
		apache_id = Get_Int_Parameter("Apache_ID");
		attacking = true;
		Commands->Disable_Physical_Collisions(obj);

		// Initial Apache Path
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ASSAULT_WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
		params.WaypathID = 101320;
		Commands->Action_Goto( obj, params );
	
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == ASSAULT_WAYPATH)
		{
			switch(apache_id)
			{
			case 1:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
					params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
					params.WaypathID = 100300;
					Commands->Action_Attack( obj, params );

					
				}
				break;
			case 2:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
					params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
					params.WaypathID = 100310;
					Commands->Action_Attack( obj, params );
				}
				break;
			}

			// Fire at attack locs
			Commands->Start_Timer(obj, this, 1.0f, apache_id);
		}
		
		
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		int random = Commands->Get_Random_Int(0,ARRAY_ELEMENT_COUNT(fire_loc)); 
		attacking = false;

		if(timer_id == 1)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 100, 0.5f, true);
			params.WaypathID = 100300;
			params.AttackCheckBlocked = false;
			Commands->Modify_Action (obj, 10, params, true, true);
		}
		if(timer_id == 2)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 100, 0.5f, true);
			params.WaypathID = 100310;
			params.AttackCheckBlocked = false;
			Commands->Modify_Action (obj, 10, params, true, true);
		}

		Commands->Start_Timer(obj, this, 6.0f, apache_id);
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		if ( sound.Type == M05_CATHEDRAL_FREE )
		{
			Commands->Apply_Damage( obj, 100000, "DEATH", obj );
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			switch(apache_id)
			{
			case 1:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
					params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
					params.Set_Attack (enemy, 100, 0.5f, true);
					params.WaypathID = 100300;
					params.AttackCheckBlocked = false;
					Commands->Modify_Action (obj, 10, params, true, true);
				}
				break;
			case 2:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
					params.Set_Movement( Vector3(0,0,0), RUN, 0.0f );
					params.Set_Attack (enemy, 100, 0.5f, true);
					params.WaypathID = 100310;
					params.AttackCheckBlocked = false;
					Commands->Modify_Action (obj, 10, params, true, true);
				}
				break;
			}

			Commands->Start_Timer(obj, this, 6.0f, apache_id);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_VEHICLE_KILLED, 1, 0.0f);
	}
	
};

DECLARE_SCRIPT(M05_Cathedral_Para_Unit, "Soldier_ID=0:int")
{
	bool cathedral_free;

	enum{GO_STAR, ATTACK_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( cathedral_free, 1 );
	}

	void Created (GameObject * obj) override
	{
		cathedral_free = false;

		ActionParamsStruct params;

		params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), GO_STAR);
		params.Set_Movement (STAR, RUN, 30.0f);
		Commands->Action_Goto (obj, params);
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		float distance = Commands->Get_Distance (Commands->Get_Position(obj), Commands->Get_Position(STAR));
		if(timer_id == GO_STAR && distance > 30.0f)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), GO_STAR);
			params.Set_Movement (STAR, RUN, 30.0f);
			Commands->Action_Goto (obj, params);
		}
		else
		{
			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_STAR && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), ATTACK_STAR);
			params.Set_Movement (STAR, RUN, 3.0f);
			Commands->Action_Goto (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, GO_STAR);
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		if ( sound.Type == M05_CATHEDRAL_FREE )
		{
			cathedral_free = true;
			Commands->Apply_Damage( obj, 100000, "DEATH", obj );
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(!cathedral_free)
		{
			int soldier_id = Get_Int_Parameter("Soldier_ID");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_REINFORCE, soldier_id, 0.0f);
		}
	}

};

DECLARE_SCRIPT(M05_Cache_Surprise, "")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-481.123f, -106.686f, 15.787f));
			Commands->Set_Facing(controller1, -90.000f);
			Commands->Attach_Script(controller1, "Test_Cinematic", "X5C_Wintroops06.txt");

			GameObject *controller2 = Commands->Create_Object("Invisible_Object", Vector3(-486.106f, -93.892f, 18.440f));
			Commands->Set_Facing(controller2, 175.000f);
			Commands->Attach_Script(controller2, "Test_Cinematic", "X5C_Wintroops07.txt");
			
		}
	}

};

DECLARE_SCRIPT(M05_Vehicle_Dec, "")
{
	bool attacking;
	
	enum{ATTACK_OVER, ATTACKING};
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
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ATTACKING );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 10.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ATTACKING );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			Commands->Modify_Action (obj, ATTACKING, params, true, true);
			
		}
		
	}

};

DECLARE_SCRIPT(M05_Roadblock_Controller, "")  //100614
{
	int unit_id1;
	int unit_id2;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id1, 1 );
		SAVE_VARIABLE( unit_id2, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		unit_id1 = 0;
		unit_id2 = 0;

		GameObject *controller1 = Commands->Create_Object("Invisible_Object", Vector3(-484.498f, -130.355f, 14.680f));
		Commands->Set_Facing(controller1, -115.000f);
		Commands->Attach_Script(controller1, "Test_Cinematic", "X5C_Wintroops18.txt");

		GameObject *controller2 = Commands->Create_Object("Invisible_Object", Vector3(-483.220f, -122.518f, 17.043f));
		Commands->Set_Facing(controller2, 90.000f);
		Commands->Attach_Script(controller2, "Test_Cinematic", "X5C_Wintroops19.txt");
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == M05_ROADBLOCK_REINFORCE)
		{
			switch(param)
			{
			case 1:
				{
					unit_id1++;
					if(unit_id1 < 3)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-484.498f, -130.355f, 14.680f));
						Commands->Set_Facing(controller, -115.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops18.txt");
					}
					else if(unit_id1 >= 3 && unit_id1 < 5)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-484.498f, -130.355f, 14.680f));
						Commands->Set_Facing(controller, -115.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops08.txt");
					}
					
				}
				break;
			case 2:
				{
					unit_id2++;
					if(unit_id2 < 3)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-483.220f, -122.518f, 17.043f));
						Commands->Set_Facing(controller, 90.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops19.txt");
					}
					else if(unit_id2 >= 3 && unit_id1 < 5)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-483.220f, -122.518f, 17.043f));
						Commands->Set_Facing(controller, 90.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X5C_Wintroops09.txt");
					}
					
				}
				break;
			}
		}
	}

	
};

DECLARE_SCRIPT(M05_Roadblock_Unit, "Unit_ID=0:int")
{
	int unit_id;
	
	enum{GO_ROADLBOCK};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer (obj, this, 3.0f, GO_ROADLBOCK);

		unit_id = Get_Int_Parameter("Unit_ID");
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == GO_ROADLBOCK && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer (obj, this, 6.0f, GO_ROADLBOCK);
		}
		if(action_id == GO_ROADLBOCK && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer (obj, this, 20.0f, GO_ROADLBOCK);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_ROADLBOCK)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_ROADLBOCK);
			params.Set_Movement (Commands->Find_Object(100614), RUN, 5.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100614), M05_ROADBLOCK_REINFORCE, unit_id, 0.0f);
	}
};

DECLARE_SCRIPT(M05_Activate_Roadblock_Tank, "")
{
		
	bool already_entered;
	
	enum{EXPLODE_BLOCK};

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

			Commands->Enable_Spawner(100618, true);

			Commands->Start_Timer (obj, this, 1.0f, EXPLODE_BLOCK);
		}
	}
	
	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == EXPLODE_BLOCK)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(101858)), NULL);
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(101859)), NULL);
			Commands->Destroy_Object(Commands->Find_Object(101858));
			Commands->Destroy_Object(Commands->Find_Object(101859));
		}
	}
};

DECLARE_SCRIPT(M05_Building_Debris, "")
{
	int health;
	bool already_entered;
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
		SAVE_VARIABLE( health, 1 );
		SAVE_VARIABLE( already_entered, 2 );
	}

	void Created (GameObject * obj) override
	{
		health = Commands->Get_Max_Health(obj);
		//Commands->Set_Shield_Type ( obj, "ShieldKevlar" );
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(Commands->Get_Player_Type(damager) == SCRIPT_PLAYERTYPE_NOD)
		{
			Commands->Set_Health(obj, health);
		}
		else
		{
			health = Commands->Get_Health(obj);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */ ) override
	{
		Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(obj), NULL);
		GameObject * explode_debris = Commands->Create_Object("M05_X5A_AG_DExplode", Vector3(0,0,0));
		Commands->Attach_Script(explode_debris, "M05_Explode_Debris", "");
		Commands->Destroy_Object(obj);
	}
};

DECLARE_SCRIPT(M05_Explode_Debris, "")
{
	
	bool already_entered;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

	//	params.Set_Basic(this, 100, 10);
	//	params.Set_Animation("X5A_Debris.X5A_Debris_Boom", true);
	//	Commands->Action_Play_Animation(obj, params);
		Commands->Set_Animation(obj, "X5A_Debris.X5A_Debris_Boom", false, NULL, 0.0f, -1.0f, false);
		
		Commands->Start_Timer (obj, this, 1.0f, 10);
		Commands->Create_Sound("Medium Explosion Sound Twiddler", Commands->Get_Position(obj), obj);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == 10)
		{
			Commands->Destroy_Object(obj);
		}
	}

};

DECLARE_SCRIPT(M05_Flyover_Controller, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 25.0f, 0);
		last = 21;
		
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char *flyovers[3] =
		{
			"X5H_Apache_1.txt",
			"X5H_Apache_2.txt",
			"X5H_Apache_3.txt",
		
		};
		int random = int(Commands->Get_Random(0, 3-WWMATH_EPSILON));
	
		while (random == last) {
			random = int(Commands->Get_Random(0, 3-WWMATH_EPSILON));
		}
		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		Commands->Start_Timer(obj, this, 10.0f, 0);
		last = random;
	}
};

DECLARE_SCRIPT(M00_InnateIsStationary, "")
{
	
	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
	}
};

DECLARE_SCRIPT(M05_Dead6_Help, "Message_ID=0:int")
{
	bool already_entered;
	int message_id;
	bool deadeye_freed;
	int mission_failed_text;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( deadeye_freed, 2 );
		SAVE_VARIABLE( message_id, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		message_id = Get_Int_Parameter("Message_ID");
		deadeye_freed = false;
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
			switch(message_id)
			{
			case 1:
				{
					// Gunner
					if(Commands->Find_Object(100003))
					{
						// I need help!  Where's the second insertion team?! Backup needed at the Town Square!\n
						const char *conv_name = ("M05_CON039");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100003);
						
					}
				}
				break;
			case 2:
				{
					// Hotwire
					if(Commands->Find_Object(100002))
					{
						// Hotwire here - I'm losing ground.  I need support or my own body bag!\n
						const char *conv_name = ("M05_CON040");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100002);
						
					}
				}
				break;
			case 3:
				{
					// Deadeye
					if(!deadeye_freed)
					{
						// Havoc, this is Deadeye. Nod pressure increasing.  Need backup immediately!\n
						const char *conv_name = ("M05_CON041");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100650);
						
					}
				}
				break;
			// Death message, mission failure
			case 4:
				{
					// Hotwire
					if(Commands->Find_Object(100002))
					{
						mission_failed_text = 1;
						// This is Hotwire, I can't hold this position! Requesting - oh great! <gunfire and static>\n
						const char *conv_name = ("M05_CON043");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100002);
						Commands->Monitor_Conversation (obj, conv_id);	
					}
					// Gunner
					else if(Commands->Find_Object(100003))
					{
						mission_failed_text = 2;
						// This is Gunner, too many Nod reinforcements... <gunfire> <static>\n
						const char *conv_name = ("M05_CON042");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100003);
						Commands->Monitor_Conversation (obj, conv_id);	
					}
					// Deadeye
					else if(!deadeye_freed)
					{
						mission_failed_text = 3;
						// This is Deadeye, I'm being overrun - <exchanges of gunfire - static>\n
						const char *conv_name = ("M05_CON044");
						int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
						Commands->Join_Conversation(NULL, conv_id, false, true, true);
						Commands->Start_Conversation (conv_id, 100652);
						Commands->Monitor_Conversation (obj, conv_id);	
					}
				}
				break;
			
			}
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_DEADEYE_FREED)
		{
			deadeye_freed = true;
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if(action_id == 100003 || action_id == 100002 || action_id == 100652)
			{
				switch(mission_failed_text)
				{
				case 1:
					{
						// hotwire killed.
						Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0174I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
						Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 503, 2, 1.0f);
					}
					break;
				case 2:
					{
						// gunner killed
						Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0176I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
						Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 501, 2, 1.0f);
					}
					break;
				case 3:
					{
						// deadeye killed
						Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0178I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
						Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 502, 2, 1.0f);
					}
					break;
				}
			}

		}
	}

};

DECLARE_SCRIPT(M05_Hotwire_Conversation, "")
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
			
			// Havoc? Bust me outta here?\n
			const char *conv_name = ("M05_CON036");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300503);
			Commands->Monitor_Conversation (obj, conv_id);	
		}
	}

};

DECLARE_SCRIPT(M05_Activate_Surprise, "Spawner_ID1=0:int, Spawner_ID2=0:int, Spawner_ID3=0:int")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
			
			int spawner_id1 = Get_Int_Parameter("Spawner_ID1");
			int spawner_id2 = Get_Int_Parameter("Spawner_ID2");
			int spawner_id3 = Get_Int_Parameter("Spawner_ID3");
			
			if(spawner_id1 != 0)
			{
				Commands->Enable_Spawner(spawner_id1, true);
			}
			if(spawner_id2 != 0)
			{
				Commands->Enable_Spawner(spawner_id2, true);
			}
			if(spawner_id3 != 0)
			{
				Commands->Enable_Spawner(spawner_id3, true);
			}
			

		}
	}

};

DECLARE_SCRIPT(M05_Surprise_Unit, "")
{
	
	enum{DIE_SURPRISE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 1 );
		params.Set_Movement( STAR, RUN, 2.0f );
		Commands->Action_Goto (  obj, params );
	
//		Commands->Start_Timer (obj, this, 10.0f, DIE_SURPRISE);
	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		
		if(timer_id == DIE_SURPRISE)
		{
			Commands->Apply_Damage( obj, 100000, "STEEL", NULL );
			Vector3 obj_pos = Commands->Get_Position(obj);
			Vector3 shot_pos;
			shot_pos.X = obj_pos.X + 4.0f;
			shot_pos.Y = obj_pos.Y + 4.0f;
			shot_pos.Z = obj_pos.Z + 4.0f;

			Commands->Create_Sound("Sniper_Fire_01", shot_pos, obj);
		}
	}
};

DECLARE_SCRIPT(M05_Activate_Artillery, "Artillery_ID1=0:int, Artillery_ID2=0:int, Artillery_ID3=0:int")
{
		
	bool already_entered;
	int loop_amnt;
	int loop_count;
	int artillery_sound1;
	int artillery_sound2;
	int artillery_sound3;
	Vector3 artillery_loc1;
	Vector3 artillery_loc2;
	Vector3 artillery_loc3;
	
	enum{START_ARTILLERY};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( loop_amnt, 2 );
		SAVE_VARIABLE( loop_count, 3 );
		SAVE_VARIABLE( artillery_sound1, 4 );
		SAVE_VARIABLE( artillery_sound2, 5 );
		SAVE_VARIABLE( artillery_sound3, 6 );
		SAVE_VARIABLE( artillery_loc1, 7 );
		SAVE_VARIABLE( artillery_loc2, 8 );
		SAVE_VARIABLE( artillery_loc3, 9 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		loop_amnt = 0;
		loop_count = 0;
		artillery_sound1 = 0;
		artillery_sound2 = 0;
		artillery_sound3 = 0;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
			
			loop_amnt = Commands->Get_Random_Int(2, 4);
			Commands->Start_Timer (obj, this, 1.0f, START_ARTILLERY);

		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		
		if(timer_id == START_ARTILLERY && loop_count < loop_amnt)
		{
			loop_count++;

			int artillery_id1 = Get_Int_Parameter("Artillery_ID1");
			artillery_loc1 = Commands->Get_Position(Commands->Find_Object(artillery_id1));

			int artillery_id2 = Get_Int_Parameter("Artillery_ID2");
			artillery_loc2 = Commands->Get_Position(Commands->Find_Object(artillery_id2));

			int artillery_id3 = Get_Int_Parameter("Artillery_ID3");
			artillery_loc3 = Commands->Get_Position(Commands->Find_Object(artillery_id3));
			
			static constexpr const char *bomb[3] =
			{
				"Falling_Bomb_01",
				"Falling_Bomb_02",
				"Falling_Bomb_03",
			};

			int random = Commands->Get_Random_Int(0, 2);

			artillery_sound1 = Commands->Create_Sound ( bomb[random], artillery_loc1, obj );
			Commands->Monitor_Sound (obj, artillery_sound1);
			
			random = Commands->Get_Random_Int(0, 2);

			artillery_sound2 = Commands->Create_Sound ( bomb[random], artillery_loc2, obj );
			Commands->Monitor_Sound (obj, artillery_sound2);
			
			random = Commands->Get_Random_Int(0, 2);

			artillery_sound3 = Commands->Create_Sound ( bomb[random], artillery_loc3, obj );
			Commands->Monitor_Sound (obj, artillery_sound3);

			float time = Commands->Get_Random(0.5f, 3.0f);
			Commands->Start_Timer (obj, this, time, START_ARTILLERY);
			
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound1)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc1, NULL);
		}
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound2)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc2, NULL);
		}
		if(type == CUSTOM_EVENT_SOUND_ENDED && param == artillery_sound3)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", artillery_loc3, NULL);
		}
	}

};

DECLARE_SCRIPT(M05_Activate_ApacheStrike, "Strike_Loc_ID=0:int")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
		//	already_entered = true;

			int strike_loc_id = Get_Int_Parameter("Strike_Loc_ID");
			GameObject * strike_loc = Commands->Find_Object(strike_loc_id);
			Vector3 pos = Commands->Get_Position(strike_loc);
			float facing = Commands->Get_Facing(strike_loc);

			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", pos);
			Commands->Set_Facing(chinook_obj, facing);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X7B_ApacheStk.txt");
			
		}
	}

};


DECLARE_SCRIPT(M05_Activate_Truck_Spill, "")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Set_Facing(chinook_obj, 0.0f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "X5E_TibSpill.txt");
			
		}
	}

};


DECLARE_SCRIPT(M05_Activate_Triangle_Tank_Drop, "")
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

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			// Drop tank
			GameObject * chinook_obj = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(Commands->Find_Object(108970)));
			Commands->Set_Facing(chinook_obj, -35.000f);
			Commands->Attach_Script(chinook_obj, "Test_Cinematic", "M05_XG_VehicleDrop2.txt");
		}
	}

};

DECLARE_SCRIPT(M05_Aggressiveness_Take_Cover, "Aggessiveness=0.0:float, Take_Cover=0.0:float")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		float aggressiveness = Get_Float_Parameter("Aggessiveness");
		float take_cover = Get_Float_Parameter("Take_Cover");

		Commands->Set_Innate_Aggressiveness(obj, aggressiveness );
		Commands->Set_Innate_Take_Cover_Probability(obj, take_cover );
	}

};

DECLARE_SCRIPT(M05_Activate_Execution, "")
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
			
			// Execution players
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101239), M05_CUSTOM_ACTIVATE, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101240), M05_CUSTOM_ACTIVATE, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101241), M05_CUSTOM_ACTIVATE, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101242), M05_CUSTOM_ACTIVATE, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101243), M05_CUSTOM_ACTIVATE, 1, 0.0f);

			// Civ lead
//			Commands->Send_Custom_Event(obj, Commands->Find_Object(100098), M05_CUSTOM_ACTIVATE, 1, 0.0f);
		}
	}

};


DECLARE_SCRIPT(M05_Swap_Artillery, "Artillery_ID=0:int")
{
	bool swapped;
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( swapped, 1 );
		
	}

	void Created (GameObject * /* obj */) override
	{
		swapped = false;
	}	

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == M05_SWAP_ARTILLERY ) 
		{
			swapped = true;
			Commands->Destroy_Object(obj);
			Vector3 curr_loc = Commands->Get_Position(obj);
			curr_loc.Z -= 1.0f;
			float facing = Commands->Get_Facing(obj);
			GameObject * new_artillery = Commands->Create_Object("Nod_Mobile_Artillery", curr_loc);
			switch(Get_Int_Parameter("Artillery_ID"))
			{
			case 4:
				{
					Commands->Attach_Script(new_artillery, "M05_Cathedral_Artillery", "100335, 100333");
				}
				break;
			case 5:
				{
					Commands->Attach_Script(new_artillery, "M05_Cathedral_Artillery", "100328, 101327");
				}
				break;
			case 6:
				{
					Commands->Attach_Script(new_artillery, "M05_Cathedral_Artillery", "100333, 100330");
				}
				break;
			}
			Commands->Set_Facing(new_artillery, facing);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_VEHICLE_CREATED, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M05_Cathedral_Artillery, "Fire_Loc1=0:int, Fire_Loc2=0:int")
{

	int fire_loc[2];	

	enum {TANK_TIMER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( fire_loc[0], 1 );
		SAVE_VARIABLE( fire_loc[1], 1 );
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		fire_loc[0] = Get_Int_Parameter("Fire_Loc1");
		fire_loc[1] = Get_Int_Parameter("Fire_Loc2");
		
		// Fire upon cathedral
		Commands->Start_Timer (obj, this, 1.0f, TANK_TIMER);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == TANK_TIMER)
		{
			ActionParamsStruct params;
			
			int random = Commands->Get_Random_Int(0,ARRAY_ELEMENT_COUNT(fire_loc)); 
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (Commands->Find_Object(fire_loc[random]), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );

			// Fire upon building
			Commands->Start_Timer (obj, this, 5.0f, TANK_TIMER);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100287), M05_CATHEDRAL_VEHICLE_KILLED, 1, 0.0f);
	}

	
};

DECLARE_SCRIPT(M05_X5N_MIDTRO_B, "")
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
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), M05_DESTROY_OBJECT, 0, 0.0f);

			Vector3 havoc_loc = Commands->Get_Position (STAR);
			havoc_loc.Z += 7.0f;
			Commands->Set_Position (STAR, havoc_loc );
			
			GameObject * cinematic_obj = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Attach_Script(cinematic_obj, "Test_Cinematic", "X5N_MIDTRO_B.txt");
		}
	}

};

DECLARE_SCRIPT(M05_Barrel_Explosion, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Destroyed(GameObject * obj) override
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Create_Explosion ( "Explosion_Barrel_Toxic", myPosition, NULL );
	}
	
/*	void Killed (GameObject * obj, GameObject * killer) override
	{
		Vector3 myPosition = Commands->Get_Position ( obj );
		Commands->Create_Explosion ( "Explosion_Barrel_Toxic", myPosition, NULL );
	}*/

};

DECLARE_SCRIPT(M05_APC_Deploy, "Preset:string, Soldier_Qty=0:int, Fire_Gun=1:int")
{
	bool attacking;
	bool fire_gun;

	enum{DEPLOY_SOLDIER1, DEPLOY_SOLDIER2, DEPLOY_SOLDIER3};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( fire_gun, 2 );
	}
		
	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		fire_gun = (Get_Int_Parameter("Fire_Gun")) ? true : false;
	}

	void Deploy_Soldiers(GameObject * obj)
	{
		Commands->Set_Animation(obj, "V_NOD_APC.V_NOD_APC", 0, NULL, 0.0f, -1.0f, false);
		int soldier_qty = Get_Int_Parameter("Soldier_Qty");
		if(soldier_qty > 0)
		{
			Commands->Start_Timer (obj, this, 50.0f/30.0f, DEPLOY_SOLDIER1);
		}
		if(soldier_qty > 1)
		{
			Commands->Start_Timer (obj, this, 70.0f/30.0f, DEPLOY_SOLDIER2);
		}
		if(soldier_qty > 2)
		{
			Commands->Start_Timer (obj, this, 97.0f/30.0f, DEPLOY_SOLDIER3);
		}
	}


	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if((enemy == STAR) && (!attacking))
		{
			attacking = true;
			Deploy_Soldiers(obj);
		}
		if(fire_gun)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if((damager == STAR) && (!attacking))
		{
			attacking = true;
			Deploy_Soldiers(obj);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		char param1[12];
		sprintf(param1, "%d", Commands->Get_ID(obj));
		const char * preset = Get_Parameter("Preset");

		if(timer_id == DEPLOY_SOLDIER1)
		{
			GameObject * soldier = Commands->Create_Object_At_Bone(obj, preset, "BN_Troop_01");
			Commands->Attach_To_Object_Bone(soldier, obj, "BN_Troop_01");
			Commands->Attach_Script(soldier, "M05_APC_Deploy_Soldier", param1);
		}
		if(timer_id == DEPLOY_SOLDIER2)
		{
			GameObject * soldier = Commands->Create_Object_At_Bone(obj, preset, "BN_Troop_02");
			Commands->Attach_To_Object_Bone(soldier, obj, "BN_Troop_02");
			Commands->Attach_Script(soldier, "M05_APC_Deploy_Soldier", param1);
		}
		if(timer_id == DEPLOY_SOLDIER3)
		{
			GameObject * soldier = Commands->Create_Object_At_Bone(obj, preset, "BN_Troop_03");
			Commands->Attach_To_Object_Bone(soldier, obj, "BN_Troop_03");
			Commands->Attach_Script(soldier, "M05_APC_Deploy_Soldier", param1);
		}
	}

};

DECLARE_SCRIPT(M05_APC_Deploy_Soldier, "APC_ID=0:int")
{
	enum{GO_STAR, CLEAR_APC};
	
	void Created (GameObject * obj) override
	{
		Commands->Set_Animation(obj, "S_A_Human.H_A_XG_NAPC_OUT", 0, NULL, 0.0f, -1.0f, false);
		Commands->Enable_Hibernation (obj, false);		
	}
	
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "S_A_Human.H_A_XG_NAPC_OUT") == 0)
		{
			GameObject * apc = Commands->Find_Object(Get_Int_Parameter("APC_ID"));
			Commands->Attach_To_Object_Bone(obj, NULL, NULL);
			
			Vector3 pos = Commands->Get_Position(apc);
			float facing = Commands->Get_Facing(apc);
			float a = cos(DEG_TO_RADF(facing)) * Commands->Get_Random(-8.0f, -10.0f);
			float b = sin(DEG_TO_RADF(facing)) * Commands->Get_Random(-8.0f, -10.0f);
			Vector3 soldier_loc = pos + Vector3(a, b, 0.0f);
			
			ActionParamsStruct params;

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), CLEAR_APC);
			params.Set_Movement (soldier_loc, RUN, 1.5f);
			Commands->Action_Goto (obj, params);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == CLEAR_APC)
		{
			Commands->Enable_Hibernation (obj, true);

			ActionParamsStruct params;

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_STAR);
			params.Set_Movement (STAR, RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		
	}

};

DECLARE_SCRIPT(M05_Inn_APC, "")
{
	int reinforcement;
	bool paradrop;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( reinforcement, 1 );
		SAVE_VARIABLE( paradrop, 2 );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M05_REINFORCEMENT_KILLED)
		{
			
			reinforcement++;
			if((reinforcement%2 == 0) && (reinforcement < 3))
			{
				
			}
			else if (reinforcement%2 == 0)
			{
				if(obj == Commands->Find_Object(100247))
				{
					paradrop = true;
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 1, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 1, 0.5f);
				}
				else
				{
					paradrop = true;
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 2, 0.0f);
					Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 2, 0.5f);
				}

			}
		}
	}


	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if((obj == Commands->Find_Object(100247)) && (!paradrop))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 1, 0.5f);
		}
		else if (!paradrop)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 2, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100244), M05_INN_REINFORCE, 2, 0.5f);
		}
	}

};

#define	RESISTANCE_CONV_TABLE_SIZE  ( sizeof(Resistance_Conv_Table) / sizeof (Resistance_Conv_Table[0]) )
const char *	Resistance_Conv_Table[] = 
{
	"M05_CON044",
	"M05_CON045",
	"M05_CON046",
	"M05_CON047",
	"M05_CON048",
	"M05_CON049",
	"M05_CON050",
	"M05_CON051",
	"M05_CON052",
	"M05_CON053",
	"M05_CON054",
	"M05_CON055",	//GCF1 11
	"M05_CON056",
	"M05_CON057",
	"M05_CON058",
	"M05_CON059",
	"M05_CON060",
	"M05_CON061",
	"M05_CON062",
	"M05_CON063",
	"M05_CON064",
	"M05_CON065",	//GCM1 21
	"M05_CON066",
	"M05_CON067",
	"M05_CON068",
	"M05_CON069",
	"M05_CON070",
	"M05_CON071",
	"M05_CON072",
	"M05_CON073",
	"M05_CON074",
	"M05_CON075",
	"M05_CON076",
	"M05_CON077",
	"M05_CON078",
	"M05_CON079",
	"M05_CON080"	//GCM2  36

};

DECLARE_SCRIPT(M05_Resistance_Poke_Conversation, "")
{
	int random;
	int last;
	bool conversation;

	enum{CONVERSATION};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( random, 1 );
		SAVE_VARIABLE( last, 2 );
		SAVE_VARIABLE( conversation, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		random = Commands->Get_Random_Int(0, RESISTANCE_CONV_TABLE_SIZE);
		conversation = false;
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		if(conversation)
		{
			return;
		}

		conversation = true;

		const char * preset_name = Commands->Get_Preset_Name( obj );
		int min = 0;
		int max = 0;

		if((::strncmp( "Civ_Resist_Female_v0a", preset_name, ::strlen( "Civ_Resist_Female_v0a" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Female_v0b", preset_name, ::strlen( "Civ_Resist_Female_v0b" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Female_v0c", preset_name, ::strlen( "Civ_Resist_Female_v0c" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Female_v0d", preset_name, ::strlen( "Civ_Resist_Female_v0d" ) ) == 0 ))

		{
			min = 0;
			max = 11;
		}

		if((::strncmp( "Civ_Resist_Male_v0a", preset_name, ::strlen( "Civ_Resist_Male_v0a" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v0b", preset_name, ::strlen( "Civ_Resist_Male_v0b" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v0c", preset_name, ::strlen( "Civ_Resist_Male_v0c" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v0d", preset_name, ::strlen( "Civ_Resist_Male_v0d" ) ) == 0 ))

		{
			min = 12;
			max = 21;
		}

		if((::strncmp( "Civ_Resist_Male_v1a", preset_name, ::strlen( "Civ_Resist_Male_v1a" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v1b", preset_name, ::strlen( "Civ_Resist_Male_v1b" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v1c", preset_name, ::strlen( "Civ_Resist_Male_v1c" ) ) == 0 ) ||
			(::strncmp( "Civ_Resist_Male_v1d", preset_name, ::strlen( "Civ_Resist_Male_v1d" ) ) == 0 ))

		{
			min = 22;
			max = 36;
		}

		Play_Conversation(obj, min, max);
	
	}

	
	int Index(int Min, int Max)
	{
		while(random == last || random < Min || random > Max)
		{
			random = Commands->Get_Random_Int(0, RESISTANCE_CONV_TABLE_SIZE);
		}
		last = random;
		return last;
	}

	void Play_Conversation(GameObject * obj, int Min, int Max)
	{
		const char *conv_name = Resistance_Conv_Table[Index(Min, Max)];
		int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 0);
	}

	void Action_Complete(GameObject * /* obj */, int /* action_id */, ActionCompleteReason reason) override
	{
		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			conversation = false;
		}
	}

};

DECLARE_SCRIPT(M05_DataDisc_01_DLS, "")//
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			// Reveal Raveshaw
			bool reveal = Commands->Reveal_Encyclopedia_Character( 42 );
			if ( reveal == 1 )
			{
				Commands->Display_Encyclopedia_Event_UI();
			}
		}
	}
};
