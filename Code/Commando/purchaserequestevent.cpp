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
 *                     $Archive:: /Commando/Code/Commando/purchaserequestevent.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Steve_t                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 8/14/02 11:57a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "purchaserequestevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "cnetwork.h"
#include "networkobjectmgr.h"
#include "gameobjmanager.h"
#include "vendor.h"
#include "playertype.h"
#include "purchaseresponseevent.h"
#include "apppackettypes.h"
#include "ConsoleMode.h"


DECLARE_NETWORKOBJECT_FACTORY(cPurchaseRequestEvent, NETCLASSID_PURCHASEREQUESTEVENT);

//-----------------------------------------------------------------------------
cPurchaseRequestEvent::cPurchaseRequestEvent(void)
{
	SenderId			= 0;
	PurchaseType	= VendorClass::TYPE_SUPPLY;
	ItemIndex		= 0;
	AltSkinIndex	= -1;

	Set_App_Packet_Type(APPPACKETTYPE_PURCHASEREQUESTEVENT);
}

//-----------------------------------------------------------------------------
void
cPurchaseRequestEvent::Init(VendorClass::PURCHASE_TYPE type, int item_index, int alt_skin_index)
{
	WWASSERT(cNetwork::I_Am_Client());

	SenderId			= cNetwork::Get_My_Id();
	PurchaseType	= type;
	ItemIndex		= item_index;
	AltSkinIndex	= alt_skin_index;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	if (cNetwork::I_Am_Server()) {
		Act();
	} else {
		Set_Object_Dirty_Bit(0, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cPurchaseRequestEvent::Act(void)
{
   WWASSERT(cNetwork::I_Am_Server());

	//
	//	Lookup the data needed to make the purchase
	//
	SoldierGameObj *player = GameObjManager::Find_Soldier_Of_Client_ID(SenderId);

	VendorClass::PURCHASE_ERROR result = VendorClass::PERR_UNKNOWN;

	WWASSERT(The_Game() != NULL);
	if (The_Game()->Is_Gameplay_Permitted()) 
	{
		//
		//	Attempt to make the purchase
		//
		result = VendorClass::Purchase_Item (player, (VendorClass::PURCHASE_TYPE)PurchaseType, ItemIndex, AltSkinIndex, false);
	} 
	else 
	{
		result = VendorClass::PERR_NO_FACTORY;
	}
	
	//
	// Show the server admin who bought the vehicle.
	//
	if (cNetwork::I_Am_Only_Server()) {
		if (result == VendorClass::PERR_SUCCESS && (VendorClass::PURCHASE_TYPE)PurchaseType == VendorClass::TYPE_VEHICLE) {
			cPlayer* player = cPlayerManager::Find_Player(SenderId);
			StringClass short_name(true);
			player->Get_Name().Convert_To(short_name);
			ConsoleBox.Print_Maybe("%s purchased a vehicle\n", short_name.Peek_Buffer());
		}	
	}

	//
	//	Now send the response to the client
	//
	cPurchaseResponseEvent * p_event = new cPurchaseResponseEvent;
	p_event->Init((int) result, SenderId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPurchaseRequestEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	cNetEvent::Export_Creation(packet);

	WWASSERT(SenderId > 0);

	packet.Add(SenderId);
	packet.Add(PurchaseType);
	packet.Add(ItemIndex);
	packet.Add(AltSkinIndex);	

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPurchaseRequestEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(SenderId);
	packet.Get(PurchaseType);
	packet.Get(ItemIndex);
	packet.Get(AltSkinIndex);	

	WWASSERT(SenderId > 0);

	Act();
}