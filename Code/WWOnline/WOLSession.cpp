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
* NAME
*     $Archive: /Commando/Code/WWOnline/WOLSession.cpp $
*
* DESCRIPTION
*     WOLSession is the entryway to Westwood Online. An object of this type
*     must exist in order to do anything WOL related. There should only be one
*     instance of this; retrieve it via GetInstance.
*
* PROGRAMMER
*     Steve Clinard & Denzil E. Long, Jr.
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 79 $
*     $Modtime: 2/08/02 11:31a $
*
******************************************************************************/

#include "atlbase_compat.h"
#include "WOLSession.h"
#include "WOLChatObserver.h"
#include "WOLNetUtilObserver.h"
#include "WOLProduct.h"
#include "WOLConnect.h"
#include "WOLServer.h"
#include "WaitCondition.h"
#include "WOLErrorUtil.h"
#include <wwdebug/wwdebug.h>
#include <stdlib.h>
#include <algorithm>

#include "systimer.h"

namespace WWOnline {

RefPtr<Session> Session::_mInstance;

/******************************************************************************
*
* NAME
*     Session::GetInstance
*
* DESCRIPTION
*     Retrieve instance to Westwood Online session
*
* INPUTS
*     Create - True if session should be created.
*
* RESULT
*     Session - Session instance
*
******************************************************************************/

RefPtr<Session> Session::GetInstance(bool okToCreate)
	{
#ifdef _WIN64
		WWDEBUG_SAY(("WOL: Disabled in 64-bit builds (missing 64-bit WOLAPI).\n"));
		return RefPtr<Session>();
#endif
	if (okToCreate && !_mInstance.IsValid())
		{
		_mInstance = new Session;

		if (_mInstance->FinalizeCreate() == false)
			{
			_mInstance.Release();
			}
		}

	return _mInstance;
	}


/******************************************************************************
*
* NAME
*     Session::Session
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

Session::Session() :
		mChatCookie(0),
		mNetUtilCookie(0),
		mCurrentConnectionStatus(ConnectionDisconnected),
		mAutoRequestFlags(0),
		mLadderPending(0),
		mPingsPending(0),
		mPingEnable(1),
		mLastUserDataRequestTime(0),
		mRequestedChannelType(-1),
		mRequestingServerList(false),
		mIgnoreServerLists(false),
		mIsInsider(false),
		mServerTime(0)
	{
	WWDEBUG_SAY(("WOL: Session instantiated\n"));

	// Initailize COM
	HRESULT hr = CoInitialize(NULL);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: CoInitialize() failed!\n"));
		}

	WWASSERT(SUCCEEDED(hr) && "CoInitialize() failed!");
	}


/******************************************************************************
*
* NAME
*     Session::FinalizeCreate
*
* DESCRIPTION
*     Finalize session creation
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if successful.
*
******************************************************************************/

bool Session::FinalizeCreate(void)
	{
	WWDEBUG_SAY(("WOL: Session Create\n"));
	Reset();

	//---------------------------------------------------------------------------
	// Create chat object
	//---------------------------------------------------------------------------
	WWDEBUG_SAY(("WOL: Creating IID_IChat object\n"));
	WOL::IChat* chatObject = NULL;
	HRESULT hr = CoCreateInstance(WOL::CLSID_Chat, NULL, CLSCTX_INPROC_SERVER,
			WOL::IID_IChat, (void**)&chatObject);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: Failed to create IID_IChat\n"));
		return false;
		}

	if (chatObject)
		{
		mChat = chatObject;

		// Create chat events observer
		WWDEBUG_SAY(("WOL: Creating chat events observer\n"));
		CComPtr<ChatObserver> chatEvents;
		chatEvents.Attach(new ChatObserver);

		if (chatEvents == NULL)
			{
			WWDEBUG_SAY(("WOLERROR: Failed to create IChatEvents observer\n"));
			return false;
			}

		chatEvents->Init(*this);

		hr = AtlAdvise(chatEvents.p, chatEvents, WOL::IID_IChatEvent, &mChatCookie);

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: Failed to advise IChatEvents observer\n"));
			return false;
			}

		mChatEvents = chatEvents;
		}

	//---------------------------------------------------------------------------
	// Create netutil object
	//---------------------------------------------------------------------------
	WWDEBUG_SAY(("WOL: Creating IID_INetUtil object\n"));

	WOL::INetUtil* utilObject = NULL;
	hr = CoCreateInstance(WOL::CLSID_NetUtil, NULL, CLSCTX_INPROC_SERVER,
			WOL::IID_INetUtil, (void **)&utilObject);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: Failed to create IID_INetUtil\n"));
		return false;
		}

	if (utilObject)
		{
		mNetUtil = utilObject;

		// Create net utility events observer
		WWDEBUG_SAY(("WOL: Creating netutil events observer\n"));
		CComPtr<NetUtilObserver> utilEvents;
		utilEvents.Attach(new NetUtilObserver);

		if (utilEvents == NULL)
			{
			WWDEBUG_SAY(("WOLERROR: Failed to create INetUtilEvents observer\n"));
			return false;
			}

		utilEvents->Init(*this);

		hr = AtlAdvise(mNetUtil, utilEvents, WOL::IID_INetUtilEvent, &mNetUtilCookie);

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: Failed to advise INetUtilEvents observer\n"));
			return false;
			}

		mNetUtilEvents = utilEvents;
		}

	//---------------------------------------------------------------------------
	// Setup for current product
	//---------------------------------------------------------------------------
	WWDEBUG_SAY(("WOL: Setting product SKU\n"));
	RefPtrConst<Product> product = Product::Current();

	if (!product.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: WOLProduct not initialized\n"));
		return false;
		}

	const char* regPath = product->GetRegistryPath();
	hr = mChat->SetAttributeValue("RegPath", regPath);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: SetAttributeValue(RegPath) HRESULT = %s\n", GetChatErrorString(hr)));
		return false;
		}

	unsigned int sku = product->GetSKU();
	hr = mChat->SetProductSKU(sku);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: SetProductSKU() HRESULT = %s\n", GetChatErrorString(hr)));
		return false;
		}

	// Enable OnChannelTopic() to be called after OnChannelJoin()
	hr = mChat->SetAttributeValue("AutoTopic", "true");

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: SetAttributeValue(AutoTopic) HRESULT = %s\n", GetChatErrorString(hr)));
		return false;
		}

	mPingRequests.reserve(8);
	mUsers.reserve(32);

	return true;
	}


/******************************************************************************
*
* NAME
*     Session::~Session
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

Session::~Session()
	{
	WWDEBUG_SAY(("WOL: Session destructing\n"));
	Reset();

	if (mIGRObject)
		{
		WWDEBUG_SAY(("WOL: Releasing IID_IIGROptions object\n"));
		mIGRObject.Release();
		}

	if (mNetUtil)
		{
		WWDEBUG_SAY(("WOL: Releasing IID_INetUtil object\n"));
		AtlUnadvise(mNetUtil, WOL::IID_INetUtilEvent, mNetUtilCookie);
		mNetUtil.Release();
		}

	if (mChat)
		{
		WWDEBUG_SAY(("WOL: Releasing IID_IChat object\n"));
		AtlUnadvise(mChat, WOL::IID_IChatEvent, mChatCookie);
		mChat.Release();
		}

	CoUninitialize();
	}


/******************************************************************************
*
* NAME
*     Session::ReleaseReference
*
* DESCRIPTION
*     Release reference count
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::ReleaseReference(void)
	{
	RefCounted::ReleaseReference();

	// If there is only the singleton reference then release the object.
	if (ReferenceCount() == 1)
		{
		_mInstance.Release();
		}
	}


/******************************************************************************
*
* NAME
*     Session::Reset
*
* DESCRIPTION
*     Reset the WWOnline session
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::Reset(void)
	{
	ClearServers();

	SquadData::Reset();
	mChatChannels.clear();
	mGameChannels.clear();
	mPatchFiles.clear();
	mUsers.clear();
	mBuddies.clear();
	mLocatePendingUsers.clear();
	mLocatingUser.Release();
	}


/******************************************************************************
*
* NAME
*     Session::Process
*
* DESCRIPTION
*     Perform periodic processing
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

bool Session::Process(void)
	{
	if (mCurrentConnectionStatus == ConnectionConnected)
		{
		MakeLocateUserRequests();
		}

	DWORD theTime = TIMEGETTIME();

	if (theTime < mLastUserDataRequestTime)
		{
		mLastUserDataRequestTime = theTime;
		}

	if (theTime >= (mLastUserDataRequestTime + 1000))
		{
		mLastUserDataRequestTime = theTime;

		MakeSquadRequests();
		MakeLocaleRequests();
		MakeTeamRequests();
		MakeLadderRequests();
		}

	MakePingRequests();

	mChat->PumpMessages();
	mNetUtil->PumpMessages();

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::GetNewServerList
*
* DESCRIPTION
*     Get a new server list.
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Wait condition to process for serverlist.
*
******************************************************************************/

RefPtr<WaitCondition> Session::GetNewServerList(void)
	{
	return ServerListWait::Create(this);
	}


/******************************************************************************
*
* NAME
*     Session::RequestServerList
*
* DESCRIPTION
*     Submit a request for server list.
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if request successful.
*
******************************************************************************/

bool Session::RequestServerList(bool ignore)
	{
	RefPtrConst<Product> product(Product::Current());

	if (!product.IsValid())
		{
		WWASSERT(product.IsValid() && "WOLERROR: Product not initialized.");
		return false;
		}

	mIgnoreServerLists = ignore;
	mRequestingServerList = !ignore;

	unsigned int sku = product->GetLanguageSKU();
	unsigned int version = product->GetVersion();

	HRESULT hr = mChat->RequestServerList(sku, version, "NoUser", "NoPass", 30000);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: RequestServerList() HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::LoginServer
*
* DESCRIPTION
*     Log user onto server.
*
* INPUTS
*     Server - Server to logon to.
*     Login  - Login information
*
* RESULT
*     Wait - Wait condition to process for logging in.
*
******************************************************************************/

RefPtr<WaitCondition> Session::LoginServer(const RefPtr<IRCServerData>& server,
			const RefPtr<LoginInfo>& login)
	{
	if (server.IsValid())
		{
		WWDEBUG_SAY(("WOL: LoginServer connecting to '%s'\n", server->GetName()));
		}
	else
		{
		WWDEBUG_SAY(("WOL: LoginServer disconnecting\n"));
		}

	//---------------------------------------------------------------------------
	// If this is the current server, check state of connection
	//---------------------------------------------------------------------------
	if (mCurrentServer == server)
		{
		if (server.IsValid())
			{
			// If already connected to this server, just return with success.
			// If connecting then wait for connection to finish.
			if (ConnectionConnected == mCurrentConnectionStatus)
				{
				WWDEBUG_SAY(("WOL: LoginServer already connected.\n"));
				RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CONNECTING"));
				wait->EndWait(WaitCondition::ConditionMet, WOLSTRING("WOL_CONNECTED"));
				return wait;
				}
			else if (ConnectionConnecting == mCurrentConnectionStatus)
				{
				WWDEBUG_SAY(("WOL: LoginServer already connecting.\n"));
				return EventValueWait<ConnectionStatus>::CreateAndObserve(*this, ConnectionConnected, WOLSTRING("WOL_CONNECTING"));
				}
			}
		else
			{
			// If already disconnected then just return with success. If disconnecting
			// then wait for disconnect to finish.
			if (ConnectionDisconnected == mCurrentConnectionStatus)
				{
				WWDEBUG_SAY(("WOL: LoginServer already disconnected.\n"));
				RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_DISCONNECTING"));
				wait->EndWait(WaitCondition::ConditionMet, WOLSTRING("WOL_DISCONNECTED"));
				return wait;
				}
			else if (ConnectionDisconnecting == mCurrentConnectionStatus)
				{
				WWDEBUG_SAY(("WOL: LoginServer already disconnecting.\n"));
				return EventValueWait<ConnectionStatus>::CreateAndObserve(*this, ConnectionDisconnected, WOLSTRING("WOL_DISCONNECTING"));
				}
			}
		}

	// Clean up on server switch or disconnect
	if (ConnectionConnected == mCurrentConnectionStatus)
		{
		WWDEBUG_SAY(("WOL: LoginServer disconnect cleanup.\n"));

		mCurrentChannelStatus = ChannelLeft;
		ChannelEvent chanEvent(mCurrentChannelStatus, mCurrentChannel);
		NotifyObservers(chanEvent);

		mCurrentChannel.Release();
		mPendingChannel.Release();

		mChatChannels.clear();
		ChannelListEvent chatListEvent(ChannelListEvent::NewList, mChatChannels, 0);
		NotifyObservers(chatListEvent);

		RefPtrConst<Product> product = Product::Current();
		WWASSERT(product.IsValid());

		mGameChannels.clear();
		ChannelListEvent gameListEvent(ChannelListEvent::NewList, mGameChannels, product->GetGameCode());
		NotifyObservers(gameListEvent);
		}

	//---------------------------------------------------------------------------
	// Process change of server connection
	//---------------------------------------------------------------------------
	RefPtr<SerialWait> serverWait = SerialWait::Create();

	// If connecting or disconnecting, let that finish
	if (ConnectionDisconnecting == mCurrentConnectionStatus)
		{
		WWDEBUG_SAY(("WOL: LoginServer wait for disconnect to finish.\n"));
		RefPtr< EventValueWait<ConnectionStatus> > finish = EventValueWait<ConnectionStatus>::CreateAndObserve(*this, ConnectionDisconnected, WOLSTRING("WOL_DISCONNECTING"));
		serverWait->Add(finish);
		}
	else if (ConnectionConnecting == mCurrentConnectionStatus)
		{
		WWDEBUG_SAY(("WOL: LoginServer wait for connect to finish.\n"));
		RefPtr< EventValueWait<ConnectionStatus> > finish = EventValueWait<ConnectionStatus>::CreateAndObserve(*this, ConnectionConnected, WOLSTRING("WOL_DISCONNECTING"));
		serverWait->Add(finish);
		}

	// If connected or connecting to another server then disconnect from that server
	// before connecting to the new one.
	if (mCurrentServer.IsValid() || (mPendingServer.IsValid() && (mPendingServer != server)))
		{
		WWDEBUG_SAY(("WOL: LoginServer changing servers.\n"));

		if ((ConnectionConnected == mCurrentConnectionStatus) || (ConnectionConnecting == mCurrentConnectionStatus))
			{
			WWDEBUG_SAY(("WOL: LoginServer disconnect from current server.\n"));
			RefPtr<DisconnectWait> disconnect = DisconnectWait::Create(this);
			serverWait->Add(disconnect);
			}
		}

	mPendingServer = server;
	mPendingLogin = login;

	// Server == NULL is just a logout
	if (!server.IsValid())
		{
		return serverWait;
		}

	if (!login.IsValid())
		{
		WWASSERT(login.IsValid());
		return serverWait;
		}

	// Finally, connect to desired server.  Request to connect is embedded in Wait_Beginning
	WWDEBUG_SAY(("WOL: LoginServer connect to new server.\n"));

	RefPtr<ConnectWait> connect = ConnectWait::Create(this, server, login);
	serverWait->Add(connect);

	return serverWait;
	}


/******************************************************************************
*
* NAME
*     Session::Logout
*
* DESCRIPTION
*     Log current user off.
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Wait condition to process for logout.
*
******************************************************************************/

RefPtr<WaitCondition> Session::Logout(void)
	{
	WWDEBUG_SAY(("WOL: Logout\n"));
	return LoginServer(RefPtr<IRCServerData>(), RefPtr<LoginInfo>());
	}


/******************************************************************************
*
* NAME
*     Session::ClearServers
*
* DESCRIPTION
*     Clear internal server lists
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::ClearServers(void)
	{
	mRequestingServerList = false;
	mIgnoreServerLists = false;

	mIRCServers.clear();
	mLadderServer.Release();
	mGameResultsServer.Release();
	mWDTServer.Release();
	mMGLServers.clear();
	mPingServers.clear();
	}


/******************************************************************************
*
* NAME
*     Session::EnableProgressiveChannelList
*
* DESCRIPTION
*     Enable / Disable the receipt of progressive channel lists.
*
* INPUTS
*     Enable - True to enable; False to disable.
*
* RESULT
*     Success - True if operation is successful.
*
******************************************************************************/

bool Session::EnableProgressiveChannelList(bool enable)
	{
	const char* onoff = ((enable == true) ? "true" : "false");
	HRESULT hr = mChat->SetAttributeValue("IncrementalChannelLists", onoff);
	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::RequestChannelList
*
* DESCRIPTION
*
* INPUTS
*     Type     - Channel type to request.
*     AutoPing -
*
* RESULT
*     Success - True if request successful.
*
******************************************************************************/

bool Session::RequestChannelList(int channelType, bool autoPing)
	{
	WWDEBUG_SAY(("WOL: RequestChannelList %d\n", channelType));

	if (mRequestedChannelType != -1)
		{
		WWDEBUG_SAY(("WOLERROR: RequestChannelList already pending\n"));
		return false;
		}

	HRESULT hr = mChat->RequestChannelList(channelType, autoPing);

	if (SUCCEEDED(hr))
		{
		mRequestedChannelType = channelType;
		return true;
		}

	WWDEBUG_SAY(("WOLERROR: RequestChannelList() HRESULT = %s\n", GetChatErrorString(hr)));

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::FindChannel
*
* DESCRIPTION
*     Find a channel by name.
*
* INPUTS
*     Name - Name of channel to look for.
*
* RESULT
*     Channel -
*
******************************************************************************/

RefPtr<ChannelData> Session::FindChannel(const unichar_t* channelName)
	{
	if (channelName == NULL)
		{
		return NULL;
		}

	if (mCurrentChannel.IsValid())
		{
		const WideStringClass& name = mCurrentChannel->GetName();

		if (name.Compare_No_Case(channelName) == 0)
			{
			return mCurrentChannel;
			}
		}

	RefPtr<ChannelData> channel = FindChatChannel(channelName);

	if (!channel.IsValid())
		{
		channel = FindGameChannel(channelName);
		}

	return channel;
	}


RefPtr<ChannelData> Session::FindChannel(const char* channelName)
	{
	if (channelName == NULL)
		{
		return NULL;
		}

	if (mCurrentChannel.IsValid())
		{
		WOL::Channel& wolChannel = mCurrentChannel->GetData();

		if (stricmp(channelName, (const char*)wolChannel.name) == 0)
			{
			return mCurrentChannel;
			}
		}

	RefPtr<ChannelData> channel = FindChatChannel(channelName);

	if (!channel.IsValid())
		{
		channel = FindGameChannel(channelName);
		}

	return channel;
	}


/******************************************************************************
*
* NAME
*     Session::JoinChannel
*
* DESCRIPTION
*     Join the specified channel.
*
* INPUTS
*     Channel - Channel to join.
*
* RESULT
*     Wait - Wait condition to process for joining channel
*
******************************************************************************/

RefPtr<WaitCondition> Session::JoinChannel(const RefPtr<ChannelData>& channel,
			const unichar_t* password)
	{
	// If we are not connected then we cannot join a channel.
	if (mCurrentConnectionStatus != ConnectionConnected)
		{
		WWASSERT(mCurrentConnectionStatus == ConnectionConnected);

		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELJOIN"));
		wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_NOTCONNECTED"));
		return wait;
		}

	// If we are already connected to the requested channel then done.
	if (mCurrentChannel == channel)
		{
		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELJOIN"));
		wait->EndWait(WaitCondition::ConditionMet, WOLSTRING("WOL_CONNECTED"));
		return wait;
		}

	// Use default product password if one is not provided
	if (password == NULL)
		{
		RefPtrConst<Product> product = Product::Current();
		WWASSERT(product.IsValid());
		password = product->GetChannelPassword();
		}

	// Set up wait condition for joining channel
	mPendingChannel = channel;

	RefPtr<SerialWait> wait = SerialWait::Create();

	if (wait.IsValid())
		{
		// Leave current channel
		if (mCurrentChannel.IsValid())
			{
			RefPtr<WaitCondition> leaveWait = LeaveChannelWait::Create(this);
			wait->Add(leaveWait);
			}

		// Join new channel
		if (channel.IsValid())
			{
			RefPtr<WaitCondition> joinWait = JoinChannelWait::Create(this, channel, password);
			wait->Add(joinWait);
			}
		}

	return wait;
	}


RefPtr<WaitCondition> Session::JoinChannel(const unichar_t* chanName, const unichar_t* password, int type)
	{
	// If we are not connected then we cannot join a channel.
	if (mCurrentConnectionStatus != ConnectionConnected)
		{
		WWASSERT(mCurrentConnectionStatus == ConnectionConnected);

		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELJOIN"));
		wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_NOTCONNECTED"));
		return wait;
		}

	// If we are already connected to the requested channel then done.
	if (mCurrentChannel.IsValid())
		{
		const WideStringClass& curChanName = mCurrentChannel->GetName();

		if (curChanName.Compare_No_Case(chanName) == 0)
			{
			RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELJOIN"));
			wait->EndWait(WaitCondition::ConditionMet, WOLSTRING("WOL_CONNECTED"));
			return wait;
			}
		}

	// Use default product password if one is not provided
	if (password == NULL)
		{
		RefPtrConst<Product> product = Product::Current();
		WWASSERT(product.IsValid());
		password = product->GetChannelPassword();
		}

	// Set up wait condition for joining channel
	RefPtr<SerialWait> wait = SerialWait::Create();

	if (wait.IsValid())
		{
		// Leave current channel
		if (mCurrentChannel.IsValid())
			{
			RefPtr<WaitCondition> leaveWait = LeaveChannelWait::Create(this);
			wait->Add(leaveWait);
			}

		if (chanName)
			{
			// Join new channel
			RefPtr<WaitCondition> joinWait = JoinChannelWait::Create(this, chanName, password, type);
			wait->Add(joinWait);
			}
		}

	return wait;
	}


/******************************************************************************
*
* NAME
*     Session::RequestChannelJoin
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if request successful.
*
******************************************************************************/

bool Session::RequestChannelJoin(const RefPtr<ChannelData>& channel, const unichar_t* password)
	{
	if (channel.IsValid() && password)
		{
		WWDEBUG_SAY(("WOL: RequestChannelJoin C:%S P:%S\n", (const unichar_t*)channel->GetName(), password));
		mPendingChannel = channel;

		WOL::Channel& wolChannel = channel->GetData();
		u_wstomb((char*)wolChannel.key, password, sizeof(wolChannel.key));

		// If the channel is not in the channel list then mark it as hidden.
		RefPtr<ChannelData> inList = FindChannel(channel->GetName());
		wolChannel.hidden = (inList.IsValid() ? 0 : 1);

		HRESULT hr = mChat->RequestChannelJoin(&wolChannel);

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: RequestChannelJoin() HRESULT = %s\n", GetChatErrorString(hr)));
			}

		return SUCCEEDED(hr);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::LeaveChannel
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Wait condition to process for leaving channel
*
******************************************************************************/

RefPtr<WaitCondition> Session::LeaveChannel(void)
	{
	if (!mCurrentChannel.IsValid() || (ChannelJoined != mCurrentChannelStatus))
		{
		mCurrentChannel.Release();
		mCurrentChannelStatus = ChannelLeft;

		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELLEAVE"));
		wait->EndWait(WaitCondition::ConditionMet, WOLSTRING("WOL_CHANNELLEFT"));
		return wait;
		}

	return LeaveChannelWait::Create(this);
	}


bool Session::RequestLeaveChannel(void)
	{
	if (ChannelJoined == mCurrentChannelStatus)
		{
		HRESULT hr = mChat->RequestChannelLeave();

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: RequestChannelLeave() failed HRESULT = %s\n", GetChatErrorString(hr)));
			return false;
			}

		mCurrentChannelStatus = ChannelLeaving;
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     Session::GetNewChatChannelList
*
* DESCRIPTION
*     Request a new chat channel list.
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Wait condition to process for chat channel list.
*
******************************************************************************/

RefPtr<WaitCondition> Session::GetNewChatChannelList(void)
	{
	return ChannelListWait::Create(this, 0);
	}


/******************************************************************************
*
* NAME
*     Session::FindChatChannel
*
* DESCRIPTION
*     Find a chat channel.
*
* INPUTS
*     ChannelName - Name of channel to search for.
*
* RESULT
*     Channel -
*
******************************************************************************/

RefPtr<ChannelData> Session::FindChatChannel(const unichar_t* channelName)
	{
	return FindChannelInList(mChatChannels, channelName);
	}


RefPtr<ChannelData> Session::FindChatChannel(const char* channelName)
	{
	return FindChannelInList(mChatChannels, channelName);
	}


/******************************************************************************
*
* NAME
*     Session::GetNewGameChannelList
*
* DESCRIPTION
*     Get new game channel list.
*
* INPUTS
*     NONE
*
* RESULT
*     Wait - Wait condition to process for obtaining new channel list.
*
******************************************************************************/

RefPtr<WaitCondition> Session::GetNewGameChannelList(void)
	{
	RefPtrConst<Product> product = Product::Current();

	if (product.IsValid())
		{
		int channelType = product->GetGameCode();
		return ChannelListWait::Create(this, channelType);
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     Session::RequestGameChannelList
*
* DESCRIPTION
*     Submit request for game channel list.
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if request successful.
*
******************************************************************************/

bool Session::RequestGameChannelList(void)
	{
	RefPtrConst<Product> product = Product::Current();

	if (!product.IsValid())
		{
		WWASSERT(product.IsValid());
		return false;
		}

	int channelType = product->GetGameCode();

	return RequestChannelList(channelType, false);
	}


/******************************************************************************
*
* NAME
*     Session::FindGameChannel
*
* DESCRIPTION
*     Find a game channel.
*
* INPUTS
*     ChannelName - Name of channel to search for.
*
* RESULT
*     Channel -
*
******************************************************************************/

RefPtr<ChannelData> Session::FindGameChannel(const unichar_t* channelName)
	{
	return FindChannelInList(mGameChannels, channelName);
	}


RefPtr<ChannelData> Session::FindGameChannel(const char* channelName)
	{
	return FindChannelInList(mGameChannels, channelName);
	}


/******************************************************************************
*
* NAME
*     Session::CreateChannel
*
* DESCRIPTION
*     Create a new channel.
*
* INPUTS
*     Name     - Name of new channel.
*     Password - Password for channel.
*
* RESULT
*     Wait - Wait condition to process for channel creation.
*
******************************************************************************/

RefPtr<WaitCondition> Session::CreateChannel(const unichar_t* name, const unichar_t* password, int type)
	{
	WWDEBUG_SAY(("WOL: CreateChannel '%S' Type %ld\n", name, type));

	if (mCurrentConnectionStatus != ConnectionConnected)
		{
		WWASSERT(mCurrentConnectionStatus == ConnectionConnected);

		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELCREATE"));
		wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_NOTCONNECTED"));
		return wait;
		}

	// If we are already connected to the requested channel then done.
	if (mCurrentChannel.IsValid())
		{
		const WideStringClass& curChanName = mCurrentChannel->GetName();

		if (curChanName.Compare_No_Case(name) == 0)
			{
			RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELCREATE"));
			wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_CONNECTED"));
			return wait;
			}
		}

	RefPtr<ChannelData> channel = ChannelData::Create(name, password, type);

	// Set up wait condition for creating channel
	mPendingChannel = channel;
	RefPtr<SerialWait> wait = SerialWait::Create();

	// Leave current channel
	if (mCurrentChannel.IsValid())
		{
		RefPtr<WaitCondition> leave = LeaveChannelWait::Create(this);
		wait->Add(leave);
		}

	// Create & Join channel
	RefPtr<WaitCondition> create = CreateChannelWait::Create(this, channel, password);
	wait->Add(create);

	return wait;
	}


/******************************************************************************
*
* NAME
*     Session::CreateChannel
*
* DESCRIPTION
*     Create a new channel
*
* INPUTS
*     Channel  - Channel data to create from.
*     Password - Password for joining channel
*
* RESULT
*     Wait - Wait condition to process for creating channel
*
******************************************************************************/

RefPtr<WaitCondition> Session::CreateChannel(const RefPtr<ChannelData>& channel,
		const unichar_t* password)
	{
	WWASSERT(channel.IsValid());
	WWDEBUG_SAY(("WOL: CreateChannel '%S'\n", (const unichar_t*)channel->GetName()));

	if (mCurrentConnectionStatus != ConnectionConnected)
		{
		WWASSERT(mCurrentConnectionStatus == ConnectionConnected);

		RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELCREATE"));
		wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_NOTCONNECTED"));
		return wait;
		}

	// If we are already connected to the requested channel then done.
	if (mCurrentChannel.IsValid())
		{
		const WideStringClass& curChanName = mCurrentChannel->GetName();

		if (curChanName.Compare_No_Case(channel->GetName()) == 0)
			{
			RefPtr<SingleWait> wait = SingleWait::Create(WOLSTRING("WOL_CHANNELCREATE"));
			wait->EndWait(WaitCondition::Error, WOLSTRING("WOL_CONNECTED"));
			return wait;
			}
		}

	mPendingChannel = channel;

	// Set up wait condition for creating channel
	RefPtr<SerialWait> wait = SerialWait::Create();

	// Leave current channel
	if (mCurrentChannel.IsValid())
		{
		RefPtr<WaitCondition> leave = LeaveChannelWait::Create(this);
		wait->Add(leave);
		}

	// Create & join channel
	RefPtr<WaitCondition> create = CreateChannelWait::Create(this, channel, password);
	wait->Add(create);

	return wait;
	}


/******************************************************************************
*
* NAME
*     Session::GetChannelTopic
*
* DESCRIPTION
*     Get the topic of the current channel
*
* INPUTS
*     NONE
*
* RESULT
*     Topic - Channel topic or NULL if no topic or not in channel.
*
******************************************************************************/

const char* Session::GetChannelTopic(void) const
	{
	const RefPtr<ChannelData>& channel = GetCurrentChannel();

	if (channel.IsValid())
		{
		return channel->GetTopic();
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     Session::SendChannelTopic
*
* DESCRIPTION
*     Send the topic of the current channel.
*
* INPUTS
*     NONE
*
* RESULT
*     True if request successful.
*
******************************************************************************/

bool Session::SendChannelTopic(void)
	{
	const RefPtr<ChannelData>& channel = GetCurrentChannel();

	if (channel.IsValid())
		{
		const char* topic = channel->GetTopic();

		if (topic)
			{
			HRESULT hr = mChat->RequestChannelTopic(topic);

			if (SUCCEEDED(hr))
				{
				return true;
				}

			WWDEBUG_SAY(("WOLERROR: SendChannelTopic() HRESULT = %s\n", GetChatErrorString(hr)));
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendChannelExtraInfo
*
* DESCRIPTION
*     Send the extra info for the current channel.
*
* INPUTS
*     NONE
*
* RESULT
*     True if request successful.
*
******************************************************************************/

bool Session::SendChannelExtraInfo(void)
	{
	const RefPtr<ChannelData>& channel = GetCurrentChannel();

	if (channel.IsValid())
		{
		const char* exInfo = channel->GetExtraInfo();

		if (exInfo)
			{
			HRESULT hr = mChat->SetChannelExInfo(exInfo);

			if (SUCCEEDED(hr))
				{
				return true;
				}

			WWDEBUG_SAY(("WOLERROR: ChangeChannelExtraInfo() HRESULT = %s\n", GetChatErrorString(hr)));
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::RequestUserList
*
* DESCRIPTION
*     Request a new user list for the current channel from the IRC server.
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if request successful.
*
******************************************************************************/

bool Session::RequestUserList(void)
	{
	HRESULT hr = mChat->RequestUserList();

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: RequestUserList() HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::FindUser
*
* DESCRIPTION
*     Search for a user in the user list by name.
*
* INPUTS
*     Nickname - Name of user to look for.
*
* RESULT
*     User - User, if found.
*
******************************************************************************/

RefPtr<UserData> Session::FindUser(const unichar_t* nickname)
	{
	// Check the current user first.
	if (mCurrentUser.IsValid())
		{
		const WideStringClass& username = mCurrentUser->GetName();

		if (username.Compare_No_Case(nickname) == 0)
			{
			return mCurrentUser;
			}
		}

	// Check in channels user list
	return FindUserInList(nickname, mUsers);
	}


/******************************************************************************
*
* NAME
*     Session::GetUserOrBuddy
*
* DESCRIPTION
*     Search for a user in the user list by name.
*
* INPUTS
*     Nickname - Name of user to look for.
*
* RESULT
*     User - User, if found.
*
******************************************************************************/

RefPtr<UserData> Session::GetUserOrBuddy(const unichar_t* name)
	{
	RefPtr<UserData> user = FindUser(name);

	if (!user.IsValid())
		{
		user = FindBuddy(name);
		}

	return user;
	}


/******************************************************************************
*
* NAME
*     Session::IsCurrentUser
*
* DESCRIPTION
*     Check if the specified user is the one logged in.
*
* INPUTS
*     User - User to check.
*
* RESULT
*     True if specified user is the current one.
*
******************************************************************************/

bool Session::IsCurrentUser(const RefPtr<UserData>& user) const
	{
	return user.IsValid() && mCurrentUser.IsValid() &&
			user->GetName() == mCurrentUser->GetName();
	}


bool Session::IsCurrentUser(const unichar_t* username) const
	{
	if (username && mCurrentUser.IsValid())
		{
		const WideStringClass& name = mCurrentUser->GetName();
		return (name.Compare_No_Case(username) == 0);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::ChangeCurrentUserLocale
*
* DESCRIPTION
*     Change the locale for the user currently logged in.
*
* INPUTS
*     Locale - New locale
*
* RESULT
*     True if successful.
*
******************************************************************************/

bool Session::ChangeCurrentUserLocale(WOL::Locale locale)
	{
	if (mCurrentUser.IsValid() && (mCurrentUser->GetLocale() != locale))
		{
		HRESULT hr = mChat->RequestSetLocale(locale);

		if (SUCCEEDED(hr))
			{
			return true;
			}

		WWDEBUG_SAY(("WOLERROR: RequestSetLocale() HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SquelchUser
*
* DESCRIPTION
*     Enable / disable squelching (ignoring) of other users.
*
* INPUTS
*     User  - User to squelch / unsquelch
*     OnOff - True to squelch; False unsquelch
*
* RESULT
*     True if operation successful.
*
******************************************************************************/

bool Session::SquelchUser(const RefPtr<UserData>& user, bool onoff)
	{
	if (user.IsValid())
		{
		WWDEBUG_SAY(("WOL: SquelchUser '%S'\n", (const unichar_t *)user->GetName()));
		HRESULT hr = mChat->SetSquelch(&user->GetData(), onoff);

		if (SUCCEEDED(hr))
			{
			user->Squelch(onoff);
			return true;
			}

		WWDEBUG_SAY(("WOLERROR: SetSquelch() HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::KickUser
*
* DESCRIPTION
*     Make a request to kick a user from the current channel. Kicking is only
*     allowed for channel owners.
*
* INPUTS
*     User - Name of user to kick from the channel
*
* RESULT
*     True if operation successful.
*
******************************************************************************/

bool Session::KickUser(const unichar_t* username)
	{
	if (username)
		{
		RefPtr<UserData> user = FindUserInList(username, mUsers);

		if (user.IsValid())
			{
			WWDEBUG_SAY(("WOL: KickUser '%S'\n", (const unichar_t *)user->GetName()));
			HRESULT hr = mChat->RequestUserKick(&user->GetData());

			if (SUCCEEDED(hr))
				{
				return true;
				}

			WWDEBUG_SAY(("WOLERROR: RequestUserKick() HRESULT = %s\n", GetChatErrorString(hr)));
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::BanUser
*
* DESCRIPTION
*     Make a request to ban / unban a user from the current channel.
*     Banning is only allowed for channel owners.
*
* INPUTS
*     User - Name of user to ban / unban from the channel
*     Ban  - True to ban; False to remove ban
*
* RESULT
*     True if operation successful.
*
******************************************************************************/

bool Session::BanUser(const unichar_t* username, bool banned)
	{
	if (username)
		{
		WWDEBUG_SAY(("WOL: BanUser '%S'\n", username));

		char ansiName[64];
		u_wstomb(ansiName, username, sizeof(ansiName));

		HRESULT hr = mChat->RequestChannelBan(ansiName, banned);

		if (SUCCEEDED(hr))
			{
			return true;
			}

		WWDEBUG_SAY(("WOLERROR: RequestChannelBan() HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::PageUser
*
* DESCRIPTION
*     Send page message to the specified user.
*
* INPUTS
*     User    - User to page.
*     Message - Message to send.
*
* RESULT
*     True if operation successful.
*
******************************************************************************/

bool Session::PageUser(const unichar_t* username, const unichar_t* message)
	{
	if ((username && u_strlen(username)) && (message && u_strlen(message)))
		{
		WOL::User wolUser;
		memset((void*)&wolUser, 0, sizeof(wolUser));
		u_wstomb((char*)&wolUser.name, username, sizeof(wolUser.name));
		wolUser.name[sizeof(wolUser.name) - 1] = 0;

		if (IsAnsiText(message))
			{
			char ansiMessage[1024];
			u_wstomb(ansiMessage, message, sizeof(ansiMessage));

			HRESULT hr = mChat->RequestPage(&wolUser, ansiMessage);

			if (FAILED(hr))
				{
				WWDEBUG_SAY(("WOLERROR: RequestPage() HRESULT = %s\n", GetChatErrorString(hr)));
				}

			return SUCCEEDED(hr);
			}
		else
			{
			HRESULT hr = mChat->RequestUnicodePage(&wolUser, (const unsigned short *)message);

			if (FAILED(hr))
				{
				WWDEBUG_SAY(("WOLERROR: RequestUnicodePage() HRESULT = %s\n", GetChatErrorString(hr)));
				}

			return SUCCEEDED(hr);
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::RequestLocateUser
*
* DESCRIPTION
*     Locate the user on Westwood Online.
*
* INPUTS
*     Name - Login name of the user to locate.
*
* RESULT
*     True if operation successful.
*
******************************************************************************/

void Session::RequestLocateUser(const unichar_t* name)
	{
	if (name && u_strlen(name))
		{
		// If we already have the user then submit it, otherwise create a new user entry.
		RefPtr<UserData> user = GetUserOrBuddy(name);

		if (!user.IsValid())
			{
			user = UserData::Create(name);
			}

		RequestLocateUser(user);
		}
	}


void Session::RequestLocateUser(const RefPtr<UserData>& user)
	{
	if (user.IsValid())
		{
		// If the user is already pending then do not add him again.
		RefPtr<UserData> pending = FindUserInList(user->GetName(), mLocatePendingUsers);

		if (pending.IsValid())
			{
			return;
			}

		// Is user currently being located then done
		if (mLocatingUser.IsValid())
			{
			const WideStringClass& userName = user->GetName();

			if (userName.Compare_No_Case(mLocatingUser->GetName()) == 0)
				{
				return;
				}
			}

		// Add the user to the pending locate list
		mLocatePendingUsers.push_back(user);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakeLocateUserRequests
*
* DESCRIPTION
*     Make a request to the server to locate users.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakeLocateUserRequests(void)
	{
	if (!mLocatePendingUsers.empty() && !mLocatingUser.IsValid())
		{
		mLocatingUser = mLocatePendingUsers[0];
		mLocatePendingUsers.erase(mLocatePendingUsers.begin());

		HRESULT hr = mChat->RequestFind(&mLocatingUser->GetData());

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: RequestFind() HRESULT = %s\n", GetChatErrorString(hr)));
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestUserLocale
*
* DESCRIPTION
*     Make a request to query a users locale. (IE: US, Germany, Korea)
*
* INPUTS
*     Name - Name of user to query locale for.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestUserLocale(const unichar_t* username)
	{
	if (username && (u_strlen(username) > 0))
		{
		// Make sure the user is not already in the list.
		const size_t count = mLocaleRequests.size();

		for (size_t index = 0; index < count; ++index)
			{
			if (mLocaleRequests[index].Compare_No_Case(username) == 0)
				{
				return;
				}
			}

		mLocaleRequests.push_back(username);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakeLocaleRequests
*
* DESCRIPTION
*     Make request to server to query user locales.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakeLocaleRequests(void)
	{
	if (!mLocaleRequests.empty())
		{
		const size_t count = std::min(static_cast<size_t>(10), mLocaleRequests.size());
		if (count == 0)
		{
			return;
		}
		WOL::User* users = new WOL::User[count];
		WWASSERT(users && "Failed to create temporary users array");

		if (users)
			{
			for (size_t index = 0; index < count; ++index)
				{
				WideStringClass& username = mLocaleRequests[index];
				WWDEBUG_SAY(("WOL: Requesting locale for '%S'\n", (const unichar_t*)username));

				WOL::User& user = users[index];
				u_wstomb((char*)user.name, username, sizeof(user.name));
				user.name[sizeof(user.name) - 1] = 0;

				if (index == (count - 1))
					{
					user.next = NULL;
					}
				else
					{
					user.next = &users[index + 1];
					}
				}

			HRESULT hr = mChat->RequestUserLocale(users);

			// If request was successful then remove from the pending list.
			if (SUCCEEDED(hr))
				{
				LocaleRequestColl::iterator first = mLocaleRequests.begin();
				mLocaleRequests.erase(first, first + static_cast<LocaleRequestColl::difference_type>(count));
				}
			else
				{
				WWDEBUG_SAY(("WOLERROR: RequestUserLocale() HRESULT = %s\n", GetChatErrorString(hr)));
				}

			delete []users;
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestSquadInfoByID
*
* DESCRIPTION
*     Request information about a squad by Squad ID.
*
* INPUTS
*     SquadID - Identifier of squad
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestSquadInfoByID(unsigned int squadID)
	{
	if (squadID != 0)
		{
		// MAGICK NUMBER -- itoa handles up to 33 digit numbers
		unichar_t idString[34];
		u_snprintf_u(idString, sizeof(idString), U_CHAR("%u"), squadID);

		// Only add a request that is not already pending.
		const size_t count = mSquadRequests.size();

		for (size_t index = 0; index < count; ++index)
			{
			if (mSquadRequests[index] == idString)
				{
				return;
				}
			}

		mSquadRequests.push_back(idString);
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestSquadInfoByMemberName
*
* DESCRIPTION
*     Request information about a squad by Member Name.
*
* INPUTS
*     memberName - user name of a member
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestSquadInfoByMemberName(const unichar_t* memberName)
	{
	if (memberName && (u_strlen(memberName) > 0))
		{
		// Only add a request that is not already pending.
		for (size_t index = 0; index < mSquadRequests.size(); index++)
			{
			if (mSquadRequests[index].Compare_No_Case(memberName) == 0)
				{
				return;
				}
			}

		mSquadRequests.push_back(memberName);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakeSquadRequests
*
* DESCRIPTION
*     Make request to server to query squad information.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakeSquadRequests(void)
	{
	if (!mSquadRequests.empty() && mSquadPending.empty())
		{
		// Send up to ten requests at a time.
		const size_t count = std::min(static_cast<size_t>(10), mSquadRequests.size());

		// Send each request in turn,
		size_t index = 0;
		for (; index < count; ++index)
			{
			const WideStringClass& request = mSquadRequests[index];

			HRESULT hr = E_FAIL;

			// Check to see if this is an ID or a name
			// - names can't have the first character be a number so this works.
			unichar_t firstChar = request[0];

			if (iswdigit(firstChar))
				{
				unsigned int squadID;
				u_sscanf_u(request, U_CHAR("%u"), &squadID);
				WWDEBUG_SAY(("WOL: SquadInfo requested for ID %ld\n", squadID));
				hr = mChat->RequestSquadInfo(squadID);
				}
			else
				{
				StringClass name(0, true);
				request.Convert_To(name);

				WWDEBUG_SAY(("WOL: SquadInfo requested for '%s'\n", (const char*)name));
				hr = mChat->RequestSquadByName(name);
				}

			if (FAILED(hr))
				{
				WWDEBUG_SAY(("WOLERROR: RequestSquadInfo() HRESULT = %s\n", GetChatErrorString(hr)));
				break;
				}
			SquadRequestColl::iterator first = mSquadRequests.begin();
			mSquadRequests.erase(first, first + static_cast<SquadRequestColl::difference_type>(index));
			}

		SquadRequestColl::iterator first = mSquadRequests.begin();
		mSquadRequests.erase(first, first + index);
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestTeamInfo
*
* DESCRIPTION
*     Make a request to query a users team information.
*
* INPUTS
*     Name - Name of user to request team information for.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestTeamInfo(const unichar_t* username)
	{
	if (username && (u_strlen(username) > 0))
		{
		for (unsigned int index = 0; index < mTeamRequests.size(); index++)
			{
			if (mTeamRequests[index].Compare_No_Case(username) == 0)
				{
				return;
				}
			}

		mTeamRequests.push_back(username);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakeTeamRequests
*
* DESCRIPTION
*     Send team requests to the server.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakeTeamRequests(void)
	{
	if (!mTeamRequests.empty())
		{
		WWDEBUG_SAY(("WOL: Requesting team information\n"));

		const size_t count = std::min(static_cast<size_t>(10), mTeamRequests.size());
		if (count == 0)
		{
			return;
		}
		WOL::User* users = new WOL::User[count];
		WWASSERT(users && "Failed to create temporary users array");

		if (users)
			{
			for (size_t index = 0; index < count; index++)
				{
				WOL::User& user = users[index];

				WideStringClass& username = mTeamRequests[index];
				u_wstomb((char*)user.name, username.Peek_Buffer(), sizeof(user.name));
				user.name[sizeof(user.name) - 1] = 0;

				if (index == count - 1)
					{
					user.next = NULL;
					}
				else
					{
					user.next = users + index + 1;
					}
				}

			HRESULT hr = mChat->RequestUserTeam(users);

			// If request was successful then remove from the pending list.
			if (SUCCEEDED(hr))
				{
				TeamRequestColl::iterator first = mTeamRequests.begin();
				mTeamRequests.erase(first, first + static_cast<TeamRequestColl::difference_type>(count));
				}
			else
				{
				WWDEBUG_SAY(("WOL: HRESULT = %s\n", GetChatErrorString(hr)));
				}

			delete []users;
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestLadderInfo
*
* DESCRIPTION
*     Request a users ladder information.
*
* INPUTS
*     User - Name of user to request information for.
*     Type - Type of ladder requesting
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestLadderInfo(const unichar_t* name, unsigned int type)
	{
	if (name && (u_strlen(name) > 0) && (type & LADDERTYPE_MASK))
		{
		WideStringClass request(64, true);
		request = U_CHAR("itc:");

		// Clan ladder requests are mutually exclusive to other ladder requests
		// because it requires the name of the clan, while the other requests use
		// the users login name.
		if (type & LadderType_Clan)
			{
			WWASSERT((type & (LadderType_Individual | LadderType_Team)) == 0);
			request[2] = U_CHAR('C');
			}
		else
			{
			if (type & LadderType_Individual)
				{
				request[0] = U_CHAR('I');
				}

			if (type & LadderType_Team)
				{
				request[1] = U_CHAR('T');
				}
			}

		// Append the name to the request
		request += name;

		// Check if the request is already in the list.
		LadderRequestList::iterator pending = mLadderRequests.begin();

		while (pending != mLadderRequests.end())
			{
			if (request.Compare_No_Case(*pending) == 0)
				{
				return;
				}

			pending++;
			}

		WWDEBUG_SAY(("WOL: LadderInfo request added '%S'.\n", (const unichar_t *)request));
		mLadderRequests.push_back(request);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakeLadderRequests
*
* DESCRIPTION
*     Send ladder requests to the server.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakeLadderRequests(void)
	{
	// If there are requests and nothing is pending then send the next request
	if (!mLadderRequests.empty() && mLadderPending == 0)
		{
		RefPtrConst<Product> product = Product::Current();

		if (mLadderServer.IsValid() && product.IsValid())
			{
			// Build a single key from up to 10 requests with the same request type.
			char keys[256];
			keys[0] = 0;

			unsigned int count = 0;
			LadderRequestList::iterator request = mLadderRequests.begin();
			const WideStringClass& firstRequest = *request;

			while ((count < 10) && (request != mLadderRequests.end()))
				{
				// Stop appending requests as soon as we encounter a different type.
				if (u_strncmp(firstRequest, *request, 3) != 0)
					{
					break;
					}

				// Seperate key entries with a colon
				if (count > 0)
					{
					strcat(keys, ":");
					}

				// The request name follows the type
				const unichar_t* widename = u_strchr(*request, U_CHAR(':'));
				WWASSERT(widename != NULL && "Invalid Ladder Request");
				widename++;

				// Add the request
				char name[64];
				u_wstomb(name, widename, 64);
				strcat(keys, name);

				// Next request
				count++;
				request++;
				}

			WWDEBUG_SAY(("WOL: LadderInfo requested for '%s'.\n", keys));
			mLadderPending = count;

			const char* hostAddr = mLadderServer->GetHostAddress();
			unsigned int port = mLadderServer->GetPort();
			unsigned int sku = product->GetLadderSKU();

			// Request individual ladder
			if (firstRequest[0] == U_CHAR('I'))
				{
				HRESULT hr = mNetUtil->RequestLadderList(hostAddr, port, keys, sku, -1, 0, 0);

				if (FAILED(hr))
					{
					WWDEBUG_SAY(("WOLERROR: RequestLadderList (Individual) HRESULT = %s\n", GetNetUtilErrorString(hr)));
					return;
					}

				mLadderPending |= LadderType_Individual;
				}

			// Request team ladder
			if (firstRequest[1] == U_CHAR('T'))
				{
				HRESULT hr = mNetUtil->RequestLadderList(hostAddr, port, keys, (sku | LadderType_Team), -1, 0, 0);

				if (FAILED(hr))
					{
					WWDEBUG_SAY(("WOLERROR: RequestLadderList (Team) HRESULT = %s\n", GetNetUtilErrorString(hr)));
					return;
					}

				mLadderPending |= LadderType_Team;
				}

			// Request clan ladder
			if (firstRequest[2] == U_CHAR('C'))
				{
				HRESULT hr = mNetUtil->RequestLadderList(hostAddr, port, keys, (sku | LadderType_Clan), -1, 0, 0);

				if (FAILED(hr))
					{
					WWDEBUG_SAY(("WOLERROR: RequestLadderList (Clan) HRESULT = %s\n", GetNetUtilErrorString(hr)));
					return;
					}

				mLadderPending |= LadderType_Clan;
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestUserDetails
*
* DESCRIPTION
*     Request detailed information about the user (Ladder, Clan, Team, etc...)
*
* INPUTS
*     User         - User to request details for.
*     RequestFlags - Flags indicating the type of information to request.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestUserDetails(const RefPtr<UserData>& user, unsigned int requestFlags)
	{
	if (user.IsValid())
		{
		if (requestFlags & REQUEST_SQUADINFO)
			{
			// Request squad information for the user.
			unsigned int squadID = user->GetSquadID();
			RefPtr<SquadData> squad = user->GetSquad();

			bool requestSquad = (!squad.IsValid() && (squadID != 0));
			bool changedSquad = (squad.IsValid() && (squadID != squad->GetID()));

			if (requestSquad || changedSquad)
				{
				// Look for the squad in the local cache
				squad = SquadData::FindByID(squadID);
				user->SetSquad(squad);

				// Request squad information if it is not in the local cache.
				if (!squad.IsValid())
					{
					RequestSquadInfoByID(squadID);
					}
				else
					{
					UserEvent event(UserEvent::SquadInfo, user);
					NotifyObservers(event);
					}
				}
			}

		// Request locale information for this user.
		if ((requestFlags & REQUEST_LOCALE) && (user->GetLocale() == WOL::LOC_UNKNOWN))
			{
			RequestUserLocale(user->GetName());
			}

		// Request ladder information for this user.
		if ((requestFlags & REQUEST_LADDERINFO) && !user->GetLadder().IsValid())
			{
			RequestLadderInfo(user->GetName(), LadderType_Team);
			}

		// Request team information for this user.
		if ((requestFlags & REQUEST_TEAMINFO) && (user->GetTeam() == 0))
			{
			RequestTeamInfo(user->GetName());
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::AutoRequestUserDetails
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::AutoRequestUserDetails(const RefPtr<UserData>& user)
	{
	RequestUserDetails(user, mAutoRequestFlags);
	}


/******************************************************************************
*
* NAME
*     Session::RequestBuddyList
*
* DESCRIPTION
*     Send a request to the server for our buddy list.
*
* INPUTS
*     NONE
*
* RESULT
*     Success - True if request successful
*
******************************************************************************/

bool Session::RequestBuddyList(void)
	{
	HRESULT hr = mChat->RequestBuddyList();

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: RequestBuddyList() failed HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::AddBuddy
*
* DESCRIPTION
*     Send a request to the server to add a buddy to our buddy list.
*
* INPUTS
*     Buddy - Name of buddy to add.
*
* RESULT
*     True if request sent successfully
*
******************************************************************************/

bool Session::AddBuddy(const unichar_t* buddyName)
	{
	WOL::User wolUser;
	memset((void*)&wolUser, 0, sizeof(wolUser));
	u_wstomb((char*)&wolUser.name, buddyName, sizeof(wolUser.name));
	wolUser.name[sizeof(wolUser.name) - 1] = 0;

	HRESULT hr = mChat->RequestBuddyAdd(&wolUser);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: RequestBuddyAdd() failed HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::RemoveBuddy
*
* DESCRIPTION
*     Send a request to the server to remove a buddy from our buddy list.
*
* INPUTS
*     Buddy - Name of buddy to remove.
*
* RESULT
*     True if request sent successfully
*
******************************************************************************/

bool Session::RemoveBuddy(const unichar_t* buddyName)
	{
	WOL::User wolUser;
	memset((void*)&wolUser, 0, sizeof(wolUser));
	u_wstomb((char*)&wolUser.name, buddyName, sizeof(wolUser.name));
	wolUser.name[sizeof(wolUser.name) - 1] = 0;

	HRESULT hr = mChat->RequestBuddyDelete(&wolUser);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: RequestBuddyDelete() failed HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::AllowFindPage
*
* DESCRIPTION
*     Set flags to allow / disallow user from being found or paged.
*
* INPUTS
*     AllowFind - True to allow user to be found; False to disallow.
*     AllowPage - True to allow user to be paged; False to disallow.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::AllowFindPage(bool allowFind, bool allowPage)
	{
	HRESULT hr = mChat->SetFindPage(allowFind, allowPage);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: SetFindPage() failed HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::SetBadLanguageFilter
*
* DESCRIPTION
*     Enable / Disable filtering of bad language.
*
* INPUTS
*     Filter - True to enable filtering, false to disable.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SetBadLanguageFilter(bool enabled)
	{
	HRESULT hr = mChat->SetLangFilter(enabled);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: SetLangFilter() failed HRESULT = %s\n", GetChatErrorString(hr)));
		}

	return SUCCEEDED(hr);
	}


/******************************************************************************
*
* NAME
*     Session::SendPublicMessage
*
* DESCRIPTION
*     Send a public message to all users in our channel.
*
* INPUTS
*     Message - Message to send
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPublicMessage(const char* message)
	{
	if (message)
		{
		return SUCCEEDED(mChat->RequestPublicMessage(message));
		}

	return false;
	}


bool Session::SendPublicMessage(const unichar_t* message)
	{
	if (message)
		{
		if (IsAnsiText(message))
			{
			char ansiMessage[1024];
			u_wstomb(ansiMessage, message, sizeof(ansiMessage));
			return SUCCEEDED(mChat->RequestPublicMessage(ansiMessage));
			}
		else
			{
			return SUCCEEDED(mChat->RequestPublicUnicodeMessage((const unsigned short *)message));
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPublicAction
*
* DESCRIPTION
*     Send an actions to all users in our channel.
*
* INPUTS
*     Action - Action message
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPublicAction(const char* action)
	{
	if (action)
		{
		return SUCCEEDED(mChat->RequestPublicAction(action));
		}

	return false;
	}


bool Session::SendPublicAction(const unichar_t* action)
	{
	if (action)
		{
		if (IsAnsiText(action))
			{
			char ansiAction[1024];
			u_wstomb(ansiAction, action, sizeof(ansiAction));
			return SUCCEEDED(mChat->RequestPublicAction(ansiAction));
			}
		else
			{
			return SUCCEEDED(mChat->RequestPublicUnicodeAction((const unsigned short *)action));
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPrivateMessage
*
* DESCRIPTION
*     Send a private message to a single user.
*
* INPUTS
*     Username - Name of user to send private message to.
*     Message  - Message to send.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPrivateMessage(const unichar_t* username, const unichar_t* message)
	{
	if (username && message)
		{
		WOL::User user;
		memset(&user, 0, sizeof(user));
		u_wstomb((char*)user.name, username, sizeof(user.name));
		user.name[sizeof(user.name) - 1] = 0;

		if (IsAnsiText(message))
			{
			char ansiMessage[512];
			u_wstomb(ansiMessage, message, sizeof(ansiMessage));
			return SUCCEEDED(mChat->RequestPrivateMessage(&user, ansiMessage));
			}

		return SUCCEEDED(mChat->RequestPrivateUnicodeMessage(&user, (const unsigned short *)message));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPrivateMessage
*
* DESCRIPTION
*     Send a private message to a set of users.
*
* INPUTS
*     Users   - List of users to send message
*     Message - Message to send.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPrivateMessage(const UserList& users, const char* message)
	{
	if (!users.empty() && message)
		{
		NativeWOLUserList wolUsers(users);
		return SUCCEEDED(mChat->RequestPrivateMessage(wolUsers, message));
		}

	return false;
	}


bool Session::SendPrivateMessage(const UserList& users, const unichar_t* message)
	{
	if (!users.empty() && message)
		{
		NativeWOLUserList wolUsers(users);

		if (IsAnsiText(message))
			{
			char ansiMessage[512];
			u_wstomb(ansiMessage, message, sizeof(ansiMessage));
			return SUCCEEDED(mChat->RequestPrivateMessage(wolUsers, ansiMessage));
			}

		return SUCCEEDED(mChat->RequestPrivateUnicodeMessage(wolUsers, (const unsigned short *)message));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPrivateAction
*
* DESCRIPTION
*     Send a private action message to a set of users.
*
* INPUTS
*     Users  - List of users to send message to.
*     Action - Action message to send.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPrivateAction(const UserList& users, const char* action)
	{
	if (!users.empty() && action)
		{
		NativeWOLUserList wolUsers(users);
		return SUCCEEDED(mChat->RequestPrivateAction(wolUsers, action));
		}

	return false;
	}


bool Session::SendPrivateAction(const UserList& users, const unichar_t* action)
	{
	if (!users.empty() && action)
		{
		NativeWOLUserList wolUsers(users);

		if (IsAnsiText(action))
			{
			char ansiAction[1024];
			u_wstomb(ansiAction, action, sizeof(ansiAction));
			return SUCCEEDED(mChat->RequestPrivateAction(wolUsers, ansiAction));
			}

		return SUCCEEDED(mChat->RequestPrivateUnicodeAction(wolUsers, (const unsigned short *)action));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPublicGameOptions
*
* DESCRIPTION
*     Send public game options
*
* INPUTS
*     Options - Options to send to all users in the channel.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPublicGameOptions(const char* options)
	{
	if (options)
		{
		return SUCCEEDED(mChat->RequestPublicGameOptions(options));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::SendPrivateGameOptions
*
* DESCRIPTION
*
* INPUTS
*     Options - Options to send to specific users.
*
* RESULT
*     True if request successful
*
******************************************************************************/

bool Session::SendPrivateGameOptions(const UserList& users, const char* options)
	{
	if (options)
		{
		for (unsigned int index = 0; index < users.size(); index++)
			{
			NativeWOLUserList wolUsers(users);
			HRESULT hr = mChat->RequestPrivateGameOptions(wolUsers, options);

			if (FAILED(hr))
				{
				return false;
				}
			}

		return true;
		}

	return false;
	}


bool Session::SendPrivateGameOptions(const unichar_t* user, const char* options)
	{
	if (user && options)
		{
		WOL::User wolUser;
		memset(&wolUser, 0, sizeof(WOL::User));
		u_wstomb((char*)wolUser.name, user, sizeof(wolUser.name));
		wolUser.name[sizeof(wolUser.name) - 1] = 0;

		return SUCCEEDED(mChat->RequestPrivateGameOptions(&wolUser, options));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::GetLocaleStrings
*
* DESCRIPTION
*     Get a list of the WWOnline locale strings.
*
* INPUTS
*     Strings - List to fill with locale name strings.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::GetLocaleStrings(std::vector<WideStringClass>& localeStrings)
	{
	localeStrings.clear();

	int numLocales = 0;
	mChat->GetLocaleCount(&numLocales);

	for (int index = 0; index < numLocales; index++)
		{
		const char* name = NULL;
		mChat->GetLocaleString(&name, (WOL::Locale)index);
		localeStrings.push_back(name);
		}
	}


/******************************************************************************
*
* NAME
*     Session::SendGameResults
*
* DESCRIPTION
*
* INPUTS
*      Packet - Game results packet to send.
*      Length - Length of packet in bytes.
*
* RESULT
*     True if request sent successfully
*
******************************************************************************/

bool Session::SendGameResults(unsigned char* packet, unsigned int length)
	{
	if (packet && mGameResultsServer.IsValid())
		{
		mNetUtil->SetGameResMD5(true);

		const char* host = mGameResultsServer->GetHostAddress();
		unsigned int port = mGameResultsServer->GetPort();

		HRESULT hr = mNetUtil->RequestLargeGameresSend(host, port, packet, length);

		if (FAILED(hr))
			{
			WWDEBUG_SAY(("WOLERROR: RequestLargeGameresSend() failed HRESULT = %s\n", GetChatErrorString(hr)));
			}

		return SUCCEEDED(hr);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::EnablePinging
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::EnablePinging(bool enable)
	{
	mPingEnable += ((enable == true) ? 1 : -1);
	}


/******************************************************************************
*
* NAME
*     Session::RequestPing
*
* DESCRIPTION
*     Request a ping to the specified address.
*
* INPUTS
*     Address - Address to ping (IE: irc.westwood.com)
*     Timeout - Maximum time to allow for response.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestPing(const char* address, int timeout)
	{
	if (address)
		{
		// If the address is already pending then do not add it again.
		std::vector<RawPing>::iterator iter = mPingRequests.begin();

		while (iter != mPingRequests.end())
			{
			if (stricmp(iter->GetHostAddress(), address) == 0)
				{
//				WWDEBUG_SAY(("WOLWARNING: Ping request already pending %s\n", address));
				return;
				}

			iter++;
			}

		WWDEBUG_SAY(("WOL: Adding ping request %s\n", address));
		RawPing ping(address, 0, timeout);
		mPingRequests.push_back(ping);
		}
	}


/******************************************************************************
*
* NAME
*     Session::MakePingRequests
*
* DESCRIPTION
*     Send out ping requests
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::MakePingRequests(void)
	{
	if (mPingEnable && (mPingRequests.size() > mPingsPending))
		{
		for (unsigned int index = 0; index < mPingRequests.size(); index++)
			{
			RawPing* ping = &mPingRequests[index];

			if (ping->GetHandle() == -1)
				{
				int handle = 0;

				HRESULT hr = mNetUtil->RequestPing(ping->GetHostAddress(), ping->GetTime(), &handle);

				if (SUCCEEDED(hr))
					{
					WWDEBUG_SAY(("WOL: Pinging '%s' [%d]\n", ping->GetHostAddress(), handle));
					ping->SetHandle(handle);
					mPingsPending++;
					}
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::UpdatePingServerTime
*
* DESCRIPTION
*     Update the ping time of a ping server.
*
* INPUTS
*     Name - Host adddress of server to update.
*     Time - Ping time.
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::UpdatePingServerTime(const char* name, int time)
	{
	if (name)
		{
		// Automatically update ping server times.
		const PingServerList& pingers = GetPingServerList();

		for (unsigned int index = 0; index < pingers.size(); index++)
			{
			const char* pinger = pingers[index]->GetHostAddress();

			if (stricmp(name, pinger) == 0)
				{
				if (time < 0)
					{
					time = INT_MAX;
					}

				pingers[index]->SetPingTime(time);
				break;
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestUserIP
*
* DESCRIPTION
*     Ask the chat server for a users IP address
*
* INPUTS
*     Name of user
*
* RESULT
*     NONE
*
******************************************************************************/

bool Session::RequestUserIP(char *user_name)
	{
	if (user_name)
		{
		WOL::User wolUser;
		memset(&wolUser, 0, sizeof(WOL::User));
		strncpy((char*)wolUser.name, user_name, sizeof(wolUser.name));
		wolUser.name[sizeof(wolUser.name) - 1] = 0;

		return SUCCEEDED(mChat->RequestUserIP(&wolUser));
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     Session::RequestInsiderStatus
*
* DESCRIPTION
*     Used to ask the chat server for if the currently logged in user is
*		an insider or not.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestInsiderStatus(void)
	{
	if (mChat != NULL && mCurrentUser.IsValid ())
		{
		WOL::User &user = mCurrentUser->GetData();
		mChat->RequestInsiderStatus(&user);
		}
	}


/******************************************************************************
*
* NAME
*     Session::RequestServerTime
*
* DESCRIPTION
*     Used to ask the chat server for its current time.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void Session::RequestServerTime(void)
	{
	if (mChat != NULL)
		{
		mChat->RequestServerTime();
		}
	}


/******************************************************************************
*
* NAME
*     Session::GetIGRObject
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

const CComPtr<WOL::IIGROptions>& Session::GetIGRObject(void)
	{
	if (mIGRObject == NULL)
		{
		WWDEBUG_SAY(("WOL: Creating IID_IIGROptions object\n"));
		WOL::IIGROptions* igrObject = NULL;

		HRESULT hr = CoCreateInstance(WOL::CLSID_IGROptions, NULL, CLSCTX_INPROC_SERVER,
			WOL::IID_IIGROptions, (void**)&igrObject);

		if (SUCCEEDED(hr))
			{
			HRESULT hr = igrObject->Init();

			if (S_FALSE == hr)
				{
				WWDEBUG_SAY(("WOLWARNING: IGR settings not found\n"));
				}
			}
		else
			{
			WWDEBUG_SAY(("WOLERROR: Failed to create IID_IIGROptions\n"));
			}

		mIGRObject = igrObject;
		}

	return mIGRObject;
	}


/******************************************************************************
*
* NAME
*     Session::IsStoreLoginAllowed
*
* DESCRIPTION
*     Check if it is okay to save users login information.
*
* INPUTS
*     NONE
*
* RESULT
*     True if allowed to save login information.
*
******************************************************************************/

bool Session::IsStoreLoginAllowed(void)
	{
	const CComPtr<WOL::IIGROptions>& igr = GetIGRObject();

	if (igr)
		{
		HRESULT hr = igr->Is_Storing_Nicks_Allowed();
		return (hr == S_OK);
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     Session::IsAutoLoginAllowed
*
* DESCRIPTION
*     Check if it is okay to automatically login users.
*
* INPUTS
*     NONE
*
* RESULT
*     True if allowed to auto login.
*
******************************************************************************/

bool Session::IsAutoLoginAllowed(void)
	{
	const CComPtr<WOL::IIGROptions>& igr = GetIGRObject();

	if (igr)
		{
		HRESULT hr = igr->Is_Auto_Login_Allowed();
		return (hr == S_OK);
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     Session::IsRunRegAppAllowed
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if allowed to run registration applet.
*
******************************************************************************/

bool Session::IsRunRegAppAllowed(void)
	{
	const CComPtr<WOL::IIGROptions>& igr = GetIGRObject();

	if (igr)
		{
		HRESULT hr = igr->Is_Running_Reg_App_Allowed();
		return (hr == S_OK);
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     ChatAdvisement::Create
*
* DESCRIPTION
*     Create a IChatEvents advisement
*
* INPUTS
*     Chat - IChat object
*     Sink - IChatEvents sink to advise.
*
* RESULT
*     Advisement - Instance of advisement
*
******************************************************************************/

RefPtr<ChatAdvisement> ChatAdvisement::Create(const CComPtr<WOL::IChat>& chat,
		const CComPtr<WOL::IChatEvent>& sink)
	{
	return new ChatAdvisement(chat, sink);
	}


/******************************************************************************
*
* NAME
*     ChatAdvisement::ChatAdvisement
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Chat - IChat object
*     Sink - IChatEvents sink to advise.
*
* RESULT
*     NONE
*
******************************************************************************/

ChatAdvisement::ChatAdvisement(const CComPtr<WOL::IChat>& chat, const CComPtr<WOL::IChatEvent>& sink) :
		mChat(chat),
		mChatCookie(0)
	{
	if (mChat && sink)
		{
		AtlAdvise(mChat, sink, WOL::IID_IChatEvent, &mChatCookie);
		}
	}


/******************************************************************************
*
* NAME
*     ChatAdvisement::~ChatAdvisement
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

ChatAdvisement::~ChatAdvisement()
	{
	if (mChat)
		{
		AtlUnadvise(mChat, WOL::IID_IChatEvent, mChatCookie);
		}
	}

} // namespace WWOnline
