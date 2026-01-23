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
*     Toolkit_Spawners.cpp
*
* DESCRIPTION
*     Designer Toolkit for Mission Construction - Spawners Subset
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 4 $
*     $Modtime: 10/23/00 5:20p $
*     $Archive: /Commando/Code/Scripts/Toolkit_Spawners.cpp $
*
******************************************************************************/

#include "Toolkit.h"

DECLARE_SCRIPT(M00_Spawner_Controller_RMV, "Spawner_ID:int, Custom_Type_To_Enable:int, Custom_Type_To_Trigger:int, On_When_Created=0:int")
{
	int id, enable, trigger;
	bool start_on;
	
	void Created(GameObject * /*obj*/) override
	{
		id = Get_Int_Parameter("Spawner_ID");
		enable = Get_Int_Parameter("Custom_Type_To_Enable");
		trigger = Get_Int_Parameter("Custom_Type_To_Trigger");
		start_on = (Get_Int_Parameter("On_When_Created") == 1) ? true : false;
		if (Commands->Find_Object(id) != NULL)
			Commands->Enable_Spawner(id, start_on);
	}
	void Custom(GameObject * /*obj*/, int type, intptr_t param, GameObject * /*sender*/) override
	{
		if (type == enable)
		{
			bool toggle = (param == 1) ? true : false;
			if (Commands->Find_Object(id) != NULL)
				Commands->Enable_Spawner(id, toggle);
		}
		if (type == trigger)
		{
//			bool force = (param == 1) ? true : false;
			if (Commands->Find_Object(id) != NULL)
				Commands->Trigger_Spawner(id);
		}
	}
};
