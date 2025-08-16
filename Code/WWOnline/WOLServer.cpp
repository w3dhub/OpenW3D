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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/WWOnline/WOLServer.cpp $
*
* DESCRIPTION
*     These classes encapsulate a Westwood Online Server.
*
*     This is a base class. Derived classes include (but not necessarily limited to)
*     ChatServer, GameResultsServer, LadderServer, and WDTServer.
*
*     Server primarily repackages the WOL Server struct
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 1/12/02 9:42p $
*
******************************************************************************/

#include <stdlib.h>
#include "WOLServer.h"
#include "WOLProduct.h"
#include <Commando/_globals.h>
#include <string.h>
#include <wwdebug/WWDebug.h>
#include <wwlib/Registry.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     ServerData::ServerData
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Server - WOLAPI Server structure representing server.
*
* RESULT
*     NONE
*
******************************************************************************/

ServerData::ServerData(const WOL::Server& server)
	{
	memcpy(&mData, &server, sizeof(mData));
	mData.next = NULL;
	}


/******************************************************************************
*
* NAME
*     ServerData::~ServerData
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

ServerData::~ServerData()
	{
	WWDEBUG_SAY(("WOL: Destructing ServerData %s:%s\n", mData.connlabel, mData.name));
	}


/******************************************************************************
*
* NAME
*     IRCServerData::Create
*
* DESCRIPTION
*     Create a IRC server instance.
*
* INPUTS
*     Server - WOLAPI server structure
*
* RESULT
*     Instance of IRC server class.
*
******************************************************************************/

RefPtr<IRCServerData> IRCServerData::Create(const WOL::Server& server)
	{
	WWASSERT((strcmp((char*)server.connlabel, "IRC") == 0) || (strcmp((char*)server.connlabel, "IGS") == 0));
	return new IRCServerData(server);
	}


/******************************************************************************
*
* NAME
*     IRCServerData::IRCServerData
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Server - WOLAPI server structure
*
* RESULT
*     NONE
*
******************************************************************************/

IRCServerData::IRCServerData(const WOL::Server& server) :
		ServerData(server),
		mMatchesLanguageCode(false),
		mHasLanguageCode(false)
	{
	mIsGameServer = (strcmp((char*)server.connlabel, "IGS") == 0);

	RefPtr<Product> product = Product::Current();
	WWASSERT(product.IsValid() && "WWOnline product not initialized.");

	char* namePart = strchr((char*)server.name, ':');

	if (namePart)
		{
		namePart++;
		}

	char name[sizeof(server.name) + 1];
	strcpy(name, (char*)server.name);
	char* langPart = strtok(name, ":");

	if (namePart && langPart)
		{
		char* token = strtok(langPart, ",");

		if (token)
			{
			long productLangCode = product->GetLanguageCode();

			do
				{
				long langCode = atol(token);

				if (langCode == productLangCode)
					{
					mMatchesLanguageCode = true;
					}

				token = strtok(NULL, ",");
				} while (token);
			}
		}

	if (namePart)
		{
		mServerName = namePart;
		mHasLanguageCode = true;
		}
	else
		{
		mServerName = (char*)server.name;
		}
	}


/******************************************************************************
*
* NAME
*     HostPortServerData::HostPortServerData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

HostPortServerData::HostPortServerData(const WOL::Server& server) :
		ServerData(server)
	{
	char data[sizeof(server.conndata) + 1];
	strcpy(data, (char*)server.conndata);

	char* token = strtok(data, ";");
	WWASSERT(token);

	if (token)
		{
		token = strtok(NULL, ";");
		}

	WWASSERT(token);

	if (token)
		{
		mHostAddress = token;
		token = strtok(NULL, ";");
		}

	WWASSERT(token);

	if (token)
		{
		mHostPort = atol(token);
		}
	}


/******************************************************************************
*
* NAME
*     LadderServerData::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<LadderServerData> LadderServerData::Create(const WOL::Server& server)
	{
	WWASSERT(strcmp((char*)server.connlabel, "LAD") == 0);
	return new LadderServerData(server);
	}


/******************************************************************************
*
* NAME
*     LadderServerData::LadderServerData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

LadderServerData::LadderServerData(const WOL::Server& server) :
		HostPortServerData(server)
	{
	}


/******************************************************************************
*
* NAME
*     GameResultsServerData::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<GameResultsServerData> GameResultsServerData::Create(const WOL::Server& server)
	{
	WWASSERT(strcmp((char*)server.connlabel, "GAM") == 0);
	return new GameResultsServerData(server);
	}


/******************************************************************************
*
* NAME
*     GameResultsServerData::GameResultsServerData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

GameResultsServerData::GameResultsServerData(const WOL::Server& server) :
		HostPortServerData(server)
	{
	}


/******************************************************************************
*
* NAME
*     WDTServerData::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<WDTServerData> WDTServerData::Create(const WOL::Server& server)
	{
	WWASSERT(strcmp((char*)server.connlabel, "WDT") == 0);
	return new WDTServerData(server);
	}


/******************************************************************************
*
* NAME
*     WDTServerData::WDTServerData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

WDTServerData::WDTServerData(const WOL::Server& server) :
		HostPortServerData(server)
	{
	}


/******************************************************************************
*
* NAME
*     MGLServerData::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<MGLServerData> MGLServerData::Create(const WOL::Server& server)
	{
	WWASSERT(strcmp((char*)server.connlabel, "MGL") == 0);
	return new MGLServerData(server);
	}


/******************************************************************************
*
* NAME
*     MGLServerData::MGLServerData
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

MGLServerData::MGLServerData(const WOL::Server& server) :
		HostPortServerData(server)
	{
	WWDEBUG_SAY(("WOL: ManglerServer %s:%d\n", GetHostAddress(), GetPort()));
	}


RefPtr<PingServerData> PingServerData::Create(const WOL::Server& server)
	{
	WWASSERT(strcmp((char*)server.connlabel, "PNG") == 0);
	return new PingServerData(server);
	}


PingServerData::PingServerData(const WOL::Server& server) :
		HostPortServerData(server),
		mPingTime(-1)
	{
	WWDEBUG_SAY(("WOL: PingServer %s:%d\n", GetHostAddress(), GetPort()));
	}


void PingServerData::SetPingTime(int time)
	{
	mPingTime = time;

	// Save the ping time in the registry.
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_SERVER_LIST);
	reg.Set_Int(GetHostAddress(), time);
	}

}
