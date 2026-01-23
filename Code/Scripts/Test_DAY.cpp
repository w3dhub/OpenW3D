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
*     $Author: David_y $
*     $Revision: 75 $
*     $Modtime: 1/14/02 12:40a $
*     $Archive: /Commando/Code/Scripts/Test_DAY.cpp $
*
******************************************************************************/

#include <cstdint>
#include "Toolkit.h"


DECLARE_SCRIPT(M00_Screenshot_Poser_DAY, "Anim_Name:string" )
{
	
	void Created( GameObject *obj ) override
	{
		Commands->Set_Loiters_Allowed( obj, false );
		Commands->Innate_Disable( obj );
		Commands->Innate_Soldier_Enable_Enemy_Seen( obj, false );
		Commands->Innate_Soldier_Enable_Gunshot_Heard( obj, false );
		Commands->Innate_Soldier_Enable_Footsteps_Heard( obj, false );
		Commands->Innate_Soldier_Enable_Bullet_Heard( obj, false );
		Commands->Innate_Soldier_Enable_Actions( obj, false );

		ActionParamsStruct params;
		params.Set_Basic( this, 99, 0 );
		params.Set_Animation( Get_Parameter( "Anim_Name" ), true );
		Commands->Action_Play_Animation( obj, params );
	}
	
	void Action_Complete( GameObject * obj, int action_id, ActionCompleteReason /*complete_reason*/ ) override
	{
		if ( action_id == 0 )
		{
			ActionParamsStruct params;
			params.Set_Basic( this, 99, 0 );
			params.Set_Animation( Get_Parameter( "Anim_Name" ), true );
			Commands->Action_Play_Animation( obj, params );
		}
	}
};


DECLARE_SCRIPT (DAY_TestScriptOne, "")
{

	int action;

	void Created ( GameObject * obj ) override
	{
		action = 1;
		ActionParamsStruct params;
		params.Set_Basic( this, 99, 1 );
		params.Set_Movement( Commands->Get_Position( Commands->Find_Object(100077) ), 0.6f, 1.0f, false );
		params.Set_Attack( Commands->Find_Object(100079), 40.0f, 0.0f, true );
		params.AttackCrouched = true;
		params.MoveCrouched = true;
		Commands->Action_Attack( obj, params );
		Commands->Send_Custom_Event( obj, obj, 0, 0, 5.0f );
	}

	void Custom( GameObject *obj, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( action == 1 )
		{
			action = 2;
			ActionParamsStruct params;
			params.Set_Basic( this, 99, 2 );
			params.Set_Movement( Commands->Get_Position( Commands->Find_Object(100078) ), 0.6f, 1.0f, false );
			params.Set_Attack( Commands->Find_Object(100079), 40.0f, 0.0f, true );
			params.AttackCrouched = true;
			params.MoveCrouched = true;
			params.AttackCheckBlocked = false;
			Commands->Action_Attack( obj, params );
			Commands->Send_Custom_Event( obj, obj, 0, 0, 5.0f );
		}
		else
		{
			action = 1;
			ActionParamsStruct params;
			params.Set_Basic( this, 99, 1 );
			params.Set_Movement( Commands->Get_Position( Commands->Find_Object(100077) ), 0.6f, 1.0f, false );
			params.Set_Attack( Commands->Find_Object(100079), 40.0f, 0.0f, true );
			params.AttackCrouched = true;
			params.MoveCrouched = true;
			Commands->Action_Attack( obj, params );
			Commands->Send_Custom_Event( obj, obj, 0, 0, 5.0f );
		}

	}

};

DECLARE_SCRIPT (DAY_TestScriptTwo, "")
{
	void Created( GameObject *obj ) override
	{
		Commands->Innate_Disable(obj);
	}

};

DECLARE_SCRIPT (DAY_TestScriptThree, "")
{
	void Created( GameObject *obj ) override
	{
		Commands->Create_Explosion( "Explosion_Rocket_Gunboat_M01", Commands->Get_Position(obj), obj);
	}

};

DECLARE_SCRIPT (DAY_VTOL_CircleAttack, "")
{

	int damager_id;

	Vector3 Get_Circle_Position( GameObject *obj, GameObject *target , float radius , float reverseprob)
	{
		double theta_angle = 90;
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
		targetlocation.Z += Commands->Get_Safe_Flight_Height( targetlocation.X, targetlocation.Y );
		return targetlocation;
	}
	
	void Created( GameObject * /*obj*/ ) override
	{
	}

	void Damaged( GameObject *obj , GameObject *damager, float /*amount*/ ) override
	{
		if ( damager )
		{
		damager_id = Commands->Get_ID(damager);
		}
		Commands->Send_Custom_Event( obj, obj, 1, 1, 0.0f );
	}

	void Custom( GameObject * obj, int type, intptr_t param, GameObject * /*sender*/ ) override
	{
		if ( type == 1 )
		{
			if ( param == 1 )
			{
				GameObject *target = Commands->Find_Object(damager_id);
				if (target)
				{
					ActionParamsStruct params;
					params.Set_Basic( this, 99, 1 );
					params.Set_Movement(Get_Circle_Position( obj, target, 5.0f, 0.1f ), 1.0f, 1.0f, false );
					params.Set_Attack(target, 40.0f, 0.0f, true );
					Commands->Action_Attack( obj, params );
					Commands->Send_Custom_Event( obj, obj, 1, 1, 5.0f );
				}
			}
		}
	}

};


DECLARE_SCRIPT (M00_GrantPowerup_Created, "WeaponDef:string")
{

	void Created( GameObject *obj) override
	{
		const char *Weapon = Get_Parameter( "WeaponDef" );
		Commands->Give_PowerUp( obj, Weapon, false );
	}

};


DECLARE_SCRIPT (M00_VisceroidInnate_DAY, "")
{
	void Created( GameObject *obj ) override
	{	
		Commands->Set_Loiters_Allowed( obj, false );
		Commands->Set_Animation( obj, "C_Visceroid.C_Visceroid", true, NULL, 0.0f, -1.0f, false );
	}

	void Killed( GameObject *obj, GameObject * /*killer*/ ) override
	{
		Commands->Set_Animation( obj, "C_Visceroid.C_Visceroid_Die", false, NULL, 0.0f, -1.0f, false );
	}

};


// Generic script to replace destroyed models (Turrets, etc)

DECLARE_SCRIPT (M00_DestroyedStateObject_DAY, "OriginalModelFacing:float,DestroyedModelPreset:string")
{

	void Destroyed( GameObject *obj ) override
	{
		Vector3 object_pos = Commands->Get_Position(obj);
		GameObject *DestroyedModel;
		DestroyedModel = Commands->Create_Object( Get_Parameter("DestroyedModelPreset"), object_pos );
		Commands->Set_Facing( DestroyedModel, Get_Float_Parameter("OriginalModelFAcing") );
	}

};


// For playing ambient sounds on characters - DAY
DECLARE_SCRIPT(M00_Play_Sound_Object_Bone_DAY, "Sound_Preset:string, Frequency_Min=-1.0:float, Frequency_Max:float")
{
	void Created(GameObject * obj) override
	{
		if (Get_Int_Parameter("Frequency_Min") == -1)
		{
			Timer_Expired(obj, 0);
		}
		else
		{
			float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min"), Get_Float_Parameter("Frequency_Max"));
			Commands->Start_Timer(obj, this, time, 0);
		}
	}

	void Timer_Expired(GameObject * obj, int /*timer_id*/) override
	{
		const char * sound = Get_Parameter("Sound_Preset");
		Vector3 pos = Commands->Get_Position(obj);
		pos += Get_Vector3_Parameter("Offset");

		int id;
		
		Commands->Debug_Message("Playing 3D Sound\n");
		id = Commands->Create_3D_Sound_At_Bone( sound, obj, "ROOTTRANSFORM" );

		Commands->Monitor_Sound(obj, id);
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if (type == CUSTOM_EVENT_SOUND_ENDED)
		{
			if (Get_Int_Parameter("Frequency_Min") < 0 )
			{
				float time = Commands->Get_Random(Get_Float_Parameter("Frequency_Min"), Get_Float_Parameter("Frequency_Max"));
				Commands->Start_Timer(obj, this, time, 0);
			}
		}
	}
};


DECLARE_SCRIPT (M00_PlayAnimation_DestroyObject_DAY, "AnimationName:string")
{

	void Created( GameObject * obj ) override
	{
		const char * animname = Get_Parameter("AnimationName");
		Commands->Set_Animation( obj, animname, 0, NULL, 0.0f, -1.0f, false );
	}

	void Animation_Complete( GameObject * obj, const char * /*animation_name*/ ) override
	{
		Commands->Destroy_Object(obj);
	}

};


DECLARE_SCRIPT (M00_Disable_Loiter_DAY, "")
{

	void Created( GameObject * obj ) override
	{
		Commands->Set_Loiters_Allowed( obj , false );
	}

	void Custom( GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/ ) override
	{
		if ( type == M00_LOITER_ENABLE_TOGGLE )
		{
			Commands->Set_Loiters_Allowed( obj, true );
		}
	}

};

DECLARE_SCRIPT (M00_Cinematic_Kill_Object_DAY, "")
{

	void Created( GameObject * obj ) override
	{
		Commands->Apply_Damage( obj, 10000.0f, "BlamoKiller", NULL );
	}

};

DECLARE_SCRIPT (M00_Set_Background_Music_DAY, "MusicFile:string")
{

	void Created( GameObject * /*obj*/ ) override
	{
		Commands->Set_Background_Music( Get_Parameter("MusicFile") );
	}

};

