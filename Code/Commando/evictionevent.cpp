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
 *                     $Archive:: /Commando/Code/Commando/evictionevent.cpp               $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/10/01 1:02p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "evictionevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"
#include "textdisplay.h"
#include "dialogtests.h"
#include "apppackettypes.h"
#include "messagewindow.h"

// 
// TSS2001 Problem - we have lost the unreliable multiblast effect for this message
//

DECLARE_NETWORKOBJECT_FACTORY(cEvictionEvent, NETCLASSID_EVICTIONEVENT);

//-----------------------------------------------------------------------------
cEvictionEvent::cEvictionEvent(void)
{
	Set_App_Packet_Type(APPPACKETTYPE_EVICTIONEVENT);
}

//-----------------------------------------------------------------------------
void
cEvictionEvent::Init(int client_id, EvictionCodeEnum code)
{
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(client_id >= 0);

	EvictionCode = code;

	Set_Object_Dirty_Bit(client_id, BIT_CREATION, true);

	//
	// This is a transient object. It is created, rendered, and destroyed immediately.
	//
	//Set_Delete_Pending ();
}

//-----------------------------------------------------------------------------
void
cEvictionEvent::Act(void)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

   WideStringClass code_string;
	switch (EvictionCode)
	{
		case EVICTION_POOR_BANDWIDTH:
			code_string = TRANSLATION(IDS_MP_BANDWIDTH_INSUFFICIENT);
			break;

		default:
			DIE;
	}

	WideStringClass widestring;
	widestring.Format(
		U_CHAR("%s: %s"), 
		TRANSLATION(IDS_MP_YOU_ARE_EVICTED), 
		code_string);
   WWASSERT(CombatManager::Get_Message_Window() != NULL);

	//
	//	Display the message...
	//
	CombatManager::Get_Message_Window ()->Add_Message (widestring);
	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cEvictionEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	packet.Add((int) EvictionCode);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cEvictionEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	int eviction_code = packet.Get(eviction_code);
	EvictionCode = (EvictionCodeEnum) eviction_code;

	Act();
}






