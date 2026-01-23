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
*     $Archive: /Commando/Code/Commando/WOLChatMgr.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 36 $
*     $Modtime: 2/25/02 5:05p $
*
******************************************************************************/

#include "WOLChatMgr.h"
#include "WOLBuddyMgr.h"
#include "mpsettingsmgr.h"
#include "DlgWOLWait.h"
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLServer.h>
#include <WWOnline/WOLChannel.h>
#include <WWOnline/WOLProduct.h>
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>

using namespace WWOnline;

WOLChatMgr* WOLChatMgr::_mInstance = NULL;

// Local prototypes
typedef void (*SlashCommandFunc)(const unichar_t*);

static void SlashCmdPage(const unichar_t*);
static void SlashCmdR(const unichar_t*);
static void SlashCmdLocate(const unichar_t*);
static void SlashCmdMsg(const unichar_t*);
static void SlashCmdInvite(const unichar_t*);
static void SlashCmdKick(const unichar_t*);
static void SlashCmdJoin(const unichar_t*);

static const unichar_t* Get_Parameter_From_String(const unichar_t* command_string, WideStringClass& parameter);

/******************************************************************************
*
* NAME
*     WOLChatMgr::GetInstance
*
* DESCRIPTION
*     Obtain Chat manager instance
*
* INPUTS
*     Create - Okay to create chat manager if not already instantiated.
*
* RESULT
*     Instance - Pointer to the chat manager.
*
******************************************************************************/

WOLChatMgr* WOLChatMgr::GetInstance(bool /* createOK */)
	{
	if (_mInstance == NULL)
		{
		new WOLChatMgr;

		if (_mInstance)
			{
			if (_mInstance->FinalizeCreate() == false)
				{
				_mInstance->Release_Ref();
				}
			}
		}
	else
		{
		_mInstance->Add_Ref();
		}

	return _mInstance;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::WOLChatMgr
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

WOLChatMgr::WOLChatMgr()
	{
	WWDEBUG_SAY(("WOLChatMgr: Instantiated\n"));
	_mInstance = this;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::~WOLChatMgr
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

WOLChatMgr::~WOLChatMgr()
	{
	WWDEBUG_SAY(("WOLChatMgr: Destroyed\n"));
	_mInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::FinalizeCreate
*
* DESCRIPTION
*     Finalize object creation.
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool WOLChatMgr::FinalizeCreate(void)
	{
	// Get reference to WWOnline session
	mWOLSession = Session::GetInstance(false);

	if (!mWOLSession.IsValid())
		{
		return false;
		}

	// Generate chat lobby prefex
	RefPtr<Product> product = Product::Current();
	
	if (!product.IsValid())
		{
		return false;
		}

	mLobbyPrefix.Format(U_CHAR("Lob_%d_"), product->GetGameCode());

	return true;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::Start
*
* DESCRIPTION
*     Start receiving and processing chat messages.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::Start(void)
	{
	if (mWOLSession.IsValid())
		{
		Observer<ServerError>::NotifyMe(*mWOLSession);
		Observer<ChannelListEvent>::NotifyMe(*mWOLSession);
		Observer<ChannelEvent>::NotifyMe(*mWOLSession);
		Observer<UserEvent>::NotifyMe(*mWOLSession);
		Observer<UserList>::NotifyMe(*mWOLSession);
		Observer<ChatMessage>::NotifyMe(*mWOLSession);
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::Stop
*
* DESCRIPTION
*     Suspend receiving and processing of chat messages.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::Stop(void)
	{
	if (mWOLSession.IsValid())
		{
		Observer<ServerError>::StopObserving();
		Observer<ChannelListEvent>::StopObserving();
		Observer<ChannelEvent>::StopObserving();
		Observer<UserEvent>::StopObserving();
		Observer<UserList>::StopObserving();
		Observer<ChatMessage>::StopObserving();
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::RefreshLobbyList
*
* DESCRIPTION
*     Request an updated list of chat channels.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::RefreshLobbyList(void)
	{
	mWOLSession->RequestChannelList(0, false);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::GetLobbyList
*
* DESCRIPTION
*     Get a list of available chat channels.
*
* INPUTS
*     NONE
*
* RESULT
*     Lobbies - List of valid chat channels.
*
******************************************************************************/

const LobbyList& WOLChatMgr::GetLobbyList(void)
	{
	return mLobbyList;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::GetCurrentLobby
*
* DESCRIPTION
*     Get the lobby we are currently joined.
*
* INPUTS
*     NONE
*
* RESULT
*     Lobby - Lobby currently joined to.
*
******************************************************************************/

const RefPtr<ChannelData>& WOLChatMgr::GetCurrentLobby(void)
	{
	return mWOLSession->GetCurrentChannel();
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::FindLobby
*
* DESCRIPTION
*     Find a chat lobby with the specified name.
*
* INPUTS
*     Name - Name of lobby to find.
*
* RESULT
*     Lobby -
*
******************************************************************************/

const RefPtr<ChannelData> WOLChatMgr::FindLobby(const unichar_t* name)
	{
	return mWOLSession->FindChatChannel(name);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::CreateLobby
*
* DESCRIPTION
*     Create a channel lobby
*
* INPUTS
*     Name     - New lobby name
*     Password - Password required to join lobby.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::CreateLobby(const unichar_t* name, const unichar_t* password)
	{
	RefPtr<WaitCondition> wait = mWOLSession->CreateChannel(name, password, 0);

	if (wait.IsValid())
		{
		WideStringClass message(255, true);
		message.Format(TRANSLATE(IDS_CHAT_LOBBYCREATE), name);
		DlgWOLWait::DoDialog((const unichar_t*)message, wait);
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::JoinLobby
*
* DESCRIPTION
*     Join a channel lobby
*
* INPUTS
*     Channel - Channel lobby to join
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::JoinLobby(const RefPtr<ChannelData>& channel)
	{
	if (channel.IsValid())
		{
		RefPtr<Product> product = Product::Current();
		WWASSERT_PRINT(product.IsValid(), "WWOnline product not initialized.");

		RefPtr<WaitCondition> wait = mWOLSession->JoinChannel(channel, product->GetChannelPassword());

		if (wait.IsValid())
			{
			WideStringClass displayName(0, true);
			GetLobbyDisplayName(channel, displayName);

			WideStringClass message(0, true);
			message.Format(TRANSLATE(IDS_CHAT_LOBBYJOIN), displayName);
			DlgWOLWait::DoDialog((const unichar_t*)message, wait);
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::LeaveLobby
*
* DESCRIPTION
*     Leave the current channel lobby.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::LeaveLobby(void)
	{
	RefPtr<WaitCondition> wait = mWOLSession->LeaveChannel();

	if (wait.IsValid())
		{
		const RefPtr<ChannelData>& channel = GetCurrentLobby();
		WideStringClass lobbyName(0, true);
		GetLobbyDisplayName(channel, lobbyName);

		WideStringClass title(0, true);
		title.Format(TRANSLATE(IDS_CHAT_LOBBYLEAVE), lobbyName); 
		DlgWOLWait::DoDialog(title, wait);
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::IsLobbyValid
*
* DESCRIPTION
*     Check if the channel is a valid chat lobby.
*
* INPUTS
*     Lobby - Lobby to validate
*
* RESULT
*     True if lobby is a valid chat lobby.
*
******************************************************************************/

bool WOLChatMgr::IsLobbyValid(const RefPtr<ChannelData>& lobby)
	{
	if (lobby.IsValid())
		{
		const int unwantedFlags = CHAN_MODE_SECRET | CHAN_MODE_INVITEONLY | CHAN_MODE_BAN;
		unsigned int lobbyFlags = lobby->GetFlags();

		// Lobbies without these flags are valid chat lobbies.
		if ((lobbyFlags & unwantedFlags) == 0)
			{
			// Lobbies that have the matching prefix are valid.
			const unichar_t* lobbyName = lobby->GetName();
			return (u_strncasecmp(mLobbyPrefix, lobbyName, mLobbyPrefix.Get_Length(), U_COMPARE_CODE_POINT_ORDER) == 0);
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::GetLobbyDisplayName
*
* DESCRIPTION
*     Get the display name for the given lobby.
*
* INPUTS
*     Lobby - Lobby to get display name for.
*     Name  - On return; display name for the lobby.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::GetLobbyDisplayName(const RefPtr<ChannelData>& lobby, WideStringClass& outName)
	{
	if (lobby.IsValid() == false)
		{
		outName = U_CHAR("");
		return;
		}

	const unichar_t* name = lobby->GetName();
	
	int prefixLength = mLobbyPrefix.Get_Length();

	if (u_strncasecmp(name, mLobbyPrefix, prefixLength, U_COMPARE_CODE_POINT_ORDER) == 0)
		{
		const unichar_t* extName = (name + prefixLength);

		// If the extended portion of the lobby name is a number then select a name
		// from the Renegade lobby names provided. Otherwise use the extended portion
		// as given.
		if (*extName >= U_CHAR('0') && *extName <= U_CHAR('9'))
			{
			static const unichar_t* _lobbies[8] =
				{
				TRANSLATE (IDS_MENU_LOBBY_NAME_01),
				TRANSLATE (IDS_MENU_LOBBY_NAME_02),
				TRANSLATE (IDS_MENU_LOBBY_NAME_03),
				TRANSLATE (IDS_MENU_LOBBY_NAME_04),
				TRANSLATE (IDS_MENU_LOBBY_NAME_05),
				TRANSLATE (IDS_MENU_LOBBY_NAME_06),
				TRANSLATE (IDS_MENU_LOBBY_NAME_07),
				TRANSLATE (IDS_MENU_LOBBY_NAME_08),
				};

			int channelNumber;
			u_sscanf_u(extName, U_CHAR("%d"), &channelNumber);
			int subnum = (channelNumber / 8);
			int nameNumber = (channelNumber % 8);

			const unichar_t* displayName = _lobbies[nameNumber];
			
			if (subnum == 0)
				{
				outName = displayName;
				}
			else
				{
				outName.Format(U_CHAR("%s_%d"), displayName, (subnum + 1));
				}
			}
		else
			{
			outName = extName;
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::FindUser
*
* DESCRIPTION
*     Find a user with the specified name.
*
* INPUTS
*     Name - Name of user to look for.
*
* RESULT
*     User - 
*
******************************************************************************/

const RefPtr<UserData> WOLChatMgr::FindUser(const unichar_t* name)
	{
	return mWOLSession->FindUser(name);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::ClearUserInList
*
* DESCRIPTION
*     Clear the list of users who have joined.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::ClearUserInList(void)
	{
	mUserInList.clear();
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::ClearUserOutList
*
* DESCRIPTION
*     Clear the list of users who have left.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::ClearUserOutList(void)
	{
	mUserOutList.clear();
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::SquelchUser
*
* DESCRIPTION
*     Enable / Disable receipt of messages from the specified user.
*
* INPUTS
*     Name    - Name of user
*     Squelch - True to enable; false to disable
*
* RESULT
*     True if request was successful.
*
******************************************************************************/

bool WOLChatMgr::SquelchUser(const RefPtr<UserData>& user, bool onoff)
	{
	bool success = mWOLSession->SquelchUser(user, onoff);

	if (success)
		{
		int stringID = (onoff ? IDS_CHAT_SQUELCH_ON : IDS_CHAT_SQUELCH_OFF);
		const unichar_t* text = TRANSLATE(stringID);

		WideStringClass message(0, true);
		message.Format(text, user->GetName());
		AddMessage(NULL, message, true, true);
		}

	return success;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::LocateUser
*
* DESCRIPTION
*     Request the location status of a user.
*
* INPUTS
*     Name - Name of user to locate
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::LocateUser(const unichar_t* name)
	{
	mLocatingUserName = name;
	mLocatingUserName.Trim();

	if (mLocatingUserName.Is_Empty() == false)
		{
		mWOLSession->RequestLocateUser(mLocatingUserName);
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::SendPublicMessage
*
* DESCRIPTION
*     Send a message that everyone can see.
*
* INPUTS
*     Message  - Message to send
*     IsAction - Is this message and action.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::SendPublicMessage(const unichar_t* message, bool isAction)
	{
	if (ProcessCommand(message))
		{
		return;
		}

	if (isAction)
		{
		mWOLSession->SendPublicAction(message);
		}
	else
		{
		mWOLSession->SendPublicMessage(message);
		}

	const unichar_t* sender = NULL;

	RefPtr<UserData> me = mWOLSession->GetCurrentUser();

	if (me.IsValid())
		{
		sender = me->GetName();
		}

	AddMessage(sender, message, false, isAction);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::SendPrivateMessage
*
* DESCRIPTION
*     Send a message that only the specified user can see.
*
* INPUTS
*     User     - Users to send message to.
*     Message  - Message to send.
*     IsAction - True if this is an emote action.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::SendPrivateMessage(const RefPtr<UserData>& user,
			const unichar_t* message, bool isAction)
	{
	if (ProcessCommand(message))
		{
		return;
		}

	UserList userList;
	userList.push_back(user);
	SendPrivateMessage(userList, message, isAction);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::SendPrivateMessage
*
* DESCRIPTION
*     Send a message that only a selected group of users can see.
*
* INPUTS
*     User     - List of users to send message to.
*     Message  - Message to send.
*     IsAction - True if this is an emote action.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::SendPrivateMessage(UserList& users, const unichar_t* message, bool isAction)
	{
	if (ProcessCommand(message))
		{
		return;
		}

	if (isAction)
		{
		mWOLSession->SendPrivateAction(users, message);
		}
	else
		{
		mWOLSession->SendPrivateMessage(users, message);
		}

	const unichar_t* sender = NULL;

	RefPtr<UserData> me = mWOLSession->GetCurrentUser();

	if (me.IsValid())
		{
		sender = me->GetName();
		}

	AddMessage(sender, message, true, isAction);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::AddMessage
*
* DESCRIPTION
*     Add a message to the chat messages pool.
*
* INPUTS
*     Sender    - User message originated from.
*     Message   - Text message
*     IsPrivate - True if the message is private.
*     IsAction  - True if the message is an action.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::AddMessage(const unichar_t* sender, const unichar_t* message, bool isPrivate, bool isAction)
	{
	ChatMessage msg(sender, message, isPrivate, isAction);
	mMessageList.push_back(msg);

	WOLChatMgrEvent event = MessageListChanged;
	NotifyObservers(event);
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::PassesFilters
*
* DESCRIPTION
*     Check message against user specified text filters.
*
* INPUTS
*     Message - Chat message to filter.
*
* RESULT
*     True if message should be displayed; False if it should be hidden.
*
******************************************************************************/

bool WOLChatMgr::PassesFilters(const ChatMessage& msg)
	{
	// Messages from the user and channel owners are never filtered.
	if (msg.IsSenderMyself() || msg.IsSenderChannelOwner())
		{
		return true;
		}

	// All messages from squelched users are filtered
	if (msg.IsSenderSquelched())
		{
		WWDEBUG_SAY(("WOLChatMgr: Filtered squelched message from %S\n",
				(const unichar_t*)msg.GetSendersName()));
		return false;
		}

	//---------------------------------------------------------------------------
	// Buddy and Clan filtering
	//---------------------------------------------------------------------------
	bool testClan = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_CLAN_CHAT_ONLY);
	bool testBuddy = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_BUDDY_CHAT_ONLY);

	// If only interested in messages from clan members then filter out all
	// messages from everybody else.
	bool passesClan = true;

	if (testClan)
		{
		RefPtr<UserData> user = mWOLSession->GetCurrentUser();
		
		if (user.IsValid())
			{
			unsigned int clanID = user->GetSquadID();
			passesClan = ((clanID == 0) || msg.IsSenderInClan(clanID));
			}
		}

	// If only interested in messages from buddies then filter out all the
	// messages that are not from users in the buddy list.
	bool passesBuddy = true;

	if (testBuddy)
		{
		WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

		if (buddyMgr)
			{
			passesBuddy = buddyMgr->IsBuddy(msg.GetSendersName());
			buddyMgr->Release_Ref();
			}
		}

	if (testClan && testBuddy)
		{
		if (!passesClan && !passesBuddy)
			{
			return false;
			}
		}
	else if (testClan && !passesClan)
		{
		return false;
		}
	else if (testBuddy && !passesBuddy)
		{
		return false;
		}

	//---------------------------------------------------------------------------
	// Western and Asian filtering
	//---------------------------------------------------------------------------

	// If we are filtering western text and the message is western then
	// filter out the message.
	bool showLatin = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_DISPLAY_NONASIAN);

	if (!showLatin && IsAnsiText(msg.GetMessage()))
		{
		WWDEBUG_SAY(("WOLChatMgr: Filtered latin text message\n"));
		return false;
		}

	// If we are filtering asian text and the message is asian then filter out
	// the message.
	bool showAsian = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_DISPLAY_ASIAN);

	if (!showAsian && !IsAnsiText(msg.GetMessage()))
		{
		WWDEBUG_SAY(("WOLChatMgr: Filtered Asian text message\n"));
		return false;
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::ClearMessageList
*
* DESCRIPTION
*     Clear all the messages.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::ClearMessageList(void)
	{
	mMessageList.clear();
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(ServerError)
*
* DESCRIPTION
*     Handle error conditions from the server.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(ServerError& error)
	{
	switch (error.GetErrorCode())
		{
		case CHAT_E_NOT_OPER:
			break;

		case CHAT_E_BANNED:
			{
			Add_Ref();
			WOLChatMgrEvent event = BannedFromChannel;
			NotifyObservers(event);
			Release_Ref();
			}
			break;

		default:
			{
			const unichar_t* msg = error.GetDescription();
			AddMessage(TRANSLATE(IDS_CHAT_SERVERERROR), msg, true, false);
			}
			break;
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(ChannelListEvent)
*
* DESCRIPTION
*     Handle channel list events.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(ChannelListEvent& event)
	{
	if (event.GetEvent() == ChannelListEvent::NewList && event.GetChannelType() == 0)
		{
		mLobbyList.clear();

		const ChannelList& channelList = event.Subject();
		ChannelList::const_iterator iter = channelList.begin();

		while (iter != channelList.end())
			{
			const RefPtr<ChannelData>& channel = (*iter);

			// If this is a valid public chat lobby then add it to the list.
			if (IsLobbyValid(channel))
				{
				mLobbyList.push_back(channel);
				}

			iter++;
			}
		
		Add_Ref();
		WOLChatMgrEvent change_event = LobbyListChanged;
		NotifyObservers(change_event);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*     Handle channel events such as response to leaving or joining.
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(ChannelEvent& event)
	{
	ChannelStatus status = event.GetStatus();

	switch (status)
		{
		case ChannelJoined:
		case ChannelLeft:
			{
			const RefPtr<ChannelData>& channel = event.Subject();

			WideStringClass displayName(0, true);
			GetLobbyDisplayName(channel, displayName);

			WideStringClass message(0, true);

			if (status == ChannelJoined)
				{
				message.Format(TRANSLATE(IDS_CHAT_LOBBYJOINED), displayName);
				}
			else
				{
				message.Format(TRANSLATE(IDS_CHAT_LOBBYLEFT), displayName);
				}

			AddMessage(NULL, message, true, true);

			Add_Ref();
			WOLChatMgrEvent change_event = LobbyChanged;
			NotifyObservers(change_event);
			Release_Ref();
			}
			break;

		case ChannelBanned:
			{
			Add_Ref();
			WOLChatMgrEvent ban_event = BannedFromChannel;
			NotifyObservers(ban_event);
			Release_Ref();
			}
			break;

		case ChannelKicked:
			{
			Add_Ref();
			WOLChatMgrEvent kick_event = KickedFromChannel;
			NotifyObservers(kick_event);
			Release_Ref();
			}
			break;

		default:
			break;
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(UserEvent)
*
* DESCRIPTION
*     Handle user related events
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(UserEvent& userEvent)
	{
	UserEvent::Event event  = userEvent.GetEvent();

	switch (event)
		{
		case UserEvent::Join:
			{
			WWDEBUG_SAY(("WOLChatMgr: Received User Join Event\n"));
			const RefPtr<UserData>& user = userEvent.Subject();
			mUserInList.push_back(user);

			mWOLSession->RequestUserDetails(user, (REQUEST_LOCALE|REQUEST_SQUADINFO));

			Add_Ref();
			WOLChatMgrEvent event = UserInListChanged;
			NotifyObservers(event);
			Release_Ref();
			}
			break;

		case UserEvent::Kicked:
			{
			const RefPtr<UserData>& user = userEvent.Subject();
			mUserOutList.push_back(user);

			WideStringClass kickMsg(0, true);
			kickMsg.Format(TRANSLATE(IDS_CHAT_USERKICKED), user->GetName());
			AddMessage(NULL, kickMsg, true, true);

			Add_Ref();
			WOLChatMgrEvent event = UserOutListChanged;
			NotifyObservers(event);
			Release_Ref();
			}
			break;

		case UserEvent::Leave:
			{
			mUserOutList.push_back(userEvent.Subject());

			Add_Ref();
			WOLChatMgrEvent event = UserOutListChanged;
			NotifyObservers(event);
			Release_Ref();
			}
			break;

		case UserEvent::Banned:
			{
			WideStringClass banMsg(0, true);
			banMsg.Format(TRANSLATE(IDS_CHAT_USERBANNED), userEvent.Subject()->GetName());
			AddMessage(NULL, banMsg, true, true);
			}
			break;

		case UserEvent::Located:
			{
			WWDEBUG_SAY(("WOLChatMgr: Received User Located Event\n"));
			const RefPtr<UserData>& user = userEvent.Subject();

			// Is this the user we were looking for?
			if (mLocatingUserName.Compare_No_Case(user->GetName()) == 0)
				{
				// Build a string containing the user's name
				WideStringClass message(0, true);
				message.Format(TRANSLATE(IDS_CHAT_LOCATEDUSER), user->GetName());

				// Append the description of the user's location
				WideStringClass location(64, true);
				WOLBuddyMgr::GetLocationDescription(user, location);
				message += U_CHAR(" - ");
				message += location;

				// Add this message to the UI
				AddMessage(NULL, message, true, false);
				mLocatingUserName = U_CHAR("");
				}
			}
			break;
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(UserList)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(WWOnline::UserList& users)
	{
	mUserInList.clear();
	mUserOutList.clear();

	const size_t count = users.size();

	for (size_t index = 0; index < count; index++)
		{
		mWOLSession->RequestUserDetails(users[index], (REQUEST_LOCALE|REQUEST_SQUADINFO));
		mUserInList.push_back(users[index]);
		}

	Add_Ref();
	WOLChatMgrEvent event = UserInListChanged;
	NotifyObservers(event);
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::HandleNotification(ChatMessage)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLChatMgr::HandleNotification(ChatMessage& chatMsg)
	{
	if (PassesFilters(chatMsg))
		{
		mMessageList.push_back(chatMsg);

		Add_Ref();
		WOLChatMgrEvent event = MessageListChanged;
		NotifyObservers(event);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLChatMgr::ProcessCommand(const unichar_t* message)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

bool WOLChatMgr::ProcessCommand(const unichar_t* message)
	{
	// Does this look like a command?
	if (message && message[0] == U_CHAR('/'))
		{
		// Separate the parameters into individual strings
		WideStringClass command(255, true);
		const unichar_t* params = Get_Parameter_From_String(&message[1], command);

		if (command.Get_Length() > 0)
			{
			static struct {const unichar_t* Token; SlashCommandFunc Dispatch;} _cmdDispatch[] =
				{
				{U_CHAR("page"), SlashCmdPage},
				{U_CHAR("r"), SlashCmdR},
				{U_CHAR("locate"), SlashCmdLocate},
				{U_CHAR("msg"), SlashCmdMsg},
				{U_CHAR("invite"), SlashCmdInvite},
				{U_CHAR("kick"), SlashCmdKick},
				{U_CHAR("join"), SlashCmdJoin},
				{NULL, NULL},
				};

			int index = 0;
			const unichar_t* token = _cmdDispatch[index].Token;

			while (token)
				{
				if (command.Compare_No_Case(token) == 0)
					{
					_cmdDispatch[index].Dispatch(params);
					return true;
					}

				++index;
				token = _cmdDispatch[index].Token;
				}
			}
		}

	return false;
	}


// Page a user
void SlashCmdPage(const unichar_t* param)
	{
	// Get the name parameter from the string
	WideStringClass name(64, true);
	WideStringClass pageMsg(0, true);
	pageMsg = Get_Parameter_From_String(param, name);
	pageMsg.Trim();

	if (!name.Is_Empty() && !pageMsg.Is_Empty())
		{
		WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

		if (buddyMgr)
			{
			// Page the requested user
			buddyMgr->PageUser(name, pageMsg);
			buddyMgr->Release_Ref();
			}
		}
	}


// Send a page reply
void SlashCmdR(const unichar_t* param)
	{
	WideStringClass reply(0, true);
	reply = param;
	reply.Trim();

	if (reply.Is_Empty() == false)
		{
		WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

		if (buddyMgr)
			{
			const unichar_t* name = buddyMgr->GetLastPagersName();

			if (name && u_strlen(name))
				{
				// Reply to the last user who paged.
				buddyMgr->PageUser(name, reply);
				}
			
			buddyMgr->Release_Ref();
			}
		}
	}


// Locate a user
void SlashCmdLocate(const unichar_t* param)
	{
	// Try to find the specified user
	WideStringClass name(64, true);
	Get_Parameter_From_String(param, name);

	if (name.Is_Empty() == false)
		{
		WOLChatMgr* chat = WOLChatMgr::GetInstance(false);

		if (chat)
			{
			chat->LocateUser(name);
			chat->Release_Ref();
			}
		}
	}


// Send private message
void SlashCmdMsg(const unichar_t* param)
	{
	// Get the name parameter from the string
	WideStringClass name(64, true);
	WideStringClass message(0, true);
	message = Get_Parameter_From_String(param, name);
	message.Trim();

	if (!name.Is_Empty() && !message.Is_Empty())
		{
		WOLChatMgr* chat = WOLChatMgr::GetInstance(false);

		if (chat)
			{
			RefPtr<UserData> user = chat->FindUser(name);

			if (user.IsValid())
				{
				chat->SendPrivateMessage(user, message, false);
				}

			chat->Release_Ref();
			}
		}
	}


// Invite a user to our location
void SlashCmdInvite(const unichar_t* param)
	{
	// Get the name parameter from the string
	WideStringClass name(64, true);
	const unichar_t* msg = Get_Parameter_From_String(param, name);

	if (name.Is_Empty() == false)
		{
		WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

		if (buddyMgr)
			{
			buddyMgr->InviteUser(name, msg);
			buddyMgr->Release_Ref();
			}
		}
	}


// Kick a user
void SlashCmdKick(const unichar_t* param)
	{
	// Get the name parameter from the string
	WideStringClass name(64, true);
	Get_Parameter_From_String(param, name);

	if (name.Is_Empty() == false)
		{
		RefPtr<Session> wolSession = Session::GetInstance(false);

		if (wolSession.IsValid())
			{
			wolSession->KickUser(name);
			}
		}
	}


// Join a user
void SlashCmdJoin(const unichar_t* param)
	{
	// Get the name parameter from the string
	WideStringClass name(64, true);
	Get_Parameter_From_String(param, name);

	if (name.Is_Empty() == false)
		{
		RefPtr<UserData> user;

		RefPtr<Session> wolSession = Session::GetInstance(false);

		if (wolSession.IsValid())
			{
			user = wolSession->FindUser(name);
			}

		// If the user we want to join is already in our user list then no need to join
		if (user.IsValid() == false)
			{
			WOLBuddyMgr* buddyMgr = WOLBuddyMgr::GetInstance(false);

			if (buddyMgr)
				{
				user = UserData::Create(name);
				buddyMgr->JoinUser(user);
				buddyMgr->Release_Ref();
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     Get_Parameter_From_String
*
* DESCRIPTION
*
* INPUTS
*     Command - Command string
*     Param   - 
*
* RESULT
*     NONE
*
******************************************************************************/

const unichar_t* Get_Parameter_From_String(const unichar_t* command, WideStringClass& param)
	{
	#define LOCAL_STRIP_WHITESPACE(str)	\
		while (str[0] != 0 && str[0] == U_CHAR(' ')) {++str;}

	//	Strip off whitespace
	LOCAL_STRIP_WHITESPACE(command);

	const unichar_t* curr_pos = command;

	//	Look for the first whitespace break
	while (curr_pos[0] != 0 && curr_pos[0] != U_CHAR(' '))
		{
		++curr_pos;
		}

	// Return the string contents to the caller
	int length = ((curr_pos + 1) - command);

	if (length > 0)
		{
		unichar_t* buffer = param.Get_Buffer(length + 1);
		u_strncpy(buffer, command, length);
		buffer[length - 1] = 0;
		}

	//	Strip off whitespace
	LOCAL_STRIP_WHITESPACE(curr_pos);

	return curr_pos;
	}
