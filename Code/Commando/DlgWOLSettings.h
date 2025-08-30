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
*     $Archive: /Commando/Code/Commando/DlgWOLSettings.h $
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
*     $Modtime: 1/19/02 2:29p $
*
******************************************************************************/

#ifndef __DLGWOLSETTINGS_H__
#define __DLGWOLSETTINGS_H__

#include "DlgWOLWait.h"
#include <wwui/menudialog.h>
#include <wwlib/Notify.h>
#include <WWOnline/RefPtr.h>
#include <WWOnline/WOLSession.h>

class LoginProfile;

class DlgWOLSettings :
		public MenuDialogClass,
		public Observer<DlgWOLWaitEvent>
	{
	public:
		static bool DoDialog(void);

	protected:
		DlgWOLSettings();
		virtual ~DlgWOLSettings();

		bool FinalizeCreate(void);

		void On_Init_Dialog(void) override;
		void On_Destroy(void) override;
		void On_Command(int ctrl, int message, DWORD param) override;

		bool SaveSettings(void);

		void InitPersonaCombo(void);
		void ClearPersonaCombo(void);
		void DeleteSelectedPersona(void);
		void UpdateForPersona(void);

		void InitServersCombo(const WWOnline::IRCServerList& servers);
		void SetServerCombo(const char* serverName);

		void InitSideCombo(void);
		void SetSideCombo(int side);

		void InitLocaleCombo(void);
		void SetLocaleCombo(WOL::Locale locale);

		void InitConnectionSpeedCombo(void);

		void On_ComboBoxCtrl_Sel_Change(ComboBoxCtrlClass* combo, int ctrl, int oldSel, int newSel) override;

		void HandleNotification(DlgWOLWaitEvent&) override;

		LoginProfile* GetLoginProfile(void);

	private:
		DlgWOLSettings(const DlgWOLSettings&);
		const DlgWOLSettings& operator=(const DlgWOLSettings&);

		RefPtr<WWOnline::Session> mWOLSession;

		bool DetectingBandwidth;
		bool WaitingToExitDialog;
	};

#endif // __DLGWOLSETTINGS_H__