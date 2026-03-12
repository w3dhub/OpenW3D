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
 *                     $Archive:: /Commando/Code/Commando/requestkillevent.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/09/01 10:56a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "requestkillevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "wwdebug.h"
#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"


DECLARE_NETWORKOBJECT_FACTORY(cRequestKillEvent, NETCLASSID_REQUESTKILLEVENT);

//-----------------------------------------------------------------------------
cRequestKillEvent::cRequestKillEvent(void)
{
	ObjectId = 0;

	Set_App_Packet_Type(APPPACKETTYPE_REQUESTKILLEVENT);
}

//-----------------------------------------------------------------------------
void
cRequestKillEvent::Init(int object_id)
{
	WWASSERT(cNetwork::I_Am_Client());

	ObjectId = object_id;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cRequestKillEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	NetworkObjectClass *	p_object = NetworkObjectMgrClass::Find_Object(ObjectId);
	if (p_object != NULL) {
		p_object->Set_Delete_Pending();
		WWDEBUG_SAY(("* cRequestKillEvent::Act: killing object id %d\n", ObjectId));
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cRequestKillEvent::Export_Creation(BitStreamClass & packet)
{
   WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	packet.Add(ObjectId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cRequestKillEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(ObjectId);

	Act();
}
