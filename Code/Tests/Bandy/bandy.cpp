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
 *                 Project Name : Bandwidth Tester Tester                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/Bandy/bandy.cpp                        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/05/02 10:22p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <winsock2.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <bandtest\bandtest.h>

char *ErrorList[13] = {
	"BANDTEST_OK",
	"BANDTEST_NO_WINSOCK2",
	"BANDTEST_NO_RAW_SOCKET_PERMISSION",
	"BANDTEST_NO_RAW_SOCKET_CREATE",
	"BANDTEST_NO_UDP_SOCKET_BIND",
	"BANDTEST_NO_TTL_SET",
	"BANDTEST_NO_PING_RESPONSE",
	"BANDTEST_NO_FINAL_PING_TIME",
	"BANDTEST_NO_EXTERNAL_ROUTER",
	"BANDTEST_NO_IP_DETECT",
	"BANDTEST_UNKNOWN_ERROR",
	"BANDTEST_WRONG_API_VERSION",
	"BANDTEST_BAD_PARAM"
};

#define NUM_BANDS 12

unsigned int Bandwidths [NUM_BANDS * 2] = {
	 12000,	14400,
	 25000,	28800,
	 33600,	33600,
	 53000,	57600,
	 62000,	67200,
	105000,	115200,
	125000,	128000,
	250000,	256000,
	500000,	512000,
	999999,	1024000,
	1999999,	2048000,
	3999999,	4096000
};

char *BandwidthNames [NUM_BANDS] = {
	"14400",
	"28800",
	"33600",
	"57600",
	"67200",
	"115200",
	"128k",
	"256k",
	"512k",
	"1M",
	"2M",
	"4M"
};


BandtestSettingsStruct DefaultSettings = {
	0,		//AlwaysICMP
	0,		//TTLScatter
	50,	//FastPingPackets
	12,	//SlowPingPackets
	25,	//def =
	0,		//Ping profile
};







ULONG Enumerate_Nics(ULONG * addresses, ULONG max_nics)
{
	assert(addresses != NULL);
	assert(max_nics > 0);

	ULONG num_addresses = 0;

	//
	// Get the local hostname
	//
	char local_host_name[300];
#ifdef _DEBUG
	int gethostname_rc =
#endif //DEBUG
		gethostname(local_host_name, sizeof(local_host_name));
	assert(gethostname_rc != SOCKET_ERROR);
	//
	// Resolve hostname for local adapter addresses.
	// This does a DNS lookup (name resolution)
	//
	LPHOSTENT p_hostent = gethostbyname(local_host_name);
	if (p_hostent == NULL)
	{
	}

	while (num_addresses < max_nics && p_hostent->h_addr_list[num_addresses] != NULL)
	{
		IN_ADDR in_addr;
		memcpy(&in_addr, p_hostent->h_addr_list[num_addresses], sizeof(in_addr));
		addresses[num_addresses] = in_addr.s_addr;
		num_addresses++;
	}

	return num_addresses;
}




char * Addr_As_String(unsigned char *addr)
{
	static char _string[128];
	sprintf(_string, "%d.%d.%d.%d", 	(int)(addr[0]), (int)(addr[1]), (int)(addr[2]), (int)(addr[3]));
	return(_string);
}




int main(int argc, char **argv)
{

	unsigned int my_addresses[8];
	int use_addr = -1;
	int retries = 3;
	int failure_code = BANDTEST_OK;
	struct sockaddr_in address;
	BandtestSettingsStruct *settings = &DefaultSettings;

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(1,1), &wsa_data) != 0) {
		printf("Bandy: WSAStartup failed: error code %d\n", GetLastError());
		return(0);
	}

	int nics = Enumerate_Nics(&my_addresses[0], 8);
	struct hostent *host = gethostbyname("www.ea.com");

	/*
	** Usage.
	*/
	printf("Bandwidth tester tester\n");
	printf("Programmer: Steve Tall\n");
	printf("V1.0\n");
	printf("Usage: bandy.exe <options>\n");

	printf("Options:\n");
	printf("   -s<server>     - Use specified server (def = www.ea.com)\n");
	printf("   -i<ip index>   - Use specified local ip index (def = auto discovery)\n");
	printf("   -r<retrys>     - Retry attempts after failure (def = 3)\n");
	printf("   -l<packets>    - Number of packets to send on low ping times (def = 50)\n");
	printf("   -h<packets>    - Number of packets to send on high ping times (def = 12)\n");
	printf("   -p<time>       - Fast ping threshold in ms (def = 25 ms)\n");
	printf("   -a             - Send bulk data as ICMP packets (def = UDP)\n");
	printf("   -t             - Scatter TTL on bulk data (def = no TTL scatter)\n");
	printf("   -x             - Do ping profiling (def = no profiling)\n\n");

	printf("Available IPs : ");

	for (int i=0 ; i<nics ; i++) {
		printf("%d - %s\n                ", i, Addr_As_String((unsigned char*)&my_addresses[i]));
	}
	printf("\n");
	//WSACleanup();
	//return(0);



	for (int a=1 ; a<argc ; a++) {
		if (argv[a][0] != '-' || strlen(&argv[a][0]) < 2) {
			printf("Bad parameter %d\n", a);
			WSACleanup();
			return(0);
		}

		switch (toupper(argv[a][1])) {

			case 'S':
				host = gethostbyname(&argv[a][2]);
				if (host == NULL) {
					printf("Unable to resolve host name %s\n", &argv[a][2]);
					WSACleanup();
					return(0);
				}
				break;

			case 'I':
				use_addr = atoi(&argv[a][2]);
				if (use_addr >= nics) {
					printf("Bad IP index\n");
					WSACleanup();
					return(0);
				}
				break;

			case 'R':
				retries = atoi(&argv[a][2]);
				break;

			case 'L':
				settings->FastPingPackets = atoi(&argv[a][2]);
				break;

			case 'H':
				settings->SlowPingPackets = atoi(&argv[a][2]);
				break;

			case 'P':
				settings->FastPingThreshold = atoi(&argv[a][2]);
				break;

			case 'A':
				settings->AlwaysICMP = 1;
				break;

			case 'T':
				settings->TTLScatter = 1;
				break;

			case 'X':
				settings->PingProfile = 1;
				break;

			case 0:
			default:
				printf("Bad parameter %d\n", a);
				WSACleanup();
				return(0);
		}
	}


	if (host == NULL) {
		printf("Unable to resolve host name\n");
		WSACleanup();
		return(0);
	}

	memcpy(&(address.sin_addr), host->h_addr, host->h_length);
	printf("Detecting bandwidth - please wait\n");

	unsigned int downstream = 0;
	unsigned int bw = Detect_Bandwidth(ntohl(address.sin_addr.s_addr), (use_addr == -1) ? 0 : ntohl(my_addresses[use_addr]), retries, failure_code, downstream, BANDTEST_API_VERSION, settings);

	if (bw == 0) {
		printf("Failed to get bandwidth - error code %s\n", ErrorList[failure_code]);
	} else {
		if (bw == 0xffffffff) {
			printf("Upstream bandwidth is huge :-)\n");
		} else {
			if (bw > 100000) {
				float floater = (float)bw / 1024;
				printf("Upstream bandwidth to external router is %.1f kilobits per second\n", floater);
			} else {
				printf("Upstream bandwidth to external router is %d bits per second\n", bw);
			}

			bool got_bw_str = false;
			for (int i=0 ; i<NUM_BANDS ; i++) {
				if (bw < Bandwidths[i*2]) {
					printf("Reported upstream connection bandwidth is %s bits per second\n", BandwidthNames[i]);
					got_bw_str = true;
					break;
				}
			}
			if (!got_bw_str) {
				printf("Reported upstream connection bandwidth is > 4M bits per second\n");
			}

		}

		printf("\n");
		if (downstream == 0xffffffff) {
			printf("Downstream bandwidth is huge :-)\n");
		} else {
			if (downstream > 100000) {
				float floater = (float)downstream / 1024;
				printf("Downstream bandwidth to external router is %.1f kilobits per second\n", floater);
			} else {
				printf("Downstream bandwidth to external router is %d bits per second\n", downstream);
			}

			bool got_downstream_str = false;
			for (int i=0 ; i<NUM_BANDS ; i++) {
				if (downstream < Bandwidths[i*2]) {
					printf("Reported downstream connection bandwidth is %s bits per second\n", BandwidthNames[i]);
					got_downstream_str = true;
					break;
				}
			}
			if (!got_downstream_str) {
				printf("Reported downstream connection bandwidth is > 4M bits per second\n");
			}

		}

	}
	WSACleanup();
	return(0);
}