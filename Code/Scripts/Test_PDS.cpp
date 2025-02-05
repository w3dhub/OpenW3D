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
*
* DESCRIPTION
*
* PROGRAMMER
*     Patrick Smith
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 31 $
*     $Modtime: 11/29/01 11:05a $
*     $Archive: /Commando/Code/Scripts/Test_PDS.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "dprint.h"



DECLARE_SCRIPT(PDS_Generic_Test, "")
{
	void Created (GameObject *game_obj)
	{
		Commands->Start_Timer(game_obj, this, 3.0F, 777);
		return ;
	}


	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id == 777) {
			GameObject *star = Commands->Get_The_Star ();
			if (star != NULL) {
				Vector3 pos = Commands->Get_Position (star);
				pos.Z = Commands->Get_Safe_Flight_Height (pos.X, pos.Y);
				Commands->Set_Position (game_obj, pos);
			}
			Commands->Start_Timer(game_obj, this, 3.0F, 777);
		}

		return ;
	}
};



DECLARE_SCRIPT(PDS_Test_Modify_Attack, "WaypathID=:int")
{
	void Created (GameObject *game_obj)
	{
		Commands->Start_Timer(game_obj, this, 3.0F, 777);
		return ;
	}


	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id == 777) {

			Commands->Enable_Engine (game_obj, true);

			ActionParamsStruct params;
			params.Set_Basic (this, 100, 777);
			params.Set_Movement (Vector3 (0, 0, 0), 1, 1);
			params.WaypathID			= Get_Int_Parameter (0);
			params.WaypathSplined	= true;
			params.AttackActive		= false;
			params.AttackCheckBlocked = false;
			Commands->Action_Attack (game_obj, params);

			//params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);			
			//params.AttackCheckBlocked = false;
			//Commands->Action_Attack( game_obj, params );
			//Commands->Action_Goto( game_obj, params );

			Commands->Start_Timer(game_obj, this, 4.0F, 888);

		} else if (timer_id == 888) {

			ActionParamsStruct params;
			params.Set_Basic (this, 100, 777);
			params.Set_Movement (Vector3 (0, 0, 0), 1, 1);
			params.WaypathID			= Get_Int_Parameter (0);
			params.WaypathSplined	= true;
			params.AttackActive		= true;
			params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);	
			params.AttackCheckBlocked = false;
			
			Commands->Modify_Action (game_obj, 777, params, true, true);

			Commands->Start_Timer(game_obj, this, 4.0F, 999);			

		} else if (timer_id == 999) {

			ActionParamsStruct params;
			params.Set_Basic (this, 100, 777);
			params.Set_Movement (Vector3 (0, 0, 0), 1, 1);
			params.WaypathID			= Get_Int_Parameter (0);
			params.WaypathSplined	= true;
			params.AttackActive		= false;
			params.AttackCheckBlocked = false;
			//params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);	
			
			Commands->Modify_Action (game_obj, 777, params, true, true);

			Commands->Start_Timer(game_obj, this, 4.0F, 888);
		}

		return ;
	}
};




DECLARE_SCRIPT(PDS_Test_Dock, "DockDestObjID=:int,DockEntranceObjID=:int")
{
	void Created (GameObject *game_obj)
	{
		Commands->Start_Timer (game_obj, this, 3.0F, 777);
		return ;
	}


	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id != 777) {
			return ;
		}		

		//
		//	Lookup the objects which mark the destination (docked) position
		// and the entrance to the docking bay
		//
		GameObject *dest_obj			= Commands->Find_Object (Get_Int_Parameter (0));
		GameObject *entrance_obj	= Commands->Find_Object (Get_Int_Parameter (1));
		if (dest_obj != NULL && entrance_obj != NULL) {

			//
			//	Start the vehicle's engine
			//
			//Commands->Enable_Engine (game_obj, true);
			
			//
			// Lookup the positions of the two docking markers
			//
			Vector3 dest_pos		= Commands->Get_Position (dest_obj);
			Vector3 entrance_pos	= Commands->Get_Position (entrance_obj);

			//
			//	Dock the vehicle
			//
			ActionParamsStruct params;
			params.Set_Basic (this, 100, 777);
			params.Dock_Vehicle (dest_pos, entrance_pos);
			params.MoveSpeed = 1.0F;
			Commands->Action_Dock (game_obj, params);
		}

		return ;
	}
};



DECLARE_SCRIPT(PDS_Test_Conversation, "Conversation Name=:string,Soldier1_ID=0:int,Soldier2_ID=0:int,Soldier3_ID=0:int")
{
	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		Commands->Start_Timer(game_obj, this, 3.0F, 777);
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Action_Complete (GameObject * game_obj, int action_id, ActionCompleteReason complete_reason)
	{
		//
		//	Just for kicks, make the unit crouch
		//
		if (action_id == 778) {
			Commands->Set_Animation (game_obj, "S_A_HUMAN.H_A_A0A0_L08", true, NULL, 0.0f, -1.0f, false);
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Custom
	////////////////////////////////////////////////////////////////////
	void Custom (GameObject *game_obj, int event, int data, GameObject *sender)
	{
		if (event == CUSTOM_EVENT_CONVERSATION_BEGAN) {
			int test = 0;
			test ++;
		} else if (event == CUSTOM_EVENT_CONVERSATION_REMARK_STARTED) {
			int test = 0;
			test ++;
		} else if (event == CUSTOM_EVENT_CONVERSATION_REMARK_ENDED) {
			int test = 0;
			test ++;
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id != 777) {
			return ;
		}

		const char *conv_name = Get_Parameter (0);

		//
		//	Create the conversation
		//
		int conv_id = Commands->Create_Conversation (conv_name, 0, 0, true);
		if (conv_id != -1) {

			int soldier1_id = Get_Int_Parameter (1);
			int soldier2_id = Get_Int_Parameter (2);
			int soldier3_id = Get_Int_Parameter (3);
			
			//
			//	Have the participants join the conversation
			//
			Commands->Join_Conversation (game_obj, conv_id, true, true, true);

			if (soldier1_id == -1) {
				GameObject *soldier = Commands->Get_The_Star ();
				if (soldier != NULL) {
					Commands->Join_Conversation (soldier, conv_id, true, true, true);
				}
			} else if (soldier1_id != 0) {
				GameObject *soldier = Commands->Find_Object (soldier1_id);
				if (soldier != NULL) {
					Commands->Join_Conversation (soldier, conv_id, true, true, true);
				}
			}
			
			if (soldier2_id != 0) {
				GameObject *soldier = Commands->Find_Object (soldier2_id);
				if (soldier != NULL) {
					Commands->Join_Conversation (soldier, conv_id, true, true, true);
				}
			}
			
			if (soldier3_id != 0) {
				GameObject *soldier = Commands->Find_Object (soldier3_id);
				if (soldier != NULL) {
					Commands->Join_Conversation (soldier, conv_id, true, true, true);
				}
			}

			//
			//	Begin the conversation
			//
			Commands->Start_Conversation (conv_id, 778);
			Commands->Monitor_Conversation (game_obj, conv_id);
		}

		return ;
	}
};




DECLARE_SCRIPT(PDS_Get_In_Vehicle_Do_Waypath, "VehicleID=:int,WaypathID=:int,V3Test=1.3 44.543 0:vector3")
{
	bool m_StartedWaypath;

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject* game_obj)
	{
		//
		//	Find the vehicle we want to get into
		//
		m_StartedWaypath = false;
		GameObject *dest_obj = Commands->Find_Object (Get_Int_Parameter (0));
		if (dest_obj != NULL) {
			
			//
			// Instruct our game object to goto the driver's side of the destination vehicle
			//
			Vector3 pos = Commands->Get_Position (dest_obj);
//			Commands->Action_Movement_Goto_Location (game_obj, pos, 1.0F);
			ActionParamsStruct params;
			params.Set_Basic( this, 50, 0 );
			params.Set_Movement( pos, 1, 1 );
			Commands->Action_Goto( game_obj, params );

		} else {
			DebugPrint ("Error! Cannot find destination for PDS_Get_In_Vehicle_Do_Waypath.\n");
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Movement_Complete
	////////////////////////////////////////////////////////////////////
//	void Movement_Complete (GameObject *game_obj, MovementCompleteReason /*reason*/)
	void Action_Complete (GameObject * game_obj, int action_id, ActionCompleteReason complete_reason)
	{
		//
		//	Now tell the object to follow the waypath
		//
		if (m_StartedWaypath == false) {			
//			Commands->Action_Movement_Follow_Waypath (game_obj, Get_Int_Parameter(1), 0, 0, true);
			ActionParamsStruct params;
			params.Set_Basic( this, 50, 0 );
			params.Set_Movement( Vector3(0,0,0), 1, 1 );
			params.WaypathID = Get_Int_Parameter(1);
			Commands->Action_Goto( game_obj, params );
			m_StartedWaypath = true;
		}
		return ;
	}

};


DECLARE_SCRIPT(PDS_Test_Goto_Player, "")
{

	void Created(GameObject* game_obj)
	{
		Commands->Start_Timer(game_obj, this, 3.0F, 777);
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id != 777) {
			return ;
		}

		ActionParamsStruct params;
		//params.ActionID = 777;
		params.Set_Basic( this, 50, 777 );
		params.Set_Movement( Commands->Get_The_Star (), 1, 2 );
		params.MoveFollow = false;
		Commands->Action_Goto( game_obj, params );

		return ;
	}

	void Action_Complete (GameObject * game_obj, int action_id, ActionCompleteReason complete_reason)
	{
		if (action_id == 777) {
			int test = 0;
			test ++;
		}

		return ;
	}
};


DECLARE_SCRIPT(PDS_Test_Goto_Loc, "ObjDestID=:int")
{
	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject* game_obj)
	{
		Commands->Start_Timer(game_obj, this, 3.0F, 777);
		return ;
	}


	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id != 777) {
			return ;
		}

		//
		//	Find the destination 'object'
		//
		GameObject* dest_obj = Commands->Find_Object(Get_Int_Parameter(0));
		if (dest_obj != NULL) {
			
			//
			// Instruct our game object to goto the location of the destination object
			//
			Vector3 pos = Commands->Get_Position(dest_obj);
//			Commands->Action_Movement_Goto_Location(game_obj, pos, 3.0F);
			ActionParamsStruct params;
			params.Set_Basic( this, 50, 0 );
			params.Set_Movement( pos, 0.5F, 3 );
			Commands->Action_Goto( game_obj, params );
		} else {
			DebugPrint("Error! Cannot find destination for PDS_Test_Goto_Loc.\n");
		}

		return ;
	}
};


DECLARE_SCRIPT(PDS_Test_Follow_Player, "")
{
	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{

		Commands->Start_Timer(game_obj, this, 3.0F, 777);
		return ;
	}

	void	Poked( GameObject * obj, GameObject * poker )
	{
		Commands->Grant_Key (obj, 1, true);
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		Commands->Innate_Disable (game_obj);
//		Commands->Action_Movement_Follow_Object (game_obj, Commands->Get_The_Star (), 2.0F);
		/*ActionParamsStruct params;
		params.Set_Basic( this, 50, 0 );
		params.Set_Movement( Commands->Get_The_Star (), 1, 15 );
		//params.Set_Attack( Commands->Get_The_Star (), 100, 15, true );
		//params.Set_Movement( Commands->Get_The_Star (), 1, 15 );
		params.MoveFollow = true;
		Commands->Action_Attack( game_obj, params );*/

		ActionParamsStruct params2;
		params2.Set_Basic( this, 50, 0 );
		params2.Set_Movement( Commands->Get_The_Star (), 1, 5 );
		params2.MoveFollow = true;
		Commands->Action_Goto( game_obj, params2 );

		return ;
	}
};


DECLARE_SCRIPT(PDS_Test_Follow_Waypath, "WaypathID=:int,WaypointStartID=:int,WaypointEndID=:int")
{
	void Created (GameObject *game_obj)
	{		
		Commands->Grant_Key (game_obj, 0, true);
		Commands->Grant_Key (game_obj, 1, true);
		Commands->Grant_Key (game_obj, 2, true);
		Commands->Grant_Key (game_obj, 3, true);
		Commands->Grant_Key (game_obj, 4, true);
		Commands->Grant_Key (game_obj, 5, true);
		Commands->Grant_Key (game_obj, 6, true);
		Commands->Grant_Key (game_obj, 7, true);

		Commands->Enable_Engine (game_obj, true);

		ActionParamsStruct params;
		params.Set_Basic( this, 100, 777 );
		params.Set_Movement( Vector3(0,0,0), 1, 5 );
		params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);
		
		params.WaypathID = Get_Int_Parameter(0);
		params.WaypathSplined = true;
		params.AttackActive = false;
		params.AttackCheckBlocked = false;

		Commands->Action_Attack( game_obj, params );

		return ;
	}


	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
//		Commands->Action_Movement_Follow_Waypath (game_obj, Get_Int_Parameter(0), Get_Int_Parameter(1), Get_Int_Parameter(2), true);

		if (timer_id == 777) {

			ActionParamsStruct params;
			params.Set_Basic (this, 100, 777);
			params.Set_Movement (Vector3 (0, 0, 0), 1, 1);
			params.WaypathID			= Get_Int_Parameter (0);
			params.WaypathSplined	= true;
			params.AttackActive		= false;
			params.AttackCheckBlocked = false;
			params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);	
			
			Commands->Modify_Action (game_obj, 777, params, true, true);

			//Commands->Action_Goto( game_obj, params );
			return ;
		}
	}

	void	Damaged( GameObject * game_obj, GameObject * damager, float amount ) 
	{
		ActionParamsStruct params;
		params.Set_Basic (this, 100, 777);
		params.Set_Movement (Vector3 (0, 0, 0), 1, 1);
		params.WaypathID			= Get_Int_Parameter (0);
		params.WaypathSplined	= true;
		params.AttackActive		= true;
		params.AttackCheckBlocked = false;
		params.Set_Attack (Commands->Get_The_Star (), 100, 0.5F, true);	
		
		Commands->Modify_Action (game_obj, 777, params, true, true);

		// attack for a while, then go back to following the waypath
		Commands->Start_Timer(game_obj, this, 5.0F, 777);
	}

};


DECLARE_SCRIPT(PDS_Test_Harvester, "TiberiumID=:int,DriveToID=:int,EntranceID=:int,DockID=:int")
{
	int				m_State;
	GameObject *	m_GameObj;

	////////////////////////////////////////////////////////////////////
	//	State
	////////////////////////////////////////////////////////////////////
	enum
	{
		DRIVE_TO_TIBERIUM	= 0,
		HARVEST_TIBERIUM,
		DRIVE_TO_REFINERY,
		DOCK_HARVESTER_STEP1,
		DOCK_HARVESTER_STEP2,
		UNLOAD_TIBERIUM,
		MAX_STATE
	};

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		//
		//	Initialize variables
		//
		m_State		= DRIVE_TO_TIBERIUM;
		m_GameObj	= game_obj;

		//
		//	Process state
		//
		Think ();
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *obj, int timer_id)
	{
		if (timer_id == 777) {
			Next_State ();
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Movement_Complete
	////////////////////////////////////////////////////////////////////
//	void Movement_Complete (GameObject *obj, MovementCompleteReason reason)
	void Action_Complete (GameObject * game_obj, int action_id, ActionCompleteReason complete_reason)
	{
		Next_State ();
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Next_State
	////////////////////////////////////////////////////////////////////
	void Next_State (void)
	{
		//
		//	Move on to the next state
		//
		m_State ++;
		if (m_State == MAX_STATE) {
			m_State = DRIVE_TO_TIBERIUM;
		}

		//
		//	Process the new state
		//
		Think ();
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Think
	////////////////////////////////////////////////////////////////////
	void Think (void)
	{
		switch (m_State)
		{
			case DRIVE_TO_TIBERIUM:
				Drive_To_Obj (Get_Int_Parameter (0), false);
				break;

			case HARVEST_TIBERIUM:
				Commands->Start_Timer (m_GameObj, this, Commands->Get_Random (3.0F, 6.0F), 777);
				break;

			case DRIVE_TO_REFINERY:
				Drive_To_Obj (Get_Int_Parameter (1), false);
				break;

			case DOCK_HARVESTER_STEP1:
				Drive_To_Obj (Get_Int_Parameter (2), true);
				break;

			case DOCK_HARVESTER_STEP2:
				Drive_To_Obj (Get_Int_Parameter (3), true);
				break;

			case UNLOAD_TIBERIUM:
				Commands->Start_Timer (m_GameObj, this, Commands->Get_Random (2.0F, 4.0F), 777);
				break;
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Drive_To_Obj
	////////////////////////////////////////////////////////////////////
	void Drive_To_Obj (int obj_id, bool force_backup)
	{
		//
		//	Find the destination object
		//
		GameObject* dest_obj = Commands->Find_Object (obj_id);
		if (dest_obj != NULL) {
			
			//
			// Instruct our game object to goto the location of the destination object
			//
			Vector3 pos = Commands->Get_Position (dest_obj);
			
			if (force_backup == false) {
//				Commands->Action_Movement_Goto_Location (m_GameObj, pos, 1.0F);
				ActionParamsStruct params;
				params.Set_Basic( this, 50, 0 );
				params.Set_Movement( pos, 1, 1 );
				Commands->Action_Goto( m_GameObj, params );
			} else {
//				Commands->Action_Movement_Backup_Goto_Location (m_GameObj, pos, 1.0F);
				ActionParamsStruct params;
				params.Set_Basic( this, 50, 0 );
				params.Set_Movement( pos, 1, 1 );
				params.MoveBackup = true;
				Commands->Action_Goto( m_GameObj, params );
			}

		} else {
			DebugPrint("Error! Cannot find destination for PDS_Test_Harvester.\n");
		}

		return ;
	}
};


////////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////////
enum
{
	INVENTORY_EMPTY		= 0,
	INVENTORY_MEDKIT,	
};

enum
{
	CUSTOM_HAS_MEDKIT		= 0,
	CUSTOM_GOT_OBJECT,
};


DECLARE_SCRIPT(PDS_Test_Inventory, "")
{
	////////////////////////////////////////////////////////////////////
	//	Member data
	////////////////////////////////////////////////////////////////////

	enum
	{
		INVENTORY_MAX	= 4
	};

	int			Inventory[INVENTORY_MAX];
	

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		//
		//	Clear the inventory
		//
		for (int index = 0; index < INVENTORY_MAX; index ++) {
			Inventory[index] = INVENTORY_MEDKIT;
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Custom
	////////////////////////////////////////////////////////////////////
	void Custom (GameObject *game_obj, int event, int data, GameObject *sender)
	{
		if (event == CUSTOM_HAS_MEDKIT) {
			int *retval = (int *)data;
			if (retval != NULL) {
				(*retval) = 0;
				
				//
				//	Check all the inventory slots for the medkit
				//
				for (int index = 0; index < INVENTORY_MAX; index ++) {
					if (Inventory[index] == INVENTORY_MEDKIT) {
						(*retval) = 1;
						break;
					}
				}
			}

		} else if (event == CUSTOM_GOT_OBJECT) {
				
			//
			//	Find an empty slot in the inventory and stick this object into it
			//
			for (int index = 0; index < INVENTORY_MAX; index ++) {
				if (Inventory[index] == INVENTORY_EMPTY) {
					Inventory[index] = data;
					break;
				}
			}			
		}

		return ;
	}

};


DECLARE_SCRIPT(PDS_Test_Bubba, "")
{
	////////////////////////////////////////////////////////////////////
	//	Member data
	////////////////////////////////////////////////////////////////////
	GameObject *	GameObj;
	int				State;

	////////////////////////////////////////////////////////////////////
	//	Constants
	////////////////////////////////////////////////////////////////////
	enum
	{
		TIMER_GROAN		= 444,
	};

	enum
	{
		STATE_DYING			= 0,
		STATE_FOLLOWING,
		STATE_CRINGING
	};

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		Commands->Clear_Map_Cell_By_Pos (Commands->Get_Position (game_obj));
		Commands->Clear_Map_Cell (0, 0);
		Commands->Clear_Map_Cell (10, 10);
		Commands->Clear_Map_Cell (19, 19);
		return ;
	}
};




DECLARE_SCRIPT(PDS_Test_Controller, "")
{
	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		Commands->Start_Timer (game_obj, this, 2.0F, 777);
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id == 777) {
			GameObject *the_player = Commands->Get_The_Star ();
			if (the_player != NULL) {
				Commands->Attach_Script (the_player, "PDS_Test_Inventory", "");
			}
		}

		return ;
	}
};



DECLARE_SCRIPT(PDS_Test_Gunboat, "")
{
	////////////////////////////////////////////////////////////////////
	//	Member Data
	////////////////////////////////////////////////////////////////////
	GameObject *GameObj;
	int State;


	////////////////////////////////////////////////////////////////////
	//	Constants
	////////////////////////////////////////////////////////////////////
	enum
	{
		TIMER_ATTACK		= 777,
	};

	enum
	{
		STATE_AT_BEACHHEAD		= 0,
		STATE_AT_INLET,
		STATE_FIGHTING_BIG_GUN,
		STATE_IN_TRANSIT
	};

	////////////////////////////////////////////////////////////////////
	//	Created
	////////////////////////////////////////////////////////////////////
	void Created (GameObject *game_obj)
	{
		GameObj	= game_obj;
		State		= STATE_AT_BEACHHEAD;
		
		Move_To_Next_Location ();
		//State = STATE_IN_TRANSIT;
		
		Commands->Enable_Engine (GameObj, true);
		Commands->Disable_All_Collisions (GameObj);
		Start_Attack ();
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Start_Attack
	////////////////////////////////////////////////////////////////////
	void Start_Attack (void)
	{
		if (State != STATE_IN_TRANSIT) {
			//Commands->Start_Timer (GameObj, this, Commands->Get_Random (10.0F, 12.0F), TIMER_ATTACK);
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Move_To_Next_Location
	////////////////////////////////////////////////////////////////////
	void Move_To_Next_Location (void)
	{
		int waypath_id = 0;

		switch (State)
		{
			case STATE_AT_BEACHHEAD:
				waypath_id = 100050;
				break;

			case STATE_AT_INLET:
				waypath_id = 0;
				break;
		}

		if (waypath_id != 0) {
			
			ActionParamsStruct params;
			params.WaypathID			= waypath_id;
			params.WaypointStartID	= 0;
			params.WaypointEndID		= 0;
			params.WaypathSplined	= true;
			Commands->Action_Goto (GameObj, params);
		}				

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Timer_Expired
	////////////////////////////////////////////////////////////////////
	void Timer_Expired (GameObject *game_obj, int timer_id)
	{
		if (timer_id == TIMER_ATTACK) {
			GameObject *the_player = Commands->Get_The_Star ();
			if (the_player != NULL) {
				
				Vector3 pos = Commands->Get_Position (the_player);
				//pos.X += Commands->Get_Random (-1.0F, 1.0F) * Commands->Get_Random (6.0F, 10.0F);
				//pos.Y += Commands->Get_Random (-1.0F, 1.0F) * Commands->Get_Random (6.0F, 10.0F);
				//pos.Z += Commands->Get_Random (-1.0F, 1.0F) * Commands->Get_Random (6.0F, 10.0F);
				
				ActionParamsStruct params;
				params.Set_Basic (this, 50, 0);
				params.Set_Attack (pos, 200, 0, true);
				Commands->Action_Attack (GameObj, params);

				Start_Attack ();
			}
		}

		return ;
	}
};


DECLARE_SCRIPT(PDS_Test_Sound, "")
{
	int SoundID;
	bool IsPlaying;

	void Created (GameObject* game_obj)
	{
		IsPlaying = true;
		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Poked
	////////////////////////////////////////////////////////////////////
	void	Poked (GameObject * obj, GameObject * poker)
	{
		if (IsPlaying) {
			Commands->Stop_Sound (1700002, false);
			IsPlaying = false;
		} else {
			Commands->Start_Sound (1700002);
			IsPlaying = true;
		}

		return ;
	}

	////////////////////////////////////////////////////////////////////
	//	Custom
	////////////////////////////////////////////////////////////////////
	void Custom (GameObject *game_obj, int event, int data, GameObject *sender)
	{
		if (event == CUSTOM_EVENT_SOUND_ENDED && data == SoundID) {
			int test = 0;
			test ++;
		}

		return ;
	}
};
