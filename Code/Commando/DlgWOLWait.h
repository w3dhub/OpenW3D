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
*     $Archive: /Commando/Code/Commando/DlgWOLWait.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Bhayes $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 2/18/02 7:51p $
*
******************************************************************************/

#ifndef __DLGWOLWAIT_H__
#define __DLGWOLWAIT_H__

#include <wwui/popupdialog.h>
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>
#include <WWOnline/WaitCondition.h>

namespace WWOnline
{
class Session;
}

class DlgWOLWait;

class DlgWOLWaitEvent :
		public TypedEventPtr<DlgWOLWaitEvent, WaitCondition>
	{
	public:
		typedef WaitCondition::WaitResult WaitResult;

		inline WaitResult Result(void) const
			{return mResult;}

		DlgWOLWaitEvent(WaitResult result, WaitCondition* wait) :
				TypedEventPtr<DlgWOLWaitEvent, WaitCondition>(wait),
				mResult(result)
			{}

	protected:
		// Prevent copy and assignment
		DlgWOLWaitEvent(const DlgWOLWaitEvent&);
		const DlgWOLWaitEvent& operator=(const DlgWOLWaitEvent&);

	private:
		WaitResult mResult;
	};


class DlgWOLWait :
		public PopupDialogClass,
		public Notifier<DlgWOLWaitEvent>
	{
	public:
		enum {SHOW_NEVER = 0xFFFFFFFF};

		static bool DoDialog(const WCHAR* title, const WCHAR* button_text, RefPtr<WaitCondition>& wait,
				Observer<DlgWOLWaitEvent>* observer = NULL, unsigned long timeout = 0, unsigned long dialog_timeout = 0);

		static bool DoDialog(const WCHAR* title, RefPtr<WaitCondition>& wait,
				Observer<DlgWOLWaitEvent>* observer = NULL, unsigned long timeout = 0, unsigned long dialog_timeout = 0);

		static bool DoDialog(int titleID, RefPtr<WaitCondition>& wait,
				Observer<DlgWOLWaitEvent>* observer = NULL, unsigned long timeout = 0, unsigned long dialog_timeout = 0);

		const RefPtr<WaitCondition>& GetWait(void)
			{return mWait;}

		static DlgWOLWait *Get_Instance (void)	{ return mTheInstance; }

	protected:
		DlgWOLWait(RefPtr<WaitCondition>& wait, unsigned long timeout, unsigned long dialog_timeout = 0);
		~DlgWOLWait();

		// Prevent copy and assignment
		DlgWOLWait(const DlgWOLWait&);
		const DlgWOLWait& operator=(const DlgWOLWait&);

		void CheckCondition(void);

		void On_Init_Dialog(void);
		void On_Destroy(void);
		void On_Periodic(void);
		void On_Command(int ctrl, int message, DWORD param);
		void Render(void);

		DECLARE_NOTIFIER(DlgWOLWaitEvent)

	private:
		RefPtr<WaitCondition> mWait;
		RefPtr<WWOnline::Session> mWOLSession;
		unsigned mStartTime;
		unsigned long mTimeout;
		unsigned long mDialogTimeout;
		bool mShowDialog;
		static DlgWOLWait *	mTheInstance;
	};

#endif // __DLGWOLLOGON_H__
