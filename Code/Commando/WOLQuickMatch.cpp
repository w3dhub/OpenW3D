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
*     $Archive: /Commando/Code/Commando/WOLQuickMatch.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 37 $
*     $Modtime: 2/20/02 5:30p $
*
******************************************************************************/

#include "WOLQuickMatch.h"
#include <WWOnline\PingProfile.h>
#include <WWOnline\WaitCondition.h>
#include <WWOnline\WOLProduct.h>
#include <WWOnline\WOLConnect.h>
#include <WWOnline\WOLServer.h>
#include <WWOnline\WOLLadder.h>
#include <WWLib\CPUDetect.h>
#include <WWDebug\WWDebug.h>
#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"


using namespace WWOnline;

typedef void (*QMDispatchFunc)(WOLQuickMatch*, const wchar_t*);

struct QMResponseDispatch
	{
	const wchar_t* Token;
	QMDispatchFunc Dispatch;
	};


#define QUICKMATCH_CHANNELNAME L"lob_39_0"
#define QUICKMATCH_BOTNAME L"matchbot"

/******************************************************************************
*
* NAME
*     WOLQuickMatch::Create
*
* DESCRIPTION
*     Create an instance to a quickmatch game matcher.
*
* INPUTS
*     NONE
*
* RESULT
*     Quickmatch - Quickmatch instance.
*
******************************************************************************/

WOLQuickMatch* WOLQuickMatch::Create(void)
	{
	WOLQuickMatch* match = new WOLQuickMatch;

	if (match)
		{
		if (match->FinalizeCreate())
			{
			return match;
			}

		match->Release_Ref();
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::WOLQuickMatch
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WOLQuickMatch::WOLQuickMatch()
	{
	WWDEBUG_SAY(("WOLQuickMatch: Instantiating\n"));
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::~WOLQuickMatch
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

WOLQuickMatch::~WOLQuickMatch()
	{
	WWDEBUG_SAY(("WOLQuickMatch: Destructing\n"));
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::FinalizeCreate
*
* DESCRIPTION
*     Creation initializaton / finalization
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if successful
*
******************************************************************************/

bool WOLQuickMatch::FinalizeCreate(void)
	{
	mWOLSession = Session::GetInstance(false);

	if (!mWOLSession.IsValid())
		{
		WWDEBUG_SAY(("WOLQuickMatch: ERROR - WWOnline not instantiated\n"));
		return false;
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ConnectClient
*
* DESCRIPTION
*     Connect a game client to the quickmatch bot.
*
* INPUTS
*     ChannelName - Name of matching channel
*     BotName     - Name of matching bot
*
* RESULT
*     Wait - Wait condition to process.
*
******************************************************************************/

RefPtr<WaitCondition> WOLQuickMatch::ConnectClient(void)
	{
	WWDEBUG_SAY(("WOLQuickMatch: Connecting client to '%S' '%S'\n",
		QUICKMATCH_CHANNELNAME, QUICKMATCH_BOTNAME));

	// Make sure that we are logged on to WWOnline
	if (mWOLSession->GetConnectionStatus() != ConnectionConnected)
		{
		WWDEBUG_SAY(("WOLQuickMatch: ERROR - Not connected to WWOnline server\n"));
		RefPtr<SingleWait> wait = SingleWait::Create(TRANSLATE(IDS_WOL_CONNECTING));
		wait->EndWait(WaitCondition::Error, TRANSLATE(IDS_WOL_NOTCONNECTED));
		return wait;
		}

	// Generate client connect wait condition
	RefPtr<SerialWait> connectWait = SerialWait::Create();

	if (connectWait.IsValid())
		{
		Observer<ServerError>::NotifyMe(*mWOLSession);
		Observer<ChatMessage>::NotifyMe(*mWOLSession);

		// Request channel list
		RefPtr<WaitCondition> channelListWait = mWOLSession->GetNewChatChannelList();
		connectWait->Add(channelListWait);

		// Join the matching channel
		RefPtr<Product> product = Product::Current();
		WWASSERT(product.IsValid() && "WOLProduct not initialized.");
		const wchar_t* password = product->GetChannelPassword();

		RefPtr<WaitCondition> joinWait = mWOLSession->JoinChannel(QUICKMATCH_CHANNELNAME, password, 0);
		connectWait->Add(joinWait);

		// Make sure the matching bot is there.
		RefPtr<WaitCondition> findBotWait = GetUserWait::Create(mWOLSession, QUICKMATCH_BOTNAME);
		connectWait->Add(findBotWait);
		}

	return connectWait;
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::Disconnect
*
* DESCRIPTION
*     Disconnect from quickmatch
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Disconnect wait condition to process.
*
******************************************************************************/

RefPtr<WaitCondition> WOLQuickMatch::Disconnect(void)
	{
	WWDEBUG_SAY(("WOLQuickMatch: Disconnecting\n"));

	Observer<ServerError>::StopObserving();
	Observer<ChatMessage>::StopObserving();

	// If we are in the matching channel then disconnect.
	RefPtr<ChannelData> channel = mWOLSession->GetCurrentChannel();

	if (channel.IsValid())
		{
		const WideStringClass& name = channel->GetName();

		if (name.Compare_No_Case(QUICKMATCH_CHANNELNAME) == 0)
			{
			return mWOLSession->LeaveChannel();
			}
		}

	return NULL;	
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::SendClientInfo
*
* DESCRIPTION
*     Send client matching preferences to the quickmatch bot.
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful.
*
******************************************************************************/

bool WOLQuickMatch::SendClientInfo(void)
	{
	unsigned long ver = cNetwork::Get_Exe_Key();

	// Get CPU speed
	int speed = CPUDetectClass::Get_Processor_Speed();

	// Get amount of physical memory
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	unsigned long memory = (memStatus.dwTotalPhys / 1048576);

	//-------------------------------------------------------------------------
	// Gather pings
	//-------------------------------------------------------------------------
	const PingProfile& pings = GetLocalPingProfile();
	char pseudoPings[18] = {0};
	EncodePingProfile(pings, pseudoPings);

	//-------------------------------------------------------------------------
	// Get clients ladder points
	//-------------------------------------------------------------------------
	RefPtr<UserData> client = mWOLSession->GetCurrentUser();
	WWASSERT(client.IsValid());

	if (client.IsValid() == false)
		{
		return false;
		}

	int tpoints = 0;
	unsigned int played = 0;

	RefPtr<LadderData> ladder = client->GetTeamLadder();

	if (ladder.IsValid())
		{
		tpoints = ladder->GetPoints();
		played = ladder->GetReserved2();
		}

	//-------------------------------------------------------------------------
	// Generate client information message
	//-------------------------------------------------------------------------
	WideStringClass clientMsg(256, true);
	clientMsg.Format(L"CINFO VER=%lu CPU=%lu MEM=%lu TPOINTS=%ld PLAYED=%lu PINGS=%S",
		ver, speed, memory, tpoints, played, pseudoPings);

	WWDEBUG_SAY(("WOLQuickMatch: '%S'\n", (const WCHAR*)clientMsg));
	return mWOLSession->SendPrivateMessage(QUICKMATCH_BOTNAME, (const WCHAR*)clientMsg);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::SendServerInfo
*
* DESCRIPTION
*     Send information describing the game,
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::SendServerInfo(const char* exInfo, const char* topic)
	{
	if (exInfo && topic)
		{
#pragma message(__FILE__" *** HACK ALERT *** SINFO msg is imitating WOLAPI IRC topic!")

		// *** WARNING *** DANGER *** HACK ALERT ****
		//
		// The SINFO message sent to the matching bot is assembled in such
		// a way as to imitate the IRC topic string that WOLAPI produces.
		WideStringClass botMsg(0, true);
		botMsg.Format(L"SINFO %S%S", exInfo, topic);
		WWDEBUG_SAY(("WOLQuickMatch: '%S'\n", (const WCHAR*)botMsg));

		mWOLSession->SendPrivateMessage(QUICKMATCH_BOTNAME, botMsg);
		}
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::SendStatus
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::SendStatus(const wchar_t* statusMsg)
	{
	WWDEBUG_SAY(("WOLQuickMatch: Status '%S'\n", statusMsg));

	WideStringClass msg(256, true);
	msg = statusMsg;
	QuickMatchEvent status(QuickMatchEvent::QMMSG, msg);
	NotifyObservers(status);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ParseResponse
*
* DESCRIPTION
*     Handle response messages from the quickmatch bot.
*
* INPUTS
*     Message - Response message from quickmatch bot.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::ParseResponse(const wchar_t* message)
	{
	if (message)
		{
		static QMResponseDispatch _dispatch[] =
			{
			{L"INFO ", WOLQuickMatch::ProcessInfo},
			{L"ERROR ", WOLQuickMatch::ProcessError},
			{L"START ", WOLQuickMatch::ProcessStart},
			{NULL, WOLQuickMatch::ProcessUnknown}
			};

		int index = 0;
		const wchar_t* token = _dispatch[index].Token;

		while (token)
			{
			// Find the first occurance of the token in the message
			const wchar_t* cmd = wcsstr(message, token);

			// If the token was found and it is at the start of the message
			// then return the type of message this is.
			if (cmd && cmd == message)
				{
				const wchar_t* data = (message + wcslen(token));
				_dispatch[index].Dispatch(this, data);
				}

			index++;
			token = _dispatch[index].Token;
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ProcessInfo
*
* DESCRIPTION
*     Process information messages.
*
* INPUTS
*     Message - 
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::ProcessInfo(WOLQuickMatch* quickmatch, const wchar_t* data)
	{
	WideStringClass msg(255, true);
	msg = data;

	QuickMatchEvent status(QuickMatchEvent::QMINFO, msg);
	quickmatch->NotifyObservers(status);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ProcessError
*
* DESCRIPTION
*     Process error messages from the quickmatch bot.
*
* INPUTS
*     Message - Error message
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::ProcessError(WOLQuickMatch* quickmatch, const wchar_t* data)
	{
	WideStringClass msg(255, true);
	msg = data;

	QuickMatchEvent status(QuickMatchEvent::QMERROR, msg);
	quickmatch->NotifyObservers(status);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ProcessStart
*
* DESCRIPTION
*     Process start message from the quickmatch bot.
*
* INPUTS
*     Data - Start game parameters
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::ProcessStart(WOLQuickMatch* quickmatch, const wchar_t* data)
	{
	// Send message indicating successful match
	WideStringClass msg(255, true);
	msg.Format(TRANSLATE(IDS_MENU_QM_MATCHED_WITH), data);

	QuickMatchEvent status(QuickMatchEvent::QMMSG, msg);
	quickmatch->NotifyObservers(status);

	// Send message that we are matched.
	msg = data;
	QuickMatchEvent matchedEvent(QuickMatchEvent::QMMATCHED, msg);
	quickmatch->NotifyObservers(matchedEvent);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::ProcessUnknown
*
* DESCRIPTION
*     Process unknown messages from the quickmatch bot.
*
* INPUTS
*     Message - Unknown message
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::ProcessUnknown(WOLQuickMatch* quickmatch, const wchar_t* data)
	{
	WideStringClass msg(255, true);
	msg = data;

	QuickMatchEvent status(QuickMatchEvent::QMUNKNOWN, msg);
	quickmatch->NotifyObservers(status);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::HandleNotification(ServerError)
*
* DESCRIPTION
*     Handle server errors.
*
* INPUTS
*     Error - Server error
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::HandleNotification(ServerError& error)
	{
	const wchar_t* errorMsg = error.GetDescription();
	WWDEBUG_SAY(("WOLQuickMatch: ERROR - ServerError '%S'\n", errorMsg));
	QuickMatchEvent status(QuickMatchEvent::QMERROR, errorMsg);
	NotifyObservers(status);
	}


/******************************************************************************
*
* NAME
*     WOLQuickMatch::HandleNotification(ChatMessageEvent)
*
* DESCRIPTION
*     Handle private messages coming from the matchbot.
*
* INPUTS
*     Message - Chat message
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLQuickMatch::HandleNotification(ChatMessage& message)
	{
	const WideStringClass& sender = message.GetSendersName();

	if (sender.Compare_No_Case(QUICKMATCH_BOTNAME) == 0)
		{
		WWDEBUG_SAY(("WOLQuickMatch: BotMsg - '%S'\n", message.GetMessage()));
		ParseResponse(message.GetMessage());
		}
	}
