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
 *                     $Archive:: /Commando/Code/Commando/purchaseresponseevent.cpp               $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 12/20/01 2:57p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 10                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "purchaseresponseevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "consolefunction.h"
#include "vendor.h"
#include "translatedb.h"
#include "string_ids.h"
#include "textdisplay.h"
#include "apppackettypes.h"
#include "messagewindow.h"
#include "WWAudio.h"


DECLARE_NETWORKOBJECT_FACTORY(cPurchaseResponseEvent, NETCLASSID_PURCHASERESPONSEEVENT);

//-----------------------------------------------------------------------------
cPurchaseResponseEvent::cPurchaseResponseEvent(void)
{
	PurchaserId = -1;
	ResponseId = -1;

	Set_App_Packet_Type(APPPACKETTYPE_PURCHASERESPONSEEVENT);
}

//-----------------------------------------------------------------------------
void
cPurchaseResponseEvent::Init(int response_id, int client_id)
{
	WWASSERT(cNetwork::I_Am_Server());

	PurchaserId	= client_id;
	ResponseId = response_id;

	if (cNetwork::I_Am_Client() && PurchaserId == cNetwork::Get_My_Id())
	{
		Act();
	}
	else
	{
		Set_Object_Dirty_Bit(client_id, BIT_CREATION, true);
	}
}

//-----------------------------------------------------------------------------
void
cPurchaseResponseEvent::Act(void)
{
	if (!GameModeManager::Find("Combat")->Is_Active()) {
		return;
	}

   WWASSERT(cNetwork::I_Am_Client());

	/*
	if (PurchaserId != cNetwork::Get_My_Id()) {
		return;
	}
	*/
   WWASSERT(PurchaserId == cNetwork::Get_My_Id());

	WideStringClass wide_string;

	if ( ResponseId == VendorClass::PERR_SUCCESS ) {
		wide_string.Format( U_CHAR("%s\n"), TRANSLATION(IDS_MP_CNC_PURCHASE_GRANTED) );
		//
		// Play a custom SFX for feedback
		//
		WWASSERT(WWAudioClass::Get_Instance() != NULL);
		WWAudioClass::Get_Instance()->Create_Instant_Sound("Purchase_Granted", Matrix3D(1));
	} else if ( ResponseId == VendorClass::PERR_NO_FUNDS ) {
		wide_string.Format( U_CHAR("%s\n"), TRANSLATION(IDS_MP_CNC_INSUFFICIENT_FUNDS) );
	} else if ( ResponseId == VendorClass::PERR_NO_FACTORY ) {
		wide_string.Format( U_CHAR("%s\n"), TRANSLATION(IDS_MP_CNC_FACTORY_UNAVAILABLE) );
	} else if ( ResponseId == VendorClass::PERR_OPERATION_PENDING ) {
		wide_string.Format( U_CHAR("%s\n"), TRANSLATION(IDS_MP_CNC_TRANSACTION_PENDING) );
	} else if ( ResponseId == VendorClass::PERR_NOT_IN_STOCK ) {			
		wide_string.Format( U_CHAR("%s\n"), TRANSLATION(IDS_MP_CNC_NOT_IN_STOCK) );
	}

	//
	//	Display the message...
	//
	CombatManager::Get_Message_Window ()->Add_Message (wide_string, Vector3 (0.7F, 0.7F, 0.7F));		
	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPurchaseResponseEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	packet.Add(PurchaserId);
	packet.Add(ResponseId);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cPurchaseResponseEvent::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Import_Creation(packet);

	packet.Get(PurchaserId);
	packet.Get(ResponseId);

	Act();
}
