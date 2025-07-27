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
* VERSION INFO
*     $Author: Darren_k $
*     $Revision: 25 $
*     $Modtime: 1/12/02 3:19p $
*     $Archive: /Commando/Code/Scripts/Test_DAK.cpp $
*
******************************************************************************/

#include "toolkit.h"

DECLARE_SCRIPT (DAK_TestScriptOne, "")
{
	void Damaged( GameObject *obj , GameObject *damager, float amount)
	{
		if ( Commands->Is_A_Star(damager) )
		{	
			ActionParamsStruct params;
			params.Set_Basic( this, 1, 1 );
			params.Set_Movement( damager, RUN, 2 );
			Commands->Action_Goto( obj, params );
		}
	}
};

DECLARE_SCRIPT (DAK_PlayerSpotted, "")
{
	void Damaged ( GameObject *obj , GameObject *damager, float amount)
	{	
		GameObject * NodSAM = Commands->Find_Object (100012);
		if (NodSAM)
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 1, 1 );
			params.Set_Movement( NodSAM, RUN, 1 );
			Commands->Action_Goto( obj, params );
		}
		else
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 1, 1 );
			params.Set_Attack( damager, 20, 0, 1 );
			Commands->Action_Attack( obj, params );
		}
	}
};

DECLARE_SCRIPT ( DAK_Fire_Gas_Elec_Death_DAK, "DeathType:string" )
{
	bool firsttime; // prevents an infinante loop each time obj is damaged by DeathType.

	void Damaged ( GameObject *obj, GameObject *damager, float amount )
	{
		// check to see if obj is at 25% or less of its health.
		if ( Commands->Get_Health ( obj ) <= 0.25 * Commands->Get_Max_Health ( obj ) )
		{
			// plays animation once.
			if ( firsttime == true )
			{
				firsttime = false;

				ActionParamsStruct params;
				params.Set_Basic( this, 99, 1 );

				// set animation based upon DeathType (Gas, Electric or Default to Fire)
				if ( strcmp( Get_Parameter( "DeathType" ), "Fire" ) )
				{
					params.Set_Animation( "S_A_HUMAN.H_A_FLMA",0 );	// fire death
				}
				else
					if ( strcmp( Get_Parameter( "DeathType" ), "Electic" ) )
					{
						params.Set_Animation( "S_A_HUMAN.H_A_6X05",0 ); // electric death
					}
					else params.Set_Animation( "S_A_HUMAN.H_A_FLMA",0 ); // gas death
					
				Commands->Action_Play_Animation( obj, params );

				// begin DeathType damage
				Commands->Apply_Damage( obj, 1.0f, Get_Parameter( "DeathType" ), NULL );
			}
		}
		else
		{
			firsttime = true;
		}
	}

	void Action_Complete(GameObject * obj, int action_id, ActionCompleteReason reason)
	{
		if((action_id == 1) && (reason == ACTION_COMPLETE_NORMAL))
		{
			// animation is complete. kill obj.
			Commands->Apply_Damage (obj, 10000.0f, "Blamokiller", NULL);
		}

	}
};

DECLARE_SCRIPT(DAK_Vehicle_Regen_DAK, "" )
{
	void Created ( GameObject *obj )
	{
		Commands->Send_Custom_Event ( obj, obj, 0, 0, 0 );
	}

	void Custom (GameObject* obj, int type, int param, GameObject* sender) 
	{
		if ( type == 0 ) // regenerate health.
		{
			// check to see if health needs to be regenerated.
			if ( Commands->Get_Health ( obj ) < Commands->Get_Max_Health ( obj ) )
			{
				Commands->Apply_Damage (obj, -10, "RegenHealth", NULL);
			}
			// restart the timer
			Commands->Send_Custom_Event ( obj, obj, 0, 0, 5 );
		}
	}
};

DECLARE_SCRIPT(DAK_Electric_Death_DAK, "" )
{
	void Created ( GameObject *obj )
	{
		int time = Commands->Get_Random(1, 3);
		Commands->Send_Custom_Event ( obj, obj, 0, 0, time );
	}

	void Damaged (GameObject *obj, GameObject *damager, float amount )
	{
		Commands->Send_Custom_Event ( obj, obj, 1, 0, 1 ); // wait a second before applying next ammount of damage.
	}

	void Custom ( GameObject *obj, int type, int param, GameObject *sender )
	{
		if ( type == 0 ) // create next soldier, attach script, kill yourself with electric damage.
		{
			Vector3 position = Commands->Get_Position (obj);
			position.X += Commands->Get_Random(-3, 3);
			position.Y += Commands->Get_Random(-3, 3);
			GameObject *new_object = Commands->Create_Object( "Nod_Minigunner_0_Def", position);
			
			Commands->Attach_Script (new_object, "DAK_Electric_Death_DAK", "");

			Commands->Apply_Damage (obj, 10, "Electric", NULL);
		}

		if ( type == 1 ) // apply next ammount of electric damage.
		{
			Commands->Apply_Damage (obj, 10, "Electric", NULL);
		}
	}
};

DECLARE_SCRIPT(DAK_PCT_Pokable_DAK, "" )
{
	void Created ( GameObject *obj )
	{
		Commands->Enable_HUD_Pokable_Indicator( obj, true );
//		Commands->Display_Health_Bar( obj, false );
	}
};


DECLARE_SCRIPT( M00_BUILDING_EXPLODE_NO_DAMAGE_DAK, "" )
{
	void Killed( GameObject *obj, GameObject *killer )
	{
		//Commands->Create_Explosion ( "Building_Explode_No_Damage", position, NULL );
		Commands->Shake_Camera( Commands->Get_Position( obj ), 25, 0.1f, 4.0f );
	}
};