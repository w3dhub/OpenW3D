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
 *                     $Archive:: /Commando/Code/Commando/natter.h                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/01/02 2:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#pragma once

#ifndef NATTER_H
#define NATTER_H

#include "always.h"
//#include "nat.h"
#include	"nataddr.h"


/*
** Project specific includes.
*/
#include <Notify.h>
#include	<WWOnline/WOLSession.h>
#include	<WWOnline/WOLUser.h>
#include	<Combat/combat.h>
#include	<wwnet/wwpacket.h>


/*
** Commonly available components.
*/
#include <wwlib/vector.h>


#ifdef WWASSERT
#ifndef fw_assert
#define fw_assert WWASSERT
#endif //fw_assert
#else //WWASSERT
#define fw_assert assert
#endif //WWASSERT

/*
** Size of temporary game options storage buffers.
*/
#define OPTIONS_STAGING_BUFFER_SIZE 512


#define PORT_BASE_MIN	1024
#define PORT_BASE_MAX	4095

#define PORT_POOL_MIN   4096
#define PORT_POOL_MAX	65535

/*
** Forward declarations.
*/
class SocketHandlerClass;
class RegistryClass;


/*
** This class performs the inevitable task of interfacing with whatever piece of code happens to be talking to WOLAPI.
**
** Will have to be completely replaced per project.
**
*/
class WOLNATInterfaceClass :	public Observer<WWOnline::UserEvent>,
										public Observer<WWOnline::GameOptionsMessage>,
										public Observer<WWOnline::ConnectionStatus>,
										public Observer<WWOnline::UserIPEvent>
{

	public:

		/*
		** Constructor, destructor.
		*/
		WOLNATInterfaceClass(void);
		~WOLNATInterfaceClass(void);

		/*
		** Startup, shutdown.
		*/
		void Init(void);
		void Shutdown(void);

		/*
		** Service.
		*/
		void Service(void);
		void Set_Service_Socket_Handler(SocketHandlerClass *socket);
		void Service_Receive_Queue(SocketHandlerClass *socket);

		/*
		** User event notification.
		*/
		void HandleNotification(WWOnline::UserEvent&) override;

		/*
		** Game options notificaton.
		*/
		void HandleNotification(WWOnline::GameOptionsMessage&) override;

		/*
		** Server connection notification.
		*/
		void HandleNotification(WWOnline::ConnectionStatus&) override;

		/*
		** User IP event.
		*/
		void HandleNotification(WWOnline::UserIPEvent&) override;

		/*
		** Server query.
		*/
		void Get_Current_Server_ConnData(char *buffer, int size);

		/*
		** Private game options.
		*/
		void Send_Private_Game_Options(WOL::User *user, char *options);
		bool Get_Private_Game_Options(WOL::User *user, char *options, int option_buffer_len);

		/*
		** Conversion.
		*/
		char *Get_Silly_String(WideStringClass *silly_string, char *buffer, int buffer_size);

		/*
		** Game packet handling.
		*/
		bool Get_Packet(char *packet_buffer, int buffer_size, IPAddressClass &address);
		void Intercept_Game_Packet(cPacket &packet);
		bool Send_Game_Format_Packet_To(IPAddressClass *address, char *payload, int payload_size, SocketHandlerClass *socket_handler = NULL);

		/*
		** General game helper functions.
		*/
		bool Get_My_Name(char *namebuf);
		bool Am_I_Server(void) {return(IsServer);};
		void Set_Server(bool is_server);
		void Set_Server_Negotiated_Address(IPAddressClass *server_address);
		DynamicVectorClass<WOL::Server*> Get_Mangler_Server_List(void);
		unsigned short Get_Mangler_Port_By_Index(int index);
		bool Get_Mangler_Name_By_Index(int index, char *mangler_name);
		int Get_Num_Mangler_Servers(void);
		unsigned int Get_Local_Address(void) {
			return(0);//return(FirewallHelper.Get_Local_Address());
		};
		void Tell_Server_That_Client_Is_In_Channel(void);
		bool Is_NAT_Thread_Busy(void);

		/*
		** Game port management.
		*/
		unsigned short Get_Next_Client_Port(void);
		unsigned short Get_Port_As_Server(void);
		unsigned short Get_Port_As_Server_Client(void);
		unsigned short Get_Force_Port(void) {return(ForcePort);};

		/*
		** Config.
		*/
		void Get_Config(RegistryClass *reg, int &port_number, bool &send_delay);
		void Set_Config(RegistryClass *reg, int port_number, bool send_delay);
		void Save_Firewall_Info_To_Registry(void);
		unsigned int Get_Reg_External_IP(void) {return(RegExternalIP);}
		unsigned int Get_Reg_External_Port(void) {return(RegExternalPort);}
		void Get_Compact_Log(StringClass &log_string);
		unsigned int Get_Chat_External_IP(void) {return(ChatExternalIP);}


		/*
		** Game options string types.
		*/
		enum {
			OPTION_ERROR = 'a',
			OPTION_INVITE_PORT_NEGOTIATION,
			OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION,
			OPTION_PORT_NOTIFICATION,
			OPTION_CONNECTION_RESULT,
			OPTION_QUEUE_STATE,
			OPTION_ABORT_NEGOTIATION,
			OPTION_CLIENT_IN_CHANNEL
		};

		/*
		** Game options format for the firewall code only. All firewall options packets are prefixed with "NAT:"
		*/
		typedef struct tPrivateGameOptions {

			/*
			** Option previx (Always "NAT:" for us.
			*/
			char			NATOptionsPrefix[4];

			/*
			** Option type. See list above.
			*/
			char			Option;

			/*
			** Option payload union. Each type of option has a different payload.
			*/
			union tOptionData {

				/*
				** Data for OPTION_INVITE_PORT_NEGOTIATION.
				*/
				struct InvitationStruct {
					char	LocalIP[9];				// 00000000,
					char	LocalPort[5];			// 0000,
					char	ExternalIP[9];			// 00000000,
					char	FirewallType[9];		// 00000000,
					char	Queued[5];				//	0000(null)
				} Invitation;

				/*
				** Data for OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION.
				*/
				struct AcceptStruct {
					char	LocalIP[9];				// 00000000,
					char	LocalPort[5];        // 0000,
					char	ExternalIP[9];			// 00000000,
					char	FirewallType[9];		// 00000000(null)
				} Accept;

				/*
				** Data for OPTION_PORT_NOTIFICATION.
				*/
				struct PortStruct {
					char	MangledPort[5];		//	0000,
					char	Name[64];				// "Players name......"(null)
				} Port;

				/*
				** Data for OPTION_CONNECTION_RESULT.
				*/
				struct ConnectionResultStruct {
					char	Result[2];				// n,
					char	Port[5];					// 0000,
					char	Name[64];				// "Players name......"(null)
				} ConnectionResult;

				/*
				** Data for OPTION_QUEUE_STATE
				*/
				struct QueueStateStruct {
					char	Position[3];			// 00(null)
				} QueueState;

				/*
				** Data for OPTION_ABORT_NEGOTIATION
				*/
				struct QuitTalkingStruct {
					char Nothing;					// 00
				} QuitTalking;

				/*
				** Data for OPTION_CLIENT_IN_CHANNEL
				*/
				struct ClientInChannelStruct {
					char	Name[64];				// "Players name......"(null)
				} ClientInChannel;

			} OptionData;
		} PrivateGameOptionsStruct;


	private:

		/*
		** PortBase is the start of the current port number range we are using. The range is added to the port base every time
		** we start a new game so we are always using a fresh port. The server uses 2 ports per game and the client uses 1 port.
		*/
		unsigned short PortBase;

		/*
		** Set this to non-zero to force it to be used as the local port.
		*/
		unsigned short ForcePort;

		/*
		** External IP and port from the registry.
		*/
		unsigned int RegExternalIP;
		unsigned short RegExternalPort;

		/*
		** Our IP according to westwood chat.
		*/
		unsigned int ChatExternalIP;

		/*
		** Bloat pointer to the Session class.
		*/
		RefPtr<WWOnline::Session> SessionPtr;

		/*
		** Address we used to send to the server.
		*/
		IPAddressClass ServerNegotiatedAddress;

		/*
		** Holding area for incoming game packets.
		*/
		struct GamePacketStruct {
			IPAddressClass		FromAddress;
			unsigned char		Payload[512];
		};

		DynamicVectorClass<GamePacketStruct*> IncomingPackets;

		/*
		** This is the socket handler to use when sending or receiving game packets. It's only used on the client side since
		** the server side has to use the games mechanisms to send and receive (the game already has a socket bound to the port
		** we want to use in the server case).
		*/
		SocketHandlerClass *ServiceSocketHandler;

		/*
		** Am I a game server?
		*/
		bool IsServer;

		/*
		** Game options staging areas.
		*/
		typedef struct tGameOptionsStagingStruct {
			WOL::User	User;
			char			Options[OPTIONS_STAGING_BUFFER_SIZE];
		} GameOptionsStagingStruct;

		DynamicVectorClass<GameOptionsStagingStruct*> IncomingOptions;
		DynamicVectorClass<GameOptionsStagingStruct*> OutgoingOptions;

		/*
		** Mutex to serialise access to game options.
		*/
		HANDLE GameOptionsMutex;

		/*
		** Thread safety for game options.
		*/
		class ThreadLockClass
		{
			public:
				/*
				** Constructor. Grabs the mutex.
				*/
				inline ThreadLockClass(WOLNATInterfaceClass *wnptr) {
					WNPtr = wnptr;
					int deadlock = WaitForSingleObject(wnptr->GameOptionsMutex, 10 * 1000);
					if (deadlock == WAIT_TIMEOUT) {
						WWDEBUG_SAY(("FirewallHelper - Timeout waiting for firewall helper data mutex\n"));
						fw_assert(deadlock != WAIT_TIMEOUT);
					}
				};

				WOLNATInterfaceClass *WNPtr;

				/*
				** Destructor, releases the mutex.
				*/
				inline ~ThreadLockClass(void) {
					ReleaseMutex(WNPtr->GameOptionsMutex);
				};
		};
		friend ThreadLockClass;
};




/*
** Extern for single instance of the WOLNATInterface.
*/
extern WOLNATInterfaceClass WOLNATInterface;


#endif //NATTER_H
