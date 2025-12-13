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
 *                     $Archive:: /Commando/Code/commando/dlghelpscreen.h      $*
 *                                                                                             *
 *                       Author:: Byon Garrabrant                                              *
 *                                                                                             *
 *                     $Modtime:: 11/20/01 7:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGHELPSCREEN_H
#define __DLGHELPSCREEN_H


#include "menudialog.h"


////////////////////////////////////////////////////////////////
//
//	EVAEncyclopediaMenuClass
//
////////////////////////////////////////////////////////////////
class HelpScreenDialogClass : public MenuDialogClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	HelpScreenDialogClass (void);
	~HelpScreenDialogClass (void);


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_Menu_Activate (bool onoff) override;
	void		On_Destroy (void) override;
	bool		On_Key_Down (uint32 key_id, uint32 key_data) override;

	//
	//	Singleton access
	//
	static void									Display (void);

private:
	MenuBackDropClass *	OldBackdrop;
	
};


#endif //__DLGHELPSCREEN_H

