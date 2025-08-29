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
 *                     $Archive:: /Commando/Code/Commando/ServerSettings.cpp                  $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/09/02 2:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "ServerSettings.h"
#include "slavemaster.h"
#include "wwdebug.h"
#include "gamedata.h"
#include "gdcnc.h"
#include "ini.h"
#include "registry.h"
#include "rawfile.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include "_globals.h"
#include "bandwidth.h"
#include "mpsettingsmgr.h"
#include "WWOnline/WOLServer.h"
#include "useroptions.h"
#include "gamespyadmin.h"
#include "servercontrol.h"
#include "gamesideservercontrol.h"
#include "AutoStart.h"
#include "GameSpy_QnR.h"
#include "bandwidthcheck.h"



const char *ConfigSettingsName = "Config";
const char *MasterServerSection = "Server";
const char *SlaveServerSection = "Slave";

#define ENCRYPTION_STRING_LENGTH				128
#define KEY_SLAVE_SERIAL						"Serial"

char ServerSettingsClass::SettingsFile[MAX_PATH];
bool ServerSettingsClass::IsActive = false;
char ServerSettingsClass::MasterPassword[128];
ServerSettingsClass::GameModeTypeEnum ServerSettingsClass::GameMode = MODE_NONE;
unsigned long ServerSettingsClass::MasterBandwidth = 0;
char ServerSettingsClass::PreferredLoginServer[256];
int ServerSettingsClass::DiskLogSize = -1;

const char *ServerListTag = "Available Westwood Servers:";
const char *ServerListEnd = ";  End generated section.";






/***********************************************************************************************
 * ServerSettingsClass::Set_Settings_File_Name -- Set the name of the settings file            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    File name                                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 12:07PM ST : Created                                                             *
 *=============================================================================================*/
void ServerSettingsClass::Set_Settings_File_Name(char *filename)
{
	if (strlen(filename) < sizeof(SettingsFile)) {
		strcpy(SettingsFile, filename);
		IsActive = true;
	}
}



/***********************************************************************************************
 * ServerSettingsClass::Parse -- Pull the server info out of the settings file                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    True if we should apply the settings. False to parse for errors only.             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 12:07PM ST : Created                                                             *
 *=============================================================================================*/
bool ServerSettingsClass::Parse(bool apply)
{
	char master_settings[MAX_PATH];
	char slave_settings[MAX_PATH];
	char slave_section[256];
	char slave_nick[128];
	char slave_serial[128];
	char slave_pass[128];
	int slave_port;
	int slave_bw;
	char master_nick[128];
	char master_serial[128];
	int master_port;
	int master_bw;
	char master_pass[128];
	char remote_admin_pass[128];
	char game_type[32];
	char heartbeat_list[512];
	bool wol = true;
	char remote_admin_ip[128];

	MasterPassword[0] = 0;

	/*
	** IsActive is set when the settigns file name is set.
	*/
	if (IsActive) {

		WWASSERT(The_Game() || !apply);
		if (apply) {
			WWDEBUG_SAY(("Applying server settings\n"));
			ConsoleBox.Print("Applying server settings\n");
		}

		/*
		** Make sure the server config file is there. It should be since it's verified at command line parsing time.
		*/
		RawFileClass file(SettingsFile);
		if (!file.Is_Available()) {
			WWDEBUG_SAY(("Server startup file '%s' not found\n", SettingsFile));
			ConsoleBox.Print("Error - server startup file '%s' not found - aborting\n", SettingsFile);
			ConsoleBox.Wait_For_Keypress();
			return(false);
		}

		/*
		** Get the name of the master server settings file.
		*/
		INIClass ini(file);
		ini.Get_String(MasterServerSection, ConfigSettingsName, "", master_settings, sizeof(master_settings));
		if (strlen(master_settings) == 0) {
			if (apply) {
				WWDEBUG_SAY(("No master server settings specified - using defaults\n"));
				ConsoleBox.Print("No master server settings specified - using defaults\n");
			}
		}

		/*
		** Load the master server settings from the ini file.
		*/


		/*
		** Game Type.
		*/
		ini.Get_String(MasterServerSection, "GameType", "WOL", game_type, sizeof(game_type));
		if (cGameSpyAdmin::Get_Is_Server_Gamespy_Listed()) {
			strcpy(game_type, "GameSpy");
		}
		if (stricmp(game_type, "WOL") == 0) {
			GameMode = MODE_WOL;
		} else if (stricmp(game_type, "LAN") == 0) {
			wol = false;
			GameMode = MODE_LAN;
		} else if (stricmp(game_type, "GameSpy") == 0) {
			wol = false;
			cGameSpyAdmin::Set_Is_Server_Gamespy_Listed(true);
			GameSpyQnR.Enable_Reporting(true);
			GameMode = MODE_GAMESPY;
		} else {
			WWDEBUG_SAY(("Bad game type specified in server.ini\n"));
			ConsoleBox.Print("Error - Unknown game type in server settings. Use 'LAN', 'WOL', or 'GameSpy' - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}

		/*
		** Parse the list of GameSpy Style Master servers from the HeartBeat List
		*/
		heartbeat_list[sizeof(heartbeat_list)-1] = 0;
		ini.Get_String(MasterServerSection, "HeartBeatServers", GameSpyQnR.Get_Default_HeartBeat_List(), heartbeat_list, sizeof(heartbeat_list)-1);
		if (!GameSpyQnR.Parse_HeartBeat_List(heartbeat_list)) {
			GameSpyQnR.Parse_HeartBeat_List(GameSpyQnR.Get_Default_HeartBeat_List());
		}

		/*
		** Make sure the master server settings file is there.
		*/
		char filename[MAX_PATH];
		sprintf(filename, "data\\%s", master_settings);
		file.Set_Name(filename);
		if (!file.Is_Available()) {
			WWDEBUG_SAY(("Server settings file '%s' not found\n", filename));
			ConsoleBox.Print("Error - server settings file '%s' not found - aborting\n", filename);
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		} else {
			if (!Check_Game_Settings_File(master_settings)) {
				WWDEBUG_SAY(("Server settings file '%s' not usable\n", master_settings));
				ConsoleBox.Print("Error - server settings file '%s' contains errors - aborting\n", master_settings);
				ConsoleBox.Wait_For_Keypress();;
				return(false);
			}
			if (apply && The_Game()) {
				The_Game()->Set_Ini_Filename(master_settings);
			}
		}

		/*
		** Restart Flag
		*/

		RegistryClass restart_reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
		if (restart_reg.Is_Valid ()) {
			restart_reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 1);
			switch (GameMode) {
				case MODE_WOL:
					restart_reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE, 1);
					break;
				case MODE_LAN:
				case MODE_GAMESPY:
					restart_reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE, 0);
					break;
			}
		}

		/*
		** Nickname.
		*/
		ini.Get_String(MasterServerSection, "Nickname", "", master_nick, sizeof(master_nick));
#ifdef FREEDEDICATEDSERVER
		/*
		** We only need to validate this for the FDS. The regular game can allow the login name to be specified in the registry.
		*/
		if (wol && strlen(master_nick) == 0) {
			WWDEBUG_SAY(("Error - No login nickname specified for master server - aborting\n"));
			ConsoleBox.Print("Error - No login nickname specified for master server - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
#endif //FREEDEDICATEDSERVER

		/*
		** Password.
		*/
		ini.Get_String(MasterServerSection, "Password", "", master_pass, sizeof(master_pass));
#ifdef FREEDEDICATEDSERVER
		/*
		** We only need to validate this for the FDS. The regular game can allow the login name to be specified in the registry.
		*/
		if (wol && strlen(master_pass) == 0) {
			WWDEBUG_SAY(("Error - No login password specified for master server - aborting\n"));
			ConsoleBox.Print("Error - No login password specified for master server - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
#endif //FREEDEDICATEDSERVER

		/*
		** Serial number.
		*/
		ini.Get_String(MasterServerSection, "Serial", "", master_serial, sizeof(master_serial));
#ifdef FREEDEDICATEDSERVER
		/*
		** We only need to validate the serial number if we are the FDS. For the regular game, the master serial will be stored
		** in the registry.
		*/
		if (wol && strlen(master_serial) == 0) {
			WWDEBUG_SAY(("Error - No serial number specified for master server - aborting\n"));
			ConsoleBox.Print("Error - No serial number specified for master server - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
#endif //FREEDEDICATEDSERVER

		/*
		** Get the port number.
		*/
		master_port = ini.Get_Int(MasterServerSection, "Port", 0xffffffff);
		if (wol && master_port != 0xffffffff && (master_port < 0 || master_port > 0xffff)) {
			WWDEBUG_SAY(("Error - Invalid port number %d specified for master server - aborting\n", master_port));
			ConsoleBox.Print("Error - Invalid port number %d specified for master server - aborting\n", master_port);
			ConsoleBox.Wait_For_Keypress();
			return(false);
		}

		/*
		** Get the GameSpy Query port number.
		*/
		int gsqport = cUserOptions::GameSpyQueryPort.Get();
		gsqport = ini.Get_Int(MasterServerSection, "GameSpyQueryPort", gsqport);
		if (!gsqport) gsqport = cUserOptions::GameSpyQueryPort.Get();
		if (gsqport < 0 || gsqport > 0xffff) {
			WWDEBUG_SAY(("Error - Invalid port number %d specified for GameSpy Query Port - aborting\n", gsqport));
			ConsoleBox.Print("Error - Invalid port number %d specified for GameSpy Query Port - aborting\n", gsqport);
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
		cUserOptions::GameSpyQueryPort.Set(gsqport);

		/*
		** Get the GameSpy Game port number.
		*/
		int gsgport = cUserOptions::GameSpyGamePort.Get();
		gsgport = ini.Get_Int(MasterServerSection, "GameSpyGamePort", gsgport);
		if (!gsgport) gsgport = cUserOptions::GameSpyGamePort.Get();
		if (gsgport < 0 || gsgport > 0xffff || gsgport == gsqport) {
			WWDEBUG_SAY(("Error - Invalid port number %d specified for GameSpy Game Port - aborting\n", gsgport));
			ConsoleBox.Print("Error - Invalid port number %d specified for GameSpy Game Port - aborting\n", gsgport);
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
		cUserOptions::GameSpyGamePort.Set(gsgport);

		/*
		** Get the bandwidth allowance.
		*/
		master_bw = ini.Get_Int(MasterServerSection, "BandwidthUp", 0xffffffff);
		if (master_bw != 0 && master_bw != 0xffffffff && master_bw < 33600) {
			WWDEBUG_SAY(("Error - Insufficient bandwidth specified for master server - aborting\n"));
			ConsoleBox.Print("Error - Insufficient bandwidth specified for master server - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
		MasterBandwidth = master_bw;

		/*
		** Get the max size of the disk log in days.
		*/
		DiskLogSize = ini.Get_Int(MasterServerSection, "DiskLogSize", 7);
		if (DiskLogSize > 365) {
			WWDEBUG_SAY(("Error - Disk log size too large - aborting\n"));
			ConsoleBox.Print("Error - Disk log size too large - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}

		/*
		** Get the preferred login server. No preference means use default from ping profile.
		*/
		ini.Get_String(MasterServerSection, "LoginServer", "", PreferredLoginServer, sizeof(PreferredLoginServer));

		/*
		** Get the Network Update Rate override.
		*/
		int nur = ini.Get_Int(MasterServerSection, "NetUpdateRate", 8);
		if (nur < 5 || nur > 30) {
			WWDEBUG_SAY(("Error - Bad NetUpdateRate specified - aborting\n"));
			ConsoleBox.Print("Error - NetUpdateRate must be between 5 and 30 - aborting\n");
			ConsoleBox.Wait_For_Keypress();;
			return(false);
		}
		cUserOptions::NetUpdateRate.Set(nur);

		/*
		** Get the remote admin settings.
		*/
		bool allow_remote = ini.Get_Bool(MasterServerSection, "AllowRemoteAdmin", false);
		RegistryClass reg_remote(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);
		if (allow_remote) {
			ini.Get_String(MasterServerSection, "RemoteAdminPassword", "", remote_admin_pass, sizeof(remote_admin_pass));
			int len = strlen(remote_admin_pass);
			if (len == 0) {
				ConsoleBox.Print("Error - Remote admin password must be specified - aborting\n");
				ConsoleBox.Wait_For_Keypress();;
				return(false);
			} else {
				if (len > 31) {
					ConsoleBox.Print("Error - Remote admin password too long - aborting\n");
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}
			}
			int admin_port = ini.Get_Int(MasterServerSection, "RemoteAdminPort", 0);
			if (admin_port != 0 && (admin_port < 1024 || admin_port > 65535-8)) {
				ConsoleBox.Print("Error - Remote admin port number out of range - aborting\n");
				ConsoleBox.Wait_For_Keypress();;
				return(false);
			}

			ServerControl.Allow_Remote_Admin(true);

			/*
			** Set the port number into the registry.
			*/
			if (admin_port == 0) {
				admin_port = DEFAULT_SERVER_CONTROL_PORT;
			}
			reg_remote.Set_Int(SERVER_CONTROL_PORT_KEY, admin_port);

			/*
			** Set the password into the registry.
			*/
			reg_remote.Set_String(SERVER_CONTROL_PASSWORD_KEY, remote_admin_pass);

			/*
			** We need to bind to more than just the loopback address when listening for control messages.
			*/
			reg_remote.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 0);

			/*
			** There may be an IP override specified.
			*/
			ini.Get_String(MasterServerSection, "RemoteAdminIP", "0.0.0.0", remote_admin_ip, sizeof(remote_admin_ip));
			unsigned long admin_ip_long = ntohl(inet_addr(remote_admin_ip));
			reg_remote.Set_Int(SERVER_CONTROL_IP_KEY, admin_ip_long);

		} else {

			/*
			** Only listen to control messages on the loopback address.
			*/
			reg_remote.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 1);
			ServerControl.Allow_Remote_Admin(false);
		}


		/*
		** Set the master settings into the registry.
		*/
		//if (apply) {

			/*
			** Serial number.
			*/
			if (wol) {
				if (strlen(master_serial)) {
					StringClass serial(master_serial, true);
					StringClass encrypted_serial;
					Encrypt_Serial(serial, encrypted_serial);
					RegistryClass reg_base(APPLICATION_SUB_KEY_NAME);
					if (reg_base.Is_Valid()) {
						reg_base.Set_String(KEY_SLAVE_SERIAL, encrypted_serial.Peek_Buffer());
					}
				}

				/*
				** Nickname.
				*/
				if (strlen(master_nick)) {
					RegistryClass reg_wol(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
					if (reg_wol.Is_Valid()) {
						reg_wol.Set_String("AutoLogin", master_nick);
						reg_wol.Set_String("LastLogin", master_nick);
						reg_wol.Set_Int("AutoLoginPrompt", 0);
						MPSettingsMgrClass::Set_Auto_Login(master_nick);
					}
				}

				/*
				** Password.
				*/
				MPSettingsMgrClass::Set_Auto_Password(master_pass);

				/*
				** Port number.
				*/
				if (master_port != 0xffffffff) {
					RegistryClass reg_fw(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
					reg_fw.Set_Int("ForcePort", master_port);
				}
			}

			/*
			** Bandwidth.
			*/
			if (master_bw != 0xffffffff) {
				RegistryClass reg_netopt(APPLICATION_SUB_KEY_NAME_NETOPTIONS);
				if (reg_netopt.Is_Valid()) {
					if (cGameSpyAdmin::Is_Gamespy_Game()) {
						if (master_bw == 0) master_bw = 1000000;
						cUserOptions::Set_Bandwidth_Bps(master_bw);
					} else {
						if (wol) {
							reg_netopt.Set_Int("BandwidthType", BANDWIDTH_AUTO);

							/*
							** We want this to be set always on the first time through, but not neccessarily on the second, apply, pass.
							*/
							if (master_bw != 0 || !apply) {
								RegistryClass reg_bw(APPLICATION_SUB_KEY_NAME_BANDTEST);
								if (reg_bw.Is_Valid()) {
									reg_bw.Set_Int("Up", master_bw);
									reg_bw.Set_Int("Down", master_bw);
								}
							}
						} else {
							cUserOptions::Set_Bandwidth_Type(BANDWIDTH_LANT1);
						}
					}
				}
			}
		//}



		if (apply) {
			SlaveMaster.Reset();
		}

		/*
		** Read the slave server info.
		*/
		for (int i=0 ; i<MAX_SLAVES ; i++) {

			slave_settings[0] = 0;
			slave_nick[0] = 0;
			slave_serial[0] = 0;
			slave_port = 0;
			slave_bw = 0xffffffff;

			/*
			** See if this slave is enabled in the .ini
			*/
			sprintf(slave_section, "%s%d", SlaveServerSection, i+1);
			bool enabled = ini.Get_Bool(slave_section, "Enable", false);

			if (enabled) {

				/*
				** Make sure the slaves config file is present.
				*/
				ini.Get_String(slave_section, ConfigSettingsName, "", slave_settings, sizeof(slave_settings));
				if (strlen(slave_settings) != 0) {
					sprintf(filename, "data\\%s", slave_settings);
					file.Set_Name(filename);
					if (!file.Is_Available()) {
						WWDEBUG_SAY(("Slave server settings file '%s' not found\n", filename));
						ConsoleBox.Print("Error - Slave server settings file '%s' not found - aborting\n", filename);
						ConsoleBox.Wait_For_Keypress();;
						return(false);
					} else {
						if (!Check_Game_Settings_File(slave_settings)) {
							WWDEBUG_SAY(("Server settings file '%s' not usable\n", slave_settings));
							ConsoleBox.Print("Error - server settings file '%s' contains errors - aborting\n", slave_settings);
							ConsoleBox.Wait_For_Keypress();;
							return(false);
						}
					}
				}

				/*
				** Make sure there's a nickname and a serial.
				*/
				ini.Get_String(slave_section, "Nickname", "", slave_nick, sizeof(slave_nick));
				if (wol && strlen(slave_nick) == 0) {
					WWDEBUG_SAY(("Error - No login nickname specified for slave %d - aborting\n", i+1));
					ConsoleBox.Print("Error - No login nickname specified for slave %d - aborting\n", i+1);
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}

				ini.Get_String(slave_section, "Serial", "", slave_serial, sizeof(slave_serial));
				if (wol && strlen(slave_serial) == 0) {
					WWDEBUG_SAY(("Error - No serial number specified for slave %d - aborting\n", i+1));
					ConsoleBox.Print("Error - No serial number specified for slave %d - aborting\n", i+1);
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}

				/*
				** Get the password.
				*/
				ini.Get_String(slave_section, "Password", "", slave_pass, sizeof(slave_pass));
#ifdef FREEDEDICATEDSERVER
				if (wol && strlen(slave_pass) == 0) {
					WWDEBUG_SAY(("Error - No login password specified for slave %d - aborting\n", i+1));
					ConsoleBox.Print("Error - No login password specified for slave %d - aborting\n", i+1);
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}
#endif //FREEDEDICATEDSERVER

				/*
				** Get the port number.
				*/
				slave_port = ini.Get_Int(slave_section, "Port", 0);

				if (slave_port < 0 || slave_port > 0xffff) {
					WWDEBUG_SAY(("Error - Invalid port number %d specified for slave %d - aborting\n", slave_port, i+1));
					ConsoleBox.Print("Error - Invalid port number %d specified for slave %d - aborting\n", slave_port, i+1);
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}

				/*
				** Get the bandwidth allowance.
				*/
				slave_bw = ini.Get_Int(slave_section, "BandwidthUp", 0xffffffff);
				if (slave_bw != 0 && slave_bw != 0xffffffff && slave_bw < 33600) {
					WWDEBUG_SAY(("Error - Insufficient bandwidth specified for slave %d - aborting\n", i+1));
					ConsoleBox.Print("Error - Insufficient bandwidth specified for slave %d - aborting\n", i+1);
					ConsoleBox.Wait_For_Keypress();;
					return(false);
				}
			}

			/*
			** Apply the settings.
			*/
			if (apply) {
				SlaveMaster.Add_Slave(enabled, slave_nick, slave_serial, (unsigned short)slave_port, slave_settings, slave_bw, slave_pass);
			}
		}
		if (apply) {
			SlaveMaster.Save();
		}
	}
	return(true);
}






/***********************************************************************************************
 * ServerSettingsClass::Encrypt_Serial -- Serial number encryption/decryption                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Input serial number                                                               *
 *           Reference to output serial number                                                 *
 *           Encrypt flag - true to encrypt, false to decrypt                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 12:08PM ST : Created                                                             *
 *=============================================================================================*/
void ServerSettingsClass::Encrypt_Serial(StringClass serial_in, StringClass &serial_out, bool encrypt)
{
	char *s;
	int numberlength = serial_in.Get_Length();
	unsigned long bytesread;
	char stringbuffer[ENCRYPTION_STRING_LENGTH];
	int p;

	WWASSERT(numberlength);

	s = new char [numberlength + 1];
	memcpy(s, serial_in.Peek_Buffer(), numberlength + 1);

	/*
	** See if the key file is available. If not, don't bother encrypting.
	*/
	HANDLE handle = CreateFileA ("woldata.key", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		delete [] s;
		serial_out = serial_in;
		return;
	}

	/*
	** Read the key.
	*/
	if (!ReadFile(handle, stringbuffer, sizeof (stringbuffer), &bytesread, NULL)) {
		WWDEBUG_SAY(("Unable to read serial encryption key file\n"));
		delete [] s;
		serial_out = serial_in;
		return;
	}

	int sign = encrypt ? 1 : -1;

	p = 0;
	for (unsigned i = 0; i < ENCRYPTION_STRING_LENGTH; i++) {

		int  t;
		char c;

		t  = s[p] - '0';
		t %= 10;
		t += (sign * stringbuffer[i]);
		t += 1000;
		t %= 10;
		c  = t + '0';
		s[p] = c;
		p++;
		if (p == numberlength) {
			p = 0;
		}
	}

	serial_out = StringClass(s, true);

	delete [] s;
}




/***********************************************************************************************
 * ServerSettingsClass::Decrypt_Serial -- Serial number decryption                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Input serial number                                                               *
 *           Reference to output serial number                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 12:08PM ST : Created                                                             *
 *=============================================================================================*/
void ServerSettingsClass::Decrypt_Serial(StringClass serial_in, StringClass &serial_out)
{
	Encrypt_Serial(serial_in, serial_out, false);
}



/***********************************************************************************************
 * ServerSettingsClass::Get_Preferred_Server -- Get the users specified server                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Server list from servserv                                                         *
 *                                                                                             *
 * OUTPUT:   Name of preferred server                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 1:18PM ST : Created                                                              *
 *=============================================================================================*/
char *ServerSettingsClass::Get_Preferred_Server(const WWOnline::IRCServerList &server_list)
{
	if (IsActive && server_list.size()) {
		Write_Server_List(server_list);
	}
	return(PreferredLoginServer);
}



/***********************************************************************************************
 * ServerSettingsClass::Write_Server_List -- Write the server list into the settings ini file  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Server list                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/1/2002 1:19PM ST : Created                                                              *
 *=============================================================================================*/
void ServerSettingsClass::Write_Server_List(const WWOnline::IRCServerList &server_list)
{
	WWASSERT(server_list.size());
	WWASSERT(strlen(SettingsFile) != 0);

	char temp[256];

	if (server_list.size()) {
		RawFileClass file(SettingsFile);

		/*
		** This is a bit hacky. Basically we just want the .ini file commented with the latest server list.
		**
		** We will do this by reading in the file, looking for a tag that tells us where to write the server list, then writing
		** the file back out with the server list in place.
		*/
		if (file.Is_Available()) {
			unsigned long size = file.Size();
			if (size) {

				/*
				** Read the file.
				*/
				char *file_buffer = new char[size + 8192];
#ifdef WWDEBUG
				unsigned long read_size =
#endif //WWDEBUG
				file.Read(file_buffer, size);
				WWASSERT(read_size == size);
				file.Close();

				/*
				** Find the placeholder tag.
				*/
				char *tag = strstr(file_buffer, ServerListTag);
				char *end_tag = strstr(file_buffer, ServerListEnd);
				if (tag && end_tag) {

					/*
					** Write out the first portion of the file unchanged.
					*/
					file.Open(FileClass::WRITE);
					file.Write(file_buffer, (tag - file_buffer) + strlen(ServerListTag));

					/*
					** Insert the server list.
					*/
					char *server_list_text = new char [8192];
					strcpy(server_list_text, "\r\n;\r\n");

					for (unsigned int i = 0; i < server_list.size(); i++)	{
						const RefPtr<WWOnline::IRCServerData> &server = server_list[i];
						if (server->HasLanguageCode()) {
							const char *server_name = server->GetName();
							sprintf(temp, ";    %s\r\n", server_name);
							strcat(server_list_text, temp);
						}
					}
					strcat(server_list_text, ";\r\n");
					file.Write(server_list_text, strlen(server_list_text));

					/*
					** Write out the post server list sections of the original file.
					*/
					file.Write(end_tag, (file_buffer + size) - end_tag);
					file.Close();
				}
			}
		}
	}
}




/***********************************************************************************************
 * ServerSettingsClass::Check_Game_Settings_File -- Check game settings for validity           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Game settings .ini file name                                                      *
 *                                                                                             *
 * OUTPUT:   True if valid.                                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/3/2002 9:37PM ST : Created                                                              *
 *=============================================================================================*/
bool ServerSettingsClass::Check_Game_Settings_File(char *config_file)
{
	cGameDataCnc *game_settings = (cGameDataCnc*) cGameData::Create_Game_Of_Type(cGameData::GAME_TYPE_CNC);
	WWASSERT(game_settings != NULL);

	if (game_settings) {
		game_settings->Set_Ini_Filename(config_file);
		game_settings->Load_From_Server_Config();

		WideStringClass outMsg;
		bool ok = game_settings->Is_Valid_Settings(outMsg, true);

		delete game_settings;
		return(ok);
	}
	return(false);
}
