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
*     $Archive: /Commando/Code/Commando/WOLLogonMgr.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Tom_s $
*
* VERSION INFO
*     $Revision: 59 $
*     $Modtime: 2/11/02 4:07p $
*
******************************************************************************/

#include "WOLLogonMgr.h"
#include "MPSettingsMgr.h"
#include "DlgMessageBox.h"
#include "DlgWOLWait.h"
#include "DlgMPWOLMain.h"
#include "DlgMPWOLAutoLoginPrompt.h"
#include "DlgMPWOLmotd.h"
#include "WOLLoginProfile.h"
#include "WOLLocaleMgr.h"
#include "NetInterface.h"
#include "nat.h"
#include "FirewallWait.h"
#include "BandwidthCheck.h"
#include <WWOnline\WOLLoginInfo.h>
#include <WWOnline\WOLServer.h>
#include <WWOnline\WOLLoginInfo.h>
#include <WWOnline\WaitCondition.h>
#include <WWOnline\PingProfile.h>
#include "string_ids.h"
#include <WWTranslateDB\TranslateDB.h>
#include <WWDebug\WWDebug.h>
#include "UserOptions.h"
#include "autostart.h"
#include "consolemode.h"
#include "specialbuilds.h"
#include "serversettings.h"

using namespace WWOnline;

bool WOLLogonMgr::mQuietMode = false;

/******************************************************************************
*
* NAME
*     WOLLogonMgr::Logon
*
* DESCRIPTION
*     Log on to Westwood Online
*
* INPUTS
*     ServerType - Type of server to log onto.
*     Observer   - Logon observer
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::Logon(Observer<WOLLogonAction>* observer)
	{
	WWDEBUG_SAY(("WOLLogonMgr: Logon\n"));

	WOLLogonMgr* logon = new WOLLogonMgr;

	if (logon)
		{
		if (observer)
			{
			logon->AddObserver(*observer);
			}

		logon->InitiateLogon(false);
		logon->Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::Logoff
*
* DESCRIPTION
*     Log the current user off of Westwood Online.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::Logoff(void)
	{
	WWDEBUG_SAY(("WOLLogonMgr: Logoff\n"));

	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (wolSession.IsValid())
		{
		ConnectionStatus status = wolSession->GetConnectionStatus();

		// If we are connected then disconnected now
		if (ConnectionConnected == status || ConnectionConnecting == status)
			{
			RefPtr<WaitCondition> wait = wolSession->Logout();
			DlgWOLWait::DoDialog(IDS_WOL_LOGOFF, wait);
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::GetLoginName
*
* DESCRIPTION
*     Get name of the logged in user.
*
* INPUTS
*     Name - On return; name of user.
*
* RESULT
*     True if logged in otherwise false.
*
******************************************************************************/

bool WOLLogonMgr::GetLoginName(WideStringClass& name)
	{
	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (wolSession.IsValid())
		{
		const RefPtr<LoginInfo>& login = wolSession->GetCurrentLogin();

		if (login.IsValid())
			{
			name = login->GetNickname();
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::GetServerName
*
* DESCRIPTION
*     Get name of the server logged onto.
*
* INPUTS
*     Name - On return; name of the server connected to.
*
* RESULT
*     True if connected to server.
*
******************************************************************************/

bool WOLLogonMgr::GetServerName(WideStringClass& name)
	{
	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (wolSession.IsValid())
		{
		const RefPtr<IRCServerData>& server = wolSession->GetCurrentServer();

		if (server.IsValid())
			{
			name = server->GetName();
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::GetDefaultServer
*
* DESCRIPTION
*     Retrieve the default server for the client.
*
* INPUTS
*     ServerType - Type of server to get default for.
*
* RESULT
*     Server -
*
******************************************************************************/

RefPtr<IRCServerData> WOLLogonMgr::GetDefaultServer(void)
	{
	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (!wolSession.IsValid())
		{
		return NULL;
		}

	// Find the server closets to us.
	RefPtr<IRCServerData> server;

	int bestTime = INT_MAX;

	const IRCServerList& serverList = wolSession->GetIRCServerList();
	const PingServerList& pingers = wolSession->GetPingServerList();

	const unsigned int numServers = serverList.size();

	for (unsigned int index = 0; index < numServers; ++index)
		{
		const RefPtr<IRCServerData>& thisServer = serverList[index];

		// check that chat server matches game client lang code
		if (thisServer->MatchesLanguageCode())
			{
			float serverLong = thisServer->GetLongitude();
			float serverLat = thisServer->GetLattitude();

			// Find the ping server with the best time.
			for (unsigned int pingindex = 0; pingindex < pingers.size(); pingindex++)
				{
				const RefPtr<PingServerData>& thisPing = pingers[pingindex];
				float pingLong = thisPing->GetLongitude();
				float pingLat  = thisPing->GetLattitude();

				// make sure the ping server is associated with the chat server
				if (serverLong == pingLong && serverLat == pingLat)
					{
					int pingTime = thisPing->GetPingTime();

					// sanity check
					if (pingTime == -1)
						{
						pingTime = INT_MAX;
						}

					// here's our main test!  Find the best ping time
					if (pingTime <= bestTime)
						{
						bestTime = pingTime;
						server = thisServer;
						}
					}
				}
			}
		else
			{
			// If we haven't found a match then use the first server that has
			// a language specification.
			if (!server.IsValid() && thisServer->HasLanguageCode())
				{
				server = thisServer;
				}
			}
		}

	return server;
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::ConfigureSession
*
* DESCRIPTION
*     Configure the WWOnline session with the users options.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::ConfigureSession(void)
	{
	RefPtr<Session> wolSession = Session::GetInstance(false);

	if (wolSession.IsValid())
		{
		if (ConnectionConnected == wolSession->GetConnectionStatus())
			{
			bool allowFind = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_ALLOW_FIND);
			bool allowPages = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_ALLOW_PAGES);
			wolSession->AllowFindPage(allowFind, allowPages);

			bool filterBadLang = MPSettingsMgrClass::Get_Option_Flag(MPSettingsMgrClass::OPTION_LANGUAGE_FILTER);
			wolSession->SetBadLanguageFilter(filterBadLang);
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::WOLLogonMgr
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

WOLLogonMgr::WOLLogonMgr() :
		mState(DISCONNECTED),
		mRememberLogin(true),
		mPasswordEncrypted(false)
	{
	WWDEBUG_SAY(("WOLLogonMgr: Instantiated\n"));

	//---------------------------------------------------------------------------
	// Get the default login to use.
	//---------------------------------------------------------------------------
	mLoginName = MPSettingsMgrClass::Get_Auto_Login();

	// If there isn't a auto login then use the last login.
	if (mLoginName.Is_Empty())
		{
		mLoginName = MPSettingsMgrClass::Get_Last_Login();
		}

#ifndef FREEDEDICATEDSERVER
	if (AutoRestart.Is_Active())
#endif //FREEDEDICATEDSERVER
		{
		mPassword = MPSettingsMgrClass::Get_Auto_Password();
		}

	// Get WOL session
	mWOLSession = Session::GetInstance(false);
	WWASSERT_PRINT(mWOLSession.IsValid(), "WOLSession not instantiated!");

	Observer<ServerError>::NotifyMe(*mWOLSession);
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::~WOLLogonMgr
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

WOLLogonMgr::~WOLLogonMgr()
	{
	WWDEBUG_SAY(("WOLLogonMgr: Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::IsConnectedToServer
*
* DESCRIPTION
*     Check if a user is connected to a server.
*
* INPUTS
*     Login  - User to check.
*     Server - Server to check.
*
* RESULT
*     True if user is connected to the specified server.
*
******************************************************************************/

bool WOLLogonMgr::IsConnectedToServer(const wchar_t* login, RefPtr<IRCServerData>& server)
	{
	if (!mWOLSession.IsValid())
		{
		return false;
		}

	// Check connection
	ConnectionStatus status = mWOLSession->GetConnectionStatus();

	// Check login
	bool loginOk = IsUserLoggedIn(login);

	// Check server
	bool serverOk = false;

	if (server.IsValid())
		{
		RefPtr<IRCServerData> curServer = mWOLSession->GetCurrentServer();
		serverOk = (curServer.IsValid() && (server == curServer));
		}

	return ((status == ConnectionConnected) && loginOk && serverOk);
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::IsUserLoggedIn
*
* DESCRIPTION
*     Check if a user is logged in.
*
* INPUTS
*     Login - User to check.
*
* RESULT
*     True if user is logged in.
*
******************************************************************************/

bool WOLLogonMgr::IsUserLoggedIn(const wchar_t* login)
	{
	if (!mWOLSession.IsValid())
		{
		return false;
		}

	// Check connection
	ConnectionStatus status = mWOLSession->GetConnectionStatus();

	// Check login
	bool loginOk = false;

	if (login)
		{
		RefPtr<LoginInfo> curLogin = mWOLSession->GetCurrentLogin();

		if (curLogin.IsValid())
			{
			loginOk = (curLogin->GetNickname().Compare_No_Case(login) == 0);
			}
		}

	return ((status == ConnectionConnected) && loginOk);
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::IsAutoLogin
*
* DESCRIPTION
*     Check if login should automatically logon.
*
* INPUTS
*     Login - Name of login to check for autologon
*
* RESULT
*     True if this login should auto logon
*
******************************************************************************/

bool WOLLogonMgr::IsAutoLogin(const wchar_t* login)
	{
	if (mWOLSession->IsAutoLoginAllowed())
		{
		WideStringClass autoName(64, true);
		autoName = MPSettingsMgrClass::Get_Auto_Login();
		return (autoName.Compare_No_Case(login) == 0);
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::GetPreferredServer
*
* DESCRIPTION
*     Get the users preferred server to connect to.
*
* INPUTS
*     Type - Type of server to obtain.
*
* RESULT
*     Server - Preferred server if provided.
*
******************************************************************************/

RefPtr<IRCServerData> WOLLogonMgr::GetPreferredServer(const wchar_t* login)
	{
	const char* preferred = "";

#ifdef FREEDEDICATEDSERVER
	preferred = ServerSettingsClass::Get_Preferred_Server(mWOLSession->GetIRCServerList());
#else  //FREEDEDICATEDSERVER
	// Get the user's preferred server if available.
	LoginProfile* profile = LoginProfile::Get(login);

	if (profile)
		{
		preferred = profile->GetPreferredServer();
		profile->Release_Ref();
		}
#endif //FREEDEDICATEDSERVER

	// Find the preferred server
	RefPtr<IRCServerData> server;
	const IRCServerList& serverList = mWOLSession->GetIRCServerList();

	for (unsigned int index = 0; index < serverList.size(); index++)
		{
		const RefPtr<IRCServerData>& thisServer = serverList[index];

		const char* name = thisServer->GetName();
		WWASSERT(name != NULL && "NULL server name");

		if (preferred && (stricmp(preferred, name) == 0))
			{
			server = thisServer;
			break;
			}
		}

	return server;
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::RememberLogin
*
* DESCRIPTION
*     Store the last successful login
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::RememberLogin(void)
	{
	// Get the successful login.
	const RefPtr<LoginInfo>& currentLogin = mWOLSession->GetCurrentLogin();

	if (currentLogin.IsValid())
		{
		// Set login name as the network nickname.
		const WideStringClass& nickname = currentLogin->GetNickname();
		WWASSERT(nickname.Get_Length() > 0);
		cNetInterface::Set_Nickname((WideStringClass&)nickname);

		// Set the name of the last successful login
		StringClass lastname(64, true);
		nickname.Convert_To(lastname);
		MPSettingsMgrClass::Set_Last_Login((const char*)lastname);

		// Remember the login preferences
		LoginProfile* profile = LoginProfile::Get(nickname, true);
		LoginProfile::SetCurrent(profile);

		// Remember this login
		currentLogin->Remember(mRememberLogin);

		// Store login information to disk
		if (mRememberLogin)
			{
			// Save preferences for this login
			if (profile)
				{
				profile->SaveSettings();
				}

			// Should we display the auto-login prompt
			bool autoLoginAllowed = (!mQuietMode && mWOLSession->IsAutoLoginAllowed());

			if (autoLoginAllowed && MPSettingsMgrClass::Is_Auto_Login_Prompt_Enabled())
				{
				MPWolAutoLoginPromptDialogClass* dialog = new MPWolAutoLoginPromptDialogClass;
				WWASSERT(dialog);

				if (dialog)
					{
					dialog->Start_Dialog();
					REF_PTR_RELEASE(dialog);
					}
				}
			}

		if (profile)
			{
			profile->Release_Ref();
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HasServerList
*
* DESCRIPTION
*     Check if there is a server list.
*
* INPUTS
*     NONE
*
* RESULT
*     True if valid server list is available
*
******************************************************************************/

bool WOLLogonMgr::HasServerList(void)
	{
	const IRCServerList& servers = mWOLSession->GetIRCServerList();
	return (servers.size() > 0);
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HasValidPings
*
* DESCRIPTION
*     Check if there are valid location pings available.
*
* INPUTS
*     NONE
*
* RESULT
*     True if valid location pings are available.
*
******************************************************************************/

bool WOLLogonMgr::HasValidPings(void)
	{
	const PingServerList& pingers = mWOLSession->GetPingServerList();

	unsigned int count = pingers.size();

	for (unsigned int index = 0; index < count; ++index)
		{
		int pingTime = pingers[index]->GetPingTime();

		// If a ping time is -1 then we dont have valid pings
		if (pingTime == -1)
			{
			return false;
			}
		}

	return (count != 0);
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::InitiateLogon
*
* DESCRIPTION
*     Start logon sequence.
*
* INPUTS
*     Forced - Attempt to logon with current persona.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::InitiateLogon(bool forced)
	{
	WWDEBUG_SAY(("WOLLogonMgr: Initiating logon sequence\n"));

	// Make sure WWOnline session is active.
	if (!mWOLSession.IsValid())
		{
		WWDEBUG_SAY(("WOLLogonMgr: Error - WWOnline session not initialized!\n"));
		DlgMsgBox::DoDialog(IDS_WOL_LOGONFAILED, IDS_WOL_NOTINITIALIZED);
		return;
		}

	// Do bandwidth detection if needed.
	if (BandwidthCheckerClass::Got_Bandwidth() == false && cUserOptions::Get_Bandwidth_Type() == BANDWIDTH_AUTO)
		{
		WWDEBUG_SAY(("WOLLogonMgr: Detecting bandwidth...\n"));
		mState = DETECTING_BANDWIDTH;

		// Detect the bandwidth.
		RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();
		bool detecting = DlgWOLWait::DoDialog(TRANSLATE(IDS_MENU_DETECTING_BANDWIDTH), bandwidth_wait, this, 0, mQuietMode ? 0xffffffff : 0);

		if (detecting)
			{
			Add_Ref();
			}

		return;
		}

	//---------------------------------------------------------------------------
	// Make sure we have a server list. If we don't then request it before allowing
	// logon to continue.
	//
	// Note: The IRCServerList notification will reinitiate the logon when the
	//       server list is obtained.
	//---------------------------------------------------------------------------
	if (HasServerList() == false)
		{
		WWDEBUG_SAY(("WOLLogonMgr: Fetching server list...\n"));
		mState = FETCHING_SERVERLIST;

		// Request a server list. If a patch is available the serverlist request will
		// fail with a ServerError notification of CHAT_E_MUSTPATH. See the ServerError
		// notification handler.
		RefPtr<WaitCondition> fetch = mWOLSession->GetNewServerList();
		bool fetching = DlgWOLWait::DoDialog(IDS_WOL_LOGON, fetch, this, 0, mQuietMode ? 0xffffffff : 0);

		// If fetching server list then keep alive while we are waiting.
		if (fetching)
			{
			Add_Ref();
			}

		return;
		}

	// Get a server to logon to.
	RefPtr<IRCServerData> server = GetPreferredServer(mLoginName);

	if (server.IsValid() == false)
		{
		// Before we can pick a default server we must first have valid server pings.
		if (HasValidPings() == false)
			{
			WWDEBUG_SAY(("WOLLogonMgr: Waiting on server pings...\n"));
			mState = WAITING_PINGS;

			// Request a server list.
			RefPtr<WaitCondition> pings = PingProfileWait::Create();
			bool pinging = DlgWOLWait::DoDialog(IDS_WOL_LOGON, pings, this, 0, mQuietMode ? 0xffffffff : 0);

			// If waiting for pings then keep alive
			if (pinging)
				{
				Add_Ref();
				}

			return;
			}

		// Choose a default server
		server = GetDefaultServer();
		}

	ConsoleBox.Print("Logging onto %s\n", server->GetName());

	// Check if the user is already logged onto the requested server.
	if (IsConnectedToServer(mLoginName, server))
		{
		WWDEBUG_SAY(("WOLLogonMgr: User already connected.\n"));
		mState = CONNECTED;

		Add_Ref();
		WOLLogonAction action = WOLLOGON_SUCCESS;
		NotifyObservers(action);
		Release_Ref();

		return;
		}

	//---------------------------------------------------------------------------
	// Show the logon dialog if:
	//   1) Not forcing a logon.
	//   2) The user is not logged in to another server and auto logon is off.
	//   3) The login information is invalid.
	//---------------------------------------------------------------------------
	bool loggedIn  = IsUserLoggedIn(mLoginName);
	bool autoLogin = IsAutoLogin(mLoginName);

	if (!forced && ((!loggedIn && !autoLogin) || mLoginName.Is_Empty()))
		{
#ifdef FREEDEDICATEDSERVER
		Add_Ref();
		WOLLogonAction action = WOLLOGON_FAILED;
		NotifyObservers(action);
		Release_Ref();
#else //FREEDEDICATEDSERVER
		WWDEBUG_SAY(("WOLLogonMgr: Requesting logon information.\n"));

		// Keep alive while we are waiting on the logon dialog.
		Add_Ref();

		// Prompt the user to enter login information.
		DlgWOLLogon::DoDialog(mLoginName, this);
#endif //FREEDEDICATEDSERVER
		return;
		}

	//---------------------------------------------------------------------------
	// Logon to WWOnline server
	//---------------------------------------------------------------------------
	WWDEBUG_SAY(("WOLLogonMgr: Connecting user '%S' to server '%s'\n", (const WCHAR*)mLoginName, server->GetName()));

	RefPtr<LoginInfo> login = LoginInfo::Find(mLoginName);

#ifndef FREEDEDICATEDSERVER
	if (login.IsValid() == false)
#endif //FREEDEDICATEDSERVER
		{
		if (AutoRestart.Is_Active() && strlen(MPSettingsMgrClass::Get_Auto_Password()))
			{
			mPasswordEncrypted = false;
			}

		login = LoginInfo::Create(mLoginName, mPassword, mPasswordEncrypted);
		}

	mState = CONNECTING;

	RefPtr<SerialWait> connectWait = SerialWait::Create();

	if (connectWait.IsValid())
		{
		// Listen for various connection events
		Observer<ConnectionStatus>::NotifyMe(*mWOLSession);
		Observer<MessageOfTheDayEvent>::NotifyMe(*mWOLSession);

		// Gather ping times
		RefPtr<WaitCondition> pingsWait = PingProfileWait::Create();
		connectWait->Add(pingsWait);

		// Detect firewall settings.
		RefPtr<WaitCondition> firewallWait = FirewallDetectWait::Create();
		connectWait->Add(firewallWait);

		// Request loging to server
		RefPtr<WaitCondition> loginWait = mWOLSession->LoginServer(server, login);
		connectWait->Add(loginWait);

		RefPtr<WaitCondition> connect = connectWait;
		bool connecting = DlgWOLWait::DoDialog(IDS_WOL_LOGON, connect, this, 0, mQuietMode ? 0xffffffff : 0);

		// If connecting then keep alive until complete...
		if (connecting)
			{
			// Keep alive while we are connecting.
			Add_Ref();
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(DlgWOLWaitEvent)
*
* DESCRIPTION
*     Handle completion of wait event dialog.
*
* INPUTS
*     WaitEvent - Completed wait event.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(DlgWOLWaitEvent& waitEvent)
	{
	WaitCondition::WaitResult result = waitEvent.Result();

	switch (mState)
		{
		// If we were waiting to detect bandwidth
		case DETECTING_BANDWIDTH:
			mState = DISCONNECTED;

			// Failure to detect bandwidth is not neccessarily fatal.
			// Or maybe it is. ST - 11/6/2001 11:33AM
			if ((result != WaitCondition::Waiting && result != WaitCondition::UserCancel)
					&& BandwidthCheckerClass::Got_Bandwidth())
				{
				WWDEBUG_SAY(("WOLLogonMgr: Finished bandwidth check\n"));
				cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);
				InitiateLogon(false);
				}
			else
				{
				//Should never get here unless there is no internet connection at all.
				WWDEBUG_SAY(("WOLLogonMgr: Failed to detect bandwidth.\n"));
				cUserOptions::Set_Bandwidth_Type(BANDWIDTH_MODEM_56);

				if (result != WaitCondition::UserCancel)
					{
					//if (BandwidthCheckerClass::Failed_Due_To_No_Connection())
					//	{
						if (mQuietMode)
							{
							// Try again. The auto detect won't fire off a second time since we switched to 56k
							if (ConsoleBox.Is_Exclusive()) {
								cUserOptions::Set_Bandwidth_Type(BANDWIDTH_LANT1);
							}
							InitiateLogon(false);
							//Add_Ref();
							//WOLLogonAction action = WOLLOGON_FAILED;
							//NotifyObservers(action);
							//Release_Ref();
							}
						else
							{
							//DlgMsgBox::DoDialog(L"IDS_WOL_LOGONFAILED", TRANSLATE (IDS_MENU_NO_INET_CONNECTION));
							// Text reads....
							// 	Renegade is unable to detect your Internet connection speed\n.
							// 	Defaulting connection speed to 56k.\n
							// 	If this is incorrect, you can change it on the My Information page.
							Add_Ref();
							DlgMsgBox::DoDialog(L"", TRANSLATE (IDS_MENU_SET_CONNECTION_SPEED), DlgMsgBox::Okay, this);
							mState = WAITING_BANDWIDTH_DIALOG_OKAY;
							}

					//	}
					//else
					//	{
					//	InitiateLogon(false);
					//	}
					}
				else
					{
					Add_Ref();
					WOLLogonAction action = WOLLOGON_FAILED;
					NotifyObservers(action);
					Release_Ref();
					}
				}
			break;

		// If we were waiting for the serverlist
		case FETCHING_SERVERLIST:
			mState = DISCONNECTED;

			if (result == WaitCondition::ConditionMet)
				{
				WWDEBUG_SAY(("WOLLogonMgr: Got server list\n"));
				ConsoleBox.Print("Got server list\n");

				if (HasServerList())
					{
					InitiateLogon(false);
					}
				else
					{
					WWDEBUG_SAY(("WOLLogonMgr: Serverlist empty!\n"));

					if (mQuietMode)
						{
						Add_Ref();
						WOLLogonAction action = WOLLOGON_FAILED;
						NotifyObservers(action);
						Release_Ref();
						}
					else
						{
						DlgMsgBox::DoDialog(IDS_WOL_LOGONFAILED, IDS_WOL_SERVERLISTERROR);
						}
					}
				}
			else
				{
				const wchar_t* resultText = waitEvent.Subject()->GetResultText();
				WWDEBUG_SAY(("WOLLogonMgr: Serverlist error %S\n", resultText));

				if (mQuietMode)
					{
					Add_Ref();
					WOLLogonAction action = WOLLOGON_FAILED;
					NotifyObservers(action);
					Release_Ref();
					}
				else if (WaitCondition::UserCancel != result)
					{
					DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_LOGONFAILED), resultText);
					}
				}
			break;

		// If we are waiting on pings.
		case WAITING_PINGS:
			mState = DISCONNECTED;

			if ((result == WaitCondition::ConditionMet) && HasValidPings())
				{
				WWDEBUG_SAY(("WOLLogonMgr: Got server pings\n"));
				ConsoleBox.Print("Got server pings\n");
				InitiateLogon(false);
				}
			else
				{
				WWDEBUG_SAY(("WOLLogonMgr: Ping server list error.\n"));

				if (mQuietMode)
					{
					Add_Ref();
					WOLLogonAction action = WOLLOGON_FAILED;
					NotifyObservers(action);
					Release_Ref();
					}
				else if (WaitCondition::UserCancel != result)
					{
					DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_LOGONFAILED), TRANSLATE(IDS_MENU_UNABLE_TO_DETERMINE_SERVER_LOC));
					}
				}
			break;

		// If we are waiting for connection
		case CONNECTING:
			if (WaitCondition::ConditionMet == result)
				{
				WWDEBUG_SAY(("WOLLogonMgr: Connection succeeded\n"));
				mState = CONNECTED;

				RememberLogin();

				Add_Ref();
				WOLLogonAction action = WOLLOGON_SUCCESS;
				NotifyObservers(action);
				Release_Ref();
				}
			else
				{
				WWDEBUG_SAY(("WOLLogonMgr: Connection failed!\n"));
				mState = DISCONNECTED;

				// Prevent event handling after error or user abort
				Observer<ConnectionStatus>::StopObserving();
				Observer<MessageOfTheDayEvent>::StopObserving();

				// No feedback to the user if we are in 'quiet' mode.
				if (mQuietMode)
					{
					Add_Ref();
					WOLLogonAction action = WOLLOGON_FAILED;
					NotifyObservers(action);
					Release_Ref();
					}
				else if (WaitCondition::UserCancel != result)
					{
					// Bring up dialog to allow user to adjust login information
					WWDEBUG_SAY(("WOLLogonMgr: Requesting login information.\n"));
					DlgWOLLogon::DoDialog(mLoginName, this);

					// Show reason we could not connect.
					RefPtr<WaitCondition> wait = waitEvent.Subject();

					WideStringClass errorMsg(255, true);
					errorMsg.Format(TRANSLATE (IDS_MENU_UNABLE_TO_CONNECT_TO_SERVER), wait->GetResultText());
					DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_LOGONFAILED), errorMsg);

					// Keep alive while waiting for user to correct logon information.
					Add_Ref();
					}
				}
			break;

		case DISCONNECTED:
		default:
			break;
		}

	// Release keep alive reference
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(DlgWOLLogonAction)
*
* DESCRIPTION
*     Handle notification from logon dialog.
*
* INPUTS
*     Event - Event action from logon dialog.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(DlgWOLLogonEvent& event)
	{
	// If the user clicked "Login" then attempt to login with the information
	// entered by the user in the login dialog.
	if (event.GetEvent() == DlgWOLLogonEvent::Login)
		{
		DlgWOLLogon& logonDialog = event.Subject();

		// Get selected login
		const wchar_t* name = NULL;
		const wchar_t* password = NULL;
		logonDialog.GetLogin(&name, &password, mPasswordEncrypted);

		mLoginName = name;
		mPassword = password;
		mRememberLogin = logonDialog.IsRememberLoginChecked();

		InitiateLogon(true);
		}

	// If the user clicked "cancel" the abort then cancel the login procedure
	else if (event.GetEvent() == DlgWOLLogonEvent::Cancel)
		{
		Add_Ref();
		WOLLogonAction action = WOLLOGON_CANCEL;
		NotifyObservers(action);
		Release_Ref();
		}

	// Release keep alive reference
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(ServerError)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(ServerError& error)
	{
	if (CHAT_E_MUSTPATCH == error.GetErrorCode())
		{
		mState = DISCONNECTED;

		Add_Ref();
		WOLLogonAction action = WOLLOGON_PATCHREQUIRED;
		NotifyObservers(action);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(ConnectionStatus)
*
* DESCRIPTION
*     Handle connection to WOL server status.
*
* INPUTS
*     Status - Connection status
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(ConnectionStatus& status)
	{
	if (status == ConnectionConnected)
		{
		ConfigureSession();

		// Nag the user about properly configuring their locale (if necessary)
		if (!mQuietMode)
			{
			WolLocaleMgrClass::Display_Nag_Dialog();
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(DlgMsgBoxEvent)
*
* DESCRIPTION
*     Handle any dialog box notifications.
*
* INPUTS
*     Event - Dialog box event
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(DlgMsgBoxEvent &event)
	{
	if (event.Event () == DlgMsgBoxEvent::Okay && mState == WAITING_BANDWIDTH_DIALOG_OKAY)
		{
		mState = DISCONNECTED;
		InitiateLogon(false);
		Release_Ref();
		}
	}


/******************************************************************************
*
* NAME
*     WOLLogonMgr::HandleNotification(MessageOfTheDayEvent)
*
* DESCRIPTION
*     Handle the message of the day text.
*
* INPUTS
*     Event - Message of the day event.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLLogonMgr::HandleNotification(MessageOfTheDayEvent &event)
	{
	// Get the name of the current server and the text of the message
	WideStringClass& message = event.Subject();

	const WCHAR* TAG_NEWS_START = L"<news>";
	const WCHAR* TAG_NEWS_END = L"</news>";
	const int TAG_NEWS_START_LEN = ::wcslen(TAG_NEWS_START);

	bool display_motd = false;

	// Does this message have the embedded news tag?
	const WCHAR* news = ::wcsstr(message, TAG_NEWS_START);

	if (news)
		{
		// Get the text of the news section
		WideStringClass news_body(0, true);
		news_body = news + TAG_NEWS_START_LEN;
		WCHAR* news_end = (WCHAR*)::wcsstr(news_body, TAG_NEWS_END);

		if (news_end)
			{
			news_end[0] = 0;
			}

		// If the message of the day has already been viewed, then
		// we just want to display the news items, otherwise display
		// the whole message (minus the embedded tags)
		if (MPSettingsMgrClass::Has_MOTD_Been_Viewed())
			{
			message = news_body;
			}
		else
			{
			// Simply erase the tags from the message
			int tag1_index = news - message.Peek_Buffer();
			int tag2_index = tag1_index + (TAG_NEWS_START_LEN) + ::wcslen(news_body);
			message.Erase(tag2_index, ::wcslen(TAG_NEWS_END));
			message.Erase(tag1_index, ::wcslen(TAG_NEWS_START));
			}

		display_motd = true;
		}

	// Only display the message if the user hasn't already seen it
	if (MPSettingsMgrClass::Has_MOTD_Been_Viewed() == false || display_motd)
		{
		// Show the MOTD dialog
		MPWolMOTDDialogClass* dialog = new MPWolMOTDDialogClass;

		if (dialog)
			{
			// Don't show the message of the day if we are in restart mode.
			if (!AutoRestart.Is_Active())
				{
				dialog->Set_Message(message);
				dialog->Start_Dialog();
				}

			REF_PTR_RELEASE(dialog);

			// Remember that we've already viewed the message of the day
			MPSettingsMgrClass::Set_MOTD_Viewed(true);
			}
		}
	}
