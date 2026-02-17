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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgcncwinscreen.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 3:31p                                                  $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCWINSCREEN_H
#define __DLGCNCWINSCREEN_H


#include "menudialog.h"
#include "player.h"
#include "widestring.h"
#include "render2d.h"
#include "menubackdrop.h"


//////////////////////////////////////////////////////////////////////
//
//	CNCWinScreenMenuClass
//
//////////////////////////////////////////////////////////////////////
class CNCWinScreenMenuClass : public MenuDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CNCWinScreenMenuClass  (void);
	~CNCWinScreenMenuClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Destroy (void) override;
	void		Render (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;
	void		On_Menu_Activate (bool onoff) override;
	void		On_Frame_Update (void) override;

	//
	//	Control population control
	//
	void		Populate_Player_Lists (int team_id, int list_ctrl_id);
	void		Build_Player_Display_Name (const cPlayer *player, WideStringClass &outName);

	//
	//	Static methods
	//
	static void	Close_Dialog (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////	
	static int	ListSortCallback (const void *elem1, const void *elem2);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	RenderObjClass *						HeaderModel;
	Render2DClass							BlackoutRenderer;
	MenuBackDropClass						WinScreenBackdrop;
	float										UpdateTimer;
	bool										ShowLadderPoints;
	static CNCWinScreenMenuClass *	_TheInstance;
	static const float					UpdateIntervalS;
};


#endif //__DLGCNCWINSCREEN_H
