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

namespace WOL 
{
#include <wolapi/WOLAPI.h>
}

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
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv) override;
		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		//---------------------------------------------------------------------------
		// IChatEvent Methods
		//---------------------------------------------------------------------------
		STDMETHOD(OnServerList)(HRESULT hr, WOL::Server* servers) override;
        
		STDMETHOD(OnUpdateList)(HRESULT hr, WOL::Update* updates) override;
    
		STDMETHOD(OnServerError)(HRESULT hr, LPCSTR ircmsg) override;
    
		STDMETHOD(OnConnection)(HRESULT hr, LPCSTR motd) override;
    
		STDMETHOD(OnMessageOfTheDay)(HRESULT hr, LPCSTR motd) override;
    
		STDMETHOD(OnChannelList)(HRESULT hr, WOL::Channel* channels) override;
    
		STDMETHOD(OnChannelCreate)(HRESULT hr, WOL::Channel* channel) override;
    
		STDMETHOD(OnChannelJoin)(HRESULT hr, WOL::Channel* channel, WOL::User* user) override;
    
		STDMETHOD(OnChannelLeave)(HRESULT hr, WOL::Channel* channel, WOL::User* user) override;
    
		STDMETHOD(OnChannelTopic)(HRESULT hr, WOL::Channel* channel, LPCSTR topic) override;
    
		STDMETHOD(OnPrivateAction)(HRESULT hr, WOL::User* user, LPCSTR action) override;
    
		STDMETHOD(OnPublicAction)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR action) override;
    
		STDMETHOD(OnUserList)(HRESULT hr, WOL::Channel* channel, WOL::User* users) override;
    
		STDMETHOD(OnPublicMessage)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR message) override;
    
		STDMETHOD(OnPrivateMessage)(HRESULT hr, WOL::User* user, LPCSTR message) override;
    
		STDMETHOD(OnSystemMessage)(HRESULT hr, LPCSTR message) override;
    
		STDMETHOD(OnNetStatus)(HRESULT hr) override;
    
		STDMETHOD(OnLogout)(HRESULT status, WOL::User* user) override;
    
		STDMETHOD(OnPrivateGameOptions)(HRESULT hr, WOL::User* user, LPCSTR options) override;
    
		STDMETHOD(OnPublicGameOptions)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR options) override;
    
		STDMETHOD(OnGameStart)(HRESULT hr, WOL::Channel* channel, WOL::User* users, int gameid) override;
    
		STDMETHOD(OnUserKick)(HRESULT hr, WOL::Channel* channel, WOL::User* kicked, WOL::User* kicker) override;
    
		STDMETHOD(OnUserIP)(HRESULT hr, WOL::User* user) override;
    
		STDMETHOD(OnFind)(HRESULT hr, WOL::Channel* chan) override;
    
		STDMETHOD(OnPageSend)(HRESULT hr) override;
    
		STDMETHOD(OnPaged)(HRESULT hr, WOL::User* user, LPCSTR message) override;
    
		STDMETHOD(OnServerBannedYou)(HRESULT hr, WOL::time_t bannedTill) override;
    
		STDMETHOD(OnUserFlags)(HRESULT hr, LPCSTR name, unsigned int flags, unsigned int mask) override;
    
		STDMETHOD(OnChannelBan)(HRESULT hr, LPCSTR name, int banned) override;
    
		STDMETHOD(OnSquadInfo)(HRESULT hr, unsigned int id, WOL::Squad* squad) override;
    
		STDMETHOD(OnUserLocale)(HRESULT hr, WOL::User* users) override;
    
		STDMETHOD(OnUserTeam)(HRESULT hr, WOL::User* users) override;
    
		STDMETHOD(OnSetLocale)(HRESULT hr, WOL::Locale newlocale) override;
    
		STDMETHOD(OnSetTeam)(HRESULT hr, int newteam) override;

		STDMETHOD(OnBuddyList)(HRESULT hr, WOL::User* buddyList) override;
        
		STDMETHOD(OnBuddyAdd)(HRESULT hr, WOL::User* buddyAdded) override;
        
		STDMETHOD(OnBuddyDelete)(HRESULT hr, WOL::User* buddyDeleted) override;

		STDMETHOD(OnPublicUnicodeMessage)(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* message) override;
        
		STDMETHOD(OnPrivateUnicodeMessage)(HRESULT hr, WOL::User* user, const unsigned short* message) override;
        
		STDMETHOD(OnPrivateUnicodeAction)(HRESULT hr, WOL::User* user, const unsigned short* action) override;
        
		STDMETHOD(OnPublicUnicodeAction)(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* action) override;
        
		STDMETHOD(OnPagedUnicode)(HRESULT hr, WOL::User* user, const unsigned short* message) override;
        
		STDMETHOD(OnServerTime)(HRESULT hr, WOL::time_t stime) override;
        
		STDMETHOD(OnInsiderStatus)(HRESULT hr, WOL::User* users) override;
        
		STDMETHOD(OnSetLocalIP)(HRESULT hr, LPCSTR message) override;

		STDMETHOD(OnChannelListBegin)(HRESULT hr) override;
        
		STDMETHOD(OnChannelListEntry)(HRESULT hr, WOL::Channel* channel) override;
        
		STDMETHOD(OnChannelListEnd)(HRESULT hr) override;

	protected:
		virtual ~ChatObserver();

		// prevent copy and assignment
		ChatObserver(ChatObserver const &);
		ChatObserver const & operator =(ChatObserver const &);

		void AssignSquadToUsers(const UserList& users, const RefPtr<SquadData>& squad);
		void ProcessSquadRequest(const RefPtr<SquadData>& squad);
		void Kick_Spammer(WOL::User *wol_user);


	private:
		ULONG mRefCount;
		Session* mOuter;
	};

}

#endif // __WOLCHATOBSERVER_H__