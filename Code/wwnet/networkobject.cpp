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
 *                     $Archive:: /Commando/Code/wwnet/networkobject.cpp                      $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 1/07/02 5:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "networkobject.h"
#include "networkobjectmgr.h"
#include "wwmath.h"
#include "vector3.h"
#include "wwprofile.h"
#include "systimer.h"

#define CLIENT_SIDE_UPDATE_FREQUENCY_SAMPLE_PERIOD (1000 * 10)

////////////////////////////////////////////////////////////////
//	Static member initializtaion
////////////////////////////////////////////////////////////////
bool		NetworkObjectClass::IsServer		= false;


////////////////////////////////////////////////////////////////
//
//	NetworkObjectClass
//
////////////////////////////////////////////////////////////////
NetworkObjectClass::NetworkObjectClass (void)	:
	ImportStateCount (0),
	LastClientsideUpdateTime (0),
	NetworkID (0),
	IsDeletePending (false),
	CachedPriority (0),
	UnreliableOverride (false),
	AppPacketType (0),
	FrequentExportPacketSize(0),
	ClientsideUpdateFrequencySampleStartTime(TIMEGETTIME()),
	ClientsideUpdateFrequencySampleCount(0),
	ClientsideUpdateRate(0),
#ifdef WWDEBUG
	CreatedByPacketID(0),
#endif //WWDEBUG
	LastObjectIdIDamaged(-1),
	LastObjectIdIGotDamagedBy(-1)

{
	if (IsServer)
	{
		//
		//	Assign the object a unique ID. This will happen on the client too during object
		// imports, but will be corrected immediately with an explicit Set_Network_ID call.
		//
		int new_id = NetworkObjectMgrClass::Get_New_Dynamic_ID();
		//WWDEBUG_SAY(("New network id = %d\n", new_id));//TSS2001
		Set_Network_ID(new_id);
	}

	//
	//	By default, objects have the modifiction dirty bit set.
	// Static objects therefore don't need to remember to set this in their constructor.
	// Game objects will set BIT_CREATION.
	//
	Clear_Object_Dirty_Bits ();

	memset(CachedPriority_2, 0, sizeof(CachedPriority_2));

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~NetworkObjectClass
//
////////////////////////////////////////////////////////////////
NetworkObjectClass::~NetworkObjectClass (void)
{
	//
	//	Unregister this object from network updates
	//
	NetworkObjectMgrClass::Unregister_Object (this);
	return ;
}

extern bool SensibleUpdates;

////////////////////////////////////////////////////////////////
//
//	Set_Network_ID
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Network_ID (int id)
{
	WWASSERT(id > 0);

	//
	//	Remove the object from the manager, change it's ID,
	// and re-insert it.
	//
	NetworkObjectMgrClass::Unregister_Object (this);
	NetworkID = id;
	NetworkObjectMgrClass::Register_Object (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Object_Dirty_Bits
//
////////////////////////////////////////////////////////////////
BYTE
NetworkObjectClass::Get_Object_Dirty_Bits (int client_id)
{
	return ClientStatus[client_id];
}


////////////////////////////////////////////////////////////////
//
//	Set_Object_Dirty_Bits
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Object_Dirty_Bits (int client_id, BYTE bits)
{
	ClientStatus[client_id] = bits;
}


////////////////////////////////////////////////////////////////
//
//	Get_Object_Dirty_Bit
//
////////////////////////////////////////////////////////////////
bool
NetworkObjectClass::Get_Object_Dirty_Bit (int client_id, DIRTY_BIT dirty_bit)
{
	return ((ClientStatus[client_id] & dirty_bit) == dirty_bit);
}


////////////////////////////////////////////////////////////////
//
//	Clear_Object_Dirty_Bits
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Clear_Object_Dirty_Bits (void)
{
	//
	//	Reset the status for each client
	//
	for (int index = 0; index < MAX_CLIENT_COUNT; index ++) {
		ClientStatus[index] = 0;
		UpdateInfo[index].LastUpdateTime = 0;
		UpdateInfo[index].UpdateRate = 50;
		UpdateInfo[index].ClientHintCount = 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Object_Dirty_Bit
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Object_Dirty_Bit (int client_id, DIRTY_BIT dirty_bit, bool onoff)
{
	if (onoff) {
		ClientStatus[client_id] |= dirty_bit;
	} else {
		ClientStatus[client_id] &= (~dirty_bit);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Object_Dirty_Bit
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Object_Dirty_Bit (DIRTY_BIT dirty_bit, bool onoff)
{
	if (!IsServer)
	{
		return;
	}

	//
	//	Change the status for each client
	// N.B. Client 0 is actually the server.
	//
	for (int index = 1; index < MAX_CLIENT_COUNT; index ++) {//TSS2001

		if (onoff) {
			ClientStatus[index] |= dirty_bit;
		} else {
			ClientStatus[index] &= (~dirty_bit);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Is_Client_Dirty
//
////////////////////////////////////////////////////////////////
bool
NetworkObjectClass::Is_Client_Dirty (int client_id)
{
	return ClientStatus[client_id] != 0;
}


////////////////////////////////////////////////////////////////
//
//	Set_Delete_Pending
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Delete_Pending (void)
{
	IsDeletePending = true;
	NetworkObjectMgrClass::Register_Object_For_Deletion (this);
	return;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Client_Hint_Count
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Reset_Client_Hint_Count(int client_id)
{
	WWASSERT(client_id >= 0 && client_id < MAX_CLIENT_COUNT);

	UpdateInfo[client_id].ClientHintCount = 0;
}


////////////////////////////////////////////////////////////////
//
//	Increment_Client_Hint_Count
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Increment_Client_Hint_Count(int client_id)
{
	WWASSERT(client_id >= 0 && client_id < MAX_CLIENT_COUNT);

	if (UpdateInfo[client_id].ClientHintCount < 255) {
		UpdateInfo[client_id].ClientHintCount++;
	}
}


////////////////////////////////////////////////////////////////
//
//	Hint_To_All_Clients
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Hint_To_All_Clients(void)
{
	//
	// Hint that an update should be sent to all clients
	//
	for (int index = 0; index < MAX_CLIENT_COUNT; index ++) {
		Increment_Client_Hint_Count(index);
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Client_Hint_Count
//
////////////////////////////////////////////////////////////////
BYTE
NetworkObjectClass::Get_Client_Hint_Count(int client_id)
{
	WWASSERT(client_id >= 0 && client_id < MAX_CLIENT_COUNT);

	return UpdateInfo[client_id].ClientHintCount;
}


////////////////////////////////////////////////////////////////
//
//	Belongs_To_Client
//
////////////////////////////////////////////////////////////////
bool
NetworkObjectClass::Belongs_To_Client (int client_id)
{
	WWASSERT(client_id > 0);

	int id_min = NETID_CLIENT_OBJECT_MIN + (client_id - 1) * 100000;
	int id_max = id_min + 100000 - 1;

	return (NetworkID >= id_min) && (NetworkID <= id_max);
}



////////////////////////////////////////////////////////////////
//
//	Get_Last_Update_Time - Returns time that the client last received an update for this object.
//
// In: ID of client
// Out: Last update time.
//
//	10/16/2001 2:45PM ST
////////////////////////////////////////////////////////////////
unsigned int
NetworkObjectClass::Get_Last_Update_Time(int client_id)
{
	// Is this assert right? ST - 10/16/2001 2:44PM
	WWASSERT(client_id > 0 && client_id <= MAX_CLIENT_COUNT);
	return(UpdateInfo[client_id].LastUpdateTime);
}


////////////////////////////////////////////////////////////////
//
//	Get_Update_Rate - Returns delay in ms between updates for this object to the given client
//
// In: ID of client
// Out: Update delay in ms
//
//	10/16/2001 2:45PM ST
////////////////////////////////////////////////////////////////
unsigned short
NetworkObjectClass::Get_Update_Rate(int client_id)
{
	// Is this assert right? ST - 10/16/2001 2:44PM
	WWASSERT(client_id > 0 && client_id <= MAX_CLIENT_COUNT);
	return(UpdateInfo[client_id].UpdateRate);
}


////////////////////////////////////////////////////////////////
//
//	Set_Last_Update_Time - Sets time that the client last received an update for this object.
//
// In: ID of client, time this object was sent to the client.
// Out: Nothing
//
//	10/16/2001 2:45PM ST
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Last_Update_Time(int client_id, unsigned int time)
{
	// Is this assert right? ST - 10/16/2001 2:44PM
	WWASSERT(client_id > 0 && client_id <= MAX_CLIENT_COUNT);
	UpdateInfo[client_id].LastUpdateTime = time;
}


////////////////////////////////////////////////////////////////
//
//	Set_Update_Rate - Sets the ms delay between updates for this client
//
// In: ID of client, delay in ms between updates
// Out: Nothing
//
//	10/16/2001 2:45PM ST
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Update_Rate(int client_id, unsigned short rate)
{
	// Is this assert right? ST - 10/16/2001 2:44PM
	WWASSERT(client_id > 0 && client_id <= MAX_CLIENT_COUNT);
	UpdateInfo[client_id].UpdateRate = rate;
}






















/*
////////////////////////////////////////////////////////////////
//
//	Clear_Object_Dirty_Bits
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Clear_Object_Dirty_Bits (int client_id)
{
	ClientStatus[client_id] = 0;
	return ;
}
*/


//float		NetworkObjectClass::RandomFloat	= 0.0F;
		//TSS2001d float perturbation = RandomFloat * 2 * distance - distance;
/*
////////////////////////////////////////////////////////////////
//
//	Set_Random_Float
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Random_Float (float random_float)
{
	WWASSERT(random_float >= 0 && random_float <= 1);

	RandomFloat = random_float;
}
*/

	//Set_Object_Dirty_Bit (BIT_RARE, true);

//float		NetworkObjectClass::MaxDistance	= 300.0F;

////////////////////////////////////////////////////////////////
//
//	Compute_Object_Priority
//
////////////////////////////////////////////////////////////////
/*
float
NetworkObjectClass::Compute_Object_Priority (int client_id, const Vector3 &client_pos)
{
	//
	// Priority depends on physical distance. Objects with no physical location will
	// have a priority of 1.
	//
	CachedPriority = 0;

	if (Is_Client_Dirty(client_id)) {

		float distance = Get_Object_Distance (client_pos);

		if (distance < MaxDistance)
		{
			//
			// This object is visible to this client.
			// Add a random perturbation in the range [-distance, distance].
			//
			float rand_float = ::rand() / (float) RAND_MAX;
			float perturbation = rand_float * 2 * distance - distance;

			distance += perturbation;
		}

		//
		// Priority simply decreases linearly with distance and is zero at MaxDistance.
		//
		CachedPriority = 1 - distance / MaxDistance;
		CachedPriority	= WWMath::Clamp (CachedPriority, 0.0F, 1.0F);
	}

	return CachedPriority;
}
*/

/*
float
NetworkObjectClass::Compute_Object_Priority (int client_id, const Vector3 &client_pos)
{
	//
	// Compute the priority of this object to the given client at his given position.
	// Priority depends on physical distance. Objects with no physical location will
	// have a priority of 1.
	//
	CachedPriority = 0;

	if (Is_Client_Dirty(client_id)) {

		float distance = Get_Object_Distance (client_pos);

		//
		// Priority simply decreases linearly with distance and is zero at MaxDistance.
		//
		CachedPriority = 1 - distance / MaxDistance;
		CachedPriority	= WWMath::Clamp (CachedPriority, 0.0F, 1.0F);
	}

	return CachedPriority;
}
*/
////////////////////////////////////////////////////////////////
//
//	Set_Cached_Priority
//
////////////////////////////////////////////////////////////////
void
NetworkObjectClass::Set_Cached_Priority (float priority)
{
	WWASSERT(priority >= 0 && priority <= 1);

	CachedPriority = priority;
}




/*
////////////////////////////////////////////////////////////////
//
//	Get_Object_Distance
//
////////////////////////////////////////////////////////////////
float
NetworkObjectClass::Get_Object_Distance (const Vector3 &client_pos)
{
	//
	// Objects without a physical location will return a distance of zero.
	//
	float distance = 0;

	//
	//	Get the object's world position (if it has one)
	//
	Vector3 position;
	if (Get_World_Position (position)) {

		//
		//	Simple distance calculation based on the distance
		// between points.
		//
		distance = (position - client_pos).Length ();
	}

	return distance;
}
*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Last_Clientside_Update_Time -- Reset the time this client object last got an update for the server
//
// In: Nothing.
// Out: Nothing
//
//	10/19/2001 12:19PM ST
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkObjectClass::Reset_Last_Clientside_Update_Time(void)
{
	LastClientsideUpdateTime = 0;
	ClientsideUpdateFrequencySampleStartTime = TIMEGETTIME();
	ClientsideUpdateFrequencySampleCount = 0;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Last_Clientside_Update_Time -- Set the time this client object last got an update for the server
//
// In: Nothing.
// Out: Nothing
//
//	10/19/2001 12:19PM ST
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkObjectClass::Set_Last_Clientside_Update_Time (ULONG time)
{
	LastClientsideUpdateTime = time;
	ClientsideUpdateFrequencySampleCount++;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Clientside_Update_Frequency -- Get the updates rate from the server in ms
//
// In: Nothing.
// Out: Average time between updates from the server in ms
//
//	10/19/2001 12:19PM ST
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NetworkObjectClass::Get_Clientside_Update_Frequency(void)
{
	unsigned int time = TIMEGETTIME();

	if (time - ClientsideUpdateFrequencySampleStartTime > CLIENT_SIDE_UPDATE_FREQUENCY_SAMPLE_PERIOD) {
		// Say 10 seconds if we don't know.
		int rate = 10000;
		if (ClientsideUpdateFrequencySampleCount) {
			rate = (time - ClientsideUpdateFrequencySampleStartTime) / ClientsideUpdateFrequencySampleCount;
			ClientsideUpdateFrequencySampleStartTime = time;
			ClientsideUpdateFrequencySampleCount = 0;
		}
		ClientsideUpdateRate = rate;
	}

	return(ClientsideUpdateRate);
}


#ifdef WWDEBUG
void NetworkObjectClass::Set_Created_By_Packet_ID (int id)
{
	CreatedByPacketID = id;
}
#endif //WWDEBUG