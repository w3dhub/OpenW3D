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

//
// Filename:     useroptions.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//

#include "useroptions.h"

#include "_globals.h"
#include "wwdebug.h"
#include "player.h"
#include "cnetwork.h"
#include "registry.h"
#include "player.h"
#include "playertype.h"
#include "bandwidth.h"
#include "bandwidthcheck.h"
#include <stdio.h>
#include <string.h>
#include "trim.h"
#include "singletoninstancekeeper.h"
#include "slavemaster.h"
#include "debug.h"
#include "rawfile.h"
#include "serversettings.h"
#include "autostart.h"
#include "consolemode.h"
#include "GameSpy_QnR.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "useroptions.h"

extern char DefaultRegistryModifier[1024];

//
// Class statics
//
cRegistryString cUserOptions::GameDir(					APPLICATION_SUB_KEY_NAME_OPTIONS, "GameDir",           "");

cRegistryBool cUserOptions::ShowNamesOnSoldier(					APPLICATION_SUB_KEY_NAME_NETOPTIONS, "ShowNamesOnSoldier",           true);
cRegistryBool cUserOptions::SkipQuitConfirmDialog(				APPLICATION_SUB_KEY_NAME_OPTIONS,	"SkipQuitConfirmDialog",			false);
cRegistryBool cUserOptions::SkipIngameQuitConfirmDialog(		APPLICATION_SUB_KEY_NAME_OPTIONS,	"SkipIngameQuitConfirmDialog",	false);
cRegistryBool cUserOptions::CameraLockedToTurret(				APPLICATION_SUB_KEY_NAME_OPTIONS,	"CameraLockedToTurret",				false);
cRegistryBool cUserOptions::PermitDiagLogging(					APPLICATION_SUB_KEY_NAME_OPTIONS,	"PermitDiagLogging",					true);

cRegistryInt cUserOptions::Sku(										APPLICATION_SUB_KEY_NAME,				 "SKU",									RENEGADE_BASE_SKU);

cRegistryInt cUserOptions::BandwidthType(							APPLICATION_SUB_KEY_NAME_NETOPTIONS, "BandwidthType",						BANDWIDTH_AUTO);
cRegistryInt cUserOptions::BandwidthBps(							APPLICATION_SUB_KEY_NAME_NETOPTIONS, "BandwidthBps",						33600);

cRegistryInt		cUserOptions::GameSpyBandwidthType(			APPLICATION_SUB_KEY_NAME_GAMESPY,	 "GameSpyBandwidthType",			BANDWIDTH_AUTO);
cRegistryInt		cUserOptions::PreferredGameSpyNic(			APPLICATION_SUB_KEY_NAME_GAMESPY,    "PreferredGameSpyNic",				0);
cRegistryString	cUserOptions::GameSpyNickname(				APPLICATION_SUB_KEY_NAME_GAMESPY,	 "GameSpyNickname",					"");
cRegistryInt		cUserOptions::GameSpyQueryPort(			APPLICATION_SUB_KEY_NAME_GAMESPY,    "GameSpyQueryPort",				25300);
cRegistryInt		cUserOptions::GameSpyGamePort(			APPLICATION_SUB_KEY_NAME_GAMESPY,    "GameSpyGamePort",				4848);
cRegistryInt		cUserOptions::SplashCount(			APPLICATION_SUB_KEY_NAME_GAMESPY,    "SplashCount",				0);
cRegistryBool		cUserOptions::DoneClientBandwidthTest(			APPLICATION_SUB_KEY_NAME_GAMESPY,    "DoneClientBandwidthTest",				false);


cRegistryInt cUserOptions::PreferredLanNic(						APPLICATION_SUB_KEY_NAME_NETOPTIONS, "PreferredLanNic",					0);
cRegistryInt cUserOptions::NetUpdateRate(							APPLICATION_SUB_KEY_NAME_NETOPTIONS, "NetUpdateRate",						10);
cRegistryFloat cUserOptions::ClientHintFactor(					APPLICATION_SUB_KEY_NAME_NETOPTIONS, "ClientHintFactor",					10.0f);
cRegistryFloat cUserOptions::MaxFacingPenalty(					APPLICATION_SUB_KEY_NAME_NETOPTIONS, "MaxFacingPenalty",					0.3f);
cRegistryFloat cUserOptions::IrrelevancePenalty(				APPLICATION_SUB_KEY_NAME_NETOPTIONS, "IrrelevancePenalty",				0.2f);

cRegistryInt cUserOptions::ResultsLogNumber(						APPLICATION_SUB_KEY_NAME_NETOPTIONS, "ResultsLogNumber",					1);

//-----------------------------------------------------------------------------
cUserOptions::ParseResult cUserOptions::Parse_Command_Line(int argc, char *argv[])
{
	ParseResult retcode = ParseResult::SUCCESS;

	//
	// Loop through all the command line arguments.
	//

	for (int i=1 ; i<argc ; i++) {
		const char *cmd = argv[i];

		if (strcmp(cmd, "--gamedir") == 0) {
			const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			Set_GameDir(argval);
			continue;
		}

		// Look for ip override.
		if (strcmp(cmd, "--ip") == 0) {
			const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			extern ULONG g_ip_override;
			g_ip_override = ::inet_addr(argval);
			continue;
		}

		// See if multiple progrram instances are allowed.
		if (strcmp(cmd, "--multi") == 0) {
			SingletonInstanceKeeperClass::Allow_Multiple_Instances(true);
			continue;
		}

		if (strcmp(cmd, "--regmod") == 0) {
            const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			strcpy(DefaultRegistryModifier, argval);
			#ifdef WWDEBUG
			OutputDebugString("Registry modifier on command line\n");
			#endif //WWDEBUG
			Reread();
			continue;
		}

		if (strcmp(cmd, "--slave") == 0) {
			SlaveMaster.Set_Slave_Mode(true);
			DebugManager::Set_Is_Slave(true);

			// Save out process ID so our master server can find us.
			char tempmod[512];
			strcpy(tempmod, DefaultRegistryModifier);
			strcpy(DefaultRegistryModifier, "");
			RegistryClass reg(APPLICATION_SUB_KEY_NAME);
			if (reg.Is_Valid()) {
				reg.Set_Int("ProcessId", GetCurrentProcessId());
			}
			strcpy(DefaultRegistryModifier, tempmod);

			RegistryClass::Set_Read_Only(true);
			continue;
		}

		if (strcmp(cmd, "--startserver") == 0) {
            const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			Set_Server_INI_File(argval);
			continue;
		}

		if (strcmp(cmd, "--gamespyserver") == 0) {
            const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			char server_config_file[MAX_PATH];
			strcpy(server_config_file, argval);
			WWDEBUG_SAY(("Set to load gamespy server settings from config file %s\n", server_config_file));
			RawFileClass file(server_config_file);
			if (file.Is_Available()) {
				ServerSettingsClass::Set_Settings_File_Name(server_config_file);

				RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				if (registry.Is_Valid ()) {
					registry.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 1);
					registry.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE, 0);
				}
				cGameSpyAdmin::Set_Is_Server_Gamespy_Listed(true);
				GameSpyQnR.Enable_Reporting(true);
			}
			continue;
		}

		if (strcmp(cmd, "--nodx") == 0) {
			ConsoleBox.Set_Exclusive(true);
			continue;
		}

#ifndef BETACLIENT
		if (strcmp(cmd, "--gamespy-connect") == 0) {
			const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}
			USHORT port = 4848;
			DWORD addr = 0;

            char *saddr_mem = nullptr;
            const char *saddr = argval;
            const char *tport = strchr(argval, ':');
			if (tport) {
                char *end_port = nullptr;
                long arg_port = strtol(tport + 1, &end_port, 10);
                if (end_port != nullptr && *end_port != '\0') {
                    return FAILURE;
                }
                if (arg_port != 0 || arg_port > 0) {
                    port = arg_port;
                }
                saddr = saddr_mem = new char[tport - argval + 1];
                memcpy(saddr_mem, argval, tport - argval);
                saddr_mem[tport - argval] = '\0';
			}

			addr = ::inet_addr(argval);

            delete[] saddr_mem;

			cGameSpyAdmin::Set_Game_Host_Ip(addr);
			cGameSpyAdmin::Set_Game_Host_Port(port);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		if (strcmp(cmd, "--gamespy-netplayername") == 0) {
			const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}

			cUserOptions::GameSpyNickname.Set(argval);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		if (strcmp(cmd, "--gamespy-password") == 0) {
			const char *argval = argv[i + 1];
			i++;
			if (i >= argc) {
				retcode = FAILURE;
				break;
			}

			WideStringClass wide_password;
			wide_password.Convert_From(argval);
			cGameSpyAdmin::Set_Password_Attempt(wide_password);
		}
#endif // !BETACLIENT

		if (strcmp(cmd, "--help") == 0) {
			retcode = ParseResult::PRINT_HELP;
			break;
		}

		retcode = ParseResult::FAILURE;
		break;
	}

	// Return true if command line options scanned OK.
	return retcode;
}

void cUserOptions::Print_Command_Line_Help(bool error)
{
	char path[MAX_PATH];
	char filename[MAX_PATH];
	char filesuffix[MAX_PATH];
	FILE *file = error ? stderr : stdout;

	GetModuleFileNameA(NULL, path, sizeof(path));
	_splitpath(path, NULL, NULL, filename, filesuffix);
	_makepath(path, NULL, NULL, filename, filesuffix);

	fprintf(file, "usage: %s [--ip IP] [--multi] [--regmod MOD] [--slave] [--startserver]\n", path);
	fprintf(file, "    [--gamedir PATH]\n");
	fprintf(file, "    [--gamespyserver ADDRESS] [--nodx]\n");
#ifndef BETACLIENT
	fprintf(file, "    [--gamespy-connect IP[:PORT]]\n");
	fprintf(file, "    [--gamespy-netplayername NAME]\n");
	fprintf(file, "    [--gamespy-password PASSWORD]\n");
#endif
	fprintf(file, "    [--help]\n");
}



//-----------------------------------------------------------------------------
void cUserOptions::Set_GameDir(const char *gamedir)
{
	GameDir.Set(gamedir);
}

//-----------------------------------------------------------------------------
void cUserOptions::Set_Server_INI_File(const char *ini_file)
{
	char server_config_file[MAX_PATH];
	strcpy(server_config_file, ini_file);
	WWDEBUG_SAY(("Set to load server settings from config file %s\n", server_config_file));
	RawFileClass file(server_config_file);
	if (file.Is_Available()) {
		ServerSettingsClass::Set_Settings_File_Name(server_config_file);

		RegistryClass registry (APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
		if (registry.Is_Valid ()) {
			registry.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 1);
			registry.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE, 1);
		}
	}
}

//-----------------------------------------------------------------------------
void cUserOptions::Set_Bandwidth_Type(BANDWIDTH_TYPE_ENUM bandwidth_type)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		GameSpyBandwidthType.Set(bandwidth_type);
	} else {
		BandwidthType.Set(bandwidth_type);
	}

	if (bandwidth_type != BANDWIDTH_CUSTOM) {
		if (bandwidth_type == BANDWIDTH_AUTO && BandwidthCheckerClass::Got_Bandwidth()) {
			ULONG bps = BandwidthCheckerClass::Get_Upstream_Bandwidth();
			WWASSERT(bps > 0);
			BandwidthBps.Set(bps);
		} else {
			ULONG bps = cBandwidth::Get_Bandwidth_Bps_From_Type(bandwidth_type);
			WWASSERT(bps > 0);
			BandwidthBps.Set(bps);
		}
	}
}

//-----------------------------------------------------------------------------
BANDWIDTH_TYPE_ENUM cUserOptions::Get_Bandwidth_Type(void)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		return (BANDWIDTH_TYPE_ENUM) GameSpyBandwidthType.Get();
	} else {
		return (BANDWIDTH_TYPE_ENUM) BandwidthType.Get();
	}
}

//-----------------------------------------------------------------------------
void cUserOptions::Set_Bandwidth_Bps(int bandwidth_bps)
{
	WWASSERT(bandwidth_bps > 0);

	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		GameSpyBandwidthType.Set(BANDWIDTH_CUSTOM);
	} else {
		BandwidthType.Set(BANDWIDTH_CUSTOM);
	}

	BandwidthBps.Set(bandwidth_bps);
}




//-----------------------------------------------------------------------------
void cUserOptions::Reread(void)
{
	Sku.Set(RegistryClass(APPLICATION_SUB_KEY_NAME).Get_Int("SKU", Sku.Get()));
	BandwidthType.Set(RegistryClass(APPLICATION_SUB_KEY_NAME_NETOPTIONS).Get_Int("BandwidthType", BandwidthType.Get()));
	BandwidthBps.Set(RegistryClass(APPLICATION_SUB_KEY_NAME_NETOPTIONS).Get_Int("BandwidthBps", BandwidthBps.Get()));
	GameSpyBandwidthType.Set(RegistryClass(APPLICATION_SUB_KEY_NAME_GAMESPY).Get_Int("GameSpyBandwidthType", GameSpyBandwidthType.Get()));
}













/*
cRegistryInt cUserOptions::GameListFilterMaxPing(				APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterMaxPing",								9999);
cRegistryInt cUserOptions::GameListFilterMinPlayersPresent(	APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterMinPlayersPresent",					0);
cRegistryInt cUserOptions::GameListFilterMaxPlayersPresent(	APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterMaxPlayersPresent",					99);
cRegistryInt cUserOptions::GameListFilterMaxPlayersPermitted( APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterMaxPlayersPermitted",			99);
cRegistryBool cUserOptions::GameListFilterShowPrivateGames(	APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterShowPrivateGames",					true);
cRegistryBool cUserOptions::GameListFilterShowOnlyDedicatedGames(	APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterShowOnlyDedicatedGames",	false);
cRegistryBool cUserOptions::GameListFilterShowOnlyGamesIRankFor(	APPLICATION_SUB_KEY_NAME_NETOPTIONS, "GameListFilterShowOnlyGamesIRankFor",	false);
*/

		/*
		//
		// Gamespy client launch params.
		// All 3 must be specified.
		// Example: Renegade.exe GAMESPY_IPADDR=192.168.10.100 GAMESPY_PORT=3333 GAMESPY_NICKNAME="Bob 1234"
		//
		LPCSTR param = NULL;
		char * value = NULL;

		param = "GAMESPY_IPADDR=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			ULONG ip = ::inet_addr(value);
			cGameSpyAdmin::Set_Game_Host_Ip(ip);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		param = "GAMESPY_PORT=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			USHORT port = (USHORT)::atol(value);
			cGameSpyAdmin::Set_Game_Host_Port(port);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		param = "GAMESPY_NICKNAME=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			WideStringClass nickname;
			char temp[200] = "";
			char seps[]   = "\"";
			char * start_token = ::strtok(value, seps);
			if (start_token != NULL) {
				start_token++;
			}
			char * end_token = ::strtok(NULL, seps);
			if (end_token != NULL && end_token > start_token) {
				::strncpy(temp, start_token, end_token - start_token);
				temp[end_token - start_token] = 0;
			}

			nickname.Convert_From(temp);
			cGameSpyAdmin::Set_Player_Nickname(nickname);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}
		*/

		/*
		char nickname[300] = "";
		::sscanf(nickname_param, "%s", nickname);
		nickname[::strlen(nickname) - 1] = ' ';
		nickname[0] = ' ';

		char nickname2[300] = "";
		::sscanf(nickname, "%s", nickname2);
		*/

		/*
		char seps[]   = "\"";
		char * start_token = ::strtok(nickname_param, seps);
		if (start_token != NULL) {
			start_token++;
		}
		char * end_token = ::strtok(NULL, seps);
		char nickname2[300] = "";
		if (end_token != NULL && end_token > start_token) {
			char nickname2[300] = "";
			::strncpy(nickname2, start_token, end_token - start_token);
			nickname2[end_token - start_token] = 0;
		}
		*/

		/*
		WideStringClass wide_nickname;
		wide_nickname.Convert_From(nickname2);
		cGameSpyAdmin::Set_Player_Nickname(wide_nickname);
		*/