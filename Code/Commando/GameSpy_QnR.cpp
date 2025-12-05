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
 *                     $Archive:: /Commando/Code/Commando/GameSpy_QnR.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/08/02 5:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "specialbuilds.h"
#include "dlgcncteaminfo.h"
#include "listctrl.h"
#include "imagectrl.h"
#include "playertype.h"
#include "combat.h"
#include "teammanager.h"
#include "playermanager.h"
#include "player.h"
#include "soldier.h"
#include "gameinitmgr.h"
#include "gamemode.h"
#include "gamedata.h"
#include "input.h"
#include "healthbarctrl.h"
#include "basecontroller.h"
#include "building.h"
#include "damage.h"
#include "vehicle.h"
#include "assets.h"
#include "translatedb.h"
#include "wolgmode.h"
#include <WWOnline/WOLUser.h>
#include "gamespyadmin.h"
#include "string_ids.h"
#include "mousemgr.h"
#include "directinput.h"
#include "GameSpy_QnR.h"
#include "verchk.h"
#include "buildnum.h"
#include "ServerSettings.h"
#include "ConsoleMode.h"
#include "useroptions.h"
#include "gdcnc.h"
#include "rawfile.h"
#include "shellapi.h"
#include "netutil.h"
#include "GameSpyBanList.h"
#include <gamespy/pt/pt.h>
#include <gamespy/gcdkey/gcdkeys.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>

#ifdef _MSC_VER
#ifdef snprintf
#undef snprintf
#endif
#endif

CGameSpyQnR GameSpyQnR;

#if defined(MULTIPLAYERDEMO)
	const char *CGameSpyQnR::gamename = "ccrenegadedemo";
	const char *CGameSpyQnR::bname = "Demo";
	const int CGameSpyQnR::prodid = 10063;
	const int CGameSpyQnR::cdkey_id = 590; // https://aluigi.altervista.org/papers/gspids.txt
#elif defined(FREEDEDICATEDSERVER)
	const char *CGameSpyQnR::bname = "FDS";
	const char *CGameSpyQnR::gamename = "ccrenegade";
	const int CGameSpyQnR::prodid = 10064;
	const int CGameSpyQnR::cdkey_id = 577;
#elif defined(BETACLIENT)
	const char *CGameSpyQnR::bname = "Beta";
	const char *CGameSpyQnR::gamename = "ccrenegade";
	const int CGameSpyQnR::prodid = 10064;
	const int CGameSpyQnR::cdkey_id = 0;
#else
	const char *CGameSpyQnR::bname = "Retail";
	const char *CGameSpyQnR::gamename = "ccrenegade";
	const int CGameSpyQnR::prodid = 10064;
	const int CGameSpyQnR::cdkey_id = 577;
#endif

const char *CGameSpyQnR::default_heartbeat_list = "master.gamespy.com:27900, master.udpsoft.com:27900";



/*********
These c-style callback stubs are used by the SDK. The current game object
(passed in qr_init) is given in userdata, so we know what object to reference
**********/

void c_players_callback(char *outbuf, int maxlen, void *userdata)
{
	((CGameSpyQnR *)userdata)->players_callback(outbuf, maxlen);
}

void c_rules_callback(char *outbuf, int maxlen, void *userdata)
{
	((CGameSpyQnR *)userdata)->rules_callback(outbuf, maxlen);
}

void c_info_callback(char *outbuf, int maxlen, void *userdata)
{
	((CGameSpyQnR *)userdata)->info_callback(outbuf, maxlen);
}

void c_basic_callback(char *outbuf, int maxlen, void *userdata)
{
	((CGameSpyQnR *)userdata)->basic_callback(outbuf, maxlen);
}

/***********
A simple game object. Consists of some data and a main loop function.
***********/
CGameSpyQnR::CGameSpyQnR(void) : m_GSInit(false), m_GSEnabled(false)
{
	// Secret keys removed per Security review requirements. LFeenanEA - 27th January 2025
	
	//set the secret key, in a semi-obfuscated manner
	// tY1S8q = FULL , LsEwS3 = DEMO

#ifdef MULTIPLAYERDEMO
	secret_key[3] = 'R';  if (secret_key[3])
	secret_key[1] = 'E';  if (secret_key[1])
	secret_key[0] = 'M';  if (secret_key[0])
	secret_key[4] = 'O';  if (secret_key[4])
	secret_key[5] = 'V';  if (secret_key[5])
	secret_key[3] = 'E';  if (secret_key[3])
	secret_key[2] = 'D';  if (secret_key[2])
	secret_key[6] = '1'; if (secret_key[2])
	secret_key[5] = '2';  if (secret_key[5])
	secret_key[4] = '3';
#else 
	secret_key[5] = 'R';  if (secret_key[5])
	secret_key[4] = 'E';  if (secret_key[4])
	secret_key[6] = 'M'; if (secret_key[4])
	secret_key[1] = 'O';  if (secret_key[1])
	secret_key[5] = 'V';  if (secret_key[5])
	secret_key[2] = 'E';  if (secret_key[2])
	secret_key[3] = 'D';  if (secret_key[3])
	secret_key[6] = '1'; if (secret_key[3])
	secret_key[0] = '2';  if (secret_key[0])
	secret_key[3] = '3';
#endif

	m_Offline = FALSE;
}

CGameSpyQnR::~CGameSpyQnR()
{
	Shutdown();
}

void CGameSpyQnR::LaunchArcade(void) {
	const char *akey = "Software\\GameSpy\\GameSpy Arcade";
	BOOL launched = false;
	HKEY key = NULL;
	int result = 0;

	result = RegOpenKeyExA(HKEY_CURRENT_USER, akey, 0, KEY_READ, &key);
	if (result == ERROR_SUCCESS) {
		StringClass value(true);
		//
		//	Get the size of the entry
		//
		DWORD data_size = 0;
		DWORD type = 0;
		result = ::RegQueryValueExA ((HKEY)key, "InstDir", NULL, &type, NULL, &data_size);
		if (result == ERROR_SUCCESS && type == REG_SZ) {

			//
			//	Read the entry from the registry
			//
			::RegQueryValueExA ((HKEY)key, "InstDir", NULL, &type,
				(LPBYTE)value.Get_Buffer(data_size), &data_size);
		}
		if (!value.Is_Empty()) {
			const size_t length = value.Get_Length();
			if (value[static_cast<int>(length - 1)] == '\\') {
				value += "Aphex.exe";
			} else {
				value += "\\Aphex.exe";
			}

			RawFileClass file(value);
			if (file.Is_Available()) {
				StringClass params("+svc ");
				params += gamename;
				if (((uintptr_t)ShellExecuteA (NULL, "open", value, params, NULL, SW_SHOW)) > 32) {
					launched = true;
				}
			}
		}
	}

	if (!launched) {
		char url[1000] = "";

#ifdef MULTIPLAYERDEMO
		::strcpy(url, "http://www.gamespyarcade.com/features/launch.asp?svcname=ccrenegadedemo&distID=432");
#else
		::strcpy(url, "http://www.gamespyarcade.com/features/launch.asp?svcname=ccrenegade&distID=391");
#endif

		ShellExecuteA (NULL, "open", url, NULL, NULL, SW_SHOW);
	}
}
void CGameSpyQnR::Shutdown(void) {

#ifndef BETACLIENT
	if (m_GSInit) {
		/*
		We don't really need to set the mode to exiting here, since we immediately
		send the statechanged heartbeat and kill off the query sockets 
		gamemode = "exiting";*/
		ConsoleBox.Print("Shutting down GameSpy Q&R\n");
		qr_send_exiting(query_reporting_rec);
		qr_shutdown(query_reporting_rec);
		m_GSEnabled = m_GSInit = false;
	}
#endif
}

void CGameSpyQnR::Init(void) {

#ifndef BETACLIENT

		m_Offline = FALSE;

		if (m_GSEnabled && !m_GSInit && The_Game() && The_Game()->Get_Game_Type() == cGameData::GAME_TYPE_CNC) {
		
			ConsoleBox.Print("Initializing GameSpy Q&R\n");

			BOOL test = false;
		// Init the GameSpy QnR engine
		extern unsigned int g_ip_override;
		char ipstr[32];
		char *ip = ipstr;

		if (g_ip_override == INADDR_NONE) {
			if (cUserOptions::PreferredGameSpyNic.Get()) {
				strcpy(ip, cNetUtil::Address_To_String(cUserOptions::PreferredGameSpyNic.Get()));
			} else {
				ip = NULL;
			}
		} else {
			strcpy(ip, cNetUtil::Address_To_String(g_ip_override));
		}

		static bool warned_no_masters = false;

		if (!get_master_count()) {
			GameSpyQnR.Parse_HeartBeat_List(Get_Default_HeartBeat_List(), false);
		}
		if (!get_master_count() && !warned_no_masters) {
			ConsoleBox.Print("GameSpy master servers unavailable; continuing without heartbeats.\n");
			warned_no_masters = true;
			m_Offline = TRUE;
		} else if (get_master_count()) {
			m_Offline = FALSE;
		}
		test = qr_init(&query_reporting_rec, ip, cUserOptions::GameSpyQueryPort.Get(),
			gamename, secret_key, c_basic_callback, c_info_callback, c_rules_callback, 
			c_players_callback, this);
		WWASSERT(!test);
		if (!m_Offline) {
			gcd_init_qr(query_reporting_rec, cdkey_id);
		}

		StartTime = time(NULL);
		m_GSInit = true;
	}
#endif
}

/*******
 DoGameStuff
Simulate whatever else a game server does 
********/
void CGameSpyQnR::DoGameStuff(void)
{
//	Sleep(100);
}

#define BANLIST_RELOAD_TIME (1*60*1000)

/*******************
CGameSpyQnR::run
Simulates a main game loop
*****************/
void CGameSpyQnR::Think()
{
	static DWORD stime = (DWORD)(0 - BANLIST_RELOAD_TIME);
	static DWORD ttime = 0;

	if (TIMEGETTIME() - stime > BANLIST_RELOAD_TIME) {
		GameSpyBanList.LoadBans();
		stime = TIMEGETTIME();
	}
	// check twice a second
	if (TIMEGETTIME() - ttime > 500) {
		GameSpyBanList.Think();
		ttime = TIMEGETTIME();
	}
	
#ifndef BETACLIENT
//	DoGameStuff();
	if (m_GSInit && m_GSEnabled && GameInitMgrClass::Is_LAN_Initialized() &&
			!CombatManager::Is_Loading_Level()) {
		qr_process_queries(query_reporting_rec);
		gcd_think();
	}
#endif
}

/*************
basic_callback
sends a (sample) response to the basic query 
includes the following keys:
\gamename\
\gamever\
\location\
*************/
void CGameSpyQnR::basic_callback(char *outbuf, int maxlen)
{

	WWDEBUG_SAY(("-->GS_QnR -- Basic callback\n"));
	WWASSERT(!CombatManager::Is_Loading_Level());

	if (!maxlen || !outbuf) return;

	outbuf[0] = 0;

	StringClass b(true);
	b.Format("%d", BuildInfoClass::Get_Build_Number());
	sprintf(outbuf, "\\gamename\\%s\\gamever\\%s", gamename, b.Peek_Buffer());
//	sprintf(outbuf, "\\gamename\\%s\\gamever\\%s\\location\\%d", gamename, b.Peek_Buffer(), 1);

#ifdef WWDEBUG
	StringClass tstr(true);
	tstr.Format("GS_QnR -- Basic callback, sent: %s\n",outbuf);
	OutputDebugStringA(tstr.Peek_Buffer());
#endif
	WWDEBUG_SAY(("<--GS_QnR -- Basic callback\n"));

//	printf("Basic callback, sent: %s\n\n",outbuf);
}

/************
info_callback
Sends a (sample) response to the info query 
including the following keys:
\hostname\
\hostport\
\mapname\
\gametype\
\numplayers\
\maxplayers\
\gamemode\
************/
void CGameSpyQnR::info_callback(char *outbuf, int maxlen)
{

	WWDEBUG_SAY(("-->GS_QnR -- Info callback\n"));
	WWASSERT(!CombatManager::Is_Loading_Level());

	if (!maxlen || !outbuf) return;

	outbuf[0] = 0;

	while(1) {

		StringClass value(true);

		WideStringClass(The_Game()->Get_Game_Title()).Convert_To(value);
		if (value.Get_Length() > 25) {
			value[25] = 0;
		}
		if (!Append_InfoKey_Pair(outbuf, maxlen, "hostname", value)) break;

		value.Format("%d", The_Game()->Get_Port());
		if (!Append_InfoKey_Pair(outbuf, maxlen, "hostport", value)) break;

		value = The_Game()->Get_Map_Name();
		char *s = value.Peek_Buffer();
		char *t = strrchr(s, '.');
		if (t) value.Erase(t-s, s+strlen(s)-t);
		if (!Append_InfoKey_Pair(outbuf, maxlen, "mapname", value)) break;

		value = The_Game()->Get_Mod_Name();
		if (!value.Is_Empty()) {
			s = value.Peek_Buffer();
			t = strrchr(s, '.');
			if (t) value.Erase(t-s, s+strlen(s)-t);
			if (!Append_InfoKey_Pair(outbuf, maxlen, "gametype", value)) break;
		} else {
			if (!Append_InfoKey_Pair(outbuf, maxlen, "gametype", "C&C")) break;
		}

		int pcount = 0;
		for (SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ()
				; player_node != NULL; player_node = player_node->Next ()) {

			cPlayer *player = player_node->Data ();
			WWASSERT (player != NULL);

			if (player->Get_Is_Active().Is_False()) {
//			if (player->Get_Is_Active().Is_False() || !player->Is_Human()) {
				continue;
			}
			pcount++;
		}

		value.Format("%d", pcount);
		if (!Append_InfoKey_Pair(outbuf, maxlen, "numplayers", value)) break;
		value.Format("%d", The_Game()->Get_Max_Players());
		if (!Append_InfoKey_Pair(outbuf, maxlen, "maxplayers", value)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "gamemode", "openplaying")) break;
		break;
	}

#ifdef WWDEBUG
	StringClass tstr(true);
	tstr.Format("GS_QnR -- Info callback, sent: %s\n",outbuf);
	OutputDebugStringA(tstr.Peek_Buffer());
#endif
	WWDEBUG_SAY(("<--GS_QnR -- Info callback\n"));

}

/***************
rules_callback
Sends a response to the rules query. You may
need to add custom fields for your game in here. Some are provided
as an example 
The following rules are included:
\timelimit\
\fraglimit\
\teamplay\
\rankedserver\
****************/
void CGameSpyQnR::rules_callback(char *outbuf, int maxlen)
{
	static StringClass b;
	WWDEBUG_SAY(("-->GS_QnR -- Rules callback\n"));
	WWASSERT(!CombatManager::Is_Loading_Level());

	if (!maxlen || !outbuf) return;

	outbuf[0] = 0;

	while(1) {

		StringClass value(true);
		const char *zero = "0"; const char *one = "1";

//		WideStringClass timelimit;
//		The_Game()->Get_Time_Limit_Text(timelimit);
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "timeleft", timelimit)) break;

//		value.Format("%d", The_Game()->Get_Time_Limit_Minutes());
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "timelimit", value)) break;

//		if (b.Is_Empty()) {
//			char filename[MAX_PATH];
//			GetModuleFileNameA(NULL, filename, sizeof(filename));
//			VS_FIXEDFILEINFO version;
//			GetVersionInfo(filename, &version);
//			int ver = version.dwFileVersionMS;
//
//			b.Format("%s %s V%d.%3.3d(%s-%d)", "Win-X86", bname, (ver&0xffff0000)>>16, ver&0xffff, 
//				BuildInfoClass::Get_Builder_Initials(), BuildInfoClass::Get_Build_Number());
//		}
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "Version", b)) break;

//		if (!Append_InfoKey_Pair(outbuf, maxlen, "ServerMOTD", WideStringClass(The_Game()->Get_Motd()))) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "OwnerName", The_Game()->Get_Owner())) break;
		value.Format("%d", (int)cUserOptions::BandwidthBps.Get());
		if (!Append_InfoKey_Pair(outbuf, maxlen, "BW", value)) break;
//		int utime = time(NULL) - StartTime;
//		value.Format("%d:%d:%d:%d", utime/60/60/24, (utime/60/60)%24, (utime/60)%60, utime%60);
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "Uptime", value)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "CSVR", ConsoleBox.Is_Exclusive() ? one : zero)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "DED", The_Game()->IsDedicated.Get() ? one : zero)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "DG", The_Game()->DriverIsAlwaysGunner.Get() ? one : zero)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "password", The_Game()->IsPassworded.Get() ? one : zero)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "TC", The_Game()->IsTeamChangingAllowed.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "WeaponSpawning", The_Game()->SpawnWeapons.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "TeamRemix", The_Game()->RemixTeams.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "BuildingRepair", The_Game()->CanRepairBuildings.Get() ? one : zero)) break;
		if (!Append_InfoKey_Pair(outbuf, maxlen, "FF", The_Game()->IsFriendlyFirePermitted.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "BaseDestructionEndsGame", The_Game()->As_Cnc()->BaseDestructionEndsGame.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "BeaconPlacementEndsGame", The_Game()->As_Cnc()->BeaconPlacementEndsGame.Get() ? one : zero)) break;
//		value.Format("%d", (int)The_Game()->Get_Radar_Mode());
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "RadarMode", value)) break;
		value.Format("%d", The_Game()->As_Cnc()->Get_Starting_Credits());
		if (!Append_InfoKey_Pair(outbuf, maxlen, "SC", value)) break;
		
//		cTeam * p_team;
//		for (SLNode<cTeam> * objnode = cTeamManager::Get_Team_Object_List()->Head()
//				; objnode != NULL; objnode = objnode->Next()) {
//
//			p_team = objnode->Data();
//			WWASSERT(p_team != NULL);
//
//			if (p_team->Get_Id() == PLAYERTYPE_GDI) {
//				value.Format("%.0f", p_team->Get_Score());
//				if (!Append_InfoKey_Pair(outbuf, maxlen, "GDI_Score", value)) break;
//			} else if (p_team->Get_Id() == PLAYERTYPE_NOD) {
//				value.Format("%.0f", p_team->Get_Score());
//				if (!Append_InfoKey_Pair(outbuf, maxlen, "NOD_Score", value)) break;
//			}
//		}

		// These don't apply to LAN/GameSpy Games
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "ClanMatch", The_Game()->IsClanGame.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "Laddered", The_Game()->IsLaddered.Get() ? one : zero)) break;
//		if (!Append_InfoKey_Pair(outbuf, maxlen, "QuickMatch", The_Game()->Is_QuickMatch_Server() ? one : zero)) break;


		break;

}

#ifdef WWDEBUG
	StringClass tstr(true);
	tstr.Format("GS_QnR -- Rules callback, sent: %s\n",outbuf);
	OutputDebugStringA(tstr.Peek_Buffer());
#endif
	WWDEBUG_SAY(("<--GS_QnR -- Rules callback\n"));

}

namespace {
bool Resolve_Master_With_Timeout(const char *host, WORD port, sockaddr_in &out_addr, std::chrono::milliseconds timeout) {
	// Fast path for dotted-quad literals so we never touch DNS.
	unsigned long numeric = inet_addr(host);
	if (numeric != INADDR_NONE) {
		out_addr = {};
		out_addr.sin_family = AF_INET;
		out_addr.sin_addr.s_addr = numeric;
		out_addr.sin_port = htons(port);
		return true;
	}

	struct ResolveState {
		std::atomic<bool> done{false};
		bool resolved{false};
		sockaddr_in addr{};
	};
	auto state = std::make_shared<ResolveState>();

	std::thread resolver([state, host, port]() {
		addrinfo hints{};
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		char port_buf[8] = {};
		std::snprintf(port_buf, sizeof(port_buf), "%u", static_cast<unsigned>(port));

		addrinfo *info = nullptr;
		if (::getaddrinfo(host, port_buf, &hints, &info) == 0 && info != nullptr) {
			state->addr = *reinterpret_cast<sockaddr_in *>(info->ai_addr);
			state->resolved = true;
		}
		if (info) {
			::freeaddrinfo(info);
		}
		state->done.store(true, std::memory_order_release);
	});

	const auto deadline = std::chrono::steady_clock::now() + timeout;
	while (!state->done.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	if (!state->done.load(std::memory_order_acquire)) {
		resolver.detach(); // DNS is still in flight; carry on without blocking.
		return false;
	}

	resolver.join();
	if (state->resolved) {
		state->addr.sin_port = htons(port);
		out_addr = state->addr;
		return true;
	}
	return false;
}
} // anonymous namespace

BOOL CGameSpyQnR::Parse_HeartBeat_List(const char *list, bool log_on_error) {

	BOOL master_added = false;

	char *str = new char[strlen(list)+1];
	strcpy(str, list);

	char *t = str;

	clear_master_list();

	while (t) {
		WORD port = 27900;
		struct sockaddr_in taddr;
		memset(&taddr, 0, sizeof(taddr));
		taddr.sin_family = AF_INET;
		char *q = strchr(t, ',');
		if (q) *q++ = 0;
		char *s = strchr(t, ':');
		if (s) {
			// Parse off the port value
			*s++ = 0;
			char *p = s;
			if (atoi(p)) port = atoi(p);
		}
		// skip white space
		while (*t == ' ' || *t == '\t') t++;
		// process the address with a bounded DNS lookup to avoid hanging the server when offline
		if (*t && Resolve_Master_With_Timeout(t, port, taddr, std::chrono::milliseconds(1000))) {
			add_master(&taddr);
			master_added = true;
		} else if (*t && log_on_error) {
			ConsoleBox.Print("Skipping HeartBeat master '%s' (unresolved)\n", t);
		}
		t = q;
	}

	delete [] str;

	if (!master_added && log_on_error) {
		ConsoleBox.Print("Error processing HeartBeat List: <%s>\n", list);
	}

	return master_added;
}

BOOL CGameSpyQnR::Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const char *value) {

	WWASSERT(value);
	WWASSERT(outbuf);
	WWASSERT(key);

	const size_t clen = ::strlen(outbuf);
	const size_t required = clen + ::strlen(key) + ::strlen(value) + 3;

	if (required > static_cast<size_t>(maxlen)) {
		return FALSE;
	}

	char *s = new char[strlen(value)+1];
	strcpy(s, value);
	char *t = s;
	while (*t) {
		if (*t == '\\') *t = '/';
		t++;
	}
	t = strtrim(s);
	sprintf(&outbuf[clen], "\\%s\\%s", key, t);
	delete [] s;

	return true;
}

BOOL CGameSpyQnR::Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const WideStringClass &value) {
	static StringClass text;

	value.Convert_To(text);
	return Append_InfoKey_Pair(outbuf, maxlen, key, text.Peek_Buffer());
}

BOOL CGameSpyQnR::Append_InfoKey_Pair(char *outbuf, int maxlen, const char *key, const StringClass &value) {

	return Append_InfoKey_Pair(outbuf, maxlen, key, value.Peek_Buffer());
}

/***************
players_callback
sends the players and their information. 
Note that \ characters are not stripped out of player names. If
your game allows players or team names with the \ character, you will need
to strip or change it here. 
The following keys are included for each player:
\player_N\
\frags_N\
\deaths_N\
\skill_N\
\ping_N\
\team_N\
***************/
void CGameSpyQnR::players_callback(char *outbuf, int maxlen)
{

	// Send the minimum for now to reduce Bandwidth usage.
	outbuf[0] = 0;
	return;



	int pindex = 0;

	WWDEBUG_SAY(("-->GS_QnR -- Players callback\n"));
	WWASSERT(!CombatManager::Is_Loading_Level());

	if (!maxlen || !outbuf) return;

	outbuf[0] = 0;

	for (SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ();
			player_node != NULL; 
			player_node = player_node->Next ()) {

		cPlayer *player = player_node->Data ();
		WWASSERT (player != NULL);

		if (player->Get_Is_Active().Is_False()) {
//		if (player->Get_Is_Active().Is_False() || !player->Is_Human()) {
			continue;
		}

		StringClass keyval(true);
		StringClass value(true);

		// Set the Player's Name [Team]
		value = player->Get_Name();
		if (player->Get_Player_Type() == PLAYERTYPE_NOD) {
			value += " [NOD]";
		} else if (player->Get_Player_Type() == PLAYERTYPE_GDI) {
			value += " [GDI]";
		}

		keyval.Format("player_%d", pindex);
		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

		// Set the Player's Score
		keyval.Format("frags_%d", pindex);
		value.Format("%.0f", player->Get_Score());
		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

		// Set the Player's Credits
//		keyval.Format("credits_%d", pindex);
//		value.Format("%.0f", player->Get_Money());
//		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

		// Set the Player's Ping to Server
		keyval.Format("ping_%d", pindex);
		value.Format("%d", player->Get_Ping());
		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

		// Set the Player's Kills
//		keyval.Format("kills_%d", pindex);
//		value.Format("%d", player->Get_Kills());
//		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

		// Set the Player's Deaths
//		keyval.Format("deaths_%d", pindex);
//		value.Format("%d", player->Get_Deaths());
//		if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), value)) break;

/*		SmartGameObj *game_obj = player->Get_GameObj ();
		if (game_obj != NULL && game_obj->As_SoldierGameObj () != NULL) {

			// Set the Player's Class (ie: Technician,Sakura,Havok)
			keyval.Format("class_%d", pindex);
			if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), 
				WideStringClass(TRANSLATE(game_obj->Get_Translated_Name_ID())) )) break;
			
			SoldierGameObj *soldier = game_obj->As_SoldierGameObj();
			VehicleGameObj *vehicle = soldier->Get_Vehicle ();

			// If they're in a vehicle set the vehicle name
			keyval.Format("vehicle_%d", pindex);
			if (vehicle != NULL) {
				if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), 
					WideStringClass(TRANSLATE(vehicle->Get_Translated_Name_ID())) )) break;
			} else {
				if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), "None")) break;
			}
		} else {
			// If there's no gameobj then set vehicle/class to unknown
			keyval.Format("class_%d", pindex);
			if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), "Unknown")) break;
			keyval.Format("vehicle_%d", pindex);
			if (!Append_InfoKey_Pair(outbuf, maxlen, keyval.Peek_Buffer(), "Unknown")) break;
		} */

		pindex++;

	}

#ifdef WWDEBUG
	StringClass tstr(true);
	tstr.Format("GS_QnR -- Players callback, sent: %s\n",outbuf);
	OutputDebugStringA(tstr.Peek_Buffer());
#endif

	WWDEBUG_SAY(("<--GS_QnR -- Players callback\n"));
	return;
}
/************
We'll actually start up two completely seperate "game servers"
Each one initializes the Query & Reporting SDK and calls processs on it during
their main loop
************/
/*
int main(int argc, char* argv[])
{
	CGameSpyQnR mygame1("Test Game Server 1"), mygame2("Test Game Server 2");

	srand( TIMEGETTIME() );
	
	printf("Press any key to quit\n");
	while (!_kbhit())
	{
		mygame1.run();
		mygame2.run();
	}
	return 0;

}
*/
