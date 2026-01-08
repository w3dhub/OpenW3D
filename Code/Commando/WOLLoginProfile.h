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
*     $Archive: /Commando/Code/Commando/WOLLoginProfile.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 7 $
*     $Modtime: 1/19/02 2:23p $
*
******************************************************************************/

#ifndef __WOLLOGINPROFILE_H__
#define __WOLLOGINPROFILE_H__

#include <wwlib/refcount.h>
#include <wwlib/wwstring.h>
#include <wwlib/widestring.h>
#include <WWOnline/WOLLadder.h>

class LoginProfile :
		public RefCountClass
	{
	public:
		struct Ranking
			{
			unsigned int Wins;
			unsigned int Losses;
			unsigned int Deaths;
			unsigned int Kills;
			unsigned int Points;
			unsigned int Rank;
			};

		static void EnableSaving(bool);
		
		static void SetCurrent(LoginProfile*);

		// Get the profile for the specified login.
		static LoginProfile* Get(const unichar_t* loginName, bool createOK = false);
		
		// Create a login profile.
		static LoginProfile* Create(const unichar_t* loginName);

		// Delete the profile.
		static void Delete(const unichar_t* loginName);

		// Get login name
		const unichar_t* GetName(void) const;

		// Set the preferred game server
		void SetPreferredServer(const char* name);

		// Get the preferred game server
		const char* GetPreferredServer(void) const
			{return (const char*)mServer;}

		void SetLocale(WOL::Locale locale);

		WOL::Locale GetLocale(void) const
			{return mLocale;}

		// Set the side preference
		void SetSidePreference(int side);

		int GetSidePreference(void) const
			{return mSidePref;}

		void SetGamesPlayed(unsigned int);

		unsigned int GetGamesPlayed(void) const
			{return mGamesPlayed;}

		// Get cached ranking
		const Ranking* GetRanking(WWOnline::LadderType type) const;

		void SetRanking(WWOnline::LadderType type, Ranking& rank);

		// Load profile settings
		void LoadSettings(void);

		// Save profile settings
		void SaveSettings(void);

	protected:
		LoginProfile();
		virtual ~LoginProfile();

		bool FinalizeCreate(const unichar_t* loginName);

		void LoadRank(const char* valueName, Ranking& rank);
		void SaveRank(const char* valueName, const Ranking& rank);

	private:
		static bool _mSaveAllowed;
		static LoginProfile* _mCurrentProfile;

		WideStringClass mName;
		StringClass mServer;
		WOL::Locale mLocale;
		int mSidePref;
		unsigned int mGamesPlayed;

		Ranking mTeamRank;
		Ranking mClanRank;
	};

class DialogBaseClass;
void ShowProfileRanking(DialogBaseClass* dialog, const LoginProfile* profile);

#endif // __WOLLOGINPROFILE_H__
