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
 *                     $Archive:: /Commando/Code/wwui/menudialog.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/25/01 5:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MENU_DIALOG_H
#define __MENU_DIALOG_H


#include "dialogbase.h"
#include "dialogspec.h"
#include "wwstring.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class MenuBackDropClass;


////////////////////////////////////////////////////////////////
//
//	MenuDialogClass
//
////////////////////////////////////////////////////////////////
class MenuDialogClass : public DialogBaseClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MenuDialogClass (const DialogResource *dialog_resource);
	virtual ~MenuDialogClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void						Initialize (void);
	static void						Shutdown (void);
	static void						Ensure_BackDrop (void);

	//
	//	RTTI
	//
	virtual MenuDialogClass *	As_MenuDialogClass (void) override	{ return this; }

	//
	//	Configuration methods
	//
	static MenuBackDropClass *	Get_BackDrop (void)		{ Ensure_BackDrop(); return BackDrop; }
	static MenuBackDropClass *	Replace_BackDrop (MenuBackDropClass *backdrop);

	//
	//	Informational
	//
	bool						Is_Active_Menu (void) const	{ return (ActiveMenu == this); }

	//
	//	From DialogBaseClass
	//
	virtual void			Start_Dialog (void) override;
	virtual void			Render (void) override;
	virtual void			End_Dialog (void) override;

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogBaseClass
	//
	virtual void			On_Activate (bool onoff) override;

	//
	//	New notifications
	//
	virtual void			On_Menu_Activate (bool onoff);
	virtual void			On_Last_Menu_Ending (void) {}


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////	
	static MenuDialogClass *								ActiveMenu;
	static MenuBackDropClass *								BackDrop;
	static DynamicVectorClass<MenuDialogClass *>		MenuStack;
};


#endif //__MENU_DIALOG_H
