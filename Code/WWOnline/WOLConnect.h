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
*     $Archive: /Commando/Code/WWOnline/WOLConnect.h $
*
* DESCRIPTION
*     Westwood Online wait conditions.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 1/29/02 1:44p $
*
******************************************************************************/

#ifndef __WOLCONNECT_H__
#define __WOLCONNECT_H__

#include "WOLSession.h"
#include "WaitCondition.h"
#include "WOLChatMsg.h"
#include <wwlib/Notify.h>
#include <wwlib/widestring.h>
#include "RefPtr.h"

namespace WWOnline {

const unichar_t* const DefaultPassword = U_CHAR("zotclot9");

//-----------------------------------------------------------------------------
// Retrieve serverlist wait condition
//-----------------------------------------------------------------------------
class ServerListWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<IRCServerList>
	{
	public:
		static RefPtr<ServerListWait> Create(const RefPtr<Session>& session);

		void WaitBeginning(void) override;

	protected:
		ServerListWait(const RefPtr<Session>&);
		virtual ~ServerListWait();

		ServerListWait(const ServerListWait&);
		const ServerListWait& operator=(const ServerListWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(IRCServerList& serverList) override;

		RefPtr<Session> mSession;
	};


//-----------------------------------------------------------------------------
// Disconnect from WWOnline wait condition
//-----------------------------------------------------------------------------
class DisconnectWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>
	{
	public:
		static RefPtr<DisconnectWait> Create(const RefPtr<Session>& session);

		void WaitBeginning(void) override;
		void EndWait(WaitResult, const unichar_t*) override;

	protected:
		DisconnectWait(const RefPtr<Session>& session);
		virtual ~DisconnectWait();

		DisconnectWait(const DisconnectWait&);
		const DisconnectWait& operator=(const DisconnectWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;

		RefPtr<Session> mSession;
	};


//-----------------------------------------------------------------------------
// Connect to WWOnline wait condition
//-----------------------------------------------------------------------------
class ConnectWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>
	{
	public:
		static RefPtr<ConnectWait> Create(const RefPtr<Session>& session,
				const RefPtr<ServerData>& server, const RefPtr<LoginInfo>& login);

		void WaitBeginning(void) override;
		void EndWait(WaitResult, const unichar_t*) override;

	protected:
		ConnectWait(const RefPtr<Session>& session, const RefPtr<ServerData>& server,
				const RefPtr<LoginInfo>& login);

		virtual ~ConnectWait();

		ConnectWait(const ConnectWait&);
		const ConnectWait& operator=(const ConnectWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;

		RefPtr<Session> mSession;
		RefPtr<ServerData> mServer;
		RefPtr<LoginInfo> mLogin;
	};


//-----------------------------------------------------------------------------
// Request for chat channel list wait condition
//-----------------------------------------------------------------------------
class ChannelListWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>,
		public Observer<ChannelListEvent>
	{
	public:
		static RefPtr<ChannelListWait> Create(const RefPtr<Session>& session, int type);
		void WaitBeginning(void) override;

	protected:
		ChannelListWait(const RefPtr<Session>& session, int type);
		virtual ~ChannelListWait();

		ChannelListWait(const ChannelListWait&);
		const ChannelListWait& operator=(const ChannelListWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;
		void HandleNotification(ChannelListEvent& event) override;

		RefPtr<Session> mSession;
		int mType;
	};


//-----------------------------------------------------------------------------
// Request to leave a channel wait condition
//-----------------------------------------------------------------------------
class LeaveChannelWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>,
		public Observer<ChannelEvent>
	{
	public:
		static RefPtr<LeaveChannelWait> Create(const RefPtr<Session>& session);

		void WaitBeginning(void) override;

	protected:
		LeaveChannelWait(const RefPtr<Session>& session);
		virtual ~LeaveChannelWait();

		LeaveChannelWait(const LeaveChannelWait&);
		const LeaveChannelWait& operator=(const LeaveChannelWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;
		void HandleNotification(ChannelEvent& status) override;

		RefPtr<Session> mSession;
	};


//-----------------------------------------------------------------------------
// Request to join a channel wait condition
//-----------------------------------------------------------------------------
class JoinChannelWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>,
		public Observer<ChannelEvent>
	{
	public:
		static RefPtr<JoinChannelWait> Create(const RefPtr<Session>& session,
				const RefPtr<ChannelData>& channel, const unichar_t* password);

		static RefPtr<JoinChannelWait> Create(const RefPtr<Session>& session,
				const unichar_t* channelName, const unichar_t* password, int type);

		void WaitBeginning(void) override;

	protected:
		JoinChannelWait(const RefPtr<Session>& session, const RefPtr<ChannelData>& channel,
				const unichar_t* password);

		JoinChannelWait(const RefPtr<Session>& session, const unichar_t* channelName,
				const unichar_t* password, int type);

		virtual ~JoinChannelWait();

		JoinChannelWait(const JoinChannelWait&);
		const JoinChannelWait& operator=(const JoinChannelWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;
		void HandleNotification(ChannelEvent& status) override;

		RefPtr<Session> mSession;
		RefPtr<ChannelData> mChannel;

		WideStringClass mChannelName;
		WideStringClass mPassword;
		int mType;
	};


//-----------------------------------------------------------------------------
// Request to create a new channel wait condition
//-----------------------------------------------------------------------------
class CreateChannelWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<ConnectionStatus>,
		public Observer<ChannelEvent>
	{
	public:
		static RefPtr<CreateChannelWait> Create(const RefPtr<Session>& session,
				const RefPtr<ChannelData>& channel, const unichar_t* password);

		void WaitBeginning(void) override;

	protected:
		CreateChannelWait(const RefPtr<Session>& session, const RefPtr<ChannelData>& channel,
					const unichar_t* password);

		virtual ~CreateChannelWait();

		CreateChannelWait(const CreateChannelWait&);
		const CreateChannelWait& operator=(const CreateChannelWait&);

		void HandleNotification(ServerError& serverError) override;
		void HandleNotification(ConnectionStatus& status) override;
		void HandleNotification(ChannelEvent& status) override;

		RefPtr<Session> mSession;
		RefPtr<ChannelData> mChannel;
		WideStringClass mPassword;
	};


//-----------------------------------------------------------------------------
// Request a new user list wait condition
//-----------------------------------------------------------------------------
class UserListWait :
		public SerialWait,
		public Observer<ServerError>,
		public Observer<UserList>
	{
	public:
		static RefPtr<UserListWait> Create(const RefPtr<Session>& session);

		void WaitBeginning(void) override;

	protected:
		UserListWait(const RefPtr<Session>& session);
		virtual ~UserListWait();

		UserListWait(const UserListWait&);
		const UserListWait& operator=(const UserListWait&);

		void HandleNotification(ServerError&) override;
		void HandleNotification(UserList&) override;

		RefPtr<Session> mSession;
	};


//-----------------------------------------------------------------------------
// Request to find a user wait condition
//-----------------------------------------------------------------------------
class GetUserWait :
		public SerialWait
	{
	public:
		static RefPtr<GetUserWait> Create(const RefPtr<Session>& session, const unichar_t* userName);

		void WaitBeginning(void) override;
		WaitResult GetResult(void) override;

	protected:
		GetUserWait(const RefPtr<Session>& session, const unichar_t* userName);
		virtual ~GetUserWait();

		GetUserWait(const GetUserWait&);
		const GetUserWait& operator=(const GetUserWait&);

		RefPtr<Session> mSession;
		WideStringClass mUserName;
	};


//-----------------------------------------------------------------------------
// Request to locate a user wait condition
//-----------------------------------------------------------------------------
class LocateUserWait :
		public SingleWait,
		public Observer<ServerError>,
		public Observer<UserEvent>
	{
	public:
		static RefPtr<LocateUserWait> Create(const RefPtr<Session>& session, const unichar_t* userName);

		void WaitBeginning(void) override;

	protected:
		LocateUserWait(const RefPtr<Session>& session, const unichar_t* userName);
		virtual ~LocateUserWait();

		LocateUserWait(const LocateUserWait&);
		const LocateUserWait& operator=(const LocateUserWait&);

		void HandleNotification(ServerError&) override;
		void HandleNotification(UserEvent&) override;

		RefPtr<Session> mSession;
		WideStringClass mUserName;
	};


//-----------------------------------------------------------------------------
// Request to send a private message to a user wait condition
//-----------------------------------------------------------------------------
class SendMsgWait :
		public SingleWait
	{
	public:
		static RefPtr<SendMsgWait> Create(const RefPtr<Session>& session, ChatMessage& message);

		void WaitBeginning(void) override;

	protected:
		SendMsgWait(const RefPtr<Session>& session, ChatMessage& message);
		virtual ~SendMsgWait();

		SendMsgWait(const SendMsgWait&);
		const SendMsgWait& operator=(const SendMsgWait&);

		RefPtr<Session> mSession;
		ChatMessage mMessage;
	};

}

#endif // __WOLCONNECT_H__
