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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/slavemaster.cpp                     $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 2/15/02 12:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include <windows.h>
#include "slavemaster.h"
#include "wwdebug.h"
#include "registry.h"
#include "_globals.h"
#include "AutoStart.h"
#include "ini.h"
#include "rawfile.h"
#include "inisup.h"
#include "natter.h"
#include "gamesideservercontrol.h"
#include "win.h"
#include "gamedata.h"
#include "ServerSettings.h"
#include "bandwidth.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include "useroptions.h"


#include <string.h>
#include <stdio.h>

#define KEY_NUM_SLAVES				"Count"
#define KEY_SLAVE_NAME				"Name"
#define KEY_SLAVE_SERIAL			"Serial"
#define KEY_SLAVE_ENABLE			"Enable"
#define KEY_SLAVE_PORT				"Port"
#define KEY_SLAVE_RUNNING_ID		"RunningID"
#define KEY_SLAVE_SETTINGS			"Settings"
#define KEY_SLAVE_BANDWIDTH		"Bandwidth"
#define KEY_SLAVE_PASSWORD			"Password"

const char *RegistryFileName = "slave.ini";

SlaveMasterClass SlaveMaster;


extern char DefaultRegistryModifier[1024];


/***********************************************************************************************
 * SlaveServerClass::SlaveServerClass -- SlaveServerClass constuctor                           *
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
 *   11/21/2001 3:51PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass::SlaveServerClass(void)
{
	Enable = false;
	NickName[0] = 0;
	Serial[0] = 0;
	Port = 0;
	Bandwidth = 0;
	Password[0] = 0;
}


/***********************************************************************************************
 * SlaveServerClass::~SlaveServerClass -- SlaveServerClass desturctor                          *
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
 *   11/21/2001 3:51PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass::~SlaveServerClass(void)
{
}



/***********************************************************************************************
 * SlaveServerClass::Set -- Set info about this slave                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname to use with this slave                                                   *
 *           Serial number to use with this slave                                              *
 *           Port to use with this slave                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:50PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveServerClass::Set(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password)
{
	Enable = enable;
	Port = port;
	Bandwidth = bandwidth;

	if (nick) {
		strncpy(NickName, nick, sizeof(NickName));
	}

	if (serial) {
		strncpy(Serial, serial, sizeof(Serial));
	}

	if (password) {
		strncpy(Password, password, sizeof(Password));
	}

	if (settings_file) {
		strncpy(SettingsFileName, settings_file, sizeof(SettingsFileName));
	}

}



/***********************************************************************************************
 * SlaveServerClass::Get -- Get info about this slave                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname to use with this slave                                                   *
 *           Serial number to use with this slave                                              *
 *           Port to use with this slave                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveServerClass::Get(bool &enable, char *nick, char *serial, unsigned short &port, char *settings_file, int &bandwidth, char *password)
{
	enable = Enable;
	port = Port;
	bandwidth = Bandwidth;

	if (nick) {
		strcpy(nick, NickName);
	}

	if (serial) {
		strcpy(serial, Serial);
	}

	if (password) {
		strcpy(password, Password);
	}

	if (settings_file) {
		strcpy(settings_file, SettingsFileName);
	}
}




/***********************************************************************************************
 * SlaveMasterClass::SlaveMasterClass -- SlaveMasterClass constructor                          *
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
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
SlaveMasterClass::SlaveMasterClass(void)
{
	NumSlaveServers = 0;
	SlaveMode = false;
}


/***********************************************************************************************
 * SlaveMasterClass::~SlaveMasterClass -- SlaveMasterClass destructor                          *
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
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
SlaveMasterClass::~SlaveMasterClass(void)
{
	/*
	** Make sure all slaves are gone before we quit.
	*/
	Wait_For_Slave_Shutdown();
}





/***********************************************************************************************
 * SlaveMasterClass::Wait_For_Slave_Shutdown -- Wait for slaves to exit                        *
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
 *   1/7/2002 2:58PM ST : Created                                                              *
 *=============================================================================================*/
void SlaveMasterClass::Wait_For_Slave_Shutdown(void)
{
	if (!SlaveMode) {

		unsigned long time = TIMEGETTIME();
		int num_running = 0;
		int last_num_running = 0;
		bool forced = false;

		/*
		** Don't wait longer than 35 secs. It takes 15 secs for the slave to do it's intermission.
		*/
		while (TIMEGETTIME() - time < 40000) {

			num_running = 0;

			for (int i=0 ; i<NumSlaveServers ; i++) {
				if (SlaveServers[i].IsRunning) {
					if (SlaveServers[i].ProcessInfo.hProcess) {
						unsigned long code;
						int res = GetExitCodeProcess(SlaveServers[i].ProcessInfo.hProcess, &code);

						if (res && code == STILL_ACTIVE) {
							num_running++;
						}
					} else {
						if (SlaveServers[i].ProcessInfo.dwProcessId) {
							unsigned long ver = GetProcessVersion(SlaveServers[i].ProcessInfo.dwProcessId);
							if (ver && ver == GetProcessVersion(GetCurrentProcessId())) {
								num_running++;
							}
						}
					}
				}
			}
			if (num_running && num_running != last_num_running) {
				ConsoleBox.Print("Waiting for %d slave(s) to shut down\n", num_running);
				WWDEBUG_SAY(("Waiting for %d slave(s) to shut down\n", num_running));
				last_num_running = num_running;
			}
			if (num_running == 0) {
				break;
			}

			/*
			** Force a shutdown if they are not cooperating.
			*/
			if (!forced && TIMEGETTIME() - time > 27000) {
				forced = true;
				for (int i=0 ; i<NumSlaveServers ; i++) {
					if (SlaveServers[i].ProcessInfo.dwProcessId) {
						unsigned long ver = GetProcessVersion(SlaveServers[i].ProcessInfo.dwProcessId);
						if (ver && ver == GetProcessVersion(GetCurrentProcessId())) {

							WWDEBUG_SAY(("Terminating process %d due to timeout\n", SlaveServers[i].ProcessInfo.dwProcessId));

							/*
							** Get a handle to the process.
							*/
							HANDLE proc_handle = OpenProcess(PROCESS_TERMINATE, false, SlaveServers[i].ProcessInfo.dwProcessId);
							if (proc_handle != INVALID_HANDLE_VALUE) {
								TerminateProcess(proc_handle, 0);
								CloseHandle(proc_handle);
							} else {
								WWDEBUG_SAY(("Failed to get process handle for termination - error code %d\n", GetLastError()));
							}
							num_running++;
						}
					}
				}
			}
		}
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Get_Slave -- Get slave server entry by index                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Index                                                                             *
 *                                                                                             *
 * OUTPUT:   Ptr to slave server                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:48PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass *SlaveMasterClass::Get_Slave(int index)
{
	WWASSERT(index < NumSlaveServers);
	WWASSERT(index >= 0);
	if (index >= 0 && index <NumSlaveServers) {
		return(&SlaveServers[index]);
	}
	return(NULL);
}



/***********************************************************************************************
 * SlaveMasterClass::Save -- Save slave server info to registry                                *
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
 *   11/21/2001 3:48PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Save(void)
{
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
	if (reg.Is_Valid()) {
		reg.Set_Int(KEY_NUM_SLAVES, NumSlaveServers);
	}

	for (int i=0 ; i<NumSlaveServers ; i++) {
		char entry[256];
		sprintf(entry, "%s%d", KEY_SLAVE_NAME, i);
		reg.Set_String(entry, SlaveServers[i].NickName);

		sprintf(entry, "%s%d", KEY_SLAVE_PASSWORD, i);
		reg.Set_String(entry, SlaveServers[i].Password);

		sprintf(entry, "%s%d", KEY_SLAVE_SETTINGS, i);
		reg.Set_String(entry, SlaveServers[i].SettingsFileName);

		sprintf(entry, "%s%d", KEY_SLAVE_ENABLE, i);
		reg.Set_Bool(entry, SlaveServers[i].Enable);

		sprintf(entry, "%s%d", KEY_SLAVE_PORT, i);
		reg.Set_Int(entry, SlaveServers[i].Port);

		sprintf(entry, "%s%d", KEY_SLAVE_BANDWIDTH, i);
		reg.Set_Int(entry, SlaveServers[i].Bandwidth);

		sprintf(entry, "%s%d", KEY_SLAVE_SERIAL, i);
		StringClass serial(SlaveServers[i].Serial, true);
		StringClass encrypted_serial = serial;
		if (serial.Get_Length()) {
			ServerSettingsClass::Encrypt_Serial(serial, encrypted_serial);
		}
		reg.Set_String(entry, encrypted_serial.Peek_Buffer());
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Load -- Fetch slave server info from registry                             *
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
 *   11/21/2001 3:47PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Load(void)
{
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
	if (reg.Is_Valid()) {
		NumSlaveServers = reg.Get_Int(KEY_NUM_SLAVES, 0);
	}

	char entry[256];
	for (int i=0 ; i<NumSlaveServers ; i++) {
		sprintf(entry, "%s%d", KEY_SLAVE_NAME, i);
		reg.Get_String(entry, SlaveServers[i].NickName, sizeof(SlaveServers[i].NickName), "");

		sprintf(entry, "%s%d", KEY_SLAVE_PASSWORD, i);
		reg.Get_String(entry, SlaveServers[i].Password, sizeof(SlaveServers[i].Password), "");

		sprintf(entry, "%s%d", KEY_SLAVE_ENABLE, i);
		SlaveServers[i].Enable = reg.Get_Bool(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_PORT, i);
		SlaveServers[i].Port = reg.Get_Int(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_BANDWIDTH, i);
		SlaveServers[i].Bandwidth = reg.Get_Int(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_SETTINGS, i);
		reg.Get_String(entry, SlaveServers[i].SettingsFileName, sizeof(SlaveServers[i].SettingsFileName), "");

		sprintf(entry, "%s%d", KEY_SLAVE_SERIAL, i);
		reg.Get_String(entry, SlaveServers[i].Serial, sizeof(SlaveServers[i].Serial), "");
		if (strlen(SlaveServers[i].Serial)) {
			StringClass serial(SlaveServers[i].Serial, true);
			StringClass decrypted_serial = serial;
			if (serial.Get_Length()) {
				ServerSettingsClass::Decrypt_Serial(serial, decrypted_serial);
			}
			strcpy(SlaveServers[i].Serial, decrypted_serial.Peek_Buffer());
		}

		char filename[MAX_PATH];
		sprintf(filename, "data\\%s", SlaveServers[i].SettingsFileName);
		RawFileClass file(filename);
		if (!file.Is_Available()) {
			strcpy(SlaveServers[i].SettingsFileName, "svrcfg_cnc.ini");
		}
	}
}


/***********************************************************************************************
 * SlaveMasterClass::Reset -- Clear out slave server list                                      *
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
 *   11/21/2001 3:47PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Reset(void)
{
	for (int i=0 ; i<NumSlaveServers ; i++) {
		SlaveServers[i].Set(false, "", "", 0, "", 0, "");
	}
	NumSlaveServers = 0;
}




/***********************************************************************************************
 * SlaveMasterClass::Add_Slave -- Add slave to list                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname for this slave to use                                                    *
 *           Serial number for this slave to use                                               *
 *           Port number for this slave to use                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:46PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Add_Slave(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password)
{
	WWASSERT(NumSlaveServers < MAX_SLAVES);
	SlaveServers[NumSlaveServers++].Set(enable, nick, serial, port, settings_file, bandwidth, password);
}




/***********************************************************************************************
 * SlaveMasterClass::Aquire_Slave -- Find a running slave by it's process ID                   *
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
 *   12/16/2001 11:34PM ST : Created                                                           *
 *=============================================================================================*/
bool SlaveMasterClass::Aquire_Slave(int index)
{
	int proc_id = 0;

	/*
	** Try the slaves record of his process ID. If it's not there, it can't have run yet.
	*/
	char slave_name[64];
	sprintf(slave_name, "\\slave_%d", index);
	strcpy(DefaultRegistryModifier, slave_name+1);
	RegistryClass slave_reg(APPLICATION_SUB_KEY_NAME);
	DefaultRegistryModifier[0] = 0;
	if (slave_reg.Is_Valid()) {
		proc_id = slave_reg.Get_Int("ProcessId", proc_id);
	}

	/*
	** Try our record of the slaves process ID.
	*/
	if (proc_id == 0) {
		RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
		if (reg.Is_Valid()) {
			char entry[128];
			sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, index);
			proc_id = reg.Get_Int(entry, 0);
		}
	}

	/*
	** Search for the slave's console window if we are in console mode. This is a better test than just hoping the Process ID is
	** correct.
	*/
	if (ConsoleBox.Is_Exclusive()) {
		HWND slave_window = ConsoleBox.Get_Slave_Window_By_Title(SlaveServers[index].NickName, SlaveServers[index].SettingsFileName);
		if (slave_window != NULL) {
			/*
			** Note the process ID for later.
			*/
			SlaveServers[index].ProcessInfo.hProcess = NULL;	// Don't know handle.
			GetWindowThreadProcessId(slave_window, &SlaveServers[index].ProcessInfo.dwProcessId);
			WWDEBUG_SAY(("Slave found by HWND with process ID %d\n", SlaveServers[index].ProcessInfo.dwProcessId));

			RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
			if (reg.Is_Valid()) {
				char entry[128];
				sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, index);
				reg.Set_Int(entry, SlaveServers[index].ProcessInfo.dwProcessId);
			}
			return(true);
		}
	}

	/*
	** See if the process is already running.
	*/
	if (proc_id) {

		unsigned long ver = GetProcessVersion(proc_id);
		if (ver && ver == GetProcessVersion(GetCurrentProcessId())) {

			/*
			** It looks like one of our slaves. See if we already know about it.
			*/
			if (SlaveServers[index].IsRunning && SlaveServers[index].ProcessInfo.dwProcessId == (unsigned) proc_id) {
				return(true);
			}

			/*
			** Note the process ID for later.
			*/
			SlaveServers[index].ProcessInfo.hProcess = NULL;	// Don't know handle.
			SlaveServers[index].ProcessInfo.dwProcessId = proc_id;
			WWDEBUG_SAY(("Slave found with process ID %d\n", SlaveServers[index].ProcessInfo.dwProcessId));
			return(true);
		}
	}

	return(false);
}




/***********************************************************************************************
 * SlaveMasterClass::Startup_Slaves -- Create extra slave server processes                     *
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
 *   11/21/2001 3:46PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Startup_Slaves(void)
{
	if (!SlaveMode) {

		/*
		** Make sure we are a dedicated server.
		*/
		if (!The_Game() || The_Game()->IsDedicated.Is_True()) {

			/*
			** Slaves only available in windowed or console mode.
			*/
			if (WW3D::Is_Windowed() || ConsoleBox.Is_Exclusive()) {

				/*
				** Slaves only available in internet mode.
				*/
				GameModeClass *game_mode = GameModeManager::Find("WOL");
				if (game_mode && game_mode->Is_Active()) {

					Wait_For_Slave_Shutdown();

					Load();
					Delete_Registry_Copies();
					Create_Registry_Copies();

					/*
					** Spawn the servers.
					*/
					char command_line[300];
					for (int i=0 ; i<NumSlaveServers ; i++) {
						if (SlaveServers[i].Enable) {

							bool slave_running = false;

							/*
							** Get an access point into the slaves registry base.
							*/
							char slave_name[64];
							sprintf(slave_name, "\\slave_%d", i);
							strcpy(DefaultRegistryModifier, slave_name+1);
							RegistryClass slave_reg(APPLICATION_SUB_KEY_NAME);
							DefaultRegistryModifier[0] = 0;

							/*
							** If we are autostarting then take inventory of which slaves are running already.
							** An autostart after a crash should still see the slaves running. An autostart after a reboot will see no slaves.
							*/
							if (AutoRestart.Is_Active()) {
								slave_running = Aquire_Slave(i);
							}

							/*
							** Figure out the name of the .exe to run.
							*/
							char path_to_exe[256];
							char drive[_MAX_DRIVE];
							char dir[_MAX_DIR];
							char path[_MAX_PATH];
							GetModuleFileNameA(ProgramInstance, path_to_exe, sizeof(path_to_exe));
							_splitpath(path_to_exe, drive, dir, NULL, NULL);
#ifdef FREEDEDICATEDSERVER
							_makepath(path, drive, dir, "renegadeserver", "exe");
#else  //FREEDEDICATEDSERVER
							_makepath(path, drive, dir, "renegade", "exe");
#endif //FREEDEDICATEDSERVER

							sprintf(command_line, "%s /MULTI /SLAVE /REGMOD=slave_%d", path, i);
							if (ConsoleBox.Is_Exclusive()) {
								strcat(command_line, " /NODX");
							}
							STARTUPINFOA startup_info;
							memset(&startup_info, 0, sizeof(startup_info));
							startup_info.cb = sizeof(startup_info);

							int result = 1;
							if (!slave_running) {

								if (slave_reg.Is_Valid()) {
									slave_reg.Set_Int("ProcessId", 0);
								}
								result = CreateProcessA(path, command_line, NULL, NULL, false, 0, NULL, NULL, &startup_info, &SlaveServers[i].ProcessInfo);
							}
							if (result) {
								SlaveServers[i].IsRunning = true;

								/*
								** The process ID we have here is actually the ID of the slaves launcher. We need the ID of the actual
								** game process. Wait a few seconds until the slave sets his ID into his registry location.
								*/
								if (!slave_running) {
									unsigned long time = TIMEGETTIME();
									while (TIMEGETTIME() - time < 10000) {
										if (!slave_reg.Is_Valid()) {
											break;
										}

										/*
										** Break out once we read the slaves process ID from the registry indicating that the slave
										** has already parsed its command line.
										*/
										int process_id = slave_reg.Get_Int("ProcessId", 0);
										if (process_id != 0) {
											SlaveServers[i].ProcessInfo.dwProcessId = process_id;
											break;
										}
										Sleep(250);
									}
								}

								/*
								** Set a registry flag to say this server is active. We need to know this if the master server (us)
								** crashes and restarts.
								*/
								RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
								if (reg.Is_Valid()) {
									char entry[128];
									sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
									reg.Set_Int(entry, SlaveServers[i].ProcessInfo.dwProcessId);
								}

							} else {
								WWDEBUG_SAY(("Failed to start slave process - error code %d\n", GetLastError()));
								SlaveServers[i].IsRunning = false;
							}
						}
					}
				}
			}
		}
	}
	GameSideServerControlClass::Set_Welcome_Message();
}





/***********************************************************************************************
 * SlaveMasterClass::Shutdown_Slaves -- Send quit message to all slaves                        *
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
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Shutdown_Slaves(void)
{
	if (!SlaveMode) {
		char password[64] = DEFAULT_SERVER_CONTROL_PASSWORD;
		RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);
		reg.Get_String(SERVER_CONTROL_PASSWORD_KEY, password, sizeof(password), password);

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning) {

				/*
				** In case the slave was restarted - we won't know it's new process ID.
				*/
				Aquire_Slave(i);

				/*
				** Set the slaves auto-restart flag to false or it will just start right up again.
				*/
				char slave_name[64];
				sprintf(slave_name, "\\slave_%d", i);
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass slave_reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				DefaultRegistryModifier[0] = 0;
				slave_reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 0);

				/*
				** Send the password to the slave to authenticate the connection.
				*/
				GameSideServerControlClass::Send_Message(password, ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);
				Sleep(10);
				GameSideServerControlClass::Send_Message("quit", ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);

				/*
				** Remember that we shut this guy down.
				*/
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
				if (reg.Is_Valid()) {
					char entry[128];
					sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
					reg.Set_Int(entry, 0);
				}
			}
		}
	}
}






/***********************************************************************************************
 * SlaveMasterClass::Shutdown_Slaves -- Send quit message to all slaves                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Slave login name                                                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
bool SlaveMasterClass::Shutdown_Slave(char *slave_login)
{
	if (!SlaveMode && slave_login) {
		char password[64] = DEFAULT_SERVER_CONTROL_PASSWORD;
		RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);
		reg.Get_String(SERVER_CONTROL_PASSWORD_KEY, password, sizeof(password), password);

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning && stricmp(slave_login, SlaveServers[i].NickName) == 0) {

				/*
				** In case the slave was restarted - we won't know it's new process ID.
				*/
				Aquire_Slave(i);

				/*
				** Set the slaves auto-restart flag to false or it will just start right up again.
				*/
				char slave_name[64];
				sprintf(slave_name, "\\slave_%d", i);
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass slave_reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				DefaultRegistryModifier[0] = 0;
				slave_reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 0);

				/*
				** Send the password to the slave to authenticate the connection.
				*/
				GameSideServerControlClass::Send_Message(password, ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);
				Sleep(10);
				GameSideServerControlClass::Send_Message("quit", ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);

				/*
				** Remember that we shut this guy down.
				*/
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SLAVE);
				if (reg.Is_Valid()) {
					char entry[128];
					sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
					reg.Set_Int(entry, 0);
				}
				SlaveServers[i].IsRunning = false;
				return(true);
			}
		}
	}
	return(false);
}




/***********************************************************************************************
 * SlaveMasterClass::Get_Slave_Info -- Get text slave info                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to text buffer                                                                *
 *           buffer size                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
char *SlaveMasterClass::Get_Slave_Info(char *buffer, int buflen)
{
	bool any = false;
	if (buffer) {
		assert(buflen >= 500);
		*buffer = 0;

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning) {
				any = true;
				char temp[64];
				sprintf(temp, " Slave %d on port %d\n", i+1, SlaveServers[i].ControlPort);
				if (strlen(temp) + strlen(buffer) < (unsigned)buflen) {
					strcat(buffer, temp);
				}
			}
		}
		if (!any) {
			strcpy(buffer, "No slave servers active\n");
		}
	}
	return(buffer);
}




/***********************************************************************************************
 * SlaveMasterClass::Create_Registry_Copies -- Create 'shadow' registry copies for slaves      *
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
 *   11/21/2001 3:44PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Create_Registry_Copies(void)
{
	WWASSERT(!SlaveMode);

	/*
	** Make sure the Process ID isn't set in our base registry. It's shouldn't be unless I ran with the /slave command during dev.
	*/
	RegistryClass reg(APPLICATION_SUB_KEY_NAME);
	if (reg.Is_Valid()) {
		reg.Delete_Value("ProcessId");
	}

	RegistryClass::Save_Registry(RegistryFileName, APPLICATION_SUB_KEY_NAME);

	char new_path[1024];
	char slave_name[64];

	for (int i=0 ; i<NumSlaveServers ; i++) {
		if (SlaveServers[i].Enable) {
			strcpy(new_path, APPLICATION_SUB_KEY_NAME);
			sprintf(slave_name, "\\slave_%d", i);
			strcat(new_path, slave_name);
			RegistryClass::Load_Registry(RegistryFileName, APPLICATION_SUB_KEY_NAME, new_path);

			/*
			** Store the slave settings into the registry.
			*/

			/*
			** Port numbers.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);
				DefaultRegistryModifier[0] = 0;
				RegistryClass my_reg(APPLICATION_SUB_KEY_NAME_NET_FIREWALL);

				if (SlaveServers[i].Port != 0) {
					reg.Set_Int("ForcePort", SlaveServers[i].Port);
				} else {
					reg.Set_Int("ForcePort", 0);

					int port = my_reg.Get_Int("PortBase", PORT_BASE_MIN);
					port = port + ((i+1) * 256);
					if (port >= PORT_BASE_MAX-1) {
						port -= (PORT_BASE_MAX - PORT_BASE_MIN);
					}
					reg.Set_Int("PortBase", port);

					port = my_reg.Get_Int("PortPool", PORT_BASE_MIN);
					port = port + ((i+1) * 1024);
					if (port >= PORT_POOL_MAX-1) {
						port -= (PORT_POOL_MAX - PORT_POOL_MIN);
					}
					reg.Set_Int("PortPool", port);
				}
			}


			/*
			** Server control info.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);
				DefaultRegistryModifier[0] = 0;
				RegistryClass my_reg(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);

				/*
				** The password will be the same for all slaves but they each need a port to listen on.
				*/
				int my_sc_port = my_reg.Get_Int(SERVER_CONTROL_PORT_KEY, DEFAULT_SERVER_CONTROL_PORT);
				int slave_port = my_sc_port;
				if (my_sc_port == 0) {
					/*
					** If server control isn't enabled for me then we need to make up some port.
					*/
					slave_port = DEFAULT_SERVER_CONTROL_PORT;
				}
				slave_port += i;
				slave_port++;
				SlaveServers[i].ControlPort = slave_port;
				reg.Set_Int(SERVER_CONTROL_PORT_KEY, slave_port);

				/*
				** Inherit this from the master now.
				*/
				//if (my_sc_port == 0) {
				//	reg.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 1);
				//} else {
				//	reg.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 0);
				//}
			}

			/*
			** Login name.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				DefaultRegistryModifier[0] = 0;

				reg.Set_String("AutoLogin", SlaveServers[i].NickName);
				reg.Set_String("LastLogin", SlaveServers[i].NickName);
			}

			/*
			** Password name.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				DefaultRegistryModifier[0] = 0;

				reg.Set_String("AutoPassword", SlaveServers[i].Password);
			}


			/*
			** Serial number.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME);
				DefaultRegistryModifier[0] = 0;

				StringClass serial(SlaveServers[i].Serial, true);
				StringClass encrypted_serial = serial;
				if (serial.Get_Length()) {
					ServerSettingsClass::Encrypt_Serial(serial, encrypted_serial);
				}
				reg.Set_String(KEY_SLAVE_SERIAL, encrypted_serial.Peek_Buffer());
			}

			/*
			** Make it autostart.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_WOLSETTINGS);
				DefaultRegistryModifier[0] = 0;

				if (reg.Is_Valid()) {
					reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_FLAG, 1);

					int game_type = 0;
					GameModeClass *game_mode = GameModeManager::Find("WOL");
					if (game_mode && game_mode->Is_Active()) {
						game_type = 1;
					}
					reg.Set_Int(AutoRestartClass::REG_VALUE_AUTO_RESTART_TYPE, game_type);
				}
			}

			/*
			** Tell it which multiplayer settings to use.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_OPTIONS);
				DefaultRegistryModifier[0] = 0;
				reg.Set_String("MultiplayerSettings", SlaveServers[i].SettingsFileName);
			}

			/*
			** Set the SKU number to be the FDS SKU. Do this whether the Master is a FDS or not.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME);
				DefaultRegistryModifier[0] = 0;
				reg.Set_Int("SKU", RENEGADE_FDS_SKU);
			}

			/*
			** Set the bandwidth information.
			** A value of 0 means auto. A value of 0xffffffff means not specified (i.e. use master settings).
			*/
			{
				int bw = SlaveServers[i].Bandwidth;
				if (bw != -1) {
					strcpy(DefaultRegistryModifier, slave_name+1);
					RegistryClass reg_netopt(APPLICATION_SUB_KEY_NAME_NETOPTIONS);
					RegistryClass reg_bw(APPLICATION_SUB_KEY_NAME_BANDTEST);
					DefaultRegistryModifier[0] = 0;
					RegistryClass my_reg_netopt(APPLICATION_SUB_KEY_NAME_NETOPTIONS);
					RegistryClass my_reg_bw(APPLICATION_SUB_KEY_NAME_BANDTEST);

					//reg_netopt.Set_Int("BandwidthType", BANDWIDTH_AUTO);
					cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);
					int slave_bw = bw;

					/*
					** If bandwidth is set to auto then divide it by the number of servers on this box.
					*/
					if (slave_bw == 0) {
						slave_bw = my_reg_bw.Get_Int("Up", 0);
						int num = Get_Num_Enabled_Slaves();
						if (num) {
							slave_bw = slave_bw / (num+1);
						}
					}
					reg_bw.Set_Int("Up", slave_bw);
					reg_bw.Set_Int("Down", slave_bw);
				}
			}


#if (0)
			/*
			** Give the window a different position so we are not completely overlapping.
			*/
			{
				strcpy(DefaultRegistryModifier, slave_name+1);
				RegistryClass reg(APPLICATION_SUB_KEY_NAME_OPTIONS);
				DefaultRegistryModifier[0] = 0;
				reg.Set_Int("WindowX", (i * 32) + 32);
				reg.Set_Int("WindowY", (i * 32) + 32);
			}
#endif //(0)
		}
	}
}





/***********************************************************************************************
 * SlaveMasterClass::Delete_Registry_Copies -- Delete old slave registry copies                *
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
 *   11/21/2001 3:44PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Delete_Registry_Copies(void)
{
	HKEY base_key;
	long result = RegOpenKeyExA(HKEY_CURRENT_USER, APPLICATION_SUB_KEY_NAME, 0, KEY_ALL_ACCESS, &base_key);
	WWASSERT(result == ERROR_SUCCESS);

	if (result == ERROR_SUCCESS) {
		int index = 0;
		char new_path[1024];
		char slave_name[64];

		while (index < MAX_SLAVES) {
			strcpy(new_path, APPLICATION_SUB_KEY_NAME);
			sprintf(slave_name, "\\slave_%d", index);
			strcat(new_path, slave_name);
			RegistryClass::Delete_Registry_Tree(new_path);
			index++;
		}
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Get_Num_Enabled_Slaves -- How many slaves are enabled?                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Number of enabled slaves                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/5/2002 11:47PM ST : Created                                                             *
 *=============================================================================================*/
int SlaveMasterClass::Get_Num_Enabled_Slaves(void)
{
	int enabled = 0;

	for (int i=0 ; i<NumSlaveServers ; i++) {
		if (SlaveServers[i].Enable) {
			enabled++;
		}
	}
	return(enabled);
}