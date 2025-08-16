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
* NAME
*     $Archive: /Commando/Code/Commando/DlgMPConnect.h $
*
* DESCRIPTION
*     Dialog to inform user that we are connecting to a game host.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Steve_t $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 2/14/02 2:03p $
*
******************************************************************************/

#ifndef __DLGMPCONNECT_H__
#define __DLGMPCONNECT_H__

#include <wwui/PopupDialog.h>

class cGameData;

class DlgMPConnect :
		public PopupDialogClass
	{
	public:
		// Display connecting dialog.
		//   TeamChoice - Team preference of connecting player
		//   ClanID - ID of players clan (0 if not a clan game)
		static bool DoDialog(int teamChoice, unsigned long clanID);

		void Connected(cGameData* theGame);
		void Failed_To_Connect(void);


	protected:
		DlgMPConnect(int teamChoice, unsigned long clanID);
		virtual ~DlgMPConnect();

		void On_Command(int ctrl, int message, DWORD param);
		void On_Periodic(void);

	private:
		// Prevent copy and assignment
		DlgMPConnect(const DlgMPConnect&);
		const DlgMPConnect& operator=(const DlgMPConnect&);

		int mTeamChoice;
		unsigned long mClanID;

		cGameData* mTheGame;
		bool mFailed;
	};

#endif // __DLGMPCONNECT_H__