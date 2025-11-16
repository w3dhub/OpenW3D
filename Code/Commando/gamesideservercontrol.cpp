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
 *                     $Archive:: /Commando/Code/Commando/gamesideservercontrol.cpp           $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/19/02 4:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "gamesideservercontrol.h"
#include "registry.h"
#include "_globals.h"
#include "consolefunction.h"
#include "gamedata.h"
#include "servercontrol.h"
#include "gamemode.h"
#include "buildnum.h"
#include "slavemaster.h"
#include "ConsoleMode.h"

#include <string.h>
#include <stdio.h>


/*
** Static class data.
*/
bool GameSideServerControlClass::Listening = false;
StringClass GameSideServerControlClass::Response;


/***********************************************************************************************
 * GameSideServerControlClass::Init -- Start listening for server control messages             *
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
 *   11/16/2001 3:55PM ST : Created                                                            *
 *=============================================================================================*/
void GameSideServerControlClass::Init(void)
{
	char password[64] = DEFAULT_SERVER_CONTROL_PASSWORD;

	if (!Listening) {

		/*
		** Only for dedicated servers.
		*/
		if (SlaveMaster.Am_I_Slave() || (The_Game() && The_Game()->IsDedicated.Is_True()) || ConsoleBox.Is_Exclusive()) {

			/*
			** Get the port number from the registry.
			*/
			RegistryClass reg(APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL);
			int port = reg.Get_Int(SERVER_CONTROL_PORT_KEY, DEFAULT_SERVER_CONTROL_PORT);
			if (port != 0) {

				/*
				** See if we should only bind to the loopback address (for slaves).
				*/
				int loopback = reg.Get_Int(SERVER_CONTROL_LOOPBACK_KEY, 0);
				if (SlaveMaster.Am_I_Slave()) {
					ServerControl.Allow_Remote_Admin((loopback == 0) ? true : false);
				}

				/*
				** Get the bind IP from the registry.
				*/
				unsigned int ip = reg.Get_Int(SERVER_CONTROL_IP_KEY, 0);

				/*
				** Get the password from the registry.
				*/
				reg.Get_String(SERVER_CONTROL_PASSWORD_KEY, password, sizeof(password), password);

				/*
				** Start listening.
				*/
				ServerControl.Start_Listening(port, password, &App_Request_Callback, NULL, loopback ? true : false, ip);
				Listening = true;

				/*
				** Update the welcome message.
				*/
				Set_Welcome_Message();
			}
		} else {
			Listening = false;
		}
	}
}


/***********************************************************************************************
 * GameSideServerControlClass::Set_Welcome_Message -- Update the welcome message               *
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
 *   11/16/2001 3:56PM ST : Created                                                            *
 *=============================================================================================*/
void GameSideServerControlClass::Set_Welcome_Message(void)
{
	char buffer[1024];
	buffer[0] = 0;
	char welcome[1024];

	/*
	** Basic welcome message includes build info.
	*/
	sprintf(welcome, "Welcome to Renegade %s\n", BuildInfoClass::Composite_Build_Info());

	/*
	** Get the slave info and add it into the welcome message.
	*/
	SlaveMaster.Get_Slave_Info(buffer, sizeof(buffer));
	strcat(welcome, buffer);
	welcome[499] = 0;

	ServerControl.Set_Welcome_Message(welcome);
}



/***********************************************************************************************
 * GameSideServerControlClass::Shutdown -- Stop listening to server control messages           *
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
 *   11/16/2001 3:57PM ST : Created                                                            *
 *=============================================================================================*/
void GameSideServerControlClass::Shutdown(void)
{
	ServerControl.Stop_Listening();
	Listening = false;
}



/***********************************************************************************************
 * GameSideServerControlClass::App_Request_Callback -- Server control request handler          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Request text                                                                      *
 *                                                                                             *
 * OUTPUT:   Response text                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 3:58PM ST : Created                                                            *
 *=============================================================================================*/
const char *GameSideServerControlClass::App_Request_Callback(char *request)
{
	Response.Erase(0, Response.Get_Length());
	if (Listening) {
		if (GameModeManager::Find("Combat")->Is_Active()) {
			ConsoleFunctionManager::Parse_Input(request);
		}
	}
	Response += "\n";
	return(Response.Peek_Buffer());
}


/***********************************************************************************************
 * GameSideServerControlClass::Print -- Adds to the response packet for the current request    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Response text                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 3:58PM ST : Created                                                            *
 *=============================================================================================*/
void GameSideServerControlClass::Print(const char *text, ...)
{
	if (Listening) {
		if (Response.Get_Length() > 32768) {
			Response.Erase(0, Response.Get_Length());
		}

		char buffer[8192];

		va_list va;

		buffer[sizeof(buffer)-1] = 0;
		va_start(va, text);
		vsnprintf(&buffer[0], sizeof(buffer)-1, text, va);
		va_end(va);

		Response += buffer;
	}
}



/***********************************************************************************************
 * GameSideServerControlClass::Send_Message -- Send a request message to another server        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Message                                                                           *
 *           IP                                                                                *
 *           Port                                                                              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 3:59PM ST : Created                                                            *
 *=============================================================================================*/
void GameSideServerControlClass::Send_Message(const char *text, unsigned int ip, unsigned short port)
{
	if (Listening) {
		ServerControl.Send_Message(text, ip, port);
	}
}