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
*     $Archive: /Commando/Code/Commando/WOLJoinGame.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 10 $
*     $Modtime: 1/16/02 4:25p $
*
******************************************************************************/

#ifndef __WOLJOINGAME_H__
#define __WOLJOINGAME_H__

#include <wwlib/refcount.h>
#include <wwlib/Notify.h>
#include <wwlib/Signaler.h>
#include <wwlib/widestring.h>
#include <WWOnline/RefPtr.h>
#include "DlgMPTeamSelect.h"

namespace WWOnline
{
class Session;
class ChannelData;
class ChannelEvent;
}

class cGameData;
class DlgWOLWaitEvent;

class WOLJoinGame :
		public RefCountClass,
		public Observer<WWOnline::ChannelEvent>,
		public Observer<DlgWOLWaitEvent>,
		public Signaler<MPChooseTeamSignal>
	{
	public:
		static void JoinTheGame(const unichar_t* gameName, const unichar_t* password, bool allowTeamSelect);

	protected:
		WOLJoinGame();
		~WOLJoinGame();

		// Prevent copy and assignment
		WOLJoinGame(const WOLJoinGame&);
		const WOLJoinGame& operator=(const WOLJoinGame&);

		bool FinalizeCreate(void);

		cGameData* CreateGameFromChannel(const RefPtr<WWOnline::ChannelData>& channel);
		
		bool Join(const unichar_t* gameName, const unichar_t* password, bool allowTeamSelect);
		void ProceedWithConnection(int teamChoice);
		void Leave(void);

		bool StartServerConnect(void);
		void ConnectToServer(void);

		void HandleNotification(WWOnline::ChannelEvent&) override;
		void HandleNotification(DlgWOLWaitEvent&) override;
		void ReceiveSignal(MPChooseTeamSignal&) override;

	private:
		RefPtr<WWOnline::Session> mWOLSession;
		
		typedef enum {IDLE_STATE = 0, JOINING_STATE, FIREWALL_STATE} JoinState;
		JoinState mJoinState;

		bool mAllowTeamSelect;

		WideStringClass mGameName;
		WideStringClass mPassword;

		RefPtr<WWOnline::ChannelData> mTheChannel;

		int mTeamChoice;
		unsigned int mClanID;
	};

#endif //__WOLJOINGAME_H__
