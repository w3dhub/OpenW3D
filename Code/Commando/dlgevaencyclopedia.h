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
 *                     $Archive:: /Commando/Code/commando/dlgevaencyclopedia.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/01/01 11:31a                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGEVAENCYCLOPEDIA_H
#define __DLGEVAENCYCLOPEDIA_H

#include "menudialog.h"
#include "DlgMessageBox.h"


////////////////////////////////////////////////////////////////
//
//	EVAEncyclopediaMenuClass
//
////////////////////////////////////////////////////////////////
class EVAEncyclopediaMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////	
	typedef enum
	{
		TAB_NONE				= -1,
		TAB_OBJECTIVES		= 0,
		TAB_MAP,
		TAB_DATA,
		TAB_CHARACTERS,
		TAB_WEAPONS,
		TAB_VEHICLES,
		TAB_BUILDINGS
	} TAB_ID;
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	EVAEncyclopediaMenuClass (void);
	~EVAEncyclopediaMenuClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Destroy (void) override;
	void		On_Command (int ctrl_id, int mesage_id, unsigned int param) override;

	//
	//	Singleton access
	//
	static void									Display (TAB_ID tab_id = TAB_NONE);
	static EVAEncyclopediaMenuClass *	Get_Instance (void)	{ return _TheInstance; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////	
	void		Prompt_User (void);
	void		HandleNotification (DlgMsgBoxEvent &event) override;
	void		Exit_Game (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	static EVAEncyclopediaMenuClass *	_TheInstance;
	static int									_NextTabIndex;
};


#endif //__DLGEVAENCYCLOPEDIA_H

