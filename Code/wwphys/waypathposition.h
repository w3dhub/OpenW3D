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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/waypathposition.h   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/04/01 9:33a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WAYPATHPOSITION_H
#define __WAYPATHPOSITION_H


////////////////////////////////////////////////////////////////
//
//	WaypathPositionClass
//
////////////////////////////////////////////////////////////////
class WaypathPositionClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	WaypathPositionClass (void)
		:	WaypathID (0),
			WaypointIndex (0),
			Percent (0)					{}
	~WaypathPositionClass (void)	{}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Accessors
	//
	void			Set_Waypath_ID (int id)				{ WaypathID = id; }
	int			Get_Waypath_ID (void) const		{ return WaypathID; }

	void			Set_Waypoint_Index (int index)	{ WaypointIndex = index; }
	int			Get_Waypoint_Index (void) const	{ return WaypointIndex; }

	void			Set_Percent (float percent)		{ Percent = percent; }
	float			Get_Percent (void) const			{ return Percent; }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int		WaypathID;
	int		WaypointIndex;
	float		Percent;
};


#endif //__WAYPATHPOSITION_H
