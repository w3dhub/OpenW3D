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
 *                     $Archive:: /Commando/Code/Combat/scobeliskevent.cpp                 $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 1/04/02 7:23p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scobeliskevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "WWAudio.h"
#include "matrix3d.h"
#include "translatedb.h"
#include "string_ids.h"
#include "apppackettypes.h"
#include "bitpackids.h"
#include "explosion.h"
#include "combat.h"


DECLARE_NETWORKOBJECT_FACTORY(cScObeliskEvent, NETCLASSID_SCOBELISKEVENT);

//-----------------------------------------------------------------------------
cScObeliskEvent::cScObeliskEvent(void)
{
	DefID = 0;
	Position.Set(Vector3(0, 0, 0));
	OwnerID = 0;

	Set_App_Packet_Type(APPPACKETTYPE_SCOBELISKEVENT);
}

//-----------------------------------------------------------------------------
void
cScObeliskEvent::Init(int def_id, const Vector3 & position, int owner_id )
{
	WWASSERT(CombatManager::I_Am_Server());

	DefID		= def_id;
	Position	= position;
	OwnerID 	= owner_id;

	//
	// This skips the local client
	//
	Set_Object_Dirty_Bit(BIT_CREATION, true);

	if (CombatManager::I_Am_Client())
	{
		Act();
	}
}

//-----------------------------------------------------------------------------
void
cScObeliskEvent::Act(void)
{
	WWASSERT(CombatManager::I_Am_Client());

	ExplosionManager::Explode( DefID, Position, OwnerID );

	WWDEBUG_SAY(("cScObeliskEvent::Act: Def %d, at (%5.2f, %5.2f, %5.2f) Owner %d\n", 
		DefID, Position.X, Position.Y, Position.Z, OwnerID ));

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cScObeliskEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Server());

	NetworkObjectClass::Export_Creation(packet);

	packet.Add(DefID);
	packet.Add(Position.X, BITPACK_WORLD_POSITION_X);
	packet.Add(Position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add(Position.Z, BITPACK_WORLD_POSITION_Z);
	packet.Add(OwnerID);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cScObeliskEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Only_Client());

	NetworkObjectClass::Import_Creation(packet);

	packet.Get(DefID);
	packet.Get(Position.X, BITPACK_WORLD_POSITION_X);
	packet.Get(Position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get(Position.Z, BITPACK_WORLD_POSITION_Z);
	packet.Get(OwnerID);

	Act();
}
