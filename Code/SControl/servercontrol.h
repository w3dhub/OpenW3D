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
 *                     $Archive:: /Commando/Code/SControl/servercontrol.h                     $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/18/02 6:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef SERVERCONTROL_H
#define SERVERCONTROL_H

#include	"assert.h"
#include "vector.h"
#include "servercontrolsocket.h"

#ifndef DebugString
#include	"wwdebug.h"
#ifdef WWDEBUG_SAY
#define DebugString WWDEBUG_SAY
#endif
#endif

#ifdef WWASSERT
#ifndef fw_assert
#define fw_assert WWASSERT
#endif //fw_assert
#else //WWASSERT
#define fw_assert assert
#endif //WWASSERT

/*
** Max size of packet this class can handle.
*/
#define MAX_SERVER_CONTROL_MESSAGE_SIZE 500

/*
** Time out non-responsive controllers after 1 minute.
*/
#define CONTROL_TIMEOUT		60*1000

/*
** This class is a simple 'out of band' authenticated server command/response system.
**
*/
class ServerControlClass
{
	public:

		/*
		** Constructor, destructor.
		*/
		ServerControlClass(void);
		~ServerControlClass(void);

		/*
		** Init, shutdown.
		*/
		bool Start_Listening(unsigned short port, char *password, const char*(*app_request_callback)(char*), void(*app_response_callback)(char*), bool loopback = false, unsigned long ip = 0);
		void Stop_Listening(void);
		void Set_Welcome_Message(char *message);

		void Allow_Remote_Admin(bool allow) {RemoteAdminAllowed = allow;}

		/*
		** Send/receive etc.
		*/
		void Send_Message(const char *text, unsigned long ip, unsigned short port);

		/*
		** Service.
		*/
		void Service(void);

	private:

		void Parse_Message(void *buffer, int len, unsigned long address, unsigned short port);
		void Add_Remote_Control(unsigned long ip, unsigned short port);
		void Remove_Remote_Control(unsigned long ip, unsigned short port);
		bool Is_Authenticated(unsigned long address, unsigned short port);
		void Reset_Timeout(unsigned long address, unsigned short port);
		void Respond(const char *message, unsigned long ip, unsigned short port);


		/*
		** Type of messages that can be sent.
		*/
		typedef enum tControlType {
			CONTROL_REQUEST,
			CONTROL_RESPONSE
		} ControlType;

		/*
		** Format of control message.
		*/
		typedef struct tControlMessageStruct {
			ControlType		Type;
			char				Message[MAX_SERVER_CONTROL_MESSAGE_SIZE];
		} ControlMessageStruct;

		/*
		** Instance of comms socket handler.
		*/
		ServerControlSocketClass Comms;

		/*
		** Port we are bound to.
		*/
		unsigned short LocalPort;

		/*
		** Are we listening for control messages?
		*/
		bool Listening;

		/*
		** Password.
		*/
		char Password[128];

		/*
		** Are we allowed to listen for control messages from a remote admin (i.e. not LOOPBACK address).
		*/
		bool RemoteAdminAllowed;

		/*
		** Welcome message sent to new controllers as the connect.
		*/
		char WelcomeMessage[1024];

		/*
		** Struct to hold info about remote controllers.
		*/
		typedef struct tRemoteControlStruct{
			unsigned short Port;
			unsigned long IP;
			bool Secure;
			unsigned long Time;
		} RemoteControlStruct;

		RemoteControlStruct *Get_Controller(unsigned long ip, unsigned short port);

		/*
		** List of remote controllers we know about.
		*/
		DynamicVectorClass<RemoteControlStruct*> RemoteControllers;

		/*
		** App callbacks for passing control messages.
		*/
		const char *(*AppRequestCallback)(char*);
		void (*AppResponseCallback)(char*);

};

/*
** Single instance of local controller.
*/
extern ServerControlClass ServerControl;


#endif //SERVERCONTROL_H