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
*     Mission3.cpp
*
* DESCRIPTION
*     Mission 3 script
*
* PROGRAMMER
*     Design Team
*
* VERSION INFO
*     $Author: Dan_e $
*     $Revision: 1 $
*     $Modtime: 5/02/02 4:31p $
*     $Archive: /Commando/Code/Scripts/MissionS04.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "Toolkit.h"
#include <string.h>
#include <stdio.h>
#include "Mission3.h"

#ifdef _XBOX
#include "wwhack.h"

#endif

DECLARE_SCRIPT(MS04_Gunboat_Waypath_Movement, "")
{
	void Created( GameObject * obj ) override
	{
		ActionParamsStruct params;
		params.Set_Basic( this, 45, 999 );
		params.Set_Movement( Vector3(0,0,0), 1.0f, 3.0f );
		params.WaypathID = 100033;
		Commands->Action_Goto( obj, params );
	}
}
