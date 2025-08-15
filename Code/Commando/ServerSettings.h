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
 *                     $Archive:: /Commando/Code/Commando/ServerSettings.h                    $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/08/02 1:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <win.h>
#include "WWOnline/WOLSession.h"

class StringClass;


class ServerSettingsClass
{

	public:

		enum GameModeTypeEnum {
			MODE_NONE = 0,
			MODE_LAN,
			MODE_WOL,
			MODE_GAMESPY
		};

		static void Set_Settings_File_Name(char *filename);
		static bool Is_Server_Settings_File_Set(void) {return((SettingsFile[0] == 0) ? false : true);}
		static bool Parse(bool apply = false);
		static bool Is_Command_Line_Mode(void) {return(IsActive);}
		static void Encrypt_Serial(StringClass serial_in, StringClass &serial_out, bool encrypt = true);
		static void Decrypt_Serial(StringClass serial_in, StringClass &serial_out);
		static const char *Get_Master_Server_Password(void) {return(MasterPassword);}
		static unsigned long Get_Master_Bandwidth(void) {return(MasterBandwidth);}
		static bool Is_Active(void) {return(IsActive);}
		static char *Get_Settings_File_Name(void) {return(SettingsFile);}
		static bool Check_Game_Settings_File(char *config_file);
		static GameModeTypeEnum Get_Game_Mode(void) {return(GameMode);}
		static int Get_Disk_Log_Size(void) {return(DiskLogSize);}

		/*
		** Populating ini file with server list.
		*/
		static char *Get_Preferred_Server(const WWOnline::IRCServerList &server_list);
		static void Write_Server_List(const WWOnline::IRCServerList &server_listvoid);
	private:
		static char SettingsFile[MAX_PATH];
		static bool IsActive;

		static char MasterPassword[128];
		static unsigned long MasterBandwidth;
		static char PreferredLoginServer[256];
		static GameModeTypeEnum GameMode;
		static int DiskLogSize;

};




