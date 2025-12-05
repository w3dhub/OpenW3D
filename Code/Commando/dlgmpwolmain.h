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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolmain.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 5:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_WOL_MAIN_H
#define __DLG_MP_WOL_MAIN_H

#include "menudialog.h"
#include "WOLLogonMgr.h"
#include "DlgMessageBox.h"

////////////////////////////////////////////////////////////////
//
//	MPWolMainMenuClass
//
////////////////////////////////////////////////////////////////
class MPWolMainMenuClass :
	public MenuDialogClass,
	public Observer<WOLLogonAction>,
	public Observer<DlgMsgBoxEvent>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	MPWolMainMenuClass (void);
	~MPWolMainMenuClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_Frame_Update (void) override;

	//
	//	Debug methods
	//
	void		Refresh_Dialog (void);

	//
	//	Singleton access
	//
	static void							Display (void);
	static MPWolMainMenuClass *	Get_Instance (void)	{ return _TheInstance; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	
	bool CheckWOLVersion(void);

	//
	//	Inherited
	//
	void		On_Activate(bool onoff) override;
	void		On_Last_Menu_Ending (void) override;
	
	void		Update_Login_Profile(void);
	void		Build_Persona_Combobox (void);
	void		Build_Server_Combobox (void);

	void HandleNotification(WOLLogonAction&) override;
	void HandleNotification(DlgMsgBoxEvent&) override;

	int mPendingCmd;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	static MPWolMainMenuClass *_TheInstance;
	bool		IsSidebarHelpPending;
};


#endif //__DLG_MP_WOL_MAIN_H
