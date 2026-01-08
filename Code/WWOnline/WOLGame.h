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
*     $Archive: /Commando/Code/WWOnline/WOLGame.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 8/21/02 12:31p $
*
******************************************************************************/

#ifndef __WOLGAME_H__
#define __WOLGAME_H__

#include "RefPtr.h"
#include "WOLUser.h"
#include "WaitCondition.h"
#include <wwlib/Notify.h>

namespace WWOnline {

class ChannelData;

class GameStartEvent
	{
	public:
		const RefPtr<ChannelData>& GetChannel(void) const
			{return mChannel;}

		const UserList& GetPlayers(void) const
			{return mPlayers;}

		const unsigned int GetGameID(void) const
			{return mGameID;}

		bool IsSuccess(void) const
			{return SUCCEEDED(mResult);}

		const char* GetErrorDescription(void) const;

		GameStartEvent(HRESULT result) :
				mResult(result),
				mGameID(0)
			{}

		GameStartEvent(const RefPtr<ChannelData>& channel, const UserList& users, unsigned int gameID);
		~GameStartEvent()
			{}

	protected:
		GameStartEvent(const GameStartEvent&);
		const GameStartEvent operator=(const GameStartEvent&);

	private:
		HRESULT mResult;
		RefPtr<ChannelData> mChannel;
		UserList mPlayers;
		unsigned int mGameID;
	};


class GameStartWait :
		public SingleWait,
		public Observer<GameStartEvent>
	{
	public:
		static RefPtr<GameStartWait> Create(const UserList& players, void(*callback)(void) = NULL);

		void WaitBeginning(void) override;

	protected:
		GameStartWait(const UserList& players, void(*callback)(void) = NULL);
		virtual ~GameStartWait();

		// Prevent copy and assignment
		GameStartWait(const GameStartWait&);
		const GameStartWait& operator=(const GameStartWait&);

		void HandleNotification(GameStartEvent&) override;
		void EndWait(WaitResult result, const unichar_t* endText) override;

		void (*mTimeoutCallback)(void);
		NativeWOLUserList mPlayers;
	};

}

#endif // __WOLGAME_H__