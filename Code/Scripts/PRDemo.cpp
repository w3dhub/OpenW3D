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
*	PRDemo.cpp
*
* DESCRIPTION
*	PR Demo Scripts
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 21 $
*     $Modtime: 7/17/00 10:13a $
*     $Archive: /Commando/Code/Scripts/PRDemo.cpp $
*
******************************************************************************/

#include "prdemo.h"

#if 0

/*
MPR_A01_TiberiumCloud

  This block controls the tiberium zones - animation and damage.
  DEBUG NAMING CONVENTION
  DEBUG INNATE?

  Parameters:

  Death_Rate		= DEBUG
  Explode_Height	= DEBUG
*/

DECLARE_SCRIPT(MPR_A01_TiberiumCloud_PMP, "Death_Rate=5:int,Explode_Height=1.0:float")
{
	int Death_Rate;
	float Explode_Height;
	Vector3 cloud_position;

	void Created(GameObject* obj)
	{
		Death_Rate = Get_Int_Parameter("Death_Rate");
		Explode_Height = Get_Float_Parameter("Explode_Height");

		Commands->_Start_Timer(obj, this, Commands->Get_Random( 1, Death_Rate ), MPR_TIMER_TIBERIUM_PMP);
	}

	void Timer_Expired(GameObject* obj, int timerID)
	{
		if (timerID == MPR_TIMER_TIBERIUM_PMP)
		{
			cloud_position = Commands->Get_Position(obj);
			cloud_position.Z += Explode_Height;

			Commands->Create_Explosion("TibCloud",cloud_position,obj);
			Commands->_Start_Timer(obj, this, Death_Rate, MPR_TIMER_TIBERIUM_PMP);
		}
	}
};


/*
MPR_A01_CivLeader

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION

  Parameters:

  Follower_ID	= DEBUG
*/
		
DECLARE_SCRIPT(MPR_A01_CivLeader_PMP, "Follower_ID:int")
{
	int Follower_ID;
	bool shooting;

	Vector3 BarnPoint;

	void Created(GameObject* obj)
	{
		BarnPoint.X = -193;
		BarnPoint.Y = -133;
		BarnPoint.Z = 4;

		MPR_nod_dead_PMP = false;
		shooting = false;
		MPR_evacing_to_chinook_PMP = false;

		Follower_ID = Get_Int_Parameter("Follower_ID");
		GameObject* target = Commands->Find_Object(Follower_ID);

		if ((Commands->Find_Object(Follower_ID)) && (shooting==false))
		{
			Commands->Send_Custom_Event(obj, target,MPR_CUSTOM_FOLLOW_ME_DEL, 0);
			//Commands->Start_Timer(obj, 2, MPR_TIMER_DELAY_01_PMP);
		}
	}

	void Custom(GameObject* obj, int event, int param, GameObject* host)
	{
		if (event == MPR_CUSTOM_I_AM_DEAD_DEL)
		{
			//Commands->Display_Text("Got dead message\n");
			MPR_nod_dead_PMP = true;
			shooting = true;

			Commands->_Start_Timer(obj, this, Commands->Get_Random( 0, 5), MPR_TIMER_DELAY_01_04_PMP);
		}
		
		if (event == MPR_CUSTOM_ESCORT_ATTACKED_DEL)
		{
			//Commands->Display_Text("Got attacked message\n");
			Commands->_Start_Timer(obj, this, Commands->Get_Random( 0, 3), MPR_TIMER_DELAY_01_03_PMP);
			shooting = true;
		}
	}

	void Timer_Expired(GameObject* obj, int timerID)
	{
		Vector3 newPoint = RandomVector();

		if ((timerID == MPR_TIMER_DELAY_01_PMP) && (Commands->Find_Object(Follower_ID)) && (MPR_nod_dead_PMP == false) && (shooting == false))
		{
			//Commands->Display_Text("Sending the call\n");
			Commands->Send_Custom_Event(obj, Commands->Find_Object(Follower_ID),MPR_CUSTOM_FOLLOW_ME_DEL, 0);
			Commands->_Start_Timer(obj, this, 2, MPR_TIMER_DELAY_01_PMP);
		}

		if (timerID == MPR_TIMER_DELAY_01_03_PMP)
		{
			ActionParamsStruct params;
			params.Set_Animation("S_A_Human.H_A_J21C", false);
			Commands->Action_Play_Animation(obj, params);
		}

		if (timerID == MPR_TIMER_DELAY_01_04_PMP)
		{
			ActionParamsStruct params;
			params.Set_Basic
			Commands->Action_Movement_Set_Forward_Speed(obj,.33f);
			Commands->Action_Movement_Goto_Location(obj, newPoint,Commands->Get_Random( 0, 2.0 ));
		}
	}

	Vector3 RandomVector(void)
	{
		Vector3 wanderPoint;

		wanderPoint.X = Commands->Get_Random( 0, 3 );
		wanderPoint.Y = Commands->Get_Random( 0, 3 );
		wanderPoint.Z = 0;
		return (BarnPoint + wanderPoint);
	}
};


/*
MPR_A01_CivFollower

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION

  Parameters:

  Host_ID		= DEBUG
  Host_distance	= DEBUG
*/

DECLARE_SCRIPT(MPR_A01_CivFollower_PMP, "Host_ID:int, Host_distance=1.5:float")
{
	float Host_distance;
	int Host_ID;
	bool shooting;

	void Created(GameObject* obj)
	{
		shooting = false;

		Host_distance = Get_Float_Parameter("Host_distance");
		Host_ID = Get_Int_Parameter("Host_ID");
	}	

	void Custom(GameObject* obj, int event, int param, GameObject* host)
	{
		if (event == MPR_CUSTOM_ESCORT_ATTACKED_DEL)
		{
			shooting = true;
		}

		if (( event == MPR_CUSTOM_FOLLOW_ME_DEL ) && ( shooting == false ))
		{
			//Commands->Display_Text("Heard the call\n");

			if (Commands->Find_Object(Host_ID))
			{
				Commands->Action_Movement_Set_Forward_Speed(obj,.25f);

				Commands->Action_Movement_Follow_Object(obj, host, Host_distance);
			}
		}
	}
};


/*
MPR_A01_EscortDestination

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG FRAGMENTED?
*/

DECLARE_SCRIPT(MPR_A01_EscortDestination_PMP,"")
{
	void Created(GameObject* obj)
	{
		//Commands->Display_Text("Moving\n");
		Commands->Action_Movement_Set_Forward_Speed(obj, .25f);

		//Commands->Action_Movement_Goto_Location(obj,Vector3 (-248,-133,6),0.5f);   //Inside Church

		Commands->Action_Movement_Goto_Location(obj, Vector3 (-264.0f,-73.0f,-0.67f),0.5f); //temp until pathfind fixed
	}
};


/*
MPR_A01_EscortDestination

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG FRAGMENTED

  Parameters:

  CIV01_ID	= DEBUG
  CIV02_ID	= DEBUG
  CIV03_ID	= DEBUG
*/

DECLARE_SCRIPT(MPR_A01_EscortDies_PMP,"CIV01_ID:int, CIV02_ID:int, CIV03_ID:int")
{
	int CIV01_ID;
	int CIV02_ID;
	int CIV03_ID;

	void Killed(GameObject*obj, GameObject* killer)
	{
		CIV01_ID = Get_Int_Parameter("CIV01_ID");
		CIV02_ID = Get_Int_Parameter("CIV02_ID");
		CIV03_ID = Get_Int_Parameter("CIV03_ID");
		
		//Commands->Display_Text("I am dead\n");
		MPR_nod_dead_PMP = true;
		Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV01_ID),MPR_CUSTOM_I_AM_DEAD_DEL, 0);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV02_ID),MPR_CUSTOM_I_AM_DEAD_DEL, 0);
		Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV03_ID),MPR_CUSTOM_I_AM_DEAD_DEL, 0);
	}
};


/*
MPR_A01_SeeEnemy

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG FRAGMENTED?

  Parameters:

  CIV01_ID	= DEBUG
  CIV02_ID	= DEBUG
  CIV03_ID	= DEBUG
  NOD01_ID	= DEBUG
*/

DECLARE_SCRIPT(MPR_A01_SeeEnemy_PMP,"CIV01_ID:int, CIV02_ID:int, CIV03_ID:int, NOD01_ID:int")
{
	int CIV01_ID;
	int CIV02_ID;
	int CIV03_ID;
	int NOD01_ID;

	Vector3 start_position;
	Vector3 newPoint;

	bool shooting;

	void Created(GameObject* obj)
	{
		Commands->Enable_Enemy_Seen(obj, true);

		shooting = false;

		CIV01_ID = Get_Int_Parameter("CIV01_ID");
		CIV02_ID = Get_Int_Parameter("CIV02_ID");
		CIV03_ID = Get_Int_Parameter("CIV03_ID");
		NOD01_ID = Get_Int_Parameter("NOD01_ID");
	}

	void Enemy_Seen(GameObject* obj, GameObject* enemy)
	{
		Vector3 shoot_point;

		if ((enemy == (Commands->Get_The_Star())) && (shooting == false))
		{
			GameObject* civie = (Commands->Find_Object(CIV02_ID));
			shoot_point = Commands->Get_Position(civie);
			Commands->Action_Attack_Location(obj, shoot_point, 0.01f, .01f);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV01_ID),MPR_CUSTOM_ESCORT_ATTACKED_DEL, 0);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV02_ID),MPR_CUSTOM_ESCORT_ATTACKED_DEL, 0);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV03_ID),MPR_CUSTOM_ESCORT_ATTACKED_DEL, 0);
			Commands->Send_Custom_Event(obj, Commands->Find_Object(NOD01_ID),MPR_CUSTOM_ESCORT_ATTACKED_DEL, 0);
			Commands->Start_Timer(obj, 4, MPR_TIMER_DELAY_01_02_PMP);
		}

		if ((enemy == (Commands->Get_The_Star())) && (shooting == true))
		{
			GameObject* target = Commands->Get_The_Star();
			Commands->Action_Attack_Object(obj,target,0,5.0f);
			Commands->Innate_Enable(obj);
		}
	}

	void Timer_Expired(GameObject* obj, int timerID)
	{
		if (timerID == MPR_TIMER_DELAY_01_02_PMP)
		{
			GameObject* target = Commands->Get_The_Star();
			Commands->Action_Attack_Object(obj,target,0,5.0f);
			shooting = true;
		}
	}
};


/*
MPR_A01_GET_BARN_ZONE_ID

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETED
*/

DECLARE_SCRIPT(MPR_A01_Get_Barn_Zone_id_PMP,"")
{
	void Created(GameObject* obj)
	{
		MPR_A01_BARNZONE_GOTHERE_ID_PMP = Commands->Get_ID (obj);
	}
};


/*
MPR_A01_EVAC_CALL

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETED?
*/

DECLARE_SCRIPT(MPR_A01_Evac_Call_PMP,"")
{
	bool evac_entered;

	void Created(GameObject* obj)
	{
		evac_entered = false;
	}

	void Entered(GameObject* obj, GameObject* enterer)
	{
		if((enterer != Commands->Get_The_Star()) && (!evac_entered))
		{
			//Commands->Display_Text("Chinook Created\n");

			GameObject* Chinook_evac = Commands->Create_Object( "Chinook_E", Vector3 (0,0,0) );		

			if (Chinook_evac)
			{
				//Commands->Display_Text("Anim Started\n");

				Commands->Attach_Script(Chinook_evac, "MPR_A01_Chinook_Evac_Civies", "");
				evac_entered = true;
			}
		}
	}
};


/*
MPR_A01_Chinook_Evac_Civies

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A01_Chinook_Evac_Civies_PMP,"")
{
	int CIV01_ID;
	int CIV02_ID;
	int CIV03_ID;

	void Created(GameObject* obj)
	{
		CIV01_ID = MPR_A01_CIV01_ID_PMP;
		CIV02_ID = MPR_A01_CIV02_ID_PMP;
		CIV03_ID = MPR_A01_CIV03_ID_PMP;

		Commands->Set_Animation( obj, "V_GDI_CHINOOKE3.m_ch_evacland", false);
	}

	void Animation_Complete(GameObject* obj, const char* name)
	{
		if (openw3d::string_compare (name,"V_GDI_CHINOOKE3.m_ch_evacland") ==0 )
		{
			Commands->Set_Animation( obj, "V_GDI_CHINOOKE3.m_ch_evacloiter", true);
			Commands->Start_Timer(obj, 13, MPR_TIMER_EVAC_PMP );

			if (Commands->Find_Object(CIV01_ID))
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV01_ID),MPR_CUSTOM_EVAC_CALL_DEL, 0);
			}
			if (Commands->Find_Object(CIV02_ID))
			{	
				Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV02_ID),MPR_CUSTOM_EVAC_CALL_DEL, 0);
			}
			if (Commands->Find_Object(CIV03_ID))
			{
				Commands->Send_Custom_Event(obj, Commands->Find_Object(CIV03_ID),MPR_CUSTOM_EVAC_CALL_DEL, 0);
			}
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if (Timer_ID == MPR_TIMER_EVAC_PMP)
		{
			Commands->Set_Animation( obj, "V_GDI_CHINOOKE3.m_ch_evacoff", false);
		}
	}
};


/*
MPR_A01_GET_TO_CHINOOK

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG FRAGMENTED
*/

DECLARE_SCRIPT(MPR_A01_Get_To_Chinook_PMP,"")
{
	void Custom(GameObject* obj, int event, int param, GameObject* host)
	{
		if (event == MPR_CUSTOM_EVAC_CALL_DEL)
		{
			//Commands->Display_Text("Going to Chinook\n");

			MPR_evacing_to_chinook_PMP = true;
			Commands->Action_Movement_Goto_Location(obj, Vector3 (-193,-153,2), 1.0f);
		}
	}

	void Movement_Complete(GameObject* obj, MovementCompleteReason reason)
	{
		if (MPR_evacing_to_chinook_PMP == true)
		{
			Commands->Destroy_Object(obj);
		}
	}
};


/*
MPR_A01_GET_CIV01_ID

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETE
*/

DECLARE_SCRIPT(MPR_A01_Get_Civ01_id_PMP,"")
{
	void Created(GameObject* obj)
	{
		MPR_A01_CIV02_ID_PMP = Commands->Get_ID (obj);
	}
};


/*
MPR_A01_GET_CIV02_ID

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETE
*/

DECLARE_SCRIPT(MPR_A01_Get_Civ02_id_PMP,"")
{
	void Created(GameObject* obj)
	{
		MPR_A01_CIV01_ID_PMP = Commands->Get_ID (obj);
	}
};


/*
MPR_A01_GET_CIV03_ID

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETE
*/

DECLARE_SCRIPT(MPR_A01_Get_Civ03_ID_PMP,"")
{
	void Created(GameObject* obj)
	{
		MPR_A01_CIV03_ID_PMP = Commands->Get_ID (obj);
	}
};


/*
M00_Generic_Wandering

  This block is a generic wandering around start point script.
  DEBUG NAMING CONVENTION
  DEBUG OBSOLETE
  DEBUG ToolKit script, M00

  Parameters:

  Wander_radius		= DEBUG
  Seconds_to_wait	= DEBUG
*/

DECLARE_SCRIPT(M00_Generic_Wandering_PMP, "Wander_radius=:float,Seconds_to_wait=:float")
{
	Vector3 start_position;
	float Seconds_to_wait;
	float Wander_radius;
	float randomNumber;
	float duration;

	void Created(GameObject* obj)
	{
		start_position = Commands->Get_Position(obj);

		Wander_radius = Get_Float_Parameter("Wander_radius");
		Seconds_to_wait = Get_Float_Parameter("Seconds_to_wait");

		Movement_Complete(obj, MOVEMENT_COMPLETE_ARRIVED);
	}

	void Movement_Complete(GameObject* obj, MovementCompleteReason reason)
	{
		duration = Commands->Get_Random(0, Seconds_to_wait);
		Commands->Display_Text("Setting new timer\n");
		Commands->Start_Timer(obj, duration, MPR_TIMER_WANDER_PMP);
	}

	void Timer_Expired(GameObject* obj, int timerID)
	{
		if (timerID == MPR_TIMER_WANDER_PMP)
		{
			Vector3 newPoint = RandomVector();
			Commands->Display_Text("Timer expired\n");
			Commands->Display_Text("Moving\n");
			Commands->Action_Movement_Set_Forward_Speed(obj, 0.30f);
			Commands->Action_Movement_Goto_Location(obj, newPoint,1.0f);
		}
	}

	Vector3 RandomVector(void)
	{
		Vector3 wanderPoint;

		wanderPoint.X = Commands->Get_Random( 0, Wander_radius );
		wanderPoint.Y = Commands->Get_Random( 0, Wander_radius );
		wanderPoint.Z = 0;
		return (start_position + wanderPoint);
	}
};


/*
MPR_Looping_Path_JDG

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG UNUSED?

  Parameters:

  DestObjID1		= DEBUG
  DestObjID2		= DEBUG
*/

DECLARE_SCRIPT(MPR_Looping_Path_JDG, "DestObjID1=:int, DestObjID2=:int")
{
	int m_CurrentDest;

	void Created(GameObject* game_obj)
	{
		m_CurrentDest = 0;
		Goto_Dest(game_obj);
	}

	void Goto_Dest(GameObject* game_obj)
	{
		//	Find the destination 'object'

		GameObject* dest_obj = Commands->Find_Object(Get_Int_Parameter(m_CurrentDest));

		if (dest_obj != NULL)
		{
			// Instruct our game object to goto the location of the destination object
			Vector3 pos = Commands->Get_Position(dest_obj);
			Commands->Action_Movement_Goto_Location(game_obj, pos, 3.0F);
		}
		else
		{
			DebugPrint("Error! Cannot find destination for MPR_Looping_Path_JDG.\n");
		}
	}
	
	void Movement_Complete(GameObject* game_obj, MovementCompleteReason)
	{
		m_CurrentDest = !m_CurrentDest;
		Goto_Dest(game_obj);
	}
};


/*
MPR_A03_Bridge_Exploding_DEL

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG UNUSED
  
  Parameters:

  AnimationName		= DEBUG
*/

DECLARE_SCRIPT(MPR_A03_Bridge_Exploding_DEL, "AnimationName=:string")
{
	void Created(GameObject* game_obj)
	{
		//	Set the animation to the first frame
		
		const char* animName = Get_Parameter("AnimationName");
		Commands->Set_Animation_Frame(game_obj, animName, 0);
	}

	void Custom(GameObject* game_obj, int type, int, GameObject*)
	{
		//	Is this the request to trigger the explosion?
		// If so, then kick off the animation

		if (type == MPR_CUSTOM_EXPLODE_BRIDGE_DEL) {
			const char* animName = Get_Parameter("AnimationName");
			Commands->Set_Animation(game_obj, animName, false);
		}
	}
};


/*
MPR_A03_Bridge_Trigger_DEL

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG UNUSED
  
  Parameters:

  BridgeID		= DEBUG
*/

DECLARE_SCRIPT(MPR_A03_Bridge_Trigger_DEL, "BridgeID=:int")
{
	void Entered(GameObject* obj, GameObject* enterer)
	{
		//	Did the player enter the zone?

		if (enterer == Commands->Get_The_Star())
		{
			int bridgeID = Get_Int_Parameter("BridgeID");
			GameObject* dest_obj = Commands->Find_Object(bridgeID);
			if (dest_obj != NULL)
			{
				Commands->Send_Custom_Event(obj, dest_obj, MPR_CUSTOM_EXPLODE_BRIDGE_DEL, 0);
			}
		}
	}
};


/*
Demo_Harvester

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  DEBUG UNUSED
  
  Parameters:

  RefineryZoneID	= DEBUG
  TiberiumZoneID	= DEBUG
*/

/*
DEBUG - Harvester Behavior being moved to an innate system - DAY

DECLARE_SCRIPT(Demo_Harvester, "RefineryZoneID:int,TiberiumZoneID:int")
{
	#define MIN_HARVEST_TIME (30)
	#define MAX_HARVEST_TIME (60)

	#define MIN_MOVE_TIME (3)
	#define MAX_MOVE_TIME (5)

	#define MIN_UNLOAD_TIME (15)
	#define MAX_UNLOAD_TIME (30)

	enum States
		{
		IDLE,
		GOING_TO_TIBERIUM,
		HARVESTING,
		MOVING,
		RETURNING_TO_REFINERY,
		DOCKING,
		UNLOADING,
		};

	States mState;
	int mRefineryID;
	int mTiberiumID;

	void Created(GameObject*)
	{
		DebugPrint("Harvester %d: Behavior created\n", Commands->Get_ID(Owner()));

		mRefineryID = Get_Int_Parameter("RefineryZoneID");
		mTiberiumID = Get_Int_Parameter("TiberiumZoneID");

		DebugPrint("RefineryID = %d, TiberiumID = %d\n", mRefineryID, mTiberiumID);

		mState = IDLE;
		GotoTiberium();
	}
	
	Vector3 FindTiberium(void)
	{
		// Locate the tiberium zone and pick a random location within the
		// zone to go to.

		GameObject* tiberium = Commands->Find_Object(mTiberiumID);

		if (tiberium != NULL)
		{
			Vector3 location = Commands->Get_Position(tiberium);
//			Vector3 extent = Commands->Get_Extent(tiberium);		// See Byon for a new way to do this, if needed

			DebugPrint("TiberiumZone Extent %f,%f,%f\n", extent.X, extent.Y, extent.Z);
			location += RandomVector3(extent.X, extent.Y, 0.0f);
			return location;
		}
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	// Go to the tiberium field

	void GotoTiberium(void)
	{
		DebugPrint("Harvester %d: Going to tiberium\n", Commands->Get_ID(Owner()));

		Vector3 location = FindTiberium();
		Commands->Action_Movement_Goto_Location(Owner(), location, 1.0f);
		mState = GOING_TO_TIBERIUM;
	}

	// Harvest tiberium

	void Harvest(void)
	{
		DebugPrint("Harvester %d: Harvesting\n", Commands->Get_ID(Owner()));

		if (mState != HARVESTING)
		{
			Commands->Set_Animation(Owner(), "v_nod_harvester.m_harvesting", true);

			float harvestTime = Commands->Get_Random(MIN_HARVEST_TIME, MAX_HARVEST_TIME);
			DebugPrint("Harvest time %f seconds\n", harvestTime);
			Commands->Start_Timer(Owner(), harvestTime, MPR_TIMER_HARVESTER_DONE_DEL);
		}
		float moveTime = Commands->Get_Random(MIN_MOVE_TIME, MAX_MOVE_TIME);
		DebugPrint("Move time %f seconds\n", moveTime);
		Commands->Start_Timer(Owner(), moveTime, MPR_TIMER_HARVESTER_MOVE_DEL);
		mState = HARVESTING;
	}

	// Move to new location within tiberium field

	void MoveToNewSpot(void)
	{
		Vector3 location = FindTiberium();
		Commands->Action_Movement_Goto_Location(Owner(), location, 1.0f);
		mState = MOVING;
	}

	void ReturnToRefinery(void)
	{
		DebugPrint("Harvester %d: Returning to refinery\n", Commands->Get_ID(Owner()));

		// If there isn't a load and we are at the refinery then just sit idle.
		GameObject* refinery = Commands->Find_Object(mRefineryID);

		if (refinery != NULL)
		{
			Vector3 location = Commands->Get_Position(refinery);
			Commands->Action_Movement_Goto_Location(Owner(), location, 1.0f);
		}

		mState = RETURNING_TO_REFINERY;
	}

	void Dock(void)
	{
		DebugPrint("Harvester %d: Docking\n", Commands->Get_ID(Owner()));
		mState = DOCKING;

		Unload(); // Denzil - Temporary
	}

	void Unload(void)
	{
		DebugPrint("Harvester %d: Unloading\n", Commands->Get_ID(Owner()));
		Commands->Start_Timer(Owner(), 5.0f, MPR_TIMER_HARVESTER_DONE_DEL);
		mState = UNLOADING;
	}

	void Movement_Complete(GameObject* obj, MovementCompleteReason reason)
	{
		DebugPrint("Harvester %d: movement complete\n", Commands->Get_ID(Owner()));

		switch (mState)
		{
			// Upon reaching tiberium begin harvesting.
			case GOING_TO_TIBERIUM:
			case MOVING:
				Harvest();
			break;

			// Upon reaching refinery begin docking sequence.
			case RETURNING_TO_REFINERY:
				Dock();
			break;

			// When docking is complete being unloading.
			case DOCKING:
				Unload();
			break;

			default:
			break;
		}
	}

	void Timer_Expired(GameObject* obj, int timer)
	{
		if (timer == MPR_TIMER_HARVESTER_DONE_DEL)
		{
			DebugPrint("Harvester %d: Done timer expired\n", Commands->Get_ID(Owner()));

			switch (mState)
			{
				// When finished harvesting return to the refinery.
				case HARVESTING:
					ReturnToRefinery();
				break;

				// When finished unloading go back to the tiberium field to resume harvesting.
				case UNLOADING:
					GotoTiberium();
				break;

				default:
				break;
			}
		}
		else if (timer == MPR_TIMER_HARVESTER_MOVE_DEL)
		{
			DebugPrint("Harvester %d: Move timer expired\n", Commands->Get_ID(Owner()));

			if (mState == HARVESTING)
			{
				MoveToNewSpot();
			}
		}
	}
};
*/

/*
MPR_ApacheTrigger

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  
  Parameters:

  ApacheID	= DEBUG
  Position	= DEBUG
*/

DECLARE_SCRIPT(MPR_ApacheTrigger, "ApacheID:int,Position:int")
{
	void Entered(GameObject*, GameObject* enterer)
	{
		GameObject* star = Commands->Get_The_Star();

		// Only trigger if the commando entered the zone

		if (enterer == star)
		{
			DebugPrint("Apache attack triggered!\n");

			// Get apache object
		
			int apacheID = Get_Int_Parameter("ApacheID");
			GameObject* apache = Commands->Find_Object(apacheID);

			// Tell the apache were to move
			
			if (apache != NULL)
			{
				int position = Get_Int_Parameter("Position");
				Commands->Send_Custom_Event(Owner(), apache, MPR_CUSTOM_APACHE_GOTO_POSITION_DEL, position);
			}
		}
	}
};


/*
MPR_ApacheShootAtPlayer

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
  
  Parameters:

  ApacheID	= DEBUG
*/

DECLARE_SCRIPT(MPR_ApacheShootAtPlayer, "ApacheID:int")
{
	void Entered(GameObject*, GameObject* enterer)
	{
		GameObject* star = Commands->Get_The_Star();

		// Only trigger if the commando entered the zone

		if (enterer == star)
		{
			DebugPrint("Apache shoot at player enabled!\n");

			// Get apache object
		
			int apacheID = Get_Int_Parameter("ApacheID");
			GameObject* apache = Commands->Find_Object(apacheID);

			// Tell the apache were to move
			
			if (apache != NULL)
			{
				Commands->Send_Custom_Event(Owner(), apache, MPR_CUSTOM_APACHE_SHOOT_DEL, (int)true);
			}
		}
	}

	void Exited(GameObject*, GameObject* exiter)
	{
		GameObject* star = Commands->Get_The_Star();

		// Only trigger if the commando entered the zone

		if (exiter == star)
		{
			DebugPrint("Apache shoot at player disabled!\n");

			// Get apache object

			int apacheID = Get_Int_Parameter("ApacheID");
			GameObject* apache = Commands->Find_Object(apacheID);

			// Tell the apache were to move

			if (apache != NULL)
			{
				Commands->Send_Custom_Event(Owner(), apache, MPR_CUSTOM_APACHE_SHOOT_DEL, (int)false);
			}
		}
	}
};


/*
MPR_ApacheController

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_ApacheController, "")
{
	typedef enum
	{
		APACHE_POSITION_FIRST = 0,
		APACHE_LANDED = APACHE_POSITION_FIRST,
		APACHE_POSITION_1,
		APACHE_POSITION_2,
		APACHE_POSITION_3,
		APACHE_POSITION_4,
		APACHE_POSITION_COUNT,
	} ApachePosition;

	typedef struct
	{
		const char* TransitionAnim;
		const char* HoverAnim;
	} Transition;

	static Transition _mTransitions[APACHE_POSITION_COUNT]; 

	ApachePosition mCurrentPosition;
	ApachePosition mDesiredPosition;
	bool mCanShoot;

	void Created(GameObject*)
	{
		DebugPrint("Apache attack script created.\n");

		//Denzil:  I added this line for an idle animation.  Change as you need.
		//David Y.
		//Commands->Set_Animation(Owner(), "V_APACHE-E3.M_AP_POSITION1", true);
		// Rotor anim fix

		Vector3 origin(0.0f, 0.0f, 0.0f);
		Commands->Set_Position(Owner(), origin);
		Commands->Set_Facing(Owner(), 0.0f);
		Commands->Set_Animation(Owner(), "M_AP_POSITION1", true);

		mCurrentPosition = APACHE_LANDED;
		mDesiredPosition = APACHE_LANDED;
		mCanShoot = false;
	}

	void Custom(GameObject* apache, int event, int param, GameObject*)
	{
		if (event == MPR_CUSTOM_APACHE_GOTO_POSITION_DEL)
		{
			Goto_Position((ApachePosition)param);
		}
		else if (event == MPR_CUSTOM_APACHE_SHOOT_DEL)
		{
			mCanShoot = ((param == 0) ? false : true);
			Shoot_At_Player();
		}
	}
	
	void Goto_Position(ApachePosition position)
	{
		assert(position >= APACHE_POSITION_FIRST);
		assert(position < APACHE_POSITION_COUNT);

		// If we are not already at the requested position or in the process of
		// going to the requested position.

		if ((position != mCurrentPosition) || (position != mDesiredPosition))
		{
			#if(0)
			
				// If currently in the landed position we must first move to 0.0.0
				// because the animations are relative to that.
				
				if (mCurrentPosition == APACHE_LANDED)
				{
					Vector3 origin(0.0f, 0.0f, 0.0f);
					Commands->Set_Position(Owner(), origin);
					Commands->Set_Facing(Owner(), 0.0f);
				}
			#endif

			mDesiredPosition = position;
			DebugPrint("Apache desired position = %d\n", mDesiredPosition);
			Transition& transition = _mTransitions[mDesiredPosition];
			Commands->Set_Animation(Owner(), transition.TransitionAnim, false);
		}
	}

	void Animation_Complete(GameObject* apache, const char* finishedAnim)
	{
		assert(finishedAnim != NULL);
		DebugPrint("Apache animation '%s' complete.\n", finishedAnim);

		Transition& transition = _mTransitions[mDesiredPosition];

		// Verify that the expected animation has completed.

		const char* animName = strrchr(finishedAnim, '.');
		animName++;

		if (openw3d::string_compare(animName, transition.TransitionAnim) == 0)
		{
			// Arrived at desired position

			mCurrentPosition = mDesiredPosition;
			DebugPrint("Apache arrived at position = %d\n", mCurrentPosition);

			// Get transition for current position

			transition = _mTransitions[mCurrentPosition];
			Commands->Set_Animation(Owner(), transition.HoverAnim, true);
		}
	}

	void Timer_Expired(GameObject* obj, int timerID)
	{
		if (timerID == MPR_TIMER_APACHE_SHOOT_DEL)
		{
			Shoot_At_Player();
		}
	}

	void Shoot_At_Player(void)
	{
		// Begin shooting if we can
		if (mCanShoot == true)
		{
			// Only shoot if the apache is not transitioning.
			if (mCurrentPosition == mDesiredPosition)
			{
				GameObject* star = Commands->Get_The_Star();
				if (star != NULL)
				{
					Vector3 targetPosition = Commands->Get_Position(star);
					targetPosition.X += Commands->Get_Random(-1.0f, 1.0f);
					targetPosition.Y += Commands->Get_Random(-1.0f, 1.0f);
					targetPosition.Z += Commands->Get_Random(0.0f, 2.0f);
					DebugPrint("Apache shooting at location %.02f,%.02f,%.02f\n",targetPosition.X, targetPosition.Y, targetPosition.Z);
					Commands->Trigger_Weapon(Owner(), true, targetPosition, true);
				}
			}
			float fireAtInterval = Commands->Get_Random(0.33f, 1.5f);
			Commands->Start_Timer(Owner(), fireAtInterval, MPR_TIMER_APACHE_SHOOT_DEL);
		}
		else
		{
			Commands->Trigger_Weapon(Owner(), false, Vector3(0.0f, 0.0f, 0.0f), true);
		}
	}

	virtual void Killed(GameObject* obj, GameObject* killer)
	{
		DebugPrint("Apache killed.\n");
		Commands->Create_Explosion_At_Bone("Vehicle Explosion 01", Owner(), "ROOTTRANSFORM", Owner());
	}

	virtual void Damaged(GameObject* obj, GameObject* damager)
	{
		DebugPrint("Apache damaged.\n");
	}
};


/*
MPR_ApacheController

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

MPR_ApacheController::Transition MPR_ApacheController::_mTransitions[APACHE_POSITION_COUNT] = 
{
	// Landed
	{NULL, NULL},

	// Position 1 - Hover above landing pad
	{"m_ap_loitr1", "m_ap_loitr1"},

	// Position 2 - Move into position on the right side and hover
	{"m_ap_move1-2", "m_ap_loitr2"},
	
	// Position 3 - Move into position on front side and hover.
	{"m_ap_move2-3", "m_ap_loitr3"},
	
	// Position 4 - Move into position on the left side and hover
	{"m_ap_move1-4", "m_ap_loitr4"},
};


/*
MPR_A01_Orca_Strike_On_Turret

  This script handles the operations of the ORCA strike on the turret. When the animations that
  are pre-placed upon the map are given a custom to start, they play their own animations and
  handle firing operations. This code will change later to incorporate full firing routines for
  ORCAs, currently being implemented by programming.

  Parameters:

  Target_Number		= What turret this ORCA should be attacking.
  Animation_Name	= What animation to play upon this ORCA.
  Fire_01			= What frame the first fire should be at.
  Fire_02			= What frame the second fire should be at.
  Fire_03			= What frame the third fire should be at.
  Fire_04			= What frame the fourth fire should be at.
  Fire_05			= What frame the fifth fire should be at.
  Fire_06			= What frame the sixth fire should be at.
  DestructionBone	= What bone the explosion animation should be played upon if shot down.
  ExplosionName		= What explosion to play if shot down.

*/

DECLARE_SCRIPT(MPR_A01_Orca_Strike_On_Turret_RAD, "Target_Number:int, AnimationName:string, Fire_01:int, Fire_02:int, Fire_03:int, Fire_04:int, Fire_05:int, Fire_06:int")
{
	Vector3 v_target_loc;
	int n_target_number;
	int n_attack_number;

	void Created(GameObject* p_obj)
	{
		n_target_number = 0;
		n_attack_number = 1;
	}

	void Custom(GameObject* p_obj, int n_type, int n_param, GameObject* p_sender)
	{
		if (n_type == MPR_A01_CUSTOM_ORCA_TURRETSTRIKE_RAD)
		{
			// Set up the appropriate target number for the turret target.
			
			switch(Get_Int_Parameter("Target_Number"))
			{
			case (1):
				{
					if (Commands->Find_Object(MPR_A01_orca_turret1_RAD))
					{
						n_target_number = MPR_A01_orca_turret1_RAD;
					}
					else
					{
						DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_On_Turret - ORCA cannot find its turret 1 target!\n");
						Commands->Destroy_Object(p_obj);
					}
					break;
				}
			case (2):
				{
					if (Commands->Find_Object(MPR_A01_orca_turret2_RAD))
					{
						n_target_number = MPR_A01_orca_turret2_RAD;
					}
					else
					{
						DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_On_Turret - ORCA cannot find its turret 2 target!\n");
						Commands->Destroy_Object(p_obj);
					}
					break;
				}
			default:
				{
					DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_On_Turret - ORCA's Target_Number is inappropriate!\n");
					Commands->Destroy_Object(p_obj);
				}
			}
			
			// Find the location of the turret, and start the attack animation on the ORCA.

			v_target_loc = Commands->Get_Position(Commands->Find_Object(n_target_number));
			v_target_loc.Z += 0.5f;
			Commands->Set_Animation(p_obj, Get_Parameter("AnimationName"), 0);
			
			// Start the timer for the first attack. When this ends the attack happens.

			Commands->Start_Timer(p_obj, (Get_Int_Parameter("Fire_01") / 30), MPR_A01_TIMER_ORCASTRIKE_RAD);
		}
		else
		{
			DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_On_Turret - Orca strike received an improper custom!\n");
			Commands->Destroy_Object(p_obj);
		}
	}

	void Timer_Expired (GameObject* p_obj, int n_timer_id)
	{
		int n_newtimercount;
		int n_timercount;
		n_newtimercount = 0;
		n_timercount = 0;

		switch (n_timer_id)
		{
		case(MPR_A01_TIMER_ORCASTRIKE_RAD):
			{
				// Fire the weapon at the target.

				Commands->Trigger_Weapon(p_obj, true, v_target_loc, true);
				
				n_attack_number++;

				// Start the timer for turning the attack off.

				if (n_attack_number < 8)
				{
					Commands->Start_Timer(p_obj, MPR_A01_orca_attack_stoptime_RAD, MPR_A01_TIMER_ORCASTRIKE_OFF_RAD);
				}
				break;
			}
		case(MPR_A01_TIMER_ORCASTRIKE_OFF_RAD):
			{
				// Turn off the attack, set the timer for the next attack.

				Commands->Trigger_Weapon(p_obj, false, v_target_loc, true);
				
				switch (n_attack_number)
				{
				case (2):
					{
						n_newtimercount = Get_Int_Parameter("Fire_02");
						n_timercount = Get_Int_Parameter("Fire_01");
						n_newtimercount -= n_timercount;
						break;
					}
				case (3):
					{
						n_newtimercount = Get_Int_Parameter("Fire_03");
						n_timercount = Get_Int_Parameter("Fire_02");
						n_newtimercount -= n_timercount;
						break;
					}
				case (4):
					{
						n_newtimercount = Get_Int_Parameter("Fire_04");
						n_timercount = Get_Int_Parameter("Fire_03");
						n_newtimercount -= n_timercount;
						break;
					}
				case (5):
					{
						n_newtimercount = Get_Int_Parameter("Fire_05");
						n_timercount = Get_Int_Parameter("Fire_04");
						n_newtimercount -= n_timercount;
						break;
					}
				case (6):
					{
						n_newtimercount = Get_Int_Parameter("Fire_06");
						n_timercount = Get_Int_Parameter("Fire_05");
						n_newtimercount -= n_timercount;
						break;
					}
				}
				if (n_newtimercount)
				{
					Commands->Start_Timer(p_obj, (n_newtimercount / 30), MPR_A01_TIMER_ORCASTRIKE_RAD);
				}
			}
		}
	}
};


/*
MPR_A01_Orca_Turret

  This script is for the turrets themselves. They simply assign their own ID numbers as per standard.

  Parameters:

  TurretNumber	 = Which turret this script is operating on.
*/

DECLARE_SCRIPT(MPR_A01_Orca_Turret_RAD, "TurretNumber:int")
{
	float f_health;
	int n_turret_number;

	void Created(GameObject* p_obj)
	{
		n_turret_number = Get_Int_Parameter("TurretNumber");
		MPR_A01_startstrike_RAD = false;
		Commands->Set_Health(p_obj, 350.0f);
		f_health = Commands->Get_Health(p_obj);

		switch (n_turret_number)
		{
		case (1):
			{
				MPR_A01_orca_turret1_RAD = Commands->Get_ID(p_obj);
				break;
			}
		case (2):
			{
				MPR_A01_orca_turret2_RAD = Commands->Get_ID(p_obj);
				break;
			}
		default:
			{
				DebugPrint("RAD-ERROR - MPR_A01_Orca_Turret - Turret has no indicated number!\n");
			}
		}
	}

	void Timer_Expired(GameObject* p_obj, int n_timer_id)
	{
		if (n_timer_id == 6543)
		{
			Commands->Create_Sound("TargetAcquired",Commands->Get_Position(Commands->Get_The_Star()),p_obj);
		}
	}

	void Damaged(GameObject* p_obj, GameObject* p_damager)
	{
		if ((p_damager == Commands->Get_The_Star()) && (!MPR_A01_startstrike_RAD))
		{
			GameObject* p_orca1;
			GameObject* p_orca2;
			Vector3		v_zeroorg( 0,0,0 );
			MPR_A01_startstrike_RAD = true;

			p_orca1 = Commands->Create_Object("Orca airstrike", v_zeroorg);
			if (p_orca1)
			{
				Commands->Attach_Script(p_orca1, "MPR_A01_Orca_Strike_On_Turret", "1,v_orcae3.m_orcastrike_a,195,255,310,348,419,459");
				Commands->Send_Custom_Event(p_obj, p_orca1, MPR_A01_CUSTOM_ORCA_TURRETSTRIKE_RAD, 0);
			}
			else
			{
				DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_Spawner - Cannot find proper animation for Orca Strike One!\n");
			}
			p_orca2 = Commands->Create_Object("Orca airstrike", v_zeroorg);
			if (p_orca2)
			{
				Commands->Create_Sound("AirStrikeReady",Commands->Get_Position(Commands->Get_The_Star()),p_obj);
				Commands->Start_Timer(p_obj, 1.5f, 6543);
				Commands->Attach_Script(p_orca2, "MPR_A01_Orca_Strike_On_Turret", "2,v_orcae3.m_orcastrike_b,180,235,295,343,400,437");
				Commands->Send_Custom_Event(p_obj, p_orca2, MPR_A01_CUSTOM_ORCA_TURRETSTRIKE_RAD, 0);
			}
			else
			{
				DebugPrint("RAD-ERROR - MPR_A01_Orca_Strike_Spawner - Cannot find proper animation for Orca Strike Two!\n");
			}
		}
	}
};

/*
MPR_A01_Orca_Turret_Attack_Zone

  This script is specifically for the turret attacks - if the commando enters this zone,
  the turrets start shooting at him if they still exist.
*/

DECLARE_SCRIPT(MPR_A01_Orca_Turret_Attack_Zone_RAD, "")
{
	void Entered(GameObject* p_obj, GameObject* p_enterer)
	{
		if (p_enterer == Commands->Get_The_Star())
		{
			if (Commands->Find_Object(MPR_A01_orca_turret1_RAD))
			{
				Commands->Action_Attack_Object(Commands->Find_Object(MPR_A01_orca_turret1_RAD), p_enterer, 1.0f, 200.0f, true);
			}
			if (Commands->Find_Object(MPR_A01_orca_turret2_RAD))
			{
				Commands->Action_Attack_Object(Commands->Find_Object(MPR_A01_orca_turret2_RAD), p_enterer, 1.0f, 200.0f, true);
			}
		}
	}
};


/*
MPR_A04_Obelisk_Weapon

  This script handles the obelisk weapon firing. It is attached to a zone. 
  It handle the following process:

  0. When the object is created, it starts a firing timer for target switching.
  1. When an object enters the zone, check if it's the player.
  2. If it's not the player, add it to an empty slot in the list of targets.
  3. When the timer expires, check the list for target ids.
  4. If it finds a target ID, it verifies that the object exists.
  5. If it exists, the object is fired upon.
  6. If it does not exist, the object ID is removed from the list.

  Parameters:

  ObeliskWeaponID	= The ID of the Obelisk weapon object.
  NodLightTankID	= The ID of Joe's Nod light tank nearby.
*/

DECLARE_SCRIPT(MPR_A04_Obelisk_Weapon_RAD, "ObeliskWeaponID:int, NodLightTankID:int")
{
	int a_obelisk_targets [6];
	bool b_obelisk_firing;
	int n_obelisk_id;
	int n_nodtank_id;

	void Created(GameObject* p_obj)
	{
		//Commands->Display_Text("RAD - Obelisk zone created, initializing.\n");
		int n_counter;

		b_obelisk_firing = true;
		n_obelisk_id = Get_Int_Parameter("ObeliskWeaponID");

		if (Commands->Find_Object(n_obelisk_id))
		{
			//Commands->Display_Text("RAD - Found the obelisk weapon itself.\n");
			for (n_counter = 0;n_counter < 6;n_counter++)
			{
				a_obelisk_targets [n_counter] = 0;
			}
			Commands->Start_Timer(p_obj, MPR_A01_obelisk_firing_time_RAD, MPR_A04_TIMER_OBELISK_PREFIRING_RAD);
		}
		else
		{
			DebugPrint("RAD-ERROR - MPR_A04_Obelisk_Weapon - Obelisk weapon is missing!\n");
		}
	}

	void Entered(GameObject* p_obj, GameObject* p_enterer)
	{
		//Commands->Display_Text("RAD - Obelisk firing zone entered.\n");
		int n_counter;
		bool b_tankfound;

		b_tankfound = false;

		// Special casing for Joe's Nod tank.

		if (Commands->Find_Object(Get_Int_Parameter("NodLightTankID")))
		{
			GameObject* p_tank = Commands->Find_Object(Get_Int_Parameter("NodLightTankID"));
			if (p_enterer == p_tank)
			{
				b_tankfound = true;
			}
		}
		if (!b_tankfound)
		{
			//Commands->Display_Text("RAD - The proper target has entered the zone.\n");

			for (n_counter = 0;n_counter < 6;n_counter++)
			{
				if (!a_obelisk_targets[n_counter])
				{
					//Commands->Display_Text("RAD - found a blank id slot, inserting.\n");
					a_obelisk_targets[n_counter] = Commands->Get_ID(p_enterer);
					n_counter = 6;
				}
				else
				{
					if (!Commands->Find_Object(a_obelisk_targets[n_counter]))
					{
						//Commands->Display_Text("RAD - found an obsolete id, replacing.\n");
						a_obelisk_targets[n_counter] = Commands->Get_ID(p_enterer);
						n_counter = 6;
					}
				}
			}
		}
	}

	void Timer_Expired(GameObject* p_obj, int n_timer_id)
	{
		int n_counter;
		Vector3 v_obeliskloc;
		Vector3 v_targetloc;
		int n_nontargets;

		n_nontargets = 0;
		v_obeliskloc = 	Commands->Get_Position(Commands->Find_Object(n_obelisk_id));

		if ((b_obelisk_firing) && (n_timer_id == MPR_A04_TIMER_OBELISK_PREFIRING_RAD))
		{
			//Commands->Display_Text("RAD - pre-firing timer expired. Creating explosion.\n");
			//Commands->Create_Explosion("Obelisk Emitter", v_obeliskloc, p_obj);
			Commands->Start_Timer(p_obj, 1.0f, MPR_A04_TIMER_OBELISK_FIRING_RAD);
		}
		if ((b_obelisk_firing) && (n_timer_id == MPR_A04_TIMER_OBELISK_FIRING_RAD))
		{
			//Commands->Display_Text("RAD - obelisk firing timer expired. Looking for a target.\n");
			for (n_counter = 0;n_counter < 6;n_counter++)
			{
				if ((Commands->Find_Object(a_obelisk_targets[n_counter])) && (Commands->Find_Object(n_obelisk_id)))
				{
					v_targetloc = Commands->Get_Position(Commands->Find_Object(a_obelisk_targets[n_counter]));
					//Commands->Display_Text("RAD - shooting at target.\n");
					if (Commands->Find_Object(a_obelisk_targets[n_counter]) != Commands->Get_The_Star())
					{
						Commands->Set_Health(Commands->Find_Object(a_obelisk_targets[n_counter]), 1.0f);
					}
					Commands->Trigger_Weapon(Commands->Find_Object(n_obelisk_id), true, v_targetloc, true);
					Commands->Create_Explosion("Obelisk Explosion", v_targetloc, p_obj);
					a_obelisk_targets[n_counter] = 0;
					n_counter = 6;
				}
				else
				{
					n_nontargets++;
				}
			}
			if (n_nontargets == 6)
			{
				Commands->Trigger_Weapon(Commands->Find_Object(n_obelisk_id), false, v_obeliskloc, true);
			}
			Commands->Start_Timer(p_obj, 2.0f, MPR_A04_TIMER_OBELISK_STOPFIRING_RAD);
		}
		if (n_timer_id == MPR_A04_TIMER_OBELISK_STOPFIRING_RAD)
		{
			//Commands->Display_Text("RAD - obelisk stop firing timer expired. Stopping firing.\n");
			v_obeliskloc = 	Commands->Get_Position(p_obj);
			Commands->Trigger_Weapon(Commands->Find_Object(n_obelisk_id), false, v_obeliskloc, true);
			Commands->Start_Timer(p_obj, MPR_A01_obelisk_firing_time_RAD, MPR_A04_TIMER_OBELISK_PREFIRING_RAD);
		}
	}

	void Custom(GameObject* p_obj, int n_type, int n_param, GameObject* p_sender)
	{
		if (n_type == MPR_A04_CUSTOM_OBELISK_FIRING_RAD)
		{
			//Commands->Display_Text("RAD - obelisk firing being turned off.\n");
			b_obelisk_firing = false;
		}
	}
};


/*
MPR_A04_Main_Control_Terminal

  This script handles the deactivation of the Obelisk. It handles the following process:

  1. When either the Obelisk Main Control Terminal or the Power Plant Main Control Terminal
  are destroyed, the Obelisk receives a custom to disable the firing action.

  ObeliskControllerID = The ID of the obelisk controller zone, to tell it to stop firing.
  BuildingID = The ID of the building that this console is in, for proper sounds and function.

  0 = Hand of Nod
  1 = Tiberium Refinery
  2 = Obelisk
  3 = Power Plant
  4 = Airfield

  Parameters:

  ObeliskControllerID	= The ID number of the Obelisk controller object.
  BuildingID			= Which building this script is operating on.
*/

DECLARE_SCRIPT(MPR_A04_Main_Control_Terminal_RAD, "ObeliskControllerID:int, BuildingID:int")
{
	void Killed(GameObject* p_obj, GameObject* p_killer)
	{
		Vector3 v_consoleloc;
		GameObject* p_created;
		int n_building_id;
		float f_rightbuilding;
		
		n_building_id = Get_Int_Parameter("BuildingID");

		if ((n_building_id == 2) || (n_building_id == 3))
		{
			f_rightbuilding = true;
		}
		else
		{
			f_rightbuilding = false;
		}
		if ((Commands->Find_Object(Get_Int_Parameter("ObeliskControllerID"))) && (f_rightbuilding))
		{
			Commands->Send_Custom_Event(p_obj, Commands->Find_Object(Get_Int_Parameter("ObeliskControllerID")), MPR_A04_CUSTOM_OBELISK_FIRING_RAD, 0);
		}
		else
		{
			DebugPrint("RAD-ERROR - MPR_A04_Main_Control_Terminal - Cannot find Obelisk Controller object!\n");
		}
		v_consoleloc = Commands->Get_Position(p_obj);
		Commands->Create_Explosion("Generic Ground 01", v_consoleloc, p_killer);
		p_created = Commands->Create_Object_At_Bone(p_obj, "NOD Console Off", "TM[0]");
		if (n_building_id != 2)
		{
			if (p_created)
			{
				Vector3 v_created = Commands->Get_Position(p_created);
				Commands->Create_Sound(MPR_control_klaxons_RAD[n_building_id], v_created, p_created);
			}
		}
	}
};


/*
MPR_A05_Tank_Controller

  This script handles the operation of the Mammoth Tank and Flame Tanks in areas 5 and 6. It performs the following
  processes:

  1. If enemies are seen, fire upon the enemies.
  2. If taking damage, move to a new location among preset coordinates.
  3. If the commmando crosses the temple area entry zone, move to the temple area.
  4. If I am a temple guarding tank, I will remain motionless until the zone is entered.

  My_Number = The number of this particular tank for reference purposes.

  1 = Mammoth Tank
  2 = Air dropped Flame Tank
  3 = Temple Guard Flame Tank
  4 = Temple Guard Flame Tank

  Four coordintates are needed:

  Coordinate 1, 2, and 3 = Primary move locations.
  Coordinate 4 = Temple movement coordinate for #1 and #2.

  Parameters:

  My_Number		= What tank number this script is attached to.
  MammothTank	= The ID number of the Mammoth Tank.
*/

DECLARE_SCRIPT(MPR_A05_Tank_Controller_RAD, "My_Number:int, MammothTank:int")
{
	int n_mynumber;
	GameObject* p_target;
	int n_flametank;

	void Created(GameObject* p_obj)
	{
		n_flametank = 0;
		int n_mammoth;
		n_mammoth = 8649;
		MPR_A05_mammoth_tank_id_RAD = n_mammoth;
		n_mynumber = Get_Int_Parameter("My_Number");
		if (n_mynumber == 2)
		{
			Commands->Action_Movement_Goto_Location(p_obj, MPR_A05_tank_move_locations_RAD[1], 2.0f);
			if (Commands->Find_Object(n_mammoth))
			{
				Commands->Send_Custom_Event(p_obj, Commands->Find_Object(n_mammoth), MPR_A05_CUSTOM_MAMMOTH_RAD, Commands->Get_ID(p_obj));
			}
		}
	}

	void Custom(GameObject* p_obj, int n_type, int n_param, GameObject* p_sender)
	{
		if (n_type == MPR_A05_CUSTOM_TEMPLE_ZONE_ENTERED_RAD)
		{
			switch (n_mynumber)
			{
			case (1):
				{
					Commands->Action_Movement_Goto_Location(p_obj, MPR_A05_tank_move_locations_RAD[(n_mynumber - 1)], 2.0f);
					break;
				}
			case (3):
				{
					Commands->Action_Movement_Goto_Location(p_obj, MPR_A05_tank_move_locations_RAD[(n_mynumber - 1)], 2.0f);
					break;
				}
			case (4):
				{
					Commands->Action_Movement_Goto_Location(p_obj, MPR_A05_tank_move_locations_RAD[(n_mynumber - 1)], 2.0f);
					break;
				}
			default:
				{
					DebugPrint("RAD-ERROR - MPR_A05_Tank_Controller - Received an improper custom!\n");
				}
			}
		}
		else if (n_type == MPR_A05_CUSTOM_MAMMOTH_RAD)
		{
			n_flametank = n_param;
		}
		else
		{
			p_target = p_sender;
			Commands->Start_Timer(p_obj, 2.0f, MPR_A05_TIMER_MAMMOTH_RAD);
		}
	}

	void Timer_Expired(GameObject* p_obj, int n_timer_id)
	{
		Vector3 c130_location;

		if (n_timer_id == MPR_A05_TIMER_MAMMOTH_RAD)
		{
			if (p_target)
			{
				c130_location = Commands->Get_Bone_Position(p_target, "BODYMAIN");
				Commands->Action_Attack_Location(p_obj,c130_location, 0.0f, 200.0f, true);
			}
			Commands->Start_Timer(p_obj, 1.0f, MPR_A05_TIMER_MAMMOTH_2_RAD);
		}
		if (n_timer_id == MPR_A05_TIMER_MAMMOTH_2_RAD)
		{
			Commands->Action_Attack_Stop(p_obj);
			Commands->Start_Timer(p_obj, 2.0f, MPR_A05_TIMER_MAMMOTH_3_RAD);
		}
		if (n_timer_id == MPR_A05_TIMER_MAMMOTH_3_RAD)
		{
			if (Commands->Find_Object(n_flametank))
			{
				Commands->Action_Attack_Object(p_obj, Commands->Find_Object(n_flametank), 1.0f, 100.0f, true);
			}
		}
	}
};


/*
MPR_A06_RMV_Final_Zone_Trigger

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A06_Final_Zone_Trigger_RMV, "")
{
	enum
	{
		apache1_die,
		apache2_die,
		end_of_demo,
		pause_animation,
		end_of_whole_thing
	};

	int apache1_id, apache2_id, end_id;
	int havoc_has_entered;

	void Created( GameObject * obj )
	{
		havoc_has_entered = 0;
	}

	void Entered( GameObject * obj, GameObject * enterer )
	{
		float apache1_die_frame = 146;
		float fadeout_frame = 155;

		if ( enterer == Commands->Get_The_Star() && havoc_has_entered == 0 )
		{
			havoc_has_entered = 1;
			GameObject *clouds = Commands->Find_Object(9876);
			if (clouds != NULL)
			{
				Commands->Destroy_Object(clouds);
			}
			Commands->Create_Sound( "Ion Cannon ready", Commands->Get_Position(Commands->Get_The_Star()), obj );			
			Commands->Start_Timer( obj, fadeout_frame/30, end_of_demo );
			Commands->Create_Object( "Outro Aggregate", Vector3(0,0,0) );
			Commands->Create_Sound( "Ion Cannon03", Vector3(-170.0f, 448.0f, 20.0f), obj );
			GameObject *apache1 = Commands->Create_Object( "Apache Outro 1", Vector3(0,0,0) );
			GameObject *apache2 = Commands->Create_Object( "Apache Outro 2", Vector3(0,0,0) );
			Commands->Start_Timer( obj, apache1_die_frame/30, apache1_die );
			apache1_id = Commands->Get_ID( apache1 );
			apache2_id = Commands->Get_ID( apache2 );
		}
	}
	void Timer_Expired( GameObject * obj, int timer_id )
	{
		if ( timer_id == apache1_die ) 
		{
			GameObject *apache1 = Commands->Find_Object( apache1_id );
			GameObject *apache2 = Commands->Find_Object( apache2_id );
			if (apache1 != NULL)
			{
				Vector3 explosion1 = Commands->Get_Bone_Position( apache1, "SEAT" );
				Commands->Create_Explosion( "Generic Air 02", explosion1 );
			}
			if (apache2 != NULL)
			{
				Vector3 explosion2 = Commands->Get_Bone_Position( apache2, "SEAT" );
				Commands->Create_Explosion( "Generic Air 02", explosion2 );
			}
		}
		if (timer_id == end_of_demo)
		{
			GameObject *fadeout = Commands->Create_Object("outro fadeout", Vector3(0,0,0));
			Commands->Create_Sound( "Outro Sound", Vector3(-170.0f, 448.0f, 20.0f), obj );
			end_id = Commands->Get_ID(fadeout);
			Commands->Enable_HUD(0);
			Commands->Control_Enable(Commands->Get_The_Star(), false);
			Commands->Start_Timer( obj, 2.0f, pause_animation);
		}
		if (timer_id == pause_animation)
		{
			GameObject *fadeout = Commands->Find_Object(end_id);
			if (fadeout != NULL)
			{
				Commands->Set_Animation_Frame(fadeout, "fadeout.fadeout", 61);
			}
			Commands->Start_Timer( obj, 4.0f, end_of_whole_thing );
		}
		if (timer_id == end_of_whole_thing)
		{
			Commands->Mission_Complete(true);
		}

	} 
};


/*
MPR_A05_RMV_C130_Zone_Trigger

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A05_C130_Zone_Trigger_RMV, "")
{
	void Entered( GameObject * obj, GameObject * enterer )
	{
		GameObject *aircraft;
		if ( enterer == Commands->Get_The_Star() )
		{
			aircraft = Commands->Create_Object( "C-130 airdrop", Vector3(160.66f,623.06f,0.00f) );
			Commands->Set_Facing( aircraft, 149.54f );
			Commands->Destroy_Object( obj );
		}
	}
};


/*
MPR_A05_RMV_A10_Trigger_When_SAM_Destroyed

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A05_A10_Trigger_When_SAM_Destroyed_RMV, "")
{
	void Killed( GameObject * obj, GameObject * killer )
	{
		Commands->Create_Object( "A-10 with bomb dropper", Vector3(0,0,0) );
		Commands->Create_Sound("AirStrikeReady",Commands->Get_Position(Commands->Get_The_Star()),obj);
		Commands->Start_Timer(obj, 1.5f, 6543);
	}

	void Timer_Expired(GameObject* obj, int n_timer_id)
	{
		if (n_timer_id == 6543)
		{
			Commands->Create_Sound("TargetAcquired",Commands->Get_Position(Commands->Get_The_Star()),obj);
		}
	}
};


/*
MPR_A02_RMV_Zone_Trigger_For_Dogfight

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Zone_Trigger_For_Dogfight_RMV, "")
{
	void Entered(GameObject * obj, GameObject * enterer)
	{
		if (enterer == Commands->Get_The_Star())
		{
			Commands->Create_Object( "Apache Dogfight", Vector3(0,0,0) );
			Commands->Create_Object( "Orca Dogfight", Vector3(0,0,0) );
			Commands->Destroy_Object( obj );
		}
	}
};

/*
MPR_A05_RMV_C130_Dropoff_Animation

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION

  Parameters:

  Cinematic		= DEBUG
  ObjToCreate	= DEBUG
  DropFrame		= DEBUG
  BoneName		= DEBUG
*/

DECLARE_SCRIPT(MPR_A05_C130_Dropoff_Animation_RMV, "Cinematic=:string,ObjToCreate=:string,DropFrame=:int,BoneName=:string")
{
	int debris_1_id, debris_2_id, debris_3_id;

	void Created( GameObject * obj )
	{
		Commands->Set_Animation ( obj, Get_Parameter( "Cinematic" ), 0 );
		int drop_frame = Get_Int_Parameter( "DropFrame" );
		float drop_time = ( float )drop_frame / 30.0;
		Commands->Start_Timer( obj, drop_time, MPR_A05_TIMER_DROP_OBJECT_RMV );
	}

	void Timer_Expired ( GameObject* obj, int Timer_ID )
	{
		if ( Timer_ID == MPR_A05_TIMER_DROP_OBJECT_RMV )
		{
			GameObject* flametank = Commands->Create_Object_At_Bone( obj, Get_Parameter( "ObjToCreate" ), Get_Parameter( "BoneName" ) );
			if (flametank)
			{
				Commands->Attach_Script( flametank,"MPR_A05_Tank_Controller","2" );
			}
			Commands->Send_Custom_Event( obj, Commands->Find_Object( 8649 ), 0, 0 );
			Commands->Start_Timer( obj, 2.0f, MPR_A05_TIMER_DIE_RMV );
		}
		if (Timer_ID == MPR_A05_TIMER_DIE_RMV)
		{
			if ( obj != NULL )
			{
				Commands->Create_Explosion_At_Bone("C-130 Explosion 01", obj, "BODYMAIN", obj);
			}
		}
	}
};


/*
MPR_A05_RMV_A10_Air_Strike

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A05_A10_Air_Strike_RMV, "") 
{
	enum
	{
		GO,
		SHOOT,
		STOP,
		STRAFE,
		NO_STRAFE
	};
	
	void Created( GameObject * obj )
	{
		float go_time = 3.0;
		float shoot_frame = 160;				//Frame 70
		float shoot_time = shoot_frame / 30;
		float stop_frame = 240;					//Frame 150
		float stop_time = stop_frame / 30;	
		float begin_strafe_frame = 420;
		float begin_strafe_time = begin_strafe_frame / 30;
		float stop_strafe_frame = 450;
		float stop_strafe_time = stop_strafe_frame / 30;
		Commands->Start_Timer( obj, go_time, GO);
		Commands->Start_Timer( obj, shoot_time, SHOOT );
		Commands->Start_Timer( obj, stop_time, STOP );
		Commands->Start_Timer( obj, begin_strafe_time, STRAFE );
		Commands->Start_Timer( obj, stop_strafe_time, NO_STRAFE );
	}

	void Timer_Expired ( GameObject* obj, int timer_id )
	{
		if ( timer_id == GO )
		{
			Commands->Set_Animation( obj, "v_gdi-a10e3.m_a10strike_a", 0 );
		}
		if ( timer_id == SHOOT )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(85,665,0), true );
		}
		if ( timer_id == STOP )
		{
			Commands->Trigger_Weapon( obj, false, Vector3(85,665,0), true );
		}
		if ( timer_id == STRAFE )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(-37,679,10), false );
			Commands->Create_Explosion( "Generic Ground 01", Vector3(-37,679,10), obj );
		}
		if ( timer_id == NO_STRAFE )
		{
			Commands->Trigger_Weapon( obj, false, Vector3(-37,679,10), false );
		}
	}

	void Animation_Complete( GameObject * obj, const char * name )
	{
		Commands->Destroy_Object( obj );
	} 
};


/*
MPR_A02_RMV_Apache_Dogfight

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Apache_Dogfight_RMV, "")
{
	void Animation_Complete( GameObject * obj, const char * name )
	{
		Commands->Destroy_Object( obj );
		Commands->Create_Object( "Apache Destroyed", Vector3(0,0,0) );
	}
};


/*
MPR_A02_RMV_Orca_Dogfight

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Orca_Dogfight_RMV, "")
{
	enum
	{
		FIRE_1,
		FIRE_2,
		FIRE_3,
		FIRE_4,
		STOP_FIRE,
	};
	
	void Created( GameObject * obj )
	{
		float fire_1_frame = 80;
		float fire_2_frame = 175;
		float fire_3_frame = 246;
		float fire_4_frame = 285;

		Commands->Start_Timer( obj, fire_1_frame / 30, FIRE_1 );
		Commands->Start_Timer( obj, fire_2_frame / 30, FIRE_2 );
		Commands->Start_Timer( obj, fire_3_frame / 30, FIRE_3 );
		Commands->Start_Timer( obj, fire_4_frame / 30, FIRE_4 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		if ( timer_id == FIRE_1 )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(-6.195326f,-117.866371f,20.833399f), true );
			Commands->Start_Timer(obj, 0.2f, STOP_FIRE);
		}
		if ( timer_id == FIRE_2 )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(107.822525f,76.754219f,23.240044f), true );
			Commands->Start_Timer( obj, 0.2f, STOP_FIRE );
		}
		if ( timer_id == FIRE_3 )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(240.367477f,28.173233f,27.428972f), true );
			Commands->Start_Timer( obj, 0.2f, STOP_FIRE );
		}
		if ( timer_id == FIRE_4 )
		{
			Commands->Trigger_Weapon( obj, true, Vector3(183.254105f,58.453369f,59.090523f), true );
			Commands->Start_Timer( obj, 0.2f, STOP_FIRE );
		}
		if ( timer_id == STOP_FIRE )
		{
			Commands->Trigger_Weapon( obj, false, Vector3(240.367477f,28.173233f,27.428972f), true );
		}
	}
};


/*
MPR_A01_RMV_Intro_Cinematic

  DEBUG DESCRIPTION
  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A01_Intro_Cinematic_RMV, "")
{
	enum
	{
		CAMERA_B_START,
		CAMERA_C_START,
		A10_START,
		BIKEA_START,
		BIKEA_DIE,
		BIKEB_START,
		BIKEB_DIE,
		HAVOC_START,
		ROPE_START,
		EFFECT_START,
		INTRO,
		REMOVE,
		HAVOC_BLEND
	};

	Vector3 start_location;

	void Created(GameObject * obj)
	{
		float camera_end_frame = 251;
		GameObject *camera;

		Commands->Start_Timer( obj, 0.5f, REMOVE );
		camera = Commands->Create_Object("Intro Camera 2", Vector3(0,0,0));
		Commands->Set_Camera_Host(camera);
		Commands->Create_Object("Chinook Intro 2", Vector3(0,0,0));
		Commands->Create_Object("Havoc Intro", Vector3(0,0,0));
		Commands->Create_Object("Rope Intro", Vector3(0,0,0));
		Commands->Start_Timer(obj, camera_end_frame/30, INTRO);
	}

	void Timer_Expired(GameObject * obj, int timer_id)
	{
		if ( timer_id == REMOVE )
		{
			Commands->Control_Enable( Commands->Get_The_Star(), false );
			Commands->Enable_HUD(0);
		}
		
		if ( timer_id == INTRO )
		{
			start_location.X = -118.69f;
			start_location.Y = -190.68f;
			start_location.Z = 3.80f;
			Commands->Set_Position( Commands->Get_The_Star(), start_location );
			Commands->Set_Facing( Commands->Get_The_Star(), 180.0f );
			Commands->Set_Animation( Commands->Get_The_Star(), "s_a_human.m_havok_e3pose", 0 );
			Commands->Set_Camera_Host( NULL );
			Commands->Control_Enable( Commands->Get_The_Star(), true );
			Commands->Enable_HUD(1);
			Commands->Destroy_Object(obj);
		}
	}
};


/*
MPR_A02_DRILL_INSTRUCTOR

The following are the scripts involved with the drill seargent routine.
It is broken up into 6 scripts, 2 zones, the drill seargent, and the three
cadet guys. The following is the script for the Drill Instructor.

  DEBUG NAMING CONVENTION
*/


//insert new drill scripts here
DECLARE_SCRIPT(MPR_A02_Drill_Instructor_JDG, "")
{
	//state is used to keep track of what DI is doing

	typedef enum
	{
		MPR_A02_drill_instructor_idle = 0,
		DI_MOVING,
		DI_INNATE,
	} DIState;

	DIState state;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG. Demo, Drill Instructor, Created and disallowing innate\n" );

		MPR_A02_drill_instructor_id_JDG = Commands->Get_ID (obj);
	}

	void Killed (GameObject* obj, GameObject* Killer)
	{
		if (Killer == (Commands->Get_The_Star()))
		{
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
		}
	}
	
	void Custom (GameObject* obj, int Type, int Param, GameObject* Sender)
	{
		if ((Param == MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG) && (Type == MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Recieved Innate Custom\n");

			state = DI_INNATE;

			float timeDelay = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), timeDelay, MPR_A02_TIMER_DRILL_INSTRUCTOR_ATTACK_DELAY_JDG);
		}


		//When Havoc enters zone come to attention
		if ((Param == MPR_A02_PARAM_CADETS_TEN_HUT_JDG) && (Type == MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Received custom from zone, about to do Ten_Hut\n" );
		
			Ten_Hut (obj);
		}

		//when cadet 1 is at attention start with pushups
		if ((Type == MPR_A02_CUSTOM_CADET_1_JDG) && (Param == MPR_A02_PARAM_CADETS_AT_ATTENTION_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, About to start first pushups\n");
		
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J04C", 0);
			//S_A_HUMAN.H_A_J04C is animation for give order

			Commands->Create_Sound ("KeepYourHeadDown02", (Commands->Get_Position (obj)), obj);
			//insert "Lets start with pushups" for this dialog line

			exercise = PUSHUPS;

			Cadet_Mistake ();
		}

		if ((Type == MPR_A02_CUSTOM_CADET_1_JDG) && (Param == MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, About to Rag on Cadet #1\n");
		
			Rag_On_Cadet (1);
		}

		if ((Type == MPR_A02_CUSTOM_CADET_2_JDG) && (Param == MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, About to Rag on Cadet #2\n");
		
			Rag_On_Cadet (2);
		}

		if ((Type == MPR_A02_CUSTOM_CADET_3_JDG) && (Param == MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, About to Rag on Cadet #3\n");
		
			Rag_On_Cadet (3);
		}
	}

	void Ten_Hut (GameObject* obj)
	{
		//DebugPrint("JDG. Demo, Drill Instructor, Starting Ten_Hut\n");
	
  		//check if cadet_2 exists

		if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Ten_Hut, DI gives order\n");
		
			//instructor faces cadet 2...
			Commands->Action_Attack_Object (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), 1, 0);

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J18C",0);
			//S_A_HUMAN.H_A_J18C is animation for ten-hut

			Commands->Create_Sound ("LookAlive02", (Commands->Get_Position (obj)), obj);
			//insert "Ten Hut" sound here when ready

			//Start reaction delay timers on the Cadets
			if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Ten_Hut, Start C1 delay timer\n");
			
				float timeDelay = Commands->Get_Random (0.5f,1.5f);
				Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_TEN_HUT_DELAY_JDG);
			}

			if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Ten_Hut, Start C2 delay timer\n");
			
				float timeDelay = Commands->Get_Random (0.5f,1.5f);
				Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_TEN_HUT_DELAY_JDG);
			}

			if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Ten_Hut, Start C3 delay timer\n");
			
				float timeDelay = Commands->Get_Random (0.5f,1.5f);
				Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_TEN_HUT_DELAY_JDG);
			}
   		}
	}

	void Cadet_Mistake (void)
	{
		//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, Start\n");
	
		//declare and define local variables here
	
		float mistake;
		mistake = Commands->Get_Random(0,100);

		//there is a 25% chance of mistake
		if (mistake > 220)
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, About to goto Choose_Cadet\n");
		
			Choose_Cadet ();
		}

		//if no mistake tell all cadets to do specific exercise
		else
		{
			if (exercise == PUSHUPS)
			{
				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #1 do pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_PUSHUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #2 do pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_PUSHUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #3 do pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_PUSHUPS_DELAY_JDG);
				}

				DI_Goes_To_New_Position ();
			}

			else if (exercise == SITUPS)
			{
				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #1 do situps\n");
					
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_SITUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #2 do situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_SITUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #3 do situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_SITUPS_DELAY_JDG);
				}

				DI_Goes_To_New_Position ();
			}

			else 
			{
				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #1 do j-jacks\n");
					
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_JUMPING_JACKS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #2 do j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_JUMPING_JACKS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Cadet_Mistake, No mistake, #3 do j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_JUMPING_JACKS_DELAY_JDG);
				}

				DI_Goes_To_New_Position ();	   
			}
		}
	}

	void Choose_Cadet (void)
	{
		//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, Start\n");
	
		float cadet;

		cadet = Commands->Get_Random(0,100);

		//define cadet as a number between 0 and 100
		//choose cadet 1 for mistake

		if ((cadet > 0) && (cadet <= 33))
		{
			if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake\n");
			
				Commands->Create_Sound ("GonnaDieHere02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))), (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)));
				//insert "Doh!" sound when ready

				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J09C", 0);
				//S_A_HUMAN.H_A_J09C is animation for make mistake
			}

			//tell cadet 2-3 to do proper animation
		
			if (exercise == PUSHUPS)
			{
				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #2 pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_PUSHUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #3 pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_PUSHUPS_DELAY_JDG);
				}		
			}

			else if (exercise == SITUPS)
			{

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #2 situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_SITUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #3 situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_SITUPS_DELAY_JDG);
				}	
			}

			// jumping jacks
			
			else 
			{

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #2 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_JUMPING_JACKS_DELAY_JDG);

				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C1 makes mistake, #3 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_JUMPING_JACKS_DELAY_JDG);

				}	
			}
		}

		//choose cadet 2 for mistake

		else if ((cadet > 33) && (cadet <= 66))
		{
			if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake\n");
			
				Commands->Create_Sound ("GonnaDieHere02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))), (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)));

				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J09C", 0);
		
				//S_A_HUMAN.H_A_J09C is animation for make mistake

			}

			//tell cadet 1 and 3 to do proper animation

			if (exercise == PUSHUPS)
			{
				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #1 pushups\n");
			
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_PUSHUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #3 pushups\n");
			
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_PUSHUPS_DELAY_JDG);
				}
				
			}
			else if (exercise == SITUPS)
			{

				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #1 situps\n");
			
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_SITUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #3 situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_SITUPS_DELAY_JDG);
				}			   
			}

			// jumping jacks

			else 
			{

				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #1 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_JUMPING_JACKS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C2 makes mistake, #3 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_JUMPING_JACKS_DELAY_JDG);
				}		
			}
		}

		//choose cadet 3 for mistake

		else
		{
			if (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))
			{
				//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake\n");
			
				Commands->Create_Sound ("GonnaDieHere02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_cadet_3_id_JDG))), (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)));

				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J09C", 0);

				//S_A_HUMAN.H_A_J09C is animation for make mistake
			}

			//tell cadet 1,2 to do proper animation

			if (exercise == PUSHUPS)
			{
				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #2 pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_PUSHUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #1 pushups\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_PUSHUPS_DELAY_JDG);
				}		
			}

			else if (exercise == SITUPS)
			{

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #2 situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_SITUPS_DELAY_JDG);
				}

				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #1 situps\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_SITUPS_DELAY_JDG);
				}	
			}

			// jumping jacks

			else 
			{

				if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #2 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_JUMPING_JACKS_DELAY_JDG);

				}

				if (Commands->Find_Object(MPR_A02_cadet_1_id_JDG))
				{
					//DebugPrint("JDG. Demo, Drill Instructor, Choose_Cadet, C3 makes mistake, #1 j-jacks\n");
				
					float timeDelay = Commands->Get_Random (0.5f,1.5f);
					Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_JUMPING_JACKS_DELAY_JDG);
				}			
			}
		}		  
	}

	void Rag_On_Cadet (int Cadet_Number)
	{
		
		//DebugPrint("JDG. Demo, Drill Instructor, Rag_On_Cadet, start\n");
	

		if ((Cadet_Number == 1) && (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Rag_On_Cadet, DI rags on C1\n");
		
			//make drill instructor face cadet 1

			Commands->Action_Attack_Object((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)),(Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), 1, 0);

			Vector3 C1_position (MPR_A02_CADET_1_X_JDG, MPR_A02_CADET_1_Y_JDG, MPR_A02_CADET_1_Z_JDG);
			
			//Drill instructor goes to cadet 1's position

			Commands->Action_Movement_Goto_Location ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), C1_position, 0.5f);			
		}

		if ((Cadet_Number == 2) && (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Rag_On_Cadet, DI rags on C2\n");
		
			//make drill instructor face cadet 2

			Commands->Action_Attack_Object((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)),(Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), 1, 0);

			Vector3 C2_position (MPR_A02_CADET_2_X_JDG, MPR_A02_CADET_2_Y_JDG, MPR_A02_CADET_2_Z_JDG);
			
			//Drill instructor goes to cadet 2's position

			Commands->Action_Movement_Goto_Location ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), C2_position, 0.5f);			
		}

		if ((Cadet_Number == 3) && (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Rag_On_Cadet, DI rags on C3\n");
		
			//make drill instructor face cadet 3

			Commands->Action_Attack_Object((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)),(Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), 1, 0);

			Vector3 C3_position (MPR_A02_CADET_3_X_JDG, MPR_A02_CADET_3_Y_JDG, MPR_A02_CADET_3_Z_JDG);
			
			//Drill instructor goes to cadet 3's position

			Commands->Action_Movement_Goto_Location ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), C3_position, 0.5f);			
		}

		Commands->Set_Animation ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), "S_A_HUMAN.H_A_J07C", 0);

		//S_A_HUMAN.H_A_J07C is animation for rag on cadet

		Yell_At_Cadet ();

		//sets up random things to yell at cadet guy	
	}


	void Yell_At_Cadet (void)
	{
		
		//DebugPrint("JDG. Demo, Drill Instructor, Yell_At_Cadet, start\n");
	
		float n = Commands->Get_Random(0, 100);

		if ((n > 0) && (n <= 25)  && (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)))
		{
			Commands->Create_Sound ("KillHim02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))); 
		}

		else if ((n > 25) && (n <= 50) && (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)))
		{
			Commands->Create_Sound ("KeepYourHeadDown02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));
		}

		else if ((n < 50) && (n <= 75) && (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)))
		{
			Commands->Create_Sound ("SoundAlarm02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));
    	}

		else 
		{
			Commands->Create_Sound ("FireAtWill02", (Commands->Get_Position (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{	

		//S_A_HUMAN.H_A_J07C is animation for rag on cadet

		if ((openw3d::string_compare("S_A_HUMAN.H_A_J07C", name) == 0) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Animation_Complete\n");
		
			DI_Goes_To_New_Position ();
		}

		if ((openw3d::string_compare("S_A_HUMAN.H_A_DIV1", name) == 0) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)) && (state == DI_INNATE))
		{
			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 1.0f, 50.0f);

			//Commands->Set_Animation (obj, "S_A_HUMAN.H_A_C2C0", 1);	
		}

	}

	void DI_Goes_To_New_Position (void)
	{
		if (state != DI_INNATE)
		{
		
			//DebugPrint("JDG. Demo, Drill Instructor, DI_Goes_To_New_Position, start\n");
	
			float xnot, ynot, znot;
			float random_number = Commands->Get_Random (0,100);

			if ((random_number > 0) && (random_number <= 25))
			{
				xnot = -56.94f;
				ynot = -11.48f;
				znot = 2.42f;
			}

			else if ((random_number > 25) && (random_number <= 50))
			{
				xnot = -56.13f;
				ynot = -10.00f;
				znot = 2.42f;
			}

			else if ((random_number > 50) && (random_number <= 75))
			{
				xnot = -54.70f;
				ynot = -8.86f;
				znot = 2.42f;
			}

			else 
			{
				xnot = -53.21f;
				ynot = -8.00f;
				znot = 2.42f;
			}

			Vector3 New_Position(xnot, ynot, znot);

			if (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG))
			{
				Commands->Action_Movement_Set_Forward_Speed ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), .15f);
				Commands->Action_Movement_Goto_Location ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), New_Position, 0.2f);

				float delayTimer = Commands->Get_Random(10,15);
				Commands->Start_Timer ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), delayTimer, MPR_A02_TIMER_DI_PRE_GIVE_ORDER_JDG);

				//make DI wait for 10-15 secs
				
				state = DI_MOVING;
			}
		}
	}

	void Movement_Complete (GameObject* obj, MovementCompleteReason)
	{	
		if ((state == DI_MOVING) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))
		{
			//when DI reaches new spot, face cadets and start new timer

			if (Commands->Find_Object(MPR_A02_cadet_2_id_JDG))
			{
			//	DebugPrint("JDG. Demo, Drill Instructor, Movement_Complete, start\n");
			
				//first, make instructor face cadets...

				Commands->Action_Attack_Object (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), 1, 0);
				state = MPR_A02_drill_instructor_idle;
			}
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A02_TIMER_DI_PRE_GIVE_ORDER_JDG) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)) && (state != DI_INNATE))
		{
			//DebugPrint("JDG. Demo, Drill Instructor, Timer_Expired, start\n");
		
			Next_Exercise ();
		}

		if ((Timer_ID == MPR_A02_TIMER_DRILL_INSTRUCTOR_ATTACK_DELAY_JDG) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)) && (state == DI_INNATE))
		{
			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 0, 0);

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_DIV1", 0);
			//forward roll
		}
	}

	void Next_Exercise (void)
	{
		//DebugPrint("JDG. Demo, Drill Instructor, Next_Exercise, start\n");
	
		exercise = (Exercise)(exercise + 1);

		if (exercise >= JUMPING_JACKS)
		{
			exercise = PUSHUPS;
		}

		if ((exercise == PUSHUPS) && (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))
		{
			//Omega = 1 is pushups
			//have DI do a command animation
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), "S_A_HUMAN.H_A_J04C", 0);
			//S_A_HUMAN.H_A_J04C is animation for give order

			//have DI tell cadets to start with pushups
			Commands->Create_Sound ("LookAlive02", (Commands->Get_Position ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));

			Cadet_Mistake ();
		}

		else if (exercise == SITUPS)
		{
			//Omega = 2 is situps
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), "S_A_HUMAN.H_A_J04C", 0);
			//S_A_HUMAN.H_A_J04C is animation for give order

			//have DI tell cadets to start with pushups
			Commands->Create_Sound ("HoldYourPositions02", (Commands->Get_Position ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));

			Cadet_Mistake ();
		}

		else
		{
			//Omega = 3 is situps
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), "S_A_HUMAN.H_A_J04C", 0);
			//S_A_HUMAN.H_A_J04C is animation for give order

			//have DI tell cadets to start with pushups
			Commands->Create_Sound ("KeepYourHeadDown02", (Commands->Get_Position ((Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)))), (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)));

			Cadet_Mistake ();
		}
	}	
};


/*
MPR_A02_DRILL_Zone

The following is the script for the kill zone

  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Drill_Start_Zone_JDG, "")
{
	//declare variables here
	int count;

	void Created(GameObject* obj)
	{
		MPR_A02_drill_zone_id_JDG = Commands->Get_ID (obj);

		count = 0;
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (count == 0))
		{
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG, MPR_A02_PARAM_CADETS_TEN_HUT_JDG);
			
			//DebugPrint("JDG. Demo, Zone sending custom to Drill instructor\n");
		
			count++;
		}
	}
};


/*
MPR_A02_DRILL_CADET_1

The following is the script for cadet number 1

  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Drill_Cadet_1_JDG, "")
{
	typedef enum
	{
		MPR_A02_cadet_1_id_JDGLE = 0,
		CADET_1_INNATE,
	} C1State;

	C1State state;

	//declare variables here

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG. Demo, Cadet 1, Created and turning off innate\n");

		MPR_A02_cadet_1_id_JDG = Commands->Get_ID (obj);

		Vector3 C1_Start_Spot(MPR_A02_CADET_1_X_JDG, MPR_A02_CADET_1_Y_JDG, MPR_A02_CADET_1_Z_JDG);

		Commands->Action_Movement_Goto_Location (obj, C1_Start_Spot, .02f);

		Vector3 Cadet_Face_Spot(-56.02f, -9.09f, 2.42f);
		Commands->Action_Attack_Location(obj, Cadet_Face_Spot, 0, 0);
	}

	void Killed (GameObject* obj, GameObject* Killer)
	{
		if (Killer == (Commands->Get_The_Star()))
		{
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
		}
	}

	void Custom (GameObject* obj, int Type, int Param, GameObject* Sender)
	{
		if ((Param == MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG) && (Type == MPR_A02_CUSTOM_CADET_1_JDG))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Recieved Innate Custom\n");
	
			state = CADET_1_INNATE;

			float timeDelay = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_ATTACK_DELAY_JDG);
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A02_TIMER_CADET_1_ATTACK_DELAY_JDG) && (state == CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Attack Havoc Timer_Expired, start\n");
	
			Commands->Action_Movement_Set_Forward_Speed(obj, 0.55f);

			Vector3 C1_Attack_Spot(-51.25f, -6.41f, 2.42f);
			Commands->Action_Movement_Goto_Location (obj, C1_Attack_Spot, .02f);

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_D2A3", 0);
			//"S_A_HUMAN.H_A_D2A3" is animation for running to left

			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 150.0f, 50);
		}


		if ((Timer_ID == MPR_A02_TIMER_CADET_1_TEN_HUT_DELAY_JDG) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Timer_Expired, start\n");
	
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_A0A0", 0);
			//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_1_PUSHUPS_DELAY_JDG) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Pushups_Timer_Expired, start\n");
	
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
			//S_A_HUMAN.H_A_J13B is animation for pushups
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_1_SITUPS_DELAY_JDG) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Situps_Timer_Expired, start\n");
	
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J13C", 0);
			//S_A_HUMAN.H_A_J14C is animation for situps
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_1_JUMPING_JACKS_DELAY_JDG) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, JJacks_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
			//S_A_HUMAN.H_A_J43B is animation for j-jacks
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_1_MISTAKE_DELAY_JDG) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Mistake_Timer_Expired, start\n");
		
			if (exercise == PUSHUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
				//S_A_HUMAN.H_A_J13B is animation for pushups
			}

			else if (exercise == SITUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
				//S_A_HUMAN.H_A_J14C is animation for situps
			}

			else
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
				//S_A_HUMAN.H_A_J43B is animation for j-jacks
			}
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43C") == 0) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J13A", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43A") == 0) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J43B", 1);
		}
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13C") == 0) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13A") == 0) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), "S_A_HUMAN.H_A_J13B", 1);
		}

		//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		if ((openw3d::string_compare (name,"S_A_HUMAN.H_A_A0A0") ==0 ) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Animation_Complete, Come to Attention\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_AT_ATTENTION_JDG);
		}
	 

		//S_A_HUMAN.H_A_J09C is animation for make mistake
		if ((openw3d::string_compare (name, "S_A_HUMAN.H_A_J09C") == 0 ) && (state != CADET_1_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Animation_Complete, Make Mistake\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG);

			float timeDelay = Commands->Get_Random (0.5f,1.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_1_MISTAKE_DELAY_JDG);
		}
	}
};


/*
MPR_A02_DRILL_CADET_2

The following is the script for cadet number 2

  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Drill_Cadet_2_JDG, "")
{
	typedef enum
	{
		MPR_A02_cadet_2_id_JDGLE = 0,
		CADET_2_INNATE,
	} C2State;

	C2State state;

	//declare variables here

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG. Demo, Cadet 2, Created and turning off innate\n");

		MPR_A02_cadet_2_id_JDG = Commands->Get_ID (obj);

		Vector3 C2_Start_Spot(MPR_A02_CADET_2_X_JDG, MPR_A02_CADET_2_Y_JDG, MPR_A02_CADET_2_Z_JDG);

		Commands->Action_Movement_Goto_Location (obj, C2_Start_Spot, .02f);

		Vector3 Cadet_Face_Spot(-56.02f, -9.09f, 2.42f);
		Commands->Action_Attack_Location(obj, Cadet_Face_Spot, 0, 0); 
	}

	void Killed (GameObject* obj, GameObject* Killer)
	{
		if (Killer == (Commands->Get_The_Star()))
		{
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
		}
	}

	void Custom (GameObject* obj, int Type, int Param, GameObject* Sender)
	{
		if ((Param == MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG) && (Type == MPR_A02_CUSTOM_CADET_2_JDG))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Recieved Innate Custom\n");
		
			state = CADET_2_INNATE;

			float timeDelay = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_ATTACK_DELAY_JDG);
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A02_TIMER_CADET_2_ATTACK_DELAY_JDG) && (state == CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Attack Havoc Timer_Expired, start\n");
		
			Commands->Action_Movement_Set_Forward_Speed(obj, 0.25f);

			Vector3 C2_Attack_Spot(-50.03f, -13.23f, 2.42f);
			Commands->Action_Movement_Goto_Location (obj, C2_Attack_Spot, .02f);

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_D2B2", 0);
			//"S_A_HUMAN.H_A_D2B2" is animation for walking backwards

			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 150.0f, 50);
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_2_TEN_HUT_DELAY_JDG) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Timer_Expired, start\n");
		
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_A0A0", 0);
			//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_2_PUSHUPS_DELAY_JDG) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Pushups_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
			//S_A_HUMAN.H_A_J13B is animation for pushups
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_2_SITUPS_DELAY_JDG) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Situps_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J13C", 0);
			//S_A_HUMAN.H_A_J14C is animation for situps
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_2_JUMPING_JACKS_DELAY_JDG) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, JJacks_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
			//S_A_HUMAN.H_A_J43B is animation for j-jacks
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_2_MISTAKE_DELAY_JDG) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Mistake_Timer_Expired, start\n");
		
			if (exercise == PUSHUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
				//S_A_HUMAN.H_A_J13B is animation for pushups
			}

			else if (exercise == SITUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
				//S_A_HUMAN.H_A_J14C is animation for situps
			}

			else
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
				//S_A_HUMAN.H_A_J43B is animation for j-jacks
			}
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43C") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J13A", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43A") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J43B", 1);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13C") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13A") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), "S_A_HUMAN.H_A_J13B", 1);
		}

		//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_A0A0") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Animation_Complete, Come to Attention\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_AT_ATTENTION_JDG);
		}

		//S_A_HUMAN.H_A_J09C is animation for make mistake
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J09C") == 0) && (state != CADET_2_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Animation_Complete, Make Mistake\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG);

			float timeDelay = Commands->Get_Random (0.5f,1.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_2_MISTAKE_DELAY_JDG);
		}
	}
};


/*
MPR_A02_DRILL_CADET_3

The following is the script for cadet number 3

  DEBUG NAMING CONVENTION
*/

DECLARE_SCRIPT(MPR_A02_Drill_Cadet_3_JDG, "")
{
	typedef enum
	{
		MPR_A02_cadet_3_id_JDGLE = 0,
		CADET_3_INNATE,
	} C3State;

	C3State state;

	//declare variables here

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG. Demo, Cadet 3, Created and turning off innate\n");

		MPR_A02_cadet_3_id_JDG = Commands->Get_ID (obj);

		Vector3 C3_Start_Spot(MPR_A02_CADET_3_X_JDG, MPR_A02_CADET_3_Y_JDG, MPR_A02_CADET_3_Z_JDG);

		Commands->Action_Movement_Goto_Location (obj, C3_Start_Spot, .02f);

		Vector3 Cadet_Face_Spot(-56.02f, -9.09f, 2.42f);
		Commands->Action_Attack_Location(obj, Cadet_Face_Spot, 0, 0);		
	}

	void Killed (GameObject* obj, GameObject* Killer)
	{
		if (Killer == (Commands->Get_The_Star()))
		{
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
		}
	}

	void Custom (GameObject* obj, int Type, int Param, GameObject* Sender)
	{
		if ((Param == MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG) && (Type == MPR_A02_CUSTOM_CADET_3_JDG))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Recieved Innate Custom\n");

			state = CADET_3_INNATE;

			float timeDelay = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_ATTACK_DELAY_JDG);
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A02_TIMER_CADET_3_ATTACK_DELAY_JDG) && (state == CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Attack Havoc Timer_Expired, start\n");
		
			Commands->Action_Movement_Set_Forward_Speed(obj, 0.55f);

			Vector3 C3_Attack_Spot(-55.32f, -16.55f, 2.42f);
			Commands->Action_Movement_Goto_Location (obj, C3_Attack_Spot, .02f);

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_D2A4", 0);
			//"S_A_HUMAN.H_A_D2A4" is animation for running to right

			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 150.0f, 50);
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_3_TEN_HUT_DELAY_JDG) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Timer_Expired, start\n");
		
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_A0A0", 0);
			//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_3_PUSHUPS_DELAY_JDG) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 1, Pushups_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
			//S_A_HUMAN.H_A_J13A is animation for transition into pushups
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_3_SITUPS_DELAY_JDG) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Situps_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J13C", 0);
			//S_A_HUMAN.H_A_J14C is animation for situps
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_3_JUMPING_JACKS_DELAY_JDG) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, JJacks_Timer_Expired, start\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
			//S_A_HUMAN.H_A_J43B is animation for j-jacks
		}

		if ((Timer_ID == MPR_A02_TIMER_CADET_3_MISTAKE_DELAY_JDG) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Mistake_Timer_Expired, start\n");
		
			if (exercise == PUSHUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J43C", 0);
				//S_A_HUMAN.H_A_J13B is animation for pushups
			}

			else if (exercise == SITUPS)
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
				//S_A_HUMAN.H_A_J13C is animation for transition outof pushups
			}

			else
			{
				Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J43A" , 0);
				//S_A_HUMAN.H_A_J43B is animation for j-jacks
			}
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43C") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J13A", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J43A") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J43B", 1);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13C") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J14C", 1);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J13A") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Pushup transition Complete, Start pushups\n");
		
			Commands->Set_Animation ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), "S_A_HUMAN.H_A_J13B", 1);
		}

		//S_A_HUMAN.H_A_A0A0 is animation for come to attention
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_A0A0") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 3, Animation_Complete, Come to Attention\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_AT_ATTENTION_JDG);
		}

		//S_A_HUMAN.H_A_J09C is animation for make mistake
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J09C") == 0) && (state != CADET_3_INNATE))
		{
			//DebugPrint("JDG. Demo, Cadet 2, Animation_Complete, Make Mistake\n");
		
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_MADE_MISTAKE_JDG);

			float timeDelay = Commands->Get_Random (0.5f,1.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), timeDelay, MPR_A02_TIMER_CADET_3_MISTAKE_DELAY_JDG);
		}
	}
};


/*
MPR_A02_Drill_Kill_Havoc_Zone

  The following script is for two zones next to the exercise routine that lets the exercise guys know when

*/

DECLARE_SCRIPT(MPR_A02_Drill_Kill_Havoc_Zone_JDG, "")
{
	int Havoc_in_this_Zone;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, Drill Instructor, Kill Havoc Zone, Created okay\n");
	
		Havoc_in_this_Zone = 0;
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//DebugPrint("JDG, PRDemo, Drill Instructor, Havoc has penetrated Kill Havoc Zone, Sending Innate Customs\n");
	
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_drill_instructor_id_JDG)), MPR_A02_CUSTOM_DRILL_INSTRUCTOR_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_1_id_JDG)), MPR_A02_CUSTOM_CADET_1_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_2_id_JDG)), MPR_A02_CUSTOM_CADET_2_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_cadet_3_id_JDG)), MPR_A02_CUSTOM_CADET_3_JDG, MPR_A02_PARAM_CADETS_GOTO_INNATE_JDG);
		}
	}
};


/*
MPR_A04_SuicideRun_StartZone

  The following are the scripts involved with the Obelisk suicide run.
  It is broken up into 7 scripts, start zone, GDI minigunner 1, GDI minigunner 2,
  GDI medium tank, NOD minigunner, NOD light tank, and the control console.

*/
DECLARE_SCRIPT(MPR_A04_SuicideRun_StartZone_JDG, "")
{
	int Havoc_in_this_Zone;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, StartZone, Created okay\n");
	
		Havoc_in_this_Zone = 0;

		MPR_A04_suiciderun_enterzone_id_JDG = Commands->Get_ID (obj);
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, Havoc has penetrated StartZone, Start GDI Suicide Team Initial Motion\n");
	
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.20f);
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.20f);
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.35f);

			Vector3 NewPosition(203.40f, 364.33f, 1.04f);
			Commands->Action_Movement_Goto_Location((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), NewPosition, 0.01f);

			Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 6.0f);
			Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 6.0f);
			
			float timeDelay_1 = Commands->Get_Random (8.5f,12.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_NOD_MINIGUNNER_HAVOC_IN_ZONE_DELAY_JDG);

			Havoc_in_this_Zone++;
		}
	}
};


/*
MPR_A04_SuicideRun_GDI_Minigunner_1

	The following is the script for the GDI minigunner by the obelisk.  Its pretty tightly scripted and fragile.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_GDI_Minigunner_1_JDG, "")
{
	typedef enum
	{
		MPR_A04_MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_IDLE_JDG = 0,
		MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_MOVING_TO_OBELISK,
		
		
	} GDIM1State;

	GDIM1State M1state;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 1, Created okay\n");
	
		MPR_A04_suiciderun_gdi_minigunner_1_id_JDG = Commands->Get_ID (obj);

		M1state = MPR_A04_MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_IDLE_JDG;
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_CHASE_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 1 ChaseDelay Timer Expired\n");
	
			if (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG))
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.75f);

				Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), 10.0f);

				Commands->Action_Attack_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), 0.0f, 50);
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_FOLLOW_TANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 1 Follow Tank Timer Expired\n");

			if (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG))
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.5f);

				Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 6.0f);

				Commands->Action_Attack_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.0f, 0);
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_ATTACK_LIGHTTANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)))
		{
			Vector3 Escort_1_go_to_Spot(153.19f, 406.50f, 0.65f);
			Commands->Action_Movement_Goto_Location (obj, Escort_1_go_to_Spot, 5.5f);

			Commands->Action_Attack_Object(obj, (Commands->Find_Object(MPR_A04_suiciderun_nod_light_tank_id_JDG)), 0.0f, 20.f);
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_ATTACK_OBELISK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)))
		{
			if ( status == MPR_A04_SUICIDERUN_CONSOLE_ALIVE)
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.35f);

				float Xnot = Commands->Get_Random (90.69f, 110.69f);
				float Ynot = Commands->Get_Random (396.69f, 416.69f);

				Vector3 Obelisk_Move_to_Location(Xnot, Ynot, 3.37f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

				Vector3 Obelisk_Attack_Location(81.60f, 403.71f, 8.77f);
				Commands->Action_Attack_Location(obj, Obelisk_Attack_Location,10.0f, 50.0f);
			}

			else
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.25f);

				Vector3 Obelisk_Move_to_Location(108.19f, 405.22f, 1.35f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

				M1state = MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_MOVING_TO_OBELISK;

			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_OBELISK_IS_DEAD_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)) && (state == MPR_A04_SUICIDERUN_LIGHTTANK_DEAD))
		{
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), 0.25f);

			Vector3 Obelisk_Move_to_Location(108.19f, 405.22f, 1.35f);
			Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

			M1state = MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_MOVING_TO_OBELISK;
		}
	}

	void Movement_Complete (GameObject* obj, MovementCompleteReason)
	{	
			
		if ((M1state == MPR_A04_SUICIDERUN_GDI_MINIGUNNER_1_MOVING_TO_OBELISK) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)))
		{
			Commands->Set_Animation(obj, "S_A_HUMAN.H_A_J33C", 0);

		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J33C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J15C", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J15C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J03C", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J03C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J33C", 0);
		}
	}
};


/*
MPR_A04_SuicideRun_GDI_Minigunner_2

	The following is the script for the GDI minigunner by the obelisk.  Its pretty tightly scripted and fragile.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_GDI_Minigunner_2_JDG, "")
{
	typedef enum
	{
		MPR_A04_SUICIDERUN_GDI_MINIGUNNER_2_MOVING_TO_OBELISK = 0,
		
		
	} GDIM2State;

	GDIM2State M2state;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 2, Created okay\n");

		MPR_A04_suiciderun_gdi_minigunner_2_id_JDG = Commands->Get_ID (obj);
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_CHASE_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 2 ChaseDelay Timer Expired\n");

			if (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG))
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.75f);

				Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), 10.0f);

				Commands->Action_Attack_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), 0.0f, 50);
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_FOLLOW_TANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, GDI Minigunner 2 Follow Tank Timer Expired\n");

			if (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG))
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.5f);

				Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 6.0f);

				Commands->Action_Attack_Object((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.0f, 0);
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_ATTACK_LIGHTTANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)))
		{
			Vector3 Escort_2_go_to_Spot(163.07f, 403.31f, 1.40f);
			Commands->Action_Movement_Goto_Location (obj, Escort_2_go_to_Spot, 5.5f);

			Commands->Action_Attack_Object(obj, (Commands->Find_Object(MPR_A04_suiciderun_nod_light_tank_id_JDG)), 0.0f, 20.f);
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_ATTACK_OBELISK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)))
		{
			if ( status == MPR_A04_SUICIDERUN_CONSOLE_ALIVE)
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.35f);

				float Xnot = Commands->Get_Random (90.69f, 110.69f);
				float Ynot = Commands->Get_Random (396.69f, 416.69f);

				Vector3 Obelisk_Move_to_Location(Xnot, Ynot, 3.37f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

				Vector3 Obelisk_Attack_Location(81.60f, 403.71f, 8.77f);
				Commands->Action_Attack_Location(obj, Obelisk_Attack_Location,10.0f, 50.0f);
			}

			else
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.25f);

				Vector3 Obelisk_Move_to_Location(97.86f, 411.18f, 2.97f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

				M2state = MPR_A04_SUICIDERUN_GDI_MINIGUNNER_2_MOVING_TO_OBELISK;
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_OBELISK_IS_DEAD_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)) && (state == MPR_A04_SUICIDERUN_LIGHTTANK_DEAD))
		{
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), 0.25f);

			Vector3 Obelisk_Move_to_Location(97.86f, 411.18f, 2.97f);
			Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

			M2state = MPR_A04_SUICIDERUN_GDI_MINIGUNNER_2_MOVING_TO_OBELISK;
		}
	}

	void Movement_Complete (GameObject* obj, MovementCompleteReason)
	{	
			
		if ((M2state == MPR_A04_SUICIDERUN_GDI_MINIGUNNER_2_MOVING_TO_OBELISK) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)))
		{
			Commands->Set_Animation(obj, "S_A_HUMAN.H_A_J09C", 0);
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J09C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J10C", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J10C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J11C", 0);
		}

		if ((openw3d::string_compare(name, "S_A_HUMAN.H_A_J11C") == 0)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J09C", 0);
		}
	}
};


/*
MPR_A04_SuicideRun_GDI_Medium_Tank

	The following is the script for the GDI medium tank by the obelisk.  Its pretty tightly scripted and fragile.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_GDI_Medium_Tank_JDG, "")
{
	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, GDI Medium Tank, Created okay\n");
	
		MPR_A04_suiciderun_gdi_medium_tank_id_JDG = Commands->Get_ID (obj);
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_ATTACK_LIGHTTANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)))
		{
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.45f);

			Vector3 Tank_go_to_Spot(201.56f, 374.40f, 1.92f);
			Commands->Action_Movement_Goto_Location (obj, Tank_go_to_Spot, 0.5f);

			Commands->Action_Attack_Object(obj, (Commands->Find_Object(MPR_A04_suiciderun_nod_light_tank_id_JDG)), 0.0f, 100.f);
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_ATTACK_OBELISK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)))
		{
			if ( status == MPR_A04_SUICIDERUN_CONSOLE_ALIVE)
			{
				float Xnot = Commands->Get_Random (90.69f, 110.69f);
				float Ynot = Commands->Get_Random (396.69f, 416.69f);

				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.25f);

				Vector3 Obelisk_Move_to_Location(Xnot, Ynot, 3.37f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

				Vector3 Obelisk_Attack_Location(81.60f, 403.71f, 8.77f);
				Commands->Action_Attack_Location(obj, Obelisk_Attack_Location,10.0f, 50.0f);
			}
			
			else
			{
				Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.25f);

				Vector3 Obelisk_Move_to_Location(95.20f, 405.23f, 4.3f);
				Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);
			}
		}

		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_OBELISK_IS_DEAD_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)) && (state == MPR_A04_SUICIDERUN_LIGHTTANK_DEAD))
		{
			Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 0.25f);

			Vector3 Obelisk_Move_to_Location(95.20f, 405.23f, 4.3f);
			Commands->Action_Movement_Goto_Location (obj, Obelisk_Move_to_Location, 0.5f);

			Vector3 Obelisk_Attack_Location(180.13f, 410.65f, 2.90f);
			Commands->Action_Attack_Location(obj, Obelisk_Attack_Location,0, 0);
		}
	}
};


/*
MPR_A04_SuicideRun_NOD_Minigunner

	The following is the script for the NOD minigunner tank by the obelisk.  Its pretty tightly scripted and fragile.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_NOD_Minigunner_JDG, "")
{
	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, Created okay\n");

		MPR_A04_suiciderun_nod_minigunner_id_JDG = Commands->Get_ID (obj);
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_NOD_MINIGUNNER_HAVOC_IN_ZONE_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)))
		{
			//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, Recieved Custom from EnterZone, Starting GDI Minigunner Timers\n");
	
			float timeDelay_1 = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_CHASE_DELAY_JDG);

			float timeDelay_2 = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), timeDelay_2, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_CHASE_DELAY_JDG);

			NOD_MINIGUNNER_ATTACKS_HAVOC ();
		}
	}

	void NOD_MINIGUNNER_ATTACKS_HAVOC (void)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, NOD_MINIGUNNER_ATTACKS_HAVOC start\n");

		Commands->Action_Movement_Set_Forward_Speed((Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), 0.6f);

		Commands->Action_Movement_Follow_Object((Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG)), (Commands->Get_The_Star()), 10.0f);
	}

	void Damaged (GameObject* obj, GameObject* Damager)
	{
		if ((Damager == (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG))) || (Damager == (Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG))))
			//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, Has been damaged\n");
		{
			GameObject* nodGunner = Commands->Find_Object(MPR_A04_suiciderun_nod_minigunner_id_JDG);

			if (nodGunner != NULL)
			{
				Commands->Action_Movement_Stop(nodGunner);

				//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, Has stopped\n");
			}
		}
	}

	void Killed(GameObject* obj, GameObject* Killer)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, NOD Minigunner, NOD_MINIGUNNER Killed, Start new GDI timers\n");

		float timeDelay_1 = Commands->Get_Random (1.5f,2.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_FOLLOW_TANK_DELAY_JDG);

		float timeDelay_2 = Commands->Get_Random (1.5f,2.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), timeDelay_2, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_FOLLOW_TANK_DELAY_JDG);

		float timeDelay_4 = Commands->Get_Random (3.5f,6.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_nod_light_tank_id_JDG)), timeDelay_4, MPR_A04_TIMER_SUICIDERUN_NOD_LIGHTTANK_ATTACK_MEDIUMTANK_DELAY_JDG);
	}
};


/*
MPR_A04_SuicideRun_NOD_Light_Tank

	The following is the script for the NOD light tank by the obelisk.  Its pretty tightly scripted and fragile.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_NOD_Light_Tank_JDG, "")
{
	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, PRDemo, SuicideRun, NOD Light Tank, Created okay\n");

		MPR_A04_suiciderun_nod_light_tank_id_JDG = Commands->Get_ID (obj);

		Commands->Set_Health(obj, 400.0f);

		state = MPR_A04_SUICIDERUN_LIGHTTANK_ALIVE;
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A04_TIMER_SUICIDERUN_NOD_LIGHTTANK_ATTACK_MEDIUMTANK_DELAY_JDG) && (Commands->Find_Object(MPR_A04_suiciderun_nod_light_tank_id_JDG)))
		{
			Commands->Action_Attack_Object(obj, (Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), 10.0f, 100.0f);

			float timeDelay_1 = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_ATTACK_LIGHTTANK_DELAY_JDG);

			float timeDelay_2 = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), timeDelay_2, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_ATTACK_LIGHTTANK_DELAY_JDG);

			float timeDelay_4 = Commands->Get_Random (1.5f,2.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), timeDelay_4, MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_ATTACK_LIGHTTANK_DELAY_JDG);
		}	
	}

	void Killed(GameObject* obj, GameObject* Killer)
	{
		float timeDelay_1 = Commands->Get_Random (1.5f,2.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_ATTACK_OBELISK_DELAY_JDG);

		float timeDelay_2 = Commands->Get_Random (1.5f,2.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), timeDelay_2, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_ATTACK_OBELISK_DELAY_JDG);

		float timeDelay_4 = Commands->Get_Random (1.5f,2.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), timeDelay_4, MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_ATTACK_OBELISK_DELAY_JDG);
	
		state = MPR_A04_SUICIDERUN_LIGHTTANK_DEAD;
	}
};


/*
MPR_A04_SuicideRun_Console

	The following is a script for the suicide run.  It goes on both the obelisk and power plant consoles and lets 
	the gdi guys know if/when the console gets destroyed.
*/

DECLARE_SCRIPT(MPR_A04_SuicideRun_Console_JDG, "")
{
	void Created(GameObject* obj)
	{
		status = MPR_A04_SUICIDERUN_CONSOLE_ALIVE;
	}

	void Killed(GameObject* obj, GameObject* Killer)
	{
		float timeDelay_1 = Commands->Get_Random (0.5f,1.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_1_id_JDG)), timeDelay_1, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_1_OBELISK_IS_DEAD_DELAY_JDG);

		float timeDelay_2 = Commands->Get_Random (0.5f,1.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_minigunner_2_id_JDG)), timeDelay_2, MPR_A04_TIMER_SUICIDERUN_GDI_MINIGUNNER_2_OBELISK_IS_DEAD_DELAY_JDG);

		float timeDelay_4 = Commands->Get_Random (0.5f,1.5f);
		Commands->Start_Timer ((Commands->Find_Object(MPR_A04_suiciderun_gdi_medium_tank_id_JDG)), timeDelay_4, MPR_A04_TIMER_SUICIDERUN_GDI_MEDIUMTANK_OBELISK_IS_DEAD_DELAY_JDG);
	
		status = MPR_A04_SUICIDERUN_CONSOLE_DEAD;
	}
};


/*
MPR_A02_DriveBy_EnterZone

	The following are the scripts involved in the harvester escort routine.
	There are 2 scripts, one for a zone and the other for the Harvester
*/

DECLARE_SCRIPT(MPR_A02_DriveBy_EnterZone_JDG, "")
{
	int Havoc_in_this_Zone;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPR_A02_HarvesterDriveBy_EnterZone, Created Okay\n");

		Havoc_in_this_Zone = 0;
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//Commands->DebugPrint("JDG, MPRA02, Havoc has Entered the DriveBy start zone, Sending customs to harvester patrol group\n");
			//Commands->Display_Text_Colored(Vector3(1.0f,0.0f,0.7f),"JDG, MPRA02, Havoc has Entered the DriveBy start zone, Sending customs to harvester patrol group\n" );
	
			Commands->Send_Custom_Event (obj, (Commands->Find_Object(MPR_A02_driveby_harvester_id_JDG)), MPR_A02_CUSTOM_DRIVEBY_HARVESTER_JDG, MPR_A00_PARAM_HAVOC_IN_ENTER_ZONE_JDG);
		
			Havoc_in_this_Zone++;
		}
	}
};


/*
MPR_A02_DriveBy_Harvester

  The following is the script for the harvester that comes from the refinery.  It basicly just follows
  a waypath to the tiberium field and then does nothing.
*/

DECLARE_SCRIPT(MPR_A02_DriveBy_Harvester_JDG, "")
{
	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA02, DriveBy, Harvester Created Okay\n");
	
		MPR_A02_driveby_harvester_id_JDG = Commands->Get_ID (obj);


		Commands->Set_Health(obj, 400.0f);
	}

	void Custom (GameObject* obj, int Type, int Param, GameObject* Sender)
	{
		if ((Type == MPR_A02_CUSTOM_DRIVEBY_HARVESTER_JDG) && (Param == MPR_A00_PARAM_HAVOC_IN_ENTER_ZONE_JDG))
		{
			//DebugPrint("JDG, MPRA02, Harvester, Recieved Custom from EnterZone, Starting Scenario\n");
	
			Commands->Action_Movement_Set_Forward_Speed(obj, 0.25f);

			Commands->Action_Movement_Follow_Waypath (  obj, MPR_A02_HARVESTER_WAYPATH_JDG, MPR_A02_HARVESTER_WAYPATH_START_JDG, MPR_A02_HARVESTER_WAYPATH_STOP_JDG, true );
		}
	}
};

/*
MPR_A03_Helicopter_EnterZone

The following are the scripts involved with the injured pilot at
the crashed Apache.  There are two scripts, the first is for a zone
that acts as a controller for the random explosions, and the second
is for the pilot himself.

*/

DECLARE_SCRIPT(MPR_A03_Helicopter_EnterZone_JDG, "")
{
	int Havoc_in_this_Zone;
	int Explosion_Count;
	int count;
	int MPR_A03_HELI_EXLOSION_ZONE_ID;

	typedef enum
	{
	MPR_A03_JDG_EXPLOSION_TIMER_1,
	MPR_A03_JDG_EXPLOSION_TIMER_2,
	MPR_A03_JDG_EXPLOSION_TIMER_3,
	MPR_A03_JDG_EXPLOSION_TIMER_4,
		
	} PRTIMER_TIMERS;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA03, Heli_Explosion EnterZone created okay\n");

		Havoc_in_this_Zone = 0;
		

		MPR_A03_HELI_EXLOSION_ZONE_ID = Commands->Get_ID (obj);
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//DebugPrint("JDG, MPRA03, Havoc has Entered the Heli_Explosion EnterZone, Starting Explosions\n");

			//Havoc_in_this_Zone++;

			Explosion_Count = (int)Commands->Get_Random (10, 15);

			count = 0;

			Commands->Start_Timer((Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)), .71f, MPR_A03_TIMER_INJURED_PILOT_CRAWL_1_JDG);

			Havoc_in_this_Zone++;

			Make_Explosions ();
		}
	}

	void Make_Explosions ()
	{
		//DebugPrint("JDG, MPRA03, Make Explosions start\n");
			
		int Explosion_Number = (int)Commands->Get_Random(1, 5);

		if ((Explosion_Number == 1) && (count <= Explosion_Count))
		{
			//DebugPrint("JDG, MPRA03, Make Explosions Starting Timer 1\n");

			count++;

			float timeDelay = Commands->Get_Random(3.0f, 5.0f);
			Commands->Start_Timer((Commands->Find_Object(MPR_A03_HELI_EXLOSION_ZONE_ID)), timeDelay, MPR_A03_JDG_EXPLOSION_TIMER_1);
		}

		else if ((Explosion_Number == 2) && (count <= Explosion_Count))
		{
			//DebugPrint("JDG, MPRA03, Make Explosions Starting Timer 2\n");

			count++;

			float timeDelay = Commands->Get_Random(3.0f, 5.0f);
			Commands->Start_Timer((Commands->Find_Object(MPR_A03_HELI_EXLOSION_ZONE_ID)), timeDelay, MPR_A03_JDG_EXPLOSION_TIMER_2);
		}

		else if ((Explosion_Number == 3) && (count <= Explosion_Count))
		{
			//DebugPrint("JDG, MPRA03, Make Explosions Starting Timer 3\n");

			count++;

			float timeDelay = Commands->Get_Random(3.0f, 5.0f);
			Commands->Start_Timer((Commands->Find_Object(MPR_A03_HELI_EXLOSION_ZONE_ID)), timeDelay, MPR_A03_JDG_EXPLOSION_TIMER_3);
		}

		else if (count <= Explosion_Count)
		{
			//DebugPrint("JDG, MPRA03, Make Explosions Starting Timer 4\n");

			count++;

			float timeDelay = Commands->Get_Random(3.0f, 5.0f);
			Commands->Start_Timer((Commands->Find_Object(MPR_A03_HELI_EXLOSION_ZONE_ID)), timeDelay, MPR_A03_JDG_EXPLOSION_TIMER_4);
		}
	}

	void Timer_Expired (GameObject*obj, int Timer_ID)
	{
		if (Timer_ID == MPR_A03_JDG_EXPLOSION_TIMER_1)
		{
			float Xnot = Commands->Get_Random (186.20f, 190.21f);
			float Ynot = Commands->Get_Random (31.0f, 35.0f);
			float Znot = Commands->Get_Random (2.59f, 4.69f);
			Vector3 Explosion_Location(Xnot, Ynot, Znot);

			//DebugPrint("JDG, MPRA03, Timer Expired Explosion 1\n");

			Commands->Create_Explosion ("Rocket Explosion", Explosion_Location, obj);

			Make_Explosions ();
		}

		if (Timer_ID == MPR_A03_JDG_EXPLOSION_TIMER_2)
		{
			float Xnot = Commands->Get_Random (178.21f, 180.21f);
			float Ynot = Commands->Get_Random (38.0f, 40.0f);
			float Znot = Commands->Get_Random (2.69f, 4.69f);
			Vector3 Explosion_Location(Xnot, Ynot, Znot);

			//DebugPrint("JDG, MPRA03, Timer Expired Explosion 2\n");

			Commands->Create_Explosion ("Rocket Explosion", Explosion_Location, obj);

			Make_Explosions ();
		}

		if (Timer_ID == MPR_A03_JDG_EXPLOSION_TIMER_3)
		{
			float Xnot = Commands->Get_Random (183.21f, 185.21f);
			float Ynot = Commands->Get_Random (46.0f, 49.0f);
			float Znot = Commands->Get_Random (6.15f, 7.69f);
			Vector3 Explosion_Location(Xnot, Ynot, Znot);

			//DebugPrint("JDG, MPRA03, Timer Expired Explosion 3\n");

			Commands->Create_Explosion ("Rocket Explosion", Explosion_Location, obj);

			Make_Explosions ();
		}

		if (Timer_ID == MPR_A03_JDG_EXPLOSION_TIMER_4)
		{
			float Xnot = Commands->Get_Random (188.21f, 190.21f);
			float Ynot = Commands->Get_Random (47.0f, 49.0f);
			float Znot = Commands->Get_Random (8.06f, 10.69f);
			Vector3 Explosion_Location(Xnot, Ynot, Znot);

			//DebugPrint("JDG, MPRA03, Timer Expired Explosion 4\n");

			Commands->Create_Explosion ("Rocket Explosion", Explosion_Location, obj);

			Make_Explosions ();
		}
	}
};


/*
MPR_A03_Helicopter_Injured_NOD_Pilot

	The following is the script for the injured pilot by the downed Apache.
*/

DECLARE_SCRIPT(MPR_A03_Helicopter_Injured_NOD_Pilot_JDG, "")
{
	typedef enum
	{
		MPR_A03_PILOT_IDLE = 0,
		MPR_A03_PILOT_STANDING_UP,
		MPR_A03_PILOT_STAMMERING,
		MPR_A03_PILOT_LOOKING_BACK,
		MPR_A03_PILOT_ADJUSTING_KEVLAR,
		MPR_A03_PILOT_ATTACKING,
		
	} PILOTState;

	PILOTState state;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA03, Injured Pilot Created Okay\n");

		MPR_A03_helicopter_injured_nod_pilot_id_JDG = Commands->Get_ID (obj);

		Commands->Set_Animation (obj, "S_A_HUMAN.H_A_601A", 1);

		state = MPR_A03_PILOT_IDLE;
	}

	void Timer_Expired (GameObject*obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A03_TIMER_INJURED_PILOT_CRAWL_1_JDG) && (Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)))
		{
			//DebugPrint("JDG, MPRA03, Injured Pilot Timer Expired, Start Crawling\n");

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_H11C", 0);
			//"S_A_HUMAN.H_A_H11C" is animation for standing up

			state = MPR_A03_PILOT_STANDING_UP;
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare("S_A_HUMAN.H_A_H11C", name) == 0) && (Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)) && (state == MPR_A03_PILOT_STANDING_UP)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_611A", 0);

			state = MPR_A03_PILOT_STAMMERING;
		}

		if ((openw3d::string_compare("S_A_HUMAN.H_A_611A", name) == 0) && (Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)) && (state == MPR_A03_PILOT_STAMMERING)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J11C", 0);

			state = MPR_A03_PILOT_LOOKING_BACK;
		}

		if ((openw3d::string_compare("S_A_HUMAN.H_A_J11C", name) == 0) && (Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)) && (state == MPR_A03_PILOT_LOOKING_BACK)) 
		{
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J22C", 0);

			state = MPR_A03_PILOT_ADJUSTING_KEVLAR;
		}

		if ((openw3d::string_compare("S_A_HUMAN.H_A_J22C", name) == 0) && (Commands->Find_Object(MPR_A03_helicopter_injured_nod_pilot_id_JDG)) && (state == MPR_A03_PILOT_ADJUSTING_KEVLAR)) 
		{
			Commands->Action_Movement_Set_Forward_Speed (obj, 0.5f);

			Commands->Action_Movement_Goto_Object (obj, (Commands->Get_The_Star()), 6.0f);

			Commands->Action_Attack_Object (obj, (Commands->Get_The_Star()), 1.0f, 20.0f);
		}
	}
};


/*
MPR_A03_BridgeScene_EnterZone

  	The following are the scripts involved in the bridge blow up scenario.
	There are presently 3 scripts, 2 zones and 1 engineer.
*/

DECLARE_SCRIPT(MPR_A03_BridgeScene_EnterZone_JDG, "")
{
	int Havoc_in_this_Zone;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA03, BridgeScene, EnterZone, Created Okay\n");
	
		Havoc_in_this_Zone = 0;

		MPR_A03_bridgescene_enterzone_id_JDG = Commands->Get_ID (obj);

		Commands->Static_Anim_Phys_Goto_Frame ( MPR_A03_BRIDGESCENE_BRIDGE_ID_JDG, 0.0f, "BRIDGE_EXPLDA02.BRIDGE_EXPLDA02");
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, EnterZone, Havoc has entered Zone. Starting Delay timers on Bridge Team\n");
		
			float timeDelay_3 = Commands->Get_Random (0.0f, 0.5f);
			Commands->Start_Timer ((Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)), timeDelay_3, MPR_A03_TIMER_BRIDGESCENE_ENGINEER_HAVOC_IN_ZONE_DELAY_JDG);
			
			float timeDelay_4 = 8.0f;
			Commands->Start_Timer (obj, timeDelay_4, MPR_A03_TIMER_BRIDGESCENE_ENTER_ZONE_EXPLOSION_DELAY_JDG);
		
			Havoc_in_this_Zone++;
		}
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if (Timer_ID == MPR_A03_TIMER_BRIDGESCENE_ENTER_ZONE_EXPLOSION_DELAY_JDG) 
		{
			Commands->Static_Anim_Phys_Goto_Last_Frame (MPR_A03_BRIDGESCENE_BRIDGE_ID_JDG, "BRIDGE_EXPLDA02.BRIDGE_EXPLDA02" );
		}	
	}
};


/*
MPR_A03_BridgeScene_BlowUpZone

	The following script is for a zone right next to the bridge that immediately blows the
	bridge when entered.  This is to prevent the player from reaching the other side before 
	the bridge blows.
*/

DECLARE_SCRIPT(MPR_A03_BridgeScene_BlowUpZone_JDG, "")
{
	int Havoc_in_this_Zone;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA03, BridgeScene, EnterZone, Created Okay\n");
	
		Havoc_in_this_Zone = 0;

		MPR_A03_bridgescene_blowupzone_id_JDG = Commands->Get_ID (obj);
	}

	void Entered (GameObject* obj, GameObject* Enterer)
	{
		if (Enterer == (Commands->Get_The_Star()) && (Havoc_in_this_Zone == 0))
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, EnterZone, Havoc has entered Zone. Starting Delay timers on Bridge Team\n");
		
			Commands->Static_Anim_Phys_Goto_Last_Frame (MPR_A03_BRIDGESCENE_BRIDGE_ID_JDG, "BRIDGE_EXPLDA02.BRIDGE_EXPLDA02" );
		}
	}
};


/*
MPR_A03_BridgeScene_Engineer

	The following is for the engineer next to the bridge.  He does a few animations then runs through the 
	tunnel and is deleted.
*/

DECLARE_SCRIPT(MPR_A03_BridgeScene_Engineer_JDG, "")
{
	typedef enum
	{
		MPR_A03_BRIDGESCENE_ENGINEER_IDLE = 0,
		MPR_A03_BRIDGESCENE_ENGINEER_PLACING_BOMB,
		MPR_A03_BRIDGESCENE_ENGINEER_WAVING,
		MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_INT_SPOT,
		MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_TUNNEL,
		MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_FINAL_POSITION,
		
	} ENGINEERState;

	ENGINEERState state;

	void Created(GameObject* obj)
	{
		//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Created Okay\n");
	
		MPR_A03_bridgescene_engineer_id_JDG = Commands->Get_ID (obj);

		Commands->Action_Attack_Object( obj, (Commands->Get_The_Star()), 0, 0);

		state = MPR_A03_BRIDGESCENE_ENGINEER_IDLE;
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		if ((Timer_ID == MPR_A03_TIMER_BRIDGESCENE_ENGINEER_HAVOC_IN_ZONE_DELAY_JDG) && (Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)))
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Havoc/EnterZone Timer Expired\n");
	
			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J12C", 0);
			//"S_A_HUMAN.H_A_J12C" is animation for tying your shoe

			state = MPR_A03_BRIDGESCENE_ENGINEER_PLACING_BOMB;
		}
	}

	void Animation_Complete (GameObject* obj, const char* name)
	{
		if ((openw3d::string_compare("S_A_HUMAN.H_A_J12C", name) == 0) && (Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)) && (state == MPR_A03_BRIDGESCENE_ENGINEER_PLACING_BOMB)) 
		{

			Commands->Set_Animation (obj, "S_A_HUMAN.H_A_J18C", 0);
			//"S_A_HUMAN.H_A_J18C" is animation for waving

			state = MPR_A03_BRIDGESCENE_ENGINEER_WAVING;
		}
		
		if ((openw3d::string_compare("S_A_HUMAN.H_A_J18C", name) == 0) && (Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)) && (state == MPR_A03_BRIDGESCENE_ENGINEER_WAVING)) 
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Finished waving\n");
	
			Vector3 Engineers_Int_Position (234.26f, 202.66f, 3.92f);
			Commands->Action_Attack_Location(obj, Engineers_Int_Position, 0, 0);

			Commands->Action_Movement_Set_Forward_Speed(obj, 0.7f);

			Commands->Action_Movement_Goto_Location(obj, Engineers_Int_Position, 0.05f);

			//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, start moving away\n");
	
			state = MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_INT_SPOT;
		}
	}

	void Movement_Complete(GameObject* obj, MovementCompleteReason reason) 
	{
		static const char* _reasons[] =
		{
			"Arrived",
			"Bad Start",
			"Bad Dest",
			"No progress"
		};

		//DebugPrint("Movement_Complete Reason = %s\n", _reasons[reason]);

		if ((Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)) && (state == MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_INT_SPOT))
		{
		//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Reached intermediate spot\n");
	
			Vector3 Engineers_Tunnel_Position (154.90f, 242.68f, 0.39f);
			Commands->Action_Attack_Location(obj, Engineers_Tunnel_Position, 0, 0);

			Commands->Action_Movement_Goto_Location(obj, Engineers_Tunnel_Position, 0.05f);

			state = MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_TUNNEL;
			
		}

		if ((Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)) && (state == MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_TUNNEL))
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Reached bridge location\n");
	
			Vector3 Engineers_Final_Position (141.95f, 297.41f, 0.86f);
			Commands->Action_Attack_Location(obj, Engineers_Final_Position, 0, 0);

			Commands->Action_Movement_Goto_Location(obj, Engineers_Final_Position, 0.05f);

			state = MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_FINAL_POSITION;
			
		}

		if ((Commands->Find_Object(MPR_A03_bridgescene_engineer_id_JDG)) && (state == MPR_A03_BRIDGESCENE_ENGINEER_MOVING_TO_FINAL_POSITION))
		{
			//DebugPrint("JDG, MPRA03, BridgeScene, Engineer, Destroying\n");

			Commands->Destroy_Object( obj );
		}
	}
};

#endif