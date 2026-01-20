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
*     $Archive: /Commando/Code/Commando/WOLGameInfo.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 18 $
*     $Modtime: 10/31/02 4:30p $
*
******************************************************************************/

#include "WOLGameInfo.h"
#include "gamedata.h"
#include "cnetwork.h"
#include <WWOnline/WOLUser.h>
#include <WWOnline/WOLChannel.h>
#include <WWOnline/PingProfile.h>
#include <algorithm>
#include <wwlib/realcrc.h>
#include "modpackagemgr.h"
#include <cstdio>
#include <algorithm>

using namespace WWOnline;

/******************************************************************************
*
* NAME
*     WOLGameInfo::WOLGameInfo
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WOLGameInfo::WOLGameInfo(void) :
		mIsDataValid(false),
		mIsMapValid(true),
		mPingTime(-1)
	{
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::WOLGameInfo
*
* DESCRIPTION
*     Import information about the game.
*
* INPUTS
*     Game - Game to import information from.
*
* RESULT
*     NONE
*
******************************************************************************/

WOLGameInfo::WOLGameInfo(const cGameData& theGame)
	{
	ImportFromGame(theGame);
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::WOLGameInfo
*
* DESCRIPTION
*     Import information about the game from a channel.
*
* INPUTS
*     Channel - Channel to import game information from.
*
* RESULT
*     NONE
*
******************************************************************************/

WOLGameInfo::WOLGameInfo(const RefPtr<ChannelData>& channel)
	{
	ImportFromChannel(channel);
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::~WOLGameInfo
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

WOLGameInfo::~WOLGameInfo(void)
	{
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::Reset
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

void WOLGameInfo::Reset(void)
	{
	mIsDataValid = false;
	mIsMapValid = true;

	mVersion = 0;
	mGameType = 0;

	mMapName[0] = 0;
	mModName[0] = 0;
	mTitle[0] = 0;

	mMinPlayers = 0;
	mMaxPlayers = 0;
	mNumPlayers = 0;
		
	mClanID1 = 0;
	mClanID2 = 0;
		
	mIsMod = false;
	mIsLaddered = false;
	mIsPassworded = false;
	mIsQuickmatch = false;
	mIsDedicated = false;
	mIsFriendlyFire = false;
	mIsFreeWeapons = false;
	mIsTeamRemix = false;
	mIsTeamChange = false;
	mIsClanGame = false;
	mIsRepairBuildings = false;
	mIsDriverGunner = false;
	mSpawnWeapons = false;
		
	mPingTime = -1;
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::ImportFromGame
*
* DESCRIPTION
*     Import information about the game.
*
* INPUTS
*     Game - Game to import information from.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLGameInfo::ImportFromGame(const cGameData& theGame)
	{
	Reset();

	mIsDataValid = true;
	mIsMapValid = true;

	mVersion  = theGame.Get_Version_Number();
	mGameType = theGame.Get_Game_Type();

	strncpy(mMapName, theGame.Get_Map_Name(), MAX_TEXT_LENGTH);
	mMapName[MAX_TEXT_LENGTH - 1] = 0;
	
	strncpy(mModName, theGame.Get_Mod_Name(), MAX_TEXT_LENGTH);
	mModName[MAX_TEXT_LENGTH - 1] = 0;

	u_wstomb(mTitle, theGame.Get_Game_Title(), MAX_TEXT_LENGTH);
	mTitle[MAX_TEXT_LENGTH - 1] = 0;

	mMinPlayers = theGame.Get_Min_Players();
	mMaxPlayers = theGame.Get_Max_Players();
	mNumPlayers = theGame.Get_Current_Players();

	mClanID1 = theGame.Get_Clan(0);
	mClanID2 = theGame.Get_Clan(1);

	const unichar_t* password = theGame.Get_Password();
	mIsPassworded = ((password != NULL) && (u_strlen(password) > 0));
	mIsLaddered   = theGame.IsLaddered.Get();
	mIsQuickmatch = theGame.Is_QuickMatch_Server();
	mIsDedicated  = theGame.IsDedicated.Get();

	mIsFriendlyFire = theGame.IsFriendlyFirePermitted.Get();
	mIsFreeWeapons  = theGame.IsFreeWeapons.Get();
	mIsTeamRemix    = theGame.RemixTeams.Get();
	mIsTeamChange   = theGame.IsTeamChangingAllowed.Get();
	mIsClanGame     = theGame.IsClanGame.Get();

	// C&C mode flags
	mIsRepairBuildings = theGame.CanRepairBuildings.Get();
	mIsDriverGunner    = theGame.DriverIsAlwaysGunner.Get();
	mSpawnWeapons      = theGame.SpawnWeapons.Get();

	mIsMod = (theGame.Get_Mod_Name().Is_Empty() == false);

	mPingTime = -1;
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::ImportFromChannel
*
* DESCRIPTION
*     Import information about the game from a channel.
*
* INPUTS
*     Channel - Channel to import game information from.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLGameInfo::ImportFromChannel(const RefPtr<ChannelData>& channel)
	{
	Reset();

	if (channel.IsValid() == false)
		{
		mIsDataValid = false;
		return;
		}
					
	//ST - Test code
	//if (channel->GetName() != WideStringClass(U_CHAR("ladtest07"))) {
	//	mIsDataValid = false;
	//	return;
	//}
		

	const char* exInfo = channel->GetExtraInfo();

	if ((exInfo == NULL) || (exInfo && (strlen(exInfo) != 36)))
		{
		WWDEBUG_SAY(("WOLERROR: Channel ExInfo NULL or wrong size\n"));
		mIsDataValid = false;
		return;
		}

	// Assume the data will be valid
	mIsDataValid = true;

	// Extract ExInfo settings
	unsigned int fileCRC = 0;
	unsigned int version = 0;
	unsigned int clanID1 = 0;
	unsigned int clanID2 = 0;
	unsigned char gameType = 0;
	unsigned char gameFlags1 = 0;
	unsigned char gameFlags2 = 0;
	unsigned char modMapIndex = 0;

	int count = sscanf(exInfo, "%08lX%08lX%08lX%08lX%c%c%c%c", &version, &fileCRC,
			&clanID1, &clanID2, &gameType, &gameFlags1, &gameFlags2, &modMapIndex);

	// There should be 8 parameters in the exinfo
	if (count != 8)
		{
		WWDEBUG_SAY(("WOLERROR: Channel ExInfo malformed\n"));
		mIsDataValid = false;
		}

	if (version != unsigned(cNetwork::Get_Exe_Key())) {
		mIsDataValid = false;
		return;
	}

	mVersion  = version;
	mGameType = (cGameData::GameTypeEnum)(gameType - '0');

	if (mGameType != cGameData::GAME_TYPE_CNC)
		{
		WWDEBUG_SAY(("WOLERROR: GameType '%d' out of range\n", mGameType));
		mIsDataValid = false;
		}

	// Extract game flags
	mIsLaddered   = (channel->GetTournament() != 0);
	mIsPassworded = channel->IsPassworded();

	mIsDedicated    = ((gameFlags1 & 0x40) == 0x40);
	mIsFriendlyFire = ((gameFlags1 & 0x10) == 0x10);
	mIsFreeWeapons  = ((gameFlags1 & 0x08) == 0x08);
	mIsTeamRemix    = ((gameFlags1 & 0x04) == 0x04);
	mIsTeamChange   = ((gameFlags1 & 0x02) == 0x02);
	mIsClanGame     = ((gameFlags1 & 0x01) == 0x01);

	mIsMod             = ((gameFlags2 & 0x40) == 0x40);
	mSpawnWeapons      = ((gameFlags2 & 0x04) == 0x04);
	mIsRepairBuildings = ((gameFlags2 & 0x02) == 0x02);
	mIsDriverGunner    = ((gameFlags2 & 0x01) == 0x01);
	
	// Find the mod and map names from their CRC
	StringClass mapName(0, true);
	mapName = "<Unknown>";

	StringClass modName(0, true);

	bool got_map_name = true;
	if (mIsMod)
		{
		// Get the true index (+32 put the index into the printable range)
		modMapIndex -= 32;

		// Find mod based on its crc and get map filename based on the index.
		if (ModPackageMgrClass::Get_Mod_Map_Name_From_CRC_Index(fileCRC, modMapIndex, &modName, &mapName) == false)
			{
			//mIsDataValid = false;
			got_map_name = false;
			}
		}
	else
		{
		// Find mod based on its crc and get map filename based on the index.
		if (ModPackageMgrClass::Get_Mod_Map_Name_From_CRC (0, fileCRC, &modName, &mapName) == false)
			{
			//mIsDataValid = false;
			got_map_name = false;
			}
		}

	strncpy(mModName, modName, MAX_TEXT_LENGTH);
	mModName[MAX_TEXT_LENGTH - 1] = 0;

	strncpy(mMapName, mapName, MAX_TEXT_LENGTH);
	mMapName[MAX_TEXT_LENGTH - 1] = 0;

	mMinPlayers = channel->GetMinUsers();
	mMaxPlayers = channel->GetMaxUsers();
	mNumPlayers = channel->GetCurrentUsers();

	// Do not count the host as a player for dedicated servers.
	if (mIsDedicated)
		{
		WWASSERT(mNumPlayers > 0);
		--mNumPlayers;

		WWASSERT(mMaxPlayers > 0);
		--mMaxPlayers;
		}

	mClanID1 = clanID1;
	mClanID2 = clanID2;

	const char* topic = channel->GetTopic();

	if (topic == NULL)
		{
		WWDEBUG_SAY(("WOLERROR: Channel Topic is NULL\n"));
		mIsDataValid = false;
		}

	// Quickmatch settings are marked by a pipe '|' character
	mIsQuickmatch = (strchr(topic, '|') != NULL);

	// Extract game title from topic
	unsigned int titleLength = (topic[0] - 0x20);
	WWASSERT(titleLength <= 32);
	titleLength = std::min<unsigned int>(titleLength, 32);
	++topic;

	strncpy(mTitle, topic, titleLength);
	mTitle[titleLength] = 0;
	WWASSERT(strlen(mTitle) == titleLength);
	topic += titleLength;

	unsigned int mapLength = (topic[0] - 0x20);
	WWASSERT(mapLength <= 16);
	mapLength = std::min<unsigned int>(mapLength, 16);
	++topic;

	if (!got_map_name) {
		strncpy(mMapName, topic, mapLength);
		mMapName[mapLength] = 0;
		WWASSERT(strlen(mMapName) == mapLength);
		mIsMapValid = false;
	}
	topic += mapLength;

	// Calculate ping
	PingProfile pings;
	DecodePingProfile(topic, pings);

	const PingProfile& localPings = GetLocalPingProfile();
	mPingTime = ComparePingProfile(localPings, pings);
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::ExportToChannel
*
* DESCRIPTION
*     Export game information into a channel.
*
* INPUTS
*     Channel - Channel to export game information into.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLGameInfo::ExportToChannel(const RefPtr<ChannelData>& channel)
	{
	if (channel.IsValid() && mIsDataValid)
		{
		// If this is a dedicated server then the max players must be incremented
		// by one to account for the host.
		unsigned int maxPlayers = (mIsDedicated == true) ? (mMaxPlayers + 1) : mMaxPlayers;

		// Set the channels min and max players
		channel->SetMinMaxUsers(mMinPlayers, maxPlayers);

		// Set the tournament type
		unsigned int tournamentType = (mIsLaddered ? 1 : 0);
		channel->SetTournament(tournamentType);

		//-------------------------------------------------------------------------
		// Encode ExInfo
		//-------------------------------------------------------------------------

		// For flags bit 6 must be set to keep the overall value at least a space character
		unsigned char gameFlags1 = 0x20;
		gameFlags1 |= mIsDedicated ? 0x40 : 0x00;
		gameFlags1 |= mIsFriendlyFire ? 0x10 : 0x00;
		gameFlags1 |= mIsFreeWeapons ? 0x08 : 0x00;
		gameFlags1 |= mIsTeamRemix ? 0x04 : 0x00;
		gameFlags1 |= mIsTeamChange ? 0x02 : 0x00;
		gameFlags1 |= mIsClanGame ? 0x01 : 0x00;

		unsigned char gameFlags2 = 0x20;
		gameFlags2 |= mIsMod ? 0x40 : 0x00;
		gameFlags2 |= mSpawnWeapons ? 0x04: 0x00;
		gameFlags2 |= mIsRepairBuildings ? 0x02: 0x00;
		gameFlags2 |= mIsDriverGunner ? 0x01: 0x00;

		// The file CRC is either the map name or the mod name depending on if
		// the mod flag is set.
		unsigned int fileCRC = 0;
		unsigned char modMapIndex = 0;

		if (mIsMod)
			{
			ModPackageClass& package = ModPackageMgrClass::Get_Current_Package();
			fileCRC = package.Get_CRC();
			modMapIndex = package.Get_Map_Index(mMapName);
			}
		else
			{
			fileCRC = ::CRC_Stringi(mMapName);
			}

		// Put the map index into the printable ASCII range
		modMapIndex += 32;

		// WARNING: The channels ExInfo field has a maximum size of 40 bytes.
		char exInfo[41];
		sprintf(exInfo, "%08lX%08lX%08lX%08lX%c%c%c%c", mVersion, fileCRC,
				mClanID1, mClanID2, (0x30 + mGameType), gameFlags1, gameFlags2, modMapIndex);

		channel->SetExtraInfo(exInfo);

		//-------------------------------------------------------------------------
		// Encode topic
		//-------------------------------------------------------------------------
		const size_t titleLength = std::min(::strlen(mTitle), static_cast<size_t>(32));
		const unsigned int encodedTitleLength = static_cast<unsigned int>(titleLength + 0x20);

		// WARNING: The channels topic field has a maximum size of 80 bytes.
		// Ping profile and quickmatch settings are encoded in the topic in
		// addition to the the game title. The combined maximum of ALL these
		// entries MUST NEVER exceed 80 bytes.
		char topic[81];
		sprintf(topic, "%c%.32s", encodedTitleLength, mTitle);
		
		//
		// Only using 61 max right now. Room for a map name maybe? ST - 10/31/2002 2:55PM
		//
		const size_t mapLength = std::min(::strlen(mMapName), static_cast<size_t>(16));
		const unsigned int encodedMapLength = static_cast<unsigned int>(mapLength + 0x20);

		// WARNING: The channels topic field has a maximum size of 80 bytes.
		// Ping profile and quickmatch settings are encoded in the topic in
		// addition to the the game title. The combined maximum of ALL these
		// entries MUST NEVER exceed 80 bytes.
		char mapinfo[81];
		sprintf(mapinfo, "%c%.16s", encodedMapLength, mMapName);
		strcat(topic, mapinfo);

		// Add our ping profile
		const PingProfile& localPings = GetLocalPingProfile();
		EncodePingProfile(localPings, topic);

		channel->SetTopic(topic);
		}
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

bool WOLGameInfo::IsValidGameChannel(const RefPtr<ChannelData>& channel)
	{
	WOLGameInfo gameInfo(channel);
	return (gameInfo.IsDataValid() && (gameInfo.mVersion == (unsigned int)cNetwork::Get_Exe_Key()));
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::IsClanCompeting
*
* DESCRIPTION
*     Check if the specified clan is competing in the game.
*
* INPUTS
*     Clan - ID of clan to check.
*
* RESULT
*     True if clan is competing.
*
******************************************************************************/

bool WOLGameInfo::IsClanCompeting(unsigned int clanID) const
	{
	return (mIsClanGame && (clanID != 0) && ((clanID == mClanID1) || (clanID == mClanID2)));
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::IsClanGameOpen
*
* DESCRIPTION
*     Check if the clan game is open for new clans to join.
*
* INPUTS
*     NONE
*
* RESULT
*     True if the game is open to clans.
*
******************************************************************************/

bool WOLGameInfo::IsClanGameOpen(void) const
	{
	return (mIsClanGame && ((0 == mClanID1) || (0 == mClanID2)));
	}


/******************************************************************************
*
* NAME
*     WOLGameInfo::CanUserJoin
*
* DESCRIPTION
*     Check if the user is able to join this game.
*
* INPUTS
*     User
*
* RESULT
*     True if user can join this game.
*
******************************************************************************/

bool WOLGameInfo::CanUserJoin(const RefPtr<UserData>& user)
	{
	if (!user.IsValid() || !mIsDataValid || !mIsMapValid)
		{
		return false;
		}

	if (mVersion != (unsigned int)cNetwork::Get_Exe_Key())
		{
		return false;
		}

	if (mIsClanGame)
		{
		unsigned int userClanID = user->GetSquadID();
		return ((0 != userClanID) && (IsClanGameOpen() || IsClanCompeting(userClanID)));
		}

	return true;
	}