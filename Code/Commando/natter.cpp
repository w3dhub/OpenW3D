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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/natter.cpp                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/22/02 4:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"_globals.h"
#include "wwdebug.h"

#include "natter.h"
#include "nat.h"
#include	"natsock.h"
#include "crandom.h"
#include	"registry.h"

#include "WOLLogonMgr.h"
#include	"packettype.h"
#include	"cnetwork.h"
#include	"fromaddress.h"
#include "packetmgr.h"

#include "../WWOnline/WOLChannel.h"
#include "../WWOnline/WOLGameOptions.h"
#include "../WWOnline/WOLLoginInfo.h"
#include "../WWOnline/WOLProduct.h"
#include "../WWOnline/WOLServer.h"
#include <cstdio>


WOLNATInterfaceClass WOLNATInterface;



/***********************************************************************************************
 * WOLNATInterfaceClass::WOLNATInterfaceClass -- Class constructor                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:45PM ST : Created                                                              *
 *=============================================================================================*/
WOLNATInterfaceClass::WOLNATInterfaceClass(void)
{
	GameOptionsMutex = CreateMutexA(NULL, false, NULL);
	ServiceSocketHandler = NULL;
	IsServer = false;
	PortBase = 0;
	ForcePort = 0;
	RegExternalIP = 0;
	RegExternalPort = 0;
	ChatExternalIP = 0;
}


/***********************************************************************************************
 * WOLNATInterfaceClass::~WOLNATInterfaceClass -- Class destructor                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:45PM ST : Created                                                              *
 *=============================================================================================*/
WOLNATInterfaceClass::~WOLNATInterfaceClass(void)
{
	CloseHandle(GameOptionsMutex);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Init -- Get the class ready for use.                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2001 1:20PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::Init(void)
{

	/*
	** Start up the firewall helper class.
	*/
	FirewallHelper.Startup();

	/*
	** Get the session pointer.
	*/
	SessionPtr = WWOnline::Session::GetInstance(false);

	/*
	** Register as an observer.
	*/
	Observer<WWOnline::UserEvent>::NotifyMe(*SessionPtr);
	Observer<WWOnline::UserIPEvent>::NotifyMe(*SessionPtr);
	Observer<WWOnline::GameOptionsMessage>::NotifyMe(*SessionPtr);
	Observer<WWOnline::ConnectionStatus>::NotifyMe(*SessionPtr);

	/*
	** Read default values from the registry.
	*/
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
	fw_assert(reg.Is_Valid());

	if (reg.Is_Valid()) {

		/*
		** Read the FirewallHelper values from the registry.
		*/
		int last_behavior = reg.Get_Int("Behavior", 0);
		int last_source_port_allocation_delta = reg.Get_Int("PortDelta", 1);
		int source_port_pool = reg.Get_Int("PortPool", 0);
		int confidence = reg.Get_Int("Confidence", 0);
		bool send_delay;	// = reg.Get_Bool("SendDelay", 0);
		int force_port;
		Get_Config(&reg, force_port, send_delay);
		ForcePort = (unsigned short) force_port;
		RegExternalIP = (unsigned int) reg.Get_Int("ExternalIP", RegExternalIP);
		RegExternalPort = (unsigned short) reg.Get_Int("ExternalPort", RegExternalPort);

		/*
		** Set the values into the firewall helper.
		*/
		FirewallHelper.Set_Firewall_Info((unsigned int)last_behavior, last_source_port_allocation_delta, (unsigned short)source_port_pool, send_delay, confidence);

		/*
		** Read the local class values from the registry.
		*/
		PortBase = reg.Get_Int("PortBase", PortBase);
		//ForcePort = reg.Get_Int("ForcePort", ForcePort);
	}

	/*
	** Make sure the base port is reasonable.
	*/
	if (PortBase < 1024) {
		PortBase = FreeRandom.Get_Int(PORT_BASE_MIN, PORT_BASE_MAX - 32);
	}

	fw_assert(PortBase >= PORT_BASE_MIN && PortBase < PORT_BASE_MAX);

	/*
	** Use the next base port. Add 2 not 1 since the server requires 2 ports if not dedicated.
	** Make sure the port isn't in use.
	*/
	bool got_port = false;
	unsigned short start_port = PortBase;
	unsigned int timeout = TIMEGETTIME() + TIMER_SECOND * 5;

	do {
		PortBase += 2;
		if (PortBase > PORT_BASE_MAX - 1) {
			PortBase = PORT_BASE_MIN;
		}

		/*
		** If we went all the way around and couldn't find a port then give up.
		*/
		if (PortBase == start_port) {
			break;
		}

		/*
		** Try binding the ports to sockets to see if they are useable.
		*/
		SocketHandlerClass socket1, socket2;
		if (socket1.Open(PortBase, 0)) {
			if (socket2.Open(PortBase+1, 0)) {
				got_port = true;
				socket2.Close();
			} else {
				WWDEBUG_SAY(("WOLNATInterface::Init - unable to open port %d\n", (int)PortBase+1));
			}
			socket1.Close();
		} else {
			WWDEBUG_SAY(("WOLNATInterface::Init - unable to open port %d\n", (int)PortBase));
		}
	} while (!got_port && timeout > TIMEGETTIME());
}


/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Port_As_Server -- Get the current server port number              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Server port                                                                       *
 *                                                                                             *
 * WARNINGS: Use on server side only                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/20/2001 12:27PM ST : Created                                                            *
 *=============================================================================================*/
unsigned short WOLNATInterfaceClass::Get_Port_As_Server(void)
{
	fw_assert(PortBase != 0);

	/*
	** If the user specified a port then use that.
	*/
	if (ForcePort) {
		WWDEBUG_SAY(("WOLNATInterface::Get_Port_As_Server - returning port %d\n", (int)ForcePort));
		return(ForcePort);
	}

	/*
	** Use the rolling port number scheme.
	*/
	WWDEBUG_SAY(("WOLNATInterface::Get_Port_As_Server - returning port %d\n", (int)PortBase));
	return(PortBase);
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Port_As_Server_Client -- Get the current client port number       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Client port for server player                                                     *
 *                                                                                             *
 * WARNINGS: Use on server side only                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/20/2001 12:27PM ST : Created                                                            *
 *=============================================================================================*/
unsigned short WOLNATInterfaceClass::Get_Port_As_Server_Client(void)
{
	fw_assert(PortBase != 0);

	/*
	** If the user specified a port then use that.
	*/
	if (ForcePort) {
		WWDEBUG_SAY(("WOLNATInterface::Get_Port_As_Server_Client - returning port %d\n", (int)ForcePort+1));
		return(ForcePort + 1);
	}

	/*
	** Use the rolling port number scheme.
	*/
	WWDEBUG_SAY(("WOLNATInterface::Get_Port_As_Server_Client - returning port %d\n", (int)PortBase+1));
	return(PortBase + 1);
}





/***********************************************************************************************
 * WOLNATInterfaceClass::Set_Server -- Set the IsServer flag for the class                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/14/2001 2:20PM ST : Created                                                             *
 *=============================================================================================*/
void WOLNATInterfaceClass::Set_Server(bool is_server)
{
	IsServer = is_server;

	if (IsServer) {
		FirewallHelper.Reset_Server();
	}
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Mangler_Server_List -- Get list if mangler servers                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/21/2001 9:59PM ST : Created                                                             *
 *=============================================================================================*/
DynamicVectorClass<WOL::Server*> WOLNATInterfaceClass::Get_Mangler_Server_List(void)
{
	DynamicVectorClass<WOL::Server*> return_list;

	const WWOnline::MGLServerList &server_list = SessionPtr->GetManglerServerList();
	const size_t num_servers = server_list.size();

	/*
	** Copy each server from the Session server list into our own vector.
	*/
	for (size_t i=0 ; i<num_servers ; i++) {
		WOL::Server *server = new WOL::Server;
		memcpy((void*)server, (void*)&(server_list[i]->GetData()), sizeof(*server));
		return_list.Add(server);
	}

	return(return_list);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Num_Mangler_Servers -- Get the number of mangler servers we have  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Number of mangler servers                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/21/2001 9:59PM ST : Created                                                             *
 *=============================================================================================*/
int WOLNATInterfaceClass::Get_Num_Mangler_Servers(void)
{
	const WWOnline::MGLServerList &server_list = SessionPtr->GetManglerServerList();
	return(static_cast<int>(server_list.size()));
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Mangler_Name_By_Index -- Get mangler name from servserv           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Index of name to retrieve                                                         *
 *           Buffer to return name in                                                          *
 *                                                                                             *
 * OUTPUT:   True if we found the name                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/21/2001 10:17PM ST : Created                                                            *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Get_Mangler_Name_By_Index(int index, char *mangler_name)
{
	/*
	** Get the server list.
	*/
	DynamicVectorClass<WOL::Server*> server_list = WOLNATInterface.Get_Mangler_Server_List();

	/*
	** Make sure it contains the server index we want.
	*/
	if (server_list.Count() <= index) {
		for (int i=0 ; i<server_list.Count() ; i++) {
			delete server_list[i];
		}
		server_list.Delete_All();
		return(false);
	}

	/*
	** Find the name we want.
	*/
	char conn_data[128];
	fw_assert(strlen((char*)server_list[index]->conndata) < 128);
	strcpy(conn_data, (char*) server_list[index]->conndata);
	for (int i=0 ; i<server_list.Count() ; i++) {
		delete server_list[i];
	}
	server_list.Delete_All();

	char *cptr = strtok(conn_data, ";");
	cptr = strtok(NULL, ";");
	if (cptr) {
		/*
		** Found the name, copy it into the return buffer and we are done.
		*/
		strcpy(mangler_name, cptr);
		return(true);
	}

	return(false);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Mangler_Port_By_Index -- Get mangler port from servserv           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Index of port number to retrieve                                                  *
 *                                                                                             *
 * OUTPUT:   Port number (0 if not found)                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/21/2001 10:17PM ST : Created                                                            *
 *=============================================================================================*/
unsigned short WOLNATInterfaceClass::Get_Mangler_Port_By_Index(int index)
{
	/*
	** Get the server list.
	*/
	DynamicVectorClass<WOL::Server*> server_list = WOLNATInterface.Get_Mangler_Server_List();

	/*
	** Make sure it contains the server index we want.
	*/
	if (server_list.Count() <= index) {
		for (int i=0 ; i<server_list.Count() ; i++) {
			delete server_list[i];
		}
		server_list.Delete_All();
		return(false);
	}

	/*
	** Find the port we want.
	*/
	char conn_data[128];
	fw_assert(strlen((char*)server_list[index]->conndata) < 128);
	strcpy(conn_data, (char*) server_list[index]->conndata);
	for (int i=0 ; i<server_list.Count() ; i++) {
		delete server_list[i];
	}
	server_list.Delete_All();

	char *cptr = strtok(conn_data, ";");
	cptr = strtok(NULL, ";");
	if (cptr) {
		cptr = strtok(NULL, ";");
		if (cptr) {
			return((unsigned short)atol(cptr));
		}
	}

	return(0);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Shutdown -- Put the class into a limbo state                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2001 1:20PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::Shutdown(void)
{
	/*
	** Unregister our observer status.
	*/
	//Observer<WWOnline::UserEvent>::StopObserving();
	Observer<WWOnline::GameOptionsMessage>::StopObserving();
	Observer<WWOnline::ConnectionStatus>::StopObserving();

	/*
	** Relase the session pointer
	*/
	SessionPtr.Release();

	/*
	** Now we need to write out any registry values that might have changed.
	*/
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
	fw_assert(reg.Is_Valid());

	if (reg.Is_Valid()) {

		/*
		** Read the FirewallHelper values from the class.
		*/
		unsigned int last_behavior = 0;
		int last_source_port_allocation_delta = 1;
		unsigned short source_port_pool = PORT_POOL_MIN;
		bool send_delay = false;
		int confidence = 0;

		FirewallHelper.Get_Firewall_Info(last_behavior, last_source_port_allocation_delta, source_port_pool, send_delay, confidence);

		reg.Set_Int("Behavior", (int)last_behavior);
		reg.Set_Int("PortDelta", last_source_port_allocation_delta);
		reg.Set_Int("PortPool", (int)source_port_pool);
		reg.Set_Int("Confidence", confidence);

		//reg.Set_Bool("SendDelay", send_delay);

		/*
		** Set the local class values into the registry.
		*/
		reg.Set_Int("PortBase", PortBase);
		//reg.Set_Int("ForcePort", ForcePort);

		Set_Config(&reg, ForcePort, send_delay);
	}

	/*
	** Shut down the firewall helper class.
	*/
	FirewallHelper.Shutdown();

}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Config -- Get config settings from the registry.                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to registry entry object (null for default)                                   *
 *           Reference to port number to set                                                   *
 *           Reference to send delay flag to set                                               *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2001 12:38PM ST : Created                                                            *
 *=============================================================================================*/
void WOLNATInterfaceClass::Get_Config(RegistryClass *reg, int &port_number, bool &send_delay)
{
	RegistryClass *local_reg = reg;

	if (!local_reg) {
		local_reg = new RegistryClass(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
	}

	send_delay = local_reg->Get_Bool("SendDelay", FirewallHelper.Get_Send_Delay());
	port_number = local_reg->Get_Int("ForcePort", ForcePort);

	FirewallHelper.Set_Send_Delay(send_delay);
	ForcePort = port_number;

	if (!reg) {
		delete local_reg;
	}
}





/***********************************************************************************************
 * WOLNATInterfaceClass::Set_Config -- Set config settings into the registry                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to registry entry object (null for default)                                   *
 *           Port number to set                                                                *
 *           Send delay flag to set                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2001 12:39PM ST : Created                                                            *
 *=============================================================================================*/
void WOLNATInterfaceClass::Set_Config(RegistryClass *reg, int port_number, bool send_delay)
{
	RegistryClass *local_reg = reg;

	if (!local_reg) {
		local_reg = new RegistryClass(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
	}

	FirewallHelper.Set_Send_Delay(send_delay);
	ForcePort = port_number;

	local_reg->Set_Bool("SendDelay", send_delay);
	local_reg->Set_Int("ForcePort", ForcePort);

	if (!reg) {
		delete local_reg;
	}
}




/***********************************************************************************************
 * WOLNATInterfaceClass::HandleNotification -- Handle notification for user event.             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    UserEvent thingy                                                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 6:57PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::HandleNotification(WWOnline::UserEvent& event)
{
	WWDEBUG_SAY(("WOLNATInterface: UserEvent received.\n"));

	/*
	** If we are in a channel the request our IP.
	*/
	if (ChatExternalIP == 0) {
		if (event.GetEvent() == WWOnline::UserEvent::Join) {

			const RefPtr<WWOnline::UserData>& joiner = event.Subject();
			WOL::User const &user = joiner->GetData();

			/*
			** Is this me?
			*/
			WideStringClass nick;
			if (WOLLogonMgr::GetLoginName(nick)) {

				WWDEBUG_SAY(("WOLNATInterface: User is %s\n", (char*)user.name));

				char name_buffer[256];
				name_buffer[0] = 0;
				Get_Silly_String(&nick, name_buffer, 256);
				if (stricmp(name_buffer, (char*)user.name) == 0) {
					/*
					** Ask the chat server what my IP is.
					*/
					SessionPtr->RequestUserIP((char*)user.name);
				}
			}
		}
	}

	/*
	** If someone is leaving the channel, and we are a server they are waiting to negotiate with, then we need to remove them
	** from our pending negotiation list.
	*/
	if (Am_I_Server() && event.GetEvent() == WWOnline::UserEvent::Leave) {
		const RefPtr<WWOnline::UserData>& joiner = event.Subject();
		WOL::User const &user = joiner->GetData();
		if (FirewallHelper.Remove_Player_From_Negotiation_Queue_If_Mutex_Available((char *)user.name)) {
			WWDEBUG_SAY(("WOLNATInterface: UserEvent Handler: Removed %s from player queue.\n", (char*)user.name));
		}
	}


#if (0)
	/*
	** Don't do anything here unless we are the server.
	*/
	if (Am_I_Server()) {
	//if (CombatManager::I_Am_Server()) {

		/*
		** If someone is joining our channel and we are a game server then we need to start talking to them.
		*/
		if (event.GetEvent() == WWOnline::UserEvent::Join) {

			WWDEBUG_SAY(("WOLNATInterface: UserEvent is 'Join'.\n"));

			/*
			** Make sure this is a game channel by comparing the channel type to the game ID in our SKU.
			*/
			/*
			** Get the channel type and our product SKU.
			*/
			const RefPtr<WWOnline::ChannelData> &channel = SessionPtr->GetCurrentChannel();
			WOL::Channel &wol_channel = channel->GetData();
			RefPtrConst<WWOnline::Product> product = WWOnline::Product::Current();

			if (product.IsValid()) {
				int sku = product->GetSKU();

				/*
				** Compare only the product type field of the sku with the game channel type.
				*/
				if (wol_channel.type == ((sku & 0xff00) >> 8)) {

					WWDEBUG_SAY(("WOLNATInterface: Channel is of our game type\n"));

					/*
					** Get the original user structure from the event.
					*/
					const RefPtr<WWOnline::UserData>& joiner = event.Subject();
					WOL::User const &user = joiner->GetData();

					/*
					** Is this me? If so, we don't want to invite ourselves.
					*/
					WideStringClass nick;
					if (WOLLogonMgr::GetLoginName(nick)) {

						WWDEBUG_SAY(("WOLNATInterface: User is %s\n", (char*)user.name));

						char name_buffer[256];
						name_buffer[0] = 0;
						Get_Silly_String(&nick, name_buffer, 256);
						if (stricmp(name_buffer, (char*)user.name) != 0) {

							WWDEBUG_SAY(("WOLNATInterface: User isn't me.\n", (char*)user.name));

							/*
							** Call the join function.
							*/
							FirewallHelper.Talk_To_New_Player((WOL::User*)&user);
						}
					}
				}
			}
		}
	}
#endif //(0)
}




/***********************************************************************************************
 * WOLNATInterfaceClass::HandleNotification -- Handle notification for game options.           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Game options message                                                              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:50PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::HandleNotification(WWOnline::GameOptionsMessage &message)
{
	WWDEBUG_SAY(("WOLNATInterface: GameOptionsMessage received.\n"));

	if (message.IsPrivate()) {

		/*
		** Get the message.
		*/
		const WOL::User& user = message.GetWOLUser();
		WideStringClass porky_options = static_cast<const char*>(message.GetOptions());

		/*
		** Convert to standard string.
		*/
		char slender_options[OPTIONS_STAGING_BUFFER_SIZE];
#ifdef WWDEBUG
		char *ptr =
#endif //WWDEBUG
				Get_Silly_String(&porky_options, slender_options, sizeof(slender_options));
		fw_assert(ptr == slender_options);

		if (strnicmp(slender_options, "NAT:", 4) == 0) {

			/*
			** Add it to the options staging area.
			*/
			ThreadLockClass locker(this);

			GameOptionsStagingStruct *new_options = new GameOptionsStagingStruct;
			new_options->User = user;
			strcpy(new_options->Options, slender_options);

			IncomingOptions.Add(new_options);
		}
	}
}


/***********************************************************************************************
 * WOLNATInterfaceClass::HandleNotification -- Handle notification for connection status.      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Connection status                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:50PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::HandleNotification(WWOnline::ConnectionStatus &status)
{
	WWDEBUG_SAY(("WOLNATInterface: ConnectionStatus received.\n"));

	if (status == WWOnline::ConnectionConnected) {
		FirewallHelper.Connected_To_WWOnline_Server();
	}
}



/***********************************************************************************************
 * WOLNATInterfaceClass::HandleNotification -- Handle notification for user IP events          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    User IP Event                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:50PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::HandleNotification(WWOnline::UserIPEvent &ipevent)
{
	WWDEBUG_SAY(("WOLNATInterface: ConnectionStatus received.\n"));

	if (ipevent.GetEvent() == WWOnline::UserIPEvent::GotIP) {
		WOL::User user = ipevent();
		char namebuf[32];
		if (Get_My_Name(namebuf)) {
			if (stricmp((char*)user.name, namebuf) == 0) {
				unsigned int ip = user.ipaddr;
				WWDEBUG_SAY(("WOLNATInterfaceClass::HandleNotification(WWOnline::UserIPEvent &ipevent) : ip = %08x\n"));
				ChatExternalIP = ip;
				if (ForcePort || !FirewallHelper.Get_External_Address().Is_Valid() || FirewallHelper.Get_External_Address().Get_Address() == 0) {
					IPAddressClass ipaddr(ip, 0);
					FirewallHelper.Set_External_Address(ipaddr);
				}
			}
		}
	}
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Get_My_Name -- Get name of local user.                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to return buffer                                                              *
 *                                                                                             *
 * OUTPUT:   True if got name                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/13/2001 12:08PM ST : Created                                                            *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Get_My_Name(char *namebuf)
{
	WideStringClass nick;
	if (WOLLogonMgr::GetLoginName(nick)) {
		namebuf[0] = 0;
		Get_Silly_String(&nick, namebuf, 256);
		return(true);
	}

	return(false);
}





/***********************************************************************************************
 * WOLNATInterfaceClass::Send_Private_Game_Options -- Send private game options                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 7:18PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::Send_Private_Game_Options(WOL::User *user, char *options)
{
	/*
	** This can be called from the firewall thread so all it does is add the options to a queue. We can't send them directly
	** here since WOLAPI doesn't have a multi-threaded interface.
	*/
	ThreadLockClass locker(this);

	fw_assert(user != NULL);
	fw_assert(options != NULL);
	fw_assert(strlen(options) < OPTIONS_STAGING_BUFFER_SIZE);

	GameOptionsStagingStruct *new_options = new GameOptionsStagingStruct;
	new_options->User = *user;
	strcpy(new_options->Options, options);

	WWDEBUG_SAY(("WOLNATInterface: Send options string: %s\n", options));

	OutgoingOptions.Add(new_options);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Private_Game_Options -- Return next game options packet           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2001 9:40PM ST : Created                                                              *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Get_Private_Game_Options(WOL::User *user, char *options_buffer, [[maybe_unused]] int option_buffer_len)
{
	ThreadLockClass locker(this);

	fw_assert(user != NULL);
	fw_assert(options_buffer != NULL);
	fw_assert(option_buffer_len >= OPTIONS_STAGING_BUFFER_SIZE);

	if (IncomingOptions.Count()) {
		GameOptionsStagingStruct *options = IncomingOptions[0];
		memcpy(user, &options->User, sizeof(*user));
		strcpy(options_buffer, options->Options);

		delete IncomingOptions[0];
		IncomingOptions.Delete(0);
		return(true);
	}

	return(false);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Tell_Server_That_Client_Is_In_Channel -- Client initiates port neg.   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/13/2001 2:15PM ST : Created                                                            *
 *=============================================================================================*/
void WOLNATInterfaceClass::Tell_Server_That_Client_Is_In_Channel(void)
{
	/*
	** Get the channel name (could be passed in maybe).
	*/
	RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(false);
	WideStringClass name_string(wol_session->GetCurrentChannel()->GetName(), true);

	char namebuf[256];
	int buffer_size = sizeof(namebuf);
	Get_Silly_String(&name_string, namebuf, buffer_size);

	/*
	** Fill in a ClientInChannel options packet to send to the host.
	*/
	PrivateGameOptionsStruct options;
	strcpy(options.NATOptionsPrefix, "NAT:");
	options.Option = WOLNATInterfaceClass::OPTION_CLIENT_IN_CHANNEL;

	char my_name[64];
	Get_My_Name(my_name);
	strcpy(options.OptionData.ClientInChannel.Name, my_name);

	/*
	** Send it.
	*/
	WOL::User user;
	memset(&user, 0, sizeof(user));
	strcpy((char*)user.name, namebuf);

	WOLNATInterface.Send_Private_Game_Options(&user, (char*)&options);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Service -- Game options service                                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 8:17PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::Service(void)
{
	ThreadLockClass locker(this);

	/*
	** Remove any names from the client queue that represent players who have already left the channel.
	*/
	FirewallHelper.Cleanup_Client_Queue();

	/*
	** Send any pending game options strings.
	*/
	while (OutgoingOptions.Count()) {

		GameOptionsStagingStruct *options = OutgoingOptions[0];

#ifdef WWDEBUG
		HRESULT res =
#endif //WWDEBUG
			SessionPtr->GetChatObject()->RequestPrivateGameOptions(&options->User, options->Options);
		fw_assert(res == S_OK);

		OutgoingOptions.Delete(0);
		delete options;
	}

	/*
	** Do any socket level servicing needed.
	*/
	if (ServiceSocketHandler) {
		Service_Receive_Queue(ServiceSocketHandler);
	}

}



/***********************************************************************************************
 * WOLNATInterfaceClass::Set_Service_Socket_Handler -- Set socket handler for service          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to socket handler class                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/13/2001 3:10PM ST : Created                                                             *
 *=============================================================================================*/
void WOLNATInterfaceClass::Set_Service_Socket_Handler(SocketHandlerClass *socket)
{
	ThreadLockClass locker(this);
	fw_assert((socket == NULL && ServiceSocketHandler != NULL) || (socket != NULL && ServiceSocketHandler == NULL));
	ServiceSocketHandler = socket;
}





/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Silly_String -- Convert wide string to char[]                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to wide string                                                                *
 *           Buffer to return string in                                                        *
 *           Size of buffer                                                                    *
 *                                                                                             *
 * OUTPUT:   Ptr to input buffer                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 8:19PM ST : Created                                                              *
 *=============================================================================================*/
char *WOLNATInterfaceClass::Get_Silly_String(WideStringClass *silly_string, char *buffer, [[maybe_unused]] int buffer_size)
{
	StringClass string;
	silly_string->Convert_To(string);
	char *string_buffer = string.Peek_Buffer();
	fw_assert(strlen(string_buffer) < (unsigned)buffer_size);
	strcpy(buffer, string_buffer);
	return(buffer);
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Current_Server -- Get server we are logged into                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Current server                                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 9:44PM ST : Created                                                              *
 *=============================================================================================*/
void WOLNATInterfaceClass::Get_Current_Server_ConnData(char *buffer, int size)
{
	const RefPtr<WWOnline::IRCServerData>& server = SessionPtr->GetCurrentServer();

	if (server.IsValid()) {
		WOL::Server& data = server->GetData();

		fw_assert(buffer != NULL);
		fw_assert(size >= sizeof(data.conndata));

		strncpy(buffer, (char*)data.conndata, size);
	} else {
		buffer[0] = 0;
	}
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Send_Game_Format_Packet_To -- Send game packet with our payload       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address to send to                                                                *
 *           Packet to send                                                                    *
 *           Size of packet                                                                    *
 *           Socket to send from as client                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2001 11:12AM ST : Created                                                            *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Send_Game_Format_Packet_To(IPAddressClass *address, char *payload, int payload_size, SocketHandlerClass *socket_handler)
{
	ThreadLockClass locker(this);

	payload_size = payload_size;
	fw_assert(address != NULL);
	fw_assert(address->Is_Valid());
	fw_assert(payload != NULL);

	/*
	** Build the packet.
	*/
	cPacket packet;
	packet.Set_Type(PACKETTYPE_FIREWALL_PROBE);
	packet.Set_Id(cPacket::UNDEFINED_ID+1);
	packet.Add_Terminated_String(payload);

	/*
	** Convert the address to game format.
	*/
	struct sockaddr_in sock_address;
	sock_address.sin_family = AF_INET;
	sock_address.sin_port = (unsigned short) htons((unsigned short)address->Get_Port());
	address->Get_Address((unsigned char*)&sock_address.sin_addr.s_addr);

	/*
	** Send the packet.
	*/
	if (Am_I_Server()) {
		/*
		** On the server, we can use the regular game packet send.
		*/
		fw_assert(cNetwork::PServerConnection != NULL);
		cNetwork::PServerConnection->Send_Packet_To_Address(packet, &sock_address);
	} else {

		fw_assert(socket != NULL);

		/*
		** If we are the client then the game comms isn't initialised and we need to send directly. Ugh.
		**
		** Calculate the packet CRC.
		*/
		cPacket full_packet;
		cPacket::Construct_Full_Packet(full_packet, packet);

		/*
		** Send it.
		*/
		PacketManager.Take_Packet((unsigned char *)full_packet.Get_Data(), full_packet.Get_Compressed_Size_Bytes(), (unsigned char*)&sock_address.sin_addr.s_addr, sock_address.sin_port, socket_handler->Get_Socket());
		PacketManager.Flush(true);	//Hmmmmm is this going to send a bunch of other packets to the wrong place?

#if (0)
		WWDEBUG_SAY(("WOLNATInterface - sendto %s\n", address->As_String()));
		int result = sendto(socket_handler->Get_Socket(), full_packet.Get_Data(), full_packet.Get_Compressed_Size_Bytes(), 0, (LPSOCKADDR) &sock_address, sizeof(struct sockaddr_in));
		if (result == SOCKET_ERROR){
			if (LAST_ERROR != WSAEWOULDBLOCK) {
				WWDEBUG_SAY(("WOLNATInterface - sendto returned error code %d\n", LAST_ERROR));
			} else {

				/*
				** No more room for outgoing packets.
				*/
				WWDEBUG_SAY(("WOLNATInterface - sendto returned WSAEWOULDBLOCK\n", LAST_ERROR));
			}
		}
#endif //(0)
	}

	return(true);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Intercept_Game_Packet -- Handle one of our packets coming in          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2001 12:08PM ST : Created                                                            *
 *=============================================================================================*/
void WOLNATInterfaceClass::Intercept_Game_Packet(cPacket &packet)
{
	ThreadLockClass locker(this);

	/*
	** All we care about is the from address and the payload string to extract those.
	*/
	char payload[512];
	payload[0] = 0;
	int payload_size = 512;

	packet.Get_Terminated_String(payload, payload_size, true);
	fw_assert(strlen(payload) > 0);

	/*
	** Get the address.
	*/
	cFromAddress const *from_address = packet.Get_From_Address_Wrapper();
	IPAddressClass my_address;
	my_address.Set_Address((unsigned char*)&(from_address->FromAddress.sin_addr.s_addr), (unsigned short) ntohs(from_address->FromAddress.sin_port));

	WWDEBUG_SAY(("WOLNATInterface: Got game packet from address %s - %s\n", my_address.As_String(), payload));

	/*
	** Add it to the incoming packet queue.
	*/
	GamePacketStruct *new_packet = new GamePacketStruct;
	new_packet->FromAddress = my_address;
	fw_assert(strlen(payload) < sizeof(new_packet->Payload));
	strncpy((char*)(new_packet->Payload), payload, sizeof(new_packet->Payload)-1);

	IncomingPackets.Add(new_packet);
}




/***********************************************************************************************
 * WOLNATInterfaceClass::Service_Receive_Queue -- Pick up game packets for clients only        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket to check on                                                                *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2001 1:42PM ST : Created                                                             *
 *=============================================================================================*/
void WOLNATInterfaceClass::Service_Receive_Queue(SocketHandlerClass *socket)
{
	ThreadLockClass locker(this);
	fw_assert(socket != NULL);
	fw_assert(socket->Get_Socket() != INVALID_SOCKET);

	/*
	** If we are not the server, just try to pick up a single packet.
	*/
	if (!Am_I_Server()) {

		cPacket packet;
		cPacket full_packet;


		unsigned char ip_address[4];
		unsigned short port;
		int bytes = PacketManager.Get_Packet(socket->Get_Socket(), (unsigned char *)full_packet.Get_Data(), full_packet.Get_Max_Size(), ip_address, port);
		if (bytes > 0) {
			sockaddr_in *addr_ptr = (struct sockaddr_in*) &full_packet.Get_From_Address_Wrapper()->FromAddress;
			memcpy(&addr_ptr->sin_addr.s_addr, ip_address, 4);
			addr_ptr->sin_port = port;
		}
		if (bytes > 0) {
			full_packet.Set_Bit_Length(bytes*8);
			cPacket::Construct_App_Packet(packet, full_packet);
			Intercept_Game_Packet(packet);
		}

#if (0)
		unsigned int bytes = 0;

		int result = ioctlsocket(socket->Get_Socket(), FIONREAD, &bytes);

		/*
		** Result of 0 is success.
		*/
		fw_assert(result == 0);

		/*
		** If there is outstanding data, 'bytes' will contain the size of the next queued datagram.
		*/
		if (bytes != 0) {

			/*
			** Call recvfrom function to get the outstanding packet.
			*/
			int addr_len = sizeof(sockaddr_in);
			result = recvfrom(socket->Get_Socket(), full_packet.Get_Data(), full_packet.Get_Max_Size(), 0, (LPSOCKADDR) &full_packet.Get_From_Address_Wrapper()->FromAddress, &addr_len);

			/*
			** See if we got an error.
			*/
			if (result != SOCKET_ERROR) {

				full_packet.Set_Bit_Length(result*8);
				cPacket::Construct_App_Packet(packet, full_packet);
				Intercept_Game_Packet(packet);
			}
		}
#endif //(0)

	}
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Packet -- Get the next queued packet.                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/10/2001 12:25PM ST : Created                                                            *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Get_Packet(char *packet_buffer, int buffer_size, IPAddressClass &address)
{
	ThreadLockClass locker(this);

	/*
	** If there is a packet in the queue then return it.
	*/
	if (IncomingPackets.Count()) {
		GamePacketStruct *packet = IncomingPackets[0];
		fw_assert(strlen((char*)(packet->Payload)) < (unsigned)buffer_size);
		strncpy(packet_buffer, (char*)(packet->Payload), buffer_size-1);
		address = packet->FromAddress;
		delete packet;
		IncomingPackets.Delete(0);
		return(true);
	}

	return(false);
}






/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Next_Client_Port -- Get next port client should use               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Next client port                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/14/2001 10:12PM ST : Created                                                            *
 *=============================================================================================*/
unsigned short WOLNATInterfaceClass::Get_Next_Client_Port(void)
{

	/*
	** If the user specified a port then use that.
	*/
	if (ForcePort) {
		WWDEBUG_SAY(("WOLNATInterface::Get_Next_Client_Port - returning port %d\n", (int)ForcePort));
		return(ForcePort);
	}

	/*
	** Use the rolling port number scheme.
	** Make sure the port isn't in use.
	*/
	bool got_port = false;
	unsigned short start_port = PortBase;
	unsigned int timeout = TIMEGETTIME() + TIMER_SECOND * 5;

	do {
		PortBase++;
		if (PortBase > PORT_BASE_MAX - 1) {
			PortBase = PORT_BASE_MIN;
		}

		/*
		** If we went all the way around and couldn't find a port then give up.
		*/
		if (PortBase == start_port) {
			break;
		}

		/*
		** Try binding the port to a socket to see if it is useable.
		*/
		SocketHandlerClass socket;
		if (socket.Open(PortBase, 0)) {
			got_port = true;
			socket.Close();
		} else {
			WWDEBUG_SAY(("WOLNATInterface::Get_Next_Client_Port - unable to open port %d\n", (int)PortBase));
		}
	} while (!got_port && timeout > TIMEGETTIME());

	WWDEBUG_SAY(("WOLNATInterface::Get_Next_Client_Port - returning port %d\n", (int)PortBase));
	return(PortBase);
}





/***********************************************************************************************
 * WOLNATInterfaceClass::Set_Server_Negotiated_Address -- Write server ip and port back to game*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Server address                                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/15/2001 6:43PM ST : Created                                                             *
 *=============================================================================================*/
void WOLNATInterfaceClass::Set_Server_Negotiated_Address(IPAddressClass *server_address)
{
	fw_assert(server_address != NULL);
	fw_assert(server_address->Is_Valid());

	if (server_address && server_address->Is_Valid()) {
		ServerNegotiatedAddress = *server_address;
		WWASSERT(PTheGameData != NULL);
		The_Game()->Set_Ip_Address((unsigned int)server_address->Get_Address());
		The_Game()->Set_Port(server_address->Get_Port());
	}
}


/***********************************************************************************************
 * WOLNATInterfaceClass::Save_Firewall_Info_To_Registry -- Save detection info to registry     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/2001 9:39PM ST : Created                                                            *
 *=============================================================================================*/
void WOLNATInterfaceClass::Save_Firewall_Info_To_Registry(void)
{
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
	fw_assert(reg.Is_Valid());

	if (reg.Is_Valid()) {
		IPAddressClass addr = FirewallHelper.Get_External_Address();
		if (addr.Is_Valid()) {
			reg.Set_Int("ExternalIP", addr.Get_Address());
			reg.Set_Int("ExternalPort", addr.Get_Port());
		}
	}
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Get_Compact_Log -- Get basic log information to send to server        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    String to add info to                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/4/2001 1:12PM ST : Created                                                             *
 *=============================================================================================*/
void WOLNATInterfaceClass::Get_Compact_Log(StringClass &log_string)
{
	char temp[1024];

	/*
	** Port Base
	** Force Port
	** Send Delay
	** Local IP
	** External IP
	** Raw firewall behavior
	** Source port allocation delta
	** Source port pool
	** Server negotiated IP
	** Server negotiated port.
	*/
	sprintf(temp, "%d\t%d\t%d\t%08X\t%08X\t%02X\t%d\t%d\t%08X\t%d\t",
			PortBase,
			ForcePort,
			FirewallHelper.Get_Send_Delay(),
			ntohl(FirewallHelper.Get_Local_Address()),
			(unsigned int) FirewallHelper.Get_External_Address().Get_Address(),
			(int) FirewallHelper.Get_Raw_Firewall_Behavior(),
			(int) FirewallHelper.Get_Source_Port_Allocation_Delta(),
			FirewallHelper.Get_Source_Port_Pool(),
			(unsigned int) ServerNegotiatedAddress.Get_Address(),
			(int) ServerNegotiatedAddress.Get_Port());

	log_string = temp;
}



/***********************************************************************************************
 * WOLNATInterfaceClass::Is_NAT_Thread_Busy -- Is the NAY thread not in it's idle state?       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   bool - true if the NAT thread not in it's idle state                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/30/2002 12:52PM ST : Created                                                            *
 *=============================================================================================*/
bool WOLNATInterfaceClass::Is_NAT_Thread_Busy(void)
{
	return(FirewallHelper.Is_Busy());
}
