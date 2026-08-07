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
 *                     $Archive:: /Commando/Code/Commando/priority.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                 $*
 *                                                                                             *
 *                     $Modtime:: 2/23/02 12:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "priority.h"

#include <math.h>

#include "wwdebug.h"
#include "wwmath.h"
#include "vector3.h"
#include "gameobjmanager.h"
#include "soldier.h"
#include "humanphys.h"
#include "apppackettypes.h"
#include "vehicle.h"
#include "useroptions.h"
#include "playermanager.h"
#include "wwprofile.h"

//
// Class statics
//
float				cPriority::MaxDistance						= 300.0F;
const float		cPriority::TURRET_FACTOR					= 0.2f;
const float		cPriority::VEHICLE_FACTOR					= 0.85f;
const float		cPriority::SOLDIER_FACTOR					= 1.0f;
const float		cPriority::SOLDIER_IN_VEHICLE_FACTOR	= 0.1f;
const float		cPriority::BUILDING_FACTOR					= 0.2f;

//-----------------------------------------------------------------------------
//
// Compute the priority of the given netobject to the given client
//
float
cPriority::Compute_Object_Priority
(
	int							client_id,
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject,
	bool							do_it_anyway,
	SoldierGameObj *			client_soldier
)
{
	WWPROFILE("ObjPri");
	WWASSERT(client_id > 0);
	WWASSERT(p_netobject != nullptr);

	//
	// Compute the priority of this object to the given client at his given position.
	// Priority depends on physical distance. Objects with no physical location will
	// have a priority of 1.
	//
	float priority = 0.0f;

	if (p_netobject->Is_Client_Dirty(client_id) || do_it_anyway)
	{
		float distance = Get_Object_Distance(client_pos, p_netobject);

		//
		// Priority simply decreases linearly with distance and is zero at MaxDistance.
		//
		priority = 1 - distance / MaxDistance;

		if (priority > 0.0f) {
			priority *= Compute_Facing_Factor(client_id, client_pos, p_netobject, client_soldier);
		}

		if (priority > 0.0f) {
			priority *= Compute_Type_Factor(p_netobject);
		}

		if (priority > 0.0f) {
			priority *= Compute_Relevance_Factor(client_id, p_netobject, client_soldier);
		}

		// Add a bit of a curve to have low priority objects drop away faster.
		if (priority > 0.0f && priority < 1.0f) {
			//float bendy = WWMath::Fast_Sin(priority * DEG_TO_RAD(90.0f));
			//priority = (priority + bendy) / 2.0f;
			float bendy = WWMath::Fast_Asin(priority);
			bendy = (RAD_TO_DEGF(bendy)) / 90.0f;
			priority = (priority + bendy) / 2.0f;
		}
	}

	//
	// Override all priority calculations if the client hints that he badly needs an
	// update for this object. The client will hint if there is a vehicle or soldier nearby
	// that hasn't been updated for a suspiciously long time.
	//
	if (p_netobject->Get_Client_Hint_Count(client_id) > 0)
	{
		priority = 1.0f;
		p_netobject->Reset_Client_Hint_Count(client_id);

		/*
		WWDEBUG_SAY(("cPriority::Compute_Object_Priority %5.3f on object %d due to client %d hint.\n",
			priority, p_netobject->Get_Network_ID(), client_id));
		*/
	}

	priority	= WWMath::Clamp(priority, 0.0f, 1.0f);

	return priority;
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Facing_Factor
(
	int							client_id,
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject,
	SoldierGameObj *client_soldier
)
{
	WWASSERT(client_id > 0);
	WWASSERT(p_netobject != nullptr);

	float facing_factor = 1;

	SoldierGameObj * p_soldier = client_soldier;
	if (p_soldier == nullptr) {
		p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(client_id);
	}

	if (p_soldier != nullptr)
	{
		WWASSERT(p_soldier->Peek_Human_Phys() != nullptr);
		float client_facing = p_soldier->Peek_Human_Phys()->Get_Heading();

		Vector3 subject_position;
		if (p_netobject->Get_World_Position(subject_position))
		{
			Vector3 position_delta = subject_position - client_pos;
			float subject_facing = WWMath::Atan2(position_delta.Y, position_delta.X);

			float facing_dif = ::fabs(subject_facing - client_facing);

			if (facing_dif > WWMATH_PI)
			{
				facing_dif = 2 * WWMATH_PI - facing_dif;
			}

			facing_factor -= facing_dif / WWMATH_PI * cUserOptions::MaxFacingPenalty.Get();
		}
	}

	return facing_factor;
}

//-----------------------------------------------------------------------------
float
cPriority::Get_Object_Distance
(
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject
)
{
	WWASSERT(p_netobject != nullptr);

	//
	// Objects without a physical location will return a distance of zero.
	//
	float distance = 0;

	//
	//	Get the object's world position (if it has one)
	//
	Vector3 position;
	if (p_netobject->Get_World_Position(position))
	{
		//
		//	Simple distance calculation based on the distance
		// between points.
		//
		distance = (position - client_pos).Length();
	}

	return distance;
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Type_Factor
(
	NetworkObjectClass *p_netobject
)
{
	float type_factor = 1.0f;

	char type = p_netobject->Get_App_Packet_Type();

	switch (type) {
		case APPPACKETTYPE_SOLDIER:
			type_factor = SOLDIER_FACTOR;
			break;

		case APPPACKETTYPE_TURRET:
			type_factor = TURRET_FACTOR;
			break;

		case APPPACKETTYPE_VEHICLE:
			type_factor = VEHICLE_FACTOR;
			break;

		case APPPACKETTYPE_BUILDING:
			type_factor = BUILDING_FACTOR;
			break;

		default:
			type_factor = 1.0f;
	}

	return(type_factor);
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Relevance_Factor
(
	int						client_id,
	NetworkObjectClass *	p_netobject,
	SoldierGameObj *client_soldier
)
{
	WWASSERT(p_netobject != nullptr);

	//
	// This bumps the priority of objects that you are shooting or that are
	// shooting at you.
	//

	bool is_relevant = false;

	int id = p_netobject->Get_Network_ID();

	SoldierGameObj * p_my_soldier = client_soldier;
	if (p_my_soldier == nullptr) {
		p_my_soldier = GameObjManager::Find_Soldier_Of_Client_ID(client_id);
	}
	VehicleGameObj * p_my_vehicle = nullptr;
	if (p_my_soldier != nullptr) {
		p_my_vehicle = GameObjManager::Find_Vehicle_Occupied_By(p_my_soldier);
	}

	if (p_my_soldier != nullptr &&
		(p_my_soldier->Get_Last_Object_Id_I_Damaged() == id ||
		 p_my_soldier->Get_Last_Object_Id_I_Got_Damaged_By() == id)) {

		is_relevant = true;
	}

	if (p_my_vehicle != nullptr &&
		(p_my_vehicle->Get_Last_Object_Id_I_Damaged() == id ||
		 p_my_vehicle->Get_Last_Object_Id_I_Got_Damaged_By() == id)) {

		is_relevant = true;
	}

	float factor = 1.0f;
	if (!is_relevant) {
		factor -= cUserOptions::IrrelevancePenalty.Get();
	}

	WWASSERT(factor > 0);

	return factor;
}





























float
cPriority::Compute_Object_Priority_2
(
	int							client_id,
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject,
	bool							do_it_anyway,
	SoldierGameObj *			client_soldier
)
{
	WWPROFILE("ObjPri");
	WWASSERT(client_id > 0);
	WWASSERT(p_netobject != nullptr);

	//
	// Compute the priority of this object to the given client at his given position.
	// Priority depends on physical distance. Objects with no physical location will
	// have a priority of 1.
	//
	float priority = 0.0f;

	if (p_netobject->Is_Client_Dirty(client_id) || do_it_anyway)
	{
		float distance = Get_Object_Distance_2(client_pos, p_netobject);

		//
		// Priority simply decreases linearly with distance and is zero at MaxDistance.
		//
		priority = 1 - distance / MaxDistance;

		if (priority > 0.0f) {
			priority *= Compute_Facing_Factor_2(client_id, client_pos, p_netobject, client_soldier);
		}

		if (priority > 0.0f) {
			priority *= Compute_Type_Factor_2(p_netobject, distance);
		}

		if (priority > 0.0f) {
			priority *= Compute_Relevance_Factor_2(client_id, p_netobject, client_soldier);
		}

		// Add a bit of a curve to have low priority objects drop away faster.
		if (priority > 0.0f && priority < 1.0f) {
			//float bendy = WWMath::Fast_Sin(priority * DEG_TO_RAD(90.0f));
			//priority = (priority + bendy) / 2.0f;
			float bendy = WWMath::Fast_Asin(priority);
			bendy = (RAD_TO_DEGF(bendy)) / 90.0f;
			priority = (priority + bendy) / 2.0f;
		}
	}

	priority	= WWMath::Clamp(priority, 0.0f, 1.0f);

	return priority;
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Facing_Factor_2
(
	int							client_id,
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject,
	SoldierGameObj *client_soldier
)
{
	WWASSERT(client_id > 0);
	WWASSERT(p_netobject != nullptr);

	float facing_factor = 1;

	SoldierGameObj * p_soldier = client_soldier;
	if (p_soldier == nullptr) {
		p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(client_id);
	}

	if (p_soldier != nullptr)
	{
		HumanPhysClass *hphys = p_soldier->Peek_Human_Phys();
		WWASSERT(hphys != nullptr);
		float client_facing = hphys->Get_Heading();

		Vector3 subject_position;
		if (p_netobject->Get_World_Position(subject_position))
		{
			Vector3 position_delta = subject_position - client_pos;
			float subject_facing = WWMath::Atan2(position_delta.Y, position_delta.X);

			float facing_dif = ::fabs(subject_facing - client_facing);

			if (facing_dif > WWMATH_PI)
			{
				facing_dif = 2 * WWMATH_PI - facing_dif;
			}

			facing_factor -= facing_dif / WWMATH_PI * cUserOptions::MaxFacingPenalty.Get();
		}
	}

	return facing_factor;
}

//-----------------------------------------------------------------------------
float
cPriority::Get_Object_Distance_2
(
	const Vector3 &			client_pos,
	NetworkObjectClass *		p_netobject
)
{
	WWASSERT(p_netobject != nullptr);

	//
	// Objects without a physical location will return a distance of zero.
	//
	float distance = 0;

	//
	//	Get the object's world position (if it has one)
	//
	Vector3 position;
	if (p_netobject->Get_World_Position(position))
	{
		//
		//	Simple distance calculation based on the distance
		// between points.
		//
		distance = (position - client_pos).Length();
	}

	return distance;
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Type_Factor_2
(
	NetworkObjectClass *p_netobject,
	float distance
)
{
	float type_factor = 1.0f;

	char type = p_netobject->Get_App_Packet_Type();

	switch (type) {
		case APPPACKETTYPE_SOLDIER:
			{
				type_factor = SOLDIER_FACTOR;
				SoldierGameObj *soldier = (SoldierGameObj*) p_netobject;
				if (soldier->Is_In_Vehicle()) {
					type_factor = SOLDIER_IN_VEHICLE_FACTOR;
				}
			}
			break;

		case APPPACKETTYPE_TURRET:
		{
			type_factor = TURRET_FACTOR;
			float range_filter = p_netobject->Get_Filter_Distance();
			if (distance > range_filter) {
				type_factor = 0.0f;
			} else {
				if (distance > 100) {
					type_factor = type_factor / 2.0f;
				}
			}
			break;
		}

		case APPPACKETTYPE_VEHICLE:
			type_factor = VEHICLE_FACTOR;
			break;

		case APPPACKETTYPE_BUILDING:
			type_factor = BUILDING_FACTOR;
			break;

		default:
			type_factor = 1.0f;
	}

	return(type_factor);
}

//-----------------------------------------------------------------------------
float
cPriority::Compute_Relevance_Factor_2
(
	int						client_id,
	NetworkObjectClass *	p_netobject,
	SoldierGameObj *client_soldier
)
{
	WWASSERT(p_netobject != nullptr);

	//
	// This bumps the priority of objects that you are shooting or that are
	// shooting at you.
	//

	bool is_relevant = false;

	int id = p_netobject->Get_Network_ID();

	SoldierGameObj * p_my_soldier = client_soldier;
	VehicleGameObj * p_my_vehicle = nullptr;

	if (p_my_soldier == nullptr) {
		p_my_soldier = GameObjManager::Find_Soldier_Of_Client_ID(client_id);
	}
	if (p_my_soldier != nullptr) {
		if (p_my_soldier->Is_In_Vehicle()) {
			p_my_vehicle = GameObjManager::Find_Vehicle_Occupied_By(p_my_soldier);
		}

		if ((p_my_soldier->Get_Last_Object_Id_I_Damaged() == id ||
		 	p_my_soldier->Get_Last_Object_Id_I_Got_Damaged_By() == id)) {
			is_relevant = true;
		}
	}

	if (p_my_vehicle != nullptr &&
		(p_my_vehicle->Get_Last_Object_Id_I_Damaged() == id ||
		 p_my_vehicle->Get_Last_Object_Id_I_Got_Damaged_By() == id)) {

		is_relevant = true;
	}

	float factor = 1.0f;
	if (!is_relevant) {
		factor -= cUserOptions::IrrelevancePenalty.Get();
	}

	WWASSERT(factor > 0);

	return factor;
}
