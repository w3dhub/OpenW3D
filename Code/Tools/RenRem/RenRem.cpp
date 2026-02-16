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
 *                 Project Name : Renegade remote server control utility                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/RenRem/RenRem.cpp                      $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/23/02 11:06a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "network-typedefs.h"
#include <assert.h>
#include <stdio.h>
#include <limits>
#include <stdlib.h>
#include <conio.h>

#include "servercontrol.h"

#include <timeapi.h>

#define RENREM_PORT 1234
#define PROMPT ">"

char CommandBuffer[16384];
int BufferPos = 0;
char RequestBuffer[512];
bool GotResponse = false;
bool Connected = false;
bool DumpOutput = false;
bool TruncateFile = true;
unsigned int ResponseTime = 0;



const char *App_Request_Callback(char *request)
{
	cprintf(request);
	return("");
}

void App_Response_Callback(char *response)
{
	cprintf(response);
	if (RequestBuffer[0] == 0) {
		cprintf(PROMPT);
	}
	BufferPos = 0;
	GotResponse = true;
	ResponseTime = timeGetTime();

	if (DumpOutput) {
		HANDLE file = INVALID_HANDLE_VALUE;
		if (TruncateFile) {
			file = CreateFileA("RenRem.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		} else {
			file = CreateFileA("RenRem.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		TruncateFile = false;

		if (file != INVALID_HANDLE_VALUE) {
			SetFilePointer(file, 0, NULL, FILE_END);
			DWORD actual = 0;
			const size_t response_length = ::strlen(response);
			assert(response_length <= std::numeric_limits<DWORD>::max());
			WriteFile(file, response, static_cast<DWORD>(response_length), &actual, NULL);
			CloseHandle(file);
		}
	}
}



int main(int argc, char **argv)
{
	cprintf("Renegade Remote Server Management Tool\n");
	cprintf("Version 1.11 - ST 7/25/2002 3:20PM\n");
	cprintf("Usage:   RenRem.exe <options> ip port password [local_port]\n\n");
	cprintf("Options: -r=Request  - Send the specified request to the server then quit\n\n");

	if (argc < 2) {
		cprintf("Bad parameters\n");
		return(0);
	}

	int arg_offset = 0;
	unsigned int quit_after_time = 0;
	if ((strnicmp(argv[1], "-R=", 3) == 0) || (strnicmp(argv[1], "/R=", 3) == 0)) {
		strcpy(RequestBuffer, argv[1]);
		arg_offset++;
		quit_after_time = timeGetTime() + 3000;
	} else {
		RequestBuffer[0] = 0;
	}

	/*
	** Must have at least 4 args.
	*/
	if (argc < 4 + arg_offset) {
		cprintf("Bad parameters\n");
		return(0);
	}

	/*
	** Get the IP.
	*/
	unsigned int ip = inet_addr(argv[1+ arg_offset]);
	unsigned int port = atoi(argv[2 + arg_offset]);
	char *password = argv[3 + arg_offset];
	unsigned int local_port = RENREM_PORT;

	if (argc > (4 + arg_offset)) {
		local_port = atoi(argv[4 + arg_offset]);
	}

	if ((port & 0xffff0000) != 0 || port < 1024) {
		cprintf("Invalid port number %d\n", port);
		return(0);
	}

	if ((local_port & 0xffff0000) != 0 || port < 1024) {
		cprintf("Invalid local port number %d\n", port);
		return(0);
	}

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(1,1), &wsa_data) != 0) {
		cprintf("WSAStartup failed: error code %d\n", GetLastError());
		return(0);
	}

	ServerControl.Allow_Remote_Admin(true);
	if (ServerControl.Start_Listening(local_port, password, &App_Request_Callback, &App_Response_Callback) == false) {
		cprintf("Failed to open socket for port %d\n", local_port);
		WSACleanup();
		return(0);
	}

	cprintf("RenRem online using port %d\n", local_port);
	cprintf("Entering console mode\n");
	cprintf("Type the remote management password to connect\n");
	cprintf("Press escape to quit\n");

	bool doit = true;
	int buffer_pos = 1;
	sprintf(CommandBuffer, PROMPT);

	cprintf(PROMPT);

	/*
	** If a initial request is to be made then connect automatically.
	*/
	if (RequestBuffer[0] != 0) {
		Connected = false;
		GotResponse = false;
		ServerControl.Send_Message(password, ip, port);
	}


	while (doit) {
		Sleep(1);
		ServerControl.Service();
		if (_kbhit()) {
			int input = _getche();

			if (input == 27) {
				cprintf("\nQuitting\n");
				break;
			}

			switch (input) {

				/*
				** TAB key used to get command line suggestions in the game. Here we just print spaces, so jump back to the start
				** of the line so it at least matches what's in the buffer.
				*/
				case 9:
					buffer_pos = 1;
					cprintf("\r\n");
					cprintf(PROMPT);
					break;

				case 13:
					if (buffer_pos > 1) {
						CommandBuffer[buffer_pos] = 0;

						/*
						** Verify quit command.
						*/
						bool send = true;
						if (stricmp(&CommandBuffer[1], "quit") == 0) {
							cprintf("\r\n Press 'Y' to shutdown server...");
							int key = _getch();
							if ((key & 0xff) != 'Y' && (key & 0xff) != 'y') {
								send = false;
							}
						}

						if (send) {
							ServerControl.Send_Message(&CommandBuffer[1], ip, port);
							TruncateFile = true;
						}
					}
					buffer_pos = 1;
					cprintf("\r\n");
					cprintf(PROMPT);
					ServerControl.Service();
					break;

				/*
				** Backspace.
				*/
				case 8:
					if (buffer_pos > 1) {
						buffer_pos--;
						cprintf(" \b");
					} else {
						/*
						** Compensate for backspace going too far.
						*/
						if (buffer_pos == 1) {
							cprintf(PROMPT);
						}
					}
					break;


				default:
					if (input == 32 || isgraph(input)) {
						CommandBuffer[buffer_pos++] = (input & 0xff);
					}
					break;

			}
		}

		if (RequestBuffer[0] != 0 && GotResponse) {
			if (!Connected) {
				/*
				** Must be connection response - send specified request now.
				*/
				Connected = true;
				GotResponse = false;
				DumpOutput = true;
				ServerControl.Send_Message(&RequestBuffer[3], ip, port);
			} else {
				/*
				** Must be request response - we are done.
				*/
				if (quit_after_time && ResponseTime && timeGetTime() - ResponseTime > 600) {
					break;
				}
			}
		}
		if (quit_after_time && (timeGetTime() > quit_after_time)) {
			break;
		}
	}

	WSACleanup();
	return(0);
}
