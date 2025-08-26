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
 *                     $Archive:: /Commando/Code/Commando/gamesideservercontrol.h             $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/18/02 5:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#ifndef _GAMESIDESERVERCONTROL_H
#define _GAMESIDESERVERCONTROL_H

#include <wwlib/wwstring.h>


#define SERVER_CONTROL_PORT_KEY		"ControlPort"
#define SERVER_CONTROL_PASSWORD_KEY	"ControlPassword"
#define SERVER_CONTROL_LOOPBACK_KEY	"ControlLoopbackOnly"
#define SERVER_CONTROL_IP_KEY			"ControlIP"

#define DEFAULT_SERVER_CONTROL_PORT 		63999
#define DEFAULT_SERVER_CONTROL_PASSWORD	"not_a_valid_password"	// Password removed per Security review requirements. LFeenanEA - 27th January 2025


/*
** This is the game side interface to the server control lib.
**
**
*/
class GameSideServerControlClass
{

	public:
		static void Init(void);
		static void Shutdown(void);
		static const char *App_Request_Callback(char *request);
		static void Print(const char *text, ...);
		static void Send_Message(const char *text, unsigned long ip, unsigned short port);
		static void Set_Welcome_Message(void);

	private:
		static bool Listening;
		static StringClass Response;

};





#endif //_GAMESIDESERVERCONTROL_H