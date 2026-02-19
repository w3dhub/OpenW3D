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
 *                     $Archive:: /Commando/Code/Commando/slavemaster.h                       $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 2:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ProcessManager.h"

#define MAX_SLAVES	7

class SlaveMasterClass;

/*
**
**  One of these classes for each slave server.
**
*/
class SlaveServerClass
{
	friend SlaveMasterClass;

	public:

		SlaveServerClass(void);
		~SlaveServerClass(void);

		void Set(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password);
		void Get(bool &enable, char *nick, char *serial, unsigned short &port, char *settings_file, int &bandwidth, char *password);


	private:

		char	NickName[32];
		char	Serial[64];
		char	Password[64];
		unsigned short Port;
		char	SettingsFileName[MAX_PATH];

		bool	Enable;
		bool	IsRunning;
		unsigned short ControlPort;
		int	Bandwidth;

		Process *ProcessInfo;
};







/*
**
**  Master server uses this class to manage slave servers.
**
**
*/
class SlaveMasterClass
{
	public:

		SlaveMasterClass(void);
		~SlaveMasterClass(void);

		void Startup_Slaves(void);
		void Shutdown_Slaves(void);
		bool Shutdown_Slave(char *slave_login);

		char *Get_Slave_Info(char *buffer, int buflen);

		void Load(void);
		void Save(void);
		void Reset(void);

		int Get_Num_Slaves(void) {return(NumSlaveServers);}
		int Get_Num_Enabled_Slaves(void);
		void Add_Slave(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password);
		SlaveServerClass *Get_Slave(int index);

		void Set_Slave_Mode(bool mode) {SlaveMode = mode;}
		bool Am_I_Slave(void) {return(SlaveMode);}


	private:

		void Delete_Registry_Copies(void);
		void Create_Registry_Copies(void);
		void Wait_For_Slave_Shutdown(void);

		SlaveServerClass SlaveServers[MAX_SLAVES];
		int NumSlaveServers;

		bool SlaveMode;	// false = master, true = slave

};

extern SlaveMasterClass SlaveMaster;
