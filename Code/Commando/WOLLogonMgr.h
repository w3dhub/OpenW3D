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
*     $Archive: /Commando/Code/Commando/WOLLogonMgr.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 25 $
*     $Modtime: 1/14/02 2:04p $
*
******************************************************************************/

#ifndef __WOLLOGONMGR_H__
#define __WOLLOGONMGR_H__

#include "DlgWOLLogon.h"
#include <wwlib/refcount.h>
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>
#include <WWOnline/WOLSession.h>

class DlgWOLWaitEvent;
class DlgMsgBoxEvent;

typedef enum
	{
	WOLLOGON_FAILED = 0,
	WOLLOGON_SUCCESS,
	WOLLOGON_CANCEL,
	WOLLOGON_PATCHREQUIRED
	} WOLLogonAction;

class WOLLogonMgr :
		public RefCountClass,
		public Notifier<WOLLogonAction>,
		public Observer<DlgWOLLogonEvent>,
		public Observer<DlgWOLWaitEvent>,
		public Observer<DlgMsgBoxEvent>,
		public Observer<WWOnline::ServerError>,
		public Observer<WWOnline::ConnectionStatus>,
		public Observer<WWOnline::MessageOfTheDayEvent>
	{
	public:
		// Log onto Westwood Online
		static void Logon(Observer<WOLLogonAction>* observer);

		// Log the current user off of Westwood Online.
		static void Logoff(void);

		// Get the name of the logged in user.
		static bool GetLoginName(WideStringClass& name);

		// Get the name of the server logged onto.
		static bool GetServerName(WideStringClass& name);

		static RefPtr<WWOnline::IRCServerData> GetDefaultServer(void);

		static void ConfigureSession(void);

		static void Set_Quiet_Mode(bool mode) {mQuietMode = mode;}

	protected:
		typedef enum {DISCONNECTED = 0, DETECTING_BANDWIDTH, FETCHING_SERVERLIST, WAITING_PINGS, WAITING_BANDWIDTH_DIALOG_OKAY, CONNECTING, CONNECTED} LogonState;

		WOLLogonMgr();
		~WOLLogonMgr();

		WOLLogonMgr(const WOLLogonMgr&);
		const WOLLogonMgr& operator=(const WOLLogonMgr&);

		RefPtr<WWOnline::IRCServerData> GetPreferredServer(const unichar_t* loginname);

		// Check if the user is connected to a server.
		bool IsConnectedToServer(const unichar_t* loginname, RefPtr<WWOnline::IRCServerData>& server);
		bool IsUserLoggedIn(const unichar_t* loginname);
		bool IsAutoLogin(const unichar_t* loginname);

		bool HasServerList(void);
		bool HasValidPings(void);

		void InitiateLogon(bool forced);

		void RememberLogin(void);

		DECLARE_NOTIFIER(WOLLogonAction)

		void HandleNotification(DlgWOLLogonEvent&) override;
		void HandleNotification(DlgWOLWaitEvent&) override;
		void HandleNotification(DlgMsgBoxEvent&) override;
		void HandleNotification(WWOnline::ServerError&) override;
		void HandleNotification(WWOnline::ConnectionStatus&) override;
		void HandleNotification(WWOnline::MessageOfTheDayEvent&) override;

	private:
		static bool mQuietMode;

		RefPtr<WWOnline::Session> mWOLSession;
		LogonState mState;

		WideStringClass mLoginName;
		WideStringClass mPassword;
		bool mPasswordEncrypted;
		bool mRememberLogin;
	};

#endif // __WOLLOGONMGR_H__
