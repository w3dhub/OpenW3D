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
*     $Archive: /Commando/Code/Commando/DlgQuickmatch.h $
*
* DESCRIPTION
*     Quick match dialog
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 8 $
*     $Modtime: 8/23/01 8:30a $
*
******************************************************************************/

#ifndef __DLGQUICKMATCH_H__
#define __DLGQUICKMATCH_H__

#include "WOLQuickMatch.h"
#include <wwui/menudialog.h>
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>

class WaitCondition;
class DlgMsgBoxEvent;

class DlgQuickMatch :
		public MenuDialogClass,
		public Observer<QuickMatchEvent>,
		public Observer<DlgMsgBoxEvent>
	{
	public:
		static bool DoDialog(void);

	protected:
		DlgQuickMatch();
		virtual ~DlgQuickMatch();

		bool FinalizeCreate(void);

		void On_Init_Dialog(void) override;
		void On_Frame_Update(void) override;
		void On_Command(int ctrl, int message, unsigned int param) override;

		void Connect(void);
		void SendMatchingInfo(void);

		void OutputMessage(int messageID);
		void OutputMessage(const unichar_t* message);

		void HandleNotification(QuickMatchEvent&) override;
		void HandleNotification(DlgMsgBoxEvent&) override;

	private:
		DlgQuickMatch(const DlgQuickMatch&);
		const DlgQuickMatch& operator=(const DlgQuickMatch&);

		WOLQuickMatch* mQuickMatch;
		RefPtr<WaitCondition> mConnectWait;
		unsigned int mTimeoutTime;
		unsigned int mResendTime;
		};

#endif // __DLGQUICKMATCH_H__
