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
*     Toolkit_Objectives.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Objectives Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 3 $
*     $Modtime: 11/30/00 2:41p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Objectives.cpp $
*
******************************************************************************/

#include "toolkit.h"

DECLARE_SCRIPT(M00_Objective_Controller_For_Objects_RMV, "Objective_ID:int, Objective_Type=1:int, Objective_Description_ID:int, Radar_Blip=1:int, Hidden=0:int, Object_ID:int, Custom_Type:int, Activate_Param=0:int, Unhide_Param=0:int, Success_Param:int, Failure_Param:int")
{
	int objective_id, objective_type, object_id, custom_type;
	int activate, success, failure;
	int description_id;
	bool blip, hidden;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(objective_id, 1);
		SAVE_VARIABLE(objective_type, 2);
		SAVE_VARIABLE(object_id, 3);
		SAVE_VARIABLE(custom_type, 4);
		SAVE_VARIABLE(activate, 5);
		SAVE_VARIABLE(success, 6);
		SAVE_VARIABLE(failure, 7);
		SAVE_VARIABLE(description_id, 8);
		SAVE_VARIABLE(blip, 9);
		SAVE_VARIABLE(hidden, 10);
	}
	
	void Created(GameObject * obj)
	{
		hidden = (Get_Int_Parameter("Hidden") == 1) ? true : false;
		blip = (Get_Int_Parameter("Radar_Blip") == 1) ? true : false;
		objective_id = Get_Int_Parameter("Objective_ID");
		objective_type = Get_Int_Parameter("Objective_Type");
		description_id = Get_Int_Parameter("Objective_Description_ID");
		object_id = Get_Int_Parameter("Object_ID");
		custom_type = Get_Int_Parameter("Custom_Type");
		activate = Get_Int_Parameter("Activate_Param");
		success = Get_Int_Parameter("Success_Param");
		failure = Get_Int_Parameter("Failure_Param");
		if ((activate == 0) && (hidden == false))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
			if ((blip) && (Commands->Find_Object(object_id) != NULL))
				Commands->Set_Objective_Radar_Blip_Object(objective_id, Commands->Find_Object(object_id));
		}
		else if ((activate == 0) && (hidden == true))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			}
			if ((blip) && (Commands->Find_Object(object_id) != NULL))
				Commands->Set_Objective_Radar_Blip_Object(objective_id, Commands->Find_Object(object_id));
		}
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if ((type == custom_type) && (param == activate))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
			if ((blip) && (Commands->Find_Object(object_id) != NULL))
				Commands->Set_Objective_Radar_Blip_Object(objective_id, Commands->Find_Object(object_id));
		}
		
		if ((type == custom_type) && (param == success))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_ACCOMPLISHED);
		}
		if ((type == custom_type) && (param == failure))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_FAILED);
		}
		if ((type == custom_type) && (hidden == true) && (param == Get_Int_Parameter("Unhide_Param")))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_PENDING);
		}
	}
};

DECLARE_SCRIPT(M00_Objective_Controller_For_Locations_RMV, "Objective_ID:int, Objective_Type=1:int, Objective_Description_ID:int, Radar_Blip=1:int, Hidden=0:int, Location:vector3, Custom_Type:int, Activate_Param=0:int, Unhide_Param=0:int, Success_Param:int, Failure_Param:int")
{
	int objective_id, objective_type, object_id, custom_type;
	int activate, success, failure;
	Vector3 location;
	int description_id;
	bool blip, hidden;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(objective_id, 1);
		SAVE_VARIABLE(objective_type, 2);
		SAVE_VARIABLE(object_id, 3);
		SAVE_VARIABLE(custom_type, 4);
		SAVE_VARIABLE(activate, 5);
		SAVE_VARIABLE(success, 6);
		SAVE_VARIABLE(failure, 7);
		SAVE_VARIABLE(description_id, 8);
		SAVE_VARIABLE(blip, 9);
		SAVE_VARIABLE(hidden, 10);
		SAVE_VARIABLE(location, 11);
	}
	
	void Created(GameObject * obj)
	{
		location = Get_Vector3_Parameter("Location");
		hidden = (Get_Int_Parameter("Hidden") == 1) ? true : false;
		blip = (Get_Int_Parameter("Radar_Blip") == 1) ? true : false;
		objective_id = Get_Int_Parameter("Objective_ID");
		objective_type = Get_Int_Parameter("Objective_Type");
		description_id = Get_Int_Parameter("Objective_Description_ID");
		custom_type = Get_Int_Parameter("Custom_Type");
		activate = Get_Int_Parameter("Activate_Param");
		success = Get_Int_Parameter("Success_Param");
		failure = Get_Int_Parameter("Failure_Param");
		if ((activate == 0) && (hidden == false))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
			if (blip)
				Commands->Set_Objective_Radar_Blip(objective_id, location);
		}
		else if ((activate == 0) && (hidden == true))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			}
			if (blip)
				Commands->Set_Objective_Radar_Blip(objective_id, location);
		}
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if ((type == custom_type) && (param == activate))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
			if (blip)
				Commands->Set_Objective_Radar_Blip(objective_id, location);
		}
		
		if ((type == custom_type) && (param == success))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_ACCOMPLISHED);
		}
		if ((type == custom_type) && (param == failure))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_FAILED);
		}
		if ((type == custom_type) && (hidden == true) && (param == Get_Int_Parameter("Unhide_Param")))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_PENDING);
		}
	}
};

DECLARE_SCRIPT(M00_Objective_Radar_Blip_On_Object_RMV, "Objective_ID:int, Activate_Type=0:int, Activate_Param=0:int")
{
	void Created(GameObject * obj)
	{
		bool activate = ((Get_Int_Parameter("Activate_Type") == 0) && (Get_Int_Parameter("Activate_Param") == 0)) ? true : false;
		int objective_id = Get_Int_Parameter("Objective_ID");
		if (activate)
			Commands->Set_Objective_Radar_Blip_Object(objective_id, obj);
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		int objective_id = Get_Int_Parameter("Objective_ID");
		if ((Get_Int_Parameter("Activate_Type") == type) && (Get_Int_Parameter("Activate_Param") == param))
			Commands->Set_Objective_Radar_Blip_Object(objective_id, obj);
	}
};

DECLARE_SCRIPT(M00_Objective_Controller_For_Objects_Multiple_Triggers_RMV, "Objective_ID:int, Objective_Type=1:int, Objective_Description_ID:int, Hidden=0:int, Custom_Type:int, Activate_Param=0:int, Number_Of_Triggers:int, Trigger_Param:int, Unhide_Param=0:int, Failure_Param:int")
{
	int objective_id, objective_type, custom_type;
	int activate, number, trigger, failure;
	int so_far;
	bool hidden;
	int description_id;

	REGISTER_VARIABLES()
	{
		SAVE_VARIABLE(objective_id, 1);
		SAVE_VARIABLE(objective_type, 2);
		SAVE_VARIABLE(custom_type, 4);
		SAVE_VARIABLE(activate, 5);
		SAVE_VARIABLE(number, 6);
		SAVE_VARIABLE(failure, 7);
		SAVE_VARIABLE(description_id, 8);
		SAVE_VARIABLE(trigger, 9);
		SAVE_VARIABLE(hidden, 10);
		SAVE_VARIABLE(so_far, 3);
	}
	
	void Created(GameObject * obj)
	{
		hidden = (Get_Int_Parameter("Hidden") == 1) ? true : false;
		so_far = 0;
		objective_id = Get_Int_Parameter("Objective_ID");
		objective_type = Get_Int_Parameter("Objective_Type");
		description_id = Get_Int_Parameter("Objective_Description_ID");
		custom_type = Get_Int_Parameter("Custom_Type");
		activate = Get_Int_Parameter("Activate_Param");
		number = Get_Int_Parameter("Number_Of_Triggers");
		trigger = Get_Int_Parameter("Trigger_Param");
		failure = Get_Int_Parameter("Failure_Param");

		if ((activate == 0) && (hidden == false))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
		}
		else if ((activate == 0) && (hidden == true))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_HIDDEN, description_id, NULL, 0);
					break;
			}
		}
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if ((type == custom_type) && (param == activate))
		{
			switch (objective_type)
			{
			case 1: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_PRIMARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 2: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_SECONDARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			case 3: Commands->Add_Objective(objective_id, OBJECTIVE_TYPE_TERTIARY, OBJECTIVE_STATUS_PENDING, description_id, NULL, 0);
					break;
			}
			if (so_far >= number)
				Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_ACCOMPLISHED);
		}
		if ((type == custom_type) && (param == trigger))
		{
			so_far++;
			if (so_far >= number)
				Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_ACCOMPLISHED);
		}
		if ((type == custom_type) && (param == failure))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_FAILED);
		}
		if ((type == custom_type) && (hidden == true) && (param == Get_Int_Parameter("Unhide_Param")))
		{
			Commands->Set_Objective_Status(objective_id, OBJECTIVE_STATUS_PENDING);
		}
	}
};

DECLARE_SCRIPT(M00_Global_Objective_Controller_RMV, "Set_Type:int, Set_Status:int, Remove:int")
{
	int set_type, set_status, remove;
	int i_obj, i_new;
	float f_obj;

	void Created(GameObject * obj)
	{
		set_type = Get_Int_Parameter("Set_Type");
		set_status = Get_Int_Parameter("Set_Status");
		remove = Get_Int_Parameter("Remove");
	}

	void Custom(GameObject * obj, int type, int param, GameObject * sender)
	{
		if (type == set_type)
		{
			f_obj = (float)param;
			f_obj /= 10.0;
			i_obj = int(f_obj);
			i_new = (f_obj - i_obj) * 10;
			switch (i_new) {
			case 1: Commands->Change_Objective_Type(i_obj, OBJECTIVE_TYPE_PRIMARY);
			case 2: Commands->Change_Objective_Type(i_obj, OBJECTIVE_TYPE_SECONDARY);
			case 3: Commands->Change_Objective_Type(i_obj, OBJECTIVE_TYPE_TERTIARY);
			}
		}
		else if (type == set_status)
		{
			f_obj = (float)param;
			f_obj /= 10.0;
			i_obj = int(f_obj);
			i_new = (f_obj - i_obj) * 10;
			switch (i_new) {
			case 1: Commands->Set_Objective_Status(i_obj, OBJECTIVE_STATUS_PENDING);
			case 2: Commands->Set_Objective_Status(i_obj, OBJECTIVE_STATUS_ACCOMPLISHED);
			case 3: Commands->Set_Objective_Status(i_obj, OBJECTIVE_STATUS_FAILED);
			case 4: Commands->Set_Objective_Status(i_obj, OBJECTIVE_STATUS_HIDDEN);
			}
		}
		else if (type == remove)
		{
			Commands->Remove_Objective(param);
		}
	}
};
