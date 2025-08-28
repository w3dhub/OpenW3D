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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/buildingmonitor.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/29/01 11:03a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __BUILDINGMONITOR_H
#define __BUILDINGMONITOR_H

#include "gameobjobserver.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class BuildingGameObj;


////////////////////////////////////////////////////////////////
//
//	BuildingMonitorClass
//
////////////////////////////////////////////////////////////////
class BuildingMonitorClass : public GameObjObserverClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	BuildingMonitorClass (void) :
		Building (NULL)					{} 
	~BuildingMonitorClass (void)		{} 

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	void				Set_Building (BuildingGameObj *building)	{ Building = building; }

	//
	//	From GameObjObeserverClass
	//
	const char *	Get_Name (void)	{ return "BuildingMonitorClass"; }
	void				Killed (GameObject *game_obj, GameObject *killer);
	void				Damaged (GameObject *game_obj, GameObject *damager, float amount );
	void				Custom (GameObject *game_obj, int type, uintptr_t param, GameObject *sender);
	
	//
	//	Unused methods from the base class
	//
	void				Attach (GameObject *)		{}
	void				Detach (GameObject *)		{}
	void				Created (GameObject *)		{}
	void				Destroyed (GameObject *)	{}
	void				Sound_Heard (GameObject *, const CombatSound &)	{}
	void				Enemy_Seen (GameObject *, GameObject *)			{}
	void				Action_Complete (GameObject *, int, ActionCompleteReason)	{}
	void				Timer_Expired (GameObject *, int)					{}
	void				Animation_Complete (GameObject *, const char *)	{}
	void				Poked (GameObject *, GameObject *)					{}
	void				Entered (GameObject *, GameObject *)				{}
	void				Exited (GameObject *, GameObject *)					{}

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	BuildingGameObj *	Building;
};


#endif //__BUILDINGMONITOR_H

