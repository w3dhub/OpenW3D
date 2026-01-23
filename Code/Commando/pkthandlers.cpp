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
 *                     $Archive:: /Commando/Code/Commando/pkthandlers.cpp                      $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/18/02 11:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 53                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cnetwork.h"

#include "debug.h"
#include "networkobjectmgr.h"
#include "networkobjectfactory.h"
#include "networkobjectfactorymgr.h"
#include "playermanager.h"
#include "apppacketstats.h"
#include "specialbuilds.h"


extern char * Addr_As_String(sockaddr_in *addr);
extern bool g_is_loading;

//-----------------------------------------------------------------------------
NetworkObjectClass *
Create_Network_Object (cPacket &packet, int class_id, int network_obj_id)
{
	WWASSERT(network_obj_id > 0);

	//
	//	Lookup the factory for this type of network object
	//
	NetworkObjectFactoryClass *factory = NetworkObjectFactoryMgrClass::Find_Factory (class_id);
	WWASSERT (factory != NULL);

	//
	//	Create the new object
	//
	NetworkObjectClass *new_object = factory->Create (packet);
	WWASSERT (new_object != NULL);

	new_object->Set_Network_ID(network_obj_id);//TSS2001b

	return new_object;
}

//-----------------------------------------------------------------------------
void cNetwork::Server_Packet_Handler(cPacket & packet, int rhost_id)
{
#ifndef BETACLIENT

	WWASSERT(I_Am_Server());
	WWASSERT(
		rhost_id >= PServerConnection->Get_Min_RHost() &&
		rhost_id <= PServerConnection->Get_Max_RHost());
	WWASSERT(Receiver != NULL);

	/*TSS082801
	// TSS - hack
	if (g_is_loading) {
		Debug_Say(("Server flushing packet during loading\n"));
		packet.Flush();
		return;
	}
	*/

	//
	// TSS112401 XXX
	//
	if (PServerConnection->Get_Remote_Host(rhost_id) == NULL) {
		//
		// We were getting this crash:
		// pkthandlers.cpp (95) - received BIT_CREATION for existing object of type
		// APPPACKETTYPE_CSDAMAGEEVENT following messages "Insufficient bandwidth to send all guaranteed packets".
		//
		// The only scenario I can envisage is that the server destroys a client, but
		// some damage packets are on the wire. Then a new client joins, gets the same client_id,
		// and sends some damage packets with the same id's.
		//
		WWDEBUG_SAY(("cNetwork::Server_Packet_Handler: flushing packet from invalid rhost_id %d\n",
			rhost_id));

		packet.Flush();
		return;
	}

	int network_obj_id		= packet.Get (network_obj_id);
	BYTE dirty_bits			= packet.Get (dirty_bits);
	bool is_delete_pending	= packet.Get (is_delete_pending);

	//
	//	Lookup the object this data belongs to
	//
	NetworkObjectClass *object = NetworkObjectMgrClass::Find_Object (network_obj_id);

	//
	//	Do we need to create this object?
	//
	if ((dirty_bits & NetworkObjectClass::BIT_CREATION) == NetworkObjectClass::BIT_CREATION) {

		int net_classid = packet.Get (net_classid);

		//WWASSERT (object == NULL);
		if (object != NULL)
		{
#ifdef WWDEBUG
			//sockaddr_in *actual_from_addr_ptr = (struct sockaddr_in*) &packet.Get_From_Address_Wrapper()->FromAddress;
			//sockaddr_in rhost_addr = PServerConnection->Get_Remote_Host(rhost_id)->Get_Address();
#endif //WWDEBUG

//// FIXME (TSS) APPPACKETTYPE_CSDAMAGEEVENT workaround for unknown crash bug.\n

			//WWDEBUG_SAY(("\n"));
			//WWDEBUG_SAY(("cNetwork::Server_Packet_Handler: received BIT_CREATION (id %d, is_delete_pending = %d, net_classid = %d, from rhost %d - %s) for existing object:\n",
			//	network_obj_id, is_delete_pending, net_classid, rhost_id, Addr_As_String(&rhost_addr)));
			//WWDEBUG_SAY(("  id                 = %d\n", object->Get_Network_ID()));
			//WWDEBUG_SAY(("  AppPacketType      = %s\n", cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type())));
			//WWDEBUG_SAY(("  IsDeletePending    = %d\n", object->Is_Delete_Pending()));
			//WWDEBUG_SAY(("  CreatedByPacketID  = %d\n", object->Get_Created_By_Packet_ID()));
			//WWDEBUG_SAY(("Packet actually from address %s\n", Addr_As_String(actual_from_addr_ptr)));
			//WWDEBUG_SAY(("\n"));

			//DIE;

			//
			// So far we've only seen the problem manifest for APPPACKETTYPE_CSDAMAGEEVENT.
			// It should be safe to kill the existing conflicting object of this type.
			//
			if (object->Get_App_Packet_Type() == APPPACKETTYPE_CSDAMAGEEVENT) {
				object->Set_Delete_Pending();
			}

			//
			// Flush this packet and bail.
			//
			packet.Flush();
			return;
		}

		//
		//	Create the network object
		//
		object = Create_Network_Object (packet, net_classid, network_obj_id);
#ifdef WWDEBUG
		object->Set_Created_By_Packet_ID(packet.Get_Id());
#endif //WWDEBUG
		object->Import_Creation (packet);

		/*
		//XXX
		WWDEBUG_SAY(("Created object # %-3d of type %-20s and id %d\n",
			NetworkObjectMgrClass::Get_Object_Count(),
			cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type()),
			network_obj_id));
		*/
	}

	//
	// TSS092301
	//
	if (PServerConnection->Get_Remote_Host(rhost_id) == NULL) {
		//
		// This was probably a quit packet. Bail !
		//
		packet.Flush();
		return;
	}

	if (object != NULL) {

		//
		//	Do we need to modify this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_RARE) == NetworkObjectClass::BIT_RARE) {
			object->Import_Rare (packet);
		}

		//
		//	Do we need to modify this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_OCCASIONAL) == NetworkObjectClass::BIT_OCCASIONAL) {
			object->Import_Occasional (packet);
		}

		//
		//	Do we need to update this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_FREQUENT) == NetworkObjectClass::BIT_FREQUENT) {
			object->Import_Frequent (packet);
			//object->Increment_Import_State_Count ();
		}

		object->Increment_Import_State_Count();

		//
		//	Do we need to delete this object?
		//
		if (is_delete_pending) {

			//
			//	Delete the object
			//
			if (object != NULL) {
				object->Set_Delete_Pending ();
			}
		}
	} else {
		packet.Flush();
		//Debug_Network_Basic(("Server received update for non-existent object %d.\n",
		//	network_obj_id));
	}

	//
	//	Did we read all the data contained in the packet?
	//
	//WWASSERT(packet.Is_Flushed());
	if (!packet.Is_Flushed())
	{
		WWDEBUG_SAY(("cNetwork::Client_Packet_Handler: packet not flushed for object of type %s\n",
			cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type())));
		DIE;
	}

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
void cNetwork::Client_Packet_Handler([[maybe_unused]] cPacket & packet)
{
#ifndef FREEDEDICATEDSERVER

	WWASSERT(I_Am_Client());
	WWASSERT(Receiver != NULL);

	// TSS - hack
	if (g_is_loading) {
		Debug_Say(("Client flushing packet during loading\n"));
		packet.Flush();
		return;
	}

	//
	// This is presently needed for team object creation which isn't filtered out on send
	//
	if (cNetwork::I_Am_Server()) {
		packet.Flush ();
		return;
	}

	int network_obj_id		= packet.Get (network_obj_id);
	BYTE dirty_bits			= packet.Get (dirty_bits);
	bool is_delete_pending	= packet.Get (is_delete_pending);
	//BYTE app_packet_type		= packet.Get (app_packet_type);

	//
	//	Lookup the object this data belongs to
	//
	NetworkObjectClass *object = NetworkObjectMgrClass::Find_Object (network_obj_id);

	//
	//	Do we need to create this object?
	//
	if ((dirty_bits & NetworkObjectClass::BIT_CREATION) == NetworkObjectClass::BIT_CREATION) {

		//WWASSERT (object == NULL);
		if (object != NULL)
		{
			WWDEBUG_SAY(("cNetwork::Client_Packet_Handler: received BIT_CREATION for existing object of type %s\n",
				cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type())));
			DIE;
		}

		//
		//	Create the network object
		//
		int net_classid = packet.Get (net_classid);
		if (object == NULL) {
			object = Create_Network_Object (packet, net_classid, network_obj_id);
		}
		object->Import_Creation (packet);

		//
		//	HACK - HACK
		//
		if (net_classid == NETCLASSID_GAMEOBJ) {
			BaseGameObj *game_obj			= (BaseGameObj *)object;
			SmartGameObj *smart_game_obj	= game_obj->As_SmartGameObj ();
			if (smart_game_obj != NULL) {
				int control_owner = smart_game_obj->Get_Control_Owner ();
				smart_game_obj->Set_Player_Data (cPlayerManager::Find_Player (control_owner));
			}
		}

		/*
		//XXX
		WWDEBUG_SAY(("Created object # %-3d of type %-20s and id %d\n",
			NetworkObjectMgrClass::Get_Object_Count(),
			cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type()),
			network_obj_id));
		*/
	}

	if (object != NULL) {

		//
		//	Do we need to delete this object?
		//
		if (is_delete_pending && object != NULL) {
			object->Set_Delete_Pending ();
		}

		//
		//	Do we need to modify this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_RARE) == NetworkObjectClass::BIT_RARE) {
			object->Import_Rare (packet);
		}

		//
		//	Do we need to modify this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_OCCASIONAL) == NetworkObjectClass::BIT_OCCASIONAL) {
			object->Import_Occasional (packet);
		}

		//
		//	Do we need to update this object?
		//
		if ((dirty_bits & NetworkObjectClass::BIT_FREQUENT) == NetworkObjectClass::BIT_FREQUENT) {
			object->Import_Frequent (packet);
			//object->Increment_Import_State_Count ();
		}

		object->Increment_Import_State_Count();
		object->Set_Last_Clientside_Update_Time(TIMEGETTIME());

		/*moving up
		//
		//	Do we need to delete this object?
		//
		if (is_delete_pending) {

			//
			//	Delete the object
			//
			if (object != NULL) {
				object->Set_Delete_Pending ();
			}
		}
		*/
	} else {
		packet.Flush();
		//Debug_Network_Basic(("Client %d received update for non-existent object %d.\n",
		//	Get_My_Id(), network_obj_id));
	}

	//
	//	Did we read all the data contained in the packet?
	//
	//WWASSERT(packet.Is_Flushed());
	if (!packet.Is_Flushed())
	{
		WWDEBUG_SAY(("cNetwork::Client_Packet_Handler: packet not flushed for object of type %s\n",
			cAppPacketStats::Interpret_Type(object->Get_App_Packet_Type())));
		//DIE;
	}

#endif // !FREEDEDICATEDSERVER
}







	//BYTE app_packet_type		= packet.Get (app_packet_type);