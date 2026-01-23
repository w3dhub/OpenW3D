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
*     $Archive: /Commando/Code/WWOnline/WOLChatObserver.cpp $
*
* DESCRIPTION
*     Westwood Online Chat events handling / dispatcher.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Byon_g $
*
* VERSION INFO
*     $Revision: 76 $
*     $Modtime: 10/31/02 3:15p $
*
******************************************************************************/

#include "atlbase_compat.h"
#include "WOLChatObserver.h"
#include "WOLSession.h"
#include "WOLProduct.h"
#include "WOLServer.h"
#include "WOLDownload.h"
#include "WOLChannel.h"
#include "WOLSquad.h"
#include "WOLLadder.h"
#include "WOLChatMsg.h"
#include "WOLPageMsg.h"
#include "WOLGameOptions.h"
#include "WOLGame.h"
#include "WOLErrorUtil.h"
#include <wwlib/registry.h>
#include <Commando/_globals.h>
#include "systimer.h"
#include "specialbuilds.h"
#include "simplevec.h"
#include "../Commando/cnetwork.h"
namespace WOL
{
#include <wolapi/chatdefs.h>
}

namespace WWOnline {

/******************************************************************************
*
* NAME
*     ChatObserver::ChatObserver
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

ChatObserver::ChatObserver() :
		mRefCount(1),
		mOuter(NULL)
	{
	WWDEBUG_SAY(("WOL: ChatObserver Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     ChatObserver::~ChatObserver
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

ChatObserver::~ChatObserver()
	{
	WWDEBUG_SAY(("WOL: ChatObserver Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     ChatObserver::Init
*
* DESCRIPTION
*     Initialize chat observer
*
* INPUTS
*     Session - Outer session to associate this observer with.
*
* RESULT
*     NONE
*
******************************************************************************/

void ChatObserver::Init(Session& outer)
	{
	mOuter = &outer;
	}


/****************************************************************************
*
* NAME
*     IUnknown::QueryInterface
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Error / Status result
*
****************************************************************************/

STDMETHODIMP ChatObserver::QueryInterface(const IID& iid, void** ppv)
	{
	if ((iid == IID_IUnknown) || (iid == WOL::IID_IChatEvent))
		{
		*ppv = static_cast<WOL::IChatEvent*>(this);
		}
	else
		{
		*ppv = NULL;
		return E_NOINTERFACE;
		}

	static_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
	}


/****************************************************************************
*
* NAME
*     IUnknown::AddRef
*
* DESCRIPTION
*     Increment objects reference count.
*
* INPUTS
*     NONE
*
* RESULT
*     RefCount - New reference count.
*
****************************************************************************/

ULONG STDMETHODCALLTYPE ChatObserver::AddRef(void)
	{
	InterlockedIncrement((LPLONG)&mRefCount);
	return mRefCount;
	}


/****************************************************************************
*
* NAME
*     IUnknown::Release
*
* DESCRIPTION
*     Release reference to object.
*
* INPUTS
*     NONE
*
* RESULT
*     RefCount - Remaining references
*
****************************************************************************/

ULONG STDMETHODCALLTYPE ChatObserver::Release(void)
	{
	InterlockedDecrement((LPLONG)&mRefCount);

	if (mRefCount == 0)
		{
		delete this;
		return 0;
		}

	return mRefCount;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnServerList
*
* DESCRIPTION
*     Handle
*
* INPUTS
*     Result  - Result / status code.
*     Servers - Server list.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnServerList(HRESULT result, WOL::Server* servers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnServerList '%s'\n", GetChatErrorString(result)));
		ServerError error(result, "WOL_SERVERLISTERROR");
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	mOuter->mRequestingServerList = false;

	// If update list received, don't provide server list
	if (mOuter->mPatchFiles.size() > 0)
		{
		return S_OK;
		}

	// If we didn't really want a server list anyway then ignore it.
	if (mOuter->mIgnoreServerLists)
		{
		return S_OK;
		}

	// Process new server list.
	mOuter->ClearServers();

	// Clear out old server info from the registry.
	RegistryClass reg(APPLICATION_SUB_KEY_NAME_SERVER_LIST);
	reg.Deleta_All_Values();

	WOL::Server* curServer = servers;

	while (curServer)
		{
		WWDEBUG_SAY(("WOL: Server '%s:%s'\n", curServer->connlabel, curServer->name));

		if ((strcmp((char*)curServer->connlabel, "IRC") == 0) ||
				(strcmp((char*)curServer->connlabel, "IGS") == 0))
			{
			mOuter->mIRCServers.push_back(IRCServerData::Create(*curServer));
			}
		else if (strcmp((char*)curServer->connlabel, "LAD") == 0)
			{
			mOuter->mLadderServer = LadderServerData::Create(*curServer);
			}
		else if (strcmp((char*)curServer->connlabel, "GAM") == 0)
			{
			mOuter->mGameResultsServer = GameResultsServerData::Create(*curServer);
			}
		else if (strcmp((char*)curServer->connlabel, "WDT") == 0)
			{
			mOuter->mWDTServer = WDTServerData::Create(*curServer);
			}
		else if (strcmp((char*)curServer->connlabel, "MGL") == 0)
			{
			mOuter->mMGLServers.push_back(MGLServerData::Create(*curServer));
			}
		else if (strcmp((char*)curServer->connlabel, "PNG") == 0)
			{
			RefPtr<PingServerData> pinger = PingServerData::Create(*curServer);
			WWASSERT(pinger.IsValid());

			if (pinger.IsValid())
				{
				mOuter->mPingServers.push_back(pinger);

				// Automatcally request the ping time to this server.
				const char* address = pinger->GetHostAddress();
				mOuter->RequestPing(address);
				}
			}

		curServer = curServer->next;
		}

	// Notify others about the server list.
	mOuter->NotifyObservers(mOuter->mIRCServers);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUpdateList
*
* DESCRIPTION
*     Handle application update requirement.
*
* INPUTS
*     Result  - Error / status code.
*     Updates - List of updates.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUpdateList(HRESULT result, WOL::Update* updates)
	{
	WWDEBUG_SAY(("WOL: OnUpdateList received\n"));

	// Make sure WOLSession is initialized
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Clear any previous patch files
	mOuter->mPatchFiles.clear();

	// Report any error condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUpdateList '%s'\n", GetChatErrorString(result)));

		const char* errorString = "WOL_UPDATELISTERROR";

		if (result == CHAT_E_STATUSERROR)
			{
			errorString = "WOL_BADINSTALL";
			}

		ServerError error(result, errorString);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	// Process the patch updates
	WOL::Update* update = updates;

	while (update)
		{
		WWDEBUG_SAY(("WOL: Patch file '%s\\%s\\%s'\n",
			update->server, update->patchpath, update->patchfile));

		mOuter->mPatchFiles.push_back(Download::Create(*update));
		update = update->next;
		}

	// If there are updates then notify the client.
	if (updates)
		{
		// If updates are received, do not provide list of servers
		mOuter->ClearServers();

		ServerError error(CHAT_E_MUSTPATCH, "WOL_PATCHREQUIRED");
		mOuter->NotifyObservers(error);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnServerError
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*     Text   - Error text description.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnServerError(HRESULT result, [[maybe_unused]] LPCSTR errorText)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (SUCCEEDED(result))
		{
		ServerError error(-1, "WOL_SERVERERROR");
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWDEBUG_SAY(("WOLERROR: OnServerError '%s' %s\n", GetChatErrorString(result), errorText));

	if (result != CHAT_E_UNKNOWNRESPONSE)
		{
		ServerError error(result, GetChatErrorString(result));
		mOuter->NotifyObservers(error);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnConnection
*
* DESCRIPTION
*
* INPUTS
*     Status - Connection error / status code.
*     MOTD   - Message of the day text.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnConnection(HRESULT result, LPCSTR motd)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	const char* errorText = NULL;

	switch (result)
		{
		// Connection successful
		case S_OK:
			{
			WWDEBUG_SAY(("WOL: Connected to server.\n"));
			mOuter->GetChatObject()->SetClientVersion(WOLAPI_BUILD_VERSION);
			
			mOuter->mCurrentConnectionStatus = ConnectionConnected;
			mOuter->mCurrentServer = mOuter->mPendingServer;
			mOuter->mCurrentLogin = mOuter->mPendingLogin;

			mOuter->mPendingServer.Release();
			mOuter->mPendingLogin.Release();

			// Check if the loginname is in the userlist. If not then create
			// a user for the login.
			RefPtr<LoginInfo> login = mOuter->mCurrentLogin;

			if (login.IsValid())
				{
				const WideStringClass& loginName = login->GetNickname();

				// Setup the current user for this session.
				RefPtr<UserData> user = mOuter->GetUserOrBuddy(loginName);

				if (!user.IsValid())
					{
					user = UserData::Create(loginName);
					}

				// Setup the current user
				WWASSERT(user.IsValid() && "OnConnection: Failed to create user");
				user->SetLocale(login->GetLocale());
				mOuter->mCurrentUser = user;

				// Request information about the current user.
				mOuter->RequestUserLocale(loginName);
				mOuter->RequestSquadInfoByMemberName(loginName);
				mOuter->RequestLadderInfo(loginName, LadderType_Team);
				mOuter->mLastUserDataRequestTime = TIMEGETTIME();
				}

			// Notify others about the connection
			mOuter->NotifyObservers(mOuter->mCurrentConnectionStatus);

			// Notify others about the message of the day.
			mOuter->mMessageOfTheDay = motd;
			MessageOfTheDayEvent motdEvent(mOuter->mMessageOfTheDay);
			mOuter->NotifyObservers(motdEvent);

			// Request insider status information and server time information
			mOuter->RequestInsiderStatus();
			mOuter->RequestServerTime();

			return S_OK;
			}
			break;

		case CHAT_E_NICKINUSE:
			errorText = "WOL_NICKINUSE";
			break;

		case CHAT_E_BADPASS:
			errorText = "WOL_BADPASSWORD";
			break;

		case CHAT_E_BANNED:
			errorText = "WOL_BANNED";
			break;

		case CHAT_E_DISABLED:
			errorText = "WOL_ACCOUNTDISABLED";
			break;

		case CHAT_E_SERIALBANNED:
			errorText = "WOL_SERIALBANNED";
			break;

		case CHAT_E_SERIALDUP:
			errorText = "WOL_SERIALDUPLICATE";
			break;

		case CHAT_E_SERIALUNKNOWN:
			errorText = "WOL_SERIALUNKNOWN";
			break;

		case CHAT_E_SKUSERIALMISMATCH:
			errorText = "WOL_SERIALSKUMISMATCH";
			break;

		default:
			errorText = "WOL_CONNECTERROR";
			break;
		}

	// We should only get here if the connection failed.
	mOuter->mPendingServer.Release();
	mOuter->mCurrentServer.Release();
	mOuter->mPendingLogin.Release();
	mOuter->mCurrentLogin.Release();
	mOuter->mCurrentConnectionStatus = ConnectionDisconnected;

	WWDEBUG_SAY(("WOLERROR: OnConnection '%s'\n", GetChatErrorString(result)));
	ServerError error(result, errorText);
	mOuter->NotifyObservers(error);
	mOuter->NotifyObservers(mOuter->mCurrentConnectionStatus);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnMessageOfTheDay
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnMessageOfTheDay(HRESULT, LPCSTR messageOfTheDay)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Notify other about the message of the day.
	mOuter->mMessageOfTheDay = messageOfTheDay;
	MessageOfTheDayEvent motdEvent(mOuter->mMessageOfTheDay);
	mOuter->NotifyObservers(motdEvent);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelList
*
* DESCRIPTION
*     Handle receipt of new channel list.
*
* INPUTS
*     Result   - Error / status code
*     Channels - Linked list of channels
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelList(HRESULT result, WOL::Channel* inChannels)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Report any error conditions
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelList '%s'\n", GetChatErrorString(result)));
		ChannelList empty;
		ChannelListEvent error(ChannelListEvent::Error, empty, -1);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	// If this is a ping list then update the latency of each channel.
	if (result == CHAT_S_PINGLIST)
		{
		WWDEBUG_SAY(("WOL: OnChannelList (PingList)\n"));

		WOL::Channel* wolChannel = inChannels;

		while (wolChannel)
			{
			const RefPtr<ChannelData>& channel = mOuter->FindChannel((const char*)wolChannel->name);

			if (channel.IsValid())
				{
				channel->SetLatency(wolChannel->latency);
				}

			wolChannel = wolChannel->next;
			}

		return S_OK;
		}

	// Get code for game channels
	RefPtrConst<Product> product = Product::Current();
	WWASSERT(product.IsValid() && "WOLProduct not initialized");

	int gameCode = -1;

	if (product.IsValid())
		{
		gameCode = product->GetGameCode();
		}
	else
		{
		WWDEBUG_SAY(("WOLERROR: WOLProduct not initialized.\n"));
		}

	// Create the new channel lists
	ChannelList chatChannels;
	ChannelList gameChannels;
	WOL::Channel* wolChannel = inChannels;

	while (wolChannel)
		{
		WWDEBUG_SAY(("WOL: Channel '%s:%ld'\n", (const char*)wolChannel->name, wolChannel->type));

		ChannelList* curList = NULL;
		ChannelList* newList = NULL;

		if (wolChannel->type == 0)
			{
			curList = &mOuter->mChatChannels;
			newList = &chatChannels;
			}
		else if (wolChannel->type == gameCode)
			{
			curList = &mOuter->mGameChannels;
			newList = &gameChannels;
			}
		else
			{
			continue;
			}

		WWASSERT(curList != NULL);
		WWASSERT(newList != NULL);

		ChannelList::iterator iter = FindChannelNode(*curList, (const char*)wolChannel->name);

		if (iter != curList->end())
			{
			RefPtr<ChannelData> channel = *iter;
			WWASSERT(channel.IsValid());
			channel->UpdateData(*wolChannel);

			ChannelList::iterator end = newList->end();
			newList->splice(end, *curList, iter);
			}
		else
			{
			RefPtr<ChannelData> channel = ChannelData::Create(*wolChannel);
			WWASSERT(channel.IsValid());
			newList->push_back(channel);
			}

		wolChannel = wolChannel->next;
		}

	// Replace lists
	mOuter->mChatChannels.swap(chatChannels);
	mOuter->mGameChannels.swap(gameChannels);

	// Notify other about the new channel lists
	ChannelListEvent chatListEvent(ChannelListEvent::NewList, mOuter->mChatChannels, 0);
	mOuter->NotifyObservers(chatListEvent);

	ChannelListEvent gameListEvent(ChannelListEvent::NewList, mOuter->mGameChannels, gameCode);
	mOuter->NotifyObservers(gameListEvent);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelCreate
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelCreate(HRESULT result, WOL::Channel* inChannel)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle failure condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelCreate '%s'\n", GetChatErrorString(result)));

		ChannelStatus status = GetChannelStatusFromHResult(result);
		mOuter->mCurrentChannelStatus = status;

		// Notify others about the channel creation failure
		ChannelEvent error(status, mOuter->mPendingChannel);
		mOuter->NotifyObservers(error);

		mOuter->mPendingChannel.Release();
		mOuter->mCurrentChannel.Release();
		return S_OK;
		}

	WWASSERT(inChannel != NULL && "OnChannelCreate parameter error");
	WWDEBUG_SAY(("WOL: Created channel '%s'\n", (char*)inChannel->name));

	// Make sure we have the channel to create pending.
	if (!mOuter->mPendingChannel.IsValid())
		{
		WWASSERT(mOuter->mPendingChannel.IsValid());
		return S_OK;
		}

	// Check if the created channel matches the pending one.
	unichar_t channelName[64];
	u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));
	const WideStringClass& pendingName = mOuter->mPendingChannel->GetName();

	if (pendingName.Compare_No_Case(channelName) != 0)
		{
		WWASSERT("OnChannelCreate: Channel name mismatch");
		return S_OK;
		}

	// Verify current login is valid
	RefPtr<LoginInfo> login = mOuter->mCurrentLogin;

	if (!login.IsValid())
		{
		WWASSERT(login.IsValid() && "OnChannelCreate: No login");
		return S_OK;
		}

	// Get or create user who created the channel
	RefPtr<UserData> user = mOuter->GetUserOrBuddy(login->GetNickname());

	if (!user.IsValid())
		{
		user = UserData::Create(login->GetNickname());
		}

	if (!user.IsValid())
		{
		WWASSERT(user.IsValid() && "OnChannelCreate: Unable to find or create user");
		return S_OK;
		}

	// The pending channel now becomes the current channel.
	mOuter->mCurrentChannel = mOuter->mPendingChannel;
	mOuter->mPendingChannel.Release();
	mOuter->mCurrentChannel->UpdateData(*inChannel);
	mOuter->mCurrentChannelStatus = ChannelJoined;

	// Notify others the channel was created
	ChannelEvent createdEvent(ChannelCreated, mOuter->mCurrentChannel);
	mOuter->NotifyObservers(createdEvent);

	// Add the logged in user to the userlist
	mOuter->mUsers.push_back(user);

	// Notify others that the logged in user has joined the newly created channel.
	UserEvent event(UserEvent::Join, user);
	mOuter->NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelJoin
*
* DESCRIPTION
*     User has joined a channel.
*
* INPUTS
*     Result  - Error / status code
*     Channel - Channel joined
*     User    - User joining channel.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelJoin(HRESULT result, WOL::Channel* inChannel,
		WOL::User* inUser)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle failure conditions
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelJoin '%s'\n", GetChatErrorString(result)));
		ChannelStatus status = GetChannelStatusFromHResult(result);
		mOuter->mCurrentChannelStatus = status;

		// Get the channel we are attempting to join
		RefPtr<ChannelData> badChannel = mOuter->mPendingChannel;

		if (badChannel.IsValid() == false)
			{
			badChannel = mOuter->mCurrentChannel;
			}

		mOuter->mPendingChannel.Release();
		mOuter->mCurrentChannel.Release();

		// Notify others the reason for the channel join failure.
		ChannelEvent error(status, badChannel);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWASSERT(inChannel != NULL && "OnChannelJoin parameter error");
	WWASSERT(inUser != NULL && "OnChannelJoin parameter error");

  	struct in_addr addr;
	addr.S_un.S_addr = inUser->ipaddr;

	WWDEBUG_SAY(("WOL: User '%s' @ %s joining channel '%s'\n", (char*)inUser->name, inet_ntoa(addr), (char*)inChannel->name));

	// Verify that the current login is valid
	if (!mOuter->mCurrentLogin.IsValid())
		{
		WWASSERT(mOuter->mCurrentLogin.IsValid() && "OnChannelJoin: No login");
		return S_OK;
		}

	// Get or create the user who is joining.
	unichar_t inUsername[64];
	u_mbtows(inUsername, (char*)inUser->name, sizeof(inUser->name));
	RefPtr<UserData> user = mOuter->GetUserOrBuddy(inUsername);

	if (!user.IsValid())
		{
		user = UserData::Create(*inUser);
		}
	else
		{
		user->UpdateData(*inUser);
		}

	if (!user.IsValid())
		{
		WWASSERT(user.IsValid() && "OnChannelJoin: No user");
		return S_OK;
		}

	// Get channel name
	unichar_t channelName[64];
	u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));

	// Is the logged in user joining this channel?
	const WideStringClass& loginName = mOuter->mCurrentLogin->GetNickname();

	if (loginName.Compare_No_Case(inUsername) == 0)
		{
		// Verify that we are wanting to join a channel
		if (!mOuter->mPendingChannel.IsValid())
			{
			WWASSERT(mOuter->mPendingChannel.IsValid() && "OnChannelJoin: Unknown channel.");
			return S_OK;
			}

		// Verify that the channel joined is the one we were expecting to join.
		const WideStringClass& pendingChannelName = mOuter->mPendingChannel->GetName();

		if (pendingChannelName.Compare_No_Case(channelName) != 0)
			{
			WWASSERT(!"OnChannelJoin: Pending channel mismatch");
			return S_OK;
			}

		// The pending channel becomes the current channel
		mOuter->mPendingChannel->UpdateData(*inChannel);
		mOuter->mCurrentChannel = mOuter->mPendingChannel;
		mOuter->mPendingChannel.Release();

		mOuter->mCurrentChannelStatus = ChannelJoined;

		// Notify others that we have joined the channel
		ChannelEvent event(ChannelJoined, mOuter->mCurrentChannel);
		mOuter->NotifyObservers(event);
		}
	else
		{
		WWDEBUG_SAY(("WOL: OnChannelJoin other user '%S'\n", inUsername));
		}

	mOuter->AutoRequestUserDetails(user);

	// Make sure the current channel is valid
	if (!mOuter->mCurrentChannel.IsValid())
		{
		WWASSERT(mOuter->mCurrentChannel.IsValid());
		return S_OK;
		}

	// Verify that the channel joined is the current one.
	const WideStringClass& curChannelName = mOuter->mCurrentChannel->GetName();

	if (curChannelName.Compare_No_Case(channelName) != 0)
		{
		WWASSERT(!"OnChannelJoin: Channel mismatch");
		return S_OK;
		}

	// Increment user count for this channel
	mOuter->mCurrentChannel->GetData().currentUsers++;

	// Get the time they joined the channel. They have 2 mins to get into the game or they are kicked.
   user->mKickTimer = TIMEGETTIME();

	// Add the user to the user list
	mOuter->mUsers.push_back(user);

	// Notify others that a user joined the current channel.
	UserEvent event(UserEvent::Join, user);
	mOuter->NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelLeave
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelLeave(HRESULT result, WOL::Channel* inChannel, WOL::User* inUser)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle error condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelLeave '%s'\n", GetChatErrorString(result)));
		ChannelStatus status = GetChannelStatusFromHResult(result);
		mOuter->mCurrentChannelStatus = status;

		ChannelEvent error(status, mOuter->mCurrentChannel);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWASSERT(inChannel != NULL && "OnChannelLeave: Parameter error");
	WWASSERT(inUser != NULL && "OnChannelLeave: Parameter error");
	WWDEBUG_SAY(("WOL: User '%s' leaving channel '%s'\n", (char*)inUser->name, (char*)inChannel->name));

	// If we are not logged in then ignore.
	if (!mOuter->mCurrentLogin.IsValid())
		{
		WWASSERT(mOuter->mCurrentLogin.IsValid());
		return S_OK;
		}

	// Get the current channel we are connected to. If we are not connected to any
	// channel then ignore.
	if (!mOuter->mCurrentChannel.IsValid())
		{
		WWASSERT(mOuter->mCurrentChannel.IsValid());
		return S_OK;
		}

	// There is a bug in WOLAPI (no surprise here!) that results in the leaving
	// channel being empty if the channel was join explicitly (without picking it
	// from the channel list) as in the case of quickmatch games. Therefore it is
	// necessary to check for this condition here. If the channel name is empty then
	// we MUST ASSUME that the channel we are leaving is the one we are currently
	// connected to.
	unichar_t channelName[64];
	const WideStringClass& curChannelName = mOuter->mCurrentChannel->GetName();

	if (strlen((const char*)inChannel->name) == 0)
		{
		u_strncpy(channelName, curChannelName, (sizeof(channelName) / sizeof(unichar_t)));
		}
	else
		{
		u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));
		}

	// If the leaving channel is not the channel we are connected to then ignore.
	if (curChannelName.Compare_No_Case(channelName) != 0)
		{
		WWASSERT(!"OnChannelLeave: Channel mismatch.");
		return S_OK;
		}

	// Decrement user count for this channel
	WWASSERT(mOuter->mCurrentChannel->GetCurrentUsers() > 0);
	mOuter->mCurrentChannel->GetData().currentUsers--;

	// If current user is leaving then disconnect from the channel clear the user list.
	unichar_t inUsername[64];
	u_mbtows(inUsername, (char*)inUser->name, sizeof(inUser->name));
	const WideStringClass& nickname = mOuter->mCurrentLogin->GetNickname();

	if (nickname.Compare_No_Case(inUsername) == 0)
		{
		RefPtr<ChannelData> leftChannel = mOuter->mCurrentChannel;

		mOuter->mCurrentChannelStatus = ChannelLeft;
		mOuter->mCurrentChannel.Release();

		// Notify others that we have left the channel
		ChannelEvent event(ChannelLeft, leftChannel);
		mOuter->NotifyObservers(event);

		mOuter->mUsers.clear();
		mOuter->NotifyObservers(mOuter->mUsers);
		}
	else
		{
		// If some other user is leaving then notify that he left the channel.
		RefPtr<UserData> leavingUser = RemoveUserInList(inUsername, mOuter->mUsers);

		if (leavingUser.IsValid())
			{
			UserEvent event(UserEvent::Leave, leavingUser);
			mOuter->NotifyObservers(event);
			}
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelTopic
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelTopic(HRESULT result, WOL::Channel* inChannel, LPCSTR topic)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle error condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelTopic '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// If inputs are invalid then ignore
	if (inChannel == NULL)
		{
		WWASSERT(inChannel);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: Channel '%s' Topic changed.\nExInfo: %s\nTopic: %s\n",
			(const char*)inChannel->name, (const char*)inChannel->exInfo, topic));

	unichar_t channelName[64];
	u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));

	RefPtr<ChannelData> channel = mOuter->FindChannel(channelName);

	if (channel.IsValid())
		{
#ifndef FREEDEDICATEDSERVER			
		channel->SetTopic(topic);
		channel->SetExtraInfo((const char*)inChannel->exInfo);

		ChannelEvent event(ChannelNewData, channel);
		mOuter->NotifyObservers(event);
#else
		bool bad_topic = false;
		if (strcmp(topic, channel->GetTopic()) != 0) {
			bad_topic = true;
			WWDEBUG_SAY(("Bad topic. Old = %s, new = %s\n", channel->GetTopic(), topic));
		}
		if (strcmp((char*)&inChannel->exInfo[0], channel->GetExtraInfo()) != 0) {
			bad_topic = true;
			WWDEBUG_SAY(("Bad exInfo. Old = %s, new = %s\n", channel->GetExtraInfo(), inChannel->exInfo));
		}
		if (bad_topic) {
			mOuter->SendChannelTopic();
		}
#endif
		}
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPrivateAction
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPrivateAction(HRESULT result, WOL::User* user, LPCSTR message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateAction '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, message, true, true);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPublicAction
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPublicAction(HRESULT result, WOL::Channel*, WOL::User* user, LPCSTR message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicAction '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, message, false, true);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserList
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserList(HRESULT result, WOL::Channel* inChannel, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserList '%s'\n", GetChatErrorString(result)));
		ServerError error(result, "WOL_USERLISTERROR");
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	if (inChannel == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: NULL channel\n"));
		WWASSERT(inChannel != NULL);
		return S_OK;
		}

	if (!mOuter->mCurrentChannel.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: UserList no current channel\n"));
		WWASSERT(!"OnUserList: No current channel");
		return S_OK;
		}

	// Verify the channel the user list is for against the channel we are in.
	unichar_t channelName[64];
	u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));
	const WideStringClass& curChannelName = mOuter->mCurrentChannel->GetName();

	if (curChannelName.Compare_No_Case(channelName) != 0)
		{
		WWDEBUG_SAY(("WOLERROR: UserList channel mismatch\n"));
		WWASSERT(!"OnUserList: Channel mismatch");
		return S_OK;
		}

	// Process the users.
	UserList userList;
	WOL::User* curUser = inUsers;

	while (curUser)
		{
		unichar_t name[64];
		u_mbtows(name, (const char*)curUser->name, sizeof(curUser->name));

		// If the user is not already in our list then create them.
		RefPtr<UserData> user = mOuter->GetUserOrBuddy(name);

		if (!user.IsValid())
			{
			user = UserData::Create(*curUser);
			}
		else
			{
			user->UpdateData(*curUser);
			}

		if (user.IsValid())
			{
			userList.push_back(user);

			// If we are connected to a game channel then request detailed
			// information about the users.
			if (mOuter->mCurrentChannel.IsValid() && (mOuter->mCurrentChannel->GetType() != 0))
				{
				mOuter->AutoRequestUserDetails(user);
				}
			}

		curUser = curUser->next;
		}

	// Update the user count for this channel.
	RefPtr<ChannelData> channel = mOuter->FindChannel((const char*)inChannel->name);

	if (channel.IsValid())
		{
		channel->GetData().currentUsers = static_cast<unsigned int>(userList.size());
		}

	// Update the user list
	mOuter->mUsers.swap(userList);
	userList.clear();

	mOuter->NotifyObservers(mOuter->mUsers);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPublicMessage
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPublicMessage(HRESULT result, WOL::Channel*, WOL::User* user, LPCSTR message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicMessage '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, message, false, false);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPrivateMessage
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPrivateMessage(HRESULT result, WOL::User* user, LPCSTR message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateMessage '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, message, true, false);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnSystemMessage
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnSystemMessage(HRESULT result, LPCSTR message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnSystemMessage '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(NULL, message, false, false);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnNetStatus
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnNetStatus(HRESULT result)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	ConnectionStatus status = mOuter->mCurrentConnectionStatus;

	switch (result)
		{
		case CHAT_E_CON_LOOKUP_FAILED:
		case CHAT_E_CON_ERROR:
		case CHAT_E_TIMEOUT:
		case CHAT_S_CON_DISCONNECTED:
			mOuter->mRequestingServerList = false;
			mOuter->mCurrentServer.Release();
			status = ConnectionDisconnected;

			if (mOuter->mCurrentChannelStatus == ChannelJoined)
				{
				// We can't be in a channel without a WOL connection.
				WWDEBUG_SAY(("WOL: Leaving channel due to chat connection loss\n"));
				mOuter->mCurrentChannel.Release();
				mOuter->mPendingChannel.Release();
				mOuter->mCurrentChannelStatus = ChannelLeft;
				}
			break;

		case CHAT_S_CON_DISCONNECTING:
			status = ConnectionDisconnecting;
			break;

		case CHAT_S_CON_CONNECTING:
			status = ConnectionConnecting;
			break;

		case CHAT_S_CON_CONNECTED:
			break;

		default:
			WWDEBUG_SAY(("WOL: OnNetStatus '%s'\n", GetChatErrorString(result)));
			break;
		}

	if (status != mOuter->mCurrentConnectionStatus)
		{
		mOuter->mCurrentConnectionStatus = status;

		if (status == ConnectionDisconnected)
			{
			mOuter->NotifyObservers(status);
			}
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnLogout
*
* DESCRIPTION
*     Handle other users that have logged out.
*
*     NOTE: This does not get called for the logged in user. When the logged in
*     user logs out OnNetStatus() is called with CHAT_S_CON_DISCONNECTED.
*
* INPUTS
*     Result - Error / status code.
*     User   - User that logged out.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnLogout(HRESULT result, WOL::User* inUser)
	{
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnLogout '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Remove the logged out user from the session
	if (inUser)
		{
		WWDEBUG_SAY(("WOL: User '%s' logged out\n", (char*)inUser->name));

		unichar_t name[64];
		u_mbtows(name, (char*)inUser->name, sizeof(inUser->name));

		// If some other user is leaving then notify that he left the channel.
		RefPtr<UserData> leavingUser = RemoveUserInList(name, mOuter->mUsers);

		if (leavingUser.IsValid())
			{
			// Decrement user count for this channel
			if (mOuter->mCurrentChannel.IsValid())
				{
				WWASSERT(mOuter->mCurrentChannel->GetData().currentUsers > 0);
				mOuter->mCurrentChannel->GetData().currentUsers--;
				}

			UserEvent event(UserEvent::Leave, leavingUser);
			mOuter->NotifyObservers(event);
			}
		}

	return S_OK;
	}




/*
** HACK. Private game options spam control.
*/
class PrivateGameOptionsTrackingClass
{
		public:
			char UserName[256];
			DynamicVectorClass<unsigned int> Times;

			bool operator == (PrivateGameOptionsTrackingClass const &whatever);
			bool operator != (PrivateGameOptionsTrackingClass const &whatever);
};

DynamicVectorClass<PrivateGameOptionsTrackingClass*> OptionsTracking;

bool PrivateGameOptionsTrackingClass::operator == (PrivateGameOptionsTrackingClass const &whatever)
{
	if (stricmp(UserName, whatever.UserName) == 0) {
		return(true);
	}
	return(false);
}			  

bool PrivateGameOptionsTrackingClass::operator != (PrivateGameOptionsTrackingClass const &whatever)
{
	if (stricmp(UserName, whatever.UserName) == 0) {
		return(false);
	}
	return(true);
}			  


bool Is_Options_Spammer(char *user_name, int &count)
{
	int index = -1;
	count = 0;
	for (int i=0 ; i<OptionsTracking.Count() ; i++) {
		if (stricmp(OptionsTracking[i]->UserName, user_name) == 0) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		
		/*
		** See if there are any old ones we can remove.
		*/
		for (int i=OptionsTracking.Count() - 1 ; i >= 0 ; i--) {
			if (OptionsTracking[i]->Times.Count() == 0) {
				delete OptionsTracking[i];
				OptionsTracking.Delete(i);
				index = i;
				break;
			}
		}
		
		
		PrivateGameOptionsTrackingClass *options = new PrivateGameOptionsTrackingClass;
		strncpy(options->UserName, user_name, 255);
		options->UserName[255] = 0;
		OptionsTracking.Add(options);
		index = OptionsTracking.Count() - 1;
	}

	unsigned int time = TIMEGETTIME();
	OptionsTracking[index]->Times.Add(time);

	unsigned int old_time = time - 4000;
	if (old_time < time) {
		
		/*
		** Remove times older than 5 secs.
		*/
		int c = OptionsTracking[index]->Times.Count();
		for (int i=0 ; i<c ; i++) {
			if (OptionsTracking[index]->Times[0] < old_time) {
				OptionsTracking[index]->Times.Delete(0);
			} else {
				break;
			}
		}
		count = OptionsTracking[index]->Times.Count();

		if (OptionsTracking[index]->Times.Count() > 16) {
			return(true);
		}
		
		
	} else {
		while (OptionsTracking.Count()) {
			delete OptionsTracking[0];
			OptionsTracking.Delete(0);
		}
	}
	
	return(false);
	
}			  


void ChatObserver::Kick_Spammer(WOL::User *wol_user)
{		
	/*
	** Get the IP of the spammer.
	*/
	const UserList& user_list = mOuter->GetUserList();
	const size_t count = user_list.size();
	unsigned int ip = 0;

	for (size_t index = 0; index < count; index++) {
		const RefPtr<UserData>& user = user_list[index];
		if (user.IsValid()) {
			WOL::User userdata = user->GetData();
			if (stricmp((char*)wol_user->name, (char*)userdata.name) == 0) {
				ip = userdata.ipaddr;
				break;
			}
		}
	}
	
	/*
	** Ban em, ban em all.
	*/
	for (size_t index = 0; index < count; index++) {
		const RefPtr<UserData>& user = user_list[index];
		if (user.IsValid()) {
			WOL::User userdata = user->GetData();
			if (userdata.ipaddr == ip) {
				WideStringClass widey((char*)userdata.name, true);

				const RefPtr<LoginInfo>& login = mOuter->GetCurrentLogin();

				WideStringClass myname(U_CHAR(""), true);
				if (login.IsValid()) {
					myname = login->GetNickname();
				}
				if (widey.Compare_No_Case(myname) != 0) {
					mOuter->BanUser(widey.Peek_Buffer(), true);
					mOuter->KickUser(widey.Peek_Buffer());
					WWDEBUG_SAY(("WOL: '%s' banned for options spamming\n", (char*)(char*)userdata.name));
				}
			}
		}
	}

	WideStringClass widey((char*)wol_user->name, true);
	mOuter->BanUser(widey.Peek_Buffer(), true);
	mOuter->KickUser(widey.Peek_Buffer());
	WWDEBUG_SAY(("WOL: '%s' banned for options spamming\n", (char*)wol_user->name));
}



/******************************************************************************
*
* NAME
*     ChatObserver::OnPrivateGameOptions
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPrivateGameOptions(HRESULT result, WOL::User* inUser, LPCSTR options)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateGameOptions '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Parameter check
	if ((inUser == NULL) || (options == NULL))
		{
		WWASSERT(inUser != NULL);
		WWASSERT(options != NULL);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: (time %d) PrivateGameOptions from '%s' : %s\n", TIMEGETTIME(), (char*)inUser->name, options));
	
	if (cNetwork::I_Am_Server()) {
	
		int count = 0;
		bool is_spammer = Is_Options_Spammer((char*)inUser->name, count);
		bool is_rginfo = (strstr(options, "RGINFO") == NULL) ? false : true;

		if (is_spammer) {
			if (count > 16 && count < 19) {
				Kick_Spammer(inUser);
			}
		} else {
			if (is_rginfo && count > 2) {
				WWDEBUG_SAY(("WOL: Ignoring RGINFO from '%s' \n", (char*)inUser->name));
			} else {
				GameOptionsMessage gameOptions(inUser, NULL, options, true);
				mOuter->NotifyObservers(gameOptions);
			}
		}
	} else {
		GameOptionsMessage gameOptions(inUser, NULL, options, true);
		mOuter->NotifyObservers(gameOptions);
	}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPublicGameOptions
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPublicGameOptions(HRESULT result, WOL::Channel* inChannel,
			WOL::User* inUser, LPCSTR options)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateGameOptions '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Parameter check
	if ((inChannel == NULL) || (inUser == NULL) || (options == NULL))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicGameOptions invalid parameters\n"));
		WWASSERT(inChannel != NULL);
		WWASSERT(inUser != NULL);
		WWASSERT(options != NULL);
		return S_OK;
		}

	// Only acknowledge options for the channel we are in.
	unichar_t inChannelName[64];
	u_mbtows(inChannelName, (char*)inChannel->name, sizeof(inChannel->name));
	RefPtr<ChannelData> channel = mOuter->FindChannel(inChannelName);

	if (!channel.IsValid() || (channel != mOuter->mCurrentChannel))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicGameOptions channel mismatch\n"));
		return S_OK;
		}

	// Make sure this is from a user we know about.
	unichar_t inUsername[64];
	u_mbtows(inUsername, (char*)inUser->name, sizeof(inUser->name));
	RefPtr<UserData> user = mOuter->FindUser(inUsername);

	if (!user.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicGameOptions unknown user\n"));
		WWASSERT(user.IsValid() && "Received game options from unknown user");
		return S_OK;
		}

	// Only handle options from other users.
	if (mOuter->mCurrentUser != user)
		{
		WWDEBUG_SAY(("WOL: PublicGameOptions [%s] from '%s': %s\n", (char*)inChannel->name, (char*)inUser->name, options));

#ifdef FREEDEDICATEDSERVER			
		int count = 0;
		if (Is_Options_Spammer((char*)inUser->name, count)) {
			if (count > 16 && count < 19) {
				Kick_Spammer(inUser);
			}
		}
#endif
		GameOptionsMessage gameOptions(inUser, inChannel, options, false);
		mOuter->NotifyObservers(gameOptions);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnGameStart
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnGameStart(HRESULT result, WOL::Channel* inChannel,
		WOL::User* inUsers, int gameID)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle error condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnGameStart '%s'\n", GetChatErrorString(result)));
		GameStartEvent gameStart(result);
		mOuter->NotifyObservers(gameStart);
		return S_OK;
		}

	// Ignore if parameters are invalid
	if (inChannel == NULL || inUsers == NULL)
		{
		WWDEBUG_SAY(("WOL: OnGameStart Invalid parameters\n"));
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: OnGameStart GameID %ld, Channel '%s'\n", gameID, (char*)inChannel->name));

	// Make sure the channel is the one we have joined.
	bool channelOkay = false;

	if (mOuter->mCurrentChannel.IsValid())
		{
		unichar_t chanName[64];
		u_mbtows(chanName, (const char*)inChannel->name, sizeof(inChannel->name));
		const WideStringClass& name = mOuter->mCurrentChannel->GetName();
		channelOkay = (name.Compare_No_Case(chanName) == 0);
		}

	// The game start is invalid if this is not the channel we expect.
	if (channelOkay == false)
		{
		WWDEBUG_SAY(("WOLERROR: OnGameStart channel mismatch\n"));
		GameStartEvent gameStart(E_INVALIDARG);
		mOuter->NotifyObservers(gameStart);
		return S_OK;
		}

	// Process the game start
	UserList userList;
	WOL::User* curUser = inUsers;

	while (curUser)
		{
		WWDEBUG_SAY(("WOL: OnGameStart Player '%s'\n", (char*)curUser->name));
		unichar_t name[64];
		u_mbtows(name, (const char*)curUser->name, sizeof(curUser->name));

		// Get user from current list. If not in list then create new user.
		RefPtr<UserData> user = mOuter->FindUser(name);

		if (!user.IsValid())
			{
			user = UserData::Create(*curUser);
			}

		if (user.IsValid())
			{
			userList.push_back(user);
			}

		curUser = curUser->next;
		}

	GameStartEvent gameStart(mOuter->mCurrentChannel, userList, gameID);
	mOuter->NotifyObservers(gameStart);
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserKick
*
* DESCRIPTION
*     Handle response to user kick request.
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserKick(HRESULT result, WOL::Channel* inChannel,
			WOL::User* inUser, [[maybe_unused]] WOL::User* kicker)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Handle error condition
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserKick '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Check parameters
	if ((inChannel == NULL) || (inUser == NULL))
		{
		WWASSERT(inChannel);
		WWASSERT(inUser);
		WWDEBUG_SAY(("WOLWARNING: Cannot handle user kick because CHANNEL or USER is NULL\n"));
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: '%s' has been kicked from channel '%s' by '%s'\n",
			(char*)inUser->name, (char*)inChannel->name, (char*)kicker->name));

	// If we are not connected to a channel then ignore.
	if (!mOuter->mCurrentChannel.IsValid())
		{
		WWDEBUG_SAY(("WOLWARNING: OnUserKick not in a channel\n"));
		return S_OK;
		}

	// If the channel is not the channel we are connected to then ignore.
	unichar_t channelName[64];
	u_mbtows(channelName, (const char*)inChannel->name, sizeof(inChannel->name));
	const WideStringClass& curChannelName = mOuter->mCurrentChannel->GetName();

	if (curChannelName.Compare_No_Case(channelName) != 0)
		{
		WWDEBUG_SAY(("WOLWARNING: OnUserKick channel mismatch\n"));
		return S_OK;
		}

	// Decrement user count for this channel
	WWASSERT(mOuter->mCurrentChannel->GetData().currentUsers > 0);
	mOuter->mCurrentChannel->GetData().currentUsers--;

	// If current user is kicked then disconnect from the channel and clear the user list.
	if (inUser->flags & CHAT_USER_MYSELF)
		{
		mOuter->mCurrentChannelStatus = ChannelKicked;

		// Release the current channel since we have been kicked
		RefPtr<ChannelData> channel = mOuter->mCurrentChannel;
		mOuter->mCurrentChannel.Release();

		// Notify others that we have been kicked from the channel.
		ChannelEvent event(ChannelKicked, channel);
		mOuter->NotifyObservers(event);

		// Clear the user list then notify others that the user list is now empty.
		mOuter->mUsers.clear();
		mOuter->NotifyObservers(mOuter->mUsers);
		}
	else
		{
		// Remove kicked user from the userlist
		unichar_t username[64];
		u_mbtows(username, (char*)inUser->name, sizeof(inUser->name));

		RefPtr<UserData> kickedUser = RemoveUserInList(username, mOuter->mUsers);

		// Notify others that the user was kicked
		if (kickedUser.IsValid())
			{
			UserEvent event(UserEvent::Kicked, kickedUser);
			mOuter->NotifyObservers(event);
			}
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserIP
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserIP(HRESULT result, WOL::User* user)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserIP '%s'\n", GetChatErrorString(result)));

		if (user)
			{
			UserIPEvent event(UserIPEvent::Error, *user);
			mOuter->NotifyObservers(event);
			}
	
		return S_OK;
		}

		if (user)
			{
			UserIPEvent event(UserIPEvent::GotIP, *user);
			mOuter->NotifyObservers(event);
			}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnFind
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnFind(HRESULT result, WOL::Channel* wolChannel)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnFind '%s'\n", GetChatErrorString(result)));
		UserEvent event(UserEvent::Located, RefPtr<UserData>());
		mOuter->NotifyObservers(event);
		mOuter->mLocatingUser.Release();
		return S_OK;
		}

	const RefPtr<UserData>& user = mOuter->mLocatingUser;

	if (user.IsValid())
		{
		UserLocation location = USERLOCATION_UNKNOWN;
		RefPtr<ChannelData> userChannel;

		if (wolChannel)
			{
			location = USERLOCATION_IN_CHANNEL;

			// Look for the channel in our existing list
			userChannel = mOuter->FindChannel((const char*)wolChannel->name);

			// Create the channel if we don't already have it.
			if (userChannel.IsValid() == false)
				{
				userChannel = ChannelData::Create(*wolChannel);
				}
			}
		else
			{
			switch (result)
				{
				default:
				case CHAT_S_FIND_NOCHAN:
					location = USERLOCATION_NO_CHANNEL;
				break;

				case CHAT_S_FIND_NOTHERE:
					location = USERLOCATION_OFFLINE;
					break;

				case CHAT_S_FIND_OFF:
					location = USERLOCATION_HIDING;
					break;
				}
			}

		user->SetLocation(location);
		user->SetChannel(userChannel);

		// Notify others about the users location.
		UserEvent event(UserEvent::Located, user);
		mOuter->NotifyObservers(event);
		}

	mOuter->mLocatingUser.Release();

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPageSend
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPageSend(HRESULT result)
	{
	PageSendStatus status = PAGESEND_ERROR;

	switch (result)
		{
		default:
		case E_FAIL:
			status = PAGESEND_ERROR;
			break;

		case CHAT_S_PAGE_NOTHERE:
			status = PAGESEND_OFFLINE;
			break;

		case CHAT_S_PAGE_OFF:
			status = PAGESEND_HIDING;
			break;

		case S_OK:
			status = PAGESEND_SENT;
			break;
		}

	mOuter->NotifyObservers(status);
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPaged
*
* DESCRIPTION
*     Handle being paged by user.
*
* INPUTS
*     Result - Error / status code.
*     User   - User who initiated page.
*     Text   - Page message
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPaged(HRESULT result, WOL::User* user, LPCSTR text)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPaged '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	if (user && text)
		{
		PageMessage page((const char*)&user->name[0], text);
		mOuter->NotifyObservers(page);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnServerBannedYou
*
* DESCRIPTION
*      The login has been banned from the server.
*
* INPUTS
*     Result     - Error / status code.
*     LiftedTime - Time banned will be lifted.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnServerBannedYou(HRESULT, WOL::time_t liftedTime)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: OnServerBannedYou\n"));

	ServerError event(CHAT_E_BANNED, "WOL_BANNED", liftedTime);
	mOuter->NotifyObservers(event);
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelBan
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelBan(HRESULT result, LPCSTR username, int banned)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelBan '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: '%s' has been %s\n", username, banned ? "banned" : "unbanned"));

	if (banned && username)
		{
		unichar_t name[64];
		u_mbtows(name, username, 63);

		RefPtr<UserData> user = mOuter->GetUserOrBuddy(name);

		if (!user.IsValid())
			{
			user = UserData::Create(name);
			}

		UserEvent event(UserEvent::Banned, user);
		mOuter->NotifyObservers(event);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserFlags
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserFlags(HRESULT result, LPCSTR username, unsigned int flags, unsigned int mask)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserFlags '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Find the user and update their flag settings
	unichar_t name[64];
	u_mbtows(name, username, 63);
	RefPtr<UserData> user = mOuter->GetUserOrBuddy(name);

	if (user.IsValid())
		{
		unsigned int userflags = user->GetData().flags;
		user->GetData().flags = ((userflags & mask) | flags);

		UserEvent event(UserEvent::NewData, user);
		mOuter->NotifyObservers(event);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnSquadInfo
*
* DESCRIPTION
*     Handle receipt of squad information.
*
* INPUTS
*     Result   - Error / status code.
*     WOLSquad - WOLAPI squad structure
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnSquadInfo(HRESULT result, unsigned int /* squadID */, WOL::Squad* inSquad)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	// Requested squad not found.
	if (CHAT_E_NONESUCH == result)
		{
		if (!mOuter->mSquadPending.empty())
			{
			RefPtr<SquadData> squad;
			ProcessSquadRequest(squad);
			return S_OK;
			}
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnSquadInfo '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	WOL::Squad* wolSquad = inSquad;

	while (wolSquad)
		{
//		WWDEBUG_SAY(("WOL: OnSquadInfo ID %ld, Name '%s', Abbr '%s'\n", inSquad->id, inSquad->name, inSquad->abbreviation));

		// If the squad is already in the local cache then update it.
		// Otherwise create and cache a local copy if this squad information.
		RefPtr<SquadData> squad = SquadData::FindByID(wolSquad->id);

		if (squad.IsValid())
			{
			squad->UpdateData(*wolSquad);
			}
		else
			{
			squad = SquadData::Create(*wolSquad);

			// Automatically request ladder information for this squad.
			if (squad.IsValid())
				{
				unichar_t abbr[64];
				u_mbtows(abbr, squad->GetAbbr(), 64);
				abbr[63] = 0;

				mOuter->RequestLadderInfo(abbr, LadderType_Clan);
				}
			}

		if (squad.IsValid())
			{
			// Assign this squad to any users that are members.
			AssignSquadToUsers(mOuter->GetUserList(), squad);
			AssignSquadToUsers(mOuter->GetBuddyList(), squad);

			ProcessSquadRequest(squad);
			}

		wolSquad = wolSquad->next;
		}

	return S_OK;
	}


void ChatObserver::AssignSquadToUsers(const UserList& users, const RefPtr<SquadData>& squad)
	{
	// Assign the squad to users who are its members.
	const size_t userCount = users.size();

	for (size_t index = 0; index < userCount; ++index)
		{
		const RefPtr<UserData>& user = users[index];

		if (user.IsValid() && (user->GetSquadID() == squad->GetID()))
			{
			user->SetSquad(squad);

			// Notify others that this user received squad information
			UserEvent event(UserEvent::SquadInfo, user);
			mOuter->NotifyObservers(event);
			}
		}
	}


void ChatObserver::ProcessSquadRequest(const RefPtr<SquadData>& squad)
	{
	// Process any particular squad requests that have been made.
	if (!mOuter->mSquadPending.empty())
		{
		// Get the first pending squad request from the queue and see if it's a request by ID or Name.
		const WideStringClass& pending = mOuter->mSquadPending[0];

		if (squad.IsValid())
			{
			WWDEBUG_SAY(("WOL: Squad %s found for %S\n", squad->GetAbbr(), (const unichar_t*)pending));

			// First character of user names cannot be numbers. Therefore if it is a number
			// then process the request by ID. Otherwise process the request by name.
			unichar_t firstChar = pending[0];

			if (iswdigit(firstChar))
				{
				unsigned int pendingID;	
				u_sscanf_u(pending, U_CHAR("%u"), &pendingID);

				if (squad->GetID() == pendingID)
					{
					SquadEvent squadEvent(NULL, squad);
					mOuter->NotifyObservers(squadEvent);
					}
				else
					{
					WWDEBUG_SAY(("WOLWARNING: OnSquadInfo PendingID '%u' doesn't match CurrentID '%u'\n", pendingID, squad->GetID()));
					}
				}
			else
				{
				RefPtr<UserData> user = mOuter->GetUserOrBuddy(pending);

				if (user.IsValid())
					{
					user->SetSquad(squad);
					UserEvent event(UserEvent::SquadInfo, user);
					mOuter->NotifyObservers(event);
					}

				// Must be a user name that we requested by
				SquadEvent squadEvent(pending, squad);
				mOuter->NotifyObservers(squadEvent);
				}
			}
		else
			{
			WWDEBUG_SAY(("WOL: Squad not found for '%S'\n", (const unichar_t*)pending));
			}

		Session::SquadRequestColl::iterator first = mOuter->mSquadPending.begin();
		mOuter->mSquadPending.erase(first);
		}
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserLocale
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserLocale(HRESULT result, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserLocale '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	WOL::User* wolUser = inUsers;

	while (wolUser)
		{
		WWDEBUG_SAY(("WOL: OnUserLocale '%s' %ld\n", wolUser->name, wolUser->locale));

		WOL::Locale locale = wolUser->locale;
		WWASSERT(locale >= WOL::LOC_UNKNOWN && locale <= WOL::LOC_TURKEY && "OnUserLocale Locale out of range!");

		// Update the users locale.
		unichar_t username[64];
		u_mbtows(username, (const char*)wolUser->name, sizeof(wolUser->name));
		RefPtr<UserData> user = mOuter->GetUserOrBuddy(username);

		if (user.IsValid() && (user->GetLocale() != locale))
			{
			// If the current user's requested locale is different than the player's
			// login locale then update the locale in the WWOnline database to match
			// the login locale.
			if (mOuter->IsCurrentUser(user))
				{
				const RefPtr<LoginInfo>& login = mOuter->GetCurrentLogin();
				WWASSERT(login.IsValid() && "OnUserLocale: Login not set");

				if (login.IsValid())
					{
					WOL::Locale loginLocale = login->GetLocale();

					if (locale != loginLocale)
						{
						mOuter->ChangeCurrentUserLocale(loginLocale);
						}
					}
				}

			// Update the locale for this user.
			user->SetLocale(locale);
			UserEvent event(UserEvent::Locale, user);
			mOuter->NotifyObservers(event);
			}

		wolUser = wolUser->next;
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnSetLocale
*
* DESCRIPTION
*     Handle response to IChat::RequestSetLocale
*
* INPUTS
*     Result - Error / status code.
*     Locale - New locale for current user.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnSetLocale(HRESULT result, WOL::Locale locale)
	{
	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnSetLocale '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	WWASSERT(mOuter && "Session not initialized");

	// Verify that the locale is what we expected.
	const RefPtr<LoginInfo>& login = mOuter->GetCurrentLogin();
	WWASSERT(login.IsValid() && "OnSetLocale: Invalid Login");

	if (login.IsValid() && (login->GetLocale() != locale))
		{
		WWDEBUG_SAY(("WOLERROR: OnSetLocale - Mismatch with login locale\n"));
		WWASSERT(login->GetLocale() == locale && "OnSetLocale: Mismatch with login locale");
		}

	// Set the current users locale to the newly requested one.
	RefPtr<UserData> user = mOuter->GetCurrentUser();

	if (user.IsValid())
		{
		user->SetLocale(locale);
		UserEvent event(UserEvent::Locale, user);
		mOuter->NotifyObservers(event);
		}
	else
		{
		WWDEBUG_SAY(("WOLERROR: OnSetLocale Invalid current user\n"));
		WWASSERT(user.IsValid() && "OnSetLocale Invalid current user");
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnUserTeam
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnUserTeam(HRESULT result, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnUserTeam '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	WOL::User* wolUser = inUsers;

	while (wolUser)
		{
		WWDEBUG_SAY(("WOL: OnUserTeam '%s' %ld\n", wolUser->name, wolUser->locale));

		// Update the users locale.
		unichar_t username[64];
		u_mbtows(username, (const char*)wolUser->name, sizeof(wolUser->name));
		RefPtr<UserData> user = mOuter->GetUserOrBuddy(username);

		if (user.IsValid())
			{
			user->SetTeam(wolUser->team);
			UserEvent event(UserEvent::NewData, user);
			mOuter->NotifyObservers(event);
			}

		wolUser = wolUser->next;
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnSetTeam
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnSetTeam(HRESULT result, int team)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnSetTeam '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	RefPtr<UserData> user = mOuter->GetCurrentUser();

	if (user.IsValid())
		{
		user->SetTeam(team);
		UserEvent event(UserEvent::NewData, user);
		mOuter->NotifyObservers(event);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnBuddyList
*
* DESCRIPTION
*
* INPUTS
*     Result  - Result / status code.
*     Buddies - User list of buddies.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnBuddyList(HRESULT result, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnBuddyList '%s'\n", GetChatErrorString(result)));
		UserList buddies;
		BuddyEvent event(BuddyEvent::NewList, buddies);
		mOuter->NotifyObservers(event);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: OnBuddyList\n"));

	// Generate new list
	UserList buddies;

	WOL::User* wolUser = inUsers;

	while (wolUser)
		{
		WideStringClass name(64, true);
		name = (char*)wolUser->name;

		// First look for buddy in our old list.
		RefPtr<UserData> buddy = mOuter->FindBuddy(name);

		// Create a new buddy if we don't already know about him.
		if (buddy.IsValid() == false)
			{
			buddy = UserData::Create(*wolUser);
			}

		if (buddy.IsValid())
			{
			buddies.push_back(buddy);
			}

		wolUser = wolUser->next;
		}

	mOuter->mBuddies = buddies;
	BuddyEvent event(BuddyEvent::NewList, mOuter->mBuddies);
	mOuter->NotifyObservers(event);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnBuddyAdd
*
* DESCRIPTION
*
* INPUTS
*     Result  - Result / status code.
*     Buddies - User list of buddies added.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnBuddyAdd(HRESULT result, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnBuddyAdd '%s'\n", GetChatErrorString(result)));
		UserList addList;
		BuddyEvent event(BuddyEvent::Added, addList);
		mOuter->NotifyObservers(event);
		return S_OK;
		}

	// Generate new list
	UserList addList;
	WOL::User* wolUser = inUsers;

	while (wolUser)
		{
		WideStringClass name(64, true);
		name = (char*)wolUser->name;

		// First look for buddy in our old list.
		RefPtr<UserData> buddy = mOuter->FindBuddy(name);

		// Create a new buddy if we don't already know about him.
		if (buddy.IsValid() == false)
			{
			buddy = UserData::Create(*wolUser);

			if (buddy.IsValid())
				{
				mOuter->mBuddies.push_back(buddy);
				addList.push_back(buddy);
				}
			}

		wolUser = wolUser->next;
		}

	if (addList.size() > 0)
		{
		BuddyEvent event(BuddyEvent::Added, addList);
		mOuter->NotifyObservers(event);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnBuddyDelete
*
* DESCRIPTION
*
* INPUTS
*     Result  - Result / status code.
*     Buddies - User list of buddies removed
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnBuddyDelete(HRESULT result, WOL::User* inUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnBuddyDelete '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	// Generate new list
	UserList deleteList;

	WOL::User* wolUser = inUsers;

	while (wolUser)
		{
		WideStringClass name(64, true);
		name = (char*)wolUser->name;

		RefPtr<UserData> buddy = RemoveUserInList(name, mOuter->mBuddies);

		if (buddy.IsValid())
			{
			deleteList.push_back(buddy);
			}

		wolUser = wolUser->next;
		}

	if (deleteList.size() > 0)
		{
		BuddyEvent event(BuddyEvent::Deleted, deleteList);
		mOuter->NotifyObservers(event);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPublicUnicodeMesssage
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPublicUnicodeMessage(HRESULT result, WOL::Channel*,
		WOL::User* user, const unsigned short* message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicUnicodeMessage '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, (const unichar_t *)message, false, false);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPrivateUnicodeMessage
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPrivateUnicodeMessage(HRESULT result, WOL::User* user,
		const unsigned short* message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateUnicodeMessage '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, (const unichar_t *)message, true, false);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPrivateUnicodeAction
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPrivateUnicodeAction(HRESULT result, WOL::User* user,
		const unsigned short* message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPrivateUnicodeAction '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, (const unichar_t *)message, true, true);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPublicUnicodeAction
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPublicUnicodeAction(HRESULT result, WOL::Channel*,
		WOL::User* user, const unsigned short* message)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPublicUnicodeAction '%s'\n", GetChatErrorString(result)));
		}
	else
		{
		ChatMessage msg(user, (const unichar_t *)message, false, true);
		mOuter->NotifyObservers(msg);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnPagedUnicode
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnPagedUnicode(HRESULT result, WOL::User* user, const unsigned short* text)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnPagedUnicode '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	if (user && text)
		{
		unichar_t name[64];
		u_mbtows(name, (const char*)&user->name[0], sizeof(user->name));
		PageMessage page(name, (const unichar_t *)text);
		mOuter->NotifyObservers(page);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnServerTime
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnServerTime(HRESULT result, WOL::time_t server_time)
	{	
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnServerTime '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	mOuter->mServerTime = server_time;
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnInsiderStatus
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnInsiderStatus(HRESULT result, WOL::User* wolUsers)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnInsiderStatus '%s'\n", GetChatErrorString(result)));
		return S_OK;
		}

	if (wolUsers != NULL)
		{
		//	Get the name of the currently logged in user
		RefPtr<UserData> curr_user = mOuter->GetCurrentUser();
		
		// Convert the WOL user's name to a wide character string
		WideStringClass wide_name(0, true);
		wide_name.Convert_From((const char*)wolUsers[0].name);

		//	Check to see if we got information back about the current user
		if (curr_user->GetName().Compare_No_Case(wide_name) == 0)
			{
			mOuter->mIsInsider = wolUsers[0].squadname[0] != '0';
			}
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnSetLocalIP
*
* DESCRIPTION
*
* INPUTS
*     Result - Error / status code.
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnSetLocalIP(HRESULT, LPCSTR)
	{
	WWDEBUG_SAY(("WOLWARNING: OnSetLocalIP not implemented\n"));
	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelBegin
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelListBegin(HRESULT result)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelListBegin '%s'\n", GetChatErrorString(result)));
		ChannelList empty;
		ChannelListEvent error(ChannelListEvent::Error, empty, -1);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: OnChannelListBegin %d\n", mOuter->mRequestedChannelType));

	mOuter->mIncommingChatChannels.clear();
	mOuter->mIncommingGameChannels.clear();
	WWASSERT(mOuter->mRequestedChannelType >= 0);

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelListEntry
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelListEntry(HRESULT result, WOL::Channel* wolChannel)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result) || (wolChannel == NULL) || (wolChannel->type != mOuter->mRequestedChannelType))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelListEntry '%s'\n", GetChatErrorString(result)));
		ChannelList empty;
		ChannelListEvent error(ChannelListEvent::Error, empty, -1);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: ChannelEntry '%s:%ld'\n", (const char*)wolChannel->name, wolChannel->type));

	// Get code for game channels
	RefPtrConst<Product> product = Product::Current();
	WWASSERT(product.IsValid() && "WOLProduct not initialized");
	int gameCode = product->GetGameCode();

	ChannelList* list = NULL;

	if (wolChannel->type == 0)
		{
		list = &mOuter->mChatChannels;
		}
	else if (wolChannel->type == gameCode)
		{
		list = &mOuter->mGameChannels;
		}
	else
		{
		return S_OK;
		}

	// Process the channel
	ChannelList inList;
	ChannelList outList;

	// Check if the channel exists in the current list. If the channel exists in
	// the current list then update the channel. Otherwise the channel is new,
	// therefore we need to add it.
	ChannelList::iterator iter = FindChannelNode(*list, (const char*)wolChannel->name);

	// If the channel already exists then just update it.
	if (iter != list->end())
		{
		// Update the channel data.
		RefPtr<ChannelData> channel = *iter;
		WWASSERT(channel.IsValid());
		channel->UpdateData(*wolChannel);

		// Channels arrive alphabetically. Therefore if there are any channels
		// in the list before this one then those channels are no longer around.
		// So move those channels into the out list.
		if (iter != list->begin())
			{
			#ifdef _DEBUG
			ChannelList::iterator remIter = list->begin();

			while (remIter != iter)
				{
				WWDEBUG_SAY(("WOL: Removing Channel '%S'\n", (*remIter)->GetName()));
				remIter++;
				}
			#endif

			ChannelList::iterator end = outList.end();
			outList.splice(end, *list, list->begin(), iter);
			}

		// Move the channel into the new list.
		ChannelList::iterator end = inList.end();
		inList.splice(end, *list, iter);
		}
	else
		{
		// Create the new channel and add it to the new list.
		RefPtr<ChannelData> channel = ChannelData::Create(*wolChannel);
		WWASSERT(channel.IsValid());
		inList.push_back(channel);
		}

	// Notify others about the channel changes
	if (inList.empty() == false)
		{
		ChannelListEvent inEvent(ChannelListEvent::Update, inList, wolChannel->type);
		mOuter->NotifyObservers(inEvent);
		}

	if (outList.empty() == false)
		{
		ChannelListEvent outEvent(ChannelListEvent::Remove, outList, wolChannel->type);
		mOuter->NotifyObservers(outEvent);
		}

	if (wolChannel->type == 0)
		{
		ChannelList::iterator end = mOuter->mIncommingChatChannels.end();
		mOuter->mIncommingChatChannels.splice(end, inList);
		}
	else
		{
		ChannelList::iterator end = mOuter->mIncommingGameChannels.end();
		mOuter->mIncommingGameChannels.splice(end, inList);
		}

	return S_OK;
	}


/******************************************************************************
*
* NAME
*     ChatObserver::OnChannelListEnd
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     Error / Status result
*
******************************************************************************/

STDMETHODIMP ChatObserver::OnChannelListEnd(HRESULT result)
	{
	if (mOuter == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Session not initialized\n"));
		WWASSERT(mOuter && "Session not initialized");
		return S_OK;
		}

	if (FAILED(result))
		{
		WWDEBUG_SAY(("WOLERROR: OnChannelListEnd '%s'\n", GetChatErrorString(result)));
		ChannelList empty;
		ChannelListEvent error(ChannelListEvent::Error, empty, -1);
		mOuter->NotifyObservers(error);
		return S_OK;
		}

	WWDEBUG_SAY(("WOL: OnChannelListEnd %d\n", mOuter->mRequestedChannelType));
	WWASSERT(mOuter->mRequestedChannelType >= 0);

	// Replace lists
	ChannelList* channelList = NULL;
	ChannelList* newList = NULL;
	int channelType = 0;

	if (mOuter->mRequestedChannelType == 0)
		{
		channelList = &mOuter->mChatChannels;
		newList = &mOuter->mIncommingChatChannels;
		}
	else
		{
		RefPtr<Product> product = Product::Current();
		WWASSERT(product.IsValid());
		channelType = product->GetGameCode();

		channelList = &mOuter->mGameChannels;
		newList = &mOuter->mIncommingGameChannels;
		}

	WWASSERT(channelList != NULL);
	WWASSERT(newList != NULL);

	// Any leftover channels were not in the new list. So, send a
	// notification that these channels are being removed then remove them.
	if (!channelList->empty())
		{
		ChannelListEvent outEvent(ChannelListEvent::Remove, *channelList, channelType);
		mOuter->NotifyObservers(outEvent);
		channelList->clear();
		}
		
	// Swap in the new channel list
	channelList->swap(*newList);

	// Send notification about the new channel list.
	ChannelListEvent listEvent(ChannelListEvent::NewList, *channelList, channelType);
	mOuter->NotifyObservers(listEvent);

	mOuter->mRequestedChannelType = -1;

	return S_OK;
	}

} // namespace WWOnline
