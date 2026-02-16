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
*     $Archive: /Commando/Code/WWOnline/WOLGame.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 10 $
*     $Modtime: 8/21/02 12:30p $
*
******************************************************************************/

#include "always.h"
#include "WOLGame.h"
#include "WOLSession.h"
#include "WOLChannel.h"
#include "WOLString.h"
#include "WOLErrorUtil.h"

namespace WWOnline {

/******************************************************************************
*
* NAME
*     GameStartEvent::GameStartEvent
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Channel  - Channel game is in.
*     UserList - List of users in game.
*     GameID   - Game ID for this game.
*
* RESULT
*     NONE
*
******************************************************************************/

GameStartEvent::GameStartEvent(const RefPtr<ChannelData>& channel, const UserList& users, unsigned int gameID) :
		mResult(S_OK),
		mChannel(channel),
		mGameID(gameID)
	{
	for (unsigned int index = 0; index < users.size(); index++)
		{
		mPlayers.push_back(users[index]);
		}
	}


/******************************************************************************
*
* NAME
*     GameStartEvent::GetErrorDescription
*
* DESCRIPTION
*     Get text description of error condition.
*
* INPUTS
*     NONE
*
* RESULT
*     ErrorText - Error text description.
*
******************************************************************************/

const char* GameStartEvent::GetErrorDescription(void) const
	{
	return GetChatErrorString(mResult);
	}


/******************************************************************************
*
* NAME
*     GameStartWait::Create
*
* DESCRIPTION
*     Create a game start wait condition.
*
* INPUTS
*     Players - Players in game.
*
* RESULT
*     Wait - Wait condition to process for game start.
*
******************************************************************************/

RefPtr<GameStartWait> GameStartWait::Create(const UserList& players, void(*timeout_callback)(void))
	{
	return new GameStartWait(players, timeout_callback);
	}


/******************************************************************************
*
* NAME
*     GameStartWait::GameStartWait
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     Players - 
*
* RESULT
*     NONE
*
******************************************************************************/

GameStartWait::GameStartWait(const UserList& players, void(*timeout_callback)(void)) :
		SingleWait(WOLSTRING("WOL_GAMEIDFETCH")),
		mPlayers(players)
	{
	mTimeoutCallback = timeout_callback;
	WWDEBUG_SAY(("WOL: GameStartWait Create\n"));
	}


/******************************************************************************
*
* NAME
*     GameStartWait::~GameStartWait
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

GameStartWait::~GameStartWait()
	{
	WWDEBUG_SAY(("WOL: GameStartWait End %S\n", mEndText));
	}


/******************************************************************************
*
* NAME
*     GameStartWait::WaitBeginning
*
* DESCRIPTION
*     Begin game start wait condition.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void GameStartWait::WaitBeginning(void)
	{
	WWDEBUG_SAY(("WOL: GameStartWait Begin\n"));

	RefPtr<Session> session = Session::GetInstance(false);

	if (!session.IsValid())
		{
		WWDEBUG_SAY(("WOLERROR: WOLSession not initialized\n"));
		EndWait(Error, WOLSTRING("WOL_NOTINITIALIZED"));
		}

	Observer<GameStartEvent>::NotifyMe(*session);

	HRESULT hr = session->GetChatObject()->RequestGameStart(mPlayers);

	if (FAILED(hr))
		{
		WWDEBUG_SAY(("WOLERROR: GameStartWait %s\n", GetChatErrorString(hr)));
		EndWait(Error, WOLSTRING("WOL_GAMEIDERROR"));
		}
	}


/******************************************************************************
*
* NAME
*     GameStartWait::HandleNotification(GameStartEvent)
*
* DESCRIPTION
*     Handle game start.
*
* INPUTS
*     Event - Game start event.
*
* RESULT
*     NONE
*
******************************************************************************/

void GameStartWait::HandleNotification(GameStartEvent& start)
	{
	if (mEndResult == Waiting)
		{
		if (start.IsSuccess())
			{
			EndWait(ConditionMet, WOLSTRING("WOL_GAMEIDRECEIVED"));
			}
		else
			{
			const char* text = start.GetErrorDescription();
			WWDEBUG_SAY(("WOLERROR: GameStartWait %s\n", text));
			EndWait(Error, WOLSTRING(text));
			}
		}
	}



//
// Override base class to check for timeout
//
void GameStartWait::EndWait(WaitResult result, const unichar_t* endText)
	{
	WWDEBUG_SAY(("GameStartWait::EndWait\n"));

	if (result == TimeOut && mTimeoutCallback) {
		mTimeoutCallback();
	}	

	SingleWait::EndWait(result, endText);
}




} // namespace WWOnline