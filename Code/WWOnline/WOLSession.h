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
*     $Archive: /Commando/Code/WWOnline/WOLSession.h $
*
* DESCRIPTION
*     WOLSession is the entryway to Westwood Online. An object of this type
*     must exist in order to do anything WOL related. There should only be one
*     instance of this; retrieve it via GetInstance.
*
* PROGRAMMER
*     Steve Clinard & Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 54 $
*     $Modtime: 1/25/02 5:02p $
*
******************************************************************************/

#ifndef __WOLSESSION_H__
#define __WOLSESSION_H__

#include <windows.h>
#include "atlbase_compat.h"
#include "WOLLoginInfo.h"
#include "WOLUser.h"
#include "WOLChannel.h"
#include "WOLPing.h"
#include "WOLAgeCheck.h"
#include "WOLDownload.h"
#include "WOLPageMsg.h"
#include "RefPtr.h"
#include <wwlib/Notify.h>
#include <wwlib/widestring.h>

namespace WOL
{
#include <wolapi/WOLAPI.h>
}

#include <vector>

class WaitCondition;

namespace WWOnline {

// The version of WOLAPI this product was build with.
#define WOLAPI_BUILD_VERSION MAKELONG(19,1) 

class ChatObserver;
class NetUtilObserver;
class ServerError;
class ServerData;
class ChannelData;
class LadderServerData;
class GameResultsServerData;
class WDTServerData;
class GameOptionsMessage;
class GameStartEvent;
class SquadEvent;
class ChatMessage;
class ConnectWait;
class DisconnectWait;

class IRCServerData;
typedef std::vector< RefPtr<IRCServerData> > IRCServerList;

class MGLServerData;
typedef std::vector< RefPtr<MGLServerData> > MGLServerList;

class PingServerData;
typedef std::vector< RefPtr<PingServerData> > PingServerList;

typedef std::vector< RefPtr<LadderData> > LadderList;

enum ConnectionStatus {ConnectionDisconnected, ConnectionDisconnecting, ConnectionConnecting, ConnectionConnected};

class MessageOfTheDayType;
typedef TypedEvent<MessageOfTheDayType, WideStringClass> MessageOfTheDayEvent;

// Information request flags
#define REQUEST_LOCALE     (1<<0L)
#define REQUEST_SQUADINFO  (1<<1L)
#define REQUEST_TEAMINFO   (1<<2L)
#define REQUEST_LADDERINFO (1<<3L)
#define REQUEST_ALL (REQUEST_LOCALE|REQUEST_SQUADINFO|REQUEST_TEAMINFO|REQUEST_LADDERINFO)
#define REQUEST_NONE 0

class Session :
		public RefCounted,
		public Notifier<ServerError>,
		public Notifier<ConnectionStatus>,
		public Notifier<IRCServerList>,
		public Notifier<ChannelListEvent>,
		public Notifier<UserList>,
		public Notifier<UserEvent>,
		public Notifier<BuddyEvent>,
		public Notifier<ChannelEvent>,
		public Notifier<MessageOfTheDayEvent>,
		public Notifier<ChatMessage>,
		public Notifier<PageMessage>,
		public Notifier<PageSendStatus>,
		public Notifier<GameOptionsMessage>,
		public Notifier<GameStartEvent>,
		public Notifier<RawPing>,
		public Notifier<LadderInfoEvent>,
		public Notifier<LadderList>,
		public Notifier<SquadEvent>,
		public Notifier<NewLoginInfoEvent>,
		public Notifier<AgeCheckEvent>,
		public Notifier<UserIPEvent>
	{
	public:
		// Retrieve WWOnline session instance.
		static RefPtr<Session> GetInstance(bool okToCreate = false);

		void Reset(void);

		// Yeild time for WWOnline processing.
		virtual bool Process(void);

		// Retrieve pointer to IChat object
		const CComPtr<WOL::IChat>& GetChatObject(void) const
			{return mChat;}

		// Retrieve pointer to INetUtil object
		const CComPtr<WOL::INetUtil>& GetNetUtilObject(void) const
			{return mNetUtil;}

		// Obtain current patch download list
		DownloadList& GetPatchDownloadList(void)
			{return mPatchFiles;}

		// Retrieve connection status
		ConnectionStatus GetConnectionStatus(void) const
			{return mCurrentConnectionStatus;}

		//-------------------------------------------------------------------------
		// Server Methods
		//-------------------------------------------------------------------------

		// Retrieve new server list
		RefPtr<WaitCondition> GetNewServerList(void);

		// Submit request for new server list
		bool RequestServerList(bool just_kidding = false);

		// Obtain current server list
		const IRCServerList& GetIRCServerList(void)
			{return mIRCServers;}

		const MGLServerList& GetManglerServerList(void)
			{return mMGLServers;}

		const PingServerList& GetPingServerList(void)
			{return mPingServers;}

		// Retrieve current server
		const RefPtr<IRCServerData>& GetCurrentServer(void) const
			{return mCurrentServer;}

		// Retrieve current login
		const RefPtr<LoginInfo>& GetCurrentLogin(void) const
			{return mCurrentLogin;}

		// Login into server
		RefPtr<WaitCondition> LoginServer(const RefPtr<IRCServerData>&, const RefPtr<LoginInfo>&);

		// Disconnection from current server.
		RefPtr<WaitCondition> Logout(void);

		// Retrieve message of the day
		const WideStringClass& GetMessageOfTheDay(void) const
			{return mMessageOfTheDay;}

		//-------------------------------------------------------------------------
		// Channel Methods
		//-------------------------------------------------------------------------

		bool EnableProgressiveChannelList(bool enable);

		// Submit request for new channel list
		bool RequestChannelList(int channelType, bool autoPing);

		// Look for a channel with the specified name in the current channel list.
		RefPtr<ChannelData> FindChannel(const unichar_t* name);
		RefPtr<ChannelData> FindChannel(const char* name);

		// Create a new channel
		RefPtr<WaitCondition> CreateChannel(const unichar_t* channelName, const unichar_t* password, int type);
		RefPtr<WaitCondition> CreateChannel(const RefPtr<ChannelData>&, const unichar_t* password);

		// Join a channel
		RefPtr<WaitCondition> JoinChannel(const RefPtr<ChannelData>& channel, const unichar_t* password);
		RefPtr<WaitCondition> JoinChannel(const unichar_t* name, const unichar_t* password, int type);
		bool RequestChannelJoin(const RefPtr<ChannelData>& channel, const unichar_t* password);

		// Leave the current channel.
		RefPtr<WaitCondition> LeaveChannel(void);

		bool RequestLeaveChannel(void);

		// Obtain current channel
		const RefPtr<ChannelData>& GetCurrentChannel(void) const
			{return mCurrentChannel;}

		const char* GetChannelTopic(void) const;

		// Send the current channels topic to the server.
		bool SendChannelTopic(void);

		// Send the current channels extra info to the server.
		bool SendChannelExtraInfo(void);

		// Retrieve current channel status
		ChannelStatus GetChannelStatus(void) const
			{return mCurrentChannelStatus;}

		// Retrieve new chat channel list
		RefPtr<WaitCondition> GetNewChatChannelList(void);

		// Obtain current channel list.
		const ChannelList& GetChatChannelList(void)
			{return mChatChannels;}

		RefPtr<ChannelData> FindChatChannel(const unichar_t* name);
		RefPtr<ChannelData> FindChatChannel(const char* name);

		// Retrieve new game channel list
		RefPtr<WaitCondition> GetNewGameChannelList(void);

		// Submit request for new game channel list.
		bool RequestGameChannelList(void);

		// Obtain current game channel list
		const ChannelList& GetGameChannelList(void)
			{return mGameChannels;}

		RefPtr<ChannelData> FindGameChannel(const unichar_t* name);
		RefPtr<ChannelData> FindGameChannel(const char* name);

		//-------------------------------------------------------------------------
		// User Methods
		//-------------------------------------------------------------------------

		// Request a new user list
		bool RequestUserList(void);

		// Obtian current user list
		const UserList& GetUserList(void)
			{return mUsers;}

		// Find specified user
		RefPtr<UserData> FindUser(const unichar_t* userName);

		// Obtain current user
		RefPtr<UserData> GetCurrentUser(void) const
			{return mCurrentUser;}

		// Test is specified user is the current one.
		bool IsCurrentUser(const RefPtr<UserData>&) const;
		bool IsCurrentUser(const unichar_t* username) const;

		// Change the locale for the user currently logged in.
		bool ChangeCurrentUserLocale(WOL::Locale locale);

		// Squelch / unsquelch user messages. Prevents messages from specified user.
		bool SquelchUser(const RefPtr<UserData>& user, bool squelched);

		// Kick a user from the current channel. Kicking is only allowed for
		// channel owners.
		bool KickUser(const unichar_t* username);

		// Ban / unban a user from the current channel. Banning is only allowed for
		// channel owners.
		bool BanUser(const unichar_t* username, bool banned);

		// Send a page to a user.
		bool PageUser(const unichar_t* userName, const unichar_t* message);

		// Request a users online location
		unsigned int GetAutoRequestFlags(void) const
			{return mAutoRequestFlags;}

		void SetAutoRequestFlags(unsigned int flags)
			{mAutoRequestFlags = flags;}

		void RequestUserDetails(const RefPtr<UserData>& user, unsigned int requestFlags);

		void RequestLocateUser(const unichar_t* userName);
		void RequestLocateUser(const RefPtr<UserData>& user);

		// Request users locale
		void RequestUserLocale(const unichar_t* userName);

		// Request information about a squad by Squad ID
		void RequestSquadInfoByID(unsigned int squadID);

		// Request information about a squad by member name
		void RequestSquadInfoByMemberName(const unichar_t* memberName);

		// Request users team information
		void RequestTeamInfo(const unichar_t* userName);

		// Request users ladder information
		void RequestLadderInfo(const unichar_t* name, unsigned int type);

		//-------------------------------------------------------------------------
		// Buddy Methods
		//-------------------------------------------------------------------------

		// Obtian current buddy list
		const UserList& GetBuddyList(void) const
			{return mBuddies;}

		// Find specified buddy
		RefPtr<UserData> FindBuddy(const unichar_t* buddyName)
			{return FindUserInList(buddyName, mBuddies);}

		bool RequestBuddyList(void);
		bool AddBuddy(const unichar_t* buddyName);
		bool RemoveBuddy(const unichar_t* buddyName);

		//-------------------------------------------------------------------------
		// Chatting Methods
		//-------------------------------------------------------------------------
		bool AllowFindPage(bool allowFind, bool allowPage);

		// Enable / disable filtering of bad language
		bool SetBadLanguageFilter(bool enabled);

		bool SendPublicMessage(const char* message);
		bool SendPublicMessage(const unichar_t* message);
		bool SendPublicAction(const char* action);
		bool SendPublicAction(const unichar_t* action);

		bool SendPrivateMessage(const unichar_t* username, const unichar_t* message);
		bool SendPrivateMessage(const UserList& users, const char* message);
		bool SendPrivateMessage(const UserList& users, const unichar_t* message);
		bool SendPrivateAction(const UserList& users, const char* action);
		bool SendPrivateAction(const UserList& users, const unichar_t* action);

		bool SendPublicGameOptions(const char* options);
		bool SendPrivateGameOptions(const unichar_t* user, const char* options);
		bool SendPrivateGameOptions(const UserList& users, const char* options);

		//-------------------------------------------------------------------------
		// Utility Methods
		//-------------------------------------------------------------------------
		void GetLocaleStrings(std::vector<WideStringClass>& localeStrings);

		void EnablePinging(bool enable);
		void RequestPing(const char* address, int timeout = 1000);

		unsigned int GetPendingPingCount(void) const
			{return mPingsPending;}

		bool SendGameResults(unsigned char* packet, unsigned int length);

		bool RequestUserIP(char *user_name);

		time_t GetServerTime(void)
			{return mServerTime;}

		//-------------------------------------------------------------------------
		// IGR (Internet Gaming Room) support
		//-------------------------------------------------------------------------
		bool IsStoreLoginAllowed(void);
		bool IsAutoLoginAllowed(void);
		bool IsRunRegAppAllowed(void);

		//-------------------------------------------------------------------------
		// Insider status support
		//-------------------------------------------------------------------------
		void RequestInsiderStatus(void);
		void RequestServerTime(void);

		bool IsCurrUserInsider(void)
			{return mIsInsider;}

		//-------------------------------------------------------------------------
		// Event notification
		//-------------------------------------------------------------------------
		DECLARE_NOTIFIER(ServerError)
		DECLARE_NOTIFIER(ConnectionStatus)
		DECLARE_NOTIFIER(IRCServerList)
		DECLARE_NOTIFIER(ChannelListEvent)
		DECLARE_NOTIFIER(UserList)
		DECLARE_NOTIFIER(UserEvent)
		DECLARE_NOTIFIER(BuddyEvent)
		DECLARE_NOTIFIER(ChannelEvent)
		DECLARE_NOTIFIER(MessageOfTheDayEvent)
		DECLARE_NOTIFIER(ChatMessage)
		DECLARE_NOTIFIER(PageMessage)
		DECLARE_NOTIFIER(PageSendStatus)
		DECLARE_NOTIFIER(GameOptionsMessage)
		DECLARE_NOTIFIER(GameStartEvent)
		DECLARE_NOTIFIER(RawPing)
		DECLARE_NOTIFIER(LadderInfoEvent)
		DECLARE_NOTIFIER(LadderList)
		DECLARE_NOTIFIER(SquadEvent)
		DECLARE_NOTIFIER(NewLoginInfoEvent)
		DECLARE_NOTIFIER(AgeCheckEvent)
		DECLARE_NOTIFIER(UserIPEvent)

	protected:
		friend ChatObserver;
		friend NetUtilObserver;
		friend Download;
		friend ConnectWait;
		friend DisconnectWait;

		Session();
		virtual ~Session();

		// Disallow copy and assignment
		Session(const Session&);
		const Session& operator=(const Session&);

		virtual void ReleaseReference(void) override;
		bool FinalizeCreate(void);

		void UpdatePingServerTime(const char* name, int time);

		void AutoRequestUserDetails(const RefPtr<UserData>& user);

		RefPtr<UserData> GetUserOrBuddy(const unichar_t*);

		const CComPtr<WOL::IIGROptions>& GetIGRObject(void);
		
	private:
		void ClearServers(void);

		void MakeLocateUserRequests(void);
		void MakeSquadRequests(void);
		void MakeLocaleRequests(void);
		void MakeTeamRequests(void);
		void MakeLadderRequests(void);
		void MakePingRequests(void);

		CComPtr<WOL::IChat> mChat;
		CComPtr<WOL::IChatEvent> mChatEvents;
		DWORD mChatCookie;

		CComPtr<WOL::INetUtil> mNetUtil;
		CComPtr<WOL::INetUtilEvent> mNetUtilEvents;
		DWORD mNetUtilCookie;

		CComPtr<WOL::IIGROptions> mIGRObject;

		// Server data
		bool mRequestingServerList;
		IRCServerList mIRCServers;
		RefPtr<LadderServerData> mLadderServer;
		RefPtr<GameResultsServerData> mGameResultsServer;
		RefPtr<WDTServerData> mWDTServer;
		MGLServerList mMGLServers;
		PingServerList mPingServers;
		bool mIgnoreServerLists;

		// Insider status data
		bool mIsInsider;
		time_t mServerTime;

		// Patch data
		DownloadList mPatchFiles;

		// Connection state
		ConnectionStatus mCurrentConnectionStatus;
		RefPtr<IRCServerData> mPendingServer;
		RefPtr<IRCServerData> mCurrentServer;
		RefPtr<LoginInfo> mPendingLogin;
		RefPtr<LoginInfo> mCurrentLogin;

		// Channel data
		ChannelList mChatChannels;
		ChannelList mGameChannels;

		int mRequestedChannelType;
		ChannelList mIncommingChatChannels;
		ChannelList mIncommingGameChannels;

		RefPtr<ChannelData> mPendingChannel;
		RefPtr<ChannelData> mCurrentChannel;

		ChannelStatus mCurrentChannelStatus;

		// User data
		UserList mUsers;
		RefPtr<UserData> mCurrentUser;

		UserList mLocatePendingUsers;
		RefPtr<UserData> mLocatingUser;

		UserList mBuddies;

		unsigned int mAutoRequestFlags;

      // Squad Request queues
		typedef std::vector<WideStringClass> SquadRequestColl;
		SquadRequestColl mSquadRequests;
		SquadRequestColl mSquadPending;

		typedef std::vector<WideStringClass> LocaleRequestColl;
		LocaleRequestColl mLocaleRequests;

		typedef std::vector<WideStringClass> TeamRequestColl;
		TeamRequestColl mTeamRequests;

		typedef std::list<WideStringClass> LadderRequestList;
		LadderRequestList mLadderRequests;
		unsigned int mLadderPending;

		// Misc.
		WideStringClass mMessageOfTheDay;

		std::vector<RawPing> mPingRequests;
		unsigned int mPingsPending;
		int mPingEnable;

		unsigned int mLastUserDataRequestTime;
		static RefPtr<Session> _mInstance;
	};


class ChatAdvisement :
		public RefCounted
	{
	public:
		static RefPtr<ChatAdvisement> Create(const CComPtr<WOL::IChat>&, const CComPtr<WOL::IChatEvent>&);

	private:
		ChatAdvisement(const CComPtr<WOL::IChat>&, const CComPtr<WOL::IChatEvent>&);
		virtual ~ChatAdvisement();

		// Prevent copy and assignment
		ChatAdvisement(const ChatAdvisement&);
		const ChatAdvisement& operator=(const ChatAdvisement&);

		CComPtr<WOL::IChat> mChat;
		DWORD mChatCookie;
	};

} // using namespace WWOnline

#endif // __WOLSESSION_H__
