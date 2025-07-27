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
*     Byon Garrabrant
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 100 $
*     $Modtime: 12/17/01 11:12a $
*     $Archive: /Commando/Code/Scripts/Test_BMG.cpp $
*
******************************************************************************/

#include "scripts.h"

DECLARE_SCRIPT(BMG_Test_Script, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "BMG Test Script\n" );
//		Commands->Start_Timer( obj, this, 5, 0 );

/*		ActionParamsStruct params;
		params.Set_Basic( this, 80, 0 );
		params.Set_Attack( Vector3( 0,0,10 ), 100, 15, 1 );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );*/
	}

	void	Sound_Heard( GameObject * obj, const CombatSound & sound )
	{
		Commands->Debug_Message("Heard Sound %d\n", sound.Type );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Start_Timer( obj, this, 5, 0 );

		GameObject * star = Commands->Get_The_Star();
		if ( star ) {
			Commands->Clear_Weapons( star );
			Commands->Give_PowerUp(star, "POW_LaserRifle_Player", false);
			Commands->Select_Weapon(star, "Weapon_LaserRifle_Player" );
		}
	}

};


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////                                   /////////////////////////////////////////
////////////////////  ALL THE REST ARE COMMENTED OUT   /////////////////////////////////////////
////////////////////                                   /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

DECLARE_SCRIPT(BMG_Toggle_Control, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "BMG Test Script\n" );
		Commands->Start_Timer( obj, this, 3, 0 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message( "BMG Test Script\n" );
		Commands->Start_Timer( obj, this, 5, timer_id+1 );
		GameObject * star = Commands->Get_The_Star();
		Commands->Control_Enable( star, ( timer_id & 1 ) );
	}

};



DECLARE_SCRIPT(BMG_Test_Enter_Vehicle, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "BMG Test Script\n" );
		Commands->Start_Timer( obj, this, 5, 0 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message( "Start Goto\n" );

		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, 0 );
		params.Set_Movement( Vector3( 0.1f, -6.1f, 0 ), 1, 0.2f );
		Commands->Action_Goto( obj, params );
	}

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		if ( action_id == 0 ) {
			Commands->Debug_Message( "Enter\n" );

			int priority = 80;
			ActionParamsStruct params;
			params.Set_Basic( this, priority, 1 );
			params.Set_Movement( Vector3( 2.5f, -6.1f, 0 ), 1, 0.2f );
			Commands->Action_Enter_Exit( obj, params );
		}
	}



};



DECLARE_SCRIPT(BMG_Test_Pogs, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this, 5, 0 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message( "Hello\n" );
		Commands->Start_Timer( obj, this, 5, timer_id+1 );

		switch( timer_id ) {
			case 0:
					Commands->Add_Objective( 100, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, IDS_BYON, NULL );
					Commands->Set_Objective_Radar_Blip( 100, Vector3( 10, 10, 0 ) );
					Commands->Set_Objective_HUD_Info( 100, 100, "HUD_OBJE_ARROW.TGA", IDS_BYON );
					break;
			case 1:
					Commands->Add_Objective( 101, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_BYON, NULL );
					Commands->Set_Objective_Radar_Blip( 101, Vector3( 10, -10, 0 ) );
					Commands->Set_Objective_HUD_Info( 101, 101, "HUD_OBJECTIVE.TGA", IDS_BYON );
					break;
			case 2:
					Commands->Set_Objective_Status( 100, OBJECTIVE_STATUS_ACCOMPLISHED );
					break;
		}


//		GameObject *controller = Commands->Create_Object("Invisible_Object", Vector3(0,0,0));
//		Commands->Attach_Script(controller, "Test_Cinematic", "test.txt");
	}


};


DECLARE_SCRIPT(BMG_Test_Misc_Script, "" )
{
	void Created(GameObject* obj)
	{
//		GameObject * star = Commands->Get_The_Star();

/*		ActionParamsStruct params;
		params.Set_Basic( this, 80, 0 );
		params.Set_Attack( Vector3( 9.6f,-3.0f,1.9f), 100, 15, 1 );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
		Commands->Debug_Message( "BMG Test Script %p\n", obj );*/
/*
		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, 0 );
		params.Set_Movement( Vector3( 13,-78,0 ), 1, 1 );
		Commands->Action_Goto( obj, params );*/
//		Commands->Start_Timer( obj, this, 20, 0 );
//		Action_Complete( obj, 1, (ActionCompleteReason)0 );

/*		Commands->Debug_Message("Objective Created\n");
		Commands->Add_Objective( 300, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, 1000 );
		Commands->Set_Objective_Radar_Blip_Object( 300, obj );*/
	}

	int state;

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		Commands->Debug_Message( "BMG Complete %p %d\n", obj, complete_reason );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Mission_Complete( true );
//		Commands->Static_Anim_Phys_Goto_Last_Frame (150047, NULL );
//		Commands->Static_Anim_Phys_Goto_Last_Frame (150047, "BASEGATE.BASEGATE" );
//		Commands->Static_Anim_Phys_Goto_Frame ( 150047, 31, "BASEGATE.BASEGATE" );
	}

	void	Sound_Heard( GameObject * obj, const CombatSound & sound )
	{
		Commands->Debug_Message("Heard Sound %d\n", sound.Type );
	}

	void Damaged( GameObject * obj, GameObject * damager, float amount ) 
	{
		GameObject * o = Commands->Create_Object ( "Invisible_Object", Vector3(-90.0f, 50.0f, 0.0f));
		Commands->Attach_Script(o, "Test_Cinematic", "X1I_troopdrop02.txt");
	}

};

DECLARE_SCRIPT(Start_Script, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "Start_Script\n" );
	}
};

DECLARE_SCRIPT(Respawn_Script, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "Respawn_Script\n" );
	}
};

DECLARE_SCRIPT(BMG_Spray_Shooter, "" )
{
	void Created(GameObject* obj)
	{
//		GameObject * star = Commands->Get_The_Star();

		ActionParamsStruct params;
		params.Set_Basic( this, 80, 0 );
		params.Set_Attack( Vector3( 9.6f,-3.0f,1.9f), 100, 15, 1 );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
		Commands->Debug_Message( "BMG Test Script %p\n", obj );
/*
		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, 0 );
		params.Set_Movement( Vector3( 13,-78,0 ), 1, 1 );
		Commands->Action_Goto( obj, params );*/
//		Commands->Start_Timer( obj, this, 20, 0 );
//		Action_Complete( obj, 1, (ActionCompleteReason)0 );

/*		Commands->Debug_Message("Objective Created\n");
		Commands->Add_Objective( 300, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, 1000 );
		Commands->Set_Objective_Radar_Blip_Object( 300, obj );*/
	}

	int state;

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		Commands->Debug_Message( "BMG Complete %p %d\n", obj, complete_reason );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Mission_Complete( true );
//		Commands->Static_Anim_Phys_Goto_Last_Frame (150047, NULL );
//		Commands->Static_Anim_Phys_Goto_Last_Frame (150047, "BASEGATE.BASEGATE" );
//		Commands->Static_Anim_Phys_Goto_Frame ( 150047, 31, "BASEGATE.BASEGATE" );
	}
};

DECLARE_SCRIPT(BMG_Monitor_Events, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "BMG Monitor Events Created\n" );
		Commands->Enable_Enemy_Seen( obj, true );
	}

	virtual	void	Custom( GameObject * obj, int type, int param, GameObject * sender ) 
	{
		switch( type ) {

			case CUSTOM_EVENT_VEHICLE_ENTERED:
				Commands->Debug_Message("BMG CUSTOM VEHICLE ENTERED %d %d\n", param, Commands->Get_ID( sender ) );
				break;

			case CUSTOM_EVENT_VEHICLE_EXITED:
				Commands->Debug_Message("BMG CUSTOM VEHICLE EXITED %d %d\n", param, Commands->Get_ID( sender ) );
				break;


			default:
				Commands->Debug_Message("BMG CUSTOM %d %d\n", type, param );
				break;
		}
	}

	void	Enemy_Seen( GameObject * obj, GameObject * enemy )
	{
		Commands->Debug_Message("BMG ENEMY_SEEN %d\n", Commands->Get_ID( enemy ) );
	}


};

DECLARE_SCRIPT(BMG_Test_Powerup, "" )
{
	virtual	void	Custom( GameObject * obj, int type, int param, GameObject * sender ) 
	{
		if ( type == CUSTOM_EVENT_POWERUP_GRANTED ) {
			Commands->Debug_Message( "POWERUP for %d\n", Commands->Get_ID( sender ) );
		}
	}
};



DECLARE_SCRIPT(BMG_Test_Walking, "" )
{
	int state;

	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Walking\n");
		Action_Complete( obj, 1, (ActionCompleteReason)0 );
	}

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		state = !state;
		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, action_id+1 );

		if ( state ) {
			params.Set_Movement( Vector3( 7,-3,0 ), 1, 1 );
		} else {
			params.Set_Movement( Vector3( 10,0,0 ), 1, 1 );
		}
		params.Set_Attack( Vector3( 20,20,0 ), 100, 1, true );
		Commands->Action_Attack( obj, params );
	}
};




DECLARE_SCRIPT(BMG_Vehicle_Awareness, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Enable_Enemy_Seen( obj, true );
	}

	void	Sound_Heard( GameObject * obj, const CombatSound & sound )
	{
		Commands->Debug_Message("Heard\n");
	}

	void	Enemy_Seen( GameObject * obj, GameObject * enemy )
	{
		Commands->Debug_Message("Seen\n");
	}

};


DECLARE_SCRIPT(BMG_Test_Score, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this, 1, 0 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		GameObject * star = Commands->Get_A_Star( Vector3(0,0,0) );
		if (star) {
			Commands->Give_Points( star, 100, 0 );
			Commands->Debug_Message("Add Score\n");
		}
	}


};

DECLARE_SCRIPT(BMG_Test_Flash, "" )
{
	void Created(GameObject* obj)
	{
		Timer_Expired( obj, 0 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Start_Timer( obj, this, 1, timer_id + 1 );
		Commands->Set_Is_Rendered( obj, (timer_id & 1) == 0 );
	}

};



DECLARE_SCRIPT(BMG_Test_Objective, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("Objective Created\n");
		Commands->Add_Objective( 300, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, 1000 );
		Commands->Set_Objective_Radar_Blip_Object( 300, obj );
		Commands->Start_Timer( obj, this, 6, 0);
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message("Objective Removed\n");
		Commands->Remove_Objective( 300 );
	}

};


DECLARE_SCRIPT(BMG_Test_Messages, "speed=1.0:float")
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG CREATED\n");
	}

	void	Killed( GameObject * obj, GameObject * killer )
	{
		Commands->Debug_Message("BMG KILLED\n");
	}

	virtual	void	Damaged( GameObject * obj, GameObject * damager, float amount )
	{
		Commands->Debug_Message("BMG DAMAGED\n");
	}

	virtual	void	Custom( GameObject * obj, int type, int param, GameObject * sender ) 
	{
		Commands->Debug_Message("BMG CUSTOM %d %d\n", type, param );
	}

};


DECLARE_SCRIPT(BMG_Test_Anim_Speed, "speed=1.0:float")
{
	void Created(GameObject* obj)
	{
		Timer_Expired( obj, -1 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Start_Timer( obj, this, 3.5, (timer_id+1)&3 );

		Vector3	position = Commands->Get_Position( obj );
		float speed = Get_Float_Parameter( "speed" );
		switch( timer_id ) {
			case 0: position += Vector3(  speed * 100, 0, 0 );	break;
			case 1: position += Vector3( -speed * 100, 0, 0 );	break;
			case 2: position += Vector3( 0,  speed * 100, 0 );	break;
			case 3: position += Vector3( 0, -speed * 100, 0 );	break;
		}
		ActionParamsStruct params;
		params.Set_Basic( this, 80, 0 );
		params.Set_Movement( position, speed, 0.5f );
		GameObject * star = Commands->Get_A_Star( position );
		params.Set_Attack( star, 0.1f, 0, 1 );
		Commands->Action_Attack( obj, params );
	}

};

DECLARE_SCRIPT( BMG_Test_Radar, "" )
{
	void Created( GameObject * obj )
	{
		Commands->Debug_Message("BMG Test Radar\n");

//		Commands->Start_Timer( obj, this, 5, 1 );
//		Commands->Start_Timer( obj, this, 10, 2 );

//		Commands->Start_Timer( obj, this, 5, 3 );
//		Commands->Start_Timer( obj, this, 10, 4 );
//		Commands->Start_Timer( obj, this, 15, 5 );
//		Commands->Start_Timer( obj, this, 20, 6 );

		Commands->Start_Timer( obj, this, 5, 7 );
		Commands->Start_Timer( obj, this, 10, 8 );

	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		switch( timer_id ) {
			case 1:
				Commands->Debug_Message("BMG Test Radar : Change Color\n");
				Commands->Set_Obj_Radar_Blip_Color( obj, RADAR_BLIP_COLOR_GDI );
				break;
			case 2:
				Commands->Debug_Message("BMG Test Radar : Change Shape\n");
				Commands->Set_Obj_Radar_Blip_Shape( obj, RADAR_BLIP_SHAPE_VEHICLE );
				break;
			case 3:
				Commands->Debug_Message("BMG Test Radar : Create Objective\n");
				Commands->Add_Objective( 6, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, IDS_M04_RescuePrisoners );
				break;
			case 4:
				Commands->Debug_Message("BMG Test Radar : Make Objective Blip\n");
				Commands->Set_Objective_Radar_Blip( 6, Vector3( -5, -15, 0 ) );
				break;
			case 5:
				Commands->Debug_Message("BMG Test Radar : Change Objective Priority\n");
				Commands->Change_Objective_Type( 6, OBJECTIVE_TYPE_SECONDARY );
				break;
			case 6:
				Commands->Debug_Message("BMG Test Radar : Change Objective Status\n");
				Commands->Set_Objective_Status( 6, OBJECTIVE_STATUS_ACCOMPLISHED );
				break;
			case 7:
				Commands->Debug_Message("BMG Test Radar : Create Marker\n");
				Commands->Add_Radar_Marker( 9, Vector3( -5, -15, 0 ), RADAR_BLIP_SHAPE_VEHICLE, RADAR_BLIP_COLOR_GDI );
				break;
			case 8:
				Commands->Debug_Message("BMG Test Radar : Delete Marker\n");
				Commands->Clear_Radar_Marker( 9 );
				break;

		}
	}
};


DECLARE_SCRIPT( BMG_Test_Animation, "" )
{
	void Created( GameObject * obj )
	{
		Commands->Debug_Message("BMG Test Animation\n");
		Timer_Expired( obj, 1 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Start_Timer( obj, this, 1, 1 );
		ActionParamsStruct params;
		params.Set_Basic( this, 190, 0 );
		params.Set_Animation( "S_A_HUMAN.H_A_611A", false );
//		Commands->Action_Play_Animation( obj, params );
		Commands->Set_Animation( obj, "S_A_HUMAN.H_A_611A", true );
	}
};


DECLARE_SCRIPT( BMG_Test_Attack, "" )
{
	void Created(GameObject* obj)
	{
		ActionParamsStruct params;
		params.Set_Basic( this, 1000, 1 );
		params.Set_Movement( Vector3( 5,5,5 ), 1, 1000 );
		params.Set_Attack( Vector3( 5,5,5 ), 1000, 0, 1 );
		params.AttackCheckBlocked = false;
		Commands->Action_Attack( obj, params );
	}
};


DECLARE_SCRIPT( BMG_Test_Damage, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this,  3, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Start_Timer( obj, this,  3, 0 );
		Commands->Debug_Message( "BMG_Test_Damage\n" );
		Commands->Apply_Damage( obj, 2, "STEEL" );
	}
};

DECLARE_SCRIPT( BMG_Test_Save_Load, "" )
{
	int	Value1;
	float Value2;

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( Value1, 1 );
		SAVE_VARIABLE( Value2, 2 );
	}

	void Created( GameObject* obj )
	{
		Timer_Expired( obj, 1 );
		Timer_Expired( obj, 2 );
		Value1 = 0;
		Value2 = 0.0f;
	}

	void Timer_Expired( GameObject* obj, int Timer_ID )
	{
		if ( Timer_ID == 1 ) {
			Commands->Debug_Message("BMG_Test_Save_Load: Value1: %d\n", Value1++ );
			Commands->Start_Timer( obj, this,  1, Timer_ID );
		} else {
			Value2 += 0.3333f;
			Commands->Debug_Message("BMG_Test_Save_Load: Value2: %f\n", Value2 );
			Commands->Start_Timer( obj, this,  1.5f, Timer_ID );
		}
	}

//*****************************************************************************
#if 0
	typedef enum
	{
		DATA_ID_VALUE1,
		DATA_ID_VALUE2,
	};

	void	Save_Data( ScriptSaver & saver )
	{
		SAVE_BEGIN();
			SAVE_DATA( DATA_ID_VALUE1, Value1 );
			SAVE_DATA( DATA_ID_VALUE2, Value2 );
		SAVE_END();
	}

	void	Load_Data( ScriptLoader & loader )
	{
		LOAD_BEGIN();					
			LOAD_DATA( DATA_ID_VALUE1, Value1 );
			LOAD_DATA( DATA_ID_VALUE2, Value2 );
		LOAD_END();
	}
#endif

};

DECLARE_SCRIPT( BMG_Test_Attach, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this,  3, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Debug_Message("BMG_Test_Attach to Commando's HEAD!!!\n" );
		GameObject * star = Commands->Get_A_Star( Commands->Get_Position( obj ) );
		if ( star != NULL ) {
			Commands->Attach_To_Object_Bone( obj, star, "C HEAD" );
		}
	}
};

DECLARE_SCRIPT( BMG_Test_Spawn, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this,  5, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Start_Timer( obj, this,  5, 0 );
		GameObject * spawn = Commands->Trigger_Spawner( 100026, true );
		Commands->Debug_Message("BMG_Test_Spawn %p\n", spawn);
	}
};

DECLARE_SCRIPT( BMG_Test_Custom, "" )
{
	void	Custom( GameObject * obj, int type, int param, GameObject * sender )
	{
		Commands->Debug_Message("BMG Test Custom %d %d\n", type, param );
	}
};



DECLARE_SCRIPT(BMG_Test_Running, "" )
{
	int state;

	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Running\n");
		Action_Complete( obj, 1, (ActionCompleteReason)0 );
	}
								 
	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		state = !state;
		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, action_id+1 );
		if ( state ) {
			params.Set_Movement( Vector3( 13,-78,0 ), 1, 1 );
		} else {
			params.Set_Movement( Vector3( 69,-21,0 ), 1, 1 );
		}
		Commands->Action_Goto( obj, params );
	}
};

DECLARE_SCRIPT(BMG_Set_Home, "")
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this,  5, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Debug_Message("BMG_Set_Home\n");
		Commands->Set_Innate_Soldier_Home_Location( obj, Vector3(0,0,0), 0.5 );
	}
};


DECLARE_SCRIPT(BMG_Look_Test, "")
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, this,  1, -10 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Debug_Message("BMG_Look_Test %d\n", Timer_ID);

		ActionParamsStruct params;
		params.Set_Basic(this, 100, 100);

		if ( Timer_ID >= 12 ) {
			Timer_ID = -10;
		} else {
			Timer_ID += 20;
		}
		params.Set_Look( Vector3( 0,2,Timer_ID ), 5 );
		params.Set_Look( Commands->Get_The_Star(), 5 );
		Commands->Start_Timer( obj, this,  10, Timer_ID );

		params.Set_Movement( Vector3( Timer_ID,Timer_ID,0 ), 0, 1 );
//		params.Set_Movement(p_leader, 1.0f, 1.0f);
		Commands->Action_Goto(obj, params);
	}
};


DECLARE_SCRIPT(BMG_Timer_Test, "")
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Timer Test Start\n");
		Commands->Start_Timer( obj, this,  5, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Debug_Message("Difficulty Level is %d\n", Commands->Get_Difficulty_Level() );
		Commands->Start_Timer( obj, this,  5, 0 );
	}

};


DECLARE_SCRIPT(BMG_Shooter, "")
{
	void Created(GameObject* obj)
	{
		GameObject * target = Commands->Find_Object( 100004 );

		if ( target ) {
			Commands->Debug_Message("Shooting\n" );
			ActionParamsStruct params;
			params.Set_Basic( this, 1000, 1 );
			params.Set_Movement( target, 1, 1000 );
			params.Set_Attack( target, 1000, 0, 1 );
			Commands->Action_Attack( obj, params );
		}
	}
};


DECLARE_SCRIPT(BMG_Test_Display_Text, "" )
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message( "BMG_Test_Display_Text %s %d %f\n", "Hi", 5, 1.5f );
		Commands->Set_Display_Color( 0,0,255 );
		Commands->Display_Text( IDS_BYON );
		Commands->Display_Float( 1.5f );
		Commands->Display_Int( 5 );
		Commands->Set_Display_Color();
		Commands->Display_Text( IDS_BYON );
		Action_Complete( obj, 1, (ActionCompleteReason)0 );
	}

};

DECLARE_SCRIPT(BMG_Test_Poked, "" )
{
	void	Poked( GameObject * obj, GameObject * poker )		
	{
		Commands->Debug_Message_2("BMG_TEST_POKED %p Poked\n", (int)obj);
	}
};

DECLARE_SCRIPT(BMG_Test_Running, "" )
{
	int state;

	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Running\n");
		Action_Complete( obj, 1, (ActionCompleteReason)0 );
	}

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		state = !state;
		int priority = 80;
		ActionParamsStruct params;
		params.Set_Basic( this, priority, action_id+1 );
		if ( state ) {
			params.Set_Movement( Vector3( 13,-78,0 ), 1, 1 );
		} else {
			params.Set_Movement( Vector3( 69,-21,0 ), 1, 1 );
		}
		Commands->Action_Goto( obj, params );
	}
};

DECLARE_SCRIPT(BMG_Test_Objectives, "")
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Objectives\n");
		Commands->Start_Timer( obj, this,  5, 0 );
		Commands->Start_Timer( obj, this, 10, 1 );
		Commands->Start_Timer( obj, this, 15, 2 );
		Commands->Start_Timer( obj, this, 20, 3 );
		Commands->Start_Timer( obj, this, 25, 4 );
		Commands->Start_Timer( obj, this, 30, 5 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		switch ( Timer_ID ) {
			case 0:	Commands->Add_Objective( 100, OBJECTIVE_TYPE_PRIMARY, "Eat Your Vegetables" );
						break;
			case 1:	Commands->Set_Objective_Radar_Blip( 100, Vector3( 10,10,10 ) );
						break;
			case 2:	Commands->Change_Objective_Type( 100, OBJECTIVE_TYPE_SECONDARY );
						break;
			case 3:	Commands->Set_Objective_Status( 100, OBJECTIVE_STATUS_FAILED );
						break;
			case 4:	Commands->Remove_Objective( 100 );
						break;
			case 5:	Commands->Set_Objective_Radar_Blip_Object( 100, obj );
						break;
		}
	}
};


DECLARE_SCRIPT( BMG_Test_Priority_1, "" )
{
	void Created( GameObject * obj )
	{
		Commands->Debug_Message("BMG Test Priority 1\n");
		Commands->Start_Timer( obj, this, 2, 100 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message_2("BMG Test Priority 1 %d\n", timer_id);
		Commands->Send_Custom_Event( obj, obj, 500, 600, 8 );
//		Commands->Start_Timer( obj, this, 6, timer_id + 1 );
//		Commands->Action_Goto_Location( obj, this, 90, Vector3( 18,5,0 ), 0.3f, 1, 0 );

		ActionParamsStruct params;
		params.Set_Basic( this, 90, 0 );
		params.Set_Animation( "S_A_HUMAN.H_A_611A", false );
		Commands->Action_Play_Animation( obj, params );
//		Commands->Action_Play_Animation( obj, this, 90, "S_A_HUMAN.H_A_611A", false, 0 );


//		Vector3 pos = Commands->Get_Position( Commands->Get_The_Star() );
//		Commands->Action_Goto_Location( obj, this, 70, pos, 0.2f, 0.5f, 0 );
	}

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		Commands->Debug_Message_2("BMG Test Priority 1 Complete %d\n", complete_reason );
	}

	void	Custom( GameObject * obj, int type, int param, GameObject * sender )
	{
		Commands->Debug_Message_2("BMG Test Priority 1 Custom from %p\n", (int)sender );
	}
};

DECLARE_SCRIPT( BMG_Test_Priority_2, "" )
{
	void Created( GameObject * obj )
	{
		Commands->Debug_Message("BMG Test Priority 2\n");
		Commands->Start_Timer( obj, this, 3, 200 );
	}

	void Timer_Expired( GameObject * obj, int timer_id )
	{
		Commands->Debug_Message_2("BMG Test Priority 2 %d\n", timer_id);
		Commands->Send_Custom_Event( obj, obj, 700, 800 );

//		Commands->Start_Timer( obj, this, 3, timer_id + 1 );

		ActionParamsStruct params;
		params.Set_Basic( this, 80, 0 );
		params.Set_Movement( Vector3( -2,25,0 ), 0.3f, 1 );
		Commands->Action_Goto( obj, params );
	}

	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )
	{
		Commands->Debug_Message_2("BMG Test Priority 2 Complete %d\n", complete_reason );
	}

	void	Custom( GameObject * obj, int type, int param, GameObject * sender )
	{
		Commands->Debug_Message_2("BMG Test Priority 2 Custom from %p\n", (int)sender );
	}

};


DECLARE_SCRIPT(BMG_Test_Zone, "")
{

	void	Entered( GameObject * obj, GameObject * enterer )
	{
		Commands->Debug_Message("Test Zone Entered\n");
	}

	void	Exited( GameObject * obj, GameObject * exiter )	
	{
		Commands->Debug_Message("Test Zone Exited\n");
	}
};


DECLARE_SCRIPT(BMG_Test_Cinematic, "ScriptName=:string")
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG Cinematic\n");
		Commands->Set_Animation (obj, Get_Parameter("ScriptName"), 1);

		Commands->Start_Timer( obj, 5, 0 );

	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Debug_Message("BMG Timer\n");
		Commands->Start_Timer( obj, 5, 0 );

		Vector3	pos = Commands->Get_Position( obj );
		Commands->Create_Object_At_Bone( obj, "health 100 standard", "V_LANDGEAR1" );
		Commands->Create_Object_At_Bone( obj, "health 100 standard", "SEAT0" );
	}
};

DECLARE_SCRIPT(BMG_Test_C130_DropOff_Cinematic, "ObjToCreate=:string")
{
	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG_Test_C130_DropOff_Cinematic\n");
		Commands->Set_Animation (obj, "v_nod_c-130e3.m_cargo-drop", 0);
		float drop_frame = 446;
		float drop_time = drop_frame / 30;	// find the time assumeing 30fps
		Commands->Start_Timer( obj, drop_time, 0 );
	}

	void Timer_Expired (GameObject* obj, int Timer_ID)
	{
		Commands->Create_Object_At_Bone( obj, Get_Parameter("ObjToCreate"), "CARGO" );
	}

	void Animation_Complete(GameObject* obj, const char* name)
	{
		Commands->Debug_Message("Destroy the C130\n");
		Commands->Destroy_Object( obj );
	}

	void Killed(GameObject* obj, GameObject* killer)
	{
		Vector3 pos = Commands->Get_Bone_Position( obj, "CARGO" );
		Commands->Create_Explosion( "Rocket Explosion", pos, killer );
	}


};

DECLARE_SCRIPT(BMG_Test_Orca_Attack_Cinematic, "")
{
	enum {
		SHOOT,
		STOP
	};

	void Created(GameObject* obj)
	{
		float shoot_frame = 100;
		float shoot_time = shoot_frame / 30;	// find the time assumeing 30fps
		float stop_frame = 200;
		float stop_time = stop_frame / 30;	// find the time assumeing 30fps
		Commands->Start_Timer( obj, shoot_time, SHOOT );
		Commands->Start_Timer( obj, stop_time, STOP );
	}

	void Timer_Expired (GameObject* obj, int timer_id)
	{
		if ( timer_id == SHOOT ) {
			Commands->Trigger_Weapon( obj, true, Vector3(0,0,0) );
		}
		if ( timer_id == STOP ) {
			Commands->Trigger_Weapon( obj, false, Vector3(0,0,0) );
		}
	}

};

DECLARE_SCRIPT(BMG_Test_SAPO, "")
{
	enum {
		OPEN,
		CLOSE
	};

	void Created(GameObject* obj)
	{
		Commands->Debug_Message("BMG_Test_SAPO\n");
		Commands->Start_Timer( obj, 1, OPEN );
	}

	void Timer_Expired (GameObject* obj, int timer_id)
	{
		if ( timer_id == OPEN ) {
			Commands->Debug_Message("BMG_Test_SAPO OPEN\n");
			Commands->Static_Anim_Phys_Goto_Frame( 1051, 0 );
			Commands->Start_Timer( obj, 6, CLOSE );
		}
		if ( timer_id == CLOSE ) {
			Commands->Debug_Message("BMG_Test_SAPO CLOSE\n");
			Commands->Static_Anim_Phys_Goto_Last_Frame( 1051 );
			Commands->Start_Timer( obj, 6, OPEN );
		}
	}

};

DECLARE_SCRIPT(BMG_Test_Follow, "TargetID=:int")
{
	void Created(GameObject* obj)
	{
		Movement_Complete( obj, (MovementCompleteReason)0 );
	}

	void Movement_Complete(GameObject* obj, MovementCompleteReason reason)
	{
		Commands->Debug_Message("BMG Follow\n");
		GameObject * target = Commands->Find_Object( Get_Int_Parameter( "TargetID" ) );
		if ( target != NULL ) {
			Commands->Action_Movement_Follow_Object( obj, target, 1 );
		}
	}
};

DECLARE_SCRIPT(BMG_Test_Complete, "")
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, 3, 0 );
	}

	void Timer_Expired (GameObject* obj, int timer_id)
	{
		Commands->Mission_Complete( true );
	}

};

DECLARE_SCRIPT(BMG_Test_Debug, "position=3.1 4.2 5.3:vector3")
{
	void Created(GameObject* obj)
	{
		Vector3 pos1 = Get_Vector3_Parameter( 0 );
		Vector3 pos2 = Get_Vector3_Parameter( "position" );
		Commands->Display_Text("Got1 %f %f %f\n", pos1.X, pos1.Y, pos1.Z );
		Commands->Display_Text("Got2 %f %f %f\n", pos2.X, pos2.Y, pos2.Z );
//		Commands->Debug_Message("Debug Test 1\n");
//		Commands->Display_Text( "Debug Test 2\n" );
//		Commands->Display_Text_Colored( Vector3( 0,0,1 ), "Debug Test 3\n" );
//		Commands->Start_Timer( obj, 1, 0 );
	}

	void Timer_Expired (GameObject* obj, int timer_id)
	{
	}

};


DECLARE_SCRIPT(BMG_Test_Anim, "")
{
	void Created(GameObject* obj)
	{
		Commands->Start_Timer( obj, 0.001f, 0 );
	}

	void Timer_Expired (GameObject* obj, int timer_id)
	{
		Commands->Start_Timer( obj, 0.001f, 0 );
		Commands->Set_Animation( obj, "S_A_HUMAN.H_A_J99C", true );
	}
};

#endif
