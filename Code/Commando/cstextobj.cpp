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
 *                     $Archive:: /Commando/Code/commando/cstextobj.cpp                    $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/10/02 11:13a                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cstextobj.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "apppackettypes.h"
#include "floodprotectionmgr.h"


DECLARE_NETWORKOBJECT_FACTORY(cCsTextObj, NETCLASSID_CSTEXTOBJ);

//-----------------------------------------------------------------------------
cCsTextObj::cCsTextObj(void)
{
	SenderId		= HOST_TEXT_SENDER;
	Type			= TEXT_MESSAGE_PUBLIC;
	Recipient	= HOST_TEXT_SENDER;

	Set_App_Packet_Type(APPPACKETTYPE_CSTEXTOBJ);
}

//-----------------------------------------------------------------------------
void
cCsTextObj::Init(WideStringClass & text, TextMessageEnum type, int sender_id, int recipient)
{
	WWASSERT(sender_id >= 0);

	WWASSERT(cNetwork::I_Am_Client());

	/*
	if (type == TEXT_MESSAGE_PRIVATE) {
		WWASSERT(recipient != -1);
	}
	*/

	Text			= text;
	Type			= type;
	SenderId		= sender_id;
	Recipient	= recipient;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	//
	//	Is this user "flooding" the server with text?
	//
	if (FloodProtectionMgrClass::Detect_Flooding (text) == false) {

		//
		//	Not flooding, so proceed as normal
		//
		if (cNetwork::I_Am_Server()) {
			Act();
		} else {
			Set_Object_Dirty_Bit(0, BIT_CREATION, true);
		}

	} else {

		//
		//	Flooding detected -- don't send the message
		//
		Set_Delete_Pending();
	}

	return ;
}

//-----------------------------------------------------------------------------
void
cCsTextObj::Act(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	if (GameModeManager::Find("Combat")->Is_Active()) {
		cScTextObj * p_test_obj = new cScTextObj;
		p_test_obj->Init(Text, Type, false, SenderId, Recipient);
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsTextObj::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	packet.Add(SenderId);
	packet.Add((BYTE) Type);
	packet.Add_Wide_Terminated_String(Text);
	packet.Add(Recipient);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsTextObj::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get(SenderId);
	BYTE type = packet.Get(type);
	Type = (TextMessageEnum) type;
	packet.Get_Wide_Terminated_String(Text.Get_Buffer(256), 256);
	packet.Get(Recipient);

	Act();
}
