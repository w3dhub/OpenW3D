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
 *                     $Archive:: /Commando/Code/Commando/dlgmainmenu.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/06/01 4:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MAIN_MENU_H
#define __DLG_MAIN_MENU_H


#include "menudialog.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class RenderObjClass;


////////////////////////////////////////////////////////////////
//
//	MainMenuDialogClass
//
////////////////////////////////////////////////////////////////
class MainMenuDialogClass : public MenuDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MainMenuDialogClass (void);
	~MainMenuDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Notifications
	//
	void			On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

	//
	//	Transition support
	//
	DialogTransitionClass *	Get_Transition_In (DialogBaseClass *prev_dlg) override;
	DialogTransitionClass *	Get_Transition_Out (DialogBaseClass *next_dlg) override;

	//
	//	Singleton access
	//
	static void							Display (void);
	static MainMenuDialogClass *	Get_Instance (void)	{ return _TheInstance; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void			On_Init_Dialog (void) override;
	void			On_Menu_Activate (bool onoff) override;
	void			Update_Version_Number (void);
	StringClass	Choose_Skirmish_Map (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	RenderObjClass *	LogoModel;
	RenderObjClass *	TitleTransModel;
	RenderObjClass *	GizmoModel;
	bool					IsStartingPractice;

	static bool			Animated;

	static MainMenuDialogClass *	_TheInstance;
};


#endif //__DLG_MAIN_MENU_H
