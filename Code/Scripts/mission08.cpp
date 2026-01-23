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
*     Mission08.cpp
*
* DESCRIPTION
*     Mission 8 specific scripts
*
* PROGRAMMER
*     David Shuman
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 64 $
*     $Modtime: 2/08/02 3:20p $
*     $Archive: /Commando/Code/Scripts/mission08.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include "Mission8.h"
#include <string.h>
#include <stdio.h>

// Objective Controller
DECLARE_SCRIPT(M08_Objective_Controller, "")   // 100002
{
	
	enum {HAVOCS_SCRIPT};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
		
	}

	void Created(GameObject * obj) override
	{
		// Background Music
		Commands->Set_Background_Music ("08-Sniper.mp3");
		

		Commands->Start_Timer (obj, this, 1.0f, HAVOCS_SCRIPT);
		Commands->Enable_Hibernation(obj, false);
		
		Add_An_Objective(801);
		Add_An_Objective(808);
		Add_An_Objective(809);
	}

	void Add_An_Objective(int id)
	{
		GameObject *object;
		
		switch (id)
		{
		// Escape Prison
		case 801: 
			{
				Commands->Add_Objective(801, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M08_01, NULL, IDS_Enc_Obj_Primary_M08_01);
				object = Commands->Find_Object(108360);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(801, object);
					Commands->Set_Objective_HUD_Info_Position(801, 90.0f, "POG_M08_1_02.tga", IDS_POG_ESCAPE, Commands->Get_Position (object));
				}
			}
			break;
		// Infiltrate Research Facility
		case 802: 
			{
				Commands->Add_Objective(802, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M08_02, NULL, IDS_Enc_Obj_Primary_M08_02);
				object = Commands->Find_Object(108361);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(802, object);
					Commands->Set_Objective_HUD_Info_Position(802, 90.0f, "POG_M08_1_03.tga", IDS_POG_INFILTRATE, Commands->Get_Position (object));
				}
			}
			break;
		// Rescue Scientists
		case 803: 
			{
				Commands->Add_Objective(803, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M08_03, NULL, IDS_Enc_Obj_Primary_M08_03);
				// Now that Havoc is out of the prison area you must remove lock level 10 so that he cannot open the mutant pens
				Commands->Grant_Key (STAR, 10, false);
				object = Commands->Find_Object(108818);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(803, object);
					Commands->Set_Objective_HUD_Info_Position(803, 90.0f, "POG_M08_1_04.tga", IDS_POG_RESCUE, Commands->Get_Position (object));
				}
			}
			break;
		// Destroy Helipad
		case 804: 
			{
				Commands->Add_Objective(804, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M08_01, NULL, IDS_Enc_Obj_Secondary_M08_05);
				object = Commands->Find_Object(106339);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(804, object);
					Commands->Set_Objective_HUD_Info_Position(804, 90.0f, "POG_M08_2_01.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		// Eliminate Raveshaw
		case 805: 
			{
				Commands->Add_Objective(805, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M08_04, NULL, IDS_Enc_Obj_Primary_M08_04);
				Commands->Set_Objective_HUD_Info(805, 90.0f, "POG_M08_1_01.tga", IDS_POG_ELIMINATE);
				
			}
			break;
		// Disable Research Station Alpha
		case 806: 
			{
				Commands->Add_Objective(806, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M08_02, NULL, IDS_Enc_Obj_Secondary_M08_06);
				object = Commands->Find_Object(100016);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(806, object);
					Commands->Set_Objective_HUD_Info_Position(806, 90.0f, "POG_M08_2_02.tga", IDS_POG_DISABLE, Commands->Get_Position (object));
				}
			}
			break;
		// Disable Research Station Beta
		case 807: 
			{
				Commands->Add_Objective(807, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M08_03, NULL, IDS_Enc_Obj_Secondary_M08_07);
				object = Commands->Find_Object(100017);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(807, object);
					Commands->Set_Objective_HUD_Info_Position(807, 90.0f, "POG_M08_2_03.tga", IDS_POG_DISABLE, Commands->Get_Position (object));
				}
			}
			break;
		// Disable excavation mechanisms at archaeological dig
		case 808: 
			{
				Commands->Add_Objective(808, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M08_01, NULL, IDS_Enc_Obj_Hidden_M08_01);
			}
			break;
		// Destroy Tiberium cultivation center
		case 809: 
			{
				Commands->Add_Objective(809, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, IDS_Enc_ObjTitle_Hidden_M08_02, NULL, IDS_Enc_Obj_Hidden_M08_02);
			}
			break;
		
		}
	}

	void Remove_Pog(int id)
	{
		switch (id)
		{
		// Escape from Nod Prison Facility
		case 801: 
			{
				Commands->Set_Objective_HUD_Info(801, -1, "POG_M08_1_02.tga", IDS_POG_ESCAPE);
			}
			break;
		// Infiltrate Research Facility
		case 802: 
			{
				Commands->Set_Objective_HUD_Info(802, -1, "POG_M08_1_03.tga", IDS_POG_INFILTRATE);
			}
			break;
		// Find and rescue kidnapped scientists
		case 803: 
			{
				Commands->Set_Objective_HUD_Info(803, -1, "POG_M08_1_04.tga", IDS_POG_RESCUE);
			}
			break;
		// Destroy Nod Helipad
		case 804: 
			{
				Commands->Set_Objective_HUD_Info(804, -1, "POG_M08_2_01.tga", IDS_POG_DESTROY);
			}
			break;
		// Destroy Mutated Raveshaw
		case 805: 
			{
				Commands->Set_Objective_HUD_Info(806, -1, "POG_M08_1_02.tga", IDS_POG_DISABLE);
			}
			break;
		// Disable Research Station Alpha
		case 806: 
			{
				Commands->Set_Objective_HUD_Info(801, -1, "POG_M08_1_02.tga", IDS_POG_ESCAPE);
			}
			break;
		// Disable Research Station Beta
		case 807: 
			{
				Commands->Set_Objective_HUD_Info(807, -1, "POG_M08_1_03.tga", IDS_POG_DISABLE);
			}
			break;
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		
		switch (param)
		{
			case 1: 
				{
					Commands->Clear_Radar_Marker (type);
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_ACCOMPLISHED);
					Remove_Pog(type);
					if(type == 805)
					{
						Commands->Mission_Complete ( true );
					}
				}
				break;
			case 2:
				{
					Commands->Clear_Radar_Marker (type);
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_FAILED);
					Remove_Pog(type);
				}
				break;
			case 3: 
					Add_An_Objective(type);
				break;
			case 4: 
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_PENDING);
				break;
			case 5: 
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_HIDDEN);
				break;
		}
		if(type == 666 && param == 666)
		{
			
			Commands->Start_Timer (obj, this, 2.0f, M08_DEAD_HAVOC);
		}
		// Relocate Havoc
		if(type == M08_RELOCATE)
		{
			// Accomplish rescue the scientists mission
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 803, 1, 1.0f);
			// Relocate Havoc
			Commands->Send_Custom_Event(obj, STAR, M08_RELOCATE, 0, 0.0f);
			// Turn Havoc off of immortal
			Commands->Send_Custom_Event(obj, STAR, M08_STAR_IMMORTAL, 0, 0.0f);

		}
	}
};

//Grants initial weapons
DECLARE_SCRIPT(M08_Havoc_DLS, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
	}

	void Created(GameObject * obj) override
	{
		// Remove all of Havocs starting weapons except for the pistol
		Commands->Clear_Weapons(STAR);
		Commands->Give_PowerUp(obj, "POW_Pistol_Player", false);

		Commands->Grant_Key (obj, 10, true);
		Commands->Set_Shield_Strength(obj, 0.0f);
		// Reveal GDI Power Suit
		Commands->Reveal_Encyclopedia_Vehicle(16);
	}

	void Custom (GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if(type == M08_RELOCATE)
		{
			Commands->Set_Position (obj, Commands->Get_Position(Commands->Find_Object(108819)));
			
			// Mission objective to destroy raveshaw
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 805, 3, 2.0f);

			//
			//	Play the boss music
			//
			Commands->Set_Background_Music ("Raveshaw_Act on Instinct.mp3");

			//
			//	Create the Raveshaw Boss character
			//
			GameObject *raveshaw = Commands->Create_Object ("Raveshaw", Vector3 (-127.209F, 484.503F, -189.532F));
			if (raveshaw != NULL) {
				Commands->Set_Facing (raveshaw, 0.0F);
			}

		}
	}

	void Destroyed(GameObject * /* obj */) override
	{
//		Commands->Send_Custom_Event(obj, Commands->Find_Object(100001), 666, 666, 0.0f);
		
	}

		
};

DECLARE_SCRIPT(M08_Activate_Objective_802, "")
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

			// Nick, Those scientists you've been digging for are being held in a Nod Research Facility at the far end of the canyon. Figure'd you might want to know.\n
			const char *conv_name = ("M08_CON001");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
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
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 802, 3, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 801, 1, 3.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M08_Activate_Objective_803, "")
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

			// Nick, we're going to lose contact when you head inside, there's no radio communication possible due to jamming signals.\n
			const char *conv_name = ("M08_CON002");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300803);
			Commands->Monitor_Conversation (obj, conv_id);	
		
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100008), 100, 100, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100009), 100, 100, 0.0f);
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

		if(action_id == 300803 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 803, 3, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 802, 1, 3.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M08_Activate_Objective_804, "")
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

			// Don't know if you care, hon, but there's a Nod Helipad in back of the Prison Facility. Might help if you blow it up.\n
			const char *conv_name = ("M08_CON003");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300804);
			Commands->Monitor_Conversation (obj, conv_id);	
		
			// Disable all activate_804 zones
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100003), 100, 100, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100004), 100, 100, 0.0f);
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

		if(action_id == 300804 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 804, 3, 0.0f);
		}
		
		
	}
};

DECLARE_SCRIPT(M08_Activate_Objective_806, "")
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

			// Disable Research Station Alpha.\n
			const char *conv_name = ("M08_CON004");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300806);
			Commands->Monitor_Conversation (obj, conv_id);	

			
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300806 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 806, 3, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 807, 3, 2.0f);
		}
		
		
	}
};


DECLARE_SCRIPT(M08_Helipad, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
	}

	void Created(GameObject * /* obj */) override
	{
	
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Logical_Sound(obj, M08_HELIPAD_DESTROYED, Vector3 (0,0,0), 2500.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 804, 1, 0.0f);

		// Spawn armed prisoner for helipad area
		Commands->Enable_Spawner(100168, true);

		// Disable Apache Spawner
		Commands->Enable_Spawner(100311, false);
	}
};

DECLARE_SCRIPT(M08_Raveshaw, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
	
	}

	void Created(GameObject * /* obj */) override
	{
	
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M08_RELOCATE)
		{
			Commands->Set_Position (obj, Commands->Get_Position(Commands->Find_Object(108818)));
			// Accomplish rescue the scientists mission
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 803, 1, 1.0f);
			// Relocate Havoc
			Commands->Send_Custom_Event(obj, STAR, M08_RELOCATE, 0, 0.0f);
			
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 805, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M08_Prison_Patrol, "Waypath_ID=0:int, Waypath_Loc:Vector3")
{
	int waypath_id;
	Vector3 waypath_loc;
	bool enemy_seen;
	
	enum {WAYPATH, GO_WAYPATH, GO_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
//		SAVE_VARIABLE( waypath_loc, 2 );
		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Created (GameObject * obj) override
	{

		// No starting units can hear footsteps until otherwise alerted
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

		enemy_seen = false;

		waypath_id = Get_Int_Parameter("Waypath_ID");
		waypath_loc = Get_Vector3_Parameter("Waypath_Loc");

		Commands->Set_Innate_Soldier_Home_Location(obj, waypath_loc, 4.0f);                                                                           

		ActionParamsStruct params;
	
		if(waypath_id == 0)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
		else if(waypath_id == 1)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		else
		{
			params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == GO_STAR && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, GO_STAR);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if((Commands->Is_A_Star(enemy)) && (!enemy_seen))
		{
			enemy_seen = true;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == WAYPATH)
		{
			params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

};



DECLARE_SCRIPT(M08_Initial_Prisoner, "Reinforcment_ID=0:int")
{
	bool enemy_seen;
	int enemy_id;

	enum{GO_ENEMY, PUNCH_ENEMY};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_seen, 1 );
		SAVE_VARIABLE( enemy_id, 2 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);                                                                           
		enemy_seen = false;
		enemy_id = 0;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			enemy_seen = true;

			enemy_id = Commands->Get_ID(enemy);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_ENEMY );
			params.Set_Movement( enemy, RUN, 0.5f );
			Commands->Action_Goto( obj, params );

			Commands->Send_Custom_Event(obj, Commands->Find_Object(enemy_id), M08_DONT_MOVE, 1, 0.0f);
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		
		if(type == M08_NOD_ATTACKEE_KILLED)
		{
			enemy_seen = false;
		}
		
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{

		if ( sound.Type == M08_FREE_PRISONER )
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Grant_Key (obj, 1, true);
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if((action_id == GO_ENEMY) && (reason == ACTION_COMPLETE_NORMAL) && (enemy_seen))
		{
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, PUNCH_ENEMY);
			params.Set_Animation("S_A_HUMAN.H_A_J26C", false);
			Commands->Action_Play_Animation(obj, params);
			
		}
		if((action_id == PUNCH_ENEMY) && (reason == ACTION_COMPLETE_NORMAL))
		{
			Commands->Apply_Damage( Commands->Find_Object(enemy_id), 10000.0f, "STEEL", NULL );
			Commands->Give_PowerUp(obj, "MG Weapon 1 Clip PowerUp", false);
			Commands->Give_PowerUp(obj, "MiniGun 2 Clips PU", false);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(!enemy_seen)
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(enemy_id), M08_PRISONER_ATTACKER_KILLED, 1, 0.0f);
		}
		int reinforcement_spawner_id = Get_Int_Parameter("Reinforcment_ID");
		Commands->Enable_Spawner(reinforcement_spawner_id, true);

	}
};

DECLARE_SCRIPT(M08_Reinforcement_Prisoner, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( enemy_seen, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
	
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		
	}
};

DECLARE_SCRIPT(M08_Nod_Prison_Unit, "")
{
	int prisoner_attack_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( prisoner_attack_id, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
		prisoner_attack_id = 0;
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * sender) override
	{
		ActionParamsStruct params;

		if(type == M08_DONT_MOVE)  
		{
			prisoner_attack_id = Commands->Get_ID(sender);
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
		if(type == M08_PRISONER_ATTACKER_KILLED)  
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
			
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(prisoner_attack_id), M08_NOD_ATTACKEE_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M08_Cell_Controller, "")
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
		Commands->Debug_Message("Initialize Cell 1 Controller");
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
		poked = false;
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker) && !poked)
		{
			poked = true;
			Commands->Create_Sound ( "SFX.Ambient_Jail_Cell_Release", Vector3 (0,0,0), obj);
			Commands->Grant_Key (STAR, 1, true);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100710), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100713), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100714), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Enable_HUD_Pokable_Indicator( obj, false );

			Commands->Destroy_Object(Commands->Find_Object(109282));
			Commands->Destroy_Object(Commands->Find_Object(109283));
			Commands->Destroy_Object(Commands->Find_Object(109273));
		}
	}
};

DECLARE_SCRIPT(M08_Prison_Storage_Guard, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( enemy_seen, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Vector3 create_position = Commands->Get_Position( obj );
		Commands->Create_Object( "Level_02_Keycard", create_position );
	}
};

DECLARE_SCRIPT(M08_Sniper, "Waypath_ID=0:int, Waypath_Loc:Vector3")
{
	int waypath_id;
	Vector3 waypath_loc;

	enum{GO_WAYPATH, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
//		SAVE_VARIABLE( waypath_loc, 2 );
	}

	void Created(GameObject * obj) override
	{
		waypath_id = Get_Int_Parameter("Waypath_ID");
		waypath_loc = Get_Vector3_Parameter("Waypath_Loc");

		ActionParamsStruct params;
		if(waypath_id != 0)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		else
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

};

DECLARE_SCRIPT(M08_Nod_Gun_Emplacement, "")
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
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
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

DECLARE_SCRIPT(M08_Nod_Turret, "")
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
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
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
		ActionParamsStruct params;

		if(!attacking)
		{
			if (Commands->Get_Player_Type (damager) != SCRIPT_PLAYERTYPE_NOD)
			{
				attacking = true;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
				params.Set_Attack (damager, 250.0f, 0.0f, 1);
				params.AttackCheckBlocked = false;
				Commands->Action_Attack( obj, params );
				
				Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
			}
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

DECLARE_SCRIPT(M08_Prison_Apache, "")
{
	bool enemy_seen;
	bool flee;

	enum{WAYPATH, NEW_CLOSEST, FLEE, INITIAL};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_seen, 1 );
		SAVE_VARIABLE( flee, 2 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		enemy_seen = false;
		flee = false;

		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INITIAL );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100322;
		Commands->Action_Goto( obj, params );

	}

	int Closest_Waypath ()
	{
		Vector3 star_pos = Commands->Get_Position(STAR);
		Vector3 waypath1 = Vector3(-96.702f, 43.992f, 24.948f);
		Vector3 waypath2 = Vector3(-41.471f, 127.891f, 26.705f);
		Vector3 waypath3 = Vector3(4.254f, 77.652f, 14.441f);

		float dist_waypath1 = Commands->Get_Distance(star_pos, waypath1);
		float dist_waypath2 = Commands->Get_Distance(star_pos, waypath2);
		float dist_waypath3 = Commands->Get_Distance(star_pos, waypath3);
		
		float least_dist = WWMath::Min(WWMath::Min(dist_waypath1, dist_waypath2), dist_waypath3);

		if(least_dist == dist_waypath1)
		{
			return 100273;
		}
		else if (least_dist == dist_waypath2)
		{
			return 100287;
		}
		else 
		{
			return 100297;
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 150.0f, 0.0f, 1);
			params.WaypathID = Closest_Waypath();
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == NEW_CLOSEST && !flee)
		{
			Commands->Start_Timer (obj, this, 30.0f, NEW_CLOSEST);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = Closest_Waypath();
			Commands->Action_Attack( obj, params );
			
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == FLEE)
		{
			Commands->Destroy_Object(obj);
		}
		if(action_id == INITIAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = Closest_Waypath();
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer (obj, this, 30.0f, NEW_CLOSEST);
		}
		
		
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		ActionParamsStruct params;
		
		if ( sound.Type == M08_HELIPAD_DESTROYED )
		{
			flee = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, FLEE );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100333;
			Commands->Action_Goto( obj, params );
		}
	}

	
};

DECLARE_SCRIPT(M08_GDI_Free_Prison, "Soldier_ID=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( soldier_id, 1 );
	}


	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int soldier_id = Get_Int_Parameter("Soldier_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100246), M08_GDI_FREE_PRISON_KILLED, soldier_id, 0.0f);
	}
};

DECLARE_SCRIPT(M08_Prison_Controller, "")  //100246
{
	int gdi1;
	int gdi2;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( gdi1, 1 );
		SAVE_VARIABLE( gdi2, 2 );
	}


	void Created(GameObject * /* obj */) override
	{
		gdi1 = 0;
		gdi2 = 0;

	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_GDI_FREE_PRISON_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					gdi1++;
					if(gdi1%2 == 0)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-13.121f, 57.855f, -0.091f));
						Commands->Set_Facing(controller, 90.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X8D_CHTroopdrop1.txt");
					}
				}
				break;
			case 2:
				{
					gdi2++;
					if(gdi2%2 == 0)
					{
						GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(-43.266f, 110.230f, -0.238f));
						Commands->Set_Facing(controller, 90.000f);
						Commands->Attach_Script(controller, "Test_Cinematic", "X8D_CHTroopdrop2.txt");
					}
				}
				break;
			}

		}
		
	}
};



DECLARE_SCRIPT(M08_APC_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		apc_id = Get_Int_Parameter("APC_ID");	
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M08_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M08_PetraA21_Buggy, "")  
{
	bool attacking;
	int reinforce;
	
	enum{WAYPATH, CHECK_ENEMY};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( reinforce, 2 );
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

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 10 );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 50.0f, 0.0f, 1);
			params.WaypathID = 100290;
			params.WaypathSplined = true;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer (obj, this, 30.0f, CHECK_ENEMY);
		}
	}

	

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_CUSTOM_ACTIVATE)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100263;
			params.WaypathSplined = true;
			Commands->Action_Goto( obj, params );
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
		
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = 100282;
			params.WaypathSplined = true;
			Commands->Action_Goto( obj, params );
			
		}

	
	}

	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		
		if(timer_id == CHECK_ENEMY)
		{
			attacking = false;
		}
		
	}

};

DECLARE_SCRIPT(M08_Activate_PetraA21, "")
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

			// Custom to activate PetraA21_APC
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100262), M08_CUSTOM_ACTIVATE, 1, 0.0f);
			// Custom to activate PetraA21_Buggy
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100289), M08_CUSTOM_ACTIVATE, 1, 0.0f);

			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(109.901f, 170.915f, -7.315f));
			Commands->Set_Facing(chinook_obj1, 100.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDrop1.txt");
			
		}
	}

};

DECLARE_SCRIPT(M08_Activate_PetraA22, "")
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

			Commands->Enable_Spawner(100308, true);
			Commands->Enable_Spawner(100309, true);
			Commands->Enable_Spawner(100310, true);
		}
	}

};

DECLARE_SCRIPT(M08_PetraA22_Stealth_Tank, "")
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

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			attacking = false;
		}
		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_CUSTOM_ACTIVATE)
		{
			Commands->Enable_Enemy_Seen( obj, true);
		}
	}

};



DECLARE_SCRIPT(M08_Petra_Convoy, "")  
{
	
	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		
	}

	
	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_CUSTOM_ACTIVATE)
		{
			switch(param)
			{
			case 1:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
					params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
					params.WaypathID = 100318;
					params.WaypathSplined = true;
					Commands->Action_Goto( obj, params );
				}
				break;
			case 2:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
					params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
					params.WaypathID = 100330;
					params.WaypathSplined = true;
					Commands->Action_Goto( obj, params );
				}
				break;
			case 3:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
					params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
					params.WaypathID = 100339;
					params.WaypathSplined = true;
					Commands->Action_Goto( obj, params );
				}
				break;

			}
		}

		
	}

	
};

DECLARE_SCRIPT(M08_Activate_Convoy, "Param=0:int")
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
			int param = Get_Int_Parameter("Param");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100326), M08_CUSTOM_ACTIVATE, param, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100327), M08_CUSTOM_ACTIVATE, param, 0.5f);
		
		}
	}

	
};

			
DECLARE_SCRIPT(M08_Activate_Stealth_Trap, "")
{
		
	bool already_entered;
	int crate1_id;
	int crate2_id;
	int crate3_id;
	int crate4_id;
	int crate5_id;
	int crate6_id;
	int crate7_id;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( crate1_id, 2 );
		SAVE_VARIABLE( crate2_id, 3 );
		SAVE_VARIABLE( crate3_id, 4 );
		SAVE_VARIABLE( crate4_id, 5 );
		SAVE_VARIABLE( crate5_id, 6 );
		SAVE_VARIABLE( crate6_id, 7 );
		SAVE_VARIABLE( crate7_id, 8 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;

		crate1_id = 0;
		crate2_id = 0;
		crate3_id = 0;
		crate4_id = 0;
		crate5_id = 0;
		crate6_id = 0;
		crate7_id = 0;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;
			
			// Front
			Commands->Create_Explosion( "Ground Explosions Twiddler", Vector3(224.119f, 218.876f, -13.746f), obj );
			Commands->Create_Explosion( "Ground Explosions Twiddler", Vector3(222.456f, 202.921f, -15.889f), obj );

			GameObject *crate1 = Commands->Create_Object("M08_Rubble_Stub", Vector3(224.119f, 218.876f, -13.746f));
			Commands->Set_Facing( crate1, 165.000f );
			crate1_id = Commands->Get_ID(crate1);

			GameObject *crate2 = Commands->Create_Object("M08_Rubble_Stub", Vector3(223.517f, 217.013f, -10.574f));
			Commands->Set_Facing( crate2, 165.000f );
			crate2_id = Commands->Get_ID(crate2);

			GameObject *crate3 = Commands->Create_Object("M08_Rubble_Stub_Destroyable", Vector3(221.917f, 212.589f, -15.192f));
			Commands->Set_Facing( crate3, 0.000f );
			crate3_id = Commands->Get_ID(crate3);

			GameObject *crate4 = Commands->Create_Object("M08_Rubble_Stub", Vector3(222.138f, 207.974f, -12.258f));
			Commands->Set_Facing( crate4, 0.000f );
			crate4_id = Commands->Get_ID(crate4);

			GameObject *crate5 = Commands->Create_Object("M08_Rubble_Stub", Vector3(221.850f, 196.753f, -15.421f));
			Commands->Set_Facing( crate5, 0.000f );
			crate5_id = Commands->Get_ID(crate5);

			GameObject *crate6 = Commands->Create_Object("M08_Rubble_Stub", Vector3(221.944f, 198.134f, -12.500f));
			Commands->Set_Facing( crate6, 0.000f );
			crate6_id = Commands->Get_ID(crate6);

			GameObject *crate7 = Commands->Create_Object("M08_Rubble_Stub", Vector3(222.456f, 202.921f, -15.889f));
			Commands->Set_Facing( crate7, 0.000f );
			crate7_id = Commands->Get_ID(crate7);


			// Rear
			Commands->Create_Explosion( "Generic Ground 01", Vector3(106.852f, 221.987f, -3.990f), obj );
			Commands->Create_Explosion( "Generic Ground 01", Vector3(89.681f, 218.165f, -8.951f), obj );

			GameObject *crate8 = Commands->Create_Object("M08_Rubble_Stub", Vector3(106.852f, 221.987f, -3.990f));
			Commands->Set_Facing( crate8, 105.000f );

			GameObject *crate9 = Commands->Create_Object("M08_Rubble_Stub", Vector3(99.245f, 220.195f, -5.858f));
			Commands->Set_Facing( crate9, 105.000f );

			GameObject *crate10 = Commands->Create_Object("M08_Rubble_Stub", Vector3(97.595f, 219.985f, -9.097f));
			Commands->Set_Facing( crate10, -75.000f );

			GameObject *crate11 = Commands->Create_Object("M08_Rubble_Stub", Vector3(89.813f, 217.622f, -5.894f));
			Commands->Set_Facing( crate11, 105.000f );

			GameObject *crate12 = Commands->Create_Object("M08_Rubble_Stub", Vector3(89.681f, 218.165f, -8.951f));
			Commands->Set_Facing( crate12, -80.000f );
			
			// Activate initial stealth tank
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100347), M08_CUSTOM_ACTIVATE, 1, 0.0f);

			//Enable stealth soldier spawners
			Commands->Enable_Spawner(100348, true);
			Commands->Enable_Spawner(100349, true);
			Commands->Enable_Spawner(100351, true);

			// Enable stealth tank at rear of trap
			Commands->Enable_Spawner(100354, true);

			// Enable stealth soldier at rear of trap
			Commands->Enable_Spawner(100351, true);

			// Enable damaged and emtpy stealth tank
			Commands->Enable_Spawner(100359, true);
		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_CUSTOM_ACTIVATE)
		{
			Commands->Destroy_Object(Commands->Find_Object(crate1_id));
			Commands->Destroy_Object(Commands->Find_Object(crate2_id));
			Commands->Destroy_Object(Commands->Find_Object(crate3_id));
			Commands->Destroy_Object(Commands->Find_Object(crate4_id));
			Commands->Destroy_Object(Commands->Find_Object(crate5_id));
			Commands->Destroy_Object(Commands->Find_Object(crate6_id));
			Commands->Destroy_Object(Commands->Find_Object(crate7_id));
		}
	};
};

DECLARE_SCRIPT(M08_Destroy_Stealth_Trap, "")
{
		
	bool already_entered;
	int stealth_tank_id;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( stealth_tank_id, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		stealth_tank_id = 0;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Get_ID(enterer) == stealth_tank_id)
		{
			already_entered = true;
			
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_CUSTOM_ACTIVATE, 1, 0.0f);

		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_UNIT_ID)
		{
			stealth_tank_id = param;
		}
	};

	
};

DECLARE_SCRIPT(M08_Player_Stealth_Tank, "")
{
		
	bool already_entered;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100360), M08_UNIT_ID, Commands->Get_ID(obj), 0.0f);
	}

	
};

DECLARE_SCRIPT(M08_Nod_Light_Tank, "")
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

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			attacking = false;
		}
		
	}

};

DECLARE_SCRIPT(M08_Nod_Stealth_Tank, "")
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

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 15.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), 1);
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			Commands->Action_Attack (obj, params);

			attacking = false;
		}
		
	}

};

DECLARE_SCRIPT(M08_Archaelogical_Site_Patrol, "Waypath_ID=0:int, Waypath_Loc:Vector3, Unit_ID=0:int")
{
	int waypath_id;
	Vector3 waypath_loc;
	int unit_id;
	
	enum {WAYPATH, GO_WAYPATH, INITIAL};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
//		SAVE_VARIABLE( waypath_loc, 2 );
	}

	void Created (GameObject * obj) override
	{
		waypath_id = Get_Int_Parameter("Waypath_ID");
		waypath_loc = Get_Vector3_Parameter("Waypath_Loc");
		unit_id = Get_Int_Parameter("Unit_ID");

		if(waypath_id != 0)
		{
			Commands->Start_Timer(obj, this, 3.0f, INITIAL);
		}
		else
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == WAYPATH)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}

		if(timer_id == INITIAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
			
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100362), M08_ARCHAELOGICAL_KILLED, unit_id, 0.0f);

	}

};

DECLARE_SCRIPT(M08_Archaelogical_Reinforcements, "Unit_ID=0:int")
{
	int unit_id;

	enum{INITIAL};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 3.0f, INITIAL);
		unit_id = Get_Int_Parameter("Unit_ID");

		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == INITIAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 10 );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
			
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100362), M08_ARCHAELOGICAL_KILLED, unit_id, 0.0f);

	}
};

DECLARE_SCRIPT(M08_Archaelogical_Site_Controller, "")
{
		
	bool already_entered;
	int unit1_killed;
	int unit2_killed;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( unit1_killed, 2 );
		SAVE_VARIABLE( unit2_killed, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		unit1_killed = 0;
		unit2_killed = 0;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_ARCHAELOGICAL_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					unit1_killed++;
					if(unit1_killed%2 == 0)
					{
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(280.143f, 256.055f, -18.745f));
						Commands->Set_Facing(chinook_obj1, 60.000f);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDrop2.txt");
					}
				}
				break;
			case 2:
				{
					unit2_killed++;
					if(unit2_killed%2 == 0)
					{
						GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(270.498f, 394.061f, -12.328f));
						Commands->Set_Facing(chinook_obj2, 60.000f);
						Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X8I_TroopDrop3.txt");
					}
				}
				break;
			}

		}
	}
};

DECLARE_SCRIPT(M08_Archaelogical_Site_Buggy, "")
{
	bool attacking;

	enum{WAYPATH, ATTACK_OVER};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.Set_Attack (NULL, 250.0f, 0.0f, 1);
		params.WaypathID = 100406;
		Commands->Action_Attack( obj, params );

		attacking = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 100406;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 10.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			params.WaypathID = 100406;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
			
		}
		
	}
	
};

DECLARE_SCRIPT(M08_Archaelogical_Site_Tomb, "")
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

			Commands->Enable_Spawner(100423, true);
			Commands->Enable_Spawner(100424, true);
		}
	}

};
		
DECLARE_SCRIPT(M08_PetraA25_Controller, "")
{
	int unit1_killed;
	int unit2_killed;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( unit1_killed, 1 );
		SAVE_VARIABLE( unit2_killed, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		unit1_killed = 0;
		unit2_killed = 0;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_PETRAA25_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					unit1_killed++;
					if(unit1_killed < 4)
					{
						GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(161.429f, 419.470f, -11.413f));
						Commands->Set_Facing(chinook_obj2, 60.000f);
						Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "M08_XG_VehicleDrop1.txt");
					}
				}
				break;
			case 2:
				{
					unit2_killed++;
					if(unit2_killed%2 == 0)
					{
						
					}
				}
				break;
			}

		}
	}
};

DECLARE_SCRIPT(M08_PetraA25_Tank, "")
{
	bool attacking;

	enum{WAYPATH, ATTACK_OVER, INITIAL};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		Commands->Start_Timer (obj, this, 4.0f, INITIAL);
		

		attacking = true;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 0.0f, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 100437;
			params.WaypathSplined = true;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 10.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == INITIAL)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			params.WaypathID = 100437;
			params.WaypathSplined = true;
			Commands->Action_Attack( obj, params );
		}

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			params.WaypathID = 100437;
			params.WaypathSplined = true;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
			
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100436), M08_PETRAA25_KILLED, 1, 0.0f);

	}
	
};

DECLARE_SCRIPT(M08_PetraA25_Patrol, "Waypath_ID=0:int, Waypath_Loc:Vector3")
{
	int waypath_id;
	Vector3 waypath_loc;
	
	enum {WAYPATH, GO_WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( waypath_id, 1 );
		SAVE_VARIABLE( waypath_loc, 2 );
	}

	void Created (GameObject * obj) override
	{
		waypath_id = Get_Int_Parameter("Waypath_ID");
		waypath_loc = Get_Vector3_Parameter("Waypath_Loc");

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_WAYPATH );
		params.Set_Movement( waypath_loc, RUN, 1.5f );
		Commands->Action_Goto( obj, params );

		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == WAYPATH)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

};

DECLARE_SCRIPT(M08_Excavation_Mechanism, "")
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 808, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Tiberium_Cultivation_MCT, "")
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "mct_nod.mct_nod", 0);
	}
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Explosion_Mine_Remote_01", Commands->Get_Position(obj), NULL); 

		Vector3 myPositon = Commands->Get_Position ( obj );
		float myFacing  = Commands->Get_Facing ( obj );

		GameObject * destroyedMCT = Commands->Create_Object ( "Nod_MCT_Alarm_Destroyed", myPositon);
		Commands->Set_Facing ( destroyedMCT, myFacing );

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 809, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Research_Station_A, "")
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "mct_nod.mct_nod", 0);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Explosion_Mine_Remote_01", Commands->Get_Position(obj), NULL); 

		Vector3 myPositon = Commands->Get_Position ( obj );
		float myFacing  = Commands->Get_Facing ( obj );

		GameObject * destroyedMCT = Commands->Create_Object ( "Nod_MCT_Alarm_Destroyed", myPositon);
		Commands->Set_Facing ( destroyedMCT, myFacing );

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 806, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Research_Station_B, "")
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "mct_nod.mct_nod", 0);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Explosion_Mine_Remote_01", Commands->Get_Position(obj), NULL); 

		Vector3 myPositon = Commands->Get_Position ( obj );
		float myFacing  = Commands->Get_Facing ( obj );

		GameObject * destroyedMCT = Commands->Create_Object ( "Nod_MCT_Alarm_Destroyed", myPositon);
		Commands->Set_Facing ( destroyedMCT, myFacing );

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100002), 807, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Facility_Patrol, "Waypath_ID=0:int, Waypath_Loc:Vector3")
{
	int waypath_id;
	Vector3 waypath_loc;
	bool enemy_seen;
	
	enum {WAYPATH, GO_WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( waypath_id, 1 );
		SAVE_VARIABLE( waypath_loc, 2 );
		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Created (GameObject * obj) override
	{
		enemy_seen = false;

		// No starting units can hear footsteps until otherwise alerted
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

		waypath_id = Get_Int_Parameter("Waypath_ID");
		waypath_loc = Get_Vector3_Parameter("Waypath_Loc");

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_WAYPATH );
		params.Set_Movement( waypath_loc, RUN, 1.5f );
		Commands->Action_Goto( obj, params );

		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason != ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer(obj, this, 30.0f, WAYPATH);
			return;
		}
		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == WAYPATH)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if((Commands->Is_A_Star(enemy)) && (!enemy_seen))
		{
			enemy_seen = true;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		}
	}

};

DECLARE_SCRIPT(M08_Facility_Go_Point, "Point1_ID=0:int, Point2_ID=0:int")
{
	int point1_id;
	int point2_id;
	bool enemy_seen;
	
	enum {GO_POINT1, GO_POINT2};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( point1_id, 1 );
		SAVE_VARIABLE( point2_id, 2 );
		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Created (GameObject * obj) override
	{
		enemy_seen = false;

		// No starting units can hear footsteps until otherwise alerted
		Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

		point1_id = Get_Int_Parameter("Point1_ID");
		point2_id = Get_Int_Parameter("Point2_ID");

      	ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_POINT1 );
		params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
		Commands->Action_Goto( obj, params );

		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason != ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer(obj, this, 30.0f, GO_POINT1);
			return;
		}
		if(action_id == GO_POINT1 && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_POINT2 );
			params.Set_Movement( Commands->Find_Object(point2_id), WALK, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_POINT2 && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_POINT1 );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_POINT1)
		{
			params.Set_Basic( this, INNATE_PRIORITY_FOOTSTEPS_HEARD - 5, GO_POINT1 );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if((Commands->Is_A_Star(enemy)) && (!enemy_seen))
		{
			enemy_seen = true;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		}
	}

};

DECLARE_SCRIPT(M08_Facility_Tank, "Waypath_ID=0:int")
{
	bool attacking;
	int waypath_id;

	enum{WAYPATH, ATTACK_OVER, INITIAL};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
//		SAVE_VARIABLE( waypath_id, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		Commands->Start_Timer (obj, this, 4.0f, INITIAL);
		
		waypath_id = Get_Int_Parameter("Waypath_ID");

		attacking = true;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 0.0f, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = waypath_id;
			params.WaypathSplined = true;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 10.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == INITIAL)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			params.WaypathID = waypath_id;
			params.WaypathSplined = true;
			Commands->Action_Attack( obj, params );
		}

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (NULL, 250.0f, 0.0f, 1);
			params.WaypathID = waypath_id;
			params.WaypathSplined = true;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
			
		}
		
	}

};

DECLARE_SCRIPT(M08_Facility_Vehicle_Dec, "")
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
		
		attacking = true;
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

DECLARE_SCRIPT(M08_Activate_Encounter, "Spawner_ID1=0:int, Spawner_ID2=0:int, Spawner_ID3=0:int, Spawner_ID4=0:int, Spawner_ID5=0:int, Spawner_ID6=0:int")
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
			int spawner_id4 = Get_Int_Parameter("Spawner_ID4");
			int spawner_id5 = Get_Int_Parameter("Spawner_ID5");
			int spawner_id6 = Get_Int_Parameter("Spawner_ID6");
			
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
			if(spawner_id4 != 0)
			{
				Commands->Enable_Spawner(spawner_id4, true);
			}
			if(spawner_id5 != 0)
			{
				Commands->Enable_Spawner(spawner_id5, true);
			}
			if(spawner_id6 != 0)
			{
				Commands->Enable_Spawner(spawner_id6, true);
			}
			

		}
	}

	void Custom(GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == M08_DEACTIVATE_ENCOUNTER)
		{
			int spawner_id1 = Get_Int_Parameter("Spawner_ID1");
			int spawner_id2 = Get_Int_Parameter("Spawner_ID2");
			int spawner_id3 = Get_Int_Parameter("Spawner_ID3");
			int spawner_id4 = Get_Int_Parameter("Spawner_ID4");
			int spawner_id5 = Get_Int_Parameter("Spawner_ID5");
			int spawner_id6 = Get_Int_Parameter("Spawner_ID6");
			
			if(spawner_id1 != 0)
			{
				Commands->Enable_Spawner(spawner_id1, false);
			}
			if(spawner_id2 != 0)
			{
				Commands->Enable_Spawner(spawner_id2, false);
			}
			if(spawner_id3 != 0)
			{
				Commands->Enable_Spawner(spawner_id3, false);
			}
			if(spawner_id4 != 0)
			{
				Commands->Enable_Spawner(spawner_id4, false);
			}
			if(spawner_id5 != 0)
			{
				Commands->Enable_Spawner(spawner_id5, false);
			}
			if(spawner_id6 != 0)
			{
				Commands->Enable_Spawner(spawner_id6, false);
			}
		}
	}

};

DECLARE_SCRIPT(M08_Deactivate_Encounter, "Activate_Zone=0:int")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Created (GameObject * /* obj */) override
	{
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			int activate_zone = Get_Int_Parameter("Activate_Zone");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(activate_zone), M08_DEACTIVATE_ENCOUNTER, 1, 0.0f);

		}
	}

};



DECLARE_SCRIPT(M08_Encounter_Unit, "Waypath_ID=0:int, Priority=0:int, Suicide=0:int, Stationary_at_End=0:int")
{
	int waypath_id;
	int priority;
	bool suicide;
	bool stationary;

	enum{DIE_SURPRISE, GO_STAR, WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		waypath_id = Get_Int_Parameter("Waypath_ID");
		priority = Get_Int_Parameter("Priority");
		suicide = (Get_Int_Parameter("Suicide") == 1) ? true : false;
		stationary = (Get_Int_Parameter("Stationary_at_End") == 1) ? true : false;

		if(waypath_id == 0)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
		else if(waypath_id == 1)
		{
			params.Set_Basic( this, priority, GO_STAR );
			params.Set_Movement( STAR, RUN, 2.0f );
			Commands->Action_Goto (  obj, params );
		}
		else
		{
			params.Set_Basic( this, priority, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		
		if(suicide)
		{
			Commands->Start_Timer (obj, this, 15.0f, DIE_SURPRISE);
		}
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

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (reason != ACTION_COMPLETE_NORMAL) 
		{
			return;
		}
		if (action_id == WAYPATH)
		{
			Commands->Innate_Force_State_Enemy_Seen(obj, STAR);
			if(stationary)
			{
				Commands->Set_Innate_Is_Stationary(obj, true);
			}
		}
	}
};

DECLARE_SCRIPT(M08_Warden_Announcement1, "")
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
			
			// This is the warden, we have a probable escape situation in solitary. Full lockdown, all personnel initiate response protocols.\n
			const char *conv_name = ("M08_CON006");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300502);
			Commands->Monitor_Conversation (obj, conv_id);	

		}
	}

};

DECLARE_SCRIPT(M08_Warden_Announcement2, "")
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
			
			// This is the warden, we have a confirmed prison break, repeat, confirmed prison break! Assume all GDI as hostile, kill every last one of them!\n
			const char *conv_name = ("M08_CON007");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300502);
			Commands->Monitor_Conversation (obj, conv_id);	

		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == 300502)
		{
			Commands->Create_Sound ( "M06_Alarm", Vector3 (0,0,0), obj);
		}
		
		
	}

};

DECLARE_SCRIPT(M08_Innate_Control, "")
{
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Innate_Disable(obj);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON)
		{
			Commands->Innate_Enable(obj);
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		}
		
	}
};

DECLARE_SCRIPT(M08_Activate_Innate, "Unit_ID1=0:int, Unit_ID2=0:int, Unit_ID3=0:int, Unit_ID4=0:int, Unit_ID5=0:int, ")
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
			
			int unit_id1 = Get_Int_Parameter("Unit_ID1");
			int unit_id2 = Get_Int_Parameter("Unit_ID2");
			int unit_id3 = Get_Int_Parameter("Unit_ID3");
			int unit_id4 = Get_Int_Parameter("Unit_ID4");
			int unit_id5 = Get_Int_Parameter("Unit_ID5");

			if(unit_id1 != 0)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(unit_id1), M08_INNATE_ON, 1, 0.0f);
			}
			if(unit_id2 != 0)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(unit_id2), M08_INNATE_ON, 1, 0.0f);
			}
			if(unit_id3 != 0)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(unit_id3), M08_INNATE_ON, 1, 0.0f);
			}
			if(unit_id4 != 0)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(unit_id4), M08_INNATE_ON, 1, 0.0f);
			}
			if(unit_id5 != 0)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(unit_id5), M08_INNATE_ON, 1, 0.0f);
			}
			

		}
	}

};

DECLARE_SCRIPT(M08_Basketball_Court_Controller, "")
{
	int m08_basketball_gun_emp_killed;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m08_basketball_gun_emp_killed, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		m08_basketball_gun_emp_killed = 0;

		// Nod Soldiers
//		Commands->Enable_Spawner(100205, true);
//		Commands->Enable_Spawner(100206, true);
		Commands->Enable_Spawner(100207, true);
		Commands->Enable_Spawner(100208, true);

		
	}

	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_BASKETBALL_GUN_EMP_KILLED)
		{
			m08_basketball_gun_emp_killed++;
			if(m08_basketball_gun_emp_killed == 1)
			{
				// GDI Prisoner
				Commands->Enable_Spawner(100201, true);
				Commands->Enable_Spawner(100202, true);
				

				// Nod Soldiers
		//		Commands->Enable_Spawner(100205, false);
		//		Commands->Enable_Spawner(100206, false);
			}
			else
			{
				// GDI Prisoner
		//		Commands->Enable_Spawner(100203, true);
		//		Commands->Enable_Spawner(100204, true);

				// Nod Soldiers
		//		Commands->Enable_Spawner(100207, false);
			}
		}
		
	}
};

DECLARE_SCRIPT(M08_Basketball_Gun_Emp, "")
{
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100197), M08_BASKETBALL_GUN_EMP_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M08_Homepoint, "Homepoint_ID=0:int")
{
	void Created (GameObject * obj) override
	{
		int homepoint_id = Get_Int_Parameter("Homepoint_ID");
		Vector3 homepoint_loc = Commands->Get_Position(Commands->Find_Object(homepoint_id));
		Commands->Set_Innate_Soldier_Home_Location(obj, homepoint_loc, 4.0f);    	
	}
};

DECLARE_SCRIPT(M08_Nod_Buggy_Dec, "")
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

DECLARE_SCRIPT(M08_Petra_A_Controller, "")  //100346
{
	int m08_petra_a1_unit;
	int m08_petra_a2_unit;
	int m08_petra_a3_unit;
	int a1_last_loc;
	int a2_last_loc;
	int a3_last_loc;
	bool petra_a_active;
	bool created_vehicle;
	int player_vehicle_id;
		
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
		SAVE_VARIABLE( m08_petra_a2_unit, 2 );
		SAVE_VARIABLE( m08_petra_a3_unit, 3 );
		SAVE_VARIABLE( a1_last_loc, 4 );
		SAVE_VARIABLE( a2_last_loc, 5 );
		SAVE_VARIABLE( a3_last_loc, 6 );
		SAVE_VARIABLE( petra_a_active, 7 );
		SAVE_VARIABLE( created_vehicle, 8 );
		SAVE_VARIABLE( player_vehicle_id, 9 );
	}

	void Created(GameObject * /* obj */) override
	{
		m08_petra_a1_unit = 0;
		m08_petra_a2_unit = 0;
		m08_petra_a3_unit = 0;

		a1_last_loc = 111522;
		a2_last_loc = 111524;
		a3_last_loc = 111526;

		petra_a_active = false;
		created_vehicle = false;
		player_vehicle_id = 0;
		
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if(type == M08_PETRA_A_UNIT_STATUS)
		{
			Commands->Send_Custom_Event(obj, sender, M08_PETRA_A_UNIT_STATUS, petra_a_active, 0.0f);
		}
		if(type == M08_PETRA_A_ACTIVATE && !petra_a_active)
		{
			petra_a_active = true;
			Commands->Create_Logical_Sound(obj, M08_PETRA_A_LOGICAL, Vector3 (0,0,0), 2500.0f);

			// Custom to bring in helo troop drop
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_HELO_KILLED, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_HELO_KILLED, 2, 1.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_HELO_KILLED, 3, 2.0f);

		}
		if(type == M08_PETRA_A_DEACTIVATE && petra_a_active)
		{
			petra_a_active = false;

			Commands->Create_Logical_Sound(obj, M08_PETRA_A_LOGICAL, Vector3 (0,0,0), 2500.0f);
		}
		if(type == M08_PETRA_A_CREATED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_a1_unit++;
				}
				break;
			case 2:
				{
					m08_petra_a2_unit++;
				}
				break;
			case 3:
				{
					m08_petra_a3_unit++;
				}
				break;
			}
		}

		if(type == M08_PETRA_A_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_a1_unit--;
				}
				break;
			case 2:
				{
					m08_petra_a2_unit--;
				}
				break;
			case 3:
				{
					m08_petra_a3_unit--;
				}
				break;
			}
		}

		if((type == M08_PETRA_A_KILLED || type == M08_PETRA_A_HELO_KILLED) && petra_a_active)
		{
			

			switch(param)
			{
			case 1:
				{
					if(m08_petra_a1_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111523);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(a1_last_loc == 111522)
						{
							drop_loc = Commands->Find_Object(111523);
							drop_facing = Commands->Get_Facing(drop_loc);
							a1_last_loc = 111523;
						}
						else if(a1_last_loc == 111523)
						{
							drop_loc = Commands->Find_Object(111522);
							drop_facing = Commands->Get_Facing(drop_loc);
							a1_last_loc = 111522;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropA1.txt");
					}
				}
				break;
			case 2:
				{
					if(m08_petra_a2_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111524);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(a2_last_loc == 111524)
						{
							drop_loc = Commands->Find_Object(111525);
							drop_facing = Commands->Get_Facing(drop_loc);
							a2_last_loc = 111525;
						}
						else if(a2_last_loc == 111525)
						{
							drop_loc = Commands->Find_Object(111524);
							drop_facing = Commands->Get_Facing(drop_loc);
							a2_last_loc = 111524;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropA2.txt");
					}
				}
				break;
			case 3:
				{
					if(m08_petra_a3_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111526);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(a3_last_loc == 111526)
						{
							drop_loc = Commands->Find_Object(111527);
							drop_facing = Commands->Get_Facing(drop_loc);
							a3_last_loc = 111527;
						}
						else if(a3_last_loc == 111527)
						{
							drop_loc = Commands->Find_Object(111522);
							drop_facing = Commands->Get_Facing(drop_loc);
							a3_last_loc = 111526;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropA3.txt");
					}
				}
				break;
			}
		}
		
	}

};

DECLARE_SCRIPT(M08_Petra_A_Unit, "Unit_ID=0:int")
{
	
	enum{GO_STAR, GO_PETRA_A};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_CREATED, unit_id, 0.0f);

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_UNIT_STATUS, unit_id, 0.0f);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );

			Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_PETRA_A_UNIT_STATUS)
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);

			bool petra_a_active = (param == 1) ? true : false;

			if(petra_a_active)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
				params.Set_Movement( STAR, RUN, 1.5f );
				Commands->Action_Goto( obj, params );

				Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
			}
			else
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_PETRA_A );
				params.Set_Movement( Commands->Find_Object(100346), RUN, 8.5f );
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		ActionParamsStruct params;
		
		if ( sound.Type == M08_PETRA_A_LOGICAL )
		{
			int unit_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_UNIT_STATUS, unit_id, 0.0f);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_PETRA_A && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL );
		}
		
		
	}


	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Petra_A_Helo, "Unit_ID=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_HELO_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Activate_Petra_A, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_ACTIVATE, 1, 0.0f);
			
		}
	}

	
};

DECLARE_SCRIPT(M08_Deactivate_Petra_A, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100346), M08_PETRA_A_DEACTIVATE, 1, 0.0f);
			
		}
	}

	
};

// Petra B

DECLARE_SCRIPT(M08_Petra_B_Controller, "")  //100346
{
	int m08_petra_b1_unit;
	int m08_petra_b2_unit;
	int m08_petra_b3_unit;
	int b1_last_loc;
	int b2_last_loc;
	int b3_last_loc;
	bool petra_b_active;
	bool created_vehicle;
	int player_vehicle_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m08_petra_b1_unit, 1 );
		SAVE_VARIABLE( m08_petra_b2_unit, 2 );
		SAVE_VARIABLE( m08_petra_b3_unit, 3 );
		SAVE_VARIABLE( b1_last_loc, 4 );
		SAVE_VARIABLE( b2_last_loc, 5 );
		SAVE_VARIABLE( b3_last_loc, 6 );
		SAVE_VARIABLE( petra_b_active, 7 );
		SAVE_VARIABLE( created_vehicle, 8 );
		SAVE_VARIABLE( player_vehicle_id, 9 );
	}

	void Created(GameObject * /* obj */) override
	{
		m08_petra_b1_unit = 0;
		m08_petra_b2_unit = 0;
		m08_petra_b3_unit = 0;

		b1_last_loc = 111753;
		b2_last_loc = 111756;
		b3_last_loc = 111757;

		petra_b_active = false;
		created_vehicle = false;
		player_vehicle_id = 0;

	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if(type == M08_PLAYER_VEHICLE_ID)
		{
			player_vehicle_id = param;
		}
		if(type == M08_PETRA_B_UNIT_STATUS)
		{
			Commands->Send_Custom_Event(obj, sender, M08_PETRA_B_UNIT_STATUS, petra_b_active, 0.0f);
		}
		if(type == M08_PETRA_B_ACTIVATE && !petra_b_active)
		{
			petra_b_active = true;
			Commands->Create_Logical_Sound(obj, M08_PETRA_B_LOGICAL, Vector3 (0,0,0), 2500.0f);

			// Custom to bring in helo troop drop
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_HELO_KILLED, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_HELO_KILLED, 2, 1.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_HELO_KILLED, 3, 2.0f);

			// Check if player has a vehicle, if not, create one
			if(!Commands->Find_Object(109047) && !created_vehicle)
			{
				created_vehicle = true;
				GameObject * player_vehicle = Commands->Create_Object("Nod_Stealth_Tank_Player", Vector3(145.898f, 269.404f, -14.918f));
				Commands->Set_Facing(player_vehicle, 150.000f);
				// Send the player vehicle id to Petra_C
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PLAYER_VEHICLE_ID, Commands->Get_ID(player_vehicle), 0.0f);
			}

		}
		if(type == M08_PETRA_B_DEACTIVATE && petra_b_active)
		{
			petra_b_active = false;

			Commands->Create_Logical_Sound(obj, M08_PETRA_B_LOGICAL, Vector3 (0,0,0), 2500.0f);
		}
		if(type == M08_PETRA_B_CREATED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_b1_unit++;
				}
				break;
			case 2:
				{
					m08_petra_b2_unit++;
				}
				break;
			case 3:
				{
					m08_petra_b3_unit++;
				}
				break;
			}
		}

		if(type == M08_PETRA_B_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_b1_unit--;
				}
				break;
			case 2:
				{
					m08_petra_b2_unit--;
				}
				break;
			case 3:
				{
					m08_petra_b3_unit--;
				}
				break;
			}
		}

		if((type == M08_PETRA_B_KILLED || type == M08_PETRA_B_HELO_KILLED) && petra_b_active)
		{
			

			switch(param)
			{
			case 1:
				{
					if(m08_petra_b1_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111753);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(b1_last_loc == 111753)
						{
							drop_loc = Commands->Find_Object(111754);
							drop_facing = Commands->Get_Facing(drop_loc);
							b1_last_loc = 111754;
						}
						else if(b1_last_loc == 111754)
						{
							drop_loc = Commands->Find_Object(111753);
							drop_facing = Commands->Get_Facing(drop_loc);
							b1_last_loc = 111753;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropB1.txt");
					}
				}
				break;
			case 2:
				{
					if(m08_petra_b2_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111755);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(b2_last_loc == 111755)
						{
							drop_loc = Commands->Find_Object(111756);
							drop_facing = Commands->Get_Facing(drop_loc);
							b2_last_loc = 111756;
						}
						else if(b2_last_loc == 111756)
						{
							drop_loc = Commands->Find_Object(111755);
							drop_facing = Commands->Get_Facing(drop_loc);
							b2_last_loc = 111755;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropB2.txt");
					}
				}
				break;
			case 3:
				{
					if(m08_petra_b3_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111757);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(b3_last_loc == 111757)
						{
							drop_loc = Commands->Find_Object(111758);
							drop_facing = Commands->Get_Facing(drop_loc);
							b3_last_loc = 111758;
						}
						else if(b3_last_loc == 111758)
						{
							drop_loc = Commands->Find_Object(111757);
							drop_facing = Commands->Get_Facing(drop_loc);
							b3_last_loc = 111757;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropB3.txt");
					}
				}
				break;
			}
		}
		
	}

};

DECLARE_SCRIPT(M08_Petra_B_Unit, "Unit_ID=0:int")
{
	
	enum{GO_STAR, GO_PETRA_B};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_CREATED, unit_id, 0.0f);

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_UNIT_STATUS, unit_id, 0.0f);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );

			Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_PETRA_B_UNIT_STATUS)
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);

			bool petra_b_active = (param == 1) ? true : false;

			if(petra_b_active)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
				params.Set_Movement( STAR, RUN, 1.5f );
				Commands->Action_Goto( obj, params );

				Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
			}
			else
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_PETRA_B );
				params.Set_Movement( Commands->Find_Object(100370), RUN, 8.5f );
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		ActionParamsStruct params;
		
		if ( sound.Type == M08_PETRA_B_LOGICAL )
		{
			int unit_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_UNIT_STATUS, unit_id, 0.0f);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_PETRA_B && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL );
		}
		
		
	}


	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Petra_B_Helo, "Unit_ID=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_HELO_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Activate_Petra_B, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_ACTIVATE, 1, 0.0f);
			
		}
	}

};

DECLARE_SCRIPT(M08_Deactivate_Petra_B, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100370), M08_PETRA_B_DEACTIVATE, 1, 0.0f);
			
		}
	}

	
};

// Petra C

DECLARE_SCRIPT(M08_Petra_C_Controller, "")  //100389
{
	int m08_petra_c1_unit;
	int m08_petra_c2_unit;
	int m08_petra_c3_unit;
	int c1_last_loc;
	int c2_last_loc;
	int c3_last_loc;
	bool petra_c_active;
	bool created_vehicle;
	int player_vehicle_id;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m08_petra_c1_unit, 1 );
		SAVE_VARIABLE( m08_petra_c2_unit, 2 );
		SAVE_VARIABLE( m08_petra_c3_unit, 3 );
		SAVE_VARIABLE( c1_last_loc, 4 );
		SAVE_VARIABLE( c2_last_loc, 5 );
		SAVE_VARIABLE( c3_last_loc, 6 );
		SAVE_VARIABLE( petra_c_active, 7 );
		SAVE_VARIABLE( created_vehicle, 8 );
		SAVE_VARIABLE( player_vehicle_id, 9 );
	}

	void Created(GameObject * /* obj */) override
	{
		m08_petra_c1_unit = 0;
		m08_petra_c2_unit = 0;
		m08_petra_c3_unit = 0;

		c1_last_loc = 111759;
		c2_last_loc = 111761;
		c3_last_loc = 111763;

		petra_c_active = false;

		created_vehicle = false;
		player_vehicle_id = 0;
		
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * sender) override
	{
		if(type == M08_PLAYER_VEHICLE_ID)
		{
			player_vehicle_id = param;
		}
		if(type == M08_PETRA_C_UNIT_STATUS)
		{
			Commands->Send_Custom_Event(obj, sender, M08_PETRA_C_UNIT_STATUS, petra_c_active, 0.0f);
		}
		if(type == M08_PETRA_C_ACTIVATE && !petra_c_active)
		{
			petra_c_active = true;
			Commands->Create_Logical_Sound(obj, M08_PETRA_C_LOGICAL, Vector3 (0,0,0), 2500.0f);

			// Custom to bring in helo troop drop
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_HELO_KILLED, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_HELO_KILLED, 2, 1.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_HELO_KILLED, 3, 2.0f);

		}
		if(type == M08_PETRA_C_DEACTIVATE && petra_c_active)
		{
			petra_c_active = false;

			Commands->Create_Logical_Sound(obj, M08_PETRA_C_LOGICAL, Vector3 (0,0,0), 2500.0f);
		}
		if(type == M08_PETRA_C_CREATED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_c1_unit++;
				}
				break;
			case 2:
				{
					m08_petra_c2_unit++;
				}
				break;
			case 3:
				{
					m08_petra_c3_unit++;
				}
				break;
			}
		}

		if(type == M08_PETRA_C_KILLED)
		{
			switch(param)
			{
			case 1:
				{
					m08_petra_c1_unit--;
				}
				break;
			case 2:
				{
					m08_petra_c2_unit--;
				}
				break;
			case 3:
				{
					m08_petra_c3_unit--;
				}
				break;
			}
		}

		if((type == M08_PETRA_C_KILLED || type == M08_PETRA_C_HELO_KILLED) && petra_c_active)
		{
			
			switch(param)
			{
			case 1:
				{
					if(m08_petra_c1_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111759);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(c1_last_loc == 111759)
						{
							drop_loc = Commands->Find_Object(111760);
							drop_facing = Commands->Get_Facing(drop_loc);
							c1_last_loc = 111760;
						}
						else if(c1_last_loc == 111760)
						{
							drop_loc = Commands->Find_Object(111759);
							drop_facing = Commands->Get_Facing(drop_loc);
							c1_last_loc = 111759;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropC1.txt");
					}
				}
				break;
			case 2:
				{
					if(m08_petra_c2_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111761);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(c2_last_loc == 111761)
						{
							drop_loc = Commands->Find_Object(111762);
							drop_facing = Commands->Get_Facing(drop_loc);
							c2_last_loc = 111762;
						}
						else if(c2_last_loc == 111762)
						{
							drop_loc = Commands->Find_Object(111761);
							drop_facing = Commands->Get_Facing(drop_loc);
							c2_last_loc = 111761;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropC2.txt");
					}
				}
				break;
			case 3:
				{
					if(m08_petra_c3_unit == 0)
					{
						GameObject * drop_loc = Commands->Find_Object(111763);
						float drop_facing = Commands->Get_Facing(drop_loc);

						if(c3_last_loc == 111763)
						{
							drop_loc = Commands->Find_Object(111764);
							drop_facing = Commands->Get_Facing(drop_loc);
							c3_last_loc = 111764;
						}
						else if(c3_last_loc == 111764)
						{
							drop_loc = Commands->Find_Object(111763);
							drop_facing = Commands->Get_Facing(drop_loc);
							c3_last_loc = 111763;
						}
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X8I_TroopDropC3.txt");
					}
				}
				break;
			}
		}
		
	}

};

DECLARE_SCRIPT(M08_Petra_C_Unit, "Unit_ID=0:int")
{
	
	enum{GO_STAR, GO_PETRA_C};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * obj) override
	{
		ActionParamsStruct params;

		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_CREATED, unit_id, 0.0f);

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_UNIT_STATUS, unit_id, 0.0f);
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );

			Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_PETRA_C_UNIT_STATUS)
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);

			bool petra_c_active = (param == 1) ? true : false;

			if(petra_c_active)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
				params.Set_Movement( STAR, RUN, 1.5f );
				Commands->Action_Goto( obj, params );

				Commands->Start_Timer (obj, this, 20.0f, GO_STAR);
			}
			else
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_PETRA_C );
				params.Set_Movement( Commands->Find_Object(100370), RUN, 8.5f );
				Commands->Action_Goto( obj, params );
			}
		}
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		ActionParamsStruct params;
		
		if ( sound.Type == M08_PETRA_C_LOGICAL )
		{
			int unit_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_UNIT_STATUS, unit_id, 0.0f);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_PETRA_C && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL );
		}
		
		
	}


	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Petra_C_Helo, "Unit_ID=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( m08_petra_a1_unit, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_HELO_KILLED, unit_id, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Activate_Petra_C, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(109626), M08_PETRA_C_ACTIVATE, 1, 0.0f);
			
		}
	}

};

DECLARE_SCRIPT(M08_Deactivate_Petra_C, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100389), M08_PETRA_C_DEACTIVATE, 1, 0.0f);
			
		}
	}

	
};

DECLARE_SCRIPT(M08_Facility_APC, "")  
{
	bool attacking;
	int reinforce;
	bool enemy_seen;
	
	enum{WAYPATH, DROP_SOLDIERS, ATTACK_OVER};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( reinforce, 2 );
		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		reinforce = 0;
		enemy_seen = false;
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			enemy_seen = true;

			char param1[12];
			sprintf(param1, "%d", Commands->Get_ID(obj));

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * -4.0;
			float b = sin(DEG_TO_RADF(facing)) * -4.0;
		
			Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
			GameObject * soldier1 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc1);
			Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

			Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
			GameObject * soldier2 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc2);
			Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
		}
		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 10 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}
	}

	

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_REINFORCEMENT_KILLED)
		{
			reinforce++;
			if(reinforce%2 == 0 && reinforce < 5)
			{
				enemy_seen = true;

				char param1[12];
				sprintf(param1, "%d", Commands->Get_ID(obj));

				Vector3 pos = Commands->Get_Position(obj);
				float facing = Commands->Get_Facing(obj);
				float a = cos(DEG_TO_RADF(facing)) * -4.0;
				float b = sin(DEG_TO_RADF(facing)) * -4.0;
			
				Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
				GameObject * soldier1 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc1);
				Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

				Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
				GameObject * soldier2 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc2);
				Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
			}
		}

	}

	
	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		
		if(timer_id == ATTACK_OVER)
		{
			attacking = false;
		}
		
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			enemy_seen = true;

			char param1[12];
			sprintf(param1, "%d", Commands->Get_ID(obj));

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * -4.0;
			float b = sin(DEG_TO_RADF(facing)) * -4.0;
		
			Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
			GameObject * soldier1 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc1);
			Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

			Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
			GameObject * soldier2 = Commands->Create_Object("Nod_MiniGunner_1Off", soldier_loc2);
			Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
		}
	}

};

DECLARE_SCRIPT(M08_Patrol_Inactive, "Waypath_ID=0:int, Waypath_Loc:Vector3, Controller_ID=0:int")
{
	int waypath_id;
	Vector3 waypath_loc;
	bool enemy_seen;
	
	enum {WAYPATH, GO_WAYPATH, GO_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
//		SAVE_VARIABLE( waypath_loc, 2 );
//		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, WAYPATH);
		}
		if(action_id == GO_STAR && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 60.0f, GO_STAR);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON)
		{
			// No starting units can hear footsteps until otherwise alerted
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

			enemy_seen = false;

			waypath_id = Get_Int_Parameter("Waypath_ID");
			waypath_loc = Get_Vector3_Parameter("Waypath_Loc");

			Commands->Set_Innate_Soldier_Home_Location(obj, waypath_loc, 4.0f);                                                                           

			ActionParamsStruct params;
	
			if(waypath_id == 0)
			{
				Commands->Set_Innate_Is_Stationary(obj, true);
			}
			else if(waypath_id == 1)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
				params.Set_Movement( STAR, RUN, 1.5f );
				Commands->Action_Goto( obj, params );
			}
			else
			{
				params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, GO_WAYPATH );
				params.Set_Movement( waypath_loc, RUN, 1.5f );
				Commands->Action_Goto( obj, params );
			}		
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if((Commands->Is_A_Star(enemy)) && (!enemy_seen))
		{
			enemy_seen = true;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == WAYPATH)
		{
			params.Set_Basic( this, INNATE_PRIORITY_GUNSHOT_HEARD - 5, GO_WAYPATH );
			params.Set_Movement( waypath_loc, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			params.Set_Movement( STAR, RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int controller_id = Get_Int_Parameter("Controller_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id), M08_PATROL_KILLED, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Scientist, "Controller_ID=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( waypath_id, 1 );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON)
		{
			Commands->Start_Random_Conversation(obj);
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int controller_id = Get_Int_Parameter("Controller_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id), M08_SCIENTIST_KILLED, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Observatory_Dome_Controller, "")
{
	bool reinforcements;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( reinforcements, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
		reinforcements = false;
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_SCIENTIST_KILLED && reinforcements)
		{
			// Spawn units to pour into observatory
			Commands->Enable_Spawner(100473, true);
			Commands->Enable_Spawner(100474, true);
			Commands->Enable_Spawner(100475, true);
			Commands->Enable_Spawner(100476, true);
		}

		if(type == M08_REINFORCEMENTS)
		{
			reinforcements = (param == 1) ? true : false;
			
		}

	}
};

DECLARE_SCRIPT(M08_Reinforcements_Active, "Controller_ID=0:int")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			int controller_id = Get_Int_Parameter("Controller_ID");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id), M08_REINFORCEMENTS, 1, 0.0f);
			
		}
	}

	
};

DECLARE_SCRIPT(M08_Reinforcements_Inactive, "Controller_ID=0:int")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if (Commands->Is_A_Star(enterer))
		{
			int controller_id = Get_Int_Parameter("Controller_ID");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id), M08_REINFORCEMENTS, 0, 0.0f);
			
		}
	}

	
};

DECLARE_SCRIPT(M08_Activate_Cavern_Entrance_Truck, "")
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
			
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100516), M08_CAVERN_ENTRANCE_TRUCK, 1, 0.0f);

		}
	}

};

DECLARE_SCRIPT(M08_Cavern_Entrance_Truck, "")
{

	enum{WAYPATH};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_CAVERN_ENTRANCE_TRUCK)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.WaypathID = 100517;
			params.WaypathSplined = true;
			Commands->Action_Goto( obj, params );
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL );
		}
	}

};

DECLARE_SCRIPT(M08_Facility_Scientist_Inactive, "Point1_ID=0:int, Point2_ID=0:int, Point3_ID=0:int, Controller_ID=0:int")
{
	int point1_id;
	int point2_id;
	int point3_id;
	int controller_id;
	bool enemy_seen;

	enum {GO_POINT1, GO_POINT2, GO_POINT3, GO_POINT1A, GO_POINT2A, GO_POINT3A,
			FROM_HANDS_OVER_HEAD, TO_HANDS_OVER_HEAD};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_seen, 5 );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON)
		{
			enemy_seen = false;

			// No starting units can hear footsteps until otherwise alerted
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

			point1_id = Get_Int_Parameter("Point1_ID");
			point2_id = Get_Int_Parameter("Point2_ID");
			point3_id = Get_Int_Parameter("Point3_ID");
			
			ActionParamsStruct params;
			
			if(point1_id != 0)
			{
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT1A );
				params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
				Commands->Action_Goto( obj, params );
			}
			
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(reason != ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer(obj, this, 30.0f, GO_POINT1);
			return;
		}

		// Play animation at point
		if(action_id == GO_POINT1A && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT2 );
			params.Set_Animation ("S_A_HUMAN.H_A_A0F0", false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == GO_POINT2A && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT3 );
			params.Set_Animation ("S_A_HUMAN.H_A_A0F0", false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == GO_POINT3A && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT1 );
			params.Set_Animation ("S_A_HUMAN.H_A_A0F0", false);
			Commands->Action_Play_Animation (obj, params);
		}
		
		// Goto point
		if(action_id == GO_POINT1 && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT1A );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_POINT2 && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT2A );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if(action_id == GO_POINT3 && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT3A );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}

		if(action_id == TO_HANDS_OVER_HEAD && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100);
			params.Set_Animation( "H_A_Host_L1b", true );
			Commands->Action_Play_Animation (obj, params);
		}

		if(action_id == FROM_HANDS_OVER_HEAD && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT1A );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}

	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == GO_POINT1 && !enemy_seen)
		{
			enemy_seen = false;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_POINT1A );
			params.Set_Movement( Commands->Find_Object(point1_id), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		else
		{
			Commands->Start_Timer(obj, this, 30.0f, GO_POINT1);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if((Commands->Is_A_Star(enemy)) && (!enemy_seen))
		{
			enemy_seen = true;
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);

			Commands->Set_Innate_Is_Stationary(obj, true);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, TO_HANDS_OVER_HEAD);
			params.Set_Animation( "H_A_Host_L1a", false );
			Commands->Action_Play_Animation (obj, params);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		int controller_id_param = Get_Int_Parameter("Controller_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id_param), M08_SCIENTIST_KILLED, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Cavern_Controller, "")
{
	int cavern_unit;

	bool cavern_activated;
	bool first_apc;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( cavern_activated, 1 );
		SAVE_VARIABLE( first_apc, 2 );
	}

	void Created(GameObject * /* obj */) override
	{
		cavern_unit = 6;
		cavern_activated = false;
		first_apc = true;
	}
	
	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if (type == M08_DEACTIVATE_ENCOUNTER)
		{
			if(cavern_activated)
			{
				cavern_activated = false;
			}
		}
		if(type == M08_SCIENTIST_KILLED)
		{
			cavern_unit--;
			if(cavern_activated)
			{
		//		Cavern_Reinforcements ();
			}
		}

		if(type == M08_PATROL_KILLED)
		{
			
			cavern_unit--;
			if(cavern_activated)
			{
		//		Cavern_Reinforcements ();
			}
		}
		if(type == M08_CUSTOM_ACTIVATE)
		{
			
			cavern_activated = true;
			Commands->Enable_Spawner(100593, true);
			Commands->Enable_Spawner(100592, true);
		}
		
	}

/*	void Cavern_Reinforcements ()
	{
		if (cavern_unit < 5 && first_apc)
		{
			first_apc = false;
			Commands->Enable_Spawner(100593, true);
			Cavern_Reinforcements ();
				
		}
		else if (cavern_unit < 3 && !first_apc)
		{
			Commands->Enable_Spawner(100592, true);
		}
	
	}*/

};

DECLARE_SCRIPT(M08_Custom_Activate, "Controller_ID=0:int")
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
			
			int controller_id = Get_Int_Parameter("Controller_ID");
			// Custom to activate
			Commands->Send_Custom_Event(obj, Commands->Find_Object(controller_id), M08_CUSTOM_ACTIVATE, 1, 0.0f);
			
		}
	}

};

DECLARE_SCRIPT(M08_Cavern_APC, "Waypath_ID=0:int")  
{
	bool attacking;
	int reinforce;
	
	enum{ATTACK_OVER, DROP_SOLDIERS};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( reinforce, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		int waypath_id = Get_Int_Parameter("Waypath_ID");

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, DROP_SOLDIERS );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = waypath_id;
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );
	}
	
};

DECLARE_SCRIPT(M08_Cavern_Tunnel_APC, "")  
{
	bool attacking;
	int reinforce;
	bool enemy_seen;
	
	enum{WAYPATH, DROP_SOLDIERS, ATTACK_OVER};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( reinforce, 2 );
		SAVE_VARIABLE( enemy_seen, 3 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		reinforce = 0;
		enemy_seen = false;
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			enemy_seen = true;

			char param1[12];
			sprintf(param1, "%d", Commands->Get_ID(obj));

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * -4.0;
			float b = sin(DEG_TO_RADF(facing)) * -4.0;
		
			Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
			GameObject * soldier1 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc1);
			Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

			Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
			GameObject * soldier2 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc2);
			Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
		}
		if(!attacking)
		{
			attacking = true;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 10 );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}
	}

	

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_REINFORCEMENT_KILLED)
		{
			reinforce++;
			if(reinforce%2 == 0 && reinforce < 7)
			{
				char param1[12];
				sprintf(param1, "%d", Commands->Get_ID(obj));

				Vector3 pos = Commands->Get_Position(obj);
				float facing = Commands->Get_Facing(obj);
				float a = cos(DEG_TO_RADF(facing)) * -4.0;
				float b = sin(DEG_TO_RADF(facing)) * -4.0;
			
				Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
				GameObject * soldier1 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc1);
				Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

				Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
				GameObject * soldier2 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc2);
				Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
			}
		}

	}

	
	void Timer_Expired(GameObject * /* obj */, int timer_id ) override
	{
		
		if(timer_id == ATTACK_OVER)
		{
			attacking = false;
		}
		
	}

	void Damaged (GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			enemy_seen = true;

			char param1[12];
			sprintf(param1, "%d", Commands->Get_ID(obj));

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * -4.0;
			float b = sin(DEG_TO_RADF(facing)) * -4.0;
		
			Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
			GameObject * soldier1 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc1);
			Commands->Attach_Script(soldier1, "M08_APC_Soldier", param1);

			Vector3 soldier_loc2 = pos + Vector3(a, (b - .5f), 0.5f);
			GameObject * soldier2 = Commands->Create_Object("Nod_FlameThrower_2SF", soldier_loc2);
			Commands->Attach_Script(soldier2, "M08_APC_Soldier", param1);
		}
	}

};

DECLARE_SCRIPT(M08_Cavern_Gun_Emplacement, "")
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
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
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
		ActionParamsStruct params;

		// Spawn buggy to patrol lower cavern
		Commands->Enable_Spawner(100620, true);

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

DECLARE_SCRIPT(M08_Lower_Cavern_Buggy, "")
{
	bool attacking;
	bool in_cavern;

	enum{WAYPATH, ATTACK_OVER, GO_WAYPATH};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( in_cavern, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100621;
		params.WaypathSplined = true;
		Commands->Action_Goto( obj, params );

		attacking = false;
		in_cavern = false;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking && !in_cavern)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), WALK, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 100631;
			params.WaypathSplined = true;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 10.0f, ATTACK_OVER);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == GO_WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			in_cavern = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (STAR, 250.0f, 0.0f, 1);
			params.WaypathID = 100631;
			params.WaypathSplined = true;
			Commands->Action_Attack( obj, params );
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (STAR, 250.0f, 0.0f, 1);
			params.WaypathID = 100631;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
			
		}
		
	}
	
};

DECLARE_SCRIPT(M08_Unarmed_Prisoner, "Exit_Cell_ID=0:int, Weapon_Loc_ID=0:int")
{
	bool free;

	enum{EXIT_CELL, WEAPONS, KEEP_TRYING};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( free, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );
		Commands->Set_Innate_Is_Stationary(obj, true);
		free = false;
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == WEAPONS && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
			Commands->Select_Weapon(obj, "Weapon_Chaingun_Ai" );
		}
		if(action_id == EXIT_CELL && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WEAPONS );
			params.Set_Movement( Commands->Find_Object(Get_Int_Parameter("Weapon_Loc_ID")), RUN, 1.0f );
			Commands->Action_Goto( obj, params );
		}
		if(reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer (obj, this, 1.0f, KEEP_TRYING);
		}

		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == KEEP_TRYING)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, EXIT_CELL );
			params.Set_Movement( Commands->Find_Object(Get_Int_Parameter("Exit_Cell_ID")), RUN, 1.0f );
			Commands->Action_Goto( obj, params );
		}
		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M08_FREE_PRISONER && !free)
		{
			free = true;

			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Set_Innate_Is_Stationary(obj, false);
			Commands->Grant_Key (obj, 1, true);


			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, EXIT_CELL );
			params.Set_Movement( Commands->Find_Object(Get_Int_Parameter("Exit_Cell_ID")), RUN, 1.0f );
			Commands->Action_Goto( obj, params );

		}

	}


	void Poked(GameObject * obj, GameObject * poker) override
	{
		if (Commands->Is_A_Star(poker))
		{
			Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
			Commands->Select_Weapon(obj, "Weapon_Chaingun_Ai" );
		}
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		
	}
};

DECLARE_SCRIPT(M08_Free_Prisoners_Zone, "Prisoner1_ID=0:int, Prisoner2_ID=0:int, Prisoner3_ID=0:int, Prisoner4_ID=0:int")
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
			
			int prisoner1_id = Get_Int_Parameter("Prisoner1_ID");
			int prisoner2_id = Get_Int_Parameter("Prisoner2_ID");
			int prisoner3_id = Get_Int_Parameter("Prisoner3_ID");
			int prisoner4_id = Get_Int_Parameter("Prisoner4_ID");

			Commands->Send_Custom_Event(obj, Commands->Find_Object(prisoner1_id), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(prisoner2_id), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(prisoner3_id), M08_FREE_PRISONER, 1, 0.0f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(prisoner4_id), M08_FREE_PRISONER, 1, 0.0f);
		}
	}


};


DECLARE_SCRIPT(M08_Sakura, "")
{
	bool enemy_seen;
	bool flee;
	int current_waypath;
	float health;
	
	enum{WAYPATH, TO_SECOND, INITIAL, FLEE};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_seen, 1 );
		SAVE_VARIABLE( flee, 2 );
		SAVE_VARIABLE( current_waypath, 3 );
		SAVE_VARIABLE( health, 4 );
		SAVE_VARIABLE( flee, 5 );
	}

	void Created(GameObject * obj) override
	{
		// Send Sakura's id to the movement zones
		int sakura_id = Commands->Get_ID(obj);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100356), M08_SAKURA_ID, sakura_id, 0.0f);

		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		Commands->Enable_Enemy_Seen( obj, true);
		enemy_seen = true;
		
		ActionParamsStruct params;
		
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INITIAL );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100774;
		Commands->Action_Goto( obj, params );

		current_waypath = 100774;
		health = Commands->Get_Max_Health(obj);
		flee = false;

	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!enemy_seen)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 150.0f, 0.0f, 1);
			params.WaypathID = current_waypath;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);
		}
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_MOVE_SAKURA)
		{
			switch(param)
			{
			case 2:
				{
					params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, TO_SECOND );
					params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
					params.WaypathID = 100795;
					Commands->Action_Goto( obj, params );

					enemy_seen = false;
				}
				break;
			}
			
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		ActionParamsStruct params;

		if(action_id == INITIAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100780;
			Commands->Action_Attack( obj, params );
			enemy_seen = false;

			current_waypath = 100780;
		}

		if(action_id == TO_SECOND)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100800;
			Commands->Action_Attack( obj, params );
			enemy_seen = false;

			current_waypath = 100800;
		}	
		if(action_id == FLEE)
		{
			Commands->Destroy_Object(obj);
		}
		
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		ActionParamsStruct params;

		

		if((Commands->Get_Health(obj) < (.2 * Commands->Get_Max_Health(obj))) && !flee)
		{
			flee = true;
			
			
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 6, FLEE );
			params.MovePathfind = false;
			params.Set_Movement( Commands->Get_Position(Commands->Find_Object(110337)), RUN, 1.5f );
			Commands->Action_Goto( obj, params );
		}
		if((Commands->Get_Health(obj) < (.2 * Commands->Get_Max_Health(obj))))
		{
			Commands->Set_Health(obj, health);
		}

		health = Commands->Get_Health(obj);
	}

};

DECLARE_SCRIPT(M08_Activate_Sakura, "")
{
	bool already_entered;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created(GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			// Spawn Sakura
			Commands->Enable_Spawner(100773, true);

			

		}
	}

};

DECLARE_SCRIPT(M08_Nod_Warden, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Innate_Disable(obj);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON)
		{
			Commands->Innate_Enable(obj);
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Vector3 create_position = Commands->Get_Position( obj );
		create_position.Z += 0.5f;
		Commands->Create_Object( "Level_02_Keycard", create_position );
	}

	
};

DECLARE_SCRIPT(M08_Move_Sakura, "Movement_Loc=0:int")
{
	bool already_entered;
	int sakura_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( sakura_id, 2 );
	}

	void Created(GameObject * /* obj */) override
	{
		sakura_id = 0;
		already_entered = false;
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)) && (!already_entered))
		{
			already_entered = true;

			int movement_loc = Get_Int_Parameter("Movement_Loc");
			
			// Custom to Sakura to move to next location in the Petra canyon
			Commands->Send_Custom_Event(obj, Commands->Find_Object(sakura_id), M08_MOVE_SAKURA, movement_loc, 0.0f);

		}
	}

	void Custom (GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_SAKURA_ID)
		{
			sakura_id = param;
		}
		
	}

};

DECLARE_SCRIPT (M08_Mobile_Vehicle, "CheckBlocked=1:int, Attack_Loc0=0:int, Attack_Loc1=0:int, Attack_Loc2=0:int, Attack_Loc3=0:int, Attack_Loc4=0:int, Attack_Loc5=0:int, Attack_Loc6=0:int, Attack_Loc7=0:int, Attack_Loc8=0:int, Attack_Loc9=0:int, Attack_Loc10=0:int")	
{
	bool attacking;
	int attack_loc [11];
	float loc_dist;
	int loc;

	Vector3 current_loc;
	Vector3 enemy_loc;

	float health;

	enum{ATTACK_OVER, GOTO_LOC};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( current_loc, 4 );
		SAVE_VARIABLE( enemy_loc, 5 );
		SAVE_VARIABLE( loc_dist, 6 );
		SAVE_VARIABLE( loc, 7 );
		SAVE_VARIABLE( health, 8 );
		SAVE_VARIABLE( attack_loc, 9 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;
		
		attack_loc [0] = Get_Int_Parameter("Attack_Loc0");
		attack_loc [1] = Get_Int_Parameter("Attack_Loc1");
		attack_loc [2] = Get_Int_Parameter("Attack_Loc2");
		attack_loc [3] = Get_Int_Parameter("Attack_Loc3");
		attack_loc [4] = Get_Int_Parameter("Attack_Loc4");
		attack_loc [5] = Get_Int_Parameter("Attack_Loc5");
		attack_loc [6] = Get_Int_Parameter("Attack_Loc6");
		attack_loc [7] = Get_Int_Parameter("Attack_Loc7");
		attack_loc [8] = Get_Int_Parameter("Attack_Loc8");
		attack_loc [9] = Get_Int_Parameter("Attack_Loc9");
		attack_loc [10] = Get_Int_Parameter("Attack_Loc10");
		
		loc_dist = 1000.0f;
		loc = 100;

		Commands->Start_Timer(obj, this, 1.0f, GOTO_LOC);			
		health = Commands->Get_Health(obj);

	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
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

		Commands->Debug_Message("Attack_Loc = %d /n", attack_loc[loc]);
		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10 );
		params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
		params.Set_Attack(enemy, 100.0f, 5.0f, true);
		params.AttackCheckBlocked = blocked;
		
		Commands->Modify_Action(obj, 10, params, true, true);				
	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		if (timer_id == GOTO_LOC)
		{
			current_loc = Commands->Get_Position ( obj );
			Vector3 star_loc = Commands->Get_Position (STAR);

			for (int x = 0; x <= 10; x++)
			{
				if(attack_loc[x] != 0)
				{				
					float dist = Commands->Get_Distance(star_loc, Commands->Get_Position (Commands->Find_Object (attack_loc [x])));
					
					if (dist < loc_dist)
					{
						loc_dist = dist;
						
						loc = x;
					}
				}
			}

			loc_dist = 1000.0f;
			
			Commands->Action_Reset(obj, 99);
			
			ActionParamsStruct params;
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN +5, 10 );
			params.Set_Movement( Commands->Get_Position (Commands->Find_Object (attack_loc [loc])), 1.0f, 5.0f );
			
			Commands->Action_Attack(obj, params);
			
			Commands->Start_Timer(obj, this, 5.0f, GOTO_LOC);			
		}
	}

};

DECLARE_SCRIPT(M08_Flyover_Controller, "")
{
	int last;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(last, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 10.0f, 0);
		last = 0;
		
	}

	void Timer_Expired(GameObject * obj, int /* timer_id */) override
	{
		static constexpr const char *flyovers[8] =
		{
			"X8B_Apache_00.txt",
			"X8B_Apache_01.txt",
			"X8B_Apache_02.txt",
			"X8B_Apache_03.txt",

			"X8B_Trnspt_00.txt",
			"X8B_Trnspt_01.txt",
			"X8B_Trnspt_02.txt",
			"X8B_Trnspt_03.txt",
			
		};
		int random = Commands->Get_Random_Int ( 0, 7 );
	
		while (random == last) 
		{
			random = Commands->Get_Random_Int ( 0, 7 );
		}

		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
		Commands->Attach_Script(controller, "Test_Cinematic", flyovers[random]);
		Commands->Start_Timer(obj, this, 10.0f, 0);
		
		last = random;
	}
};

DECLARE_SCRIPT(M08_Activate_Excavation, "")
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
			
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(210.156f, 301.252f, -7.404f));
			Commands->Set_Facing(chinook_obj1, 25.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M08_XG_VehicleDrop1.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(202.507f, 308.211f, -6.972f));
			Commands->Set_Facing(chinook_obj2, 0.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "M08_XG_VehicleDrop1.txt");
		}
	}

};

DECLARE_SCRIPT(M08_Facility_Scientist, "Loc1_ID=0:int, Loc2_ID=0:int, Loc3_ID=0:int, Animation:string, Spawner_ID=0:int")
{
	bool enemy_seen;
	bool start_over;
	
	enum{GO_FIRST_LOC, FACING_FIRST_LOC, ANIMATE_FIRST_LOC, GO_SECOND_LOC, FACING_SECOND_LOC, ANIMATE_SECOND_LOC, 
		GO_THIRD_LOC, FACING_THIRD_LOC, ANIMATE_THIRD_LOC, HANDS_OVER_HEAD, START_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( enemy_seen, 1 );
		SAVE_VARIABLE( start_over, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		start_over = false;
		enemy_seen = false;
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
		params.Set_Basic (this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_FIRST_LOC);
		params.Set_Movement (Commands->Find_Object(Get_Int_Parameter("Loc1_ID")), WALK, 1.5f);
		Commands->Action_Goto (obj, params);
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		const char *anim = Get_Parameter("Animation");

		if(reason != ACTION_COMPLETE_NORMAL && action_id != Commands->Get_Action_ID(obj) && !start_over)
		{
			start_over = true;
			Commands->Start_Timer (obj, this, 10.0f, START_OVER);
			return;
		}
		// First Loc
		if(action_id == GO_FIRST_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, FACING_FIRST_LOC);
			params.Set_Face_Location( Commands->Get_Position(Commands->Find_Object(Get_Int_Parameter("Loc1_ID"))), 1.5f);
			Commands->Action_Face_Location ( obj, params );
		}
		if(action_id == FACING_FIRST_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, ANIMATE_FIRST_LOC );
			params.Set_Animation (anim, false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == ANIMATE_FIRST_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_SECOND_LOC);
			params.Set_Movement (Commands->Find_Object(Get_Int_Parameter("Loc2_ID")), WALK, 1.5f);
			Commands->Action_Goto (obj, params);
		}

		// Second Loc
		if(action_id == GO_SECOND_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, FACING_SECOND_LOC);
			params.Set_Face_Location( Commands->Get_Position(Commands->Find_Object(Get_Int_Parameter("Loc2_ID"))), 1.5f);
			Commands->Action_Face_Location ( obj, params );
		}
		if(action_id == FACING_SECOND_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, ANIMATE_SECOND_LOC );
			params.Set_Animation (anim, false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == ANIMATE_SECOND_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_THIRD_LOC);
			params.Set_Movement (Commands->Find_Object(Get_Int_Parameter("Loc3_ID")), WALK, 1.5f);
			Commands->Action_Goto (obj, params);
		}
		
		// Third Loc

		if(action_id == GO_THIRD_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, FACING_THIRD_LOC);
			params.Set_Face_Location( Commands->Get_Position(Commands->Find_Object(Get_Int_Parameter("Loc3_ID"))), 1.5f);
			Commands->Action_Face_Location ( obj, params );
		}
		if(action_id == FACING_THIRD_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, ANIMATE_THIRD_LOC );
			params.Set_Animation (anim, false);
			Commands->Action_Play_Animation (obj, params);
		}
		if(action_id == ANIMATE_THIRD_LOC && reason == ACTION_COMPLETE_NORMAL)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), GO_FIRST_LOC);
			params.Set_Movement (Commands->Find_Object(Get_Int_Parameter("Loc1_ID")), WALK, 1.5f);
			Commands->Action_Goto (obj, params);
		}

		if(action_id == HANDS_OVER_HEAD && reason == ACTION_COMPLETE_NORMAL)
		{
			// In hands over head anim
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
			params.Set_Animation( "H_A_HOST_L1B", true );
			Commands->Action_Play_Animation (obj, params);
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject * /* enemy */ ) override
	{
		ActionParamsStruct params;

		Commands->Create_Logical_Sound(obj, SOUND_TYPE_GUNSHOT, Commands->Get_Position(obj), 8.0f);
		if(!enemy_seen)
		{
			// Help!  Intruder!  Help!\n
			const char *conv_name = ("M08_CON008");
			int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
			Commands->Join_Conversation(obj, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 1010211);

			// Assume hands over head anim
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, HANDS_OVER_HEAD);
			params.Set_Animation( "H_A_HOST_L1A", false );
			Commands->Action_Play_Animation (obj, params);

			enemy_seen = true;
		}
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		// Spawn stealth soldier to come after STAR
		int spawner_id = Get_Int_Parameter("Spawner_ID");
		Commands->Enable_Spawner(spawner_id, true);

	}

};

DECLARE_SCRIPT(M08_Apache_Controller, "")
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
		apache_id[1] = Commands->Get_ID(Commands->Create_Object("Nod_Apache", Vector3(-45.993f, -66.675f, 12.382f)));
		GameObject * apache = Commands->Find_Object(apache_id[1]);
		Commands->Attach_Script(apache, "M08_Apache", "1");
		Commands->Send_Custom_Event(Owner(), apache, 400, 400, 0.0f);
		
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

	void Attack_Player(int /* current_area */)
	{
		GameObject * apache = Commands->Find_Object(apache_id[area]);
		if (apache)
		{
			Commands->Send_Custom_Event(Owner(), apache, 100, 100, 0.0f);
			attacking_apache = area;
		}
	}

	void Replace_Apache(int current_area)
	{
		Vector3 start_loc;
		start_loc.Set(-45.993f, -66.675f, 12.382f);

		GameObject * apache = Commands->Create_Object("Nod_Apache_No_Idle", start_loc);
		Commands->Enable_Engine(apache, true);
		char param[12];
		sprintf(param, "%d", current_area);
		Commands->Attach_Script(apache, "M08_Apache", param);
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

DECLARE_SCRIPT(M08_Apache, "Area:int")
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
			Commands->Action_Goto(obj, params);
		}
		if (type == 200 && param == 200)
		{
			int pad_obj = 106339;
						
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 2);
			params.Set_Movement(Commands->Find_Object(pad_obj), 1.0f, 0.1f);
			Commands->Action_Goto(obj, params);

		}
		if (type == 300 && param == 300)
		{
			pad_destroyed = true;
			if (on_pad)
			{
				Commands->Apply_Damage(obj, 10000.0f, "EXPLOSIVE", NULL );
			}
		}
		if (type == 400 && param == 400)
		{
			int waypath = 100322;
						
			ActionParamsStruct params;
			params.Set_Basic(this, 90, 2);
			params.Set_Movement(Vector3(0,0,0), 1.0f, 1.0f);
			params.WaypathID = waypath;
			Commands->Action_Goto(obj, params);

		}
		if (type == 500 && param == 500)
		{
			int pad_obj = 106339;
			
			ActionParamsStruct params;
			params.Set_Basic(this, 91, 3);
			params.Set_Movement(Commands->Find_Object(pad_obj), 1.0f, 0.1f);
			Commands->Action_Goto(obj, params);

		}
	}
	
	void Timer_Expired(GameObject * obj, int timer_id) override
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
			GameObject * controller = Commands->Find_Object(106339);
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

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
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
			Commands->Action_Attack(obj, params);
	
			Commands->Start_Timer(obj, this, Commands->Get_Random(4.0f, 6.0f), 2);
		}
		if (action_id == 2)
		{
			Commands->Start_Timer(obj, this, 1.0f, 4);

			on_pad = true;
			Commands->Start_Timer(obj, this, 5.0f, 1);

			GameObject * controller = Commands->Find_Object(106339);
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

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		GameObject * controller = Commands->Find_Object(106339);
		if (controller)
		{
			Commands->Send_Custom_Event(obj, controller, 2000, Get_Int_Parameter("Area"), 0.0f);
		}
	}
};


DECLARE_SCRIPT(M08_Mobile_Apache, "Entrance_Path_ID=0:int, Helipad_ID=0:int")
{
	
	enum {ENTRANCE, GO_STAR, MOVE_TO_STAR, AIRBORNE, GO_HELIPAD, ARRIVED_HELIPAD, FLEE};

	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE(on_pad, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, ENTRANCE );
		params.MovePathfind = false;
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = Get_Int_Parameter("Entrance_Path_ID");
		Commands->Action_Goto( obj, params );
	}
	
	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(reason != ACTION_COMPLETE_NORMAL)
		{
			Commands->Start_Timer (obj, this, 2.0f, MOVE_TO_STAR);
			return;
		}
		if(action_id == ENTRANCE)
		{
			Go_Star(obj);
			Commands->Start_Timer (obj, this, 60.0f, GO_HELIPAD);
		}
		if(action_id == GO_STAR)
		{
			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 2);
			params.MovePathfind = false;
			params.Set_Attack(STAR, 150.0f, 2.0f, true);
			Commands->Action_Attack(obj, params);
			Commands->Start_Timer (obj, this, 2.0f, MOVE_TO_STAR);
		}
		if(action_id == ARRIVED_HELIPAD)
		{
			Commands->Enable_Engine(obj, false);
			Commands->Start_Timer (obj, this, 5.0f, AIRBORNE);
		}
		if(action_id == FLEE)
		{
			Commands->Destroy_Object(obj);
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;
		
		if(timer_id == MOVE_TO_STAR)
		{
			Go_Star(obj);		
		}
		if(timer_id == GO_HELIPAD)
		{
			
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 6, ARRIVED_HELIPAD);
			params.MovePathfind = false;
			params.Set_Movement(Commands->Find_Object(Get_Int_Parameter("Helipad_ID")), 1.0f, 0.1f);
			Commands->Action_Goto(obj, params);
		}
		if(timer_id == AIRBORNE)
		{
			Commands->Enable_Engine(obj, true);
			Commands->Start_Timer (obj, this, 2.0f, MOVE_TO_STAR);
			Commands->Start_Timer (obj, this, 60.0f, GO_HELIPAD);
		}
		
	}

	void Go_Star(GameObject * obj)
	{
		Vector3 pos = Commands->Get_Position(STAR);
		float facing = Commands->Get_Facing(STAR);
		pos.X -= cos(DEG_TO_RADF(facing)) * 6.0f;
		pos.Y -= sin(DEG_TO_RADF(facing)) * 6.0f;

		pos.Z = WWMath::Max(pos.Z + 12.0f, Commands->Get_Safe_Flight_Height(pos.X, pos.Y) + 6.0f);
			
		ActionParamsStruct params;
		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_STAR);
		params.MovePathfind = false;
		params.Set_Movement(pos, 1.0f, 5.0f);
		Commands->Action_Goto(obj, params);
	}

	void Sound_Heard(GameObject* obj, const CombatSound & sound) override
	{
		ActionParamsStruct params;
		
		if ( sound.Type == M08_HELIPAD_DESTROYED )
		{
			Vector3 pos = Commands->Get_Position(obj);
			pos.Z = pos.Z + 300.0f;
			params.Set_Basic(this, 100, FLEE);
			params.MovePathfind = false;
			params.Set_Movement(pos, 1.0f, 5.0f);
			Commands->Action_Goto(obj, params);
		}
	}
};

DECLARE_SCRIPT(M08_Mutant_Behavior, "")
{
	bool freed;
	
	enum {DOING_ANIMATION, GO_STAR};

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(freed, 1);
	}
	
	void Created(GameObject * obj) override
	{
	//	Commands->Innate_Disable(obj);
		ActionParamsStruct params;
		params.Set_Basic( this, 99, DOING_ANIMATION );
		params.Set_Animation ("S_C_HUMAN.H_C_7001", true);
		Commands->Action_Play_Animation (obj, params);

		freed = false;
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;
		if(type == M08_INNATE_ON && !freed)
		{
			
			ActionParamsStruct params;
			params.Set_Basic( this, 99, DOING_ANIMATION );
			params.Set_Animation ("S_C_HUMAN.H_C_7001", false);
			Commands->Action_Play_Animation (obj, params);
			//Commands->Innate_Enable(obj);

			//params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR );
			//params.Set_Movement( STAR, RUN, 1.5f );
			//Commands->Action_Goto( obj, params );

			//freed = true;
		}
		
	}
};

DECLARE_SCRIPT(M08_Free_Jailed_Mutants, "")
{
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE(on_pad, 1);
	}
	
	void Created(GameObject * obj) override
	{
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
	}

	void Poked(GameObject * obj, GameObject * /* poker */) override
	{
		Commands->Enable_HUD_Pokable_Indicator( obj, false );
		Commands->Grant_Key (STAR, 10, true);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(106816), M08_INNATE_ON, 1, 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(106818), M08_INNATE_ON, 1, 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(106819), M08_INNATE_ON, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M08_Activate_Midtro, "")
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
			
			Commands->Set_Position (STAR, Commands->Get_Position(Commands->Find_Object(111994)));

			//Commands->Static_Anim_Phys_Goto_Last_Frame (1450630, "res_elev06.res_elev06" );

			//Commands->Set_Animation_Frame ( Commands->Find_Object (1450630), "res_elev06.res_elev06", 51 );

			GameObject * cinematic_obj = Commands->Create_Object ( "Invisible_Object", Vector3(0.0f, 0.0f, 0.0f));
			Commands->Attach_Script(cinematic_obj, "Test_Cinematic", "X8A_MIDTRO.txt");
		}
	}
};

DECLARE_SCRIPT(M08_Prisoner_Conversation, "Orator_ID=0:int, Captive=0:int")
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
			
			int orator_id = Get_Int_Parameter("Orator_ID");
			bool captive = (Get_Int_Parameter("Captive") == 1) ? true : false;
			if(Commands->Find_Object(orator_id))
			{
				if(captive)
				{
					// Save us!\n
					const char *conv_name = ("M08_CON010");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(Commands->Find_Object(orator_id), conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 2);
					
				}
				else
				{
					// It's the Commando!\n
					const char *conv_name = ("M08_CON009");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(Commands->Find_Object(orator_id), conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 2);
				}
			}
		}
	}
};

DECLARE_SCRIPT (M08_Elevator_Movement_Zone, "Direction:int, Anim_num:int, Elev_obj_num:int")
{
	void Entered (GameObject * /* obj */, GameObject * enterer) override
	{
		static constexpr const char *elevators[1] =
		{
			"res_elev06.res_elev06",
			
		};

		if (enterer == STAR)
		{
			Commands->Attach_Script(STAR, "M00_No_Falling_Damage_DME", "");
			GameObject * blocker = Commands->Create_Object ("M08_Rubble_Stub", Vector3 (-100.952F, 483.541F, -155.645F));
			Commands->Set_Is_Rendered(blocker, false);

			int elev_obj_num = Get_Int_Parameter("Elev_obj_num");

			if (Get_Int_Parameter("Direction") == 0)
			{
				Commands->Static_Anim_Phys_Goto_Frame (elev_obj_num, 0, elevators [Get_Int_Parameter("Anim_num")] );
			}

			if (Get_Int_Parameter("Direction") == 1)
			{
				Commands->Static_Anim_Phys_Goto_Last_Frame (elev_obj_num, elevators [Get_Int_Parameter("Anim_num")] );
			}
		}
	}
};

DECLARE_SCRIPT (M08_Immortal_Star_DLS, "")
{
	float health;
	bool immortal;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 1 );		
		SAVE_VARIABLE( immortal, 2 );		
	}

	void Created (GameObject * obj) override
	{
		health = Commands->Get_Health(obj);
		immortal = true;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == M08_STAR_IMMORTAL ) 
		{
			immortal = param ? true : false;
		}
	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override
	{
		if(immortal)
		{
			Commands->Set_Health(obj, health);
		}
	}
};


DECLARE_SCRIPT(M08_Enable_Stealth, "On=1:int")
{

	void Created (GameObject * obj) override
	{
		bool on = (Get_Int_Parameter("On")) ? true : false;
		if(on)
		{
			Commands->Enable_Stealth(obj, true);
		}
		else
		{
			Commands->Enable_Stealth(obj, false);
		}
	}

};

#define	PRISONER_CONV_TABLE_SIZE  ( sizeof(Prisoner_Conv_Table) / sizeof (Prisoner_Conv_Table[0]) )
const char *	Prisoner_Conv_Table[] = 
{
	"M08_CON011",	//GCP1
	"M08_CON012",
	"M08_CON013",
	"M08_CON014",
	"M08_CON015",
	"M08_CON016",
	"M08_CON017",
	"M08_CON018",
	"M08_CON019",
	"M08_CON020",		// [9]
	"M08_CON021",	//GCP2
	"M08_CON022",
	"M08_CON023",
	"M08_CON024",
	"M08_CON025",
	"M08_CON026",
	"M08_CON027",
	"M08_CON028",
	"M08_CON029",
	"M08_CON030",		// [19]
	"M08_CON031",	//GCP3
	"M08_CON032",
	"M08_CON033",
	"M08_CON034",
	"M08_CON035",
	"M08_CON036",
	"M08_CON037",
	"M08_CON038",
	"M08_CON039",
	"M08_CON040"		// [29]
};

DECLARE_SCRIPT(M08_Prisoner_Poke_Conversation, "")
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
		random = Commands->Get_Random_Int(0, PRISONER_CONV_TABLE_SIZE);
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
		
		if((::strncmp( "GDI_Prisoner_v0a", preset_name, ::strlen( "GDI_Prisoner_v0a" ) ) == 0)
			|| (::strncmp( "GDI_Prisoner_v0b", preset_name, ::strlen( "GDI_Prisoner_v0a" ) ) == 0))
		{
			Play_Conversation(obj, 10, 19);			
		}
		else if((::strncmp( "GDI_Prisoner_v1a", preset_name, ::strlen( "GDI_Prisoner_v1a" ) ) == 0)
			|| (::strncmp( "GDI_Prisoner_v1b", preset_name, ::strlen( "GDI_Prisoner_v1a" ) ) == 0))
		{
			Play_Conversation(obj, 10, 19);			
		}
		else if((::strncmp( "GDI_Prisoner_v2a", preset_name, ::strlen( "GDI_Prisoner_v2a" ) ) == 0)
			|| (::strncmp( "GDI_Prisoner_v2b", preset_name, ::strlen( "GDI_Prisoner_v2a" ) ) == 0))
		{
			Play_Conversation(obj, 20, 29);			
		}
	}

	
	int Index(int Min, int Max)
	{
		while(random == last || random < Min || random > Max)
		{
			random = Commands->Get_Random_Int(0, PRISONER_CONV_TABLE_SIZE);
		}
		last = random;
		return last;
	}

	void Play_Conversation(GameObject * obj, int Min, int Max)
	{
		const char *conv_name = Prisoner_Conv_Table[Index(Min, Max)];
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

DECLARE_SCRIPT(M08_Enable_KaneConversation, "")
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

			// Kane conversation
			GameObject * holograph = Commands->Find_Object(109884);
			GameObject * kane = Commands->Create_Object_At_Bone ( holograph, "Nod_Kane_HologramHead", "O_ARROW" );
			Commands->Attach_To_Object_Bone( kane, holograph, "O_ARROW" );
			Commands->Disable_All_Collisions ( kane );
			Commands->Set_Facing (kane, Commands->Get_Facing (holograph));
			Commands->Set_Loiters_Allowed( kane, false );
			Commands->Attach_Script(kane, "M08_KaneHead", "");
		}
	}

};

DECLARE_SCRIPT(M08_KaneHead, "")
{
		
	bool already_entered;
	
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		//  Parker!  Your interference is becoming troublesome.   
		const char *conv_name = ("M08_CON041");
		int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 0, true);
		Commands->Join_Conversation(obj, conv_id, false, true, true);
		Commands->Join_Conversation(STAR, conv_id, false, true, true);
		Commands->Start_Conversation (conv_id, 300123);
		Commands->Monitor_Conversation (obj, conv_id);	
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if((action_id == 300123 && reason == ACTION_COMPLETE_CONVERSATION_ENDED) || (action_id == 300123 && reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Destroy_Object(obj);
			// Spawn soldiers surrounding hologram
			Commands->Enable_Spawner(100695, true);
			Commands->Enable_Spawner(100692, true);
			Commands->Enable_Spawner(100693, true);

		}
	}

};

DECLARE_SCRIPT(M08_DataDisc_01_DLS, "")//
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0104I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			// Reveal Tiberium Flechette
			Commands->Reveal_Encyclopedia_Weapon(22);
		}
	}
};

DECLARE_SCRIPT(M08_DataDisc_02_DLS, "")//
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			Commands->Set_HUD_Help_Text (IDS_M00EVAG_DSGN0104I1EVAG_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY);

			// Reveal Tiberium Auto Rifle
			Commands->Reveal_Encyclopedia_Weapon(21);
		}
	}
};
