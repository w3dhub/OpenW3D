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
*     $Archive: /Commando/Code/Commando/DlgWOLLogon.h $
*
* DESCRIPTION
*     Quick match dialog
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 14 $
*     $Modtime: 1/18/02 11:44p $
*
******************************************************************************/

#ifndef __DLGWOLLOGON_H__
#define __DLGWOLLOGON_H__

#include "DlgWOLWait.h"
#include <wwui/popupdialog.h>
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>

class DlgWOLLogon;

class DlgWOLLogonEvent :
		public TypedEvent<DlgWOLLogonEvent, DlgWOLLogon>
	{
	public:
		enum Event {Cancel = 0, Login};

		Event GetEvent(void) const
			{return mEvent;}

		DlgWOLLogonEvent(Event event, DlgWOLLogon& dialog) :
				TypedEvent<DlgWOLLogonEvent, DlgWOLLogon>(dialog),
				mEvent(event)
			{}

		~DlgWOLLogonEvent()
			{}

	private:
		Event mEvent;
	};


class DlgWOLLogon :
		public PopupDialogClass,
		public Notifier<DlgWOLLogonEvent>
	{
	public:
		static bool DoDialog(const wchar_t* login, Observer<DlgWOLLogonEvent>* observer);

		void GetLogin(const wchar_t** login, const wchar_t** password, bool& encrypted);
		bool IsRememberLoginChecked(void);

	protected:
		DlgWOLLogon();
		virtual ~DlgWOLLogon();

		bool FinalizeCreate(const wchar_t* login);

		void On_Init_Dialog(void);
		void On_Command(int ctrl, int message, DWORD param);

		void UpdatePersonas(void);
		void SelectPersona(const wchar_t* name);
		void AutoComplete(const wchar_t* name);

		// Notifications
		void On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* combo, int id, int oldsel, int newsel);
		void On_ComboBoxCtrl_Edit_Change(ComboBoxCtrlClass* combo, int id);
		void On_EditCtrl_Change(EditCtrlClass* edit, int id);
		void On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id);

		DECLARE_NOTIFIER(DlgWOLLogonEvent)

	private:
		DlgWOLLogon(const DlgWOLLogon&);
		const DlgWOLLogon& operator=(const DlgWOLLogon&);

		WideStringClass mLogin;
		bool mIsPasswordEncrypted;
		};

#endif // __DLGWOLLOGON_H__
