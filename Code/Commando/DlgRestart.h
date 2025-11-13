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
*     $Archive: /Commando/Code/Commando/DlgRestart.h $
*
* DESCRIPTION
*     Dialog to notify the user that the game requires a restart.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 1 $
*     $Modtime: 10/15/01 3:12p $
*
******************************************************************************/

#ifndef __DLGRESTART_H__
#define __DLGRESTART_H__

#include <wwui/popupdialog.h>

class DlgRestart :
		public PopupDialogClass
	{
	public:
		static bool DoDialog(void);

	protected:
		DlgRestart();
		virtual ~DlgRestart();

		void On_Init_Dialog(void) override;
		void On_Command(int ctrl, int message, unsigned int param) override;

	private:
		// Prevent copy and assignment
		DlgRestart(const DlgRestart&);
		const DlgRestart& operator=(const DlgRestart&);
	};

#endif // __DLGRESTART_H__
