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
 *                     $Archive:: /Commando/Code/Commando/clienthintmanager.cpp                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/09/01 6:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clienthintmanager.h"

#include "cshint.h"
#include "cnetwork.h"
#include "combat.h"
#include "gameobjmanager.h"
#include "pscene.h"
#include "apppackettypes.h"
#include "vistable.h"
#include "useroptions.h"
#include "priority.h"
#include "apppacketstats.h"

//
// Class statics
//


//-----------------------------------------------------------------------------
void
cClientHintManager::Think
(
	void
)
{
	if (cNetwork::I_Am_Server() ||
		COMBAT_SCENE == NULL ||
		cUserOptions::ClientHintFactor.Get() < 1)
	{
		//
		// Bail:
		// Only dedicated clients send hints.
		// Also, ClientHintFactor is disabled by using any value < 1.
		//
		return;
	}

	WWASSERT(cNetwork::I_Am_Only_Client());

   SoldierGameObj * p_my_soldier = GameObjManager::Find_Soldier_Of_Client_ID(cNetwork::Get_My_Id());

	if (p_my_soldier == NULL)
	{
		//
		// Bail...
		//
		return;
	}

	//
	// Do not allow hints to go out at too high a frequency.
	//
	static DWORD last_hint_time_ms = 0;
	DWORD	time_now_ms	= TIMEGETTIME();
	const DWORD MIN_HINT_DELAY_MS = 1000;
	if (time_now_ms - last_hint_time_ms < MIN_HINT_DELAY_MS)
	{
		//
		// Bail...
		//
		return;
	}

	//
	// Examine all the vis-visible soldiers and vehicles. If we find one that hasn't been
	// updated for a suspiciously long time, send a hint to the server, He'll follow
	// up with an update.
	//

	Vector3 my_position;
	p_my_soldier->Get_Position(&my_position);
	my_position.Z += 1.5;

	VisTableClass * pvs = COMBAT_SCENE->Get_Vis_Table(my_position);

	int		num_objects					= 0;
	//ULONG		total_delay_ms				= 0;
	//ULONG		maximum_delay_ms			= 0;
	//int		longest_delayed_index	= -1;

	int count = NetworkObjectMgrClass::Get_Object_Count();

	NetworkObjectClass **object_list = (NetworkObjectClass **) _alloca((count * sizeof(NetworkObjectClass*)) + 128);
	WWASSERT(object_list != NULL);
	SmartGameObj * player_ptr = GameObjManager::Find_Soldier_Of_Client_ID(cNetwork::Get_My_Id());
	WWASSERT(player_ptr != NULL);

	//
	// Traverse the vehicles and soldiers and gather data about update rates.
	//
	for (int index = 0; index < count; index ++)
	{
		NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(index);
		WWASSERT(p_object != NULL);

		BYTE type = p_object->Get_App_Packet_Type();

		if (type == APPPACKETTYPE_SOLDIER || type == APPPACKETTYPE_VEHICLE)
		{

			if (p_object->Get_Clientside_Update_Frequency()) {

				int vis_id = p_object->Get_Vis_ID();

				//
				// If there is no vis data, proceed.
				// If vis data does exist then only proceed with this object if it is vis-visible.
				//

				if (pvs == NULL || vis_id == -1 || pvs->Get_Bit(vis_id))
				{
					// Just add the object to the list on this pass.
					object_list[num_objects++] = p_object;

					// Work out the priority of this object from the clients perspective.
					if (player_ptr) {
						Vector3 position;
						player_ptr->Get_Position(&position);
						float priority = cPriority::Compute_Object_Priority(cNetwork::Get_My_Id(), position, p_object, true);
						p_object->Set_Cached_Priority(priority);
					}

#if (0)
					ULONG delay_ms = time_now_ms - p_object->Get_Last_Clientside_Update_Time();

					total_delay_ms += delay_ms;
					num_objects++;

					if (delay_ms > maximum_delay_ms)
					{
						maximum_delay_ms = delay_ms;
						longest_delayed_index = index;
					}
#endif //(0)
				}
			}
		}
	}

	if (num_objects < 2) {
		return;
	}

	//
	// Sort the object list. Lowest priority first.
	//
	qsort(object_list, num_objects, sizeof(unsigned int), (int (__cdecl *)(const void *,const void *)) &Priority_Compare);


	//
	// Look for an object that has a higher priority than it's neighbor but is getting updated much less frequently
	//
	// Ignore the worst priority object. This might have to change....?
	//
	int most_broken_object_index = -1;
	int worst_percentage = 0;
	unsigned int time = TIMEGETTIME();
	for (int i=1 ; i<num_objects ; i++) {
		int higher_priority_rate = object_list[i]->Get_Clientside_Update_Frequency();

		//
		// Don't hint for objects with a recent updates. This should prevent us from hinting about objects we just hinted about.
		//
		if (time - object_list[i]->Get_Last_Clientside_Update_Time() > 1500) {

			int lower_priority_rate = object_list[i-1]->Get_Clientside_Update_Frequency();
			if (lower_priority_rate && higher_priority_rate) {

				// Lower priority rate should be higher in terms of milliseconds.
				if (lower_priority_rate < higher_priority_rate) {

					// The higher priority object is being updated by the server less frequently. That doesn't seem right.
					int percentage = (higher_priority_rate * 100) / lower_priority_rate;
					if (percentage > worst_percentage) {
						worst_percentage = percentage;
						most_broken_object_index = i;
					}
				}
			}
		}
	}


	//
	// Calculate the average delay for this set of soldiers and vehicles
	//
	//float average_delay_ms = 0;
	//if (num_objects > 0)
	//{
	//	average_delay_ms = total_delay_ms / (float) num_objects;
	//}

	//
	// If the most OOD object is much more OOD than the average object, then request an update.
	//
	//if ((longest_delayed_index != -1) &&
	//	 (maximum_delay_ms > cUserOptions::ClientHintFactor.Get() * average_delay_ms))
	//{
	float hint_factor = cUserOptions::ClientHintFactor.Get();
	if (most_broken_object_index != -1 && worst_percentage > 100 + (10.0f * hint_factor)) {
		//NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(longest_delayed_index);
		NetworkObjectClass * p_object = object_list[most_broken_object_index];
		WWASSERT(p_object != NULL);

		cCsHint * p_hint = new cCsHint;
		p_hint->Init(p_object->Get_Network_ID());

		last_hint_time_ms = time_now_ms;

		//WWDEBUG_SAY(("cClientHintManager::Think, requesting hint for object id %d, avg = %5.2f, max = %d\n",
		//	p_object->Get_Network_ID(), average_delay_ms, maximum_delay_ms));
		WWDEBUG_SAY(("cClientHintManager::Think, requesting hint for object id %d (%s), priority = %5.2f, ave update rate = %dms\n",
			p_object->Get_Network_ID(),
			cAppPacketStats::Interpret_Type(p_object->Get_App_Packet_Type()),
			p_object->Get_Cached_Priority(),
			p_object->Get_Clientside_Update_Frequency()));
		WWDEBUG_SAY(("      Compared to object id %d with priority %5.2f, avg update rate %dms\n",
			object_list[most_broken_object_index-1]->Get_Network_ID(),
			object_list[most_broken_object_index-1]->Get_Cached_Priority(),
			object_list[most_broken_object_index-1]->Get_Clientside_Update_Frequency()));
		WWDEBUG_SAY(("      Client hint factor = %5.2f\n", hint_factor));
	}
}










//
// Qsort compare function for array of object pointers.
//
int __cdecl cClientHintManager::Priority_Compare(const void **object1, const void **object2)
{
	WWASSERT(object1 != NULL);
	WWASSERT(object2 != NULL);

	NetworkObjectClass *n1 = (NetworkObjectClass*) *object1;
	NetworkObjectClass *n2 = (NetworkObjectClass*) *object2;

	float p1 = n1->Get_Cached_Priority();
	float p2 = n2->Get_Cached_Priority();

	if (p1 == p2) {
		return(0);
	}

	if (p1 < p2) {
		return(-1);
	}

	return(1);
}