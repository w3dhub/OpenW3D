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
 *                     $Archive:: /Commando/Code/Commando/suicideevent.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 12/01/01 1:38p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 9                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "suicideevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"
#include "playermanager.h"
#include "sctextobj.h"
#include "translatedb.h"
#include "string_ids.h"


DECLARE_NETWORKOBJECT_FACTORY(cSuicideEvent, NETCLASSID_SUICIDEEVENT);

//-----------------------------------------------------------------------------
cSuicideEvent::cSuicideEvent(void)
{
	SenderId = 0;

	Set_App_Packet_Type(APPPACKETTYPE_SUICIDEEVENT);
}

//-----------------------------------------------------------------------------
void
cSuicideEvent::Init(void)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId = cNetwork::Get_My_Id();

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cSuicideEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

   WWDEBUG_SAY(("Client %d committed suicide.\n", SenderId));

	SmartGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(SenderId);
	if (p_soldier != NULL)
	{
		p_soldier->Set_Delete_Pending();
	}

	//
	// Increment Deaths
	//
	cPlayer * p_player = cPlayerManager::Find_Player(SenderId);
	if (p_player != NULL) 
	{
		//p_player->Increment_Deaths();
		p_player->Set_Money(0);
	}

	//
	// Tell everyone
	//
	WideStringClass text;
	//text.Format(U_CHAR("_%s_committed_suicide_"), p_player->Get_Name());
	text.Format(U_CHAR("%s %s"), p_player->Get_Name(), TRANSLATE(IDS_MP_COMMITTED_SUICIDE));
	cScTextObj * p_message = new cScTextObj;
	p_message->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);


	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cSuicideEvent::Export_Creation(BitStreamClass & packet)
{
   WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cSuicideEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);

	WWASSERT(SenderId > 0);

	Act();
}
