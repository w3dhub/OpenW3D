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
 *                     $Archive:: /Commando/Code/Commando/csconsolecommandevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/11/01 1:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "csconsolecommandevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "consolefunction.h"
#include "apppackettypes.h"
#include "networkobjectmgr.h"


DECLARE_NETWORKOBJECT_FACTORY(cCsConsoleCommandEvent, NETCLASSID_CSCONSOLECOMMANDEVENT);

//-----------------------------------------------------------------------------
cCsConsoleCommandEvent::cCsConsoleCommandEvent(void)
{
	::strcpy(Command, "");

	Set_App_Packet_Type(APPPACKETTYPE_CSCONSOLECOMMANDEVENT);
}

//-----------------------------------------------------------------------------
void
cCsConsoleCommandEvent::Init(LPCSTR command)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	WWASSERT(command != NULL);
   WWASSERT(::strlen(command) > 0);
   WWASSERT(::strlen(command) < sizeof(Command));

	::strcpy(Command, command);

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	Set_Object_Dirty_Bit(0, BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
cCsConsoleCommandEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	if (GameModeManager::Find("Combat")->Is_Active()) {
		ConsoleFunctionManager::Parse_Input(Command);
	}
}

//-----------------------------------------------------------------------------
void
cCsConsoleCommandEvent::Export_Creation(BitStreamClass & packet)
{
   WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);

	packet.Add_Terminated_String(Command);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cCsConsoleCommandEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);

	packet.Get_Terminated_String(Command, sizeof(Command));

	Act();

	Set_Delete_Pending();
}
