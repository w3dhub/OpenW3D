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
*     $Archive: /Commando/Code/WWOnline/WOLConnect.cpp $
*
* DESCRIPTION
*     Westwood Online wait conditions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 35 $
*     $Modtime: 1/29/02 1:47p $
*
******************************************************************************/

#include "WOLConnect.h"
#include "WOLProduct.h"
#include "WOLServer.h"
#include "WOLChannel.h"
#include "WOLErrorUtil.h"
#include "WOLString.h"
#include <wwdebug/wwdebug.h>

namespace WOL
{
#include <wolapi/chatdefs.h>
}

namespace WWOnline {

/******************************************************************************
*
* NAME
*     ServerListWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Wait - Wait condition to process.
*
******************************************************************************/

RefPtr<ServerListWait> ServerListWait::Create(const RefPtr<Session>& session)
	{
	WWASSERT(session.IsValid() && "ServerListWait");

	if (!session.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: WWOnline session not instantiated\n"));
		return NULL;
		}

	return new ServerListWait(session);
	}


/******************************************************************************
*
* NAME
*     ServerListWait::ServerListWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

ServerListWait::ServerListWait(const RefPtr<Session>& session) :
		SingleWait(WOLSTRING("WOL_SERVERLISTFETCH")),
		mSession(session)
	{
	WWDEBUG_SAY(("WOL: ServerListWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     ServerListWait::~ServerListWait
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

ServerListWait::~ServerListWait()
	{
	WWDEBUG_SAY(("WOL: ServerListWait End '%S'\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     ServerListWait::WaitBeginning
*
* DESCRIPTION
*     Start server list wait condition
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ServerListWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: ServerListWait Begin\n"));
	WWASSERT(mSession.IsValid() && "ServerListWait");

	RefPtrConst<Product> product(Product::Current());
	WWASSERT(product.IsValid() && "ServerListWait");

	if (!product.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: WWOnline product not instantiated\n"));
		EndWait(WaitCondition::Error, WOLSTRING("WOL_SERVERLISTERROR"));
		return;
		}

	mSession->AddObserver(*static_cast< Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast< Observer<IRCServerList> *>(this));

	bool success = mSession->RequestServerList();

	if (!success)
		{
		WWDEBUG_SAY(("WOLERROR: ServerListWait request failed\n"));
		EndWait(Error, WOLSTRING("WOL_SERVERLISTERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     ServerListWait::HandleNotification(ServerError)
*
* DESCRIPTION
*     General error notificaiton
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ServerListWait::HandleNotification(ServerError& serverError)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: ServerListWait '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     ServerListWait::HandleNotification(IRCServerList)
*
* DESCRIPTION
*     Server list recieved notification.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ServerListWait::HandleNotification(IRCServerList&)
	{
	if (mEndResult == Waiting)
		{
		WWDEBUG_SAY(("WOL: ServerListWait successful\n"));
		EndWait(ConditionMet, WOLSTRING("WOL_SERVERLISTRECEIVED"));
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     DisconnectWait::Create
*
* DESCRIPTION
*     Create a disconnect wait condition.
*
* INPUTS
*     Session - WWOnline session to initiate disconnect from.
*
* RESULT
*     Wait - Wait condition to process.
*
******************************************************************************/

RefPtr<DisconnectWait> DisconnectWait::Create(const RefPtr<Session>& session)
	{
	WWASSERT(session.IsValid() && "DisconnectWait");

	if (!session.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: WWOnline session not instantiated\n"));
		return NULL;
		}

	return new DisconnectWait(session);
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::DisconnectWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

DisconnectWait::DisconnectWait(const RefPtr<Session>& session) :
		SingleWait(WOLSTRING("WOL_DISCONNECTING")),
		mSession(session)
	{
	WWDEBUG_SAY(("WOL: DisconnectWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::~DisconnectWait
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

DisconnectWait::~DisconnectWait()
	{
	WWDEBUG_SAY(("WOL: DisconnectWait End '%S'\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::WaitBeginning
*
* DESCRIPTION
*     Start disconnect wait condition
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DisconnectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: DisconnectWait Begin\n"));
	WWASSERT(mSession.IsValid() && "DisconnectWait");

	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));

	WWASSERT((mSession->GetChatObject() != NULL) && "DisconnectWait");
	HRESULT hr = mSession->GetChatObject()->RequestLogout();

	if (SUCCEEDED(hr))
		{
		mSession->mCurrentConnectionStatus = ConnectionDisconnecting;
		}
	else
		{
		WWDEBUG_SAY(("WOLERROR: DisconnectWait request failed\n"));
		EndWait(Error, WOLSTRING("WOL_DISCONNECTERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::EndWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DisconnectWait::EndWait(WaitResult result, const unichar_t* msg)
	{
	SingleWait::EndWait(result, msg);

	if (ConditionMet != result)
		{
		mSession->mCurrentConnectionStatus = ConnectionDisconnected;
		}
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::HandleNotification
*
* DESCRIPTION
*     General server error notification
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DisconnectWait::HandleNotification(ServerError& serverError)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: DisconnectWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DisconnectWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		if (status == ConnectionDisconnected)
			{
			WWDEBUG_SAY(("WOL: DisconnectWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_DISCONNECTED"));
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: DisconnectWait did not receive disconnect\n"));
			EndWait(Error, WOLSTRING("WOL_DISCONNECTERROR"));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     ConnectWait::Create
*
* DESCRIPTION
*     Create a server connect wait condition.
*
* INPUTS
*     Session - WWOnline session
*     Server  - Server to connect to.
*     Login   - Login to connect with.
*
* RESULT
*     Wait - Wait condition to process.
*
******************************************************************************/

RefPtr<ConnectWait> ConnectWait::Create(const RefPtr<Session>& session,
		const RefPtr<ServerData>& server, const RefPtr<LoginInfo>& login)
	{
	WWASSERT(session.IsValid() && "ConnectWait");
	WWASSERT(server.IsValid() && "ConnectWait");
	WWASSERT(login.IsValid() && "ConnectWait");

	if (!session.IsValid() || !server.IsValid() || !login.IsValid())
		{
		return NULL;
		}

	return new ConnectWait(session, server, login);
	}


/******************************************************************************
*
* NAME
*     ConnectWait::ConnectWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

ConnectWait::ConnectWait(const RefPtr<Session>& session, const RefPtr<ServerData>& server,
			const RefPtr<LoginInfo>& login) :
		SingleWait(WOLSTRING("WOL_CONNECTING")),
		mSession(session),
		mServer(server),
		mLogin(login)
	{
	WWDEBUG_SAY(("WOL: ConnectWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     ConnectWait::~ConnectWait
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

ConnectWait::~ConnectWait()
	{
	WWDEBUG_SAY(("WOL: ConnectWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     ConnectWait::WaitBeginning
*
* DESCRIPTION
*     Start connection wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ConnectWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: ConnectWait Begin\n"));
	WWASSERT(mSession.IsValid() && "ConnectWait");

	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));

	WWASSERT(mServer.IsValid() && "ConnectWait");
	WOL::Server& server = mServer->GetData();

	WWASSERT(mLogin.IsValid() && "ConnectWait");
	u_wstomb((char*)server.login, (const unichar_t*)mLogin->GetNickname(), sizeof(server.login));
	u_wstomb((char*)server.password, (const unichar_t*)mLogin->GetPassword(), sizeof(server.password));

	WWASSERT((mSession->GetChatObject() != NULL) && "ConnectWait");
	HRESULT hr = mSession->GetChatObject()->RequestConnection(&server, 20, !mLogin->IsPasswordEncrypted());

	if (SUCCEEDED(hr))
		{
		mSession->mCurrentConnectionStatus = ConnectionConnecting;
		}
	else
		{
		WWDEBUG_SAY(("WOLERROR: ConnectWait request failed '%s'\n", GetChatErrorString(hr)));
		EndWait(Error, WOLSTRING("WOL_CONNECTERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     DisconnectWait::EndWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ConnectWait::EndWait(WaitResult result, const unichar_t* msg)
	{
	SingleWait::EndWait(result, msg);

	if (ConditionMet != result)
		{
		mSession->mCurrentConnectionStatus = ConnectionDisconnected;
		}
	}


/******************************************************************************
*
* NAME
*     ConnectWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ConnectWait::HandleNotification(ServerError& serverError)
	{
	if (mEndResult == Waiting)
		{
		mSession->mCurrentConnectionStatus = ConnectionDisconnected;

		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: ConnectWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     ConnectWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ConnectWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		if (status == ConnectionConnected)
			{
			WWDEBUG_SAY(("WOL: ConnectWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_CONNECTED"));
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: ConnectWait did not receive connect\n"));
			EndWait(Error, WOLSTRING("WOL_CONNECTERROR"));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     ChannelListWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<ChannelListWait> ChannelListWait::Create(const RefPtr<Session>& session, int type)
	{
	WWASSERT(session.IsValid() && "ChannelListWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new ChannelListWait(session, type);
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::ChannelListWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

ChannelListWait::ChannelListWait(const RefPtr<Session>& session, int type) :
		SingleWait(WOLSTRING("WOL_CHANNELLISTFETCH")),
		mSession(session),
		mType(type)
	{
	WWDEBUG_SAY(("WOL: ChannelListWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::~ChannelListWait
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

ChannelListWait::~ChannelListWait()
	{
	WWDEBUG_SAY(("WOL: ChannelListWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::WaitBeginning
*
* DESCRIPTION
*     Start channel list request wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelListWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: ChannelListWait Begin\n"));
	WWASSERT(mSession.IsValid() && "ChannelListWait");

	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));
	mSession->AddObserver(*static_cast<Observer<ChannelListEvent> *>(this));

	bool success = mSession->RequestChannelList(mType, false);

	if (!success)
		{
		WWDEBUG_SAY(("WOLERROR: ChannelListWait request failed\n"));
		EndWait(Error, WOLSTRING("WOL_CHANNELLISTERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelListWait::HandleNotification(ServerError& serverError)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: ChannelListWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelListWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		if (status != ConnectionConnected)
			{
			WWDEBUG_SAY(("WOLERROR: ChannelListWait not connected to server\n"));
			EndWait(Error, WOLSTRING("WOL_NOTCONNECTED"));
			}
		}
	}


/******************************************************************************
*
* NAME
*     ChannelListWait::HandleNotification(ChannelListEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void ChannelListWait::HandleNotification(ChannelListEvent& event)
	{
	if (mEndResult == Waiting)
		{
		if ((event.GetEvent() == ChannelListEvent::NewList) && (event.GetChannelType() == mType))
			{
			WWDEBUG_SAY(("WOL: ChannelListWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_CHANNELLISTRECEIVED"));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     LeaveChannelWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<LeaveChannelWait> LeaveChannelWait::Create(const RefPtr<Session>& session)
	{
	WWASSERT(session.IsValid() && "LeaveChannelWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new LeaveChannelWait(session);
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::LeaveChannelWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

LeaveChannelWait::LeaveChannelWait(const RefPtr<Session>& session) :
		SingleWait(WOLSTRING("WOL_CHANNELLEAVE")),
		mSession(session)
	{
	WWDEBUG_SAY(("WOL: LeaveChannelWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::~LeaveChannelWait
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

LeaveChannelWait::~LeaveChannelWait()
	{
	WWDEBUG_SAY(("WOL: LeaveChannelWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::WaitBeginning
*
* DESCRIPTION
*     Start leave channel wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void LeaveChannelWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: LeaveChannelWait Begin\n"));
	WWASSERT(mSession.IsValid() && "LeaveChannelWait");

	ChannelStatus status = mSession->GetChannelStatus();

	// If we are not joined to a channel and not already leaving
	// a channel then we have successfully left a channel.
	if (ChannelJoined != status && ChannelLeaving != status)
		{
		EndWait(ConditionMet, WOLSTRING("WOL_CHANNELLEFT"));
		return;
		}

	// Listen for various events
	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));
	mSession->AddObserver(*static_cast<Observer<ChannelEvent> *>(this));

	// If we are currently joined the request to leave.
	if (ChannelJoined == status)
		{
		// Make the request to leave the channel
		bool success = mSession->RequestLeaveChannel();

		if (!success)
			{
			EndWait(Error, WOLSTRING("WOL_CHANNELLEAVEERROR"));
			}
		}
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LeaveChannelWait::HandleNotification(ServerError& serverError)
	{
	// If we get a server error while leaving a channel then something is wrong.
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: LeaveChannelWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LeaveChannelWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		if (status != ConnectionConnected)
			{
			WWDEBUG_SAY(("WOLWARNING: LeaveChannelWait not connected to server\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_NOTCONNECTED"));
			}
		}
	}


/******************************************************************************
*
* NAME
*     LeaveChannelWait::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LeaveChannelWait::HandleNotification(ChannelEvent& event)
	{
	if (mEndResult == Waiting)
		{
		// If we left the channel then the condition is met.
		// Otherwise something went wrong.
		if (event.GetStatus() == ChannelLeft)
			{
			WWDEBUG_SAY(("WOL: LeaveChannelWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_CHANNELLEFT"));
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: LeaveChannelWait channel not left\n"));
			EndWait(Error, WOLSTRING("WOL_CHANNELLEAVEERROR"));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     JoinChannelWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<JoinChannelWait> JoinChannelWait::Create(const RefPtr<Session>& session,
		const RefPtr<ChannelData>& channel, const unichar_t* password)
	{
	WWASSERT(session.IsValid() && "JoinChannelWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new JoinChannelWait(session, channel, password);
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<JoinChannelWait> JoinChannelWait::Create(const RefPtr<Session>& session,
		const unichar_t* channelName, const unichar_t* password, int type)
	{
	WWASSERT(session.IsValid() && "JoinChannelWait");
	WWASSERT(channelName && "JoinChannelWait");

	if (!session.IsValid() || (channelName == NULL))
		{
		return NULL;
		}

	return new JoinChannelWait(session, channelName, password, type);
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::JoinChannelWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

JoinChannelWait::JoinChannelWait(const RefPtr<Session>& session,
			const RefPtr<ChannelData>& channel, const unichar_t* password) :
		SingleWait(WOLSTRING("WOL_CHANNELJOIN")),
		mSession(session),
		mChannel(channel),
		mPassword(password),
		mType(0)
	{
	WWDEBUG_SAY(("WOL: JoinChannelWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::JoinChannelWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

JoinChannelWait::JoinChannelWait(const RefPtr<Session>& session,
			const unichar_t* channelName, const unichar_t* password, int type) :
		SingleWait(WOLSTRING("WOL_CHANNELJOIN")),
		mSession(session),
		mChannelName(channelName),
		mPassword(password),
		mType(type)
	{
	WWDEBUG_SAY(("WOL: JoinChannelWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::~JoinChannelWait
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

JoinChannelWait::~JoinChannelWait()
	{
	WWDEBUG_SAY(("WOL: JoinChannelWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::WaitCondition
*
* DESCRIPTION
*     Start channel join wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void JoinChannelWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: JoinChannelWait Begin\n"));
	WWASSERT(mSession.IsValid() && "JoinChannelWait");

	// If a channel is not specified then search for the named channel
	if (!mChannel.IsValid())
		{
		mChannel = mSession->FindChannel(mChannelName);

		// If the channel cannot be found then create one
		if (!mChannel.IsValid())
			{
			mChannel = ChannelData::Create(mChannelName, mPassword, mType);
			}
		}

	if (!mChannel.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: JoinChanneWait Channel not found\n"));
		EndWait(Error, WOLSTRING("WOL_CHANNELNOTFOUND"));
		return;
		}

	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));
	mSession->AddObserver(*static_cast<Observer<ChannelEvent> *>(this));

	bool success = mSession->RequestChannelJoin(mChannel, mPassword);

	if (!success)
		{
		WWDEBUG_SAY(("WOLERROR: JoinChanneWait request failed\n"));
		EndWait(Error, WOLSTRING("WOL_CHANNELJOINERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void JoinChannelWait::HandleNotification(ServerError& serverError)
	{
	// Join failed if we get a server error.
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: JoinChannelWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void JoinChannelWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		// Join failed if the connection is not present or lost.
		if (status != ConnectionConnected)
			{
			WWDEBUG_SAY(("WOLERROR: JoinChanneWait not connected to server\n"));
			EndWait(Error, WOLSTRING("WOL_NOTCONNECTED"));
			}
		}
	}


/******************************************************************************
*
* NAME
*     JoinChannelWait::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void JoinChannelWait::HandleNotification(ChannelEvent& event)
	{
	if (mEndResult == Waiting)
		{
		// If we joined the channel the condition is met
		if (ChannelJoined == event.GetStatus())
			{
			WWDEBUG_SAY(("WOL: JoinChanneWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_CHANNELJOINED"));
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: JoinChanneWait failed\n"));
			EndWait(Error, GetChannelStatusDescription(event.GetStatus()));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     CreateChannelWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<CreateChannelWait> CreateChannelWait::Create(const RefPtr<Session>& session,
		const RefPtr<ChannelData>& channel, const unichar_t* password)
	{
	WWASSERT(session.IsValid() && "CreateChannelWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new CreateChannelWait(session, channel, password);
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::CreateChannelWait
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

CreateChannelWait::CreateChannelWait(const RefPtr<Session>& session,
			const RefPtr<ChannelData>& channel, const unichar_t* password) :
		SingleWait(WOLSTRING("WOL_CHANNELCREATE")),
		mSession(session),
		mChannel(channel),
		mPassword(password)
	{
	WWDEBUG_SAY(("WOL: CreateChannelWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::~CreateChannelWait
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

CreateChannelWait::~CreateChannelWait()
	{
	WWDEBUG_SAY(("WOL: CreateChannelWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::WaitBeginning
*
* DESCRIPTION
*     Start channel creation wait condition
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void CreateChannelWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: CreateChannelWait Begin\n"));
	WWASSERT(mSession.IsValid() && "CreateChannelWait");

	mSession->AddObserver(*static_cast<Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast<Observer<ConnectionStatus> *>(this));
	mSession->AddObserver(*static_cast<Observer<ChannelEvent> *>(this));

	WWASSERT(mChannel.IsValid() && "CreateChannelWait");
	WOL::Channel& channel = mChannel->GetData();
	u_wstomb((char*)channel.key, (const unichar_t*)mPassword, sizeof(channel.key));

	WWASSERT((mSession->GetChatObject() != NULL) && "CreateChannelWait");
	HRESULT hr = mSession->GetChatObject()->RequestChannelCreate(&channel);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: CreateChanneWait request failed '%s'\n", GetChatErrorString(hr)));
		EndWait(Error, WOLSTRING("WOL_CHANNELCREATEERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void CreateChannelWait::HandleNotification(ServerError& serverError)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = serverError.GetDescription();
		WWDEBUG_SAY(("WOLERROR: CreateChannelWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void CreateChannelWait::HandleNotification(ConnectionStatus& status)
	{
	if (mEndResult == Waiting)
		{
		if (status != ConnectionConnected)
			{
			WWDEBUG_SAY(("WOLERROR: CreateChannelWait not connected to server\n"));
			EndWait(Error, WOLSTRING("WOL_NOTCONNECTED"));
			}
		}
	}


/******************************************************************************
*
* NAME
*     CreateChannelWait::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void CreateChannelWait::HandleNotification(ChannelEvent& event)
	{
	if (mEndResult == Waiting)
		{
		ChannelStatus status = event.GetStatus();

		if (ChannelCreated == status)
			{
			WWDEBUG_SAY(("WOL: CreateChannelWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_CHANNELJOINED"));
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: CreateChannelWait failed\n"));
			EndWait(Error, GetChannelStatusDescription(status));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     UserListWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<UserListWait> UserListWait::Create(const RefPtr<Session>& session)
	{
	WWASSERT(session.IsValid() && "UserListWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new UserListWait(session);
	}


/******************************************************************************
*
* NAME
*     UserListWait::WaitBeginning
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void UserListWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: UserListWait Begin\n"));
	WWASSERT(mSession.IsValid() && "UserListWait");

	Observer<ServerError>::NotifyMe(*mSession);
	Observer<UserList>::NotifyMe(*mSession);
	}


/******************************************************************************
*
* NAME
*     UserListWait::UserListWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

UserListWait::UserListWait(const RefPtr<Session>& session) :
		mSession(session)
	{
	WWDEBUG_SAY(("WOL: UserListWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     UserListWait::~UserListWait
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

UserListWait::~UserListWait()
	{
	WWDEBUG_SAY(("WOL: UserListWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     UserListWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserListWait::HandleNotification(ServerError& error)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = error.GetDescription();
		WWDEBUG_SAY(("WOLERROR: UserListWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     UserListWait::HandleNotification(UserList)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void UserListWait::HandleNotification(UserList&)
	{
	if (mEndResult == Waiting)
		{
		WWDEBUG_SAY(("WOL: UserListWait successful\n"));
		EndWait(ConditionMet, WOLSTRING("WOL_USERLISTRECEIVED"));
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     GetUserWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<GetUserWait> GetUserWait::Create(const RefPtr<Session>& session,
		const unichar_t* userName)
	{
	WWASSERT(session.IsValid() && "GetUserWait");
	WWASSERT(userName && "GetUserWait");

	if (!session.IsValid() || (userName == NULL))
		{
		return NULL;
		}

	return new GetUserWait(session, userName);
	}


/******************************************************************************
*
* NAME
*     GetUserWait::GetUserWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

GetUserWait::GetUserWait(const RefPtr<Session>& session, const unichar_t* userName) :
		mSession(session),
		mUserName(userName)
	{
	WWDEBUG_SAY(("WOL: GetUserWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     GetUserWait::GetUserWait
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

GetUserWait::~GetUserWait()
	{
	WWDEBUG_SAY(("WOL: GetUserWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     GetUserWait::WaitBeginning
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void GetUserWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: GetUserWait Begin\n"));
	WWASSERT(mSession.IsValid() && "GetUserWait");

	RefPtr<UserData> user = mSession->FindUser(mUserName);

	if (user.IsValid())
		{
		WWDEBUG_SAY(("WOL: GetUserWait successful\n"));
		EndWait(ConditionMet, WOLSTRING("WOL_USERFOUND"));
		}
	else
		{
		Add(UserListWait::Create(mSession));
		}
	}


/******************************************************************************
*
* NAME
*     GetUserWait::GetResult
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

WaitCondition::WaitResult GetUserWait::GetResult(void)
	{
	WaitResult result = SerialWait::GetResult();

	if (result == ConditionMet)
		{
		RefPtr<UserData> user = mSession->FindUser(mUserName);

		if (user.IsValid())
			{
			WWDEBUG_SAY(("WOL: GetUserWait successful\n"));
			mEndText = WOLSTRING("WOL_USERFOUND");
			}
		else
			{
			WWDEBUG_SAY(("WOL: GetUserWait failed\n"));
			mEndResult = Error;
			mEndText = WOLSTRING("WOL_USERNOTFOUND");
			}
		}

	return mEndResult;
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     LocateUserWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<LocateUserWait> LocateUserWait::Create(const RefPtr<Session>& session,
		const unichar_t* userName)
	{
	WWASSERT(session.IsValid() && "LocateUserWait");
	WWASSERT(userName && "LocateUserWait");

	if (!session.IsValid() || (userName == NULL))
		{
		return NULL;
		}

	return new LocateUserWait(session, userName);
	}


/******************************************************************************
*
* NAME
*     LocateUserWait::LocateUserWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

LocateUserWait::LocateUserWait(const RefPtr<Session>& session, const unichar_t* userName) :
		SingleWait(WOLSTRING("WOL_LOCATINGUSER")),
		mSession(session),
		mUserName(userName)
	{
	WWDEBUG_SAY(("WOL: LocateUserWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     LocateUserWait::~LocateUserWait
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

LocateUserWait::~LocateUserWait()
	{
	WWDEBUG_SAY(("WOL: LocateUserWait End '%S'\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     LocateUserWait::WaitBeginning
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void LocateUserWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: LocateUserWait Begin '%S'\n", (const unichar_t*)mUserName));
	WWASSERT(mSession.IsValid() && "LocateUserWait");

	RefPtr<UserData> user = mSession->FindUser(mUserName);

	if (user.IsValid())
		{
		WWDEBUG_SAY(("WOL: LocateUserWait successful\n"));
		EndWait(ConditionMet, WOLSTRING("WOL_USERFOUND"));
		}

	mSession->AddObserver(*static_cast< Observer<ServerError> *>(this));
	mSession->AddObserver(*static_cast< Observer<UserEvent> *>(this));
	mSession->RequestLocateUser(mUserName);
	}


/******************************************************************************
*
* NAME
*     LocateUserWait::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LocateUserWait::HandleNotification(ServerError& error)
	{
	if (mEndResult == Waiting)
		{
		const unichar_t* errorMsg = error.GetDescription();
		WWDEBUG_SAY(("WOLERROR: LocateUserWait server error '%S'\n", errorMsg));
		EndWait(Error, errorMsg);
		}
	}


/******************************************************************************
*
* NAME
*     LocateUserWait::LocateUserWait
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LocateUserWait::HandleNotification(UserEvent& event)
	{
	if (mEndResult == Waiting)
		{
		const RefPtr<UserData>& user = event.Subject();

		if (mUserName.Compare_No_Case(user->GetName()) == 0)
			{
			WWDEBUG_SAY(("WOL: LocateUserWait successful\n"));
			EndWait(ConditionMet, WOLSTRING("WOL_USERFOUND"));
			}
		}
	}


//-----------------------------------------------------------------------------


/******************************************************************************
*
* NAME
*     SendMsgWait::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<SendMsgWait> SendMsgWait::Create(const RefPtr<Session>& session, ChatMessage& message)
	{
	WWASSERT(session.IsValid() && "SendMsgWait");

	if (!session.IsValid())
		{
		return NULL;
		}

	return new SendMsgWait(session, message);
	}


/******************************************************************************
*
* NAME
*     SendMsgWait::SendMsgWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

SendMsgWait::SendMsgWait(const RefPtr<Session>& session, ChatMessage& message) :
		SingleWait(WOLSTRING("WOL_SENDINGMSG")),
		mSession(session),
		mMessage(message)
	{
	WWDEBUG_SAY(("WOL: SendMsgWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     SendMsgWait::~SendMsgWait
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

SendMsgWait::~SendMsgWait()
	{
	WWDEBUG_SAY(("WOL: SendMsgWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     SendMsgWait::WaitBeginning
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void SendMsgWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: SendMsgWait Begin\n"));
	WWASSERT(mSession.IsValid() && "SendMsgWait");

	if (mMessage.IsPrivate())
		{
		RefPtr<UserData> user = mSession->FindUser(mMessage.GetSendersName());

		if (!user.IsValid())
			{
			WWDEBUG_SAY(("WOLERROR: SendMsgWait user not found\n"));
			EndWait(Error, WOLSTRING("WOL_USERNOTFOUND"));
			return;
			}

		UserList userList;
		userList.push_back(user);

		if (mMessage.IsAction())
			{
			mSession->SendPrivateAction(userList, mMessage.GetMessage());
			}
		else
			{
			mSession->SendPrivateMessage(userList, mMessage.GetMessage());
			}
		}
	else
		{
		if (mMessage.IsAction())
			{
			mSession->SendPublicAction(mMessage.GetMessage());
			}
		else
			{
			mSession->SendPublicMessage(mMessage.GetMessage());
			}
		}

	WWDEBUG_SAY(("WOL: SendMsgWait successful\n"));
	EndWait(ConditionMet, mMessage.GetMessage());
	}

} // namespace WWOnline
