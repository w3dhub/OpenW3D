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
 *                     $Archive:: /Commando/Code/Commando/bandwidthcheck.cpp                  $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/06/02 11:48a                                              $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
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
#include	<stddef.h>

#include "bandwidthcheck.h"
#include "AutoStart.h"
#include "registry.h"
#include "_globals.h"
#include "useroptions.h"
#include "translatedb.h"
#include "string_ids.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include "gamespyadmin.h"

#include <WWOnline/WOLSession.h>
#include <cstdio>
#include <winsock2.h>


/*
** Class statics.
*/
BandwidthCheckerClass::BandwidthCheckerThreadClass BandwidthCheckerClass::Thread;
HANDLE BandwidthCheckerClass::EventNotify = NULL;
unsigned int BandwidthCheckerClass::UpstreamBandwidth = 0;
unsigned int BandwidthCheckerClass::ReportedUpstreamBandwidth = 0;
const unichar_t *BandwidthCheckerClass::UpstreamBandwidthString = NULL;
unsigned int BandwidthCheckerClass::DownstreamBandwidth = 0;
unsigned int BandwidthCheckerClass::ReportedDownstreamBandwidth = 0;
const unichar_t *BandwidthCheckerClass::DownstreamBandwidthString = NULL;
int BandwidthCheckerClass::FailureCode = BANDTEST_OK;
bool BandwidthCheckerClass::GotBandwidth = false;
const char *BandwidthCheckerClass::DefaultServerName = "www.westwood.com";

/*
** Possible error codes from the bandwidth test.
*/
const char *BandwidthCheckerClass::ErrorList[13] = {
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
#ifdef FREEDEDICATEDSERVER
#define DEFAULT_BAND 9
#else  //FREEDEDICATEDSERVER
#define DEFAULT_BAND 3
#endif //FREEDEDICATEDSERVER

/*
** Lower and upper limits for each level of bandwidth.
*/
unsigned int BandwidthCheckerClass::Bandwidths[NUM_BANDS * 2] = {
	 12000,	14400,
	 28000,	28800,
	 29999,	33600,
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

/*
** Human readable names for each bandwidth level.
*/
const unichar_t *BandwidthCheckerClass::BandwidthNames [NUM_BANDS+1] = {
	U_CHAR("14400"),
	U_CHAR("28800"),
	U_CHAR("33600"),
	U_CHAR("57600"),
	U_CHAR("67200"),
	U_CHAR("115200"),
	U_CHAR("128k"),
	U_CHAR("256k"),
	U_CHAR("512k"),
	U_CHAR("1M"),
	U_CHAR("2M"),
	U_CHAR("4M"),
	U_CHAR("> 4M")
};




/***********************************************************************************************
 * BandwidthCheckerClass::Detect -- Create the bandwidth detect wait object                    *
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
 *   11/21/2001 2:53PM ST : Created                                                            *
 *=============================================================================================*/
RefPtr<WaitCondition> BandwidthCheckerClass::Detect(void)
{
	return(BandwidthDetectWait::Create());
}



/***********************************************************************************************
 * Start the bandwidth detection thread                                                        *
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
 *   11/21/2001 2:53PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthCheckerClass::Check_Now(HANDLE event)
{
	EventNotify = event;

	/*
	** If the thread didn't finish for some reason then we need to take action.
	** This will stall the dialogs but at least it won't crash.
	*/
	if (Thread.Is_Running()) {
		unsigned int timeout = 10 * 1000;
		unsigned int time = TIMEGETTIME();
		while (Thread.Is_Running() && (TIMEGETTIME() - time) < timeout) {
			Sleep(1);
		}
	}
	if (Thread.Is_Running()) {
		Thread.Stop(2000);
	}
	WWASSERT(!Thread.Is_Running());
	Thread.Execute();
}



/***********************************************************************************************
 * BandwidthCheckerClass::Get_Ping_Server_Name -- Get the name of a server to ping             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to server name                                                                *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/27/2001 1:17PM ST : Created                                                            *
 *=============================================================================================*/
const char *BandwidthCheckerClass::Get_Ping_Server_Name(void)
{
	static char server_name_copy[128];
	DynamicVectorClass<StringClass> list;
	const char *server_name = DefaultServerName;

	/*
	** See if there are ping servers in the registry from a previous run.
	*/
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_SERVER_LIST);
	WWASSERT(list.Count() == 0);
	reg.Get_Value_List(list);
	if (list.Count() > 0) {

		/*
		** Get average and lowest ping server ping time.
		*/
		int num_times = 0;
		unsigned int total = 0;
		int lowest = 0xffff;
		int lowest_index = -1;
		int i;
		for (i=0 ; i<list.Count() ; i++) {
			int time = reg.Get_Int(list[i].Peek_Buffer(), 0);
			if (time > 0 && time < 0xffff) {
				total += (unsigned int) time;
				num_times++;

				if (time < lowest) {
					lowest = time;
					lowest_index = i;
				}
			}
		}

		if (num_times) {
			int average_time = (int) (total / (unsigned) num_times);

			/*
			** Pick one a little closer.
			*/
			average_time = average_time / 2;

			/*
			** Find the server closest to the average time.
			*/
			int closest_index = -1;
			int closest_diff = 0x7fffffff;

			for (i=0 ; i<list.Count() ; i++) {
				int time = reg.Get_Int(list[i].Peek_Buffer(), 0);
				if (time > 0 && time < 0xffff) {
					int diff = abs(time - average_time);
					if (diff < closest_diff) {

						/*
						** Ignore the nearest server.
						*/
						if (i != lowest_index) {
							closest_diff = diff;
							closest_index = i;
						}
					}
				}
			}

			if (closest_index != -1) {
				WWASSERT(closest_index >= 0);
				WWASSERT(closest_index < list.Count());
				strncpy(server_name_copy, list[closest_index].Peek_Buffer(), sizeof(server_name_copy));
				server_name_copy[sizeof(server_name_copy) - 1] = 0;
				server_name = (const char*) server_name_copy;
			}
		}
	}
	return(server_name);
}







/***********************************************************************************************
 * BandwidthCheckerClass::Check -- Check bandwidth. This is called from bandwidth thread.      *
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
 *   11/21/2001 2:54PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthCheckerClass::Check(void)
{
	struct hostent *host;
	struct sockaddr_in address;
	int failure_code;

	/*
	** If we are auto starting then just use the previous settings from the registry.
	*/
	if (AutoRestart.Is_Active()) {
		RegistryClass reg(APPLICATION_SUB_KEY_NAME_BANDTEST);
		int up = reg.Get_Int("Up", 0);
		int down = reg.Get_Int("Down", up);
		UpstreamBandwidth = up;
		DownstreamBandwidth = down;
		if (up) {
			GotBandwidth = true;
			SetEvent(EventNotify);
			return;
		}
	}

	ConsoleBox.Print("Detecting bandwidth...\n");

	const char *host_name = NULL;
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		// US West Ping server
		host_name = "159.153.192.10";
	} else {
		host_name = Get_Ping_Server_Name();
	}
	WWDEBUG_SAY(("BandwidthCheckerClass::Check -- Trying server %s\n", host_name));

	host = gethostbyname(host_name);
	if (host == NULL) {
		host_name = DefaultServerName;
		WWDEBUG_SAY(("BandwidthCheckerClass::Check -- Trying server %s\n", host_name));
		host = gethostbyname(host_name);
	}

	if (host == NULL) {
		/*
		** No DNS or no connection at all. Either way we are in trouble.
		*/
		WWDEBUG_SAY(("BandwidthCheckerClass - Unable to resolve host name\n"));
		ConsoleBox.Print("Unable to resolve host name for bandwidth check\n");
#ifdef FREEDEDICATEDSERVER
		UpstreamBandwidth = 1000000;
		DownstreamBandwidth = 1000000;
		ReportedUpstreamBandwidth = 1000000;
		ReportedDownstreamBandwidth = 1000000;
#else  //FREEDEDICATEDSERVER
		UpstreamBandwidth = 55000;
		DownstreamBandwidth = 55000;
		ReportedUpstreamBandwidth = 57600;
		ReportedDownstreamBandwidth = 57600;
#endif //FREEDEDICATEDSERVER
		UpstreamBandwidthString = BandwidthNames[DEFAULT_BAND];
		DownstreamBandwidthString = BandwidthNames[DEFAULT_BAND];
		FailureCode = BANDTEST_NO_IP_DETECT;
		GotBandwidth = false;
	} else {

		memcpy(&(address.sin_addr), host->h_addr, host->h_length);

		/*
		** Init the settings for the detection.
		*/
		BandtestSettingsStruct settings = {
			0,		//AlwaysICMP
			0,		//TTLScatter
			50,	//FastPingPackets
			12,	//SlowPingPackets
			25,	//Fast ping threshold
			0		//PingProfile
		};

		/*
		** Call the .dll function to do the actual detection.
		*/
		UpstreamBandwidth = Detect_Bandwidth(ntohl(address.sin_addr.s_addr), 0, 2, failure_code, DownstreamBandwidth, BANDTEST_API_VERSION, &settings, APPLICATION_SUB_KEY_NAME_BANDTEST);

		/*
		** If we failed due to a missing final ping then try again with fewer packets and no retries.
		*/
		if (UpstreamBandwidth == 0) {
			if (failure_code == BANDTEST_NO_FINAL_PING_TIME || 
					(cGameSpyAdmin::Is_Gamespy_Game() && 
					failure_code == BANDTEST_NO_EXTERNAL_ROUTER)) {
				BandtestSettingsStruct settings = {
					0,		//AlwaysICMP
					0,		//TTLScatter
					25,	//FastPingPackets
					8,		//SlowPingPackets
					25,	//Fast ping threshold
					0		//PingProfile
				};

				if (cGameSpyAdmin::Is_Gamespy_Game()) {
					// US East Ping server
					address.sin_addr.s_addr = inet_addr("159.153.224.10");
				}
				UpstreamBandwidth = Detect_Bandwidth(ntohl(address.sin_addr.s_addr), 0, 0, failure_code, DownstreamBandwidth, BANDTEST_API_VERSION, &settings);
			}
		}

		/*
		** If it's 0, we failed.
		*/
		if (UpstreamBandwidth == 0) {
			WWDEBUG_SAY(("Failed to get bandwidth - error code %s\n", ErrorList[failure_code]));
			/*
			** Default to 57600.
			*/
#ifdef FREEDEDICATEDSERVER
			ConsoleBox.Print("Failed to get bandwidth - error code %s, Setting 1Mbps bandwidth\n", ErrorList[failure_code]);
			UpstreamBandwidth = 1000000;
			DownstreamBandwidth = 1000000;
#else  //FREEDEDICATEDSERVER
			if (cGameSpyAdmin::Is_Gamespy_Game()) {
				UpstreamBandwidth = 128000;
				DownstreamBandwidth = 128000;
			} else {
				UpstreamBandwidth = 55000;
				DownstreamBandwidth = 55000;
			}
#endif //FREEDEDICATEDSERVER
			FailureCode = failure_code;
			GotBandwidth = true;	//false;
		} else {
			GotBandwidth = true;
		}

		/*
		** Fix up the upstream bandwidth into one of our connection type bands.
		*/
		if (UpstreamBandwidth > 0x7fffffff) {
			WWDEBUG_SAY(("Upstream bandwidth is huge :-)\n"));
			WWDEBUG_SAY(("Reported upstream connection bandwidth is > 4M bits per second\n"));
			UpstreamBandwidth = 4096000;
			ReportedUpstreamBandwidth = 4096000;
			UpstreamBandwidthString = BandwidthNames[NUM_BANDS];
		} else {
			if (UpstreamBandwidth > 100000) {
#ifdef WWDEBUG
				float floater = (float)UpstreamBandwidth / 1024;
#endif //WWDEBUG
				WWDEBUG_SAY(("Upstream bandwidth to external router is %.1f kilobits per second\n", floater));
			} else {
				WWDEBUG_SAY(("Upstream bandwidth to external router is %d bits per second\n", UpstreamBandwidth));
			}

			bool got_bw_str = false;
			for (int i=0 ; i<NUM_BANDS ; i++) {
				if (UpstreamBandwidth < Bandwidths[(i*2) + 1]) {
					//WWDEBUG_SAY(("\nReported upstream connection bandwidth is %s bits per second\n", BandwidthNames[i]));
					UpstreamBandwidthString = BandwidthNames[i];
					ReportedUpstreamBandwidth = Bandwidths[(i*2)+1];
					got_bw_str = true;
					break;
				}
			}
			if (!got_bw_str) {
				WWDEBUG_SAY(("\nReported upstream connection bandwidth is > 4M bits per second\n"));
				ReportedUpstreamBandwidth = 4096000;
				UpstreamBandwidthString = BandwidthNames[NUM_BANDS];
			}
			StringClass little_string;
			WideStringClass(UpstreamBandwidthString, true).Convert_To(little_string);

			ConsoleBox.Print("Upstream bandwidth of %s bps detected\n", little_string.Peek_Buffer());
		}

		/*
		** Fix up the downstream bandwidth into one of our connection type bands.
		*/
		if (DownstreamBandwidth > 0x7fffffff) {
			WWDEBUG_SAY(("Downstream bandwidth is huge :-)\n"));
			WWDEBUG_SAY(("Reported downstream connection bandwidth is > 4M bits per second\n"));
			DownstreamBandwidth = 4096000;
			ReportedDownstreamBandwidth = 4096000;
			DownstreamBandwidthString = BandwidthNames[NUM_BANDS];
		} else {
			if (DownstreamBandwidth > 100000) {
#ifdef WWDEBUG
				float floater = (float)DownstreamBandwidth / 1024;
#endif //WWDEBUG
				WWDEBUG_SAY(("Downstream bandwidth to external router is %.1f kilobits per second\n", floater));
			} else {
				WWDEBUG_SAY(("Downstream bandwidth to external router is %d bits per second\n", DownstreamBandwidth));
			}

			bool got_bw_str = false;
			for (int i=0 ; i<NUM_BANDS ; i++) {
				if (DownstreamBandwidth < Bandwidths[(i*2) + 1]) {
					//WWDEBUG_SAY(("\nReported downstream connection bandwidth is %s bits per second\n", BandwidthNames[i]));
					DownstreamBandwidthString = BandwidthNames[i];
					ReportedDownstreamBandwidth = Bandwidths[(i*2)+1];
					got_bw_str = true;
					break;
				}
			}
			if (!got_bw_str) {
				WWDEBUG_SAY(("\nReported downstream connection bandwidth is > 4M bits per second\n"));
				ReportedDownstreamBandwidth = 4096000;
				DownstreamBandwidthString = BandwidthNames[NUM_BANDS];
			}
		}

		/*
		** Testy.
		*/
#ifdef _DEBUG
		PackedBandwidthType packed = Get_Packed_Bandwidth();
		WWDEBUG_SAY(("Packed bandwidth as string = %s\n", Get_Bandwidth_As_String(packed)));
#endif //_DEBUG

	}
	SetEvent(EventNotify);
}



/***********************************************************************************************
 * BandwidthCheckerClass::Force_Upstream_Bandwidth -- Set actual upstream bandwidth              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:   Upstream bandwidth in buts per second                                             *
 *                                                                                             *
 * OUTPUT:    Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:54PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthCheckerClass::Force_Upstream_Bandwidth(unsigned int up)
{
	WWASSERT(up);
	WWASSERT(cGameSpyAdmin::Is_Gamespy_Game());

	UpstreamBandwidth = up;
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Upstream_Bandwidth -- Get actual upstream bandwidth              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Upstream bandwidth in buts per second                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:54PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int BandwidthCheckerClass::Get_Upstream_Bandwidth(void)
{
	return(UpstreamBandwidth);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Reported_Upstream_Bandwidth -- Get reported up bandwidth         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Reported upstream bandwidth in bits per second                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:55PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int BandwidthCheckerClass::Get_Reported_Upstream_Bandwidth(void)
{
	return(ReportedUpstreamBandwidth);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Upstream_Bandwidth_As_String -- Human readable upsream bw        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to bandwidth string                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:56PM ST : Created                                                            *
 *=============================================================================================*/
const unichar_t *BandwidthCheckerClass::Get_Upstream_Bandwidth_As_String(void)
{
	return(UpstreamBandwidthString);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Downstream_Bandwidth -- Get actual downstream bandwidth          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Actual down bw in bits per second                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:56PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int BandwidthCheckerClass::Get_Downstream_Bandwidth(void)
{
	return(DownstreamBandwidth);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Reported_Downstream_Bandwidth -- Get reported down bw            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Reported down bw in bits per second                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:57PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int BandwidthCheckerClass::Get_Reported_Downstream_Bandwidth(void)
{
	return(ReportedDownstreamBandwidth);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Downstream_Bandwidth_As_String -- Get down bw as a string        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to down bw string                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:57PM ST : Created                                                            *
 *=============================================================================================*/
const unichar_t *BandwidthCheckerClass::Get_Downstream_Bandwidth_As_String(void)
{
	return(DownstreamBandwidthString);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Bandwidth_As_String -- Get bandwidth description string          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to string                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:58PM ST : Created                                                            *
 *=============================================================================================*/
const unichar_t *BandwidthCheckerClass::Get_Bandwidth_As_String(void)
{

	if (cUserOptions::Get_Bandwidth_Type() == BANDWIDTH_AUTO) {
		static unichar_t _build_string[256];
		u_snprintf_u(_build_string, sizeof(_build_string), U_CHAR("%s,%s"), DownstreamBandwidthString, UpstreamBandwidthString);
		return(_build_string);
	} else {
		return((unichar_t*)cBandwidth::Get_Bandwidth_String_From_Type(
			(BANDWIDTH_TYPE_ENUM)cUserOptions::Get_Bandwidth_Type()));
	}
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Bandwidth_As_String -- Get bandwidth description string          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Packed up/down bandwidth                                                          *
 *                                                                                             *
 * OUTPUT:   Ptr to bandwidth description string                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:58PM ST : Created                                                            *
 *=============================================================================================*/
const unichar_t *BandwidthCheckerClass::Get_Bandwidth_As_String(PackedBandwidthType bandwidth)
{
	static unichar_t _build_string[256];

	assert(bandwidth.Bandwidth.Up < NUM_BANDS + 1);
	assert(bandwidth.Bandwidth.Down < NUM_BANDS + 1);

	u_snprintf_u(_build_string, sizeof(_build_string), U_CHAR("%s,%s"), BandwidthNames[bandwidth.Bandwidth.Down], BandwidthNames[bandwidth.Bandwidth.Up]);
	return(_build_string);
}


/***********************************************************************************************
 * BandwidthCheckerClass::Get_Packed_Bandwidth -- Get bandwidth packed into a byte             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Bandwidth as a byte                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 2:59PM ST : Created                                                            *
 *=============================================================================================*/
BandwidthCheckerClass::PackedBandwidthType BandwidthCheckerClass::Get_Packed_Bandwidth(void)
{
	PackedBandwidthType bandwidth = {0,0};
	assert(sizeof(bandwidth) == 1);

	unsigned int bwu = ReportedUpstreamBandwidth;
	unsigned int bwd = ReportedDownstreamBandwidth;
	bool automode = true;

	if (cUserOptions::Get_Bandwidth_Type() != BANDWIDTH_AUTO) {
		bwu = cBandwidth::Get_Bandwidth_Bps_From_Type((BANDWIDTH_TYPE_ENUM)cUserOptions::Get_Bandwidth_Type());
		bwd = bwu;
		automode = false;
	}

	/*
	** Just return 0s if we haven't detected bandwidth yet.
	*/
	if (!automode || UpstreamBandwidth != 0) {

		int i;
		for (i=0 ; i<NUM_BANDS+1 ; i++) {
			if (bwu <= Bandwidths[(i*2)+1]) {
				bandwidth.Bandwidth.Up = i;
				break;
			}
		}

		for (i=0 ; i<NUM_BANDS+1 ; i++) {
			if (bwd <= Bandwidths[(i*2)+1]) {
				bandwidth.Bandwidth.Down = i;
				break;
			}
		}
	}

	return(bandwidth);
}






/***********************************************************************************************
 * BandwidthCheckerClass::Get_Compact_Log -- Get basic log information to send to server       *
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
void BandwidthCheckerClass::Get_Compact_Log(StringClass &log_string)
{
	char temp[128];
	sprintf(temp, "%d\t%d\t%d\t", UpstreamBandwidth, DownstreamBandwidth, cUserOptions::Get_Bandwidth_Type());
	log_string = temp;
}






/***********************************************************************************************
 * BandwidthDetectWait::Create -- Create the wait object for bandwidth detection               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ref Ptr to bandwidth wait                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:00PM ST : Created                                                            *
 *=============================================================================================*/
RefPtr<BandwidthDetectWait> BandwidthDetectWait::Create(void)
{
	return (new BandwidthDetectWait());
}


/***********************************************************************************************
 * BandwidthDetectWait::BandwidthDetectWait -- BandwidthDetectWait constructor                 *
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
 *   11/21/2001 3:22PM ST : Created                                                            *
 *=============================================================================================*/
BandwidthDetectWait::BandwidthDetectWait(void) :
	SingleWait(TRANSLATE (IDS_MENU_TESTING_BANDWIDTH), 60000),
	mEvent(NULL),
	mPingsRemaining(0xffffffff)
{
	if (!cGameSpyAdmin::Is_Gamespy_Game()) {
		WOLSession = WWOnline::Session::GetInstance(false);
		assert(WOLSession.IsValid());
	}

}


/***********************************************************************************************
 * BandwidthDetectWait::~BandwidthDetectWait -- BandwidthDetectWait destructor                 *
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
 *   11/21/2001 3:22PM ST : Created                                                            *
 *=============================================================================================*/
BandwidthDetectWait::~BandwidthDetectWait()
{
	WWDEBUG_SAY(("BandwidthDetectWait: End - %S\n", mEndText));

	if (WOLSession.IsValid()) WOLSession->EnablePinging(true);

	if (mEvent) {
		CloseHandle(mEvent);
	}
}


/***********************************************************************************************
 * BandwidthDetectWait::WaitBeginning -- Called to init the wait                               *
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
 *   11/21/2001 3:22PM ST : Created                                                            *
 *=============================================================================================*/
void BandwidthDetectWait::WaitBeginning(void)
{
	WWDEBUG_SAY(("BandwidthDetectWait: Beginning\n"));

	mEvent = CreateEventA(NULL, true, false, NULL);

	if (mEvent == NULL) {
		WWDEBUG_SAY(("BandwidthDetectWait: Can't create event\n"));
		EndWait(Error, TRANSLATE (IDS_MENU_FAILED_TO_CREATE_BW_EVENT));
	} else {
		if (WOLSession.IsValid()) WOLSession->EnablePinging(false);
		mTimeout = 15000;
	}
}



/***********************************************************************************************
 * BandwidthDetectWait::GetResult -- See if there is a result for the wait condition           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Result code                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:23PM ST : Created                                                            *
 *=============================================================================================*/
WaitCondition::WaitResult BandwidthDetectWait::GetResult(void)
{
	if (mEndResult == Waiting) {
		/*
		** Wait for pings to finish first.
		*/

		unsigned int pingsWaiting = 0;
		if (!cGameSpyAdmin::Is_Gamespy_Game()) {
			pingsWaiting = WOLSession->GetPendingPingCount();
		}


		if (mPingsRemaining != pingsWaiting) {
			mPingsRemaining = pingsWaiting;
			if (mPingsRemaining == 0) {
				mTimeout = 60000;
				BandwidthCheckerClass::Check_Now(mEvent);
			}
		}

		if (mPingsRemaining == 0) {
			DWORD result = WaitForSingleObject(mEvent, 0);

			if (result == WAIT_OBJECT_0) {
				WWDEBUG_SAY(("BandwidthDetectWait: ConditionMet\n"));
				EndWait(ConditionMet, TRANSLATE (IDS_MENU_BW_DETECTION_COMPLETE));
			} else {
				if (result == WAIT_FAILED)	{
					WWDEBUG_SAY(("BandwidthDetectWait: WAIT_FAILED\n"));
					EndWait(Error, TRANSLATE (IDS_MENU_BW_DETECTION_FAILED));
				}
			}
		}
	}

	if (mEndResult != Waiting && WOLSession.IsValid()) {
		WOLSession->EnablePinging(true);
	}

	return(mEndResult);
}