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
*     Test_DEL.cpp
*
* DESCRIPTION
*     Denzil's test scripts
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Ryan_v $
*     $Revision: 4 $
*     $Modtime: 7/17/00 10:15a $
*     $Archive: /Commando/Code/Scripts/Test_DEL.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "common.h"
#include "DPrint.h"
#include <string.h>
#include <assert.h>

#if 0
DECLARE_SCRIPT(DEL_VehicleEnterTest, "X:float,Y:float,Z:float")
	{
	void Created(GameObject* object) override
		{
		float x = Get_Float_Parameter("X");
		float y = Get_Float_Parameter("Y");
		float z = Get_Float_Parameter("Z");
		DebugPrint("Enter Location %.2f,%.2f,%.2f\n", x, y, z);

		Commands->Action_Movement_Goto_Location(object, Vector3(4.88f, -66.22f, 0.0f), 1.0f);
		}
	};
#endif