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
*     Mission07.cpp
*
* DESCRIPTION
*     Mission 7 specific scripts
*
* PROGRAMMER
*     Ryan Vervack
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 122 $
*     $Modtime: 1/08/02 12:14p $
*     $Archive: /Commando/Code/Scripts/Mission07.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include "Mission7.h"

// M07B 
// Objective Controller
DECLARE_SCRIPT(M07_Objective_Controller, "") // 100657
{
	bool accomplished_705;
	bool accomplished_701;
	
	enum {HAVOCS_SCRIPT, M07_DEAD_HAVOC, INITIAL_CONV, PROTECT_TEAM};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( accomplished_705, 1 );
		SAVE_VARIABLE( accomplished_701, 2 );
	}

	void Created(GameObject * obj) override
	{
		// Background Music
		Commands->Set_Background_Music ("07-Got A Present For Ya.mp3");

		Commands->Start_Timer (obj, this, 0.5f, HAVOCS_SCRIPT);
		Commands->Start_Timer (obj, this, 1.0f, INITIAL_CONV);
		Add_An_Objective(709);
		accomplished_705 = false;
		accomplished_701 = false;
	}

	void Add_An_Objective(int id)
	{
		GameObject *object;
						
		switch (id)
		{
		// Evacuate Sydney
		case 701: 
			{
				Commands->Add_Objective(701, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M07_01, NULL, IDS_Enc_Obj_Primary_M07_01);
				object = Commands->Find_Object(100664);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M11DSGN_DSGN1007I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(701, object);
					Commands->Set_Objective_HUD_Info_Position(701, 90.0f, "POG_M07_1_04.tga", IDS_POG_EVACUATE, Commands->Get_Position (object));
				}
			}
			break;
		// Secure SAM Sites
		case 702: 
			{
				Commands->Add_Objective(702, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M07_03, NULL, IDS_Enc_Obj_Primary_M07_03);
				object = Commands->Find_Object(100673);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0089I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(702, object);
					Commands->Set_Objective_HUD_Info_Position(702, 90.0f, "POG_M07_1_01.tga", IDS_POG_SECURE, Commands->Get_Position (object));
				}
			}
			break;
		// Destroy SSM Launchers
		case 703: 
			{
				Commands->Add_Objective(703, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M07_04, NULL, IDS_Enc_Obj_Primary_M07_04);
				object = Commands->Find_Object(100798);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(703, object);
					Commands->Set_Objective_HUD_Info_Position(703, 90.0f, "POG_M07_1_02.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		// Destroy Radar Installations
		case 704: 
			{
				Commands->Add_Objective(704, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M07_01, NULL, IDS_Enc_Obj_Secondary_M07_01);
				object = Commands->Find_Object(100721);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(704, object);
					Commands->Set_Objective_HUD_Info_Position(704, 90.0f, "POG_M07_2_07.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		// Rescue Resistance
		case 705: 
			{
				Commands->Add_Objective(705, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M07_02, NULL, IDS_Enc_Obj_Secondary_M07_02);
				object = Commands->Find_Object(115842);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(705, object);
					Commands->Set_Objective_HUD_Info_Position(705, 90.0f, "POG_M07_2_04.tga", IDS_POG_RESCUE, Commands->Get_Position (object));
				}
			}
			break;
		// Destroy Napalm Stockpile
		case 706: 
			{
				Commands->Add_Objective(706, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M07_03, NULL, IDS_Enc_Obj_Secondary_M07_03);
				object = Commands->Find_Object(100781);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(706, object);
					Commands->Set_Objective_HUD_Info_Position(706, 90.0f, "POG_M07_2_06.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		// Destroy Stealth Tank
		case 707: 
			{
				Commands->Add_Objective(707, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M07_04, NULL, IDS_Enc_Obj_Secondary_M07_04);
				object = Commands->Find_Object(100801);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(707, object);
					Commands->Set_Objective_HUD_Info_Position(707, 90.0f, "POG_M07_2_03.tga", IDS_POG_DESTROY, Commands->Get_Position (object));
				}
			}
			break;
		// Defeat Obelisk
		case 708: 
			{
				Commands->Add_Objective(708, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Secondary_M07_05, NULL, IDS_Enc_Obj_Secondary_M07_05);
				object = Commands->Find_Object(168926);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(708, object);
					Commands->Set_Objective_HUD_Info_Position(708, 90.0f, "POG_M07_2_05.tga", IDS_POG_DEFEAT, Commands->Get_Position (object));
				}
			}
			break;
		// Protect Team Members
		case 709: 
			{
				Commands->Add_Objective(709, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M07_05, NULL, IDS_Enc_Obj_Primary_M07_05);
				object = Commands->Find_Object(100717);
				if(object)
				{
					Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0091I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
					Commands->Set_Objective_Radar_Blip_Object(709, object);
					Commands->Set_Objective_HUD_Info_Position(709, 90.0f, "POG_M07_1_05.tga", IDS_POG_PROTECT, Commands->Get_Position (object));
				}
			}
			break;
		// Escape Nuclear Strike
		case 710: 
			{
				Commands->Add_Objective(710, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_Enc_ObjTitle_Primary_M07_02, NULL, IDS_Enc_Obj_Primary_M07_02);
				object = Commands->Find_Object(100717);
				if(object)
				{
					Commands->Set_Objective_Radar_Blip_Object(710, object);
					Commands->Set_Objective_HUD_Info_Position(710, 90.0f, "POG_M07_1_03.tga", IDS_POG_ESCAPE, Commands->Get_Position (object));
				}
			}
			break;
		}
	}

	void Remove_Pog(int id)
	{
		switch (id)
		{
		// Evacuate Dr. Sydney Mobius
		case 701: 
			{
				Commands->Set_Objective_HUD_Info (701, -1, "POG_M07_1_04.tga", IDS_POG_PROTECT);
			}
			break;
		}
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		switch (param)
		{
			case 1: 
				{
					if(type == 705 && accomplished_705)
					{
						return;
					}
					if(type == 705)
					{
						accomplished_705 = true;
					}
					if(type == 701)
					{
						accomplished_701 = true;
					}
					Commands->Clear_Radar_Marker (type);
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_ACCOMPLISHED);
					Remove_Pog(type);
					if(type == 703)
					{
						Commands->Mission_Complete ( true );
					}
					
				}
				break;
			case 2:
				{
					if(type == 702 && accomplished_701)
					{
						return;
					}
					Commands->Clear_Radar_Marker (type);
					Commands->Set_Objective_Status(type, OBJECTIVE_STATUS_FAILED);
					Remove_Pog(type);
					if(type == 701)
					{
						Commands->Mission_Complete ( false );
					}
					if(type == 702)
					{
						Commands->Mission_Complete ( false );
					}
					if(type == 709)
					{
						Commands->Mission_Complete ( false );
					}
					if(type == 710)
					{
						Commands->Mission_Complete ( false );
					}
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
	}

		
	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		if(timer_id == INITIAL_CONV)
		{
			// How are Dr. Mobius and Sydney?\n
			const char *conv_name = ("M07_CON001");
			int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN - 5, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(GUNNER, conv_id, true, true, true);
			Commands->Start_Conversation (conv_id, 300701);
			Commands->Monitor_Conversation (obj, conv_id);	
		}
		if(timer_id == PROTECT_TEAM)
		{
			Add_An_Objective(710);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if(action_id == 300701 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Add_An_Objective(701);

			Commands->Start_Timer (obj, this, 2.0f, PROTECT_TEAM);
			
			// Start nuclear strike countdown
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100663), M07_GO_EVAC_SITE, 1, 0.0f);
			// Customs to move team to evac site
			// Gunner
			Commands->Send_Custom_Event(obj, GUNNER, M07_GO_ASSEMBLY, 1, 0.0f);
			// Deadeye
			Commands->Send_Custom_Event(obj, DEADEYE, M07_GO_ASSEMBLY, 1, 0.0f);
			// Sydney
			Commands->Send_Custom_Event(obj, SYDNEY, M07_GO_ASSEMBLY, 1, 0.0f);
			// Hotwire
			Commands->Send_Custom_Event(obj, HOTWIRE, M07_GO_ASSEMBLY, 1, 2.0f);
			// Patch
			Commands->Send_Custom_Event(obj, PATCH, M07_GO_ASSEMBLY, 1, 2.0f);
		}
	}
};

//Grants initial weapons
DECLARE_SCRIPT(M07_Havoc_DLS, "")
{
	bool nuke_blast;

	enum {HAVOC_ALARM_SOUND};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		nuke_blast = true;
		Commands->Give_PowerUp(obj, "POW_Chaingun_Player", false);
		Commands->Give_PowerUp(obj, "POW_SniperRifle_Player", false);
		Commands->Give_PowerUp(obj, "POW_RocketLauncher_Player", false);
		Commands->Give_PowerUp(obj, "POW_MineRemote_Player", false);
	}

	void Custom( GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "DEATH", NULL);
			}
		} 
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		if(enemy == Commands->Find_Object(100801))
		{
			// You've got to be kidding.\n
			const char *conv_name = ("M07_CON015");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300707);
			Commands->Monitor_Conversation (obj, conv_id);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300702 && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			// Defeat Stealth Tank.
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 707, 3, 0.0f);
			// Change soundtrack to attack
			Commands->Set_Background_Music ("Raveshaw_Act on Instinct");
		}
	}

	void Killed (GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Mission_Complete(false);
	}

};

DECLARE_SCRIPT(M07_Dead6_Minigunner, "")  // Deadeye
{

	bool nuke_blast;
	float health;
	float shield;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
		SAVE_VARIABLE( health, 2 );
		SAVE_VARIABLE( shield, 3 );
	}

	void Created(GameObject * obj) override
	{
		nuke_blast = true;
		Commands->Set_Innate_Is_Stationary(obj, true);
		health = Commands->Get_Max_Health(obj);
		shield = Commands->Get_Max_Shield_Strength(obj);
		Commands->Enable_Hibernation(obj, false);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_GO_ASSEMBLY ) 
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, M07_GO_ASSEMBLY );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 102495;
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_GO_EVAC_SITE ) 
		{
			Commands->Apply_Damage( obj, -10000.0f, "DEATH", NULL);

			Commands->Set_Innate_Is_Stationary(obj, false);
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}
		if ( type == M07_CAPTURE_SAM ) 
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), M07_CAPTURE_SAM);
			params.Set_Movement (Commands->Find_Object(100682), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
			Commands->Enable_Hibernation(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == M07_GO_EVAC_SITE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 999999.0f);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_GO_EVAC_SITE);
		}
		if (action_id == M07_CAPTURE_SAM && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == obj || damager == GUNNER || damager == PATCH || damager == HOTWIRE)
		{
			Commands->Set_Health(obj, health);
			Commands->Set_Shield_Strength(obj, shield);
		}
		else
		{
			health = Commands->Get_Health(obj);
			shield = Commands->Get_Shield_Strength(obj);
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Deadeye Killed
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0178I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Dead6_Rocket_Soldier, "")  // Gunner
{

	bool nuke_blast;
	float health;
	float shield;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
		SAVE_VARIABLE( health, 2 );
		SAVE_VARIABLE( shield, 3 );
	}

	void Created(GameObject * obj) override
	{
		nuke_blast = true;
		Commands->Set_Innate_Is_Stationary(obj, true);
		health = Commands->Get_Max_Health(obj);
		shield = Commands->Get_Max_Shield_Strength(obj);
		Commands->Enable_Hibernation(obj, false);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_GO_ASSEMBLY ) 
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, M07_GO_ASSEMBLY );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 102479;
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_GO_EVAC_SITE ) 
		{
			Commands->Set_Innate_Is_Stationary(obj, false);
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}
		if ( type == M07_CAPTURE_SAM ) 
		{
			Commands->Apply_Damage( obj, -10000.0f, "DEATH", NULL);

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), M07_CAPTURE_SAM);
			params.Set_Movement (Commands->Find_Object(100681), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
			Commands->Enable_Hibernation(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == M07_GO_EVAC_SITE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 999999.0f);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_GO_EVAC_SITE);
		}
		if (action_id == M07_CAPTURE_SAM && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == obj || damager == DEADEYE || damager == PATCH || damager == HOTWIRE)
		{
			Commands->Set_Health(obj, health);
			Commands->Set_Shield_Strength(obj, shield);
		}
		else
		{
			health = Commands->Get_Health(obj);
			shield = Commands->Get_Shield_Strength(obj);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Gunner Killed
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0176I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Dead6_Grenadier, "")  // Patch
{

	bool nuke_blast;
	float health;
	float shield;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
		SAVE_VARIABLE( health, 2 );
		SAVE_VARIABLE( shield, 3 );
	}

	void Created(GameObject * obj) override
	{
		nuke_blast = true;
		Commands->Set_Innate_Is_Stationary(obj, true);
		health = Commands->Get_Max_Health(obj);
		shield = Commands->Get_Max_Shield_Strength(obj);
		Commands->Enable_Hibernation(obj, false);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_GO_ASSEMBLY ) 
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, M07_GO_ASSEMBLY );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 102495;
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_GO_EVAC_SITE ) 
		{
			Commands->Apply_Damage( obj, -10000.0f, "DEATH", NULL);

			Commands->Set_Innate_Is_Stationary(obj, false);
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}
		if ( type == M07_CAPTURE_SAM ) 
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_CAPTURE_SAM);
			params.Set_Movement (SYDNEY, RUN, 3.0f);
			params.MoveFollow = true;
			Commands->Action_Goto (obj, params);
			Commands->Enable_Hibernation(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == M07_GO_EVAC_SITE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if(timer_id == M07_CAPTURE_SAM)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_CAPTURE_SAM);
			params.Set_Movement (SYDNEY, RUN, 3.0f);
			params.MoveFollow = true;
			Commands->Action_Goto (obj, params);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 999999.0f);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_GO_EVAC_SITE);
		}
		if (action_id == M07_CAPTURE_SAM && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_CAPTURE_SAM);
		}
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == obj || damager == DEADEYE || damager == GUNNER || damager == HOTWIRE)
		{
			Commands->Set_Health(obj, health);
			Commands->Set_Shield_Strength(obj, shield);
		}
		else
		{
			health = Commands->Get_Health(obj);
			shield = Commands->Get_Shield_Strength(obj);
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Patch Killed
		Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0092I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Dead6_Grenadier2, "")  // Patch2
{

	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 103837;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == WAYPATH)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}
		
	}

	void Damaged( GameObject * /* obj */, GameObject * /* damager */, float /* amount */ ) override 
	{
		

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Patch Killed
		Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0092I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Dead6_MiniGunner2, "")  // Deadeye2
{
	enum{WAYPATH};
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 103837;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == WAYPATH)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Deadeye Killed
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0178I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}

};

DECLARE_SCRIPT(M07_Dead6_Rocket_Soldier2, "")  // Gunner2
{

	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 103837;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == WAYPATH)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Gunner Killed
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0176I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}

};

DECLARE_SCRIPT(M07_Sydney2, "")  
{
	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( nuke_blast, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
		params.WaypathID = 103837;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason /* reason */) override
	{
		if (action_id == WAYPATH)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Set_HUD_Help_Text ( IDS_M11DSGN_DSGN1008I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 701, 2, 1.0f);
		
	}

};

DECLARE_SCRIPT(M07_Dead6_Engineer, "")  // Hotwire
{

	bool nuke_blast;
	bool go_evac_site;
	bool dont_move;
	int current_move_loc;
	int move_loc[10];
	bool evac;
	float health;
	float shield;

	enum{GO_SAM1, ATTACK_SAM1, GO_SAM2, ATTACK_SAM2, 
		HOTWIRE_MOVE_LOC, MOVE_LOC_LOW_PRIORITY, TANK_STILL_THERE, ARRIVE_EVAC_SPOT};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
		SAVE_VARIABLE( go_evac_site, 2 );
		SAVE_VARIABLE( dont_move, 3 );
		SAVE_VARIABLE( current_move_loc, 4 );
		SAVE_VARIABLE( evac, 5 );
		SAVE_VARIABLE( health, 6 );
		SAVE_VARIABLE( shield, 7 );
		SAVE_VARIABLE( move_loc, 8 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);

		nuke_blast = true;
		go_evac_site = false;
		Commands->Set_Innate_Is_Stationary(obj, true);
		dont_move = true;
		evac = false;
		current_move_loc = 0;
		move_loc[0] = 102469;
		move_loc[1] = 102470;
		move_loc[2] = 102471;
		move_loc[3] = 102472;
		move_loc[4] = 102473;
		move_loc[5] = 102474;
		move_loc[6] = 102475;
		move_loc[7] = 102476;
		move_loc[8] = 102477;
		move_loc[9] = 102478;

		health = Commands->Get_Max_Health(obj);
		shield = Commands->Get_Max_Shield_Strength(obj);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if (type == M07_EVAC_INN) //SAM sites captured, evac DEAD6 and Sydney
		{
			evac = true;
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), ARRIVE_EVAC_SPOT);
			params.Set_Movement (Commands->Find_Object(105151), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_GO_ASSEMBLY ) 
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, M07_GO_ASSEMBLY );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 102510;
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_GO_EVAC_SITE ) 
		{
			Commands->Apply_Damage( obj, -10000.0f, "DEATH", NULL);

			Commands->Set_Innate_Is_Stationary(obj, false);
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}

		if ( type == M07_CAPTURE_SAM ) 
		{
			go_evac_site = true;

		//	params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), M07_CAPTURE_SAM);
		//	params.Set_Movement (STAR, RUN, 3.0f);
		//	params.MoveFollow = true;
		//	Commands->Action_Goto (obj, params);

			dont_move = false;

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), HOTWIRE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(move_loc[current_move_loc]), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_HOTWIRE_CAPTURE_SAMS && (!Commands->Find_Object(100905))) 
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);
			if(Commands->Find_Object(100674))
			{
				Commands->Set_Innate_Is_Stationary(obj, false);
				dont_move = true;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_SAM1 );
				params.Set_Movement (Commands->Find_Object(100674), RUN, 10.0f);
				Commands->Action_Goto( obj, params );

				GameObject * inn_loc = Commands->Find_Object(100718);
				Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(inn_loc), 999999.0f);
			}

			else if(Commands->Find_Object(100673))
			{
				Commands->Set_Innate_Is_Stationary(obj, false);
				dont_move = true;
				params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_SAM2 );
				params.Set_Movement (Commands->Find_Object(100673), RUN, 10.0f);
				Commands->Action_Goto( obj, params );

				GameObject * inn_loc = Commands->Find_Object(100718);
				Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(inn_loc), 999999.0f);
			}
		}

		if ( type == M07_HOTWIRE_CAPTURE_SAMS && (Commands->Find_Object(100905)))
		{
			Commands->Start_Timer(obj, this, 5.0f, TANK_STILL_THERE);

		}

		if(type == M07_MOVE_HOTWIRE && !dont_move)
		{
			current_move_loc = param;

			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), HOTWIRE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(move_loc[current_move_loc]), RUN, 3.0f);
			Commands->Action_Goto (obj, params);
		}

	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == M07_GO_EVAC_SITE && !go_evac_site && !evac)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if(timer_id == ATTACK_SAM1 && !evac)
		{
			// Custom to change SAMs playertype to GDI
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100674), M07_CHANGE_SAM_TEAM, 1, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100679), M07_CHANGE_SAM_TEAM, 1, 0.0f);

			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_SAM2 );
			params.Set_Movement (Commands->Find_Object(100673), RUN, 10.0f);
			Commands->Action_Goto( obj, params );

			
		}
		if(timer_id == ATTACK_SAM2 && !evac)
		{
			// Custom to change SAMs playertype to GDI
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100673), M07_CHANGE_SAM_TEAM, 1, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100680), M07_CHANGE_SAM_TEAM, 1, 0.0f);

			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);
			Commands->Select_Weapon(obj, NULL );
			
		}

		if(timer_id == MOVE_LOC_LOW_PRIORITY)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), HOTWIRE_MOVE_LOC);
			params.Set_Movement (Commands->Find_Object(move_loc[current_move_loc]), RUN, 3.0f);
			Commands->Action_Goto (obj, params);

		}

		if(timer_id == TANK_STILL_THERE)
		{
			Commands->Send_Custom_Event (obj, obj, M07_HOTWIRE_CAPTURE_SAMS, 1, 0.0f);

		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if (action_id == ARRIVE_EVAC_SPOT && reason == ACTION_COMPLETE_NORMAL && evac)
		{
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}

		if (action_id == ARRIVE_EVAC_SPOT && reason == ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE && evac)
		{
			// This is a hack for the moment, need to have pathfinding around dec_phys vehicles at inn
			int dead6_id = Commands->Get_ID(obj);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, dead6_id, 0.0f);
		}

		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_NORMAL && !evac)
		{
			go_evac_site = true;
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 999999.0f);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_LOW_PRIORITY && !evac)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_GO_EVAC_SITE);
		}
		
		if (action_id == GO_SAM1 && reason == ACTION_COMPLETE_NORMAL && !evac)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100 );
			params.Set_Attack (Commands->Find_Object(100679), 250.0f, 0.0f, 0);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer(obj, this, 6.0f, ATTACK_SAM1);
		}
		if (action_id == GO_SAM1 && reason == ACTION_COMPLETE_LOW_PRIORITY && !evac)
		{
			// Keep trying to GO_SAM1
			Commands->Send_Custom_Event (obj, HOTWIRE, M07_HOTWIRE_CAPTURE_SAMS, 1, 2.0f);
		}
		if (action_id == GO_SAM2 && reason == ACTION_COMPLETE_NORMAL && !evac)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100 );
			params.Set_Attack (Commands->Find_Object(100680), 250.0f, 0.0f, 0);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer(obj, this, 6.0f, ATTACK_SAM2);
		}

		if(action_id == HOTWIRE_MOVE_LOC && reason == ACTION_COMPLETE_NORMAL && !evac)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 4.0f);
		}
		if(action_id == HOTWIRE_MOVE_LOC && reason == ACTION_COMPLETE_LOW_PRIORITY && !evac)
		{
			Commands->Start_Timer(obj, this, 4.0f, MOVE_LOC_LOW_PRIORITY);
		}

		
	}

	void Destroyed(GameObject * obj) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_DEAD6_EVAC, 1, 0.0f);
		
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL);
			}
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == obj || damager == DEADEYE || damager == GUNNER || damager == HOTWIRE)
		{
			Commands->Set_Health(obj, health);
			Commands->Set_Shield_Strength(obj, shield);
		}
		else
		{
			health = Commands->Get_Health(obj);
			shield = Commands->Get_Shield_Strength(obj);
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Hotwire Killed
		Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0174I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Sydney, "")  
{
	bool nuke_blast;
	float health;
	float shield;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( nuke_blast, 1 );
		SAVE_VARIABLE( health, 2 );
		SAVE_VARIABLE( shield, 3 );
	}

	void Created(GameObject * obj) override
	{
		nuke_blast = true;
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Enable_Hibernation(obj, false);
		health = Commands->Get_Max_Health(obj);
		shield = Commands->Get_Max_Shield_Strength(obj);
		// Give Sydney initial pistol
		Commands->Give_PowerUp(obj, "POW_Pistol_AI", false);
		Commands->Select_Weapon(obj, "Weapon_Pistol_Ai" );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_GO_ASSEMBLY ) 
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, M07_GO_ASSEMBLY );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 102479;
			Commands->Action_Goto (obj, params);
		}
		if(type == M07_GO_EVAC_SITE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
		if ( type == M07_NUKE_BLAST ) 
		{
			nuke_blast = (param == 1) ? true : false;
		}

		if ( type == M07_CAPTURE_SAM ) 
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN + 5), M07_CAPTURE_SAM);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 3.0f);
			Commands->Action_Goto (obj, params);

			Commands->Enable_Hibernation(obj, true);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == M07_CAPTURE_SAM && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Soldier_Home_Location(obj, Commands->Get_Position(obj), 999999.0f);
		}
		if (action_id == M07_GO_EVAC_SITE && reason == ACTION_COMPLETE_LOW_PRIORITY)
		{
			Commands->Start_Timer(obj, this, 6.0f, M07_GO_EVAC_SITE);
		}
		
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_NUKE_IMPACT)
		{
			if(nuke_blast)
			{
				Commands->Apply_Damage( obj, 10000.0f, "STEEL", NULL);
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == M07_GO_EVAC_SITE)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), M07_GO_EVAC_SITE);
			params.Set_Movement (Commands->Find_Object(100664), RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == obj || damager == DEADEYE || damager == GUNNER || damager == HOTWIRE || damager == PATCH)
		{
			Commands->Set_Health(obj, health);
			Commands->Set_Shield_Strength(obj, shield);
		}
		else
		{
			health = Commands->Get_Health(obj);
			shield = Commands->Get_Shield_Strength(obj);
		}

	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Sydney Killed
		Commands->Set_HUD_Help_Text ( IDS_M11DSGN_DSGN1008I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 701, 2, 1.0f);
		
	}
};

DECLARE_SCRIPT(M07_Cathedral_Controller, "")  // 100663
{

	enum{TWO_MIN, ONE_MIN, THIRTY, TWENTY, TEN, FIVE, FOUR, THREE, TWO, ONE, ESCAPED, NUKEBITS};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( alarmed, 1 );
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_GO_EVAC_SITE ) 
		{
			// Impact: 2 Minutes.\n
			const char *conv_name = ("M07_CON003");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 60.0f, TWO_MIN);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == TWO_MIN)
		{
			// Impact: 1 Minute.\n
			const char *conv_name = ("M07_CON004");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 30.0f, ONE_MIN);
		}
		if(timer_id == ONE_MIN)
		{
			// Impact: 30 seconds.\n
			const char *conv_name = ("M07_CON005");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 10.0f, THIRTY);
		}
		if(timer_id == THIRTY)
		{
			// Impact: 20 seconds.\n
			const char *conv_name = ("M07_CON006");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 10.0f, TWENTY);
		}
		if(timer_id == TWENTY)
		{
			// Impact: 10 seconds.\n
			const char *conv_name = ("M07_CON007");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 5.0f, TEN);
		}
		if(timer_id == TEN)
		{
			// Impact: 5 seconds.\n
			const char *conv_name = ("M07_CON008");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 1.0f, FIVE);
		}
		if(timer_id == FIVE)
		{
			GameObject * cinematic_controller = Commands->Create_Object ( "Invisible_Object", Vector3(-100.879f, 88.570f, 29.184f));
			Commands->Attach_Script(cinematic_controller, "Test_Cinematic", "XG_NukeStrike.txt");

			// 4 seconds.\n
			const char *conv_name = ("M07_CON009");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 1.0f, FOUR);
		}
		if(timer_id == FOUR)
		{
			// 3 seconds.\n
			const char *conv_name = ("M07_CON010");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 1.0f, THREE);
		}
		if(timer_id == THREE)
		{
			// 2 seconds.\n
			const char *conv_name = ("M07_CON011");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 1.0f, TWO);

			
		}
		if(timer_id == TWO)
		{
			// 1 seconds.\n
			const char *conv_name = ("M07_CON012");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			Commands->Start_Timer(obj, this, 1.0f, ONE);

			Commands->Set_Clouds (1.0f, 1.0f, 3.0f);
		}
		if(timer_id == ONE)
		{
			// Send customs to Nuke Blast Radius determination zones to deactivate them
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100665), M07_DEACTIVATE_NUKE_BLAST, 1, 0.0f);
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100666), M07_DEACTIVATE_NUKE_BLAST, 1, 0.0f);
			// Boom!
			Commands->Create_Logical_Sound(obj, M07_NUKE_IMPACT, Commands->Get_Position(obj), 2500.0f);
			Commands->Set_Ash(0.15f, 3.0f, false);
			Commands->Set_Wind(90.0f, 5.0f, 2.0f, 0.0f);
			
			Commands->Start_Timer(obj, this, 2.0f, NUKEBITS);
		}
		if(timer_id == NUKEBITS)
		{
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(119890)), NULL); 
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(119891)), NULL); 
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(119892)), NULL); 
			Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(Commands->Find_Object(119893)), NULL); 
			GameObject * nukebits = Commands->Create_Object("M07_Nukebits", Vector3(-128.295f, 71.892f, 26.754f));
			Commands->Set_Facing( nukebits, -10.000f );
			Commands->Start_Timer(obj, this, 3.0f, ESCAPED);
		}
		if(timer_id == ESCAPED)
		{
			if(SYDNEY)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 1, 0.0f);
			}
		}
	}
};

DECLARE_SCRIPT(M07_In_Nuke_Blast, "")
{
		
	bool already_entered;
	bool star_in_blast;
	bool active;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( star_in_blast, 2 );
		SAVE_VARIABLE( active, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		star_in_blast = true;
		active = true;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == M07_NUKE_BLAST ) 
		{
			star_in_blast = false;
			
		}
		if ( type == M07_DEACTIVATE_NUKE_BLAST ) 
		{
			active = false;
			
		}
	}


	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if(!active)
		{
			return;
		}
		// Send to enterer to let script know in blast zone
		Commands->Send_Custom_Event (obj, enterer, M07_NUKE_BLAST, 1, 0.0f);
		if(!star_in_blast && enterer == STAR)
		{
			star_in_blast = true;

			// Send custom to in blast script to let it know player is out of blast
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100666), M07_NUKE_BLAST, 1, 0.0f);

			// Warning - Your are within the Nuclear Strike blast radius. Evacuate immediately.\n
			const char *conv_name = ("M07_CON013");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);
		}

	}
};

DECLARE_SCRIPT(M07_Out_Nuke_Blast, "")
{
		
	bool already_entered;
	bool star_in_blast;
	bool active;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( star_in_blast, 2 );
		SAVE_VARIABLE( active, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
		star_in_blast = true;
		active = true;
	}

	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == M07_NUKE_BLAST ) 
		{
			star_in_blast = true;
		}
		if ( type == M07_DEACTIVATE_NUKE_BLAST ) 
		{
			active = false;
			
		}
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if(!active)
		{
			return;
		}
		// Send to enterer to let script know out blast zone
		Commands->Send_Custom_Event (obj, enterer, M07_NUKE_BLAST, 0, 0.0f);
		if(star_in_blast && enterer == STAR)
		{
			star_in_blast = false;

			// Minimum safe distance achieved.\n
			const char *conv_name = ("M07_CON014");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 10);

			// Send custom to in blast script to let it know player is out of blast
			Commands->Send_Custom_Event (obj, Commands->Find_Object(100665), M07_NUKE_BLAST, 1, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Nod_Gun_Emplacement, "")
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

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Custom to evac site controller to count when four vehicles in area destroyed
		Commands->Send_Custom_Event (obj, Commands->Find_Object(100664), M07_EVAC_SITE_VEHICLE_KILLED, 1, 0.0f);
		Commands->Debug_Message("Evac Site Vehicle Killed = %d \n", Commands->Get_ID(obj));
	}
	
};

DECLARE_SCRIPT(M07_Evac_Site_Controller, "")  // 100664
{
	int m07_evac_site_vehicle_killed;

	enum{WHERE_EVAC};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_evac_site_vehicle_killed, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_evac_site_vehicle_killed = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == M07_EVAC_SITE_VEHICLE_KILLED)
		{
			
			m07_evac_site_vehicle_killed++;
			Commands->Debug_Message("Evac Site Vehicle Killed = %d \n", m07_evac_site_vehicle_killed);
			if(m07_evac_site_vehicle_killed == 4)
			{
				Commands->Debug_Message("Evac Site is clear.  Play Locke, where evac conversation");
				Commands->Start_Timer (obj, this, 4.0f, WHERE_EVAC);
			}
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		
		if(timer_id == WHERE_EVAC)
		{
			// Locke, what's my evac status?\n
			const char *conv_name = ("M07_CON002");
			int conv_id = Commands->Create_Conversation (conv_name, INNATE_PRIORITY_ENEMY_SEEN + 5, 2000.0f, false);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(GUNNER, conv_id, false, true, true);
			Commands->Join_Conversation(HOTWIRE, conv_id, false, true, true);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300702);
			Commands->Monitor_Conversation (obj, conv_id);
		}
		
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300702 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			// Activate Hotwire to SAM sites primary objective
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 702, 3, 0.0f);

			// Customs to position team for Primary MO 702
			// Hotwire
			Commands->Send_Custom_Event(obj, HOTWIRE, M07_CAPTURE_SAM, 1, 0.0f);
			// Patch
			Commands->Send_Custom_Event(obj, PATCH, M07_CAPTURE_SAM, 1, 0.0f);
			// Gunner
			Commands->Send_Custom_Event(obj, GUNNER, M07_CAPTURE_SAM, 1, 0.0f);
			// Deadeye
			Commands->Send_Custom_Event(obj, DEADEYE, M07_CAPTURE_SAM, 1, 0.0f);
			// Sydney
			Commands->Send_Custom_Event(obj, SYDNEY, M07_CAPTURE_SAM, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Evac_Site_APC, "")  
{	
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Custom to evac site controller to count when four vehicles in area destroyed
		Commands->Send_Custom_Event (obj, Commands->Find_Object(100664), M07_EVAC_SITE_VEHICLE_KILLED, 1, 0.0f);
		Commands->Debug_Message("Evac Site Vehicle Killed = %d \n", Commands->Get_ID(obj));
	}

};


DECLARE_SCRIPT(M07_Evac_Site_APC_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		apc_id = Get_Int_Parameter("APC_ID");	

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 1 );
		params.Set_Movement( STAR, RUN, 20.0f );
		Commands->Action_Goto (obj, params);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Evac_Site_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		apc_id = Get_Int_Parameter("APC_ID");	

		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Activate_Hotwire, "")
{
	bool already_entered;
	
	enum{HOTWIRE_CAPTURE_SAMS};

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
		if(enterer == HOTWIRE && !already_entered)
		{
			already_entered = true;
			Commands->Enable_Spawner(100715, true);

			// Keep those Nod forces off my back, Havoc.  I'll crack these SAM's in no time.\n
			const char *conv_name = ("M07_CON016");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(HOTWIRE, conv_id, true, true, true);
			Commands->Start_Conversation (conv_id, HOTWIRE_CAPTURE_SAMS);
			Commands->Monitor_Conversation(obj, conv_id);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == HOTWIRE_CAPTURE_SAMS && reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			Commands->Send_Custom_Event (obj, HOTWIRE, M07_HOTWIRE_CAPTURE_SAMS, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Fancy_Inn_Sam, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_CHANGE_SAM_TEAM)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100685), M07_SAM_CONVERTED, 1, 0.0f);
		}

	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100685), M07_SAM_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_SAM_Truck_100680, "")
{
	float health;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 1 );
	}

	void Created (GameObject * obj) override
	{
		health = Commands->Get_Max_Health(obj);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_CHANGE_SAM_TEAM)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		}

	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
	}

	void Killed(GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Apply_Damage( Commands->Find_Object(100680), 10000.0f, "DEATH", NULL);
		
	}
};

DECLARE_SCRIPT(M07_SAM_Truck_100679, "")
{
	float health;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 1 );
	}

	void Created (GameObject * obj) override
	{
		health = Commands->Get_Max_Health(obj);
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_CHANGE_SAM_TEAM)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		}

	}

	void Damaged( GameObject * obj, GameObject * /* damager */, float /* amount */ ) override 
	{
		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
	}

	void Killed(GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Apply_Damage( Commands->Find_Object(100679), 10000.0f, "DEATH", NULL);
	
	}
};

DECLARE_SCRIPT(M07_Fancy_Inn_Controller, "")
{
	int m07_sam_converted;
	int m07_sam_killed;

	enum{CONTROL_SAMS, DESTROY_SYDNEY, DROP_DEAD6_INN};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_sam_converted, 1 );
		SAVE_VARIABLE( m07_sam_killed, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_sam_converted = 0;
		m07_sam_killed = 0;
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M07_SAM_CONVERTED)
		{
			m07_sam_converted++;
			if(m07_sam_converted == 1)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 702, 1, 0.0f);
				Commands->Enable_Spawner(100715, false);
				Commands->Send_Custom_Event(obj, Commands->Find_Object(104496), M07_EVAC_INN, 1, 0.0f);
				Commands->Start_Timer(obj, this, 3.0f, CONTROL_SAMS);

				// Spawn DEAD6 and Sydney to evacuate
				Commands->Enable_Spawner(103833, true);
				Commands->Enable_Spawner(103834, true);
				Commands->Enable_Spawner(103835, true);
				Commands->Enable_Spawner(103836, true);

				// Destroy the initial group of people
				Commands->Destroy_Object(Commands->Find_Object(100659));
				Commands->Destroy_Object(Commands->Find_Object(100660));
				Commands->Destroy_Object(Commands->Find_Object(100661));
				Commands->Destroy_Object(Commands->Find_Object(100662));
			
				// Grant health increase medal
				Commands->Create_Object ( "POW_Medal_Health", Vector3(-45.888f, -73.569f, 4.222f));
				// Grant armor increase medal
				Commands->Create_Object ( "POW_Medal_Armor", Vector3(-43.445f, -73.384f, 4.222f));
			}
		}
		if(type == M07_SAM_KILLED)
		{
			m07_sam_killed++;
			if(m07_sam_killed == 2)
			{
				// SAMs Killed
				Commands->Set_HUD_Help_Text ( IDS_M07DSGN_DSGN0090I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
				// Objective failed if both SAMs destroyed
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 702, 2, 1.0f);
			}
		}

	}

	
};

DECLARE_SCRIPT(M07_Inn_Apache, "")
{
	bool attacking;

	enum{WAYPATH, INN_PATROL, ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		attacking = false;
		Commands->Disable_Physical_Collisions(obj);

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 100707;
		params.MovePathfind = false;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if (action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Enemy_Seen(obj, true);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INN_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 100687;
			params.MovePathfind = false;
			Commands->Action_Attack( obj, params );
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INN_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 100687;
			params.MovePathfind = false;
			Commands->Modify_Action (obj, INN_PATROL, params, true, true);

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

DECLARE_SCRIPT(M07_Activate_Objective_704, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			// Attention GDI, this is Resistance Radio. Nod's aerial reinforcements are coordinated from mobile radar stations in the Town Square.  We need those radar dishes destroyed.\n
			const char *conv_name = ("M07_CON018");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300704);
			Commands->Monitor_Conversation (obj, conv_id);

			// Engineers in Town Square
			Commands->Enable_Spawner(100726, true);
			Commands->Enable_Spawner(100727, true);

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100746), M07_TOWNSQUARE_CHINOOK, 1, 0.0f);
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300704 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 704, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Activate_Objective_705, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			// This is Resistance Radio, I have reports of captured resistance fighters west of your position.  Can anyone assist?\n
			const char *conv_name = ("M07_CON019");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300705);
			Commands->Monitor_Conversation (obj, conv_id);

			
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300705 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 705, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Activate_Objective_706, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			// Havoc, we've spotted an SSM napalm stockpile to your west.\n
			const char *conv_name = ("M07_CON020");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300706);
			Commands->Monitor_Conversation (obj, conv_id);

			
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300706 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 706, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Activate_Objective_707, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			// Those must be the SSMs. Time to get to work.\n
			const char *conv_name = ("M07_CON021");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300707);
			Commands->Monitor_Conversation (obj, conv_id);

			
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300707 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 707, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Activate_Objective_708, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			// \\Warning - Nod Obelisk detected.\n
			const char *conv_name = ("M07_CON022");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300708);
			Commands->Monitor_Conversation (obj, conv_id);

			
		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300708 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 708, 3, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Activate_Objective_710, "")
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
		if ( Commands->Is_A_Star(enterer) ) 
		{
			already_entered = true;
			
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 710, 3, 0.0f);
		}

	}
};

DECLARE_SCRIPT(M07_SAM_Site_Logic, "")
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

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == M00_CUSTOM_SAM_SITE_IGNORE && current < 10)
		{
			ignore_ids[current++] = param;
		}
	}
};

DECLARE_SCRIPT(M07_Destroyed_SAM_Site, "")
{
	void Created(GameObject * obj) override
	{
		Commands->Start_Timer(obj, this, 1.0f, 0);
	}

	void Timer_Expired(GameObject * /* obj */, int /* timer_id */) override
	{
		Commands->Create_2D_Sound("00-N170E");
	}
};

DECLARE_SCRIPT(M07_Mobile_Radar, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation(obj, "v_nod_radar.v_nod_radar", true, NULL, 0.0f, -1.0f, false);
	}

	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		if(Commands->Get_Health(obj) < Commands->Get_Max_Health(obj))
		{
			Commands->Create_Logical_Sound(obj, M07_RADAR_DAMAGED, Commands->Get_Position(obj), 10.0f);
		}
		else
		{
			Commands->Create_Logical_Sound(obj, M07_RADAR_FIXED, Commands->Get_Position(obj), 10.0f);
		}
			
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(obj), NULL); 
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100746), M07_RADAR_KILLED, 0, 0.0f);
	}
};

DECLARE_SCRIPT(M07_Radar_Engineer, "Radar_ID=0:int")
{
	
	enum{GO_RADAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen(obj, false);
		
		int radar_id = Get_Int_Parameter("Radar_ID");

		ActionParamsStruct params;
		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_RADAR);
		params.Set_Movement(Commands->Find_Object(radar_id), RUN, 1.0f);
		Commands->Action_Goto(obj, params);
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		ActionParamsStruct params;

		if (sound.Type == M07_RADAR_DAMAGED)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100 );
			params.Set_Attack (sound.Creator, 250.0f, 0.0f, 0);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
		if (sound.Type == M07_RADAR_FIXED)
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == GO_RADAR && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}
};

DECLARE_SCRIPT(M07_TownSquare_Unit, "Spawn_ID=0:int, Waypath_ID=0:int")
{
	
	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		int waypath_id = Get_Int_Parameter("Waypath_ID");

		if(waypath_id == 0)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
		else
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Goto( obj, params );
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Killed(GameObject * /* obj */, GameObject * /* killer */) override
	{
		int spawn_id = Get_Int_Parameter("Spawn_ID");
		Commands->Enable_Spawner(spawn_id, true);
	}
};

DECLARE_SCRIPT(M07_TownSquare_Controller, "")  // 
{
	bool chinooks_active;
	int m07_radar_killed;

	enum{CHINOOK1, CHINOOK2};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( chinooks_active, 1 );
		SAVE_VARIABLE( m07_radar_killed, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		chinooks_active = false;
		m07_radar_killed = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == M07_TOWNSQUARE_CHINOOK)
		{
			if(param == 1)
			{
				chinooks_active = true;

				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-316.374f, -149.140f, 5.023f));
				Commands->Set_Facing(chinook_obj1, 95.000f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_EV3_1.txt");

				Commands->Start_Timer(obj, this, 20.0f, CHINOOK1);
				Commands->Start_Timer(obj, this, 10.0f, CHINOOK2);
			}
			if(param == 0)
			{
				chinooks_active = false;
			}


		}
		if (type == M07_RADAR_KILLED)
		{
			m07_radar_killed++;
			if(m07_radar_killed == 2)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 704, 1, 0.0f);
			}

		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == CHINOOK1 && chinooks_active)
		{
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-316.374f, -149.140f, 5.023f));
			Commands->Set_Facing(chinook_obj1, 95.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_EV3_1.txt");

			Commands->Start_Timer(obj, this, 20.0f, CHINOOK1);
		}
		if(timer_id == CHINOOK2 && chinooks_active)
		{
			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-343.696f, -145.495f, 5.136f));
			Commands->Set_Facing(chinook_obj2, -55.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "M07_XG_EV3_1.txt");

			Commands->Start_Timer(obj, this, 20.0f, CHINOOK2);
		}
	}
};

DECLARE_SCRIPT(M07_Deactivate_TownSquare_Chinook, "")
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
		if (Commands->Is_A_Star(enterer) && !already_entered ) 
		{
			already_entered = true;
			
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100746), M07_TOWNSQUARE_CHINOOK, 0, 0.0f);
		}

	}

};

DECLARE_SCRIPT(M07_TownSquare_Gun_Emp, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Killed(GameObject * /* obj */, GameObject * /* killer */) override
	{
		Commands->Enable_Spawner(100748, true);
	}
};

DECLARE_SCRIPT(M07_Prisoner_Gate, "")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "cht_jail.cht_jail", 0);
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(115842), M07_FREE_CIV, 1, 0.0f);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100751), M07_FREE_CIV, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M07_Captured_Civ_Resist, "")
{

	bool poked;
	bool freed;

	

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );
		SAVE_VARIABLE( freed, 2 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Innate_Is_Stationary(obj, true);
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
		poked = false;
		freed = false;
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if ((Commands->Is_A_Star(poker)) && (!poked) && (!freed))
		{
			Commands->Create_Logical_Sound(obj, M07_FREE_CIV_RESIST, Commands->Get_Position(obj), 40.0f);
		}
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_FREE_CIV_RESIST)
		{
			freed = true;
			Commands->Enable_HUD_Pokable_Indicator( obj, false );
			Commands->Set_Innate_Is_Stationary(obj, false);
			Commands->Give_PowerUp(obj, "POW_Chaingun_AI", false);
			Commands->Select_Weapon(obj, "Weapon_Chaingun_Ai" );
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 705, 1, 0.0f);

			ActionParamsStruct params;
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, 10);
			params.Set_Movement(Commands->Find_Object(115823), RUN, 1.0f);
			Commands->Action_Goto(obj, params);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		// Spawn reinforcment civ resist
		Commands->Send_Custom_Event(obj, Commands->Find_Object(101056), M07_SPAWN_TRIANGLE_CIV, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M07_Activate_Present, "")
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
		if ( Commands->Is_A_Star(enterer) && !already_entered) 
		{
			already_entered = true;

			// Havoc, got a present for ya.\n
			const char *conv_name = ("M07_CON023");
			int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
			Commands->Join_Conversation(NULL, conv_id, false, true, true);
			Commands->Join_Conversation(STAR, conv_id, false, true, true);
			Commands->Start_Conversation (conv_id, 300700);
						
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-446.944f, -25.105f, 14.134f));
			Commands->Set_Facing(chinook_obj1, 155.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_VehicleDrop1.txt");
		}

	}
};

DECLARE_SCRIPT(M07_Player_Vehicle, "")
{
	float health;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		health = Commands->Get_Max_Health(obj);
		int vehicle_id = Commands->Get_ID(obj);
		// Let enemy tank drop controller know player vehicle id
		Commands->Send_Custom_Event(obj, Commands->Find_Object(101131), M07_PLAYER_VEHICLE_ID, vehicle_id, 0.0f);
	}

	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}

	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100757), M07_PLAYER_VEHICLE_KILLED, 1, 0.0f);
	}
	
};

DECLARE_SCRIPT(M07_Vehicle_Drop_Controller, "")
{
		
	int vehicle_drop[8];
	int drop_zone;
	int m07_player_vehicle_killed;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( vehicle_drop, 1 );
		SAVE_VARIABLE( drop_zone, 2 );
		SAVE_VARIABLE( m07_player_vehicle_killed, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		
		vehicle_drop[0] = 100755;
		vehicle_drop[1] = 100758;
		vehicle_drop[2] = 100759;
		vehicle_drop[3] = 100760;
		vehicle_drop[4] = 100761;
		vehicle_drop[5] = 100762;
		vehicle_drop[6] = 100768;
		vehicle_drop[7] = 100769;

		drop_zone = 0;
		m07_player_vehicle_killed = 0;
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == M07_VEHICLE_DROP_ZONE)
		{
			
			if(param == 10 && m07_player_vehicle_killed < 7)
			{
				// Havoc!  Taking missile fire over the park.  Launcher is unseen.  Repeat, launcher is unseen.\n
				const char *conv_name = ("M07_CON024");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Join_Conversation(STAR, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 300700);
			}
			else
			{
				drop_zone = param;
			}

		}
		if (type == M07_PLAYER_VEHICLE_KILLED)
		{
			m07_player_vehicle_killed++;
			if(m07_player_vehicle_killed < 4)
			{
				if(m07_player_vehicle_killed < 3)
				{
					// Got you another ride, inbound now!\n
					const char *conv_name = ("M07_CON025");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(NULL, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300700);

					GameObject * drop_loc = Commands->Find_Object(vehicle_drop[drop_zone]);
					float drop_facing = Commands->Get_Facing(drop_loc);
				
					GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
					Commands->Set_Facing(chinook_obj1, drop_facing);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_VehicleDrop1.txt");
				}
				else
				{
					// Complements of the local resistance.\n
					const char *conv_name = ("M07_CON026");
					int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
					Commands->Join_Conversation(NULL, conv_id, false, true, true);
					Commands->Join_Conversation(STAR, conv_id, false, true, true);
					Commands->Start_Conversation (conv_id, 300700);

					GameObject * drop_loc = Commands->Find_Object(vehicle_drop[drop_zone]);
					float drop_facing = Commands->Get_Facing(drop_loc);
				
					GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
					Commands->Set_Facing(chinook_obj1, drop_facing);
					Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_VehicleDrop4.txt");
				}

				
			}
			else
			{
				// Last one, Captain.  Recon show Nod's pulling back to the park.\n
				const char *conv_name = ("M07_CON027");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Join_Conversation(STAR, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 300700);

				GameObject * drop_loc = Commands->Find_Object(vehicle_drop[drop_zone]);
				float drop_facing = Commands->Get_Facing(drop_loc);
			
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
				Commands->Set_Facing(chinook_obj1, drop_facing);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_VehicleDrop3.txt");
			}
			

		}
	}

};

DECLARE_SCRIPT(M07_Vehicle_Drop_Zone, "Zone_ID=0:int")
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

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		int zone_id = Get_Int_Parameter("Zone_ID");

		if(Commands->Is_A_Star(enterer) && !already_entered)
		{
			if(zone_id == 10)
			{
				already_entered = true;
			}

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100757), M07_VEHICLE_DROP_ZONE, zone_id, 0.0f);
		}

	}
};

DECLARE_SCRIPT(M07_Biohazard_Barrel, "")
{

	enum{BOOM};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( vehicle_drop, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "o_barrl_bio.o_barrl_bio", 0);
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(obj), NULL);
		
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100781), M07_EXPLODE_BARRELS, 1, 0.0f);

	}
	
};

DECLARE_SCRIPT(M07_Stockpile_Object, "")
{

	bool damaged;

	enum{BOOM, STILL_DAMAGED};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( damaged, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		damaged = false;
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		if (sound.Type == M07_EXPLODE_BARRELS)
		{
			Commands->Apply_Damage( obj, 100000.0f, "STEEL", NULL);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == STILL_DAMAGED && damaged)
		{
			Commands->Create_Logical_Sound(obj, M07_ENGINEER_DAMAGED, Commands->Get_Position(obj), 40.0f);
			Commands->Start_Timer(obj, this, 8.0f, STILL_DAMAGED);
			damaged = true;
		}

	}

	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		if(Commands->Get_Health(obj) < Commands->Get_Max_Health(obj))
		{
			Commands->Create_Logical_Sound(obj, M07_ENGINEER_DAMAGED, Commands->Get_Position(obj), 40.0f);
			Commands->Start_Timer(obj, this, 8.0f, STILL_DAMAGED);
			damaged = true;
		}
		else
		{
			damaged = false;
			Commands->Create_Logical_Sound(obj, M07_ENGINEER_FIXED, Commands->Get_Position(obj), 40.0f);
		}
			
	}

};

DECLARE_SCRIPT(M07_Stockpile_Controller, "")
{

	int ssm_crate_killed;

	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( ssm_crate_killed, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		Commands->Enable_Spawner(100795, true);
		ssm_crate_killed = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == M07_SSM_CRATE_KILLED)
		{
			ssm_crate_killed++;
			if(ssm_crate_killed == 4)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 706, 1, 0.0f);
			}
		}
	}

};

DECLARE_SCRIPT(M07_Stockpile_Engineer, "")
{
	
	enum{GO_STOCKPILE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen(obj, false);
		
		ActionParamsStruct params;
		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_STOCKPILE);
		params.Set_Movement(Commands->Find_Object(100781), RUN, 1.0f);
		Commands->Action_Goto(obj, params);
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		ActionParamsStruct params;

		if (sound.Type == M07_ENGINEER_DAMAGED)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100 );
			params.Set_Attack (sound.Creator, 250.0f, 0.0f, 0);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
		if (sound.Type == M07_ENGINEER_FIXED)
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == GO_STOCKPILE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}
};

DECLARE_SCRIPT(M07_Park_Controller, "")
{
	int m07_artillery_killed;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_artillery_killed, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_artillery_killed = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M07_ARTILLERY_KILLED)
		{
			m07_artillery_killed++;
			if(m07_artillery_killed == 2)
			{
				// Destroy Artillery Piece Objective
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 703, 1, 5.0f);
			}
		}
		
	}

};

DECLARE_SCRIPT(M07_Park_SSM, "")
{

	enum{STILL_DAMAGED};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( destroy_stockpile, 1 );
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == STILL_DAMAGED)
		{
			if(Commands->Get_Health(obj) < Commands->Get_Max_Health(obj))
			{
				Commands->Create_Logical_Sound(obj, M07_SSM_DAMAGED, Commands->Get_Position(obj), 1000.0f);
				Commands->Start_Timer(obj, this, 10.0f, STILL_DAMAGED);
			}
			else
			{
				Commands->Create_Logical_Sound(obj, M07_SSM_FIXED, Commands->Get_Position(obj), 1000.0f);
			}

			
		}
	}

	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		if(Commands->Get_Health(obj) < Commands->Get_Max_Health(obj))
		{
			Commands->Create_Logical_Sound(obj, M07_SSM_DAMAGED, Commands->Get_Position(obj), 1000.0f);
		}
		else
		{
			Commands->Create_Logical_Sound(obj, M07_SSM_FIXED, Commands->Get_Position(obj), 1000.0f);
		}
			
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100799), M07_ARTILLERY_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_Nod_Obelisk, "")
{
	int obelisk_weapon_id;
	bool converted;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( obelisk_weapon_id, 1 );
		SAVE_VARIABLE( converted, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		obelisk_weapon_id = 0;
		converted = false;
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == M07_PLAYERTYPE_CHANGE_OBELISK)
		{
			converted = true;
			Commands->Set_Player_Type(Commands->Find_Object(obelisk_weapon_id), SCRIPT_PLAYERTYPE_GDI );
			
		}
		if(type == M00_OBELISK_WEAPON_ID)
		{
			obelisk_weapon_id = param;
			
		}
		
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(!converted)
		{
			// Custom to Objective_Controller that Obelisk has been destroyed
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 708, 1, 0.0f);
			// Custom to Obelisk Poke to turn it off so that it cannot be converted
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100805), M07_OBELISK_KILLED, 1, 0.0f);
		}
	}
	
};

DECLARE_SCRIPT(M07_Nod_Obelisk_MCT, "")
{
	bool poked;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );
	}

	void Created (GameObject * obj) override
	{
		poked = false;
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		if ((Commands->Is_A_Star(poker)) && (!poked))
		{
			Commands->Enable_HUD_Pokable_Indicator( obj, false );
			Commands->Create_Sound("Computer_Operate_02", Commands->Get_Position(obj), obj);
			poked = true;
			// Change playertype of Nod Obelisk
			Commands->Send_Custom_Event(obj, Commands->Find_Object(168926), M07_PLAYERTYPE_CHANGE_OBELISK, 1, 0.0f);

			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 708, 1, 0.0f);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M07_OBELISK_KILLED)
		{
			Commands->Enable_HUD_Pokable_Indicator( obj, false );
			poked = true;
		}
		
	}

};


DECLARE_SCRIPT(M07_Park_Zone, "Zone=0:int")
{
		
	bool already_entered;
			
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if ((Commands->Is_A_Star(enterer)))
		{
						
			int activate_zone = Get_Int_Parameter("Zone");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100801), M07_MOVE_STEALTH_TANK, activate_zone, 0.0f);

		}
	}

};


DECLARE_SCRIPT(M07_Activate_Encounter, "Spawner_ID1=0:int, Spawner_ID2=0:int, Spawner_ID3=0:int, Spawner_ID4=0:int, Spawner_ID5=0:int, Spawner_ID6=0:int")
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
		if (type == M07_DEACTIVATE_ENCOUNTER)
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

DECLARE_SCRIPT(M07_Deactivate_Encounter, "Activate_Zone=0:int")
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
			
			int activate_zone = Get_Int_Parameter("Activate_Zone");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(activate_zone), M07_DEACTIVATE_ENCOUNTER, 1, 0.0f);

		}
	}

};



DECLARE_SCRIPT(M07_Encounter_Unit, "Waypath_ID=0:int, Priority=0:int, Suicide=0:int, Stationary_at_End=0:int")
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

DECLARE_SCRIPT(M07_V05_Controller, "")
{
	bool encounter;
	int m07_v05_units;
	bool reinforce;

	enum{REINFORCEMENT_CHECK};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( encounter, 1 );
		SAVE_VARIABLE( m07_v05_units, 2 );
		SAVE_VARIABLE( reinforce, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_v05_units = 0;
		
		encounter= true;
		reinforce = true;
	}

	void Custom(GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M07_REINFORCEMENT_UNIT_KILLED)
		{
			m07_v05_units--;
			
		}
		if(type == M07_REINFORCEMENT_UNIT_CREATED)
		{
			m07_v05_units++;
		}

		if (type == M07_DEACTIVATE_ENCOUNTER)
		{
			encounter = false;
		}

		if (type == M07_ACTIVATE_ENCOUNTER)
		{
			Commands->Start_Timer(obj, this, 0.0f, REINFORCEMENT_CHECK);
		}

		if (type == M07_V05_OFFICER_KILLED)
		{
			reinforce = false;
		}
		
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == REINFORCEMENT_CHECK && encounter)
		{
			if(m07_v05_units < 3 && reinforce)
			{
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(30.873f, -9.825f, 5.346f));
				Commands->Set_Facing(chinook_obj1, -80.000f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7I_TroopDrop1.txt");
			}
		}

		Commands->Start_Timer(obj, this, 30.0f, REINFORCEMENT_CHECK);
	}

	
};

DECLARE_SCRIPT(M07_V05_Officer, "")
{
	
	enum{REINFORCEMENT_CHECK};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( encounter, 1 );
//		SAVE_VARIABLE( m07_v05_units, 2 );
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100899), M07_V05_OFFICER_KILLED, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M07_V05_Unit, "Stationary=0:int")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poke_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		bool stationary = (Get_Int_Parameter("Stationary") == 1) ? true : false;
		if(stationary)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}

		Commands->Send_Custom_Event(obj, Commands->Find_Object(100899), M07_REINFORCEMENT_UNIT_CREATED, 1, 2.0f);
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100899), M07_REINFORCEMENT_UNIT_KILLED, 1, 2.0f);
	}
};

DECLARE_SCRIPT(M07_Activate_V05, "")
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

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		{
			if(Commands->Is_A_Star(enterer) && !already_entered)
			{
				already_entered = true;
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100899), M07_ACTIVATE_ENCOUNTER, 1, 0.0f);				
			}

		}
	}

	
};

DECLARE_SCRIPT(M07_Nod_Light_Tank_Dec, "Attack_ID0=0:int, Attack_ID1=0:int, Attack_ID2=0:int")
{
	bool attacking;
	int attack_id[3];
	
	enum{ATTACK_OVER, ATTACK_LOC};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( attack_id, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NOD );
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = false;

		attack_id[0] = Get_Int_Parameter("Attack_ID0");
		attack_id[1] = Get_Int_Parameter("Attack_ID1");
		attack_id[2] = Get_Int_Parameter("Attack_ID2");

		if(attack_id[0] != 0)
		{
			int random = Commands->Get_Random_Int(0,2); 

			while(attack_id[random] == 0)
			{
				random = Commands->Get_Random_Int(0,2); 
			}

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (Commands->Find_Object(attack_id[random]), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer(obj, this, 6.0f, ATTACK_LOC);
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
			params.AttackCheckBlocked = true;
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
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );
			
			Commands->Start_Timer(obj, this, 6.0f, ATTACK_OVER);
		}

	}

	void Timer_Expired (GameObject* obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

		}
		if(timer_id == ATTACK_LOC && !attacking)
		{
			
			int random = Commands->Get_Random_Int(0,2); 

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, 1 );
			params.Set_Attack (Commands->Find_Object(attack_id[random]), 250.0f, 0.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack( obj, params );

			Commands->Start_Timer(obj, this, 6.0f, ATTACK_LOC);
		}
	}
	
};

DECLARE_SCRIPT(M07_Nod_Inn_Light_Tank_Dec, "APC_ID=0:int")
{
	bool attacking;
	int apc_id;
	
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
		apc_id = Get_Int_Parameter("APC_ID");	
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

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
	}
	
};

DECLARE_SCRIPT(M07_Inn_APC, "")  
{
	bool attacking;
	int reinforce;
	
	enum{DROP_SOLDIERS, CHECK_ENEMY};
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
		reinforce = 0;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (enemy, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 30.0f, CHECK_ENEMY);
		}
	}

	

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_REINFORCEMENT_KILLED)
		{
			reinforce++;
			if(reinforce < 7)
			{
				char param1[12];
				sprintf(param1, "%d", Commands->Get_ID(obj));

				Vector3 pos = Commands->Get_Position(obj);
				float facing = Commands->Get_Facing(obj);
				float a = cos(DEG_TO_RADF(facing)) * -4.0;
				float b = sin(DEG_TO_RADF(facing)) * -4.0;
			
				Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
				GameObject * soldier1 = Commands->Create_Object("M07_Nod_APC", soldier_loc1);
				Commands->Attach_Script(soldier1, "M07_Inn_APC_Soldier", param1);

			}
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

DECLARE_SCRIPT(M07_Inn_APC_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		apc_id = Get_Int_Parameter("APC_ID");	

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 1 );
		params.Set_Movement( STAR, RUN, 20.0f );
		Commands->Action_Goto (obj, params);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Inn_Balcony_Sniper, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		apc_id = Get_Int_Parameter("APC_ID");	

		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};


DECLARE_SCRIPT(M07_Activate_V01, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			

			Commands->Send_Custom_Event (obj, Commands->Find_Object(100952), M07_ACTIVATE_V01, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_Deactivate_V01, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			

			Commands->Send_Custom_Event (obj, Commands->Find_Object(100953), M07_DEACTIVATE_V01, 1, 0.0f);
		}
	}
};

DECLARE_SCRIPT(M07_V01_Controller, "")
{
	bool activated;
	int m07_v01_unit_killed;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( activated, 1 );
		SAVE_VARIABLE( m07_v01_unit_killed, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		activated = false;	
		m07_v01_unit_killed = 0;
	}
	void Custom (GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_ACTIVATE_V01 && !activated)
		{
			activated = true;	

			Commands->Enable_Spawner(100956, true);
			Commands->Enable_Spawner(100962, true);

			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-193.706f, -82.114f, 4.987f));
			Commands->Set_Facing(chinook_obj1, 20.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop2.txt");

		}

		if(type == M07_V01_UNIT_KILLED && activated)
		{
			m07_v01_unit_killed++;
			if(m07_v01_unit_killed%2 == 0)
			{
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-193.706f, -82.114f, 4.987f));
				Commands->Set_Facing(chinook_obj1, 20.000f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop2.txt");
			}

		}

		if(type == M07_DEACTIVATE_V01 && activated)
		{
			activated = false;	

			Commands->Enable_Spawner(100956, false);
			Commands->Enable_Spawner(100962, false);

		}

		
	}

	
};

DECLARE_SCRIPT(M07_V01_Destroyed_Flame_Tank, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_ACTIVATE_V01)
		{
			Commands->Apply_Damage( obj, 10000.0f, "DEATH", NULL);

		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(100953), M07_ACTIVATE_V01, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_Nod_Buggy_Dec, "")
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

DECLARE_SCRIPT(M07_Nod_Recon_Bike_Dec, "")
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

DECLARE_SCRIPT(M07_V01_Unit, "")
{
	
	enum{GO_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		
		Commands->Start_Timer(obj, this, 3.0f, 10);
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;


		if(timer_id == GO_STAR)
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN, GO_STAR );
			params.Set_Movement( STAR, RUN, 2.0f );
			Commands->Action_Goto (  obj, params );
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(100953), M07_V01_UNIT_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_Activate_Triangle_Apache, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			
			Commands->Enable_Spawner(101010, true);
			
		}
	}
};

DECLARE_SCRIPT(M07_Triangle_Apache, "")
{
	bool attacking;
	int curr_waypath;

	enum{WAYPATH, TRIANGLE_PATROL, ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( curr_waypath, 2 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		attacking = false;
		curr_waypath = 101011;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 101011;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if (action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Enemy_Seen(obj, true);
			curr_waypath = 101016;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, TRIANGLE_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = curr_waypath;
			Commands->Action_Attack( obj, params );
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, TRIANGLE_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = curr_waypath;
			Commands->Modify_Action (obj, TRIANGLE_PATROL, params, true, true);

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

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		ActionParamsStruct params;

		if (sound.Type == M07_RELOCATE_TRIANGLE_APACHE)
		{
			curr_waypath = 101033;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, TRIANGLE_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = curr_waypath;
			Commands->Action_Attack( obj, params );
		}
	}
	
};


DECLARE_SCRIPT(M07_Relocate_Triangle_Apache, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			
			Commands->Create_Logical_Sound(obj, M07_RELOCATE_TRIANGLE_APACHE, Commands->Get_Position(obj), 100.0f);
			
		}
	}
};

DECLARE_SCRIPT(M07_Triangle_APC, "")  
{
	
	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(101056), M07_TRIANGLE_APC_KILLED, 1, 0.0f);
	}

};

DECLARE_SCRIPT(M07_Triangle_APC_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		apc_id = Get_Int_Parameter("APC_ID");	

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 1 );
		params.Set_Movement( STAR, RUN, 20.0f );
		Commands->Action_Goto (obj, params);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Triangle_Guard, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		apc_id = Get_Int_Parameter("APC_ID");	

		Commands->Set_Innate_Is_Stationary(obj, true);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Triangle_Controller, "")
{
	int m07_triangle_apc_killed;
	int m07_triangle_unit_killed;
	int civ_captive_killed;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_triangle_apc_killed, 1 );
		SAVE_VARIABLE( m07_triangle_unit_killed, 2 );
		SAVE_VARIABLE( civ_captive_killed, 3 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_triangle_apc_killed = 0;
		m07_triangle_unit_killed = 0;
		civ_captive_killed = 0;
	}

	void Custom (GameObject * obj, int type, intptr_t param, GameObject * /* sender */) override
	{
		if(type == M07_TRIANGLE_APC_KILLED)
		{
			m07_triangle_apc_killed++;
			if(m07_triangle_apc_killed == 2)
			{
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-531.675f, -87.333f, 13.908f));
				Commands->Set_Facing(chinook_obj1, -70.000f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop3.txt");
			}
		}

		if(type == M07_TRIANGLE_UNIT_KILLED)
		{
			m07_triangle_unit_killed++;
			if(m07_triangle_unit_killed%3 == 0 && m07_triangle_unit_killed < 9)
			{
				GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-531.675f, -87.333f, 13.908f));
				Commands->Set_Facing(chinook_obj1, -70.000f);
				Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop3.txt");
			}
		}
		if(type == M07_SPAWN_TRIANGLE_CIV)
		{
			civ_captive_killed++;
			if(civ_captive_killed == 2)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 705, 2, 0.0f);
			}
			switch(param)
			{
			case 1:
				{
					Commands->Enable_Spawner(101057, true);
				}
				break;
			case 2:
				{
					Commands->Enable_Spawner(101058, true);
				}
				break;
			}
		}
		
	}

};

DECLARE_SCRIPT(M07_Triangle_Unit, "")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Send_Custom_Event (obj, Commands->Find_Object(101056), M07_TRIANGLE_UNIT_KILLED, 1, 0.0f);
		
	}
};

DECLARE_SCRIPT(M07_Activate_Hostage_Encounter, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			// Hostage
			Commands->Enable_Spawner(101066, true);

			// Hostage Takers
			Commands->Enable_Spawner(101073, true);
			Commands->Enable_Spawner(101074, true);
			Commands->Enable_Spawner(101079, true);
			
		}
	}
};

DECLARE_SCRIPT(M07_Fuel_Barrel, "")
{

	enum{BOOM};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( vehicle_drop, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Animation_Frame(obj, "o_barrl_fuel.o_barrl_fuel", 0);
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Ground Explosions Twiddler", Commands->Get_Position(obj), NULL);
		
		Commands->Send_Custom_Event(obj, Commands->Find_Object(101059), M07_EXPLODE_BARRELS, 1, 0.0f);

	}
	
};

DECLARE_SCRIPT(M07_Hostage_Controller, "")
{

	bool destroy_stockpile;

	enum{BOOM};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( destroy_stockpile, 1 );
	}

	void Created (GameObject * /* obj */) override
	{
		destroy_stockpile = false;
		Commands->Enable_Spawner(100795, true);
	}

	void Custom(GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if (type == M07_EXPLODE_BARRELS && destroy_stockpile == false)
		{
			destroy_stockpile = true;
			
			Commands->Apply_Damage( Commands->Find_Object(101060), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101061), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101062), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101063), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101064), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101065), 10000.0f, "STEEL", NULL);
			Commands->Apply_Damage( Commands->Find_Object(101084), 10000.0f, "STEEL", NULL);
			
			
		}
	}

};

DECLARE_SCRIPT(M07_Hostage, "")
{
	bool poked;


	enum{WAYPATH, SWITCH_TYPE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( poked, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 101067;
		Commands->Action_Goto( obj, params );

		Commands->Start_Timer(obj, this, 6.0f, SWITCH_TYPE);
		poked = false;
	}

	void Poked(GameObject * obj, GameObject * poker) override
	{
		ActionParamsStruct params;

		if ((Commands->Is_A_Star(poker)) && (!poked))
		{
			poked = true;

			// Assume hands over head anim
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1);
			params.Set_Animation( "H_A_HOST_L1C", false );
			Commands->Action_Play_Animation (obj, params);

			Commands->Set_Innate_Is_Stationary(obj, false);

			Vector3 pos = Commands->Get_Position(obj);
			float facing = Commands->Get_Facing(obj);
			float a = cos(DEG_TO_RADF(facing)) * 1.5;
			float b = sin(DEG_TO_RADF(facing)) * 1.5;
			Vector3 powerup_loc = pos + Vector3(a, b, 0.5f);
			Commands->Create_Object("Ramjet_Weapon_Powerup", powerup_loc);

		}
	}

	


	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if (reason != ACTION_COMPLETE_NORMAL) 
		{
			return;
		}
		if (action_id == WAYPATH)
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
			
			// Assume hands over head anim
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1);
			params.Set_Animation( "H_A_HOST_L1B", true );
			Commands->Action_Play_Animation (obj, params);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == SWITCH_TYPE)
		{
			Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_GDI );
		}
	}
};

DECLARE_SCRIPT(M07_Hostage_Taker, "Waypath_ID=0:int")
{

	bool destroy_stockpile;

	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( destroy_stockpile, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		int waypath_id = Get_Int_Parameter("Waypath_ID");

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = waypath_id;
		Commands->Action_Goto( obj, params );
	}

	
};

DECLARE_SCRIPT(M07_APC_Dec, "")  
{
	bool attacking;
	int reinforce;
	
	enum{DROP_SOLDIERS, CHECK_ENEMY};
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
		reinforce = 0;
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 1 );
			params.Set_Attack (enemy, 200.0f, 5.0f, 1);
			params.AttackCheckBlocked = true;
			Commands->Action_Attack (obj, params);

			Commands->Start_Timer (obj, this, 30.0f, CHECK_ENEMY);
		}
	}

	

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		ActionParamsStruct params;

		if(type == M07_REINFORCEMENT_KILLED)
		{
			reinforce++;
			if(reinforce < 7)
			{
				char param1[12];
				sprintf(param1, "%d", Commands->Get_ID(obj));

				Vector3 pos = Commands->Get_Position(obj);
				float facing = Commands->Get_Facing(obj);
				float a = cos(DEG_TO_RADF(facing)) * -4.0;
				float b = sin(DEG_TO_RADF(facing)) * -4.0;
			
				Vector3 soldier_loc1 = pos + Vector3(a, b + .5f, 0.5f);
				GameObject * soldier1 = Commands->Create_Object("M07_Nod_APC", soldier_loc1);
				Commands->Attach_Script(soldier1, "M07_APC_Dec_Soldier", param1);

			}
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

DECLARE_SCRIPT(M07_APC_Dec_Soldier, "APC_ID=0:int")
{

	int apc_id;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		apc_id = Get_Int_Parameter("APC_ID");	

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN - 5, 1 );
		params.Set_Movement( STAR, RUN, 20.0f );
		Commands->Action_Goto (obj, params);
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		if(Commands->Find_Object(apc_id))
		{
			Commands->Send_Custom_Event (obj, Commands->Find_Object(apc_id), M07_REINFORCEMENT_KILLED, apc_id, 0.0f);
		}
		
	}
};

DECLARE_SCRIPT(M07_Activate_Bridge_Civ, "")
{


	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			// Bridge Civilians
			Commands->Enable_Spawner(101109, true);
			Commands->Enable_Spawner(101114, true);
			// Bridge overlook Apache
			Commands->Enable_Spawner(109150, true);
		}
	}
};

DECLARE_SCRIPT(M07_Bridge_Civ, "Waypath_ID=0:int")
{

	enum{WAYPATH};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		int waypath_id = Get_Int_Parameter("Waypath_ID");

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 0.5f );
		params.WaypathID = waypath_id;
		Commands->Action_Goto( obj, params );
		

	}

	void Action_Complete(GameObject * obj, int /* action_id */, ActionCompleteReason /* reason */) override
	{
		

		Commands->Set_Innate_Is_Stationary(obj, true);
	
	}

	
};

DECLARE_SCRIPT(M07_Activate_E10_Tank_Drop, "")
{
	int m07_player_vehicle_id;
	bool already_entered;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_player_vehicle_id, 1 );
		SAVE_VARIABLE( already_entered, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		m07_player_vehicle_id = 0;
		already_entered = false;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * sender ) override
	{
		if ( type == M07_PLAYER_VEHICLE_ID ) 
		{
			m07_player_vehicle_id = param;
		}
		if ( type == M07_E10_TANK_CREATED ) 
		{
			Commands->Send_Custom_Event(obj, sender, M07_PLAYER_VEHICLE_ID, Commands->Get_ID(sender), 0.0f);
		}
	}

	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer) && !already_entered)
		{
			already_entered = true;

			GameObject * drop_loc = Commands->Find_Object(101132);
			float drop_facing = Commands->Get_Facing(drop_loc);
			
			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
			Commands->Set_Facing(chinook_obj1, drop_facing);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "M07_XG_VehicleDrop2.txt");
		}
	}
};

DECLARE_SCRIPT(M07_E10_Tank, "")
{
	int m07_player_vehicle_id;

	enum{GO_PLAYER_VEHICLE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( m07_player_vehicle_id, 1 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen(obj, true);
		m07_player_vehicle_id = 0;
		Commands->Send_Custom_Event(obj, Commands->Find_Object(101131), M07_E10_TANK_CREATED, 1, 0.0f);
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if ( type == M07_PLAYER_VEHICLE_ID ) 
		{
			m07_player_vehicle_id = param;
			Commands->Start_Timer(obj, this, 3.0f, 10);
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject * enemy) override
	{
		ActionParamsStruct params;

		if (Commands->Is_A_Star(enemy))
		{
			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_PLAYER_VEHICLE);
			params.Set_Attack(Commands->Find_Object(m07_player_vehicle_id), 40.0f, 1.0f, true);
			params.AttackCheckBlocked = false;
			params.MoveFollow = true;
			Commands->Modify_Action (obj, GO_PLAYER_VEHICLE, params, true, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		if(timer_id == GO_PLAYER_VEHICLE)
		{
			ActionParamsStruct params;

			params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN + 5, GO_PLAYER_VEHICLE);
			params.Set_Attack(Commands->Find_Object(m07_player_vehicle_id), 40.0f, 1.0f, true);
			params.AttackCheckBlocked = false;
			params.MoveFollow = true;
			Commands->Action_Attack(obj, params);
		}
	}
};

DECLARE_SCRIPT(M07_Para_Drop_Controller, "")
{
		
	int para_drop[7];
	int drop_zone;
	int m07_paradrop_unit1_killed;
	int m07_paradrop_unit2_killed;
	int m07_paradrop_unit3_killed;
	int m07_paradrop_unit4_killed;
	bool active;

	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( para_drop, 1 );
		SAVE_VARIABLE( drop_zone, 2 );
		SAVE_VARIABLE( m07_paradrop_unit1_killed, 3 );
		SAVE_VARIABLE( m07_paradrop_unit2_killed, 4 );
		SAVE_VARIABLE( m07_paradrop_unit3_killed, 5 );
		SAVE_VARIABLE( m07_paradrop_unit4_killed, 6 );
		SAVE_VARIABLE( active, 7 );
	}

	void Created (GameObject * /* obj */) override
	{
		
		para_drop[0] = 101134;
		para_drop[1] = 101138;
		para_drop[2] = 101139;
		para_drop[3] = 101140;
		para_drop[4] = 101141;
		para_drop[5] = 101142;
		para_drop[6] = 101143;
		
		
		drop_zone = 0;
		m07_paradrop_unit1_killed = 0;
		m07_paradrop_unit2_killed = 0;
		m07_paradrop_unit3_killed = 0;
		m07_paradrop_unit4_killed = 0;

		active = true;
		
	}

	void Custom(GameObject * /* obj */, int type, intptr_t param, GameObject * /* sender */) override
	{
		if (type == M07_DEACTIVATE_PARADROP)
		{
			
			active = false;
			
		}
		if (type == M07_VEHICLE_DROP_ZONE)
		{
			
			drop_zone = param;
			
		}
		if(type == M07_PARADROP_UNIT_KILLED && active)
		{
			switch(param)
			{
			case 1:
				{

					m07_paradrop_unit1_killed++;
					if(m07_paradrop_unit1_killed%3 == 0 && m07_paradrop_unit1_killed < 9)
					{
						GameObject * drop_loc = Commands->Find_Object(para_drop[drop_zone]);
						float drop_facing = Commands->Get_Facing(drop_loc);
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop4.txt");
					}
				}
				break;
			case 2:
				{

					m07_paradrop_unit2_killed++;
					if(m07_paradrop_unit2_killed < 2)
					{
						
						GameObject * drop_loc = Commands->Find_Object(109837);
						float drop_facing = Commands->Get_Facing(drop_loc);
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop5.txt");
					}
				}
				break;
			case 3:
				{

					m07_paradrop_unit3_killed++;
					if(m07_paradrop_unit3_killed < 2)
					{
						
						GameObject * drop_loc = Commands->Find_Object(109838);
						float drop_facing = Commands->Get_Facing(drop_loc);
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop6.txt");
					}
				}
				break;
			case 4:
				{

					m07_paradrop_unit4_killed++;
					if(m07_paradrop_unit4_killed < 2)
					{
						
						GameObject * drop_loc = Commands->Find_Object(109839);
						float drop_facing = Commands->Get_Facing(drop_loc);
						
						GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Commands->Get_Position(drop_loc));
						Commands->Set_Facing(chinook_obj1, drop_facing);
						Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop7.txt");
					}
				}
				break;
			}
		}

		
	}

};

DECLARE_SCRIPT(M07_Para_Drop_Zone, "Zone_ID=0:int")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		int zone_id = Get_Int_Parameter("Zone_ID");

		if(Commands->Is_A_Star(enterer))
		{
			Commands->Send_Custom_Event(obj, Commands->Find_Object(101133), M07_VEHICLE_DROP_ZONE, zone_id, 0.0f);
		}

	}
};

DECLARE_SCRIPT(M07_Para_Drop_Unit, "Unit_ID=0:int")
{

	enum{GO_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		if(Get_Int_Parameter("Unit_ID") == 1)
		{
			Commands->Start_Timer(obj, this, 3.0f, GO_STAR);
		}
		else
		{
			Commands->Set_Innate_Is_Stationary(obj, true);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id) override
	{
		ActionParamsStruct params;

		if(timer_id == GO_STAR)
		{
			params.Set_Basic (this, (INNATE_PRIORITY_ENEMY_SEEN - 5), 10);
			params.Set_Movement (STAR, RUN, 10.0f);
			Commands->Action_Goto (obj, params);
		}
	}

	void Killed(GameObject * obj, GameObject * /* killer */) override
	{
		int unit_id = Get_Int_Parameter("Unit_ID");
		Commands->Send_Custom_Event(obj, Commands->Find_Object(101133), M07_PARADROP_UNIT_KILLED, unit_id, 0.0f);
	}
	
};

DECLARE_SCRIPT(M07_Activate_Para_Drop, "")
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

	void Entered(GameObject * /* obj */, GameObject * enterer) override
	{
		

		if(Commands->Is_A_Star(enterer) && !already_entered)
		{
			already_entered = true;

			GameObject * chinook_obj1 = Commands->Create_Object ( "Invisible_Object", Vector3(-417.090f, 39.921f, 20.435f));
			Commands->Set_Facing(chinook_obj1, 0.000f);
			Commands->Attach_Script(chinook_obj1, "Test_Cinematic", "X7D_CHTroopdrop4.txt");

			GameObject * chinook_obj2 = Commands->Create_Object ( "Invisible_Object", Vector3(-437.171f, 35.261f, 36.155f));
			Commands->Set_Facing(chinook_obj2, -5.000f);
			Commands->Attach_Script(chinook_obj2, "Test_Cinematic", "X7D_CHTroopdrop5.txt");

			GameObject * chinook_obj3 = Commands->Create_Object ( "Invisible_Object", Vector3(-474.645f, 36.897f, 32.859f));
			Commands->Set_Facing(chinook_obj3, 0.000f);
			Commands->Attach_Script(chinook_obj3, "Test_Cinematic", "X7D_CHTroopdrop6.txt");

			GameObject * chinook_obj4 = Commands->Create_Object ( "Invisible_Object", Vector3(-516.036f, 40.087f, 30.558f));
			Commands->Set_Facing(chinook_obj4, -5.000f);
			Commands->Attach_Script(chinook_obj4, "Test_Cinematic", "X7D_CHTroopdrop7.txt");

		}

	}
};

DECLARE_SCRIPT(M07_Deactivate_Para_Drop, "")
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

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		

		if(Commands->Is_A_Star(enterer) && !already_entered)
		{
			already_entered = true;

			Commands->Send_Custom_Event(obj, Commands->Find_Object(101133), M07_DEACTIVATE_PARADROP, 1, 0.0f);

		}

	}
};

DECLARE_SCRIPT(M07_Flyover_Controller, "")
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
			"X7A_CPlane_00.txt",
			"X7A_CPlane_01.txt",
			"X7A_CPlane_02.txt",
			"X7A_CPlane_03.txt",

			"X7A_Apache_00.txt",
			"X7A_Apache_01.txt",
			"X7A_Apache_02.txt",
			"X7A_Apache_03.txt",
			
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


DECLARE_SCRIPT(M07_Activate_Flyover, "Text_File:string")
{
	bool already_entered;

	enum {PLAY_AGAIN};

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(already_entered, 1);
	}
	
	void Created(GameObject * /* obj */) override
	{
		already_entered = false;
	}

	void Entered(GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer) && !already_entered)
		{
			already_entered = true;

			const char * flyover = Get_Parameter("Text_File");

			GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
			Commands->Attach_Script(controller, "Test_Cinematic", flyover);

			Commands->Start_Timer(obj, this, 10.0f, PLAY_AGAIN);
		}
	}

	void Timer_Expired(GameObject * /* obj */, int timer_id) override
	{
		if(timer_id == PLAY_AGAIN)
		{
			already_entered = false;
		}
	}
};

DECLARE_SCRIPT(M07_Disable_Hibernation, "")
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

};

DECLARE_SCRIPT(M07_Move_To_Evac, "")
{
		
	bool already_entered;
	bool star_in_blast;
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( already_entered, 1 );
		SAVE_VARIABLE( star_in_blast, 2 );
	}

	void Created (GameObject * /* obj */) override
	{
		already_entered = false;
	}


	void Entered (GameObject * obj, GameObject * enterer) override
	{
		if(Commands->Is_A_Star(enterer))
		{
			// Customs to move team to evac site
			// Havoc
			Commands->Send_Custom_Event(obj, STAR, M07_GO_EVAC_SITE, 1, 0.0f);
			// Hotwire
			Commands->Send_Custom_Event(obj, HOTWIRE, M07_GO_EVAC_SITE, 1, 0.0f);
			// Patch
			Commands->Send_Custom_Event(obj, PATCH, M07_GO_EVAC_SITE, 1, 0.0f);
			// Gunner
			Commands->Send_Custom_Event(obj, GUNNER, M07_GO_EVAC_SITE, 1, 0.0f);
			// Deadeye
			Commands->Send_Custom_Event(obj, DEADEYE, M07_GO_EVAC_SITE, 1, 0.0f);
			// Sydney
			Commands->Send_Custom_Event(obj, SYDNEY, M07_GO_EVAC_SITE, 1, 0.0f);
		}

	}
};

DECLARE_SCRIPT(M07_Move_Hotwire, "Move_Loc=0:int")
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
			int move_loc = Get_Int_Parameter("Move_Loc");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100658), M07_MOVE_HOTWIRE, move_loc, 0.0f);
		}
	}

};

DECLARE_SCRIPT(M07_Player_Rocket_Emplacement, "")
{
		
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		if(damager == Commands->Find_Object(100905))
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

};


DECLARE_SCRIPT(M07_Playertype_Neutral, "")
{
		
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );
		
	}

	void Created (GameObject * obj) override
	{
		Commands->Set_Player_Type(obj, SCRIPT_PLAYERTYPE_NEUTRAL );
	}

};


DECLARE_SCRIPT(M07_Inn_Evac, "")//104496
{
	
	int evac_waypath_id;
	int evac_helicopter_id;
	int dead6_cnt;
	int evac_rope_id;

	enum{LOWER_ROPE, CHINOOK_OUT, DESTROY_SYDNEY};

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(evac_waypath_id, 1);
		SAVE_VARIABLE(evac_helicopter_id, 2);
		SAVE_VARIABLE(dead6_cnt, 3);
		SAVE_VARIABLE(evac_rope_id, 4);
	}

	void Created( GameObject * /* obj */ ) override
	{
		dead6_cnt = 0;
		evac_waypath_id = 0;
		evac_helicopter_id = 0;
		evac_rope_id = 0;
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /* sender */ ) override
	{
		if (type == M07_DEAD6_EVAC) //SAM sites captured, evac DEAD6 and Sydney
		{

			char param1[12];
			sprintf(param1, "%d", (int)param);

			Vector3 evacPosition = Commands->Get_Position ( obj );
			float evacFacing = Commands->Get_Facing ( obj );
			GameObject *troopBone = Commands->Create_Object("Saveable_Invisible_Object", evacPosition );
			Commands->Set_Model ( troopBone, "XG_EV5_troopBN" );
			Commands->Set_Facing ( troopBone, evacFacing );
			Commands->Attach_Script(troopBone, "M07_Inn_Evac_Climb", param1);
			
			dead6_cnt++;
			if(dead6_cnt == 4)
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100658), M07_EVAC_INN, 1, 0.0f);

			}
			if(dead6_cnt == 5)
			{
				// This is Resistance Radio.  Nod SSM deployment confirmed.  All available personnel to the park.\n
				const char *conv_name = ("M07_CON017");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Join_Conversation(STAR, conv_id, false, true, true);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 300703);
				Commands->Monitor_Conversation (obj, conv_id);
				
				
			}
		}
		if (type == M07_EVAC_INN) //SAM sites captured, evac DEAD6 and Sydney
		{
			Vector3 evacPosition = Commands->Get_Position ( obj );
			float evacFacing = Commands->Get_Facing ( obj );
			GameObject *evacWaypath = Commands->Create_Object("Saveable_Invisible_Object", evacPosition );
			Commands->Attach_Script(evacWaypath, "M07_Inn_Evac_Trajectory", "");
			Commands->Set_Model ( evacWaypath, "XG_EV5_Path" );
			Commands->Set_Facing ( evacWaypath, evacFacing );
			Commands->Set_Animation ( evacWaypath, "XG_EV5_Path.XG_EV5_PathA", false, NULL, 0.0f, -1.0f, false );
			Commands->Start_Timer (obj, this, 130.0f/30.0f, LOWER_ROPE);
			evac_waypath_id = Commands->Get_ID(evacWaypath);
			
			GameObject *evacChopper = Commands->Create_Object_At_Bone( evacWaypath, "GDI_Transport_Helicopter_Flyover", "BN_Trajectory" );
			Commands->Attach_Script(evacChopper, "M07_Inn_Evac_Helicopter", "");
			Commands->Set_Animation ( evacChopper, "v_GDI_trnspt.XG_EV5_trnsA", false, NULL, 0.0f, -1.0f, false );
			Commands->Attach_To_Object_Bone( evacChopper, evacWaypath, "BN_Trajectory" );
			evac_helicopter_id = Commands->Get_ID ( evacChopper );

		}

	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if (action_id == 300703 && (reason == ACTION_COMPLETE_CONVERSATION_ENDED || reason == ACTION_COMPLETE_CONVERSATION_INTERRUPTED))
		{
			// Destroy Artillery Piece Objective
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 703, 3, 0.0f);
			Commands->Start_Timer (obj, this, 3.0f, CHINOOK_OUT);
			

			Commands->Start_Timer(obj, this, 10.0f, DESTROY_SYDNEY);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == DESTROY_SYDNEY)
		{
			// Once GDI evac has arrived destroy Sydney and DEAD-6 to give appearance of liftoff
			Commands->Destroy_Object(DEADEYE);
			Commands->Destroy_Object(GUNNER);
			Commands->Destroy_Object(PATCH);
			Commands->Destroy_Object(SYDNEY);

			

	//		Commands->Start_Timer(obj, this, 10.0f, DROP_DEAD6_INN);
		}
		if(timer_id == LOWER_ROPE)
		{
			Vector3 evacPosition = Commands->Get_Position ( obj );
			float evacFacing = Commands->Get_Facing ( obj );
			GameObject * evacRope = Commands->Create_Object("Saveable_Invisible_Object", evacPosition );
			Commands->Attach_Script(evacRope, "M07_Inn_Evac_Rope", "");
			Commands->Set_Model ( evacRope, "XG_EV5_rope" );
			Commands->Set_Facing ( evacRope, evacFacing );
			Commands->Set_Animation ( evacRope, "XG_EV5_rope.XG_EV5_ropeA", false, NULL, 0.0f, -1.0f, false );
			evac_rope_id = Commands->Get_ID ( evacRope );
		}

		if(timer_id == CHINOOK_OUT)
		{
			GameObject * evacChopper = Commands->Find_Object ( evac_helicopter_id );
			GameObject * evacWaypath = Commands->Find_Object ( evac_waypath_id );
			GameObject * evacRope = Commands->Find_Object ( evac_rope_id );

			if (evacChopper != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacChopper, M07_INN_HELICOPTER_OUT, 1, 0 );
			}

			if (evacWaypath != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacWaypath, M07_INN_HELICOPTER_OUT, 1, 0 );
			}

			if (evacRope != NULL)
			{
				Commands->Send_Custom_Event ( obj, evacRope, M07_INN_HELICOPTER_OUT, 1, 0 );
			}

			// Objective accomplished for rescuing Sydney
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 701, 1, 0.0f);
			// Objective accomplished for protecting team
			Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 709, 1, 1.0f);
		}
		
	}

	
};

DECLARE_SCRIPT(M07_Inn_Evac_Climb, "Dead6_ID=0:int")
{

	void Created( GameObject * obj ) override
	{
		int dead6_id = Get_Int_Parameter("Dead6_ID");
		GameObject * dead6 = Commands->Find_Object(dead6_id);
		Commands->Attach_Script(dead6, "M07_Climb_Rope", "");
		Commands->Attach_To_Object_Bone(dead6, obj, "Troop_L" );
		Commands->Set_Animation ( obj, "XG_EV5_troopBN.XG_EV5_troopBN", false, NULL, 0.0f, -1.0f, false );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_troopBN.XG_EV5_troopBN") == 0)
		{
			Commands->Destroy_Object ( obj );
		}

	}

};

DECLARE_SCRIPT(M07_Climb_Rope, "")
{
	enum{CLIMB_ROPE};
	
	void Created( GameObject * obj ) override
	{
		ActionParamsStruct params;

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, CLIMB_ROPE );
		params.Set_Animation ("S_A_Human.XG_EV5_Troop", false);
	//	params.Set_Animation ("S_A_HUMAN.H_A_A0F0", true);
		Commands->Action_Play_Animation (obj, params);
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(action_id == CLIMB_ROPE && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Destroy_Object(obj);
		}
		
	}

};

DECLARE_SCRIPT(M07_Inn_Evac_Rope, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeA") == 0)
		{
			Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeL", true, NULL, 0.0f, -1.0f, false );
		}

		else if (stricmp(anim, "XG_EV5_rope.XG_EV5_ropeZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		switch (type)
		{
			case M07_INN_HELICOPTER_OUT: 
				{
					Commands->Set_Animation ( obj, "XG_EV5_rope.XG_EV5_ropeZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M07_Inn_Evac_Trajectory, "")
{
	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathA") == 0)
		{
			Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathL", true, NULL, 0.0f, -1.0f, false );
		}

		else if (stricmp(anim, "XG_EV5_Path.XG_EV5_PathZ") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		switch (type)
		{
			case M07_INN_HELICOPTER_OUT: 
				{
					Commands->Set_Animation ( obj, "XG_EV5_Path.XG_EV5_PathZ", false, NULL, 0.0f, -1.0f, false );
				}
				break;
		}
	}
};

DECLARE_SCRIPT(M07_Inn_Evac_Helicopter, "")
{
	void Created( GameObject * obj ) override
	{
		Commands->Enable_Hibernation( obj, false );
		Commands->Disable_Physical_Collisions ( obj );
		Commands->Send_Custom_Event ( obj, obj, M07_INN_HELO_COLLISIONS, 1, 3.0F );
	}

	void Animation_Complete(GameObject * obj, const char *anim) override
	{
		if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsA") == 0)
		{
			Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsL", true, NULL, 0.0f, -1.0f, false );
			Commands->Enable_Collisions ( obj );

			
		}

		else if (stricmp(anim, "v_GDI_trnspt.XG_EV5_trnsz") == 0)
		{
			Commands->Destroy_Object ( obj );
		}
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		switch (type)
		{
			case M07_INN_HELICOPTER_OUT: 
				{
					Commands->Disable_Physical_Collisions ( obj );
					Commands->Set_Animation ( obj, "v_GDI_trnspt.XG_EV5_trnsz", false, NULL, 0.0f, -1.0f, false );
				}
				break;

			case M07_INN_HELO_COLLISIONS: 
				{
					Commands->Enable_Collisions ( obj );
				}
				break;
		
		}
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
};

DECLARE_SCRIPT(M07_Deadeye_Nod_Chinook, "")
{

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( apc_id, 1 );
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

};

DECLARE_SCRIPT(M07_Alley_Vehicle, "Waypath_ID=0:int")
{
	bool attacking;
	int waypath_id;
	bool active;
	
	enum{ATTACK_OVER, ATTACKING, WAYPATH};
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
		SAVE_VARIABLE( active, 2 );
		SAVE_VARIABLE( waypath_id, 3 );
	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Enemy_Seen( obj, true);
		attacking = true;
		active = false;
		waypath_id = Get_Int_Parameter("Waypath_ID");
	}

	void Custom( GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		ActionParamsStruct params;

		if ( type == M07_CUSTOM_ACTIVATE ) 
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Action_Attack( obj, params );
		}
	}

	void Action_Complete(GameObject * /* obj */, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;
		
		if(action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			attacking = false;
		}
		
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!active)
		{
			active = true;
			Commands->Send_Custom_Event (obj, obj, M07_CUSTOM_ACTIVATE, 0.0f, 0.0f);
		}
		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 0.0f, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 0);
			params.WaypathID = waypath_id;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 5.0f, ATTACK_OVER);
		}
	}

	void Damaged( GameObject * obj, GameObject * damager, float /* amount */ ) override 
	{
		ActionParamsStruct params;

		if(!active)
		{
			active = true;
			Commands->Send_Custom_Event (obj, obj, M07_CUSTOM_ACTIVATE, 0.0f, 0.0f);
		}

		if(!attacking && (Commands->Get_Player_Type(damager) != SCRIPT_PLAYERTYPE_NOD))
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 0.0f, 1.5f );
			params.Set_Attack (damager, 250.0f, 0.0f, 0);
			params.WaypathID = waypath_id;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

			Commands->Start_Timer (obj, this, 5.0f, ATTACK_OVER);
		}
	}

	void Timer_Expired(GameObject * obj, int timer_id ) override
	{
		ActionParamsStruct params;

		if(timer_id == ATTACK_OVER)
		{
			attacking = false;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
			params.Set_Movement( Vector3(0,0,0), 1.0f, 1.5f );
			params.WaypathID = waypath_id;
			Commands->Modify_Action (obj, WAYPATH, params, true, true);

		}
		
	}

};

DECLARE_SCRIPT(M07_Custom_Activate, "Object1_ID=0:int, Object2_ID=0:int, Object3_ID=0:int,")
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
			int object1_id = Get_Int_Parameter("Object1_ID");
			int object2_id = Get_Int_Parameter("Object2_ID");
			int object3_id = Get_Int_Parameter("Object3_ID");

			if(object1_id != 0)
			{
				Commands->Send_Custom_Event (obj, Commands->Find_Object(object1_id), M07_CUSTOM_ACTIVATE, 1, 0.0f);
			}
			if(object2_id != 0)  
			{
				Commands->Send_Custom_Event (obj, Commands->Find_Object(object2_id), M07_CUSTOM_ACTIVATE, 1, 0.0f);
			}
			if(object3_id != 0)
			{
				Commands->Send_Custom_Event (obj, Commands->Find_Object(object3_id), M07_CUSTOM_ACTIVATE, 1, 0.0f);
			}
			
		}
	}
};

DECLARE_SCRIPT(M07_Bridge_Overlook_Apache, "")
{
	bool attacking;

	enum{WAYPATH, INN_PATROL, ATTACK_OVER};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( attacking, 1 );
	}

	void Created (GameObject * obj) override
	{
		ActionParamsStruct params;

		attacking = false;
		Commands->Disable_Physical_Collisions(obj);

		params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, WAYPATH );
		params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
		params.WaypathID = 109145;
		Commands->Action_Goto( obj, params );
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		ActionParamsStruct params;

		if (action_id == WAYPATH && reason == ACTION_COMPLETE_NORMAL)
		{
			Commands->Enable_Enemy_Seen(obj, true);

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INN_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.WaypathID = 109151;
			Commands->Action_Attack( obj, params );
		}
	}

	void Enemy_Seen(GameObject * obj, GameObject *enemy ) override
	{
		ActionParamsStruct params;

		if(!attacking)
		{
			attacking = true;

			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, INN_PATROL );
			params.Set_Movement( Vector3(0,0,0), RUN, 1.5f );
			params.Set_Attack (enemy, 250.0f, 0.0f, 1);
			params.WaypathID = 109151;
			Commands->Modify_Action (obj, INN_PATROL, params, true, true);

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

DECLARE_SCRIPT(M07_SSM_Unit, "")
{
	
	enum{GO_STAR};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( already_entered, 1 );

	}

	void Created (GameObject * obj) override
	{
		Commands->Enable_Hibernation(obj, false);
		Commands->Set_Innate_Aggressiveness(obj, 1.0f );
		Commands->Set_Innate_Take_Cover_Probability(obj, 0.0f );

		ActionParamsStruct params;
		params.Set_Basic(this, INNATE_PRIORITY_ENEMY_SEEN - 5, GO_STAR);
		params.Set_Movement(STAR, RUN, 1.0f);
		Commands->Action_Goto(obj, params);
	}

	void Sound_Heard(GameObject * obj, const CombatSound &sound) override
	{
		ActionParamsStruct params;

		const char * preset_name = Commands->Get_Preset_Name( obj );

		if (sound.Type == M07_SSM_DAMAGED && ( ::strncmp( "Nod_Engineer_0", preset_name, ::strlen( "Nod_Engineer_0" ) ) == 0 ))
		{
			params.Set_Basic( this, INNATE_PRIORITY_ENEMY_SEEN + 5, 100 );
			params.Set_Movement( Vector3(0,0,0), RUN, 5.5f );
			params.Set_Attack (sound.Creator, 250.0f, 0.0f, 0);
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
		}
		if (sound.Type == M07_SSM_FIXED && ( ::strncmp( "Nod_Engineer_0", preset_name, ::strlen( "Nod_Engineer_0" ) ) == 0 ))
		{
			Commands->Action_Reset(obj, INNATE_PRIORITY_ENEMY_SEEN + 5);
		}
	}

};

DECLARE_SCRIPT(M07_Obelisk_Unit, "")
{
	
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( poked, 1 );
	}

	void Created(GameObject * obj) override
	{
		Commands->Innate_Disable(obj);		
	}

	void Custom (GameObject * obj, int type, intptr_t /* param */, GameObject * /* sender */) override
	{
		if(type == M07_CUSTOM_ACTIVATE)
		{
			Commands->Innate_Enable(obj);
		}
	}

};



DECLARE_SCRIPT (M07_Park_Stealth_Tank, "")	
{
	float health;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( health, 8 );
	}

	void Created(GameObject * obj) override
	{
		health = Commands->Get_Health(obj);
	}

	void Damaged(GameObject * obj, GameObject * /* damager */, float /* amount */) override
	{
		if(health > (.2 * Commands->Get_Max_Health(obj)))
		{
			float damage = health - Commands->Get_Health(obj);
			damage *= 0.5f;
			health -= damage;
			Commands->Set_Health(obj, health);
		}
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		// Defeat Stealth Tank.
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 707, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_SSM_Crate, "")
{
	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
//		SAVE_VARIABLE( destroy_stockpile, 1 );
	}

	void Killed (GameObject * obj, GameObject * /* killer */) override
	{
		Commands->Create_Explosion("Explosion_Rocket_SSM", Commands->Get_Position(obj), NULL); 
		Commands->Send_Custom_Event(obj, Commands->Find_Object(100781), M07_SSM_CRATE_KILLED, 1, 0.0f);
	}
};

DECLARE_SCRIPT(M07_DataDisc_01_DLS, "")//
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			// Reveal Stealth Tank
			Commands->Reveal_Encyclopedia_Vehicle(33);
		}
	}
};

DECLARE_SCRIPT(M07_DataDisc_02_DLS, "")//
{
	void Custom( GameObject * /* obj */, int type, intptr_t /* param */, GameObject * /* sender */ ) override
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) 
		{
			// Reveal Stealth Soldier
			Commands->Reveal_Encyclopedia_Character(47);
		}
	}
};

DECLARE_SCRIPT(M07_Hotwire_Help, "")
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
			
			// Hotwire
			if(Commands->Find_Object(100658))
			{
				// Hotwire here, I need help!  Now!  Anyone!?
				const char *conv_name = ("M07_CON028");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 100658);
				
			}
				
		}
	}
	
};

DECLARE_SCRIPT(M07_Hotwire_Dead, "")
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
			
			// Hotwire
			if(Commands->Find_Object(100658))
			{
				// This is Hotwire, I can't hold this position! Requesting - oh great! <gunfire and static>
				const char *conv_name = ("M07_CON029");
				int conv_id = Commands->Create_Conversation (conv_name, 100.0f, 200.0f, false);
				Commands->Join_Conversation(NULL, conv_id, false, true, true);
				Commands->Start_Conversation (conv_id, 100658);
				Commands->Monitor_Conversation (obj, conv_id);	
			}
				
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason) override
	{
		if(reason == ACTION_COMPLETE_CONVERSATION_ENDED)
		{
			if(action_id == 100658)
			{
				// Hotwire Killed
				Commands->Set_HUD_Help_Text ( IDS_M05DSGN_DSGN0174I1DSGN_TXT, TEXT_COLOR_OBJECTIVE_PRIMARY );
				// Protect Team Members
				Commands->Send_Custom_Event(obj, Commands->Find_Object(100657), 709, 2, 0.0f);
				
			}

		}
	}

};


