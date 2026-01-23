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
*     Toolkit_Explosions.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Explosions Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: David_y $
*     $Revision: 5 $
*     $Modtime: 10/25/01 3:36p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Explosions.cpp $
*
******************************************************************************/

#include "Toolkit.h"

/*
Editor Script - M00_Explosion_Create_RMV

  This creates an explosion at a location.

  Parameters:

  Explosion_Name	= The name of the explosion type to make.
  Origin			= The Vector3 location of the explosion.
*/

DECLARE_SCRIPT(M00_Explosion_Create_RMV, "Start_Now=0:int, Create_At_Obj=0:int, Receive_Type:int, Receive_Param_On=1:int, Explosion_Name:string, Origin:vector3, Debug_Mode=0:int")
{
	Vector3	explosion_origin;
	int		custom_type;
	int		parameter;
	bool	debug_mode;

	void Created(GameObject * obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		if (Get_Int_Parameter("Create_At_Obj"))
		{
			explosion_origin = Commands->Get_Position(obj);
		}
		else
		{
			explosion_origin = Get_Vector3_Parameter("Origin");
		}
		custom_type = Get_Int_Parameter("Receive_Type");
		parameter = Get_Int_Parameter("Receive_Param_On");
		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_RMV ACTIVATED.\n"));
			Commands->Create_Explosion(Get_Parameter("Explosion_Name"), explosion_origin, obj);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_RMV received custom type %d, param %d.\n", type, param));
		if ((type == custom_type) && (param == parameter))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_RMV ACTIVATED.\n"));
			Commands->Create_Explosion(Get_Parameter("Explosion_Name"), explosion_origin, obj);
		}
	}
};

/*
Editor Script - M00_Explosion_Create_At_Bone_RMV

  This creates an explosion on a specific bone of an object.

  Parameters:

  Explosion_Name	= The name of the explosion to create.
  Object_ID			= The ID of the object to create the explosion on.
  Bone_Name			= The bone name to create the explosion on.
*/

DECLARE_SCRIPT(M00_Explosion_Create_At_Bone_RMV, "Start_Now=0:int, Receive_Type:int, Receive_Param_On:int, Explosion_Name:string, Object_ID:int, Bone_Name:string, Debug_Mode=0:int")
{
	int		target_id;
	int		custom_type;
	int		parameter;
	bool	debug_mode;

	void Created(GameObject * obj) override
	{
		debug_mode = (Get_Int_Parameter("Debug_Mode") == 1) ? true : false;
		custom_type = Get_Int_Parameter("Receive_Type");
		parameter = Get_Int_Parameter("Receive_Param_On");
		target_id = Get_Int_Parameter("Object_ID");
		if (Get_Int_Parameter("Start_Now"))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_At_Bone_RMV ACTIVATED.\n"));
			if (Commands->Find_Object(target_id) != NULL)
				Commands->Create_Explosion_At_Bone(Get_Parameter("Explosion_Name"), Commands->Find_Object(target_id), Get_Parameter("Bone_Name"), obj);
		}
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_At_Bone_RMV received custom type %d, param %d.\n", type, param));
		if ((type == custom_type) && (param == parameter))
		{
			SCRIPT_DEBUG_MESSAGE(("M00_Explosion_Create_At_Bone_RMV ACTIVATED.\n"));
			if (Commands->Find_Object(target_id) != NULL)	
				Commands->Create_Explosion_At_Bone(Get_Parameter("Explosion_Name"), Commands->Find_Object(target_id), Get_Parameter("Bone_Name"), obj);
		}
	}
};


DECLARE_SCRIPT(M00_Create_Random_Explosion_DLS, "Explosion_Name:string, Delay_Min=0.0f:float, Delay_Max=4.0f:float, Loc_ID0=0:int, Loc_ID1=0:int, Loc_ID2=0:int, Loc_ID3=0:int, Loc_ID4=0:int, Loc_ID5=0:int, Loc_ID6=0:int, Loc_ID7=0:int, Loc_ID8=0:int, Loc_ID9=0:int")
{
	bool active;
	int random;
	int last;
	int loc_id[10];
	

	enum{EXPLODE};

	// Register variables to be Auto-Saved
	// All variables must have a unique ID, less than 256, that never changes
	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE( active, 1 );
		SAVE_VARIABLE( random, 2 );
		SAVE_VARIABLE( last, 3 );
	}

	void Created(GameObject * /*obj*/) override
	{
		loc_id[0] = Get_Int_Parameter("Loc_ID0");
		loc_id[1] = Get_Int_Parameter("Loc_ID1");
		loc_id[2] = Get_Int_Parameter("Loc_ID2");
		loc_id[3] = Get_Int_Parameter("Loc_ID3");
		loc_id[4] = Get_Int_Parameter("Loc_ID4");
		loc_id[5] = Get_Int_Parameter("Loc_ID5");
		loc_id[6] = Get_Int_Parameter("Loc_ID6");
		loc_id[7] = Get_Int_Parameter("Loc_ID7");
		loc_id[8] = Get_Int_Parameter("Loc_ID8");
		loc_id[9] = Get_Int_Parameter("Loc_ID9");

		active = false;
		random = 0;
		last = 0;
	}

	void Custom(GameObject * obj, int type, intptr_t param, GameObject * /*sender*/) override
	{
		
		if(type == M00_CREATE_RANDOM_EXPLOSION && param == 1)
		{
			active = true;
			float delay = Commands->Get_Random(Get_Float_Parameter("Delay_Min"), Get_Float_Parameter("Delay_Max"));
			Commands->Start_Timer (obj, this, delay, EXPLODE);
		}
		if(type == M00_CREATE_RANDOM_EXPLOSION && param == 0)
		{
			active = false;
		}

	}

	void Timer_Expired(GameObject* obj, int timer_id) override
	{
		if(timer_id == EXPLODE && active)
		{
			while (random == last || loc_id[random] == 0) 
			{
				random = Commands->Get_Random_Int(0, 9);
			}

			Commands->Create_Explosion(Get_Parameter("Explosion_Name"), Commands->Get_Position(Commands->Find_Object(loc_id[random])), obj);
			last = random;

			float delay = Commands->Get_Random(Get_Float_Parameter("Delay_Min"), Get_Float_Parameter("Delay_Max"));
			Commands->Start_Timer (obj, this, delay, EXPLODE);
		}
	}
};


DECLARE_SCRIPT( M00_NukeStrike_Anim, "")
{

	void Created(GameObject * obj) override
	{
		Commands->Send_Custom_Event( obj, obj, 0, 1, 206/30 );
		Commands->Send_Custom_Event( obj, obj, 0, 2, 250/30 );
		Commands->Send_Custom_Event( obj, obj, 0, 3, 350/30 );

		GameObject *Nuke;
		Nuke = Commands->Create_Object( "Generic_Cinematic", Commands->Get_Position(obj) );
		if ( Nuke )
		{
			Commands->Set_Model( Nuke, "XG_AG_Nuke" );
			Commands->Attach_Script( Nuke, "M00_PlayAnimation_DestroyObject_DAY", "Nuke_Missle.Nuke_Missle");
			Commands->Create_3D_Sound_At_Bone( "SFX.Nuclear_Strike_Buildup", Nuke, "ROOTTRANSFORM" );
		}
	}

	void Custom(GameObject * obj, int type, intptr_t /*param*/, GameObject * /*sender*/) override
	{
		if ( type == 1 )
		{
			GameObject *NukeCloud;
			NukeCloud = Commands->Create_Object( "Generic_Cinematic", Commands->Get_Position(obj) );
			if ( NukeCloud )
			{
				Commands->Set_Model( NukeCloud, "XG_AG_Nukecloud" );
				Commands->Attach_Script( NukeCloud, "M00_PlayAnimation_DestroyObject_DAY", "Nuke_cloud.Nuke_cloud");
			}

			Commands->Shake_Camera( Commands->Get_Position(obj), 2.0f, 0.5f, 2.0f );
		}
		else if ( type == 2 )
		{
			Commands->Shake_Camera( Commands->Get_Position(obj), 2.0f, 1.0f, 3.0f );
		}
		else if ( type == 3 )
		{
			GameObject *NukeCloudTwo;
			NukeCloudTwo = Commands->Create_Object( "Generic_Cinematic", Commands->Get_Position(obj) );
			if ( NukeCloudTwo )
			{
				Commands->Set_Model( NukeCloudTwo, "XG_AG_NukeCloud" );
				Commands->Attach_Script( NukeCloudTwo, "M00_PlayAnimation_DestroyObject_DAY", "XG_Nukecloud_02");
			}
		}
	}

};
