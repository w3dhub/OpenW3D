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
*     $Archive: /Commando/Code/Commando/DlgPasswordPrompt.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 11/29/01 9:17p $
*
******************************************************************************/

#include <wwui/popupdialog.h>
#include <wwlib/Signaler.h>

class DlgPasswordPrompt :
		public PopupDialogClass,
		public Signaler<DlgPasswordPrompt>
	{
	public:
		static bool DoDialog(Signaler<DlgPasswordPrompt>* target);

		const unichar_t* GetPassword(void) const;

	protected:
		DlgPasswordPrompt();
		~DlgPasswordPrompt();

		// Prevent copy and assignment
		DlgPasswordPrompt(const DlgPasswordPrompt&);
		const DlgPasswordPrompt& operator=(const DlgPasswordPrompt&);

		void On_Init_Dialog(void) override;
		void On_Command(int ctrlID, int mesage, unsigned int param) override;
		void On_EditCtrl_Change(EditCtrlClass* edit, int id) override;
		void On_EditCtrl_Enter_Pressed(EditCtrlClass* edit, int id) override;
	};


