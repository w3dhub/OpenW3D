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
*     $Archive: /Commando/Code/Commando/WOLBuddyMgr.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 40 $
*     $Modtime: 8/28/02 12:59p $
*
******************************************************************************/

#include "WOLBuddyMgr.h"
#include "_globals.h"
#include "GameInitMgr.h"
#include "MPSettingsMgr.h"
#include "DlgMPWolInvitation.h"
#include "DlgMPWolPageReply.h"
#include "DlgMessageBox.h"
#include "DlgPasswordPrompt.h"
#include "DlgMPWolChat.h"
#include "WOLChatMgr.h"
#include "WOLJoinGame.h"
#include "WOLGameInfo.h"
#include "consolemode.h"
#include <WWOnline\WOLChannel.h>
#include <WWOnline\WOLProduct.h>
#include <WWLib\Registry.h>

#include "String_IDs.h"
#include <WWTranslateDB\TranslateDB.h>
#include <WWDebug\WWDebug.h>

using namespace WWOnline;

static const int MAX_USERNAME_LEN = 64;

static const WCHAR INVITE_CMD[] = L"<WWINVITE>";
static const WCHAR DECLINE_CMD[] = L"<WWDECLINE>";
static const unsigned long INVITE_CMD_LEN = ((sizeof(INVITE_CMD) / sizeof(WCHAR)) - 1);
static const unsigned long DECLINE_CMD_LEN = ((sizeof(DECLINE_CMD) / sizeof(WCHAR)) - 1);

WOLBuddyMgr* WOLBuddyMgr::_mInstance = NULL;

/******************************************************************************
*
* NAME
*     WOLBuddyMgr::GetInstance
*
* DESCRIPTION
*     Obtain Buddy manager instance
*
* INPUTS
*     Create - Okay to create buddy manager if not already instantiated.
*
* RESULT
*     Instance -
*
******************************************************************************/

WOLBuddyMgr* WOLBuddyMgr::GetInstance(bool createOK)
	{
	if (_mInstance == NULL)
		{
		new WOLBuddyMgr;

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
*     WOLBuddyMgr::WOLBuddyMgr
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

WOLBuddyMgr::WOLBuddyMgr() :
		mHidePagedDialog(0)
	{
	WWDEBUG_SAY(("WOLBuddyMgr: Instantiated\n"));
	_mInstance = this;
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::~WOLBuddyMgr
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

WOLBuddyMgr::~WOLBuddyMgr()
	{
	WWDEBUG_SAY(("WOLBuddyMgr: Destroyed\n"));
	_mInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::FinalizeCreate
*
* DESCRIPTION
*     Perform post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool WOLBuddyMgr::FinalizeCreate(void)
	{
	mWOLSession = Session::GetInstance(false);

	if (!mWOLSession.IsValid())
		{
		return false;
		}

	Observer<BuddyEvent>::NotifyMe(*mWOLSession);
	Observer<UserEvent>::NotifyMe(*mWOLSession);
	Observer<PageMessage>::NotifyMe(*mWOLSession);
	Observer<PageSendStatus>::NotifyMe(*mWOLSession);

	LoadIgnoreList();

	return true;
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::LoadIgnoreList
*
* DESCRIPTION
*     Load the list of users to ignore.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::LoadIgnoreList(void)
	{
	mIgnoreList.clear();

	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, APPLICATION_SUB_KEY_NAME_IGNORE_LIST, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == result)
		{
		// Build a list of users to ignore
		char valueName[128];
		unsigned long valueSize = sizeof(valueName);
		int index = 0;

		while (RegEnumValue(hKey, index, valueName, &valueSize, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
			DWORD type = 0;
			char name[MAX_USERNAME_LEN];
			DWORD nameSize = sizeof(name);
			result = RegQueryValueEx(hKey, valueName, NULL, &type, (LPBYTE)name, (DWORD*)&nameSize);

			if ((ERROR_SUCCESS == result) && (REG_SZ == type) && strlen(name))
				{
				// Add the name to the ignore list
				WideStringClass wideName(name);
				mIgnoreList.push_back(wideName);
				}

			index++;
			valueSize = sizeof(valueName);
			}

		RegCloseKey(hKey);
		}

	Add_Ref();
	WOLBuddyMgrEvent event(IGNORELIST_CHANGED, this);
	NotifyObservers(event);
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::SaveIgnoreList
*
* DESCRIPTION
*     Save the list of users to ignore.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::SaveIgnoreList(void)
	{
	if (mWOLSession->IsStoreLoginAllowed())
		{
		RegistryClass reg(APPLICATION_SUB_KEY_NAME_IGNORE_LIST);

		if (reg.Is_Valid())
			{
			reg.Deleta_All_Values();

			for (unsigned int index = 0; index < mIgnoreList.size(); ++index)
				{
				char valueName[64];
				sprintf(valueName, "Ignore%d", (index + 1));

				const WideStringClass& buddy = mIgnoreList[index];
				char name[MAX_USERNAME_LEN];
				wcstombs(name, buddy, sizeof(name));

				reg.Set_String(valueName, (char*)name);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::RefreshBuddyList
*
* DESCRIPTION
*     Request the buddy list for the current login.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::RefreshBuddyList(void)
	{
	mWOLSession->RequestBuddyList();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::FindBuddy
*
* DESCRIPTION
*     Find a user in our buddy list.
*
* INPUTS
*     Name - Nickname of buddy to find.
*
* RESULT
*     NONE
*
******************************************************************************/

const RefPtr<WWOnline::UserData> WOLBuddyMgr::FindBuddy(const WCHAR* name) const
	{
	return mWOLSession->FindBuddy(name);
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::AddBuddy
*
* DESCRIPTION
*     Add a user to our buddy list.
*
* INPUTS
*     Name - Nickname of user to add.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::AddBuddy(const WCHAR* name)
	{
	if (IsBuddy(name) == false)
		{
		mWOLSession->AddBuddy(name);
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::RemoveBuddy
*
* DESCRIPTION
*     Remove a user from our buddy list.
*
* INPUTS
*     Name - Nickname of user to remove.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::RemoveBuddy(const WCHAR* name)
	{
	mWOLSession->RemoveBuddy(name);
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::IsBuddy
*
* DESCRIPTION
*     Check if a user is our buddy.
*
* INPUTS
*     Name - Nickname of user to check.
*
* RESULT
*     True if user is in our buddy list.
*
******************************************************************************/

bool WOLBuddyMgr::IsBuddy(const WCHAR* name) const
	{
	RefPtr<UserData> user = mWOLSession->FindBuddy(name);
	return user.IsValid();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::RefreshBuddyInfo
*
* DESCRIPTION
*     Update the locations / status of our buddies.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::RefreshBuddyInfo(void)
	{
	const UserList& buddies = mWOLSession->GetBuddyList();
	const unsigned int count = buddies.size();

	for (unsigned int index = 0; index < count; ++index)
		{
		const RefPtr<UserData>& buddy = buddies[index];
		mWOLSession->RequestLocateUser(buddy);

		if (!buddy->GetTeamLadder().IsValid())
			{
			mWOLSession->RequestLadderInfo(buddy->GetName(), LadderType_Team);
			}

		RefPtr<SquadData> squad = buddy->GetSquad();

		if (squad.IsValid())
			{
			WideStringClass abbr(0, true);
			abbr = squad->GetAbbr();
			mWOLSession->RequestLadderInfo((const WCHAR*)abbr, LadderType_Clan);
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::GetBuddyLocationDescription
*
* DESCRIPTION
*     Get a text description of where our buddy is located.
*
* INPUTS
*     User        - User to get description for.
*     Description - String to receice description text.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::GetLocationDescription(const RefPtr<UserData>& user, WideStringClass& description)
	{
	description = TRANSLATE(IDS_MP_WOL_BUDDY_UNKNOWN);

	if (user.IsValid())
		{
		// Where is this user?
		switch (user->GetLocation())
			{
			default:
			case USERLOCATION_UNKNOWN:
				description = TRANSLATE(IDS_MP_WOL_BUDDY_UNKNOWN);
				break;

			case USERLOCATION_OFFLINE:
				description = TRANSLATE(IDS_MP_WOL_BUDDY_OFFLINE);
				break;

			case USERLOCATION_HIDING:
				description = TRANSLATE(IDS_MP_WOL_BUDDY_HIDING);
				break;

			case USERLOCATION_NO_CHANNEL:
				description = TRANSLATE(IDS_MP_WOL_BUDDY_NO_CHANNEL);
				break;

			case USERLOCATION_IN_CHANNEL:
				{
				const RefPtr<ChannelData>& channel = user->GetChannel();

				if (channel.IsValid())
					{
					WideStringClass format(0, true);
					WideStringClass channelName(0, true);

					// Check to see what type of channel this is, chat or game.
					if (channel->GetType() == 0)
						{
						format = TRANSLATE(IDS_MP_WOL_BUDDY_CHAT_ROOM);

						WOLChatMgr* chatMgr = WOLChatMgr::GetInstance(false);

						if (chatMgr)
							{
							chatMgr->GetLobbyDisplayName(channel, channelName);
							chatMgr->Release_Ref();
							}
						}
					else
						{
						format = TRANSLATE(IDS_MP_WOL_BUDDY_GAME);
						channelName = channel->GetName();
						}

					description.Format(format, channelName);
					}
				}
				break;
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::AddIgnore
*
* DESCRIPTION
*     Add a user to our ignore list.
*
* INPUTS
*     Name - Nickname of user to ignore.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::AddIgnore(const WCHAR* name)
	{
	if (!IsIgnored(name))
		{
		WideStringClass ignore(0, true);
		ignore = name;
		ignore.Trim();

		if (ignore.Get_Length() > 0)
			{
			WWDEBUG_SAY(("WOLBuddyMgr: Adding '%S' to ignore list.", name));
			mIgnoreList.push_back(ignore);

			SaveIgnoreList();

			Add_Ref();
			WOLBuddyMgrEvent event(IGNORELIST_CHANGED, this);
			NotifyObservers(event);
			Release_Ref();
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::RemoveIgnore
*
* DESCRIPTION
*     Remove a user from our ignore list.
*
* INPUTS
*     Name - Nickname of user to remove.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::RemoveIgnore(const WCHAR* name)
	{
	IgnoreList::iterator iter = mIgnoreList.begin();

	while (iter != mIgnoreList.end())
		{
		const WideStringClass& ignore = (*iter);

		if (ignore.Compare_No_Case(name) == 0)
			{
			WWDEBUG_SAY(("WOLBuddyMgr: Removing '%S' from ignore list.", (const WCHAR*)ignore));
			mIgnoreList.erase(iter);

			SaveIgnoreList();

			Add_Ref();
			WOLBuddyMgrEvent event(IGNORELIST_CHANGED, this);
			NotifyObservers(event);
			Release_Ref();
			break;
			}

		iter++;
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::IsIgnored
*
* DESCRIPTION
*     Check if a user is being ignored.
*
* INPUTS
*     Name - Nickname of user to check.
*
* RESULT
*     True if user is being ignored.
*
******************************************************************************/

bool WOLBuddyMgr::IsIgnored(const WCHAR* name) const
	{
	for (unsigned int index = 0; index < mIgnoreList.size(); index++)
		{
		const WideStringClass& ignore = mIgnoreList[index];

		if (ignore.Compare_No_Case(name) == 0)
			{
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::PageUser
*
* DESCRIPTION
*     Page a user.
*
* INPUTS
*     Name    - Nickname of user to page.
*     Message - Message to send.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::PageUser(const WCHAR* name, const WCHAR* message)
	{
	mWOLSession->PageUser(name, message);
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::ShowPagedDialog
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

void WOLBuddyMgr::ShowPagedDialog(void)
	{
	WWASSERT(mHidePagedDialog > 0 && "ShowPagedDialog() mismatch");

	if (mHidePagedDialog > 0)
		{
		--mHidePagedDialog;
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::HidePagedDialog
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

void WOLBuddyMgr::HidePagedDialog(void)
	{
	++mHidePagedDialog;
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::ClearPageList
*
* DESCRIPTION
*     Clear user pages.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::ClearPageList(void)
	{
	mPageList.clear();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::JoinUser
*
* DESCRIPTION
*     Join the user in a chat or game channel.
*
* INPUTS
*     Name - Nickname of user to join.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::JoinUser(const RefPtr<UserData>& user)
	{
	// Release any current join request.
	mPendingJoin.Release();

	if (user.IsValid())
		{
		// Before we can join a user we must first find out their location.
		// See UserEvent::Located in HandleNotification(UserEvent)
		mPendingJoin = user;
		mWOLSession->RequestLocateUser(user);
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::ProcessPendingJoin
*
* DESCRIPTION
*     Process the pending join request. Once we have the location of the user
*     we want to join we can then attempt to goto that location. If the user
*     is not in a channel then the join request is aborted.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::ProcessPendingJoin(void)
	{
	if (mPendingJoin.IsValid())
		{
		// Is the user in a place where we can join them?
		if (mPendingJoin->GetLocation() == USERLOCATION_IN_CHANNEL)
			{
			const RefPtr<ChannelData>& channel = mPendingJoin->GetChannel();

			if (channel.IsValid())
				{
				// If the channel to join is passworded then prompt the user to enter
				// a password. The join will continue after the user enters a password.
				// NOTE: See ReceiveSignal()
				if ((channel->GetType() != 0) && channel->IsPassworded())
					{
					DlgPasswordPrompt::DoDialog(this);
					}
				else
					{
					// Go ahead and attempt to join the pending location.
					GotoPendingJoinLocation(NULL);
					}
				
				return;
				}
			}

		// If we got here then we cannot join the pending location. Abort the request
		// and notify the user.
		mPendingJoin.Release();
		DlgMsgBox::DoDialog(IDS_WOL_ERROR, IDS_BUDDY_CANNOTJOIN, DlgMsgBox::Okay, NULL);
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::GotoPendingLocation
*
* DESCRIPTION
*     Goto the pending chat lobby or game.
*
* INPUTS
*     Password - Password to use to join chat lobby or game. Can be NULL if
*                a password is not required.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::GotoPendingJoinLocation(const wchar_t* password)
	{
	const RefPtr<ChannelData>& channel = mPendingJoin->GetChannel();
	WWASSERT(channel.IsValid() && "Pending join channel invalid");

	if (GameInitMgrClass::Is_Game_In_Progress())
		{
		GameInitMgrClass::End_Game();
		}

	// Determine if we should jump to a chat channel or game channel
	if (channel->GetType() == 0)
		{
		MPWolChatMenuClass::DoDialog(channel);
		}
	else if (channel->GetType() == Product::Current()->GetGameCode())
		{
		GameInitMgrClass::Set_WOL_Return_Dialog(RenegadeDialogMgrClass::LOC_INTERNET_GAME_LIST);
		WOLJoinGame::JoinTheGame(channel->GetName(), password, true);
		}

	mPendingJoin.Release();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::ReceiveSignal(DlgPasswordPrompt)
*
* DESCRIPTION
*     Handle receipt of signal that the user has entered a password to join
*     the pending join channel.
*
* INPUTS
*     PasswordDialog - Reference to password prompt dialog.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::ReceiveSignal(DlgPasswordPrompt& passwordDialog)
	{
	GotoPendingJoinLocation(passwordDialog.GetPassword());
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::CanInviteUsers
*
* DESCRIPTION
*     Check if we are in a position to invite users.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

bool WOLBuddyMgr::CanInviteUsers(void) const
	{
	return (ChannelJoined == mWOLSession->GetChannelStatus());
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::InviteUser
*
* DESCRIPTION
*     Invite a user to join our location (Chat or Game)
*
* INPUTS
*     Name - Nickname of user to invite.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::InviteUser(const WCHAR* username, const WCHAR* message)
	{
	WideStringClass name(0, true);
	name = username;
	name.Trim();

	if (name.Get_Length() > 0)
		{
		if (message && (wcslen(message) > 0))
			{
			WideStringClass invitation(0, true);
			invitation.Format(L"%s%s", INVITE_CMD, message);
			PageUser(name, invitation);
			}
		else
			{
			PageUser(name, INVITE_CMD);
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::DeclineInvitation
*
* DESCRIPTION
*     Decline an invitation to join a user.
*
* INPUTS
*     Name - Nickname of user to decline.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::DeclineInvitation(const WCHAR* username, DECLINE_REASON reason)
	{
	if (username && (wcslen(username) > 0))
		{
		// Build a decline "page"
		WideStringClass response(0, true);
		response.Format(L"%s%d", DECLINE_CMD, reason);

		// Send the response
		PageUser(username, response);
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::IsCommand
*
* DESCRIPTION
*     Check if a message is a command.
*
* INPUTS
*     Message - Message to check.
*
* RESULT
*     True if message is a command
*
******************************************************************************/

bool WOLBuddyMgr::IsCommand(const WCHAR* message)
	{
	// All commands begin with "<WW"
	return (message && (wcsstr(message, L"<WW") == message));
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::IsInvitation
*
* DESCRIPTION
*     Check if a message is an invitation.
*
* INPUTS
*     Message - Message to check.
*
* RESULT
*     True if message is an invitation.
*
******************************************************************************/

bool WOLBuddyMgr::IsInvitation(const WCHAR* message)
	{
	return (message && (wcslen(message) >= INVITE_CMD_LEN) &&
			(wcsncmp(message, INVITE_CMD, INVITE_CMD_LEN) == 0));
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::InvitationReceived
*
* DESCRIPTION
*     Handle invitation page from another user.
*
* INPUTS
*     Page - Invitation page.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::InvitationReceived(PageMessage& page)
	{
	const WideStringClass& pagerName = page.GetPagersName();

	// Check if we already have an invitation request from this user.
	// If so then ignore any subsequent invitations from him.
	PageMessageList::iterator iter = mInvitations.begin();

	while (iter != mInvitations.end())
		{
		// If this is in response to a another users invitation to join
		// then display the invitation since we now know were they are.
		if (pagerName.Compare_No_Case((*iter).GetPagersName()) == 0)
			{
			return;
			}

		iter++;
		}

	// Before we can display the invitation we must first find out where the
	// invitor is located. Once we have his location we can prompt the user.
	// See UserEvent::Located handling in HandleNotification(UserEvent)
	mInvitations.push_back(page);
	mWOLSession->RequestLocateUser(pagerName);
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::DisplayInvitation
*
* DESCRIPTION
*     Display an invitation from another user.
*
* INPUTS
*     User - User inviting us.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::DisplayInvitation(const RefPtr<UserData>& user, const WCHAR* message)
	{
	if (user->GetLocation() != USERLOCATION_IN_CHANNEL)
		{
		// Decline the invitation because the user is not in a channel.
		DeclineInvitation(user->GetName(), DECLINE_NOTAPPLICABLE);
		}
	else
		{
		//-------------------------------------------------------------------------
		// Compose the invitation message
		//-------------------------------------------------------------------------

		// Get the user's name
		const WideStringClass& name = user->GetName();

		// Build a textual description of the user's location
		WideStringClass location(0, true);
		GetLocationDescription(user, location);

		// Format the invitation message
		WideStringClass inviteMsg(0, true);
		inviteMsg.Format(TRANSLATE(IDS_MP_WOL_INVITATION_FORMAT), (const WCHAR*)name, (const WCHAR*)location);
		inviteMsg += L"\n";
		inviteMsg += message;

		if (!mHidePagedDialog)
			{
			// Display the invitation dialog
			MPWolInvitationPopupClass* dialog = new MPWolInvitationPopupClass(user, inviteMsg);
			WWASSERT(dialog && "Failed to create invitation dialog");

			if (dialog)
				{
				dialog->Start_Dialog();
				dialog->Release_Ref();
				}
			}

		Add_Ref();
		PageMessage invite(user->GetName(), inviteMsg);
		WOLPagedEvent event(INVITATION_RECEIVED, &invite);
		NotifyObservers(event);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::IsInvitationDeclined
*
* DESCRIPTION
*     Check if the invitation was declined.
*
* INPUTS
*     Message - Message to check.
*
* RESULT
*     True if nvitation was declined.
*
******************************************************************************/

bool WOLBuddyMgr::IsInvitationDeclined(const WCHAR *message)
	{
	return (message && (wcslen(message) >= DECLINE_CMD_LEN) &&
			(wcsncmp(message, DECLINE_CMD, DECLINE_CMD_LEN) == 0));
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::InvitationDeclined
*
* DESCRIPTION
*     Handle declined invitation.
*
* INPUTS
*     Name   - Nickname of user.
*     Reason - Reason for declining.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::InvitationDeclined(const WCHAR* username, DECLINE_REASON reason)
	{
	WideStringClass message(0, true);

	// Build an appropriate message based on the reason
	switch (reason)
		{
		default:
		case DECLINE_BYUSER:
			message = TRANSLATE(IDS_INVITE_DECLINE_BYUSER);
			break;

		case DECLINE_NOTBUDDY:
			message.Format(TRANSLATE(IDS_INVITE_DECLINE_NOTBUDDY), username);
			break;

		case DECLINE_NOTAPPLICABLE:
			message = TRANSLATE(IDS_INVITE_DECLINE_NOTAPPLICABLE);
			break;

		case DECLINE_BUSY:
			message.Format(TRANSLATE(IDS_INVITE_DECLINE_BUSY), username);
			break;
		}

	if (!mHidePagedDialog)
		{
		// Let the user know they've been declined
		DlgMsgBox::DoDialog(NULL, message, DlgMsgBox::Okay, NULL);
		}

	Add_Ref();
	PageMessage decline(username, message);
	WOLPagedEvent event(INVITATION_DECLINED, &decline);
	NotifyObservers(event);
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::HandleNotification(BuddyEvent)
*
* DESCRIPTION
*     Handle buddy event notifications. (List, Add, Remove)
*
* INPUTS
*     Event
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::HandleNotification(BuddyEvent& event)
	{
	bool listChanged = false;

	if ((event.GetEvent() == BuddyEvent::NewList) || (event.GetEvent() == BuddyEvent::Added))
		{
		const UserList& buddies = event.Subject();
		unsigned int count = buddies.size();

		for (unsigned int index = 0; index < count; ++index)
			{
			const RefPtr<UserData>& buddy = buddies[index];

			// Request details about the buddy
			if (buddy.IsValid())
				{
				mWOLSession->RequestLocateUser(buddy);

				// Request squad affliation for this user.
				if (!buddy->GetSquad().IsValid())
					{
					mWOLSession->RequestSquadInfoByMemberName(buddy->GetName());
					}

				// Request ranking information for this user.
				if (!buddy->GetTeamLadder().IsValid())
					{
					mWOLSession->RequestLadderInfo(buddy->GetName(), LadderType_Team);
					}
				}
			}

		listChanged = true;
		}
	else if (event.GetEvent() == BuddyEvent::Deleted)
		{
		listChanged = true;
		}

	if (listChanged)
		{
		Add_Ref();
		WOLBuddyMgrEvent event(BUDDYLIST_CHANGED, this);
		NotifyObservers(event);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::HandleNotification(UserEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::HandleNotification(UserEvent& event)
	{
	switch (event.GetEvent())
		{
		// Users location status obtained.
		case UserEvent::Located:
			{
			const RefPtr<UserData>& user = event.Subject();
			const WideStringClass& username = user->GetName();

			// Try to find an invitation from this user.
			bool foundInvitation = false;
			PageMessageList::iterator iter = mInvitations.begin();

			while (iter != mInvitations.end())
				{
				// If this is in response to a another users invitation to join them
				// then display the invitation since we now know were they are.
				if (username.Compare_No_Case((*iter).GetPagersName()) == 0)
					{
					const WCHAR* invite = (*iter).GetPageMessage();
					DisplayInvitation(user, &invite[INVITE_CMD_LEN]);

					mInvitations.erase(iter);
					foundInvitation = true;
					break;
					}

				iter++;
				}

			// Check if we requested to join a user.
			if ((foundInvitation == false) && mPendingJoin.IsValid())
				{
				// If this is the user we requested to join then goto their
				// location.
				if (username.Compare_No_Case(mPendingJoin->GetName()) == 0)
					{
					ProcessPendingJoin();
					return;
					}
				}

			// Report buddy locations
			if (IsBuddy(username))
				{
				Add_Ref();
				WOLBuddyMgrEvent event(BUDDYINFO_CHANGED, this);
				NotifyObservers(event);
				Release_Ref();
				}
			}
		break;

		case UserEvent::SquadInfo:
		case UserEvent::LadderInfo:
			if (IsBuddy(event.Subject()->GetName()))
				{
				Add_Ref();
				WOLBuddyMgrEvent event(BUDDYINFO_CHANGED, this);
				NotifyObservers(event);
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
*     WOLBuddyMgr::HandleNotification(PageMessage)
*
* DESCRIPTION
*     Handle pages from other users.
*
* INPUTS
*     Page - Page message to process.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::HandleNotification(PageMessage& page)
	{
	// Only accept pages from users we are not ignoring.
	if (IsIgnored(page.GetPagersName()) == false)
		{
		const WideStringClass& pager = page.GetPagersName();
		const WideStringClass& message = page.GetPageMessage();
		mLastPagersName = pager;

		if (ConsoleBox.Is_Exclusive()) {
			if (((WideStringClass*)&message)->Is_ANSI()) {
				WideStringClass temp(L"[Page] ", true);
				temp += pager;
				temp += L": ";
				temp += message;
				temp += L"\n";
				Vector3 text_color(0.0f, 1.0f, 0.0f);
				ConsoleBox.Add_Message(&temp, &text_color, true);
			}
		} else {

			// If this page is not a command then show it to the user.
			if (!IsCommand(message))
				{
				bool allowPages = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_ALLOW_PAGES);

				if (allowPages)
					{
					// Popup a page reply dialog for the player to respond to.
					if (!mHidePagedDialog)
						{
						DlgWOLPageReply::DoDialog();
						}

					// Notify that there are new page.
					Add_Ref();
					WOLPagedEvent event(PAGE_RECEIVED, &page);
					NotifyObservers(event);
					Release_Ref();
					}
				}
			else if (IsInvitation(message))
				{
				// A user has sent us an invitation
				InvitationReceived(page);
				}
			else if (IsInvitationDeclined(message))
				{
				// A user declined our invitation.
				DECLINE_REASON reason = DECLINE_BYUSER;

				// Grab the reason code
				const WCHAR* codeString = message + DECLINE_CMD_LEN;
				int code = _wtoi(codeString);

				if (code > DECLINE_MIN && code < DECLINE_MAX)
					{
					reason = (DECLINE_REASON)code;
					}

				// The user we invited has declined our invitation.
				InvitationDeclined(pager, reason);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLBuddyMgr::HandleNotification(PageSendStatus)
*
* DESCRIPTION
*
* INPUTS
*     Status
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLBuddyMgr::HandleNotification(PageSendStatus& pageStatus)
	{
	WOLPagedAction action = PAGE_ERROR;
	uint32 msgID = 0;

	switch (pageStatus)
		{
		case PAGESEND_ERROR:
			action = PAGE_ERROR;
			msgID = IDS_PAGESEND_ERROR;
			break;

		case PAGESEND_OFFLINE:
			action = PAGE_NOT_THERE;
			msgID = IDS_PAGESEND_OFFLINE;
			break;

		case PAGESEND_HIDING:
			action = PAGE_TURNED_OFF;
			msgID = IDS_PAGESEND_HIDING;
			break;

		case PAGESEND_SENT:
		default:
			return;
		}

	// If the page was undeliverable then tell the user why.
	if (!mHidePagedDialog && !DlgWOLPageReply::IsOpen())
		{
		DlgMsgBox::DoDialog(IDS_WOL_PAGEUSERERROR, msgID);
		}

	Add_Ref();

	PageMessage page(NULL, TRANSLATE(msgID));
	WOLPagedEvent event(action, &page);
	NotifyObservers(event);

	Release_Ref();
	}