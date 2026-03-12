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
 *                 Project Name : combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/unitcoordinationzonemgr.h                                                                                                                                                                                                                                                                                                                                              $Modtime:: 11/15/2001 2:58p                                            $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __UNITCOORDINATIONZONEMGR_H
#define __UNITCOORDINATIONZONEMGR_H

#include "vector.h"
#include "aabox.h"


//////////////////////////////////////////////////////////////////////
//
//	UnitCoordinationZoneMgr
//
//////////////////////////////////////////////////////////////////////
class UnitCoordinationZoneMgr
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	static void		Build_Zones (void);
	static void		Reset (void);
	static bool		Is_Unit_In_Zone (const Vector3 &pos);
	static void		Display_Debug_Boxes (void);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	static void		Detect_Ladder_Zones (void);
	static void		Detect_Elevator_Zones (void);


	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static DynamicVectorClass<AABoxClass>	ZoneList;
};


#endif //__UNITCOORDINATIONZONEMGR_H
