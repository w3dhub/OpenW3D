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
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/unitcoordinationzonemgr.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/15/01 3:55p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "unitcoordinationzonemgr.h"
#include "transition.h"
#include "obbox.h"
#include "aabox.h"
#include "combat.h"
#include "pscene.h"
#include "staticanimphys.h"
#include "elevator.h"
#include "colmathaabox.h" // Agressive inlining causes linker issues if this isn't here.


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
DynamicVectorClass<AABoxClass>	UnitCoordinationZoneMgr::ZoneList;


//////////////////////////////////////////////////////////////////////
//
//	Build_Zones
//
//////////////////////////////////////////////////////////////////////
void
UnitCoordinationZoneMgr::Build_Zones (void)
{
	Reset ();

	//
	//	Build the zone lists for both ladder and elevators
	//
	Detect_Ladder_Zones ();
	Detect_Elevator_Zones ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Reset
//
//////////////////////////////////////////////////////////////////////
void
UnitCoordinationZoneMgr::Reset (void)
{
	ZoneList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Detect_Ladder_Zones
//
//////////////////////////////////////////////////////////////////////
void
UnitCoordinationZoneMgr::Detect_Ladder_Zones (void)
{
	//
	//	Build the list of transitions
	//
	DynamicVectorClass<TransitionInstanceClass *> transition_list;
	TransitionManager::Build_Ladder_List (transition_list);

	//
	//	Now, add a zone around the entrance and exit for each transition
	//
	for (int index = 0; index < transition_list.Count (); index ++) {
		TransitionInstanceClass *transition = transition_list[index];

		//
		//	Convert the obbox to an aabox
		//
		const OBBoxClass &zone_box = transition->Get_Zone ();
		AABoxClass zone_box_as_aabox;
		zone_box_as_aabox.Center = zone_box.Center;
		zone_box.Compute_Axis_Aligned_Extent (&zone_box_as_aabox.Extent);

		//
		//	Start a new zone centered about the ending transform for the ladder and
		// including the trigger zone
		//
		AABoxClass new_zone;
		new_zone.Center.Set (transition->Get_Ending_TM ().Get_Translation ());
		new_zone.Add_Box (zone_box_as_aabox);

		//
		//	Add this zone to our list
		//
		ZoneList.Add (new_zone);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Detect_Elevator_Zones
//
//////////////////////////////////////////////////////////////////////
void
UnitCoordinationZoneMgr::Detect_Elevator_Zones (void)
{
	RefPhysListIterator iterator = COMBAT_SCENE->Get_Static_Anim_Object_Iterator ();

	//
	//	Loop over all the static anim objects in the world
	//
	for (iterator.First (); iterator.Is_Done () == false; iterator.Next ()) {	
		StaticAnimPhysClass *phys_obj = (StaticAnimPhysClass *)iterator.Peek_Obj ();
		if (phys_obj != NULL && phys_obj->As_ElevatorPhysClass () != NULL) {

			//
			//	Dig out the definition for this elevator
			//
			ElevatorPhysClass *elevator					= phys_obj->As_ElevatorPhysClass ();
			const ElevatorPhysDefClass *definition		= elevator->Get_ElevatorPhysDef ();

			//
			//	Add a coordination zone for each of our trigger zones
			//
			for (int index = 0; index < ZONE_MAX; index ++) {
				const OBBoxClass &obj_space_zone = definition->Get_Zone ((ELEVATOR_ZONE)index);

				OBBoxClass world_space_zone;
				OBBoxClass::Transform (elevator->Get_Transform (), obj_space_zone, &world_space_zone);

				//
				//	Convert the obbox to an aabox
				//
				AABoxClass zone_box_as_aabox;
				zone_box_as_aabox.Center = world_space_zone.Center;
				world_space_zone.Compute_Axis_Aligned_Extent (&zone_box_as_aabox.Extent);

				//
				//	Add this box to our list
				//
				ZoneList.Add (zone_box_as_aabox);
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Is_Unit_In_Zone
//
//////////////////////////////////////////////////////////////////////
bool
UnitCoordinationZoneMgr::Is_Unit_In_Zone (const Vector3 &pos)
{
	bool retval = false;

	//
	//	Simply check each zone to see if the point lies inside the box
	//
	for (int index = 0; !retval && index < ZoneList.Count (); index ++) {
		retval = ZoneList[index].Contains (pos);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Debug_Boxes
//
//////////////////////////////////////////////////////////////////////
void
UnitCoordinationZoneMgr::Display_Debug_Boxes (void)
{
	//
	//	Pretty simple, just add a debug box for each zone
	//
	for (int index = 0; index < ZoneList.Count (); index ++) {	
		PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (ZoneList[index], Vector3 (1.0F, 0.0F, 0.25F));
	}

	return ;
}

