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
 *                     $Archive:: /Commando/Code/Commando/donateevent.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 12/06/01 5:13p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "donateevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "playermanager.h"
#include "apppackettypes.h"
#include "sctextobj.h"
#include "translatedb.h"
#include "string_ids.h"

//
// Class statics
//
const int	cDonateEvent::MinimumAcceptableDonation = 100;

DECLARE_NETWORKOBJECT_FACTORY(cDonateEvent, NETCLASSID_DONATEEVENT);

//-----------------------------------------------------------------------------
cDonateEvent::cDonateEvent(void)
{
	SenderId		= 0;
	Amount		= 0;
	RecipientId	= 0;

	Set_App_Packet_Type(APPPACKETTYPE_DONATEEVENT);
}

//-----------------------------------------------------------------------------
void
cDonateEvent::Init(int amount, int recipient_id)
{
	WWASSERT(cNetwork::I_Am_Client());

	WWASSERT(amount >= MinimumAcceptableDonation);

	SenderId		= cNetwork::Get_My_Id();
	Amount		= amount;
	RecipientId	= recipient_id;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cDonateEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_donor = cPlayerManager::Find_Player(SenderId);
	cPlayer * p_recipient = cPlayerManager::Find_Player(RecipientId);

	if (p_donor != NULL && 
		 p_recipient != NULL &&
		 Amount >= cDonateEvent::Get_Minimum_Acceptable_Donation() &&
		 (p_donor->Get_Player_Type() == p_recipient->Get_Player_Type()) &&
		 p_donor->Get_Money() >= Amount) {

		p_donor->Increment_Money(-Amount);
		p_recipient->Increment_Money(Amount);

		//
		// Notify recipient
		//
		WideStringClass text;

		text.Format(U_CHAR("%s (%d %s, %s)."), 
			TRANSLATE(IDS_MP_DONATION_RECEIVED),
			Amount, 
			TRANSLATE(IDS_MP_MONEY),
			p_donor->Get_Name());

		cScTextObj * p_message = new cScTextObj;
		p_message->Init(text, TEXT_MESSAGE_PRIVATE, false, HOST_TEXT_SENDER, 
			p_recipient->Get_Id());
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cDonateEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);
	WWASSERT(RecipientId > 0);

	packet.Add(SenderId);
	packet.Add(Amount);
	packet.Add(RecipientId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cDonateEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get(Amount);
	packet.Get(RecipientId);

	WWASSERT(SenderId > 0);
	WWASSERT(RecipientId > 0);

	Act();
}





