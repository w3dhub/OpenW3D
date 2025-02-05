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
*     $Archive: /Commando/Code/Commando/WOLLoginProfile.cpp $
*
* DESCRIPTION
*     Login profile holds the users login preferences as well as other
*     information that needs to be persistantly cached between game sessions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Greg_h $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 6/21/02 11:40a $
*
******************************************************************************/

#include "WOLLoginProfile.h"
#include "_globals.h"
#include "Resource.h"
#include <WWOnline\WOLLoginInfo.h>
#include <WWUI\DialogBase.h>
#include <WWUI\ListCtrl.h>
#include <WWLib\Registry.h>
#include "String_IDs.h"
#include <WWTranslateDB\TranslateDB.h>
#include <stdio.h>

using namespace WWOnline;

#define MAX_STRING_LEN 64;

static const char* REG_VALUE_SERVER   = "Server";
static const char* REG_VALUE_SIDEPREF = "SidePref";
static const char* REG_VALUE_GAMESPLAYED = "Played";
static const char* REG_VALUE_TEAMRANK = "RankTeam";
static const char* REG_VALUE_CLANRANK = "RankClan";

// Profile ranking columns
enum
{
	COL_LADDERNAME = 0,
	COL_WINS,
	COL_DEATHS,
	COL_POINTS,
	COL_RANK,

	COL_MAXCOUNT
};

static void ShowRanking(ListCtrlClass* list, LadderType type, const LoginProfile::Ranking* rank);


bool LoginProfile::_mSaveAllowed = true;
LoginProfile* LoginProfile::_mCurrentProfile = NULL;

/******************************************************************************
*
* NAME
*     LoginProfile::EnableSaving
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::EnableSaving(bool allowed)
	{
	_mSaveAllowed = allowed;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetCurrent
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetCurrent(LoginProfile* profile)
	{
	if (_mCurrentProfile != NULL)
		{
		_mCurrentProfile->Release_Ref();
		_mCurrentProfile = NULL;
		}

	if (profile)
		{
		profile->Add_Ref();
		_mCurrentProfile = profile;
		}
	}


/******************************************************************************
*
* NAME
*     LoginProfile::Get
*
* DESCRIPTION
*     Get the login profile for the specified user.
*
* INPUTS
*     Name - Name of profile to get.
*
* RESULT
*     Profile - Instance to login profile
*
******************************************************************************/

LoginProfile* LoginProfile::Get(const wchar_t* loginName, bool createOK)
	{
	if (loginName && wcslen(loginName))
		{
		// Check the current profile first
		if (_mCurrentProfile)
			{
			if (wcsicmp(_mCurrentProfile->GetName(), loginName) == 0)
				{
				_mCurrentProfile->Add_Ref();
				return _mCurrentProfile;
				}
			}

		StringClass regKey(255, true);
		regKey.Format("%s\\%S", APPLICATION_SUB_KEY_NAME_LOGINS, loginName);

		if (RegistryClass::Exists(regKey) || createOK)
			{
			return Create(loginName);
			}
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::Create
*
* DESCRIPTION
*     Create an instance of a login profile.
*
* INPUTS
*     Login - Login to create profile for.
*
* RESULT
*     Profile - Instance to login profile
*
******************************************************************************/

LoginProfile* LoginProfile::Create(const wchar_t* loginName)
	{
	LoginProfile* profile = NULL;

	if (loginName && wcslen(loginName))
		{
		profile = new LoginProfile;

		if (profile)
			{
			if (profile->FinalizeCreate(loginName) == false)
				{
				profile->Release_Ref();
				profile = NULL;
				}
			}
		}

	return profile;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::Delete
*
* DESCRIPTION
*     Delete a login profile.
*
* INPUTS
*     Name - Name of login profile to delete.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::Delete(const wchar_t* loginName)
	{
	if (loginName && wcslen(loginName))
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_LOGINS, false);
	
		if (registry.Is_Valid())
			{
			char valueName[64];
			wcstombs(valueName, loginName, sizeof(valueName));
			registry.Delete_Value(valueName);
			}
		}
	}


/******************************************************************************
*
* NAME
*     LoginProfile::LoginProfile
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

LoginProfile::LoginProfile() :
		mLocale(WOL::LOC_UNKNOWN),
		mSidePref(-1),
		mGamesPlayed(0)
	{
	memset(&mTeamRank, 0, sizeof(Ranking));
	memset(&mClanRank, 0, sizeof(Ranking));
	}


/******************************************************************************
*
* NAME
*     LoginProfile::~LoginProfile
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

LoginProfile::~LoginProfile()
	{
	WWDEBUG_SAY(("LoginProfile destroyed (%S)\n", (const WCHAR*)mName));
	}


/******************************************************************************
*
* NAME
*     LoginProfile::FinalizeCreate
*
* DESCRIPTION
*     Finalize object creation
*
* INPUTS
*    Login - Login to initialize profile with.
*
* RESULT
*    True if successfull
*
******************************************************************************/

bool LoginProfile::FinalizeCreate(const wchar_t* loginName)
	{
	WWDEBUG_SAY(("LoginProfile created %S\n", loginName));
	mName = loginName;
	LoadSettings();
	return true;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::GetName
*
* DESCRIPTION
*     Get the name of this profile
*
* INPUTS
*     NONE
*
* RESULT
*     Name of profile
*
******************************************************************************/

const wchar_t* LoginProfile::GetName(void) const
	{
	return mName;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetPreferredServer
*
* DESCRIPTION
*     Set the preferred server for this login.
*
* INPUTS
*     Server - Name of preferred server.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetPreferredServer(const char* name)
	{
	mServer = name;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetSidePreference
*
* DESCRIPTION
*     Set the side preference
*
* INPUTS
*     Side - Prefered side
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetSidePreference(int side)
	{
	mSidePref = side;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetGamesPlayed
*
* DESCRIPTION
*     Set the number of games played for this login.
*
* INPUTS
*     Played - Number of games played.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetGamesPlayed(unsigned long played)
	{
	mGamesPlayed = played;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetLocale
*
* DESCRIPTION
*     Set locale for this profile
*
* INPUTS
*     Locale - New locale
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetLocale(WOL::Locale locale)
	{
	mLocale = max<WOL::Locale>(locale, WOL::LOC_UNKNOWN);
	}


/******************************************************************************
*
* NAME
*     LoginProfile::GetRanking
*
* DESCRIPTION
*     Get ladder ranking information.
*
* INPUTS
*     Ladder - Type of ladder to get ranking information for.
*
* RESULT
*     Rank - Ladder ranking data
*
******************************************************************************/

const LoginProfile::Ranking* LoginProfile::GetRanking(LadderType type) const
	{
	if (type == LadderType_Team)
		{
		return &mTeamRank;
		}
	else if (type == LadderType_Clan)
		{
		return &mClanRank;
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SetRanking
*
* DESCRIPTION
*     Set ladder ranking information
*
* INPUTS
*     Ladder - Type of ladder to set ranking for.
*     Rank   - Ranking data
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SetRanking(WWOnline::LadderType type, Ranking& rank)
	{
	if (type == LadderType_Team)
		{
		memcpy(&mTeamRank, &rank, sizeof(Ranking));
		}
	else if (type == LadderType_Clan)
		{
		memcpy(&mClanRank, &rank, sizeof(Ranking));
		}
	}


/******************************************************************************
*
* NAME
*     LoginProfile::LoadSettings
*
* DESCRIPTION
*     Load cached profile settings.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::LoadSettings(void)
	{
	// Get this logins locale
	mLocale = WOL::LOC_UNKNOWN;

	RefPtr<LoginInfo> login = LoginInfo::Find(GetName());

	if (login.IsValid())
		{
		mLocale = login->GetLocale();
		}

	// Get login preferences
	StringClass regKey(255, true);
	regKey.Format("%s\\%S", APPLICATION_SUB_KEY_NAME_LOGINS, GetName());

	RegistryClass registry(regKey, false);
	
	if (registry.Is_Valid())
		{
		registry.Get_String(REG_VALUE_SERVER, mServer, "");
		mSidePref = registry.Get_Int(REG_VALUE_SIDEPREF, -1);
		mGamesPlayed = registry.Get_Int(REG_VALUE_GAMESPLAYED, 0);
		}

	LoadRank(REG_VALUE_TEAMRANK, mTeamRank);
	LoadRank(REG_VALUE_CLANRANK, mClanRank);
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SaveSettings
*
* DESCRIPTION
*     Save current profile settings.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SaveSettings(void)
	{
	bool isStored = true;
	RefPtr<LoginInfo> login = LoginInfo::Find(GetName());

	if (login.IsValid())
		{
		isStored = login->IsStored();

		// Save locale
		login->SetLocale((WOL::Locale)mLocale);
		}

	// If we are allowed to 
	if (_mSaveAllowed && isStored)
		{
		// Save login preferences
		StringClass regKey(255, true);
		regKey.Format("%s\\%S", APPLICATION_SUB_KEY_NAME_LOGINS, GetName());

		RegistryClass registry(regKey);
	
		if (registry.Is_Valid())
			{
			registry.Set_String(REG_VALUE_SERVER, mServer);
			registry.Set_Int(REG_VALUE_SIDEPREF, mSidePref);
			registry.Set_Int(REG_VALUE_GAMESPLAYED, mGamesPlayed);
			}

		SaveRank(REG_VALUE_TEAMRANK, mTeamRank);
		SaveRank(REG_VALUE_CLANRANK, mClanRank);
		}
	}


/******************************************************************************
*
* NAME
*     LoginProfile::LoadRank
*
* DESCRIPTION
*     Load cached ranking data.
*
* INPUTS
*     Key  - Registry key to load ranking from.
*     Rank - Ranking data to initialize.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::LoadRank(const char* valueName, LoginProfile::Ranking& rank)
	{
	WWASSERT(valueName);

	StringClass regKey(255, true);
	regKey.Format("%s\\%S", APPLICATION_SUB_KEY_NAME_LOGINS, GetName());

	RegistryClass registry(regKey, false);
	
	if (registry.Is_Valid())
		{
 		char rankData[255];
		registry.Get_String(valueName, rankData, sizeof(rankData), "");

		sscanf(rankData, "%d,%d,%d,%d,%d,%d", &rank.Wins, &rank.Losses,
			&rank.Deaths, &rank.Kills, &rank.Points, &rank.Rank);
		}
	else
		{
		rank.Wins = (unsigned)-1;
		rank.Losses = (unsigned)-1;
		rank.Deaths = (unsigned)-1;
		rank.Kills = (unsigned)-1;
		rank.Points = (unsigned)-1;
		rank.Rank = (unsigned)-1;
		}
	}


/******************************************************************************
*
* NAME
*     LoginProfile::SaveRank
*
* DESCRIPTION
*     Save current ranking data.
*
* INPUTS
*     Key  - Registry key to save ranking data to.
*     Rank - Ranking data to save.
*
* RESULT
*     NONE
*
******************************************************************************/

void LoginProfile::SaveRank(const char* valueName, const LoginProfile::Ranking& rank)
	{
	WWASSERT(valueName);

	StringClass regKey(255, true);
	regKey.Format("%s\\%S", APPLICATION_SUB_KEY_NAME_LOGINS, GetName());

	RegistryClass registry(regKey);
	
	if (registry.Is_Valid())
		{
 		char rankData[255];
		sprintf(rankData, "%d,%d,%d,%d,%d,%d", rank.Wins, rank.Losses, rank.Deaths,
			rank.Kills, rank.Points, rank.Rank);

		registry.Set_String(valueName, rankData);
		}
	}


/******************************************************************************
*
* NAME
*     ShowProfileRanking
*
* DESCRIPTION
*
* INPUTS
*     Dialog  - Dialog to display ranking on. (Must have neccessary controls)
*     Profile - Profile to display
*
* RESULT
*     NONE
*
******************************************************************************/

void ShowProfileRanking(DialogBaseClass* dialog, const LoginProfile* profile)
	{
	dialog->Set_Dlg_Item_Text(IDC_PROFILENAME, L"");

	ListCtrlClass* list = (ListCtrlClass*)dialog->Get_Dlg_Item(IDC_PROFILERANK);

	if (list)
		{
		list->Allow_Selection(false);
		list->Delete_All_Entries();

		if (list->Get_Column_Count() != COL_MAXCOUNT)
			{
			list->Delete_All_Columns();

			// Configure the columns
			Vector3 color(1, 1, 1);

			list->Add_Column(TRANSLATE(IDS_MENU_RANKING), 0.2F, color);
			list->Add_Column(TRANSLATE(IDS_MENU_WINS_LOSSES), 0.2F, color);
			list->Add_Column(TRANSLATE(IDS_BUDDY_COL_DEATHS_KILLS), 0.2F, color);
			list->Add_Column(TRANSLATE(IDS_BUDDY_COL_POINTS), 0.2F, color);
			list->Add_Column(TRANSLATE(IDS_BUDDY_COL_RANK), 0.2F, color);
			}

		//(gth) Renegade day 120 Patch: re-translate these strings each time!
		struct {const wchar_t* name; WWOnline::LadderType type;} _ladders[] =
			{
			{TRANSLATE(IDS_MENU_INDIVIDUAL), WWOnline::LadderType_Team},
			{TRANSLATE(IDS_MENU_CLAN), WWOnline::LadderType_Clan}
			};

		// Initialize with unknown data
		for (int index = 0; index < 2; ++index)
			{
			int item = list->Insert_Entry(index, _ladders[index].name);
		
			if (item != -1)
				{
				list->Set_Entry_Data(item, COL_LADDERNAME, _ladders[index].type);
				list->Set_Entry_Text(item, COL_WINS, L"- / -");
				list->Set_Entry_Text(item, COL_DEATHS, L"- / -");
				list->Set_Entry_Text(item, COL_POINTS, L"-");
				list->Set_Entry_Text(item, COL_RANK, L"-");
				}
			}

		// Show the profile's ranking
		if (profile)
			{
			WideStringClass profileName(128, true);
			profileName.Format(TRANSLATE(IDS_MENU_RANKING_PROFILE), profile->GetName());
			dialog->Set_Dlg_Item_Text(IDC_PROFILENAME, profileName);

			ShowRanking(list, WWOnline::LadderType_Team, profile->GetRanking(WWOnline::LadderType_Team));
			ShowRanking(list, WWOnline::LadderType_Clan, profile->GetRanking(WWOnline::LadderType_Clan));
			}
		}
	}


/******************************************************************************
*
* NAME
*     ShowRanking
*
* DESCRIPTION
*
* INPUTS
*     List - List control to show ranking in
*     Type - Type of ladder
*     Rank - Ladder ranking data
*
* RESULT
*     NONE
*
******************************************************************************/

void ShowRanking(ListCtrlClass* list, WWOnline::LadderType type, const LoginProfile::Ranking* rank)
	{
	WWASSERT(list != NULL);
	WWASSERT(rank != NULL);

	int count = list->Get_Entry_Count();

	for (int index = 0; index < count; index++)
		{
		if ((WWOnline::LadderType)list->Get_Entry_Data(index, COL_LADDERNAME) == type)
			{
			WideStringClass text(64, true);

			if (rank->Wins != (unsigned)-1)
				{
				text.Format(L"%u / %u", rank->Wins, rank->Losses);
				list->Set_Entry_Text(index, COL_WINS, text);
				}
			else
				{
				list->Set_Entry_Text(index, COL_WINS, L"- / -");
				}

			if ((rank->Deaths != (unsigned)-1) && (rank->Kills != (unsigned)-1))
				{
				text.Format(L"%u / %u", rank->Deaths, rank->Kills);
				list->Set_Entry_Text(index, COL_DEATHS, text);
				}
			else
				{
				list->Set_Entry_Text(index, COL_DEATHS, L"- / -");
				}

			if (rank->Points != (unsigned)-1)
				{
				text.Format(L"%u", rank->Points);
				list->Set_Entry_Text(index, COL_POINTS, text);
				}
			else
				{
				list->Set_Entry_Text(index, COL_POINTS, L"-");
				}

			if (rank->Rank != (unsigned)-1)
				{
				text.Format(L"%u", rank->Rank);
				list->Set_Entry_Text(index, COL_RANK, text);
				}
			else
				{
				list->Set_Entry_Text(index, COL_RANK, L"-");
				}

			break;
			}
		}
	}
