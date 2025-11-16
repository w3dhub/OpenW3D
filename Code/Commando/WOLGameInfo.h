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
*     $Archive: /Commando/Code/Commando/WOLGameInfo.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 11 $
*     $Modtime: 10/31/02 3:07p $
*
******************************************************************************/

#ifndef __WOLGAMEINFO_H__
#define __WOLGAMEINFO_H__

#include <WWOnline/RefPtr.h>

class cGameData;

namespace WWOnline
{
class ChannelData;
class UserData;
}

// Game information used to describe the type of game.
class WOLGameInfo
	{
	public:
		static bool IsValidGameChannel(const RefPtr<WWOnline::ChannelData>& channel);

		enum {MAX_TEXT_LENGTH = 32};

		WOLGameInfo(void);
		
		WOLGameInfo(const cGameData& theGame);
		
		WOLGameInfo(const RefPtr<WWOnline::ChannelData>& channel);
		
		~WOLGameInfo(void);

		void Reset(void);

		bool IsDataValid(void) const
			{return mIsDataValid;}

		bool IsMapValid(void) const
			{return mIsMapValid;}
		
		void ImportFromGame(const cGameData& theGame);

		void ImportFromChannel(const RefPtr<WWOnline::ChannelData>& channel);

		void ExportToChannel(const RefPtr<WWOnline::ChannelData>& channel);

		unsigned int Version(void) const
			{return mVersion;};

		unsigned int GameType(void) const
			{return mGameType;}

		const char* MapName(void) const
			{return mMapName;}

		const char* ModName(void) const
			{return mModName;}

		const char* Title(void) const
			{return mTitle;}

		unsigned int MinPlayers(void) const
			{return mMinPlayers;}

		unsigned int MaxPlayers(void) const
			{return mMaxPlayers;}

		unsigned int NumPlayers(void) const
			{return mNumPlayers;}
		
		unsigned int ClanID1(void) const
			{return mClanID1;}

		unsigned int ClanID2(void) const
			{return mClanID2;}
		
		bool IsLaddered(void) const
			{return mIsLaddered;}
		
		bool IsPassworded(void) const
			{return mIsPassworded;}
		
		bool IsQuickmatch(void) const
			{return mIsQuickmatch;}
		
		bool IsDedicated(void) const
			{return mIsDedicated;}
		
		bool IsFriendlyFire(void) const
			{return mIsFriendlyFire;}
		
		bool IsFreeWeapons(void) const
			{return mIsFreeWeapons;}
		
		bool IsTeamRemix(void) const
			{return mIsTeamRemix;}
		
		bool IsTeamChange(void) const
			{return mIsTeamChange;}
		
		bool IsClanGame(void) const
			{return mIsClanGame;}
		
		bool IsRepairBuildings(void) const
			{return mIsRepairBuildings;}
		
		bool IsDriverGunner(void) const
			{return mIsDriverGunner;}

		bool IsSpawnWeapons(void) const
			{return mSpawnWeapons;}

		int PingTime(void) const
			{return mPingTime;}

		// Is the clan competing in the game.		
		bool IsClanCompeting(unsigned int clanID) const;

		bool IsClanGameOpen(void) const;

		bool CanUserJoin(const RefPtr<WWOnline::UserData>& user);

	protected:
		// Prevent copy and assignment
		WOLGameInfo(const WOLGameInfo&);
		const WOLGameInfo& operator=(const WOLGameInfo&);

	protected:
		bool mIsDataValid;
		bool mIsMapValid;

		unsigned int mVersion;
		unsigned int  mGameType;

		char mMapName[MAX_TEXT_LENGTH];
		char mModName[MAX_TEXT_LENGTH];
		char mTitle[MAX_TEXT_LENGTH];

		unsigned int mMinPlayers;
		unsigned int mMaxPlayers;
		unsigned int mNumPlayers;
		
		unsigned int mClanID1;
		unsigned int mClanID2;
		
		bool mIsMod;
		bool mIsLaddered;
		bool mIsPassworded;
		bool mIsQuickmatch;
		bool mIsDedicated;
		bool mIsFriendlyFire;
		bool mIsFreeWeapons;
		bool mIsTeamRemix;
		bool mIsTeamChange;
		bool mIsClanGame;
		bool mIsRepairBuildings;
		bool mIsDriverGunner;
		bool mSpawnWeapons;
		
		int mPingTime;
	};

#endif // __WOLGAMEINFO_H__