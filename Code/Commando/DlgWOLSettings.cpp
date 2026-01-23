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
*     $Archive: /Commando/Code/Commando/DlgWOLSettings.cpp $
*
* DESCRIPTION
*     Westwood Online Account Settings
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Greg_h $
*
* VERSION INFO
*     $Revision: 43 $
*     $Modtime: 6/21/02 11:40a $
*
******************************************************************************/

#include "DlgWOLSettings.h"
#include "renegadedialog.h"
#include "useroptions.h"
#include "renegadedialogmgr.h"
#include "mpsettingsmgr.h"
#include "DlgMessageBox.h"
#include "DlgWOLWait.h"
#include "DlgWebPage.h"
#include "WOLLoginProfile.h"
#include "WOLLogonMgr.h"
#include <WWOnline/WOLServer.h>
#include <WWOnline/WOLLoginInfo.h>
#include <WWOnline/PingProfile.h>
#include <wwui/comboboxctrl.h>
#include <wwui/shortcutbarctrl.h>
#include "bandwidthcheck.h"

#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <algorithm>

using namespace WWOnline;

/******************************************************************************
*
* NAME
*     DlgWOLSettings::DoDialog
*
* DESCRIPTION
*     Start Westwood Online settings dialog.
*
* INPUTS
*     NONE
*
* RESULT
*     True if dialog successfully started.
*
******************************************************************************/

bool DlgWOLSettings::DoDialog(void)
	{
	DlgWOLSettings* dialog = new DlgWOLSettings;

	if (dialog)
		{
		if (dialog->FinalizeCreate())
			{
			dialog->Start_Dialog();
			}

		dialog->Release_Ref();
		}

	return (dialog != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::DlgWOLSettings
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

DlgWOLSettings::DlgWOLSettings() :
		MenuDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_WOL_SETTINGS)),
		DetectingBandwidth(false),
		WaitingToExitDialog(false)
	{
	WWDEBUG_SAY(("DlgWOLSettings: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::~DlgWOLSettings
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

DlgWOLSettings::~DlgWOLSettings()
	{
	WWDEBUG_SAY(("DlgWOLSettings: Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::FinalizeCreate
*
* DESCRIPTION
*     Finalize the creation of this object (Initialize).
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool DlgWOLSettings::FinalizeCreate(void)
	{
	mWOLSession = Session::GetInstance(false);
	return mWOLSession.IsValid();
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::On_Init_Dialog
*
* DESCRIPTION
*     Initialize dialog for first time.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::On_Init_Dialog(void)
	{
	// Configure the shortcut bar
	ShortcutBarCtrlClass* bar = (ShortcutBarCtrlClass*)Get_Dlg_Item(IDC_SHORTCUT_BAR);

	if (bar)
		{
		bar->Add_Button(IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE(IDS_INTERNET_OPTIONS));

		#ifdef QUICKMATCH_OPTIONS
		bar->Add_Button(IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS, TRANSLATE(IDS_MENU_TEXT364));
		#endif

		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));
		}

	// Initialize combos
	InitPersonaCombo();
	InitSideCombo();
	InitLocaleCombo();
	InitConnectionSpeedCombo();

	UpdateForPersona();

	// Initialize server combos
	const IRCServerList& servers = mWOLSession->GetIRCServerList();

	// If we already have a list of servers then use those. Otherwise
	// request a server list from WOL
	if (servers.size() > 0)
		{
		InitServersCombo(servers);
		}
	else
		{
		RefPtr<SerialWait> serverWait = SerialWait::Create();
		WWASSERT(serverWait.IsValid());

		if (BandwidthCheckerClass::Got_Bandwidth() == 0 && cUserOptions::Get_Bandwidth_Type() == BANDWIDTH_AUTO)
			{
			DetectingBandwidth = true;
			Add_Ref();
			RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();
			serverWait->Add(bandwidth_wait);
			}

		// Request server list.
		WWDEBUG_SAY(("DlgWOLSettings: Requesting ServerList\n"));
		RefPtr<WaitCondition> listWait = mWOLSession->GetNewServerList();
		serverWait->Add(listWait);

		RefPtr<WaitCondition> pingWait = PingProfileWait::Create();
		serverWait->Add(pingWait);

		RefPtr<WaitCondition> wait(serverWait);
		DlgWOLWait::DoDialog(IDS_WOL_SERVERLISTFETCH, wait, this);
		}

	MenuDialogClass::On_Init_Dialog();
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::On_Destroy
*
* DESCRIPTION
*     Handle dialog shutdown.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::On_Destroy(void)
	{
	ClearPersonaCombo();
	MenuDialogClass::On_Destroy();
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::On_Command
*
* DESCRIPTION
*
* INPUTS
*     Ctrl  - Control command is from.
*     Msg   - Command message
*     Param - Command parameter
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::On_Command(int ctrl, int message, unsigned int param)
	{
	switch (ctrl)
		{
		case IDOK:
			{
			ComboBoxCtrlClass* connectCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CONNECTION_SPEED_COMBO);

			if (connectCombo && BandwidthCheckerClass::Got_Bandwidth() == 0)
				{
				int sel = connectCombo->Get_Curr_Sel();

				if (sel >= 0)
					{
					BANDWIDTH_TYPE_ENUM bandwidth = std::min<BANDWIDTH_TYPE_ENUM>((BANDWIDTH_TYPE_ENUM)(sel + BANDWIDTH_FIRST), BANDWIDTH_LAST);

					if (bandwidth == BANDWIDTH_AUTO)
						{
						if (!DetectingBandwidth)
							{
							RefPtr<SerialWait> serverWait = SerialWait::Create();
							WWASSERT(serverWait.IsValid());

							DetectingBandwidth = true;
							Add_Ref();
							RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();
							DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), bandwidth_wait, this);
							WaitingToExitDialog = true;
							break;
						}
					}
				}
			}

			if (SaveSettings())
				{
				End_Dialog();
				}
			}

			return;
			break;

		case IDC_DELETE_ACCOUNT_BUTTON:
			DeleteSelectedPersona();
			break;

		case IDC_NEWPERSONA:
			DlgWebPage::DoDialog("Signup");
			break;

		case IDC_CHECK_BANDWIDTH:
			if (!DetectingBandwidth)
				{
				RefPtr<SerialWait> serverWait = SerialWait::Create();
				WWASSERT(serverWait.IsValid());

				DetectingBandwidth = true;
				Add_Ref();
				RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();
				DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), bandwidth_wait, this);
				}
			break;

		case IDC_AUTOLOGIN_CHECK:
			if (param && mWOLSession->IsAutoLoginAllowed())
				{
				// Get preferred login
				WideStringClass name(64, true);
				name = Get_Dlg_Item_Text(IDC_PERSONA_COMBO);

				// Save preferred login
				StringClass saveName(64, true);
				name.Convert_To(saveName);
				MPSettingsMgrClass::Set_Auto_Login(saveName);
				}
			else
				{
				MPSettingsMgrClass::Set_Auto_Login("");
				}
			break;

		case IDC_MENU_BACK_BUTTON:
			break;
		}

	MenuDialogClass::On_Command(ctrl, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::SaveSettings
*
* DESCRIPTION
*     Save the users settings.
*
* INPUTS
*     NONE
*
* RESULT
*     True if save successful.
*
******************************************************************************/

bool DlgWOLSettings::SaveSettings(void)
	{
	WWDEBUG_SAY(("DlgWOLSettings: Saving settings\n"));

	// Save the settings for each login.
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		int count = combo->Get_Item_Count();

		for (int index = 0; index < count; ++index)
			{
			LoginProfile* profile = (LoginProfile*)combo->Get_Item_Data(index);

			if (profile)
				{
				profile->SaveSettings();
				}
			}
		}

	// Save connection speed
	ComboBoxCtrlClass* connectCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CONNECTION_SPEED_COMBO);

	if (connectCombo)
		{
		int sel = connectCombo->Get_Curr_Sel();

		if (sel >= 0)
			{
			BANDWIDTH_TYPE_ENUM bandwidth = std::min<BANDWIDTH_TYPE_ENUM>((BANDWIDTH_TYPE_ENUM)(sel + BANDWIDTH_FIRST), BANDWIDTH_LAST);
			cUserOptions::Set_Bandwidth_Type(bandwidth);
			}
		}

	return true;
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::InitPersonaCombo
*
* DESCRIPTION
*     Initialize the preferred login combo box with the cached WWOnline
*     accounts.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::InitPersonaCombo(void)
	{
	ClearPersonaCombo();

	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		// Populate the persona combo box with the list of WWOnline logins.
		const LoginInfoList& logins = LoginInfo::GetList();
		const size_t count = logins.size();

		for (size_t index = 0; index < count; ++index)
			{
			RefPtr<LoginInfo> login = logins[index];
			WWASSERT(login.IsValid());

			if (login.IsValid())
				{
				const WideStringClass& name = login->GetNickname();
				int item = combo->Add_String(name);

				// Create a profile for each login
				if (item >= 0)
					{
					LoginProfile* profile = LoginProfile::Get(name, true);
					combo->Set_Item_Data(item, (uintptr_t)profile);
					}
				}
			}

		combo->Set_Curr_Sel(0);

		// Select the last login
		WideStringClass loginName(64, true);
		loginName = MPSettingsMgrClass::Get_Last_Login();
		combo->Select_String(loginName);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::ClearPersonaCombo
*
* DESCRIPTION
*     Release the personas.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::ClearPersonaCombo(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		// Release the login profiles
		const int count = combo->Get_Item_Count();

		for (int index = 0; index < count; ++index)
			{
			LoginProfile* profile = (LoginProfile*)combo->Get_Item_Data(index);

			if (profile)
				{
				profile->Release_Ref();
				}
			}

		combo->Reset_Content();
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::DeleteSelectedPersona
*
* DESCRIPTION
*     Delete the currently selected persona.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::DeleteSelectedPersona(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		int sel = combo->Get_Curr_Sel();

		if (sel >= 0)
			{
			const unichar_t* name = combo->Get_Text();

			// Delete this login from our local cache and purge it from disk.
			RefPtr<LoginInfo> login = LoginInfo::Find(name);

			if (login.IsValid())
				{
				login->Forget(true);
				}

			// Release the login profile
			LoginProfile* profile = (LoginProfile*)combo->Get_Item_Data(sel);

			if (profile)
				{
				profile->Release_Ref();
				}

			// Delete the persistent profile.
			LoginProfile::Delete(name);

			// Remove the persona from the combo box.
			combo->Delete_String(sel);

			// Refresh the dialog
			UpdateForPersona();
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::UpdateForPersona
*
* DESCRIPTION
*     Update the dialog for the selected persona
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::UpdateForPersona(void)
	{
	int sel = -1;

	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		sel = combo->Get_Curr_Sel();

		if (sel >= 0)
			{
			// Update the dialog with settings from the persona's profile.
			WideStringClass newLogin(64, true);
			combo->Get_String(sel, newLogin);

			LoginProfile* profile = (LoginProfile*)combo->Get_Item_Data(sel);

			// Set dialog to reflect the profile settings
			if (profile)
				{
				SetServerCombo(profile->GetPreferredServer());
				SetSideCombo(profile->GetSidePreference());
				SetLocaleCombo(profile->GetLocale());
				}
			else
				{
				SetServerCombo("");
				SetSideCombo(-1);
				SetLocaleCombo(WOL::LOC_UNKNOWN);
				}

			ShowProfileRanking(this, profile);

			// Set the autologin checkbox appropriately for this persona.
			WideStringClass preferred(64, true);
			preferred = MPSettingsMgrClass::Get_Auto_Login();

			bool autoLogin = (mWOLSession->IsAutoLoginAllowed() && (newLogin.Compare_No_Case(preferred) == 0));
			Check_Dlg_Button(IDC_AUTOLOGIN_CHECK, autoLogin);

			// Set the selected login to the persona that is selected.
			StringClass loginName(64, true);
			newLogin.Convert_To(loginName);
			MPSettingsMgrClass::Set_Last_Login(loginName);
			}
		else
			{
			SetServerCombo("");
			SetSideCombo(-1);
			SetLocaleCombo(WOL::LOC_UNKNOWN);
			ShowProfileRanking(this, NULL);
			}
		}

	bool isValid = (sel >= 0);

	Enable_Dlg_Item(IDC_PERSONA_COMBO, isValid);
	Enable_Dlg_Item(IDC_GAMESERVER_COMBO, isValid);
	Enable_Dlg_Item(IDC_CHOOSESIDE_COMBO, isValid);
	Enable_Dlg_Item(IDC_LOCALE_COMBO, isValid);
	Enable_Dlg_Item(IDC_AUTOLOGIN_CHECK, isValid && mWOLSession->IsAutoLoginAllowed());
	Enable_Dlg_Item(IDC_DELETE_ACCOUNT_BUTTON, isValid);
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::InitServersCombo
*
* DESCRIPTION
*     Initialize server combo boxes
*
* INPUTS
*     Servers - List of servers to populate combo with.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::InitServersCombo(const IRCServerList& servers)
	{
	ComboBoxCtrlClass* gameCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_GAMESERVER_COMBO);

	if (gameCombo)
		{
		// Remember current chat selection
		WideStringClass gameSelName(0, true);

		LoginProfile* profile = GetLoginProfile();

		if (profile)
			{
			gameSelName = profile->GetPreferredServer();
			}

		int gameSel = -1;
		gameCombo->Reset_Content();

		// convenience reference for list of ping servers
		const PingServerList& pingers = mWOLSession->GetPingServerList();

		int bestTime = INT_MAX;
		int tmpSel = -1;

		// walk the list of servers and add them to the combo box
		const size_t serverCount = servers.size();

		for (size_t index = 0; index < serverCount; ++index)
			{
			const RefPtr<IRCServerData>& server = servers[index];

			// Filter out servers that do not have a set of supported languages.
			if (server->HasLanguageCode())
				{
				WideStringClass serverName(0, true);
				serverName = server->GetName();

				int pos = gameCombo->Add_String(serverName);

				// Now, figure out which one to pick

				// they haven't picked a preferred server
				if (gameSelName.Is_Empty())
					{
					// lets pick a default server for them!
					if (server->MatchesLanguageCode())
						{
						float serverLong = server->GetLongitude();
						float serverLat = server->GetLattitude();

						// Find the ping server with the best time that matches this server's lat/long
						const size_t pingersCount = pingers.size();

						for (size_t pingindex = 0; pingindex < pingersCount; ++pingindex)
							{
							const RefPtr<PingServerData>& thisPing = pingers[pingindex];
							float pingLong = thisPing->GetLongitude();
							float pingLat = thisPing->GetLattitude();

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
								if (pingTime < bestTime)
									{
									bestTime = pingTime;
									tmpSel = pos;
									}
								}
							} // end for each ping server
						}
					}

				// If this is their preferred server
				else if ((gameSelName.Compare_No_Case(serverName) == 0) && (gameSel == -1))
					{
					gameSel = pos;              // they picked one, and this is it!
					gameCombo->Set_Curr_Sel(pos);

					if (profile)
						{
						profile->SetPreferredServer(server->GetName());
						}
					}
				} // if server has a langcode - sanity check
			}

		if ((gameSel == -1) && (tmpSel != -1))
			{
			gameSel = tmpSel;
			gameCombo->Set_Curr_Sel(tmpSel);
			}
		} // if valid gameCombo box
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::SetServerCombo
*
* DESCRIPTION
*     Set the server combo to the specified server.
*
* INPUTS
*     Name - Server to set server combo to.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::SetServerCombo(const char* serverName)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_GAMESERVER_COMBO);

	if (combo)
		{
		WWDEBUG_SAY(("ServerCombo = %s\n", serverName));
		WideStringClass name(64, true);
		name = serverName;

		int sel = combo->Select_String(name);

		if (sel == -1)
			{
			RefPtr<IRCServerData> defaultServer = WOLLogonMgr::GetDefaultServer();

			if (defaultServer.IsValid())
				{
				name = defaultServer->GetName();
				combo->Select_String(name);
				}
			else
				{
				combo->Set_Curr_Sel(0);
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::InitSideCombo
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

void DlgWOLSettings::InitSideCombo(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CHOOSESIDE_COMBO);

	if (combo)
		{
		//(gth) Renegade day 120 Patch: re-translate these strings each time!
		struct {const unichar_t* TeamName; int TeamID;} _teams[] =
			{
			{TRANSLATE (IDS_MENU_RANDOM), -1},
			{TRANSLATE (IDS_MENU_TEXT933), 1},
			{TRANSLATE (IDS_MENU_TEXT934), 0},
			{NULL, -1},
			};

		int index = 0;

		while (_teams[index].TeamName != NULL)
			{
			int item = combo->Add_String(_teams[index].TeamName);

			if (item >= 0)
				{
				combo->Set_Item_Data(item, (uint32)_teams[index].TeamID);
				}

			++index;
			}
		}

	// Selected current logins preference
	int side = -1;

	LoginProfile* profile = GetLoginProfile();

	if (profile)
		{
		side = profile->GetSidePreference();
		}

	SetSideCombo(side);
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::SetSideCombo
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::SetSideCombo(int side)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CHOOSESIDE_COMBO);

	if (combo)
		{
		const int count = combo->Get_Item_Count();

		for (int index = 0; index < count; ++index)
			{
			if (combo->Get_Item_Data(index) == (uint32)side)
				{
				combo->Set_Curr_Sel(index);
				break;
				}
			}
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::InitLocaleCombo
*
* DESCRIPTION
*     Update the locale combo box with the selectable WWOnline locales.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::InitLocaleCombo(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_LOCALE_COMBO);

	if (combo)
		{
		combo->Reset_Content();

		std::vector<WideStringClass> localeNames;
		mWOLSession->GetLocaleStrings(localeNames);

		const size_t localeCount = localeNames.size();

		for (size_t index = 0; index < localeCount; ++index)
			{
			WideStringClass& locale = localeNames[index];
			combo->Add_String(locale);
			}

		int locale = WOL::LOC_UNKNOWN;
		LoginProfile* profile = GetLoginProfile();

		if (profile)
			{
			locale = profile->GetLocale();
			}

		combo->Set_Curr_Sel(locale);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::SetLocaleCombo
*
* DESCRIPTION
*
* INPUTS
*     Name - Persona to set locale for.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::SetLocaleCombo(WOL::Locale locale)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_LOCALE_COMBO);

	if (combo)
		{
		WWDEBUG_SAY(("LocaleCombo = %ld\n", locale));
		combo->Set_Curr_Sel((int)locale);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::InitConnectionSpeedCombo
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

void DlgWOLSettings::InitConnectionSpeedCombo(void)
	{
	// Configure the connection type combobox
	ComboBoxCtrlClass* connectCombo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_CONNECTION_SPEED_COMBO);

	if (connectCombo)
		{
		connectCombo->Reset_Content();
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_288));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_336));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_56));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_ISDN));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_CABLE));
		connectCombo->Add_String(TRANSLATE(IDS_MP_CONNECTION_T1));

		WideStringClass auto_set_str(256, true);

		if (BandwidthCheckerClass::Get_Reported_Upstream_Bandwidth())
			{
			auto_set_str.Format(TRANSLATE (IDS_MENU_AUTO_FORMAT), BandwidthCheckerClass::Get_Bandwidth_As_String());
			}
		else
			{
			auto_set_str.Format(TRANSLATE (IDS_MENU_AUTO));
			}

		connectCombo->Add_String(auto_set_str);

		// Set the current selection
		connectCombo->Set_Curr_Sel(cUserOptions::Get_Bandwidth_Type() - BANDWIDTH_FIRST);
		connectCombo->Set_Dirty(true);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::On_ComboBoxCtrl_Sel_Change
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* combo, int ctrl,
		int /* oldSel */, int newSel)
	{
	switch (ctrl)
		{
		case IDC_PERSONA_COMBO:
			UpdateForPersona();
			break;

		case IDC_GAMESERVER_COMBO:
			{
			LoginProfile* profile = GetLoginProfile();

 			if (profile)
				{
				// Save selected game server
				WideStringClass name(64, true);
				combo->Get_String(newSel, name);

				StringClass saveName(64, true);
				name.Convert_To(saveName);
				profile->SetPreferredServer(saveName);
				}
			}
			break;

		case IDC_CHOOSESIDE_COMBO:
			{
			LoginProfile* profile = GetLoginProfile();

 			if (profile)
				{
				int side = combo->Get_Item_Data(newSel);
				profile->SetSidePreference(side);
				}
			}
			break;

		case IDC_LOCALE_COMBO:
			{
			LoginProfile* profile = GetLoginProfile();

 			if (profile)
				{
				profile->SetLocale((WOL::Locale)newSel);
				}
			}
			break;

		default:
			break;
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::HandleNotification(DlgWOLWaitEvent)
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgWOLSettings::HandleNotification(DlgWOLWaitEvent& event)
	{
	if (WaitCondition::ConditionMet == event.Result())
		{
		if (DetectingBandwidth)
			{
			DetectingBandwidth = false;
			cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);

			// Re-init the connection speed combo.
			InitConnectionSpeedCombo();
			Release_Ref();

			if (WaitingToExitDialog)
				{
				if (SaveSettings())
					{
					End_Dialog();
					return;
					}
				}
			}

		// Check if we got the server list.
		const IRCServerList& servers = mWOLSession->GetIRCServerList();

		if (servers.size())
			{
			InitServersCombo(servers);
			}
		}
	else if (DetectingBandwidth == false)
		{
		Enable_Dlg_Item(IDC_GAMESERVER_COMBO, false);
		}
	}


/******************************************************************************
*
* NAME
*     DlgWOLSettings::GetLoginProfile
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

LoginProfile* DlgWOLSettings::GetLoginProfile(void)
	{
	ComboBoxCtrlClass* combo = (ComboBoxCtrlClass*)Get_Dlg_Item(IDC_PERSONA_COMBO);

	if (combo)
		{
		int sel = combo->Get_Curr_Sel();

		if (sel >= 0)
			{
			return (LoginProfile*)combo->Get_Item_Data(sel);
			}
		}

	return NULL;
	}
