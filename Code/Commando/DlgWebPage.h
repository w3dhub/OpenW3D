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
*     $Archive: /Commando/Code/Commando/DlgWebPage.h $
*
* DESCRIPTION
*     Web Browser dialog
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 10/30/01 4:35p $
*
******************************************************************************/

#ifndef __DLGWEBPAGE_H__
#define __DLGWEBPAGE_H__

#include "DialogBase.h"
#include <atlbase.h>
#include <WWLib\Notify.h>

class WebBrowser;
class WebEvent;
class DlgMsgBoxEvent;

class DlgWebPage :
		public DialogBaseClass,
		public Observer<WebEvent>,
		public Observer<DlgMsgBoxEvent>
	{
	public:
		static void DoDialog(const char* page);

	protected:
		DlgWebPage();
		virtual ~DlgWebPage();

		bool FinalizeCreate(void);

		// Handle web browser events
		void HandleNotification(WebEvent& event);

		// Handle message box dialog events
		void HandleNotification(DlgMsgBoxEvent& event);

	// DialogBassClass methods
	protected:
		void Start_Dialog(void);
		void End_Dialog(void);
		void On_Frame_Update(void);

	private:
		// Declare private to prevent copy and assignment
		DlgWebPage(const DlgWebPage&);
		const DlgWebPage& operator=(const DlgWebPage&);

	private:
		WebBrowser* mBrowser;
		const char* mPage;
		unsigned long mTimer;
	};

#endif // __DLGWEBPAGE_H__
