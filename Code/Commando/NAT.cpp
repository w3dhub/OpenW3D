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
 *                     $Archive:: /Commando/Code/Commando/NAT.cpp                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/26/02 3:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 37                                                          $*
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
#include	<windows.h>
#include "systimer.h"
#include <snmp.h>
#include	<stddef.h>
#include <process.h>
#include <limits>


#include "crandom.h"
#include "Except.h"
#include	"nat.h"
#include	"natsock.h"
#include	"nataddr.h"
#include "slavemaster.h"

#include	"gamedata.h"
#include <cstdio>

/*
** Set this flag in the debugger to cause the client to fail to connect.
*/
//bool CrapFlag = false;

/*
** Single instance of firewall helper class.
*/
FirewallHelperClass FirewallHelper;


/***********************************************************************************************
 * FirewallHelperClass::FirewallHelperClass -- Constructor                                     *
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
 *   3/15/01 5:03PM ST : Created                                                               *
 *=============================================================================================*/
FirewallHelperClass::FirewallHelperClass(void)
{
	Behavior = FIREWALL_TYPE_UNKNOWN;
	LastBehavior = FIREWALL_TYPE_UNKNOWN;
	SourcePortAllocationDelta = 0;
	LastSourcePortAllocationDelta = 0;
	NumManglerServers = 0;
	CurrentManglerServer = -1;
	SourcePortPool = 0;
	ThreadActive = false;
	ThreadState = THREAD_IDLE;
	ThreadEvent = INVALID_HANDLE_VALUE;
	NATThreadMutex = CreateMutexA(NULL, false, NULL);
	NATDataMutex = CreateMutexA(NULL, false, NULL);
	ThreadHandle = INVALID_HANDLE_VALUE;
	QueueNotifyPtr = NULL;
	SuccessFlagPtr = NULL;
	CancelPlayer[0] = 0;
	SendDelay = false;
	Confidence = 0;
}




/***********************************************************************************************
 * FirewallHelperClass::Startup -- Start up the firewall thread                                *
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
 *   8/31/2001 12:35PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Startup(void)
{
	/*
	** Start the firewall thread.
	*/
	WWDEBUG_SAY(("FirewallHelper: Starting firewall thread\n"));
	ThreadState = THREAD_IDLE;
	ThreadEvent = INVALID_HANDLE_VALUE;
	ThreadActive = true;
	//ThreadHandle = CreateThread(NULL, 128*1024, &NAT_Thread_Start, this, 0, &ThreadID);
	ThreadHandle = (HANDLE)_beginthreadex(NULL, 128*1024, &NAT_Thread_Start, this, 0, (unsigned int*)&ThreadID);
	fw_assert(ThreadHandle != NULL);
}




/***********************************************************************************************
 * FirewallHelperClass::Shutdown -- Shut down the firewall thread                              *
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
 *   8/31/2001 12:35PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Shutdown(void)
{
	if (ThreadActive) {
		WWDEBUG_SAY(("FirewallHelper: Stopping firewall thread\n"));

		/*
		** Signal the thread to go away.
		*/
		ThreadActive = false;

		/*
		** Wait for the thread to go away.
		*/
		int deadlock = WaitForSingleObject(NATThreadMutex, 5 * 1000);
		if (deadlock == WAIT_TIMEOUT) {
			WWDEBUG_SAY(("FirewallHelperClass - Timeout waiting for firewall thread mutex\n"));
			fw_assert(deadlock != WAIT_TIMEOUT);
		} else {
			ReleaseMutex(NATThreadMutex);
		}
	}

	// Release the thread
	if (INVALID_HANDLE_VALUE != ThreadHandle) {
		CloseHandle(ThreadHandle);
		ThreadHandle = INVALID_HANDLE_VALUE;
	}
}



/***********************************************************************************************
 * FirewallHelperClass::Set_Firewall_Info -- Set firewall info from the game                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    last_behavior - how the firewall behaved last time we tried it                    *
 *           last_delta - the firewalls last known good port delta                             *
 *           port_pool - base port number to use when allocating temporary ports               *
 *           send_delay - value of send delay flag used for netgear firewalls.                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/20/2001 12:16PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Set_Firewall_Info(unsigned int last_behavior, int last_delta, unsigned short port_pool, bool send_delay, int confidence)
{
	if (port_pool) {
		SourcePortPool = port_pool;
	} else {
		if (SourcePortPool == 0) {
			SourcePortPool = PORT_POOL_MIN + FreeRandom.Get_Int(0, 32768);
		}
	}

	LastBehavior = (FirewallBehaviorType) last_behavior;
	LastSourcePortAllocationDelta = last_delta;
	SendDelay = send_delay;
	Confidence = confidence;
}



/***********************************************************************************************
 * FirewallHelperClass::Set_Firewall_Info -- Set firewall info from the game                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    last_behavior - how the firewall behaved last time we tried it                    *
 *           last_delta - the firewalls last known good port delta                             *
 *           port_pool - base port number to use when allocating temporary ports               *
 *           send_delay - value of send delay flag used for netgear firewalls.                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/20/2001 12:16PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Get_Firewall_Info(unsigned int &last_behavior, int &last_delta, unsigned short &port_pool, bool &send_delay, int &confidence) const
{
	port_pool = SourcePortPool;
	last_behavior = (unsigned int) LastBehavior;
	last_delta = LastSourcePortAllocationDelta;
	send_delay = SendDelay;
	confidence = Confidence;
}




/***********************************************************************************************
 *                                                                                             *
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
 *   8/7/2001 2:15PM ST : Created                                                              *
 *=============================================================================================*/
FirewallHelperClass::~FirewallHelperClass(void)
{
	Shutdown();

	CloseHandle(NATThreadMutex);
	CloseHandle(NATDataMutex);
}



/***********************************************************************************************
 * FirewallHelperClass::NAT_Thread_Start -- Thread start for NAT thread                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    This ptr                                                                          *
 *                                                                                             *
 * OUTPUT:   Undefined                                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 2:39PM ST : Created                                                              *
 *=============================================================================================*/
unsigned int __stdcall FirewallHelperClass::NAT_Thread_Start(void *thisptr)
{
	unsigned int thread_exit_code = 0;

	Register_Thread_ID(GetCurrentThreadId(), "Firewall thread");

#ifdef _MSC_VER
	__try {
#endif
		thread_exit_code = ((FirewallHelperClass*)thisptr)->NAT_Thread_Main_Loop();
#ifdef _MSC_VER
	} __except(Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {};
#endif

	Unregister_Thread_ID(GetCurrentThreadId(), "Firewall thread");

	return(thread_exit_code);
}


/***********************************************************************************************
 * FirewallHelperClass::NAT_Thread_Main_Loop -- Main loop for NAT thread                       *
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
 *   8/7/2001 2:42PM ST : Created                                                              *
 *=============================================================================================*/
unsigned int FirewallHelperClass::NAT_Thread_Main_Loop(void)
{
	/*
	** Take ownership of the thread mutex.
	*/
	int deadlock = WaitForSingleObject(NATThreadMutex, 10 * 1000);
	if (deadlock == WAIT_TIMEOUT) {
		WWDEBUG_SAY(("FirewallHelperClass - Timeout waiting for thread mutex\n"));
		fw_assert(deadlock != WAIT_TIMEOUT);
	}

	/*
	** Thread main loop.
	*/
	while (ThreadActive) {

		/*
		** Always do some sleeping here since this thread is a relatively low priority compared to stuff like running the
		** game engine.
		** If we are not doing anything much then sleep for longer.
		*/
		if (ThreadState == THREAD_IDLE && ThreadQueue.Count() == 0) {
			Sleep(50);
		} else {
			Sleep(0);
		}

		/*
		** Check for incoming private game options packets.
		*/
		Process_Game_Options();

		/*
		** Lock data access while we process each loop.
		*/
		ThreadLockClass locker(this);

		switch (ThreadState) {

			/*
			** Idle state, no activity required.
			*/
			case THREAD_IDLE:
				/*
				** See if there is a pending thread action to execute.
				*/
				if (ThreadQueue.Count()) {
					ThreadState = ThreadQueue[0].ThreadAction;
					ThreadEvent = ThreadQueue[0].ThreadEvent;
					//delete ThreadQueue[0];
					ThreadQueue.Delete(0);
				} else {
					if (WOLNATInterface.Am_I_Server()) {
						if (ClientQueue.Count()) {
							WWDEBUG_SAY(("Client with no thread action!\n"));
							Add_Thread_Action(THREAD_CONNECT_FIREWALL, INVALID_HANDLE_VALUE);
						}
					}
				}
				break;

			/*
			** Time to connect to another player.
			*/
			case THREAD_CONNECT_FIREWALL:
			{
				int connected = Negotiate_Port();
				if (connected == FW_RESULT_SUCCEEDED && !WOLNATInterface.Am_I_Server()) {
					WOLNATInterface.Set_Server_Negotiated_Address(&PlayersFirewallAddress);
				}
				Set_Client_Success(connected);
				ThreadState = THREAD_CONNECT_FIREWALL_DONE;
				break;
			}

			/*
			** Connection done, signal as required.
			*/
			case THREAD_CONNECT_FIREWALL_DONE:
				Set_Thread_Event();
				Send_Queue_States();
				ThreadState = THREAD_IDLE;
				break;

			/*
			** Detect the firewall settings.
			*/
			case THREAD_DETECT_FIREWALL:
				Behavior = Detect_Firewall_Behavior();
				WWDEBUG_SAY(("FirewallHelper: Behavior is = %08x\n", (unsigned int)Behavior));
				ThreadState = THREAD_DETECT_FIREWALL_DONE;
				Set_Thread_Event();
				break;

			/*
			** Detection done, signal as required.
			*/
			case THREAD_DETECT_FIREWALL_DONE:
				Set_Thread_Event();
				ThreadState = THREAD_IDLE;
				break;

			/*
			** Get the local chat connection address.
			*/
			case THREAD_GET_LOCAL_ADDRESS:
				Get_Local_Chat_Connection_Address();
				ThreadState = THREAD_IDLE;
				break;

			/*
			** We finished getting the local chat connection address.
			*/
			case THREAD_GET_LOCAL_ADDRESS_DONE:
				ThreadState = THREAD_IDLE;
				Set_Thread_Event();
				break;

			default:
				fw_assert(false);
				break;
		}

	}

	ReleaseMutex(NATThreadMutex);
	return(0);
}




/***********************************************************************************************
 * FirewallHelperClass::Reset -- Causes a re-detect of the firewall                            *
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
 *   3/29/01 1:04AM ST : Created                                                               *
 *=============================================================================================*/
void FirewallHelperClass::Reset(void)
{
	ThreadLockClass locker(this);
	Behavior = FIREWALL_TYPE_UNKNOWN;
}




/***********************************************************************************************
 * FirewallHelperClass::Reset_Server -- Reset server side variables                            *
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
 *   8/10/2001 3:23PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Reset_Server(void)
{
	ConnectionHistory.Delete_All();
	MangledPortHistory.Delete_All();
}



/***********************************************************************************************
 * FirewallHelperClass::Detect_Firewall -- See what our firewall is up to                      *
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
 *   3/15/01 6:47PM ST : Created                                                               *
 *=============================================================================================*/
void FirewallHelperClass::Detect_Firewall(HANDLE event)
{
	ThreadLockClass locker(this);

	if (Behavior == FIREWALL_TYPE_UNKNOWN) {
		Add_Thread_Action(THREAD_DETECT_FIREWALL, event);
	} else {
		SetEvent(event);
	}
}



/***********************************************************************************************
 * FirewallHelperClass::Connected_To_WWOnline_Server                                           *
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
 *   8/7/2001 10:02PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Connected_To_WWOnline_Server(void)
{
	/*
	** Don't need to go through this again if we already got the local chat connection address.
	*/
	if (!LocalChatConnectionAddress.Is_Valid()) {

		/*
		** Get the local chat connection address.
		*/
		ThreadLockClass locker(this);
		Add_Thread_Action(THREAD_GET_LOCAL_ADDRESS, INVALID_HANDLE_VALUE);
	}
}







/***********************************************************************************************
 * FHC::Get_Next_Temporary_Source_Port -- Get a throwaway source port for temporary use        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    number of ports in sequence to skip                                               *
 *                                                                                             *
 * OUTPUT:   port number                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:06PM ST : Created                                                              *
 *=============================================================================================*/
unsigned short FirewallHelperClass::Get_Next_Temporary_Source_Port(int skip)
{
	unsigned short return_port = (unsigned short) SourcePortPool;

	/*
	** Try max 256 ports until we find one we can bind to a socket.
	*/
	int tries = 256;
	if (skip == 0) {
		skip = 1;
	}

	while (tries--) {

		SourcePortPool += skip;
		return_port = (unsigned short) SourcePortPool;

		if (SourcePortPool > PORT_POOL_MAX) {
			SourcePortPool = PORT_POOL_MIN;
		}

		/*
		** Validate the port by trying to bind it to a socket.
		*/
		SocketHandlerClass *temp = new SocketHandlerClass;
		bool result = temp->Open(return_port, 0);
		delete temp;
		if (result) {
			return(return_port);
		}
	}

	return(return_port);

}



/***********************************************************************************************
 * FHC::Send_To_Mangler -- Send to the mangler from the specified socket handler               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address of mangler server                                                         *
 *           Socket handler to use when sending                                                *
 *           ID to insert into the packet                                                      *
 *           blitzme flag (obsolete, must be false)                                            *
 *                                                                                             *
 * OUTPUT:   True if sent OK                                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:47PM ST : Created                                                              *
 *=============================================================================================*/
bool FirewallHelperClass::Send_To_Mangler(IPAddressClass *address, SocketHandlerClass *socket_handler, unsigned int packet_id, bool blitzme)
{
	/*
	** Build the packet to send out.
	*/
	unsigned char packet_buf[512];
	int packet_size = Build_Mangler_Packet(packet_buf, socket_handler->Get_Incoming_Port(), packet_id, blitzme);

	WWDEBUG_SAY(("FirewallHelper: Sending from port %d to %s\n", (unsigned int)(socket_handler->Get_Incoming_Port()), address->As_String()));

	/*
	** Send it.
	*/
	socket_handler->Write(packet_buf, packet_size, address, address->Get_Port());
	return(true);
}




/***********************************************************************************************
 * FHC::Get_Mangler_Response -- Get the manglers response to a specific query                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Packet id of packet we are looking for                                            *
 *           Ptr to socket handler class to query for packets                                  *
 *           Timeout, 0=default                                                                *
 *           service_all - true if all sockets should be serviced                              *
 *                                                                                             *
 * OUTPUT:   Port the mangler saw this packet come from.                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:51PM ST : Created                                                              *
 *=============================================================================================*/
unsigned short FirewallHelperClass::Get_Mangler_Response(unsigned int packet_id, SocketHandlerClass *socket_handler, int time, bool all_service)
{
	/*
	** Locals.
	*/
	int peek_packet = 0;
	CnCPacketType *packet;
	unsigned char packet_buf[1024];
	int packet_size = sizeof(packet_buf);
	IPAddressClass address;
	unsigned int id;
	unsigned int timeout = TIMEGETTIME();
	unsigned char temp_address[4];
	unsigned short temp_port;

	/*
	** Asserts.
	*/
	fw_assert(socket_handler != NULL);

	if (time) {
		timeout += (unsigned int) time;
	} else {
		timeout += (unsigned int) TIMER_SECOND;
	}

	while (timeout > TIMEGETTIME()) {

		if (all_service) {
			SocketHandlerClass::Service_All();
		} else {
			socket_handler->Service();
		}

		/*
		** Get a copy of the global packet we want to examine. This doesn't remove it from the queue.
		*/
		int result = socket_handler->Peek(packet_buf, packet_size, &temp_address, &temp_port, peek_packet);

		/*
		** Check if we got a packet from the mangler.
		*/
    	if (result) {

			packet = (CnCPacketType*)packet_buf;

			if (packet->Packet.Command == NET_MANGLER_RESPONSE) {

				WWDEBUG_SAY(("FirewallHelper: Got NET_MANGLER_RESPONSE packet\n"));

				/*
				** We got a mangler packet, see if it's the one we are looking for.
				*/
#ifdef WWDEBUG
				int original_port = packet->Packet.ManglerData.OriginalPortNumber;
#endif //WWDEBUG
				id = packet->GHeader.Header.PacketID;

				if (packet_id == id) {

					/*
					** Looks good, lets get the mangled port number out.
					*/
					unsigned short mangled_port = packet->Packet.ManglerData.MangledPortNumber;
					WWDEBUG_SAY(("FirewallHelper: Mangler is seeing packets from port %d as coming from port %d\n", (unsigned int)original_port, (unsigned int)mangled_port));

					/*
					** Now the mangled address. This should never change.
					*/
					ExternalAddress.Set_Address(packet->Packet.ManglerData.MangledAddress);
					WWDEBUG_SAY(("FirewallHelper: Mangler is seeing our packets coming from address %s\n", ExternalAddress.As_String()));

					/*
					** Remove the packet from the queue.
					*/
					result = socket_handler->Read(packet_buf, packet_size, &temp_address, &temp_port, peek_packet);
					fw_assert(result != 0);
					return(mangled_port);
				}
			}
			peek_packet++;
		} else {
			peek_packet = 0;
		}

		Sleep(0);
	}
	return(0);
}




/***********************************************************************************************
 * FHC::Detect_Firewall_Behavior -- What is that wacky firewall doing to our packet headers?   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Firewall behavior                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:30PM ST : Created                                                              *
 *=============================================================================================*/
FirewallHelperClass::FirewallBehaviorType FirewallHelperClass::Detect_Firewall_Behavior(void)
{
	unsigned short mangler_port = 4321;
	char temp_mangler_name[128];
	unsigned int packet_id = 0x7f000000;

	/*
	** Well, we are going to need some manglers.
	*/
	static char _manglername[5][80] = {
		"mangler1.westwood.com",
		"mangler2.westwood.com",
		"mangler3.westwood.com",
		"mangler.westwood.com",
		""
	};

	unsigned char mangler_addresses[4][4];
	int num_mangler_addresses = 0;
	FirewallBehaviorType behavior = FIREWALL_TYPE_SIMPLE;
	IPAddressClass manglers[4];
	unsigned int timeout;
	int mangler_index_offset = 0;

	unsigned short source_ports[NUM_TEST_PORTS];
	unsigned short mangled_ports[NUM_TEST_PORTS];
	SocketHandlerClass *port_sockets[NUM_TEST_PORTS];
	int delta = 0;

	bool save_firewall_type = true;
	bool looks_good = true;

	/*
	** Figure out which mangler to use this time. Pick randomly from the available manglers.
	*/
	NumManglerServers = WOLNATInterface.Get_Num_Mangler_Servers();
	if (CurrentManglerServer == -1) {
		CurrentManglerServer = FreeRandom.Get_Int(0, NumManglerServers-1);
		WWDEBUG_SAY(("FirewallHelper - Using mangler server %d\n", CurrentManglerServer));
	}

	/*
	** If the user specified a particular port to use then we act as if there is no firewall.
	*/
	if (WOLNATInterface.Get_Force_Port()) {
		behavior = FIREWALL_TYPE_SIMPLE;
		WWDEBUG_SAY(("FirewallHelper: Source port %d specified by user\n", WOLNATInterface.Get_Force_Port()));

		if (SendDelay) {
			unsigned int addbehavior = FIREWALL_TYPE_NETGEAR_BUG;
			addbehavior |= (unsigned int)behavior;
			behavior = (FirewallBehaviorType) addbehavior;
			WWDEBUG_SAY(("FirewallHelper: Netgear bug specified by command line or SendDelay flag\n"));
		}
		return(behavior);
	}

	/*
	** Slave servers inherit their firewall info from the master server.
	*/
	if (SlaveMaster.Am_I_Slave()) {
		behavior = LastBehavior;
		SourcePortAllocationDelta = LastSourcePortAllocationDelta;
		ExternalAddress.Set_Address(WOLNATInterface.Get_Reg_External_IP(), WOLNATInterface.Get_Reg_External_Port());
		return(behavior);
	}

	timeout = TIMEGETTIME() + (5*TIMER_SECOND);
	WWDEBUG_SAY(("FirewallHelper: About to call gethostbyname for the mangler address\n"));
	int namenum = 0;

	/*
	** Convert the list of mangler servers to numeric IP addresses.
	**
	*/
	do {
		char *mangler_name_ptr = &_manglername[namenum][0];

		/*
		** Use the wolapi supplied mangler info if available.
		*/
		bool got_name = WOLNATInterface.Get_Mangler_Name_By_Index(namenum, temp_mangler_name);

		if (got_name) {
			mangler_name_ptr = temp_mangler_name;
			unsigned short servserv_port = WOLNATInterface.Get_Mangler_Port_By_Index(namenum);
			fw_assert(servserv_port != 0);
			if (servserv_port) {
				mangler_port = servserv_port;
			}
			WWDEBUG_SAY(("FirewallHelper - Using mangler server from servserv - address is %s ; %d\n", mangler_name_ptr, mangler_port));
		} else {
			WWDEBUG_SAY(("FirewallHelper - Using default mangler name for mangler %d\n", namenum));
		}

		namenum++;

		if (strlen(mangler_name_ptr) == 0) {
			break;
		}

		/*
		** Do the lookup.
		*/
		char temp_name[256];
		strcpy(temp_name, mangler_name_ptr);
		struct hostent *host_info = gethostbyname(temp_name);

		if (!host_info) {
			WWDEBUG_SAY(("FirewallHelper: gethostbyname failed! Error code %d\n", WSAGetLastError()));
			break;
		}

		/*
		** See if we already have that address in the list.
		*/
		bool found = false;
		for (int i=0 ; i<num_mangler_addresses ; i++) {
			if (memcmp(mangler_addresses[i], &host_info->h_addr_list[0][0], 4) == 0) {
				found = true;
				break;
			}
		}
		/*
		** Add the address in if we didn't find it.
		*/
		if (!found) {
			int m = num_mangler_addresses++;
			memcpy(&mangler_addresses[m][0], &host_info->h_addr_list[0][0], 4);
			WWDEBUG_SAY(("FirewallHelper: Found mangler address at %d.%d.%d.%d\n", mangler_addresses[m][0], mangler_addresses[m][1], mangler_addresses[m][2], mangler_addresses[m][3]));
		}

	} while (num_mangler_addresses < 4 && TIMEGETTIME() < timeout);


	/*
	** We need a certain number of manglers to be able to do the detection.
	*/
	fw_assert(num_mangler_addresses > 2);
	if (num_mangler_addresses < 3) {
		WWDEBUG_SAY(("FirewallHelper: Not enough manglers - returning default behavior\n"));
		if (Confidence > 0) {
			behavior = LastBehavior;
			SourcePortAllocationDelta = LastSourcePortAllocationDelta;
		}
		return(behavior);
	}

	/*
	** Convert the mangler addresses to IPAddressClass format.
	*/
	int i;
	for (i=0 ; i<num_mangler_addresses ; i++) {
		unsigned char addr[4];
		memcpy(addr, &mangler_addresses[i][0], 4);
		manglers[i].Set_Address(addr, mangler_port);
	}


	/*
	** OK, we have our manglers.
	**
	** First test, see if there is any port mangling at all.
	**
	**
	**
	*/

	/*
	** Get a spare port number and create a new socket to bind it to.
	*/
	unsigned short port = Get_Next_Temporary_Source_Port(0);
	SocketHandlerClass socket;
	if (!socket.Open(port, 4321)) {
		WWDEBUG_SAY(("FirewallHelper: Unable to open temp source port - returning default behavior\n"));
		return(behavior);
	}

	/*
	** Send to the mangler from this port until we get a response.
	*/
	timeout = TIMEGETTIME() + TIMER_SECOND * 6;
	unsigned short mangled_port = 0;
	while (TIMEGETTIME() < timeout && mangled_port == 0) {
		Send_To_Mangler(&manglers[0], &socket, packet_id);
		mangled_port = Get_Mangler_Response(packet_id, &socket);
	}

	if (mangled_port == 0) {
		/*
		** No response from mangler 0. That's bad. I guess we could try the other manglers.
		** We already tried the first one so skip that. Don't use the last one either since that will be needed later to detect
		** port per ip behavior.
		*/
		for (int m=1 ; m<num_mangler_addresses - 1 ; m++) {
			mangler_index_offset = m;
			timeout = TIMEGETTIME() + TIMER_SECOND * 3;
			while (TIMEGETTIME() < timeout && mangled_port == 0) {
				Send_To_Mangler(&manglers[mangler_index_offset], &socket, packet_id);
				mangled_port = Get_Mangler_Response(packet_id, &socket);
			}
		}
	}

	/*
	** See if we got no response or a non-mangled response.
	*/
	if (mangled_port == 0 || mangled_port == port) {
		if (mangled_port == port) {
			SourcePortAllocationDelta = 0;
			LastSourcePortAllocationDelta = 0;
			LastBehavior = FIREWALL_TYPE_SIMPLE;
			Confidence = 0;
		} else {
			WWASSERT(mangled_port == 0);
			if (Confidence) {
				SourcePortAllocationDelta = LastSourcePortAllocationDelta;
				return(LastBehavior);
			}
		}
		return(FIREWALL_TYPE_SIMPLE);
	}


	/*
	**
	** Second test. See if the ports are mangled differently for different destination IPs.
	**
	** We can use the spare socket from the last test and send to a different mangler.
	**
	*/

	/*
	** Send to the mangler from this port until we get a response.
	*/
	timeout = TIMEGETTIME() + (TIMER_SECOND * 6);
	unsigned short second_ip_mangled_port = 0;
	while (TIMEGETTIME() < timeout && second_ip_mangled_port == 0) {
		Send_To_Mangler(&manglers[mangler_index_offset + 1], &socket, packet_id+1);
		second_ip_mangled_port = Get_Mangler_Response(packet_id+1, &socket);
	};

	/*
	** We are done with this socket/port
	*/
	socket.Close();

	/*
	** See if we got no response or a non-mangled response.
	*/
	if (second_ip_mangled_port == 0 || second_ip_mangled_port == port) {
		WWDEBUG_SAY(("FirewallHelper: No response or response is unmangled - returning simple firewall behavior\n"));
		return(FIREWALL_TYPE_SIMPLE);
	}

	/*
	** Got a mangled port. Is it the same as the last one (mangles the same regardless of destination IP).
	*/
	if (mangled_port == second_ip_mangled_port) {
		behavior = FIREWALL_TYPE_DUMB_MANGLING;
		WWDEBUG_SAY(("FirewallHelper: Source ports mangled the same way regardless of destination IP\n"));
	} else {
		behavior = FIREWALL_TYPE_SMART_MANGLING;
		WWDEBUG_SAY(("FirewallHelper: Source ports mangled differently per destination IP\n"));
	}




	/*
	** Third test.
	**
	** This test tries to detect a pattern in the ports allocated by the NAT.
	** We use several source ports for this one.
	**
	*/

	/*
	** Try this whole thing a max of 3 times.
	*/
	int try_again;
	for (try_again = 0 ; try_again < 3 ; try_again++) {

		memset(source_ports, 0, sizeof(source_ports));
		memset(mangled_ports, 0, sizeof(source_ports));
		memset(port_sockets, 0, sizeof(port_sockets));

		/*
		** Open a socket for each source port.
		** We should use a non-linear set of source ports so we can detect the NAT32 relative offset
		** case.
		*/
		bool port_open_failed = false;
		for (i=0 ; i<NUM_TEST_PORTS ; i++) {
			source_ports[i] = Get_Next_Temporary_Source_Port(i);
			port_sockets[i] = new SocketHandlerClass;
			if (!port_sockets[i]->Open(source_ports[i], 4321)) {

				port_open_failed = true;

				/*
				** Close any spare ports we allocated already before we bail.
				*/
				for (int j=0 ; j<i ; j++) {
					if (port_sockets[j]) {
						delete port_sockets[j];
						port_sockets[j] = NULL;
					}
				}
				WWDEBUG_SAY(("FirewallHelper: Unable to open all source ports for allocation pattern test - returning default behavior\n"));
				break;	//return(behavior);
			}
		}

		if (port_open_failed) {
			continue;
		}


		/*
		** OK, lets get some numbers from the mangler.
		**
		** Keep sending from each port until we get a response to all the sends. There's a implied
		** delay between initial sends due to the timeout in Get_Mangler_Response.
		*/
		int num_responses = 0;
		packet_id = packet_id + 10;
		timeout = TIMEGETTIME() + (TIMER_SECOND * 12);

		while (TIMEGETTIME() < timeout && num_responses < NUM_TEST_PORTS) {
			for (i=0 ; i<NUM_TEST_PORTS ; i++) {
				if (mangled_ports[i] == 0) {
					Send_To_Mangler(&manglers[mangler_index_offset], port_sockets[i], packet_id+i);
					mangled_ports[i] = Get_Mangler_Response(packet_id+i, port_sockets[i], 0, true);
					if (mangled_ports[i] != 0) {
						num_responses++;
					}
				}
			}
		}

		/*
		** Close down those sockets - we are finished with them.
		*/
		for (int j=0 ; j<i ; j++) {
			if (port_sockets[j]) {
				delete port_sockets[j];
				port_sockets[j] = NULL;
			}
		}

		/*
		** We need at least 4 responses to be sure of the port allocation scheme.
		*/
		if (num_responses < 4) {
			if (LastSourcePortAllocationDelta != 0 && (int)LastBehavior > (int)FIREWALL_TYPE_SIMPLE) {
				/*
				** If the delta we got last time we played looks good then use that.
				*/
				SourcePortAllocationDelta = LastSourcePortAllocationDelta;
				WWDEBUG_SAY(("FirewallHelper: Unable to verify responses from 4 mangler servers - returning default behavior\n"));
				return(behavior);
			}
			/*
			** Try again.
			*/
			continue;
		}


		bool relative_delta = false;
		delta = Get_NAT_Port_Allocation_Scheme(num_responses, source_ports, mangled_ports, relative_delta, looks_good);

		if (delta) {

			/*
			** Hey, we got it!
			*/
			unsigned int addbehavior = 0;
			if (relative_delta) {
				addbehavior = (unsigned int)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION;
			} else {
				addbehavior = (unsigned int)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION;
			}
			addbehavior |= (unsigned int)behavior;
			behavior = (FirewallBehaviorType) addbehavior;

			SourcePortAllocationDelta = delta;
			if (!looks_good) {
				save_firewall_type = false;
			}
			break;
		} else {
			if (LastSourcePortAllocationDelta != 0 && (int)LastBehavior > (int)FIREWALL_TYPE_SIMPLE) {
				/*
				** If the delta we got last time we played looks good then use that.
				*/
				SourcePortAllocationDelta = LastSourcePortAllocationDelta;
				behavior = LastBehavior;
				save_firewall_type = false;
				break;
			}
		}
	}

	/*
	** See if we screwed up.
	*/
	if (try_again == 3) {
		/*
		** We know it mangles differently per IP but we don't know how. Make something up.
		*/
		SourcePortAllocationDelta = 1;
		save_firewall_type = false;
	}


	/*
	** Test to see if the NAT mangles differently per destination port at the same IP.
	**
	**
	*/
	if ((behavior & FIREWALL_TYPE_SMART_MANGLING) != 0) {

		if ((behavior & FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION) != 0) {

			WWDEBUG_SAY(("FirewallHelper: Testing to see if the NAT mangles differently per destination port at the same IP\n"));

			/*
			** We need 2 source ports for this.
			*/
			unsigned short port1 = Get_Next_Temporary_Source_Port(0);
			SocketHandlerClass socket1;
			if (!socket1.Open(port1, 4321)) {
				return(behavior);
			}

			unsigned short port2 = Get_Next_Temporary_Source_Port(0);
			SocketHandlerClass socket2;
			if (!socket2.Open(port2, 4321)) {
				return(behavior);
			}

			/*
			** Get a reference port.
			*/
			timeout = TIMEGETTIME() + (TIMER_SECOND * 4);
			mangled_port = 0;
			packet_id = packet_id + 10;

			/*
			** Wait for a response.
			*/
			while (TIMEGETTIME() < timeout && mangled_port == 0) {
				Send_To_Mangler(&manglers[mangler_index_offset], &socket1, packet_id);
				mangled_port = Get_Mangler_Response(packet_id, &socket1);
			}

			if (mangled_port == 0) {
				return(behavior);
			}

			/*
			** Send out to a different port at that IP.
			** We won't get a response for this.
			*/
			IPAddressClass addr = manglers[mangler_index_offset];
			addr.Set_Port(addr.Get_Port() + 1);
			Send_To_Mangler(&addr, &socket1, packet_id);
			Send_To_Mangler(&addr, &socket1, packet_id);
			Send_To_Mangler(&addr, &socket1, packet_id);

			/*
			** We can't get a response from a different destination port so the only way to detect
			** this behavior is to check the next mangled port allocation to see if it's double
			** what we would normally expect.
			*/
			packet_id++;
			unsigned short new_mangled_port = 0;
			timeout = TIMEGETTIME() + (TIMER_SECOND * 4);
			while (TIMEGETTIME() < timeout && new_mangled_port == 0) {
				Send_To_Mangler(&manglers[mangler_index_offset], &socket2, packet_id);
				new_mangled_port = Get_Mangler_Response(packet_id, &socket2);
			}

			if (new_mangled_port != 0) {
				if (new_mangled_port != mangled_port + SourcePortAllocationDelta) {
					WWDEBUG_SAY(("FirewallHelper: NAT uses different source ports for different destination ports\n"));

					unsigned int addbehavior = 0;
					addbehavior = (unsigned int)FIREWALL_TYPE_DESTINATION_PORT_DELTA;
					addbehavior |= (unsigned int)behavior;
					behavior = (FirewallBehaviorType) addbehavior;
				} else {
					fw_assert(new_mangled_port == mangled_port + SourcePortAllocationDelta);
					if (new_mangled_port == mangled_port + SourcePortAllocationDelta) {
						WWDEBUG_SAY(("FirewallHelper: NAT uses the same source port for different destination ports\n"));
					} else {
						WWDEBUG_SAY(("FirewallHelper: Unable to complete destination port mangling test\n"));
						fw_assert(false);
					}
				}
			}
		} else {
			/*
			** NAT32 uses different mangled source ports for different destination ports.
			*/
			unsigned int addbehavior = 0;
			addbehavior = (unsigned int)FIREWALL_TYPE_DESTINATION_PORT_DELTA;
			addbehavior |= (unsigned int)behavior;
			behavior = (FirewallBehaviorType) addbehavior;
		}
	}

	/*
	** See if the user specified a netgear firewall - that will save us the trouble of detecting it.
	*/
	if (SendDelay) {
		unsigned int addbehavior = FIREWALL_TYPE_NETGEAR_BUG;
		addbehavior |= (unsigned int)behavior;
		behavior = (FirewallBehaviorType) addbehavior;
		WWDEBUG_SAY(("FirewallHelper: Netgear bug specified by command line or SendDelay flag\n"));
	}

	/*
	** Remember this firewall type for next time.
	*/
	if (save_firewall_type) {
		LastBehavior = behavior;
		LastSourcePortAllocationDelta = SourcePortAllocationDelta;
		Confidence = 0;
	} else {

		/*
		** If we have high confidence in the last settings we detected then we might want to use those in preference.
		*/
		if (LastBehavior == behavior && LastSourcePortAllocationDelta != SourcePortAllocationDelta) {
			if (Confidence > 0) {

				/*
				** Only do this max 3 times before accepting the new delta.
				*/
				Confidence = (Confidence / 3) - 1;
				SourcePortAllocationDelta = LastSourcePortAllocationDelta;
			}
		}
	}

	return(behavior);
}





/***********************************************************************************************
 * FHC::Get_NAT_Port_Allocation_Scheme -- Find out how a NAT is allocating ports               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of ports we should analyze                                                 *
 *           List of original port numbers                                                     *
 *           List of mangled port numbers                                                      *
 *           relative_delta (out) Is the delta relative to the original port number?           *
 *           looks_good (out) Do all the values point to the same delta?                       *
 *                                                                                             *
 * OUTPUT:   Port allocation delta                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 4:45PM ST : Created                                                               *
 *=============================================================================================*/
int FirewallHelperClass::Get_NAT_Port_Allocation_Scheme(int num_ports, unsigned short *original_ports, unsigned short *mangled_ports, bool &relative_delta, bool &looks_good)
{
	fw_assert(num_ports > 3);

	WWDEBUG_SAY(("FirewallHelper: Looking for port allocation pattern in original_ports %d, %d, %d, %d\n", original_ports[0], original_ports[1], original_ports[2], original_ports[3]));

	/*
	** Sort the mangled ports into order - should be easier to detect patterns.
	** Stupid bubble sort will do. original_ports may be out of oder after the sort.
	*/
	for (int x=0 ; x<num_ports ; x++) {
		for (int y=0 ; y<num_ports-1 ; y++) {
			if (mangled_ports[y] > mangled_ports[y+1]) {
				int temp = mangled_ports[y];
				mangled_ports[y] = mangled_ports[y+1];
				mangled_ports[y+1] = temp;
				temp = original_ports[y];
				original_ports[y] = original_ports[y+1];
				original_ports[y+1] = temp;
			}
		}
	}

	/*
	** Now start looking for patterns in the port numbers. Possible patterns include.
	**
	** Incremental. Port numbers are allocated incrementally.
	** Every 'n' ports. NAT adds 'n' port numbers when allocating ports.
	**
	** Also, schemes may be absolute or relative to the original port number.
	*/

	/*
	** 1. Check for absolute sequential allocation.
	*/
	if (mangled_ports[1] - mangled_ports[0] == 1) {
		if (mangled_ports[2] - mangled_ports[1] == 1) {
			if (mangled_ports[3] - mangled_ports[2] == 1) {
				WWDEBUG_SAY(("FirewallHelper: Incremental port allocation detected\n"));
				relative_delta = false;
				looks_good = true;
				return(1);
			}
		}
	}

	/*
	** 2. Check for semi sequential.
	*/
	if (mangled_ports[1] - mangled_ports[0] == 2) {
		if (mangled_ports[2] - mangled_ports[1] == 2) {
			if (mangled_ports[3] - mangled_ports[2] == 2) {
				WWDEBUG_SAY(("FirewallHelper: Semi-incremental port allocation detected\n"));
				relative_delta = false;
				looks_good = true;
				return(2);
			}
		}
	}

	int diff1 = mangled_ports[1] - mangled_ports[0];
	int diff2 = mangled_ports[2] - mangled_ports[1];
	int diff3 = mangled_ports[3] - mangled_ports[2];


	/*
	** 3. Check for absolute scheme skipping 'n' ports.
	*/
	if (diff1 == diff2 && diff2 == diff3) {
		WWDEBUG_SAY(("FirewallHelper: Looks good for absolute allocation sequence delta of %d\n", diff1));
		relative_delta = false;
		looks_good = true;
		return(diff1);
	}

	if (diff1 == diff2) {
		WWDEBUG_SAY(("FirewallHelper: Probable absolute allocation sequence delta of %d\n", diff1));
		relative_delta = false;
		looks_good = false;
		return(diff1);
	}

	if (diff2 == diff3) {
		WWDEBUG_SAY(("FirewallHelper: Probable absolute allocation sequence delta of %d\n", diff2));
		relative_delta = false;
		looks_good = false;
		return(diff2);
	}




	/*
	** Insert more tests here if we can think of any!!!!!
	*/



	/*
	** 4. Check for relative scheme skipping 'n' ports. NAT32 behaves this way, it skips 100 ports
	** each time.
	*/
	for (int i=0 ; i<num_ports ; i++) {
		mangled_ports[i] -= original_ports[i];
	}

	diff1 = mangled_ports[1] - mangled_ports[0];
	diff2 = mangled_ports[2] - mangled_ports[1];
	diff3 = mangled_ports[3] - mangled_ports[2];

	/*
	** Look for a linear pattern.
	*/
	if (diff1 == diff2 && diff2 == diff3) {
		/*
		** Return a -ve result to indicate that port mangling is relative.
		*/
		WWDEBUG_SAY(("FirewallHelper: Looks good for a relative port range delta of %d\n", diff1));
		relative_delta = true;
		looks_good = true;
		return(diff1);
	}

	/*
	** Look for a broken pattern. Maybe the NAT skipped a whole range.
	*/
	if (diff1 == diff2 || diff1 == diff3) {
		WWDEBUG_SAY(("FirewallHelper: Detected probable broken relative port range delta of %d\n", diff1));
		relative_delta = true;
		looks_good = false;
		return(diff1);
	}

	if (diff2 == diff3) {
		WWDEBUG_SAY(("FirewallHelper: Detected probable broken relative port range delta of %d\n", diff2));
		relative_delta = true;
		looks_good = false;
		return(diff2);
	}


	/*
	** Aw hell, I don't know what it is.
	*/
	looks_good = false;
	relative_delta = false;
	return(0);
}






/***********************************************************************************************
 * FHC::Get_Firewall_Hardness -- How hard is it to connect to this firewall                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Firewall behavior                                                                 *
 *                                                                                             *
 * OUTPUT:   Hardness                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/01 11:43AM ST : Created                                                              *
 *=============================================================================================*/
int FirewallHelperClass::Get_Firewall_Hardness(FirewallBehaviorType behavior)
{

	int hardness = 0;


	unsigned int fw = (unsigned int) behavior;

	if (((unsigned int)FIREWALL_TYPE_SIMPLE & fw) != 0) {
		hardness++;
	}

	if (((unsigned int)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		hardness += 2;
	}

	if (((unsigned int)FIREWALL_TYPE_SMART_MANGLING & fw) != 0) {
		hardness += 3;
	}

	if (((unsigned int)FIREWALL_TYPE_NETGEAR_BUG & fw) != 0) {
		hardness += 10;
	}

	if (((unsigned int)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION & fw) != 0) {
		hardness += 1;
	}

	if (((unsigned int)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION & fw) != 0) {
		hardness += 2;
	}

	return(hardness);
}





/***********************************************************************************************
 * FHC::Get_Firewall_Retries -- How many retries is it likely to take before we connect?       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Firewall behavior                                                                 *
 *                                                                                             *
 * OUTPUT:   Hardness                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/01 11:43AM ST : Created                                                              *
 *=============================================================================================*/
int FirewallHelperClass::Get_Firewall_Retries(FirewallBehaviorType behavior)
{

	int retries = 2;


	unsigned int fw = (unsigned int) behavior;

	if (((unsigned int)FIREWALL_TYPE_SIMPLE & fw) != 0) {
		retries++;
	}

	if (((unsigned int)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		retries += 1;
	}

	if (((unsigned int)FIREWALL_TYPE_SMART_MANGLING & fw) != 0) {
		retries += 1;
	}

	if (((unsigned int)FIREWALL_TYPE_NETGEAR_BUG & fw) != 0) {
		//retries += 10;
	}

	if (((unsigned int)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION & fw) != 0) {
		//retries += 1;
	}

	if (((unsigned int)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION & fw) != 0) {
		retries += 5;
	}

	return(retries);
}





/***********************************************************************************************
 * FHC::Get_Next_Mangled_Source_Port -- How will the firewall mangle our next source port      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Source port that we will use                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/01 3:30PM ST : Created                                                               *
 *=============================================================================================*/
unsigned short FirewallHelperClass::Get_Next_Mangled_Source_Port(unsigned short source_port)
{

	/*
	** Locals.
	*/
	static unsigned int _packet_id = 0x7f100000;
	unsigned int timeout;
	int return_port = source_port;
	SocketHandlerClass socket;

	/*
	** If our firewall is stupid then just return the source port.
	*/
	unsigned int fw = (unsigned int) Behavior;
	if (fw == 0) {
		return(source_port);
	}
	if (((unsigned int)FIREWALL_TYPE_SIMPLE & fw) != 0) {
		return(source_port);
	}

	/*
	** If our NAT uses the same mangled source port regardless of the dest IP then we can use any previous connection to a different IP.
	*/
	if (WOLNATInterface.Am_I_Server() && ((unsigned int)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		for (int h=0 ; h<ConnectionHistory.Count() ; h++) {

			/*
			** Make sure it isn't my address (should never be, even with players from behind the same firewall).
			*/
			if (!ConnectionHistory[h].Is_IP_Equal(ExternalAddress)) {
				WWDEBUG_SAY(("FirewallHelper - same port regardless of dest ip - using port %d from connection to %s ; %d\n", (unsigned int) MangledPortHistory[h], ConnectionHistory[h].As_String()));
				return(MangledPortHistory[h]);
			}
		}
	}


	/*
	** Get the address of a mangler server.
	*/
	unsigned short mangler_port = 4321;
	char mangler_name[256];
	strcpy(mangler_name, "mangler2.westwood.com");
	unsigned char maddress[4];

	if (NumManglerServers > 0) {
		fw_assert(CurrentManglerServer >= 0 && CurrentManglerServer < NumManglerServers);

		bool got_name = WOLNATInterface.Get_Mangler_Name_By_Index(CurrentManglerServer, mangler_name);

		if (got_name) {
			unsigned short servserv_port = WOLNATInterface.Get_Mangler_Port_By_Index(CurrentManglerServer);
			fw_assert(servserv_port != 0);
			if (servserv_port) {
				mangler_port = servserv_port;
			}
			WWDEBUG_SAY(("FirewallHelper - Using mangler server from servserv - address is %s ; %d\n", mangler_name, mangler_port));
		} else {
			WWDEBUG_SAY(("FirewallHelper - Using default mangler name for mangler %d\n", CurrentManglerServer));
		}
	}

	/*
	** Get the address to send the packets to.
	*/
	//WWDEBUG_SAY (("Firewall helper - About to call gethostbyname for %s\n", mangler_name));
	struct hostent *host_info = gethostbyname(mangler_name);

	if (!host_info) {
		WWDEBUG_SAY(("FirewallHelper - gethostbyname failed! Error code %d\n", WSAGetLastError()));
		return(source_port);
	}
	memcpy(maddress, &host_info->h_addr_list[0][0], 4);
	//WWDEBUG_SAY(("FirewallHelper: Mangler address is %d.%d.%d.%d ; %d\n", maddress[0], maddress[1], maddress[2], maddress[3], (int)mangler_port));
	IPAddressClass mangler_address;
	mangler_address.Set_Address(maddress, mangler_port);
	//WWDEBUG_SAY(("FirewallHelper: Mangler address is %s\n", mangler_address.As_String()));

	//WWDEBUG_SAY(("FirewallHelper: fw = %08x\n", fw));

	/*
	** Send to the mangler to establish a reference port.
	*/
	unsigned short port = Get_Next_Temporary_Source_Port(0);
	fw_assert(port != source_port);

	if (!socket.Open(port, 4321)) {
		return(source_port);
	}

	WWDEBUG_SAY(("FirewallHelper - Getting mangling for temp source port %d\n", (int)port));

	/*
	** Send to the mangler from this port until we get a response.
	*/
	timeout = TIMEGETTIME() + (TIMER_SECOND * 3);
	unsigned short mangled_port = 0;
	while (TIMEGETTIME() < timeout && mangled_port == 0) {
		Send_To_Mangler(&mangler_address, &socket, _packet_id);
		mangled_port = Get_Mangler_Response(_packet_id, &socket, TIMER_SECOND / 2);
		if (mangled_port != 0) {
			WWDEBUG_SAY(("FirewallHelper - Mangled port = %d\n", (int)mangled_port));
		}
	}

	_packet_id++;

	fw_assert(mangled_port != 0 && mangled_port != source_port);

	if (mangled_port == 0) {
		/*
		** We failed to get a response from the mangler, try a different one next retry.
		*/
		CurrentManglerServer++;
		if (CurrentManglerServer >= NumManglerServers) {
			CurrentManglerServer = 0;
		}

		WWDEBUG_SAY(("FirewallHelper: Couldn't find mangled port, returning source port\n"));
		return(source_port);
	}

	/*
	** Our new reference port is 'mangled port'. If we don't care about IP then we are done.
	*/
	if (((unsigned int)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		WWDEBUG_SAY(("FirewallHelper - Dumb firewall, returning next mangled port as %d\n", mangled_port));
		return(mangled_port);
	}


	/*
	** Apply our known delta to the mangled port.
	*/
	if (((unsigned int)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION & fw) != 0) {

		/*
		** Simple port allocation.
		*/
		return_port = (int)mangled_port + SourcePortAllocationDelta;
	} else {

		/*
		** Rats. It's a relative mangler. This is much harder. Damn NAT32 guy.
		*/
		if (SourcePortAllocationDelta == 100) {

			/*
			** Special NAT32 section.
			**
			** NAT32 mangles source UDP port by adding 1700 + 100 * internal table index
			**
			** To get the current index we need to take the mangled port and subtract 1700 and the source port
			*/
			return_port = mangled_port - port;
			return_port -= 1700;

			return_port += SourcePortAllocationDelta;
			return_port += source_port;
			return_port += 1700;
		} else {
			WWASSERT(SourcePortAllocationDelta != 0);
			if (SourcePortAllocationDelta == 0) {
				/*
				** This should never happen....
				*/
				return_port = mangled_port;
			} else {

				return_port = mangled_port / SourcePortAllocationDelta;
				return_port = return_port * SourcePortAllocationDelta;

				return_port += (source_port % SourcePortAllocationDelta);
				return_port += SourcePortAllocationDelta;
			}
		}
	}

	/*
	** This bit is probably doomed.
	*/
	if (return_port > 65535) {
		return_port -= 65535;
	}
	if (return_port < 1024) {
		return_port += 1024;
	}

	WWDEBUG_SAY(("FirewallHelper - Returning next mangled port as %d\n", return_port));

	return static_cast<unsigned short>(return_port);
}



/***********************************************************************************************
 * NatterClass::Build_Mangler_Packet -- Build a packet to send to the mangler.                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Buffer to build packet in                                                         *
 *           Port to set into the packets Port area                                            *
 *           Packet ID.                                                                        *
 *           Blitzme flag (shouldn't be used)                                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/6/2001 1:11PM ST : Created                                                              *
 *=============================================================================================*/
int FirewallHelperClass::Build_Mangler_Packet(unsigned char *buffer, unsigned short port, unsigned int packet_id, bool blitzme)
{
	/*
	** Asserts.
	*/
	fw_assert(buffer != NULL);
	fw_assert(blitzme == false);

	/*
	** Need the buffer to exist.
	*/
	if (buffer == NULL) {
		return(0);
	}

	/*
	** Cast the buffer to a CnCPacketType.
	*/
	CnCPacketType *packet = (CnCPacketType*)buffer;

	/*
	** Clear out the buffer prior to setting fields.
	*/
	memset(packet, 0, sizeof(CnCPacketType));

	/*
	** Set the required Global Header firelds.
	*/
	packet->GHeader.Header.MagicNumber = GLOBAL_MAGICNUM;
	packet->GHeader.Header.Code = PACKET_DATA_NOACK;
	packet->GHeader.Header.ForwardTo = 0;
	packet->GHeader.Header.PacketID = packet_id;
	packet->GHeader.ProductID = COMMAND_AND_CONQUER_RA2;

	/*
	** Set the Global Packet fields.
	*/
	packet->Packet.Command = NET_MANGLER_REQUEST;
	strcpy(packet->Packet.Name, "ren");
	packet->Packet.ManglerData.MangledPortNumber = port;
	packet->Packet.ManglerData.OriginalPortNumber = port;
	packet->Packet.ManglerData.BlitzMe = 0;

	/*
	** Blitzme field is obsolete and should never be used.
	*/
	if (blitzme) {
		packet->Packet.ManglerData.BlitzMe = 1;
	}

	/*
	** Return success.
	*/
	return(sizeof(CnCPacketType));
}





/***********************************************************************************************
 * FirewallHelperClass::Add_Thread_Action -- Add a new action to the thread control FIFO       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Thread action                                                                     *
 *           Notification event                                                                *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/9/2001 9:06PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::Add_Thread_Action(int thread_action, HANDLE thread_event)
{
	ThreadLockClass locker(this);
	ThreadActionClass thread;
	thread.ThreadAction = thread_action;
	thread.ThreadEvent = thread_event;
	ThreadQueue.Add(thread);
}



/***********************************************************************************************
 * FirewallHelperClass::Set_Thread_Event -- Set the event associated with action completion    *
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
 *   8/9/2001 9:09PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::Set_Thread_Event(void)
{
	ThreadLockClass locker(this);
	if (ThreadEvent != INVALID_HANDLE_VALUE) {
		SetEvent(ThreadEvent);
		ThreadEvent = INVALID_HANDLE_VALUE;
	}
}


/***********************************************************************************************
 * FirewallHelperClass::Talk_To_New_Player -- A new player wants to negotiate a NAT port       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    User struct for player                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 8:31PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::Talk_To_New_Player(WOL::User *user)
{
	/*
	** We are only reading here - no need to get exclusive access.
	*/
	//ThreadLockClass locker(this);

	WWASSERT(PTheGameData != NULL);

	/*
	** Fill in an invitation options packet to send to the guest.
	*/
	WOLNATInterfaceClass::PrivateGameOptionsStruct options;
	strcpy(options.NATOptionsPrefix, "NAT:");
	options.Option = WOLNATInterfaceClass::OPTION_INVITE_PORT_NEGOTIATION;
	sprintf(options.OptionData.Invitation.LocalIP, "%08x,", (unsigned int) LocalChatConnectionAddress.Get_Address());
	sprintf(options.OptionData.Invitation.LocalPort, "%04x,", The_Game()->Get_Port());
	sprintf(options.OptionData.Invitation.ExternalIP, "%08x,", (unsigned int) ExternalAddress.Get_Address());
	sprintf(options.OptionData.Invitation.FirewallType, "%08x,", Get_Raw_Firewall_Behavior());
	sprintf(options.OptionData.Invitation.Queued, "%04x", ClientQueue.Count());

	/*
	** Send it.
	*/
	WOLNATInterface.Send_Private_Game_Options(user, (char*)&options);
}



/***********************************************************************************************
 * FirewallHelperClass::Process_Game_Options -- Process private game options packets           *
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
 *   8/9/2001 10:01PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Process_Game_Options(void)
{
	/*
	** Locals.
	*/
	WOL::User user;
	char options_buffer[OPTIONS_STAGING_BUFFER_SIZE];
	unsigned int addr_ip = 0;
	unsigned short addr_port = 0;
	unsigned int firewall = 0;

	ThreadLockClass locker(this);

	/*
	** See if there are any pending game options packets.
	*/
	if (WOLNATInterface.Get_Private_Game_Options(&user, options_buffer, OPTIONS_STAGING_BUFFER_SIZE)) {

		/*
		** Cast the options buffer to a more manageable type.
		*/
		WOLNATInterfaceClass::PrivateGameOptionsStruct *options = (WOLNATInterfaceClass::PrivateGameOptionsStruct*) options_buffer;

		/*
		** Get the option type.
		*/
		switch (options->Option) {

			/*
			** This option represents a game server inviting us to negotiate a connection through firewalls. It will contain his
			** local IP and port. This is a client side message only.
			*/
			case WOLNATInterfaceClass::OPTION_INVITE_PORT_NEGOTIATION:

				WWDEBUG_SAY(("FirewallHelper - Got OPTION_INVITE_PORT_NEGOTIATION\n"));
				fw_assert(!WOLNATInterface.Am_I_Server());

				if (!WOLNATInterface.Am_I_Server()) {

					/*
					** Pull out the players info from the packet.
					*/
					sscanf(options->OptionData.Invitation.LocalIP, "%08x", &addr_ip);
					sscanf(options->OptionData.Invitation.LocalPort, "%04hx", &addr_port);
					PlayersLocalAddress.Set_Address(addr_ip, addr_port);
					sscanf(options->OptionData.Invitation.ExternalIP, "%08x", &addr_ip);
					PlayersExternalAddress.Set_Address(addr_ip, addr_port);
					sscanf(options->OptionData.Invitation.FirewallType, "%08x", (unsigned int*)(&PlayersFirewallType));
					sscanf(options->OptionData.Invitation.Queued, "%04x", &QueuedPlayers);
					strcpy(PlayersName, (char*)user.name);
					PlayerAsUser = user;
					if (QueueNotifyPtr) {
						*QueueNotifyPtr = QueuedPlayers;
					}

					WWDEBUG_SAY(("FirewallHelper - Received port negotiation invitation. %d players in the queue ahead of me\n", QueuedPlayers));
					WWDEBUG_SAY(("FirewallHelper - Server is %s. Local addr = %s, ", PlayersName, PlayersLocalAddress.As_String()));
					WWDEBUG_SAY(("external addr = %s, firewall = %08x\n", PlayersExternalAddress.As_String(), (unsigned int) PlayersFirewallType));

					/*
					** Wait for queue notification.
					*/
					if (QueuedPlayers == 0) {
						QueuedPlayers = -1;
					}

					/*
					** Respond to the server with my info.
					*/
					ClientPort = WOLNATInterface.Get_Next_Client_Port();
					WOLNATInterfaceClass::PrivateGameOptionsStruct send_options;
					strcpy(send_options.NATOptionsPrefix, "NAT:");
					send_options.Option = WOLNATInterfaceClass::OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION;
					sprintf(send_options.OptionData.Accept.LocalIP, "%08x,", (unsigned int) LocalChatConnectionAddress.Get_Address());
					sprintf(send_options.OptionData.Accept.LocalPort, "%04x,", ClientPort);
					sprintf(send_options.OptionData.Accept.ExternalIP, "%08x,", (unsigned int) ExternalAddress.Get_Address());
					sprintf(send_options.OptionData.Accept.FirewallType, "%08x", Get_Raw_Firewall_Behavior());

					/*
					** Send it.
					*/
					WOLNATInterface.Send_Private_Game_Options(&user, (char*)&send_options);

					/*
					** Go into connect mode.
					** Don't do this until we get a confirmation of our queue position.
					*/
					Add_Thread_Action(THREAD_CONNECT_FIREWALL, INVALID_HANDLE_VALUE);
				}
				break;



			/*
			** This option represents a game client accepting our invitation to negotiate a connection through firewalls. It
			** will contain his local IP and port. This is a server side message only.
			*/
			case WOLNATInterfaceClass::OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION:

				WWDEBUG_SAY(("FirewallHelper - Got OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION\n"));

				fw_assert(WOLNATInterface.Am_I_Server());

				if (WOLNATInterface.Am_I_Server()) {

					/*
					** Pull out the players info from the packet.
					*/
					unsigned int ext_ip = 0;
					sscanf(options->OptionData.Accept.LocalIP, "%08x", &addr_ip);
					sscanf(options->OptionData.Accept.LocalPort, "%04hx", &addr_port);
					sscanf(options->OptionData.Accept.ExternalIP, "%08x", &ext_ip);
					sscanf(options->OptionData.Accept.FirewallType, "%08x", &firewall);

					/*
					** Create a queue entry for the player.
					*/
					ClientStruct *client = new ClientStruct;
					strcpy(client->Name, (char*)user.name);
					client->LocalAddress.Set_Address(addr_ip, addr_port);
					client->ExternalAddress.Set_Address(ext_ip, addr_port);
					client->FirewallType = (FirewallBehaviorType) firewall;
					client->User = user;

					/*
					** Make sure this client isn't in the client list already (Should never happen but...)
					*/
					if (Remove_Player_From_Negotiation_Queue(client->Name)) {
						WWDEBUG_SAY(("FirewallHelper OPTION_ACCEPT_PORT_NEGOTIATION_INVITATION - Removed %s from ClientQueue\n", client->Name));
					}

					/*
					** Add this player to the negotiation queue.
					*/
					ClientQueue.Add(client);
					WWDEBUG_SAY(("FirewallHelper - Got client accept from %s. Local addr = %s,", client->Name, client->LocalAddress.As_String()));
					WWDEBUG_SAY((" external addr = %s, firewall = %08x\n", client->ExternalAddress.As_String(), (unsigned int) firewall));

					Add_Thread_Action(THREAD_CONNECT_FIREWALL, INVALID_HANDLE_VALUE);
				}
				break;

			/*
			** Handle connection result message.
			**
			*/
			case WOLNATInterfaceClass::OPTION_CONNECTION_RESULT:
			{
				int result = options->OptionData.ConnectionResult.Result[0] - 'a';
				WWDEBUG_SAY(("FirewallHelper - Got OPTION_CONNECTION_RESULT %d from %s\n", result, options->OptionData.ConnectionResult.Name));
				if (stricmp(PlayersName, options->OptionData.ConnectionResult.Name) == 0) {
					PlayersConnectionResult = result;
					unsigned int port;
					sscanf(options->OptionData.ConnectionResult.Port, "%04x", &port);
					PlayersConnectionResultPort = (unsigned short) port;
					if (WOLNATInterface.Am_I_Server()) {
						LastOptionsFromClient = TIMEGETTIME();
					}
				}
				break;
			}


			/*
			** Other player is telling us what port to use when talking to him.
			*/
			case WOLNATInterfaceClass::OPTION_PORT_NOTIFICATION:
			{
				WWDEBUG_SAY(("FirewallHelper - Got OPTION_PORT_NOTIFICATION from %s\n", options->OptionData.Port.Name));
				unsigned int port;
				sscanf(options->OptionData.Port.MangledPort, "%04x", &port);
				WWDEBUG_SAY(("FirewallHelper - Port is %d\n", port));
				//fw_assert(port >= 1024 && port < 65536);

				//if (port >= 1024 && port < 65536) {
					if (stricmp(PlayersName, options->OptionData.Port.Name) == 0) {
						PlayersMangledPort = (unsigned short) port;
						if (WOLNATInterface.Am_I_Server()) {
							LastOptionsFromClient = TIMEGETTIME();
						}
					}
				//}
				break;
			}


			/*
			** Server is telling us how close to the front of the queue we are.
			*/
			case WOLNATInterfaceClass::OPTION_QUEUE_STATE:
			{
				WWDEBUG_SAY(("FirewallHelper - Got OPTION_QUEUE_STATE\n"));
				int queue;
				sscanf(options->OptionData.QueueState.Position, "%02x", &queue);
				WWDEBUG_SAY(("FirewallHelper - Queue position is %d\n", queue));
				QueuedPlayers = queue;
				if (QueueNotifyPtr) {
					*QueueNotifyPtr = queue;
				}
				//if (QueuedPlayers == 0) {
				//	if (ThreadState != THREAD_DETECT_FIREWALL) {
				//		Add_Thread_Action(THREAD_CONNECT_FIREWALL, INVALID_HANDLE_VALUE);
				//	}
				//}
				break;
			}

			/*
			** Client has cancelled out of the game channel.
			*/
			case WOLNATInterfaceClass::OPTION_ABORT_NEGOTIATION:
				WWDEBUG_SAY(("FirewallHelper - Got OPTION_ABORT_NEGOTIATION from %s\n", (char*)user.name));
			{
				ThreadLockClass locker(this);
				if (WOLNATInterface.Am_I_Server()) {
					if (stricmp((char*)user.name, PlayersName) == 0) {
						strcpy(CancelPlayer, (char*)user.name);
					}
				}

				/*
				** Remove this player from the client queue if he is in there.
				*/
				if (Remove_Player_From_Negotiation_Queue((char*)user.name)) {
					WWDEBUG_SAY(("FirewallHelper OPTION_ABORT_NEGOTIATION - Removed %s from ClientQueue\n", (char*)user.name));
				}

				break;
			}


			/*
			** Client has joined a channel and is available for negotiation.
			*/
			case WOLNATInterfaceClass::OPTION_CLIENT_IN_CHANNEL:
				WWDEBUG_SAY(("FirewallHelper - Got OPTION_CLIENT_IN_CHANNEL from %s\n", (char*)user.name));
				if (WOLNATInterface.Am_I_Server()) {
					Talk_To_New_Player(&user);
				}
				break;


			default:
				fw_assert(false);
				break;

		}
	}
}



/***********************************************************************************************
 * FirewallHelperClass::Cleanup_Client_Queue -- Remove pending entries from client queue       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: ONLY CALL THIS FROM THE MAIN THREAD                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/22/2002 4:09PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Cleanup_Client_Queue(void)
{
	/*
	** This is the main thread removing entries from the client queue that it wasn't able to remove before due to thread
	** contention.
	*/
	fw_assert(Get_Main_Thread_ID() == GetCurrentThreadId());

	while (ClientQueueRemoveList.Count()) {
		if (Remove_Player_From_Negotiation_Queue_If_Mutex_Available(ClientQueueRemoveList[0]->Name)) {
			delete ClientQueueRemoveList[0];
			ClientQueueRemoveList.Delete(0);
		} else {
			break;
		}
	}
}


/***********************************************************************************************
 * FirewallHelperClass::Remove_Player_From_Negotiation_Queue_If_Mutex_Available - What it says *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Player name                                                                       *
 *                                                                                             *
 * OUTPUT:   True if mutex was available                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/22/2002 3:41PM ST : Created                                                             *
 *=============================================================================================*/
bool FirewallHelperClass::Remove_Player_From_Negotiation_Queue_If_Mutex_Available(char *player_name)
{
	ThreadLockClass locker(this, 0);
	/*
	** If we can grab the mutex then remove the item immediately, otherwise add it to a list and return.
	*/
	if (locker.WaitResult == WAIT_OBJECT_0) {
		Remove_Player_From_Negotiation_Queue(player_name);
		return(true);
	} else {

		/*
		** Just add the name to a list that will be checked later when it's safe to remove stuff from the client queue.
		** This list must only ever be accessed from the main thread.
		*/
		fw_assert(Get_Main_Thread_ID() == GetCurrentThreadId());
		for (int i=0 ; i<ClientQueueRemoveList.Count() ; i++) {
			if (stricmp(ClientQueueRemoveList[i]->Name, player_name) == 0) {
				return(false);
			}
		}
		ClientStruct *clientptr = new ClientStruct;
		strcpy(clientptr->Name, player_name);
		ClientQueueRemoveList.Add(clientptr);
	}
	return(false);
}


/***********************************************************************************************
 * FirewallHelperClass::Remove_Player_From_Negotiation_Queue - What it says                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Player name                                                                       *
 *                                                                                             *
 * OUTPUT:   True if player was in queue                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/2002 10:20PM ST : Created                                                             *
 *=============================================================================================*/
bool FirewallHelperClass::Remove_Player_From_Negotiation_Queue(char *player_name)
{
	fw_assert(WOLNATInterface.Am_I_Server());
	if (ClientQueue.Count() == 0) {
		return(false);
	}
	ThreadLockClass locker(this);
	ClientStruct *clientptr = NULL;
	bool retcode = false;

	for (int i=0 ; i<ClientQueue.Count() ; i++) {
		clientptr = ClientQueue[i];
		if (stricmp(clientptr->Name, player_name) == 0) {
			WWDEBUG_SAY(("FirewallHelper - Removed %s from ClientQueue\n", clientptr->Name));
			delete clientptr;
			ClientQueue.Delete(i);
			i--;
			retcode = true;
		}
	}
	return(retcode);
}



/***********************************************************************************************
 * FirewallHelperClass::Send_My_Port -- Send my mangled port number to the other guy           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Port                                                                              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/13/2001 11:37AM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Send_My_Port(unsigned short port)
{

	ThreadLockClass locker(this);
	WWDEBUG_SAY(("FirewallHelper - Sending my port number (%d) to %s\n", (unsigned int)port, PlayersName));

	/*
	** Fill in an port notification options packet to send to the guest.
	*/
	WOLNATInterfaceClass::PrivateGameOptionsStruct options;
	strcpy(options.NATOptionsPrefix, "NAT:");
	options.Option = WOLNATInterfaceClass::OPTION_PORT_NOTIFICATION;
	sprintf(options.OptionData.Port.MangledPort, "%04x,", port);

	char my_name[64];
	WOLNATInterface.Get_My_Name(my_name);
	strcpy(options.OptionData.Port.Name, my_name);

	/*
	** Send it.
	*/
	WOLNATInterface.Send_Private_Game_Options(&PlayerAsUser, (char*)&options);
}




/***********************************************************************************************
 * FirewallHelperClass::Send_Connection_Result -- Send connection result game option           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Result enum                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/13/2001 2:17PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Send_Connection_Result(int result, unsigned short port)
{

	ThreadLockClass locker(this);

	/*
	** Fill in a connection result options packet to send to the other player.
	*/
	WOLNATInterfaceClass::PrivateGameOptionsStruct options;
	strcpy(options.NATOptionsPrefix, "NAT:");
	options.Option = WOLNATInterfaceClass::OPTION_CONNECTION_RESULT;
	options.OptionData.ConnectionResult.Result[0] = 'a' + result;
	options.OptionData.ConnectionResult.Result[1] = ',';
	sprintf(options.OptionData.ConnectionResult.Port, "%04x,", (unsigned int) port);

	char my_name[64];
	WOLNATInterface.Get_My_Name(my_name);
	strcpy(options.OptionData.ConnectionResult.Name, my_name);

	/*
	** Send it.
	*/
	WOLNATInterface.Send_Private_Game_Options(&PlayerAsUser, (char*)&options);
}




/***********************************************************************************************
 * FirewallHelperClass::Send_Connection_Result -- Send connection result game option           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Result enum                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/13/2001 2:17PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Send_Queue_States(void)
{

	if (WOLNATInterface.Am_I_Server()) {

		if (ClientQueue.Count()) {

			ThreadLockClass locker(this);

			/*
			** Fill in a queue state options packet to send to the other players.
			*/
			WOLNATInterfaceClass::PrivateGameOptionsStruct options;
			strcpy(options.NATOptionsPrefix, "NAT:");
			options.Option = WOLNATInterfaceClass::OPTION_QUEUE_STATE;

			char my_name[64];
			WOLNATInterface.Get_My_Name(my_name);
			strcpy(options.OptionData.ConnectionResult.Name, my_name);

			/*
			** Send updates to all queued players.
			*/
			for (int i=0 ; i<ClientQueue.Count() ; i++) {
				sprintf(options.OptionData.QueueState.Position, "%02x", i);
				ClientStruct *client = ClientQueue[i];
				fw_assert(client != NULL);

				if (client) {
					/*
					** Send it.
					*/
					WOLNATInterface.Send_Private_Game_Options(&client->User, (char*)&options);
				}
			}
		}
	}
}




/***********************************************************************************************
 * FirewallHelperClass::Set_Client_Connect_Event -- Set client event completion info           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Event                                                                             *
 *           Ptr to success flag                                                               *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/14/2001 10:56PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Set_Client_Connect_Event(HANDLE thread_event, HANDLE cancel_event, int *flag_ptr, int *queue_ptr)
{
	ThreadLockClass locker(this);

	fw_assert(!WOLNATInterface.Am_I_Server());

	if (!WOLNATInterface.Am_I_Server()) {
		SuccessFlagPtr = flag_ptr;
		ClientConnectEvent = thread_event;
		ClientCancelEvent = cancel_event;
		QueueNotifyPtr = queue_ptr;
	}
}



/***********************************************************************************************
 * FirewallHelperClass::Set_Client_Success -- Set the client connect return status             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Success - Did the client get a good port and IP for the server                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/14/2001 10:59PM ST : Created                                                            *
 *=============================================================================================*/
void FirewallHelperClass::Set_Client_Success(int success)
{
	if (!WOLNATInterface.Am_I_Server()) {
		if (SuccessFlagPtr) {
			*SuccessFlagPtr = success;
		}
		SuccessFlagPtr = NULL;
		if (QueueNotifyPtr) {
			*QueueNotifyPtr = 0;
		}
		QueueNotifyPtr = NULL;
		if (ClientConnectEvent != INVALID_HANDLE_VALUE) {
			SetEvent(ClientConnectEvent);
		}
		ClientConnectEvent = INVALID_HANDLE_VALUE;
		ClientCancelEvent = INVALID_HANDLE_VALUE;
	}
}




/***********************************************************************************************
 * FirewallHelperClass::Client_Cancelled -- Checks to see if the player hit cancel.            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   True if player cancelled                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/17/2001 12:04PM ST : Created                                                            *
 *=============================================================================================*/
bool FirewallHelperClass::Client_Cancelled(void)
{
	if (!WOLNATInterface.Am_I_Server()) {
		ThreadLockClass locker(this);
		if (ClientCancelEvent != INVALID_HANDLE_VALUE) {
			int result = WaitForSingleObject(ClientCancelEvent, 0);

			if (result == WAIT_OBJECT_0) {
				WWDEBUG_SAY(("FirewallHelper - Client cancelled\n"));
				return(true);
			}
		}
	}
	return(false);
}



/***********************************************************************************************
 * FirewallHelperClass::Remote_Client_Cancelled -- Did the remote client hit cancel?           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    True if our client cancelled                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: Requres PlayersName variable to be correctly set up                               *
 *           Will only return true once for each cancellation                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/19/2001 9:26PM ST : Created                                                             *
 *=============================================================================================*/
bool FirewallHelperClass::Remote_Client_Cancelled(void)
{
	ThreadLockClass locker(this);

	if (WOLNATInterface.Am_I_Server() && CancelPlayer[0] != 0) {
		if (stricmp(CancelPlayer, PlayersName) == 0) {
			CancelPlayer[0] = 0;
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * FirewallHelperClass::Send_Cancel_Notification -- Send abort to host                         *
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
 *   8/19/2001 9:32PM ST : Created                                                             *
 *=============================================================================================*/
void FirewallHelperClass::Send_Cancel_Notification(void)
{
	fw_assert(!WOLNATInterface.Am_I_Server());

	if (!WOLNATInterface.Am_I_Server()) {
		ThreadLockClass locker(this);
		WWDEBUG_SAY(("FirewallHelper - Sending my cancellation notice to the server\n"));

		/*
		** Fill in an abort notification options packet to send to the host.
		*/
		WOLNATInterfaceClass::PrivateGameOptionsStruct options;
		strcpy(options.NATOptionsPrefix, "NAT:");
		options.Option = WOLNATInterfaceClass::OPTION_ABORT_NEGOTIATION;
		options.OptionData.QuitTalking.Nothing = 0;

		/*
		** Send it.
		*/
		WOLNATInterface.Send_Private_Game_Options(&PlayerAsUser, (char*)&options);
	}
}




/***********************************************************************************************
 * FirewallHelperClass::Negotiate_Port -- Option communications with one other player          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   True if we heard from the other player directly                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/2001 8:54PM ST : Created                                                              *
 *=============================================================================================*/
int FirewallHelperClass::Negotiate_Port(void)
{
	int retries = 5;

	unsigned int timeout;
	unsigned int resend_timer;
	bool mangling = true;
	bool sharing_a_nat = false;

	int result;
	IPAddressClass address;
	bool gotit = false;
	SocketHandlerClass *socket = NULL;
	char receive_packet[512];

	char my_name[256];
	my_name[0] = 0;

	bool server = WOLNATInterface.Am_I_Server();
	bool got_name = WOLNATInterface.Get_My_Name(my_name);

	fw_assert(got_name);
	if (!got_name) {
		WWDEBUG_SAY(("FirewallHelper - Failed to get user log on name\n"));
		return(FW_RESULT_FAILED);
	}

	PlayersMangledPort = 0;
	PlayersConnectionResult = -1;
	PlayersConnectionResultPort = 0;

	/*
	** On the server side, we need to find out who we are supposed to be connecting to.
	** We will use the game comms to send and receive on the server side since we already have a socket bound to the port
	** we want to use.
	*/
	if (server) {
		if (ClientQueue.Count() == 0) {
			return(FW_RESULT_FAILED);
		}

		/*
		** Update everyones queue state.
		*/
		Send_Queue_States();

		ClientStruct *client = ClientQueue[0];

		PlayersLocalAddress = client->LocalAddress;
		PlayersExternalAddress = client->ExternalAddress;
		PlayersFirewallType = client->FirewallType;
		PlayerAsUser = client->User;
		strcpy(PlayersName, client->Name);
		LastOptionsFromClient = TIMEGETTIME();

		delete client;
		ClientQueue.Delete(0);

		WWDEBUG_SAY(("FirewallHelper - Removed next player %s from client queue\n", PlayersName));

		if (Remote_Client_Cancelled()) {
			return(FW_RESULT_FAILED);
		}
		CancelPlayer[0] = 0;

	} else {
		/*
		** On the client side, we need to open a new socket using the client side port number. Use any port number for the
		** second parameter since it's the default send port and will always be overridden when we send anyway.
		*/
		socket = new SocketHandlerClass;
		socket->Service_Never();
		if (!socket->Open(ClientPort, PlayersLocalAddress.Get_Port())) {
			WWDEBUG_SAY(("FirewallHelperClass: Failed to open socket for client\n"));
			fw_assert(false);
			delete socket;
			return(FW_RESULT_FAILED);
		}
		WOLNATInterface.Set_Service_Socket_Handler(socket);

		/*
		** If we aren't sure of the queue size then wait until we are.
		*/
		if (QueuedPlayers == -1) {
			timeout = TIMEGETTIME() + (8 * TIMER_SECOND);
			while (TIMEGETTIME() < timeout && QueuedPlayers == -1) {
				Process_Game_Options();
				Sleep(5);

				/*
				** See if the user cancelled.
				*/
				if (Client_Cancelled()) {
					Send_Cancel_Notification();
					WOLNATInterface.Set_Service_Socket_Handler(NULL);
					delete socket;
					return(FW_RESULT_CANCELLED);
				}
			}
		}

		if (QueuedPlayers == -1) {

			/*
			** This should never happen right....?
			*/
			if (socket) {
				WOLNATInterface.Set_Service_Socket_Handler(NULL);
				delete socket;
			}
			return(FW_RESULT_FAILED);
		}

		/*
		** If there are players queued ahead of us, wait til we get to the front of the queue.
		*/
		timeout = TIMEGETTIME() + (QueuedPlayers * 32 * TIMER_SECOND);
		while (TIMEGETTIME() < timeout && QueuedPlayers != 0) {
			Process_Game_Options();
			Sleep(5);

			/*
			** See if the user cancelled.
			*/
			if (Client_Cancelled()) {
				Send_Cancel_Notification();
				WOLNATInterface.Set_Service_Socket_Handler(NULL);
				delete socket;
				return(FW_RESULT_CANCELLED);
			}
		}
	}

	PlayersFirewallAddress = PlayersExternalAddress;
	PlayersFirewallAddress.Set_Port(PlayersLocalAddress.Get_Port());


	WWDEBUG_SAY(("FirewallHelper - Negotiating port with player %s, local address = %s,", PlayersName, PlayersLocalAddress.As_String()));
	WWDEBUG_SAY((" ext address = %s, firewall = %08x\n", PlayersExternalAddress.As_String(), PlayersFirewallType));

	/*
	** What will our mangled port be if we try to talk to this player?
	**
	** If they are behind the same firewall then there will be no mangling.
	**
	*/
	if (PlayersExternalAddress.Is_IP_Equal(ExternalAddress)) {
		mangling = false;
		sharing_a_nat = true;
		PlayersFirewallAddress = PlayersLocalAddress;
		WWDEBUG_SAY(("Both external IPs match - we are behind the same firewall\n"));
	}

	/*
	** Maybe neither of our firewalls use port mangling. If so, then we can use the default port number.
	*/
	if (mangling) {
		if (Behavior == 0 || (Behavior & FIREWALL_TYPE_SIMPLE) != 0) {
			if (PlayersFirewallType == 0 || (PlayersFirewallType & FIREWALL_TYPE_SIMPLE) != 0) {
				/*
				** Ok, we both have crap firewalls or modems or somesuch. Anyway, this should be easy.
				*/
				mangling = false;
			}
		}
	}

	int trying = 0;
	unsigned short last_send_port = 0;
	unsigned short my_last_mangled_port = 0;
	unsigned short verified_mangled_port = 0;
	bool used_old_port = false;

	do {

		Sleep(0);

		/*
		** See if the user cancelled.
		*/
		if (Client_Cancelled()) {
			Send_Cancel_Notification();
			WOLNATInterface.Set_Service_Socket_Handler(NULL);
			delete socket;
			return(FW_RESULT_CANCELLED);
		}

		if (Remote_Client_Cancelled()) {
			return(FW_RESULT_FAILED);
		}

		/*
		** If we haven't got any kind of options at all from the client for 10 seconds then give up.
		*/
		if (trying > 0 && server && (TIMEGETTIME() - LastOptionsFromClient > TIMER_SECOND * 10)) {
			break;
		}

		if (!mangling) {

			/*
			** Use their unmangled port number. Nothing fancy needed here.
			*/
			PlayersMangledPort = PlayersLocalAddress.Get_Port();
		} else {

			unsigned short mangled_port = 0;

			/*
			** If our firewall uses the same source port regardless of destination port and we have
			** already connected to someone at this address (two players behind the same NAT) then
			** we have to try the same source port we are already talking to the other guy on.
			**
			** If that's not working out then try a different port after two attempts.
			*/
			if (server && trying < 2) {
				if ((Behavior & FirewallHelperClass::FIREWALL_TYPE_DESTINATION_PORT_DELTA) == 0) {
					WWDEBUG_SAY(("FirewallHelper: No destination port delta - looking for earlier connections at the same address\n"));

					for (int r = 0 ; r < ConnectionHistory.Count() ; r++) {
						if (ConnectionHistory[r].Is_IP_Equal(PlayersExternalAddress)) {
							mangled_port = MangledPortHistory[r];
							used_old_port = true;
							WWDEBUG_SAY(("FirewallHelper - Found old connection port number to use - port number %d\n", mangled_port));
							break;
						}
					}
				}
			}

			/*
			** If this isn't the first try then we might already know what the port will be since it can't change between tries.
			*/
			if (mangled_port == 0) {
				if ((Behavior & FirewallHelperClass::FIREWALL_TYPE_DESTINATION_PORT_DELTA) == 0) {
					if (trying > 0) {
						if (!used_old_port) {
							mangled_port = my_last_mangled_port;
							WWDEBUG_SAY(("FirewallHelper - No dest port delta - using last port number %d\n", mangled_port));
						} else {
							used_old_port = false;
						}
					}
				}
			}

			if (mangled_port == 0) {

				/*
				** Work out what my port will be.
				*/
				unsigned short base_port = 0;
				if (server) {
					base_port = WOLNATInterface.Get_Port_As_Server();
				} else {

					/*
					** Client only. Roll onto the next port and try that.
					*/
					if (trying > 0) {
						WOLNATInterface.Set_Service_Socket_Handler(NULL);
						Sleep(100);
						socket->Close();
						for (int cp=0 ; cp < 2048 ; cp++) {
							ClientPort = WOLNATInterface.Get_Next_Client_Port();
							if (socket->Open(ClientPort, PlayersLocalAddress.Get_Port())) {
								break;
							} else {
								socket->Close();
								ClientPort = 0;
							}
						}
						fw_assert(ClientPort != 0);
						WOLNATInterface.Set_Service_Socket_Handler(socket);
					}

					if (ClientPort != 0) {
						base_port = ClientPort;
					}

				}

				/*
				** Get my mangled port. This can take up to 4 seconds.
				*/
				mangled_port = FirewallHelper.Get_Next_Mangled_Source_Port(base_port);
			}

			/*
			** Send my mangled port to the other chap.
			*/
			fw_assert(mangled_port != 0);
			Send_My_Port(mangled_port);
			my_last_mangled_port = mangled_port;
		}

		/*
		** Wait until we know what port to send to. Allow plenty of time for slow chat server response.
		** Players are more likely to be in 'sync' with each other after the first try.
		*/
		timeout = TIMEGETTIME() + (6 * TIMER_SECOND);
		if (trying > 0) {
			timeout = timeout - (3*TIMER_SECOND);
		}
		while (TIMEGETTIME() < timeout && ((mangling == true && PlayersMangledPort == last_send_port) || (mangling == false && PlayersMangledPort == 0))) {
			Process_Game_Options();
			Sleep(5);

			/*
			** See if the user cancelled.
			*/
			if (Client_Cancelled()) {
				Send_Cancel_Notification();
				WOLNATInterface.Set_Service_Socket_Handler(NULL);
				delete socket;
				return(FW_RESULT_CANCELLED);
			}

			if (Remote_Client_Cancelled()) {
				return(FW_RESULT_FAILED);
			}

		}

		if (verified_mangled_port != 0) {
			PlayersMangledPort = verified_mangled_port;
		}

		last_send_port = PlayersMangledPort;
		PlayersFirewallAddress.Set_Port(PlayersMangledPort);

		if (PlayersMangledPort == 0) {
			WWDEBUG_SAY(("FirewallHelper: PlayersMangledPort = 0 - skipping packet send\n"));
		}

		if (PlayersMangledPort != 0) {

			/*
			** OK, we know what port to send to. Let dooooo it.
			*/
			/*
			** Build the port probe packet to send.
			*/
			char packet[256];
			strcpy(packet, my_name);

			/*
			** If we are a netgear and talking to a non netgear then pause before sending.
			**
			** This is needed when playing a netgear against a linksys. If the netgear sends first
			** then the linksys can respond with a ICMP port unreachable message in the case that
			** it already has an open port table mapping with a pre-existing player.
			**
			** When it gets an ICMP port unreachable, the netgear invalidates any existing port mappings.
			**
			** The delay has to be long enough for our port number to be passed through the chat server
			** to the other guy and for him to start sending.
			*/
			if (Is_Netgear() && !Is_Netgear(PlayersFirewallType)) {
				WWDEBUG_SAY(("FirewallHelper: Doing the netgear sleep thing\n"));
				Sleep(TIMER_SECOND * 4);
			}

			/*
			** Send the packet to the other player. If a NAT mangles an outgoing port number then it will be noted
			** as the packet comes in.
			*/
			WWDEBUG_SAY(("FirewallHelper: Sending PACKETTYPE_FIREWALL_PROBE packet to %s at port %d\n", PlayersName, PlayersMangledPort));
			timeout = TIMEGETTIME() + (TIMER_SECOND * 6);
			resend_timer = 0;

			do {
				if (resend_timer < TIMEGETTIME()) {
					//if (!CrapFlag)
					const size_t payload_size = ::strlen(packet) + 1;
					WWASSERT(payload_size <= size_t(std::numeric_limits<int>::max()));
					WOLNATInterface.Send_Game_Format_Packet_To(&PlayersFirewallAddress, packet, static_cast<int>(payload_size), socket);
					resend_timer = TIMEGETTIME() + (TIMER_SECOND / 2);
				}

				Sleep(5);
				Process_Game_Options();

				/*
				** See if the user cancelled.
				*/
				if (Client_Cancelled()) {
					Send_Cancel_Notification();
					WOLNATInterface.Set_Service_Socket_Handler(NULL);
					delete socket;
					return(FW_RESULT_CANCELLED);
				}

				if (Remote_Client_Cancelled()) {
					return(FW_RESULT_FAILED);
				}


				//if (!gotit) {

					/*
					** See if a port probe came in from this player.
					*/
					IPAddressClass receive_address;
					result = WOLNATInterface.Get_Packet(receive_packet, sizeof(receive_packet), receive_address);

					/*
					** Check if we got a packet from this game.
					*/
       			if (result) {

						WWDEBUG_SAY(("FirewallHelper: Got a packet from player %s at address %s\n", receive_packet, receive_address.As_String()));

						/*
						** Is it from the guy we are trying to talk to?
						*/
						if (stricmp(receive_packet, PlayersName) == 0) {

							WWDEBUG_SAY(("FirewallHelper: Packet is from other guy\n"));

							/*
							** Send a connection result game option.
							*/
							Send_Connection_Result(CONNRESULT_CONNECTED, receive_address.Get_Port());

							/*
							** Note the address the packet came in on.
							*/
							PlayersFirewallAddress = receive_address;
							verified_mangled_port = receive_address.Get_Port();

							/*
							** Got it.
							*/
							gotit = true;
						}
					}
				//}

				/*
				** If the other player got our packet and we got theirs then we are finished.
				*/
				if (gotit && PlayersConnectionResult == CONNRESULT_CONNECTED) {
					WWDEBUG_SAY(("FirewallHelper: Got connection result - my packet must have got through\n"));
					break;
				}

				/*
				** If the other guy has started re-trying then we might as well do that too.
				*/
				if (PlayersConnectionResult >= CONNRESULT_TRY1 && PlayersConnectionResult < CONNRESULT_CONNECTED) {
					int their_try = PlayersConnectionResult - CONNRESULT_TRY1;
					if (their_try > trying) {
						WWDEBUG_SAY(("FirewallHelper: Other player is trying again - their_try = %d\n", their_try));
						trying = their_try - 1;
						break;
					}
				}

			} while(TIMEGETTIME() < timeout);
		}

		if (gotit && PlayersConnectionResult == CONNRESULT_CONNECTED) {
			break;
		}

		if (trying >= retries) {
			break;
		}

		/*
		** Well, that didn't go too well, let's try again.
		*/
		trying++;
		Send_Connection_Result(CONNRESULT_TRY1 + trying, 0);
		WWDEBUG_SAY(("FirewallHelper: Trying again - trying = %d\n", trying));
	} while(true);

	if (socket) {
		WOLNATInterface.Set_Service_Socket_Handler(NULL);
		delete socket;
	}

	if (gotit && PlayersConnectionResult == CONNRESULT_CONNECTED) {
		WWDEBUG_SAY(("FirewallHelper: Port negotiation successful! Correct address is %s\n", PlayersFirewallAddress.As_String()));
		WWDEBUG_SAY(("Player saw our port as %d\n", (unsigned int)PlayersConnectionResultPort));

		/*
		** Well it worked so we have more confidence in our firewall settings.
		*/
		Confidence++;

		/*
		** Save the port number in case we have to connect to this guy again.
		*/
		if (server) {

			unsigned short history_port = PlayersConnectionResultPort;
			if (history_port == 0) {
				history_port = my_last_mangled_port;
			}
			bool found_history = false;

			for (int h=0 ; h<ConnectionHistory.Count() ; h++) {
				if (ConnectionHistory[h].Is_IP_Equal(PlayersExternalAddress)) {
					MangledPortHistory[h] = history_port;
					found_history = true;
					break;
				}
			}
			if (!found_history) {
				ConnectionHistory.Add(PlayersExternalAddress);
				MangledPortHistory.Add(history_port);
			}
			fw_assert(ConnectionHistory.Count() == MangledPortHistory.Count());
		}
		return(FW_RESULT_SUCCEEDED);
	} else {
		WWDEBUG_SAY(("FirewallHelper: *** Port negotiation failed! ***\n"));
		Confidence--;
		if (Confidence < 0) {
			Confidence = 0;
		}
	}

	return(FW_RESULT_FAILED);
}










/*
** Function definitions for the MIB-II entry points.
*/
BOOL (__stdcall *SnmpExtensionInitPtr)(IN DWORD dwUpTimeReference, OUT HANDLE *phSubagentTrapEvent, OUT AsnObjectIdentifier *pFirstSupportedRegion);
BOOL (__stdcall *SnmpExtensionQueryPtr)(IN BYTE bPduType, IN OUT RFC1157VarBindList *pVarBindList, OUT AsnInteger32 *pErrorStatus, OUT AsnInteger32 *pErrorIndex);
LPVOID (__stdcall *SnmpUtilMemAllocPtr)(IN DWORD bytes);
VOID (__stdcall *SnmpUtilMemFreePtr)(IN LPVOID pMem);

typedef struct tConnInfoStruct {
	unsigned int State;
	unsigned int LocalIP;
	unsigned short LocalPort;
	unsigned int RemoteIP;
	unsigned short RemotePort;
} ConnInfoStruct;


/***********************************************************************************************
 * Get_Local_Chat_Connection_Address -- Which address are we using to talk to the chat server? *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to address to return local address                                            *                                                                                             *
 *                                                                                             *
 * OUTPUT:   True if success                                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/00 3:24PM ST : Created                                                              *
 *=============================================================================================*/
bool FirewallHelperClass::Get_Local_Chat_Connection_Address(void)
{
	/*
	** Local defines.
	*/
	enum {
		CLOSED = 1,
		LISTENING,
		SYN_SENT,
		SEN_RECEIVED,
		ESTABLISHED,
		FIN_WAIT,
		FIN_WAIT2,
		CLOSE_WAIT,
		LAST_ACK,
		CLOSING,
		TIME_WAIT,
		DELETE_TCB
	};

	enum {
		tcpConnState = 1,
		tcpConnLocalAddress,
		tcpConnLocalPort,
		tcpConnRemAddress,
		tcpConnRemPort
	};


	/*
	** Locals.
	*/
	char server_name[128];
	unsigned char server_address[4];
	unsigned char remote_address[4];
	HANDLE trap_handle;
	AsnObjectIdentifier first_supported_region;
	DynamicVectorClass<ConnInfoStruct*> connection_list;
	int last_field;
	int index;
	AsnInteger error_status;
	AsnInteger error_index;
	int conn_entry_type_index;
	int conn_entry_type;
	bool found;
	unsigned int server_port = 0;
	IPAddressClass my_address;

	/*
	** Statics.
	*/
	static char _conn_state[][32] = {
		"?",
		"CLOSED",
		"LISTENING",
		"SYN_SENT",
		"SEN_RECEIVED",
		"ESTABLISHED",
		"FIN_WAIT",
		"FIN_WAIT2",
		"CLOSE_WAIT",
		"LAST_ACK",
		"CLOSING",
		"TIME_WAIT",
		"DELETE_TCB"
	};

	/*
	** If we already did this then there's no need to do it again.
	*/
	if (LocalChatConnectionAddress.Is_Valid()) {
		return(true);
	}


	WWDEBUG_SAY(("FirewallHelper - Finding local address used to talk to the chat server\n"));

	/*
	** Get the name of the current server.
	*/
	WOLNATInterface.Get_Current_Server_ConnData(server_name, sizeof(server_name));

	if (strlen(server_name) == 0) {
		return(false);
	}

	/*
	** the conndata field will look something like
	**
	** 	TCP;ra2chat.westwood.com;7000
	*/
	fw_assert(strnicmp((char*)server_name, "TCP;", 4) == 0);
	char *server_name_ptr = &server_name[4];
	char *semi_colon_ptr = strchr(server_name_ptr, ';');

	if (semi_colon_ptr) {
		*semi_colon_ptr = 0;
		sscanf(semi_colon_ptr+1, "%d", &server_port);
	} else {
		WWDEBUG_SAY(("FirewallHelper - Failed to parse server name\n"));
		return(false);
	}
	WWDEBUG_SAY(("FirewallHelper - Current chat server name is %s\n", server_name_ptr));
	WWDEBUG_SAY(("FirewallHelper - Chat server port is %d\n", server_port));

	/*
	** Get the address of the chat server.
	*/
	WWDEBUG_SAY(("FirewallHelper - About to call gethostbyname\n"));
	struct hostent *host_info = gethostbyname(server_name_ptr);

	if (!host_info) {
		WWDEBUG_SAY(("FirewallHelper - gethostbyname failed! Error code %d\n", WSAGetLastError()));
		return(false);
	}

	memcpy(server_address, &host_info->h_addr_list[0][0], 4);
	unsigned int temp = *((unsigned int*)(&server_address[0]));
	temp = ntohl(temp);
	*((unsigned int*)(&server_address[0])) = temp;

	WWDEBUG_SAY(("FirewallHelper - Host address is %d.%d.%d.%d\n", server_address[3], server_address[2], server_address[1], server_address[0]));

	/*
	** Load the MIB-II SNMP DLL.
	*/
	WWDEBUG_SAY(("FirewallHelper - About to load INETMIB1.DLL\n"));

	HINSTANCE mib_ii_dll = LoadLibraryA("inetmib1.dll");
	if (mib_ii_dll == NULL) {
		WWDEBUG_SAY(("FirewallHelper - Failed to load INETMIB1.DLL\n"));
		return(false);
	}

	WWDEBUG_SAY(("FirewallHelper - About to load SNMPAPI.DLL\n"));

	HINSTANCE snmpapi_dll = LoadLibraryA("snmpapi.dll");
	if (snmpapi_dll == NULL) {
		WWDEBUG_SAY(("FirewallHelper - Failed to load SNMPAPI.DLL\n"));
		FreeLibrary(mib_ii_dll);
		return(false);
	}

	/*
	** Get the function pointers into the .dll
	*/
	SnmpExtensionInitPtr = (int (__stdcall *)(DWORD,void ** ,AsnObjectIdentifier *)) GetProcAddress(mib_ii_dll, "SnmpExtensionInit");
	SnmpExtensionQueryPtr = (BOOL (__stdcall *)(BYTE,SnmpVarBindList *,AsnInteger32 *,AsnInteger32 *)) GetProcAddress(mib_ii_dll, "SnmpExtensionQuery");
	SnmpUtilMemAllocPtr = (void *(__stdcall *)(DWORD)) GetProcAddress(snmpapi_dll, "SnmpUtilMemAlloc");
	SnmpUtilMemFreePtr = (void (__stdcall *)(void *)) GetProcAddress(snmpapi_dll, "SnmpUtilMemFree");
	if (SnmpExtensionInitPtr == NULL || SnmpExtensionQueryPtr == NULL || SnmpUtilMemAllocPtr == NULL || SnmpUtilMemFreePtr == NULL) {
		WWDEBUG_SAY(("FirewallHelper - Failed to get proc addresses for linked functions\n"));
		FreeLibrary(snmpapi_dll);
		FreeLibrary(mib_ii_dll);
		return(false);
	}


	RFC1157VarBindList *bind_list_ptr = (RFC1157VarBindList *) SnmpUtilMemAllocPtr(sizeof(RFC1157VarBindList) + 8192);
	RFC1157VarBind *bind_ptr = (RFC1157VarBind *) SnmpUtilMemAllocPtr(sizeof(RFC1157VarBind) + 128);

	/*
	** OK, here we go. Try to initialise the .dll
	*/
	WWDEBUG_SAY(("FirewallHelper - About to init INETMIB1.DLL\n"));
	int ok = SnmpExtensionInitPtr(GetCurrentTime(), &trap_handle, &first_supported_region);

	if (!ok) {
		/*
		** Aw crap.
		*/
		WWDEBUG_SAY(("FirewallHelper - Failed to init the .dll\n"));
		SnmpUtilMemFreePtr(bind_list_ptr);
		SnmpUtilMemFreePtr(bind_ptr);
		FreeLibrary(snmpapi_dll);
		FreeLibrary(mib_ii_dll);
		return(false);
	}

	/*
	** Name of mib_ii object we want to query. See RFC 1213.
	**
	** iso.org.dod.internet.mgmt.mib-2.tcp.tcpConnTable.TcpConnEntry.tcpConnState
	**  1   3   6      1      2     1   6        13          1             1
	*/
	unsigned int mib_ii_name[] = {1,3,6,1,2,1,6,13,1,1};
	unsigned int *mib_ii_name_ptr = (unsigned int *) SnmpUtilMemAllocPtr(sizeof(mib_ii_name) + 1024);
	memcpy(mib_ii_name_ptr, mib_ii_name, sizeof(mib_ii_name));

	/*
	** Get the index of the conn entry data.
	*/
	conn_entry_type_index = ARRAY_SIZE(mib_ii_name) - 1;

	/*
	** Set up the bind list.
	*/
	bind_ptr->name.idLength = ARRAY_SIZE(mib_ii_name);
	bind_ptr->name.ids = mib_ii_name_ptr;
	bind_list_ptr->list = bind_ptr;
	bind_list_ptr->len = 1;


	/*
	** We start with the tcpConnLocalAddress field.
	*/
	last_field = 1;

	/*
	** First connection.
	*/
	index = 0;

	/*
	** Suck out that tcp connection info....
	*/
	while (true) {

		if (!SnmpExtensionQueryPtr(ASN_RFC1157_GETNEXTREQUEST, bind_list_ptr, &error_status, &error_index)) {
			WWDEBUG_SAY(("FirewallHelper - SnmpExtensionQuery returned false\n"));
			SnmpUtilMemFreePtr(bind_list_ptr);
			SnmpUtilMemFreePtr(bind_ptr);
			FreeLibrary(snmpapi_dll);
			FreeLibrary(mib_ii_dll);
			return(false);
		}

		/*
		** If this is something new we aren't looking for then we are done.
		*/
		if (bind_ptr->name.idLength < ARRAY_SIZE(mib_ii_name)) {
			break;
		}

		/*
		** Get the type of info we are looking at. See RFC1213.
		**
		** 1 = tcpConnState
		** 2 = tcpConnLocalAddress
		** 3 = tcpConnLocalPort
		** 4 = tcpConnRemAddress
		** 5 = tcpConnRemPort
		**
		** tcpConnState is one of the following...
		**
		**   1  closed
		**   2  listen
		**   3  synSent
		**   4  synReceived
		**   5  established
		**   6  finWait1
		**   7  finWait2
		**   8  closeWait
		**   9  lastAck
		**   10 closing
		**   11 timeWait
		**   12 deleteTCB
		*/
		conn_entry_type = bind_ptr->name.ids[conn_entry_type_index];

		if (last_field != conn_entry_type) {
			index = 0;
			last_field = conn_entry_type;
		}

		switch (conn_entry_type) {

			/*
			** 1. First field in the entry. Need to create a new connection info struct
			** here to store this connection in.
			*/
			case tcpConnState:
			{
				ConnInfoStruct *new_conn = new ConnInfoStruct;
				new_conn->State = bind_ptr->value.asnValue.number;
				connection_list.Add(new_conn);
				break;
			}

			/*
			** 2. Local address field.
			*/
			case tcpConnLocalAddress:
				fw_assert(index < connection_list.Count());
				connection_list[index]->LocalIP = *((unsigned int*)bind_ptr->value.asnValue.address.stream);
				index++;
				break;

			/*
			** 3. Local port field.
			*/
			case tcpConnLocalPort:
				fw_assert(index < connection_list.Count());
				connection_list[index]->LocalPort = bind_ptr->value.asnValue.number;
				index++;
				break;

			/*
			** 4. Remote address field.
			*/
			case tcpConnRemAddress:
				fw_assert(index < connection_list.Count());
				connection_list[index]->RemoteIP = *((unsigned int*)bind_ptr->value.asnValue.address.stream);
				index++;
				break;

			/*
			** 5. Remote port field.
			*/
			case tcpConnRemPort:
				fw_assert(index < connection_list.Count());
				connection_list[index]->RemotePort = bind_ptr->value.asnValue.number;
				index++;
				break;
		}
	}

	SnmpUtilMemFreePtr(bind_list_ptr);
	SnmpUtilMemFreePtr(bind_ptr);
	//SnmpUtilMemFreePtr(mib_ii_name_ptr);		// Don't free this - the SnmpExtensionQueryPtr call frees it apparently

	WWDEBUG_SAY(("FirewallHelper - Got %d connections in list, parsing...\n", connection_list.Count()));

	/*
	** Right, we got the lot. Lets see if any of them have the same address as the chat
	** server we think we are talking to.
	*/
	found = false;
	while (connection_list.Count()) {
		ConnInfoStruct *connection = connection_list[0];

		temp = ntohl(connection->RemoteIP);
		memcpy(remote_address, (unsigned char*)&temp, 4);

		/*
		** See if this connection has the same address as our server.
		*/
		if (!found && memcmp(remote_address, server_address, 4) == 0) {
			WWDEBUG_SAY(("FirewallHelper - Found connection with same remote address as server\n"));

			if (server_port == 0 || server_port == (unsigned int)connection->RemotePort) {

				WWDEBUG_SAY(("FirewallHelper - Connection has same port\n"));
				/*
				** Make sure the connection is current.
				*/
				if (connection->State == ESTABLISHED) {
					WWDEBUG_SAY(("FirewallHelper - Connection is ESTABLISHED\n"));
					my_address.Set_Address((unsigned char*)&connection->LocalIP, connection->LocalPort);
					found = true;
				} else {
					WWDEBUG_SAY(("FirewallHelper - Connection is not ESTABLISHED - skipping\n"));
				}
			} else {
				WWDEBUG_SAY(("FirewallHelper - Connection has different port. Port is %d, looking for %d\n", connection->RemotePort, server_port));
			}
		}

		/*
		** Free the memory.
		*/
		delete connection_list[0];
		connection_list.Delete(0);
	}

	if (found) {
		WWDEBUG_SAY(("FirewallHelper - Using address %s to talk to chat server\n", my_address.As_String()));
		LocalChatConnectionAddress = my_address;
	}

	FreeLibrary(snmpapi_dll);
	FreeLibrary(mib_ii_dll);

	ThreadLockClass locker(this);
	ThreadState = THREAD_GET_LOCAL_ADDRESS_DONE;

	return(found);
}




/***********************************************************************************************
 * FirewallHelperClass::Get_Local_Address -- Get local chat address ip as a int               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Local IP - 0 if unknown                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/11/2001 4:30PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int FirewallHelperClass::Get_Local_Address(void)
{
	unsigned int ip = 0;
	if (LocalChatConnectionAddress.Is_Valid()) {
		ip = LocalChatConnectionAddress.Get_Address();
		ip = htonl(ip);
	}

	return(ip);
}



/***********************************************************************************************
 * FirewallHelperClass::Get_Raw_Firewall_Behavior -- Get firewall behavior bits                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Firewall behavior                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/3/2001 8:50PM ST : Created                                                             *
 *=============================================================================================*/
unsigned short FirewallHelperClass::Get_Raw_Firewall_Behavior(void)
{
	unsigned short behave = (unsigned short)Behavior;

	/*
	** If we are forcing a particular port to be used then set the behavior type to be as if there was no firewall there at all.
	*/
	if (WOLNATInterface.Get_Force_Port() != 0) {
		behave = (unsigned short) FIREWALL_TYPE_SIMPLE;
	}
	return(behave);
}
