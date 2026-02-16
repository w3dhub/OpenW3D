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
*     $Archive: /Commando/Code/Commando/DlgMPConnectionRefused.h $
*
* DESCRIPTION
*
******************************************************************************/

#ifndef __DLGMPCONNECTIONREFUSED_H__
#define __DLGMPCONNECTIONREFUSED_H__

#include <wwui/popupdialog.h>

class DlgMPConnectionRefused :
		public PopupDialogClass
	{
	public:
		static bool DoDialog(const unichar_t * text, bool show_splash_screen);
		void On_Init_Dialog(void) override;

	protected:
		DlgMPConnectionRefused(const unichar_t * text, bool show_splash_screen);
		virtual ~DlgMPConnectionRefused();

		void On_Command(int ctrl, int message, unsigned int param) override;

	private:
		// Prevent copy and assignment
		DlgMPConnectionRefused(const DlgMPConnectionRefused&);
		const DlgMPConnectionRefused& operator=(const DlgMPConnectionRefused&);

		WideStringClass	Text;
		bool					ShowSplashScreen;
	};

#endif // __DLGMPCONNECTIONREFUSED_H__
