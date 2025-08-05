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
 *                     $Archive:: /Commando/Code/SControl/servercontrol.cpp                   $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/18/02 6:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	<stdlib.h>
#include	"servercontrol.h"
#include "systimer.h"

/*
** Single instance of server control.
*/
ServerControlClass ServerControl;


/***********************************************************************************************
 * ServerControlClass::ServerControlClass -- Class constructor                                 *
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
 *   11/16/2001 4:00PM ST : Created                                                            *
 *=============================================================================================*/
ServerControlClass::ServerControlClass(void)
{
	AppRequestCallback = NULL;
	AppResponseCallback = NULL;
	LocalPort = 63999;
	strcpy(Password, "");
	Listening = false;
	WelcomeMessage[0] = 0;
	RemoteAdminAllowed = false;
}


/***********************************************************************************************
 * ServerControlClass::ServerControlClass -- Class destructor                                  *
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
 *   11/16/2001 4:01PM ST : Created                                                            *
 *=============================================================================================*/
ServerControlClass::~ServerControlClass(void)
{
	Stop_Listening();
}



/***********************************************************************************************
 * ServerControlClass::Start_Listening -- Start listening for server control messages          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Port to listen on                                                                 *
 *           Password to authenticate controllers                                              *
 *           App request callback                                                              *
 *           App response callback                                                             *
 *           Loopback - if set then only bind to the loopback address                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:01PM ST : Created                                                            *
 *=============================================================================================*/
bool ServerControlClass::Start_Listening(unsigned short port, char *password, const char*(*app_request_callback)(char*), void(*app_response_callback)(char*), bool loopback, unsigned long ip)
{
	LocalPort = port;
	if (LocalPort != 0) {
		strcpy(Password, password);
		if (Comms.Open(LocalPort, loopback, ip)) {
			Listening = true;
			AppRequestCallback = app_request_callback;
			AppResponseCallback = app_response_callback;
			Comms.Set_Encryption_Key(password);
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * ServerControlClass::Stop_Listening -- Stop listening to control messages                    *
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
 *   11/16/2001 4:02PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Stop_Listening(void)
{
	if (Listening) {

		/*
		** Throw off any connected controllers.
		*/
		RemoteControlStruct *control;
		while (RemoteControllers.Count()) {
			control = RemoteControllers[0];
			if (control && control->Secure) {
				Respond("** Server exiting - Connection closed! **\n", control->IP, control->Port);
			}
			Remove_Remote_Control(control->IP, control->Port);
		}

		Comms.Close();
		Listening = false;
	}
	while (RemoteControllers.Count()) {
		delete RemoteControllers[0];
		RemoteControllers.Delete(0);
	}
}



/***********************************************************************************************
 * ServerControlClass::Service -- Service control messages                                     *
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
 *   11/16/2001 4:03PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Service(void)
{
	if (Listening) {

		char buffer[1024];
		int buffer_len = sizeof(buffer);
		unsigned long address;
		unsigned short port;

		Comms.Service();
		int bytes = Comms.Read(buffer, buffer_len, &address, &port);
		if (bytes > 0 && bytes <= sizeof(ControlMessageStruct)) {

			if (Is_Authenticated(address, port)) {
				Reset_Timeout(address, port);
			}

			Parse_Message(buffer, bytes, address, port);
		}


		/*
		** See if anyone timed out.
		*/
		RemoteControlStruct *control;
		for (int i=0 ; i<RemoteControllers.Count() ; i++) {
			control = RemoteControllers[i];
			if (control && control->Secure) {
				if (TIMEGETTIME() - control->Time > CONTROL_TIMEOUT) {
					Respond("** Connection timed out - Bye! **\n", control->IP, control->Port);
					Remove_Remote_Control(control->IP, control->Port);
					break;
				}
			}
		}
	}
}





/***********************************************************************************************
 * ServerControlClass::Parse_Message -- Parse server control message                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Buffer containing message struct                                                  *
 *           Length of message                                                                 *
 *           Address message came from                                                         *
 *           Port message came from                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:03PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Parse_Message(void *buffer, int len, unsigned long address, unsigned short port)
{
	ControlMessageStruct *message = (ControlMessageStruct*) buffer;
	assert(len <= sizeof(ControlMessageStruct));
	assert(message != NULL);

	/*
	** This line prevents external control by only accepting messages from the loopback address.
	*/
	if (ntohl(address) == INADDR_LOOPBACK || RemoteAdminAllowed) {

		/*
		** Convert to upper case for parsing.
		*/
		char text[MAX_SERVER_CONTROL_MESSAGE_SIZE + 1];
		memcpy(text, message->Message, MAX_SERVER_CONTROL_MESSAGE_SIZE);
		text[MAX_SERVER_CONTROL_MESSAGE_SIZE] = 0;
		strupr(text);

		switch (message->Type) {
			/*
			** This is a request from a remote controller.
			*/
			case CONTROL_REQUEST:
				if (strstr(text, "CONNECT")) {
					/*
					** Someone is requesting to talk to us.
					*/
					if (strlen(Password)) {
						Respond("Password required:", address, port);
					} else {
						Add_Remote_Control(address, port);
					}
					break;
				}

				if (strstr(text, "BYE")) {
					/*
					** Remote controller is going away.
					*/
					if (Is_Authenticated(address, port)) {
						Respond("Goodbye!\n", address, port);
						Remove_Remote_Control(address, port);
					}
					break;
				}

				/*
				** Check for password authentication.
				*/
				if (strlen(Password) && strstr(message->Message, Password)) {
					char buildbuf[256];
					strcpy(buildbuf, "Password accepted.\n");
					strcat(buildbuf, WelcomeMessage);
					strcat(buildbuf, "\n");
					Respond(buildbuf, address, port);
					Add_Remote_Control(address, port);
					break;
				}

				/*
				** If we know about this controller and we have got a valid password from him then pass his message to the game.
				*/
				if (AppRequestCallback && Is_Authenticated(address, port)) {
					const char * response = AppRequestCallback(message->Message);
					if (response && strlen(response)) {
						Respond(response, address, port);
					}
				}
				break;

			/*
			** This is a response to a control message we sent out.
			*/
			case CONTROL_RESPONSE:
				if (AppResponseCallback) {
					AppResponseCallback(message->Message);
				}
				break;
		}
	}
}





/***********************************************************************************************
 * ServerControlClass::Add_Remote_Control -- Add a new remote controller                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP of remote controller                                                           *
 *           Port of remote controller                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:06PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Add_Remote_Control(unsigned long ip, unsigned short port)
{
	RemoteControlStruct *control = Get_Controller(ip, port);

	if (control == NULL) {
		control = new RemoteControlStruct;
		RemoteControllers.Add(control);
	}

	control->Time = TIMEGETTIME();
	control->Secure = true;
	control->IP = ip;
	control->Port = port;
}



/***********************************************************************************************
 * ServerControlClass::Remove_Remote_Control -- Remove a remote controller from our list       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP of remote controller                                                           *
 *           Port of remote controller                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:07PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Remove_Remote_Control(unsigned long ip, unsigned short port)
{
	RemoteControlStruct *control;
	for (int i=0 ; i<RemoteControllers.Count() ; i++) {
		control = RemoteControllers[i];
		if (control->IP == ip && control->Port == port) {
			delete control;
			RemoteControllers.Delete(i);
			break;
		}
	}
}



/***********************************************************************************************
 * ServerControlClass::Is_Authenticated -- Has this controller presented a valid password?     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    IP, port                                                                          *
 *                                                                                             *
 * OUTPUT:   True if controller knows password                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:07PM ST : Created                                                            *
 *=============================================================================================*/
bool ServerControlClass::Is_Authenticated(unsigned long ip, unsigned short port)
{
	RemoteControlStruct *control = Get_Controller(ip, port);
	if (control && control->Secure && TIMEGETTIME() - control->Time < CONTROL_TIMEOUT) {
		return(true);
	}

	return(false);
}


/***********************************************************************************************
 * ServerControlClass::Get_Controller -- Find remote controller info based on ip and port      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ip, port                                                                          *
 *                                                                                             *
 * OUTPUT:   Ptr to remote control struct. Null if not found                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:08PM ST : Created                                                            *
 *=============================================================================================*/
ServerControlClass::RemoteControlStruct *ServerControlClass::Get_Controller(unsigned long ip, unsigned short port)
{
	RemoteControlStruct *control;
	for (int i=0 ; i<RemoteControllers.Count() ; i++) {
		control = RemoteControllers[i];
		if (control->IP == ip && control->Port == port) {
			return(control);
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * ServerControlClass::Reset_Timeout -- Reset idle timeout for remote controller               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ip, port                                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:09PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Reset_Timeout(unsigned long ip, unsigned short port)
{
	RemoteControlStruct *control = Get_Controller(ip, port);
	if (control && control->Secure && TIMEGETTIME() - control->Time < CONTROL_TIMEOUT) {
		control->Time = TIMEGETTIME();
	}
}



/***********************************************************************************************
 * ServerControlClass::Send_Message -- Send control message to remote server                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Message text                                                                      *
 *           IP, port to send to                                                               *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:09PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Send_Message(const char *text, unsigned long ip, unsigned short port)
{
	ControlMessageStruct message;
	message.Type = CONTROL_REQUEST;
	strcpy(message.Message, text);

	Comms.Write(&message, sizeof(message.Type) + strlen(text) + 1, &ip, port);
	Comms.Service();
}




/***********************************************************************************************
 * ServerControlClass::Respond -- Send a control request response to a remote controller       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Text of response                                                                  *
 *           IP to send to                                                                     *
 *           port to send to                                                                   *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:10PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Respond(const char *text, unsigned long ip, unsigned short port)
{

	ControlMessageStruct message;

	const char *outmsg = text;
	while (strlen(outmsg)) {
		message.Type = CONTROL_RESPONSE;
		strncpy(message.Message, outmsg, sizeof(message.Message)-1);
		message.Message[sizeof(message.Message)-1] = 0;
		int outlen = 0;

		if (strlen(outmsg) > sizeof(message.Message)-1) {
			outlen = sizeof(message.Message)-1;
			outmsg += (sizeof(message.Message)-1);
		} else {
			outlen = strlen(outmsg);
			outmsg += outlen;
		}


		Comms.Write(&message, sizeof(message.Type) + outlen + 1, &ip, port);
		Comms.Service();
	}
}



/***********************************************************************************************
 * ServerControlClass::Set_Welcome_Message -- Set the new connection welcome message           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Text of message                                                                   *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/2001 4:11PM ST : Created                                                            *
 *=============================================================================================*/
void ServerControlClass::Set_Welcome_Message(char *message)
{
	if (message) {
		memset(WelcomeMessage, 0, sizeof(WelcomeMessage));
		strncpy(WelcomeMessage, message, sizeof(WelcomeMessage)-1);
	}
}