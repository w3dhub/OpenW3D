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

#ifdef WWDEBUG
/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/WOLDiags.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/11/02 5:43p $
*
******************************************************************************/

#include "WOLDiags.h"
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLServer.h>

using namespace WWOnline;

void ShowWOLVersion(const RefPtr<Session>& session)
	{
	unsigned long ver = 0;
	session->GetChatObject()->GetVersion(&ver);
	ConsoleFunctionClass::Print("WOLAPI V%u.%u\n", (ver >> 16), (ver & 0xFFFF));
	}


void ShowServer(const RefPtr<Session>& session)
	{
	const RefPtr<IRCServerData>& server = session->GetCurrentServer();

	if (server.IsValid())
		{
		WOL::Server& data = server->GetData();
		ConsoleFunctionClass::Print("Server: %s:%s  TimeZone: %d  Position:: %f/%f\n",
			(const char*)data.connlabel, (const char*)data.name, data.timezone, data.longitude, data.lattitude);
		}
	else
		{
		ConsoleFunctionClass::Print("WOL not connected\n");
		}
	}


void ShowTopic(const RefPtr<Session>& session)
	{
	ConsoleFunctionClass::Print("Channel Topic: %s\n", session->GetChannelTopic());
	}


void ShowPingServers(const RefPtr<Session>& session)
	{
	const PingServerList& pingServers = session->GetPingServerList();
	unsigned int count = pingServers.size();

	for (unsigned int index = 0; index < count; index++)
		{
		const RefPtr<PingServerData>& ping = pingServers[index];
		ConsoleFunctionClass::Print("%-18s %-4dms - %s\n",
			ping->GetHostAddress(), ping->GetPingTime(), ping->GetName());
		}
	}


struct Dispatch
	{
	const char* Cmd;
	void (*Func)(const RefPtr<Session>&);
	};


const char* WOLConsoleFunctionClass::Get_Help(void)
	{
	return ("WOL <command> - Show WOL information");
	}

void WOLConsoleFunctionClass::Activate(const char* input)
	{
	static Dispatch _dispatch[] = 
		{
		{"pings", ShowPingServers},
		{"server", ShowServer},
		{"topic", ShowTopic},
		{"ver", ShowWOLVersion},
		{NULL, NULL}
		};

	RefPtr<Session> session = Session::GetInstance(false);

	if (!session.IsValid())
		{
		ConsoleFunctionClass::Print("Westwood Online not active\n");
		return;
		}

	int index = 0;
	const char* cmd = _dispatch[0].Cmd;

	while (cmd != NULL)
		{
		if (stricmp(cmd, input) == 0)
			{
			_dispatch[index].Func(session);
			return;
			}

		++index;
		cmd = _dispatch[index].Cmd;
		}
	}

#endif // WWDEBUG
