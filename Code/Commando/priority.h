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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/priority.h                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                 $*
 *                                                                                             *
 *                     $Modtime:: 2/23/02 12:34p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __PRIORITY_H__
#define __PRIORITY_H__

#include "networkobject.h"

class SoldierGameObj;

//-----------------------------------------------------------------------------
//
// Server computation of object priority for a given client
//
class	cPriority
{
public:
	static float			Compute_Object_Priority(int client_id, const Vector3 & client_pos, NetworkObjectClass * p_netobject, bool do_it_anyway = false, SoldierGameObj *client_soldier = NULL);
	static float			Get_Object_Distance(const Vector3 &	client_pos, NetworkObjectClass * p_netobject);

	static float			Compute_Object_Priority_2(int client_id, const Vector3 & client_pos, NetworkObjectClass * p_netobject, bool do_it_anyway = false, SoldierGameObj *client_soldier = NULL);
	static float			Get_Object_Distance_2(const Vector3 &	client_pos, NetworkObjectClass * p_netobject);

private:
	static float			Compute_Facing_Factor(int client_id, const Vector3 &	client_pos, NetworkObjectClass * p_netobject, SoldierGameObj *client_soldier = NULL);
	static float			Compute_Type_Factor(NetworkObjectClass * p_netobject);
	static float			Compute_Relevance_Factor(int client_id, NetworkObjectClass * p_netobject, SoldierGameObj *client_soldier = NULL);

	static float			Compute_Facing_Factor_2(int client_id, const Vector3 &	client_pos, NetworkObjectClass * p_netobject, SoldierGameObj *client_soldier = NULL);
	static float			Compute_Type_Factor_2(NetworkObjectClass * p_netobject, float distance);
	static float			Compute_Relevance_Factor_2(int client_id, NetworkObjectClass * p_netobject, SoldierGameObj *client_soldier = NULL);


	static float			MaxDistance;
	static const float	MAX_FACING_PENALTY;
	static const float	TURRET_FACTOR;
	static const float	VEHICLE_FACTOR;
	static const float	SOLDIER_FACTOR;
	static const float	SOLDIER_IN_VEHICLE_FACTOR;
	static const float	BUILDING_FACTOR;

};

//-----------------------------------------------------------------------------

#endif	// __PRIORITY_H__
