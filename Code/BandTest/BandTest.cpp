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
 *                 Project Name : Bandwidth Tester                                             *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/BandTest/BandTest.cpp                        $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 3/04/02 5:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "BandTest.h"

#include "../Combat/specialbuilds.h"
#include "../wwnet/network-typedefs.h"
#include "../wwlib/systimer.h"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <ws2tcpip.h>
#include <mmsystem.h>
#include <conio.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <thread>
#include <alloca.h>
#endif

#ifndef _WIN32
#ifndef LPSOCKADDR
#define LPSOCKADDR sockaddr*
#endif

struct WSADATA { int dummy; };

inline int WSAStartup(unsigned short, WSADATA*) { return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }
#ifndef MAKEWORD
#define MAKEWORD(low, high) (((low) & 0xff) | (((high) & 0xff) << 8))
#endif
inline int ioctlsocket(SOCKET sock, int cmd, unsigned int* argp) {
	return ::ioctl(sock, cmd, argp);
}
inline void timeBeginPeriod(unsigned) {}
inline void timeEndPeriod(unsigned) {}
#ifndef _alloca
#define _alloca(size) alloca(size)
#endif
inline void Sleep(unsigned int duration_ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}
#ifndef _cprintf
#define _cprintf printf
#endif
#ifndef GetCurrentProcessId
#define GetCurrentProcessId getpid
#endif
#endif // !_WIN32

#ifdef _WIN32
using socklen_t = int;
#endif

/***********************************************************************************************
** Data structures
*/

#pragma pack(4)

/*
** IP header definition.
*/
typedef struct tIPHeaderType {
	unsigned char	Length : 4;		// length of the header
	unsigned char	Version : 4;	// Version of IP (must be 4 for IP4)
	unsigned char	TOS;				// Type of service (usually 0)
	unsigned short	PacketLen;		// total length of the packet
	unsigned short	ID;				// unique identifier
	unsigned short	Flags;	  		// flags
	unsigned char	TTL; 				// Time to live
	unsigned char	Protocol;		// protocol (TCP, UDP etc)
	unsigned short	Checksum;		// IP checksum
	unsigned int	SourceIP;		// IP this packet came from
	unsigned int	DestIP;        // IP this packet is going to
} IPHeaderType;

/*
** ICMP packet header. Sits after the IP header when used.
*/
typedef struct tICMPHeaderType {
  char				Type;
  char				Code;
  unsigned short	Checksum;
  unsigned short	ID;
  unsigned short	Sequence;
} ICMPHeaderType;

/*
** UDP header. Sits after the IP header when used.
*/
typedef struct tUDPHeaderType {
	unsigned short SourcePort;
	unsigned short DestPort;
	unsigned short Length;
	unsigned short Checksum;
} UDPHeaderType;




/***********************************************************************************************
** Defines
*/
/*
** Total size of ICMP echo packet. 20 + 8
*/
#define ICMP_ECHO_SIZE	28

/*
** ICMP message numbers.
*/
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO 8
#define ICMP_TIME_EXCEEDED 11

/*
** IP protocol numbers.
*/
#define PROTOCOL_ICMP 1
#define PROTOCOL_UDP 17

/*
** Timer.
*/
#ifndef TIMER_SECOND
#define TIMER_SECOND 1000
#endif //TIMER_SECOND


/***********************************************************************************************
** Global data.
*/
/*
** Sockets.
*/
SOCKET RawSocket = INVALID_SOCKET;
SOCKET ICMPRawSocket = INVALID_SOCKET;

#ifdef _WIN32
/*
** Registry handle (Windows).
*/
static HKEY RegistryKey;
#endif

//static char BandTestRegistryLocation[64] = {"Software\\Westwood\\Renegade\\BandTest\\"};

#if	defined(FREEDEDICATEDSERVER)
static char BandTestRegistryLocation[64] = {"Software\\Westwood\\RenegadeFDS\\BandTest\\"};
#elif defined(MULTIPLAYERDEMO)
static char BandTestRegistryLocation[64] = {"Software\\Westwood\\RenegadeMPDemo\\BandTest\\"};
#elif defined(BETACLIENT)
static char BandTestRegistryLocation[64] = {"Software\\Westwood\\RenegadeBeta\\BandTest\\"};
#else
static char BandTestRegistryLocation[64] = {"Software\\Westwood\\Renegade\\BandTest\\"};
#endif


static char RegistryPath[1024];

#ifndef _WIN32
namespace {

std::unordered_map<std::string, int> g_registry_store;
std::mutex g_registry_mutex;
bool g_registry_loaded = false;
std::filesystem::path g_registry_file_path;

std::filesystem::path Get_Linux_Registry_Path()
{
	if (!g_registry_file_path.empty()) {
		return g_registry_file_path;
	}

	const char* home = std::getenv("HOME");
	std::filesystem::path base = (home && *home) ? std::filesystem::path(home) : std::filesystem::path(".");
	base /= ".config";
	base /= "OpenW3D";
	std::error_code ec;
	std::filesystem::create_directories(base, ec);
	base /= "bandtest.cfg";
	g_registry_file_path = base;
	return g_registry_file_path;
}

void Load_Linux_Registry()
{
	if (g_registry_loaded) {
		return;
	}
	g_registry_loaded = true;

	std::ifstream input(Get_Linux_Registry_Path());
	if (!input) {
		return;
	}

	std::string line;
	while (std::getline(input, line)) {
		auto pos = line.find('=');
		if (pos == std::string::npos) {
			continue;
		}
		std::string key = line.substr(0, pos);
		int value = std::strtol(line.c_str() + pos + 1, nullptr, 10);
		g_registry_store[key] = value;
	}
}

void Save_Linux_Registry()
{
	if (!g_registry_loaded) {
		return;
	}

	std::ofstream output(Get_Linux_Registry_Path(), std::ios::trunc);
	if (!output) {
		return;
	}

	for (const auto& entry : g_registry_store) {
		output << entry.first << '=' << entry.second << '\n';
	}
}

} // namespace
#endif

/*
** Packet loss.
*/
int PingsSent = 0;
int PingsLost = 0;

/*
** Connection quality.
*/
int NumConsistentPings = 0;
int NumPingsCheckedForConsistency = 0;

/*
** State.
*/
bool StatsValid = false;

#if defined(_WIN32) && defined(_DEBUG)
HANDLE DebugFile = INVALID_HANDLE_VALUE;
char DebugFileName[256];
#endif //_DEBUG


/***********************************************************************************************
** Function prototypes.
*/
static bool Open_Raw_Sockets(int &failure_code);
static void Close_Raw_Sockets(void);
static bool Send_Ping(char *payload, int payload_size, SOCKET socket, struct sockaddr *address, int sequence_id);
static bool Get_Ping_Response(SOCKET socket, int &seq_id, struct sockaddr *address, unsigned int validate_addr, unsigned int &my_address);
static unsigned short Get_IP_Checksum(unsigned short *buffer, int size);
static bool Send_Raw_UDP(char *payload, int payload_size, SOCKET socket, struct sockaddr *address, unsigned short source_port, unsigned short dest_port);
static unsigned int Upstream_Detect(unsigned int server_ip, unsigned int my_ip, int &failure_code, unsigned int &downstream, BandtestSettingsStruct *settings);
static int Ping_Host(unsigned int host_ip, unsigned int my_ip, int times, int payload_size, unsigned int *ping_times, unsigned int timeout);
static float Average_Ping(int num_pings, unsigned int *ping_times, bool ignore_low_high);
static float Lowest_Ping(int num_pings, unsigned int *ping_times);
static int Get_Path_To_Server(unsigned int *path, unsigned int my_ip, unsigned int server_ip);
static void Ping_Profile(struct sockaddr_in *router_addr, unsigned int my_ip);

static bool Set_Registry_Int(const char *name, int value);
static int Get_Registry_Int(const char *name, int def_value);
static bool Open_Registry(void);
static void Close_Registry(void);


#ifdef _DEBUG
void DebugString (char const * string, ...);
char * Addr_As_String2(struct sockaddr_in *addr);
char * Addr_As_String(unsigned char *addr);
#else //_DEBUG
#define DebugString
//_forceinline void DebugString(char const *, ...) {};
//inline char * Addr_As_String(sockaddr_in *) {};
inline char * Addr_As_String(unsigned char *) {return NULL;};
inline char * Addr_As_String2(struct sockaddr_in *){return NULL;};
#endif //_DEBUG


/*
** Default settings.
*/
BandtestSettingsStruct DefaultSettings = {
	0,		//AlwaysICMP
	0,		//TTLScatter
	50,	//FastPingPackets
	20,	//SlowPingPackets
	15,	//FastPingThreshold
	0,		//PingProfile
};



/***********************************************************************************************
** Your actual Code. No, really.
*/



#ifdef _WIN32
/***********************************************************************************************
 * DllMain -- Dll entry point. Not used for much                                               *
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
 *   10/3/2001 11:20AM ST : Created                                                            *
 *=============================================================================================*/
bool APIENTRY DllMain(HANDLE, DWORD, void *)
{
	return(true);
}
#endif





/***********************************************************************************************
 * Detect_Upstream_Bandwidth -- Try and figure out what our upstream bandwidth is              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP address of server to use as destination IP in tests.Server receives no packets *
 *           My IP.                                                                            *
 *           Number of times to retry on error                                                 *
 *           (out) Extended error code                                                         *
 *                                                                                             *
 * OUTPUT:   Bandwidth in bits per second. 0 = couldn't detect. 0xffffffff means **BIG**       *
 *                                                                                             *
 * WARNINGS: All IPs in host order                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:21AM ST : Created                                                            *
 *=============================================================================================*/
BANDTEST_API unsigned int Detect_Bandwidth(unsigned int server_ip, unsigned int my_ip, int retries, int &failure_code, unsigned int &downstream, unsigned int api_version, BandtestSettingsStruct *settings, char *regpath)
{
	if (api_version != BANDTEST_API_VERSION) {
		return(BANDTEST_WRONG_API_VERSION);
	}

	if (regpath == NULL) {
		strcpy(RegistryPath, BandTestRegistryLocation);
	} else {
		strcpy(RegistryPath, regpath);
	}

	if (!Open_Registry()) {
		failure_code = BANDTEST_UNKNOWN_ERROR;
		return(0);
	}

	timeBeginPeriod(1);
	PingsSent = 0;
	PingsLost = 0;
	NumConsistentPings = 0;
	NumPingsCheckedForConsistency = 0;
	StatsValid = false;

	int tried = 0;
	int bps = 0;
	if (settings == NULL) {
		settings = &DefaultSettings;
	}

	while (tried < retries + 1) {

		tried++;

		bps = Upstream_Detect(server_ip, my_ip, failure_code, downstream, settings);
		if (bps != 0) {
			StatsValid = true;
			if (PingsSent) {
				DebugString("Packet loss: %d percent\n", (100 * PingsLost) / PingsSent);
				Set_Registry_Int("PingLoss", (100 * PingsLost) / PingsSent);
				if (NumPingsCheckedForConsistency) {
					DebugString("Connection quality: %d percent\n", (100 * NumConsistentPings) / NumPingsCheckedForConsistency);
					Set_Registry_Int("Quality", (100 * NumConsistentPings) / NumPingsCheckedForConsistency);
				}
			}
			break;
		}

		/*
		** Error. See what the error code tells us.
		*/
		switch (failure_code) {

			/*
			** Fatal errors.
			*/
			case BANDTEST_NO_WINSOCK2:
			case BANDTEST_NO_RAW_SOCKET_PERMISSION:
			case BANDTEST_NO_RAW_SOCKET_CREATE:
			case BANDTEST_NO_UDP_SOCKET_BIND:
				tried = retries + 1;
				break;

			/*
			** Erros where retrying might help.
			*/
			case BANDTEST_NO_TTL_SET:
			case BANDTEST_NO_PING_RESPONSE:
			case BANDTEST_NO_FINAL_PING_TIME:
			case BANDTEST_NO_EXTERNAL_ROUTER:
			case BANDTEST_NO_IP_DETECT:
				break;

			/*
			** Errors we should never get.
			*/
			case BANDTEST_OK:
			case BANDTEST_UNKNOWN_ERROR:
			case BANDTEST_BAD_PARAM:
			default:
				DebugString("Failed with error code %d\n", failure_code);
				//assert(false);
				break;

		}
		if (tried < retries + 1) {
			DebugString("Retry %d\n", tried);
		}
	}


	timeEndPeriod(1);
	Close_Registry();
	return(bps);
}







/***********************************************************************************************
 * Upstream_Detect-- Try and figure out what our upstream bandwidth is                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP address of server to use as destination IP in tests.Server receives no packets *
 *           My IP.                                                                            *
 *           (out) Extended error code                                                         *
 *                                                                                             *
 * OUTPUT:   Bandwidth in bits per second. 0 = couldn't detect. 0xffffffff means **BIG**       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:21AM ST : Created                                                            *
 *=============================================================================================*/
unsigned int Upstream_Detect(unsigned int server_ip, unsigned int my_ip, int &failure_code, unsigned int &downstream, BandtestSettingsStruct *settings)
{
	struct sockaddr_in host_address;
	struct sockaddr_in address;
	char temp_buffer[640];
	int seq_id;
	int router_ttl = 0;
	struct sockaddr_in router_addr;
	unsigned short source_port = 1230;
	int ttl;
	unsigned short packet_sequencer = 0;
	float average_ping = 0.0f;
	//float lowest_ping = 0.0f;
	unsigned int ping_dest_address = 0;
	unsigned int path_to_server[256];
	int hops_to_server = 0;
	unsigned int upstream_bandwidth;
	unsigned int ping_times[100];
	unsigned int performance_timer = timeGetTime();
	unsigned int detect_start_time = performance_timer;
	int i;


	/*
	** Fill the temp buffer with garbage so it doesn't compress so well on modems.
	*/
	srand(timeGetTime());
	for (int b=0 ; b<sizeof(temp_buffer) ; b++) {
		temp_buffer[b] = (unsigned char) (rand() & 0xff);
	}

	/*
	** Try to open the sockets we need.
	*/
	if (!Open_Raw_Sockets(failure_code)) {
		return(0);
	}

	/*
	** Bind the UDP socket to a port.
	*/
	struct sockaddr_in temp_addr;
	temp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	temp_addr.sin_port = htons(source_port);
	temp_addr.sin_family = AF_INET;
	if (bind(RawSocket, (LPSOCKADDR) &temp_addr, sizeof(temp_addr)) == SOCKET_ERROR) {
		DebugString("bind failed with error code %d\n", WSAGetLastError());
		failure_code = BANDTEST_NO_UDP_SOCKET_BIND;
		Close_Raw_Sockets();
		return(0);
	}

	/*
	** Bind the ICMP socket to our local address.
	*/
#if (0)
	temp_addr.sin_port = 0;
	if (bind(ICMPRawSocket, (LPSOCKADDR) &temp_addr, sizeof(temp_addr)) == SOCKET_ERROR) {
		DebugString("bind failed with error code %d\n", WSAGetLastError());
		failure_code = BANDTEST_NO_UDP_SOCKET_BIND;
		Close_Raw_Sockets();
		return(0);
	}
#endif //(0)


	address.sin_addr.s_addr = htonl(server_ip);
	address.sin_port = 80;		//www port number. We can use anything here.
	address.sin_family = AF_INET;
	memcpy(&host_address, &address, sizeof(host_address));



	/*
	** If we don't know our own IP, ping someone we do know to find out.
	*/
	if (my_ip == 0) {

		performance_timer = timeGetTime();

		DebugString("Detecting my IP\n");
		Send_Ping(temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &address, 50);

		/*
		** Wait for a ping response.
		*/
		unsigned int start_time = timeGetTime();
		seq_id = -1;
		while (seq_id == -1) {
			Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
			if (timeGetTime() - start_time > 2500) {
				DebugString("Failed to get response to IP detect ping\n");
				failure_code = BANDTEST_NO_IP_DETECT;
				Close_Raw_Sockets();
				return(0);
			}
		};

		/*
		** See if the router is on the same network as me.
		*/
		if (seq_id == 50) {
			my_ip = ping_dest_address;
#ifdef _DEBUG
			ping_dest_address = htonl(my_ip);
			DebugString("Detected my IP as %s\n", Addr_As_String((unsigned char*)&ping_dest_address));
#endif //_DEBUG
		} else {
			DebugString("Unexpected response to IP detect ping\n");
			failure_code = BANDTEST_NO_IP_DETECT;
			Close_Raw_Sockets();
			return(0);
		}

		DebugString("Took %d ms to discover my IP address\n", timeGetTime() - performance_timer);
	}



	/*
	** Get the path to the server.
	**
	*/
	performance_timer = timeGetTime();
	hops_to_server = Get_Path_To_Server(&path_to_server[0], my_ip, server_ip);
	DebugString("Took %d ms to find path to server\n", timeGetTime() - performance_timer);

	if (hops_to_server == 0) {
		DebugString("Failed to get path to server\n");
		failure_code = BANDTEST_NO_EXTERNAL_ROUTER;
		Close_Raw_Sockets();
		return(0);
	}

	/*
	** Dump out the whole path to the server.
	*/
#ifdef _DEBUG
	DebugString("Found path to server...\n");
	for (i=0 ; i<hops_to_server ; i++) {
		unsigned int temp = htonl(path_to_server[i]);
		DebugString("   %02d : %s\n", i, Addr_As_String((unsigned char*)(&temp)));
	}
#endif //_DEBUG

	int first_router = 0;
	if (hops_to_server > 3) {
		first_router = 2;
	}

	for (i=first_router ; i<hops_to_server ; i++) {
		if ((path_to_server[i] & 0xffff0000) != (my_ip & 0xffff0000)) {

			/*
			** Ping the router once to see if it responds.
			*/
			int pings = Ping_Host(path_to_server[i], my_ip, 1, 0, ping_times, 2*TIMER_SECOND);
			if (pings == 0) {
				pings = Ping_Host(path_to_server[i], my_ip, 1, 0, ping_times, 2*TIMER_SECOND);
			}

			/*
			** If it responded, this is our man, ping it some more to establish a reference ping average.
			*/
			if (pings) {
				performance_timer = timeGetTime();
				router_ttl = i + 1;
				router_addr.sin_addr.s_addr = htonl(path_to_server[i]);
				router_addr.sin_port = 80;
				router_addr.sin_family = AF_INET;

				int num_pings = 15;
				unsigned int timeout = ping_times[0] * 3;
				if (ping_times[0] < 50) {
					num_pings = 50;
					timeout = 100;
				} else {
					if (ping_times[0] > 100) {
						num_pings = 6;
					}
				}

				pings = Ping_Host(path_to_server[i], my_ip, num_pings, 0, ping_times, timeout);

				if (pings) {
					//lowest_ping = Lowest_Ping(pings, ping_times);
					average_ping = Average_Ping(pings, ping_times, true);
					//DebugString("Lowest ping time to external router is %.2f ms\n", lowest_ping);
					DebugString("Average ping time to external router is %.2f ms\n", average_ping);
					DebugString("Took %d ms to find average ping\n", timeGetTime() - performance_timer);
					performance_timer = timeGetTime();
					break;
				} else {
					DebugString("Failed to ping external router\n");
					failure_code = BANDTEST_NO_PING_RESPONSE;
					Close_Raw_Sockets();
					return(0);
				}
			}
		}
	}


	if (router_ttl == 0) {
		DebugString("Failed to find external router\n");
		failure_code = BANDTEST_NO_EXTERNAL_ROUTER;
		Close_Raw_Sockets();
		return(0);
	}

	assert(router_ttl != 0);

	/*
	** If the ping time is low, it's *probably* a high bandwidth connection so we can get a more accurate test by sending more
	** packets without taking tooooo long.
	*/
	int num_udp_packets = settings->SlowPingPackets;
	unsigned int timeout = 8*TIMER_SECOND;
	if (average_ping < (float)(settings->FastPingThreshold)) {
		num_udp_packets = settings->FastPingPackets;
		timeout = 4*TIMER_SECOND;
	}

	/*
	** Set the UDP TTL to the next router down the list after the one we just pinged.
	*/
	SOCKET test_socket = RawSocket;
	if (settings->AlwaysICMP) {
		test_socket = ICMPRawSocket;
	}
	ttl = router_ttl + 1;
	int result = setsockopt(test_socket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
	if (result == SOCKET_ERROR) {
		DebugString("setsockopt failed to set IP_TTL = %d on test socket - error code %d\n", ttl, WSAGetLastError());
		failure_code = BANDTEST_NO_TTL_SET;
		Close_Raw_Sockets();
		return(0);
	}

	/*
	** Make the UDP socket buffer a bit bigger. Not really important...
	*/
	int socket_transmit_buffer_size = 128000;
	result = setsockopt(test_socket, SOL_SOCKET, SO_RCVBUF, (char*)&socket_transmit_buffer_size, 4);
	if (result == SOCKET_ERROR) {
		DebugString("setsockopt failed to set SO_RECVBUF with error code %d\n", WSAGetLastError());
	}

	/*
	** Make a note of the current time so we can see how long this whole process takes.
	*/
	unsigned int start_time = timeGetTime();
	int base_ttl = ttl;
	int max_ttl = hops_to_server - 1;
	if (max_ttl < base_ttl) {
		max_ttl = base_ttl;
	}

	performance_timer = timeGetTime();

	/*
	** Send a shitload of UDP packets to the next router down the list after the one we just pinged.
	*/
	DebugString("Sending %d 500 byte UDP packets\n", num_udp_packets);
	for (i=0 ; i<num_udp_packets ; i++) {

		/*
		** Use a different TTL for each packet if requested.
		*/
		if (settings->TTLScatter) {
			ttl++;
			if (ttl > max_ttl) {
				ttl = base_ttl;
			}
			int result = setsockopt(test_socket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
			if (result == SOCKET_ERROR) {
				DebugString("setsockopt failed to set IP_TTL = %d on test socket - error code %d\n", ttl, WSAGetLastError());
				failure_code = BANDTEST_NO_TTL_SET;
			}
		}

		/*
		** We have to fill the packet with different garbage every time we send it otherwise a modem will be able to
		** do delta compression on the packets.
		*/
		for (int b=0 ; b<sizeof(temp_buffer) ; b++) {
			temp_buffer[b] = (unsigned char) (rand() & 0xff);
		}
		if (settings->AlwaysICMP) {
			Send_Ping(temp_buffer, 466, ICMPRawSocket, (struct sockaddr *) &host_address, 0);
		} else {
			Send_Raw_UDP(temp_buffer, 466, RawSocket, (struct sockaddr *) &host_address, 1234, 4321);
		}
	}

	DebugString("Took %d ms to send bulk packets\n", timeGetTime() - performance_timer);
	performance_timer = timeGetTime();

	/*
	** Set the TTL to max on the ICMP socket. This shouldn't be needed but I'm doing it just in case there are any bugs in
	** windoze that might confuse TTL settings between sockets.
	*/
	ttl = 255;
	result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
	if (result == SOCKET_ERROR) {
		DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", ttl, WSAGetLastError());
	}

	/*
	** Now see what the ping time to the router is. Since there are n UDP packets ahead of this ping before it goes out, the
	** ping time will include the time taken to send the UDP packets.
	*/
	unsigned int new_router_ping_time = 0xffffffff;
	packet_sequencer = 5;
	unsigned int second_start_time = timeGetTime();
	float total_time = 0.0f;

	/*
	** Send a couple of pings to reduce the possibility of packet loss being a factor.
	*/
	Send_Ping((char*)temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &router_addr, packet_sequencer);
	Send_Ping((char*)temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &router_addr, packet_sequencer + 1);

	seq_id = -1;
	while (seq_id != packet_sequencer && seq_id != packet_sequencer+1) {
		Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, 0, ping_dest_address);
		if (timeGetTime() - second_start_time > timeout) {
			break;
		}
	};
	if (seq_id == packet_sequencer || seq_id == packet_sequencer + 1) {
		unsigned int time_now = timeGetTime();
		new_router_ping_time = time_now - second_start_time;
		total_time = (float)(time_now - start_time);
		DebugString("Ping time to external router is now %d ms\n", new_router_ping_time);
		DebugString("Total time to send %d bytes plus send and receive ping is %d ms\n", num_udp_packets * 500, total_time);
	} else {
		DebugString("Failed to get final ping response in %d seconds\n", timeout / 1000);
		failure_code = BANDTEST_NO_FINAL_PING_TIME;
		Close_Raw_Sockets();
		return(0);
	}

	if (new_router_ping_time == 0xffffffff) {
		DebugString("Failed to get final ping response\n");
		failure_code = BANDTEST_NO_FINAL_PING_TIME;
		return(0);
	} else {

		total_time -= average_ping;

		/*
		** Work out the bandwidth.
		** Approx bps up = ((10000 + 28) * 8) / (time2 - time1).
		*/
		if (((unsigned int) total_time) == 0 || (total_time > ((float)0x10000000))) {
			DebugString("Upstream bandwidth is huge :-)\n");
			failure_code = BANDTEST_OK;
			upstream_bandwidth =  0xffffffff;
		} else {
			unsigned int bw = (((num_udp_packets * 500) * 8) * 1000) / (unsigned int)total_time;
			if (bw > 100000) {
				float floater = (float)bw / 1024;
				DebugString("Upstream bandwidth to external router is %.1f kilobits per second\n", floater);
			} else {
				DebugString("Upstream bandwidth to external router is %d bits per second\n", bw);
			}
			failure_code = BANDTEST_OK;
			upstream_bandwidth = bw;
			//return(bw);
		}
	}

	DebugString("Took %d ms to get new ping time\n", timeGetTime() - performance_timer);


	/*
	** If the bandwidth in the registry is close to what we just calculated then use the old downstream calculation from the
	** registry.
	*/
	unsigned int downstream_bandwidth = upstream_bandwidth;
	int old_band = Get_Registry_Int("Up", 0);
	unsigned int diff = abs(int(upstream_bandwidth - old_band));
	bool calc_down = true;
	if (diff < upstream_bandwidth / 10) {
		downstream_bandwidth = Get_Registry_Int("Down", upstream_bandwidth);
		if (downstream_bandwidth) {
			calc_down = false;
		}
	}

	/*
	** Store the calculated bandwidth into the registry.
	*/
	Set_Registry_Int("Up", upstream_bandwidth);


	/*
	**
	**  Well, I suppose, since we are here, we might as well have a stab at downstream bandwidth too.
	**
	**
	**  Try sending max size pings to our friendly router and subtracting how long we think the upstream should have taken
	**  from the average ping time. Assume the routers latency is 0 (which of course it isn't).
	*/
	bool method_one = false;
	//float average_time_exceeded = 0.0f;
	performance_timer = timeGetTime();

	if (calc_down && upstream_bandwidth < 576 * 1000 && upstream_bandwidth > 8) {

		int new_ping_timeout = std::max((int)(average_ping * 5), 200);
#if (0)
		if (upstream_bandwidth > 80000) {
			method_one = true;
		}

		if (method_one) {

			DebugString("Detecting downstream bandwidth - method 1\n");

			ttl = router_ttl + 1;
			int result = setsockopt(RawSocket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
			if (result == SOCKET_ERROR) {
				DebugString("setsockopt failed to set IP_TTL = %d on test socket - error code %d\n", ttl, WSAGetLastError());
				failure_code = BANDTEST_NO_TTL_SET;
				Close_Raw_Sockets();
				return(0);
			}

			/*
			** First we had better wait for all those time exceeded packets to come back.
			*/
			seq_id = -1;
			start_time = timeGetTime();
			unsigned int last_icmp_in_time = start_time;

			for (;;) {
				Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
				if (seq_id != -1) {
					seq_id = -1;
					last_icmp_in_time = timeGetTime();
				}
				if (timeGetTime() - start_time > 3*TIMER_SECOND) {
					break;
				}

				if (timeGetTime() - last_icmp_in_time > TIMER_SECOND / 2) {
					break;
				}
			}


			/*
			** Send a shitload of UDP packets to the next router down the list after the one we just pinged.
			** Wait for a response for each packet before sending the next.
			*/
			num_udp_packets = 50;
			DebugString("Sending %d 500 byte UDP packets\n", num_udp_packets);
			int num_pings = 0;
			for (i=0 ; i<num_udp_packets ; i++) {

				/*
				** We have to fill the packet with different garbage every time we send it otherwise a modem will be able to
				** do delta compression on the packets.
				*/
				for (int b=0 ; b<sizeof(temp_buffer) ; b++) {
					temp_buffer[b] = (unsigned char) (rand() & 0xff);
				}
				start_time = timeGetTime();
				Send_Raw_UDP(temp_buffer, 540, RawSocket, (struct sockaddr *) &host_address, 1234, 4321);

				seq_id = -1;
				for (;;) {
					Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
					if (seq_id != -1) {
						seq_id = -1;
						unsigned int ping_time = timeGetTime() - start_time;
						ping_times[num_pings++] = ping_time;
						break;
					}
					if (timeGetTime() - start_time > (unsigned int)new_ping_timeout) {
						break;
					}
				}
			}

			/*
			** This is the average time taken to get the TIME EXCEEDED message back.
			*/
			DebugString("Quickest time exceeded came in after %.2f ms\n", Lowest_Ping(num_pings, ping_times));
			average_time_exceeded = Average_Ping(num_pings, ping_times, true);
			DebugString("Average time exceeded came in after %.2f ms\n", average_time_exceeded);

		}
#endif //(0)


		DebugString("Detecting downstream bandwidth - method 2\n");

		/*
		** First we had better wait for all those time exceeded packets to come back.
		*/
		seq_id = -1;
		start_time = timeGetTime();
		unsigned int last_icmp_in_time = start_time;

		for (;;) {
			Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
			if (seq_id != -1) {
				seq_id = -1;
				last_icmp_in_time = timeGetTime();
			}
			if (timeGetTime() - start_time > 3*TIMER_SECOND) {
				break;
			}

			if (timeGetTime() - last_icmp_in_time > TIMER_SECOND / 2) {
				break;
			}
		}

		//int new_ping_timeout = std::max((int)(average_ping * 5), 200);
		float old_average_ping = average_ping;
		//float old_lowest_ping = lowest_ping;
		average_ping = 0.0f;

		/*
		** Set the TTL back to max.
		*/
		int new_ttl = 255;
		int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&new_ttl, sizeof(new_ttl));
		if (result == SOCKET_ERROR) {
			DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", new_ttl, WSAGetLastError());
			failure_code = BANDTEST_NO_TTL_SET;
			Close_Raw_Sockets();
			return(upstream_bandwidth);
		}

		/*
		** Ping the router once to get a ballpark trip time.
		*/
		int pings = Ping_Host(ntohl(router_addr.sin_addr.s_addr), my_ip, 1, 540, ping_times, new_ping_timeout);
		if (pings == 0) {
			pings = Ping_Host(ntohl(router_addr.sin_addr.s_addr), my_ip, 1, 540, ping_times, 2 * TIMER_SECOND);
			if (pings == 0) {
				pings = Ping_Host(ntohl(router_addr.sin_addr.s_addr), my_ip, 1, 540, ping_times, 2 * TIMER_SECOND);
			}
		}

		if (pings) {

			/*
			** Do more pings if the ping time is low. User a smaller timeout too.
			*/
			int num_pings = 15;
			unsigned int timeout = ping_times[0] * 3;
			if (ping_times[0] < 100) {
				num_pings = 50;
				timeout = 200;
			} else {
				if (ping_times[0] > 250) {
					num_pings = 6;
				}
			}
			if (method_one) {
				num_pings = 50;
			}

			DebugString("Sending large pings\n");
			pings = Ping_Host(ntohl(router_addr.sin_addr.s_addr), my_ip, num_pings, 540, ping_times, timeout);

			if (pings) {
				//lowest_ping = Lowest_Ping(pings, ping_times);
				DebugString("Quickest ping came in after %.2f ms\n", Lowest_Ping(num_pings, ping_times));
				average_ping = Average_Ping(pings, ping_times, true);
				//DebugString("Lowest ping time to external router is now %.2f ms\n", lowest_ping);
				DebugString("Average ping time to external router is now %.2f ms\n", average_ping);
			} else {
				DebugString("Failed to ping external router\n");
				failure_code = BANDTEST_NO_PING_RESPONSE;
				Close_Raw_Sockets();
				return(upstream_bandwidth);
			}

			/*
			** Just to make sure we can never get a divide by 0.
			*/
			if (upstream_bandwidth / 8 == 0) {
				upstream_bandwidth = 56000;
			}

			float time_upstream = (1000.0f * 540.0f) / ((float)(upstream_bandwidth / 8));
			DebugString("Time upstream is %.1f ms\n", time_upstream);
			float time_downstream = (float)((average_ping - time_upstream) - old_average_ping);

			/*
			** 576 bytes took 'time_downstream' ms to come downstream.
			*/
			if (time_downstream > 0.0) {
				float dbw = ((1000.0f / time_downstream) * 4320.0f);		// 540*8 = 4320
				//downstream_bandwidth = ((1000.0f / time_downstream) * 576 * 8);
				downstream_bandwidth = (int)dbw;
			}

			DebugString("Took %d ms to calculate downstream bandwidth\n", timeGetTime() - performance_timer);
			Set_Registry_Int("Down", downstream_bandwidth);
		}
	}

	if (settings->PingProfile) {
		Ping_Profile(&router_addr, my_ip);
	}

	Close_Raw_Sockets();

	if (downstream_bandwidth > 100000) {
		float floater = (float)downstream_bandwidth / 1024;
		DebugString("Downstream bandwidth from external router is %.1f kilobits per second\n", floater);
	} else {
		DebugString("Downstream bandwidth from external router is %d bits per second\n", downstream_bandwidth);
	}

	/*
	** Method 1 just uses the difference between then time exceeded pings and the echo pings.
	*/
#if (0)
	if (method_one) {
		float time_down = average_ping - average_time_exceeded;

		if (time_down > 0) {
			float bps_down = (1000.0f / time_down) * (float)(540);
			bps_down = bps_down * 8.0f;

			if (bps_down > 100000) {
				float floater = (float)bps_down / 1024;
				DebugString("Downstream bandwidth from external router by alt method is %.1f kilobits per second\n", floater);
			} else {
				DebugString("Downstream bandwidth from external router by alt method is %f bits per second\n", bps_down);
			}
		}
	}
#endif //(0)


	/*
	** Assume down is at least as much as up.
	*/
	if (downstream_bandwidth < upstream_bandwidth) {
		downstream_bandwidth = upstream_bandwidth;
	}

	downstream = downstream_bandwidth;

	DebugString("Total time to detect bandwidth = %d ms\n", timeGetTime() - detect_start_time);

	failure_code = BANDTEST_OK;
	return(upstream_bandwidth);
}








void Ping_Profile(struct sockaddr_in *router_addr, unsigned int my_ip)
{
	float ping_averages[1000];
	unsigned int ping_times[100];
	char temp_buffer[128];
	char temp_graph[30][80];

	static char _ping_graph[30][80] = {
		"      ms|                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"  Ping  |                                                       \n",
		"  Time  |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"        |                                                       \n",
		"      ms|                                                       \n",
		"________|_______________________________________________________\n",
		"         0                Payload Size                       540\n",
		"                                                                \n",
		"                                                                \n",
	};



	DebugString("Profiling ping responses\n");
	memcpy(temp_graph, _ping_graph, sizeof(temp_graph));

	/*
	** Set the TTL back to max.
	*/
	int new_ttl = 255;
	int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&new_ttl, sizeof(new_ttl));
	if (result == SOCKET_ERROR) {
		DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", new_ttl, WSAGetLastError());
	}
	int ping_number = 0;

	for (int packet_size = 0 ; packet_size < 541 ; packet_size += 10) {

		/*
		** Ping the router once to get a ballpark trip time.
		*/
		int pings = Ping_Host(ntohl(router_addr->sin_addr.s_addr), my_ip, 1, packet_size, ping_times, 1000);
		if (pings == 0) {
			pings = Ping_Host(ntohl(router_addr->sin_addr.s_addr), my_ip, 1, packet_size, ping_times, 2 * TIMER_SECOND);
			if (pings == 0) {
				pings = Ping_Host(ntohl(router_addr->sin_addr.s_addr), my_ip, 1, packet_size, ping_times, 2 * TIMER_SECOND);
			}
		}

		if (pings) {

			/*
			** Do more pings if the ping time is low. User a smaller timeout too.
			*/
			int num_pings = 15;
			unsigned int timeout = ping_times[0] * 3;
			if (ping_times[0] < 100) {
				num_pings = 30;
				timeout = 200;
			} else {
				if (ping_times[0] > 250) {
					num_pings = 6;
				}
			}

			DebugString("Sending %d byte pings\n", packet_size);
			pings = Ping_Host(ntohl(router_addr->sin_addr.s_addr), my_ip, num_pings, packet_size, ping_times, timeout);

			if (pings) {
				float average_ping = Average_Ping(pings, ping_times, true);
				DebugString("Average ping time to external router is now %.2f ms\n", average_ping);
				ping_averages[ping_number] = average_ping;
			} else {
				DebugString("Failed to ping external router\n");
				ping_averages[ping_number] = 0.0;

			}
		}
		ping_number++;
	}


	/*
	** Scale the ping graph.
	*/
	float min_ping = 10000.0f;
	float max_ping = -1.0f;
	int i;

	for (i=0 ; i<ping_number ; i++) {
		min_ping = std::min(min_ping, ping_averages[i]);
		max_ping = std::max(max_ping, ping_averages[i]);
	}

	sprintf(temp_buffer, "%3.1f", max_ping);
	strncpy(&temp_graph[0][0], temp_buffer, strlen(temp_buffer));
	sprintf(temp_buffer, "%3.1f", min_ping);
	strncpy(&temp_graph[25][0], temp_buffer, strlen(temp_buffer));

	float scale = 25.0f / (max_ping - min_ping);

	/*
	** Draw the pings onto the graph.
	*/
	for (i=0 ; i<ping_number ; i++) {
		float ping = ping_averages[i];
		int position = (int)((ping - min_ping) * scale);

		position = 25 - position;
		temp_graph[position][i+9] = '*';
	}

	/*
	** Dump it out.
	*/
	for (i=0 ; i<30 ; i++) {
		DebugString(temp_graph[i]);
		_cprintf(temp_graph[i]);
	}
}












/***********************************************************************************************
 * Get_Path_To_Server -- Trace the route to the server                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to array to receive path IPs                                                  *
 *           My IP address                                                                     *
 *           Servers IP address                                                                *
 *                                                                                             *
 * OUTPUT:   Number of hops in path                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/2001 2:07PM ST : Created                                                             *
 *=============================================================================================*/
int Get_Path_To_Server(unsigned int *path, unsigned int my_ip, unsigned int server_ip)
{
	char reg_name[128];
	int path_size = 0;
	struct sockaddr_in host_address;
	struct sockaddr_in address;
	char temp_buffer[640];
	int seq_id;
	unsigned int ping_dest_address = htonl(my_ip);

	/*
	** See if the path in the registry looks valid.
	*/
	int reg_my_ip = Get_Registry_Int("MyIP", 0);
	int reg_server_ip = Get_Registry_Int("ServerIP", 0);
	int reg_path_length = Get_Registry_Int("PathLength", 0);
	int reg_path_time = Get_Registry_Int("PathValid", 0);

	/*
	** If the ip at either end of the route has changed then the path isn't valid anymore.
	*/
	if (((unsigned int)reg_my_ip) == my_ip && ((unsigned int)reg_server_ip) == server_ip) {

		/*
		** If the path is too old then we should probably not consider it valid.
		*/
		unsigned int time = timeGetTime();
		unsigned int last_path_time = (unsigned int) reg_path_time;

		/*
		** Lets only use it if it's less than 2 hours old.
		*/
		if ((last_path_time < time) && (time - last_path_time < (TIMER_SECOND * 60 * 120))) {

			/*
			** OK, the path in the registry looks good - just return that.
			*/
			for (int i=0 ; i<reg_path_length ; i++) {
				sprintf(reg_name, "Path%02d", i);
				path[i] = (unsigned int) Get_Registry_Int(reg_name, 0);
				if (path[i]) {
					path_size++;
				} else {
					break;
				}
			}

			if (reg_path_length == path_size) {
				DebugString("Using path from registry\n");
				return(path_size);
			}
		}
	}


	/*
	** The path in the registry isn't any good. Discover it for ourselves.
	*/
	address.sin_addr.s_addr = htonl(server_ip);
	address.sin_port = 80;		//www port number. We can use anything here.
	address.sin_family = AF_INET;
	memcpy(&host_address, &address, sizeof(host_address));



	/*
	** Try to find a router with a different address class than ours. Start at TTL = 1 and increase until we find what we
	** are looking for.
	*/
	int ttl = 1;
	int hops_to_server = 0;
	int packet_sequencer = 0;

	for (ttl = 1 ; ttl < 100 ; ttl++) {

		memcpy(&address, &host_address, sizeof(address));

		/*
		** Set the TTL.
		*/
		int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
		if (result == SOCKET_ERROR) {
			DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", ttl, WSAGetLastError());
			return(0);
		}

		unsigned int start_time = timeGetTime();

		/*
		** Send a ping with the previously set TTL.
		*/
		DebugString("Sending ping with TTL = %d\n", ttl);
		Send_Ping(temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &address, packet_sequencer);
		//DebugString("Ping sent\n");

		/*
		** Wait for a ping response.
		*/
		seq_id = -1;
		while (seq_id == -1) {
			Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
			if (timeGetTime() - start_time > 1500) {
				DebugString("Failed to get any response to ping with TTL = %d\n", ttl);
				break;
			}
		};

		if (seq_id != -1) {
			unsigned int long_router_addr = ntohl(address.sin_addr.s_addr);
			path[hops_to_server++] = long_router_addr;

			/*
			** See if this router is the target. If so, we are at the end of the path.
			*/
			if (long_router_addr  == server_ip || seq_id == packet_sequencer) {
				break;
			}
		}
		packet_sequencer++;
	}

	/*
	** If we got a good path then store it in the registry.
	*/
	if (hops_to_server > 0 && path[hops_to_server - 1] == server_ip) {

		Set_Registry_Int("MyIP", my_ip);
		Set_Registry_Int("ServerIP", server_ip);
		Set_Registry_Int("PathLength", hops_to_server);
		Set_Registry_Int("PathValid", (int)timeGetTime());

		for (int i=0 ; i<hops_to_server ; i++) {
			sprintf(reg_name, "Path%02d", i);
			Set_Registry_Int(reg_name, path[i]);
		}
		return(hops_to_server);
	}

	return(0);
}





/***********************************************************************************************
 * Ping_Host -- Send a number of pings to a host and wait for responses                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Host IP address to ping                                                           *
 *           My IP address                                                                     *
 *           Number of pings to send                                                           *
 *           Payload size of ping                                                              *
 *           Ptr to array to receive ping times                                                *
 *           Timeout for each ping                                                             *
 *                                                                                             *
 * OUTPUT:   Number of pings received                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/2001 2:09PM ST : Created                                                             *
 *=============================================================================================*/
int Ping_Host(unsigned int host_ip, unsigned int my_ip, int times, int payload_size, unsigned int *ping_times, unsigned int timeout)
{
	static int _packet_sequencer = 0;
	int num_pings = 0;
	char temp_buffer[640];
	struct sockaddr_in host_addr;
	struct sockaddr_in address;
	unsigned int ping_dest_address = htonl(my_ip);

	/*
	** Set the TTL back to max.
	*/
	int new_ttl = 255;
	int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&new_ttl, sizeof(new_ttl));
	if (result == SOCKET_ERROR) {
		DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", new_ttl, WSAGetLastError());
		return(0);
	}

	host_addr.sin_addr.s_addr = htonl(host_ip);
	host_addr.sin_port = 80;								//www port number. We can use anything here.
	host_addr.sin_family = AF_INET;

	/*
	** Ping n times.
	*/
	for (int i=0 ; i<times ; i++) {

		for (int b=0 ; b<sizeof(temp_buffer) ; b++) {
			temp_buffer[b] = (unsigned char) (rand() & 0xff);
		}

		/*
		** Record the time before sending the ping.
		*/
		unsigned int start_time = timeGetTime();

		/*
		** Send the ping.
		*/
		Send_Ping((char*)temp_buffer, payload_size, ICMPRawSocket, (struct sockaddr *) &host_addr, _packet_sequencer);
		PingsSent++;

		int seq_id = -1;

		/*
		** Wait for the ping response.
		*/
		while (seq_id != _packet_sequencer) {
			Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_addr.sin_addr.s_addr), ping_dest_address);
			if (timeGetTime() - start_time > timeout) {
				DebugString("Failed to get response to reference ping %d\n", i);
				PingsLost++;
				break;
			}
		};

		/*
		** Check the time now and record it as a ping time.
		*/
		if (seq_id == _packet_sequencer) {
			unsigned int router_ping_time = timeGetTime() - start_time;
			DebugString("Ping time %d to external router %s is %d ms\n", num_pings, Addr_As_String2(&host_addr), router_ping_time);
			ping_times[num_pings++] = router_ping_time;
		}
		_packet_sequencer++;
	}

	/*
	** Try and get a measure of how far off center some of the pings are.
	*/
	if (num_pings > 5) {
		NumPingsCheckedForConsistency += num_pings;

		unsigned int *ping_copies = (unsigned int*) _alloca(num_pings * 4);
		memcpy(ping_copies, ping_times, num_pings * 4);
		float average_ping = Average_Ping(num_pings, ping_copies, true);
		float error_permit = 0.25f;
		if (average_ping < 100.0f) {
			error_permit = 0.35f;
		}
		if (average_ping < 30.0f) {
			error_permit = 0.5f;
		}
		for (int p=0 ; p<num_pings ; p++) {
			float diff = ((float)ping_times[p]) - average_ping;
			if (diff < 0) {
				diff = -diff;
			}
			if (diff < average_ping * error_permit) {
				NumConsistentPings++;
			}
		}
	}

	return(num_pings);
}







/***********************************************************************************************
 * Ping_Compare -- qsort compare function for unsigned longs                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ping 1                                                                            *
 *           ping 2                                                                            *
 *                                                                                             *
 * OUTPUT:   result of comparison                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/2001 2:13PM ST : Created                                                             *
 *=============================================================================================*/
int __cdecl Ping_Compare(const void *ping1, const void *ping2)
{
	unsigned int p1 = *((unsigned int*)ping1);
	unsigned int p2 = *((unsigned int*)ping2);

	if (p1 == p2) {
		return(0);
	}

	if (p1 < p2) {
		return(-1);
	}

	return(1);
}


/***********************************************************************************************
 * Sort_Pings -- Sort the ping list into ascending order                                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of pings in list                                                           *
 *           Ptr to ping time array                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/2001 2:14PM ST : Created                                                             *
 *=============================================================================================*/
void Sort_Pings(int num_pings, unsigned int *ping_times)
{
	qsort(ping_times, num_pings, sizeof(unsigned int), &Ping_Compare);
}


/***********************************************************************************************
 * Average_Ping -- Calculate the average ping time from a list of pings.                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of pings                                                                   *
 *           Ptr to ping times array                                                           *
 *           Ignore low & high - filter out very low and very high values from average         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/2001 2:14PM ST : Created                                                             *
 *=============================================================================================*/
float Average_Ping(int num_pings, unsigned int *ping_times, bool ignore_low_high)
{
	if (ignore_low_high && num_pings > 2) {
		Sort_Pings(num_pings, ping_times);
		memmove((char*)ping_times, ((char*)ping_times) + 4, 4 * (num_pings - 2));
		num_pings -= 2;
	}

	float average_ping = 0.0;
	for (int i=0 ; i<num_pings ; i++) {
		average_ping += (float)(ping_times[i]);
	}
	average_ping = average_ping / (float) num_pings;

	/*
	** If we have lots of pings, see if we can improve the accurracy by filtering out very small or very big pings.
	*/
	if (num_pings > 5) {
		float filter_percent = 0.25f;
		if (average_ping < 10.0) {
			filter_percent = 0.30f;
		}

		float new_average = 0;
		int num_considered = 0;
		for (int i=0 ; i<num_pings ; i++) {
			float diff = ((float)ping_times[i]) - average_ping;
			if (diff < 0) {
				diff = -diff;
			}
			if (diff < average_ping * 0.25f) {
				new_average += (float) ping_times[i];
				num_considered++;
			}
		}
		if (num_considered > 3) {
			average_ping = new_average / (float) num_considered;
		}
	}
	return(average_ping);
}





/***********************************************************************************************
 * Lowest_Ping -- Get the lowest ping time from a list of pings.                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of pings                                                                   *
 *           Ptr to ping times array                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/9/2001 5:03PM ST : Created                                                             *
 *=============================================================================================*/
float Lowest_Ping(int num_pings, unsigned int *ping_times)
{
	float lowest_ping = 1000000.0;
	for (int i=0 ; i<num_pings ; i++) {
		lowest_ping = std::min(lowest_ping, (float)(ping_times[i]));
	}
	return(lowest_ping);
}






/***********************************************************************************************
 * Send_Raw_UDP -- Send a raw UDP packet.                                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to buffer to copy packet payload from                                         *
 *           Size of packet payload                                                            *
 *           Raw UDP socket to use                                                             *
 *           Address to send packet to                                                         *
 *           Source port to put in the UDP header                                              *
 *           Dest port to put in the UDP header                                                *
 *                                                                                             *
 * OUTPUT:   True if sent OK                                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:46AM ST : Created                                                            *
 *=============================================================================================*/
bool Send_Raw_UDP(char *payload, int payload_size, SOCKET socket, struct sockaddr *address, unsigned short source_port, unsigned short dest_port)
{
	/*
	** Asserts.
	*/
	assert(payload_size < 550);
	assert(socket != INVALID_SOCKET);
	assert(source_port >= 1024);
	assert(dest_port >= 1024);
	assert(address != NULL);

	if (address == NULL) {
		return(false);
	}
	if (payload_size > 550) {
		return(false);
	}

	/*
	** Build the header on the stack for convenience.
	*/
	unsigned char packetbuf[1024];
	UDPHeaderType *header = (UDPHeaderType*) packetbuf;

	/*
	** Fill in the header fields.
	*/
	header->SourcePort = source_port;
	header->DestPort = htons(dest_port);
	header->Length = (unsigned short) (sizeof(UDPHeaderType) + payload_size);
	header->Checksum = 0;

	/*
	** Copy the payload into place.
	*/
	char *payload_ptr = ((char*)header) + sizeof(UDPHeaderType);
	memcpy(payload_ptr, payload, payload_size);

	/*
	** Fix up the UDP header checksum.
	*/
	int packet_size = payload_size + sizeof(*header);
	header->Checksum = Get_IP_Checksum((unsigned short *)header, packet_size);

	/*
	** Send it.
	*/
	((struct sockaddr_in*)address)->sin_port = htons(dest_port);
	int result = sendto(socket, (char*)header, packet_size, 0, address, sizeof(*address));

	if (result == SOCKET_ERROR) {
		DebugString("Send_Raw_UDP - sendto failed with error code %d\n", WSAGetLastError());
		return(false);
	}

	if (result != packet_size) {
		return(false);
	}

	return(true);
}





/***********************************************************************************************
 * Send_Ping -- Use raw ICMP socket to send a ping message.                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to buffer to copy packet payload from                                         *
 *           Size of packet payload                                                            *
 *           Socket to use                                                                     *
 *           Address to ping                                                                   *
 *                                                                                             *
 * OUTPUT:   True if ping sent OK                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:51AM ST : Created                                                            *
 *=============================================================================================*/
bool Send_Ping(char *payload, int payload_size, SOCKET socket, struct sockaddr *address, int sequence_id)
{
	//DebugString("Send ping. payload=%08X, size=%d, socket=%d, address=%s, seq_id=%d\n", payload, payload_size, socket, Addr_As_String2((struct sockaddr_in *)address), sequence_id);
	/*
	** Asserts.
	*/
	assert(payload_size < 550);
	assert(socket != INVALID_SOCKET);
	assert(address != NULL);

	if (address == NULL) {
		return(false);
	}
	if (payload_size > 550) {
		return(false);
	}

	/*
	** Build the header on the stack for convenience.
	*/
	unsigned char packetbuf[1024];
	ICMPHeaderType *header = (ICMPHeaderType*) packetbuf;

	/*
	** Set up the ICMP header fields.
	*/
	header->Type = ICMP_ECHO;
	header->Code = 0;
	header->ID = (unsigned short) (GetCurrentProcessId() & 0xffff);
	header->Checksum = 0;
	header->Sequence = (unsigned short) sequence_id;

	/*
	** Copy the payload into position.
	*/
	char *payload_ptr = ((char*)header) + sizeof(ICMPHeaderType);
	memcpy(payload_ptr, payload, payload_size);

	/*
	** Fix up the checksum in the ICMP header.
	*/
	int packet_size = payload_size + sizeof(*header);
	header->Checksum = Get_IP_Checksum((unsigned short *)header, packet_size);

	/*
	** Send it.
	*/
	//DebugString("sendto\n");
	int result = sendto(socket, (char*)header, packet_size, 0, address, sizeof(*address));
	//DebugString("returned from sendto\n");

	if (result == SOCKET_ERROR) {
		DebugString("sendto failed with error code %d\n", WSAGetLastError());
		return(false);
	}

	if (result != packet_size) {
		return(false);
	}

	return(true);
}






/***********************************************************************************************
 * Get_Ping_Response -- Wait for an ICMP echo reply or time exceeded message                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket                                                                            *
 *           Packet sequence id to look for                                                    *
 *           (out) Ptr to buffer to put sender address in                                      *
 *           Address to use to validate pings (i.e. address ping was sent to)                  *
 *           (out) My address - filled in on ping responses                                    *
 *                                                                                             *
 * OUTPUT:   True if we got a ping response                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 12:57PM ST : Created                                                            *
 *=============================================================================================*/
bool Get_Ping_Response(SOCKET socket, int &seq_id, struct sockaddr *address, unsigned int validate_address, unsigned int &my_address)
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	char recv_buffer[1024];

	u_long bytes;
	int result = ioctlsocket(socket, FIONREAD, &bytes);

	/*
	** Result of 0 is success.
	*/
	if (result != 0) {
		Sleep(0);
		return(false);
	} else {

		/*
		** If there is outstanding data, 'bytes' will contain the size of the next queued datagram.
		*/
		if (bytes == 0) {
			Sleep(0);
			return(false);
		} else {

			/*
			** Call recvfrom function to get the outstanding packet.
			*/
			addr_len = sizeof(addr);
			result = recvfrom(socket, recv_buffer, sizeof(recv_buffer), 0, (LPSOCKADDR)&addr, &addr_len);

			if (result == SOCKET_ERROR) {
				DebugString("recvfrom failed with error code %d\n", WSAGetLastError());
				return(false);
			}

			//DebugString("Get_Ping_Response - received packet %d bytes long\n", result);
			//DebugString("Get_Ping_Response - recvfrom %s\n", Addr_As_String2(&addr));

			if (result < (sizeof(IPHeaderType) + sizeof(ICMPHeaderType))) {
				DebugString("Get_Ping_Response - packet header too small\n");
				return(false);
			}

			/*
			** Decode the ping response. We get the IP header and all.
			*/
			IPHeaderType *ip_header = (IPHeaderType*) recv_buffer;

			/*
			** Get the header length. Length specified in the header is in longs so multiply it by 4 to get bytes.
			*/
			int ip_header_size = ip_header->Length * 4;

			if (result < (ip_header_size + (int)sizeof(ICMPHeaderType))) {
				DebugString("Get_Ping_Response - packet header reported size too big\n");
				return(false);
			}

			if (ip_header->Protocol == PROTOCOL_ICMP) {

				//DebugString("Protocol is ICMP\n");

				/*
				** Figure out where the ICMP header is.
				*/
				//IPHeaderType *ip_header = (IPHeaderType*) recv_buffer;
				ICMPHeaderType *icmp_header = (ICMPHeaderType*) (recv_buffer + ip_header_size);

				my_address = ntohl(ip_header->DestIP);

				switch (icmp_header->Type) {

					/*
					** An echo reply is basically a ping response.
					*/
					case ICMP_ECHO_REPLY:
						//DebugString("Type is ICMP_ECHO_REPLY\n");
						if (icmp_header->ID == (unsigned short)(GetCurrentProcessId() & 0xffff)) {
							memcpy(address, &addr, addr_len);
							seq_id = icmp_header->Sequence;
							return(true);
						}
						break;

					/*
					** A time exceeded is sent when a router discards a packet due to a TTL of 0.
					*/
					case ICMP_TIME_EXCEEDED:
						//DebugString("Type is ICMP_TIME_EXCEEDED\n");
						{
							/*
							** Find the packets original IP header. ICMP_TIME_EXCEEDED is 8 bytes followed by a copy of the original IP
							** header followed by the original ICMP header followed by 64 bits of the original payload. Phew. (the original
							** payload seems to be frequently lost).
							*/
							IPHeaderType *original_ip_header = (IPHeaderType*)(((char*)icmp_header) + 8);
							if (ntohl(original_ip_header->DestIP) == validate_address) {
								DebugString("Received ICMP_TIME_EXCEEDED from %s\n", Addr_As_String((unsigned char*)&ip_header->SourceIP));
								memcpy(address, &addr, addr_len);
								seq_id = 100000;
								return(true);
							}
						}
						break;

					default:
						DebugString("Type is %d\n", icmp_header->Type);
						break;
				}
			} else {
				DebugString("Protocol is %d\n", ip_header->Protocol);
			}

		}
	}
	return(false);
}









/***********************************************************************************************
 * Open_Raw_Sockets -- Initialize winsock and create the raw sockets we need                   *
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
 *   10/3/2001 11:31AM ST : Created                                                            *
 *=============================================================================================*/
bool Open_Raw_Sockets(int &failure_code)
{
	WSADATA wsa_data;
	bool use_group = false;	//true;

	/*
	** We need Winsocl 2 for raw sockets.
	*/
	if (WSAStartup(MAKEWORD(2,1), &wsa_data) != 0) {
		DebugString("WSAStartup failed: error code %d\n", WSAGetLastError());
		failure_code = BANDTEST_NO_WINSOCK2;
		return(false);
	}

	/*
	** Create a socket for UDP packets.
	*/
#ifdef _WIN32
	if (use_group) {
		RawSocket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, SG_UNCONSTRAINED_GROUP, 0);
		if (RawSocket == INVALID_SOCKET) {
			DebugString("Unable to create raw UDP socket with SG_UNCONSTRAINED_GROUP - error code \n", WSAGetLastError());
			use_group = false;
		}
	}
#endif

	if (!use_group) {
		RawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	}

	if (RawSocket == INVALID_SOCKET) {
		DebugString("Unable to create raw UDP socket - error code \n", WSAGetLastError());
		WSACleanup();

		if (WSAGetLastError() == WSAEACCES) {
			failure_code = BANDTEST_NO_RAW_SOCKET_PERMISSION;
		} else {
			failure_code = BANDTEST_NO_RAW_SOCKET_CREATE;
		}
		return(false);
	}

	/*
	** Get the group number.
	*/
#ifdef _WIN32
	unsigned int group = 0;
	int length = 4;

	if (use_group) {
		if (getsockopt (RawSocket, SOL_SOCKET, SO_GROUP_ID, (char*)&group, &length) == SOCKET_ERROR) {
			DebugString("Unable to get group for raw socket - error code \n", WSAGetLastError());
			use_group = false;
		}
	}
#endif


	/*
	** Create a socket for ICMP packets.
	*/
#ifdef _WIN32
	if (use_group) {
		ICMPRawSocket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, group, 0);
	} else
#endif
	{
		ICMPRawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	}

	if (ICMPRawSocket == INVALID_SOCKET) {
		DebugString("Unable to create raw ICMP socket - error code \n", WSAGetLastError());
		closesocket(RawSocket);
		WSACleanup();

		if (WSAGetLastError() == WSAEACCES) {
			failure_code = BANDTEST_NO_RAW_SOCKET_PERMISSION;
		} else {
			failure_code = BANDTEST_NO_RAW_SOCKET_CREATE;
		}
		return(false);
	}

#ifdef _WIN32
	/*
	** Set the priority for the sockets.
	*/
	unsigned int new_priority = 50;
	int result = setsockopt(RawSocket, SOL_SOCKET, SO_GROUP_PRIORITY, (char*)&new_priority, sizeof(new_priority));
	if (result != 0) {
		DebugString("Unable to set priority on UDP socket - error code %d\n", WSAGetLastError());
	}
	new_priority = 1;
	result = setsockopt(ICMPRawSocket, SOL_SOCKET, SO_GROUP_PRIORITY, (char*)&new_priority, sizeof(new_priority));
	if (result != 0) {
		DebugString("Unable to set priority on ICMP socket - error code %d\n", WSAGetLastError());
	}
#endif

	return(true);
}




/***********************************************************************************************
 * Close_Raw_Sockets -- Initialize winsock and create the raw sockets we need                  *
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
 *   10/3/2001 11:31AM ST : Created                                                            *
 *=============================================================================================*/
void Close_Raw_Sockets(void)
{
	if (RawSocket != INVALID_SOCKET) {
		closesocket(RawSocket);
	}

	if (RawSocket != INVALID_SOCKET) {
		closesocket(ICMPRawSocket);
	}

	WSACleanup();
}





/***********************************************************************************************
 * Get_IP_Checksum -- Create a checksum value for an IP packets                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Buffer                                                                            *
 *           Buffer size                                                                       *
 *                                                                                             *
 * OUTPUT:   Checksum                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:44AM ST : Created                                                            *
 *=============================================================================================*/
unsigned short Get_IP_Checksum(unsigned short *buffer, int size)
{
	unsigned int checksum = 0;
	int new_size = size;
	unsigned short *bufptr = buffer;

	while(new_size >1) {
		checksum += *bufptr++;
		new_size -= sizeof(unsigned short);
	}

	if (new_size) {
		checksum += *(unsigned char*) bufptr;
	}

	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);
	checksum = ~checksum;
	return ((unsigned short) checksum);
}







bool Set_Registry_Int(const char *name, int value)
{
#ifdef _WIN32
	int result = RegSetValueExA(RegistryKey, name, 0, REG_DWORD, (unsigned char*)&value, sizeof(value));
	return((result == ERROR_SUCCESS) ? true : false);
#else
	if (name == nullptr) {
		return false;
	}
	std::lock_guard<std::mutex> lock(g_registry_mutex);
	Load_Linux_Registry();
	g_registry_store[name] = value;
	Save_Linux_Registry();
	return true;
#endif
}

int Get_Registry_Int(const char *name, int def_value)
{
#ifdef _WIN32
	DWORD type;
	DWORD data;
	DWORD data_size = sizeof(data);

	if (RegQueryValueExA(RegistryKey, name, NULL, &type, (unsigned char*)&data, &data_size) == ERROR_SUCCESS) {
		return(data);
	}
	return(def_value);
#else
	if (name == nullptr) {
		return def_value;
	}
	std::lock_guard<std::mutex> lock(g_registry_mutex);
	Load_Linux_Registry();
	auto it = g_registry_store.find(name);
	if (it != g_registry_store.end()) {
		return it->second;
	}
	return def_value;
#endif
}


bool Open_Registry(void)
{
#ifdef _WIN32
	HKEY key;
	DWORD disposition;
	LSTATUS result = RegCreateKeyExA(HKEY_CURRENT_USER, RegistryPath, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, &disposition);
	if (result == ERROR_SUCCESS) {
		RegistryKey = key;
		return(true);
	}
	return(false);
#else
	std::lock_guard<std::mutex> lock(g_registry_mutex);
	Load_Linux_Registry();
	return true;
#endif
}

void Close_Registry(void)
{
#ifdef _WIN32
	RegCloseKey(RegistryKey);
#else
	std::lock_guard<std::mutex> lock(g_registry_mutex);
	Save_Linux_Registry();
#endif
}








#ifdef _DEBUG

/***********************************************************************************************
 * DebugString -- Debug output                                                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Printf format                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/3/2001 11:36AM ST : Created                                                            *
 *=============================================================================================*/
#if defined(_WIN32)
void DebugString (char const * string, ...)
{
	static char buffer[1024];
	static char filebuf[1024];
	static char path_to_exe[512];
	static char drive[_MAX_DRIVE];
	static char dir[_MAX_DIR];

	va_list	va;

	strcpy(buffer, "BandTest: ");
	va_start(va, string);
	vsprintf(&buffer[10], string, va);
	va_end(va);

	DWORD	actual;
	if (DebugFile == INVALID_HANDLE_VALUE) {
		GetModuleFileNameA (GetModuleHandleA(NULL), &path_to_exe[0], 512);
		_splitpath(path_to_exe, drive, dir, NULL, NULL);
		_makepath(DebugFileName, drive, dir, "bandtest", "txt");
		DebugFile = CreateFileA(DebugFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	} else {
		DebugFile = CreateFileA(DebugFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	OutputDebugStringA (buffer);

	if (DebugFile != INVALID_HANDLE_VALUE) {
		SetFilePointer (DebugFile, 0, NULL, FILE_END);

		char *srcbuf = buffer;
		char *destbuf = filebuf;
		char c;

		while (*srcbuf != 0) {
			c = *srcbuf++;
			if (c == '\n') {
				*destbuf++ = '\r';
			}
			*destbuf++ = c;
		}
		*destbuf = 0;

		WriteFile(DebugFile, filebuf, strlen(filebuf), &actual, NULL);
		CloseHandle (DebugFile);
	}
}
#else
void DebugString (char const * string, ...)
{
	va_list	va;
	va_start(va, string);
	std::fprintf(stderr, "BandTest: ");
	std::vfprintf(stderr, string, va);
	va_end(va);
}
#endif



/***********************************************************************************************
 * Addr_As_String -- Get a human readable internet address                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address ptr                                                                       *
 *                                                                                             *
 * OUTPUT:   String representation                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/31/2001 3:48PM ST : Created                                                             *
 *=============================================================================================*/
char * Addr_As_String2(struct sockaddr_in *addr)
{
	static char _string[128];
	const auto *bytes = reinterpret_cast<const unsigned char*>(&addr->sin_addr);
	std::snprintf(_string, sizeof(_string), "%u.%u.%u.%u ; %u",
		(unsigned)bytes[0],
		(unsigned)bytes[1],
		(unsigned)bytes[2],
		(unsigned)bytes[3],
		(unsigned)ntohs(addr->sin_port));
	return(_string);
}


/***********************************************************************************************
 * Addr_As_String -- Get a human readable internet address                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address ptr                                                                       *
 *                                                                                             *
 * OUTPUT:   String representation                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/31/2001 3:48PM ST : Created                                                             *
 *=============================================================================================*/
char * Addr_As_String(unsigned char *addr)
{
	static char _string[128];
	std::snprintf(_string, sizeof(_string), "%u.%u.%u.%u",
		(unsigned)addr[0], (unsigned)addr[1], (unsigned)addr[2], (unsigned)addr[3]);
	return(_string);
}






#endif //_DEBUG


#if (0)

	/*
	** Try to find a router with a different address class than ours. Start at TTL = 1 and increase until we find what we
	** are looking for.
	*/
	ttl = 1;
	hops_to_server = 0;
	found_whole_path = false;
	do {
		performance_timer = timeGetTime();

		router_ttl = 0;
		for (ttl ; ttl < 100 ; ttl++) {

			memcpy(&address, &host_address, sizeof(address));

			/*
			** Set the TTL.
			*/
			int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));
			if (result == SOCKET_ERROR) {
				DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", ttl, WSAGetLastError());
				failure_code = BANDTEST_NO_TTL_SET;
				Close_Raw_Sockets();
				return(0);
			}

			unsigned int start_time = timeGetTime();

			/*
			** Send a ping with the previously set TTL.
			*/
			DebugString("Sending ping with TTL = %d\n", ttl);
			Send_Ping(temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &address, packet_sequencer);

			/*
			** Wait for a ping response.
			*/
			seq_id = -1;
			while (seq_id == -1) {
				Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
				if (timeGetTime() - start_time > 5000) {
					DebugString("Failed to get response to ping with TTL = %d\n", ttl);
					break;
					//failure_code = BANDTEST_NO_PING_RESPONSE;
					//Close_Raw_Sockets();
					//return(0);
				}
			};

			if (seq_id != -1) {
				unsigned int long_router_addr = ntohl(address.sin_addr.s_addr);
				if (!found_whole_path) {
					path_to_server[hops_to_server++] = long_router_addr;
				}

				/*
				** See if this router is the target. If so, we are at the end of the path.
				*/
				if (long_router_addr  == server_ip || seq_id == packet_sequencer) {

					/*
					** We better have found an external router by now...
					*/
					assert(router_ttl != 0);
					found_whole_path = true;
					break;
				}

				/*
				** See if the router is on the same network as me.
				*/
				if (router_ttl == 0 && (long_router_addr & 0xffff0000) != (my_ip & 0xffff0000)) {
					memcpy(&router_addr, &address, sizeof(router_addr));
					router_ttl = ttl;
					DebugString("Found external router at %s - ttl = %d\n", Addr_As_String((unsigned char*)&address.sin_addr.s_addr), router_ttl);
					if (found_whole_path || settings->TTLScatter == 0) {
						break;
					}
				}
			}
			packet_sequencer++;
		}

		//if (router_ttl == 0) {
		//	DebugString("Failed to find external router\n");
		//	failure_code = BANDTEST_NO_EXTERNAL_ROUTER;
		//	Close_Raw_Sockets();
		//	return(0);
		//}

		//assert(router_ttl != 0);

		if (router_ttl == 0) {
			continue;
		}

		DebugString("Took %d ms to find external router\n", timeGetTime() - performance_timer);
		performance_timer = timeGetTime();

		average_ping = 0;

		/*
		** Set the TTL back to max.
		*/
		int new_ttl = 255;
		int result = setsockopt(ICMPRawSocket, IPPROTO_IP, IP_TTL, (char*)&new_ttl, sizeof(new_ttl));
		if (result == SOCKET_ERROR) {
			DebugString("setsockopt failed to set IP_TTL = %d - error code %d\n", new_ttl, WSAGetLastError());
			failure_code = BANDTEST_NO_TTL_SET;
			Close_Raw_Sockets();
			return(0);
		}

		/*
		** Ping the router directly to get a reference round trip time.
		** Try 3 pings and take an average.
		*/
		packet_sequencer = 0;
		memset(ping_times, 0xff, sizeof(ping_times));
		num_pings = 0;

		for (i=0 ; i<3 ; i++) {

			unsigned int start_time = timeGetTime();

			Send_Ping((char*)temp_buffer, 0, ICMPRawSocket, (struct sockaddr *) &router_addr, packet_sequencer);

			seq_id = -1;

			while (seq_id != packet_sequencer) {
				Get_Ping_Response(ICMPRawSocket, seq_id, (struct sockaddr *) &address, ntohl(host_address.sin_addr.s_addr), ping_dest_address);
				if (timeGetTime() - start_time > 2000) {
					DebugString("Failed to get response to reference ping %d\n", i);
					failure_code = BANDTEST_NO_PING_RESPONSE;
					break;
					//Close_Raw_Sockets();
					//return(0);
				}
			};

			//assert(seq_id == packet_sequencer);
			//assert(seq_id < 3);
			if (seq_id == packet_sequencer) {
				unsigned int router_ping_time = timeGetTime() - start_time;
				DebugString("Ping time %d to external router %s is %d ms\n", num_pings, Addr_As_String2(&router_addr), router_ping_time);
				ping_times[num_pings++] = router_ping_time;
			}
			packet_sequencer++;
		}

		//assert(num_pings == 3);
		average_ping = 0;
		if (num_pings > 0) {
			for (int p=0 ; p<num_pings ; p++) {
				average_ping += ping_times[p];
			}
			average_ping = average_ping / num_pings;
			DebugString("Average ping time to external router is %d ms\n", average_ping);
			DebugString("Took %d ms to find average ping\n", timeGetTime() - performance_timer);
			performance_timer = timeGetTime();
		} else {
			/*
			** If we didn't get all three pings then go round again.
			*/
			router_ttl = 0;
		}
	} while (router_ttl == 0 && ttl < 8);

#endif //(0)
