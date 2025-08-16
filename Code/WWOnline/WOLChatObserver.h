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
#include <wolapi/wolapi.h>
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
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
		virtual ULONG STDMETHODCALLTYPE AddRef(void);
		virtual ULONG STDMETHODCALLTYPE Release(void);

		//---------------------------------------------------------------------------
		// IChatEvent Methods
		//---------------------------------------------------------------------------
		STDMETHOD(OnServerList)(HRESULT hr, WOL::Server* servers);
        
		STDMETHOD(OnUpdateList)(HRESULT hr, WOL::Update* updates);
    
		STDMETHOD(OnServerError)(HRESULT hr, LPCSTR ircmsg);
    
		STDMETHOD(OnConnection)(HRESULT hr, LPCSTR motd);
    
		STDMETHOD(OnMessageOfTheDay)(HRESULT hr, LPCSTR motd);
    
		STDMETHOD(OnChannelList)(HRESULT hr, WOL::Channel* channels);
    
		STDMETHOD(OnChannelCreate)(HRESULT hr, WOL::Channel* channel);
    
		STDMETHOD(OnChannelJoin)(HRESULT hr, WOL::Channel* channel, WOL::User* user);
    
		STDMETHOD(OnChannelLeave)(HRESULT hr, WOL::Channel* channel, WOL::User* user);
    
		STDMETHOD(OnChannelTopic)(HRESULT hr, WOL::Channel* channel, LPCSTR topic);
    
		STDMETHOD(OnPrivateAction)(HRESULT hr, WOL::User* user, LPCSTR action);
    
		STDMETHOD(OnPublicAction)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR action);
    
		STDMETHOD(OnUserList)(HRESULT hr, WOL::Channel* channel, WOL::User* users);
    
		STDMETHOD(OnPublicMessage)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR message);
    
		STDMETHOD(OnPrivateMessage)(HRESULT hr, WOL::User* user, LPCSTR message);
    
		STDMETHOD(OnSystemMessage)(HRESULT hr, LPCSTR message);
    
		STDMETHOD(OnNetStatus)(HRESULT hr);
    
		STDMETHOD(OnLogout)(HRESULT status, WOL::User* user);
    
		STDMETHOD(OnPrivateGameOptions)(HRESULT hr, WOL::User* user, LPCSTR options);
    
		STDMETHOD(OnPublicGameOptions)(HRESULT hr, WOL::Channel* channel, WOL::User* user, LPCSTR options);
    
		STDMETHOD(OnGameStart)(HRESULT hr, WOL::Channel* channel, WOL::User* users, int gameid);
    
		STDMETHOD(OnUserKick)(HRESULT hr, WOL::Channel* channel, WOL::User* kicked, WOL::User* kicker);
    
		STDMETHOD(OnUserIP)(HRESULT hr, WOL::User* user);
    
		STDMETHOD(OnFind)(HRESULT hr, WOL::Channel* chan);
    
		STDMETHOD(OnPageSend)(HRESULT hr);
    
		STDMETHOD(OnPaged)(HRESULT hr, WOL::User* user, LPCSTR message);
    
		STDMETHOD(OnServerBannedYou)(HRESULT hr, WOL::time_t bannedTill);
    
		STDMETHOD(OnUserFlags)(HRESULT hr, LPCSTR name, unsigned int flags, unsigned int mask);
    
		STDMETHOD(OnChannelBan)(HRESULT hr, LPCSTR name, int banned);
    
		STDMETHOD(OnSquadInfo)(HRESULT hr, unsigned long id, WOL::Squad* squad);
    
		STDMETHOD(OnUserLocale)(HRESULT hr, WOL::User* users);
    
		STDMETHOD(OnUserTeam)(HRESULT hr, WOL::User* users);
    
		STDMETHOD(OnSetLocale)(HRESULT hr, WOL::Locale newlocale);
    
		STDMETHOD(OnSetTeam)(HRESULT hr, int newteam);

		STDMETHOD(OnBuddyList)(HRESULT hr, WOL::User* buddyList);
        
		STDMETHOD(OnBuddyAdd)(HRESULT hr, WOL::User* buddyAdded);
        
		STDMETHOD(OnBuddyDelete)(HRESULT hr, WOL::User* buddyDeleted);

		STDMETHOD(OnPublicUnicodeMessage)(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* message);
        
		STDMETHOD(OnPrivateUnicodeMessage)(HRESULT hr, WOL::User* user, const unsigned short* message);
        
		STDMETHOD(OnPrivateUnicodeAction)(HRESULT hr, WOL::User* user, const unsigned short* action);
        
		STDMETHOD(OnPublicUnicodeAction)(HRESULT hr, WOL::Channel* channel, WOL::User* user, const unsigned short* action);
        
		STDMETHOD(OnPagedUnicode)(HRESULT hr, WOL::User* user, const unsigned short* message);
        
		STDMETHOD(OnServerTime)(HRESULT hr, WOL::time_t stime);
        
		STDMETHOD(OnInsiderStatus)(HRESULT hr, WOL::User* users);
        
		STDMETHOD(OnSetLocalIP)(HRESULT hr, LPCSTR message);

		STDMETHOD(OnChannelListBegin)(HRESULT hr);
        
		STDMETHOD(OnChannelListEntry)(HRESULT hr, WOL::Channel* channel);
        
		STDMETHOD(OnChannelListEnd)(HRESULT hr);

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