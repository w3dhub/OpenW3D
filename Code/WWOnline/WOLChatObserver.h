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
*     $Archive: /Commando/Code/WWOnline/WOLChatObserver.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 10/14/02 12:38p $
*
******************************************************************************/

#ifndef __WOLCHATOBSERVER_H__
#define __WOLCHATOBSERVER_H__

#include <objbase.h>
#include "RefPtr.h"
#include "WOLUser.h"

#include <wolapi/WOLAPI.h>

namespace WWOnline {

class Session;
class SquadData;

class ChatObserver :
		public WOL::IChatEvent
	{
	public:
		ChatObserver();

		void Init(Session& outer);

		//---------------------------------------------------------------------------
		// IUnknown methods
		//---------------------------------------------------------------------------
		HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv) override;
		RefCountType WOLAPI_CALLTYPE AddRef(void) override;
		RefCountType WOLAPI_CALLTYPE Release(void) override;

		//---------------------------------------------------------------------------
		// IChatEvent Methods
		//---------------------------------------------------------------------------
		WOLAPI_STDMETHOD(OnServerList)(WOL::WOLAPI_RESULT hr, WOL::Server* servers) override;

		WOLAPI_STDMETHOD(OnUpdateList)(WOL::WOLAPI_RESULT hr, WOL::Update* updates) override;

		WOLAPI_STDMETHOD(OnServerError)(WOL::WOLAPI_RESULT hr, LPCSTR ircmsg) override;

		WOLAPI_STDMETHOD(OnConnection)(WOL::WOLAPI_RESULT hr, LPCSTR motd) override;

		WOLAPI_STDMETHOD(OnMessageOfTheDay)(WOL::WOLAPI_RESULT hr, LPCSTR motd) override;

		WOLAPI_STDMETHOD(OnChannelList)(WOL::WOLAPI_RESULT hr, WOL::Channel* channels) override;

		WOLAPI_STDMETHOD(OnChannelCreate)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel) override;

		WOLAPI_STDMETHOD(OnChannelJoin)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user) override;

		WOLAPI_STDMETHOD(OnChannelLeave)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user) override;

		WOLAPI_STDMETHOD(OnChannelTopic)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, LPCSTR topic) override;

		WOLAPI_STDMETHOD(OnPrivateAction)(WOL::WOLAPI_RESULT hr, WOL::User* user, LPCSTR action) override;

		WOLAPI_STDMETHOD(OnPublicAction)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR action) override;

		WOLAPI_STDMETHOD(OnUserList)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* users) override;

		WOLAPI_STDMETHOD(OnPublicMessage)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR message) override;

		WOLAPI_STDMETHOD(OnPrivateMessage)(WOL::WOLAPI_RESULT hr, WOL::User* user, LPCSTR message) override;

		WOLAPI_STDMETHOD(OnSystemMessage)(WOL::WOLAPI_RESULT hr, LPCSTR message) override;

		WOLAPI_STDMETHOD(OnNetStatus)(WOL::WOLAPI_RESULT hr) override;

		WOLAPI_STDMETHOD(OnLogout)(WOL::WOLAPI_RESULT status, WOL::User* user) override;

		WOLAPI_STDMETHOD(OnPrivateGameOptions)(WOL::WOLAPI_RESULT hr, WOL::User* user, LPCSTR options) override;

		WOLAPI_STDMETHOD(OnPublicGameOptions)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR options) override;

		WOLAPI_STDMETHOD(OnGameStart)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* users, int gameid) override;

		WOLAPI_STDMETHOD(OnUserKick)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* kicked, WOL::User* kicker) override;

		WOLAPI_STDMETHOD(OnUserIP)(WOL::WOLAPI_RESULT hr, WOL::User* user) override;

		WOLAPI_STDMETHOD(OnFind)(WOL::WOLAPI_RESULT hr, WOL::Channel* chan) override;

		WOLAPI_STDMETHOD(OnPageSend)(WOL::WOLAPI_RESULT hr) override;

		WOLAPI_STDMETHOD(OnPaged)(WOL::WOLAPI_RESULT hr, WOL::User* user, LPCSTR message) override;

		WOLAPI_STDMETHOD(OnServerBannedYou)(WOL::WOLAPI_RESULT hr, WOL::time_t bannedTill) override;

		WOLAPI_STDMETHOD(OnUserFlags)(WOL::WOLAPI_RESULT hr, LPCSTR name, unsigned int flags, unsigned int mask) override;

		WOLAPI_STDMETHOD(OnChannelBan)(WOL::WOLAPI_RESULT hr, LPCSTR name, int banned) override;

		WOLAPI_STDMETHOD(OnSquadInfo)(WOL::WOLAPI_RESULT hr, unsigned int id, WOL::Squad* squad) override;

		WOLAPI_STDMETHOD(OnUserLocale)(WOL::WOLAPI_RESULT hr, WOL::User* users) override;

		WOLAPI_STDMETHOD(OnUserTeam)(WOL::WOLAPI_RESULT hr, WOL::User* users) override;

		WOLAPI_STDMETHOD(OnSetLocale)(WOL::WOLAPI_RESULT hr, WOL::Locale newlocale) override;

		WOLAPI_STDMETHOD(OnSetTeam)(WOL::WOLAPI_RESULT hr, int newteam) override;

		WOLAPI_STDMETHOD(OnBuddyList)(WOL::WOLAPI_RESULT hr, WOL::User* buddyList) override;

		WOLAPI_STDMETHOD(OnBuddyAdd)(WOL::WOLAPI_RESULT hr, WOL::User* buddyAdded) override;

		WOLAPI_STDMETHOD(OnBuddyDelete)(WOL::WOLAPI_RESULT hr, WOL::User* buddyDeleted) override;

		WOLAPI_STDMETHOD(OnPublicUnicodeMessage)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* message) override;

		WOLAPI_STDMETHOD(OnPrivateUnicodeMessage)(WOL::WOLAPI_RESULT hr, WOL::User* user, const unsigned short* message) override;

		WOLAPI_STDMETHOD(OnPrivateUnicodeAction)(WOL::WOLAPI_RESULT hr, WOL::User* user, const unsigned short* action) override;

		WOLAPI_STDMETHOD(OnPublicUnicodeAction)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* action) override;

		WOLAPI_STDMETHOD(OnPagedUnicode)(WOL::WOLAPI_RESULT hr, WOL::User* user, const unsigned short* message) override;

		WOLAPI_STDMETHOD(OnServerTime)(WOL::WOLAPI_RESULT hr, WOL::time_t stime) override;

		WOLAPI_STDMETHOD(OnInsiderStatus)(WOL::WOLAPI_RESULT hr, WOL::User* users) override;

		WOLAPI_STDMETHOD(OnSetLocalIP)(WOL::WOLAPI_RESULT hr, LPCSTR message) override;

		WOLAPI_STDMETHOD(OnChannelListBegin)(WOL::WOLAPI_RESULT hr) override;

		WOLAPI_STDMETHOD(OnChannelListEntry)(WOL::WOLAPI_RESULT hr, WOL::Channel* channel) override;

		WOLAPI_STDMETHOD(OnChannelListEnd)(WOL::WOLAPI_RESULT hr) override;

	protected:
		virtual ~ChatObserver();

		// prevent copy and assignment
		ChatObserver(ChatObserver const &);
		ChatObserver const & operator =(ChatObserver const &);

		void AssignSquadToUsers(const UserList& users, const RefPtr<SquadData>& squad);
		void ProcessSquadRequest(const RefPtr<SquadData>& squad);
		void Kick_Spammer(WOL::User *wol_user);


	private:
		std::atomic<unsigned int> mRefCount;
		Session* mOuter;
	};

}

#endif // __WOLCHATOBSERVER_H__
