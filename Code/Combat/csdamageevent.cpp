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
 *                     $Archive:: /Commando/Code/Combat/csdamageevent.cpp                      $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 10:36a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "csdamageevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "combat.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"
#include "damage.h"
#include "armedgameobj.h"
#include "debug.h"

DECLARE_NETWORKOBJECT_FACTORY(cCsDamageEvent, NETCLASSID_CSDAMAGEEVENT);

//
// Class statics
//
bool	cCsDamageEvent::AreClientsTrusted	= true;

//-----------------------------------------------------------------------------
cCsDamageEvent::cCsDamageEvent(void)
{
	SenderId		= 0;
	DamagerGOID	= 0;
	DamageeGOID = 0;
	Damage		= 0;
	Warhead		= 0;

	Set_App_Packet_Type(APPPACKETTYPE_CSDAMAGEEVENT);

	Set_Unreliable_Override(true);
}

//-----------------------------------------------------------------------------
void
cCsDamageEvent::Init
(
	int	damager_go_id,
	int	damagee_go_id,
	float	damage,
	int	warhead
)
{
	WWASSERT(CombatManager::I_Am_Only_Client());

	WWASSERT(AreClientsTrusted);

	SenderId			= CombatManager::Get_My_Id();
	DamagerGOID		= damager_go_id;
	DamageeGOID		= damagee_go_id;
	Damage			= damage;
	Warhead			= warhead;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	Set_Object_Dirty_Bit(0, BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
cCsDamageEvent::Act
(
	void
)
{
   WWASSERT(CombatManager::I_Am_Server());

	if (AreClientsTrusted)
	{
		// process the member data here. Include sanity checks.
		// Find the gameobj with the damagee id
		PhysicalGameObj * obj = GameObjManager::Find_PhysicalGameObj( DamageeGOID );
		if ( obj ) {
			// Make an offense object
			PhysicalGameObj * damager = GameObjManager::Find_PhysicalGameObj( DamagerGOID );
			if ( damager != NULL && damager->As_ArmedGameObj() != NULL ) {
				OffenseObjectClass offense( Damage, Warhead, damager->As_ArmedGameObj());

//				obj->Get_Defense_Object()->Do_Damage( offense );
				// We need to use apply damage extended in order to allow things to die.
				offense.ForceServerDamage = true;

#if 0
				// guess at the damage direction
				Vector3 direction;
				Vector3 pos;
				damager->Get_Position( &pos );
				obj->Get_Position( &direction );
				direction -= pos;
				direction.Normalize();

				obj->Apply_Damage_Extended( offense, 1.0f, direction );
#else
				obj->Apply_Damage_Extended( offense );
#endif

//				Debug_Say(( "Applying Client damage of %f from %d to %d\n", Damage, DamagerGOID, DamageeGOID ));
			} else {
//				Debug_Say(( "Error: Client damage Damagee %d not found\n", DamageeGOID ));
			}
		} else {
//			Debug_Say(( "Error: Client damage Damagee %d not found\n", DamageeGOID ));
		}
	} else {
		Debug_Say(( "Error: Receiving Client damage when clients are not trusted\n" ));
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsDamageEvent::Export_Creation
(
	BitStreamClass & packet
)
{
   WWASSERT(CombatManager::I_Am_Only_Client());

	NetworkObjectClass::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add(DamagerGOID);
	packet.Add(DamageeGOID);
	packet.Add(Damage);
	packet.Add(Warhead);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsDamageEvent::Import_Creation
(
	BitStreamClass & packet
)
{
	WWASSERT(CombatManager::I_Am_Server());

	NetworkObjectClass::Import_Creation(packet);

	packet.Get(SenderId);
	packet.Get(DamagerGOID);
	packet.Get(DamageeGOID);
	packet.Get(Damage);
	packet.Get(Warhead);

	WWASSERT(SenderId > 0);

	Act();
}
